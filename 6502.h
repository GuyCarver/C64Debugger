
#pragma once

#include <cstdint>
//Macros from wingdi.h we don't like.
#undef RELATIVE
#undef ABSOLUTE
#undef ERROR

//----------------------------------------------------------------
///Upper case char
char Upper( char aChar );

//----------------------------------------------------------------
///Return true if char is a hex digit
bool IsHex( char aChar );

//----------------------------------------------------------------
///Possible address modes available for opcodes
enum class ADDRESS_MODE : uint8_t
{					//bytes - example
	NONE,				//1 - op
	IMMEDIATE,			//2 - op #$v
	ZERO_PAGE,			//2 - op $aa
	ZERO_PAGE_X,		//2 - op $aa,x
	ZERO_PAGE_Y,		//2 - op $aa,y
	INDIRECT_X,			//2 - op ($aa,x)
	INDIRECT_Y,			//2 - op ($aa),y
	RELATIVE,			//2 - op $aa
	ABSOLUTE,			//3 - op $aaaa
	ABSOLUTE_X,			//3 - op $aaaa,x
	ABSOLUTE_Y,			//3 - op $aaaa,y
	INDIRECT,			//3 - op ($aaaa)
	ERROR				//Indicates compile error
};

//----------------------------------------------------------------
///Element data for opcode array
struct OpCode
{
	const char *pName;							//Opcode name
	ADDRESS_MODE eMode;							//Addressing mode
	uint8_t Cycles;								//Cycles for operation

	//----------------------------------------------------------------
	///Case sensitive compare of string against name
	bool operator==( const char *apSource ) const
	{
		return (apSource[0] == pName[0])
			&& (apSource[1] == pName[1])
			&& (apSource[2] == pName[2]);
	}

	bool operator==( ADDRESS_MODE aeMode ) const { return aeMode == eMode; }

	uint8_t QSize(  ) const { return AddrModeSize(eMode); }

	//----------------------------------------------------------------
	///Find OpCode index matching given data, 0xff if not found
	static uint8_t Find( const char *apName, ADDRESS_MODE aeMode );

	//----------------------------------------------------------------
	///Return OpCode for given index
	static const OpCode &Get( uint8_t aIndex );

	//----------------------------------------------------------------
	///Find opcode matches. apString should be uppercase.
	/// Matching names are saved to apDest and the count of entries is returned
	static uint32_t FindMatches( const char **apDest, uint32_t MaxEntries, const char *apString );

	//----------------------------------------------------------------
	///Get name for given ADDRESS_MODE
	static const char *AddrModeName( ADDRESS_MODE aeMode );

	//----------------------------------------------------------------
	///Get size of given ADDRESS_MODE in bytes (0-3)
	static uint8_t AddrModeSize( ADDRESS_MODE aeMode );
};




