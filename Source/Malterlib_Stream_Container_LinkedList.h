// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Container/LinkedList>

namespace NMib::NStream
{
	template <typename t_CStream, typename t_CData, typename t_CAllocator>
	struct TCBinaryStreamTypeReference<t_CStream, NContainer::TCLinkedList<t_CData, t_CAllocator>>
	{
		static constexpr void fs_Feed(t_CStream &_Stream, NContainer::TCLinkedList<t_CData, t_CAllocator> const &_Data);
		static constexpr void fs_Feed(t_CStream &_Stream, NContainer::TCLinkedList<t_CData, t_CAllocator> &&_Data);
		static constexpr void fs_Consume(t_CStream &_Stream, NContainer::TCLinkedList<t_CData, t_CAllocator> &_Data);
	};
}

#include "Malterlib_Stream_Container_LinkedList.hpp"
