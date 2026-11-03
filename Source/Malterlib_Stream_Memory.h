// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Core/Core>

namespace NMib::NStream
{
	template <typename t_CStreamType = NStream::CBinaryStreamDefault, typename t_CVector = NContainer::CByteVector>
	struct TCBinaryStreamMemory : public t_CStreamType
	{
		using CStorage = t_CVector;

	protected:
		umint mp_Position;
		umint mp_Length;
		umint mp_BufferSize;
		uint8 *mp_pBuffer;

		CStorage mp_Buffer;

		inline_never void fp_GrowBufferGrow(umint _NeededBytes)
		{
			if (mp_BufferSize == 0 && _NeededBytes < 2048)
				_NeededBytes = 2048;
			mp_Buffer.f_Grow(_NeededBytes);
			mp_pBuffer = mp_Buffer.f_GetArray();
			mp_BufferSize = mp_Buffer.f_GetLen();
		}

		inline_small void fp_GrowBuffer(umint _NeededBytes)
		{
			umint CurrentLen = mp_BufferSize;
			umint NeededSize = _NeededBytes + mp_Position;
			if (CurrentLen < NeededSize)
				fp_GrowBufferGrow(NeededSize);
		}

		void fp_SetPositionInternal(CFilePos _Pos)
		{
			if (_Pos < 0 || fg_SafeLargerThan(_Pos, umint(TCLimitsInt<umint>::mc_Max)))
				DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

			mp_Position = _Pos;
		}
		DMibStreamImplementProtected(TCBinaryStreamMemory);

	public:
		DMibStreamImplementOperators(TCBinaryStreamMemory);

		void *f_GetBuffer()
		{
			return mp_pBuffer;
		}

		void const *f_GetBufferConst() const
		{
			return mp_pBuffer;
		}

		void f_GrowBuffer(umint _nBytes)
		{
			fp_GrowBuffer(_nBytes);
		}

		CStorage &f_GetVector()
		{
			mp_Buffer.f_SetLen(mp_Length, false);
			mp_pBuffer = mp_Buffer.f_GetArray();
			mp_BufferSize = mp_Buffer.f_GetLen();
			return (CStorage &)mp_Buffer;
		}

		CStorage &f_GetVectorOptimized()
		{
			mp_Buffer.f_SetLen(mp_Length, true);
			mp_pBuffer = mp_Buffer.f_GetArray();
			mp_BufferSize = mp_Buffer.f_GetLen();
			return (CStorage &)mp_Buffer;
		}

		CStorage f_GetVector() const
		{
			CStorage Ret;
			Ret.f_Insert(mp_pBuffer, mp_Length);
			return Ret;
		}

		CStorage f_GetVectorOptimized() const
		{
			CStorage Ret;
			Ret.f_Insert(mp_pBuffer, mp_Length);
			return Ret;
		}

		CStorage f_MoveVector()
		{
			mp_Buffer.f_SetLen(mp_Length, false);
			mp_Length = 0;
			mp_BufferSize = 0;
			mp_Position = 0;
			return fg_Move(mp_Buffer);
		}

		CStorage f_MoveVectorOptimized()
		{
			mp_Buffer.f_SetLen(mp_Length, true);
			mp_Length = 0;
			mp_BufferSize = 0;
			mp_Position = 0;
			return fg_Move(mp_Buffer);
		}

		TCBinaryStreamMemory(CStorage const &_Buffer)
			: mp_Buffer(_Buffer)
		{
			mp_Position = 0;
			mp_Length = _Buffer.f_GetLen();
			mp_BufferSize = mp_Length;
			mp_pBuffer = mp_Buffer.f_GetArray();
		}

		TCBinaryStreamMemory(CStorage &&_Buffer)
			: mp_Buffer(fg_Move(_Buffer))
		{
			mp_Position = 0;
			mp_Length = mp_Buffer.f_GetLen();
			mp_BufferSize = mp_Length;
			mp_pBuffer = mp_Buffer.f_GetArray();
		}

