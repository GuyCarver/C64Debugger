#include "pch.h"
#include "CppUnitTest.h"
#include "CppUnitTestAssert.h"
#include "Framework.h"
#include "../Assembler.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{

	struct TestData
	{
		const char *Str;
		uint8_t OP;
		uint8_t B0;
		uint8_t B1;
		uint8_t Len;
		bool res;
	};

	TEST_CLASS(AssemblerTests)
	{
	public:
		
		TEST_METHOD(TestAssemble)
		{
			TestData TestA[] =
			{
				{"BNE $2180", 0xD0, 0x80, 0, 2, true},		//Maximum negative
				{"BPL $2184", 0x10, 0x84, 0, 2, true},		//Middle negative
				{"BEQ $227f", 0xF0, 0x7F, 0, 2, true},		//Maximum forward
				{"BEQ $2280", 0xFF, 0, 0, 0, false},		//Out of range
				{"LDA ($34,x)", 0xA1, 0x34, 0, 2, true},
				{"JUNK", 0xFF, 0, 0, 0, false},
				{"LDA $$,X", 0xFF, 0, 0, 0, false},
				{"SEC", 0x38, 0, 0, 1, true},
				{"DEX", 0xCA, 0, 0, 1, true},
				{"BNE $23", 0xD0, 0x23, 0, 2, true},
				{"JMP 1234", 0x4C, 0x34, 0x12, 3, true},
				{"AND $33,X", 0x35, 0x33, 0, 2, true},
				{"STA (73),y", 0x91, 0x73, 0, 2, true},
				{"CMP ($74),Y", 0xD1, 0x74, 0, 2, true},
				{"LDX (#$75),Y", 0xFF, 0, 0, 0, false},
				{"cmp $AbcD", 0xCD, 0xCD, 0xAB, 3, true},
				{"STA $567A,X", 0x9D, 0x7A, 0x56, 3, true},
				{"lda $567B,Y", 0xB9, 0x7B, 0x56, 3, true},
				{"STA 567D,y", 0x99, 0x7D, 0x56, 3, true},
				{"LDA 567D,x", 0xBD, 0x7D, 0x56, 3, true},
			};

			Assembler::Assemble ass;
			uint16_t address = 0x2200;

			for ( const auto t : TestA ) {
				Logger::WriteMessage(t.Str);
				Logger::WriteMessage("\n");  //Have to cr for Test Detail Summary window. Output window doesn't need it but also seems to ignore it.
				bool bres = ass.FromString(t.Str, static_cast<uint32_t>(strlen(t.Str)), address);
				Assert::AreEqual(bres, t.res, L"Result missmatch");
				Assert::AreEqual(t.OP, ass.OP, L"Op missmatch");
				Assert::AreEqual(t.B0, ass.B0, L"B0 missmatch");
				Assert::AreEqual(t.B1, ass.B1, L"B1 missmatch");
				Assert::AreEqual(t.Len, ass.Len, L"Len missmatch");
			}
		}
	};
}
