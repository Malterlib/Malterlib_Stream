// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>
//#include <boost/type_traits.hpp>
#include <Mib/Storage/Indirection>

#define DMibIncluded_Stream

namespace NMib::NStream
{
	DMibImpErrorClassDefine(CExceptionStream, NMib::NException::CException);
	DMibImpErrorClassDefine(CExceptionStreamVersionMismatch, CExceptionStream);

#	define DMibErrorStream(_Description) DMibImpError(NMib::NStream::CExceptionStream, _Description)

#	ifndef DMibPNoShortCuts
#		define DErrorStream(_Description) DMibErrorStream(_Description)
#	endif

#	define DMibErrorStreamVersionMismatch(_Description) DMibImpError(NMib::NStream::CExceptionStreamVersionMismatch, _Description)

#	ifndef DMibPNoShortCuts
#		define DErrorStreamVersionMismatch(_Description) DMibErrorStreamVersionMismatch(_Description)
#	endif

	typedef CMibFilePos CFilePos;

	enum EStreamDirection
	{
		EStreamDirection_Feed
		, EStreamDirection_Consume
	};

	template <typename t_CStream, typename t_CData>
	class TCBinaryStreamTypeReference;

	template <typename t_CStream, typename t_CData>
	class TCBinaryStreamTypeReferenceStream;

	template <typename t_CStream, typename t_CData>
	class TCBinaryStreamTypePtr;

#	define DMibStreamImplementOperators(_Class) \
		template <typename tf_CData>	inline_small auto f_Feed(tf_CData &&_Data){return NMib::NStream::TCBinaryStreamTypeReference<_Class, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Feed(*this, NMib::fg_Forward<tf_CData>(_Data));} \
		template <typename tf_CData> inline_small auto f_Feed(const tf_CData *_pData){return NMib::NStream::TCBinaryStreamTypePtr<_Class, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Feed(*this, _pData);} \
		template <typename tf_CData>	inline_small auto f_Consume(tf_CData &&_Data){return NMib::NStream::TCBinaryStreamTypeReference<_Class, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Consume(*this, NMib::fg_Forward<tf_CData>(_Data));} \
		template <typename tf_CData> inline_small auto f_Consume(tf_CData *_pData){return NMib::NStream::TCBinaryStreamTypePtr<_Class, tf_CData>::fs_Consume(*this, _pData);} \
		template <typename tf_CData>	inline_small auto f_Stream(tf_CData &&_Data){return NMib::NStream::TCBinaryStreamTypeReferenceStream<_Class, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Stream(*this, NMib::fg_Forward<tf_CData>(_Data));}

	template <typename t_CStream, EStreamDirection t_Direction>
	struct TCStreamDirection : public t_CStream
	{
//			DMibStreamImplementOperators(t_CStream);

		template <typename tf_CData>	inline_small auto f_Feed(tf_CData &&_Data)
		{
			return NMib::NStream::TCBinaryStreamTypeReference<t_CStream, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Feed(*this, NMib::fg_Forward<tf_CData>(_Data));
		}
		template <typename tf_CData> inline_small auto f_Feed(const tf_CData *_pData)
		{
			return NMib::NStream::TCBinaryStreamTypePtr<t_CStream, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Feed(*this, _pData);
		}
		template <typename tf_CData>	inline_small auto f_Consume(tf_CData &&_Data)
		{
			return NMib::NStream::TCBinaryStreamTypeReference<t_CStream, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Consume(*this, NMib::fg_Forward<tf_CData>(_Data));
		}
		template <typename tf_CData> inline_small auto f_Consume(tf_CData *_pData)
		{
			return NMib::NStream::TCBinaryStreamTypePtr<t_CStream, tf_CData>::fs_Consume(*this, _pData);
		}
		template <typename tf_CData>	inline_small auto f_Stream(tf_CData &&_Data)
		{
			return NMib::NStream::TCBinaryStreamTypeReferenceStream<t_CStream, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Stream(*this, NMib::fg_Forward<tf_CData>(_Data));
		}

		static constexpr EStreamDirection mc_Direction = t_Direction;
		static constexpr bool mc_bConsume = mc_Direction == EStreamDirection_Consume;
		static constexpr bool mc_bFeed = mc_Direction == EStreamDirection_Feed;
	};

#	define DMibStreamDeclare(d_Class, d_Stream, d_Direction) extern template void d_Class::f_Stream<NStream::TCStreamDirection<d_Stream, NStream::EStreamDirection_##d_Direction>>(NStream::TCStreamDirection<d_Stream, NStream::EStreamDirection_##d_Direction> &);
#	define DMibStreamImplement(d_Class, d_Stream, d_Direction) template void d_Class::f_Stream<NStream::TCStreamDirection<d_Stream, NStream::EStreamDirection_##d_Direction>>(NStream::TCStreamDirection<d_Stream, NStream::EStreamDirection_##d_Direction> &);

#	ifndef DMibPNoShortCuts
#		define DStreamDeclare DMibStreamDeclare
#		define DStreamImplement DMibStreamImplement
#	endif

	namespace NPrivate
	{
		struct CDummy
		{
		};

		template <typename t_CStream, typename t_CData, typename t_CEnableIf = void>
		struct TCHasStream
		{
			static constexpr bool mc_Value = false;
		};

		template <typename t_CStream, typename t_CData>
		struct TCHasStream
		<
			t_CStream
			, t_CData
			, TCEnableIfType
			<
				!NTraits::TCIsSame
				<
					decltype(fg_GetReference<typename NTraits::TCRemoveReferenceAndQualifiers<t_CData>::CType>().f_Stream(fg_GetReference<t_CStream>())), NPrivate::CDummy
				>::mc_Value
			>
		>
		{
			static constexpr bool mc_Value = true;
		};

		template <typename t_CStream, typename t_CData, typename t_CEnableIf = void>
		struct TCStreamHasStream
		{
			static constexpr bool mc_Value = false;
		};

		template <typename t_CStream, typename t_CData>
		struct TCStreamHasStream
		<
			t_CStream
			, t_CData
			, TCEnableIfType
			<
				!NTraits::TCIsSame
				<
					decltype(fg_GetReference<t_CStream>().f_Stream(fg_GetType<t_CData>())), NPrivate::CDummy
				>::mc_Value
			>
		>
		{
			static constexpr bool mc_Value = true;
		};


		template <typename t_CStream, typename t_CData, typename t_CEnableIf = void>
		struct TCStreamHasFeed
		{
			static constexpr bool mc_Value = false;
		};

		template <typename t_CStream, typename t_CData >
		struct TCStreamHasFeed
		<
			t_CStream
			, t_CData
			, TCEnableIfType
			<
				!NTraits::TCIsSame
				<
					decltype(fg_GetReference<t_CStream>().f_Feed(fg_GetType<t_CData>())), NPrivate::CDummy
				>::mc_Value
			>
		>
		{
			static constexpr bool mc_Value = true;
		};

		template <typename t_CStream, typename t_CData, typename t_CEnableIf = void>
		struct TCStreamHasConsume
		{
			static constexpr bool mc_Value = false;
		};

		template <typename t_CStream, typename t_CData >
		struct TCStreamHasConsume
		<
			t_CStream
			, t_CData
			, TCEnableIfType
			<
				!NTraits::TCIsSame
				<
					decltype(fg_GetReference<t_CStream>().f_Consume(fg_GetType<t_CData>())), NPrivate::CDummy
				>::mc_Value
			>
		>
		{
			static constexpr bool mc_Value = true;
		};
	}

	template <typename t_CStream, typename t_CData>
	class TCBinaryStreamTypeReferenceStream
	{
	public:
		// Stream

