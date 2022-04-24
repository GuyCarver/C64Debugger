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
// FILE    Diagnostics.cpp
// DATE    04/15/2022 03:16 AM
//----------------------------------------------------------------------

#include "Diagnostics.h"
#include "types.h"
#include "Numbers.h"
#include <imgui.h>

namespace Diagnostics
{

/*{
[ ] The view flickers when the buffer gets full.  What's going on there?

}*/

bool Enabled = false;							//Display enabled state

constexpr uint32_t DIAGLINELEN = 49;
constexpr uint32_t DIAGLINES = 120;

//----------------------------------------------------------------
struct CommandName
{
	COMMAND eValue;
	const char *pName;
};

const CommandName CommandNameA[] =
{
	{COMMAND::NONE, "NONE"},
	{COMMAND::MEMORY_GET, "MEMORY_GET"},
	{COMMAND::MEMORY_SET, "MEMORY_SET"},
	{COMMAND::CHECKPOINT_GET, "CHECKPOINT_GET"},
	{COMMAND::CHECKPOINT_SET, "CHECKPOINT_SET"},
	{COMMAND::CHECKPOINT_DEL, "CHECKPOINT_DEL"},
	{COMMAND::CHECKPOINT_LST, "CHECKPOINT_LST"},
	{COMMAND::CHECKPOINT_TGL, "CHECKPOINT_TGL"},
	{COMMAND::CONDITION_SET, "CONDITION_SET"},
	{COMMAND::REGISTERS_GET, "REGISTERS_GET"},
	{COMMAND::REGISTERS_SET, "REGISTERS_SET"},
	{COMMAND::DUMP, "DUMP"},
	{COMMAND::UNDUMP, "UNDUMP"},
	{COMMAND::RESOURCE_GET, "RESOURCE_GET"},
	{COMMAND::RESOURCE_SET, "RESOURCE_SET"},
	{COMMAND::JAM, "JAM"},
	{COMMAND::STOPPED, "STOPPED"},
	{COMMAND::RESUMED, "RESUMED"},
	{COMMAND::ADVANCE, "ADVANCE"},
	{COMMAND::KEYBOARD, "KEYBOARD"},
	{COMMAND::STEP_OUT, "STEP_OUT"},
	{COMMAND::PING, "PING"},
	{COMMAND::BANKS_AVAIL, "BANKS_AVAIL"},
	{COMMAND::REGISTERS_AVAIL, "REGISTERS_AVAIL"},
	{COMMAND::DISPLAY_GET, "DISPLAY_GET"},
	{COMMAND::VICE_INFO, "VICE_INFO"},
	{COMMAND::PALETTE_GET, "PALETTE_GET"},
	{COMMAND::JOYPORT_SET, "JOYPORT_SET"},
	{COMMAND::USERPORT_SET, "USERPORT_SET"},
	{COMMAND::EXIT, "EXIT"},
	{COMMAND::QUIT, "QUIT"},
	{COMMAND::RESET, "RESET"},
	{COMMAND::AUTOSTART, "AUTOSTART"}
};

//----------------------------------------------------------------
struct ErrorCodeName
{
	ERRORCODES eValue;
	const char *pName;
};

//----------------------------------------------------------------
const ErrorCodeName ErrorCodeNameA[] =
{
	{ERRORCODES::EC_OK, "EC_OK"},
	{ERRORCODES::EC_NOEXIST, "EC_NOEXIST"},
	{ERRORCODES::EC_MEMINVALID, "EC_MEMINVALID"},
	{ERRORCODES::EC_BADCMDLEN, "EC_BADCMDLEN"},
	{ERRORCODES::EC_BADPARAM, "EC_BADPARAM"},
	{ERRORCODES::EC_BADVERSION, "EC_BADVERSION"},
	{ERRORCODES::EC_BADCMD, "EC_BADCMD"},
	{ERRORCODES::EC_BADRESPONSE, "EC_BADRESPONSE"},
	{ERRORCODES::EC_GENFAIL, "EC_GENFAIL"}
};

//----------------------------------------------------------------
template <class T, typename C>
const char *FindName( const C &arContainer, T aeID )
{
	const char *pname = "UNKNOWN";
	for ( const auto &entry : arContainer ) {
		if (entry.eValue== aeID) {
			pname = entry.pName;
			break;
		}
	}
	return pname;
}

//----------------------------------------------------------------
class ScrollingContent
{
public:
	//----------------------------------------------------------------
	ScrollingContent(  ) { Clear(); }

	//----------------------------------------------------------------
	//Call function on each line from Start to End
	template <class Fn> void ForEach( Fn Func )
	{
		uint32_t i = Start;
		while (i != Index) {
			Func(Lines[i]);
			i = Next(i);
		};
	}

