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
// FILE    BreakPoints.cpp
//----------------------------------------------------------------------

#include "BreakPoints.h"
#include "Command.h"
#include "Monitor.h"
#include "Numbers.h"
#include "Response.h"

#include <imgui.h>
#include <memory>								// For shared_ptr
#include <unordered_map>

/*{
[ ] Kill all received checkpoints we didn't set

}*/

namespace BreakPoints
{

class BreakPoint;
using BreakPointPtr = std::shared_ptr<BreakPoint>;
using BreakPointContainer = std::unordered_map<uint16_t, BreakPointPtr>;

BreakPointContainer BreakPointA;

//Memory access type
enum CHECKOP : uint8_t
{
	LOAD = 0_bit,								//The address is loaded
	STORE = 1_bit,								//The address is Stored to
	EXEC = 2_bit								//The address is executed (breakpoint)
};

//----------------------------------------------------------------
class BreakPoint
{
public:
	BreakPoint(  ) = delete;

	//----------------------------------------------------------------
	explicit BreakPoint( uint16_t aAddress ) : Address(aAddress)
	{
		//Create a command, don't use an ID so we always process the responses
		// without having to look for them.
		pCommand = CommandPtr(new Command(COMMAND::CHECKPOINT_SET, Address));
		pCommand->Add(Address);					//Start address
		pCommand->Add(Address);					//End address
		pCommand->Add(1_u8);					//Stop when hit
		pCommand->Add(Enabled);					//Enabled state
		pCommand->Add(CHECKOP::EXEC);			//Execution operations
		pCommand->Add(0_u16);					//Not temporary and memspace 0

		Monitor::Send(pCommand);				//Send add command
	}

	//----------------------------------------------------------------
	///Constructor for memory checkpoint/breakpoint
	explicit BreakPoint( uint16_t aStartAddress, uint16_t aEndAddress, uint8_t aBreak = 0 ) : Address(aStartAddress)
	{
		//Create a command, don't use an ID so we always process the responses
		// without having to look for them.
		pCommand = CommandPtr(new Command(COMMAND::CHECKPOINT_SET, Address));
		pCommand->Add(Address);					//Start address
		pCommand->Add(aEndAddress);				//End address
		pCommand->Add(aBreak);					//Stop when hit?
		pCommand->Add(Enabled);					//Enabled state
		pCommand->Add(CHECKOP::STORE);			//Execution operations
		pCommand->Add(0_u16);					//Not temporary and memspace 0

		Monitor::Send(pCommand);				//Send add command
	}

	//----------------------------------------------------------------
	~BreakPoint(  )
	{
		if (Legit()) {
			pCommand->SetCommand(COMMAND::CHECKPOINT_DEL);
			pCommand->Reset();
			pCommand->Add(Index);				//ID of checkpoint to delete
			Monitor::Send(pCommand);			//Send delete command
		}
	}

	//----------------------------------------------------------------
	uint32_t QID(  ) const { return pCommand->QID(); }

	//----------------------------------------------------------------
	///Return true if a VICE checkpoint number has been set, otherwise
	/// VICE hasn't replied that it's aware of this breakpoint
	bool Legit(  ) const { return Index != 0xFFFFFFFF; }

	//----------------------------------------------------------------
	bool QEnabled(  ) const { return Enabled != 0; }

	//----------------------------------------------------------------
	///If not already desired state then toggle
	void Enable( bool abTF ) { if (Enabled != static_cast<uint8_t>(abTF)) Toggle(); }

	//----------------------------------------------------------------
	///Send VICE command to toggle enable state
	void Toggle(  )
	{
		if (Legit()) {
			//NOTE: We set the local state here because VICE doesn't
			// send a response. So we assume VICE changed the enable state
			Enabled = !Enabled;
			pCommand->SetCommand(COMMAND::CHECKPOINT_TGL);
			pCommand->Reset();
			pCommand->Add(Index);
			pCommand->Add(Enabled);
			Monitor::Send(pCommand);
		}
	}

