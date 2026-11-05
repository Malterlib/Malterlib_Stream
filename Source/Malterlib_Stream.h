// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>

#define DMibIncluded_Stream

namespace NMib::NStream
{
	DMibImpErrorClassDefine(CExceptionStream, NMib::NException::CException);
	DMibImpErrorClassDefine(CExceptionStreamVersionMismatch, CExceptionStream);

#define DMibErrorStream(d_Description) DMibImpError(NMib::NStream::CExceptionStream, d_Description)

#ifndef DMibPNoShortCuts
#	define DErrorStream(d_Description) DMibErrorStream(d_Description)
#endif

#define DMibErrorStreamVersionMismatch(d_Description) DMibImpError(NMib::NStream::CExceptionStreamVersionMismatch, d_Description)

#ifndef DMibPNoShortCuts
#	define DErrorStreamVersionMismatch(d_Description) DMibErrorStreamVersionMismatch(d_Description)
#endif

	using CFilePos = CMibFilePos;

	enum EStreamDirection
	{
		EStreamDirection_Feed
		, EStreamDirection_Consume
	};

	template <typename t_CStream, typename t_CData>
	struct TCBinaryStreamTypeReference;

	template <typename t_CStream, typename t_CData>
	struct TCBinaryStreamTypeReferenceStream;

	template <typename t_CStream, typename t_CData>
	struct TCBinaryStreamTypePtr;

	template <typename t_CType>
	concept cIsValidStreamVersion
		= (NTraits::cIsEnum<t_CType> && (sizeof(NTraits::TCEnumUnderlyingType<t_CType>) <= sizeof(uint32)) && !NTraits::cIsSigned<NTraits::TCEnumUnderlyingType<t_CType>>)
		|| (NTraits::cIsInteger<t_CType> && (sizeof(t_CType) <= sizeof(uint32)) && !NTraits::cIsSigned<t_CType>)
	;

	struct CBinaryStream;

	struct [[nodiscard("You need to store version to keep it in scope")]] CScopeBinaryStreamVersion
	{
		CScopeBinaryStreamVersion() = delete;
		CScopeBinaryStreamVersion(CScopeBinaryStreamVersion &&_Other) = delete;
		CScopeBinaryStreamVersion &operator = (CScopeBinaryStreamVersion &&_Other) = delete;
		CScopeBinaryStreamVersion(CScopeBinaryStreamVersion const &_Other) = delete;
		CScopeBinaryStreamVersion &operator = (CScopeBinaryStreamVersion const &_Other) = delete;

		constexpr CScopeBinaryStreamVersion(CBinaryStream &_Stream, uint32 _Version);

		template <typename tf_CVersion>
		constexpr CScopeBinaryStreamVersion(CBinaryStream &_Stream, tf_CVersion _Version)
			requires (cIsValidStreamVersion<tf_CVersion>)
		;

		constexpr void f_SetVersion(CBinaryStream &_Stream, uint32 _Version);

		template <typename tf_CVersion>
		constexpr void f_SetVersion(CBinaryStream &_Stream, tf_CVersion _Version)
			requires (cIsValidStreamVersion<tf_CVersion>)
		;

		constexpr ~CScopeBinaryStreamVersion();
		constexpr void f_Clear();

