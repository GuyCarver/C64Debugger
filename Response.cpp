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
// FILE    Response.cpp
//----------------------------------------------------------------------

#include "Response.h"

//----------------------------------------------------------------
///Used for verifying responses
COMMAND CommandList[] =
{
	COMMAND::NONE,
	COMMAND::MEMORY_GET,
	COMMAND::MEMORY_SET,
	COMMAND::CHECKPOINT_GET,
	COMMAND::CHECKPOINT_SET,
	COMMAND::CHECKPOINT_DEL,
	COMMAND::CHECKPOINT_LST,
	COMMAND::CHECKPOINT_TGL,
	COMMAND::CONDITION_SET,
	COMMAND::REGISTERS_GET,
	COMMAND::REGISTERS_SET,
	COMMAND::DUMP,
	COMMAND::UNDUMP,
	COMMAND::RESOURCE_GET,
	COMMAND::RESOURCE_SET,
	COMMAND::JAM,
	COMMAND::STOPPED,
	COMMAND::RESUMED,
	COMMAND::ADVANCE,
	COMMAND::KEYBOARD,
	COMMAND::STEP_OUT,
	COMMAND::PING,
	COMMAND::BANKS_AVAIL,
	COMMAND::REGISTERS_AVAIL,
	COMMAND::DISPLAY_GET,
	COMMAND::VICE_INFO,
	COMMAND::PALETTE_GET,
	COMMAND::JOYPORT_SET,
	COMMAND::USERPORT_SET,
	COMMAND::EXIT,
	COMMAND::QUIT,
	COMMAND::RESET,
	COMMAND::AUTOSTART
};

//----------------------------------------------------------------
Response::Response( const Response &arFrom )
{
	memcpy(this, &arFrom, arFrom.QSize());
}

//----------------------------------------------------------------
const Response *Response::Clone(  ) const
{
	//Allocate memory to hold body and header of Response
	uint8_t *pbuffer = new uint8_t[QSize()];
	//Copy constructor into buffer
	auto *pnew = new (pbuffer) Response(*this);
	return pnew;
}

//----------------------------------------------------------------
bool Response::LooksGood(  ) const
{
	bool bres = false;

	//We assume no requests of greater that 0x200 bytes
	// That will not work if we support DISPLAY_GET
	if ((Header == HEADER) && (QSize() < 0x200)) {
		for ( const auto c : CommandList ) {
			if (Cmd == c) {
				bres = true;
				break;
			}
		}
	}

	return bres;
}

//----------------------------------------------------------------
const ProcessRes Response::Process( const uint8_t *apBuffer, uint32_t aLen )
{
	ProcessRes res;

	if (aLen >= sizeof(Response)) {
		//Attempt to consume the whole buffer if necessary
		for ( uint32_t i = 0; i < aLen - sizeof(Response); ++i) {
			auto &possible = FromBuffer(&apBuffer[i]);
			if (possible.LooksGood()) {
				if (possible.BodyLen <= aLen) {
					res.pResponse = ResponsePtr(possible.Clone());
					res.Next = i + res.pResponse->QSize();
					break;							// Exit loop
				}
			}
		}
	}

	return res;
}
