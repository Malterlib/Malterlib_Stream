// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

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

	using CFilePos = CMibFilePos;

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

	template <typename t_CType>
	concept cIsValidStreamVersion =
		(
			NTraits::cIsEnum<t_CType>
			&& (sizeof(NTraits::TCEnumUnderlyingType<t_CType>) <= sizeof(uint32))
			&& !NTraits::cIsSigned<NTraits::TCEnumUnderlyingType<t_CType>>
		)
		||
		(
			NTraits::cIsInteger<t_CType>
			&& (sizeof(t_CType) <= sizeof(uint32))
			&& !NTraits::cIsSigned<t_CType>
		)
	;

	class CBinaryStream;

	struct [[nodiscard("You need to store version to keep it in scope")]] CScopeBinaryStreamVersion
	{
		CScopeBinaryStreamVersion() = delete;
		CScopeBinaryStreamVersion(CScopeBinaryStreamVersion &&_Other) = delete;
		CScopeBinaryStreamVersion & operator = (CScopeBinaryStreamVersion &&_Other) = delete;
		CScopeBinaryStreamVersion(CScopeBinaryStreamVersion const &_Other) = delete;
		CScopeBinaryStreamVersion & operator = (CScopeBinaryStreamVersion const &_Other) = delete;

		constexpr CScopeBinaryStreamVersion(CBinaryStream &_Stream, uint32 _Version);

		template <typename tf_CVersion>
		constexpr CScopeBinaryStreamVersion(CBinaryStream &_Stream, tf_CVersion _Version)
			requires(cIsValidStreamVersion<tf_CVersion>)
		;

		constexpr void f_SetVersion(CBinaryStream &_Stream, uint32 _Version);

		template <typename tf_CVersion>
		constexpr void f_SetVersion(CBinaryStream &_Stream, tf_CVersion _Version)
			requires(cIsValidStreamVersion<tf_CVersion>)
		;

		constexpr ~CScopeBinaryStreamVersion();
		constexpr void f_Clear();

	private:
		CBinaryStream *mp_pStream = nullptr;
		uint32 mp_OldVersion;
	};

#	define DMibStreamImplementOperatorsOperators(_Class) \
		template <typename tf_CData> constexpr inline_small _Class &operator << (tf_CData &&_Data) { this->f_Feed(fg_Forward<tf_CData>(_Data)); return *this; }\
		template <typename tf_CData> constexpr inline_small _Class &operator >> (tf_CData &&_Data) { this->f_Consume(fg_Forward<tf_CData>(_Data)); return *this; }\
		template <typename tf_CData> constexpr inline_small _Class &operator << (tf_CData const *_pData) { this->f_Feed(_pData); return *this; }\
		template <typename tf_CData> constexpr inline_small _Class &operator >> (tf_CData *_pData) { this->f_Consume(_pData); return *this; }\
		template <typename tf_CData> constexpr inline_small _Class &operator % (tf_CData &&_Data) { this->f_Stream(fg_Forward<tf_CData>(_Data)); return *this; }

