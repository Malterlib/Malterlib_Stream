// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "Malterlib_Stream_BinaryStorage.h"

namespace NMib::NStream
{
#if DMibConfig_IoDebug_Enable
	NAtomic::TCAtomic<uint64> g_BinaryStorageRangeCopyBytes = 0;
	NAtomic::TCAtomic<uint64> g_BinaryStorageFeedCopyBytes = 0;
	NAtomic::TCAtomic<uint64> g_BinaryStorageFeedConstCopyBytes = 0;
	NAtomic::TCAtomic<uint64> g_BinaryStorageConsumeCopyBytes = 0;
#endif

	// The tail pointer may point at the inline first segment, which lives inside the moved from
	// object, so it is retargeted at this object's slot rather than carried across
	CBinaryStorage::CBinaryStorage(CBinaryStorage &&_Other)
		: mp_Storage(fg_Move(_Other.mp_Storage))
		, mp_FirstSegment(fg_Move(_Other.mp_FirstSegment))
		, mp_ByteStream(fg_Move(_Other.mp_ByteStream))
		, mp_TotalLength(_Other.mp_TotalLength)
		, mp_pTailLocal(_Other.mp_pTailLocal == &_Other.mp_FirstSegment.m_Local ? &mp_FirstSegment.m_Local : _Other.mp_pTailLocal)
	{
		_Other.mp_FirstSegment = {};
		_Other.mp_TotalLength = 0;
		_Other.mp_pTailLocal = nullptr;
	}

	CBinaryStorage &CBinaryStorage::operator = (CBinaryStorage &&_Other)
	{
		mp_Storage = fg_Move(_Other.mp_Storage);
		mp_FirstSegment = fg_Move(_Other.mp_FirstSegment);
		mp_ByteStream = fg_Move(_Other.mp_ByteStream);
		mp_TotalLength = _Other.mp_TotalLength;
		mp_pTailLocal = _Other.mp_pTailLocal == &_Other.mp_FirstSegment.m_Local ? &mp_FirstSegment.m_Local : _Other.mp_pTailLocal;
		_Other.mp_FirstSegment = {};
		_Other.mp_TotalLength = 0;
		_Other.mp_pTailLocal = nullptr;

		return *this;
	}

	umint CBinaryStorage::f_GetTotalLength() const
	{
		return mp_TotalLength;
	}

	bool CBinaryStorage::f_IsEmpty() const
	{
		return mp_TotalLength == 0;
	}

	umint CBinaryStorage::f_GetArenaLength() const
	{
		return mp_Storage.f_GetLen();
	}

	umint CBinaryStorage::f_GetSegmentCount() const
	{
		return (mp_FirstSegment.m_Kind != CFirstSegment::EKind::mc_None ? 1 : 0) + mp_ByteStream.f_GetLen();
	}

	bool CBinaryStorage::fp_HasSegments() const
	{
		return mp_FirstSegment.m_Kind != CFirstSegment::EKind::mc_None || !mp_ByteStream.f_IsEmpty();
	}

	umint CBinaryStorage::fp_GetFirstSegmentLength() const
	{
		if (mp_FirstSegment.m_Kind == CFirstSegment::EKind::mc_Local)
			return mp_FirstSegment.m_Local.m_Length;

		if (mp_FirstSegment.m_Kind == CFirstSegment::EKind::mc_Vector)
			return mp_FirstSegment.m_Vector.f_GetLen();

		if (mp_FirstSegment.m_Kind == CFirstSegment::EKind::mc_Shared)
			return mp_FirstSegment.m_Shared.f_GetLen();

		return 0;
	}

	CBinaryStorage::CBinaryStorage(NContainer::CIOByteVector &&_Vector)
	{
		f_AppendVector(fg_Move(_Vector));
	}

	static umint fsg_GetBinaryStorageSegmentLength(CBinaryStorage::CSegment const &_Segment)
	{
		umint Length = 0;
		_Segment.f_Visit
			(
				[&](auto const &_Data)
				{
					using CData = NTraits::TCRemoveReferenceAndQualifiers<decltype(_Data)>;

					if constexpr (NTraits::cIsSame<CData, CBinaryStorageLocalReference>)
						Length = _Data.m_Length;
					else if constexpr (NTraits::cIsSame<CData, NContainer::CIOByteVector>)
						Length = _Data.f_GetLen();
					else if constexpr (NTraits::cIsSame<CData, NContainer::CSharedByteVector>)
						Length = _Data.f_GetLen();
					else if constexpr (NTraits::cIsSame<CData, NStorage::TCIndirection<CBinaryStorage>>)
						Length = _Data.f_Get().f_GetTotalLength();
					else
					{
						static_assert(NTraits::cIsSame<CData, NStorage::TCSharedPointer<CBinaryStorage const>>);
						if (_Data)
							Length = _Data->f_GetTotalLength();
					}
				}
			)
		;

		return Length;
	}

