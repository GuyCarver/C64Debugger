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
// FILE    Code.cpp
//----------------------------------------------------------------------

#include "Code.h"
#include "6502.h"
#include "Assembler.h"
#include "BreakPoints.h"
#include "DisAssembler.h"
#include "Command.h"
#include "ImGuiUtils.h"
#include "Labels.h"
#include "Monitor.h"
#include "Numbers.h"
#include "Response.h"

#include <imgui.h>
#include <memory>
#include <unordered_map>

/*{
TODO:
[ ] Fix activation to try to keep disassembly view active
[*] Why does every other key press for stepping not register? This was due to F10 being a
	windows syskey to toggle menubar activation I added a syskeyup event to correctly
	report the up event to ImGui.
[*] Added dropdown for label selection for goto. In but only works once
[*] Add scrolling
	[ ] Add extra lines above/below display area to allow for smoother scrolling to next legit line
	[*] Add array of OpCode pointers to store for each line so we can get some info about the line
[ ] Add tab stops, I can't figure out how to modify this.  I only have tab stops for the buttons
[*] Add editing
	[ ] parse for labels
[ ] More than 1 window?
[ ] Refactor MemoryView and CodeView to share code using components
[ ] Add Tooltips for opcodes
[ ] Ctrl+click on line will jump to address if op has an address
	[ ] Keep history of addresses and have a back button to go back?

}*/

namespace Code
{

constexpr uint32_t ASSEMBLYLINES = 0x20;					//Number of lines to display at a time
constexpr uint32_t ASSEMBLYBLOCKSIZE = ASSEMBLYLINES * 3;	//Maximum 3 bytes per op
constexpr uint16_t LASTADDRESS = 0x10000 - ASSEMBLYBLOCKSIZE;
constexpr uint32_t CODELINELEN = 0x20;			//Suggested maximum length of view line
constexpr uint32_t ADDRLINELEN = 5;				//Length of line in AddressView
constexpr uint32_t CODEVIEWSIZE = ASSEMBLYLINES * CODELINELEN;
constexpr uint32_t BYTELINELEN = 9;				//Up to 3 bytes taking 3 chars each
constexpr uint32_t BYTEVIEWSIZE = ASSEMBLYLINES * BYTELINELEN;
constexpr uint32_t LABELLINELEN = 13;			//12 char maximum label size
constexpr uint32_t LABELVIEWSIZE = ASSEMBLYLINES * LABELLINELEN;

const CommandPtr StepOutCommand(new Command(COMMAND::STEP_OUT, NOID));
CommandPtr StepCommand(new Command(COMMAND::ADVANCE, NOID));

//----------------------------------------------------------------
class CodeView
{
public:

	//----------------------------------------------------------------
	explicit CodeView(  ) : LabelFilter(32.0f, 10.0f)
	{
		Clear();

		//Allocate Command object large enough to send updates
		pCommand = Command::Create(COMMAND::MEMORY_GET, ASSEMBLYBLOCKSIZE + 8);
	}

	//----------------------------------------------------------------
	///Indicate we continuously ask for new data
	void SetContinuous( bool abTF ) { Continuous = abTF; }

	//----------------------------------------------------------------
	///Return continuous update state
	bool QContinuous( ) const { return Continuous; }

	//----------------------------------------------------------------
	///Get address view is looking at
	uint16_t QAddress(  ) const { return Address; }

	//----------------------------------------------------------------
	///Get bank view is looking at
	uint16_t QBank(  ) const { return Bank; }

	//----------------------------------------------------------------
	///Clear the view display buffers
	void Clear(  )
	{
		AddressView[0] = 0;
		DisView[0] = 0;
		ByteView[0] = 0;
		LabelView[0] = 0;
	}

	//----------------------------------------------------------------
	///Return FollowIP
	bool QFollowIP(  ) const { return FollowIP; }

	//----------------------------------------------------------------
	void SetFollowIP( bool abTF ) { FollowIP = abTF; }

