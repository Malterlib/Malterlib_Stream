// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Container/RegistryMixed>

namespace
{
	using namespace NMib::NContainer;
	using namespace NMib::NStream;
	using namespace NMib::NContainer;
	using namespace NMib::NStr;

	struct CStream_Tests : public NMib::NTest::CTest
	{
		void f_DoTests()
		{
			DMibTestSuite("Wrapper")
			{
				TCBinaryStreamMemory<> Stream;

				Stream << fg_GetUnsafeStreamWrapper((umint)1) << "ouoeuaoaa   eeee e" << CStr("Oeuoeuoeu");

				Stream.f_SetPosition(0);
				umint Test2;
				ch8 Test2Buffer[200];
				CStr String;
				Stream >> fg_GetUnsafeStreamWrapper(Test2) >> Test2Buffer >> String;
				DMibTest(DMibExpr(Test2) == DMibExpr(umint(1)));
				DMibTest(DMibExpr(Test2Buffer) == DMibExpr(CStr("ouoeuaoaa   eeee e")));
				DMibTest(DMibExpr(String) == DMibExpr(CStr("Oeuoeuoeu")));

				// We should also be able to read the string as a CStr
				Stream.f_SetPosition(0);
				CStr String2;
				Stream >> fg_GetUnsafeStreamWrapper(Test2) >> String2 >> String;
				DMibTest(DMibExpr(String2) == DMibExpr(CStr("ouoeuaoaa   eeee e")));
			};
			DMibTestSuite("Move Construct")
			{
				CByteVector Bytes;
				for (umint iByte = 0; iByte < 1000; ++iByte)
					Bytes.f_Insert(uint8(iByte));

				TCBinaryStreamMemory<> Stream(NMib::fg_Move(Bytes));
				DMibExpect(Stream.f_GetLength(), ==, 1000);

				for (umint iByte = 0; iByte < 1000; ++iByte)
				{
					uint8 Value = 0;
					Stream >> fg_GetUnsafeStreamWrapper(Value);
					if (Value != uint8(iByte))
					{
						DMibExpect(Value, ==, uint8(iByte));
						break;
					}
				}
				DMibExpect(Stream.f_GetPosition(), ==, 1000);
			};
			DMibTestSuite("Length Limit")
			{
				TCBinaryStreamMemory<> OutStream;

				uint64 Value1 = 0;
				uint64 Value2 = 1;
				uint64 Value3 = 2;
				uint64 Value4 = 3;
				CStr Value5 = "Testing";
				CStr Value6;
				for (umint iChar = 0; iChar < 22728937; ++iChar)
					Value6 += "T";
				{
					OutStream << Value1;
					OutStream << Value2;
					OutStream << Value3;
					OutStream << Value4;
					OutStream << Value5;
					OutStream << Value6;
				}

				auto Data = OutStream.f_MoveVector();

				TCBinaryStreamMemoryPtr<> InStreamFull;
				InStreamFull.f_OpenRead(Data.f_GetArray(), Data.f_GetLen());

				CBinaryStreamDefault &InStream = InStreamFull;

				uint64 InValue1 = 0;
				uint64 InValue2 = 1;
				uint64 InValue3 = 2;
				uint64 InValue4 = 3;
				CStr InValue5;
				CStr InValue6;
				{
					InStream >> InValue1;
					InStream >> InValue2;
					InStream >> InValue3;
					InStream >> InValue4;
					InStream >> InValue5;
					InStream >> InValue6;
				}

				DMibExpect(InValue1, ==, Value1);
				DMibExpect(InValue2, ==, Value2);
				DMibExpect(InValue3, ==, Value3);
				DMibExpect(InValue4, ==, Value4);
				DMibExpect(InValue5, ==, Value5);
				DMibExpect(InValue6, ==, Value6);
			};
		}
	};

	DMibTestRegister(CStream_Tests, Malterlib::Stream);
}

