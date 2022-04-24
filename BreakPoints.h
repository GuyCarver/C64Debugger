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
// FILE    BreakPoints.h
//----------------------------------------------------------------------

#pragma once

#include "types.h"
#include "json/json.hpp"
#include <functional>

class Response;

namespace BreakPoints
{
	//Return true to continue processing, false to stop
	using BREAKPOINTFN = std::function<bool(uint16_t aAddress, bool bEnabled)>;

	//----------------------------------------------------------------
	///Call given function for each BreakPoint
	/// Returns result of aFunction call
	bool ForEach( BREAKPOINTFN aFunction );

	//----------------------------------------------------------------
	///Returns true if a breakpoint for given address exists and is enabled
	bool CheckHit( uint16_t aAddress );

	//----------------------------------------------------------------
	///Add a BreakPoint for given address
	void Add( uint16_t aAddress );

	//----------------------------------------------------------------
	///Add a memory range BreakPoint or CheckPoint
	void Add( uint16_t aStartAddress, uint16_t aEndAddress, uint8_t aBreak );

	//----------------------------------------------------------------
	///Remove breakpoint at given address
	void Remove( uint16_t aAddress );

	//----------------------------------------------------------------
	///Enable/Disable breakpoint at given address
	void Enable( uint16_t aAddress, bool abTF = true );

	//----------------------------------------------------------------
	///Toggle breakpoint at given address
	void Toggle( uint16_t aAddress );

	//----------------------------------------------------------------
	///Delete all breakpoints and empty map
	void Close(  );

	//----------------------------------------------------------------
	///Process breakpoint responses
	void ProcessInfo( const Response &arResponse );

	//----------------------------------------------------------------
	///Display view of BreakPoints
	void Display(  );

	//----------------------------------------------------------------
	///Save settings to json
	void ToJson( nlohmann::json &arData );

	//----------------------------------------------------------------
	///Load settings from json
	void FromJson( nlohmann::json &arData );

	//----------------------------------------------------------------
	///Enable display
	void DisplayOn(  );

}	//namespace BreakPoints