	//----------------------------------------------------------------
	///Draw this view
	void Display( bool abInputEnabled )
	{
		InputEnabled = abInputEnabled;

		ImGui::SetNextWindowPos(ImVec2(0, 95), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(386, 512), ImGuiCond_FirstUseEver);
		ImGui::Begin("Code", nullptr, ImGuiWindowFlags_NoResize);

		uint16_t loc = QAddress();

		//Lambda to add hex number input
		auto button = [&]( const char *aID, uint16_t *apValue, bool abSameLine = false ) {
			if (abSameLine) {
				ImGui::SameLine(0.0f, 1.0f);
			}
			if (ImGui::InputScalar(aID, ImGuiDataType_U16, apValue,
				nullptr, nullptr, "%04x",
				ImGuiInputTextFlags_CharsHexadecimal
				| ImGuiInputTextFlags_EnterReturnsTrue
				| ImGuiInputTextFlags_AlwaysOverwrite)) {
				SetAddress(loc);
			}
		};

		const float h = ImGui::GetTextLineHeightWithSpacing(); //ImGui::GetFontSize();
		const float w = ImGui::GetTextLineHeight() - 5.4f;

		ImGui::Text("Label");
		ImGui::SameLine(0.0f, w * 8.0f);
		ImGui::Text("Address");
		ImGui::SameLine(0.0f, w * 8.0f);
		ImGui::Checkbox("FollowIP", &FollowIP);

		//Show label search
		ImGui::PushItemWidth(w * 12.0f);
		//If display returns nonzero, set a new address
		if (uint32_t activated = LabelFilter.Display(); activated) {
			if (LabelFilter.Selected >= 0) {
				FollowIP = false;				//If we select a label to go to, stop following IP
				SetAddress(LabelFilter.Value);
			}
		}
		ImGui::PopItemWidth();
		ImGui::SameLine(0.0f, 1.0f);

		//Add address and bank inputs
		ImGui::PushItemWidth(w * 5.0f);
		button("##Addr", &loc);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		//Continuous update checkbox
		ImGui::Checkbox("Continuous", &Continuous);
		ImGui::SameLine();
		//View refresh button
		if (ImGui::Button("Refresh")) {
			RequestMemory();
		}

		//Change border color based on input enable
		uint32_t frameColor = InputEnabled
			? IM_COL32(200, 255, 255, 100)
			: IM_COL32(80, 135, 135, 100);

		//Display frame around LabelView
		ImGui::PushStyleColor(ImGuiCol_Border, frameColor);
		ImGui::BeginChild("##LabelViewFrame", ImVec2(w * 17.0f, -1.0f),
			true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs);

		//Display data changed since last update
		auto currentPos = ImGui::GetCursorPos();	//Get position relative to frame
		currentPos.x -= 4.0f;
		currentPos.y -= 3.0f;					//Offset reltive to InputTextMultiline
		//Set the width for 12 bytes
		ImGui::SetCursorPos(currentPos);
		ImGui::Text(LabelView);
		ImGui::SameLine(0.0f, 1.0f);

		currentPos.x += w * (LABELLINELEN - 1);
		ImGui::SetCursorPos(currentPos);

		//Add address lines
		ImGui::Text(AddressView);

		ImGui::EndChild();
		ImGui::PopStyleColor();					//Border color

		ImGui::SameLine(0.0f, 1.0f);

		//Change border color based on input enable
		ImGui::PushStyleColor(ImGuiCol_Border, frameColor);
//This doesn't work, some child windows is always focused so the 2nd check fails
//		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow)
//			&& (!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))) {
//			ImGui::SetNextWindowFocus();
//		}
		ImGui::BeginChild("##CodeViewFrame", ImVec2(-1.0f, -1.0f),
			true, ImGuiWindowFlags_NoScrollbar);

		currentPos = ImGui::GetCursorPos();		//Get position relative to frame
		currentPos.x -= 4.0f;
		currentPos.y -= 3.0f;					//Offset reltive to InputTextMultiline
		ImGui::SetCursorPos(currentPos);
		constexpr ImVec4 blue(0.1f, 0.4f, 0.6f, 1.0f);
		ImGui::TextColored(blue, ByteView, ImVec2(w * 9.0f, (h * ASSEMBLYLINES) + 5));

		currentPos.x += w * 9.0f;
		ImGui::SetCursorPos(currentPos);

		ImGui::Text(DisView);

		HandleInputs();

		//Save the position for the disassembly view
		auto savePos = currentPos;
		savePos.x -= (w + 2.0f);

		//Draw cursor
		float y = currentPos.y;
		currentPos.y += ImGui::GetFontSize() * Cursor;
		ImGui::SetCursorPos(currentPos);

		static const ImVec4 reg(0.0f, 0.4f, 0.6f, 0.5f);
		static const ImVec4 hov(0.0f, 0.6f, 0.7f, 0.5f);
		static const ImVec4 act(0.0f, 0.7f, 0.9f, 0.5f);
		ImGui::PushStyleColor(ImGuiCol_Button, reg);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hov);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, act);
		if (Editing) {
			currentPos.x -= 4.0f;
			currentPos.y -= 4.0f;
			ImGui::SetCursorPos(currentPos);
			if (ImGui::InputText("##ass", AssText, sizeof(AssText), ImGuiInputTextFlags_EnterReturnsTrue)) {
				AssembleEntry();
			}
			Editing = ImGui::IsItemActive();
		}
		else if (ImGui::Selectable("##sel", true, ImGuiSelectableFlags_AllowDoubleClick)) {
			StartAssembly();
		}
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		//Place Instruction Pointer
		if (IPCursor >= 0.0f) {
			currentPos.y = y + (ImGui::GetFontSize() * IPCursor);
			currentPos.x -= w; //ImGui::GetTextLineHeight();
			ImGui::SetCursorPos(currentPos);
			constexpr ImVec4 red(1.0f, 0.0f, 0.0f, 1.0f);
			ImGui::TextColored(red, ">");
			currentPos.x -= 2.0f;
			ImGui::SetCursorPos(currentPos);
		}