		template
		<
			EStreamDirection t_Direction
			, typename tf_CData
			, typename NMib::TCEnableIfType
			<
				NPrivate::TCHasStream<TCStreamDirection<t_CStream, t_Direction>, tf_CData>::mc_Value
			> * = nullptr
		>
		inline_small static auto fs_Stream(TCStreamDirection<t_CStream, t_Direction> &_Stream, tf_CData &&_Data)
		{
			return fg_Forward<tf_CData>(_Data).f_Stream(_Stream);
		}

		template
		<
			typename tf_CData
			, typename NMib::TCEnableIfType
			<
				!NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Feed>, tf_CData>::mc_Value
			> * = nullptr
		>
		inline_small static auto fs_Stream(TCStreamDirection<t_CStream, EStreamDirection_Feed> &_Stream, tf_CData &&_Data)
		{
			static_cast<t_CStream &>(_Stream) << fg_ConstOrMove<tf_CData>(fg_Forward<tf_CData>(_Data));
		}

		template
		<
			typename tf_CData
			, typename NMib::TCEnableIfType
			<
				!NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Consume>, tf_CData>::mc_Value
			> * = nullptr
		>
		inline_small static auto fs_Stream(TCStreamDirection<t_CStream, EStreamDirection_Consume> &_Stream, tf_CData &&_Data)
		{
			static_cast<t_CStream &>(_Stream) >> const_cast<typename NTraits::TCRemoveReferenceAndQualifiers<tf_CData>::CType &>(_Data);
		}
	};

	template <typename t_CStream, typename t_CData>
	class TCBinaryStreamTypeReference
	{
	public:
		// Feed

		template
		<
			typename tf_CData
			, typename NMib::TCEnableIfType
			<
				!NMib::NTraits::TCIsEnum<typename NTraits::TCRemoveReference<tf_CData>::CType>::mc_Value
				&& !NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Feed>, tf_CData>::mc_Value
			> * = nullptr
		>
		inline_small static auto fs_Feed(t_CStream &_Stream, tf_CData &&_Data)
		{
			return fg_Forward<tf_CData>(_Data).f_Feed(_Stream);
		}

		template
		<
			typename tf_CData
			, typename NMib::TCEnableIfType
			<
				!NMib::NTraits::TCIsEnum<typename NTraits::TCRemoveReference<tf_CData>::CType>::mc_Value
				&& NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Feed>, tf_CData>::mc_Value
			> * = nullptr
		>
		inline_small static auto fs_Feed(t_CStream &_Stream, tf_CData &&_Data)
		{
			return const_cast<typename NTraits::TCRemoveReferenceAndQualifiers<tf_CData>::CType &>(_Data).f_Stream(reinterpret_cast<TCStreamDirection<t_CStream, EStreamDirection_Feed> &>(_Stream));
		}

		// Consume

		template
		<
			typename tf_CData
			, typename NMib::TCEnableIfType
			<
				!NMib::NTraits::TCIsEnum<typename NTraits::TCRemoveReference<tf_CData>::CType>::mc_Value
				&& !NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Consume>, tf_CData>::mc_Value
			> * = nullptr
		>
		inline_small static auto fs_Consume(t_CStream &_Stream, tf_CData &&_Data)
		{
			return _Data.f_Consume(_Stream);
		}

		template
		<
			typename tf_CData
			, typename NMib::TCEnableIfType
			<
				!NMib::NTraits::TCIsEnum<typename NTraits::TCRemoveReference<tf_CData>::CType>::mc_Value
				&& NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Consume>, tf_CData>::mc_Value
			> * = nullptr
		>
		inline_small static auto fs_Consume(t_CStream &_Stream, tf_CData &&_Data)
		{
			return fg_Forward<tf_CData>(_Data).f_Stream(reinterpret_cast<TCStreamDirection<t_CStream, EStreamDirection_Consume> &>(_Stream));
		}

		// Enum

		template <typename tf_CData, typename NMib::TCEnableIf<NMib::NTraits::TCIsEnum<tf_CData>::mc_Value, void>::CType * = nullptr>
		inline_small static void fs_Feed(t_CStream &_Stream, tf_CData const &_Data)
		{
			_Stream << uint32(_Data);
		}

		template <typename tf_CData, typename NMib::TCEnableIf<NMib::NTraits::TCIsEnum<tf_CData>::mc_Value, void>::CType * = nullptr>
		inline_small static void fs_Consume(t_CStream &_Stream, tf_CData &_Data)
		{
			uint32 Temp;
			_Stream >> Temp;
			_Data = static_cast<tf_CData>(Temp);
		}
	};


	template <typename t_CStream, typename t_CData>
	class TCBinaryStreamTypePtr
	{
	public:
		static void fs_Feed(t_CStream &_Stream, const t_CData *_pData)
		{
			t_CStream::Implement_Error; // Type not implemented
			// _Stream.f_FeedBytes(_pData, sizeof(*_pData));
		}
		static void fs_Consume(t_CStream &_Stream, t_CData *_pData)
		{
			t_CStream::Implement_Error;
			// _Stream.f_ConsumeBytes(_pData, sizeof(*_pData));
		}
	};

	template <typename t_CType>
	class TCBinaryStreamUnsafeWrapper
	{
		TCBinaryStreamUnsafeWrapper &operator = (const TCBinaryStreamUnsafeWrapper &_Other)
		{
		}
	public:
		typedef t_CType CType;
		CType &m_Data;
		TCBinaryStreamUnsafeWrapper(t_CType &_Data)
			:m_Data(_Data)
		{
		}
	};

	template <typename t_CKeyStr, typename t_CToStream>
	class TCNamedStreamInfo
	{
		const t_CKeyStr &m_Key;
		t_CToStream &m_ToStream;
		const t_CToStream &m_Default;
	public:
		typedef t_CKeyStr CKey;
		typedef t_CToStream CData;
		TCNamedStreamInfo(t_CKeyStr const &_Key, t_CToStream &_ToStream, const t_CToStream &_Default)
			: m_Key(_Key)
			, m_ToStream(_ToStream)
			, m_Default(_Default)
		{
		}

		const t_CKeyStr &f_GetKey() const
		{
			return m_Key;
		}

		t_CToStream &f_GetValue() const
		{
			return m_ToStream;
		}

		t_CToStream const &f_GetDefault() const
		{
			return m_Default;
		}

	};

	template <typename t_CKeyStr, typename t_CToStream, typename t_CToStreamDefault>
	TCNamedStreamInfo<t_CKeyStr, t_CToStream> fg_Named(t_CKeyStr const &_Key, t_CToStream &_ToStream, t_CToStreamDefault const &_Default)
	{
		return TCNamedStreamInfo<t_CKeyStr, t_CToStream>(_Key, _ToStream, _Default);
	}

	template <typename t_CKeyStr, typename t_CToStream>
	TCNamedStreamInfo<t_CKeyStr, t_CToStream const> fg_Named(t_CKeyStr const &_Key, t_CToStream const &_ToStream)
	{
		static t_CToStream s_Default;
		return TCNamedStreamInfo<t_CKeyStr, t_CToStream const>(_Key, _ToStream, s_Default);
	}

	template <typename t_CStream, typename t_CData>
	class TCBinaryStreamTypeReferenceNamed
	{
	public:
		template <typename t_CData2>
		inline_small static typename NMib::TCDisableIf<NMib::NTraits::TCIsEnum<typename t_CData2::CData>::mc_Value, void>::CType fs_Feed(t_CStream &_Stream, t_CData2 const &_Data)
		{
			auto pChild = _Stream.f_CreateChild(_Data.f_GetKey());
			_Data.f_GetValue().f_FeedNamed(*pChild);
		}
		template <typename t_CData2>
		inline_small static typename NMib::TCDisableIf<NMib::NTraits::TCIsEnum<typename t_CData2::CData>::mc_Value, void>::CType fs_Consume(t_CStream &_Stream, t_CData2 const &_Data)
		{
			auto pChild = _Stream.f_GetChild(_Data.f_GetKey());
			if (pChild && _Data.f_GetValue().f_ConsumeNamed(*pChild))
				return;
			_Data.f_GetValue() = _Data.f_GetDefault();
		}

