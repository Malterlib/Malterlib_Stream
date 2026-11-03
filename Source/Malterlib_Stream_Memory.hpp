// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NStream
{
	template <typename t_CStreamType, typename t_CVector>
	inline_never void TCBinaryStreamMemory<t_CStreamType, t_CVector>::fp_GrowBufferGrow(umint _NeededBytes)
	{
		if (mp_BufferSize == 0 && _NeededBytes < 2048)
			_NeededBytes = 2048;
		mp_Buffer.f_Grow(_NeededBytes);
		mp_pBuffer = mp_Buffer.f_GetArray();
		mp_BufferSize = mp_Buffer.f_GetLen();
	}

	template <typename t_CStreamType, typename t_CVector>
	inline_small void TCBinaryStreamMemory<t_CStreamType, t_CVector>::fp_GrowBuffer(umint _NeededBytes)
	{
		umint CurrentLen = mp_BufferSize;
		umint NeededSize = _NeededBytes + mp_Position;
		if (CurrentLen < NeededSize)
			fp_GrowBufferGrow(NeededSize);
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::fp_SetPositionInternal(CFilePos _Pos)
	{
		if (_Pos < 0 || fg_SafeLargerThan(_Pos, umint(TCLimitsInt<umint>::mc_Max)))
			DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

		mp_Position = _Pos;
	}

	template <typename t_CStreamType, typename t_CVector>
	void *TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_GetBuffer()
	{
		return mp_pBuffer;
	}

	template <typename t_CStreamType, typename t_CVector>
	void const *TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_GetBufferConst() const
	{
		return mp_pBuffer;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_GrowBuffer(umint _nBytes)
	{
		fp_GrowBuffer(_nBytes);
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_GetVector() -> CStorage &
	{
		mp_Buffer.f_SetLen(mp_Length, false);
		mp_pBuffer = mp_Buffer.f_GetArray();
		mp_BufferSize = mp_Buffer.f_GetLen();
		return (CStorage &)mp_Buffer;
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_GetVectorOptimized() -> CStorage &
	{
		mp_Buffer.f_SetLen(mp_Length, true);
		mp_pBuffer = mp_Buffer.f_GetArray();
		mp_BufferSize = mp_Buffer.f_GetLen();
		return (CStorage &)mp_Buffer;
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_GetVector() const -> CStorage
	{
		CStorage Ret;
		Ret.f_Insert(mp_pBuffer, mp_Length);
		return Ret;
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_GetVectorOptimized() const -> CStorage
	{
		CStorage Ret;
		Ret.f_Insert(mp_pBuffer, mp_Length);
		return Ret;
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_MoveVector() -> CStorage
	{
		mp_Buffer.f_SetLen(mp_Length, false);
		mp_Length = 0;
		mp_BufferSize = 0;
		mp_Position = 0;
		return fg_Move(mp_Buffer);
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_MoveVectorOptimized() -> CStorage
	{
		mp_Buffer.f_SetLen(mp_Length, true);
		mp_Length = 0;
		mp_BufferSize = 0;
		mp_Position = 0;
		return fg_Move(mp_Buffer);
	}

	template <typename t_CStreamType, typename t_CVector>
	TCBinaryStreamMemory<t_CStreamType, t_CVector>::TCBinaryStreamMemory(CStorage const &_Buffer)
		: mp_Buffer(_Buffer)
	{
		mp_Position = 0;
		mp_Length = _Buffer.f_GetLen();
		mp_BufferSize = mp_Length;
		mp_pBuffer = mp_Buffer.f_GetArray();
	}

	template <typename t_CStreamType, typename t_CVector>
	TCBinaryStreamMemory<t_CStreamType, t_CVector>::TCBinaryStreamMemory(CStorage &&_Buffer)
		: mp_Buffer(fg_Move(_Buffer))
	{
		mp_Position = 0;
		mp_Length = mp_Buffer.f_GetLen();
		mp_BufferSize = mp_Length;
		mp_pBuffer = mp_Buffer.f_GetArray();
	}

	template <typename t_CStreamType, typename t_CVector>
	TCBinaryStreamMemory<t_CStreamType, t_CVector>::TCBinaryStreamMemory(TCBinaryStreamMemory &&_ToMove)
		: mp_Buffer(fg_Move(_ToMove.mp_Buffer))
		, mp_Position(_ToMove.mp_Position)
		, mp_Length(_ToMove.mp_Length)
		, mp_BufferSize(_ToMove.mp_BufferSize)
		, mp_pBuffer(_ToMove.mp_pBuffer)
	{
		_ToMove.mp_Position = 0;
		_ToMove.mp_Length = 0;
		_ToMove.mp_BufferSize = 0;
		_ToMove.mp_pBuffer = nullptr;
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemory<t_CStreamType, t_CVector>::operator = (TCBinaryStreamMemory &&_ToMove) -> TCBinaryStreamMemory &
	{
		mp_Buffer = fg_Move(_ToMove.mp_Buffer);
		mp_Position = _ToMove.mp_Position;
		mp_Length = _ToMove.mp_Length;
		mp_BufferSize = _ToMove.mp_BufferSize;
		mp_pBuffer = _ToMove.mp_pBuffer;

		_ToMove.mp_Position = 0;
		_ToMove.mp_Length = 0;
		_ToMove.mp_BufferSize = 0;
		_ToMove.mp_pBuffer = nullptr;

		return *this;
	}

	template <typename t_CStreamType, typename t_CVector>
	TCBinaryStreamMemory<t_CStreamType, t_CVector>::TCBinaryStreamMemory()
	{
		mp_Position = 0;
		mp_Length = 0;
		mp_BufferSize = 0;
		mp_pBuffer = nullptr;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_Open(CStorage const &_Buffer)
	{
		mp_Buffer = _Buffer;
		mp_Position = 0;
		mp_Length = _Buffer.f_GetLen();
		mp_BufferSize = mp_Length;
		mp_pBuffer = mp_Buffer.f_GetArray();
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_Open(CStorage &&_Buffer)
	{
		mp_Buffer = fg_Move(_Buffer);
		mp_Position = 0;
		mp_Length = mp_Buffer.f_GetLen();
		mp_BufferSize = mp_Length;
		mp_pBuffer = mp_Buffer.f_GetArray();
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_ResetStream()
	{
		mp_Position = 0;
		mp_Length = 0;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_Clear()
	{
		mp_Position = 0;
		mp_Length = 0;
		mp_BufferSize = 0;
		mp_Buffer.f_Clear();
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_RemoveData(umint _Pos, umint _Len)
	{
		mp_Buffer.f_Remove(_Pos, _Len);
		mp_pBuffer = mp_Buffer.f_GetArray();
		mp_BufferSize = mp_Buffer.f_GetLen();
		mp_Length -= _Len;

		if (mp_Position > _Pos && mp_Position <= _Pos + _Len)
			mp_Position = _Pos;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_FeedBytes(void const *_pMem, umint _nBytes)
	{
		umint CurrentLen = mp_BufferSize;
		umint Position = mp_Position;
		umint NeededSize = _nBytes + Position;

		if (CurrentLen < NeededSize)
			fp_GrowBufferGrow(NeededSize);

		if (_nBytes != 0)
			NMemory::fg_MemCopy(mp_pBuffer + Position, _pMem, _nBytes);

		Position += _nBytes;

		if (Position > mp_Length)
			mp_Length = Position;

		mp_Position = Position;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_ConsumeBytes(void *_pMem, umint _nBytes)
	{
		if (mp_Length < (mp_Position + _nBytes)) [[unlikely]]
			this->fp_ThrowEndOfStreamException();

		if (_nBytes != 0)
			NMemory::fg_MemCopy(_pMem, mp_pBuffer + mp_Position, _nBytes);

		mp_Position += _nBytes;
	}

	template <typename t_CStreamType, typename t_CVector>
	bool TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_IsValid() const
	{
		return true;
	}

	template <typename t_CStreamType, typename t_CVector>
	bool TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_IsAtEndOfStream() const
	{
		return mp_Position == mp_Length;
	}

	template <typename t_CStreamType, typename t_CVector>
	CFilePos TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_GetPosition() const
	{
		return mp_Position;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_SetPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(_Pos);
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_SetPositionFromEnd(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Length + _Pos);
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_AddPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Position + _Pos);
	}

	template <typename t_CStreamType, typename t_CVector>
	bool TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_IsValidReadPosition(NStream::CFilePos _Pos) const
	{
		return _Pos >= 0 && _Pos < NStream::CFilePos(mp_Length);
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_Flush(bool _bLocalCacheOnly)
	{
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_SetCacheSize(umint _CacheSize)
	{
	}

	template <typename t_CStreamType, typename t_CVector>
	CFilePos TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_GetLength() const
	{
		return mp_Length;
	}

	template <typename t_CStreamType, typename t_CVector>
	umint TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_ContainerLengthLimit() const
	{
		return f_GetLength() - f_GetPosition();
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemory<t_CStreamType, t_CVector>::f_SetLength(NStream::CFilePos _Length)
	{
		if (_Length > NStream::CFilePos(mp_Length))
		{
			fp_GrowBuffer(_Length - mp_Length);
			NMemory::fg_SecureMemClear(mp_pBuffer + mp_Length, _Length - mp_Length);
		}
		mp_Length = _Length;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::fp_GrowBuffer(umint _NeededBytes)
	{
		umint CurrentLen = mp_Buffer.f_GetLen();
		umint NeededSize = _NeededBytes + mp_Position;
		if (CurrentLen < NeededSize)
			mp_Buffer.f_Grow(NeededSize);
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::fp_SetPositionInternal(CFilePos _Pos)
	{
		if ((_Pos < 0) || fg_SafeLargerThan(_Pos, TCLimitsInt<umint>::mc_Max))
			DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

		// if (_Pos > (CFilePos)mp_Length)
		// 	DMibError("Position is past end of stream");

		mp_Position = _Pos;
	}

	template <typename t_CStreamType, typename t_CVector>
	void *TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_GetBuffer()
	{
		return mp_Buffer.f_GetArray();
	}

	template <typename t_CStreamType, typename t_CVector>
	void const *TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_GetBufferConst() const
	{
		return mp_Buffer.f_GetArray();
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_GetVector() -> CStorage &
	{
		mp_Buffer.f_SetLen(mp_Length, false);
		return (CStorage &)mp_Buffer;
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_GetVectorOptimized() -> CStorage &
	{
		mp_Buffer.f_SetLen(mp_Length, true);
		return (CStorage &)mp_Buffer;
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_GetVector() const -> CStorage
	{
		CStorage Ret;
		Ret.f_Insert(mp_Buffer.f_GetArray(), mp_Length);
		return Ret;
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_GetVectorOptimized() const -> CStorage
	{
		CStorage Ret;
		Ret.f_Insert(mp_Buffer.f_GetArray(), mp_Length);
		return Ret;
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_MoveVector() -> CStorage &&
	{
		mp_Buffer.f_SetLen(mp_Length, false);
		mp_Length = 0;
		mp_Position = 0;
		return (CStorage &&)mp_Buffer;
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_MoveVectorOptimized() -> CStorage &&
	{
		mp_Buffer.f_SetLen(mp_Length, true);
		mp_Length = 0;
		mp_Position = 0;
		return (CStorage &&)mp_Buffer;
	}

	template <typename t_CStreamType, typename t_CVector>
	TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::TCBinaryStreamMemoryRef(CStorage &_Buffer)
		: mp_Buffer(_Buffer)
	{
		mp_Position = 0;
		mp_Length = _Buffer.f_GetLen();
	}

	template <typename t_CStreamType, typename t_CVector>
	TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::TCBinaryStreamMemoryRef(CStorage &_Buffer, umint _Length)
		: mp_Buffer(_Buffer)
	{
		mp_Position = 0;
		mp_Length = _Length;
	}

	template <typename t_CStreamType, typename t_CVector>
	TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::~TCBinaryStreamMemoryRef()
	{
		try
		{
			mp_Buffer.f_SetLen(mp_Length, false);
		}
		catch (...)
		{
		}
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_ResetStream()
	{
		mp_Position = 0;
		mp_Length = 0;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_Clear()
	{
		mp_Position = 0;
		mp_Length = 0;
		mp_Buffer.f_Clear();
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_RemoveData(umint _Pos, umint _Len)
	{
		mp_Buffer.f_Remove(_Pos, _Len);
		mp_Length -= _Len;
		if (mp_Position > _Pos && mp_Position <= _Pos + _Len)
			mp_Position = _Pos;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_FeedBytes(void const *_pMem, umint _nBytes)
	{
		fp_GrowBuffer(_nBytes);

		if (_nBytes != 0)
			NMemory::fg_MemCopy(mp_Buffer.f_GetArray() + mp_Position, _pMem, _nBytes);

		mp_Position += _nBytes;
		if (mp_Position > mp_Length)
			mp_Length = mp_Position;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_ConsumeBytes(void *_pMem, umint _nBytes)
	{
		if (mp_Length < (mp_Position + _nBytes)) [[unlikely]]
			this->fp_ThrowEndOfStreamException();

		if (_nBytes != 0)
			NMemory::fg_MemCopy(_pMem, mp_Buffer.f_GetArray() + mp_Position, _nBytes);

		mp_Position += _nBytes;
	}

	template <typename t_CStreamType, typename t_CVector>
	bool TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_IsValid() const
	{
		return true;
	}

	template <typename t_CStreamType, typename t_CVector>
	bool TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_IsAtEndOfStream() const
	{
		return mp_Position == mp_Length;
	}

	template <typename t_CStreamType, typename t_CVector>
	CFilePos TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_GetPosition() const
	{
		return mp_Position;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_SetPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(_Pos);
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_SetPositionFromEnd(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Length + _Pos);
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_AddPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Position + _Pos);
	}

	template <typename t_CStreamType, typename t_CVector>
	bool TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_IsValidReadPosition(NStream::CFilePos _Pos) const
	{
		return _Pos >= 0 && _Pos < NStream::CFilePos(mp_Length);
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_Flush(bool _bLocalCacheOnly)
	{
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_SetCacheSize(umint _CacheSize)
	{
	}

	template <typename t_CStreamType, typename t_CVector>
	CFilePos TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_GetLength() const
	{
		return mp_Length;
	}

	template <typename t_CStreamType, typename t_CVector>
	umint TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_ContainerLengthLimit() const
	{
		return f_GetLength() - f_GetPosition();
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryRef<t_CStreamType, t_CVector>::f_SetLength(NStream::CFilePos _Length)
	{
		if (_Length > NStream::CFilePos(mp_Length))
		{
			fp_GrowBuffer(_Length - mp_Length);
			NMemory::fg_SecureMemClear(mp_Buffer.f_GetArray() + mp_Length, _Length - mp_Length);
		}
		mp_Length = _Length;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::fp_SetPositionInternal(CFilePos _Pos)
	{
		if ((_Pos < 0) || fg_SafeLargerThan(_Pos, TCLimitsInt<umint>::mc_Max))
			DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

		if (_Pos > (CFilePos)mp_Length)
			DMibError("Position is past end of stream");

		mp_Position = _Pos;
	}

	template <typename t_CStreamType, typename t_CVector>
	void *TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_GetBuffer()
	{
		return mp_Buffer.f_GetArray();
	}

	template <typename t_CStreamType, typename t_CVector>
	void const *TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_GetBufferConst() const
	{
		return mp_Buffer.f_GetArray();
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_GetVector() const -> CStorage
	{
		CStorage Ret;
		Ret.f_Insert(mp_Buffer.f_GetArray(), mp_Length);
		return Ret;
	}

	template <typename t_CStreamType, typename t_CVector>
	auto TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_GetVectorOptimized() const -> CStorage
	{
		CStorage Ret;
		Ret.f_Insert(mp_Buffer.f_GetArray(), mp_Length);
		return Ret;
	}

	template <typename t_CStreamType, typename t_CVector>
	TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::TCBinaryStreamMemoryConstRef(CStorage const &_Buffer)
		: mp_Buffer(_Buffer)
	{
		mp_Position = 0;
		mp_Length = _Buffer.f_GetLen();
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_ResetStream()
	{
		mp_Position = 0;
		mp_Length = mp_Buffer.f_GetLen();
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_Clear()
	{
		DMibError("Const stream cannot be cleared");
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_RemoveData(umint _Pos, umint _Len)
	{
		mp_Buffer.f_Remove(_Pos, _Len);
		mp_Length -= _Len;
		if (mp_Position > _Pos && mp_Position <= _Pos + _Len)
			mp_Position = _Pos;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_FeedBytes(void const *_pMem, umint _nBytes)
	{
		DMibError("Const stream cannot be written to");
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_ConsumeBytes(void *_pMem, umint _nBytes)
	{
		if (mp_Length < (mp_Position + _nBytes)) [[unlikely]]
			this->fp_ThrowEndOfStreamException();

		if (_nBytes != 0)
			NMemory::fg_MemCopy(_pMem, mp_Buffer.f_GetArray() + mp_Position, _nBytes);

		mp_Position += _nBytes;
	}

	template <typename t_CStreamType, typename t_CVector>
	bool TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_IsValid() const
	{
		return true;
	}

	template <typename t_CStreamType, typename t_CVector>
	bool TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_IsAtEndOfStream() const
	{
		return mp_Position == mp_Length;
	}

	template <typename t_CStreamType, typename t_CVector>
	CFilePos TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_GetPosition() const
	{
		return mp_Position;
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_SetPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(_Pos);
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_SetPositionFromEnd(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Length + _Pos);
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_AddPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Position + _Pos);
	}

	template <typename t_CStreamType, typename t_CVector>
	bool TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_IsValidReadPosition(NStream::CFilePos _Pos) const
	{
		return _Pos >= 0 && _Pos < NStream::CFilePos(mp_Length);
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_Flush(bool _bLocalCacheOnly)
	{
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_SetCacheSize(umint _CacheSize)
	{
	}

	template <typename t_CStreamType, typename t_CVector>
	CFilePos TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_GetLength() const
	{
		return mp_Length;
	}

	template <typename t_CStreamType, typename t_CVector>
	umint TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_ContainerLengthLimit() const
	{
		return f_GetLength() - f_GetPosition();
	}

	template <typename t_CStreamType, typename t_CVector>
	void TCBinaryStreamMemoryConstRef<t_CStreamType, t_CVector>::f_SetLength(NStream::CFilePos _Length)
	{
		DMibError("Const stream cannot change length");
	}

	template <typename t_CStreamType>
	TCBinaryStreamMemoryPtr<t_CStreamType>::TCBinaryStreamMemoryPtr()
	{
		mp_MaxLength = 0;
		mp_Mode = 0;
		mp_Position = 0;
		mp_Length = 0;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamMemoryPtr<t_CStreamType>::f_OpenRead(void const *_pData, umint _Length)
	{
		mp_pMemoryData = (uint8 *)_pData;
		mp_Length = _Length;
		mp_MaxLength = 0; // We can write no bytes
	}

	template <typename t_CStreamType>
	template <typename tf_CVector>
	void TCBinaryStreamMemoryPtr<t_CStreamType>::f_OpenRead(tf_CVector const &_Buffer)
	{
		mp_pMemoryData = (uint8 *)_Buffer.f_GetArray();
		mp_Length = _Buffer.f_GetLen();
		mp_MaxLength = 0; // We can write no bytes
	}

	template <typename t_CStreamType>
	void TCBinaryStreamMemoryPtr<t_CStreamType>::f_OpenReadWrite(void *_pData, umint _MaxLength, umint _Length)
	{
		mp_pMemoryData = (uint8 *)_pData;
		mp_Length = _Length;
		mp_MaxLength = _MaxLength;
	}

	template <typename t_CStreamType>
	template <typename tf_CVector>
	void TCBinaryStreamMemoryPtr<t_CStreamType>::f_OpenReadWrite(tf_CVector &_Vector, umint _Length)
	{
		mp_pMemoryData = (uint8 *)_Vector.f_GetArray();
		mp_Length = _Length;
		mp_MaxLength = _Vector.f_GetLen();
	}

	template <typename t_CStreamType>
	void TCBinaryStreamMemoryPtr<t_CStreamType>::f_FeedBytes(void const *_pMem, umint _nBytes)
	{
		if (mp_MaxLength < (mp_Position + _nBytes)) [[unlikely]]
			this->fp_ThrowEndOfStreamException();

		if (_nBytes != 0)
			NMemory::fg_MemCopy(mp_pMemoryData + mp_Position, _pMem, _nBytes);

		mp_Position += _nBytes;
		if (mp_Position > mp_Length)
			mp_Length = mp_Position;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamMemoryPtr<t_CStreamType>::f_ConsumeBytes(void *_pMem, umint _nBytes)
	{
		if (mp_Length < (mp_Position + _nBytes)) [[unlikely]]
			this->fp_ThrowEndOfStreamException();

		if (_nBytes != 0)
			NMemory::fg_MemCopy(_pMem, mp_pMemoryData + mp_Position, _nBytes);

		mp_Position += _nBytes;
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamMemoryPtr<t_CStreamType>::f_IsValid() const
	{
		return true;
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamMemoryPtr<t_CStreamType>::f_IsAtEndOfStream() const
	{
		return mp_Position == mp_Length;
	}

	template <typename t_CStreamType>
	CFilePos TCBinaryStreamMemoryPtr<t_CStreamType>::f_GetPosition() const
	{
		return mp_Position;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamMemoryPtr<t_CStreamType>::f_SetPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(_Pos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamMemoryPtr<t_CStreamType>::f_SetPositionFromEnd(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Length + _Pos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamMemoryPtr<t_CStreamType>::f_AddPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Position + _Pos);
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamMemoryPtr<t_CStreamType>::f_IsValidReadPosition(NStream::CFilePos _Pos) const
	{
		return _Pos >= 0 && _Pos < NStream::CFilePos(mp_Length);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamMemoryPtr<t_CStreamType>::f_Flush(bool _bLocalCacheOnly)
	{
	}

	template <typename t_CStreamType>
	void TCBinaryStreamMemoryPtr<t_CStreamType>::f_SetCacheSize(umint _CacheSize)
	{
	}

	template <typename t_CStreamType>
	CFilePos TCBinaryStreamMemoryPtr<t_CStreamType>::f_GetLength() const
	{
		return mp_Length;
	}

	template <typename t_CStreamType>
	umint TCBinaryStreamMemoryPtr<t_CStreamType>::f_ContainerLengthLimit() const
	{
		return f_GetLength() - f_GetPosition();
	}

	template <typename t_CStreamType>
	void TCBinaryStreamMemoryPtr<t_CStreamType>::f_SetLength(NStream::CFilePos _Length)
	{
		if (_Length > NStream::CFilePos(mp_MaxLength)) [[unlikely]]
			this->fp_ThrowEndOfStreamException();

		if (_Length > NStream::CFilePos(mp_Length))
			NMemory::fg_SecureMemClear(mp_pMemoryData + mp_Length, _Length - mp_Length);
		mp_Length = _Length;
	}

	template <typename t_CStreamType>
	void *TCBinaryStreamMemoryPtr<t_CStreamType>::f_GetBuffer()
	{
		return (void *)mp_pMemoryData;
	}

	template <typename t_CStreamType>
	void const *TCBinaryStreamMemoryPtr<t_CStreamType>::f_GetBufferConst() const
	{
		return (void const *)mp_pMemoryData;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamMemoryPtr<t_CStreamType>::fp_SetPositionInternal(CFilePos _Pos)
	{
		if (_Pos < 0 || fg_SafeLargerThan(_Pos, umint(TCLimitsInt<umint>::mc_Max)))
			DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

		if (_Pos > (CFilePos)mp_Length)
			DMibError("Position is past end of stream");

		mp_Position = _Pos;
	}

	template <typename t_CStreamType>
	TCBinaryStreamConstMemoryPtr<t_CStreamType>::TCBinaryStreamConstMemoryPtr()
		: mp_Position(0)
		, mp_Length(0)
		, mp_pMemoryData(nullptr)
	{
	}

	template <typename t_CStreamType>
	TCBinaryStreamConstMemoryPtr<t_CStreamType>::TCBinaryStreamConstMemoryPtr(TCBinaryStreamConstMemoryPtr &&_ToMove)
		: mp_Position(_ToMove.mp_Position)
		, mp_Length(_ToMove.mp_Length)
		, mp_pMemoryData(_ToMove.mp_pMemoryData)
	{
		_ToMove.mp_Position = 0;
		_ToMove.mp_Length = 0;
		_ToMove.mp_pMemoryData = nullptr;
	}

	template <typename t_CStreamType>
	auto TCBinaryStreamConstMemoryPtr<t_CStreamType>::operator = (TCBinaryStreamConstMemoryPtr &&_ToMove) -> TCBinaryStreamConstMemoryPtr &
	{
		mp_Position = _ToMove.mp_Position;
		mp_Length = _ToMove.mp_Length;
		mp_pMemoryData = _ToMove.mp_pMemoryData;

		_ToMove.mp_Position = 0;
		_ToMove.mp_Length = 0;
		_ToMove.mp_pMemoryData = nullptr;

		return *this;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_OpenRead(void const *_pData, umint _Length)
	{
		mp_pMemoryData = (uint8 *)_pData;
		mp_Length = _Length;
		mp_Position = 0;
	}

	template <typename t_CStreamType>
	template <typename tf_CVector>
	void TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_OpenRead(tf_CVector const &_Buffer)
	{
		mp_pMemoryData = (uint8 *)_Buffer.f_GetArray();
		mp_Length = _Buffer.f_GetLen();
		mp_Position = 0;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_FeedBytes(void const *_pMem, umint _nBytes)
	{
		DMibError("Const stream cannot be written to");
	}

	template <typename t_CStreamType>
	void TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_ConsumeBytes(void *_pMem, umint _nBytes)
	{
		if (mp_Length < (mp_Position + _nBytes)) [[unlikely]]
			this->fp_ThrowEndOfStreamException();

		if (_nBytes != 0)
			NMemory::fg_MemCopy(_pMem, mp_pMemoryData + mp_Position, _nBytes);

		mp_Position += _nBytes;
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_IsValid() const
	{
		return mp_pMemoryData ? true : false;
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_IsAtEndOfStream() const
	{
		return mp_Position == mp_Length;
	}

	template <typename t_CStreamType>
	CFilePos TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_GetPosition() const
	{
		return mp_Position;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_SetPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(_Pos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_SetPositionFromEnd(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Length + _Pos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_AddPosition(CFilePos _Pos)
	{
		fp_SetPositionInternal(mp_Position + _Pos);
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_IsValidReadPosition(NStream::CFilePos _Pos) const
	{
		return _Pos >= 0 && _Pos < NStream::CFilePos(mp_Length);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_Flush(bool _bLocalCacheOnly)
	{
	}

	template <typename t_CStreamType>
	void TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_SetCacheSize(umint _CacheSize)
	{
	}

	template <typename t_CStreamType>
	CFilePos TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_GetLength() const
	{
		return mp_Length;
	}

	template <typename t_CStreamType>
	umint TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_ContainerLengthLimit() const
	{
		return f_GetLength() - f_GetPosition();
	}

	template <typename t_CStreamType>
	void TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_SetLength(NStream::CFilePos _Length)
	{
		DMibError("Const stream cannot be written to");
	}

	template <typename t_CStreamType>
	void *TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_GetBuffer()
	{
		DMibError("Const stream cannot be written to");
		return nullptr;
	}

	template <typename t_CStreamType>
	void const *TCBinaryStreamConstMemoryPtr<t_CStreamType>::f_GetBufferConst() const
	{
		return (void const *)mp_pMemoryData;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamConstMemoryPtr<t_CStreamType>::fp_SetPositionInternal(CFilePos _Pos)
	{
		if (_Pos < 0 || fg_SafeLargerThan(_Pos, umint(TCLimitsInt<umint>::mc_Max)))
			DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

		if (_Pos > (CFilePos)mp_Length)
			DMibError("Position is past end of stream");

		mp_Position = _Pos;
	}

	template <typename t_CStreamType>
	TCBinaryStreamSubStream<t_CStreamType>::TCBinaryStreamSubStream()
	{
		mp_pSubStream = nullptr;
		mp_SubPos = 0;
		mp_MaxLength = -1;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamSubStream<t_CStreamType>::f_Close()
	{
		mp_pSubStream = nullptr;
		mp_SubPos = 0;
		mp_MaxLength = -1;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamSubStream<t_CStreamType>::f_Open(CBinaryStream *_pSubStream, CFilePos _SubPos, CFilePos _MaxLength)
	{
		mp_pSubStream = _pSubStream;
		mp_SubPos = _SubPos;
		mp_MaxLength = _MaxLength;
		mp_pSubStream->f_SetPosition(mp_SubPos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamSubStream<t_CStreamType>::f_FeedBytes(void const *_pMem, umint _nBytes)
	{
		if (mp_MaxLength >= 0)
		{
			CFilePos ResultingPos = mp_pSubStream->f_GetPosition() + _nBytes;
			if (ResultingPos > mp_SubPos + mp_MaxLength)
				DMibError("Trying to write outside of stream");
		}

		mp_pSubStream->f_FeedBytes(_pMem, _nBytes);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamSubStream<t_CStreamType>::f_ConsumeBytes(void *_pMem, umint _nBytes)
	{
		if (mp_MaxLength >= 0)
		{
			CFilePos ResultingPos = mp_pSubStream->f_GetPosition() + _nBytes;
			if (ResultingPos > mp_SubPos + mp_MaxLength)
				DMibError("Trying to read outside of stream");
		}
		mp_pSubStream->f_ConsumeBytes(_pMem, _nBytes);
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamSubStream<t_CStreamType>::f_IsValid() const
	{
		return mp_pSubStream->f_IsValid();
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamSubStream<t_CStreamType>::f_IsAtEndOfStream() const
	{
		if (mp_MaxLength >= 0)
		{
			if (mp_pSubStream->f_GetPosition() >= mp_SubPos + mp_MaxLength)
				return true;
		}
		return mp_pSubStream->f_IsAtEndOfStream();
	}

	template <typename t_CStreamType>
	CFilePos TCBinaryStreamSubStream<t_CStreamType>::f_GetPosition() const
	{
		return mp_pSubStream->f_GetPosition() - mp_SubPos;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamSubStream<t_CStreamType>::f_SetPosition(CFilePos _Pos)
	{
		if (mp_MaxLength >= 0)
		{
			if (_Pos - mp_SubPos > mp_MaxLength)
				DMibError("Trying to seek outside of stream");
		}
		if (_Pos < 0)
			DMibError("Trying to seek outside of stream");
		return mp_pSubStream->f_SetPosition(mp_SubPos + _Pos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamSubStream<t_CStreamType>::f_SetPositionFromEnd(CFilePos _Pos)
	{
		CFilePos ResultingPos;
		if (mp_MaxLength >= 0)
		{
			if (_Pos > 0)
				DMibError("Trying to seek outside of stream");
			ResultingPos = mp_SubPos + (mp_MaxLength + _Pos);
		}
		else
			ResultingPos = mp_pSubStream->f_GetLength() + _Pos;
		if (ResultingPos < mp_SubPos)
			DMibError("Trying to seek outside of stream");

		return mp_pSubStream->f_SetPosition(_Pos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamSubStream<t_CStreamType>::f_AddPosition(CFilePos _Pos)
	{
		CFilePos ResultingPos = mp_pSubStream->f_GetPosition() + _Pos;

		if (mp_MaxLength >= 0)
		{
			if (ResultingPos > mp_SubPos + mp_MaxLength)
				DMibError("Trying to seek outside of stream");
		}
		if (ResultingPos < mp_SubPos)
			DMibError("Trying to seek outside of stream");

		return mp_pSubStream->f_SetPosition(ResultingPos);
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamSubStream<t_CStreamType>::f_IsValidReadPosition(NStream::CFilePos _Pos) const
	{
		if (mp_MaxLength >= 0)
		{
			if (_Pos - mp_SubPos >= mp_MaxLength)
				return false;
		}
		if (_Pos < 0)
			return false;
		return mp_pSubStream->f_IsValidReadPosition(mp_SubPos + _Pos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamSubStream<t_CStreamType>::f_Flush(bool _bLocalCacheOnly)
	{
		mp_pSubStream->f_Flush(_bLocalCacheOnly);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamSubStream<t_CStreamType>::f_SetCacheSize(umint _CacheSize)
	{
		mp_pSubStream->f_SetCacheSize(_CacheSize);
	}

	template <typename t_CStreamType>
	CFilePos TCBinaryStreamSubStream<t_CStreamType>::f_GetLength() const
	{
		if (mp_MaxLength >= 0)
			return fg_Min(mp_pSubStream->f_GetLength() - mp_SubPos, mp_MaxLength);
		else
			return mp_pSubStream->f_GetLength() - mp_SubPos;
	}

	template <typename t_CStreamType>
	umint TCBinaryStreamSubStream<t_CStreamType>::f_ContainerLengthLimit() const
	{
		return NStream::fg_CapLengthLimit(f_GetLength() - f_GetPosition());
	}

	template <typename t_CStreamType>
	void TCBinaryStreamSubStream<t_CStreamType>::f_SetLength(NStream::CFilePos _Length)
	{
		return mp_pSubStream->f_SetLength(mp_SubPos + _Length);
	}

	template <typename t_CStreamType>
	TCBinaryStreamCompare<t_CStreamType>::TCBinaryStreamCompare()
	{
		mp_pCompareToStream = nullptr;
		mp_pWriteToStream = nullptr;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamCompare<t_CStreamType>::f_Close()
	{
		mp_pCompareToStream = nullptr;
		mp_pWriteToStream = nullptr;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamCompare<t_CStreamType>::f_Open(CBinaryStream *_pWriteStream, CBinaryStream *_pCompareToStream)
	{
		mp_pCompareToStream = _pCompareToStream;
		mp_pWriteToStream = _pWriteStream;
	}

	template <typename t_CStreamType>
	void TCBinaryStreamCompare<t_CStreamType>::f_FeedBytes(void const *_pMem, umint _nBytes)
	{
		mp_TempVector.f_SetAtLeastLen(_nBytes);

		auto NewPos = mp_pWriteToStream->f_GetPosition();
		if (!mp_pCompareToStream->f_IsValidReadPosition(NewPos))
			DMibPDebugBreak;

		mp_pCompareToStream->f_ConsumeBytes(mp_TempVector.f_GetArray(), _nBytes);

		if (NMib::NMemory::fg_MemCmp((uint8 const *)_pMem, mp_TempVector.f_GetArray(), _nBytes) != 0)
			DMibPDebugBreak;

		mp_pWriteToStream->f_FeedBytes(_pMem, _nBytes);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamCompare<t_CStreamType>::f_ConsumeBytes(void *_pMem, umint _nBytes)
	{
		mp_TempVector.f_SetAtLeastLen(_nBytes);

		auto NewPos = mp_pWriteToStream->f_GetPosition();
		if (!mp_pCompareToStream->f_IsValidReadPosition(NewPos))
			DMibPDebugBreak;

		mp_pCompareToStream->f_ConsumeBytes(mp_TempVector.f_GetArray(), _nBytes);
		mp_pWriteToStream->f_ConsumeBytes(_pMem, _nBytes);

		if (NMib::NMemory::fg_MemCmp((uint8 const *)_pMem, mp_TempVector.f_GetArray(), _nBytes) != 0)
			DMibPDebugBreak;
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamCompare<t_CStreamType>::f_IsValid() const
	{
		return mp_pWriteToStream->f_IsValid();
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamCompare<t_CStreamType>::f_IsAtEndOfStream() const
	{
		return mp_pWriteToStream->f_IsAtEndOfStream();
	}

	template <typename t_CStreamType>
	CFilePos TCBinaryStreamCompare<t_CStreamType>::f_GetPosition() const
	{
		return mp_pWriteToStream->f_GetPosition();
	}

	template <typename t_CStreamType>
	void TCBinaryStreamCompare<t_CStreamType>::f_SetPosition(CFilePos _Pos)
	{
		mp_pWriteToStream->f_SetPosition(_Pos);
		auto NewPos = mp_pWriteToStream->f_GetPosition();
		if (!mp_pCompareToStream->f_IsValidReadPosition(NewPos))
			DMibPDebugBreak;
		mp_pCompareToStream->f_SetPosition(NewPos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamCompare<t_CStreamType>::f_SetPositionFromEnd(CFilePos _Pos)
	{
		mp_pWriteToStream->f_SetPositionFromEnd(_Pos);
		auto NewPos = mp_pWriteToStream->f_GetPosition();
		if (!mp_pCompareToStream->f_IsValidReadPosition(NewPos))
			DMibPDebugBreak;
		mp_pCompareToStream->f_SetPosition(NewPos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamCompare<t_CStreamType>::f_AddPosition(CFilePos _Pos)
	{
		mp_pWriteToStream->f_AddPosition(_Pos);
		auto NewPos = mp_pWriteToStream->f_GetPosition();
		if (!mp_pCompareToStream->f_IsValidReadPosition(NewPos))
			DMibPDebugBreak;
		mp_pCompareToStream->f_SetPosition(NewPos);
	}

	template <typename t_CStreamType>
	bool TCBinaryStreamCompare<t_CStreamType>::f_IsValidReadPosition(NStream::CFilePos _Pos) const
	{
		return mp_pWriteToStream->f_IsValidReadPosition(_Pos);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamCompare<t_CStreamType>::f_Flush(bool _bLocalCacheOnly)
	{
		mp_pWriteToStream->f_Flush(_bLocalCacheOnly);
	}

	template <typename t_CStreamType>
	void TCBinaryStreamCompare<t_CStreamType>::f_SetCacheSize(umint _CacheSize)
	{
		mp_pWriteToStream->f_SetCacheSize(_CacheSize);
	}

	template <typename t_CStreamType>
	CFilePos TCBinaryStreamCompare<t_CStreamType>::f_GetLength() const
	{
		return mp_pWriteToStream->f_GetLength();
	}

	template <typename t_CStreamType>
	umint TCBinaryStreamCompare<t_CStreamType>::f_ContainerLengthLimit() const
	{
		return mp_pWriteToStream->f_ContainerLengthLimit();
	}

	template <typename t_CStreamType>
	void TCBinaryStreamCompare<t_CStreamType>::f_SetLength(NStream::CFilePos _Length)
	{
		return mp_pWriteToStream->f_SetLength(_Length);
	}
}
