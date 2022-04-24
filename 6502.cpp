
#include "6502.h"
#include "Numbers.h"

//----------------------------------------------------------------
///Size of address mode in bytes
uint8_t AddressModeSizeA[] =
{
	1,		//NONE,				//1 - op
	2,		//IMMEDIATE,		//2 - op #$v
	2,		//ZERO_PAGE,		//2 - op $aa
	2,		//ZERO_PAGE_X,		//2 - op $aa,x
	2,		//ZERO_PAGE_Y,		//2 - op $aa,y
	2,		//INDIRECT_X,		//2 - op ($aa,x)
	2,		//INDIRECT_Y,		//2 - op ($aa),y
	2,		//RELATIVE,			//2 - op $aa
	3,		//ABSOLUTE,			//3 - op $aaaa
	3,		//ABSOLUTE_X,		//3 - op $aaaa,x
	3,		//ABSOLUTE_Y,		//3 - op $aaaa,y
	3,		//INDIRECT,			//3 - op ($aaaa)
	1,		//ERROR				//Indicates compile error
};

//----------------------------------------------------------------
///Address mode names
const char *AddressModeNameA[] =
{
	"NONE",				//1 - op
	"IMMEDIATE",		//2 - op #$v
	"ZERO_PAGE",		//2 - op $aa
	"ZERO_PAGE_X",		//2 - op $aa,x
	"ZERO_PAGE_Y",		//2 - op $aa,y
	"INDIRECT_X",		//2 - op ($aa,x)
	"INDIRECT_Y",		//2 - op ($aa),y
	"RELATIVE",			//2 - op $aa
	"ABSOLUTE",			//3 - op $aaaa
	"ABSOLUTE_X",		//3 - op $aaaa,x
	"ABSOLUTE_Y",		//3 - op $aaaa,y
	"INDIRECT",			//3 - op ($aaaa)
	"ERROR"				//Indicates compile error
};

//----------------------------------------------------------------
///Opcode Names in alphabetical order for auto complete
const char *OpNameA[] =
{
	"ADC",
	"AND",
	"ASL",
	"BCC",
	"BCS",
	"BEQ",
	"BIT",
	"BMI",
	"BNE",
	"BPL",
	"BRK",
	"BVC",
	"BVS",
	"CLC",
	"CLD",
	"CLI",
	"CLV",
	"CMP",
	"CPX",
	"CPY",
	"DEC",
	"DEX",
	"DEY",
	"EOR",
	"INC",
	"INX",
	"INY",
	"JMP",
	"JSR",
	"LDA",
	"LDX",
	"LDY",
	"LSR",
	"NOP",
	"ORA",
	"PHA",
	"PHP",
	"PLA",
	"PLP",
	"ROL",
	"ROR",
	"RTI",
	"RTS",
	"SBC",
	"SEC",
	"SED",
	"SEI",
	"STA",
	"STX",
	"STY",
	"TAX",
	"TAY",
	"TSX",
	"TXA",
	"TXS",
	"TYA"
};