		template <typename t_CData2>
		inline_small static typename NMib::TCEnableIf<NMib::NTraits::TCIsEnum<typename t_CData2::CData>::mc_Value, void>::CType fs_Feed(t_CStream &_Stream, t_CData2 const &_Data)
		{
			_Stream << fg_Named(_Data.f_GetKey(), uint32(_Data.f_GetValue()));
		}
		template <typename t_CData2>
		inline_small static typename NMib::TCEnableIf<NMib::NTraits::TCIsEnum<typename t_CData2::CData>::mc_Value, void>::CType fs_Consume(t_CStream &_Stream, t_CData2 const &_Data)
		{
			uint32 Temp;
			_Stream >> fg_Named(_Data.f_GetKey(), Temp, uint32(_Data.f_GetDefault()));
			_Data.f_GetValue() = static_cast<typename t_CData2::CData>(Temp);
		}
	};

	//&& !NTraits::TCIsSame<decltype(NMib::NStream::TCBinaryStreamTypeReference<tf_CStream, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Feed(_Stream, NMib::fg_Forward<tf_CData>(_Data))), NPrivate::CDummy>::mc_Value


#	define DMibStreamImplementProtected(_Class) \
		void fp_FeedBytes(const void *_pMem, mint _nBytes){_Class::f_FeedBytes(_pMem, _nBytes);}\
		void fp_ConsumeBytes(void *_pMem, mint _nBytes){_Class::f_ConsumeBytes(_pMem, _nBytes);}\
		bint fp_IsValid() const {bint Ret = 0; Ret = _Class::f_IsValid(); return Ret;}\
		bint fp_IsAtEndOfStream() const {bint Ret = 0; Ret = _Class::f_IsAtEndOfStream(); return Ret;}\
		NMib::NStream::CFilePos fp_GetPosition() const {return _Class::f_GetPosition();}\
		void fp_SetPosition(NMib::NStream::CFilePos _Pos){_Class::f_SetPosition(_Pos);}\
		void fp_SetPositionFromEnd(NMib::NStream::CFilePos _Pos){_Class::f_SetPositionFromEnd(_Pos);}\
		void fp_AddPosition(NMib::NStream::CFilePos _Pos){_Class::f_AddPosition(_Pos);}\
		bint fp_IsValidReadPosition(NMib::NStream::CFilePos _Pos) const {bint bRet = 0; bRet = _Class::f_IsValidReadPosition(_Pos); return bRet; }\
		void fp_Flush(bint _bLocalCacheOnly) {_Class::f_Flush(_bLocalCacheOnly);}\
		void fp_SetCacheSize(mint _CacheSize) {_Class::f_SetCacheSize(_CacheSize);}\
		NMib::NStream::CFilePos fp_GetLength() const {NMib::NStream::CFilePos Ret = 0; Ret =_Class::f_GetLength(); return Ret;}\
		void fp_SetLength(NMib::NStream::CFilePos _Length) {return _Class::f_SetLength(_Length);}\
		aint fp_LengthSize() const {aint Ret = 0; Ret = _Class::f_LengthSize(); return Ret;}\
		aint fp_Endian() const {aint Ret = 0; Ret = _Class::f_Endian(); return Ret;}\
		mint fp_ContainerLengthLimit() const {mint Ret = 0; Ret = _Class::f_ContainerLengthLimit(); return Ret;}\
		void fp_FeedFromStream(NMib::NStream::CBinaryStream &_Stream, NMib::NStream::CFilePos _nBytes){_Class::f_FeedFromStream(_Stream, _nBytes);}\

#	if defined(DMibDebug) && 0
#		define DMibTempStreamDebug
#		define DMibTempStreamPre virtual
#		define DMibTempStreamPost =0
#	else
#		define DMibTempStreamPre inline_small
#		define DMibTempStreamPost
#	endif

	class CScopeBinaryStreamVersion;
	class CScopeBinaryStreamContext;
	class CScopeBinaryStreamContainerLengthLimit;

	class CBinaryStream
	{
		friend class CScopeBinaryStreamVersion;
		friend class CScopeBinaryStreamContext;
		friend class CScopeBinaryStreamContainerLengthLimit;
	public:
		CBinaryStream()
		{
		}
	private:

		void *m_pContext = nullptr;
		mint m_ContainerLengthLimit = 0;
		uint32 m_Version = 0;

		CBinaryStream(CBinaryStream const &) = delete;
		CBinaryStream &operator = (CBinaryStream const &) = delete;

	protected:
		virtual void fp_FeedBytes(const void *_pMem, mint _nBytes) = 0;
		virtual void fp_ConsumeBytes(void *_pMem, mint _nBytes) = 0;
		virtual bint fp_IsValid() const = 0;
		virtual bint fp_IsAtEndOfStream() const = 0;
		virtual CFilePos fp_GetPosition() const = 0;
		virtual void fp_SetPosition(CFilePos _Pos) = 0;
		virtual void fp_SetPositionFromEnd(CFilePos _Pos) = 0;
		virtual void fp_AddPosition(CFilePos _Pos) = 0;
		virtual bint fp_IsValidReadPosition(NStream::CFilePos _Pos) const = 0;
		virtual void fp_Flush(bint _bLocalCacheOnly) = 0;
		virtual void fp_SetCacheSize(mint _CacheSize) = 0;
		virtual CFilePos fp_GetLength() const = 0;
		virtual void fp_SetLength(CFilePos _Length) = 0;
		virtual	aint fp_LengthSize() const = 0;
		virtual aint fp_Endian() const = 0;
		virtual mint fp_ContainerLengthLimit() const = 0;
		virtual void fp_FeedFromStream(CBinaryStream &_Stream, CFilePos _nBytes) = 0;

		inline_never void fp_ThrowEndOfStreamException()
		{
			DMibError("End of stream Overrun");
		}

	public:
		virtual ~CBinaryStream(){}

#ifdef DMibTempStreamDebug
		DMibTempStreamPre void f_FeedBytes(const void *_pMem, mint _nBytes) DMibTempStreamPost;
		DMibTempStreamPre void f_ConsumeBytes(void *_pMem, mint _nBytes) DMibTempStreamPost;
		DMibTempStreamPre bint f_IsValid() const DMibTempStreamPost;
		DMibTempStreamPre bint f_IsAtEndOfStream() const DMibTempStreamPost;
		DMibTempStreamPre CFilePos f_GetPosition() const DMibTempStreamPost;
		DMibTempStreamPre void f_SetPosition(CFilePos _Pos) DMibTempStreamPost;
		DMibTempStreamPre void f_SetPositionFromEnd(CFilePos _Pos) DMibTempStreamPost;
		DMibTempStreamPre void f_AddPosition(CFilePos _Pos) DMibTempStreamPost;
		DMibTempStreamPre bint f_IsValidReadPosition(CFilePos _Pos) const DMibTempStreamPost;
		DMibTempStreamPre void f_Flush(bint _bLocalCacheOnly) DMibTempStreamPost;
		DMibTempStreamPre void f_SetCacheSize(mint _CacheSize) DMibTempStreamPost;
		DMibTempStreamPre CFilePos f_GetLength() const DMibTempStreamPost;
		DMibTempStreamPre void f_SetLength(CFilePos _Length) DMibTempStreamPost;
		DMibTempStreamPre aint f_LengthSize() const DMibTempStreamPost;
		DMibTempStreamPre aint f_Endian() const DMibTempStreamPost;
		DMibTempStreamPre mint f_ContainerLengthLimit() const DMibTempStreamPost;
		DMibTempStreamPre void f_FeedFromStream(CBinaryStream &_Stream, CFilePos _nBytes) DMibTempStreamPost;
#else
		DMibTempStreamPre void f_FeedBytes(const void *_pMem, mint _nBytes) DMibTempStreamPost
		{
			fp_FeedBytes(_pMem, _nBytes);
		}