	private:
		CBinaryStream *mp_pStream = nullptr;
		uint32 mp_OldVersion;
	};

#define DMibStreamImplementOperatorsOperators(d_Class) \
	template <typename tf_CData> constexpr inline_small d_Class &operator << (tf_CData &&_Data) { this->f_Feed(fg_Forward<tf_CData>(_Data)); return *this; }\
	template <typename tf_CData> constexpr inline_small d_Class &operator >> (tf_CData &&_Data) { this->f_Consume(fg_Forward<tf_CData>(_Data)); return *this; }\
	template <typename tf_CData> constexpr inline_small d_Class &operator << (tf_CData const *_pData) { this->f_Feed(_pData); return *this; }\
	template <typename tf_CData> constexpr inline_small d_Class &operator >> (tf_CData *_pData) { this->f_Consume(_pData); return *this; }\
	template <typename tf_CData> constexpr inline_small d_Class &operator % (tf_CData &&_Data) { this->f_Stream(fg_Forward<tf_CData>(_Data)); return *this; }

#define DMibStreamImplementOperators(d_Class) \
	template <typename tf_CData> constexpr inline_small auto f_Feed(tf_CData &&_Data) \
	{ \
		return NMib::NStream::TCBinaryStreamTypeReference<d_Class, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Feed(*this, NMib::fg_Forward<tf_CData>(_Data)); \
	} \
	template <typename tf_CData> constexpr inline_small auto f_Feed(const tf_CData *_pData) \
	{ \
		return NMib::NStream::TCBinaryStreamTypePtr<d_Class, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Feed(*this, _pData); \
	} \
	template <typename tf_CData> constexpr inline_small auto f_Consume(tf_CData &&_Data) \
	{ \
		return NMib::NStream::TCBinaryStreamTypeReference<d_Class, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Consume(*this, NMib::fg_Forward<tf_CData>(_Data)); \
	} \
	template <typename tf_CData> constexpr inline_small auto f_Consume(tf_CData *_pData) \
	{ \
		return NMib::NStream::TCBinaryStreamTypePtr<d_Class, tf_CData>::fs_Consume(*this, _pData); \
	} \
	template <typename tf_CData> constexpr inline_small auto f_Stream(tf_CData &&_Data) \
	{ \
		return NMib::NStream::TCBinaryStreamTypeReferenceStream<d_Class, NMib::NTraits::TCRemoveReferenceAndQualifiers<tf_CData>>::fs_Stream(*this, NMib::fg_Forward<tf_CData>(_Data)); \
	} \
	template <typename tf_CData> constexpr inline_small NMib::NStream::CScopeBinaryStreamVersion f_StreamVersion(tf_CData &&_DefaultVersion) \
	{ \
		auto Version = _DefaultVersion; \
		this->f_Stream(Version); \
		return NMib::NStream::CScopeBinaryStreamVersion(*this, Version); \
	} \
	DMibStreamImplementOperatorsOperators(d_Class)

	template <typename t_CStream, EStreamDirection t_Direction>
	struct TCStreamDirection : public t_CStream
	{
		template <typename tf_CData>
		constexpr inline_small auto f_Feed(tf_CData &&_Data);
		template <typename tf_CData>
		constexpr inline_small auto f_Feed(tf_CData const *_pData);

		template <typename tf_CData>
		constexpr inline_small auto f_Consume(tf_CData &&_Data);
		template <typename tf_CData>
		constexpr inline_small auto f_Consume(tf_CData *_pData);

		template <typename tf_CData>
		constexpr inline_small auto f_Stream(tf_CData &&_Data);
		template <typename tf_CData>
		constexpr inline_small CScopeBinaryStreamVersion f_StreamVersion(tf_CData &&_DefaultVersion);

		DMibStreamImplementOperatorsOperators(TCStreamDirection);

		t_CStream &f_GetStream();
		t_CStream const &f_GetStream() const;

