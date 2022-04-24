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
// FILE    Monitor.h
//----------------------------------------------------------------------

#pragma once

#include "types.h"
#include "Command.h"
#include "json/json.hpp"

struct ImFont;

//----------------------------------------------------------------
enum VICESTATE
{
	DISCONNECTED,
	RUNNING,
	STOPPED
};

namespace Monitor
{
	//----------------------------------------------------------------
	///Get current state of VICE
	VICESTATE ViceState(  );

	//----------------------------------------------------------------
	///Initialize Communications with VICE
	/// apFontData, aFontDataSize point to ttf data
	bool Init( void *apFontData, int32_t aFontDataSize );

	//----------------------------------------------------------------
	///Flush any pending commnds from the queue
	/// abWait to force wait until flush complete
	void FlushCommands( bool abWait = false );

	//----------------------------------------------------------------
	///Display the monitor windows
	void Display(  );

	//----------------------------------------------------------------
	///Process all incoming responses from VICE and dispach to correct system
	void ProcessResponses(  );

	//----------------------------------------------------------------
	///Resume VICE
	void Resume(  );

	//----------------------------------------------------------------
	///Close VICE communication
	void Close(  );

	//----------------------------------------------------------------
	///Get the C64 ImGui Font
	ImFont *C64Font(  );

	//----------------------------------------------------------------
	///Queue the command for send to VICE. It is sent on FlushCommands()
	void Send( CommandPtr apCommand );

}	//namespace Monitor

