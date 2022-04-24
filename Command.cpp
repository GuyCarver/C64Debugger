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
// FILE    Command.cpp
//----------------------------------------------------------------------

#include "Command.h"
#include <iostream>								// For placement new

// Start ID's after CommandIDs so they can be used as the CommandRegisty key without conflict
uint32_t Command::RequestIDS = 0x100;

//Cmd, UniqueID, value
const CommandPtr Command::GetRegsCommand(new Command(COMMAND::REGISTERS_GET, NOID, 0));
const CommandPtr Command::ExitCommand(new Command(COMMAND::EXIT, 1));
const CommandPtr Command::PingCommand(new Command(COMMAND::PING, 2));
const CommandPtr Command::QuitCommand(new Command(COMMAND::QUIT, 3));
const CommandPtr Command::SoftResetCommand(new Command(COMMAND::RESET, 4, 0));
const CommandPtr Command::HardResetCommand(new Command(COMMAND::RESET, 5, 1));
const CommandPtr Command::RegsAvailCommand(new Command(COMMAND::REGISTERS_AVAIL, 6, 0));
const CommandPtr Command::CheckpointListCommand(new Command(COMMAND::CHECKPOINT_LST, 7));

//----------------------------------------------------------------
CommandPtr Command::Create( COMMAND aCmd, uint32_t aSize, uint32_t aID  )
{
	uint8_t *pcmd = new uint8_t[sizeof(Command) + aSize - sizeof(MaxSize) - DEFBODYLEN];
	CommandPtr pcommand = CommandPtr(new (pcmd) Command(aCmd, aID));
	pcommand->SetMaxSize(aSize);
	return pcommand;
}

//----------------------------------------------------------------
uint32_t Command::NextID(  )
{
	if (++RequestIDS < 0x100) {
		RequestIDS = 0x100;
	}
	return RequestIDS;
}

//----------------------------------------------------------------
void Command::Add( uint8_t aByte )
{
	if (BodyLen < MaxSize) {
		Body[BodyLen++] = aByte;
	}
}

//----------------------------------------------------------------
void Command::Add( uint16_t aWord )
{
	Add(static_cast<uint8_t>(aWord));
	Add(static_cast<uint8_t>(aWord >> 8));
}

//----------------------------------------------------------------
void Command::Add( uint32_t aLong )
{
	Add(static_cast<uint16_t>(aLong));
	Add(static_cast<uint16_t>(aLong >> 16));
}

//----------------------------------------------------------------
void Command::Add( uint8_t *apData, uint32_t aLength  )
{
	if (BodyLen + aLength <= MaxSize) {
		memcpy_s(&Body[BodyLen], aLength, apData, aLength);
		BodyLen += aLength;
	}
}

//----------------------------------------------------------------
void Command::Add( const char *apString )
{
	uint8_t c = 1;
	while (c && (BodyLen < MaxSize)) {
		c = *apString++;
		Add(c);
	}
}