	CommandPtr pCommand;						//Command object used to control this CheckPoint
	uint32_t Index = 0xFFFFFFFF;				//Index for breakpoint assigned by VICE
	uint16_t Address;							//Address of the breakpoint
	uint8_t Enabled = true;						//Enabled state
};

//----------------------------------------------------------------
bool ForEach( BREAKPOINTFN aFunction )
{
	bool bres = true;

	//Loop for each breakpoint and call the function with the address and enable state
	for ( const auto &entry : BreakPointA ) {
		auto addr = entry.second->Address;
		auto enabled = entry.second->QEnabled();
		bres = aFunction(addr, enabled);
		if (!bres) {
			break;
		}
	}
	return bres;
}

//----------------------------------------------------------------
bool CheckHit( uint16_t aAddress )
{
	bool bres = false;
	auto entry = BreakPointA.find(aAddress);
	if (entry != BreakPointA.end()) {
		bres = entry->second->QEnabled();
	}
	return bres;
}

//----------------------------------------------------------------
void Add( uint16_t aAddress )
{
	//If found in map then delete
	if (auto entry = BreakPointA.find(aAddress); entry != BreakPointA.end()) {
		Remove(aAddress);
	}
	else {
		BreakPointPtr ptr(new BreakPoint(aAddress));
		BreakPointA[aAddress] = ptr;
	}
}

//----------------------------------------------------------------
void Add( uint16_t aStartAddress, uint16_t aEndAddress, uint8_t aBreak )
{
	//If found in map then delete
	//NOTE: Code breakpoints and memory checkpoints use the same map with
	// the address as the key, so you can't have both for the same memory address.
	// A limitation I don't expect to be much of an issue
	if (auto entry = BreakPointA.find(aStartAddress); entry != BreakPointA.end()) {
		Remove(aStartAddress);
	}
	else {
		BreakPointPtr ptr(new BreakPoint(aStartAddress, aEndAddress, aBreak));
		BreakPointA[aStartAddress] = ptr;
	}
}

//----------------------------------------------------------------
void Remove( uint16_t aAddress )
{
	BreakPointA.erase(aAddress);				//Remove BreakPoint from map
}

//----------------------------------------------------------------
void Enable( uint16_t aAddress, bool abTF )
{
	if (auto entry = BreakPointA.find(aAddress); entry != BreakPointA.end()) {
		entry->second->Enable(abTF);
	}
}

//----------------------------------------------------------------
void Toggle( uint16_t aAddress )
{
	if (auto entry = BreakPointA.find(aAddress); entry != BreakPointA.end()) {
		entry->second->Toggle();
	}
}

//----------------------------------------------------------------
void Close(  )
{
	BreakPointA.clear();
}

//The hope was that we'd get a response from the delete/toggle commands
// and set the state of the breakpoints based on those, but we don't
// get responses to our commands from VICE. So the functions that send
// the commands also update our state
//----------------------------------------------------------------
//void ProcessDelete( const Response &arResponse )
//{
//	auto addr = static_cast<uint16_t>(arResponse.QID());	//Command ID will be the address
//	BreakPointA.erase(addr);
//}
//
////----------------------------------------------------------------
//void ProcessToggle( const Response &arResponse )
//{
//	auto addr = static_cast<uint16_t>(arResponse.QID());	//Command ID will be the address
//	if (auto entry = BreakPointA.find(addr); entry != BreakPointA.end()) {
//		entry->second->Enabled = !entry->second->Enabled;
//	}
//}

//----------------------------------------------------------------
void ProcessInfo( const Response &arResponse )
{
	//VICE assigns a unique number to each checkpoint, we use this to associate
	// the response with a local checkpoint Instance
	auto index = arResponse.Get<uint32_t>(0);
	auto addr = arResponse.Get16(5);

	if (auto entry = BreakPointA.find(addr); entry != BreakPointA.end()) {
		entry->second->Index = index;			//Make sure index is set
		entry->second->Enabled = arResponse.Get8(10);
	}
	else {
		//This isn't one of our breakpoints so delete it
		auto pcmd = CommandPtr(new Command(COMMAND::CHECKPOINT_DEL, addr));
		pcmd->Add(index);						//ID of checkpoint to delete
		Monitor::Send(pcmd);					//Send delete command
	}

}

bool bDisplayEnabled = false;					//Display enabled state

//----------------------------------------------------------------
void Display(  )
{
	if (bDisplayEnabled) {
		ImGui::SetNextWindowSize(ImVec2(160, 190), ImGuiCond_FirstUseEver);
		ImGui::Begin("BreakPoints", &bDisplayEnabled, ImGuiWindowFlags_NoResize);

		ImGui::Text("Address");
		ImGui::SameLine();
		ImGui::Text("Enabled");

		char addrText[8];
		int32_t id = 0;							//Id of checkboxes

		ImGui::PushItemWidth(-1.0f);
		//list box of labels, addresses, enabled
		if (ImGui::BeginListBox("##BPList")) {
			ForEach([&]( uint16_t aAddress, bool abEnabled) {
				Numbers::ToHex(addrText, aAddress);
				ImGui::Text(addrText);
				ImGui::SameLine();
				auto pos = ImGui::GetCursorPos();
				pos.x += 16.0f;
				pos.y -= 2.0f;
				ImGui::SetCursorPos(pos);
				ImGui::PushID(id);

				static bool bstate = abEnabled;

				//Use bstate as checkbox bool, but the actual state
				// change on click is done by Toggle()
				if (ImGui::Checkbox("", &bstate)) {
					Toggle(aAddress);
				}
				ImGui::PopID();
				++id;
				return true;
			});
			ImGui::EndListBox();
		}
		ImGui::PopItemWidth();
		ImGui::End();
	}
	//TODO: Add ability to delete BreakPoints
}

//----------------------------------------------------------------
void ToJson( nlohmann::json &arData )
{
	arData["BreakPoints"] = {
		{"On", bDisplayEnabled}
	};
}

//----------------------------------------------------------------
void FromJson( nlohmann::json &arData )
{
	auto obj = arData["BreakPoints"];
	if (!obj.is_null()) {
		bDisplayEnabled = obj["On"];
	}
}

//----------------------------------------------------------------
void DisplayOn(  )
{
	bDisplayEnabled = true;
}

}	//namespace BreakPoints
