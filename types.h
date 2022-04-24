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
// FILE    types.h
//----------------------------------------------------------------------

#pragma once

#include <cstdint>

//In additions to winsock, this also defines some core shared values for VICE communication

inline constexpr uint8_t operator "" _u8( unsigned long long int aValue ) noexcept
{ return static_cast<uint8_t>(aValue); }

inline constexpr uint16_t operator "" _u16( unsigned long long int aValue ) noexcept
{ return static_cast<uint16_t>(aValue); }

inline constexpr uint32_t operator "" _bit( unsigned long long int aValue ) noexcept
{ return static_cast<uint32_t>(1 << aValue); }

constexpr uint32_t bit( uint32_t aShift ) { return (1 << aShift); }

//VICE Command/Response header
constexpr uint16_t HEADER = 0x0202;

//----------------------------------------------------------------
/// VICE command and response IDs
enum class COMMAND : uint8_t
{
	NONE			= 0x00,
	MEMORY_GET		= 0x01,
	MEMORY_SET		= 0x02,
	CHECKPOINT_GET	= 0x11,
	CHECKPOINT_INFO	= 0x11,						//Response for all CHECKPOINT_??? commands
	CHECKPOINT_SET	= 0x12,
	CHECKPOINT_DEL	= 0x13,
	CHECKPOINT_LST	= 0x14,
	CHECKPOINT_TGL	= 0x15,
	CONDITION_SET	= 0x22,
	REGISTERS_GET	= 0x31,
	REGISTERS_SET	= 0x32,
	DUMP			= 0x41,
	UNDUMP			= 0x42,
	RESOURCE_GET	= 0x51,
	RESOURCE_SET	= 0x52,
	JAM				= 0x61,						//Response
	STOPPED			= 0x62,						//Response
	RESUMED			= 0x63,						//Response
	ADVANCE			= 0x71,
	KEYBOARD		= 0x72,
	STEP_OUT		= 0x73,
	PING			= 0x81,
	BANKS_AVAIL		= 0x82,
	REGISTERS_AVAIL	= 0x83,
	DISPLAY_GET		= 0x84,
	VICE_INFO		= 0x85,
	PALETTE_GET		= 0x91,
	JOYPORT_SET		= 0xa2,
	USERPORT_SET	= 0xb2,
	EXIT			= 0xaa,
	QUIT			= 0xbb,
	RESET			= 0xcc,
	AUTOSTART		= 0xdd
};

