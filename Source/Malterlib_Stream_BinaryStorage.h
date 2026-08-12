// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>
#include <Mib/Atomic/Atomic>
#include <Mib/Storage/Variant>
#include <Mib/Storage/Indirection>
#include <Mib/Storage/SharedPointer>
#include <Mib/Container/SharedByteVector>

namespace NMib::NStream
{
	struct CBinaryStorage;

	// Diagnostic counters for the storage zero copy paths: bytes that fell back to a copy
	// when a range was appended from another storage, and when a storage was fed to or
	// consumed from a stream without adopt/consume support. Relaxed adds on the copy paths
	// only; read by the io statistics dumps
#if DMibConfig_IoDebug_Enable
	extern NAtomic::TCAtomic<uint64> g_BinaryStorageRangeCopyBytes;
	extern NAtomic::TCAtomic<uint64> g_BinaryStorageFeedCopyBytes;
	extern NAtomic::TCAtomic<uint64> g_BinaryStorageFeedConstCopyBytes;
	extern NAtomic::TCAtomic<uint64> g_BinaryStorageConsumeCopyBytes;
#endif

	// References a range of the local arena (CBinaryStorage::m_Storage). Offsets stay valid
	// across arena reallocation; pointers derived from them are only valid until the next
	// mutation of the storage
	struct CBinaryStorageLocalReference
	{
		umint m_Start;
		umint m_Length;
	};

	// A logical byte stream stored as a sequence of segments: ranges of a local append
	// arena interleaved with adopted buffers, shared buffers and nested storages. This lets
	// serialization reference large payloads instead of copying them, while headers and
	// small data still land contiguously in the arena.
	//
	// Move-only by design: normal forwarding moves the storage (nesting via
	// TCIndirection moves a pointer); TCSharedPointer<CBinaryStorage const> is reserved for
	// storages that are cached and sent several times. Adopted and nested content is frozen
	// - it must not be mutated after adoption since consumers may read it concurrently
	struct CBinaryStorage
	{
		using CSegment = NStorage::TCVariant
			<
				CBinaryStorageLocalReference
				, NContainer::CIOByteVector
				, NContainer::CSharedByteVector
				, NStorage::TCIndirection<CBinaryStorage>
				, NStorage::TCSharedPointer<CBinaryStorage const>
			>
		;

		CBinaryStorage() = default;
		explicit CBinaryStorage(NContainer::CIOByteVector &&_Vector);

		CBinaryStorage(CBinaryStorage &&_Other);

		CBinaryStorage &operator = (CBinaryStorage &&_Other);

		CBinaryStorage(CBinaryStorage const &) = delete;
		CBinaryStorage &operator = (CBinaryStorage const &) = delete;

		void f_AppendBytes(void const *_pData, umint _nBytes);
		void f_AppendZeroed(umint _nBytes);
		void f_AppendVector(NContainer::CIOByteVector &&_Vector);
		void f_AppendShared(NContainer::CSharedByteVector &&_Shared);
		void f_AppendStorage(CBinaryStorage &&_Storage);
		void f_AppendStorageShared(NStorage::TCSharedPointer<CBinaryStorage const> &&_pStorage);

		// Appends [_Offset, _Offset + _nBytes) of _Source: ranges of shared segments become
		// sub views holding the same owner, everything else (arena, adopted vectors) is
		// copied. Nested storages are recursed so their shared content stays referenced
		void f_AppendRangeOf(CBinaryStorage const &_Source, umint _Offset, umint _nBytes);

		// Removes _nBytes from the logical end. The removed range must lie wholly inside a
		// trailing local arena segment; adopted and nested content cannot be truncated
		void f_TruncateTail(umint _nBytes);

		void f_Clear();

		umint f_GetTotalLength() const;

		bool f_IsEmpty() const;

		uint8 f_GetByte(umint _Offset) const;
		void f_CopyTo(void *_pDest, umint _Offset, umint _nBytes) const;

		// Returns true with a view of [_Offset, _Offset + _nBytes) when that range lies
		// wholly inside a single shared vector segment. Receive paths use this to hand a
		// packet payload onwards without copying it; the view keeps the shared buffer alive
		bool f_TryGetSharedView(umint _Offset, umint _nBytes, NContainer::CSharedByteVector &o_View) const;

