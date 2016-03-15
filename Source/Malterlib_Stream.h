// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>
//#include <boost/type_traits.hpp>
#include <Mib/Storage/Indirection>

#define DMibIncluded_Stream

namespace NMib
{

	namespace NStream
	{
		DMibImpErrorClass(CExceptionStream, NMib::NException::CException);
		DMibImpErrorClass(CExceptionStreamVersionMismatch, CExceptionStream);

#		define DMibErrorStream(_Description) DMibImpError(NMib::NStream::CExceptionStream, _Description)

#		ifndef DMibPNoShortCuts
#			define DErrorStream(_Description) DMibErrorStream(_Description)
#		endif

#		define DMibErrorStreamVersionMismatch(_Description) DMibImpError(NMib::NStream::CExceptionStreamVersionMismatch, _Description)

#		ifndef DMibPNoShortCuts
#			define DErrorStreamVersionMismatch(_Description) DMibErrorStreamVersionMismatch(_Description)
#		endif

		typedef CMibFilePos CFilePos;

		template <typename t_CStream, typename t_CData>
		class TCBinaryStreamTypeReference
		{
		public:
			template <typename t_CData2>
			inline_small static typename NMib::TCDisableIf<NMib::NTraits::TCIsEnum<t_CData2>::mc_Value, void>::CType fs_Feed(t_CStream &_Stream, t_CData2 const &_Data)
			{
				_Data.f_Feed(_Stream);
			}
			template <typename t_CData2>
			inline_small static typename NMib::TCDisableIf<NMib::NTraits::TCIsEnum<t_CData2>::mc_Value, void>::CType fs_Consume(t_CStream &_Stream, t_CData2 &_Data)
			{
				_Data.f_Consume(_Stream);
			}

