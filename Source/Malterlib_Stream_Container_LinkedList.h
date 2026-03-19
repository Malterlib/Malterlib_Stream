// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Container/LinkedList>

namespace NMib::NStream
{
	template <typename t_CStream, typename t_CData, typename t_CAllocator>
	class TCBinaryStreamTypeReference<t_CStream, NContainer::TCLinkedList<t_CData, t_CAllocator> >
	{
	public:
		static constexpr void fs_Feed(t_CStream &_Stream, NContainer::TCLinkedList<t_CData, t_CAllocator> const &_Data)
		{
			umint nItems = _Data.f_GetLen();

			fg_FeedLenToStream(_Stream, nItems);

			for (auto iItem = _Data.f_GetIterator(); iItem; ++iItem)
				_Stream << *iItem;
		}

		static constexpr void fs_Feed(t_CStream &_Stream, NContainer::TCLinkedList<t_CData, t_CAllocator> &&_Data)
		{
			umint nItems = _Data.f_GetLen();

			fg_FeedLenToStream(_Stream, nItems);

			for (auto iItem = _Data.f_GetIterator(); iItem; ++iItem)
				_Stream << fg_Move(*iItem);
		}

		static constexpr void fs_Consume(t_CStream &_Stream, NContainer::TCLinkedList<t_CData, t_CAllocator> &_Data)
		{
			uint64 nItems;
			fg_ConsumeLenFromStream(_Stream, nItems);
			fg_CheckLengthLimit(_Stream, nItems);

			for (umint i = 0; i < nItems; ++i)
				_Stream >> _Data.f_Insert();
		}
	};
}