		// Mutable access for back-patching already written header bytes. The range must lie
		// wholly inside a single local arena segment; adopted and nested content is frozen.
		// The returned pointer is only valid until the next mutation of the storage
		uint8 *f_GetMutableLocalSpan(umint _Offset, umint _nBytes);

		umint f_GetSpanCount() const;

		// Number of segments without recursing into nested storages (the inline first
		// segment plus the overflow vector)
		umint f_GetSegmentCount() const;

		// Bytes currently held in the local append arena, for tests that assert on the adopt
		// threshold picking arena copies over segments
		umint f_GetArenaLength() const;

		NContainer::CIOByteVector f_Flatten() const;

		// Calls _fVisitor(uint8 const *_pData, umint _nBytes) for every non-empty flattened
		// span in logical order, recursing into nested storages
		template <typename tf_FVisitor>
		void f_VisitSpans(tf_FVisitor &&_fVisitor) const;

		// Adopted vectors below this size are copied into the arena instead: per-segment
		// bookkeeping and scattered framing cost more than a short copy
		static constexpr umint mc_AdoptThreshold = 128;

	private:
		// Inline slot for the first segment so single segment storages (small all-arena
		// packets, wrapped buffers) never allocate the segment vector. Nested storage kinds
		// cannot live here since they contain CBinaryStorage, which is still incomplete at
		// this point. The slot is only used while the storage has no segments; a first
		// segment of a nested kind leaves it unused so segment order is preserved
		struct CFirstSegment
		{
			enum class EKind : uint8
			{
				mc_None
				, mc_Local
				, mc_Vector
				, mc_Shared
			};

			EKind m_Kind = EKind::mc_None;
			CBinaryStorageLocalReference m_Local = {};
			NContainer::CIOByteVector m_Vector;
			NContainer::CSharedByteVector m_Shared;
		};

		void fp_AppendLocalReference(umint _Start, umint _nBytes);

		// Shared and nested segments can be appended repeatedly, so the logical length can grow
		// without any matching allocation. Every append checks before it mutates: throwing after
		// a segment is inserted would leave bytes that the length does not count
		void fp_CheckTotalLengthAdd(umint _nBytes) const;

		bool fp_HasSegments() const;

		umint fp_GetFirstSegmentLength() const;

		void fp_RemoveTailSegment();

		template <typename tf_FVisitor>
		void fp_VisitSegmentSpans(CSegment const &_Segment, tf_FVisitor &_fVisitor) const;

		NContainer::CIOByteVector mp_Storage;
		CFirstSegment mp_FirstSegment;
		NContainer::TCVector<CSegment> mp_ByteStream;
		umint mp_TotalLength = 0;

		// Cached pointer to the trailing local reference when the tail segment is one, so
		// sequential arena writes extend it without touching the segment vector. Points into
		// the inline first segment or the vector; invalidated by any segment push/pop and by
		// moves, which the mutation paths maintain
		CBinaryStorageLocalReference *mp_pTailLocal = nullptr;
	};

	// Write stream that serializes into a CBinaryStorage: sequential writes land in the
	// local arena, while adopted buffers, shared buffers and nested storages become
	// segments referenced in place. Positions and lengths are logical across all segments.
	// Rewriting already written bytes (back-patching headers) is only supported wholly
	// inside a single local arena span; adopted content is frozen
	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	struct TCBinaryStreamStorage : public t_CStreamType
	{
		DMibStreamImplementOperators(TCBinaryStreamStorage);

		TCBinaryStreamStorage() = default;

		explicit TCBinaryStreamStorage(CBinaryStorage &&_Storage);
		TCBinaryStreamStorage(TCBinaryStreamStorage &&_ToMove);
		TCBinaryStreamStorage &operator = (TCBinaryStreamStorage &&_ToMove);

		TCBinaryStreamStorage(TCBinaryStreamStorage const &) = delete;
		TCBinaryStreamStorage &operator = (TCBinaryStreamStorage const &) = delete;

		CBinaryStorage &f_GetStorage();
		CBinaryStorage const &f_GetStorage() const;
		CBinaryStorage f_MoveStorage();
		void f_ResetStream();

		// Adopt customization points: reference the buffer as a segment instead of copying.
		// Adopting appends a segment, so it only expresses the caller's intent at the end of the
		// stream. Callers that may be positioned inside existing content test f_CanAdopt() and
		// copy instead; reaching these at an interior position is a programming error
		void f_FeedBytesAdopt(NContainer::CIOByteVector &&_Vector);
		void f_FeedShared(NContainer::CSharedByteVector &&_Shared);
		void f_FeedStorage(CBinaryStorage &&_Storage);
		void f_FeedStorageShared(NStorage::TCSharedPointer<CBinaryStorage const> &&_pStorage);

		// True when an adopting feed is valid at the current position. Serialization glue that can
		// run either at the end or as an in place rewrite tests this to pick adopting or copying
		bool f_CanAdopt() const;

		void f_FeedBytes(const void *_pMem, umint _nBytes);
		void f_ConsumeBytes(void *_pMem, umint _nBytes);

		bool f_IsValid() const;
		bool f_IsAtEndOfStream() const;
		CFilePos f_GetPosition() const;
		void f_SetPosition(CFilePos _Pos);
		void f_SetPositionFromEnd(CFilePos _Pos);
		void f_AddPosition(CFilePos _Pos);
		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const;
		void f_Flush(bool _bLocalCacheOnly);
		void f_SetCacheSize(umint _CacheSize);
		CFilePos f_GetLength() const;
		umint f_ContainerLengthLimit() const;
		void f_SetLength(NStream::CFilePos _Length);

	protected:
		void fp_SetPositionInternal(CFilePos _Pos);

		DMibStreamImplementProtected(TCBinaryStreamStorage);

		umint mp_Position = 0;
		CBinaryStorage mp_Storage;

	private:
		void fp_CheckAtEndForAdopt();
	};

