// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Core/Core>

namespace NMib::NStream
{
	template <typename t_CStreamType = NStream::CBinaryStreamDefault, typename t_CVector = NContainer::CByteVector>
	class CBinaryStreamMemory : public t_CStreamType
	{
	private:
		CBinaryStreamMemory(CBinaryStreamMemory const &) = delete;
		CBinaryStreamMemory &operator = (CBinaryStreamMemory const &) = delete;

	public:
		using CStorage = t_CVector;

	protected:
		umint m_Position;
		umint m_Length;
		umint m_BufferSize;
		uint8 *m_pBuffer;

		CStorage m_Buffer;

		inline_never void fp_GrowBufferGrow(umint _NeededBytes)
		{
			if (m_BufferSize == 0 && _NeededBytes < 2048)
				_NeededBytes = 2048;
			m_Buffer.f_Grow(_NeededBytes);
			m_pBuffer = m_Buffer.f_GetArray();
			m_BufferSize = m_Buffer.f_GetLen();
		}

		inline_small void fp_GrowBuffer(umint _NeededBytes)
		{
			umint CurrentLen = m_BufferSize;
			umint NeededSize = _NeededBytes + m_Position;
			if (CurrentLen < NeededSize)
				fp_GrowBufferGrow(NeededSize);
		}

		void fp_SetPositionInternal(CFilePos _Pos)
		{
			if (_Pos < 0 || fg_SafeLargerThan(_Pos, umint(TCLimitsInt<umint>::mc_Max)))
				DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

			m_Position = _Pos;
		}
		DMibStreamImplementProtected(CBinaryStreamMemory);
	public:
		DMibStreamImplementOperators(CBinaryStreamMemory);

		void *f_GetBuffer()
		{
			return m_pBuffer;
		}

		const void *f_GetBufferConst() const
		{
			return m_pBuffer;
		}

		void f_GrowBuffer(umint _nBytes)
		{
			fp_GrowBuffer(_nBytes);
		}

		CStorage &f_GetVector()
		{
			m_Buffer.f_SetLen(m_Length, false);
			m_pBuffer = m_Buffer.f_GetArray();
			m_BufferSize = m_Buffer.f_GetLen();
			return (CStorage &)m_Buffer;
		}

		CStorage &f_GetVectorOptimized()
		{
			m_Buffer.f_SetLen(m_Length, true);
			m_pBuffer = m_Buffer.f_GetArray();
			m_BufferSize = m_Buffer.f_GetLen();
			return (CStorage &)m_Buffer;
		}

		CStorage f_GetVector() const
		{
			CStorage Ret;
			Ret.f_Insert(m_pBuffer, m_Length);
			return Ret;
		}

		CStorage f_GetVectorOptimized() const
		{
			CStorage Ret;
			Ret.f_Insert(m_pBuffer, m_Length);
			return Ret;
		}

		CStorage f_MoveVector()
		{
			m_Buffer.f_SetLen(m_Length, false);
			m_Length = 0;
			m_BufferSize = 0;
			m_Position = 0;
			return fg_Move(m_Buffer);
		}

		CStorage f_MoveVectorOptimized()
		{
			m_Buffer.f_SetLen(m_Length, true);
			m_Length = 0;
			m_BufferSize = 0;
			m_Position = 0;
			return fg_Move(m_Buffer);
		}

		CBinaryStreamMemory(CStorage const& _Buffer)
			: m_Buffer(_Buffer)
		{
			m_Position = 0;
			m_Length = _Buffer.f_GetLen();
			m_BufferSize = m_Length;
			m_pBuffer = m_Buffer.f_GetArray();
		}

		CBinaryStreamMemory(CStorage && _Buffer)
			: m_Buffer(fg_Move(_Buffer))
		{
			m_Position = 0;
			m_Length = m_Buffer.f_GetLen();
			m_BufferSize = m_Length;
			m_pBuffer = m_Buffer.f_GetArray();
		}

