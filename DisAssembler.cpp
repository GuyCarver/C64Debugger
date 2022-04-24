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
// FILE    DisAssembler.cpp
//----------------------------------------------------------------------

#include "DisAssembler.h"
#include "Numbers.h"
#include "Labels.h"

namespace DisAssembler
{

//----------------------------------------------------------------
///On construction, parse source data into destination
/// address string buffer and disassembly string buffer
/// Only process the desired number of lines
class Parser
{
public:
	Parser(  ) = delete;

	//----------------------------------------------------------------
	explicit Parser( const Input &arInput )
	: pDest(arInput.pDest)
	, pStart(arInput.pDest)
	, pBytes(arInput.pBytes)
	, pOpCodes(arInput.pOpCodes)
	, pSource(arInput.pSource)
	, Lines(arInput.Lines)
	, DestLen(arInput.DestLen)
	, SourceLen(arInput.SourceLen)
	, Address(arInput.Address)
	{
		Parse();
	}

	//----------------------------------------------------------------
	bool QGood(  ) const { return bGood; }

	//NOTE: Other that pDest and pSource, we assume array sizes are large
	// enough to hold the data as the sizes are fixed

	char *pDest;								//Destination buffer
	char *pStart;								//Pointer to start of destination buffer
	char *pBytes;								//Memory for raw byte hex strings
	const uint8_t *pSource;						//Source buffer
	const OpCode **pOpCodes;					//OpCode array
	uint32_t Lines;								//Number of lines to parse
	uint32_t DestLen;							//Length of dest buffer
	uint32_t SourceLen;							//Length of source buffer
	uint16_t Address;							//C64 Address
	bool bGood = true;

	//----------------------------------------------------------------
	///Add char to the destination buffer
	bool AddChr( char aChar )
	{
		if (bGood = DestLen > 0; bGood) {
			*pDest++ = aChar;
			--DestLen;
		}
		return bGood;
	}

	//----------------------------------------------------------------
	///Add string to the destination buffer without the null terminator
	bool AddStr( const char *apStr )
	{
		while (bGood && *apStr) {
			AddChr(*apStr++);
		}
		return bGood;
	}

	//----------------------------------------------------------------
	///Add 16 bit value to destination surrounded by given prefix/suffix
	bool Add16( uint16_t aAddr, const char *apPrefix, const char *apSuffix = nullptr )
	{
		//If a prefix then add it
		if (bGood && apPrefix) {
			AddStr(apPrefix);
		}

		//If good and enough room in dest
		if (bGood = DestLen >= 4; bGood) {
			Numbers::ToHex(pDest, aAddr);
			pDest += 4;
			DestLen -= 4;

			//If a suffix then add it
			if (apSuffix) {
				AddStr(apSuffix);
			}
		}
		return bGood;
	}

	//----------------------------------------------------------------
	///Get value from source
	template<class T>
	bool Get( T &aValue )
	{
		if (bGood = SourceLen > 0; bGood) {
			aValue = *reinterpret_cast<const T*>(pSource);
			pSource += sizeof(T);
			Address += sizeof(T);
			SourceLen -= sizeof(T);
			ToBytesStr(aValue);
		}
		return bGood;
	}

	//----------------------------------------------------------------
	///Add a label for the given value to the output stream
	/// Return true if found and added
	bool AddLabel( uint16_t aValue, const char *apSuffix = nullptr )
	{
		const char *plabel = Labels::Find(aValue);
		if (plabel) {
			AddStr(plabel);

			//If still good and a suffix was supplied, add it as well
			if (bGood && apSuffix) {
				AddStr(apSuffix);
			}
		}
		return (plabel != nullptr);
	}

	//----------------------------------------------------------------
	///Add 16 bit address to dest with given suffix
	bool AddAddr( const char *apSuffix )
	{
		uint16_t addr;
		if (Get(addr)) {
			//If didn't add a label, simply had the hex value
			if (!AddLabel(addr, apSuffix)) {
				Add16(addr, "$", apSuffix );
			}
		}
		return bGood;
	}

