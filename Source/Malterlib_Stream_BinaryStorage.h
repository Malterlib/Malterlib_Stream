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

#if DMibConfig_IoDebug_Enable
	extern NAtomic::TCAtomic<uint64> g_BinaryStorageRangeCopyBytes;
	extern NAtomic::TCAtomic<uint64> g_BinaryStorageFeedCopyBytes;
	extern NAtomic::TCAtomic<uint64> g_BinaryStorageFeedConstCopyBytes;
	extern NAtomic::TCAtomic<uint64> g_BinaryStorageConsumeCopyBytes;
#endif

	// Arena offsets survive reallocation; derived pointers are valid only until the next storage mutation.
	struct CBinaryStorageLocalReference
	{
		umint m_Start;
		umint m_Length;
	};

	// A byte stream of local arena ranges, adopted buffers, shared buffers and nested storages.
	// Adopted and nested content must remain frozen while concurrent consumers can access it.
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
		void f_AppendRangeOf(CBinaryStorage const &_Source, umint _Offset, umint _nBytes);
		void f_TruncateTail(umint _nBytes);
		void f_Clear();

		umint f_GetTotalLength() const;
		bool f_IsEmpty() const;

		uint8 f_GetByte(umint _Offset) const;
		void f_CopyTo(void *_pDest, umint _Offset, umint _nBytes) const;
		bool f_TryGetSharedView(umint _Offset, umint _nBytes, NContainer::CSharedByteVector &o_View) const;
		uint8 *f_GetMutableLocalSpan(umint _Offset, umint _nBytes);

		umint f_GetSpanCount() const;
		umint f_GetSegmentCount() const;
		umint f_GetArenaLength() const;

		NContainer::CIOByteVector f_Flatten() const;

		template <typename tf_FVisitor>
		void f_VisitSpans(tf_FVisitor &&_fVisitor) const;

		static constexpr umint mc_AdoptThreshold = 128; // Below this size, copying costs less than segment bookkeeping.

	private:
		// Nested kinds cannot occupy the inline slot because CBinaryStorage is still incomplete here.
		// If the first segment is nested, this slot stays unused to preserve segment order.
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
		CBinaryStorageLocalReference *mp_pTailLocal = nullptr; // Points into the inline slot or segment vector; segment mutations and moves must refresh it.
	};

	// Write stream over segmented storage. Back-patching is limited to a single local arena span; adopted content is frozen.
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

		void f_FeedBytesAdopt(NContainer::CIOByteVector &&_Vector);
		void f_FeedShared(NContainer::CSharedByteVector &&_Shared);
		void f_FeedStorage(CBinaryStorage &&_Storage);
		void f_FeedStorageShared(NStorage::TCSharedPointer<CBinaryStorage const> &&_pStorage);
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

	// The storage must outlive the read stream and remain unmodified while it is open.
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
		NContainer::CSharedByteVector f_ConsumeShared(umint _nBytes);
		NStream::CBinaryStorage f_ConsumeStorage(umint _nBytes);

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
		CBinaryStorage const *mp_pStorage = nullptr; // Null until opened.
		umint mp_OpenOffset = 0; // storage offset of logical position 0
		umint mp_Position = 0;
		umint mp_Length = 0;
		uint8 const *mp_pCurrent = nullptr;
		umint mp_nCurrentRemaining = 0;
		umint mp_iCurrentSpan = 0;
	};
}

#include "Malterlib_Stream_BinaryStorage.hpp"
