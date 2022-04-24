#include "pch.h"
#include "CppUnitTest.h"
#include "CppUnitTestAssert.h"
#include "Framework.h"
#include "../Disassembler.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Labels
{
	const char *Find( uint16_t ) { return nullptr; }
}

namespace UnitTests
{

uint8_t Data[] =
{
	0xA0, 0x02,
	0xB1, 0x64,
	0xC5, 0x34,
	0x90, 0x17,
	0xD0, 0x07,
	0x88,
	0xB1, 0x64,
	0xC5, 0x33,
	0x90, 0x0E,
	0xA4, 0x65,
	0xC4, 0x2E,
	0x90, 0x08,
	0xD0, 0x0D,
	0xA5, 0x64,
	0xC5, 0x2D,
	0xB0, 0x07,
	0xA5, 0x64,
	0xA4, 0x65,
	0x4C, 0x68, 0xAA,
	0xA0, 0x00,
	0xB1, 0x64,
	0x20, 0x75, 0xB4,
	0xA5, 0x50,
	0xA4, 0x51,
	0x85, 0x6F,
	0x84, 0x70,
	0x20, 0x7A, 0xB6
};

const char *DisResult =
	"LDY #$02\nLDA ($64),Y\nCMP $34\nBCC $AA4B\nBNE $AA3D\n"
	"DEY\nLDA ($64),Y\nCMP $33\nBCC $AA4B\nLDY $65\nCPY $2E\n"
	"BCC $AA4B\nBNE $AA52\nLDA $64\nCMP $2D\nBCS $AA52\n"
	"LDA $64\nLDY $65\nJMP $AA68\nLDY #$00\nLDA ($64),Y\n"
	"JSR $B475\nLDA $50\nLDY $51\nSTA $6F\nSTY $70\nJSR $B67A";

const char *ByteResult =
	"A0 02\nB1 64\nC5 34\n90 17\nD0 07\n88\nB1 64\nC5 33\n"
	"90 0E\nA4 65\nC4 2E\n90 08\nD0 0D\nA5 64\nC5 2D\nB0 07\n"
	"A5 64\nA4 65\n4C 68 AA\nA0 00\nB1 64\n20 75 B4\nA5 50\n"
	"A4 51\n85 6F\n84 70\n20 7A B6";

const char *AddrResult =
	"AA2C\nAA2E\nAA30\nAA32\nAA34\nAA36\nAA37\nAA39\nAA3B\n"
	"AA3D\nAA3F\nAA41\nAA43\nAA45\nAA47\nAA49\nAA4B\nAA4D\n"
	"AA4F\nAA52\nAA54\nAA56\nAA59\nAA5B\nAA5D\nAA5F\nAA61";

const uint8_t OpCodeA[] =
{
	0xA0,
	0xB1,
	0xC5,
	0x90,
	0xD0,
	0x88,
	0xB1,
	0xC5,
	0x90,
	0xA4,
	0xC4,
	0x90,
	0xD0,
	0xA5,
	0xC5,
	0xB0,
	0xA5,
	0xA4,
	0x4C,
	0xA0,
	0xB1,
	0x20,
	0xA5,
	0xA4,
	0x85,
	0x84,
	0x20
};

constexpr uint32_t LINES = sizeof(OpCodeA);

char DisDest[LINES * 0x20];
char ByteDest[LINES * 9];
const OpCode *OpCodeDest[LINES];

	TEST_CLASS(DisassemblerTests)
	{
	public:

		TEST_METHOD(TestDisassemble)
		{
			DisAssembler::Input input{
				DisDest,
				ByteDest,
				OpCodeDest,
				Data,
				sizeof(DisDest),
				sizeof(Data),
				27,
				0xAA2C
			};

			DisAssembler::DisAssemble(input);

			Logger::WriteMessage(DisDest);
			Logger::WriteMessage("\n");  //Have to cr for Test Detail Summary window. Output window doesn't need it but also seems to ignore it.
			Assert::AreEqual(DisResult, DisDest, false, L"DisAssembly missmatch");
			Assert::AreEqual(ByteResult, ByteDest, false, L"Bytecode missmatch");

			//Initialize opcode result array
			for ( uint32_t i = 0; i < LINES; ++i) {
				auto &op = OpCode::Get(OpCodeA[i]);
				if (OpCodeDest[i] != &op) {
					wchar_t wchar[8];
					size_t slen = 3;
					mbstowcs_s(&slen, wchar, OpCodeDest[i]->pName, slen);
					Logger::WriteMessage(wchar);
					Logger::WriteMessage(" != ");
					mbstowcs_s(&slen, wchar, op.pName, slen);
					Assert::Fail(L"Opcode missmatch");
				}
			}
		}
	};
}