#	define DMibStreamImplementOperators(_Class) \
		template <typename tf_CData> constexpr inline_small auto f_Feed(tf_CData &&_Data) \
		{ \
			return NMib::NStream::TCBinaryStreamTypeReference<_Class, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Feed(*this, NMib::fg_Forward<tf_CData>(_Data)); \
		} \
		template <typename tf_CData> constexpr inline_small auto f_Feed(const tf_CData *_pData) \
		{ \
			return NMib::NStream::TCBinaryStreamTypePtr<_Class, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Feed(*this, _pData); \
		} \
		template <typename tf_CData> constexpr inline_small auto f_Consume(tf_CData &&_Data) \
		{ \
			return NMib::NStream::TCBinaryStreamTypeReference<_Class, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Consume(*this, NMib::fg_Forward<tf_CData>(_Data)); \
		} \
		template <typename tf_CData> constexpr inline_small auto f_Consume(tf_CData *_pData) \
		{ \
			return NMib::NStream::TCBinaryStreamTypePtr<_Class, tf_CData>::fs_Consume(*this, _pData); \
		} \
		template <typename tf_CData> constexpr inline_small auto f_Stream(tf_CData &&_Data) \
		{ \
			return NMib::NStream::TCBinaryStreamTypeReferenceStream<_Class, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Stream(*this, NMib::fg_Forward<tf_CData>(_Data)); \
		} \
		template <typename tf_CData> constexpr inline_small NMib::NStream::CScopeBinaryStreamVersion f_StreamVersion(tf_CData &&_DefaultVersion) \
		{ \
			auto Version = _DefaultVersion; \
			this->f_Stream(Version); \
			return NMib::NStream::CScopeBinaryStreamVersion(*this, Version); \
		} \
		DMibStreamImplementOperatorsOperators(_Class)

	template <typename t_CStream, EStreamDirection t_Direction>
	struct TCStreamDirection : public t_CStream
	{
		template <typename tf_CData>
		constexpr inline_small auto f_Feed(tf_CData &&_Data)
		{
			return TCBinaryStreamTypeReference<t_CStream, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Feed(*this, NMib::fg_Forward<tf_CData>(_Data));
		}
		template <typename tf_CData>
		constexpr inline_small auto f_Feed(const tf_CData *_pData)
		{
			return TCBinaryStreamTypePtr<t_CStream, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Feed(*this, _pData);
		}
		template <typename tf_CData>
		constexpr inline_small auto f_Consume(tf_CData &&_Data)
		{
			return TCBinaryStreamTypeReference<t_CStream, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Consume(*this, NMib::fg_Forward<tf_CData>(_Data));
		}
		template <typename tf_CData>
		constexpr inline_small auto f_Consume(tf_CData *_pData)
		{
			return TCBinaryStreamTypePtr<t_CStream, tf_CData>::fs_Consume(*this, _pData);
		}
		template <typename tf_CData>
		constexpr inline_small auto f_Stream(tf_CData &&_Data)
		{
			return TCBinaryStreamTypeReferenceStream<t_CStream, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Stream(*this, NMib::fg_Forward<tf_CData>(_Data));
		}
		template <typename tf_CData>
		constexpr inline_small CScopeBinaryStreamVersion f_StreamVersion(tf_CData &&_DefaultVersion)
		{
			auto Version = _DefaultVersion;
			this->f_Stream(Version);
			return CScopeBinaryStreamVersion(*this, Version);
		}

		DMibStreamImplementOperatorsOperators(TCStreamDirection);

		t_CStream &f_GetStream()
		{
			return *this;
		}

		t_CStream const &f_GetStream() const
		{
			return *this;
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
			, TCEnableIf
			<
				!NTraits::cIsSame
				<
					decltype(fg_GetReference<NTraits::TCRemoveReferenceAndQualifiers<t_CData>>().f_Stream(fg_GetReference<t_CStream>())), NPrivate::CDummy
				>
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
			, TCEnableIf
			<
				!NTraits::cIsSame
				<
					decltype(fg_GetReference<t_CStream>().f_Stream(fg_GetType<t_CData>())), NPrivate::CDummy
				>
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
			, TCEnableIf
			<
				!NTraits::cIsSame
				<
					decltype(fg_GetReference<t_CStream>().f_Feed(fg_GetType<t_CData>())), NPrivate::CDummy
				>
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
			, TCEnableIf
			<
				!NTraits::cIsSame
				<
					decltype(fg_GetReference<t_CStream>().f_Consume(fg_GetType<t_CData>())), NPrivate::CDummy
				>
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
			, NMib::TCEnableIf
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
			, NMib::TCEnableIf
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
			, NMib::TCEnableIf
			<
				!NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Consume>, tf_CData>::mc_Value
			> * = nullptr
		>
		inline_small static auto fs_Stream(TCStreamDirection<t_CStream, EStreamDirection_Consume> &_Stream, tf_CData &&_Data)
		{
			static_cast<t_CStream &>(_Stream) >> const_cast<NTraits::TCRemoveReferenceAndQualifiers<tf_CData> &>(_Data);
		}
	};

	template <EStreamDirection tf_StreamDirection, typename tf_CStream>
	decltype(auto) fg_StreamDirection(tf_CStream &_Stream)
	{
		return reinterpret_cast<TCStreamDirection<tf_CStream, tf_StreamDirection> &>(_Stream);
	}

	template <typename tf_CStream>
	decltype(auto) fg_FeedStream(tf_CStream &_Stream)
	{
		return fg_StreamDirection<EStreamDirection_Feed>(_Stream);
	}

	template <typename tf_CStream>
	decltype(auto) fg_ConsumeStream(tf_CStream &_Stream)
	{
		return fg_StreamDirection<EStreamDirection_Consume>(_Stream);
	}

	template <typename t_CStream, typename t_CData>
	class TCBinaryStreamTypeReference
	{
	public:
		// Feed

		template
		<
			typename tf_CData
			, NMib::TCEnableIf
			<
				!NMib::NTraits::cIsEnum<NTraits::TCRemoveReference<tf_CData>>
				&& !NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Feed>, tf_CData>::mc_Value
			> * = nullptr
		>
		constexpr inline_small static auto fs_Feed(t_CStream &_Stream, tf_CData &&_Data)
		{
			return fg_Forward<tf_CData>(_Data).f_Feed(_Stream);
		}

		template
		<
			typename tf_CData
			, NMib::TCEnableIf
			<
				!NMib::NTraits::cIsEnum<NTraits::TCRemoveReference<tf_CData>>
				&& NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Feed>, tf_CData>::mc_Value
			> * = nullptr
		>
		constexpr inline_small static auto fs_Feed(t_CStream &_Stream, tf_CData &&_Data)
		{
			return const_cast<NTraits::TCRemoveReferenceAndQualifiers<tf_CData> &>(_Data).f_Stream(reinterpret_cast<TCStreamDirection<t_CStream, EStreamDirection_Feed> &>(_Stream));
		}

		// Consume

		template
		<
			typename tf_CData
			, NMib::TCEnableIf
			<
				!NMib::NTraits::cIsEnum<NTraits::TCRemoveReference<tf_CData>>
				&& !NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Consume>, tf_CData>::mc_Value
			> * = nullptr
		>
		constexpr inline_small static auto fs_Consume(t_CStream &_Stream, tf_CData &&_Data)
		{
			return _Data.f_Consume(_Stream);
		}

		template
		<
			typename tf_CData
			, NMib::TCEnableIf
			<
				!NMib::NTraits::cIsEnum<NTraits::TCRemoveReference<tf_CData>>
				&& NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Consume>, tf_CData>::mc_Value
			> * = nullptr
		>
		constexpr inline_small static auto fs_Consume(t_CStream &_Stream, tf_CData &&_Data)
		{
			return fg_Forward<tf_CData>(_Data).f_Stream(reinterpret_cast<TCStreamDirection<t_CStream, EStreamDirection_Consume> &>(_Stream));
		}

		template <typename tf_CData, NMib::TCEnableIf<NMib::NTraits::cIsEnum<tf_CData>, void> * = nullptr>
		constexpr inline_small static void fs_Feed(t_CStream &_Stream, tf_CData const &_Data)
		{
			if constexpr (NTraits::cIsScopedEnum<tf_CData>)
				_Stream << NTraits::TCEnumUnderlyingType<tf_CData>(_Data);
			else
				_Stream << uint32(_Data);
		}

		template <typename tf_CData, NMib::TCEnableIf<NMib::NTraits::cIsEnum<tf_CData>, void> * = nullptr>
		constexpr inline_small static void fs_Consume(t_CStream &_Stream, tf_CData &_Data)
		{
			if constexpr (NTraits::cIsScopedEnum<tf_CData>)
			{
				NTraits::TCEnumUnderlyingType<tf_CData> Temp;
				_Stream >> Temp;
				_Data = static_cast<tf_CData>(Temp);
			}
			else
			{
				uint32 Temp;
				_Stream >> Temp;
				_Data = static_cast<tf_CData>(Temp);
			}
		}
	};


	template <typename t_CStream, typename t_CData>
	class TCBinaryStreamTypePtr
	{
	public:
		constexpr static void fs_Feed(t_CStream &_Stream, const t_CData *_pData)
		{
			t_CStream::Implement_Error; // Type not implemented
			// _Stream.f_FeedBytes(_pData, sizeof(*_pData));
		}
		constexpr static void fs_Consume(t_CStream &_Stream, t_CData *_pData)
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
		using CType = t_CType;

		CType &m_Data;
		TCBinaryStreamUnsafeWrapper(t_CType &_Data)
			:m_Data(_Data)
		{
		}
	};

#	if defined(DMibDebug) && 0
#		define DMibTempStreamDebug
#		define DMibTempStreamPre virtual
#		define DMibTempStreamPost =0
#	else
#		define DMibTempStreamPre inline_small
#		define DMibTempStreamPost
#	endif

#ifdef DMibTempStreamDebug

#	define DMibStreamImplementProtected(_Class) \
		void fp_FeedBytes(const void *_pMem, umint _nBytes){this->f_FeedBytes(_pMem, _nBytes);}\
		void fp_ConsumeBytes(void *_pMem, umint _nBytes){this->f_ConsumeBytes(_pMem, _nBytes);}\
		bool fp_IsValid() const {bool Ret = 0; Ret = this->f_IsValid(); return Ret;}\
		bool fp_IsAtEndOfStream() const {bool Ret = 0; Ret = this->f_IsAtEndOfStream(); return Ret;}\
		NMib::NStream::CFilePos fp_GetPosition() const {return this->f_GetPosition();}\
		void fp_SetPosition(NMib::NStream::CFilePos _Pos){this->f_SetPosition(_Pos);}\
		void fp_SetPositionFromEnd(NMib::NStream::CFilePos _Pos){this->f_SetPositionFromEnd(_Pos);}\
		void fp_AddPosition(NMib::NStream::CFilePos _Pos){this->f_AddPosition(_Pos);}\
		bool fp_IsValidReadPosition(NMib::NStream::CFilePos _Pos) const {bool bRet = 0; bRet = this->f_IsValidReadPosition(_Pos); return bRet; }\
		void fp_Flush(bool _bLocalCacheOnly) {this->f_Flush(_bLocalCacheOnly);}\
		void fp_SetCacheSize(umint _CacheSize) {this->f_SetCacheSize(_CacheSize);}\
		NMib::NStream::CFilePos fp_GetLength() const {NMib::NStream::CFilePos Ret = 0; Ret = this->f_GetLength(); return Ret;}\
		void fp_SetLength(NMib::NStream::CFilePos _Length) {return this->f_SetLength(_Length);}\
		aint fp_LengthSize() const {aint Ret = 0; Ret = this->f_LengthSize(); return Ret;}\
		aint fp_Endian() const {aint Ret = 0; Ret = this->f_Endian(); return Ret;}\
		umint fp_ContainerLengthLimit() const {umint Ret = 0; Ret = this->f_ContainerLengthLimit(); return Ret;}\
		void fp_FeedFromStream(NMib::NStream::CBinaryStream &_Stream, NMib::NStream::CFilePos _nBytes){this->f_FeedFromStream(_Stream, _nBytes);}\

#else

#	define DMibStreamImplementProtected(_Class) \
		void fp_FeedBytes(const void *_pMem, umint _nBytes){_Class::f_FeedBytes(_pMem, _nBytes);}\
		void fp_ConsumeBytes(void *_pMem, umint _nBytes){_Class::f_ConsumeBytes(_pMem, _nBytes);}\
		bool fp_IsValid() const {bool Ret = 0; Ret = _Class::f_IsValid(); return Ret;}\
		bool fp_IsAtEndOfStream() const {bool Ret = 0; Ret = _Class::f_IsAtEndOfStream(); return Ret;}\
		NMib::NStream::CFilePos fp_GetPosition() const {return _Class::f_GetPosition();}\
		void fp_SetPosition(NMib::NStream::CFilePos _Pos){_Class::f_SetPosition(_Pos);}\
		void fp_SetPositionFromEnd(NMib::NStream::CFilePos _Pos){_Class::f_SetPositionFromEnd(_Pos);}\
		void fp_AddPosition(NMib::NStream::CFilePos _Pos){_Class::f_AddPosition(_Pos);}\
		bool fp_IsValidReadPosition(NMib::NStream::CFilePos _Pos) const {bool bRet = 0; bRet = _Class::f_IsValidReadPosition(_Pos); return bRet; }\
		void fp_Flush(bool _bLocalCacheOnly) {_Class::f_Flush(_bLocalCacheOnly);}\
		void fp_SetCacheSize(umint _CacheSize) {_Class::f_SetCacheSize(_CacheSize);}\
		NMib::NStream::CFilePos fp_GetLength() const {NMib::NStream::CFilePos Ret = 0; Ret =_Class::f_GetLength(); return Ret;}\
		void fp_SetLength(NMib::NStream::CFilePos _Length) {return _Class::f_SetLength(_Length);}\
		aint fp_LengthSize() const {aint Ret = 0; Ret = _Class::f_LengthSize(); return Ret;}\
		aint fp_Endian() const {aint Ret = 0; Ret = _Class::f_Endian(); return Ret;}\
		umint fp_ContainerLengthLimit() const {umint Ret = 0; Ret = _Class::f_ContainerLengthLimit(); return Ret;}\
		void fp_FeedFromStream(NMib::NStream::CBinaryStream &_Stream, NMib::NStream::CFilePos _nBytes){_Class::f_FeedFromStream(_Stream, _nBytes);}\

#endif

	class CScopeBinaryStreamContext;
	class CScopeBinaryStreamContainerLengthLimit;

	class CBinaryStream
	{
		friend struct CScopeBinaryStreamVersion;
		friend class CScopeBinaryStreamContext;
		friend class CScopeBinaryStreamContainerLengthLimit;
	public:
		constexpr CBinaryStream() = default;
		constexpr virtual ~CBinaryStream() = default;
	private:

		void *m_pContext = nullptr;
		umint m_ContainerLengthLimit = 0;
		uint32 m_Version = 0;

		CBinaryStream(CBinaryStream const &) = delete;
		CBinaryStream &operator = (CBinaryStream const &) = delete;

	protected:
		virtual void fp_FeedBytes(const void *_pMem, umint _nBytes) = 0;
		virtual void fp_ConsumeBytes(void *_pMem, umint _nBytes) = 0;
		virtual bool fp_IsValid() const = 0;
		virtual bool fp_IsAtEndOfStream() const = 0;
		virtual CFilePos fp_GetPosition() const = 0;
		virtual void fp_SetPosition(CFilePos _Pos) = 0;
		virtual void fp_SetPositionFromEnd(CFilePos _Pos) = 0;
		virtual void fp_AddPosition(CFilePos _Pos) = 0;
		virtual bool fp_IsValidReadPosition(NStream::CFilePos _Pos) const = 0;
		virtual void fp_Flush(bool _bLocalCacheOnly) = 0;
		virtual void fp_SetCacheSize(umint _CacheSize) = 0;
		virtual CFilePos fp_GetLength() const = 0;
		virtual void fp_SetLength(CFilePos _Length) = 0;
		virtual	aint fp_LengthSize() const = 0;
		virtual aint fp_Endian() const = 0;
		virtual umint fp_ContainerLengthLimit() const = 0;
		virtual void fp_FeedFromStream(CBinaryStream &_Stream, CFilePos _nBytes) = 0;

		inline_never void fp_ThrowEndOfStreamException()
		{
			DMibError("End of stream Overrun");
		}

	public:

#ifdef DMibTempStreamDebug
		DMibTempStreamPre void f_FeedBytes(const void *_pMem, umint _nBytes) DMibTempStreamPost;
		DMibTempStreamPre void f_ConsumeBytes(void *_pMem, umint _nBytes) DMibTempStreamPost;
		DMibTempStreamPre bool f_IsValid() const DMibTempStreamPost;
		DMibTempStreamPre bool f_IsAtEndOfStream() const DMibTempStreamPost;
		DMibTempStreamPre CFilePos f_GetPosition() const DMibTempStreamPost;
		DMibTempStreamPre void f_SetPosition(CFilePos _Pos) DMibTempStreamPost;
		DMibTempStreamPre void f_SetPositionFromEnd(CFilePos _Pos) DMibTempStreamPost;
		DMibTempStreamPre void f_AddPosition(CFilePos _Pos) DMibTempStreamPost;
		DMibTempStreamPre bool f_IsValidReadPosition(CFilePos _Pos) const DMibTempStreamPost;
		DMibTempStreamPre void f_Flush(bool _bLocalCacheOnly) DMibTempStreamPost;
		DMibTempStreamPre void f_SetCacheSize(umint _CacheSize) DMibTempStreamPost;
		DMibTempStreamPre CFilePos f_GetLength() const DMibTempStreamPost;
		DMibTempStreamPre void f_SetLength(CFilePos _Length) DMibTempStreamPost;
		DMibTempStreamPre aint f_LengthSize() const DMibTempStreamPost;
		DMibTempStreamPre aint f_Endian() const DMibTempStreamPost;
		DMibTempStreamPre umint f_ContainerLengthLimit() const DMibTempStreamPost;
		DMibTempStreamPre void f_FeedFromStream(CBinaryStream &_Stream, CFilePos _nBytes) DMibTempStreamPost;
#else
		DMibTempStreamPre void f_FeedBytes(const void *_pMem, umint _nBytes) DMibTempStreamPost
		{
			fp_FeedBytes(_pMem, _nBytes);
		}

		DMibTempStreamPre void f_ConsumeBytes(void *_pMem, umint _nBytes) DMibTempStreamPost
		{
			fp_ConsumeBytes(_pMem, _nBytes);
		}

		DMibTempStreamPre bool f_IsValid() const DMibTempStreamPost
		{
			return fp_IsValid();
		}

		DMibTempStreamPre bool f_IsAtEndOfStream() const DMibTempStreamPost
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
		DMibTempStreamPre bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return fp_IsValidReadPosition(_Pos);
		}

		DMibTempStreamPre void f_Flush(bool _bLocalCacheOnly)
		{
			return fp_Flush(_bLocalCacheOnly);
		}

		DMibTempStreamPre void f_SetCacheSize(umint _CacheSize)
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

		DMibTempStreamPre umint f_ContainerLengthLimit() const DMibTempStreamPost
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

		template <typename tf_CVersion>
		inline_small bool f_SupportsVersion(tf_CVersion _Version) const
			requires(cIsValidStreamVersion<tf_CVersion>)
		{
			return m_Version >= uint32(_Version);
		}

		inline_small void *f_GetContext() const
		{
			return m_pContext;
		}

		inline_small umint f_ClaimContainerLengthLimitOverride()
		{
			umint Return = m_ContainerLengthLimit;
			m_ContainerLengthLimit = 0;
			return Return;
		}

		DMibStreamImplementOperators(CBinaryStream);
	};

	template <typename tf_CStream>
	void fg_PadAlignStream(tf_CStream &_Stream, umint _Alignment)
	{
		CFilePos Position = _Stream.f_GetPosition();
		CFilePos EndPosition = fg_AlignUp(Position, CFilePos(_Alignment));
		while (Position != EndPosition)
		{
			uint8 PaddingData[128] = {0};

			umint ThisTime = fg_Min(EndPosition - Position, CFilePos(128));

			_Stream.f_FeedBytes(PaddingData, ThisTime);
			Position += ThisTime;
		}
	}

	template <typename tf_CStream>
	void fg_AlignStream(tf_CStream &_Stream, umint _Alignment)
	{
		CFilePos Position = _Stream.f_GetPosition();
		CFilePos EndPosition = fg_AlignUp(Position, CFilePos(_Alignment));
		_Stream.f_SetPosition(EndPosition);
	}

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

		constexpr CBinaryStreamDefault() = default;

		constexpr inline_small aint f_LengthSize() const
		{
			return sizeof(uint32);
		}

		constexpr inline_small aint f_Endian() const
		{
			return EEndian_Little;
		}

		constexpr inline_small void f_FeedFromStream(CBinaryStream &_Stream, CFilePos _nBytes)
		{
			if_consteval
			{
				uint8 Temp[1024];
				CFilePos ToTransfer = _nBytes;
				while (ToTransfer)
				{
					umint ThisTime = fg_Min(ToTransfer, 1024);
					_Stream.f_ConsumeBytes(Temp, ThisTime);
					fp_FeedBytes(Temp, ThisTime);
					ToTransfer -= ThisTime;
				}
			}
			else
			{
				fp_FeedFromStreamImplementation(_Stream, _nBytes);
			}
		}

	private:
		void fp_FeedFromStreamImplementation(CBinaryStream &_Stream, CFilePos _nBytes);
	};

	class CBinaryStreamDefaultRef : public CBinaryStreamDefault
	{
	public:
		NStorage::CIntrusiveRefCount m_RefCount;

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
		umint m_Position = 0;
		umint m_Length = 0;

		constexpr void fp_SetPositionInternal(CFilePos _Pos)
		{
			if (_Pos < 0)
				DMibError("Memory stream positions are limited to 0 -> TCLimitsInt<umint>::mc_Max");

			m_Position = _Pos;
		}
		DMibStreamImplementProtected(TCBinaryStreamNull);
	public:
		DMibStreamImplementOperators(TCBinaryStreamNull);

		constexpr TCBinaryStreamNull() = default;

		constexpr void f_Reset()
		{
			m_Position = 0;
		}

		constexpr void f_Clear()
		{
			m_Position = 0;
		}

		constexpr void f_FeedBytes(const void *_pMem, umint _nBytes)
		{
			m_Position += _nBytes;
			if (m_Position > m_Length)
				m_Length = m_Position;

		}

		constexpr void f_ConsumeBytes(void *_pMem, umint _nBytes)
		{
			if (m_Length < (m_Position + _nBytes) )
				DMibError("End of stream Overrun");
			m_Position += _nBytes;
		}

		constexpr bool f_IsValid() const
		{
			return true;
		}

		constexpr bool f_IsAtEndOfStream() const
		{
			return m_Position == m_Length;
		}

		constexpr CFilePos f_GetPosition() const
		{
			return m_Position;
		}

		constexpr void f_SetPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(_Pos);
		}

		constexpr void f_SetPositionFromEnd(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Length + _Pos);
		}

		constexpr void f_AddPosition(CFilePos _Pos)
		{
			fp_SetPositionInternal(m_Position + _Pos);
		}

		constexpr bool f_IsValidReadPosition(NStream::CFilePos _Pos) const
		{
			return _Pos >= 0 && _Pos < NStream::CFilePos(m_Length);
		}

		constexpr void f_Flush(bool _bLocalCacheOnly)
		{
		}

		constexpr void f_SetCacheSize(umint _CacheSize)
		{
		}

		constexpr CFilePos f_GetLength() const
		{
			return m_Length;
		}

		constexpr umint f_ContainerLengthLimit() const
		{
			return f_GetLength() - f_GetPosition();
		}

		constexpr void f_SetLength(NStream::CFilePos _Length)
		{
			m_Length = _Length;
		}
	};
	// Default types

	template <typename tf_CType>
	CFilePos fg_GetBinaryStreamSize(tf_CType const &_Value)
	{
		TCBinaryStreamNull<> SizeStream;
		SizeStream << _Value;
		return SizeStream.f_GetPosition();
	}

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

	inline_always umint fg_CapLengthLimit(NStream::CFilePos const &_Len)
	{
		if constexpr (sizeof(umint) < sizeof(NStream::CFilePos))
		{
			if (_Len > NStream::CFilePos{TCLimitsInt<smint>::mc_Max})
				return TCLimitsInt<smint>::mc_Max;
		}
		return _Len;
	}

	void fg_ReportLengthLimitError(uint64 _Len, uint64 _LengthLimit);

	template <typename tf_CStream, typename tf_CLen>
	void fg_CheckLengthLimit(tf_CStream &_Stream, tf_CLen const &_Len)
	{
		umint LengthLimit = _Stream.f_ClaimContainerLengthLimitOverride();
		if (!LengthLimit)
			LengthLimit = _Stream.f_ContainerLengthLimit();

		if (_Len > uint64(LengthLimit))
			fg_ReportLengthLimitError(_Len, LengthLimit);
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
	void fg_FeedEndianArrayToStream(t_CStream &_Stream, const t_CData *_pData, umint _Len, aint _Endian)
	{
		if (_Endian == EEndian_Native || _Endian == gc_MachineEndian)
		{
			if (_Len != 0)
				_Stream.f_FeedBytes(_pData, _Len * sizeof(t_CData));
		}
		else
		{
			for (umint i = 0; i < _Len; ++i)
			{
				t_CData Temp = fg_ByteSwap(_pData[i]);
				_Stream.f_FeedBytes(&Temp, sizeof(t_CData));
			}
		}
	}

	template <typename t_CStream, typename t_CData>
	void fg_ByteSwapArray(t_CStream &_Stream, t_CData *_pData, umint _Len, aint _Endian)
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
		static constexpr void fs_Feed(t_CStream &_Stream, const ch8 *_pData)
		{
			uint64 Len = NMib::NStr::fg_StrLen(_pData);
			uint64 LenStream = Len;
			fg_StrEncodeLenType(LenStream, _Stream.f_LengthSize(), NStr::EStrType_UTF); // Presume UTF, as this is the most probable case
			fg_FeedLenToStream(_Stream, LenStream);
			if (Len != 0)
				_Stream.f_FeedBytes(_pData, Len);
		}
		static constexpr void fs_Consume(t_CStream &_Stream, ch8 *_pData)
		{
			uint64 Len;
			fg_ConsumeLenFromStream(_Stream, Len);
			NStr::EStrType Type = NStr::EStrType_Ansi;
			fg_StrDecodeLenType(Len, _Stream.f_LengthSize(), Type);
			fg_CheckLengthLimit(_Stream, Len);
			if (Len != 0)
				_Stream.f_ConsumeBytes(_pData, Len);
			_pData[Len] = 0;
		}
	};

	template <typename t_CStream>
	class TCBinaryStreamTypePtr<t_CStream, ch16>
	{
	public:
		static constexpr void fs_Feed(t_CStream &_Stream, const ch16 *_pData)
		{
			umint Len = NMib::NStr::fg_StrLen(_pData);
			uint64 LenStream = Len;
			fg_StrEncodeLenType(LenStream, _Stream.f_LengthSize(), NStr::EStrType_UTF); // Presume UTF, as this is the most probable case
			fg_FeedLenToStream(_Stream, LenStream);
			fg_FeedEndianArrayToStream(_Stream, _pData, Len, _Stream.f_Endian());
		}
		static constexpr void fs_Consume(t_CStream &_Stream, ch16 *_pData)
		{
			uint64 Len;
			fg_ConsumeLenFromStream(_Stream, Len);
			NStr::EStrType Type = NStr::EStrType_Ansi;
			fg_StrDecodeLenType(Len, _Stream.f_LengthSize(), Type);
			fg_CheckLengthLimit(_Stream, Len);
			if (Len != 0)
				_Stream.f_ConsumeBytes(_pData, Len);
			fg_ByteSwapArray(_Stream, _pData, Len, _Stream.f_Endian());
			_pData[Len] = 0;
		}
	};

	template <typename t_CStream>
	class TCBinaryStreamTypePtr<t_CStream, ch32>
	{
	public:
		static constexpr void fs_Feed(t_CStream &_Stream, const ch32 *_pData)
		{
			umint Len = NMib::NStr::fg_StrLen(_pData);
			uint64 LenStream = Len;
			fg_StrEncodeLenType(LenStream, _Stream.f_LengthSize(), NStr::EStrType_Unicode);
			fg_FeedLenToStream(_Stream, LenStream);
			fg_FeedEndianArrayToStream(_Stream, _pData, Len, _Stream.f_Endian());
		}
		static constexpr void fs_Consume(t_CStream &_Stream, ch32 *_pData)
		{
			uint64 Len;
			fg_ConsumeLenFromStream(_Stream, Len);
			NStr::EStrType Type = NStr::EStrType_Ansi;
			fg_StrDecodeLenType(Len, _Stream.f_LengthSize(), Type);
			fg_CheckLengthLimit(_Stream, Len);
			if (Len != 0)
				_Stream.f_ConsumeBytes(_pData, Len);
			fg_ByteSwapArray(_Stream, _pData, Len, _Stream.f_Endian());
			_pData[Len] = 0;
		}
	};

	template <typename t_CStream, typename t_CType>
	class TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<const t_CType> >
	{
	public:
		static constexpr void fs_Feed(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<const t_CType> const &_Data)
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
		static constexpr void fs_Feed(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> const &_Data)
		{
			_Stream << _Data.m_Data;
		}
		static constexpr void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> &_Data)
		{
			_Stream >> _Data.m_Data;
		}
		static constexpr void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> &&_Data)
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