		DisplayBreakPoints(savePos);

		ImGui::EndChild();
		ImGui::PopStyleColor();					//Border color

		//Handle mouse selection of item
		// This must be done here for position calculations to
		// be relative to the frame we just ended
		if (ImGui::IsItemClicked(0)) {
			auto ul = ImGui::GetItemRectMin();
			auto mouse = ImGui::GetMousePos();
			y = (mouse.y - ul.y) / ImGui::GetFontSize();
			if ((y >= 0.0f) && (y < ASSEMBLYLINES)) {
				Cursor = floor(y);
			}
		}

		ImGui::End();
	}

	//----------------------------------------------------------------
	///Set memory Address for memory to show in view
	void SetAddress( uint16_t aAddress )
	{
		if (Address != aAddress) {
			NewAddress = true;
			Address = aAddress;
			RequestMemory();
		}
	}

	//----------------------------------------------------------------
	///Set Instruction Pointer address
	void SetIP( uint16_t aAddress )
	{
		if (QFollowIP()) {
			constexpr uint16_t IPADJ = 0xa * 3;				//Number of lines * maximum 3 bytes per op

			//If the IPAddress has changed find the new one.
			if (IPAddress != aAddress) {
				IPAddress = aAddress;
				IPCursor = AddressToIndex(aAddress);
				//If the IPCursor isn't in view, place it close to the middle of the view
				// and get new data
				if (IPCursor < 0.0f) {
					uint16_t newAddress = aAddress > IPADJ ? aAddress - IPADJ : 0_u16;
					SetAddress(newAddress);
				}
			}
		}
	}

	//----------------------------------------------------------------
	///Update the disassembly view
	void UpdateDisView(  )
	{
		DisAssembler::Input input{
			DisView,
			ByteView,
			OpCodes,
			Data,
			CODEVIEWSIZE,
			ASSEMBLYBLOCKSIZE,
			ASSEMBLYLINES,
			Address
		};

		EndAddr = DisAssembler::DisAssemble(input);
		SetAddressView();						//Now calculate address for each line
		IPCursor = AddressToIndex(IPAddress);
	}

	//----------------------------------------------------------------
	///Process new memory data from response
	bool FromResponse( const Response &arResponse )
	{
		bool bres;
		//Only process responses we specifically asked for
		if (bres = arResponse.QID() == pCommand->QID(); bres) {
			uint16_t size = arResponse.Get16(0);

			//TODO: Should this just fail if it's wrong?
			if (size > ASSEMBLYBLOCKSIZE) { size = ASSEMBLYBLOCKSIZE; }

			//Copy data from response, skipping size
			memcpy_s(Data, ASSEMBLYBLOCKSIZE, arResponse.QBody() + 2, size);

			UpdateDisView();						//Update Disassembly view

			//If we want continuous updates and VICE is running
			if ((Continuous) && (Monitor::ViceState() == VICESTATE::RUNNING)) {
				RequestMemory();
			}
		}
		return bres;
	}

	//----------------------------------------------------------------
	///Send command for memory refresh
	void RequestMemory(  )
	{
		if (Address != 0xFFFF) {
			pCommand->Reset();
			pCommand->SetCommand(COMMAND::MEMORY_GET);

			pCommand->Add(0_u8);				//No side effects
			auto loc = QAddress();
			pCommand->Add(loc);					//Start Address
			auto end = loc + ASSEMBLYBLOCKSIZE - 1;
			pCommand->Add(end);					//End Address
			pCommand->Add(0_u8);				//Main Memory
			pCommand->Add(QBank());
			Monitor::Send(pCommand);			//Send the command
		}
	}

