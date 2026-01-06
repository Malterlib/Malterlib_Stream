// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include "Malterlib_Stream.h"
#include <Mib/Container/Vector>

namespace NMib::NStream
{
	template <typename tf_CType>
	NContainer::CByteVector fg_ToByteVector(const tf_CType &_CreateStreamFrom);
	template <typename tf_CType>
	tf_CType fg_FromByteVector(const NContainer::CByteVector &_Data);
	template <typename tf_CType>
	void fg_FromByteVector(const NContainer::CByteVector &_Data, tf_CType &_Destination);

	template <typename tf_CType>
	NContainer::CByteVector fg_ToByteVectorBE(const tf_CType &_CreateStreamFrom);
	template <typename tf_CType>
	tf_CType fg_FromByteVectorBE(const NContainer::CByteVector &_Data);
	template <typename tf_CType>
	void fg_FromByteVectorBE(const NContainer::CByteVector &_Data, tf_CType &_Destination);

	template <typename tf_CType>
	NContainer::CSecureByteVector fg_ToSecureByteVector(const tf_CType &_CreateStreamFrom);
	template <typename tf_CType>
	tf_CType fg_FromSecureByteVector(const NContainer::CSecureByteVector &_Data);
	template <typename tf_CType>
	void fg_FromSecureByteVector(const NContainer::CSecureByteVector &_Data, tf_CType &_Destination);

	template <typename tf_CType>
	NContainer::CSecureByteVector fg_ToSecureByteVectorBE(const tf_CType &_CreateStreamFrom);
	template <typename tf_CType>
	tf_CType fg_FromSecureByteVectorBE(const NContainer::CSecureByteVector &_Data);
	template <typename tf_CType>
	void fg_FromSecureByteVectorBE(const NContainer::CSecureByteVector &_Data, tf_CType &_Destination);
}

#include "Malterlib_Stream_ByteVector.hpp"

#ifndef DMibPNoShortCuts
	using namespace NMib::NStream;
#endif