		DMibTempStreamPre void f_ConsumeBytes(void *_pMem, mint _nBytes) DMibTempStreamPost
		{
			fp_ConsumeBytes(_pMem, _nBytes);
		}

		DMibTempStreamPre bint f_IsValid() const DMibTempStreamPost
		{
			return fp_IsValid();
		}

		DMibTempStreamPre bint f_IsAtEndOfStream() const DMibTempStreamPost
		{
			return fp_IsAtEndOfStream();
		}

		DMibTempStreamPre CFilePos f_GetPosition() const DMibTempStreamPost
		{
			return fp_GetPosition();
		}

		DMibTempStreamPre void f_SetPosition(CFilePos _Pos) DMibTempStreamPost
		{
			fp_SetPosition(_Pos);
		}

		DMibTempStreamPre void f_SetPositionFromEnd(CFilePos _Pos) DMibTempStreamPost
		{
			fp_SetPositionFromEnd(_Pos);
		}

		DMibTempStreamPre void f_AddPosition(CFilePos _Pos) DMibTempStreamPost
		{
			fp_AddPosition(_Pos);
		}
		DMibTempStreamPre bint f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return fp_IsValidReadPosition(_Pos);
		}

		DMibTempStreamPre void f_Flush(bint _bLocalCacheOnly)
		{
			return fp_Flush(_bLocalCacheOnly);
		}

		DMibTempStreamPre void f_SetCacheSize(mint _CacheSize)
		{
			return fp_SetCacheSize(_CacheSize);
		}

		DMibTempStreamPre CFilePos f_GetLength() const DMibTempStreamPost
		{
			return fp_GetLength();
		}

		DMibTempStreamPre void f_SetLength(CFilePos _Length) DMibTempStreamPost
		{
			return fp_SetLength(_Length);
		}

		DMibTempStreamPre aint f_LengthSize() const DMibTempStreamPost
		{
			return fp_LengthSize();
		}

		DMibTempStreamPre aint f_Endian() const DMibTempStreamPost
		{
			return fp_Endian();
		}

		DMibTempStreamPre mint f_ContainerLengthLimit() const DMibTempStreamPost
		{
			return fp_ContainerLengthLimit();
		}

		DMibTempStreamPre void f_FeedFromStream(CBinaryStream &_Stream, CFilePos _nBytes) DMibTempStreamPost
		{
			fp_FeedFromStream(_Stream, _nBytes);
		}

#endif
		inline_small uint32 f_GetVersion() const
		{
			return m_Version;
		}
		inline_small void *f_GetContext() const
		{
			return m_pContext;
		}

		inline_small mint f_ClaimContainerLengthLimitOverride()
		{
			mint Return = m_ContainerLengthLimit;
			m_ContainerLengthLimit = 0;
			return Return;
		}

		DMibStreamImplementOperators(CBinaryStream);
	};

	template <typename tf_CStream, typename tf_CData>
	inline_small auto operator << (tf_CStream &_Stream, tf_CData &&_Data)
	-> typename NMib::TCEnableIf
		<
			(NTraits::TCIsBaseOf<tf_CStream, CBinaryStream>::mc_Value || NTraits::TCIsSame<tf_CStream, CBinaryStream>::mc_Value)
			&& !NMib::NStorage::TCIsIndirection<typename NTraits::TCRemoveReferenceAndQualifiers<tf_CData>::CType>::mc_Value
			&& NPrivate::TCStreamHasFeed<tf_CStream, tf_CData>::mc_Value
			, tf_CStream &
		>::CType
	{
		_Stream.f_Feed(fg_Forward<tf_CData>(_Data));
		return _Stream;
	}

	template <typename tf_CStream, typename tf_CData>
	inline_small auto operator >> (tf_CStream &_Stream, tf_CData &&_Data)
	-> typename NMib::TCEnableIf
		<
			(NTraits::TCIsBaseOf<tf_CStream, CBinaryStream>::mc_Value || NTraits::TCIsSame<tf_CStream, CBinaryStream>::mc_Value)
			&& !NMib::NStorage::TCIsIndirection<typename NTraits::TCRemoveReferenceAndQualifiers<tf_CData>::CType>::mc_Value
			&& NPrivate::TCStreamHasConsume<tf_CStream, tf_CData>::mc_Value
			, tf_CStream &
		>::CType
	{
		_Stream.f_Consume(fg_Forward<tf_CData>(_Data));
		return _Stream;
	}

	template <typename tf_CStream, typename tf_CData>
	inline_small auto operator << (tf_CStream &_Stream, tf_CData const *_pData)
	-> typename NMib::TCEnableIf
		<
			(NTraits::TCIsBaseOf<tf_CStream, CBinaryStream>::mc_Value || NTraits::TCIsSame<tf_CStream, CBinaryStream>::mc_Value)
			&& NPrivate::TCStreamHasFeed<tf_CStream, tf_CData const *>::mc_Value
			, tf_CStream &
		>::CType
	{
		_Stream.f_Feed(_pData);
		return _Stream;
	}

	template <typename tf_CStream, typename tf_CData>
	inline_small auto operator >> (tf_CStream &_Stream, tf_CData *_pData)
	-> typename NMib::TCEnableIf
		<
			(NTraits::TCIsBaseOf<tf_CStream, CBinaryStream>::mc_Value || NTraits::TCIsSame<tf_CStream, CBinaryStream>::mc_Value)
			&& NPrivate::TCStreamHasConsume<tf_CStream, tf_CData *>::mc_Value
			, tf_CStream &
		>::CType
	{
		_Stream.f_Consume(_pData);
		return _Stream;
	}

	template <typename tf_CStream, typename tf_CData>
	inline_small auto operator % (tf_CStream &_Stream, tf_CData &&_Data)
	-> typename NMib::TCEnableIf
		<
			(NTraits::TCIsBaseOf<tf_CStream, CBinaryStream>::mc_Value || NTraits::TCIsSame<tf_CStream, CBinaryStream>::mc_Value)
			&& !NMib::NStorage::TCIsIndirection<typename NTraits::TCRemoveReferenceAndQualifiers<tf_CData>::CType>::mc_Value
			&& NPrivate::TCStreamHasStream<tf_CStream, tf_CData>::mc_Value
			, tf_CStream &
		>::CType
	{
		_Stream.f_Stream(fg_Forward<tf_CData>(_Data));
		return _Stream;
	}

	template <typename tf_CStream>
	void fg_PadAlignStream(tf_CStream &_Stream, mint _Alignment)
	{
		CFilePos Position = _Stream.f_GetPosition();
		CFilePos EndPosition = fg_AlignUp(Position, CFilePos(_Alignment));
		while (Position != EndPosition)
		{
			uint8 PaddingData[128] = {0};

			mint ThisTime = fg_Min(EndPosition - Position, CFilePos(128));

			_Stream.f_FeedBytes(PaddingData, ThisTime);
			Position += ThisTime;
		}
	}

	template <typename tf_CStream>
	void fg_AlignStream(tf_CStream &_Stream, mint _Alignment)
	{
		CFilePos Position = _Stream.f_GetPosition();
		CFilePos EndPosition = fg_AlignUp(Position, CFilePos(_Alignment));
		_Stream.f_SetPosition(EndPosition);
	}

	class CScopeBinaryStreamVersion
	{
	public:
		CScopeBinaryStreamVersion() = delete;
		CScopeBinaryStreamVersion(const CScopeBinaryStreamVersion &_Other) = delete;
		CScopeBinaryStreamVersion & operator = (CScopeBinaryStreamVersion const &) = delete;

		CScopeBinaryStreamVersion(CBinaryStream &_Stream, uint32 _Version)
			: mp_pStream(&_Stream)
		{
			mp_OldVersion = _Stream.m_Version;
			_Stream.m_Version = _Version;
		}
		void f_SetVersion(CBinaryStream &_Stream, uint32 _Version)
		{
			f_Clear();
			mp_pStream = &_Stream;
			mp_OldVersion = _Stream.m_Version;
			_Stream.m_Version = _Version;
		}
		~CScopeBinaryStreamVersion()
		{
			f_Clear();
		}
		void f_Clear()
		{
			if (mp_pStream)
			{
				mp_pStream->m_Version = mp_OldVersion;
				mp_pStream = nullptr;
			}
		}

	private:
		CBinaryStream *mp_pStream;
		uint32 mp_OldVersion;
	};

