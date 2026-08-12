// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NStream
{
	// Calls _fVisitor(data, length) for nonempty spans in logical order, recursively flattening nested storages.
	template <typename tf_FVisitor>
	void CBinaryStorage::f_VisitSpans(tf_FVisitor &&_fVisitor) const
	{
		if (mp_FirstSegment.m_Kind == CFirstSegment::EKind::mc_Local)
		{
			if (mp_FirstSegment.m_Local.m_Length)
				_fVisitor(mp_Storage.f_GetArray() + mp_FirstSegment.m_Local.m_Start, mp_FirstSegment.m_Local.m_Length);
		}
		else if (mp_FirstSegment.m_Kind == CFirstSegment::EKind::mc_Vector)
		{
			if (mp_FirstSegment.m_Vector.f_GetLen())
				_fVisitor(mp_FirstSegment.m_Vector.f_GetArray(), mp_FirstSegment.m_Vector.f_GetLen());
		}
		else if (mp_FirstSegment.m_Kind == CFirstSegment::EKind::mc_Shared)
		{
			if (mp_FirstSegment.m_Shared.f_GetLen())
				_fVisitor(mp_FirstSegment.m_Shared.f_GetArray(), mp_FirstSegment.m_Shared.f_GetLen());
		}

		for (CSegment const &Segment : mp_ByteStream)
			fp_VisitSegmentSpans(Segment, _fVisitor);
	}

	template <typename tf_FVisitor>
	void CBinaryStorage::fp_VisitSegmentSpans(CSegment const &_Segment, tf_FVisitor &_fVisitor) const
	{
		_Segment.f_Visit
			(
				[&](auto const &_Data)
				{
					using CData = NTraits::TCRemoveReferenceAndQualifiers<decltype(_Data)>;

					if constexpr (NTraits::cIsSame<CData, CBinaryStorageLocalReference>)
					{
						if (_Data.m_Length)
							_fVisitor(mp_Storage.f_GetArray() + _Data.m_Start, _Data.m_Length);
					}
					else if constexpr (NTraits::cIsSame<CData, NContainer::CIOByteVector>)
					{
						if (_Data.f_GetLen())
							_fVisitor(_Data.f_GetArray(), _Data.f_GetLen());
					}
					else if constexpr (NTraits::cIsSame<CData, NContainer::CSharedByteVector>)
					{
						if (_Data.f_GetLen())
							_fVisitor(_Data.f_GetArray(), _Data.f_GetLen());
					}
					else if constexpr (NTraits::cIsSame<CData, NStorage::TCIndirection<CBinaryStorage>>)
					{
						_Data.f_Get().f_VisitSpans(_fVisitor);
					}
					else
					{
						static_assert(NTraits::cIsSame<CData, NStorage::TCSharedPointer<CBinaryStorage const>>);
						if (_Data)
							_Data->f_VisitSpans(_fVisitor);
					}
				}
			)
		;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::fp_SetPositionInternal(CFilePos _Pos)
	{
		if (_Pos < 0 || fg_SafeLargerThan(_Pos, umint(TCLimitsInt<umint>::mc_Max)))
			DMibError("Binary storage stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

		mp_Position = _Pos;
	}

	template <typename t_CStreamType>
	TCBinaryStreamStorage<t_CStreamType>::TCBinaryStreamStorage(CBinaryStorage &&_Storage)
		: mp_Position(_Storage.f_GetTotalLength())
		, mp_Storage(fg_Move(_Storage))
	{
	}

	template <typename t_CStreamType>
	TCBinaryStreamStorage<t_CStreamType>::TCBinaryStreamStorage(TCBinaryStreamStorage &&_ToMove)
		: mp_Position(_ToMove.mp_Position)
		, mp_Storage(fg_Move(_ToMove.mp_Storage))
	{
		_ToMove.mp_Position = 0;
	}

	template <typename t_CStreamType>
	TCBinaryStreamStorage<t_CStreamType> &TCBinaryStreamStorage<t_CStreamType>::operator = (TCBinaryStreamStorage &&_ToMove)
	{
		mp_Position = _ToMove.mp_Position;
		mp_Storage = fg_Move(_ToMove.mp_Storage);
		_ToMove.mp_Position = 0;

		return *this;
	}

	template <typename t_CStreamType>
	CBinaryStorage &TCBinaryStreamStorage<t_CStreamType>::f_GetStorage()
	{
		return mp_Storage;
	}

	template <typename t_CStreamType>
	CBinaryStorage const &TCBinaryStreamStorage<t_CStreamType>::f_GetStorage() const
	{
		return mp_Storage;
	}

	template <typename t_CStreamType>
	CBinaryStorage TCBinaryStreamStorage<t_CStreamType>::f_MoveStorage()
	{
		mp_Position = 0;

		return fg_Move(mp_Storage);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::f_ResetStream()
	{
		mp_Position = 0;
		mp_Storage.f_Clear();
	}

	// Adoption requires the stream position to be at its logical end.
	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::f_FeedBytesAdopt(NContainer::CIOByteVector &&_Vector)
	{
		fp_CheckAtEndForAdopt();
		mp_Storage.f_AppendVector(fg_Move(_Vector));
		mp_Position = mp_Storage.f_GetTotalLength();
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::f_FeedShared(NContainer::CSharedByteVector &&_Shared)
	{
		fp_CheckAtEndForAdopt();
		mp_Storage.f_AppendShared(fg_Move(_Shared));
		mp_Position = mp_Storage.f_GetTotalLength();
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::f_FeedStorage(CBinaryStorage &&_Storage)
	{
		fp_CheckAtEndForAdopt();
		mp_Storage.f_AppendStorage(fg_Move(_Storage));
		mp_Position = mp_Storage.f_GetTotalLength();
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::f_FeedStorageShared(NStorage::TCSharedPointer<CBinaryStorage const> &&_pStorage)
	{
		fp_CheckAtEndForAdopt();
		mp_Storage.f_AppendStorageShared(fg_Move(_pStorage));
		mp_Position = mp_Storage.f_GetTotalLength();
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamStorage<t_CStreamType>::f_CanAdopt() const
	{
		return mp_Position == mp_Storage.f_GetTotalLength();
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::f_FeedBytes(const void *_pMem, umint _nBytes)
	{
		if (!_nBytes)
			return;

		umint TotalLength = mp_Storage.f_GetTotalLength();
		if (mp_Position == TotalLength)
		{
			mp_Storage.f_AppendBytes(_pMem, _nBytes);
			mp_Position += _nBytes;
			return;
		}

		if (mp_Position > TotalLength || _nBytes > TotalLength - mp_Position)
			DMibError("Binary storage stream rewrites cannot extend past the end of the stream");

		NMemory::fg_MemCopy(mp_Storage.f_GetMutableLocalSpan(mp_Position, _nBytes), _pMem, _nBytes);
		mp_Position += _nBytes;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::f_ConsumeBytes(void *_pMem, umint _nBytes)
	{
		DMibError("Binary storage write streams are write only; open a TCBinaryStreamStoragePtr over the storage to read");
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamStorage<t_CStreamType>::f_IsValid() const
	{
		return true;
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamStorage<t_CStreamType>::f_IsAtEndOfStream() const
	{
		return mp_Position == mp_Storage.f_GetTotalLength();
	}

	template <typename t_CStreamType>
	CFilePos TCBinaryStreamStorage<t_CStreamType>::f_GetPosition() const
	{
		return mp_Position;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::f_SetPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(_Pos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::f_SetPositionFromEnd(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Storage.f_GetTotalLength() + _Pos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::f_AddPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Position + _Pos);
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamStorage<t_CStreamType>::f_IsValidReadPosition(NStream::CFilePos _Pos) const
	{
		return _Pos >= 0 && _Pos < NStream::CFilePos(mp_Storage.f_GetTotalLength());
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::f_Flush(bool _bLocalCacheOnly)
	{
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::f_SetCacheSize(umint _CacheSize)
	{
	}

	template <typename t_CStreamType>
	CFilePos TCBinaryStreamStorage<t_CStreamType>::f_GetLength() const
	{
		return mp_Storage.f_GetTotalLength();
	}

	template <typename t_CStreamType>
	umint TCBinaryStreamStorage<t_CStreamType>::f_ContainerLengthLimit() const
	{
		return mp_Storage.f_GetTotalLength() - mp_Position;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::f_SetLength(NStream::CFilePos _Length)
	{
		// The storage counts in umint; a length past it would narrow into the delta below
		if (_Length < 0 || fg_SafeLargerThan(_Length, umint(TCLimitsInt<umint>::mc_Max)))
			DMibError("Binary storage stream lengths are limited to 0 -> TCLimitsInt<umint>::mc_Max");

		umint TotalLength = mp_Storage.f_GetTotalLength();
		if (_Length > NStream::CFilePos(TotalLength))
			mp_Storage.f_AppendZeroed(_Length - TotalLength);
		else if (_Length < NStream::CFilePos(TotalLength))
			mp_Storage.f_TruncateTail(TotalLength - _Length);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStorage<t_CStreamType>::fp_CheckAtEndForAdopt()
	{
		if (!f_CanAdopt())
			DMibError("Binary storage stream buffers can only be adopted at the end of the stream");
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStoragePtr<t_CStreamType>::fp_SetPositionInternal(CFilePos _Pos)
	{
		if (_Pos < 0 || fg_SafeLargerThan(_Pos, mp_Length))
			DMibError("Position is outside the binary storage stream");

		mp_Position = _Pos;
		fp_RebuildSpanCache();
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStoragePtr<t_CStreamType>::fp_RebuildSpanCache()
	{
		umint iSpan = 0;
		umint nSpans = mp_Spans.f_GetLen();
		while (iSpan < nSpans && mp_Spans[iSpan].m_LogicalStart + mp_Spans[iSpan].m_nBytes <= mp_Position)
			++iSpan;

		mp_iCurrentSpan = iSpan;
		if (iSpan < nSpans)
		{
			CSpan const &Span = mp_Spans[iSpan];
			umint SpanOffset = mp_Position - Span.m_LogicalStart;
			mp_pCurrent = Span.m_pData + SpanOffset;
			mp_nCurrentRemaining = Span.m_nBytes - SpanOffset;
		}
		else
		{
			mp_pCurrent = nullptr;
			mp_nCurrentRemaining = 0;
		}
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStoragePtr<t_CStreamType>::fp_AdvanceToNextSpan()
	{
		++mp_iCurrentSpan;
		if (mp_iCurrentSpan < mp_Spans.f_GetLen())
		{
			CSpan const &Span = mp_Spans[mp_iCurrentSpan];
			mp_pCurrent = Span.m_pData;
			mp_nCurrentRemaining = Span.m_nBytes;
		}
		else
		{
			mp_pCurrent = nullptr;
			mp_nCurrentRemaining = 0;
		}
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStoragePtr<t_CStreamType>::f_OpenRead(CBinaryStorage const &_Storage)
	{
		f_OpenRead(_Storage, 0, _Storage.f_GetTotalLength());
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStoragePtr<t_CStreamType>::f_OpenRead(CBinaryStorage const &_Storage, umint _Offset, umint _Length)
	{
		if (_Offset > _Storage.f_GetTotalLength() || _Length > _Storage.f_GetTotalLength() - _Offset)
			DMibError("Binary storage stream range exceeds storage length");

		mp_Spans.f_Clear();
		mp_pStorage = &_Storage;
		mp_OpenOffset = _Offset;
		mp_Position = 0;
		mp_Length = _Length;

		umint SourcePosition = 0;
		umint LogicalStart = 0;
		umint RangeEnd = _Offset + _Length;

		_Storage.f_VisitSpans
			(
				[&](uint8 const *_pData, umint _nBytes)
				{
					umint SpanStart = SourcePosition;
					umint SpanEnd = SourcePosition + _nBytes;
					SourcePosition = SpanEnd;

					if (SpanEnd <= _Offset || SpanStart >= RangeEnd)
						return;

					umint ClipStart = _Offset > SpanStart ? _Offset - SpanStart : 0;
					umint ClipEnd = fg_Min(SpanEnd, RangeEnd) - SpanStart;

					mp_Spans.f_InsertLast(CSpan{.m_pData = _pData + ClipStart, .m_nBytes = ClipEnd - ClipStart, .m_LogicalStart = LogicalStart});
					LogicalStart += ClipEnd - ClipStart;
				}
			)
		;

		fp_RebuildSpanCache();
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStoragePtr<t_CStreamType>::f_FeedBytes(const void *_pMem, umint _nBytes)
	{
		DMibError("Binary storage read stream cannot be written to");
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStoragePtr<t_CStreamType>::f_ConsumeBytes(void *_pMem, umint _nBytes)
	{
		if (_nBytes > mp_Length - mp_Position) [[unlikely]]
			this->fp_ThrowEndOfStreamException();

		uint8 *pDest = (uint8 *)_pMem;
		umint nRemaining = _nBytes;

		while (nRemaining)
		{
			if (!mp_nCurrentRemaining)
				fp_AdvanceToNextSpan();

			umint nToCopy = fg_Min(nRemaining, mp_nCurrentRemaining);
			NMemory::fg_MemCopy(pDest, mp_pCurrent, nToCopy);

			pDest += nToCopy;
			nRemaining -= nToCopy;
			mp_pCurrent += nToCopy;
			mp_nCurrentRemaining -= nToCopy;
		}

		mp_Position += _nBytes;
	}

	template <typename t_CStreamType>
	NContainer::CSharedByteVector TCBinaryStreamStoragePtr<t_CStreamType>::f_ConsumeShared(umint _nBytes)
	{
		if (_nBytes > mp_Length - mp_Position) [[unlikely]]
			this->fp_ThrowEndOfStreamException();

		NContainer::CSharedByteVector View;
		if (mp_pStorage && mp_pStorage->f_TryGetSharedView(mp_OpenOffset + mp_Position, _nBytes, View))
		{
			fp_SetPositionInternal(mp_Position + _nBytes);
			return View;
		}

		NContainer::CIOByteVector Vector;
		Vector.f_SetLen(_nBytes);
		f_ConsumeBytes(Vector.f_GetArray(), _nBytes);

		return NContainer::CSharedByteVector(fg_Move(Vector));
	}

	template <typename t_CStreamType>
	NStream::CBinaryStorage TCBinaryStreamStoragePtr<t_CStreamType>::f_ConsumeStorage(umint _nBytes)
	{
		if (_nBytes > mp_Length - mp_Position) [[unlikely]]
			this->fp_ThrowEndOfStreamException();

		NStream::CBinaryStorage Storage;
		if (mp_pStorage)
		{
			Storage.f_AppendRangeOf(*mp_pStorage, mp_OpenOffset + mp_Position, _nBytes);
			fp_SetPositionInternal(mp_Position + _nBytes);
		}

		return Storage;
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamStoragePtr<t_CStreamType>::f_IsValid() const
	{
		return mp_pStorage != nullptr;
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamStoragePtr<t_CStreamType>::f_IsAtEndOfStream() const
	{
		return mp_Position == mp_Length;
	}

	template <typename t_CStreamType>
	CFilePos TCBinaryStreamStoragePtr<t_CStreamType>::f_GetPosition() const
	{
		return mp_Position;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStoragePtr<t_CStreamType>::f_SetPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(_Pos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStoragePtr<t_CStreamType>::f_SetPositionFromEnd(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Length + _Pos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStoragePtr<t_CStreamType>::f_AddPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Position + _Pos);
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamStoragePtr<t_CStreamType>::f_IsValidReadPosition(NStream::CFilePos _Pos) const
	{
		return _Pos >= 0 && _Pos < NStream::CFilePos(mp_Length);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStoragePtr<t_CStreamType>::f_Flush(bool _bLocalCacheOnly)
	{
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStoragePtr<t_CStreamType>::f_SetCacheSize(umint _CacheSize)
	{
	}

	template <typename t_CStreamType>
	CFilePos TCBinaryStreamStoragePtr<t_CStreamType>::f_GetLength() const
	{
		return mp_Length;
	}

	template <typename t_CStreamType>
	umint TCBinaryStreamStoragePtr<t_CStreamType>::f_ContainerLengthLimit() const
	{
		return mp_Length - mp_Position;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamStoragePtr<t_CStreamType>::f_SetLength(NStream::CFilePos _Length)
	{
		DMibError("Binary storage read stream cannot change length");
	}
}

namespace NMib::NStream
{
	namespace NPrivate
	{
		template <typename t_CStream, typename t_CEnableIf = void>
		struct TCStreamHasFeedStorage
		{
			static constexpr bool mc_Value = false;
		};

		template <typename t_CStream>
		struct TCStreamHasFeedStorage
		<
			t_CStream
			, TCEnableIf
			<
				!NTraits::cIsSame
				<
					decltype(fg_GetReference<t_CStream>().f_FeedStorage(fg_GetType<CBinaryStorage>())), CDummy
				>
			>
		>
		{
			static constexpr bool mc_Value = true;
		};

		template <typename t_CStream, typename t_CEnableIf = void>
		struct TCStreamHasConsumeStorage
		{
			static constexpr bool mc_Value = false;
		};

		template <typename t_CStream>
		struct TCStreamHasConsumeStorage
		<
			t_CStream
			, TCEnableIf
			<
				!NTraits::cIsSame
				<
					decltype(fg_GetReference<t_CStream>().f_ConsumeStorage(fg_GetType<umint>())), CDummy
				>
			>
		>
		{
			static constexpr bool mc_Value = true;
		};
	}

	// Wire compatible with a byte vector: length prefix followed by bytes.
	template <typename t_CStream>
	class TCBinaryStreamTypeReference<t_CStream, CBinaryStorage>
	{
	public:
		static void fs_Feed(t_CStream &_Stream, CBinaryStorage const &_Data)
		{
			umint nItems = _Data.f_GetTotalLength();
			fg_FeedLenToStream(_Stream, nItems);

			// Only shared spans have owners that can be retained by a const feed; arena and adopted bytes must be copied.
			if constexpr (NPrivate::TCStreamHasFeedStorage<t_CStream>::mc_Value)
			{
				if (_Stream.f_CanAdopt())
				{
					CBinaryStorage Shared;
					Shared.f_AppendRangeOf(_Data, 0, nItems);
					_Stream.f_FeedStorage(fg_Move(Shared));
					return;
				}
			}

#if DMibConfig_IoDebug_Enable
			g_BinaryStorageFeedConstCopyBytes.f_FetchAdd(nItems, NAtomic::gc_MemoryOrder_Relaxed);
#endif
			_Data.f_VisitSpans
				(
					[&](uint8 const *_pData, umint _nBytes)
					{
						_Stream.f_FeedBytes(_pData, _nBytes);
					}
				)
			;
		}

		static void fs_Feed(t_CStream &_Stream, CBinaryStorage &&_Data)
		{
			umint nItems = _Data.f_GetTotalLength();
			fg_FeedLenToStream(_Stream, nItems);
			if constexpr (NPrivate::TCStreamHasFeedStorage<t_CStream>::mc_Value)
			{
				if (_Stream.f_CanAdopt())
				{
					_Stream.f_FeedStorage(fg_Move(_Data));
					return;
				}
			}

#if DMibConfig_IoDebug_Enable
			g_BinaryStorageFeedCopyBytes.f_FetchAdd(nItems, NAtomic::gc_MemoryOrder_Relaxed);
#endif
			_Data.f_VisitSpans
				(
					[&](uint8 const *_pData, umint _nBytes)
					{
						_Stream.f_FeedBytes(_pData, _nBytes);
					}
				)
			;
		}

		static void fs_Consume(t_CStream &_Stream, CBinaryStorage &_Data)
		{
			uint64 nItems;
			fg_ConsumeLenFromStream(_Stream, nItems);
			fg_CheckLengthLimit(_Stream, nItems);

			if constexpr (NPrivate::TCStreamHasConsumeStorage<t_CStream>::mc_Value)
			{
				_Data = _Stream.f_ConsumeStorage(nItems);
			}
			else
			{
#if DMibConfig_IoDebug_Enable
				g_BinaryStorageConsumeCopyBytes.f_FetchAdd(nItems, NAtomic::gc_MemoryOrder_Relaxed);
#endif

				NContainer::CIOByteVector Vector;
				Vector.f_SetLen(nItems);
				_Stream.f_ConsumeBytes(Vector.f_GetArray(), nItems);

				_Data = CBinaryStorage(fg_Move(Vector));
			}
		}
	};
}
