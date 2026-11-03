// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NStream
{
	template <typename tf_CType>
	NContainer::CByteVector fg_ToByteVector(tf_CType const &_CreateStreamFrom)
	{
		TCBinaryStreamMemory<> Stream;
		Stream << _CreateStreamFrom;

		return Stream.f_MoveVector();
	}

	template <typename tf_CType>
	NContainer::CByteVector fg_ToByteVectorBE(tf_CType const &_CreateStreamFrom)
	{
		TCBinaryStreamMemory<NStream::CBinaryStreamBigEndian> Stream;
		Stream << _CreateStreamFrom;
		return Stream.f_MoveVector();
	}

	template <typename tf_CType>
	void fg_FromByteVector(NContainer::CByteVector const &_Data, tf_CType &_Destination)
	{
		TCBinaryStreamMemoryPtr<> Stream;
		Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
		Stream >> _Destination;
	}

	template <typename tf_CType>
	tf_CType fg_FromByteVector(NContainer::CByteVector const &_Data)
	{
		tf_CType Return;
		fg_FromByteVector(_Data, Return);
		return Return;
	}

	template <typename tf_CType>
	void fg_FromByteVectorBE(NContainer::CByteVector const &_Data, tf_CType &_Destination)
	{
		TCBinaryStreamMemoryPtr<NStream::CBinaryStreamBigEndian> Stream;
		Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
		Stream >> _Destination;
	}

	template <typename tf_CType>
	tf_CType fg_FromByteVectorBE(NContainer::CByteVector const &_Data)
	{
		tf_CType Return;
		fg_FromByteVectorBE(_Data, Return);
		return Return;
	}

	template <typename tf_CType>
	NContainer::CSecureByteVector fg_ToSecureByteVector(tf_CType const &_CreateStreamFrom)
	{
		TCBinaryStreamMemory<NStream::CBinaryStreamDefault, NContainer::CSecureByteVector> Stream;
		Stream << _CreateStreamFrom;

		return Stream.f_MoveVector();
	}

	template <typename tf_CType>
	NContainer::CSecureByteVector fg_ToSecureByteVectorBE(tf_CType const &_CreateStreamFrom)
	{
		TCBinaryStreamMemory<NStream::CBinaryStreamBigEndian, NContainer::CSecureByteVector> Stream;
		Stream << _CreateStreamFrom;
		return Stream.f_MoveVector();
	}

	template <typename tf_CType>
	void fg_FromSecureByteVector(NContainer::CSecureByteVector const &_Data, tf_CType &_Destination)
	{
		TCBinaryStreamMemoryPtr<> Stream;
		Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
		Stream >> _Destination;
	}

	template <typename tf_CType>
	tf_CType fg_FromSecureByteVector(NContainer::CSecureByteVector const &_Data)
	{
		tf_CType Return;
		fg_FromSecureByteVector(_Data, Return);
		return Return;
	}

	template <typename tf_CType>
	void fg_FromSecureByteVectorBE(NContainer::CSecureByteVector const &_Data, tf_CType &_Destination)
	{
		TCBinaryStreamMemoryPtr<NStream::CBinaryStreamBigEndian> Stream;
		Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
		Stream >> _Destination;
	}

	template <typename tf_CType>
	tf_CType fg_FromSecureByteVectorBE(NContainer::CSecureByteVector const &_Data)
	{
		tf_CType Return;
		fg_FromSecureByteVectorBE(_Data, Return);
		return Return;
	}
}