#	define DMibBinaryStreamVersion(_Stream, _Version) NMib::NStream::CScopeBinaryStreamVersion ScopeBinaryStreamVersion(_Stream, _Version)

#	ifndef DMibPNoShortCuts
#		define DBinaryStreamVersion(_Stream, _Version) DMibBinaryStreamVersion(_Stream, _Version)
#	endif


	class CScopeBinaryStreamContainerLengthLimit
	{
	public:
		CScopeBinaryStreamContainerLengthLimit() = delete;
		CScopeBinaryStreamContainerLengthLimit(const CScopeBinaryStreamContainerLengthLimit &_Other) = delete;
		CScopeBinaryStreamContainerLengthLimit & operator = (CScopeBinaryStreamContainerLengthLimit const &) = delete;

		CScopeBinaryStreamContainerLengthLimit(CBinaryStream &_Stream, uint32 _ContainerLengthLimit)
			: mp_pStream(&_Stream)
		{
			mp_OldContainerLengthLimit = _Stream.m_ContainerLengthLimit;
			_Stream.m_ContainerLengthLimit = _ContainerLengthLimit;
		}
		void f_SetContainerLengthLimit(CBinaryStream &_Stream, uint32 _ContainerLengthLimit)
		{
			f_Clear();
			mp_pStream = &_Stream;
			mp_OldContainerLengthLimit = _Stream.m_ContainerLengthLimit;
			_Stream.m_ContainerLengthLimit = _ContainerLengthLimit;
		}
		~CScopeBinaryStreamContainerLengthLimit()
		{
			f_Clear();
		}
		void f_Clear()
		{
			if (mp_pStream)
			{
				mp_pStream->m_ContainerLengthLimit = mp_OldContainerLengthLimit;
				mp_pStream = nullptr;
			}
		}

	private:
		CBinaryStream *mp_pStream;
		uint32 mp_OldContainerLengthLimit;
	};

#	define DMibBinaryStreamContainerLengthLimit(_Stream, _ContainerLengthLimit) NMib::NStream::CScopeBinaryStreamContainerLengthLimit ScopeBinaryStreamContainerLengthLimit(_Stream, _ContainerLengthLimit)

#	ifndef DMibPNoShortCuts
#		define DBinaryStreamContainerLengthLimit(_Stream, _ContainerLengthLimit) DMibBinaryStreamContainerLengthLimit(_Stream, _ContainerLengthLimit)
#	endif


	class CScopeBinaryStreamContext
	{
	public:
		CScopeBinaryStreamContext(const CScopeBinaryStreamContext &_Other) = delete;
		CScopeBinaryStreamContext &operator = (const CScopeBinaryStreamContext &_Other) = delete;
		CScopeBinaryStreamContext() = delete;

		CScopeBinaryStreamContext(CBinaryStream &_Stream, void *_pContext)
			: m_pStream(&_Stream)
		{
			m_pOldContext = _Stream.m_pContext;
			_Stream.m_pContext = _pContext;
		}

		void f_SetContext(CBinaryStream &_Stream, void *_pContext)
		{
			f_Clear();
			m_pStream = &_Stream;
			m_pOldContext = _Stream.m_pContext;
			_Stream.m_pContext = _pContext;
		}

		void f_Clear()
		{
			if (m_pStream)
			{
				m_pStream->m_pContext = m_pOldContext;
				m_pStream = nullptr;
			}
		}
		~CScopeBinaryStreamContext()
		{
			f_Clear();
		}
	private:
		CBinaryStream *m_pStream;
		void *m_pOldContext;
	};

#	define DMibBinaryStreamContext(_Stream, _Context) NMib::NStream::CScopeBinaryStreamContext ScopeBinaryStreamContext(_Stream, _Context)

#	ifndef DMibPNoShortCuts
#		define DBinaryStreamContext(_Stream, _Context) DMibBinaryStreamContext(_Stream, _Context)
#	endif