//----------------------------------------------------------------
///OpCode entry for all 255 possible values
const OpCode OpCodeA[] = {
{"BRK", ADDRESS_MODE::NONE, 7},			//0x00
{"ORA", ADDRESS_MODE::INDIRECT_X, 6},	//0x01
{"BAD", ADDRESS_MODE::NONE, 2},			//0x02
{"BAD", ADDRESS_MODE::NONE, 2},			//0x03
{"BAD", ADDRESS_MODE::NONE, 2},			//0x04
{"ORA", ADDRESS_MODE::ZERO_PAGE, 3},	//0x05
{"ASL", ADDRESS_MODE::ZERO_PAGE, 5},	//0x06
{"BAD", ADDRESS_MODE::NONE, 2},			//0x07
{"PHP", ADDRESS_MODE::NONE, 3},			//0x08
{"ORA", ADDRESS_MODE::IMMEDIATE, 2},	//0x09
{"ASL", ADDRESS_MODE::NONE, 2},			//0x0a
{"BAD", ADDRESS_MODE::NONE, 2},			//0x0b
{"BAD", ADDRESS_MODE::NONE, 2},			//0x0c
{"ORA", ADDRESS_MODE::ABSOLUTE, 4},		//0x0d
{"ASL", ADDRESS_MODE::ABSOLUTE, 6},		//0x0e
{"BAD", ADDRESS_MODE::NONE, 2},			//0x0f
{"BPL", ADDRESS_MODE::RELATIVE, 3},		//0x10
{"ORA", ADDRESS_MODE::INDIRECT_Y, 5},	//0x11
{"BAD", ADDRESS_MODE::NONE, 2},			//0x12
{"BAD", ADDRESS_MODE::NONE, 2},			//0x13
{"BAD", ADDRESS_MODE::NONE, 2},			//0x14
{"ORA", ADDRESS_MODE::ZERO_PAGE_X, 4},	//0x15
{"ASL", ADDRESS_MODE::ZERO_PAGE_X, 6},	//0x16
{"BAD", ADDRESS_MODE::NONE, 2},			//0x17
{"CLC", ADDRESS_MODE::NONE, 2},			//0x18
{"ORA", ADDRESS_MODE::ABSOLUTE_Y, 4},	//0x19
{"BAD", ADDRESS_MODE::NONE, 2},			//0x1a
{"BAD", ADDRESS_MODE::NONE, 2},			//0x1b
{"BAD", ADDRESS_MODE::NONE, 2},			//0x1c
{"ORA", ADDRESS_MODE::ABSOLUTE_X, 4},	//0x1d
{"ASL", ADDRESS_MODE::ABSOLUTE_X, 7},	//0x1e
{"BAD", ADDRESS_MODE::NONE, 2},			//0x1f
{"JSR", ADDRESS_MODE::ABSOLUTE, 6},		//0x20
{"AND", ADDRESS_MODE::INDIRECT_X, 6},	//0x21
{"BAD", ADDRESS_MODE::NONE, 2},			//0x22
{"BAD", ADDRESS_MODE::NONE, 2},			//0x23
{"BIT", ADDRESS_MODE::ZERO_PAGE, 3},	//0x24
{"AND", ADDRESS_MODE::ZERO_PAGE, 3},	//0x25
{"ROL", ADDRESS_MODE::ZERO_PAGE, 5},	//0x26
{"BAD", ADDRESS_MODE::NONE, 2},			//0x27
{"PLP", ADDRESS_MODE::NONE, 4},			//0x28
{"AND", ADDRESS_MODE::IMMEDIATE, 2},	//0x29
{"ROL", ADDRESS_MODE::NONE, 2},			//0x2a
{"BAD", ADDRESS_MODE::NONE, 2},			//0x2b
{"BIT", ADDRESS_MODE::ABSOLUTE, 4},		//0x2c
{"AND", ADDRESS_MODE::ABSOLUTE, 4},		//0x2d
{"ROL", ADDRESS_MODE::ABSOLUTE, 6},		//0x2e
{"BAD", ADDRESS_MODE::NONE, 2},			//0x2f
{"BMI", ADDRESS_MODE::RELATIVE, 3},		//0x30
{"AND", ADDRESS_MODE::INDIRECT_Y, 5},	//0x31
{"BAD", ADDRESS_MODE::NONE, 2},			//0x32
{"BAD", ADDRESS_MODE::NONE, 2},			//0x33
{"BAD", ADDRESS_MODE::NONE, 2},			//0x34
{"AND", ADDRESS_MODE::ZERO_PAGE_X, 4},	//0x35
{"ROL", ADDRESS_MODE::ZERO_PAGE_X, 6},	//0x36
{"BAD", ADDRESS_MODE::NONE, 2},			//0x37
{"SEC", ADDRESS_MODE::NONE, 2},			//0x38
{"AND", ADDRESS_MODE::ABSOLUTE_Y, 4},	//0x39
{"BAD", ADDRESS_MODE::NONE, 2},			//0x3a
{"BAD", ADDRESS_MODE::NONE, 2},			//0x3b
{"BAD", ADDRESS_MODE::NONE, 2},			//0x3c
{"AND", ADDRESS_MODE::ABSOLUTE_X, 4},	//0x3d
{"ROL", ADDRESS_MODE::ABSOLUTE_X, 7},	//0x3e
{"BAD", ADDRESS_MODE::NONE, 2},			//0x3f
{"RTI", ADDRESS_MODE::NONE, 6},			//0x40
{"EOR", ADDRESS_MODE::INDIRECT_X, 6},	//0x41
{"BAD", ADDRESS_MODE::NONE, 2},			//0x42
{"BAD", ADDRESS_MODE::NONE, 2},			//0x43
{"BAD", ADDRESS_MODE::NONE, 2},			//0x44
{"EOR", ADDRESS_MODE::ZERO_PAGE, 3},	//0x45
{"LSR", ADDRESS_MODE::ZERO_PAGE, 5},	//0x46
{"BAD", ADDRESS_MODE::NONE, 2},			//0x47
{"PHA", ADDRESS_MODE::NONE, 3},			//0x48
{"EOR", ADDRESS_MODE::IMMEDIATE, 2},	//0x49
{"LSR", ADDRESS_MODE::NONE, 2},			//0x4a
{"BAD", ADDRESS_MODE::NONE, 2},			//0x4b
{"JMP", ADDRESS_MODE::ABSOLUTE, 3},		//0x4c
{"EOR", ADDRESS_MODE::ABSOLUTE, 4},		//0x4d
{"LSR", ADDRESS_MODE::ABSOLUTE, 6},		//0x4e
{"BAD", ADDRESS_MODE::NONE, 2},			//0x4f
{"BVC", ADDRESS_MODE::RELATIVE, 3},		//0x50
{"EOR", ADDRESS_MODE::INDIRECT_Y, 5},	//0x51
{"BAD", ADDRESS_MODE::NONE, 2},			//0x52
{"BAD", ADDRESS_MODE::NONE, 2},			//0x53
{"BAD", ADDRESS_MODE::NONE, 2},			//0x54
{"EOR", ADDRESS_MODE::ZERO_PAGE_X, 4},	//0x55
{"LSR", ADDRESS_MODE::ZERO_PAGE_X, 6},	//0x56
{"BAD", ADDRESS_MODE::NONE, 2},			//0x57
{"CLI", ADDRESS_MODE::NONE, 2},			//0x58
{"EOR", ADDRESS_MODE::ABSOLUTE_Y, 4},	//0x59
{"BAD", ADDRESS_MODE::NONE, 2},			//0x5a
{"BAD", ADDRESS_MODE::NONE, 2},			//0x5b
{"BAD", ADDRESS_MODE::NONE, 2},			//0x5c
{"EOR", ADDRESS_MODE::ABSOLUTE_X, 4},	//0x5d
{"LSR", ADDRESS_MODE::ABSOLUTE_X, 7},	//0x5e
{"BAD", ADDRESS_MODE::NONE, 2},			//0x5f
{"RTS", ADDRESS_MODE::NONE, 6},			//0x60
{"ADC", ADDRESS_MODE::INDIRECT_X, 6},	//0x61
{"BAD", ADDRESS_MODE::NONE, 2},			//0x62
{"BAD", ADDRESS_MODE::NONE, 2},			//0x63
{"BAD", ADDRESS_MODE::NONE, 2},			//0x64
{"ADC", ADDRESS_MODE::ZERO_PAGE, 3},	//0x65
{"ROR", ADDRESS_MODE::ZERO_PAGE, 5},	//0x66
{"BAD", ADDRESS_MODE::NONE, 2},			//0x67
{"PLA", ADDRESS_MODE::NONE, 4},			//0x68
{"ADC", ADDRESS_MODE::IMMEDIATE, 2},	//0x69
{"ROR", ADDRESS_MODE::NONE, 2},			//0x6a
{"BAD", ADDRESS_MODE::NONE, 2},			//0x6b
{"JMP", ADDRESS_MODE::INDIRECT, 5},		//0x6c
{"ADC", ADDRESS_MODE::ABSOLUTE, 4},		//0x6d
{"ROR", ADDRESS_MODE::ABSOLUTE, 6},		//0x6e
{"BAD", ADDRESS_MODE::NONE, 2},			//0x6f
{"BVS", ADDRESS_MODE::RELATIVE, 3},		//0x70
{"ADC", ADDRESS_MODE::INDIRECT_Y, 5},	//0x71
{"BAD", ADDRESS_MODE::NONE, 2},			//0x72
{"BAD", ADDRESS_MODE::NONE, 2},			//0x73
{"BAD", ADDRESS_MODE::NONE, 2},			//0x74
{"ADC", ADDRESS_MODE::ZERO_PAGE_X, 4},	//0x75
{"ROR", ADDRESS_MODE::ZERO_PAGE_X, 6},	//0x76
{"BAD", ADDRESS_MODE::NONE, 2},			//0x77
{"SEI", ADDRESS_MODE::NONE, 2},			//0x78
{"ADC", ADDRESS_MODE::ABSOLUTE_Y, 4},	//0x79
{"BAD", ADDRESS_MODE::NONE, 2},			//0x7a
{"BAD", ADDRESS_MODE::NONE, 2},			//0x7b
{"BAD", ADDRESS_MODE::NONE, 2},			//0x7c
{"ADC", ADDRESS_MODE::ABSOLUTE_X, 4},	//0x7d
{"ROR", ADDRESS_MODE::ABSOLUTE_X, 7},	//0x7e
{"BAD", ADDRESS_MODE::NONE, 2},			//0x7f
{"BAD", ADDRESS_MODE::NONE, 2},			//0x80
{"STA", ADDRESS_MODE::INDIRECT_X, 6},	//0x81
{"BAD", ADDRESS_MODE::NONE, 2},			//0x82
{"BAD", ADDRESS_MODE::NONE, 2},			//0x83
{"STY", ADDRESS_MODE::ZERO_PAGE, 3},	//0x84
{"STA", ADDRESS_MODE::ZERO_PAGE, 3},	//0x85
{"STX", ADDRESS_MODE::ZERO_PAGE, 3},	//0x86
{"BAD", ADDRESS_MODE::NONE, 2},			//0x87
{"DEY", ADDRESS_MODE::NONE, 2},			//0x88
{"BAD", ADDRESS_MODE::NONE, 2},			//0x89
{"TXA", ADDRESS_MODE::NONE, 2},			//0x8a
{"BAD", ADDRESS_MODE::NONE, 2},			//0x8b
{"STY", ADDRESS_MODE::ABSOLUTE, 4},		//0x8c
{"STA", ADDRESS_MODE::ABSOLUTE, 4},		//0x8d
{"STX", ADDRESS_MODE::ABSOLUTE, 4},		//0x8e
{"BAD", ADDRESS_MODE::NONE, 2},			//0x8f
{"BCC", ADDRESS_MODE::RELATIVE, 3},		//0x90
{"STA", ADDRESS_MODE::INDIRECT_Y, 6},	//0x91
{"BAD", ADDRESS_MODE::NONE, 2},			//0x92
{"BAD", ADDRESS_MODE::NONE, 2},			//0x93
{"STY", ADDRESS_MODE::ZERO_PAGE_X, 4},	//0x94
{"STA", ADDRESS_MODE::ZERO_PAGE_X, 4},	//0x95
{"STX", ADDRESS_MODE::ZERO_PAGE_Y, 4},	//0x96
{"BAD", ADDRESS_MODE::NONE, 2},			//0x97
{"TYA", ADDRESS_MODE::NONE, 2},			//0x98
{"STA", ADDRESS_MODE::ABSOLUTE_Y, 5},	//0x99
{"TXS", ADDRESS_MODE::NONE, 2},			//0x9a
{"BAD", ADDRESS_MODE::NONE, 2},			//0x9b
{"BAD", ADDRESS_MODE::NONE, 2},			//0x9c
{"STA", ADDRESS_MODE::ABSOLUTE_X, 5},	//0x9d
{"BAD", ADDRESS_MODE::NONE, 2},			//0x9e
{"BAD", ADDRESS_MODE::NONE, 2},			//0x9f
{"LDY", ADDRESS_MODE::IMMEDIATE, 2},	//0xa0
{"LDA", ADDRESS_MODE::INDIRECT_X, 6},	//0xa1
{"LDX", ADDRESS_MODE::IMMEDIATE, 2},	//0xa2
{"BAD", ADDRESS_MODE::NONE, 2},			//0xa3
{"LDY", ADDRESS_MODE::ZERO_PAGE, 3},	//0xa4
{"LDA", ADDRESS_MODE::ZERO_PAGE, 3},	//0xa5
{"LDX", ADDRESS_MODE::ZERO_PAGE, 3},	//0xa6
{"BAD", ADDRESS_MODE::NONE, 2},			//0xa7
{"TAY", ADDRESS_MODE::NONE, 2},			//0xa8
{"LDA", ADDRESS_MODE::IMMEDIATE, 2},	//0xa9
{"TAX", ADDRESS_MODE::NONE, 2},			//0xaa
{"BAD", ADDRESS_MODE::NONE, 2},			//0xab
{"LDY", ADDRESS_MODE::ABSOLUTE, 4},		//0xac
{"LDA", ADDRESS_MODE::ABSOLUTE, 4},		//0xad
{"LDX", ADDRESS_MODE::ABSOLUTE, 4},		//0xae
{"BAD", ADDRESS_MODE::NONE, 2},			//0xaf
{"BCS", ADDRESS_MODE::RELATIVE, 3},		//0xb0
{"LDA", ADDRESS_MODE::INDIRECT_Y, 5},	//0xb1
{"BAD", ADDRESS_MODE::NONE, 2},			//0xb2
{"BAD", ADDRESS_MODE::NONE, 2},			//0xb3
{"LDY", ADDRESS_MODE::ZERO_PAGE_X, 4},	//0xb4
{"LDA", ADDRESS_MODE::ZERO_PAGE_X, 4},	//0xb5
{"LDX", ADDRESS_MODE::ZERO_PAGE_Y, 4},	//0xb6
{"BAD", ADDRESS_MODE::NONE, 2},			//0xb7
{"CLV", ADDRESS_MODE::NONE, 2},			//0xb8
{"LDA", ADDRESS_MODE::ABSOLUTE_Y, 4},	//0xb9
{"TSX", ADDRESS_MODE::NONE, 2},			//0xba
{"BAD", ADDRESS_MODE::NONE, 2},			//0xbb
{"LDY", ADDRESS_MODE::ABSOLUTE_X, 4},	//0xbc
{"LDA", ADDRESS_MODE::ABSOLUTE_X, 4},	//0xbd
{"LDX", ADDRESS_MODE::ABSOLUTE_Y, 4},	//0xbe
{"BAD", ADDRESS_MODE::NONE, 2},			//0xbf
{"CPY", ADDRESS_MODE::IMMEDIATE, 2},	//0xc0
{"CMP", ADDRESS_MODE::INDIRECT_X, 6},	//0xc1
{"BAD", ADDRESS_MODE::NONE, 2},			//0xc2
{"BAD", ADDRESS_MODE::NONE, 2},			//0xc3
{"CPY", ADDRESS_MODE::ZERO_PAGE, 3},	//0xc4
{"CMP", ADDRESS_MODE::ZERO_PAGE, 3},	//0xc5
{"DEC", ADDRESS_MODE::ZERO_PAGE, 5},	//0xc6
{"BAD", ADDRESS_MODE::NONE, 2},			//0xc7
{"INY", ADDRESS_MODE::NONE, 2},			//0xc8
{"CMP", ADDRESS_MODE::IMMEDIATE, 2},	//0xc9
{"DEX", ADDRESS_MODE::NONE, 2},			//0xca
{"BAD", ADDRESS_MODE::NONE, 2},			//0xcb
{"CPY", ADDRESS_MODE::ABSOLUTE, 4},		//0xcc
{"CMP", ADDRESS_MODE::ABSOLUTE, 4},		//0xcd
{"DEC", ADDRESS_MODE::ABSOLUTE, 6},		//0xce
{"BAD", ADDRESS_MODE::NONE, 2},			//0xcf
{"BNE", ADDRESS_MODE::RELATIVE, 3},		//0xd0
{"CMP", ADDRESS_MODE::INDIRECT_Y, 5},	//0xd1
{"BAD", ADDRESS_MODE::NONE, 2},			//0xd2
{"BAD", ADDRESS_MODE::NONE, 2},			//0xd3
{"BAD", ADDRESS_MODE::NONE, 2},			//0xd4
{"CMP", ADDRESS_MODE::ZERO_PAGE_X, 4},	//0xd5
{"DEC", ADDRESS_MODE::ZERO_PAGE_X, 6},	//0xd6
{"BAD", ADDRESS_MODE::NONE, 2},			//0xd7
{"CLD", ADDRESS_MODE::NONE, 2},			//0xd8
{"CMP", ADDRESS_MODE::ABSOLUTE_Y, 4},	//0xd9
{"BAD", ADDRESS_MODE::NONE, 2},			//0xda
{"BAD", ADDRESS_MODE::NONE, 2},			//0xdb
{"BAD", ADDRESS_MODE::NONE, 2},			//0xdc
{"CMP", ADDRESS_MODE::ABSOLUTE_X, 4},	//0xdd
{"DEC", ADDRESS_MODE::ABSOLUTE_X, 7},	//0xde
{"BAD", ADDRESS_MODE::NONE, 2},			//0xdf
{"CPX", ADDRESS_MODE::IMMEDIATE, 2},	//0xe0
{"SBC", ADDRESS_MODE::INDIRECT_X, 6},	//0xe1
{"BAD", ADDRESS_MODE::NONE, 2},			//0xe2
{"BAD", ADDRESS_MODE::NONE, 2},			//0xe3
{"CPX", ADDRESS_MODE::ZERO_PAGE, 3},	//0xe4
{"SBC", ADDRESS_MODE::ZERO_PAGE, 3},	//0xe5
{"INC", ADDRESS_MODE::ZERO_PAGE, 5},	//0xe6
{"BAD", ADDRESS_MODE::NONE, 2},			//0xe7
{"INX", ADDRESS_MODE::NONE, 2},			//0xe8
{"SBC", ADDRESS_MODE::IMMEDIATE, 2},	//0xe9
{"NOP", ADDRESS_MODE::NONE, 2},			//0xea
{"BAD", ADDRESS_MODE::NONE, 2},			//0xeb
{"CPX", ADDRESS_MODE::ABSOLUTE, 4},		//0xec
{"SBC", ADDRESS_MODE::ABSOLUTE, 4},		//0xed
{"INC", ADDRESS_MODE::ABSOLUTE, 6},		//0xee
{"BAD", ADDRESS_MODE::NONE, 2},			//0xef
{"BEQ", ADDRESS_MODE::RELATIVE, 3},		//0xf0
{"SBC", ADDRESS_MODE::INDIRECT_Y, 5},	//0xf1
{"BAD", ADDRESS_MODE::NONE, 2},			//0xf2
{"BAD", ADDRESS_MODE::NONE, 2},			//0xf3
{"BAD", ADDRESS_MODE::NONE, 2},			//0xf4
{"SBC", ADDRESS_MODE::ZERO_PAGE_X, 4},	//0xf5
{"INC", ADDRESS_MODE::ZERO_PAGE_X, 6},	//0xf6
{"BAD", ADDRESS_MODE::NONE, 2},			//0xf7
{"SED", ADDRESS_MODE::NONE, 2},			//0xf8
{"SBC", ADDRESS_MODE::ABSOLUTE_Y, 4},	//0xf9
{"BAD", ADDRESS_MODE::NONE, 2},			//0xfa
{"BAD", ADDRESS_MODE::NONE, 2},			//0xfb
{"BAD", ADDRESS_MODE::NONE, 2},			//0xfc
{"SBC", ADDRESS_MODE::ABSOLUTE_X, 4},	//0xfd
{"INC", ADDRESS_MODE::ABSOLUTE_X, 7},	//0xfe
{"BAD", ADDRESS_MODE::NONE, 2}			//0xff
};