	// Read-only stream over a frozen CBinaryStorage. The segment tree is flattened to a
	// span table at open, with the current span cached so sequential reads advance without
	// searching. The storage must outlive the stream and stay unmodified while open
	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	struct TCBinaryStreamStoragePtr : public t_CStreamType
	{
		DMibStreamImplementOperators(TCBinaryStreamStoragePtr);

		TCBinaryStreamStoragePtr() = default;

		TCBinaryStreamStoragePtr(TCBinaryStreamStoragePtr const &) = delete;
		TCBinaryStreamStoragePtr &operator = (TCBinaryStreamStoragePtr const &) = delete;

		void f_OpenRead(CBinaryStorage const &_Storage);
		void f_OpenRead(CBinaryStorage const &_Storage, umint _Offset, umint _Length);

		void f_FeedBytes(const void *_pMem, umint _nBytes);
		void f_ConsumeBytes(void *_pMem, umint _nBytes);

		// Consumes _nBytes as a view of the backing shared buffer when the range lies wholly
		// inside one shared segment, so shared payloads deserialize without copying; falls
		// back to an owned copy for arena and nested content
		NContainer::CSharedByteVector f_ConsumeShared(umint _nBytes);

		// Consumes _nBytes as a storage that references the backing storage's shared
		// content as sub views and copies the rest; disjoint payloads deserialize without
		// copying their shared spans
		NStream::CBinaryStorage f_ConsumeStorage(umint _nBytes);

		// A stream that was never opened has no storage to read, unlike the memory streams
		// that always own a usable buffer
		bool f_IsValid() const;
		bool f_IsAtEndOfStream() const;
		CFilePos f_GetPosition() const;
		void f_SetPosition(CFilePos _Pos);
		void f_SetPositionFromEnd(CFilePos _Pos);
		void f_AddPosition(CFilePos _Pos);
		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const;
		void f_Flush(bool _bLocalCacheOnly);
		void f_SetCacheSize(umint _CacheSize);
		CFilePos f_GetLength() const;
		umint f_ContainerLengthLimit() const;
		void f_SetLength(NStream::CFilePos _Length);

	protected:
		struct CSpan
		{
			uint8 const *m_pData;
			umint m_nBytes;
			umint m_LogicalStart;
		};

		void fp_SetPositionInternal(CFilePos _Pos);
		void fp_RebuildSpanCache();
		void fp_AdvanceToNextSpan();

		DMibStreamImplementProtected(TCBinaryStreamStoragePtr);

		NContainer::TCVector<CSpan> mp_Spans;
		CBinaryStorage const *mp_pStorage = nullptr; // null until opened; also backs f_ConsumeShared
		umint mp_OpenOffset = 0; // storage offset of logical position 0
		umint mp_Position = 0;
		umint mp_Length = 0;
		uint8 const *mp_pCurrent = nullptr;
		umint mp_nCurrentRemaining = 0;
		umint mp_iCurrentSpan = 0;
	};
}

#include "Malterlib_Stream_BinaryStorage.hpp"
