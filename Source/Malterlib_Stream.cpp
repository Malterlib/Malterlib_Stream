// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "Malterlib_Stream.h"

namespace NMib::NStream
{
	DMibImpErrorClassImplement(CExceptionStream);
	DMibImpErrorClassImplement(CExceptionStreamVersionMismatch);

	void fg_ReportLengthLimitError(uint64 _Len, uint64 _LengthLimit)
	{
		using namespace NStr;
		DMibErrorStream("Container length would cause stream to overrun. {} > {}"_f << _Len << _LengthLimit);
	}

	inline_never void CBinaryStream::fp_ThrowEndOfStreamException()
	{
		DMibError("End of stream Overrun");
	}

	CScopeBinaryStreamContainerLengthLimit::CScopeBinaryStreamContainerLengthLimit(CBinaryStream &_Stream, uint32 _ContainerLengthLimit)
		: mp_pStream(&_Stream)
	{
		mp_OldContainerLengthLimit = _Stream.mp_ContainerLengthLimit;
		_Stream.mp_ContainerLengthLimit = _ContainerLengthLimit;
	}

	void CScopeBinaryStreamContainerLengthLimit::f_SetContainerLengthLimit(CBinaryStream &_Stream, uint32 _ContainerLengthLimit)
	{
		f_Clear();
		mp_pStream = &_Stream;
		mp_OldContainerLengthLimit = _Stream.mp_ContainerLengthLimit;
		_Stream.mp_ContainerLengthLimit = _ContainerLengthLimit;
	}

	CScopeBinaryStreamContainerLengthLimit::~CScopeBinaryStreamContainerLengthLimit()
	{
		f_Clear();
	}

	void CScopeBinaryStreamContainerLengthLimit::f_Clear()
	{
		if (mp_pStream)
		{
			mp_pStream->mp_ContainerLengthLimit = mp_OldContainerLengthLimit;
			mp_pStream = nullptr;
		}
	}

	CScopeBinaryStreamContext::CScopeBinaryStreamContext(CBinaryStream &_Stream, void *_pContext)
		: mp_pStream(&_Stream)
	{
		mp_pOldContext = _Stream.mp_pContext;
		_Stream.mp_pContext = _pContext;
	}

	void CScopeBinaryStreamContext::f_SetContext(CBinaryStream &_Stream, void *_pContext)
	{
		f_Clear();
		mp_pStream = &_Stream;
		mp_pOldContext = _Stream.mp_pContext;
		_Stream.mp_pContext = _pContext;
	}

	void CScopeBinaryStreamContext::f_Clear()
	{
		if (mp_pStream)
		{
			mp_pStream->mp_pContext = mp_pOldContext;
			mp_pStream = nullptr;
		}
	}

	CScopeBinaryStreamContext::~CScopeBinaryStreamContext()
	{
		f_Clear();
	}

	void CBinaryStreamDefault::fp_FeedFromStreamImplementation(CBinaryStream &_Stream, CFilePos _nBytes)
	{
		NFile::CFileIoTempBuffer Buffer;

		CFilePos ToTransfer = _nBytes;
		while (ToTransfer)
		{
			auto BufferResult = Buffer.f_UseBuffer(ToTransfer);
			_Stream.f_ConsumeBytes(BufferResult.m_pBuffer, BufferResult.m_nBytes);
			fp_FeedBytes(BufferResult.m_pBuffer, BufferResult.m_nBytes);
			ToTransfer -= BufferResult.m_nBytes;
		}
	}

	CBinaryStreamDefaultRef::CBinaryStreamDefaultRef()
	{
	}

	CBinaryStreamBigEndian::CBinaryStreamBigEndian()
	{
	}

	CBinaryStreamLittleEndian::CBinaryStreamLittleEndian()
	{
	}

	CBinaryStreamNativeEndian::CBinaryStreamNativeEndian()
	{
	}
}
