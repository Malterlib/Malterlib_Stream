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
}
