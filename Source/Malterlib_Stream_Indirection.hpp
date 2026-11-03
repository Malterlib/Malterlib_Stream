// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NStream
{
	template <typename t_CStream, typename t_CType, typename t_CAllocator>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NStorage::TCIndirection<t_CType, t_CAllocator>>::fs_Feed
		(
			t_CStream &_Stream
			, NStorage::TCIndirection<t_CType, t_CAllocator> const &_Data
		)
	{
		_Stream << _Data.f_Get();
	}

	template <typename t_CStream, typename t_CType, typename t_CAllocator>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NStorage::TCIndirection<t_CType, t_CAllocator>>::fs_Feed(t_CStream &_Stream, NStorage::TCIndirection<t_CType, t_CAllocator> &&_Data)
	{
		_Stream << fg_Move(_Data.f_Get());
	}

	template <typename t_CStream, typename t_CType, typename t_CAllocator>
	constexpr void TCBinaryStreamTypeReference<t_CStream, NStorage::TCIndirection<t_CType, t_CAllocator>>::fs_Consume
		(
			t_CStream &_Stream
			, NStorage::TCIndirection<t_CType, t_CAllocator> &_Data
		)
	{
		_Stream >> _Data.f_Get();
	}
}
