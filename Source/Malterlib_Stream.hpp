// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NStream
{
	constexpr CScopeBinaryStreamVersion::CScopeBinaryStreamVersion(CBinaryStream &_Stream, uint32 _Version)
		: mp_pStream(&_Stream)
	{
		mp_OldVersion = _Stream.mp_Version;
		_Stream.mp_Version = _Version;
	}

	template <typename tf_CVersion>
	constexpr CScopeBinaryStreamVersion::CScopeBinaryStreamVersion(CBinaryStream &_Stream, tf_CVersion _Version)
		requires (cIsValidStreamVersion<tf_CVersion>)
		: CScopeBinaryStreamVersion(_Stream, uint32(_Version))
	{
	}

	constexpr void CScopeBinaryStreamVersion::f_SetVersion(CBinaryStream &_Stream, uint32 _Version)
	{
		f_Clear();
		mp_pStream = &_Stream;
		mp_OldVersion = _Stream.mp_Version;
		_Stream.mp_Version = _Version;
	}

	template <typename tf_CVersion>
	constexpr void CScopeBinaryStreamVersion::f_SetVersion(CBinaryStream &_Stream, tf_CVersion _Version)
		requires (cIsValidStreamVersion<tf_CVersion>)
	{
		f_SetVersion(_Stream, uint32(_Version));
	}

	constexpr CScopeBinaryStreamVersion::~CScopeBinaryStreamVersion()
	{
		f_Clear();
	}

	constexpr void CScopeBinaryStreamVersion::f_Clear()
	{
		if (mp_pStream)
		{
			mp_pStream->mp_Version = mp_OldVersion;
			mp_pStream = nullptr;
		}
	}

	template <typename t_CStream, EStreamDirection t_Direction>
	template <typename tf_CData>
	constexpr inline_small auto TCStreamDirection<t_CStream, t_Direction>::f_Feed(tf_CData &&_Data)
	{
		return TCBinaryStreamTypeReference<t_CStream, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Feed(*this, NMib::fg_Forward<tf_CData>(_Data));
	}

	template <typename t_CStream, EStreamDirection t_Direction>
	template <typename tf_CData>
	constexpr inline_small auto TCStreamDirection<t_CStream, t_Direction>::f_Feed(tf_CData const *_pData)
	{
		return TCBinaryStreamTypePtr<t_CStream, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Feed(*this, _pData);
	}

	template <typename t_CStream, EStreamDirection t_Direction>
	template <typename tf_CData>
	constexpr inline_small auto TCStreamDirection<t_CStream, t_Direction>::f_Consume(tf_CData &&_Data)
	{
		return TCBinaryStreamTypeReference<t_CStream, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Consume(*this, NMib::fg_Forward<tf_CData>(_Data));
	}

	template <typename t_CStream, EStreamDirection t_Direction>
	template <typename tf_CData>
	constexpr inline_small auto TCStreamDirection<t_CStream, t_Direction>::f_Consume(tf_CData *_pData)
	{
		return TCBinaryStreamTypePtr<t_CStream, tf_CData>::fs_Consume(*this, _pData);
	}

	template <typename t_CStream, EStreamDirection t_Direction>
	template <typename tf_CData>
	constexpr inline_small auto TCStreamDirection<t_CStream, t_Direction>::f_Stream(tf_CData &&_Data)
	{
		return TCBinaryStreamTypeReferenceStream<t_CStream, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Stream(*this, NMib::fg_Forward<tf_CData>(_Data));
	}

	template <typename t_CStream, EStreamDirection t_Direction>
	template <typename tf_CData>
	constexpr inline_small CScopeBinaryStreamVersion TCStreamDirection<t_CStream, t_Direction>::f_StreamVersion(tf_CData &&_DefaultVersion)
	{
		auto Version = _DefaultVersion;
		this->f_Stream(Version);
		return CScopeBinaryStreamVersion(*this, Version);
	}

	template <typename t_CStream, EStreamDirection t_Direction>
	t_CStream &TCStreamDirection<t_CStream, t_Direction>::f_GetStream()
	{
		return *this;
	}

	template <typename t_CStream, EStreamDirection t_Direction>
	t_CStream const &TCStreamDirection<t_CStream, t_Direction>::f_GetStream() const
	{
		return *this;
	}

	template <typename t_CStream, typename t_CData>
	template <EStreamDirection tf_Direction, typename tf_CData, NMib::TCEnableIf<NPrivate::TCHasStream<TCStreamDirection<t_CStream, tf_Direction>, tf_CData>::mc_Value> *>
	inline_small auto TCBinaryStreamTypeReferenceStream<t_CStream, t_CData>::fs_Stream(TCStreamDirection<t_CStream, tf_Direction> &_Stream, tf_CData &&_Data)
	{
		return fg_Forward<tf_CData>(_Data).f_Stream(_Stream);
	}

	template <typename t_CStream, typename t_CData>
	template <typename tf_CData, NMib::TCEnableIf<!NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Feed>, tf_CData>::mc_Value> *>
	inline_small auto TCBinaryStreamTypeReferenceStream<t_CStream, t_CData>::fs_Stream(TCStreamDirection<t_CStream, EStreamDirection_Feed> &_Stream, tf_CData &&_Data)
	{
		static_cast<t_CStream &>(_Stream) << fg_ConstOrMove<tf_CData>(fg_Forward<tf_CData>(_Data));
	}

	template <typename t_CStream, typename t_CData>
	template <typename tf_CData, NMib::TCEnableIf<!NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Consume>, tf_CData>::mc_Value> *>
	inline_small auto TCBinaryStreamTypeReferenceStream<t_CStream, t_CData>::fs_Stream(TCStreamDirection<t_CStream, EStreamDirection_Consume> &_Stream, tf_CData &&_Data)
	{
		static_cast<t_CStream &>(_Stream) >> const_cast<NTraits::TCRemoveReferenceAndQualifiers<tf_CData> &>(_Data);
	}

	template <EStreamDirection tf_StreamDirection, typename tf_CStream>
	decltype(auto) fg_StreamDirection(tf_CStream &_Stream)
	{
		return reinterpret_cast<TCStreamDirection<tf_CStream, tf_StreamDirection> &>(_Stream);
	}

	template <typename tf_CStream>
	decltype(auto) fg_FeedStream(tf_CStream &_Stream)
	{
		return fg_StreamDirection<EStreamDirection_Feed>(_Stream);
	}

	template <typename tf_CStream>
	decltype(auto) fg_ConsumeStream(tf_CStream &_Stream)
	{
		return fg_StreamDirection<EStreamDirection_Consume>(_Stream);
	}

	template <typename t_CStream, typename t_CData>
	template
	<
		typename tf_CData
		, NMib::TCEnableIf<!NMib::NTraits::cIsEnum<NTraits::TCRemoveReference<tf_CData>> && !NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Feed>, tf_CData>::mc_Value> *
	>
	constexpr inline_small auto TCBinaryStreamTypeReference<t_CStream, t_CData>::fs_Feed(t_CStream &_Stream, tf_CData &&_Data)
	{
		return fg_Forward<tf_CData>(_Data).f_Feed(_Stream);
	}

	template <typename t_CStream, typename t_CData>
	template
	<
		typename tf_CData
		, NMib::TCEnableIf<!NMib::NTraits::cIsEnum<NTraits::TCRemoveReference<tf_CData>> && NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Feed>, tf_CData>::mc_Value> *
	>
	constexpr inline_small auto TCBinaryStreamTypeReference<t_CStream, t_CData>::fs_Feed(t_CStream &_Stream, tf_CData &&_Data)
	{
		return const_cast<NTraits::TCRemoveReferenceAndQualifiers<tf_CData> &>(_Data).f_Stream(reinterpret_cast<TCStreamDirection<t_CStream, EStreamDirection_Feed> &>(_Stream));
	}

	template <typename t_CStream, typename t_CData>
	template
	<
		typename tf_CData
		, NMib::TCEnableIf
		<
			!NMib::NTraits::cIsEnum<NTraits::TCRemoveReference<tf_CData>> && !NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Consume>, tf_CData>::mc_Value
		>
		*
	>
	constexpr inline_small auto TCBinaryStreamTypeReference<t_CStream, t_CData>::fs_Consume(t_CStream &_Stream, tf_CData &&_Data)
	{
		return _Data.f_Consume(_Stream);
	}

	template <typename t_CStream, typename t_CData>
	template
	<
		typename tf_CData
		, NMib::TCEnableIf
		<
			!NMib::NTraits::cIsEnum<NTraits::TCRemoveReference<tf_CData>> && NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Consume>, tf_CData>::mc_Value
		>
		*
	>
	constexpr inline_small auto TCBinaryStreamTypeReference<t_CStream, t_CData>::fs_Consume(t_CStream &_Stream, tf_CData &&_Data)
	{
		return fg_Forward<tf_CData>(_Data).f_Stream(reinterpret_cast<TCStreamDirection<t_CStream, EStreamDirection_Consume> &>(_Stream));
	}

	template <typename t_CStream, typename t_CData>
	template <typename tf_CData, NMib::TCEnableIf<NMib::NTraits::cIsEnum<tf_CData>, void> *>
	constexpr inline_small void TCBinaryStreamTypeReference<t_CStream, t_CData>::fs_Feed(t_CStream &_Stream, tf_CData const &_Data)
	{
		if constexpr (NTraits::cIsScopedEnum<tf_CData>)
			_Stream << NTraits::TCEnumUnderlyingType<tf_CData>(_Data);
		else
			_Stream << uint32(_Data);
	}

	template <typename t_CStream, typename t_CData>
	template <typename tf_CData, NMib::TCEnableIf<NMib::NTraits::cIsEnum<tf_CData>, void> *>
	constexpr inline_small void TCBinaryStreamTypeReference<t_CStream, t_CData>::fs_Consume(t_CStream &_Stream, tf_CData &_Data)
	{
		if constexpr (NTraits::cIsScopedEnum<tf_CData>)
		{
			NTraits::TCEnumUnderlyingType<tf_CData> Temp;
			_Stream >> Temp;
			_Data = static_cast<tf_CData>(Temp);
		}
		else
		{
			uint32 Temp;
			_Stream >> Temp;
			_Data = static_cast<tf_CData>(Temp);
		}
	}

	template <typename t_CStream, typename t_CData>
	constexpr void TCBinaryStreamTypePtr<t_CStream, t_CData>::fs_Feed(t_CStream &_Stream, t_CData const *_pData)
	{
		t_CStream::Implement_Error; // Type not implemented
		// _Stream.f_FeedBytes(_pData, sizeof(*_pData));
	}

	template <typename t_CStream, typename t_CData>
	constexpr void TCBinaryStreamTypePtr<t_CStream, t_CData>::fs_Consume(t_CStream &_Stream, t_CData *_pData)
	{
		t_CStream::Implement_Error;
		// _Stream.f_ConsumeBytes(_pData, sizeof(*_pData));
	}

	template <typename t_CType>
	TCBinaryStreamUnsafeWrapper<t_CType>::TCBinaryStreamUnsafeWrapper(t_CType &_Data)
		: m_Data(_Data)
	{
	}

#ifndef DMibTempStreamDebug
	inline void CBinaryStream::f_FeedBytes(void const *_pMem, umint _nBytes)
	{
		fp_FeedBytes(_pMem, _nBytes);
	}

	inline void CBinaryStream::f_ConsumeBytes(void *_pMem, umint _nBytes)
	{
		fp_ConsumeBytes(_pMem, _nBytes);
	}

	inline bool CBinaryStream::f_IsValid() const
	{
		return fp_IsValid();
	}

	inline bool CBinaryStream::f_IsAtEndOfStream() const
	{
		return fp_IsAtEndOfStream();
	}

	inline CFilePos CBinaryStream::f_GetPosition() const
	{
		return fp_GetPosition();
	}

	inline void CBinaryStream::f_SetPosition(CFilePos _Pos)
	{
		fp_SetPosition(_Pos);
	}

	inline void CBinaryStream::f_SetPositionFromEnd(CFilePos _Pos)
	{
		fp_SetPositionFromEnd(_Pos);
	}

	inline void CBinaryStream::f_AddPosition(CFilePos _Pos)
	{
		fp_AddPosition(_Pos);
	}

	inline bool CBinaryStream::f_IsValidReadPosition(NStream::CFilePos _Pos) const
	{
		return fp_IsValidReadPosition(_Pos);
	}

	inline void CBinaryStream::f_Flush(bool _bLocalCacheOnly)
	{
		return fp_Flush(_bLocalCacheOnly);
	}

	inline void CBinaryStream::f_SetCacheSize(umint _CacheSize)
	{
		return fp_SetCacheSize(_CacheSize);
	}

	inline CFilePos CBinaryStream::f_GetLength() const
	{
		return fp_GetLength();
	}

	inline void CBinaryStream::f_SetLength(CFilePos _Length)
	{
		return fp_SetLength(_Length);
	}

	inline aint CBinaryStream::f_LengthSize() const
	{
		return fp_LengthSize();
	}

	inline aint CBinaryStream::f_Endian() const
	{
		return fp_Endian();
	}

	inline umint CBinaryStream::f_ContainerLengthLimit() const
	{
		return fp_ContainerLengthLimit();
	}

	inline void CBinaryStream::f_FeedFromStream(CBinaryStream &_Stream, CFilePos _nBytes)
	{
		fp_FeedFromStream(_Stream, _nBytes);
	}
#endif

#undef DMibTempStreamDebug
#undef DMibTempStreamPre
#undef DMibTempStreamPost

	inline uint32 CBinaryStream::f_GetVersion() const
	{
		return mp_Version;
	}

	template <typename tf_CVersion>
	inline_small bool CBinaryStream::f_SupportsVersion(tf_CVersion _Version) const
		requires (cIsValidStreamVersion<tf_CVersion>)
	{
		return mp_Version >= uint32(_Version);
	}

	inline void *CBinaryStream::f_GetContext() const
	{
		return mp_pContext;
	}

	inline umint CBinaryStream::f_ClaimContainerLengthLimitOverride()
	{
		umint Return = mp_ContainerLengthLimit;
		mp_ContainerLengthLimit = 0;
		return Return;
	}

	template <typename tf_CStream>
	void fg_PadAlignStream(tf_CStream &_Stream, umint _Alignment)
	{
		CFilePos Position = _Stream.f_GetPosition();
		CFilePos EndPosition = fg_AlignUp(Position, CFilePos(_Alignment));
		while (Position != EndPosition)
		{
			uint8 PaddingData[128] = {0};

			umint ThisTime = fg_Min(EndPosition - Position, CFilePos(128));

			_Stream.f_FeedBytes(PaddingData, ThisTime);
			Position += ThisTime;
		}
	}

	template <typename tf_CStream>
	void fg_AlignStream(tf_CStream &_Stream, umint _Alignment)
	{
		CFilePos Position = _Stream.f_GetPosition();
		CFilePos EndPosition = fg_AlignUp(Position, CFilePos(_Alignment));
		_Stream.f_SetPosition(EndPosition);
	}

	constexpr inline_small aint CBinaryStreamDefault::f_LengthSize() const
	{
		return sizeof(uint32);
	}

	constexpr inline_small aint CBinaryStreamDefault::f_Endian() const
	{
		return EEndian_Little;
	}

	constexpr inline_small void CBinaryStreamDefault::f_FeedFromStream(CBinaryStream &_Stream, CFilePos _nBytes)
	{
		if_consteval
		{
			uint8 Temp[1024];
			CFilePos ToTransfer = _nBytes;
			while (ToTransfer)
			{
				umint ThisTime = fg_Min(ToTransfer, 1024);
				_Stream.f_ConsumeBytes(Temp, ThisTime);
				fp_FeedBytes(Temp, ThisTime);
				ToTransfer -= ThisTime;
			}
		}
		else
		{
			fp_FeedFromStreamImplementation(_Stream, _nBytes);
		}
	}

	inline aint CBinaryStreamBigEndian::f_Endian() const
	{
		return EEndian_Big;
	}

	inline aint CBinaryStreamLittleEndian::f_Endian() const
	{
		return EEndian_Little;
	}

	inline aint CBinaryStreamNativeEndian::f_Endian() const
	{
		return EEndian_Native;
	}

	template <typename t_CStreamType>
	constexpr void TCBinaryStreamNull<t_CStreamType>::f_Reset()
	{
		mp_Position = 0;
	}

	template <typename t_CStreamType>
	constexpr void TCBinaryStreamNull<t_CStreamType>::f_Clear()
	{
		mp_Position = 0;
	}

	template <typename t_CStreamType>
	constexpr void TCBinaryStreamNull<t_CStreamType>::f_FeedBytes(void const *_pMem, umint _nBytes)
	{
		mp_Position += _nBytes;
		if (mp_Position > mp_Length)
			mp_Length = mp_Position;
	}

	template <typename t_CStreamType>
	constexpr void TCBinaryStreamNull<t_CStreamType>::f_ConsumeBytes(void *_pMem, umint _nBytes)
	{
		if (mp_Length < (mp_Position + _nBytes))
			DMibError("End of stream Overrun");
		mp_Position += _nBytes;
	}

	template <typename t_CStreamType>
	constexpr bool TCBinaryStreamNull<t_CStreamType>::f_IsValid() const
	{
		return true;
	}

	template <typename t_CStreamType>
	constexpr bool TCBinaryStreamNull<t_CStreamType>::f_IsAtEndOfStream() const
	{
		return mp_Position == mp_Length;
	}

	template <typename t_CStreamType>
	constexpr CFilePos TCBinaryStreamNull<t_CStreamType>::f_GetPosition() const
	{
		return mp_Position;
	}

	template <typename t_CStreamType>
	constexpr void TCBinaryStreamNull<t_CStreamType>::f_SetPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(_Pos);
	}

	template <typename t_CStreamType>
	constexpr void TCBinaryStreamNull<t_CStreamType>::f_SetPositionFromEnd(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Length + _Pos);
	}

	template <typename t_CStreamType>
	constexpr void TCBinaryStreamNull<t_CStreamType>::f_AddPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Position + _Pos);
	}

	template <typename t_CStreamType>
	constexpr bool TCBinaryStreamNull<t_CStreamType>::f_IsValidReadPosition(NStream::CFilePos _Pos) const
	{
		return _Pos >= 0 && _Pos < NStream::CFilePos(mp_Length);
	}

	template <typename t_CStreamType>
	constexpr void TCBinaryStreamNull<t_CStreamType>::f_Flush(bool _bLocalCacheOnly)
	{
	}

	template <typename t_CStreamType>
	constexpr void TCBinaryStreamNull<t_CStreamType>::f_SetCacheSize(umint _CacheSize)
	{
	}

	template <typename t_CStreamType>
	constexpr CFilePos TCBinaryStreamNull<t_CStreamType>::f_GetLength() const
	{
		return mp_Length;
	}

	template <typename t_CStreamType>
	constexpr umint TCBinaryStreamNull<t_CStreamType>::f_ContainerLengthLimit() const
	{
		return f_GetLength() - f_GetPosition();
	}

	template <typename t_CStreamType>
	constexpr void TCBinaryStreamNull<t_CStreamType>::f_SetLength(NStream::CFilePos _Length)
	{
		mp_Length = _Length;
	}

	template <typename t_CStreamType>
	constexpr void TCBinaryStreamNull<t_CStreamType>::fp_SetPositionInternal(CFilePos _Pos)
	{
		if (_Pos < 0)
			DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

		mp_Position = _Pos;
	}

	template <typename tf_CType>
	CFilePos fg_GetBinaryStreamSize(tf_CType const &_Value)
	{
		TCBinaryStreamNull<> SizeStream;
		SizeStream << _Value;
		return SizeStream.f_GetPosition();
	}

	template <typename tf_CStream>
	void fg_FeedLenToStream(tf_CStream &_Stream, uint64 _Len)
	{
		aint nBytes = _Stream.f_LengthSize();
		switch (nBytes)
		{
		case 1: _Stream << (uint8)_Len; break;
		case 2: _Stream << (uint16)_Len; break;
		case 4: _Stream << (uint32)_Len; break;
		case 8: _Stream << (uint64)_Len; break;
		default: DMibSafeCheck(0, "Unsupported save size"); break;
		}
	}

	inline_always umint fg_CapLengthLimit(NStream::CFilePos const &_Len)
	{
		if constexpr (sizeof(umint) < sizeof(NStream::CFilePos))
		{
			if (_Len > NStream::CFilePos{TCLimitsInt<smint>::mc_Max})
				return TCLimitsInt<smint>::mc_Max;
		}
		return _Len;
	}

	template <typename tf_CStream, typename tf_CLen>
	void fg_CheckLengthLimit(tf_CStream &_Stream, tf_CLen const &_Len)
	{
		umint LengthLimit = _Stream.f_ClaimContainerLengthLimitOverride();
		if (!LengthLimit)
			LengthLimit = _Stream.f_ContainerLengthLimit();

		if (_Len > uint64(LengthLimit))
			fg_ReportLengthLimitError(_Len, LengthLimit);
	}

	template <typename tf_CStream>
	void fg_ConsumeLenFromStream(tf_CStream &_Stream, uint64 &_Len)
	{
		aint nBytes = _Stream.f_LengthSize();
		switch (nBytes)
		{
		case 1:
			{
				uint8 Len;
				_Stream >> Len;
				_Len = Len;
			}
			break;
		case 2:
			{
				uint16 Len;
				_Stream >> Len;
				_Len = Len;
			}
			break;
		case 4:
			{
				uint32 Len;
				_Stream >> Len;
				_Len = Len;
			}
			break;
		case 8:
			{
				uint64 Len;
				_Stream >> Len;
				_Len = Len;
			}
			break;
		default: DMibSafeCheck(0, "Unsupported save size"); break;
		}
	}

	template <typename tf_CStream, typename tf_CData>
	void fg_FeedEndianArrayToStream(tf_CStream &_Stream, tf_CData const *_pData, umint _Len, aint _Endian)
	{
		if (_Endian == EEndian_Native || _Endian == gc_MachineEndian)
		{
			if (_Len != 0)
				_Stream.f_FeedBytes(_pData, _Len * sizeof(tf_CData));
		}
		else
		{
			for (umint iData = 0; iData < _Len; ++iData)
			{
				tf_CData Temp = fg_ByteSwap(_pData[iData]);
				_Stream.f_FeedBytes(&Temp, sizeof(tf_CData));
			}
		}
	}

	template <typename tf_CStream, typename tf_CData>
	void fg_ByteSwapArray(tf_CStream &_Stream, tf_CData *_pData, umint _Len, aint _Endian)
	{
		if (_Endian == EEndian_Native || _Endian == gc_MachineEndian)
			return;
		else
			fg_ByteSwap(_pData, _Len);
	}

	template <typename t_CStream>
	constexpr void TCBinaryStreamTypePtr<t_CStream, ch8>::fs_Feed(t_CStream &_Stream, ch8 const *_pData)
	{
		uint64 Len = NMib::NStr::fg_StrLen(_pData);
		uint64 LenStream = Len;
		fg_StrEncodeLenType(LenStream, _Stream.f_LengthSize(), NStr::EStrType_UTF); // Presume UTF, as this is the most probable case
		fg_FeedLenToStream(_Stream, LenStream);
		if (Len != 0)
			_Stream.f_FeedBytes(_pData, Len);
	}

	template <typename t_CStream>
	constexpr void TCBinaryStreamTypePtr<t_CStream, ch8>::fs_Consume(t_CStream &_Stream, ch8 *_pData)
	{
		uint64 Len;
		fg_ConsumeLenFromStream(_Stream, Len);
		NStr::EStrType Type = NStr::EStrType_Ansi;
		fg_StrDecodeLenType(Len, _Stream.f_LengthSize(), Type);
		fg_CheckLengthLimit(_Stream, Len);
		if (Len != 0)
			_Stream.f_ConsumeBytes(_pData, Len);
		_pData[Len] = 0;
	}

	template <typename t_CStream>
	constexpr void TCBinaryStreamTypePtr<t_CStream, ch16>::fs_Feed(t_CStream &_Stream, ch16 const *_pData)
	{
		umint Len = NMib::NStr::fg_StrLen(_pData);
		uint64 LenStream = Len;
		fg_StrEncodeLenType(LenStream, _Stream.f_LengthSize(), NStr::EStrType_UTF); // Presume UTF, as this is the most probable case
		fg_FeedLenToStream(_Stream, LenStream);
		fg_FeedEndianArrayToStream(_Stream, _pData, Len, _Stream.f_Endian());
	}

	template <typename t_CStream>
	constexpr void TCBinaryStreamTypePtr<t_CStream, ch16>::fs_Consume(t_CStream &_Stream, ch16 *_pData)
	{
		uint64 Len;
		fg_ConsumeLenFromStream(_Stream, Len);
		NStr::EStrType Type = NStr::EStrType_Ansi;
		fg_StrDecodeLenType(Len, _Stream.f_LengthSize(), Type);
		fg_CheckLengthLimit(_Stream, Len);
		if (Len != 0)
			_Stream.f_ConsumeBytes(_pData, Len);
		fg_ByteSwapArray(_Stream, _pData, Len, _Stream.f_Endian());
		_pData[Len] = 0;
	}

	template <typename t_CStream>
	constexpr void TCBinaryStreamTypePtr<t_CStream, ch32>::fs_Feed(t_CStream &_Stream, ch32 const *_pData)
	{
		umint Len = NMib::NStr::fg_StrLen(_pData);
		uint64 LenStream = Len;
		fg_StrEncodeLenType(LenStream, _Stream.f_LengthSize(), NStr::EStrType_Unicode);
		fg_FeedLenToStream(_Stream, LenStream);
		fg_FeedEndianArrayToStream(_Stream, _pData, Len, _Stream.f_Endian());
	}

	template <typename t_CStream>
	constexpr void TCBinaryStreamTypePtr<t_CStream, ch32>::fs_Consume(t_CStream &_Stream, ch32 *_pData)
	{
		uint64 Len;
		fg_ConsumeLenFromStream(_Stream, Len);
		NStr::EStrType Type = NStr::EStrType_Ansi;
		fg_StrDecodeLenType(Len, _Stream.f_LengthSize(), Type);
		fg_CheckLengthLimit(_Stream, Len);
		if (Len != 0)
			_Stream.f_ConsumeBytes(_pData, Len);
		fg_ByteSwapArray(_Stream, _pData, Len, _Stream.f_Endian());
		_pData[Len] = 0;
	}

	template <typename t_CStream, typename t_CType>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType const>>::fs_Feed
		(
			t_CStream &_Stream
			, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType const> const &_Data
		)
	{
		_Stream << _Data.m_Data;
	}

	template <typename t_CStream, typename t_CType>
	void TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> const>::fs_Consume
		(
			t_CStream &_Stream
			, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> const &_Data
		)
	{
		_Stream >> _Data.m_Data;
	}

	template <typename t_CStream, typename t_CType>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType>>::fs_Feed
		(
			t_CStream &_Stream
			, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> const &_Data
		)
	{
		_Stream << _Data.m_Data;
	}

	template <typename t_CStream, typename t_CType>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType>>::fs_Consume
		(
			t_CStream &_Stream
			, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> &_Data
		)
	{
		_Stream >> _Data.m_Data;
	}

	template <typename t_CStream, typename t_CType>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType>>::fs_Consume
		(
			t_CStream &_Stream
			, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> &&_Data
		)
	{
		_Stream >> _Data.m_Data;
	}

	template <typename tf_CType>
	inline_small TCBinaryStreamUnsafeWrapper<tf_CType> fg_GetUnsafeStreamWrapper(tf_CType &_Type)
	{
		return TCBinaryStreamUnsafeWrapper<tf_CType>(_Type);
	}

	template <typename tf_CType>
	inline_small TCBinaryStreamUnsafeWrapper<tf_CType const> fg_GetUnsafeStreamWrapper(tf_CType const &_Type)
	{
		return TCBinaryStreamUnsafeWrapper<tf_CType const>(_Type);
	}

	template <typename t_CStream>
	constexpr void TCBinaryStreamTypeReference<t_CStream, bool>::fs_Feed(t_CStream &_Stream, bool const &_bData)
	{
		uint8 Byte = _bData ? 1 : 0;
		_Stream.f_FeedBytes(&Byte, sizeof(Byte));
	}

	template <typename t_CStream>
	constexpr void TCBinaryStreamTypeReference<t_CStream, bool>::fs_Consume(t_CStream &_Stream, bool &_bData)
	{
		uint8 Byte;
		_Stream.f_ConsumeBytes(&Byte, sizeof(Byte));
		_bData = Byte != 0;
	}

	template <typename t_CStream, typename t_CData, typename t_CTranslator, typename t_CLink, typename t_CLinkInList, bool t_bAutoDelete, typename t_CAllocator>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>>::fs_Feed
		(
			t_CStream &_Stream
			, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> const &_Data
		)
	{
		typename NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>::CIteratorConst Iter(_Data);

		umint nItems = 0;

		while (Iter)
		{
			++nItems;
			++Iter;
		};

		fg_FeedLenToStream(_Stream, nItems);

		Iter = _Data;

		while (Iter)
		{
			_Stream.f_Feed(*Iter);
			++Iter;
		};
	}

	template <typename t_CStream, typename t_CData, typename t_CTranslator, typename t_CLink, typename t_CLinkInList, bool t_bAutoDelete, typename t_CAllocator>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>>::fs_Feed
		(
			t_CStream &_Stream
			, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &&_Data
		)
	{
		typename NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>::CIteratorConst Iter(_Data);

		umint nItems = 0;

		while (Iter)
		{
			++nItems;
			++Iter;
		};

		fg_FeedLenToStream(_Stream, nItems);

		Iter = _Data;

		while (Iter)
		{
			_Stream.f_Feed(fg_Move(*Iter));
			++Iter;
		};
	}

	template <typename t_CStream, typename t_CData, typename t_CTranslator, typename t_CLink, typename t_CLinkInList, bool t_bAutoDelete, typename t_CAllocator>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>>::fs_Consume
		(
			t_CStream &_Stream
			, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &_Data
		)
	{
		uint64 nItems;
		fg_ConsumeLenFromStream(_Stream, nItems);
		fg_CheckLengthLimit(_Stream, nItems);

		while (nItems)
		{
			auto Memory = t_CAllocator::f_AllocSafe(sizeof(t_CData), alignof(t_CData));
			t_CData *pNewItem = new(Memory.m_pMemory) t_CData();
			Memory.f_Claim();
			auto Cleanup = g_OnScopeExit / [&]
				{
					pNewItem->~t_CData();
					t_CAllocator::f_Free(pNewItem, sizeof(t_CData));
				}
			;
			_Stream.f_Consume(*pNewItem);
			_Data.f_Insert(pNewItem);
			Cleanup.f_Clear();
			--nItems;
		}
	}

	template <typename t_CStream, typename t_CData, typename t_CTranslator, typename t_CLink, typename t_CLinkInList, bool t_bAutoDelete, typename t_CAllocator>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>>::fs_Feed
		(
			t_CStream &_Stream
			, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> const &_Data
		)
	{
		_Stream << (NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> const &)_Data;
	}

	template <typename t_CStream, typename t_CData, typename t_CTranslator, typename t_CLink, typename t_CLinkInList, bool t_bAutoDelete, typename t_CAllocator>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>>::fs_Feed
		(
			t_CStream &_Stream
			, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &&_Data
		)
	{
		_Stream << fg_Move((NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &)_Data);
	}

	template <typename t_CStream, typename t_CData, typename t_CTranslator, typename t_CLink, typename t_CLinkInList, bool t_bAutoDelete, typename t_CAllocator>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>>::fs_Consume
		(
			t_CStream &_Stream
			, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &_Data
		)
	{
		_Stream >> (NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &)_Data;
	}

	template <typename t_CStream, typename t_CType, typename ...tp_COptions>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NStorage::TCUniquePointer<t_CType, tp_COptions...>>::fs_Feed
		(
			t_CStream &_Stream
			, NStorage::TCUniquePointer<t_CType, tp_COptions...> const &_pData
		)
	{
		uint8 bNonEmpty = !_pData.f_IsEmpty();
		_Stream << bNonEmpty;
		if (bNonEmpty)
			_Stream << *_pData;
	}

	template <typename t_CStream, typename t_CType, typename ...tp_COptions>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NStorage::TCUniquePointer<t_CType, tp_COptions...>>::fs_Feed
		(
			t_CStream &_Stream
			, NStorage::TCUniquePointer<t_CType, tp_COptions...> &&_pData
		)
	{
		uint8 bNonEmpty = !_pData.f_IsEmpty();
		_Stream << bNonEmpty;
		if (bNonEmpty)
			_Stream << fg_Move(*_pData);
	}

	template <typename t_CStream, typename t_CType, typename ...tp_COptions>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NStorage::TCUniquePointer<t_CType, tp_COptions...>>::fs_Consume
		(
			t_CStream &_Stream
			, NStorage::TCUniquePointer<t_CType, tp_COptions...> &_pData
		)
	{
		uint8 bNonEmpty;
		_Stream >> bNonEmpty;
		if (bNonEmpty)
		{
			_pData = fg_Construct();
			_Stream >> *_pData;
		}
	}

	template <typename t_CStream, typename t_CType, typename ...tp_COptions>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NStorage::TCSharedPointer<t_CType, tp_COptions...>>::fs_Feed
		(
			t_CStream &_Stream
			, NStorage::TCSharedPointer<t_CType, tp_COptions...> const &_pData
		)
	{
		uint8 bNonEmpty = !_pData.f_IsEmpty();
		_Stream << bNonEmpty;
		if (bNonEmpty)
			_Stream << *_pData;
	}

	template <typename t_CStream, typename t_CType, typename ...tp_COptions>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NStorage::TCSharedPointer<t_CType, tp_COptions...>>::fs_Feed
		(
			t_CStream &_Stream
			, NStorage::TCSharedPointer<t_CType, tp_COptions...> &&_pData
		)
	{
		return fs_Feed(_Stream, _pData);
	}

	template <typename t_CStream, typename t_CType, typename ...tp_COptions>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NStorage::TCSharedPointer<t_CType, tp_COptions...>>::fs_Consume
		(
			t_CStream &_Stream
			, NStorage::TCSharedPointer<t_CType, tp_COptions...> &_pData
		)
	{
		uint8 bNonEmpty;
		_Stream >> bNonEmpty;
		if (bNonEmpty)
		{
			_pData = fg_Construct();
			_Stream >> *_pData;
		}
	}
}