		static constexpr EStreamDirection mc_Direction = t_Direction;
		static constexpr bool mc_bConsume = mc_Direction == EStreamDirection_Consume;
		static constexpr bool mc_bFeed = mc_Direction == EStreamDirection_Feed;
	};

#define DMibStreamDeclare(d_Class, d_Stream, d_Direction) \
	extern template void d_Class::f_Stream<NStream::TCStreamDirection<d_Stream, NStream::EStreamDirection_##d_Direction>> \
		(NStream::TCStreamDirection<d_Stream, NStream::EStreamDirection_##d_Direction> &);
#define DMibStreamImplement(d_Class, d_Stream, d_Direction) \
	template void d_Class::f_Stream<NStream::TCStreamDirection<d_Stream, NStream::EStreamDirection_##d_Direction>> \
		(NStream::TCStreamDirection<d_Stream, NStream::EStreamDirection_##d_Direction> &);

#ifndef DMibPNoShortCuts
#	define DStreamDeclare DMibStreamDeclare
#	define DStreamImplement DMibStreamImplement
#endif

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
			, TCEnableIf<!NTraits::cIsSame<decltype(fg_GetReference<NTraits::TCRemoveReferenceAndQualifiers<t_CData>>().f_Stream(fg_GetReference<t_CStream>())), NPrivate::CDummy>>
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
		struct TCStreamHasStream<t_CStream, t_CData, TCEnableIf<!NTraits::cIsSame<decltype(fg_GetReference<t_CStream>().f_Stream(fg_GetType<t_CData>())), NPrivate::CDummy>>>
		{
			static constexpr bool mc_Value = true;
		};

		template <typename t_CStream, typename t_CData, typename t_CEnableIf = void>
		struct TCStreamHasFeed
		{
			static constexpr bool mc_Value = false;
		};

		template <typename t_CStream, typename t_CData>
		struct TCStreamHasFeed<t_CStream, t_CData, TCEnableIf<!NTraits::cIsSame<decltype(fg_GetReference<t_CStream>().f_Feed(fg_GetType<t_CData>())), NPrivate::CDummy>>>
		{
			static constexpr bool mc_Value = true;
		};

		template <typename t_CStream, typename t_CData, typename t_CEnableIf = void>
		struct TCStreamHasConsume
		{
			static constexpr bool mc_Value = false;
		};

		template <typename t_CStream, typename t_CData>
		struct TCStreamHasConsume<t_CStream, t_CData, TCEnableIf<!NTraits::cIsSame<decltype(fg_GetReference<t_CStream>().f_Consume(fg_GetType<t_CData>())), NPrivate::CDummy>>>
		{
			static constexpr bool mc_Value = true;
		};
	}

	template <typename t_CStream, typename t_CData>
	struct TCBinaryStreamTypeReferenceStream
	{
		// Stream

		template <EStreamDirection tf_Direction, typename tf_CData, NMib::TCEnableIf<NPrivate::TCHasStream<TCStreamDirection<t_CStream, tf_Direction>, tf_CData>::mc_Value> * = nullptr>
		inline_small static auto fs_Stream(TCStreamDirection<t_CStream, tf_Direction> &_Stream, tf_CData &&_Data);
		template <typename tf_CData, NMib::TCEnableIf<!NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Feed>, tf_CData>::mc_Value> * = nullptr>
		inline_small static auto fs_Stream(TCStreamDirection<t_CStream, EStreamDirection_Feed> &_Stream, tf_CData &&_Data);
		template <typename tf_CData, NMib::TCEnableIf<!NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Consume>, tf_CData>::mc_Value> * = nullptr>
		inline_small static auto fs_Stream(TCStreamDirection<t_CStream, EStreamDirection_Consume> &_Stream, tf_CData &&_Data);
	};

	template <EStreamDirection tf_StreamDirection, typename tf_CStream>
	decltype(auto) fg_StreamDirection(tf_CStream &_Stream);
	template <typename tf_CStream>
	decltype(auto) fg_FeedStream(tf_CStream &_Stream);
	template <typename tf_CStream>
	decltype(auto) fg_ConsumeStream(tf_CStream &_Stream);

	template <typename t_CStream, typename t_CData>
	struct TCBinaryStreamTypeReference
	{
		// Feed

		template
		<
			typename tf_CData
			, NMib::TCEnableIf
			<
				!NMib::NTraits::cIsEnum<NTraits::TCRemoveReference<tf_CData>> && !NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Feed>, tf_CData>::mc_Value
			>
			* = nullptr
		>
		static constexpr inline_small auto fs_Feed(t_CStream &_Stream, tf_CData &&_Data);

		template
		<
			typename tf_CData
			, NMib::TCEnableIf
			<
				!NMib::NTraits::cIsEnum<NTraits::TCRemoveReference<tf_CData>> && NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Feed>, tf_CData>::mc_Value
			>
			* = nullptr
		>
		static constexpr inline_small auto fs_Feed(t_CStream &_Stream, tf_CData &&_Data);

		// Consume

		template
		<
			typename tf_CData
			, NMib::TCEnableIf
			<
				!NMib::NTraits::cIsEnum<NTraits::TCRemoveReference<tf_CData>> && !NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Consume>, tf_CData>::mc_Value
			>
			* = nullptr
		>
		static constexpr inline_small auto fs_Consume(t_CStream &_Stream, tf_CData &&_Data);

		template
		<
			typename tf_CData
			, NMib::TCEnableIf
			<
				!NMib::NTraits::cIsEnum<NTraits::TCRemoveReference<tf_CData>> && NPrivate::TCHasStream<TCStreamDirection<t_CStream, EStreamDirection_Consume>, tf_CData>::mc_Value
			>
			* = nullptr
		>
		static constexpr inline_small auto fs_Consume(t_CStream &_Stream, tf_CData &&_Data);

		template <typename tf_CData, NMib::TCEnableIf<NMib::NTraits::cIsEnum<tf_CData>, void> * = nullptr>
		static constexpr inline_small void fs_Feed(t_CStream &_Stream, tf_CData const &_Data);
		template <typename tf_CData, NMib::TCEnableIf<NMib::NTraits::cIsEnum<tf_CData>, void> * = nullptr>
		static constexpr inline_small void fs_Consume(t_CStream &_Stream, tf_CData &_Data);
	};

	template <typename t_CStream, typename t_CData>
	struct TCBinaryStreamTypePtr
	{
		static constexpr void fs_Feed(t_CStream &_Stream, t_CData const *_pData);
		static constexpr void fs_Consume(t_CStream &_Stream, t_CData *_pData);
	};

	template <typename t_CType>
	struct TCBinaryStreamUnsafeWrapper
	{
		using CType = t_CType;

		CType &m_Data;
		TCBinaryStreamUnsafeWrapper(t_CType &_Data);
		TCBinaryStreamUnsafeWrapper &operator = (TCBinaryStreamUnsafeWrapper const &_Other) = delete;
	};

#if defined(DMibDebug) && 0
#	define DMibTempStreamDebug
#	define DMibTempStreamPre virtual
#	define DMibTempStreamPost =0
#else
#	define DMibTempStreamPre inline_small
#	define DMibTempStreamPost
#endif

#ifdef DMibTempStreamDebug

#	define DMibStreamImplementProtected(d_Class) \
		void fp_FeedBytes(const void *_pMem, umint _nBytes){this->f_FeedBytes(_pMem, _nBytes);}\
		void fp_ConsumeBytes(void *_pMem, umint _nBytes){this->f_ConsumeBytes(_pMem, _nBytes);}\
		bool fp_IsValid() const {bool bRet = 0; bRet = this->f_IsValid(); return bRet;}\
		bool fp_IsAtEndOfStream() const {bool bRet = 0; bRet = this->f_IsAtEndOfStream(); return bRet;}\
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

#	define DMibStreamImplementProtected(d_Class) \
		void fp_FeedBytes(const void *_pMem, umint _nBytes){d_Class::f_FeedBytes(_pMem, _nBytes);}\
		void fp_ConsumeBytes(void *_pMem, umint _nBytes){d_Class::f_ConsumeBytes(_pMem, _nBytes);}\
		bool fp_IsValid() const {bool bRet = 0; bRet = d_Class::f_IsValid(); return bRet;}\
		bool fp_IsAtEndOfStream() const {bool bRet = 0; bRet = d_Class::f_IsAtEndOfStream(); return bRet;}\
		NMib::NStream::CFilePos fp_GetPosition() const {return d_Class::f_GetPosition();}\
		void fp_SetPosition(NMib::NStream::CFilePos _Pos){d_Class::f_SetPosition(_Pos);}\
		void fp_SetPositionFromEnd(NMib::NStream::CFilePos _Pos){d_Class::f_SetPositionFromEnd(_Pos);}\
		void fp_AddPosition(NMib::NStream::CFilePos _Pos){d_Class::f_AddPosition(_Pos);}\
		bool fp_IsValidReadPosition(NMib::NStream::CFilePos _Pos) const {bool bRet = 0; bRet = d_Class::f_IsValidReadPosition(_Pos); return bRet; }\
		void fp_Flush(bool _bLocalCacheOnly) {d_Class::f_Flush(_bLocalCacheOnly);}\
		void fp_SetCacheSize(umint _CacheSize) {d_Class::f_SetCacheSize(_CacheSize);}\
		NMib::NStream::CFilePos fp_GetLength() const {NMib::NStream::CFilePos Ret = 0; Ret =d_Class::f_GetLength(); return Ret;}\
		void fp_SetLength(NMib::NStream::CFilePos _Length) {return d_Class::f_SetLength(_Length);}\
		aint fp_LengthSize() const {aint Ret = 0; Ret = d_Class::f_LengthSize(); return Ret;}\
		aint fp_Endian() const {aint Ret = 0; Ret = d_Class::f_Endian(); return Ret;}\
		umint fp_ContainerLengthLimit() const {umint Ret = 0; Ret = d_Class::f_ContainerLengthLimit(); return Ret;}\
		void fp_FeedFromStream(NMib::NStream::CBinaryStream &_Stream, NMib::NStream::CFilePos _nBytes){d_Class::f_FeedFromStream(_Stream, _nBytes);}\

#endif

	struct CScopeBinaryStreamContext;
	struct CScopeBinaryStreamContainerLengthLimit;

	struct CBinaryStream
	{
		constexpr CBinaryStream() = default;
		constexpr virtual ~CBinaryStream() = default;

	private:
		void *mp_pContext = nullptr;
		umint mp_ContainerLengthLimit = 0;
		uint32 mp_Version = 0;

		CBinaryStream(CBinaryStream const &_Other) = delete;
		CBinaryStream &operator = (CBinaryStream const &_Other) = delete;

	protected:
		virtual void fp_FeedBytes(void const *_pMem, umint _nBytes) = 0;
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
		virtual aint fp_LengthSize() const = 0;
		virtual aint fp_Endian() const = 0;
		virtual umint fp_ContainerLengthLimit() const = 0;
		virtual void fp_FeedFromStream(CBinaryStream &_Stream, CFilePos _nBytes) = 0;

		inline_never void fp_ThrowEndOfStreamException();

	public:
		DMibTempStreamPre void f_FeedBytes(void const *_pMem, umint _nBytes) DMibTempStreamPost;
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

		inline_small uint32 f_GetVersion() const;

		template <typename tf_CVersion>
		inline_small bool f_SupportsVersion(tf_CVersion _Version) const
			requires (cIsValidStreamVersion<tf_CVersion>)
		;

		inline_small void *f_GetContext() const;
		inline_small umint f_ClaimContainerLengthLimitOverride();

		DMibStreamImplementOperators(CBinaryStream);

	private:
		friend struct CScopeBinaryStreamVersion;
		friend struct CScopeBinaryStreamContext;
		friend struct CScopeBinaryStreamContainerLengthLimit;
	};

	template <typename tf_CStream>
	void fg_PadAlignStream(tf_CStream &_Stream, umint _Alignment);
	template <typename tf_CStream>
	void fg_AlignStream(tf_CStream &_Stream, umint _Alignment);

#define DMibBinaryStreamVersion(d_Stream, d_Version) NMib::NStream::CScopeBinaryStreamVersion ScopeBinaryStreamVersion(d_Stream, d_Version)

#ifndef DMibPNoShortCuts
#	define DBinaryStreamVersion(d_Stream, d_Version) DMibBinaryStreamVersion(d_Stream, d_Version)
#endif

	struct CScopeBinaryStreamContainerLengthLimit
	{
		CScopeBinaryStreamContainerLengthLimit() = delete;
		CScopeBinaryStreamContainerLengthLimit(CScopeBinaryStreamContainerLengthLimit const &_Other) = delete;
		CScopeBinaryStreamContainerLengthLimit &operator = (CScopeBinaryStreamContainerLengthLimit const &_Other) = delete;

		CScopeBinaryStreamContainerLengthLimit(CBinaryStream &_Stream, uint32 _ContainerLengthLimit);
		void f_SetContainerLengthLimit(CBinaryStream &_Stream, uint32 _ContainerLengthLimit);
		~CScopeBinaryStreamContainerLengthLimit();
		void f_Clear();

	private:
		CBinaryStream *mp_pStream;
		uint32 mp_OldContainerLengthLimit;
	};

#define DMibBinaryStreamContainerLengthLimit(d_Stream, d_ContainerLengthLimit) \
	NMib::NStream::CScopeBinaryStreamContainerLengthLimit ScopeBinaryStreamContainerLengthLimit(d_Stream, d_ContainerLengthLimit)

#ifndef DMibPNoShortCuts
#	define DBinaryStreamContainerLengthLimit(d_Stream, d_ContainerLengthLimit) DMibBinaryStreamContainerLengthLimit(d_Stream, d_ContainerLengthLimit)
#endif

	struct CScopeBinaryStreamContext
	{
		CScopeBinaryStreamContext(CScopeBinaryStreamContext const &_Other) = delete;
		CScopeBinaryStreamContext &operator = (CScopeBinaryStreamContext const &_Other) = delete;
		CScopeBinaryStreamContext() = delete;

		CScopeBinaryStreamContext(CBinaryStream &_Stream, void *_pContext);
		void f_SetContext(CBinaryStream &_Stream, void *_pContext);
		void f_Clear();
		~CScopeBinaryStreamContext();

	private:
		CBinaryStream *mp_pStream;
		void *mp_pOldContext;
	};

#define DMibBinaryStreamContext(d_Stream, d_Context) NMib::NStream::CScopeBinaryStreamContext ScopeBinaryStreamContext(d_Stream, d_Context)

#ifndef DMibPNoShortCuts
#	define DBinaryStreamContext(d_Stream, d_Context) DMibBinaryStreamContext(d_Stream, d_Context)
#endif

	struct CBinaryStreamDefault : public CBinaryStream
	{
		DMibStreamImplementOperators(CBinaryStreamDefault);

		constexpr CBinaryStreamDefault() = default;

		constexpr inline_small aint f_LengthSize() const;
		constexpr inline_small aint f_Endian() const;
		constexpr inline_small void f_FeedFromStream(CBinaryStream &_Stream, CFilePos _nBytes);

	private:
		void fp_FeedFromStreamImplementation(CBinaryStream &_Stream, CFilePos _nBytes);

	private:
		CBinaryStreamDefault(CBinaryStreamDefault const &_Other) = delete;
		CBinaryStreamDefault &operator = (CBinaryStreamDefault const &_Other) = delete;

	protected:
		DMibStreamImplementProtected(CBinaryStreamDefault);
	};

	struct CBinaryStreamDefaultRef : public CBinaryStreamDefault
	{
		NStorage::CIntrusiveRefCount m_RefCount;

	private:
		CBinaryStreamDefaultRef(CBinaryStreamDefaultRef const &_Other) = delete;
		CBinaryStreamDefaultRef &operator = (CBinaryStreamDefaultRef const &_Other) = delete;

	protected:
		DMibStreamImplementProtected(CBinaryStreamDefaultRef);

	public:
		DMibStreamImplementOperators(CBinaryStreamDefaultRef);

		CBinaryStreamDefaultRef();
	};

	struct CBinaryStreamBigEndian : public CBinaryStreamDefault
	{
		DMibStreamImplementOperators(CBinaryStreamBigEndian);

		CBinaryStreamBigEndian();
		inline_small aint f_Endian() const;

	private:
		CBinaryStreamBigEndian(CBinaryStreamBigEndian const &_Other) = delete;
		CBinaryStreamBigEndian &operator = (CBinaryStreamBigEndian const &_Other) = delete;

	protected:
		DMibStreamImplementProtected(CBinaryStreamBigEndian);
	};

	struct CBinaryStreamLittleEndian : public CBinaryStreamDefault
	{
		DMibStreamImplementOperators(CBinaryStreamLittleEndian);

		CBinaryStreamLittleEndian();
		inline_small aint f_Endian() const;

	private:
		CBinaryStreamLittleEndian(CBinaryStreamLittleEndian const &_Other) = delete;
		CBinaryStreamLittleEndian &operator = (CBinaryStreamLittleEndian const &_Other) = delete;

	protected:
		DMibStreamImplementProtected(CBinaryStreamLittleEndian);
	};

	struct CBinaryStreamNativeEndian : public CBinaryStreamDefault
	{
		DMibStreamImplementOperators(CBinaryStreamNativeEndian);

		CBinaryStreamNativeEndian();
		inline_small aint f_Endian() const;

	private:
		CBinaryStreamNativeEndian(CBinaryStreamNativeEndian const &_Other) = delete;
		CBinaryStreamNativeEndian &operator = (CBinaryStreamNativeEndian const &_Other) = delete;

	protected:
		DMibStreamImplementProtected(CBinaryStreamNativeEndian);
	};

	template <typename t_CStreamType = NStream::CBinaryStreamDefault>
	struct TCBinaryStreamNull : public t_CStreamType
	{
		DMibStreamImplementOperators(TCBinaryStreamNull);

		constexpr TCBinaryStreamNull() = default;

		constexpr void f_Reset();
		constexpr void f_Clear();
		constexpr void f_FeedBytes(void const *_pMem, umint _nBytes);
		constexpr void f_ConsumeBytes(void *_pMem, umint _nBytes);
		constexpr bool f_IsValid() const;
		constexpr bool f_IsAtEndOfStream() const;
		constexpr CFilePos f_GetPosition() const;
		constexpr void f_SetPosition(CFilePos _Pos);
		constexpr void f_SetPositionFromEnd(CFilePos _Pos);
		constexpr void f_AddPosition(CFilePos _Pos);
		constexpr bool f_IsValidReadPosition(NStream::CFilePos _Pos) const;
		constexpr void f_Flush(bool _bLocalCacheOnly);
		constexpr void f_SetCacheSize(umint _CacheSize);
		constexpr CFilePos f_GetLength() const;
		constexpr umint f_ContainerLengthLimit() const;
		constexpr void f_SetLength(NStream::CFilePos _Length);

	private:
		TCBinaryStreamNull(TCBinaryStreamNull const &_Other) = delete;
		TCBinaryStreamNull &operator = (TCBinaryStreamNull const &_Other) = delete;

	protected:
		umint mp_Position = 0;
		umint mp_Length = 0;

		constexpr void fp_SetPositionInternal(CFilePos _Pos);
		DMibStreamImplementProtected(TCBinaryStreamNull);
	};

	// Default types

	template <typename tf_CType>
	CFilePos fg_GetBinaryStreamSize(tf_CType const &_Value);

	template <typename tf_CStream>
	void fg_FeedLenToStream(tf_CStream &_Stream, uint64 _Len);

	inline_always umint fg_CapLengthLimit(NStream::CFilePos const &_Len);
	void fg_ReportLengthLimitError(uint64 _Len, uint64 _LengthLimit);
	template <typename tf_CStream, typename tf_CLen>
	void fg_CheckLengthLimit(tf_CStream &_Stream, tf_CLen const &_Len);

	template <typename tf_CStream>
	void fg_ConsumeLenFromStream(tf_CStream &_Stream, uint64 &_Len);

	template <typename tf_CStream, typename tf_CData>
	void fg_FeedEndianArrayToStream(tf_CStream &_Stream, tf_CData const *_pData, umint _Len, aint _Endian);
	template <typename tf_CStream, typename tf_CData>
	void fg_ByteSwapArray(tf_CStream &_Stream, tf_CData *_pData, umint _Len, aint _Endian);

	static void fg_StrDecodeLenType(uint64 &_Len, uint32 _StreamSize, NStr::EStrType &_Type);
	static void fg_StrEncodeLenType(uint64 &_Len, uint32 _StreamSize, NStr::EStrType _Type);

	template <typename t_CStream>
	struct TCBinaryStreamTypePtr<t_CStream, ch8>
	{
		static constexpr void fs_Feed(t_CStream &_Stream, ch8 const *_pData);
		static constexpr void fs_Consume(t_CStream &_Stream, ch8 *_pData);
	};

	template <typename t_CStream>
	struct TCBinaryStreamTypePtr<t_CStream, ch16>
	{
		static constexpr void fs_Feed(t_CStream &_Stream, ch16 const *_pData);
		static constexpr void fs_Consume(t_CStream &_Stream, ch16 *_pData);
	};

	template <typename t_CStream>
	struct TCBinaryStreamTypePtr<t_CStream, ch32>
	{
		static constexpr void fs_Feed(t_CStream &_Stream, ch32 const *_pData);
		static constexpr void fs_Consume(t_CStream &_Stream, ch32 *_pData);
	};

	template <typename t_CStream, typename t_CType>
	struct TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType const>>
	{
		static constexpr void fs_Feed(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType const> const &_Data);
	};

	template <typename t_CStream, typename t_CType>
	struct TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> const>
	{
		static void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> const &_Data);
	};

