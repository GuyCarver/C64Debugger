#include "pch.h"
#include "CppUnitTest.h"
#include "CppUnitTestAssert.h"
#include "Framework.h"
#include "../Command.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(TestCommand)
	{
	public:
		TEST_METHOD(HardReset)
		{
			CommandPtr p = Command::HardResetCommand;
			Assert::AreEqual(p->QMaxSize(), DEFBODYLEN, L"Incorrect Max Size");
			Assert::AreEqual<uint32_t>(p->QSize(), 12, L"Incorrect Size");
			Assert::AreEqual<uint32_t>(p->QBodyLen(), 1, L"Incorrect Body Length");
			Assert::AreEqual<uint32_t>(p->QID(), 5, L"Incorrect ID");
			Assert::AreEqual<uint32_t>(static_cast<uint32_t>(p->QCommand()), 0xcc, L"Incorrect Command");
			Assert::AreEqual<uint8_t>(p->QBody()[0], 1, L"Incorrect Body Value");
		}
	};
}
