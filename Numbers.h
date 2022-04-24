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
// FILE    Numbers.h
//----------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace Numbers
{

//----------------------------------------------------------------
//Convert char to a single hex digit
uint32_t ToNum( char aChar );

//----------------------------------------------------------------
/// Convert 4 char hex string into a 16 bit value
uint16_t HexToUInt16( const char *apString );

//----------------------------------------------------------------
/// Convert a 2 char hex string into an 8 bit value
uint8_t HexToUInt8( const char *apString );

//----------------------------------------------------------------
/// Convert 8 bit value to a 2 char string
void ToHex( char *apDest, uint8_t aValue );

//----------------------------------------------------------------
/// Convert a 16 bit value to a 4 char string
void ToHex( char *apDest, uint16_t aValue );

//----------------------------------------------------------------
/// Convert a 32 bit value to a 4 char string
void ToHex( char *apDest, uint32_t aValue );

//----------------------------------------------------------------
/// Convert stream of uint8_t to hex string - Returns number of chars written
uint32_t ToHex( char *apDest, uint32_t aDestLen, const uint8_t *apSource, uint32_t aSourceLen );

//----------------------------------------------------------------
/// Convert stream of uint16_t to hex string - Returns number of chars written
uint32_t ToHex( char *apDest, uint32_t aDestLen, const uint16_t *apSource, uint32_t aSourceLen );

//----------------------------------------------------------------
/// Convert stream of uint8_t to ascii or '.'' - Returns number of chars written
uint32_t ToAscii( char *apDest, uint32_t aDestLen, const uint8_t *apSource, uint32_t aSourceLen );

}	//namespace Numbers