#undef DMibTempStreamDebug
#undef DMibTempStreamPre
#undef DMibTempStreamPost

	class CBinaryStreamDefault : public CBinaryStream
	{
	private:
		CBinaryStreamDefault(CBinaryStreamDefault const &) = delete;
		CBinaryStreamDefault &operator = (CBinaryStreamDefault const &) = delete;

	protected:
		DMibStreamImplementProtected(CBinaryStreamDefault);
	public:
		DMibStreamImplementOperators(CBinaryStreamDefault);

		CBinaryStreamDefault()
		{
		}

		inline_small aint f_LengthSize() const
		{
			return sizeof(uint32);
		}

		inline_small aint f_Endian() const
		{
			return EEndian_Little;
		}

		mint f_ContainerLengthLimit() const
		{
			return 1 * 1024 * 1024; // This is for streams that don't have a length
		}

		void f_FeedFromStream(CBinaryStream &_Stream, CFilePos _nBytes)
		{
			uint8 Temp[1024];
			CFilePos ToTransfer = _nBytes;
			while (ToTransfer)
			{
				mint ThisTime = fg_Min(ToTransfer, 1024);
				_Stream.f_ConsumeBytes(Temp, ThisTime);
				fp_FeedBytes(Temp, ThisTime);
				ToTransfer -= ThisTime;
			}
		}

	};

	class CBinaryStreamDefaultRef : public CBinaryStreamDefault, public NStorage::TCSharedPointerIntrusiveBase<>
	{
	private:
		CBinaryStreamDefaultRef(CBinaryStreamDefaultRef const &) = delete;
		CBinaryStreamDefaultRef &operator = (CBinaryStreamDefaultRef const &) = delete;

	protected:
		DMibStreamImplementProtected(CBinaryStreamDefaultRef);
	public:
		DMibStreamImplementOperators(CBinaryStreamDefaultRef);

		CBinaryStreamDefaultRef()
		{
		}

	};

	class CBinaryStreamBigEndian : public CBinaryStreamDefault
	{
	private:
		CBinaryStreamBigEndian(CBinaryStreamBigEndian const &) = delete;
		CBinaryStreamBigEndian &operator = (CBinaryStreamBigEndian const &) = delete;

	protected:
		DMibStreamImplementProtected(CBinaryStreamBigEndian);
	public:
		DMibStreamImplementOperators(CBinaryStreamBigEndian);

		CBinaryStreamBigEndian()
		{
		}

		inline_small aint f_Endian() const
		{
			return EEndian_Big;
		}
	};

	class CBinaryStreamLittleEndian : public CBinaryStreamDefault
	{
	private:
		CBinaryStreamLittleEndian(CBinaryStreamLittleEndian const &) = delete;
		CBinaryStreamLittleEndian &operator = (CBinaryStreamLittleEndian const &) = delete;

	protected:

		DMibStreamImplementProtected(CBinaryStreamLittleEndian);
	public:
		DMibStreamImplementOperators(CBinaryStreamLittleEndian);

		CBinaryStreamLittleEndian()
		{
		}

		inline_small aint f_Endian() const
		{
			return EEndian_Little;
		}
	};

	class CBinaryStreamNativeEndian : public CBinaryStreamDefault
	{
	private:
		CBinaryStreamNativeEndian(CBinaryStreamNativeEndian const &) = delete;
		CBinaryStreamNativeEndian &operator = (CBinaryStreamNativeEndian const &) = delete;

	protected:

		DMibStreamImplementProtected(CBinaryStreamNativeEndian);
	public:
		DMibStreamImplementOperators(CBinaryStreamNativeEndian);

		CBinaryStreamNativeEndian()
		{
		}

		inline_small aint f_Endian() const
		{
			return EEndian_Native;
		}
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	class TCBinaryStreamNull : public t_CStreamType
	{
	private:
		TCBinaryStreamNull(TCBinaryStreamNull const &) = delete;
		TCBinaryStreamNull &operator = (TCBinaryStreamNull const &) = delete;

	protected:
		mint m_Position;
		mint m_Length;

		void fp_SetPositionInternal(CFilePos _Pos)
		{
			if (_Pos < 0)
				DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<mint>::mc_Max");

			m_Position = _Pos;
		}
		DMibStreamImplementProtected(TCBinaryStreamNull);
	public:
		DMibStreamImplementOperators(TCBinaryStreamNull);

		TCBinaryStreamNull()
		{
			m_Position = 0;
			m_Length = 0;
		}

		void f_Reset()
		{
			m_Position = 0;
		}

		void f_Clear()
		{
			m_Position = 0;
		}

		void f_FeedBytes(const void *_pMem, mint _nBytes)
		{
			m_Position += _nBytes;
			if (m_Position > m_Length)
				m_Length = m_Position;

		}

		void f_ConsumeBytes(void *_pMem, mint _nBytes)
		{
			if (m_Length < (m_Position + _nBytes) )
				DMibError("End of stream Overrun");
			m_Position += _nBytes;
		}

		bint f_IsValid() const
		{
			return true;
		}

		bint f_IsAtEndOfStream() const
		{
			return m_Position == m_Length;
		}

		CFilePos f_GetPosition() const
		{
			return m_Position;
		}

		void f_SetPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(_Pos);
		}

		void f_SetPositionFromEnd(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Length + _Pos);
		}

		void f_AddPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Position + _Pos);
		}

		bint f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return _Pos >= 0 && _Pos < NStream::CFilePos(m_Length);
		}

		void f_Flush(bint _bLocalCacheOnly)
		{
		}

		void f_SetCacheSize(mint _CacheSize)
		{
		}

		CFilePos f_GetLength() const
		{
			return m_Length;
		}

		mint f_ContainerLengthLimit() const
		{
			return f_GetLength() - f_GetPosition();
		}

		void f_SetLength(NStream::CFilePos _Length)
		{
			m_Length = _Length;
		}
	};
	// Default types

	template <typename t_CStream>
	void fg_FeedLenToStream(t_CStream &_Stream, uint64 _Len)
	{
		aint nBytes = _Stream.f_LengthSize();
		switch(nBytes)
		{
		case 1:	_Stream << (uint8)_Len; break;
		case 2:	_Stream << (uint16)_Len; break;
		case 4:	_Stream << (uint32)_Len; break;
		case 8:	_Stream << (uint64)_Len; break;
		default: DMibSafeCheck(0, "Unsupported save size"); break;
		}
	}

	inline_always mint fg_CapLengthLimit(NStream::CFilePos const &_Len)
	{
		if constexpr (sizeof(mint) < sizeof(NStream::CFilePos))
		{
			if (_Len > NStream::CFilePos{TCLimitsInt<smint>::mc_Max})
				return TCLimitsInt<smint>::mc_Max;
		}
		return _Len;
	}

	template <typename tf_CStream, typename tf_CLen>
	void fg_CheckLengthLimit(tf_CStream &_Stream, tf_CLen const &_Len)
	{
		mint LengthLimit = _Stream.f_ClaimContainerLengthLimitOverride();
		if (!LengthLimit)
			LengthLimit = _Stream.f_ContainerLengthLimit();

		if (_Len > uint64(LengthLimit))
			DMibErrorStream("Container length would cause stream to overrun");
	}

	template <typename t_CStream>
	void fg_ConsumeLenFromStream(t_CStream &_Stream, uint64 &_Len)
	{
		aint nBytes = _Stream.f_LengthSize();
		switch(nBytes)
		{
		case 1:	{uint8 Len; _Stream >> Len; _Len = Len;} break;
		case 2:	{uint16 Len; _Stream >> Len; _Len = Len;} break;
		case 4:	{uint32 Len; _Stream >> Len; _Len = Len;} break;
		case 8:	{uint64 Len; _Stream >> Len; _Len = Len;} break;
		default: DMibSafeCheck(0, "Unsupported save size"); break;
		}
	}

	template <typename t_CStream, typename t_CData>
	void fg_FeedEndianArrayToStream(t_CStream &_Stream, const t_CData *_pData, mint _Len, aint _Endian)
	{
		if (_Endian == EEndian_Native || _Endian == gc_MachineEndian)
			_Stream.f_FeedBytes(_pData, _Len * sizeof(t_CData));
		else
		{
			for (mint i = 0; i < _Len; ++i)
			{
				t_CData Temp = fg_ByteSwap(_pData[i]);
				_Stream.f_FeedBytes(&Temp, sizeof(t_CData));
			}
		}
	}

	template <typename t_CStream, typename t_CData>
	void fg_ByteSwapArray(t_CStream &_Stream, t_CData *_pData, mint _Len, aint _Endian)
	{
		if (_Endian == EEndian_Native || _Endian == gc_MachineEndian)
			return;
		else
		{
			fg_ByteSwap(_pData, _Len);
		}
	}

	static void fg_StrDecodeLenType(uint64 &_Len, uint32 _StreamSize, NStr::EStrType &_Type);
	static void fg_StrEncodeLenType(uint64 &_Len, uint32 _StreamSize, NStr::EStrType _Type);

	template <typename t_CStream>
	class TCBinaryStreamTypePtr<t_CStream, ch8>
	{
	public:
		static void fs_Feed(t_CStream &_Stream, const ch8 *_pData)
		{
			uint64 Len = NMib::NStr::fg_StrLen(_pData);
			uint64 LenStream = Len;
			fg_StrEncodeLenType(LenStream, _Stream.f_LengthSize(), NStr::EStrType_UTF); // Presume UTF, as this is the most probable case
			fg_FeedLenToStream(_Stream, LenStream);
			_Stream.f_FeedBytes(_pData, Len);
		}
		static void fs_Consume(t_CStream &_Stream, ch8 *_pData)
		{
			uint64 Len;
			fg_ConsumeLenFromStream(_Stream, Len);
			NStr::EStrType Type = NStr::EStrType_Ansi;
			fg_StrDecodeLenType(Len, _Stream.f_LengthSize(), Type);
			fg_CheckLengthLimit(_Stream, Len);
			_Stream.f_ConsumeBytes(_pData, Len);
			_pData[Len] = 0;
		}
	};

	template <typename t_CStream>
	class TCBinaryStreamTypePtr<t_CStream, ch16>
	{
	public:
		static void fs_Feed(t_CStream &_Stream, const ch16 *_pData)
		{
			mint Len = NMib::NStr::fg_StrLen(_pData);
			uint64 LenStream = Len;
			fg_StrEncodeLenType(LenStream, _Stream.f_LengthSize(), NStr::EStrType_UTF); // Presume UTF, as this is the most probable case
			fg_FeedLenToStream(_Stream, LenStream);
			fg_FeedEndianArrayToStream(_Stream, _pData, Len, _Stream.f_Endian());
		}
		static void fs_Consume(t_CStream &_Stream, ch16 *_pData)
		{
			uint64 Len;
			fg_ConsumeLenFromStream(_Stream, Len);
			NStr::EStrType Type = NStr::EStrType_Ansi;
			fg_StrDecodeLenType(Len, _Stream.f_LengthSize(), Type);
			fg_CheckLengthLimit(_Stream, Len);
			_Stream.f_ConsumeBytes(_pData, Len);
			fg_ByteSwapArray(_Stream, _pData, Len, _Stream.f_Endian());
			_pData[Len] = 0;
		}
	};

	template <typename t_CStream>
	class TCBinaryStreamTypePtr<t_CStream, ch32>
	{
	public:
		static void fs_Feed(t_CStream &_Stream, const ch32 *_pData)
		{
			mint Len = NMib::NStr::fg_StrLen(_pData);
			uint64 LenStream = Len;
			fg_StrEncodeLenType(LenStream, _Stream.f_LengthSize(), NStr::EStrType_Unicode);
			fg_FeedLenToStream(_Stream, LenStream);
			fg_FeedEndianArrayToStream(_Stream, _pData, Len, _Stream.f_Endian());
		}
		static void fs_Consume(t_CStream &_Stream, ch32 *_pData)
		{
			uint64 Len;
			fg_ConsumeLenFromStream(_Stream, Len);
			NStr::EStrType Type = NStr::EStrType_Ansi;
			fg_StrDecodeLenType(Len, _Stream.f_LengthSize(), Type);
			fg_CheckLengthLimit(_Stream, Len);
			_Stream.f_ConsumeBytes(_pData, Len);
			fg_ByteSwapArray(_Stream, _pData, Len, _Stream.f_Endian());
			_pData[Len] = 0;
		}
	};

	template <typename t_CStream, typename t_CType>
	class TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<const t_CType> >
	{
	public:
		static void fs_Feed(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<const t_CType> const &_Data)
		{
			_Stream << _Data.m_Data;
		}
	};

	template <typename t_CStream, typename t_CType>
	class TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> const >
	{
	public:
		static void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> const &_Data)
		{
			_Stream >> _Data.m_Data;
		}
	};

	template <typename t_CStream, typename t_CType>
	class TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> >
	{
	public:
		static void fs_Feed(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> const &_Data)
		{
			_Stream << _Data.m_Data;
		}
		static void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> &_Data)
		{
			_Stream >> _Data.m_Data;
		}
	};

	template <typename t_CType>
	inline_small TCBinaryStreamUnsafeWrapper<t_CType> fg_GetUnsafeStreamWrapper(t_CType &_Type)
	{
		return TCBinaryStreamUnsafeWrapper<t_CType>(_Type);
	}

	template <typename t_CType>
	inline_small TCBinaryStreamUnsafeWrapper<const t_CType> fg_GetUnsafeStreamWrapper(const t_CType &_Type)
	{
		return TCBinaryStreamUnsafeWrapper<const t_CType>(_Type);
	}

	template <typename t_CStream, typename t_CKeyStr, typename t_CData>
	class TCBinaryStreamTypeReference<t_CStream, TCNamedStreamInfo<t_CKeyStr, t_CData> >
	{
	public:
		static void fs_Feed(t_CStream &_Stream, TCNamedStreamInfo<t_CKeyStr, t_CData> const &_Data)
		{
			_Stream << _Data.f_GetValue();
		}
		static void fs_Consume(t_CStream &_Stream, TCNamedStreamInfo<t_CKeyStr, t_CData> const &_Data)
		{
			_Stream >> _Data.f_GetValue();
		}
	};
	template <typename t_CStream, typename t_CKeyStr, typename t_CData>
	class TCBinaryStreamTypeReference<t_CStream, TCNamedStreamInfo<t_CKeyStr, t_CData const> >
	{
	public:
		static void fs_Feed(t_CStream &_Stream, TCNamedStreamInfo<t_CKeyStr, t_CData const> const &_Data)
		{
			_Stream << _Data.f_GetValue();
		}
	};


