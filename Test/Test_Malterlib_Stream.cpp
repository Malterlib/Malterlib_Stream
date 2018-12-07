// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Container/RegistryMixed>

namespace
{
	enum ETesting
	{
		ETesting_0,
		ETesting_1,
	};

	using namespace NMib::NContainer;
	using namespace NMib::NStream;
	using namespace NMib::NContainer;
	using namespace NMib::NStr;

	// Vector
	// Doubly linked list
	// TCIndirection
	// Time
	// TimeSpan
	// Avl tree
	// Variant
	// Hash digest
	// TCClearing
	// TCClearingPtr
	// 


	class CStream_Tests : public NMib::NTest::CTest
	{
	public:

		class CTestNamedStream
		{
			uint32 m_TestInt;
			TCMap<int32> m_Set;
			TCMap<int32, CStr> m_Map;

			struct CLinked
			{
				uint32 m_Value;
				DMibListLinkDS_Link(CLinked, m_Link);
				template <typename t_CRegistry>
				void f_FeedNamed(t_CRegistry &_Registry) const
				{
					_Registry << fg_Named("LinkedValue", m_Value);
				}

				template <typename t_CRegistry>
				bint f_ConsumeNamed(t_CRegistry const &_Registry)
				{
					_Registry >> fg_Named("LinkedValue", m_Value, int32(-1));
					return true;
				}
			};				

			DMibListLinkDS_List(CLinked, m_Link) m_DList;


		public:
			CTestNamedStream()
			{
				m_Set[10];
				m_Set[20];
				m_Set[30];
				m_Set[40];

				m_Map[11] = "Eleven";
				m_Map[21] = "Twenty one";
				m_Map[31] = "Thirty one";
				m_Map[41] = "Forty one";
			}

			template <typename t_CRegistry>
			void f_FeedNamed(t_CRegistry &_Registry) const
			{
				_Registry << fg_Named("TestInt", m_TestInt);
				_Registry << fg_Named("TestSet", m_Set);
				_Registry << fg_Named("TestMap", m_Map);
				_Registry << fg_Named("DList", m_DList);
			}

			template <typename t_CRegistry>
			bint f_ConsumeNamed(t_CRegistry const &_Registry)
			{
				_Registry >> fg_Named("TestInt", m_TestInt, int32(-1));
				_Registry >> fg_Named("TestSet", m_Set, TCMap<uint32>());
				_Registry >> fg_Named("TestMap", m_Map, TCMap<uint32, CStr>());
				_Registry >> fg_Named("DList", m_DList);
				return true;
			}

		};

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

			DMibTestCategory("Named Stream")
			{
				NMib::NContainer::CRegistry_CMStrDeprecated TestRegSource;
				TestRegSource.f_SetValue("Testing123", "TestVal123");
				TestRegSource.f_SetValue("Testing321", "TestVal321");

				DMibTestSuite("Registry")
				{
					NMib::NContainer::CRegistry_CStr Reg;

					{
						int32 Test = 334;
						Reg << fg_Named("Test", Test);
						Reg << fg_Named("TestEnum", ETesting_1);
						Reg << fg_Named("TestReg", TestRegSource);

						CStream_Tests::CTestNamedStream Testing;

						Reg << fg_Named("Testing", Testing);

						int32 Test2;
						Reg >> fg_Named("Test", Test2, int32(-1));
						DMibTest(DMibExpr(Test2) == DMibExpr(Test));

						ETesting TestEnum;
						Reg >> fg_Named("TestEnum", TestEnum, ETesting_0);
						DMibTest(DMibExpr(TestEnum) == DMibExpr(ETesting_1));


						NMib::NContainer::CRegistry_CMStrDeprecated TestReg;
						Reg >> fg_Named("TestReg", TestReg, NMib::NContainer::CRegistry_CMStrDeprecated());
						DMibTest(DMibExpr(TestReg == TestRegSource));

						Reg >> fg_Named("TestRegNotFound", TestReg, NMib::NContainer::CRegistry_CMStrDeprecated());
						DMibTest(DMibExpr(TestReg != TestRegSource));

						int32 Test3;
						Reg >> fg_Named("TestNotFound", Test3, int32(-1));
						DMibTest(DMibExpr(Test3) == DMibExpr(int32(-1)));

					}
				};

				DMibTestSuite("Normal")
				{
					CBinaryStreamMemory<> Stream;

					{
						int32 Test = 334;
						Stream << fg_Named("Test", Test);
						Stream << fg_Named("TestEnum", ETesting_1);
						Stream << fg_Named("TestReg", TestRegSource);

						int32 Test2;
						Stream.f_SetPosition(0);
						Stream >> fg_Named("Test", Test2, int32(-1));
						DMibTest(DMibExpr(Test2) == DMibExpr(Test));

						ETesting TestEnum;
						Stream >> fg_Named("TestEnum", TestEnum, ETesting_0);
						DMibTest(DMibExpr(TestEnum) == DMibExpr(ETesting_1));

						NMib::NContainer::CRegistry_CMStrDeprecated TestReg;
						Stream >> fg_Named("TestReg", TestReg, NMib::NContainer::CRegistry_CMStrDeprecated());
						DMibTest(DMibExpr(TestReg == TestRegSource));

						int32 Test3;
						Stream.f_SetPosition(0);
						Stream >> fg_Named("TestNotFound", Test3, int32(-1));
						DMibTest(DMibExpr(Test3) == DMibExpr(Test)); // Name is ignored
					}
				};
			};

		}
			
	};

	DMibTestRegister(CStream_Tests, Malterlib::Stream);
}


