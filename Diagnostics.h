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
// FILE    Diagnostics.h
//----------------------------------------------------------------------

#pragma once

#include "Response.h"
#include "Command.h"
#include "json/json.hpp"

///Diagnostics system to record communications with VICE
/// Displays a record of send commands and received responses
namespace Diagnostics
{
	//----------------------------------------------------------------
	///Turn on the display
	void DisplayOn(  );

	//----------------------------------------------------------------
	///Save settings to json
	void ToJson( nlohmann::json &arData );

	//----------------------------------------------------------------
	///Load settings from json
	void FromJson( nlohmann::json &arData );

	//----------------------------------------------------------------
	///Display Diagnostics view
	void Display(  );

	//----------------------------------------------------------------
	///Add command data to display
	void AddCommand( const Command &arCommand );

	//----------------------------------------------------------------
	///Add response data to display
	void AddResponse( ResponsePtr apResponse );

	//----------------------------------------------------------------
	///Add text
	void AddText( const char *apText );

}	//namespace Diagnostics
