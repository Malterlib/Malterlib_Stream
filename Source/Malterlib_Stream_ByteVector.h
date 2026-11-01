// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include "Malterlib_Stream.h"
#include <Mib/Container/Vector>

namespace NMib::NStream
{
	template <typename tf_CType>
	NContainer::CByteVector fg_ToByteVector(tf_CType const &_CreateStreamFrom);
	template <typename tf_CType>
	tf_CType fg_FromByteVector(NContainer::CByteVector const &_Data);
	template <typename tf_CType>
	void fg_FromByteVector(NContainer::CByteVector const &_Data, tf_CType &_Destination);

	template <typename tf_CType>
	NContainer::CByteVector fg_ToByteVectorBE(tf_CType const &_CreateStreamFrom);
	template <typename tf_CType>
	tf_CType fg_FromByteVectorBE(NContainer::CByteVector const &_Data);
	template <typename tf_CType>
	void fg_FromByteVectorBE(NContainer::CByteVector const &_Data, tf_CType &_Destination);

	template <typename tf_CType>
	NContainer::CSecureByteVector fg_ToSecureByteVector(tf_CType const &_CreateStreamFrom);
	template <typename tf_CType>
	tf_CType fg_FromSecureByteVector(NContainer::CSecureByteVector const &_Data);
	template <typename tf_CType>
	void fg_FromSecureByteVector(NContainer::CSecureByteVector const &_Data, tf_CType &_Destination);

	template <typename tf_CType>
	NContainer::CSecureByteVector fg_ToSecureByteVectorBE(tf_CType const &_CreateStreamFrom);
	template <typename tf_CType>
	tf_CType fg_FromSecureByteVectorBE(NContainer::CSecureByteVector const &_Data);
	template <typename tf_CType>
	void fg_FromSecureByteVectorBE(NContainer::CSecureByteVector const &_Data, tf_CType &_Destination);
}

#include "Malterlib_Stream_ByteVector.hpp"

#ifndef DMibPNoShortCuts
using namespace NMib::NStream;
#endif
