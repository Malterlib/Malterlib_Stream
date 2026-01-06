// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

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
}