private:
	Labels::LabelCombo LabelFilter;				//Filter for the label combo box
	CommandPtr pCommand;						//Command object used to update this view
	const OpCode *OpCodes[ASSEMBLYLINES];		//Pointers to the OpCodes for each line
	uint16_t Addresses[ASSEMBLYLINES];			//Address for each line of disassembly
	uint8_t Data[ASSEMBLYBLOCKSIZE];			//Memory do disassemble
	char AddressView[ASSEMBLYLINES * ADDRLINELEN];	//16 bit address plus null terminator
	char DisView[CODEVIEWSIZE + 1];				//+ 1 for null terminator
	char LabelView[LABELVIEWSIZE + 1];			//View for labels on addresses
	char ByteView[BYTEVIEWSIZE + 1];			//View of bytes for opcodes
	char AssText[CODELINELEN];
	float Cursor = ASSEMBLYLINES / 2.0f;		//Cursor position for Code edit
	float IPCursor = 0.0f;						//Cursor for the instruction pointer
	uint32_t ID = 0;							//Command ID for memory get commands
	uint16_t IPAddress = 0xffff;				//Address of instruction pointer
	uint16_t Address = 0xffff;					//c64 memory address
	uint16_t EndAddr = 0xffff;					//End address for disassembly
	uint16_t Bank = 0;							//c64 memory bank
	bool NewAddress = true;						//New Address set, need address view refresh
	bool Continuous = false;					//When true will continuously ask for new data while Vice is running
	bool FollowIP = true;						//Follow intruction pointer
	bool InputEnabled = false;					//Indicate if can edit memory
	bool Editing = false;						//Indicate if editing disassembly

	//----------------------------------------------------------------
	///Move the cursor in the given direction and adjust visible address if necessary
	void MoveCursor( float aDirection )
	{
		//Use 32 bit math in this functio to avoid overrun
		Cursor += aDirection;
		if (Cursor < 0.0f) {
			auto adj = static_cast<uint32_t>(-Cursor);
			uint32_t newAddr = QAddress();
			newAddr = adj <= newAddr ? newAddr - adj : 0u;
			SetAddress(static_cast<uint16_t>(newAddr));
			Cursor = 0.0f;
		}
		else if (Cursor >= ASSEMBLYLINES) {
			auto adj = static_cast<uint32_t>(Cursor - (ASSEMBLYLINES - 1.0f));
			auto newAddr = QAddress() + adj;
			if (newAddr >= 0x10000) {
				newAddr = 0xFFFF;
			}
			SetAddress(static_cast<uint16_t>(newAddr));
			Cursor = ASSEMBLYLINES - 1.0f;
		}
	}

	//----------------------------------------------------------------
	///Do Keyboard/mouse input events
	void HandleInputs(  )
	{
		//NOTE: Can't use IsItemFocused because text doesn't get focus, but the frame does
		if (ImGui::IsWindowFocused())
		{
			if (!Editing && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
				StartAssembly();
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
				MoveCursor(-1.0f);
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
				MoveCursor(1.0f);
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_PageUp)) {
				MoveCursor(-static_cast<float>(ASSEMBLYLINES));
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_PageDown)) {
				MoveCursor(ASSEMBLYLINES);
			}
			//Handle mouse wheel scrolling
			else {
				constexpr float mscale = 0.5f;

				auto &io = ImGui::GetIO();
				float move = io.MouseWheel;
				if (move < -mscale) {
					MoveCursor(1.0f);
				}
				else if (move > mscale) {
					MoveCursor(-1.0f);
				}
			}
		}

		//Run
		if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
			Monitor::Resume();
		}
		//FollowIP Toggle
		else if (ImGui::IsKeyPressed(ImGuiKey_F8)) {
			FollowIP = !FollowIP;
		}
		//BreakPoint add/remove, Ctrl+F9 = toggle
		else if (ImGui::IsKeyPressed(ImGuiKey_F9)) {
			//NOTE: With 0 being our non-value, we can't set a breakpoint on address 0
			if (uint16_t addr = IndexToAddress(Cursor); addr) {
				CtrlDown() ? BreakPoints::Toggle(addr) : BreakPoints::Add(addr);
			}
		}
		//Step over JSR
		else if (ImGui::IsKeyPressed(ImGuiKey_F10, false)) {
			Step(true);							//Step over
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_F11, false)) {
			Step(false, ShiftDown());			//Stip in or out if Shift
		}
	}

	//----------------------------------------------------------------
	///Display breakpoints on the line corresponding to their address
	/// starting at the given position
	void DisplayBreakPoints( ImVec2 aPos )
	{
		constexpr ImVec4 red(1.0f, 0.0f, 0.0f, 1.0f);
		constexpr ImVec4 gray(0.7f, 0.7f, 0.7f, 1.0f);
		auto y = aPos.y;							//Save top of disassembly view

		//Loop for each breakpoint
		BreakPoints::ForEach([&]( uint16_t aAddress, bool abEnabled ) {
			float index = AddressToIndex(aAddress);
			//If index for the address is in range of display, draw it
			if (index >= 0.0f) {
				aPos.y = y + (ImGui::GetFontSize() * index);
				ImGui::SetCursorPos(aPos);
				const char ic[2] = {static_cast<const char>(0x95), 0};
				ImGui::TextColored(abEnabled ? red : gray, ic);
			}
			return true;
		});
	}

	//----------------------------------------------------------------
	///Start assembly input
	void StartAssembly(  )
	{
		if (Cursor >= 0.0f) {
			uint32_t line = 0;
			const char *pdis = DisView;
			auto cursor = static_cast<uint32_t>(Cursor);
			if (cursor == 0) {
				Editing = true;
			}
			else {
				for ( uint32_t i = 0; i < sizeof(DisView); ++i) {
					auto c = *pdis++;
					if (c == 0) {
						break;
					}
					else if (c == '\n') {
						if (++line == cursor) {
							Editing = true;
							break;
						}
					}
				}
			}
			if (Editing) {
				char c = 1;
				for ( uint32_t i = 0; (c != 0) && (i < sizeof(AssText)); ++i) {
					c = pdis[i];
					if (c == '\n') {
						c = 0;
					}
					AssText[i] = c;
				}
			}
		}
	}

	//----------------------------------------------------------------
	///Assemble the entry
	void AssembleEntry(  )
	{
		Assembler::Assemble as2(AssText, sizeof(AssText), Address);
		if (as2.QGood()) {
			auto cursor = static_cast<uint32_t>(Cursor);
			auto addr = static_cast<uint16_t>(Addresses[cursor] - Address);
			auto pdata = &Data[addr];
			//Copy results to the destination
			pdata[0] = as2.OP;
			if (as2.Len > 1) {
				pdata[1] = as2.B0;
				if (as2.Len > 2) {
					pdata[2] = as2.B1;
 				}
			}
			//Send memory to VICE
			//If new size < old size, pad with NOP.
			//If new size > old size pad next entry with nop
			ApplyChange(addr);
//			UpdateDisView();					//Redo DisView
		}

		AssText[0] = 0;
	}

	//----------------------------------------------------------------
	///Convert line index into address, 0 if not in range
	uint16_t IndexToAddress( float aIndex )
	{
		auto i = static_cast<int32_t>(aIndex);
		return ((i >= 0) && (i < ASSEMBLYLINES))
		? Addresses[i]
		: 0;
	}

	//----------------------------------------------------------------
	///Convert address into line index, -1.0f if not in range
	/// Uses float as this value is used to adjust Imgui cursor pos
	/// which is floating point.
	float AddressToIndex( uint16_t aAddress )
	{
		int32_t index = -1;
		//If address is in range of visible memory
		if (aAddress >= Addresses[0] && (aAddress < Addresses[ASSEMBLYLINES - 1])) {
			for ( uint32_t i = 1; i < ASSEMBLYLINES; ++i) {
				if (aAddress < Addresses[i] ) {
					index = i - 1;
					break;
				}
			}
		}
		return static_cast<float>(index);
	}

	//----------------------------------------------------------------
	///Fill address array with address for each line and print to AddressView
	/// Also fill the label view if label found
	void SetAddressView(  )
	{
		uint16_t curAddr = Address;

		char *pdest = AddressView;
		char *plabels = LabelView;

		//Loop for number of lines in disassembly view
		for ( uint32_t i = 0; i < ASSEMBLYLINES; ++i) {
			Addresses[i] = curAddr;
			Numbers::ToHex(pdest, curAddr);
			pdest[4] = '\n';
			pdest += 5;

			//Now add a label for the address
			if (const char *pl = Labels::Find(curAddr); pl) {
				for ( uint32_t j = 0; j < LABELLINELEN - 1; ++j) {
					char c = *pl++;
					if (c == 0) break;			//Null terminator
					*plabels++ = c;
				}
			}
			*plabels++ = '\n';					//Next line
			//TODO: Ensure this isn't a nullptr? Shouldn't have to since it should at least be filled with bad.
			curAddr += OpCodes[i]->QSize();		//Adjust by number of bytes the opcode uses
		}
		*(plabels - 1) = 0;						//Last /n is a null terminator
		*(pdest - 1) = 0;
	}

	//----------------------------------------------------------------
	///Send change to memory buffer to VICE
	void ApplyChange( uint16_t aIndex )
	{
		pCommand->Reset();
		pCommand->SetCommand(COMMAND::MEMORY_SET);

		pCommand->Add(1_u8);					//Side effects
		uint16_t loc = Address + aIndex;
		pCommand->Add(loc);						//Start Address
		loc += 3;
		pCommand->Add(loc);						//End Address
		pCommand->Add(0_u16);					//Main Memory, Bank 0
		pCommand->Add(Data[aIndex++]);			//Add byte 0
		pCommand->Add(Data[aIndex++]);			//Add byte 1
		pCommand->Add(Data[aIndex]);			//Add byte 2
		Monitor::Send(pCommand);				//Send the command
	}

	//----------------------------------------------------------------
	///Sent step command
	/// abStepOver controls stepping into or over JSR
	/// abStepOut controls stepping out of a subroutine
	void Step( bool abStepOver, bool abStepOut = false )
	{
		if (Monitor::ViceState() == VICESTATE::STOPPED) {
			if (abStepOut) {
				Monitor::Send(StepOutCommand);
			}
			else {
				StepCommand->Reset();
				StepCommand->Add(abStepOver ? 1_u8 : 0_u8);
				StepCommand->Add(01_u16);		//1 instruction
				Monitor::Send(StepCommand);
			}
		}
	}
};

