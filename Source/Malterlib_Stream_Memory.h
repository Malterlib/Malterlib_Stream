// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Core/Core>

namespace NMib::NStream
{
	template <typename t_CStreamType = NStream::CBinaryStreamDefault, typename t_CVector = NContainer::CByteVector>
	struct TCBinaryStreamMemory : public t_CStreamType
	{
		using CStorage = t_CVector;

		DMibStreamImplementOperators(TCBinaryStreamMemory);

		TCBinaryStreamMemory(CStorage const &_Buffer);
		TCBinaryStreamMemory(CStorage &&_Buffer);
		TCBinaryStreamMemory(TCBinaryStreamMemory &&_ToMove);
		TCBinaryStreamMemory();

		TCBinaryStreamMemory &operator = (TCBinaryStreamMemory &&_ToMove);
		void *f_GetBuffer();
		void const *f_GetBufferConst() const;
		void f_GrowBuffer(umint _nBytes);
		CStorage &f_GetVector();
		CStorage &f_GetVectorOptimized();
		CStorage f_GetVector() const;
		CStorage f_GetVectorOptimized() const;
		CStorage f_MoveVector();
		CStorage f_MoveVectorOptimized();
		void f_Open(CStorage const &_Buffer);
		void f_Open(CStorage &&_Buffer);
		void f_ResetStream();
		void f_Clear();
		void f_RemoveData(umint _Pos, umint _Len);
		void f_FeedBytes(void const *_pMem, umint _nBytes);
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
		inline_never void fp_GrowBufferGrow(umint _NeededBytes);
		inline_small void fp_GrowBuffer(umint _NeededBytes);
		void fp_SetPositionInternal(CFilePos _Pos);
		DMibStreamImplementProtected(TCBinaryStreamMemory);

		umint mp_Position;
		umint mp_Length;
		umint mp_BufferSize;
		uint8 *mp_pBuffer;

		CStorage mp_Buffer;

	private:
		TCBinaryStreamMemory(TCBinaryStreamMemory const &_Other) = delete;
		TCBinaryStreamMemory &operator = (TCBinaryStreamMemory const &_Other) = delete;
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault, typename t_CVector = NContainer::CByteVector>
	struct TCBinaryStreamMemoryRef : public t_CStreamType
	{
		using CStorage = t_CVector;

		DMibStreamImplementOperators(TCBinaryStreamMemoryRef);

		TCBinaryStreamMemoryRef(CStorage &_Buffer);
		TCBinaryStreamMemoryRef(CStorage &_Buffer, umint _Length);
		~TCBinaryStreamMemoryRef();

		void *f_GetBuffer();
		void const *f_GetBufferConst() const;
		CStorage &f_GetVector();
		CStorage &f_GetVectorOptimized();
		CStorage f_GetVector() const;
		CStorage f_GetVectorOptimized() const;
		CStorage &&f_MoveVector();
		CStorage &&f_MoveVectorOptimized();
		void f_ResetStream();
		void f_Clear();
		void f_RemoveData(umint _Pos, umint _Len);
		void f_FeedBytes(void const *_pMem, umint _nBytes);
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
		void fp_GrowBuffer(umint _NeededBytes);
		void fp_SetPositionInternal(CFilePos _Pos);
		DMibStreamImplementProtected(TCBinaryStreamMemoryRef);

		umint mp_Position;
		umint mp_Length;

		CStorage &mp_Buffer;

	private:
		TCBinaryStreamMemoryRef(TCBinaryStreamMemoryRef const &_Other) = delete;
		TCBinaryStreamMemoryRef &operator = (TCBinaryStreamMemoryRef const &_Other) = delete;
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault, typename t_CVector = NContainer::CByteVector>
	struct TCBinaryStreamMemoryConstRef : public t_CStreamType
	{
		using CStorage = t_CVector;

		DMibStreamImplementOperators(TCBinaryStreamMemoryConstRef);

		TCBinaryStreamMemoryConstRef(CStorage const &_Buffer);

		void *f_GetBuffer();
		void const *f_GetBufferConst() const;
		CStorage f_GetVector() const;
		CStorage f_GetVectorOptimized() const;
		void f_ResetStream();
		void f_Clear();
		void f_RemoveData(umint _Pos, umint _Len);
		void f_FeedBytes(void const *_pMem, umint _nBytes);
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
		DMibStreamImplementProtected(TCBinaryStreamMemoryConstRef);

