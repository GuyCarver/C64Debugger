#pragma once

#include "../6502.h"

namespace Microsoft{ namespace VisualStudio {namespace CppUnitTestFramework
{
	template<> inline std::wstring ToString<ADDRESS_MODE> (const ADDRESS_MODE &t) { RETURN_WIDE_STRING(OpCode::AddrModeName(t)); }
	template<> inline std::wstring ToString<uint16_t> (const uint16_t &t) { RETURN_WIDE_STRING(t); }
}}}