using CodeViewPtr = std::unique_ptr<CodeView>;

CodeViewPtr pView(new CodeView());

//----------------------------------------------------------------
void ToJson( nlohmann::json &arData )
{
	if (pView) {
		arData["Code"] = {
			{"Address", pView->QAddress()},
			{"FollowIP", pView->QFollowIP()}
		};
	}
}

//----------------------------------------------------------------
void FromJson( nlohmann::json &arData )
{
	if (pView) {
		auto obj = arData["Code"];
		if (!obj.is_null()) {
			pView->SetFollowIP(obj["FollowIP"]);
			pView->SetAddress(obj["Address"]);
		}
	}
}

//----------------------------------------------------------------
void FromResponse( const Response &arResponse )
{
	if (pView) {
		pView->FromResponse(arResponse);
	}
}

//----------------------------------------------------------------
void Refresh(  )
{
	if (pView) {
		pView->RequestMemory();
	}
}

//----------------------------------------------------------------
void UpdateDisView(  )
{
	if (pView) {
		pView->UpdateDisView();
	}
}

//----------------------------------------------------------------
void SetAddress( uint16_t aAddress )
{
	if (pView) {
		pView->SetAddress(aAddress);
	}
}

//----------------------------------------------------------------
void NewIP( uint16_t aAddress )
{
	if (pView) {
		pView->SetIP(aAddress);
	}
}

//----------------------------------------------------------------
void Display( bool abInputEnabled )
{
	if (pView) {
		pView->Display(abInputEnabled);
	}
}

}	//namespace Code