		TCBinaryStreamMemory(TCBinaryStreamMemory &&_ToMove)
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

		TCBinaryStreamMemory &operator = (TCBinaryStreamMemory &&_ToMove)
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

		TCBinaryStreamMemory()
		{
			mp_Position = 0;
			mp_Length = 0;
			mp_BufferSize = 0;
			mp_pBuffer = nullptr;
		}

		void f_Open(CStorage const &_Buffer)
		{
			mp_Buffer = _Buffer;
			mp_Position = 0;
			mp_Length = _Buffer.f_GetLen();
			mp_BufferSize = mp_Length;
			mp_pBuffer = mp_Buffer.f_GetArray();
		}

		void f_Open(CStorage &&_Buffer)
		{
			mp_Buffer = fg_Move(_Buffer);
			mp_Position = 0;
			mp_Length = mp_Buffer.f_GetLen();
			mp_BufferSize = mp_Length;
			mp_pBuffer = mp_Buffer.f_GetArray();
		}

		void f_ResetStream()
		{
			mp_Position = 0;
			mp_Length = 0;
		}

		void f_Clear()
		{
			mp_Position = 0;
			mp_Length = 0;
			mp_BufferSize = 0;
			mp_Buffer.f_Clear();
		}

		void f_RemoveData(umint _Pos, umint _Len)
		{
			mp_Buffer.f_Remove(_Pos, _Len);
			mp_pBuffer = mp_Buffer.f_GetArray();
			mp_BufferSize = mp_Buffer.f_GetLen();
			mp_Length -= _Len;

			if (mp_Position > _Pos && mp_Position <= _Pos + _Len)
				mp_Position = _Pos;
		}

		void f_FeedBytes(void const *_pMem, umint _nBytes)
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

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			if (mp_Length < (mp_Position + _nBytes)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();

			if (_nBytes != 0)
				NMemory::fg_MemCopy(_pMem, mp_pBuffer + mp_Position, _nBytes);

			mp_Position += _nBytes;
		}

		bool f_IsValid() const
		{
			return true;
		}

		bool f_IsAtEndOfStream() const
		{
			return mp_Position == mp_Length;
		}

		CFilePos f_GetPosition() const
		{
			return mp_Position;
		}

		void f_SetPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(_Pos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			fp_SetPositionInternal(mp_Length + _Pos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(mp_Position + _Pos);
		}

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return _Pos >= 0 && _Pos < NStream::CFilePos(mp_Length);
		}

		void f_Flush(bool _bLocalCacheOnly)
		{
		}

		void f_SetCacheSize(umint _CacheSize)
		{
		}

		CFilePos f_GetLength() const
		{
			return mp_Length;
		}

		umint f_ContainerLengthLimit() const
		{
			return f_GetLength() - f_GetPosition();
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			if (_Length > NStream::CFilePos(mp_Length))
			{
				fp_GrowBuffer(_Length - mp_Length);
				NMemory::fg_SecureMemClear(mp_pBuffer + mp_Length, _Length - mp_Length);
			}
			mp_Length = _Length;
		}

	private:
		TCBinaryStreamMemory(TCBinaryStreamMemory const &_Other) = delete;
		TCBinaryStreamMemory &operator = (TCBinaryStreamMemory const &_Other) = delete;
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault, typename t_CVector = NContainer::CByteVector>
	struct TCBinaryStreamMemoryRef : public t_CStreamType
	{
		using CStorage = t_CVector;

	protected:
		umint mp_Position;
		umint mp_Length;

		CStorage &mp_Buffer;

		void fp_GrowBuffer(umint _NeededBytes)
		{
			umint CurrentLen = mp_Buffer.f_GetLen();
			umint NeededSize = _NeededBytes + mp_Position;
			if (CurrentLen < NeededSize)
				mp_Buffer.f_Grow(NeededSize);
		}

