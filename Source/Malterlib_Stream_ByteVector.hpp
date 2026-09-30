// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NStream
{
	template <typename tf_CType>
	NContainer::CByteVector fg_ToByteVector(tf_CType const &_CreateStreamFrom)
	{
		CBinaryStreamMemory<> Stream;
		Stream << _CreateStreamFrom;

		return Stream.f_MoveVector();
	}

	template <typename tf_CType>
	NContainer::CByteVector fg_ToByteVectorBE(tf_CType const &_CreateStreamFrom)
	{
		CBinaryStreamMemory<NStream::CBinaryStreamBigEndian> Stream;
		Stream << _CreateStreamFrom;
		return Stream.f_MoveVector();
	}

	template <typename tf_CType>
	void fg_FromByteVector(NContainer::CByteVector const &_Data, tf_CType &_Destination)
	{
		CBinaryStreamMemoryPtr<> Stream;
		Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
		Stream >> _Destination;
	}

	template <typename tf_CType>
	tf_CType fg_FromByteVector(NContainer::CByteVector const &_Data)
	{
		tf_CType ret;
		fg_FromByteVector(_Data, ret);
		return ret;
	}

	template <typename tf_CType>
	void fg_FromByteVectorBE(NContainer::CByteVector const &_Data, tf_CType &_Destination)
	{
		CBinaryStreamMemoryPtr<NStream::CBinaryStreamBigEndian> Stream;
		Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
		Stream >> _Destination;
	}

	template <typename tf_CType>
	tf_CType fg_FromByteVectorBE(NContainer::CByteVector const &_Data)
	{
		tf_CType ret;
		fg_FromByteVectorBE(_Data, ret);
		return ret;
	}

	template <typename tf_CType>
	NContainer::CSecureByteVector fg_ToSecureByteVector(tf_CType const &_CreateStreamFrom)
	{
		CBinaryStreamMemory<NStream::CBinaryStreamDefault, NContainer::CSecureByteVector> Stream;
		Stream << _CreateStreamFrom;

		return Stream.f_MoveVector();
	}

	template <typename tf_CType>
	NContainer::CSecureByteVector fg_ToSecureByteVectorBE(tf_CType const &_CreateStreamFrom)
	{
		CBinaryStreamMemory<NStream::CBinaryStreamBigEndian, NContainer::CSecureByteVector> Stream;
		Stream << _CreateStreamFrom;
		return Stream.f_MoveVector();
	}

	template <typename tf_CType>
	void fg_FromSecureByteVector(NContainer::CSecureByteVector const &_Data, tf_CType &_Destination)
	{
		CBinaryStreamMemoryPtr<> Stream;
		Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
		Stream >> _Destination;
	}

	template <typename tf_CType>
	tf_CType fg_FromSecureByteVector(NContainer::CSecureByteVector const &_Data)
	{
		tf_CType ret;
		fg_FromSecureByteVector(_Data, ret);
		return ret;
	}

	template <typename tf_CType>
	void fg_FromSecureByteVectorBE(NContainer::CSecureByteVector const &_Data, tf_CType &_Destination)
	{
		CBinaryStreamMemoryPtr<NStream::CBinaryStreamBigEndian> Stream;
		Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
		Stream >> _Destination;
	}

	template <typename tf_CType>
	tf_CType fg_FromSecureByteVectorBE(NContainer::CSecureByteVector const &_Data)
	{
		tf_CType ret;
		fg_FromSecureByteVectorBE(_Data, ret);
		return ret;
	}
}
