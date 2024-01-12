// Copyright © 2024 Favro Holding AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NStream
{
	constexpr CScopeBinaryStreamVersion::CScopeBinaryStreamVersion(CBinaryStream &_Stream, uint32 _Version)
		: mp_pStream(&_Stream)
	{
		mp_OldVersion = _Stream.m_Version;
		_Stream.m_Version = _Version;
	}

	template <typename tf_CVersion>
	constexpr CScopeBinaryStreamVersion::CScopeBinaryStreamVersion(CBinaryStream &_Stream, tf_CVersion _Version)
		requires(cIsValidStreamVersion<tf_CVersion>)
		: CScopeBinaryStreamVersion(_Stream, uint32(_Version))
	{
	}

	constexpr void CScopeBinaryStreamVersion::f_SetVersion(CBinaryStream &_Stream, uint32 _Version)
	{
		f_Clear();
		mp_pStream = &_Stream;
		mp_OldVersion = _Stream.m_Version;
		_Stream.m_Version = _Version;
	}

	template <typename tf_CVersion>
	constexpr void CScopeBinaryStreamVersion::f_SetVersion(CBinaryStream &_Stream, tf_CVersion _Version)
		requires(cIsValidStreamVersion<tf_CVersion>)
	{
		f_SetVersion(_Stream, uint32(_Version));
	}

	constexpr CScopeBinaryStreamVersion::~CScopeBinaryStreamVersion()
	{
		f_Clear();
	}

	constexpr void CScopeBinaryStreamVersion::f_Clear()
	{
		if (mp_pStream)
		{
			mp_pStream->m_Version = mp_OldVersion;
			mp_pStream = nullptr;
		}
	}
}