		void fp_SetPositionInternal(CFilePos _Pos)
		{
			if ((_Pos < 0) || fg_SafeLargerThan(_Pos, TCLimitsInt<umint>::mc_Max))
				DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

			// if (_Pos > (CFilePos)mp_Length)
			// 	DMibError("Position is past end of stream");

			mp_Position = _Pos;
		}
		DMibStreamImplementProtected(TCBinaryStreamMemoryRef);

	public:
		DMibStreamImplementOperators(TCBinaryStreamMemoryRef);

		void *f_GetBuffer()
		{
			return mp_Buffer.f_GetArray();
		}

		void const *f_GetBufferConst() const
		{
			return mp_Buffer.f_GetArray();
		}

		CStorage &f_GetVector()
		{
			mp_Buffer.f_SetLen(mp_Length, false);
			return (CStorage &)mp_Buffer;
		}

		CStorage &f_GetVectorOptimized()
		{
			mp_Buffer.f_SetLen(mp_Length, true);
			return (CStorage &)mp_Buffer;
		}

		CStorage f_GetVector() const
		{
			CStorage Ret;
			Ret.f_Insert(mp_Buffer.f_GetArray(), mp_Length);
			return Ret;
		}

		CStorage f_GetVectorOptimized() const
		{
			CStorage Ret;
			Ret.f_Insert(mp_Buffer.f_GetArray(), mp_Length);
			return Ret;
		}

		CStorage &&f_MoveVector()
		{
			mp_Buffer.f_SetLen(mp_Length, false);
			mp_Length = 0;
			mp_Position = 0;
			return (CStorage &&)mp_Buffer;
		}

		CStorage &&f_MoveVectorOptimized()
		{
			mp_Buffer.f_SetLen(mp_Length, true);
			mp_Length = 0;
			mp_Position = 0;
			return (CStorage &&)mp_Buffer;
		}

		TCBinaryStreamMemoryRef(CStorage &_Buffer)
			: mp_Buffer(_Buffer)
		{
			mp_Position = 0;
			mp_Length = _Buffer.f_GetLen();
		}

		TCBinaryStreamMemoryRef(CStorage &_Buffer, umint _Length)
			: mp_Buffer(_Buffer)
		{
			mp_Position = 0;
			mp_Length = _Length;
		}

		~TCBinaryStreamMemoryRef()
		{
			try
			{
				mp_Buffer.f_SetLen(mp_Length, false);
			}
			catch (...)
			{
			}
		}

		void f_ResetStream()
		{
			mp_Position = 0;
			mp_Length = 0;
		}

		void f_Clear()
		{
			mp_Position = 0;
			mp_Length = 0;
			mp_Buffer.f_Clear();
		}

		void f_RemoveData(umint _Pos, umint _Len)
		{
			mp_Buffer.f_Remove(_Pos, _Len);
			mp_Length -= _Len;
			if (mp_Position > _Pos && mp_Position <= _Pos + _Len)
				mp_Position = _Pos;
		}

		void f_FeedBytes(void const *_pMem, umint _nBytes)
		{
			fp_GrowBuffer(_nBytes);

			if (_nBytes != 0)
				NMemory::fg_MemCopy(mp_Buffer.f_GetArray() + mp_Position, _pMem, _nBytes);

			mp_Position += _nBytes;
			if (mp_Position > mp_Length)
				mp_Length = mp_Position;
		}

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			if (mp_Length < (mp_Position + _nBytes)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();

			if (_nBytes != 0)
				NMemory::fg_MemCopy(_pMem, mp_Buffer.f_GetArray() + mp_Position, _nBytes);

			mp_Position += _nBytes;
		}

		bool f_IsValid() const
		{
			return true;
		}

		bool f_IsAtEndOfStream() const
		{
			return mp_Position == mp_Length;
		}