#	define DMibStreamImplementSimpleTypeDefault(_Type) \
	template <typename t_CNamedStream, typename t_CKeyStrInfo> \
	class TCBinaryStreamTypeReferenceNamed<t_CNamedStream, TCNamedStreamInfo<t_CKeyStrInfo, _Type> > \
	{ \
	public: \
		static void fs_Feed(t_CNamedStream &_Stream, TCNamedStreamInfo<t_CKeyStrInfo, _Type const> const &_Data) \
		{ \
			_Stream.f_SetValue(_Data.f_GetKey(), t_CNamedStream::CData::fs_ToStr(_Data.f_GetValue()));\
		}\
		static void fs_Consume(t_CNamedStream &_Stream, TCNamedStreamInfo<t_CKeyStrInfo, _Type> const &_Data)\
		{\
			_Data.f_GetValue() = _Stream.f_GetValue(_Data.f_GetKey(), typename t_CNamedStream::CData()).f_ToValue(_Data.f_GetDefault());\
		}\
	};\

#	define DMibStreamImplementSimpleEndianSwappedType(_Type) DMibStreamImplementSimpleTypeDefault(_Type) \
	template <typename t_CStream> \
	class TCBinaryStreamTypeReference<t_CStream, _Type> \
	{ \
	public: \
		static void fs_Feed(t_CStream &_Stream, _Type const &_Data) \
		{ \
			aint Endian = _Stream.f_Endian();\
			if (Endian == gc_MachineEndian || Endian == EEndian_Native)\
				_Stream.f_FeedBytes(&_Data, sizeof(_Data));\
			else\
			{\
				auto Temp = fg_ByteSwap(reinterpret_cast<typename NTraits::TCIntFromSize<sizeof(_Data)>::CType const &>(_Data));\
				_Stream.f_FeedBytes(&Temp, sizeof(_Data));\
			} \
		}\
		static void fs_Consume(t_CStream &_Stream, _Type &_Data)\
		{\
			aint Endian = _Stream.f_Endian();\
			if (Endian == gc_MachineEndian || Endian == EEndian_Native)\
				_Stream.f_ConsumeBytes(&_Data, sizeof(_Data));\
			else\
			{\
				typename NTraits::TCIntFromSize<sizeof(_Data)>::CType Data;\
				_Stream.f_ConsumeBytes(&Data, sizeof(Data));\
				_Data = fg_ByteSwap(reinterpret_cast<_Type &>(Data));\
			}\
		}\
	};

#	define DMibStreamImplementSimpleEndianSwappedTypeUnsafe(_Type) DMibStreamImplementSimpleTypeDefault(_Type) \
	template <typename t_CStream> \
	class TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<const _Type> > \
	{ \
	public: \
		static void fs_Feed(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<const _Type> const &_Data) \
		{ \
			aint Endian = _Stream.f_Endian();\
			if (Endian == gc_MachineEndian || Endian == EEndian_Native)\
				_Stream.f_FeedBytes(&_Data.m_Data, sizeof(_Data.m_Data));\
			else\
			{\
				_Type Temp = fg_ByteSwap(_Data.m_Data);\
				_Stream.f_FeedBytes(&Temp, sizeof(_Data.m_Data));\
			} \
		}\
	};\
	template <typename t_CStream> \
	class TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<_Type> const > \
	{ \
	public: \
		static void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<_Type> const &_Data)\
		{\
			aint Endian = _Stream.f_Endian();\
			if (Endian == gc_MachineEndian || Endian == EEndian_Native)\
				_Stream.f_ConsumeBytes(&_Data.m_Data, sizeof(_Data.m_Data));\
			else\
			{\
				_Stream.f_ConsumeBytes(&_Data.m_Data, sizeof(_Data.m_Data));\
				_Data.m_Data = fg_ByteSwap(_Data.m_Data);\
			}\
		}\
	};\
	template <typename t_CStream> \
	class TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<_Type> > \
	{ \
	public: \
		static void fs_Feed(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<_Type> const &_Data) \
		{ \
			aint Endian = _Stream.f_Endian();\
			if (Endian == gc_MachineEndian || Endian == EEndian_Native)\
				_Stream.f_FeedBytes(&_Data.m_Data, sizeof(_Data.m_Data));\
			else\
			{\
				_Type Temp = fg_ByteSwap(_Data.m_Data);\
				_Stream.f_FeedBytes(&Temp, sizeof(_Data.m_Data));\
			} \
		}\
		static void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<_Type> &_Data)\
		{\
			aint Endian = _Stream.f_Endian();\
			if (Endian == gc_MachineEndian || Endian == EEndian_Native)\
				_Stream.f_ConsumeBytes(&_Data.m_Data, sizeof(_Data.m_Data));\
			else\
			{\
				_Stream.f_ConsumeBytes(&_Data.m_Data, sizeof(_Data.m_Data));\
				_Data.m_Data = fg_ByteSwap(_Data.m_Data);\
			}\
		}\
		static void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<_Type> &&_Data)\
		{\
			aint Endian = _Stream.f_Endian();\
			if (Endian == gc_MachineEndian || Endian == EEndian_Native)\
				_Stream.f_ConsumeBytes(&_Data.m_Data, sizeof(_Data.m_Data));\
			else\
			{\
				_Stream.f_ConsumeBytes(&_Data.m_Data, sizeof(_Data.m_Data));\
				_Data.m_Data = fg_ByteSwap(_Data.m_Data);\
			}\
		}\
	};