	void CBinaryStorage::fp_RemoveTailSegment()
	{
		if (!mp_ByteStream.f_IsEmpty())
			mp_ByteStream.f_PopBack();
		else
			mp_FirstSegment = {};

		if (!mp_ByteStream.f_IsEmpty())
			mp_pTailLocal = mp_ByteStream.f_GetLast().f_TryGetAsType<CBinaryStorageLocalReference>();
		else if (mp_FirstSegment.m_Kind == CFirstSegment::EKind::mc_Local)
			mp_pTailLocal = &mp_FirstSegment.m_Local;
		else
			mp_pTailLocal = nullptr;
	}

	void CBinaryStorage::fp_CheckTotalLengthAdd(umint _nBytes) const
	{
		if (_nBytes > TCLimitsInt<umint>::mc_Max - mp_TotalLength)
			DMibError("Binary storage length overflow");

		// The streams hand the logical length out as a signed CFilePos, so a length past that
		// range would surface as a negative length and break every seek
		if constexpr (sizeof(umint) >= sizeof(NStream::CFilePos))
		{
			if (_nBytes > umint(TCLimitsInt<NStream::CFilePos>::mc_Max) - mp_TotalLength)
				DMibError("Binary storage length exceeds the stream position range");
		}
	}

	void CBinaryStorage::fp_AppendLocalReference(umint _Start, umint _nBytes)
	{
		if (mp_pTailLocal && mp_pTailLocal->m_Start + mp_pTailLocal->m_Length == _Start)
		{
			mp_pTailLocal->m_Length += _nBytes;
			mp_TotalLength += _nBytes;
			return;
		}

		if (!fp_HasSegments())
		{
			mp_FirstSegment.m_Kind = CFirstSegment::EKind::mc_Local;
			mp_FirstSegment.m_Local = CBinaryStorageLocalReference{.m_Start = _Start, .m_Length = _nBytes};
			mp_pTailLocal = &mp_FirstSegment.m_Local;
		}
		else
		{
			mp_ByteStream.f_InsertLast(CSegment{CBinaryStorageLocalReference{.m_Start = _Start, .m_Length = _nBytes}});
			mp_pTailLocal = mp_ByteStream.f_GetLast().f_TryGetAsType<CBinaryStorageLocalReference>();
		}

		mp_TotalLength += _nBytes;
	}

	void CBinaryStorage::f_AppendBytes(void const *_pData, umint _nBytes)
	{
		fp_CheckTotalLengthAdd(_nBytes);

		if (!_nBytes)
			return;

		umint Start = mp_Storage.f_GetLen();
		mp_Storage.f_Insert((uint8 const *)_pData, _nBytes);

		fp_AppendLocalReference(Start, _nBytes);
	}

	void CBinaryStorage::f_AppendZeroed(umint _nBytes)
	{
		fp_CheckTotalLengthAdd(_nBytes);

		if (!_nBytes)
			return;

		umint Start = mp_Storage.f_GetLen();
		mp_Storage.f_SetLen(Start + _nBytes);
		NMemory::fg_SecureMemClear(mp_Storage.f_GetArray() + Start, _nBytes);

		fp_AppendLocalReference(Start, _nBytes);
	}

	void CBinaryStorage::f_TruncateTail(umint _nBytes)
	{
		if (!_nBytes)
			return;

		if (!fp_HasSegments())
			DMibError("Binary storage truncation on empty storage");

		if (!mp_pTailLocal || mp_pTailLocal->m_Length < _nBytes)
			DMibError("Binary storage truncation must stay within a trailing local storage segment");

		mp_pTailLocal->m_Length -= _nBytes;
		mp_Storage.f_SetLen(mp_Storage.f_GetLen() - _nBytes);
		mp_TotalLength -= _nBytes;

		if (!mp_pTailLocal->m_Length)
			fp_RemoveTailSegment();
	}

