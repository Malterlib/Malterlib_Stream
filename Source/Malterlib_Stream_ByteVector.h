// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include "Malterlib_Stream.h"
#include <Mib/Container/Vector>

namespace NMib
{
	namespace NStream
	{
		template <typename tf_CType>
		NContainer::TCVector<uint8> fg_ToByteVector(const tf_CType &_CreateStreamFrom);
		template <typename tf_CType>
		tf_CType fg_FromByteVector(const NContainer::TCVector<uint8> &_Data);
		template <typename tf_CType>
		void fg_FromByteVector(const NContainer::TCVector<uint8> &_Data, tf_CType &_Destination);

		template <typename tf_CType>
		NContainer::TCVector<uint8> fg_ToByteVectorBE(const tf_CType &_CreateStreamFrom);
		template <typename tf_CType>
		tf_CType fg_FromByteVectorBE(const NContainer::TCVector<uint8> &_Data);
		template <typename tf_CType>
		void fg_FromByteVectorBE(const NContainer::TCVector<uint8> &_Data, tf_CType &_Destination);
	}
}

#include "Malterlib_Stream_ByteVector.hpp"
