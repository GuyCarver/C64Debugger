
#pragma once

#include "6502.h"

namespace Assembler
{

//----------------------------------------------------------------
class Assemble
{
public:
	Assemble(  ) = default;

	//----------------------------------------------------------------
	///Constructor to assemble from given string. See FromString()
	explicit Assemble( const char *apSource, uint32_t aSourceLen, uint16_t aAddress );

	//----------------------------------------------------------------
	///Returns true if Len is not 0
	bool QGood(  ) const { return Len; }

	//----------------------------------------------------------------
	///Assemble given string. aAddress is used to calculate relative addressing
	bool FromString( const char *apSource, uint32_t aSourceLen, uint16_t aAddress );

	//----------------------------------------------------------------
	///Copy compiled opcode to destination buffer and return
	/// number of bytes written
	uint8_t ToDest( uint8_t *apDest, uint32_t aDestLen );

	//----------------------------------------------------------------
	///Print compiled opcode to destination buffer and return
	/// number of chars written
	uint8_t Print( char *apDest, uint32_t aDestLen );

//private:
	uint8_t OP = 0;								//Opcode
	uint8_t B0 = 0;								//Operand bytes
	uint8_t B1 = 0;
	uint8_t Len = 0;							//Length of op 1-3, 0 if error
};

}	//namespace Assembler