	//----------------------------------------------------------------
	///Add 8 bit value to destination surrounded by given prefix/suffix
	bool AddByte( const char *apPrefix, const char *apSuffix = "\n" )
	{
		//If good and prefix given, add it
		if (bGood && apPrefix) {
			AddStr(apPrefix);
		}

		//If still good after potentially adding a prefix
		if (bGood) {
			uint8_t zp;
			if (Get(zp)) {
				//If didn't add a label and room for at least 2 chars
				if (!AddLabel(zp, apSuffix)) {
					if (bGood = DestLen > 1; bGood) {
						Numbers::ToHex(pDest, zp);
						pDest += 2;
						DestLen -= 2;
						//If suffix then add it
						if (apSuffix) {
							AddStr(apSuffix);
						}
					}
				}
			}
		}
		return bGood;
	}

	//----------------------------------------------------------------
	///Add data to the opcode based on address mode
	bool AddMode( ADDRESS_MODE aeMode )
	{
		//If address mode has more than 1 byte
		if (OpCode::AddrModeSize(aeMode) > 1) {
			//If any label points to memory after the opcode, add it
			AddLabel(Address, ": ");
		}

		switch (aeMode) {
			case ADDRESS_MODE::NONE:
				*(pDest - 1) = '\n';			//Replace space after opcode with cr
				break;
			case ADDRESS_MODE::IMMEDIATE:
				AddByte("#$");					//#$??
				break;
			case ADDRESS_MODE::ZERO_PAGE:
				AddByte("$");					//$??
				break;
			case ADDRESS_MODE::ZERO_PAGE_X:
				AddByte("$", ",X\n");			//$??,X
				break;
			case ADDRESS_MODE::ZERO_PAGE_Y:
				AddByte("$", ",Y\n");			//$??,Y
				break;
			case ADDRESS_MODE::INDIRECT_X:
				AddByte("($", ",X)\n");			//($??,X)
				break;
			case ADDRESS_MODE::INDIRECT_Y:
				AddByte("($", "),Y\n");			//($??),Y
				break;
			case ADDRESS_MODE::RELATIVE:
			{
				int8_t rel;
				//Get 8 bit relative offset and add it to current address
				if (Get(rel)) {
					uint16_t addr = Address + rel;
					Add16(addr, "$", "\n");
				}
				break;
			}
			case ADDRESS_MODE::ABSOLUTE:
				AddAddr("\n");
				break;
			case ADDRESS_MODE::ABSOLUTE_X:
				AddAddr(",X\n");				//$????,X
				break;
			case ADDRESS_MODE::ABSOLUTE_Y:
				AddAddr(",Y\n");				//$????,Y
				break;
			default:
				bGood = false;
				break;
		}
		return bGood;
	}

	//----------------------------------------------------------------
	///Add bytes to raw byte display string followed by space
	template<class T>
	void ToBytesStr( T aValue )
	{
		const uint8_t *psrc = reinterpret_cast<const uint8_t*>(&aValue);
		for ( uint32_t i = 0; i < sizeof(aValue); ++i) {
			Numbers::ToHex(pBytes, *psrc++);
			pBytes += 2;
			*pBytes++ = ' ';
		}
	}

	//----------------------------------------------------------------
	///Parse the input data into the disassembly view
	void Parse(  )
	{
		//Loop while good and we have data or lines
		uint32_t i = 0;
		//Loop for desired number of lines or until we run out of source
		// data or hit an unrecoverable error
		for ( i = 0; bGood && SourceLen && (i < Lines); ++i) {
			uint8_t op;
			//If got op from source
			if (Get(op)) {
				//Add op name
				auto &ropCode = OpCode::Get(op);
				pOpCodes[i] = &ropCode;			//Store opcode for line
				if (AddStr(ropCode.pName)) {
					if (AddChr(' ')) {			//Separate with space
						AddMode(ropCode.eMode);	//Add rest of op
					}
				}
			}
			*(pBytes - 1) = '\n';				//Turn last ' ' into \n
																// This will be used for replacement during editing
		}
		//We assume at least 1 byte was written to all of these destinations
		// if i > 0.
		if (i) {
			*(pDest - 1) = 0;					//Null Terminate Disassembly view
			*(pBytes - 1) = 0;
		}

		//Make sure if we didn't finish to fill out some data
		const OpCode *pbad = &OpCode::Get(0xff);
		for ( ; i < Lines; ++i) {
			pOpCodes[i] = pbad;
		}
	}
};

//----------------------------------------------------------------
uint16_t DisAssemble( const Input &arInput )
{
	Parser p(arInput);
	return p.Address;
}

}	//namespace DisAssembler