	void CBinaryStorage::f_AppendVector(NContainer::CIOByteVector &&_Vector)
	{
		umint nBytes = _Vector.f_GetLen();
		if (!nBytes)
			return;

		fp_CheckTotalLengthAdd(nBytes);

		if (nBytes < mc_AdoptThreshold)
		{
			f_AppendBytes(_Vector.f_GetArray(), nBytes);
			return;
		}

		if (!fp_HasSegments())
		{
			mp_FirstSegment.m_Kind = CFirstSegment::EKind::mc_Vector;
			mp_FirstSegment.m_Vector = fg_Move(_Vector);
		}
		else
			mp_ByteStream.f_InsertLast(CSegment{fg_Move(_Vector)});

		mp_pTailLocal = nullptr;
		mp_TotalLength += nBytes;
	}

	void CBinaryStorage::f_AppendShared(NContainer::CSharedByteVector &&_Shared)
	{
		umint nBytes = _Shared.f_GetLen();
		if (!nBytes)
			return;

		fp_CheckTotalLengthAdd(nBytes);

		// A slice that continues the trailing shared segment extends it in place, so
		// receive paths appending consecutive pieces of one kernel buffer end up with a
		// single view instead of a segment per piece
		{
			NContainer::CSharedByteVector *pTailShared = nullptr;
			if (!mp_ByteStream.f_IsEmpty())
				pTailShared = mp_ByteStream.f_GetLast().f_TryGetAsType<NContainer::CSharedByteVector>();
			else if (mp_FirstSegment.m_Kind == CFirstSegment::EKind::mc_Shared)
				pTailShared = &mp_FirstSegment.m_Shared;

			if (pTailShared && pTailShared->f_TryAppendContiguous(_Shared))
			{
				mp_TotalLength += nBytes;
				return;
			}
		}

		if (nBytes < mc_AdoptThreshold)
		{
			f_AppendBytes(_Shared.f_GetArray(), nBytes);
			return;
		}

		if (!fp_HasSegments())
		{
			mp_FirstSegment.m_Kind = CFirstSegment::EKind::mc_Shared;
			mp_FirstSegment.m_Shared = fg_Move(_Shared);
		}
		else
			mp_ByteStream.f_InsertLast(CSegment{fg_Move(_Shared)});

		mp_pTailLocal = nullptr;
		mp_TotalLength += nBytes;
	}

	void CBinaryStorage::f_AppendStorage(CBinaryStorage &&_Storage)
	{
		umint nBytes = _Storage.f_GetTotalLength();
		if (!nBytes)
			return;

		fp_CheckTotalLengthAdd(nBytes);

		if (nBytes < mc_AdoptThreshold)
		{
			// A tiny nested storage costs more in indirection and per segment bookkeeping
			// than copying its bytes into the arena
			_Storage.f_VisitSpans
				(
					[&](uint8 const *_pData, umint _nSpanBytes)
					{
						f_AppendBytes(_pData, _nSpanBytes);
					}
				)
			;
			return;
		}

		mp_ByteStream.f_InsertLast(CSegment{NStorage::TCIndirection<CBinaryStorage>(fg_Move(_Storage))});
		mp_pTailLocal = nullptr;
		mp_TotalLength += nBytes;
	}

	void CBinaryStorage::f_AppendStorageShared(NStorage::TCSharedPointer<CBinaryStorage const> &&_pStorage)
	{
		if (!_pStorage)
			return;

		umint nBytes = _pStorage->f_GetTotalLength();
		if (!nBytes)
			return;

		fp_CheckTotalLengthAdd(nBytes);

		mp_ByteStream.f_InsertLast(CSegment{fg_Move(_pStorage)});
		mp_pTailLocal = nullptr;
		mp_TotalLength += nBytes;
	}

