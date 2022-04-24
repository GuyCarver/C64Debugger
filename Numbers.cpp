//----------------------------------------------------------------------
// Copyright (c) 2022, Guy Carver
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution.
//
//     * The name of Guy Carver may not be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// FILE    Numbers.cpp
//----------------------------------------------------------------------

#include "Numbers.h"

//Note: Could use std::from_chars, to_chars, but this code is super simple and efficient

namespace Numbers
{

//----------------------------------------------------------------
uint32_t ToNum( char aChar )
{
	uint32_t v = 0;
	if ((aChar >= '0') && (aChar <= '9')) {
		v = aChar - '0';
	}
	else if ((aChar >= 'a') && (aChar <= 'f')) {
		v = aChar - 'a' + 0xa;
	}
	else if ((aChar >= 'A') && (aChar <= 'F')) {
		v = aChar - 'A' + 0xa;
	}
	return v;
}

//----------------------------------------------------------------
uint16_t HexToUInt16( const char *apString )
{
	return static_cast<uint16_t>(
		  ToNum(apString[0]) << 12
		| ToNum(apString[1]) << 8
		| ToNum(apString[2]) << 4
		| ToNum(apString[3]));
}

//----------------------------------------------------------------
uint8_t HexToUInt8( const char *apString )
{
	return static_cast<uint8_t>(
		  ToNum(apString[0]) << 4
		| ToNum(apString[1]));
}

//----------------------------------------------------------------
//Recursive function to convert a hex digit to a char and store in destination
void Digit( char *apDest, uint32_t aValue, uint32_t aIndex )
{
	constexpr const char phex[] = "0123456789ABCDEF";
	if (aIndex) {
		//Convert to char 0-9 or a-f
		apDest[--aIndex] = phex[aValue & 0x0f];
		Digit(apDest, aValue >> 4, aIndex);		// Tail recursion
	}
}

//----------------------------------------------------------------
void ToHex( char *apDest, uint8_t aValue )
{
	apDest[2] = 0;								// Ensure null termination
	Digit(apDest, aValue, 2);					// Recurse and set 2 digits
}

//----------------------------------------------------------------
void ToHex( char *apDest, uint16_t aValue )
{
	apDest[4] = 0;								// Ensure null termination
	Digit(apDest, aValue, 4);					// Recurse and set 4 digits
}

//----------------------------------------------------------------
void ToHex( char *apDest, uint32_t aValue )
{
	apDest[8] = 0;								// Ensure null termination
	Digit(apDest, aValue, 8);					// Recurse and set 8 digits
}

//----------------------------------------------------------------
///Convert binary stream of given size to hex string and add cr at 16 char interval
template<class T>
uint32_t ToHexT( char *apDest, uint32_t aDestLen, const T *apSource, uint32_t aSourceLen )
{
	uint32_t written = 0;

	const uint32_t numchars = sizeof(T) * 2;

	//Calculate how many elements will fit into the destination buffer
	//1 off the destination for the null terminator
	uint32_t fit = (aDestLen - 1) / (numchars + 1);

	//Clamp to minimum of aSourceLen or fit so we don't write more chars than dest can hold
	if (aSourceLen > fit) {
		aSourceLen = fit;
	}

	//If anything to convert
	if (aSourceLen) {
		//Loop for each value and convert to apDest
		for ( uint32_t i = 0; i < aSourceLen; ++i) {
			ToHex(apDest + written, apSource[i]);
			written += numchars;				// Next position
			apDest[written++] = (i + 1) & 0xF ? ' ' : '\n';	// Space in between
		}
	}

	apDest[written] = 0;						// Null terminate stringf

	return written;
}

//----------------------------------------------------------------
uint32_t ToHex( char *apDest, uint32_t aDestLen, const uint8_t *apSource, uint32_t aSourceLen )
{
	return ToHexT(apDest, aDestLen, apSource, aSourceLen);
}

//----------------------------------------------------------------
uint32_t ToHex( char *apDest, uint32_t aDestLen, const uint16_t *apSource, uint32_t aSourceLen )
{
	return ToHexT(apDest, aDestLen, apSource, aSourceLen);
}

//----------------------------------------------------------------
uint32_t ToAscii( char *apDest, uint32_t aDestLen, const uint8_t *apSource, uint32_t aSourceLen )
{
	//Return displayable ascii or .
	auto getchr = []( uint8_t V ) {
		return (V >= 0x20) ? static_cast<char>(V) : '.';
//		return ((V >= 0x20) && (V <= 0x7e)) ? static_cast<char>(V) : '.';
	};

	uint32_t fit = aSourceLen <= aDestLen ? aSourceLen : aDestLen;
	uint32_t res = fit;

	while(fit--) {
		*apDest++ = getchr(*apSource++);
	}
	*apDest = 0;								//Null terminate

	return res;
}

}	//namespace Numbers