		CBinaryStreamMemory(CBinaryStreamMemory && _ToMove)
			: m_Buffer( fg_Move( _ToMove.m_Buffer ) )
			, m_Position( _ToMove.m_Position )
			, m_Length( _ToMove.m_Length )
			, m_BufferSize( _ToMove.m_BufferSize )
			, m_pBuffer( _ToMove.m_pBuffer )
		{
			_ToMove.m_Position = 0;
			_ToMove.m_Length = 0;
			_ToMove.m_BufferSize = 0;
			_ToMove.m_pBuffer = nullptr;
		}

		CBinaryStreamMemory& operator=(CBinaryStreamMemory && _ToMove)
		{
			m_Buffer = fg_Move( _ToMove.m_Buffer );
			m_Position = _ToMove.m_Position;
			m_Length = _ToMove.m_Length;
			m_BufferSize = _ToMove.m_BufferSize;
			m_pBuffer = _ToMove.m_pBuffer;

			_ToMove.m_Position = 0;
			_ToMove.m_Length = 0;
			_ToMove.m_BufferSize = 0;
			_ToMove.m_pBuffer = nullptr;

			return *this;
		}

		CBinaryStreamMemory()
		{
			m_Position = 0;
			m_Length = 0;
			m_BufferSize = 0;
			m_pBuffer = nullptr;
		}

		void f_Open(CStorage const& _Buffer)
		{
			m_Buffer = _Buffer;
			m_Position = 0;
			m_Length = _Buffer.f_GetLen();
			m_BufferSize = m_Length;
			m_pBuffer = m_Buffer.f_GetArray();
		}

		void f_Open(CStorage &&_Buffer)
		{
			m_Buffer = fg_Move(_Buffer);
			m_Position = 0;
			m_Length = m_Buffer.f_GetLen();
			m_BufferSize = m_Length;
			m_pBuffer = m_Buffer.f_GetArray();
		}

		void f_ResetStream()
		{
			m_Position = 0;
			m_Length = 0;
		}

		void f_Clear()
		{
			m_Position = 0;
			m_Length = 0;
			m_BufferSize = 0;
			m_Buffer.f_Clear();
		}

		void f_RemoveData(umint _Pos, umint _Len)
		{
			m_Buffer.f_Remove(_Pos, _Len);
			m_pBuffer = m_Buffer.f_GetArray();
			m_BufferSize = m_Buffer.f_GetLen();
			m_Length -= _Len;

			if (m_Position > _Pos && m_Position <= _Pos + _Len)
				m_Position = _Pos;
		}

		void f_FeedBytes(const void *_pMem, umint _nBytes)
		{
			umint CurrentLen = m_BufferSize;
			umint Position = m_Position;
			umint NeededSize = _nBytes + Position;

			if (CurrentLen < NeededSize)
				fp_GrowBufferGrow(NeededSize);

			if (_nBytes != 0)
				NMemory::fg_MemCopy(m_pBuffer + Position, _pMem, _nBytes);

			Position += _nBytes;

			if (Position > m_Length)
				m_Length = Position;

			m_Position = Position;
		}

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			if (m_Length < (m_Position + _nBytes)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();

			if (_nBytes != 0)
				NMemory::fg_MemCopy(_pMem, m_pBuffer + m_Position, _nBytes);

			m_Position += _nBytes;
		}

		bool f_IsValid() const
		{
			return true;
		}

		bool f_IsAtEndOfStream() const
		{
			return m_Position == m_Length;
		}

		CFilePos f_GetPosition() const
		{
			return m_Position;
		}