	void CBinaryStorage::f_AppendRangeOf(CBinaryStorage const &_Source, umint _Offset, umint _nBytes)
	{
		DMibFastCheck(&_Source != this);

		if (!_nBytes)
			return;

		// Subtraction form so an offset plus length that wraps cannot pass as in range
		if (_Offset > _Source.mp_TotalLength || _nBytes > _Source.mp_TotalLength - _Offset)
			DMibError("Binary storage range to append is out of range");

		umint RangeEnd = _Offset + _nBytes;
		umint Position = 0;

		// One segment's overlap with the range: shared content becomes a sub view carrying
		// the same owner, nested storages recurse so their shared content stays referenced,
		// and arena or adopted bytes are copied — they have no owner to hang a view on
		auto fAppendSegmentRange = [&](CSegment const &_Segment, umint _SegmentLength)
			{
				umint SegmentEnd = Position + _SegmentLength;
				if (SegmentEnd <= _Offset || Position >= RangeEnd || !_SegmentLength)
					return;

				umint Begin = _Offset > Position ? _Offset - Position : 0;
				umint End = RangeEnd < SegmentEnd ? RangeEnd - Position : _SegmentLength;
				umint nPart = End - Begin;

				_Segment.f_Visit
					(
						[&](auto const &_Data)
						{
							using CData = NTraits::TCRemoveReferenceAndQualifiers<decltype(_Data)>;

							if constexpr (NTraits::cIsSame<CData, CBinaryStorageLocalReference>)
							{
#if DMibConfig_IoDebug_Enable
								g_BinaryStorageRangeCopyBytes.f_FetchAdd(nPart, NAtomic::gc_MemoryOrder_Relaxed);
#endif
								f_AppendBytes(_Source.mp_Storage.f_GetArray() + _Data.m_Start + Begin, nPart);
							}
							else if constexpr (NTraits::cIsSame<CData, NContainer::CIOByteVector>)
							{
#if DMibConfig_IoDebug_Enable
								g_BinaryStorageRangeCopyBytes.f_FetchAdd(nPart, NAtomic::gc_MemoryOrder_Relaxed);
#endif
								f_AppendBytes(_Data.f_GetArray() + Begin, nPart);
							}
							else if constexpr (NTraits::cIsSame<CData, NContainer::CSharedByteVector>)
								f_AppendShared(NContainer::CSharedByteVector(_Data, Begin, nPart));
							else if constexpr (NTraits::cIsSame<CData, NStorage::TCIndirection<CBinaryStorage>>)
								f_AppendRangeOf(_Data.f_Get(), Begin, nPart);
							else
							{
								static_assert(NTraits::cIsSame<CData, NStorage::TCSharedPointer<CBinaryStorage const>>);
								if (!Begin && nPart == _Data->f_GetTotalLength())
									f_AppendStorageShared(NStorage::TCSharedPointer<CBinaryStorage const>(_Data));
								else
									f_AppendRangeOf(*_Data, Begin, nPart);
							}
						}
					)
				;
			}
		;

		if (_Source.mp_FirstSegment.m_Kind != CFirstSegment::EKind::mc_None)
		{
			umint FirstLength = _Source.fp_GetFirstSegmentLength();
			umint SegmentEnd = FirstLength;
			if (SegmentEnd > _Offset && FirstLength)
			{
				umint Begin = _Offset;
				umint End = RangeEnd < SegmentEnd ? RangeEnd : FirstLength;
				umint nPart = End - Begin;

				if (_Source.mp_FirstSegment.m_Kind == CFirstSegment::EKind::mc_Local)
				{
#if DMibConfig_IoDebug_Enable
					g_BinaryStorageRangeCopyBytes.f_FetchAdd(nPart, NAtomic::gc_MemoryOrder_Relaxed);
#endif
					f_AppendBytes(_Source.mp_Storage.f_GetArray() + _Source.mp_FirstSegment.m_Local.m_Start + Begin, nPart);
				}
				else if (_Source.mp_FirstSegment.m_Kind == CFirstSegment::EKind::mc_Vector)
				{
#if DMibConfig_IoDebug_Enable
					g_BinaryStorageRangeCopyBytes.f_FetchAdd(nPart, NAtomic::gc_MemoryOrder_Relaxed);
#endif
					f_AppendBytes(_Source.mp_FirstSegment.m_Vector.f_GetArray() + Begin, nPart);
				}
				else
					f_AppendShared(NContainer::CSharedByteVector(_Source.mp_FirstSegment.m_Shared, Begin, nPart));
			}
			Position = FirstLength;
		}

		for (CSegment const &Segment : _Source.mp_ByteStream)
		{
			if (Position >= RangeEnd)
				break;

			umint SegmentLength = fsg_GetBinaryStorageSegmentLength(Segment);
			fAppendSegmentRange(Segment, SegmentLength);
			Position += SegmentLength;
		}
	}

	void CBinaryStorage::f_Clear()
	{
		mp_Storage.f_Clear();
		mp_FirstSegment = {};
		mp_ByteStream.f_Clear();
		mp_TotalLength = 0;
		mp_pTailLocal = nullptr;
	}

