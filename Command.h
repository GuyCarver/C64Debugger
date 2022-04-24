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
// FILE    Command.h
//----------------------------------------------------------------------

#pragma once

#include "types.h"
#include <memory>

constexpr uint32_t NOID 			= 0xFFFFFFFF;

//Default maximum size of command body. Most commands fit within this size.
// If they do not the Command should be initialized with a memory block large
// enough for the command:
//	uint8_t *pcmd = new uint8_t[sizeof(Command) + 32];
//	pCommand = CommandPtr(new (pcmd) Command(COMMAND::CHECKPOINT_SET));
constexpr uint32_t DEFBODYLEN		= 0x20;

class Command;

using CommandPtr = std::shared_ptr<Command>;

//Pack class with no padding so they match the VICE data format.
#pragma pack(push, 1)
//----------------------------------------------------------------
class Command
{
public:

	//----------------------------------------------------------------
	explicit Command( COMMAND aCmd, uint32_t aID = 0 )
	: RequestID(aID ? aID : NextID())
	, Cmd(aCmd)
	{  }

	//----------------------------------------------------------------
	///Constructor for a command with a single value
	explicit Command( COMMAND aCmd, uint32_t aID, uint8_t aValue ) : Command(aCmd, aID)
	{
		Add(aValue);
	}

	//----------------------------------------------------------------
	static CommandPtr Create( COMMAND aCmd, uint32_t aSize, uint32_t aID = 0 );

	//----------------------------------------------------------------
	///Get maximum size the buffer may hold. May be larger the declared
	/// due to Placement new allocation to create a larger buffer.
	uint32_t QMaxSize(  ) const { return MaxSize; }

	//----------------------------------------------------------------
	///Set maximum size the buffer for this command may contain.
	void SetMaxSize( uint32_t aSize ) { MaxSize = aSize >= DEFBODYLEN ? aSize : DEFBODYLEN; }

	//----------------------------------------------------------------
	///Get size of the command to send by subtracting the Command members
	uint32_t QSize(  ) const
	{ return (BodyLen + sizeof(Command) - DEFBODYLEN - sizeof(MaxSize)); }

	//----------------------------------------------------------------
	///Get number of bytes in the body
	uint32_t QBodyLen(  ) const { return BodyLen; }

	//----------------------------------------------------------------
	///Reset the body length for recalculation
	void Reset(  ) { BodyLen = 0;}

	//----------------------------------------------------------------
	///Get the ID for this command
	uint32_t QID(  ) const { return RequestID; }

	//----------------------------------------------------------------
	///Get VICE command ID
	COMMAND QCommand(  ) const { return Cmd; }

	//----------------------------------------------------------------
	///Set VICE command ID
	void SetCommand( COMMAND aCommand ) { Cmd = aCommand; }

	//----------------------------------------------------------------
	///Get pointer to the command body
	const uint8_t *QBody(  ) const { return Body; }

	//----------------------------------------------------------------
	///Get command as buffer to send to VICE
	const char *AsBuffer(  ) const { return reinterpret_cast<const char*>(&Header); }

	//----------------------------------------------------------------
	///Add a byte to the body
	void Add( uint8_t aByte );

	//----------------------------------------------------------------
	///Add a word to the body
	void Add( uint16_t aWord );

	//----------------------------------------------------------------
	///Add a long to the body
	void Add( uint32_t aLong );

	//----------------------------------------------------------------
	///Add stream of bytes to the body
	void Add( uint8_t *apData, uint32_t aLength );

	//----------------------------------------------------------------
	///Add string to body
	void Add( const char *apString );

	//----------------------------------------------------------------
	///Index into the body
	uint8_t& operator[]( uint32_t aIndex ) { return Body[aIndex]; }

	//Static common commands
	static const CommandPtr GetRegsCommand;
	static const CommandPtr RegsAvailCommand;
	static const CommandPtr CheckpointListCommand;
	static const CommandPtr ExitCommand;
	static const CommandPtr PingCommand;
	static const CommandPtr QuitCommand;
	static const CommandPtr SoftResetCommand;
	static const CommandPtr HardResetCommand;

private:
	static uint32_t RequestIDS;					//Unique Id used to pair response with command

	uint32_t MaxSize = DEFBODYLEN;
	uint16_t Header = HEADER;
	uint32_t BodyLen = 0;						// Length of the body not including this header
	uint32_t RequestID;							// Unique ID used to pair respones with commands
	COMMAND Cmd;								// Command ID
protected:
	uint8_t Body[DEFBODYLEN] = { 0 };			// Body of command if any
												// Note that commands with a body still
												//  have this entry and it's initialized to 0
												//  but not included in QSize() value

	static uint32_t NextID(  );
};
#pragma pack(pop)