#if 0
//Determine address mode from opcode
// There are some outliers such as ZPY and ABSOLUTE (jsr) that don't fit in 1st 5 bits
//NONE = 0x0, 0x2, 0x8, 0xa, 0x12, 0x18, 0x1a
//IMMEDIATE = 0x9, 0xb
//ZERO_PAGE = 0x4, 0x5, 0x6, 0x7
//ZERO_PAGE_X = 0x14, 0x15, 0x16, 0x17
//ZERO_PAGE_Y = 0x96, 0x97, 0xb6, 0xb7
//INDIRECT_X = 0x1, 0x3
//INDIRECT_Y = 0x11, 0x13
//RELATIVE = 0x10
//ABSOLUTE = 0xc, 0xd, 0xe, 0xf, 0x20
//ABSOLUTE_X = 0x1c, 0x1d, 0x1e, 0x1f
//ABSOLUTE_Y = 0x19, 0x1b, 0x9e, 0x9f

ADDRESS_MODE OPCODEADDR[0x20] =
{
	ADDRESS_MODE::NONE,
	ADDRESS_MODE::INDIRECT_X,
	ADDRESS_MODE::NONE,
	ADDRESS_MODE::INDIRECT_X,
	ADDRESS_MODE::ZERO_PAGE,
	ADDRESS_MODE::ZERO_PAGE,
	ADDRESS_MODE::ZERO_PAGE,
	ADDRESS_MODE::ZERO_PAGE,
	ADDRESS_MODE::NONE,
	ADDRESS_MODE::IMMEDIATE,
	ADDRESS_MODE::NONE,
	ADDRESS_MODE::IMMEDIATE,
	ADDRESS_MODE::ABSOLUTE,
	ADDRESS_MODE::ABSOLUTE,
	ADDRESS_MODE::ABSOLUTE,
	ADDRESS_MODE::ABSOLUTE,
	ADDRESS_MODE::RELATIVE,
	ADDRESS_MODE::INDIRECT_Y,
	ADDRESS_MODE::NONE,
	ADDRESS_MODE::INDIRECT_Y,
	ADDRESS_MODE::ZERO_PAGE_X,
	ADDRESS_MODE::ZERO_PAGE_X,
	ADDRESS_MODE::ZERO_PAGE_X,
	ADDRESS_MODE::ZERO_PAGE_X,
	ADDRESS_MODE::NONE,
	ADDRESS_MODE::ABSOLUTE_Y,
	ADDRESS_MODE::NONE,
	ADDRESS_MODE::ABSOLUTE_Y,
	ADDRESS_MODE::ABSOLUTE_X,
	ADDRESS_MODE::ABSOLUTE_X,
	ADDRESS_MODE::ABSOLUTE_X,
	ADDRESS_MODE::ABSOLUTE_X
};

