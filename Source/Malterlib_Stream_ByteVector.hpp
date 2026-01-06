// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NStream
{
	template <typename tf_CType>
	NContainer::CByteVector fg_ToByteVector(const tf_CType &_CreateStreamFrom)
	{
		CBinaryStreamMemory<> Stream;
		Stream << _CreateStreamFrom;

		return Stream.f_MoveVector();
	}

	template <typename tf_CType>
	NContainer::CByteVector fg_ToByteVectorBE(const tf_CType &_CreateStreamFrom)
	{
		CBinaryStreamMemory<NStream::CBinaryStreamBigEndian> Stream;
		Stream << _CreateStreamFrom;
		return Stream.f_MoveVector();
	}

	template <typename tf_CType>
	void fg_FromByteVector(const NContainer::CByteVector &_Data, tf_CType &_Destination)
	{
		CBinaryStreamMemoryPtr<> Stream;
		Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
		Stream >> _Destination;
	}

	template <typename tf_CType>
	tf_CType fg_FromByteVector(const NContainer::CByteVector &_Data)
	{
		tf_CType ret;
		fg_FromByteVector(_Data,ret);
		return ret;
	}

	template <typename tf_CType>
	void fg_FromByteVectorBE(const NContainer::CByteVector &_Data, tf_CType &_Destination)
	{
		CBinaryStreamMemoryPtr<NStream::CBinaryStreamBigEndian> Stream;
		Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
		Stream >> _Destination;
	}

	template <typename tf_CType>
	tf_CType fg_FromByteVectorBE(const NContainer::CByteVector &_Data)
	{
		tf_CType ret;
		fg_FromByteVectorBE(_Data,ret);
		return ret;
	}


	template <typename tf_CType>
	NContainer::CSecureByteVector fg_ToSecureByteVector(const tf_CType &_CreateStreamFrom)
	{
		CBinaryStreamMemory<NStream::CBinaryStreamDefault, NContainer::CSecureByteVector> Stream;
		Stream << _CreateStreamFrom;

		return Stream.f_MoveVector();
	}

	template <typename tf_CType>
	NContainer::CSecureByteVector fg_ToSecureByteVectorBE(const tf_CType &_CreateStreamFrom)
	{
		CBinaryStreamMemory<NStream::CBinaryStreamBigEndian, NContainer::CSecureByteVector> Stream;
		Stream << _CreateStreamFrom;
		return Stream.f_MoveVector();
	}

	template <typename tf_CType>
	void fg_FromSecureByteVector(const NContainer::CSecureByteVector &_Data, tf_CType &_Destination)
	{
		CBinaryStreamMemoryPtr<> Stream;
		Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
		Stream >> _Destination;
	}

	template <typename tf_CType>
	tf_CType fg_FromSecureByteVector(const NContainer::CSecureByteVector &_Data)
	{
		tf_CType ret;
		fg_FromSecureByteVector(_Data,ret);
		return ret;
	}

	template <typename tf_CType>
	void fg_FromSecureByteVectorBE(const NContainer::CSecureByteVector &_Data, tf_CType &_Destination)
	{
		CBinaryStreamMemoryPtr<NStream::CBinaryStreamBigEndian> Stream;
		Stream.f_OpenRead(_Data.f_GetArray(), _Data.f_GetLen());
		Stream >> _Destination;
	}

	template <typename tf_CType>
	tf_CType fg_FromSecureByteVectorBE(const NContainer::CSecureByteVector &_Data)
	{
		tf_CType ret;
		fg_FromSecureByteVectorBE(_Data,ret);
		return ret;
	}
}