#	define DMibStreamImplementSimpleType(_Type) DMibStreamImplementSimpleTypeDefault(_Type) \
	template <typename t_CStream> \
	class TCBinaryStreamTypeReference<t_CStream, _Type> \
	{ \
	public: \
		static void fs_Feed(t_CStream &_Stream, _Type const &_Data) \
		{ \
			_Stream.f_FeedBytes(&_Data, sizeof(_Data));\
		}\
		static void fs_Consume(t_CStream &_Stream, _Type &_Data)\
		{\
			_Stream.f_ConsumeBytes(&_Data, sizeof(_Data));\
		}\
	};

	template <typename t_CStream>
	class TCBinaryStreamTypeReference<t_CStream, bool>
	{
	public:
		static void fs_Feed(t_CStream &_Stream, bool const &_Data)
		{
			uint8 Byte = _Data ? 1 : 0;
			_Stream.f_FeedBytes(&Byte, sizeof(Byte));
		}
		static void fs_Consume(t_CStream &_Stream, bool &_Data)
		{
			uint8 Byte;
			_Stream.f_ConsumeBytes(&Byte, sizeof(Byte));
			_Data = Byte != 0;
		}
	};

	DMibStreamImplementSimpleType(int8);
	DMibStreamImplementSimpleEndianSwappedType(int16);
	DMibStreamImplementSimpleEndianSwappedType(int32);
	DMibStreamImplementSimpleEndianSwappedType(int64);
	DMibStreamImplementSimpleType(uint8);
	DMibStreamImplementSimpleEndianSwappedType(uint16);
	DMibStreamImplementSimpleEndianSwappedType(uint32);
	DMibStreamImplementSimpleEndianSwappedType(uint64);


#ifdef DMibPUniqueType_mint
	DMibStreamImplementSimpleEndianSwappedTypeUnsafe(mint);
#endif

#ifdef DMibPUniqueType_smint
	DMibStreamImplementSimpleEndianSwappedTypeUnsafe(smint);
#endif

#ifdef DMibPUniqueType_int
	DMibStreamImplementSimpleEndianSwappedTypeUnsafe(int);
#endif
#ifdef DMibPUniqueType_uint
	DMibStreamImplementSimpleEndianSwappedTypeUnsafe(unsigned int);
#endif

#ifdef DMibPUniqueType_aint
	DMibStreamImplementSimpleEndianSwappedTypeUnsafe(aint);
#endif

#ifdef DMibPUniqueType_uaint
	DMibStreamImplementSimpleEndianSwappedTypeUnsafe(uaint);
#endif

#ifdef DMibPUniqueType_ch8
	DMibStreamImplementSimpleType(ch8);
#endif

#ifdef DMibPUniqueType_ch16
	DMibStreamImplementSimpleEndianSwappedType(ch16);
#endif

#ifdef DMibPUniqueType_ch32
	DMibStreamImplementSimpleEndianSwappedType(ch32);
#endif
	DMibStreamImplementSimpleEndianSwappedType(fp32);
	DMibStreamImplementSimpleEndianSwappedType(fp64);


	template <typename t_CStream, typename t_CData, typename t_CTranslator, typename t_CLink, typename t_CLinkInList, bint t_bAutoDelete, typename t_CAllocator>
	class TCBinaryStreamTypeReference<t_CStream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> >
	{
	public:
		static void fs_Feed(t_CStream &_Stream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> const &_Data)
		{
			typename NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>::CIteratorConst Iter(_Data);

			mint nItems = 0;

			while (Iter)
			{
				++nItems;
				++Iter;
			};

			fg_FeedLenToStream(_Stream, nItems);

			Iter = _Data;

			while (Iter)
			{
				_Stream.f_Feed(*Iter);
				++Iter;
			};
		}

		static void fs_Feed(t_CStream &_Stream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &&_Data)
		{
			typename NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>::CIteratorConst Iter(_Data);

			mint nItems = 0;

			while (Iter)
			{
				++nItems;
				++Iter;
			};

			fg_FeedLenToStream(_Stream, nItems);

			Iter = _Data;

			while (Iter)
			{
				_Stream.f_Feed(fg_Move(*Iter));
				++Iter;
			};
		}

		static void fs_Consume(t_CStream &_Stream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &_Data)
		{
			uint64 nItems;
			fg_ConsumeLenFromStream(_Stream, nItems);
			fg_CheckLengthLimit(_Stream, nItems);

			while(nItems)
			{
				auto Memory = t_CAllocator::f_AllocSafe(sizeof(t_CData), NTraits::TCAlignmentOf<t_CData>::mc_Value);
				t_CData *pNewItem = new(Memory.m_pMemory) t_CData();
				Memory.f_Claim();
				auto Cleanup = g_OnScopeExit > [&]
					{
						pNewItem->~CNode();
						t_CAllocator::f_Free(pNewItem, sizeof(t_CData));
					}
				;
				_Stream.f_Consume(*pNewItem);
				_Data.f_Insert(pNewItem);
				Cleanup.f_Clear();
				--nItems;
			}
		}
	};

	template <typename t_CStream, typename t_CData, typename t_CTranslator, typename t_CLink, typename t_CLinkInList, bint t_bAutoDelete, typename t_CAllocator>
	class TCBinaryStreamTypeReference<t_CStream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> >
	{
	public:
		static void fs_Feed(t_CStream &_Stream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> const &_Data)
		{
			_Stream << (NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> const &)_Data;
		}

		static void fs_Feed(t_CStream &_Stream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &&_Data)
		{
			_Stream << fg_Move((NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &)_Data);
		}

		static void fs_Consume(t_CStream &_Stream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &_Data)
		{
			_Stream >> (NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &)_Data;
		}
	};


	template <typename t_CStream, typename t_CType, typename t_CAllocator, typename t_CPtr>
	class TCBinaryStreamTypeReference<t_CStream, NStorage::TCIndirection<t_CType, t_CAllocator, t_CPtr>>
	{
	public:
		static void fs_Feed(t_CStream &_Stream, NStorage::TCIndirection<t_CType, t_CAllocator, t_CPtr> const &_Data)
		{
			_Stream << _Data.f_Get();
		}

		static void fs_Feed(t_CStream &_Stream, NStorage::TCIndirection<t_CType, t_CAllocator, t_CPtr> &&_Data)
		{
			_Stream << fg_Move(_Data.f_Get());
		}

		static void fs_Consume(t_CStream &_Stream, NStorage::TCIndirection<t_CType, t_CAllocator, t_CPtr> &_Data)
		{
			_Stream >> _Data.f_Get();
		}
	};
}

#ifdef DMibIncluded_IntusiveAVLTree
#	include "../../Intrusive/Source/Malterlib_Intrusive_AVLTree_Stream.h"
#endif

#ifndef DMibPNoShortCuts
	using namespace NMib::NStream;
#endif
