// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NStream
	{
		template <typename tf_CType>
		NContainer::TCVector<uint8> fg_ToByteVector(const tf_CType &_CreateStreamFrom)
		{
			CBinaryStreamMemory<> Stream;
			Stream << _CreateStreamFrom;

			return Stream.f_MoveVector();
		}

		template <typename tf_CType>
		NContainer::TCVector<uint8> fg_ToByteVectorBE(const tf_CType &_CreateStreamFrom)
		{
			CBinaryStreamMemory<NStream::CBinaryStreamBigEndian> Stream;
			Stream << _CreateStreamFrom;
			return Stream.f_MoveVector();
		}

		template <typename tf_CType>
		void fg_FromByteVector(const NContainer::TCVector<uint8> &_Data, tf_CType &_Destination)
		{
			CBinaryStreamMemoryPtr<> Stream;
			Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
			Stream >> _Destination;
		}

		template <typename tf_CType>
		tf_CType fg_FromByteVector(const NContainer::TCVector<uint8> &_Data)
		{
			tf_CType ret;
			fg_FromByteVector(_Data,ret);
			return ret;
		}

		template <typename tf_CType>
		void fg_FromByteVectorBE(const NContainer::TCVector<uint8> &_Data, tf_CType &_Destination)
		{
			CBinaryStreamMemoryPtr<NStream::CBinaryStreamBigEndian> Stream;
			Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
			Stream >> _Destination;
		}

		template <typename tf_CType>
		tf_CType fg_FromByteVectorBE(const NContainer::TCVector<uint8> &_Data)
		{
			tf_CType ret;
			fg_FromByteVectorBE(_Data,ret);
			return ret;
		}
	}
}
