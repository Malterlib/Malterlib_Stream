// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Stream/BinaryStorage>
#include <Mib/Stream/Streams/Vector>
#include <Mib/Test/Exception>

namespace
{
	using namespace NMib;
	using namespace NMib::NContainer;
	using namespace NMib::NStorage;
	using namespace NMib::NStream;
	using namespace NMib::NStr;

	CIOByteVector fg_MakePattern(umint _nBytes, uint8 _Seed)
	{
		CIOByteVector Vector;
		Vector.f_SetLen(_nBytes);

		uint8 *pData = Vector.f_GetArray();
		for (umint i = 0; i < _nBytes; ++i)
			pData[i] = uint8(_Seed + i);

		return Vector;
	}

	class CBinaryStorage_Tests : public NMib::NTest::CTest
	{
	public:
		void f_DoTests()
		{
			DMibTestSuite("LocalWriteMerging")
			{
				CBinaryStorage Storage;

				uint8 Data0[3] = {1, 2, 3};
				uint8 Data1[2] = {4, 5};
				Storage.f_AppendBytes(Data0, 3);
				Storage.f_AppendBytes(Data1, 2);
				Storage.f_AppendBytes(nullptr, 0);

				DMibExpect(Storage.f_GetTotalLength(), ==, umint(5));
				DMibExpect(Storage.f_GetSegmentCount(), ==, umint(1));
				DMibExpect(Storage.f_GetSegmentCount() <= 1, ==, true);
				DMibExpect(Storage.f_GetSpanCount(), ==, umint(1));

				uint8 ReadBack[5];
				Storage.f_CopyTo(ReadBack, 0, 5);
				for (umint i = 0; i < 5; ++i)
				{
					DMibTestPath("Byte {}"_f << i);
					DMibExpect(ReadBack[i], ==, uint8(i + 1));
				}
			};

			DMibTestSuite("AdoptThreshold")
			{
				{
					DMibTestPath("SmallVectorCopiesIntoArena");
					CBinaryStorage Storage;
					Storage.f_AppendVector(fg_MakePattern(CBinaryStorage::mc_AdoptThreshold - 1, 10));

					DMibExpect(Storage.f_GetSegmentCount(), ==, umint(1));
					DMibExpect(Storage.f_GetSegmentCount() <= 1, ==, true);
					DMibExpect(Storage.f_GetArenaLength(), ==, CBinaryStorage::mc_AdoptThreshold - 1);
				}

				{
					DMibTestPath("LargeVectorBecomesSegment");
					CBinaryStorage Storage;
					Storage.f_AppendVector(fg_MakePattern(CBinaryStorage::mc_AdoptThreshold, 10));

					DMibExpect(Storage.f_GetSegmentCount(), ==, umint(1));
					DMibExpect(Storage.f_GetSegmentCount() <= 1, ==, true);
					DMibExpect(Storage.f_GetArenaLength(), ==, umint(0));
					DMibExpect(Storage.f_GetTotalLength(), ==, CBinaryStorage::mc_AdoptThreshold);
				}

				{
					DMibTestPath("SharedBecomesSegment");
					CBinaryStorage Storage;
					CSharedByteVector Shared(fg_MakePattern(256, 20));
					CSharedByteVector SharedCopy = Shared;
					Storage.f_AppendShared(fg_Move(SharedCopy));

					DMibExpect(Storage.f_GetTotalLength(), ==, umint(256));
					DMibExpect(Storage.f_GetArenaLength(), ==, umint(0));
					DMibExpect(NContainer::CSharedByteVector(Storage.f_Flatten()) == Shared, ==, true);
				}
			};

			DMibTestSuite("SegmentFreezeAfterAdoption")
			{
				CBinaryStorage Storage;

				uint8 Data0[4] = {1, 2, 3, 4};
				Storage.f_AppendBytes(Data0, 4);
				Storage.f_AppendVector(fg_MakePattern(200, 50));
				Storage.f_AppendBytes(Data0, 4);

				// The local write after the adopted segment starts a new local reference even
				// though both ranges are adjacent in the arena
				DMibExpect(Storage.f_GetSegmentCount(), ==, umint(3));
				DMibExpect(Storage.f_GetTotalLength(), ==, umint(208));
				DMibExpect(Storage.f_GetSpanCount(), ==, umint(3));
			};

			DMibTestSuite("NestedStorage")
			{
				CBinaryStorage Payload;
				Payload.f_AppendVector(fg_MakePattern(300, 3));

				CBinaryStorage Packet;
				uint8 Header[8] = {9, 9, 9, 9, 8, 8, 8, 8};
				Packet.f_AppendBytes(Header, 8);
				Packet.f_AppendStorage(fg_Move(Payload));

				DMibExpect(Packet.f_GetTotalLength(), ==, umint(308));
				DMibExpect(Packet.f_GetSpanCount(), ==, umint(2));

				CIOByteVector Flat = Packet.f_Flatten();
				DMibAssert(Flat.f_GetLen(), ==, umint(308));
				DMibExpect(Flat[0], ==, uint8(9));
				DMibExpect(Flat[8], ==, uint8(3));
				DMibExpect(Flat[307], ==, uint8(uint8(3 + 299)));

				{
					DMibTestPath("SharedNesting");
					TCSharedPointer<CBinaryStorage> pBuilding = fg_Construct();
					pBuilding->f_AppendVector(fg_MakePattern(200, 7));

					TCSharedPointer<CBinaryStorage const> pCached = pBuilding.f_ShareAsConst();

					CBinaryStorage Outer0;
					Outer0.f_AppendStorageShared(fg_TempCopy(pCached));
					CBinaryStorage Outer1;
					Outer1.f_AppendStorageShared(fg_TempCopy(pCached));

					DMibExpect(Outer0.f_GetTotalLength(), ==, umint(200));
					DMibExpect(Outer0.f_Flatten() == Outer1.f_Flatten(), ==, true);
				}
			};

			DMibTestSuite("RandomAccess")
			{
				CBinaryStorage Storage;

				uint8 Data0[4] = {1, 2, 3, 4};
				Storage.f_AppendBytes(Data0, 4);
				Storage.f_AppendVector(fg_MakePattern(200, 100));
				uint8 Data1[2] = {201, 202};
				Storage.f_AppendBytes(Data1, 2);

				DMibExpect(Storage.f_GetByte(0), ==, uint8(1));
				DMibExpect(Storage.f_GetByte(3), ==, uint8(4));
				DMibExpect(Storage.f_GetByte(4), ==, uint8(100));
				DMibExpect(Storage.f_GetByte(203), ==, uint8(uint8(100 + 199)));
				DMibExpect(Storage.f_GetByte(204), ==, uint8(201));
				DMibExpect(Storage.f_GetByte(205), ==, uint8(202));

				{
					DMibTestPath("CopyAcrossBoundaries");
					uint8 Crossing[4];
					Storage.f_CopyTo(Crossing, 2, 4);
					DMibExpect(Crossing[0], ==, uint8(3));
					DMibExpect(Crossing[1], ==, uint8(4));
					DMibExpect(Crossing[2], ==, uint8(100));
					DMibExpect(Crossing[3], ==, uint8(101));
				}

				{
					DMibTestPath("OutOfRange");
					uint8 Dummy[2];
					DMibExpectExceptionType(Storage.f_CopyTo(Dummy, 205, 2), NException::CException);
				}
			};

			DMibTestSuite("MutableLocalSpan")
			{
				CBinaryStorage Storage;

				uint8 Header[10] = {};
				Storage.f_AppendBytes(Header, 10);
				Storage.f_AppendVector(fg_MakePattern(200, 1));

				{
					DMibTestPath("PatchInsideLocalSegment");
					uint8 *pPatch = Storage.f_GetMutableLocalSpan(2, 8);
					DMibAssertTrue(pPatch != nullptr);
					for (umint i = 0; i < 8; ++i)
						pPatch[i] = uint8(0x40 + i);

					DMibExpect(Storage.f_GetByte(2), ==, uint8(0x40));
					DMibExpect(Storage.f_GetByte(9), ==, uint8(0x47));
				}

				{
					DMibTestPath("PatchIntoAdoptedSegmentFails");
					DMibExpectExceptionType(Storage.f_GetMutableLocalSpan(10, 4), NException::CException);
				}

				{
					DMibTestPath("PatchSpanningSegmentsFails");
					DMibExpectExceptionType(Storage.f_GetMutableLocalSpan(8, 4), NException::CException);
				}

				{
					DMibTestPath("PatchOutsideStorageFails");
					DMibExpectExceptionType(Storage.f_GetMutableLocalSpan(210, 4), NException::CException);
				}
			};

			DMibTestSuite("WrapConstructorAndClear")
			{
				CBinaryStorage Storage(fg_MakePattern(500, 5));

				DMibExpect(Storage.f_GetTotalLength(), ==, umint(500));
				DMibExpect(Storage.f_GetSpanCount(), ==, umint(1));

				Storage.f_Clear();
				DMibExpect(Storage.f_GetTotalLength(), ==, umint(0));
				DMibExpectTrue(Storage.f_IsEmpty());
			};

			DMibTestSuite("SharedByteVector")
			{
				CSharedByteVector Empty;
				DMibExpectTrue(Empty.f_IsEmpty());
				DMibExpectTrue(Empty.f_GetArray() == nullptr);

				CSharedByteVector Shared(fg_MakePattern(64, 30));
				CSharedByteVector SharedCopy = Shared;

				DMibExpect(Shared.f_GetLen(), ==, umint(64));
				DMibExpectTrue(Shared.f_GetArray() == SharedCopy.f_GetArray());
				DMibExpectTrue(Shared == SharedCopy);

				CSharedByteVector SameContent(fg_MakePattern(64, 30));
				DMibExpectTrue(Shared == SameContent);

				CSharedByteVector OtherContent(fg_MakePattern(64, 31));
				DMibExpectFalse(Shared == OtherContent);
			};

			DMibTestSuite("WriteStream")
			{
				{
					DMibTestPath("SequentialWriteAndReadBack");
					TCBinaryStreamStorage<> Stream;

					Stream << uint32(0x12345678) << CStr("Hello storage") << uint64(99);

					DMibExpect(Stream.f_GetLength(), ==, NStream::CFilePos(Stream.f_GetStorage().f_GetTotalLength()));
					DMibExpectTrue(Stream.f_IsAtEndOfStream());

					TCBinaryStreamStoragePtr<> ReadBack;
					ReadBack.f_OpenRead(Stream.f_GetStorage());
					uint32 Value32;
					CStr Text;
					uint64 Value64;
					ReadBack >> Value32 >> Text >> Value64;

					DMibExpect(Value32, ==, uint32(0x12345678));
					DMibExpect(Text, ==, CStr("Hello storage"));
					DMibExpect(Value64, ==, uint64(99));
				}

				{
					DMibTestPath("BackPatchHeader");
					TCBinaryStreamStorage<> Stream;

					Stream << uint8(2) << uint64(0);
					Stream << CStr("PayloadPayloadPayload");

					Stream.f_SetPosition(1);
					Stream << uint64(0x1122334455667788);

					TCBinaryStreamStoragePtr<> ReadBack;
					ReadBack.f_OpenRead(Stream.f_GetStorage());
					uint8 Command;
					uint64 PacketID;
					ReadBack >> Command >> PacketID;
					DMibExpect(Command, ==, uint8(2));
					DMibExpect(PacketID, ==, uint64(0x1122334455667788));
				}

				{
					DMibTestPath("BackPatchIntoAdoptedFails");
					TCBinaryStreamStorage<> Stream;

					Stream << uint32(5);
					Stream.f_FeedBytesAdopt(fg_MakePattern(200, 60));

					Stream.f_SetPosition(6);
					uint32 Dummy = 0;
					auto fWrite = [&]
						{
							Stream << Dummy;
						}
					;
					DMibExpectExceptionType(fWrite(), NException::CException);
				}

				{
					DMibTestPath("RewritePastEndFails");
					TCBinaryStreamStorage<> Stream;

					Stream << uint32(5);
					Stream.f_SetPosition(2);
					uint32 Dummy = 7;
					auto fWrite = [&]
						{
							Stream << Dummy;
						}
					;
					DMibExpectExceptionType(fWrite(), NException::CException);
				}

				{
					DMibTestPath("AdoptOnlyAtEnd");
					TCBinaryStreamStorage<> Stream;

					Stream << uint32(5);
					Stream.f_SetPosition(0);
					DMibExpectExceptionType(Stream.f_FeedBytesAdopt(fg_MakePattern(200, 60)), NException::CException);
				}

				{
					DMibTestPath("SetLength");
					TCBinaryStreamStorage<> Stream;

					Stream << uint32(5);
					Stream.f_SetLength(16);
					DMibExpect(Stream.f_GetLength(), ==, NStream::CFilePos(16));
					DMibExpect(Stream.f_GetStorage().f_GetByte(15), ==, uint8(0));

					Stream.f_SetLength(4);
					DMibExpect(Stream.f_GetLength(), ==, NStream::CFilePos(4));
				}

				{
					DMibTestPath("NestedFeedStorage");
					TCBinaryStreamStorage<> PayloadStream;
					PayloadStream << CStr("The payload");

					TCBinaryStreamStorage<> PacketStream;
					PacketStream << uint8(14) << uint64(0);
					PacketStream.f_FeedStorage(PayloadStream.f_MoveStorage());

					DMibExpectTrue(PacketStream.f_IsAtEndOfStream());

					TCBinaryStreamStoragePtr<> ReadBack;
					ReadBack.f_OpenRead(PacketStream.f_GetStorage());
					ReadBack.f_SetPosition(1 + 8);
					CStr Text;
					ReadBack >> Text;
					DMibExpect(Text, ==, CStr("The payload"));
				}
			};

			DMibTestSuite("Equivalence")
			{
				auto fSerialize = [](auto &_Stream)
					{
						_Stream << uint8(14);
						_Stream << uint64(0x1020304050607080);
						_Stream << CStr("com.malterlib/TransportBench");
						_Stream << uint32(0x101);
						_Stream << fg_MakePattern(64, 4);
						_Stream << fg_MakePattern(5000, 9);
						_Stream << int32(-5);
						_Stream << fp64(3.25);
					}
				;

				CBinaryStreamMemory<> MemoryStream;
				fSerialize(MemoryStream);

				TCBinaryStreamStorage<> StorageStream;
				fSerialize(StorageStream);

				CIOByteVector MemoryBytes = MemoryStream.f_MoveVector();
				CIOByteVector StorageBytes = StorageStream.f_GetStorage().f_Flatten();

				DMibAssert(MemoryBytes.f_GetLen(), ==, StorageBytes.f_GetLen());
				DMibExpectTrue(MemoryBytes == StorageBytes);

				{
					DMibTestPath("MovedVectorEquivalence");
					CBinaryStreamMemory<> MemoryStream2;
					TCBinaryStreamStorage<> StorageStream2;

					MemoryStream2 << fg_MakePattern(1000, 17);
					StorageStream2 << fg_Move(fg_MakePattern(1000, 17));

					DMibExpectTrue(MemoryStream2.f_MoveVector() == StorageStream2.f_GetStorage().f_Flatten());
				}
			};

			DMibTestSuite("AdoptGlue")
			{
				{
					DMibTestPath("RvalueByteVectorFeedAdopts");
					TCBinaryStreamStorage<> Stream;

					CIOByteVector Chunk = fg_MakePattern(4096, 3);
					uint8 const *pChunkData = Chunk.f_GetArray();
					Stream << fg_Move(Chunk);

					bool bAdopted = false;
					Stream.f_GetStorage().f_VisitSpans
						(
							[&](uint8 const *_pData, umint _nBytes)
							{
								if (_pData == pChunkData && _nBytes == 4096)
									bAdopted = true;
							}
						)
					;
					DMibExpectTrue(bAdopted);
					DMibExpect(Stream.f_GetStorage().f_GetArenaLength(), ==, umint(sizeof(uint32)));
				}

				{
					DMibTestPath("LvalueByteVectorFeedCopies");
					TCBinaryStreamStorage<> Stream;

					CIOByteVector Chunk = fg_MakePattern(4096, 3);
					Stream << Chunk;

					DMibExpect(Chunk.f_GetLen(), ==, umint(4096));
					DMibExpect(Stream.f_GetStorage().f_GetArenaLength(), ==, umint(sizeof(uint32) + 4096));
				}

				{
					DMibTestPath("SharedFeedAdoptsWithoutConsuming");
					TCBinaryStreamStorage<> Stream;

					CSharedByteVector Shared(fg_MakePattern(4096, 5));
					Stream << Shared;

					DMibExpect(Shared.f_GetLen(), ==, umint(4096));

					bool bAdopted = false;
					Stream.f_GetStorage().f_VisitSpans
						(
							[&](uint8 const *_pData, umint _nBytes)
							{
								if (_pData == Shared.f_GetArray() && _nBytes == 4096)
									bAdopted = true;
							}
						)
					;
					DMibExpectTrue(bAdopted);
				}

				{
					DMibTestPath("MemoryStreamUnchanged");
					CBinaryStreamMemory<> Stream;

					CIOByteVector Chunk = fg_MakePattern(4096, 3);
					Stream << fg_Move(Chunk);
					CSharedByteVector Shared(fg_MakePattern(64, 9));
					Stream << Shared;

					Stream.f_SetPosition(0);
					CIOByteVector ChunkBack;
					CSharedByteVector SharedBack;
					Stream >> ChunkBack >> SharedBack;

					DMibExpectTrue(ChunkBack == fg_MakePattern(4096, 3));
					DMibExpectTrue(SharedBack == Shared);
				}

				{
					DMibTestPath("WireCompatibleWithByteVector");
					TCBinaryStreamStorage<> Stream;

					Stream << CSharedByteVector(fg_MakePattern(300, 12));

					TCBinaryStreamStoragePtr<> ReadStream;
					ReadStream.f_OpenRead(Stream.f_GetStorage());
					CIOByteVector AsVector;
					ReadStream >> AsVector;

					DMibExpectTrue(AsVector == fg_MakePattern(300, 12));
				}
			};

			DMibTestSuite("ReadStream")
			{
				CBinaryStorage Storage;

				uint8 Header[6] = {10, 11, 12, 13, 14, 15};
				Storage.f_AppendBytes(Header, 6);
				Storage.f_AppendVector(fg_MakePattern(300, 40));
				uint8 Tail[2] = {250, 251};
				Storage.f_AppendBytes(Tail, 2);

				{
					DMibTestPath("SequentialConsume");
					TCBinaryStreamStoragePtr<> Stream;
					Stream.f_OpenRead(Storage);

					DMibExpect(Stream.f_GetLength(), ==, NStream::CFilePos(308));

					uint8 ReadBack[308];
					Stream.f_ConsumeBytes(ReadBack, 308);
					DMibExpect(ReadBack[0], ==, uint8(10));
					DMibExpect(ReadBack[5], ==, uint8(15));
					DMibExpect(ReadBack[6], ==, uint8(40));
					DMibExpect(ReadBack[305], ==, uint8(uint8(40 + 299)));
					DMibExpect(ReadBack[306], ==, uint8(250));
					DMibExpect(ReadBack[307], ==, uint8(251));
					DMibExpectTrue(Stream.f_IsAtEndOfStream());
				}

				{
					DMibTestPath("Seeks");
					TCBinaryStreamStoragePtr<> Stream;
					Stream.f_OpenRead(Storage);

					uint8 Byte;
					Stream.f_SetPosition(306);
					Stream.f_ConsumeBytes(&Byte, 1);
					DMibExpect(Byte, ==, uint8(250));

					Stream.f_SetPosition(3);
					Stream.f_ConsumeBytes(&Byte, 1);
					DMibExpect(Byte, ==, uint8(13));

					Stream.f_SetPositionFromEnd(-1);
					Stream.f_ConsumeBytes(&Byte, 1);
					DMibExpect(Byte, ==, uint8(251));

					DMibExpectExceptionType(Stream.f_ConsumeBytes(&Byte, 1), NException::CException);
				}

				{
					DMibTestPath("SubRange");
					TCBinaryStreamStoragePtr<> Stream;
					Stream.f_OpenRead(Storage, 4, 6);

					DMibExpect(Stream.f_GetLength(), ==, NStream::CFilePos(6));

					uint8 ReadBack[6];
					Stream.f_ConsumeBytes(ReadBack, 6);
					DMibExpect(ReadBack[0], ==, uint8(14));
					DMibExpect(ReadBack[1], ==, uint8(15));
					DMibExpect(ReadBack[2], ==, uint8(40));
					DMibExpect(ReadBack[5], ==, uint8(43));
				}

				{
					DMibTestPath("WriteFails");
					TCBinaryStreamStoragePtr<> Stream;
					Stream.f_OpenRead(Storage);

					uint32 Dummy = 5;
					auto fWrite = [&]
						{
							Stream << Dummy;
						}
					;
					DMibExpectExceptionType(fWrite(), NException::CException);
				}

				{
					DMibTestPath("SerializedRoundTrip");
					TCBinaryStreamStorage<> WriteStream;
					WriteStream << uint32(77) << CStr("RoundTrip") << fg_MakePattern(400, 8);

					CBinaryStorage WrittenStorage = WriteStream.f_MoveStorage();

					TCBinaryStreamStoragePtr<> ReadStream;
					ReadStream.f_OpenRead(WrittenStorage);

					uint32 Value;
					CStr Text;
					CIOByteVector Pattern;
					ReadStream >> Value >> Text >> Pattern;

					DMibExpect(Value, ==, uint32(77));
					DMibExpect(Text, ==, CStr("RoundTrip"));
					DMibExpectTrue(Pattern == fg_MakePattern(400, 8));
					DMibExpectTrue(ReadStream.f_IsAtEndOfStream());
				}
			};
		}
	};

	DMibTestRegister(CBinaryStorage_Tests, Malterlib::Stream);
}