#	define DMibStreamImplementSimpleEndianSwappedType(_Type) \
	template <typename t_CStream> \
	class TCBinaryStreamTypeReference<t_CStream, _Type> \
	{ \
	public: \
		static constexpr void fs_Feed(t_CStream &_Stream, _Type const &_Data) \
		{ \
			aint Endian = _Stream.f_Endian();\
			if (Endian == gc_MachineEndian || Endian == EEndian_Native)\
				_Stream.f_FeedBytes(&_Data, sizeof(_Data));\
			else\
			{\
				auto Temp = fg_ByteSwap(reinterpret_cast<NTraits::TCIntFromSize<sizeof(_Data)> const &>(_Data));\
				_Stream.f_FeedBytes(&Temp, sizeof(_Data));\
			} \
		}\
		static constexpr void fs_Consume(t_CStream &_Stream, _Type &_Data)\
		{\
			aint Endian = _Stream.f_Endian();\
			if (Endian == gc_MachineEndian || Endian == EEndian_Native)\
				_Stream.f_ConsumeBytes(&_Data, sizeof(_Data));\
			else\
			{\
				NTraits::TCIntFromSize<sizeof(_Data)> Data;\
				_Stream.f_ConsumeBytes(&Data, sizeof(Data));\
				_Data = fg_ByteSwap(reinterpret_cast<_Type &>(Data));\
			}\
		}\
	};