	template <typename t_CStream, typename t_CType>
	struct TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType>>
	{
		static constexpr void fs_Feed(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> const &_Data);
		static constexpr void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> &_Data);
		static constexpr void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<t_CType> &&_Data);
	};

	template <typename tf_CType>
	inline_small TCBinaryStreamUnsafeWrapper<tf_CType> fg_GetUnsafeStreamWrapper(tf_CType &_Type);
	template <typename tf_CType>
	inline_small TCBinaryStreamUnsafeWrapper<tf_CType const> fg_GetUnsafeStreamWrapper(tf_CType const &_Type);

#define DMibStreamImplementSimpleEndianSwappedType(d_Type) \
	template <typename t_CStream> \
	struct TCBinaryStreamTypeReference<t_CStream, d_Type> \
	{ \
		static constexpr void fs_Feed(t_CStream &_Stream, d_Type const &_Data) \
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
		static constexpr void fs_Consume(t_CStream &_Stream, d_Type &_Data)\
		{\
			aint Endian = _Stream.f_Endian();\
			if (Endian == gc_MachineEndian || Endian == EEndian_Native)\
				_Stream.f_ConsumeBytes(&_Data, sizeof(_Data));\
			else\
			{\
				NTraits::TCIntFromSize<sizeof(_Data)> Data;\
				_Stream.f_ConsumeBytes(&Data, sizeof(Data));\
				_Data = fg_ByteSwap(reinterpret_cast<d_Type &>(Data));\
			}\
		}\
	};