	//----------------------------------------------------------------
	//Get a line for use
	char *Get(  )
	{
		uint32_t v = Index;
		Index = Next(Index);
		if (Index == Start) {
			Start = Next(Start);
		}
		return Lines[v];
	}

	//----------------------------------------------------------------
	void Clear(  )
	{
		//Null terminate all lines
		for ( uint32_t i = 0; i < DIAGLINES; ++i) {
			Lines[i][0] = 0;
		}
		Start = Index = 0;
	}

	char Lines[DIAGLINES][DIAGLINELEN];
	uint32_t Start = 0;							// Start point in ring buffer
	uint32_t Index = 0;							// Index of current line
	bool NewContent = false;

	//----------------------------------------------------------------
	//Increment and wrap value at DIAGLINES
	static uint32_t Next( uint32_t aValue ) {
		if (++aValue >= DIAGLINES) {
			aValue = 0;
		}
		return aValue;
	}
};

ScrollingContent HexBuffer;

//----------------------------------------------------------------
//Copy source into destination assuming apDest is large enough to
// contain apSource aMaxLen can be used to limit input length, but is
// not guaranteed to prevent overflow. It's up to the caller to worry about that
// Returns new position
char *CopyTo( char *apDest, const char *apSource, uint32_t aMaxLen = DIAGLINELEN )
{
	while(*apSource != 0 && --aMaxLen) {
		*apDest++ = *apSource++;
	}
	*apDest = 0;								// Null terminate

	return apDest;
}

//----------------------------------------------------------------
void AddCommand( const Command &arCommand )
{
	char *pline = HexBuffer.Get();
	pline = CopyTo(pline, "CMD: ");
	pline = CopyTo(pline, FindName(CommandNameA, arCommand.QCommand()));
	pline = CopyTo(pline, " ID: ");
	Numbers::ToHex(pline, arCommand.QID());	//Print ID.

	uint32_t size = arCommand.QBodyLen();
	const uint8_t *pbody = arCommand.QBody();

	//Split lines into 16 bytes
	while (size) {
		uint32_t len = size < 16 ? size : 16;
		size -= len;
		pline = HexBuffer.Get();
		Numbers::ToHex(pline, DIAGLINELEN, pbody, len);
		pbody += len;
	}
}

//----------------------------------------------------------------
void AddResponse( ResponsePtr apResponse )
{
	char *pline = HexBuffer.Get();
	pline = CopyTo(pline, "RSP: ");
	pline = CopyTo(pline, FindName(CommandNameA, apResponse->QCommand()));
	pline = CopyTo(pline, " ID: ");
	Numbers::ToHex(pline, apResponse->QID());	//Print ID.

	if (apResponse->QError() != ERRORCODES::EC_OK) {
		pline = HexBuffer.Get();
		pline = CopyTo(pline, "ERR: ");
		CopyTo(pline, FindName(ErrorCodeNameA, apResponse->QError()));
	}

	uint32_t size = apResponse->QBodyLen();
	const uint8_t *pbody = apResponse->QBody();
	//Split lines into 16 bytes
	while (size) {
		uint32_t len = size < 16 ? size : 16;
		size -= len;
		pline = HexBuffer.Get();
		Numbers::ToHex(pline, DIAGLINELEN, pbody, len);
		pbody += len;
	}
	HexBuffer.NewContent = true;
}

//----------------------------------------------------------------
void AddText( const char *apText )
{
	char *pline = HexBuffer.Get();
	CopyTo(pline, apText, DIAGLINELEN);
}

//----------------------------------------------------------------
void DisplayOn(  )
{
	Enabled = true;
}

//----------------------------------------------------------------
void ToJson( nlohmann::json &arData )
{
	arData["Diagnostics"] = {
		{"On", Enabled}
	};
}

//----------------------------------------------------------------
void FromJson( nlohmann::json &arData )
{
	auto obj = arData["Diagnostics"];
	if (!obj.is_null()) {
		Enabled = obj["On"];
	}
}

//----------------------------------------------------------------
void Display(  )
{
	if (Enabled) {
		ImGui::SetNextWindowSize(ImVec2(380, 480), ImGuiCond_FirstUseEver);
		ImGui::Begin("Diagnostics", &Enabled);	// Pass pointer to bool controlling visibility

		if (ImGui::BeginListBox("Output", ImVec2(-FLT_MIN, -FLT_MIN))) {
			HexBuffer.ForEach([]( char *apLine ) {
				ImGui::Text(apLine);
			});
			if (HexBuffer.NewContent) {
				ImGui::SetScrollHereY(1.0f);
				HexBuffer.NewContent = false;
			}
			ImGui::EndListBox();
		}

		ImGui::End();
	}
}

}	//namespace Diagnostics
