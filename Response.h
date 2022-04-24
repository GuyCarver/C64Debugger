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
// FILE    Response.h
//----------------------------------------------------------------------

#pragma once

#include "types.h"
#include <memory>								// For shared_ptr

/*{
[ ] Add Read() functions to read the body


}*/

//----------------------------------------------------------------
//Error Codes
enum class ERRORCODES : uint8_t {
	EC_OK =           0x00,
	EC_NOEXIST =      0x01,
	EC_MEMINVALID =   0x02,
	EC_BADCMDLEN =    0x80,
	EC_BADPARAM =     0x81,
	EC_BADVERSION =   0x82,
	EC_BADCMD =       0x83,
	EC_BADRESPONSE =  0x84,
	EC_GENFAIL =      0x8f
};

struct ProcessRes;								// Forward declare

//Pack the headers with no padding so they match the VICE data format
#pragma pack(push, 1)
//----------------------------------------------------------------
class Response
{
public:
	//----------------------------------------------------------------
	Response( ) = default;

	//----------------------------------------------------------------
	explicit Response( const Response &arFrom );

	//----------------------------------------------------------------
	const Response *Clone(  ) const;

	//----------------------------------------------------------------
	//If -1 or 0 then no command ID (Not a response to specific sent command)
	bool HasCommand(  ) const
	{ return ((RequestID != 0xFFFFFFFF) && (RequestID != 0)); }

	//----------------------------------------------------------------
	uint32_t QSize(  ) const { return (BodyLen + sizeof(Response) - 1); }

	//----------------------------------------------------------------
	static const Response &FromBuffer( const uint8_t *apBuffer )
	{ return reinterpret_cast<const Response&>(*apBuffer); }

	//----------------------------------------------------------------
	uint32_t QBodyLen(  ) const { return BodyLen; }

	//----------------------------------------------------------------
	COMMAND QCommand(  ) const { return Cmd; }

	//----------------------------------------------------------------
	ERRORCODES QError(  ) const { return ErrorCode; }

	//----------------------------------------------------------------
	uint32_t QID(  ) const { return RequestID; }

	//----------------------------------------------------------------
	const uint8_t *QBody(  ) const { return Body; }

	//----------------------------------------------------------------
	template <typename T> T Get( uint32_t aIndex ) const
	{ return *(reinterpret_cast<const T*>(&Body[aIndex])); }

	//----------------------------------------------------------------
	uint8_t Get8( uint32_t aIndex ) const { return Get<uint8_t>(aIndex); }

	//----------------------------------------------------------------
	uint16_t Get16( uint32_t aIndex ) const { return Get<uint16_t>(aIndex); }

	//----------------------------------------------------------------
	/// Attempt to ensure the response data is good to throw out corrupted
	///  data.
	bool LooksGood(  ) const;

	//----------------------------------------------------------------
	static const ProcessRes Process( const uint8_t *apBuffer, uint32_t aLen );

	//----------------------------------------------------------------
private:
	uint16_t Header = HEADER;
	uint32_t BodyLen = 0;						// Length of the body not including this header
	COMMAND Cmd = COMMAND::NONE;				// Command type response is for
	ERRORCODES ErrorCode = ERRORCODES::EC_OK;
	uint32_t RequestID = 0;						// ID matching that of the command that triggered the response
												// 0 = No matching command
	uint8_t Body[1];							// Variable length body of the response
};
#pragma pack(pop)

//----------------------------------------------------------------
using ResponsePtr = std::shared_ptr<const Response>;

//----------------------------------------------------------------
struct ProcessRes
{
	ResponsePtr pResponse;						//Pointer to response
	uint32_t Next = 0;							//Index to next position in buffer
};