#define DMibStreamImplementSimpleEndianSwappedTypeUnsafe(d_Type) \
	template <typename t_CStream> \
	struct TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<const d_Type> > \
	{ \
		static constexpr void fs_Feed(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<const d_Type> const &_Data) \
		{ \
			aint Endian = _Stream.f_Endian();\
			if (Endian == gc_MachineEndian || Endian == EEndian_Native)\
				_Stream.f_FeedBytes(&_Data.m_Data, sizeof(_Data.m_Data));\
			else\
			{\
				d_Type Temp = fg_ByteSwap(_Data.m_Data);\
				_Stream.f_FeedBytes(&Temp, sizeof(_Data.m_Data));\
			} \
		}\
	};\
	template <typename t_CStream> \
	struct TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<d_Type> const > \
	{ \
		static constexpr void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<d_Type> const &_Data)\
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
	struct TCBinaryStreamTypeReference<t_CStream, NMib::NStream::TCBinaryStreamUnsafeWrapper<d_Type> > \
	{ \
		static constexpr void fs_Feed(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<d_Type> const &_Data) \
		{ \
			aint Endian = _Stream.f_Endian();\
			if (Endian == gc_MachineEndian || Endian == EEndian_Native)\
				_Stream.f_FeedBytes(&_Data.m_Data, sizeof(_Data.m_Data));\
			else\
			{\
				d_Type Temp = fg_ByteSwap(_Data.m_Data);\
				_Stream.f_FeedBytes(&Temp, sizeof(_Data.m_Data));\
			} \
		}\
		static constexpr void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<d_Type> &_Data)\
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
		static constexpr void fs_Consume(t_CStream &_Stream, NMib::NStream::TCBinaryStreamUnsafeWrapper<d_Type> &&_Data)\
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

