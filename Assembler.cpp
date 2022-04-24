
#include "Assembler.h"
#include "6502.h"
#include "Numbers.h"

/*{
[ ] Store Address Mode, Address
[ ] Set AddresssMode and Size on compile of Address Mode

}*/


namespace Assembler
{

//----------------------------------------------------------------
///Structure used to consume chars in a string constricting
/// read to string size
struct SourceString
{
	const char *pString;
	uint32_t Len;

	//----------------------------------------------------------------
	const char *SkipName(  )
	{
		//Skip until space
		while (Len && *pString != ' ') {
			++pString;
			--Len;
		}

		while (Len) {
			if (*pString != ' ') {
				return pString;
			}
			++pString;
			--Len;
		}

		return nullptr;
	}

	//----------------------------------------------------------------
	char Char(  ) { return *pString; }

	//----------------------------------------------------------------
	///NOTE: In debug this compiles to a function call to the lambda,
	/// but in release it's very similar to the non-lambda version.
	/// I like the flow of this code a bit more so I'm using this one.
	char Next(  )
	{
		return Len
		? [&]() {
			--Len;
			return *pString++;
		  }()
		: 0;
	}

	//----------------------------------------------------------------
	///Traditional version
//	char Next(  )
//	{
//		char c = 0;
//		if (Len) {
//			--Len;
//			c = *pString++;
//		}
//		return c;
//	}

	//----------------------------------------------------------------
	char NextSkip( char aSkip )
	{
		auto c = Next();
		while (c == aSkip) {
			c = Next();
		}
		return c;
	}

	//----------------------------------------------------------------
	bool Byte( uint8_t &arValue ) {
		bool bres = false;
		if (Len > 1) {
			if (*pString == '$') {				//Skip $ if it exists
				++pString;
				if (--Len < 2) {
					return false;				//Exit here with not enough data
				}
			}

			bres = (IsHex(pString[0]) && IsHex(pString[1]));
			arValue = bres ? Numbers::HexToUInt8(pString) : 0;
			pString += 2;
			Len -= 2;
		}
		return bres;
	}
};

//----------------------------------------------------------------
ADDRESS_MODE CompileAddressMode( SourceString &arSource, uint16_t &arValue )
{
	arValue = 0;

	uint8_t b;
	char c = arSource.Char();
	if (c == 0) {
		return ADDRESS_MODE::NONE;
	}

	if (c == '#') {
		arSource.Next();						//Skip #
		if (arSource.Byte(b) == false) {
			return ADDRESS_MODE::ERROR;
		}
		arValue = b;
		return ADDRESS_MODE::IMMEDIATE;
	}

	if (c == '(') {
		arSource.Next();						//Skip (
		if (arSource.Byte(b) == false) {
			return ADDRESS_MODE::ERROR;
		}

		arValue = b;

		char c0 = arSource.Next();
		char c1 = Upper(arSource.Next());
		char c2 = Upper(arSource.Next());

		if ((c0 == ',') && (c1 == 'X') && (c2 == ')')) {
			return ADDRESS_MODE::INDIRECT_X;
		}

		if ((c0 == ')') && (c1 == ',') && (c2 == 'Y')) {
			return ADDRESS_MODE::INDIRECT_Y;
		}
	}
	else {
		if (arSource.Byte(b) == false) {
			return ADDRESS_MODE::ERROR;
		}
		arValue = b;

		c = arSource.Char();
		if (c == 0) {
			return ADDRESS_MODE::ZERO_PAGE;
		}

		//If ZERO_PAGE X or Y
		if (c == ',') {
			arSource.Next();
			c = Upper(arSource.Next());
			if (c == 'X') {
				return ADDRESS_MODE::ZERO_PAGE_X;
			}
			else if (c == 'Y') {
				return ADDRESS_MODE::ZERO_PAGE_Y;
			}
		}
		else {
			if (arSource.Byte(b) == false) {
				return ADDRESS_MODE::ERROR;
			}

			arValue <<= 8;
			arValue |= b;

			c = arSource.Next();
			if (c == 0) {
				return ADDRESS_MODE::ABSOLUTE;
			}

			if (c == ',') {
				c = Upper(arSource.Next());
				if (c == 'X') {
					return ADDRESS_MODE::ABSOLUTE_X;
				}
				else if (c == 'Y') {
					return ADDRESS_MODE::ABSOLUTE_Y;
				}
			}
		}
	}

	return ADDRESS_MODE::ERROR;
}

//----------------------------------------------------------------
Assemble::Assemble( const char *apSource, uint32_t aSourceLen, uint16_t aAddress )
{
	FromString(apSource, aSourceLen, aAddress);
}

//----------------------------------------------------------------
bool AbsoluteToRelative( uint16_t aAddress, uint16_t &aValue )
{
	bool bres;
	int16_t relative = aValue - aAddress;
	//If relative address in range -128 < x <= 127
	if (bres = ((relative <= 127) && (relative >= -128)); bres) {
		aValue = relative & 0xff;
	}
	return bres;
}

//----------------------------------------------------------------
bool Assemble::FromString( const char *apSource, uint32_t aSourceLen, uint16_t aAddress )
{
	SourceString src{apSource, aSourceLen};

	src.SkipName();

	uint16_t val = 0;
	ADDRESS_MODE mode = CompileAddressMode(src, val);

	if (mode != ADDRESS_MODE::ERROR) {
		OP = OpCode::Find(apSource, mode);

		//If we didn't find the op and ADDRESS_MODE::ZERO_PAGE
		// it may be a relative branch, so look for that
		if ((OP == 0xff) && (Upper(apSource[0]) == 'B')) {
			if (mode == ADDRESS_MODE::ZERO_PAGE) {
				mode = ADDRESS_MODE::RELATIVE;
				OP = OpCode::Find(apSource, mode);
			}
			//If absolute address mode, attempt to convert to relative
			// using supplied address
			else if (mode == ADDRESS_MODE::ABSOLUTE) {
				if (AbsoluteToRelative(aAddress, val)) {
					mode = ADDRESS_MODE::RELATIVE;
					OP = OpCode::Find(apSource, mode);
				}
			}
		}
	}
	else {
		OP = 0xff;
	}

	//If not the last operation (which is BAD) then found.
	if (OP != 0xff) {
		Len = OpCode::AddrModeSize(mode);
		B0 = static_cast<uint8_t>(val);
		B1 = static_cast<uint8_t>(val >> 8);
	}
	else {
		Len = 0;
		B0 = 0;
		B1 = 0;
	}

	return QGood();
}

//----------------------------------------------------------------
uint8_t Assemble::ToDest( uint8_t *apDest, uint32_t aDestLen )
{
	uint8_t w = (aDestLen >= Len) ? Len : 0;
	if (w) {
		*apDest++ = OP;
		if (Len > 1) {
			*apDest++ = B0;
			if (Len > 2) {
				*apDest++ = B1;
			}
		}
	}

	return w;
}

}	//namespace Assembler
