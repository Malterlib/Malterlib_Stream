// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include "Malterlib_Stream.h"

namespace NMib::NStream
{
	template <typename t_CStream, typename t_CType, typename t_CAllocator, typename t_CPtr>
	class TCBinaryStreamTypeReference<t_CStream, NStorage::TCIndirection<t_CType, t_CAllocator, t_CPtr>>
	{
	public:
		static void fs_Feed(t_CStream &_Stream, NStorage::TCIndirection<t_CType, t_CAllocator, t_CPtr> const &_Data)
		{
			_Stream << _Data.f_Get();
		}

		static void fs_Feed(t_CStream &_Stream, NStorage::TCIndirection<t_CType, t_CAllocator, t_CPtr> &&_Data)
		{
			_Stream << fg_Move(_Data.f_Get());
		}

		static void fs_Consume(t_CStream &_Stream, NStorage::TCIndirection<t_CType, t_CAllocator, t_CPtr> &_Data)
		{
			_Stream >> _Data.f_Get();
		}
	};
}

#ifndef DMibPNoShortCuts
	using namespace NMib::NStream;
#endif