#define DMibStreamImplementSimpleType(d_Type) \
	template <typename t_CStream> \
	struct TCBinaryStreamTypeReference<t_CStream, d_Type> \
	{ \
		static constexpr void fs_Feed(t_CStream &_Stream, d_Type const &_Data) \
		{ \
			_Stream.f_FeedBytes(&_Data, sizeof(_Data));\
		}\
		static constexpr void fs_Consume(t_CStream &_Stream, d_Type &_Data)\
		{\
			_Stream.f_ConsumeBytes(&_Data, sizeof(_Data));\
		}\
	};

	template <typename t_CStream>
	struct TCBinaryStreamTypeReference<t_CStream, bool>
	{
		static constexpr void fs_Feed(t_CStream &_Stream, bool const &_bData);
		static constexpr void fs_Consume(t_CStream &_Stream, bool &_bData);
	};

	DMibStreamImplementSimpleType(int8);
	DMibStreamImplementSimpleEndianSwappedType(int16);
	DMibStreamImplementSimpleEndianSwappedType(int32);
	DMibStreamImplementSimpleEndianSwappedType(int64);
	DMibStreamImplementSimpleType(uint8);
	DMibStreamImplementSimpleEndianSwappedType(uint16);
	DMibStreamImplementSimpleEndianSwappedType(uint32);
	DMibStreamImplementSimpleEndianSwappedType(uint64);