		CFilePos f_GetPosition() const
		{
			return mp_Position;
		}

		void f_SetPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(_Pos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			fp_SetPositionInternal(mp_Length + _Pos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(mp_Position + _Pos);
		}

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return _Pos >= 0 && _Pos < NStream::CFilePos(mp_Length);
		}

		void f_Flush(bool _bLocalCacheOnly)
		{
		}

		void f_SetCacheSize(umint _CacheSize)
		{
		}

		CFilePos f_GetLength() const
		{
			return mp_Length;
		}

		umint f_ContainerLengthLimit() const
		{
			return f_GetLength() - f_GetPosition();
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			if (_Length > NStream::CFilePos(mp_Length))
			{
				fp_GrowBuffer(_Length - mp_Length);
				NMemory::fg_SecureMemClear(mp_Buffer.f_GetArray() + mp_Length, _Length - mp_Length);
			}
			mp_Length = _Length;
		}

	private:
		TCBinaryStreamMemoryRef(TCBinaryStreamMemoryRef const &_Other) = delete;
		TCBinaryStreamMemoryRef &operator = (TCBinaryStreamMemoryRef const &_Other) = delete;
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault, typename t_CVector = NContainer::CByteVector>
	struct TCBinaryStreamMemoryConstRef : public t_CStreamType
	{
		using CStorage = t_CVector;

	protected:
		umint mp_Position;
		umint mp_Length;

		CStorage const &mp_Buffer;

		void fp_SetPositionInternal(CFilePos _Pos)
		{
			if ((_Pos < 0) || fg_SafeLargerThan(_Pos, TCLimitsInt<umint>::mc_Max))
				DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

			if (_Pos > (CFilePos)mp_Length)
				DMibError("Position is past end of stream");

			mp_Position = _Pos;
		}
		DMibStreamImplementProtected(TCBinaryStreamMemoryConstRef);

	public:
		DMibStreamImplementOperators(TCBinaryStreamMemoryConstRef);

		void *f_GetBuffer()
		{
			return mp_Buffer.f_GetArray();
		}

		void const *f_GetBufferConst() const
		{
			return mp_Buffer.f_GetArray();
		}

		CStorage f_GetVector() const
		{
			CStorage Ret;
			Ret.f_Insert(mp_Buffer.f_GetArray(), mp_Length);
			return Ret;
		}

		CStorage f_GetVectorOptimized() const
		{
			CStorage Ret;
			Ret.f_Insert(mp_Buffer.f_GetArray(), mp_Length);
			return Ret;
		}

		TCBinaryStreamMemoryConstRef(CStorage const &_Buffer)
			: mp_Buffer(_Buffer)
		{
			mp_Position = 0;
			mp_Length = _Buffer.f_GetLen();
		}

		void f_ResetStream()
		{
			mp_Position = 0;
			mp_Length = mp_Buffer.f_GetLen();
		}

		void f_Clear()
		{
			DMibError("Const stream cannot be cleared");
		}

		void f_RemoveData(umint _Pos, umint _Len)
		{
			mp_Buffer.f_Remove(_Pos, _Len);
			mp_Length -= _Len;
			if (mp_Position > _Pos && mp_Position <= _Pos + _Len)
				mp_Position = _Pos;
		}

		void f_FeedBytes(void const *_pMem, umint _nBytes)
		{
			DMibError("Const stream cannot be written to");
		}

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			if (mp_Length < (mp_Position + _nBytes)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();

			if (_nBytes != 0)
				NMemory::fg_MemCopy(_pMem, mp_Buffer.f_GetArray() + mp_Position, _nBytes);

			mp_Position += _nBytes;
		}

		bool f_IsValid() const
		{
			return true;
		}

		bool f_IsAtEndOfStream() const
		{
			return mp_Position == mp_Length;
		}

		CFilePos f_GetPosition() const
		{
			return mp_Position;
		}

