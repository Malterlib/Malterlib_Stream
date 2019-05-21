// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Container/RegistryMixed>

namespace
{
	using namespace NMib::NContainer;
	using namespace NMib::NStream;
	using namespace NMib::NContainer;
	using namespace NMib::NStr;

	class CStream_Tests : public NMib::NTest::CTest
	{
	public:
		void f_DoTests()
		{
			DMibTestSuite("Wrapper")
			{
				CBinaryStreamMemory<> Stream;

				Stream << fg_GetUnsafeStreamWrapper((mint)1) << "ouoeuaoaa   eeee e" << CStr("Oeuoeuoeu");

				Stream.f_SetPosition(0);
				mint Test2;
				ch8 pTest2[200];
				CStr String;
				Stream >> fg_GetUnsafeStreamWrapper(Test2) >> pTest2 >> String;
				DMibTest(DMibExpr(Test2) == DMibExpr(mint(1)));
				DMibTest(DMibExpr(pTest2) == DMibExpr(CStr("ouoeuaoaa   eeee e")));
				DMibTest(DMibExpr(String) == DMibExpr(CStr("Oeuoeuoeu")));

				// We should also be able to read the string as a CStr
				Stream.f_SetPosition(0);
				CStr String2;
				Stream >> fg_GetUnsafeStreamWrapper(Test2) >> String2 >> String;
				DMibTest(DMibExpr(String2) == DMibExpr(CStr("ouoeuaoaa   eeee e")));
			};
		}
	};

	DMibTestRegister(CStream_Tests, Malterlib::Stream);
}