		umint mp_Position;
		umint mp_Length;

		CStorage const &mp_Buffer;

	private:
		TCBinaryStreamMemoryConstRef(TCBinaryStreamMemoryConstRef const &_Other) = delete;
		TCBinaryStreamMemoryConstRef &operator = (TCBinaryStreamMemoryConstRef const &_Other) = delete;
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	struct TCBinaryStreamMemoryPtr : public t_CStreamType
	{
		DMibStreamImplementOperators(TCBinaryStreamMemoryPtr);

		TCBinaryStreamMemoryPtr();
		void f_OpenRead(void const *_pData, umint _Length);
		template <typename tf_CVector>
		void f_OpenRead(tf_CVector const &_Buffer);

		void f_OpenReadWrite(void *_pData, umint _MaxLength, umint _Length = 0);
		template <typename tf_CVector>
		void f_OpenReadWrite(tf_CVector &_Vector, umint _Length = 0);

		void f_FeedBytes(void const *_pMem, umint _nBytes);
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
		void *f_GetBuffer();
		void const *f_GetBufferConst() const;

	protected:
		void fp_SetPositionInternal(CFilePos _Pos);
		DMibStreamImplementProtected(TCBinaryStreamMemoryPtr);

		umint mp_Position;
		umint mp_Length;
		umint mp_MaxLength;
		aint mp_Mode;
		uint8 *mp_pMemoryData;

	private:
		TCBinaryStreamMemoryPtr(TCBinaryStreamMemoryPtr const &_Other) = delete;
		TCBinaryStreamMemoryPtr &operator = (TCBinaryStreamMemoryPtr const &_Other) = delete;
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	struct TCBinaryStreamConstMemoryPtr : public t_CStreamType
	{
		DMibStreamImplementOperators(TCBinaryStreamConstMemoryPtr);

		TCBinaryStreamConstMemoryPtr();
		TCBinaryStreamConstMemoryPtr(TCBinaryStreamConstMemoryPtr &&_ToMove);
		TCBinaryStreamConstMemoryPtr &operator = (TCBinaryStreamConstMemoryPtr &&_ToMove);
		void f_OpenRead(void const *_pData, umint _Length);
		template <typename tf_CVector>
		void f_OpenRead(tf_CVector const &_Buffer);

		void f_FeedBytes(void const *_pMem, umint _nBytes);
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
		void *f_GetBuffer();
		void const *f_GetBufferConst() const;

	protected:
		void fp_SetPositionInternal(CFilePos _Pos);
		DMibStreamImplementProtected(TCBinaryStreamConstMemoryPtr);

		umint mp_Position;
		umint mp_Length;
		uint8 const *mp_pMemoryData;

	private:
		TCBinaryStreamConstMemoryPtr(TCBinaryStreamConstMemoryPtr const &_Other) = delete;
		TCBinaryStreamConstMemoryPtr &operator = (TCBinaryStreamConstMemoryPtr const &_Other) = delete;
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	struct TCBinaryStreamSubStream : public t_CStreamType
	{
		DMibStreamImplementOperators(TCBinaryStreamSubStream);

		TCBinaryStreamSubStream();
		void f_Close();
		void f_Open(CBinaryStream *_pSubStream, CFilePos _SubPos, CFilePos _MaxLength = -1);
		void f_FeedBytes(void const *_pMem, umint _nBytes);
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
		DMibStreamImplementProtected(TCBinaryStreamSubStream);

		CBinaryStream *mp_pSubStream;
		CFilePos mp_SubPos;
		CFilePos mp_MaxLength;
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	struct TCBinaryStreamCompare : public t_CStreamType
	{
		DMibStreamImplementOperators(TCBinaryStreamCompare);

		TCBinaryStreamCompare();
		void f_Close();
		void f_Open(CBinaryStream *_pWriteStream, CBinaryStream *_pCompareToStream);
		void f_FeedBytes(void const *_pMem, umint _nBytes);
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
		DMibStreamImplementProtected(TCBinaryStreamCompare);

		CBinaryStream *mp_pCompareToStream;
		CBinaryStream *mp_pWriteToStream;
		NContainer::CByteVector mp_TempVector;
	};
}

#ifndef DMibPNoShortCuts
using namespace NMib::NStream;
#endif

#include "Malterlib_Stream_Memory.hpp"