#	define DMibStreamImplementSimpleEndianSwappedTypeUnsafe(_Type) \
	template <typename t_CStream> \
	class TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<const _Type> > \
	{ \
	public: \
		static constexpr void fs_Feed(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<const _Type> const &_Data) \
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
		static constexpr void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<_Type> const &_Data)\
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
		static constexpr void fs_Feed(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<_Type> const &_Data) \
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
		static constexpr void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<_Type> &_Data)\
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
		static constexpr void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<_Type> &&_Data)\
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


#	define DMibStreamImplementSimpleType(_Type) \
	template <typename t_CStream> \
	class TCBinaryStreamTypeReference<t_CStream, _Type> \
	{ \
	public: \
		static constexpr void fs_Feed(t_CStream &_Stream, _Type const &_Data) \
		{ \
			_Stream.f_FeedBytes(&_Data, sizeof(_Data));\
		}\
		static constexpr void fs_Consume(t_CStream &_Stream, _Type &_Data)\
		{\
			_Stream.f_ConsumeBytes(&_Data, sizeof(_Data));\
		}\
	};

	template <typename t_CStream>
	class TCBinaryStreamTypeReference<t_CStream, bool>
	{
	public:
		static constexpr void fs_Feed(t_CStream &_Stream, bool const &_Data)
		{
			uint8 Byte = _Data ? 1 : 0;
			_Stream.f_FeedBytes(&Byte, sizeof(Byte));
		}
		static constexpr void fs_Consume(t_CStream &_Stream, bool &_Data)
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
	DMibStreamImplementSimpleEndianSwappedTypeUnsafe(umint);
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


	template <typename t_CStream, typename t_CData, typename t_CTranslator, typename t_CLink, typename t_CLinkInList, bool t_bAutoDelete, typename t_CAllocator>
	class TCBinaryStreamTypeReference<t_CStream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> >
	{
	public:
		static constexpr void fs_Feed(t_CStream &_Stream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> const &_Data)
		{
			typename NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>::CIteratorConst Iter(_Data);

			umint nItems = 0;

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

		static constexpr void fs_Feed(t_CStream &_Stream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &&_Data)
		{
			typename NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>::CIteratorConst Iter(_Data);

			umint nItems = 0;

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

		static constexpr void fs_Consume(t_CStream &_Stream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &_Data)
		{
			uint64 nItems;
			fg_ConsumeLenFromStream(_Stream, nItems);
			fg_CheckLengthLimit(_Stream, nItems);

			while(nItems)
			{
				auto Memory = t_CAllocator::f_AllocSafe(sizeof(t_CData), alignof(t_CData));
				t_CData *pNewItem = new(Memory.m_pMemory) t_CData();
				Memory.f_Claim();
				auto Cleanup = g_OnScopeExit / [&]
					{
						pNewItem->~t_CData();
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

	template <typename t_CStream, typename t_CData, typename t_CTranslator, typename t_CLink, typename t_CLinkInList, bool t_bAutoDelete, typename t_CAllocator>
	class TCBinaryStreamTypeReference<t_CStream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> >
	{
	public:
		static constexpr void fs_Feed(t_CStream &_Stream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> const &_Data)
		{
			_Stream << (NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> const &)_Data;
		}

		static constexpr void fs_Feed(t_CStream &_Stream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &&_Data)
		{
			_Stream << fg_Move((NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &)_Data);
		}

		static constexpr void fs_Consume(t_CStream &_Stream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &_Data)
		{
			_Stream >> (NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &)_Data;
		}
	};

	template <typename t_CStream, typename t_CType, typename ...tp_COptions>
	class TCBinaryStreamTypeReference<t_CStream, NStorage::TCUniquePointer<t_CType, tp_COptions...>>
	{
	public:
		static constexpr void fs_Feed(t_CStream &_Stream, NStorage::TCUniquePointer<t_CType, tp_COptions...> const &_pData)
		{
			uint8 bNonEmpty = !_pData.f_IsEmpty();
			_Stream << bNonEmpty;
			if (bNonEmpty)
				_Stream << *_pData;
		}

		static constexpr void fs_Feed(t_CStream &_Stream, NStorage::TCUniquePointer<t_CType, tp_COptions...> &&_pData)
		{
			uint8 bNonEmpty = !_pData.f_IsEmpty();
			_Stream << bNonEmpty;
			if (bNonEmpty)
				_Stream << fg_Move(*_pData);
		}

		static constexpr void fs_Consume(t_CStream &_Stream, NStorage::TCUniquePointer<t_CType, tp_COptions...> &_pData)
		{
			uint8 bNonEmpty;
			_Stream >> bNonEmpty;
			if (bNonEmpty)
			{
				_pData = fg_Construct();
				_Stream >> *_pData;
			}
		}
	};

	template <typename t_CStream, typename t_CType, typename ...tp_COptions>
	class TCBinaryStreamTypeReference<t_CStream, NStorage::TCSharedPointer<t_CType, tp_COptions...>>
	{
	public:
		static constexpr void fs_Feed(t_CStream &_Stream, NStorage::TCSharedPointer<t_CType, tp_COptions...> const &_pData)
		{
			uint8 bNonEmpty = !_pData.f_IsEmpty();
			_Stream << bNonEmpty;
			if (bNonEmpty)
				_Stream << *_pData;
		}

		static constexpr void fs_Feed(t_CStream &_Stream, NStorage::TCSharedPointer<t_CType, tp_COptions...> &&_pData)
		{
			return fs_Feed(_Stream, _pData);
		}

		static constexpr void fs_Consume(t_CStream &_Stream, NStorage::TCSharedPointer<t_CType, tp_COptions...> &_pData)
		{
			uint8 bNonEmpty;
			_Stream >> bNonEmpty;
			if (bNonEmpty)
			{
				_pData = fg_Construct();
				_Stream >> *_pData;
			}
		}
	};
}

#ifdef DMibIncluded_IntusiveAVLTree
#	include "../../Intrusive/Source/Malterlib_Intrusive_AVLTree_Stream.h"
#endif

#ifndef DMibPNoShortCuts
	using namespace NMib::NStream;
#endif

#include "Malterlib_Stream.hpp"