//----------------------------------------------------------------
///This code works but it takes more space than the direct values in the arrays
ADDRESS_MODE GetMode( uint8_t aOp )
{
	ADDRESS_MODE emode;

	switch (aOp) {
		case 0x20:								//Special case jsr
			emode = ADDRESS_MODE::ABSOLUTE;
			break;
		case 0x96:
		case 0x97:
		case 0xb6:
		case 0xb7:
			emode = ADDRESS_MODE::ZERO_PAGE_Y;
			break;
		default:
		{
			uint32_t index = aOp & 0x1f;
			emode = OPCODEADDR[index];
			//If special cases
			if (aOp & 0x80) {
				if (((index == 0x1e) || (index == 0x1f)) && ((aOp & 0xC0) == 0x80)) {
					emode = ADDRESS_MODE::ABSOLUTE_Y;
				}
				else if ((index == 0x00) || (index == 0x02)) {
					emode = ADDRESS_MODE::IMMEDIATE;
				}
			}
			break;
		}
	}

	return emode;
}

//----------------------------------------------------------------
uint32_t Test(  )
{
	uint32_t bad = 0;

	for ( uint8_t i = 0; i != 255; ++i) {
		auto m1 = GetMode(i);
		if (m1 != OpCodeA[i].eMode) {
			++bad;
		}
	}

	return bad;
}
#endif //0