	void CBinaryStorage::f_CopyTo(void *_pDest, umint _Offset, umint _nBytes) const
	{
		// Subtraction form so an offset plus length that wraps cannot pass as in range
		if (_Offset > mp_TotalLength || _nBytes > mp_TotalLength - _Offset)
			DMibError("Binary storage copy range exceeds total length");

		uint8 *pDest = (uint8 *)_pDest;
		umint Position = 0;
		umint nRemaining = _nBytes;

		f_VisitSpans
			(
				[&](uint8 const *_pData, umint _nSpanBytes)
				{
					if (!nRemaining)
						return;

					umint SpanEnd = Position + _nSpanBytes;
					if (SpanEnd > _Offset)
					{
						umint SourceStart = _Offset > Position ? _Offset - Position : 0;
						umint nToCopy = fg_Min(_nSpanBytes - SourceStart, nRemaining);
						NMemory::fg_ObjectCopy(pDest, _pData + SourceStart, nToCopy);
						pDest += nToCopy;
						nRemaining -= nToCopy;
					}
					Position = SpanEnd;
				}
			)
		;
	}

	bool CBinaryStorage::f_TryGetSharedView(umint _Offset, umint _nBytes, NContainer::CSharedByteVector &o_View) const
	{
		// Subtraction form so an offset plus length that wraps cannot pass as in range
		if (!_nBytes || _Offset > mp_TotalLength || _nBytes > mp_TotalLength - _Offset)
			return false;

		umint Position = 0;

		if (mp_FirstSegment.m_Kind != CFirstSegment::EKind::mc_None)
		{
			umint FirstLength = fp_GetFirstSegmentLength();
			if (_Offset < FirstLength)
			{
				if (mp_FirstSegment.m_Kind != CFirstSegment::EKind::mc_Shared || _nBytes > FirstLength - _Offset)
					return false;

				o_View = NContainer::CSharedByteVector(mp_FirstSegment.m_Shared, _Offset, _nBytes);
				return true;
			}
			Position = FirstLength;
		}

		for (CSegment const &Segment : mp_ByteStream)
		{
			umint SegmentLength = fsg_GetBinaryStorageSegmentLength(Segment);
			if (_Offset < Position + SegmentLength)
			{
				auto const *pShared = Segment.f_TryGetAsType<NContainer::CSharedByteVector>();
				if (!pShared || _nBytes > Position + SegmentLength - _Offset)
					return false;

				o_View = NContainer::CSharedByteVector(*pShared, _Offset - Position, _nBytes);
				return true;
			}
			Position += SegmentLength;
		}

		return false;
	}

	uint8 CBinaryStorage::f_GetByte(umint _Offset) const
	{
		uint8 Byte;
		f_CopyTo(&Byte, _Offset, 1);
		return Byte;
	}

	uint8 *CBinaryStorage::f_GetMutableLocalSpan(umint _Offset, umint _nBytes)
	{
		umint Position = 0;

		if (mp_FirstSegment.m_Kind != CFirstSegment::EKind::mc_None)
		{
			umint FirstLength = fp_GetFirstSegmentLength();
			if (_Offset < FirstLength)
			{
				if (mp_FirstSegment.m_Kind != CFirstSegment::EKind::mc_Local)
					DMibError("Binary storage range to patch is not local storage");

				if (_nBytes > FirstLength - _Offset)
					DMibError("Binary storage range to patch spans multiple segments");

				return mp_Storage.f_GetArray() + mp_FirstSegment.m_Local.m_Start + _Offset;
			}
			Position = FirstLength;
		}

		for (CSegment &Segment : mp_ByteStream)
		{
			umint SegmentLength = fsg_GetBinaryStorageSegmentLength(Segment);
			if (_Offset < Position + SegmentLength)
			{
				auto *pLocal = Segment.f_TryGetAsType<CBinaryStorageLocalReference>();
				if (!pLocal)
					DMibError("Binary storage range to patch is not local storage");

				if (_nBytes > Position + SegmentLength - _Offset)
					DMibError("Binary storage range to patch spans multiple segments");

				return mp_Storage.f_GetArray() + pLocal->m_Start + (_Offset - Position);
			}
			Position += SegmentLength;
		}

		DMibError("Binary storage range to patch is outside the storage");
	}

	umint CBinaryStorage::f_GetSpanCount() const
	{
		umint nSpans = 0;
		f_VisitSpans
			(
				[&](uint8 const *, umint)
				{
					++nSpans;
				}
			)
		;

		return nSpans;
	}

	NContainer::CIOByteVector CBinaryStorage::f_Flatten() const
	{
		NContainer::CIOByteVector Result;
		Result.f_Reserve(mp_TotalLength);

		f_VisitSpans
			(
				[&](uint8 const *_pData, umint _nBytes)
				{
					Result.f_Insert(_pData, _nBytes);
				}
			)
		;

		return Result;
	}
}