			template <typename t_CData2>
			inline_small static typename NMib::TCEnableIf<NMib::NTraits::TCIsEnum<t_CData2>::mc_Value, void>::CType fs_Feed(t_CStream &_Stream, t_CData2 const &_Data)
			{
				_Stream << uint32(_Data);
			}
			template <typename t_CData2>
			inline_small static typename NMib::TCEnableIf<NMib::NTraits::TCIsEnum<t_CData2>::mc_Value, void>::CType fs_Consume(t_CStream &_Stream, t_CData2 &_Data)
			{
				uint32 Temp;
				_Stream >> Temp;
				_Data = static_cast<t_CData2>(Temp);
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

#		define DMibStreamImplementOperators(_Class) \
			template <typename t_CData>	inline_small void f_Feed(t_CData &&_Data){NMib::NStream::TCBinaryStreamTypeReference<_Class, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<t_CData>::CType>::CType>::fs_Feed(*this, NMib::fg_Forward<t_CData>(_Data));} \
			template <typename t_CData> inline_small void f_Feed(const t_CData *_pData){NMib::NStream::TCBinaryStreamTypePtr<_Class, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<t_CData>::CType>::CType>::fs_Feed(*this, _pData);} \
			template <typename t_CData>	inline_small void f_Consume(t_CData &&_Data){NMib::NStream::TCBinaryStreamTypeReference<_Class, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<t_CData>::CType>::CType>::fs_Consume(*this, NMib::fg_Forward<t_CData>(_Data));} \
			template <typename t_CData> inline_small void f_Consume(t_CData *_pData){NMib::NStream::TCBinaryStreamTypePtr<_Class, t_CData>::fs_Consume(*this, _pData);}

		
#		define DMibStreamImplementProtected(_Class) \
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
			void fp_FeedFromStream(NMib::NStream::CBinaryStream &_Stream, NMib::NStream::CFilePos _nBytes){_Class::f_FeedFromStream(_Stream, _nBytes);}\

#		if defined(DMibDebug) && 0
#			define DMibTempStreamDebug
#			define DMibTempStreamPre virtual
#			define DMibTempStreamPost =0
#		else
#			define DMibTempStreamPre inline_small
#			define DMibTempStreamPost 
#		endif

		class CScopeBinaryStreamVersion;

		namespace NPrivate
		{
			struct CDummy
			{
			};
		}
		
		class CBinaryStream
		{
			friend class CScopeBinaryStreamVersion;
			friend class CScopeBinaryStreamContext;
		public:
			class CVersionEntry
			{
			public:
				DMibListLinkS_Link(CVersionEntry, m_Link);
				uint32 m_Version;
			};
			class CContextEntry
			{
			public:
				DMibListLinkS_Link(CContextEntry, m_Link);
				void *m_pContext;
			};
			CBinaryStream()
			{
			}
		private:

			DMibListLinkS_ListNoLastPtr(CVersionEntry, m_Link) m_VersionStack;
			DMibListLinkS_ListNoLastPtr(CContextEntry, m_Link) m_ContextStack;

			DMibClassNoCopyAllowed(CBinaryStream);

		protected:
			virtual void fp_FeedBytes(const void *_pMem, mint _nBytes) pure;
			virtual void fp_ConsumeBytes(void *_pMem, mint _nBytes) pure;
			virtual bint fp_IsValid() const pure;
			virtual bint fp_IsAtEndOfStream() const pure;
			virtual CFilePos fp_GetPosition() const pure;
			virtual void fp_SetPosition(CFilePos _Pos) pure;
			virtual void fp_SetPositionFromEnd(CFilePos _Pos) pure;
			virtual void fp_AddPosition(CFilePos _Pos) pure;
			virtual bint fp_IsValidReadPosition(NStream::CFilePos _Pos) const pure;
			virtual void fp_Flush(bint _bLocalCacheOnly) pure;
			virtual void fp_SetCacheSize(mint _CacheSize) pure;
			virtual CFilePos fp_GetLength() const pure;
			virtual void fp_SetLength(CFilePos _Length) pure;
			virtual	aint fp_LengthSize() const pure;
			virtual aint fp_Endian() const pure;
			virtual void fp_FeedFromStream(CBinaryStream &_Stream, CFilePos _nBytes) pure;

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

			DMibTempStreamPre void f_FeedFromStream(CBinaryStream &_Stream, CFilePos _nBytes) DMibTempStreamPost
			{
				fp_FeedFromStream(_Stream, _nBytes);
			}

#endif
			inline_small uint32 f_GetVersion()
			{
				CVersionEntry *pVersion = m_VersionStack.f_GetFirst();
				if (pVersion)
					return pVersion->m_Version;
				else
					DMibError("No version specified for stream.");
			}
			inline_small void *f_GetContext()
			{
				CContextEntry *pContext = m_ContextStack.f_GetFirst();
				if (pContext)
					return pContext->m_pContext;
				else
					return nullptr;
			}


			DMibStreamImplementOperators(CBinaryStream);
			
			template <typename tf_CData> 
			inline_small auto operator << (tf_CData &&_Data)
			-> typename NMib::TCEnableIf
				<
					!NMib::NIndirection::TCIsIndirection<tf_CData>::mc_Value
					&& !NTraits::TCIsSame<decltype(NMib::NStream::TCBinaryStreamTypeReference<CBinaryStream, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Feed(*this, NMib::fg_Forward<tf_CData>(_Data))), NPrivate::CDummy>::mc_Value
					, CBinaryStream &
				>::CType
			{
				NMib::NStream::TCBinaryStreamTypeReference<CBinaryStream, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Feed(*this, NMib::fg_Forward<tf_CData>(_Data));
				return *this;
			}

			template <typename tf_CData> 
			inline_small auto operator >> (tf_CData &&_Data)
			-> typename NMib::TCEnableIf
				<
					!NMib::NIndirection::TCIsIndirection<tf_CData>::mc_Value
					&& !NTraits::TCIsSame<decltype(NMib::NStream::TCBinaryStreamTypeReference<CBinaryStream, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Consume(*this, NMib::fg_Forward<tf_CData>(_Data))), NPrivate::CDummy>::mc_Value
					, CBinaryStream &
				>::CType
			{
				NMib::NStream::TCBinaryStreamTypeReference<CBinaryStream, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Consume(*this, NMib::fg_Forward<tf_CData>(_Data));
				return *this;
			}
		
			template <typename tf_CData> 
			inline_small auto operator << (const tf_CData *_pData) 
			-> typename NMib::TCEnableIf
				<
					true
					&& !NTraits::TCIsSame<decltype(NMib::NStream::TCBinaryStreamTypePtr<CBinaryStream, tf_CData>::fs_Feed(*this, _pData)), NPrivate::CDummy>::mc_Value
					, CBinaryStream &
				>::CType
			{
				NMib::NStream::TCBinaryStreamTypePtr<CBinaryStream, tf_CData>::fs_Feed(*this, _pData);
				return *this;
			}
		
			template <typename tf_CData> 
			inline_small auto operator >> (tf_CData *_pData)
			-> typename NMib::TCEnableIf
				<
					true
					&& !NTraits::TCIsSame<decltype(NMib::NStream::TCBinaryStreamTypePtr<CBinaryStream, tf_CData>::fs_Consume(*this, _pData)), NPrivate::CDummy>::mc_Value
					, CBinaryStream &
				>::CType
			{
				NMib::NStream::TCBinaryStreamTypePtr<CBinaryStream, tf_CData>::fs_Consume(*this, _pData);
				return *this;
			}
		};

		template <typename tf_CStream, typename tf_CData> 
		inline_small auto operator << (tf_CStream &_Stream, tf_CData &&_Data)
		-> typename NMib::TCEnableIf
			<
				NTraits::TCIsBaseOf<tf_CStream, CBinaryStream>::mc_Value 
				&& !NMib::NIndirection::TCIsIndirection<tf_CData>::mc_Value
				&& !NTraits::TCIsSame<decltype(NMib::NStream::TCBinaryStreamTypeReference<tf_CStream, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Feed(_Stream, NMib::fg_Forward<tf_CData>(_Data))), NPrivate::CDummy>::mc_Value
				, tf_CStream &
			>::CType
		{
			NMib::NStream::TCBinaryStreamTypeReference<tf_CStream, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Feed(_Stream, NMib::fg_Forward<tf_CData>(_Data));
			return _Stream;
		}

		template <typename tf_CStream, typename tf_CData> 
		inline_small auto operator >> (tf_CStream &_Stream, tf_CData &&_Data)
		-> typename NMib::TCEnableIf
			<
				NTraits::TCIsBaseOf<tf_CStream, CBinaryStream>::mc_Value 
				&& !NMib::NIndirection::TCIsIndirection<tf_CData>::mc_Value
				&& !NTraits::TCIsSame<decltype(NMib::NStream::TCBinaryStreamTypeReference<tf_CStream, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Consume(_Stream, NMib::fg_Forward<tf_CData>(_Data))), NPrivate::CDummy>::mc_Value
				, tf_CStream &
			>::CType
		{
			NMib::NStream::TCBinaryStreamTypeReference<tf_CStream, typename NMib::NTraits::TCRemoveQualifiers<typename NMib::NTraits::TCRemoveReference<tf_CData>::CType>::CType>::fs_Consume(_Stream, NMib::fg_Forward<tf_CData>(_Data));
			return _Stream;
		}
	
		template <typename tf_CStream, typename tf_CData> 
		inline_small auto operator << (tf_CStream &_Stream, const tf_CData *_pData) 
		-> typename NMib::TCEnableIf
			<
				NTraits::TCIsBaseOf<tf_CStream, CBinaryStream>::mc_Value
				, tf_CStream &
			>::CType
		{
			NMib::NStream::TCBinaryStreamTypePtr<tf_CStream, tf_CData>::fs_Feed(_Stream, _pData);
			return _Stream;
		}
	
		template <typename tf_CStream, typename tf_CData> 
		inline_small auto operator >> (tf_CStream &_Stream, tf_CData *_pData)
		-> typename NMib::TCEnableIf
			<
				NTraits::TCIsBaseOf<tf_CStream, CBinaryStream>::mc_Value
				, tf_CStream &
			>::CType
		{
			NMib::NStream::TCBinaryStreamTypePtr<tf_CStream, tf_CData>::fs_Consume(_Stream, _pData);
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

		class CScopeBinaryStreamVersion : CBinaryStream::CVersionEntry
		{
		private:
			CScopeBinaryStreamVersion(const CScopeBinaryStreamVersion &_Other) : m_pStream(_Other.m_pStream)
			{
			}
			CScopeBinaryStreamVersion &operator = (const CScopeBinaryStreamVersion &_Other)
			{
				return *this;
			}
		public:
			CBinaryStream *m_pStream;
			CScopeBinaryStreamVersion() : 
			m_pStream(0)
			{
			}
			CScopeBinaryStreamVersion(CBinaryStream &_Stream, uint32 _Version) : 
			m_pStream(&_Stream)				
			{
				m_Version = _Version;
				m_pStream->m_VersionStack.f_Push(this);
			}
			void f_SetVersion(CBinaryStream &_Stream, uint32 _Version)
			{
				f_Clear();
				m_pStream = &_Stream;
				m_Version = _Version;
			}
			~CScopeBinaryStreamVersion()
			{
				f_Clear();
			}
			void f_Clear()
			{
				if (m_pStream)
				{
#					if DMibEnableSafeCheck > 0
						CBinaryStream::CVersionEntry *pEntry = m_pStream->m_VersionStack.f_Pop();
						DMibSafeCheck(pEntry == this, "Must be");
#					else
						m_pStream->m_VersionStack.f_Pop();
#					endif
				}
				m_pStream = nullptr;
			}
		};

#		define DMibBinaryStreamVersion(_Stream, _Version) NMib::NStream::CScopeBinaryStreamVersion ScopeBinaryStreamVersion(_Stream, _Version)

#		ifndef DMibPNoShortCuts
#			define DBinaryStreamVersion(_Stream, _Version) DMibBinaryStreamVersion(_Stream, _Version)
#		endif // DMibPNoShortCuts


		class CScopeBinaryStreamContext : CBinaryStream::CContextEntry
		{
		private:
			CScopeBinaryStreamContext(const CScopeBinaryStreamContext &_Other) : m_pStream(_Other.m_pStream)
			{
			}
			CScopeBinaryStreamContext &operator = (const CScopeBinaryStreamContext &_Other)
			{
				return *this;
			}
		public:
			CBinaryStream *m_pStream;
			CScopeBinaryStreamContext():
			m_pStream(0)
			{
			}
			CScopeBinaryStreamContext(CBinaryStream &_Stream, void *_pContext) : 
			m_pStream(&_Stream)
			{
				m_pContext = _pContext;
				m_pStream->m_ContextStack.f_Push(this);
			}
			void f_SetContext(CBinaryStream &_Stream, void *_pContext)
			{
				f_Clear();
				m_pStream = &_Stream;
				m_pContext = _pContext;
				m_pStream->m_ContextStack.f_Push(this);
			}
			void f_Clear()
			{
				if (m_pStream)
				{
#					if DMibEnableSafeCheck > 0
						CBinaryStream::CContextEntry *pEntry = m_pStream->m_ContextStack.f_Pop();
						DMibSafeCheck(pEntry == this, "Must be");
#					else
						m_pStream->m_ContextStack.f_Pop();
#					endif
					m_pStream = nullptr;
				}
			}
			~CScopeBinaryStreamContext()
			{
				f_Clear();
			}
		};

#		define DMibBinaryStreamContext(_Stream, _Context) NMib::NStream::CScopeBinaryStreamContext ScopeBinaryStreamContext(_Stream, _Context)

#		ifndef DMibPNoShortCuts
#			define DBinaryStreamContext(_Stream, _Context) DMibBinaryStreamContext(_Stream, _Context)
#		endif // DMibPNoShortCuts

#undef DMibTempStreamDebug
#undef DMibTempStreamPre
#undef DMibTempStreamPost

		class CBinaryStreamDefault : public CBinaryStream
		{
		private:
			DMibClassNoCopyAllowed(CBinaryStreamDefault);
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

		class CBinaryStreamDefaultRef : public CBinaryStreamDefault, public NPtr::TCSharedPointerIntrusiveBase<>
		{
		private:
			DMibClassNoCopyAllowed(CBinaryStreamDefaultRef);
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
			DMibClassNoCopyAllowed(CBinaryStreamBigEndian);
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
			DMibClassNoCopyAllowed(CBinaryStreamLittleEndian);
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
			DMibClassNoCopyAllowed(CBinaryStreamNativeEndian);
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
			DMibClassNoCopyAllowed(TCBinaryStreamNull);
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


#		define DMibStreamImplementSimpleTypeDefault(_Type) \
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

#		define DMibStreamImplementSimpleEndianSwappedType(_Type) DMibStreamImplementSimpleTypeDefault(_Type) \
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
					_Type Temp = fg_ByteSwap(_Data);\
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
					_Stream.f_ConsumeBytes(&_Data, sizeof(_Data));\
					_Data = fg_ByteSwap(_Data);\
				}\
			}\
		};

#		define DMibStreamImplementSimpleEndianSwappedTypeUnsafe(_Type) DMibStreamImplementSimpleTypeDefault(_Type) \
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


#		define DMibStreamImplementSimpleType(_Type) DMibStreamImplementSimpleTypeDefault(_Type) \
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

			static void fs_Consume(t_CStream &_Stream, NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &_Data)
			{
				uint64 nItems;
				fg_ConsumeLenFromStream(_Stream, nItems);

				while(nItems)
				{
					t_CData *pNewItem = new(t_CAllocator::f_Alloc(sizeof(t_CData))) t_CData();
					auto Cleanup = g_OnScopeExit > [&]
						{
							pNewItem->~CNode();
							t_CAllocator::f_Free(pNewItem);
						}
					;
					_Data.f_Insert(pNewItem);
					_Stream.f_Consume(*pNewItem);
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
				_Stream << (NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &)_Data;
			}

			static void fs_Consume(t_CStream &_Stream, NIntrusive::TCDLinkList<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &_Data)
			{
				_Stream >> (NIntrusive::TCDLinkListAggregate<t_CData, t_CTranslator, t_CLink, t_CLinkInList, t_bAutoDelete, t_CAllocator> &)_Data;
			}
		};


		template <typename t_CStream, typename t_CType, typename t_CAllocator, typename t_CPtr>		
		class TCBinaryStreamTypeReference<t_CStream, NIndirection::TCIndirection<t_CType, t_CAllocator, t_CPtr>>
		{

		public:

			static void fs_Feed(t_CStream &_Stream, NIndirection::TCIndirection<t_CType, t_CAllocator, t_CPtr> const &_Data)
			{
				_Stream << _Data.f_Get();
			}
	
			static void fs_Consume(t_CStream &_Stream, NIndirection::TCIndirection<t_CType, t_CAllocator, t_CPtr> &_Data)
			{
				_Stream >> _Data.f_Get();
			}
		};


	}
}

#ifdef DMibIncluded_IntusiveAVLTree
#	include "../../Intrusive/Source/Malterlib_Intrusive_AVLTree_Stream.h"
#endif