//----------------------------------------------------------------
int32_t Match( const char *apDest, const char *apSource )
{
	while (*apSource) {
		int32_t dif = *apDest++ - *apSource++;
		if (dif) {
			return dif;
		}
	}

	return 0;
}

//----------------------------------------------------------------
uint8_t OpCode::Find( const char *apName, ADDRESS_MODE aeMode )
{
	//Make sure opcode chars are uppercase
	char upperName[3] = { Upper(apName[0]), Upper(apName[1]), Upper(apName[2])};

	uint8_t i = 0;
	//Loop through all opcodes to find name and address mode match
	for ( const auto &op : OpCodeA ) {
		if ((op == upperName) && (op == aeMode)) {
			return i;							//Found it!
		}
		++i;
	}
	return 0xff;								//Error
}

//----------------------------------------------------------------
const OpCode &OpCode::Get( uint8_t aIndex )
{
	return OpCodeA[aIndex];
}

//----------------------------------------------------------------
uint8_t OpCode::AddrModeSize( ADDRESS_MODE aeMode )
{
	return AddressModeSizeA[static_cast<uint32_t>(aeMode)];
}

//----------------------------------------------------------------
const char *OpCode::AddrModeName( ADDRESS_MODE aeMode )
{
	return AddressModeNameA[static_cast<uint32_t>(aeMode)];
}

//----------------------------------------------------------------
uint32_t OpCode::FindMatches( const char **apDest, uint32_t MaxEntries, const char *apString )
{
	uint32_t entries = 0;

	for ( auto n : OpNameA ) {
		auto res = Match(n, apString);
		if (res == 0) {
			apDest[entries++] = n;
			if (entries == MaxEntries) {
				break;
			}
		}
		else if (res > 0) {
			break;
		}
	}
	return entries;
}

//----------------------------------------------------------------
char Upper( char aChar )
{
	return ((aChar >= 'a') && (aChar <= 'z'))
		? (aChar - 'a') + 'A'
		: aChar;
}

//----------------------------------------------------------------
bool IsHex( char aChar )
{
	return ((aChar >= '0') && (aChar <= '9'))
		|| ((aChar >= 'a') && (aChar <= 'f'))
		|| ((aChar >= 'A') && (aChar <= 'F'));
}