#ifdef DMibPUniqueType_umint
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
	struct TCBinaryStreamTypeReference<t_CStream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>>
	{
		static constexpr void fs_Feed(t_CStream &_Stream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> const &_Data);
		static constexpr void fs_Feed(t_CStream &_Stream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &&_Data);
		static constexpr void fs_Consume(t_CStream &_Stream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &_Data);
	};

	template <typename t_CStream, typename t_CData, typename t_CTranslator, typename t_CLink, typename t_CLinkInList, bool t_bAutoDelete, typename t_CAllocator>
	struct TCBinaryStreamTypeReference<t_CStream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator>>
	{
		static constexpr void fs_Feed(t_CStream &_Stream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> const &_Data);
		static constexpr void fs_Feed(t_CStream &_Stream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &&_Data);
		static constexpr void fs_Consume(t_CStream &_Stream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &_Data);
	};

	template <typename t_CStream, typename t_CType, typename ...tp_COptions>
	struct TCBinaryStreamTypeReference<t_CStream, NStorage::TCUniquePointer<t_CType, tp_COptions...>>
	{
		static constexpr void fs_Feed(t_CStream &_Stream, NStorage::TCUniquePointer<t_CType, tp_COptions...> const &_pData);
		static constexpr void fs_Feed(t_CStream &_Stream, NStorage::TCUniquePointer<t_CType, tp_COptions...> &&_pData);
		static constexpr void fs_Consume(t_CStream &_Stream, NStorage::TCUniquePointer<t_CType, tp_COptions...> &_pData);
	};

	template <typename t_CStream, typename t_CType, typename ...tp_COptions>
	struct TCBinaryStreamTypeReference<t_CStream, NStorage::TCSharedPointer<t_CType, tp_COptions...>>
	{
		static constexpr void fs_Feed(t_CStream &_Stream, NStorage::TCSharedPointer<t_CType, tp_COptions...> const &_pData);
		static constexpr void fs_Feed(t_CStream &_Stream, NStorage::TCSharedPointer<t_CType, tp_COptions...> &&_pData);
		static constexpr void fs_Consume(t_CStream &_Stream, NStorage::TCSharedPointer<t_CType, tp_COptions...> &_pData);
	};
}

#ifdef DMibIncluded_IntusiveAVLTree
#	include "../../Intrusive/Source/Malterlib_Intrusive_AVLTree_Stream.h"
#endif

#ifndef DMibPNoShortCuts
using namespace NMib::NStream;
#endif

#include "Malterlib_Stream.hpp"