		void f_SetPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(_Pos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			fp_SetPositionInternal(mp_Length + _Pos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(mp_Position + _Pos);
		}

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return _Pos >= 0 && _Pos < NStream::CFilePos(mp_Length);
		}

		void f_Flush(bool _bLocalCacheOnly)
		{
		}

		void f_SetCacheSize(umint _CacheSize)
		{
		}

		CFilePos f_GetLength() const
		{
			return mp_Length;
		}

		umint f_ContainerLengthLimit() const
		{
			return f_GetLength() - f_GetPosition();
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			DMibError("Const stream cannot change length");
		}

	private:
		TCBinaryStreamMemoryConstRef(TCBinaryStreamMemoryConstRef const &_Other) = delete;
		TCBinaryStreamMemoryConstRef &operator = (TCBinaryStreamMemoryConstRef const &_Other) = delete;
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	struct TCBinaryStreamMemoryPtr : public t_CStreamType
	{
		DMibStreamImplementOperators(TCBinaryStreamMemoryPtr);

		TCBinaryStreamMemoryPtr()
		{
			mp_MaxLength = 0;
			mp_Mode = 0;
			mp_Position = 0;
			mp_Length = 0;
		}

		void f_OpenRead(void const *_pData, umint _Length)
		{
			mp_pMemoryData = (uint8 *)_pData;
			mp_Length = _Length;
			mp_MaxLength = 0; // We can write no bytes
		}

		template <typename tf_CVector>
		void f_OpenRead(tf_CVector const &_Buffer)
		{
			mp_pMemoryData = (uint8 *)_Buffer.f_GetArray();
			mp_Length = _Buffer.f_GetLen();
			mp_MaxLength = 0; // We can write no bytes
		}

		void f_OpenReadWrite(void *_pData, umint _MaxLength, umint _Length = 0)
		{
			mp_pMemoryData = (uint8 *)_pData;
			mp_Length = _Length;
			mp_MaxLength = _MaxLength;
		}

		template <typename tf_CVector>
		void f_OpenReadWrite(tf_CVector &_Vector, umint _Length = 0)
		{
			mp_pMemoryData = (uint8 *)_Vector.f_GetArray();
			mp_Length = _Length;
			mp_MaxLength = _Vector.f_GetLen();
		}

		void f_FeedBytes(void const *_pMem, umint _nBytes)
		{
			if (mp_MaxLength < (mp_Position + _nBytes)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();

			if (_nBytes != 0)
				NMemory::fg_MemCopy(mp_pMemoryData + mp_Position, _pMem, _nBytes);

			mp_Position += _nBytes;
			if (mp_Position > mp_Length)
				mp_Length = mp_Position;
		}

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			if (mp_Length < (mp_Position + _nBytes)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();

			if (_nBytes != 0)
				NMemory::fg_MemCopy(_pMem, mp_pMemoryData + mp_Position, _nBytes);

			mp_Position += _nBytes;
		}

		bool f_IsValid() const
		{
			return true;
		}

		bool f_IsAtEndOfStream() const
		{
			return mp_Position == mp_Length;
		}

		CFilePos f_GetPosition() const
		{
			return mp_Position;
		}

		void f_SetPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(_Pos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			fp_SetPositionInternal(mp_Length + _Pos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(mp_Position + _Pos);
		}

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return _Pos >= 0 && _Pos < NStream::CFilePos(mp_Length);
		}

		void f_Flush(bool _bLocalCacheOnly)
		{
		}

		void f_SetCacheSize(umint _CacheSize)
		{
		}

		CFilePos f_GetLength() const
		{
			return mp_Length;
		}

		umint f_ContainerLengthLimit() const
		{
			return f_GetLength() - f_GetPosition();
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			if (_Length > NStream::CFilePos(mp_MaxLength)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();

			if (_Length > NStream::CFilePos(mp_Length))
				NMemory::fg_SecureMemClear(mp_pMemoryData + mp_Length, _Length - mp_Length);
			mp_Length = _Length;
		}

		void *f_GetBuffer()
		{
			return (void *)mp_pMemoryData;
		}

		void const *f_GetBufferConst() const
		{
			return (void const *)mp_pMemoryData;
		}

	private:
		TCBinaryStreamMemoryPtr(TCBinaryStreamMemoryPtr const &_Other) = delete;
		TCBinaryStreamMemoryPtr &operator = (TCBinaryStreamMemoryPtr const &_Other) = delete;

	protected:
		umint mp_Position;
		umint mp_Length;
		umint mp_MaxLength;
		aint mp_Mode;
		uint8 *mp_pMemoryData;

		void fp_SetPositionInternal(CFilePos _Pos)
		{
			if (_Pos < 0 || fg_SafeLargerThan(_Pos, umint(TCLimitsInt<umint>::mc_Max)))
				DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

			if (_Pos > (CFilePos)mp_Length)
				DMibError("Position is past end of stream");

			mp_Position = _Pos;
		}
		DMibStreamImplementProtected(TCBinaryStreamMemoryPtr);
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	struct TCBinaryStreamConstMemoryPtr : public t_CStreamType
	{
		DMibStreamImplementOperators(TCBinaryStreamConstMemoryPtr);

		TCBinaryStreamConstMemoryPtr()
			: mp_Position(0)
			, mp_Length(0)
			, mp_pMemoryData(nullptr)
		{
		}

		TCBinaryStreamConstMemoryPtr(TCBinaryStreamConstMemoryPtr &&_ToMove)
			: mp_Position(_ToMove.mp_Position)
			, mp_Length(_ToMove.mp_Length)
			, mp_pMemoryData(_ToMove.mp_pMemoryData)
		{
			_ToMove.mp_Position = 0;
			_ToMove.mp_Length = 0;
			_ToMove.mp_pMemoryData = nullptr;
		}

		TCBinaryStreamConstMemoryPtr &operator = (TCBinaryStreamConstMemoryPtr &&_ToMove)
		{
			mp_Position = _ToMove.mp_Position;
			mp_Length = _ToMove.mp_Length;
			mp_pMemoryData = _ToMove.mp_pMemoryData;

			_ToMove.mp_Position = 0;
			_ToMove.mp_Length = 0;
			_ToMove.mp_pMemoryData = nullptr;
		}

		void f_OpenRead(void const *_pData, umint _Length)
		{
			mp_pMemoryData = (uint8 *)_pData;
			mp_Length = _Length;
			mp_Position = 0;
		}

		template <typename tf_CVector>
		void f_OpenRead(tf_CVector const &_Buffer)
		{
			mp_pMemoryData = (uint8 *)_Buffer.f_GetArray();
			mp_Length = _Buffer.f_GetLen();
			mp_Position = 0;
		}

		void f_FeedBytes(void const *_pMem, umint _nBytes)
		{
			DMibError("Const stream cannot be written to");
		}

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			if (mp_Length < (mp_Position + _nBytes)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();

			if (_nBytes != 0)
				NMemory::fg_MemCopy(_pMem, mp_pMemoryData + mp_Position, _nBytes);

			mp_Position += _nBytes;
		}

		bool f_IsValid() const
		{
			return mp_pMemoryData ? true : false;
		}

		bool f_IsAtEndOfStream() const
		{
			return mp_Position == mp_Length;
		}

		CFilePos f_GetPosition() const
		{
			return mp_Position;
		}

		void f_SetPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(_Pos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			fp_SetPositionInternal(mp_Length + _Pos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(mp_Position + _Pos);
		}

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return _Pos >= 0 && _Pos < NStream::CFilePos(mp_Length);
		}

		void f_Flush(bool _bLocalCacheOnly)
		{
		}

		void f_SetCacheSize(umint _CacheSize)
		{
		}

		CFilePos f_GetLength() const
		{
			return mp_Length;
		}

		umint f_ContainerLengthLimit() const
		{
			return f_GetLength() - f_GetPosition();
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			DMibError("Const stream cannot be written to");
		}

		void *f_GetBuffer()
		{
			DMibError("Const stream cannot be written to");
			return nullptr;
		}

		void const *f_GetBufferConst() const
		{
			return (void const *)mp_pMemoryData;
		}

	private:
		TCBinaryStreamConstMemoryPtr(TCBinaryStreamConstMemoryPtr const &_Other) = delete;
		TCBinaryStreamConstMemoryPtr &operator = (TCBinaryStreamConstMemoryPtr const &_Other) = delete;

	protected:
		umint mp_Position;
		umint mp_Length;
		uint8 const *mp_pMemoryData;

		void fp_SetPositionInternal(CFilePos _Pos)
		{
			if (_Pos < 0 || fg_SafeLargerThan(_Pos, umint(TCLimitsInt<umint>::mc_Max)))
				DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

			if (_Pos > (CFilePos)mp_Length)
				DMibError("Position is past end of stream");

			mp_Position = _Pos;
		}
		DMibStreamImplementProtected(TCBinaryStreamConstMemoryPtr);
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	struct TCBinaryStreamSubStream : public t_CStreamType
	{
	protected:
		CBinaryStream *mp_pSubStream;
		CFilePos mp_SubPos;
		CFilePos mp_MaxLength;

		DMibStreamImplementProtected(TCBinaryStreamSubStream);

	public:
		DMibStreamImplementOperators(TCBinaryStreamSubStream);

		TCBinaryStreamSubStream()
		{
			mp_pSubStream = nullptr;
			mp_SubPos = 0;
			mp_MaxLength = -1;
		}

		void f_Close()
		{
			mp_pSubStream = nullptr;
			mp_SubPos = 0;
			mp_MaxLength = -1;
		}

		void f_Open(CBinaryStream *_pSubStream, CFilePos _SubPos, CFilePos _MaxLength = -1)
		{
			mp_pSubStream = _pSubStream;
			mp_SubPos = _SubPos;
			mp_MaxLength = _MaxLength;
			mp_pSubStream->f_SetPosition(mp_SubPos);
		}

		void f_FeedBytes(void const *_pMem, umint _nBytes)
		{
			if (mp_MaxLength >= 0)
			{
				CFilePos ResultingPos = mp_pSubStream->f_GetPosition() + _nBytes;
				if (ResultingPos > mp_SubPos + mp_MaxLength)
					DMibError("Trying to write outside of stream");
			}

			mp_pSubStream->f_FeedBytes(_pMem, _nBytes);
		}

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			if (mp_MaxLength >= 0)
			{
				CFilePos ResultingPos = mp_pSubStream->f_GetPosition() + _nBytes;
				if (ResultingPos > mp_SubPos + mp_MaxLength)
					DMibError("Trying to read outside of stream");
			}
			mp_pSubStream->f_ConsumeBytes(_pMem, _nBytes);
		}

		bool f_IsValid() const
		{
			return mp_pSubStream->f_IsValid();
		}

		bool f_IsAtEndOfStream() const
		{
			if (mp_MaxLength >= 0)
			{
				if (mp_pSubStream->f_GetPosition() >= mp_SubPos + mp_MaxLength)
					return true;
			}
			return mp_pSubStream->f_IsAtEndOfStream();
		}

		CFilePos f_GetPosition() const
		{
			return mp_pSubStream->f_GetPosition() - mp_SubPos;
		}

		void f_SetPosition(CFilePos _Pos)
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

		void f_SetPositionFromEnd(CFilePos _Pos)
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

		void f_AddPosition(CFilePos _Pos)
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

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
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

		void f_Flush(bool _bLocalCacheOnly)
		{
			mp_pSubStream->f_Flush(_bLocalCacheOnly);
		}

		void f_SetCacheSize(umint _CacheSize)
		{
			mp_pSubStream->f_SetCacheSize(_CacheSize);
		}

		CFilePos f_GetLength() const
		{
			if (mp_MaxLength >= 0)
				return fg_Min(mp_pSubStream->f_GetLength() - mp_SubPos, mp_MaxLength);
			else
				return mp_pSubStream->f_GetLength() - mp_SubPos;
		}

		umint f_ContainerLengthLimit() const
		{
			return NStream::fg_CapLengthLimit(f_GetLength() - f_GetPosition());
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			return mp_pSubStream->f_SetLength(mp_SubPos + _Length);
		}
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	struct TCBinaryStreamCompare : public t_CStreamType
	{
	protected:
		CBinaryStream *mp_pCompareToStream;
		CBinaryStream *mp_pWriteToStream;
		NContainer::CByteVector mp_TempVector;

		DMibStreamImplementProtected(TCBinaryStreamCompare);

	public:
		DMibStreamImplementOperators(TCBinaryStreamCompare);

		TCBinaryStreamCompare()
		{
			mp_pCompareToStream = nullptr;
			mp_pWriteToStream = nullptr;
		}

		void f_Close()
		{
			mp_pCompareToStream = nullptr;
			mp_pWriteToStream = nullptr;
		}

		void f_Open(CBinaryStream *_pWriteStream, CBinaryStream *_pCompareToStream)
		{
			mp_pCompareToStream = _pCompareToStream;
			mp_pWriteToStream = _pWriteStream;
		}

		void f_FeedBytes(void const *_pMem, umint _nBytes)
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

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
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

		bool f_IsValid() const
		{
			return mp_pWriteToStream->f_IsValid();
		}

		bool f_IsAtEndOfStream() const
		{
			return mp_pWriteToStream->f_IsAtEndOfStream();
		}

		CFilePos f_GetPosition() const
		{
			return mp_pWriteToStream->f_GetPosition();
		}

		void f_SetPosition(CFilePos _Pos)
		{
			mp_pWriteToStream->f_SetPosition(_Pos);
			auto NewPos = mp_pWriteToStream->f_GetPosition();
			if (!mp_pCompareToStream->f_IsValidReadPosition(NewPos))
				DMibPDebugBreak;
			mp_pCompareToStream->f_SetPosition(NewPos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			mp_pWriteToStream->f_SetPositionFromEnd(_Pos);
			auto NewPos = mp_pWriteToStream->f_GetPosition();
			if (!mp_pCompareToStream->f_IsValidReadPosition(NewPos))
				DMibPDebugBreak;
			mp_pCompareToStream->f_SetPosition(NewPos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			mp_pWriteToStream->f_AddPosition(_Pos);
			auto NewPos = mp_pWriteToStream->f_GetPosition();
			if (!mp_pCompareToStream->f_IsValidReadPosition(NewPos))
				DMibPDebugBreak;
			mp_pCompareToStream->f_SetPosition(NewPos);
		}

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return mp_pWriteToStream->f_IsValidReadPosition(_Pos);
		}

		void f_Flush(bool _bLocalCacheOnly)
		{
			mp_pWriteToStream->f_Flush(_bLocalCacheOnly);
		}

		void f_SetCacheSize(umint _CacheSize)
		{
			mp_pWriteToStream->f_SetCacheSize(_CacheSize);
		}

		CFilePos f_GetLength() const
		{
			return mp_pWriteToStream->f_GetLength();
		}

		umint f_ContainerLengthLimit() const
		{
			return mp_pWriteToStream->f_ContainerLengthLimit();
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			return mp_pWriteToStream->f_SetLength(_Length);
		}
	};
}

#ifndef DMibPNoShortCuts
using namespace NMib::NStream;
#endif