		void f_SetPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(_Pos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Length + _Pos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Position + _Pos);
		}

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return _Pos >= 0 && _Pos < NStream::CFilePos(m_Length);
		}

		void f_Flush(bool _bLocalCacheOnly)
		{
		}

		void f_SetCacheSize(umint _CacheSize)
		{
		}

		CFilePos f_GetLength() const
		{
			return m_Length;
		}

		umint f_ContainerLengthLimit() const
		{
			return f_GetLength() - f_GetPosition();
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			if (_Length > NStream::CFilePos(m_Length))
			{
				fp_GrowBuffer(_Length - m_Length);
				NMemory::fg_SecureMemClear(m_pBuffer + m_Length, _Length - m_Length);
			}
			m_Length = _Length;
		}
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault, typename t_CVector = NContainer::CByteVector>
	class CBinaryStreamMemoryRef : public t_CStreamType
	{
	private:
		CBinaryStreamMemoryRef(CBinaryStreamMemoryRef const &) = delete;
		CBinaryStreamMemoryRef &operator = (CBinaryStreamMemoryRef const &) = delete;

	public:
		using CStorage = t_CVector;

	protected:
		umint m_Position;
		umint m_Length;

		CStorage &m_Buffer;

		void fp_GrowBuffer(umint _NeededBytes)
		{
			umint CurrentLen = m_Buffer.f_GetLen();
			umint NeededSize = _NeededBytes + m_Position;
			if (CurrentLen < NeededSize)
			{
				m_Buffer.f_Grow(NeededSize);
			}
		}

		void fp_SetPositionInternal(CFilePos _Pos)
		{
			if ((_Pos < 0) || fg_SafeLargerThan(_Pos, TCLimitsInt<umint>::mc_Max))
				DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

//				if (_Pos > (CFilePos)m_Length)
//					DMibError("Position is past end of stream");

			m_Position = _Pos;
		}
		DMibStreamImplementProtected(CBinaryStreamMemoryRef);
	public:
		DMibStreamImplementOperators(CBinaryStreamMemoryRef);

		void *f_GetBuffer()
		{
			return m_Buffer.f_GetArray();
		}

		const void *f_GetBufferConst() const
		{
			return m_Buffer.f_GetArray();
		}

		CStorage &f_GetVector()
		{
			m_Buffer.f_SetLen(m_Length, false);
			return (CStorage &)m_Buffer;
		}

		CStorage &f_GetVectorOptimized()
		{
			m_Buffer.f_SetLen(m_Length, true);
			return (CStorage &)m_Buffer;
		}

		CStorage f_GetVector() const
		{
			CStorage Ret;
			Ret.f_Insert(m_Buffer.f_GetArray(), m_Length);
			return Ret;
		}

		CStorage f_GetVectorOptimized() const
		{
			CStorage Ret;
			Ret.f_Insert(m_Buffer.f_GetArray(), m_Length);
			return Ret;
		}

		CStorage &&f_MoveVector()
		{
			m_Buffer.f_SetLen(m_Length, false);
			m_Length = 0;
			m_Position = 0;
			return (CStorage &&)m_Buffer;
		}

		CStorage &&f_MoveVectorOptimized()
		{
			m_Buffer.f_SetLen(m_Length, true);
			m_Length = 0;
			m_Position = 0;
			return (CStorage &&)m_Buffer;
		}


		CBinaryStreamMemoryRef(CStorage &_Buffer)
			: m_Buffer(_Buffer)
		{
			m_Position = 0;
			m_Length = _Buffer.f_GetLen();
		}

		CBinaryStreamMemoryRef(CStorage &_Buffer, umint _Length)
			: m_Buffer(_Buffer)
		{
			m_Position = 0;
			m_Length = _Length;
		}

		~CBinaryStreamMemoryRef()
		{
			try
			{
				m_Buffer.f_SetLen(m_Length, false);
			}
			catch (...)
			{
			}
		}

		void f_ResetStream()
		{
			m_Position = 0;
			m_Length = 0;
		}

		void f_Clear()
		{
			m_Position = 0;
			m_Length = 0;
			m_Buffer.f_Clear();
		}

		void f_RemoveData(umint _Pos, umint _Len)
		{
			m_Buffer.f_Remove(_Pos, _Len);
			m_Length -= _Len;
			if (m_Position > _Pos && m_Position <= _Pos + _Len)
				m_Position = _Pos;
		}

		void f_FeedBytes(const void *_pMem, umint _nBytes)
		{
			fp_GrowBuffer(_nBytes);

			if (_nBytes != 0)
				NMemory::fg_MemCopy(m_Buffer.f_GetArray() + m_Position, _pMem, _nBytes);

			m_Position += _nBytes;
			if (m_Position > m_Length)
				m_Length = m_Position;

		}

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			if (m_Length < (m_Position + _nBytes)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();

			if (_nBytes != 0)
				NMemory::fg_MemCopy(_pMem, m_Buffer.f_GetArray() + m_Position, _nBytes);

			m_Position += _nBytes;
		}

		bool f_IsValid() const
		{
			return true;
		}

		bool f_IsAtEndOfStream() const
		{
			return m_Position == m_Length;
		}

		CFilePos f_GetPosition() const
		{
			return m_Position;
		}

		void f_SetPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(_Pos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Length + _Pos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Position + _Pos);
		}

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return _Pos >= 0 && _Pos < NStream::CFilePos(m_Length);
		}

		void f_Flush(bool _bLocalCacheOnly)
		{
		}

		void f_SetCacheSize(umint _CacheSize)
		{
		}

		CFilePos f_GetLength() const
		{
			return m_Length;
		}

		umint f_ContainerLengthLimit() const
		{
			return f_GetLength() - f_GetPosition();
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			if (_Length > NStream::CFilePos(m_Length))
			{
				fp_GrowBuffer(_Length - m_Length);
				NMemory::fg_SecureMemClear(m_Buffer.f_GetArray() + m_Length, _Length - m_Length);
			}
			m_Length = _Length;
		}

	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault, typename t_CVector = NContainer::CByteVector>
	class CBinaryStreamMemoryConstRef : public t_CStreamType
	{
	private:
		CBinaryStreamMemoryConstRef(CBinaryStreamMemoryConstRef const &) = delete;
		CBinaryStreamMemoryConstRef &operator = (CBinaryStreamMemoryConstRef const &) = delete;

	public:
		using CStorage = t_CVector;

	protected:
		umint m_Position;
		umint m_Length;

		CStorage const &m_Buffer;

		void fp_SetPositionInternal(CFilePos _Pos)
		{
			if ((_Pos < 0) || fg_SafeLargerThan(_Pos, TCLimitsInt<umint>::mc_Max))
				DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

			if (_Pos > (CFilePos)m_Length)
				DMibError("Position is past end of stream");

			m_Position = _Pos;
		}
		DMibStreamImplementProtected(CBinaryStreamMemoryConstRef);
	public:
		DMibStreamImplementOperators(CBinaryStreamMemoryConstRef);

		void *f_GetBuffer()
		{
			return m_Buffer.f_GetArray();
		}

		const void *f_GetBufferConst() const
		{
			return m_Buffer.f_GetArray();
		}

		CStorage f_GetVector() const
		{
			CStorage Ret;
			Ret.f_Insert(m_Buffer.f_GetArray(), m_Length);
			return Ret;
		}

		CStorage f_GetVectorOptimized() const
		{
			CStorage Ret;
			Ret.f_Insert(m_Buffer.f_GetArray(), m_Length);
			return Ret;
		}

		CBinaryStreamMemoryConstRef(CStorage const &_Buffer)
			: m_Buffer(_Buffer)
		{
			m_Position = 0;
			m_Length = _Buffer.f_GetLen();
		}

		void f_ResetStream()
		{
			m_Position = 0;
			m_Length = m_Buffer.f_GetLen();
		}

		void f_Clear()
		{
			DMibError("Const stream cannot be cleared");
		}

		void f_RemoveData(umint _Pos, umint _Len)
		{
			m_Buffer.f_Remove(_Pos, _Len);
			m_Length -= _Len;
			if (m_Position > _Pos && m_Position <= _Pos + _Len)
				m_Position = _Pos;
		}

		void f_FeedBytes(const void *_pMem, umint _nBytes)
		{
			DMibError("Const stream cannot be written to");
		}

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			if (m_Length < (m_Position + _nBytes)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();;

			if (_nBytes != 0)
				NMemory::fg_MemCopy(_pMem, m_Buffer.f_GetArray() + m_Position, _nBytes);

			m_Position += _nBytes;
		}

		bool f_IsValid() const
		{
			return true;
		}

		bool f_IsAtEndOfStream() const
		{
			return m_Position == m_Length;
		}

		CFilePos f_GetPosition() const
		{
			return m_Position;
		}

		void f_SetPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(_Pos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Length + _Pos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Position + _Pos);
		}

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return _Pos >= 0 && _Pos < NStream::CFilePos(m_Length);
		}

		void f_Flush(bool _bLocalCacheOnly)
		{
		}

		void f_SetCacheSize(umint _CacheSize)
		{
		}

		CFilePos f_GetLength() const
		{
			return m_Length;
		}

		umint f_ContainerLengthLimit() const
		{
			return f_GetLength() - f_GetPosition();
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			DMibError("Const stream cannot change length");
		}

	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	class CBinaryStreamMemoryPtr : public t_CStreamType
	{
	private:
		CBinaryStreamMemoryPtr(CBinaryStreamMemoryPtr const &) = delete;
		CBinaryStreamMemoryPtr &operator = (CBinaryStreamMemoryPtr const &) = delete;

	protected:
		umint m_Position;
		umint m_Length;
		umint m_MaxLength;
		aint m_Mode;
		uint8* m_pMemoryData;


		void fp_SetPositionInternal(CFilePos _Pos)
		{
			if (_Pos < 0 || fg_SafeLargerThan(_Pos, umint(TCLimitsInt<umint>::mc_Max)))
				DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

			if (_Pos > (CFilePos)m_Length)
				DMibError("Position is past end of stream");

			m_Position = _Pos;
		}
		DMibStreamImplementProtected(CBinaryStreamMemoryPtr);
	public:
		DMibStreamImplementOperators(CBinaryStreamMemoryPtr);

		CBinaryStreamMemoryPtr()
		{
			m_MaxLength = 0;
			m_Mode = 0;
			m_Position = 0;
			m_Length = 0;
		}

		void f_OpenRead(const void *_pData, umint _Length)
		{
			m_pMemoryData = (uint8 *)_pData;
			m_Length = _Length;
			m_MaxLength = 0; // We can write no bytes
		}

		template <typename tf_CVector>
		void f_OpenRead(tf_CVector const &_Buffer)
		{
			m_pMemoryData = (uint8 *)_Buffer.f_GetArray();
			m_Length = _Buffer.f_GetLen();
			m_MaxLength = 0; // We can write no bytes
		}

		void f_OpenReadWrite(void *_pData, umint _MaxLength, umint _Length = 0)
		{
			m_pMemoryData = (uint8 *)_pData;
			m_Length = _Length;
			m_MaxLength = _MaxLength;
		}

		template <typename tf_CVector>
		void f_OpenReadWrite(tf_CVector &_Vector, umint _Length = 0)
		{
			m_pMemoryData = (uint8 *)_Vector.f_GetArray();
			m_Length = _Length;
			m_MaxLength = _Vector.f_GetLen();
		}

		void f_FeedBytes(const void *_pMem, umint _nBytes)
		{
			if (m_MaxLength < (m_Position + _nBytes)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();;

			if (_nBytes != 0)
				NMemory::fg_MemCopy(m_pMemoryData + m_Position, _pMem, _nBytes);

			m_Position += _nBytes;
			if (m_Position > m_Length)
				m_Length = m_Position;
		}

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			if (m_Length < (m_Position + _nBytes)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();;

			if (_nBytes != 0)
				NMemory::fg_MemCopy(_pMem, m_pMemoryData + m_Position, _nBytes);

			m_Position += _nBytes;
		}

		bool f_IsValid() const
		{
			return true;
		}

		bool f_IsAtEndOfStream() const
		{
			return m_Position == m_Length;
		}

		CFilePos f_GetPosition() const
		{
			return m_Position;
		}

		void f_SetPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(_Pos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Length + _Pos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Position + _Pos);
		}

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return _Pos >= 0 && _Pos < NStream::CFilePos(m_Length);
		}

		void f_Flush(bool _bLocalCacheOnly)
		{
		}

		void f_SetCacheSize(umint _CacheSize)
		{
		}

		CFilePos f_GetLength() const
		{
			return m_Length;
		}

		umint f_ContainerLengthLimit() const
		{
			return f_GetLength() - f_GetPosition();
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			if (_Length > NStream::CFilePos(m_MaxLength)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();;

			if (_Length > NStream::CFilePos(m_Length))
				NMemory::fg_SecureMemClear(m_pMemoryData + m_Length, _Length - m_Length);
			m_Length = _Length;
		}
		void *f_GetBuffer()
		{
			return (void*)m_pMemoryData;
		}

		const void *f_GetBufferConst() const
		{
			return (const void*)m_pMemoryData;
		}
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	class CBinaryStreamConstMemoryPtr : public t_CStreamType
	{
	private:
		CBinaryStreamConstMemoryPtr(CBinaryStreamConstMemoryPtr const &) = delete;
		CBinaryStreamConstMemoryPtr &operator = (CBinaryStreamConstMemoryPtr const &) = delete;

	protected:
		umint m_Position;
		umint m_Length;
		uint8 const* m_pMemoryData;


		void fp_SetPositionInternal(CFilePos _Pos)
		{
			if (_Pos < 0 || fg_SafeLargerThan(_Pos, umint(TCLimitsInt<umint>::mc_Max)))
				DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

			if (_Pos > (CFilePos)m_Length)
				DMibError("Position is past end of stream");

			m_Position = _Pos;
		}
		DMibStreamImplementProtected(CBinaryStreamConstMemoryPtr);
	public:

		DMibStreamImplementOperators(CBinaryStreamConstMemoryPtr);

		CBinaryStreamConstMemoryPtr()
			: m_Position(0)
			, m_Length(0)
			, m_pMemoryData(nullptr)
		{
		}

		CBinaryStreamConstMemoryPtr(CBinaryStreamConstMemoryPtr&& _ToMove)
			: m_Position(_ToMove.m_Position)
			, m_Length(_ToMove.m_Length)
			, m_pMemoryData(_ToMove.m_pMemoryData)
		{
			_ToMove.m_Position = 0;
			_ToMove.m_Length = 0;
			_ToMove.m_pMemoryData = nullptr;
		}

		CBinaryStreamConstMemoryPtr& operator=(CBinaryStreamConstMemoryPtr&& _ToMove)
		{
			m_Position = _ToMove.m_Position;
			m_Length = _ToMove.m_Length;
			m_pMemoryData = _ToMove.m_pMemoryData;

			_ToMove.m_Position = 0;
			_ToMove.m_Length = 0;
			_ToMove.m_pMemoryData = nullptr;
		}

		void f_OpenRead(const void *_pData, umint _Length)
		{
			m_pMemoryData = (uint8 *)_pData;
			m_Length = _Length;
			m_Position = 0;
		}

		template <typename tf_CVector>
		void f_OpenRead(tf_CVector const &_Buffer)
		{
			m_pMemoryData = (uint8 *)_Buffer.f_GetArray();
			m_Length = _Buffer.f_GetLen();
			m_Position = 0;
		}

		void f_FeedBytes(const void *_pMem, umint _nBytes)
		{
			DMibError("Const stream cannot be written to");
		}

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			if (m_Length < (m_Position + _nBytes)) [[unlikely]]
				this->fp_ThrowEndOfStreamException();;

			if (_nBytes != 0)
				NMemory::fg_MemCopy(_pMem, m_pMemoryData + m_Position, _nBytes);

			m_Position += _nBytes;
		}

		bool f_IsValid() const
		{
			return m_pMemoryData ? true : false;
		}

		bool f_IsAtEndOfStream() const
		{
			return m_Position == m_Length;
		}

		CFilePos f_GetPosition() const
		{
			return m_Position;
		}

		void f_SetPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(_Pos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Length + _Pos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Position + _Pos);
		}

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return _Pos >= 0 && _Pos < NStream::CFilePos(m_Length);
		}

		void f_Flush(bool _bLocalCacheOnly)
		{
		}

		void f_SetCacheSize(umint _CacheSize)
		{
		}

		CFilePos f_GetLength() const
		{
			return m_Length;
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

		const void *f_GetBufferConst() const
		{
			return (const void*)m_pMemoryData;
		}
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	class CBinaryStreamSubStream : public t_CStreamType
	{
	protected:
		CBinaryStream *m_pSubStream;
		CFilePos m_SubPos;
		CFilePos m_MaxLength;

		DMibStreamImplementProtected(CBinaryStreamSubStream);
	public:
		DMibStreamImplementOperators(CBinaryStreamSubStream);

		CBinaryStreamSubStream()
		{
			m_pSubStream = nullptr;
			m_SubPos = 0;
			m_MaxLength = -1;
		}

		void f_Close()
		{
			m_pSubStream = nullptr;
			m_SubPos = 0;
			m_MaxLength = -1;
		}

		void f_Open(CBinaryStream *_pSubStream, CFilePos _SubPos, CFilePos _MaxLength = -1)
		{
			m_pSubStream = _pSubStream;
			m_SubPos = _SubPos;
			m_MaxLength = _MaxLength;
			m_pSubStream->f_SetPosition(m_SubPos);
		}

		void f_FeedBytes(const void *_pMem, umint _nBytes)
		{
			if (m_MaxLength >= 0)
			{
				CFilePos ResultingPos = m_pSubStream->f_GetPosition() + _nBytes;
				if (ResultingPos > m_SubPos + m_MaxLength)
				{
					DMibError("Trying to write outside of stream");
				}
			}

			m_pSubStream->f_FeedBytes(_pMem, _nBytes);
		}

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			if (m_MaxLength >= 0)
			{
				CFilePos ResultingPos = m_pSubStream->f_GetPosition() + _nBytes;
				if (ResultingPos > m_SubPos + m_MaxLength)
				{
					DMibError("Trying to read outside of stream");
				}
			}
			m_pSubStream->f_ConsumeBytes(_pMem, _nBytes);
		}

		bool f_IsValid() const
		{
			return m_pSubStream->f_IsValid();
		}

		bool f_IsAtEndOfStream() const
		{
			if (m_MaxLength >= 0)
			{
				if (m_pSubStream->f_GetPosition() >= m_SubPos + m_MaxLength)
					return true;
			}
			return m_pSubStream->f_IsAtEndOfStream();
		}

		CFilePos f_GetPosition() const
		{
			return m_pSubStream->f_GetPosition() - m_SubPos;
		}

		void f_SetPosition(CFilePos _Pos)
		{
			if (m_MaxLength >= 0)
			{
				if (_Pos - m_SubPos > m_MaxLength)
				{
					DMibError("Trying to seek outside of stream");
				}
			}
			if (_Pos < 0)
				DMibError("Trying to seek outside of stream");
			return m_pSubStream->f_SetPosition(m_SubPos + _Pos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			CFilePos ResultingPos;
			if (m_MaxLength >= 0)
			{
				if (_Pos > 0)
				{
					DMibError("Trying to seek outside of stream");
				}
				ResultingPos = m_SubPos + (m_MaxLength + _Pos);
			}
			else
			{
				ResultingPos = m_pSubStream->f_GetLength() + _Pos;
			}
			if (ResultingPos < m_SubPos)
				DMibError("Trying to seek outside of stream");

			return m_pSubStream->f_SetPosition(_Pos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			CFilePos ResultingPos = m_pSubStream->f_GetPosition() + _Pos;

			if (m_MaxLength >= 0)
			{
				if (ResultingPos > m_SubPos + m_MaxLength)
				{
					DMibError("Trying to seek outside of stream");
				}
			}
			if (ResultingPos < m_SubPos)
				DMibError("Trying to seek outside of stream");

			return m_pSubStream->f_SetPosition(ResultingPos);
		}

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			if (m_MaxLength >= 0)
			{
				if (_Pos - m_SubPos >= m_MaxLength)
				{
					return false;
				}
			}
			if (_Pos < 0)
				return false;
			return m_pSubStream->f_IsValidReadPosition(m_SubPos + _Pos);
		}

		void f_Flush(bool _bLocalCacheOnly)
		{
			m_pSubStream->f_Flush(_bLocalCacheOnly);
		}

		void f_SetCacheSize(umint _CacheSize)
		{
			m_pSubStream->f_SetCacheSize(_CacheSize);
		}

		CFilePos f_GetLength() const
		{
			if (m_MaxLength >= 0)
				return fg_Min(m_pSubStream->f_GetLength() - m_SubPos, m_MaxLength);
			else
				return m_pSubStream->f_GetLength() - m_SubPos;
		}

		umint f_ContainerLengthLimit() const
		{
			return NStream::fg_CapLengthLimit(f_GetLength() - f_GetPosition());
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			return m_pSubStream->f_SetLength(m_SubPos + _Length);
		}

	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	class CBinaryStreamCompare : public t_CStreamType
	{
	protected:
		CBinaryStream *m_pCompareToStream;
		CBinaryStream *m_pWriteToStream;
		NContainer::CByteVector m_TempVector;

		DMibStreamImplementProtected(CBinaryStreamCompare);
	public:
		DMibStreamImplementOperators(CBinaryStreamCompare);

		CBinaryStreamCompare()
		{
			m_pCompareToStream = nullptr;
			m_pWriteToStream = nullptr;
		}

		void f_Close()
		{
			m_pCompareToStream = nullptr;
			m_pWriteToStream = nullptr;
		}

		void f_Open(CBinaryStream *_pWriteStream, CBinaryStream *_pCompareToStream)
		{
			m_pCompareToStream = _pCompareToStream;
			m_pWriteToStream = _pWriteStream;
		}

		void f_FeedBytes(const void *_pMem, umint _nBytes)
		{
			m_TempVector.f_SetAtLeastLen(_nBytes);

			auto NewPos = m_pWriteToStream->f_GetPosition();
			if (!m_pCompareToStream->f_IsValidReadPosition(NewPos))
				DMibPDebugBreak;

			m_pCompareToStream->f_ConsumeBytes(m_TempVector.f_GetArray(), _nBytes);

			if (NMib::NMemory::fg_MemCmp((uint8 const *)_pMem, m_TempVector.f_GetArray(), _nBytes) != 0)
				DMibPDebugBreak;

			m_pWriteToStream->f_FeedBytes(_pMem, _nBytes);
		}

		void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			m_TempVector.f_SetAtLeastLen(_nBytes);

			auto NewPos = m_pWriteToStream->f_GetPosition();
			if (!m_pCompareToStream->f_IsValidReadPosition(NewPos))
				DMibPDebugBreak;

			m_pCompareToStream->f_ConsumeBytes(m_TempVector.f_GetArray(), _nBytes);
			m_pWriteToStream->f_ConsumeBytes(_pMem, _nBytes);

			if (NMib::NMemory::fg_MemCmp((uint8 const *)_pMem, m_TempVector.f_GetArray(), _nBytes) != 0)
				DMibPDebugBreak;
		}

		bool f_IsValid() const
		{
			return m_pWriteToStream->f_IsValid();
		}

		bool f_IsAtEndOfStream() const
		{
			return m_pWriteToStream->f_IsAtEndOfStream();
		}

		CFilePos f_GetPosition() const
		{
			return m_pWriteToStream->f_GetPosition();
		}

		void f_SetPosition(CFilePos _Pos)
		{
			m_pWriteToStream->f_SetPosition(_Pos);
			auto NewPos = m_pWriteToStream->f_GetPosition();
			if (!m_pCompareToStream->f_IsValidReadPosition(NewPos))
				DMibPDebugBreak;
			m_pCompareToStream->f_SetPosition(NewPos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			m_pWriteToStream->f_SetPositionFromEnd(_Pos);
			auto NewPos = m_pWriteToStream->f_GetPosition();
			if (!m_pCompareToStream->f_IsValidReadPosition(NewPos))
				DMibPDebugBreak;
			m_pCompareToStream->f_SetPosition(NewPos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			m_pWriteToStream->f_AddPosition(_Pos);
			auto NewPos = m_pWriteToStream->f_GetPosition();
			if (!m_pCompareToStream->f_IsValidReadPosition(NewPos))
				DMibPDebugBreak;
			m_pCompareToStream->f_SetPosition(NewPos);
		}

		bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return m_pWriteToStream->f_IsValidReadPosition(_Pos);
		}

		void f_Flush(bool _bLocalCacheOnly)
		{
			m_pWriteToStream->f_Flush(_bLocalCacheOnly);
		}

		void f_SetCacheSize(umint _CacheSize)
		{
			m_pWriteToStream->f_SetCacheSize(_CacheSize);
		}

		CFilePos f_GetLength() const
		{
			return m_pWriteToStream->f_GetLength();
		}

		umint f_ContainerLengthLimit() const
		{
			return m_pWriteToStream->f_ContainerLengthLimit();
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			return m_pWriteToStream->f_SetLength(_Length);
		}

	};

}

#ifndef DMibPNoShortCuts
	using namespace NMib::NStream;
#endif
