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
// FILE    Memory.cpp
//----------------------------------------------------------------------

#include "Memory.h"
#include "BreakPoints.h"
#include "Command.h"
#include "Labels.h"
#include "Monitor.h"
#include "Numbers.h"
#include "Response.h"

#include <imgui.h>
#include <memory>

/*
//Layout of byte, word or lword views
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
0000 0000 0000 0000 0000 0000 0000 0000
00000000 00000000 00000000 00000000
*/

/*{
todo:
[ ] Handle views looking at the same memory.
  Have all memory views process all memory responses. If
  memory falls in range, then do the update of that memory
[x] Test allowing edit of both Hex and Ascii. Not worth doing ascii
[ ] Support for breakpoints on memory Address read/write with conditions
[x] Switch update method to use checkpoints - Get too much data back.  Will stick with a polling method

}*/

namespace Memory
{

constexpr uint32_t BYTESPERLINE = 0x10;
constexpr uint32_t MEMLINES = 0x14;
constexpr uint32_t HEXLINELEN = BYTESPERLINE * 3;
constexpr uint32_t HEXVIEWSIZE = MEMLINES * HEXLINELEN;
constexpr uint32_t ADDRLINELEN = 5;
constexpr uint32_t ASCIILINELEN = BYTESPERLINE + 1;
constexpr uint32_t ASCIIVIEWSIZE = MEMLINES * ASCIILINELEN;
constexpr uint32_t MEMBLOCKSIZE = BYTESPERLINE * MEMLINES;
constexpr uint16_t LASTADDRESS = 0x10000 - MEMBLOCKSIZE;
constexpr uint32_t NUMVIEWS = 2;
constexpr uint32_t WAITCOUNT = 60;

//----------------------------------------------------------------
/// View into 16x20 bytes of memory. Edit and update.
class MemoryView
{
public:
	//----------------------------------------------------------------
	///Constructor with given ID used to reference the view
	explicit MemoryView( uint8_t aID )
	: LabelFilter(32.0f, 10.0f)
	, IDNum(aID)
	{
		ID[6] += aID;							//Set ID number in ImGui view name
		Clear();

		//Allocate Command object large enough to send updates
		pCommand = Command::Create(COMMAND::MEMORY_GET, MEMBLOCKSIZE + 8);
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
		HexView[0] = 0;
		AddressView[0] = 0;
		AsciiView[0] = 0;
	}

	//----------------------------------------------------------------
	///Returned enabled (visible) state
	bool QEnabled(  ) const { return Enabled; }

	//----------------------------------------------------------------
	///Set enabled state
	void SetEnabled( bool abTF )
	{
		if (Enabled != abTF) {
			Enabled = abTF;

			//If we need new data send request
			if (Enabled && !QContinuous()) {
				//Make sure an address is set
				if (Address == 0xffff) {
					SetAddress(0x1000);
				}
				Refresh(true);
			}

			//Set state of CheckPoint
			SetCheckPoint(Enabled && Monitor::ViceState() == VICESTATE::STOPPED);
		}
	}

	//----------------------------------------------------------------
	///Display the view
	void Display( bool abInputEnabled )
	{
		if (!Enabled) return;					//Early out if view not visible

		InputEnabled = abInputEnabled;

		Refresh();								//Make sure data is up to date

		//Set start position and size on first run
		ImGui::SetNextWindowPos(ImVec2(384.0f, 95.0f + (290.0f * IDNum)), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(610.0f, 350.0f), ImGuiCond_FirstUseEver);

		ImGui::Begin(ID, &Enabled, ImGuiWindowFlags_NoResize);

		const float w = ImGui::GetFontSize();

		auto currentPos = ImGui::GetCursorPos();
		currentPos.x += 4.0f;
		ImGui::SetCursorPos(currentPos);

		//Add address and bank inputs
		ImGui::Text("Addr");
		ImGui::SameLine(0.0f, w * 5.0f);
		ImGui::Text("Labels");

		ImGui::PushItemWidth(((w - 3) * 4.0f) - 2.0f);
		uint16_t loc = NewAddress;
		if (ImGui::InputScalar("##A", ImGuiDataType_U16, &loc,
			nullptr, nullptr, "%04x",
			ImGuiInputTextFlags_CharsHexadecimal
			| ImGuiInputTextFlags_EnterReturnsTrue
			| ImGuiInputTextFlags_AlwaysOverwrite)) {
			SetAddress(loc);
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		//Show label search
		ImGui::PushItemWidth(w * 12.0f);
		//If display returns nonzero, set a new address
		if (uint32_t activated = LabelFilter.Display(); activated) {
			if (LabelFilter.Selected >= 0) {
				SetAddress(LabelFilter.Value);
			}
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		//Continuous update checkbox
		ImGui::Checkbox("Continuous", &Continuous);
		ImGui::SameLine();

		//View refresh button
		if (ImGui::Button("Refresh")) {
			Refresh(true);
		}

		//Add address lines
		currentPos = ImGui::GetCursorPos();
		currentPos.x += 4.0f;					//Align with Address input
		ImGui::SetCursorPos(currentPos);
		ImGui::Text(AddressView, ImVec2(w * 2.75f, MEMLINES * ImGui::GetTextLineHeightWithSpacing() + 5));

		ImGui::SameLine();

		uint32_t sz = sizeof(HexView) - 1;
		currentPos = ImGui::GetCursorPos();

		//Display frame around HexView
		currentPos.x -= 4.0f;
		currentPos.y -= 4.0f;
		ImGui::SetCursorPos(currentPos);
		//Change border color based on input enable
		const uint32_t enabledColor = IM_COL32(200, 255, 255, 255);
		const uint32_t disabledColor = IM_COL32(80, 135, 135, 255);
		ImGui::PushStyleColor(ImGuiCol_Border, InputEnabled ? enabledColor : disabledColor);
		ImGui::BeginChild("##MemViewFrame", ImVec2(w * 26.0f + 4, (w * MEMLINES) + 16), true);

		//Display data changed since last update
		currentPos = ImGui::GetCursorPos();		//Get position relative to frame
		currentPos.y -= 3.0f;					//Offset reltive to InputTextMultiline
		ImGui::SetCursorPos(currentPos);
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
		ImGui::Text(HexChangeView);
		ImGui::PopStyleColor();

		//Display edited data
		ImGui::SetCursorPos(currentPos);		//Reset to draw in same position
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 128, 255));
		ImGui::Text(HexEditView);
		ImGui::PopStyleColor();
		currentPos.x -= 4.0f;					//Reset to draw in same position
		currentPos.y -= 3.0f;
		ImGui::SetCursorPos(currentPos);

		auto cursorPos = PrevPos;				//Get copy of cursor position before update to check if we are on the 1st line

		//Draw at alpha 99 so Changes and Edits drawn previously will show
		// as red or yellow text
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 150));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
		ImGui::InputTextMultiline("##MemL", HexView, sz
				, ImVec2(w * 26.0f, (MEMLINES * ImGui::GetFontSize()) + 6)
				, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsNoBlank
				| ImGuiInputTextFlags_AlwaysOverwrite
				| ImGuiInputTextFlags_NoUndoRedo
				| ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_CallbackCharFilter
				, HexViewCallback, this);

		//If HexView has focus then process arrow and page up/down keys
		if (ImGui::IsItemFocused()) {
			if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
				if (cursorPos <= HEXLINELEN) {
					if (QAddress() >= BYTESPERLINE) {
						SetAddress(QAddress() - BYTESPERLINE);
					}
				}
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
				if (PrevPos >= HEXVIEWSIZE - HEXLINELEN) {
					if (QAddress() < LASTADDRESS) {
						SetAddress(QAddress() + BYTESPERLINE);
					}
				}
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_PageUp)) {
				auto newAddr = QAddress();
				if (newAddr > 0) {
					newAddr = newAddr >= MEMBLOCKSIZE ? newAddr - MEMBLOCKSIZE : 0;
					SetAddress(newAddr);
				}
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_PageDown)) {
				auto newAddr = QAddress();
				if (newAddr < LASTADDRESS) {
					newAddr += MEMBLOCKSIZE;
					if (newAddr > LASTADDRESS) {
						newAddr = LASTADDRESS;
					}
					SetAddress(newAddr);
				}
			}
		}

		ImGui::PopStyleColor();					//Frame background
		ImGui::PopStyleColor();					//Text color

		ImGui::EndChild();
		ImGui::PopStyleColor();					//Border color

		//Display ASCII text view in PETASCII font
		ImGui::SameLine();
		currentPos = ImGui::GetCursorPos();
		ImGui::PushFont(Monitor::C64Font());
		ImGui::Text(AsciiView, ImVec2(w * 17.0f, MEMLINES * ImGui::GetTextLineHeightWithSpacing() + 5));

		//Calculate cursor position
		currentPos.x += ((CursorPos % HEXLINELEN) / 3) * w;
		currentPos.y += (CursorPos / HEXLINELEN) * w;

		ImGui::SetCursorPos(currentPos);
		ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(255, 135, 255, 100));
		ImGui::BeginChild("##AsciiCursor", ImVec2(w + 2, w + 2), true);
		ImGui::Text(" ");
		ImGui::EndChild();
		ImGui::PopStyleColor();					//Border color

		ImGui::PopFont();
		ImGui::End();
	}

	//----------------------------------------------------------------
	/// Send command for new data or process pending data
	void Refresh( bool abForce = false )
	{
		//Decrement wait count if not 0.
		if (Waiting) {
			--Waiting;
		}
		else {
			//If we want continuous updates and VICE is running, send a new command
			// But not if we are waiting for a response
			abForce |= NewAddress != Address;	//If new address then force send
			if (abForce || (Continuous && Monitor::ViceState() == VICESTATE::RUNNING)) {
				RequestMemory();
			}
		}
	}

	//----------------------------------------------------------------
	///Return NewAddress != Address
	bool QNewAddress(  ) const { return NewAddress != Address; }

	//----------------------------------------------------------------
	/// Set memory Address/bank for memory to show in view
	void SetAddress( uint16_t aLoc )
	{
		NewAddress = aLoc & 0xfff0;		//Align downward to 16 bytes
		//Keep from > than value that would allow us to display full set of data
		if (NewAddress > LASTADDRESS) {
			NewAddress = LASTADDRESS;
		}

		if (QNewAddress()) {
			Refresh();
		}
	}

	//----------------------------------------------------------------
	///Process new memory data from response
	bool FromResponse( const Response &arResponse )
	{
		bool bres;

		//Only process responses to our requests
		if (bres = arResponse.QID() == pCommand->QID(); bres) {
			uint16_t size = arResponse.Get16(0);
			//TODO: This assumes we are reading in all the data
			// We need to add support for reading just portions into Data and calculate
			// the correct address.  We'll only need that though if we are going to support
			// writing out portions of data rather than just the whole block each change

			//TODO: Should this just fail if it's wrong?
			if (size > MEMBLOCKSIZE) { size = MEMBLOCKSIZE; }

			//Copy Data into PrevData
			auto setPrevData = [&](  ) {
				memcpy_s(PrevData, MEMBLOCKSIZE, Data, MEMBLOCKSIZE);
			};

			//If a new Address we'll reset prevdata to data later, so don't bother
			// to copy here
			if (!QNewAddress()) {
				setPrevData();						//Copy current buffer into previous for diff view
			}

			//Copy data from response, skipping size
			memcpy_s(Data, MEMBLOCKSIZE, arResponse.QBody() + 2, size);

			//If we also got a new Address, update the address view
			//Need to set previous data before UpdateHexView() or differences will be displayed
			if (NewAddress != Address) {
				Address = NewAddress;
				setPrevData();
				UpdateAddressView();
				SetCheckPoint(Monitor::ViceState() == VICESTATE::STOPPED);
			}

			UpdateHexView();						//Update hex view from data
			UpdateAsciiView();						//Update ASCII view from data
			Waiting = 0;
		}
		return bres;
	}

	//----------------------------------------------------------------
	///If vice not running, enable memory CheckPoint
	void ViceRunning( bool abTF )
	{
		SetCheckPoint(!abTF);
		if (!abTF) {
			Refresh(true);
		}
	}

	//----------------------------------------------------------------
	///Enable/Disable CheckPoint and update address
	void SetCheckPoint( bool abTF )
	{
		//Make sure CheckPoint is set to address
		if (CheckPoint != Address) {
			if (CheckPoint != 0xffff) {
				BreakPoints::Remove(CheckPoint);
			}
			CheckPoint = Address;
			//Add a breakpoint that does not break on hit
			BreakPoints::Add(CheckPoint, CheckPoint + MEMBLOCKSIZE - 1, false);
		}

		//If CheckPoint set then set enable state
		if (CheckPoint != 0xffff) {
			BreakPoints::Enable(CheckPoint, abTF);
		}
	}

private:
	Labels::LabelCombo LabelFilter;				//Filter for the label combo box
	CommandPtr pCommand;						//Command object used to update this view
	int32_t PrevPos = -1;						//Previous position for Memory edit cursor
	uint32_t Waiting = 0;						//Waiting for response
	int32_t CursorPos = 0;						//Current cursor position
	uint16_t Address = 0xffff;					//c64 memory address
	uint16_t NewAddress = 0xffff;				//New Address set if != Address, need address view refresh
	uint16_t CheckPoint = 0xffff;				//Address of CheckPoint, 0xFFFF if not set
	uint16_t Bank = 0;							//c64 memory bank
	//Memory and display buffers.
	uint8_t PrevData[MEMBLOCKSIZE];				//Previous data used for change detection
	uint8_t Data[MEMBLOCKSIZE];
	char AddressView[MEMLINES * ADDRLINELEN];	//16 bit address plus null terminator
	char HexView[HEXVIEWSIZE + 1];				//+ 1 for null terminator
	char HexChangeView[HEXVIEWSIZE + 1];		//Used to display highlights on changed values
	char HexEditView[HEXVIEWSIZE + 1];			//Used to display highlights on edited values
	char AsciiView[ASCIIVIEWSIZE];
	char ID[8] = "Memory0";						//View Identifier string. The number is set to the IDNum value
	uint8_t IDNum = 0;							//ID value as a number
	bool Enabled = false;						//Display enabled
	bool Continuous = false;					//When true will continuously ask for new data while Vice is running
	bool InputEnabled = false;					//Indicate if can edit memory

	//----------------------------------------------------------------
	///Callback for HexView input
	int32_t DoHexViewCallback( ImGuiInputTextCallbackData *apData )
	{
		//If buf value is set, check to make sure the cursor isn't over a space or cr
		switch (apData->EventFlag) {
			case ImGuiInputTextFlags_CallbackAlways:
				apData->ClearSelection();		//We don't allow selection

				//If Buf value is set then make sure cursor is never
				// in a non-data position
				if (apData->Buf) {
					int32_t pos = apData->CursorPos;

					if (pos >= apData->BufTextLen - 1) {
						apData->CursorPos = PrevPos;
					}
					else {
						auto c = apData->Buf[pos];
						if (c == ' ' || c == '\n') {
							pos += (PrevPos < pos) ? 1 : -2;
							apData->CursorPos = pos;
						}
						PrevPos = pos;
					}
					CursorPos = pos;
				}
				break;
			case ImGuiInputTextFlags_CallbackCharFilter:
			{
				auto c = apData->EventChar;
				if (InputEnabled) {
					if (c == '\n') {
						return 1;				//Return 1 to decline char
					}
					HexEditView[PrevPos] = static_cast<char>(c);
					HexChangeView[PrevPos] = ' ';
					//If edited the 2nd nibble of a byte, send the byte
					if (PrevPos % 3) {
						ApplyChange(PrevPos);
					}
				}
				else {
					return 1;					//Don't allow input
				}
				break;
			}
			default:
				break;
		}

		return 0;
	}

	//----------------------------------------------------------------
	///Direct callback for HexViewInput to the MemoryView stored
	/// in apData->UserData
	static int32_t HexViewCallback( ImGuiInputTextCallbackData *apData )
	{
		auto pthis = reinterpret_cast<MemoryView*>(apData->UserData);
		return pthis->DoHexViewCallback(apData);
	}

	//----------------------------------------------------------------
	///Send command for memory refresh
	void RequestMemory(  )
	{
		pCommand->Reset();
		pCommand->SetCommand(COMMAND::MEMORY_GET);

		pCommand->Add(0_u8);					//No side effects
		auto loc = NewAddress;					//Set new address
		pCommand->Add(loc);						//Start Address
		auto end = loc + MEMBLOCKSIZE - 1;
		pCommand->Add(end);						//End Address
		pCommand->Add(0_u8);					//Main Memory
		pCommand->Add(QBank());
		Monitor::Send(pCommand);				//Send the command
		Waiting = WAITCOUNT;					//Set counter indicating waiting for response
	}

	//----------------------------------------------------------------
	///Apply change made to the memory buffer and send to VICE
	void ApplyChange( uint32_t aPos )
	{
		//Get change from Edits and apply to location
		auto dataPos = static_cast<uint16_t>(aPos / 3);
		aPos = dataPos * 3;						//Round to start of byte

		//If only edited the 2nd nibble, copy the 1st nibble value
		// to HexEditView
		if (HexEditView[aPos] == ' ') {
			HexEditView[aPos] = HexView[aPos];
			HexChangeView[aPos] = ' ';
		}

		auto b = Numbers::HexToUInt8(&HexEditView[aPos]);
		Data[dataPos] = b;

		pCommand->Reset();
		pCommand->SetCommand(COMMAND::MEMORY_SET);

		pCommand->Add(1_u8);					//Side effects
		uint16_t loc = dataPos + QAddress();
		pCommand->Add(loc);						//Start Address
		pCommand->Add(loc);						//End Address
		pCommand->Add(0_u8);					//Main Memory
		pCommand->Add(QBank());
		pCommand->Add(b);						//Add the byte
		Monitor::Send(pCommand);				//Send the command

		UpdateAsciiView();
	}

	//----------------------------------------------------------------
	///Update Address lines to MemoryView address
	void UpdateAddressView(  )
	{
		uint16_t loc = QAddress();
		char *pdest = AddressView;
		for ( uint32_t i = 0; i < MEMLINES; ++i) {
			Numbers::ToHex(pdest, loc);
			pdest += 4;
			*pdest++ = '\n';
			loc += BYTESPERLINE;
		}
		*(pdest - 1) = 0;						//Remove last cr
	}

	//----------------------------------------------------------------
	///Update the HexChange view by examining differences between
	/// Data and PrevData
	/// NOTE: This assumes HexView is up to date
	void UpdateHexChangeView(  )
	{
		char *pdest = HexChangeView;
		char *psrc = HexView;
		uint32_t written = 0;

		//Loop through the memory.
		for ( uint32_t i = 0; i < MEMBLOCKSIZE; ++i) {
			uint8_t v = Data[i];
			//If the data has changed, copy the ascii values from HexView to HexChangeView
			if (v !=  PrevData[i]) {
				pdest[written] = psrc[written];
				++written;
				pdest[written] = psrc[written];
				++written;
			}
			else {
				pdest[written++] = ' ';			//Set values to spaces
				pdest[written++] = ' ';
			}
			pdest[written] = psrc[written];		//Copy char in between entries
			written++;
		}
	}

	//----------------------------------------------------------------
	///Set HexView from Data
	void UpdateHexView(  )
	{
		Numbers::ToHex(HexView, sizeof(HexView), Data, MEMBLOCKSIZE);
		HexView[sizeof(HexView) - 2] = 0;		//Kill final cr

		UpdateHexChangeView();					//Now update changes

		//Clear the HexEdit view to spaces with lines separated by cr
		char *pdest = HexEditView;
		for ( uint32_t i = 0; i < MEMLINES; ++i) {
			memset(pdest, ' ', HEXLINELEN - 1);
			pdest[HEXLINELEN - 1] = '\n';
			pdest += HEXLINELEN;
		}
		pdest[HEXLINELEN - 1] = 0;				//Null terminate
	}

	//----------------------------------------------------------------
	///Update AsciiView from the Data
	void UpdateAsciiView(  )
	{
		uint8_t *pdata = Data;
		char *pdest = AsciiView;
		//Loop for all lines
		for ( uint32_t i = 0; i < MEMLINES; ++i) {
			pdest += Numbers::ToAscii(pdest, ASCIILINELEN, pdata, BYTESPERLINE);
			*pdest++ = '\n';					//Add cr
			pdata += BYTESPERLINE;
		}

		*(pdest - 1) = 0;						//Null terminate by overwriting last cr
	}
};

using MemoryViewPtr = std::unique_ptr<MemoryView>;

//----------------------------------------------------------------
///Array of views
MemoryViewPtr Views[NUMVIEWS] =
{
	MemoryViewPtr(new MemoryView(0)),
	MemoryViewPtr(new MemoryView(1))
};

//----------------------------------------------------------------
void ToJson( nlohmann::json &arData )
{
	char key[] = "Mem0";
	for ( uint32_t i = 0; i < NUMVIEWS; ++i) {
		if (Views[i]) {
			arData[key] = {
				{"Address", Views[i]->QAddress()},
				{"On", Views[i]->QEnabled()}
			};
		}
		++key[3];								//Increment number in key
	}
}

//----------------------------------------------------------------
void FromJson( nlohmann::json &arData )
{
	char key[] = "Mem0";
	for ( uint32_t i = 0; i < NUMVIEWS; ++i) {
		if (Views[i]) {
			auto obj = arData[key];
			if (!obj.is_null()) {
				Views[i]->SetAddress(obj["Address"]);
				Views[i]->SetEnabled(obj["On"]);
			}
			else {
				Views[i]->SetAddress(0);		//Make sure to set to something as views start at 0xffff
			}
		}
		++key[3];								//Increment number in key
	}
}

//----------------------------------------------------------------
void Refresh(  )
{
	for ( const auto &view : Views ) {
		if (view->QEnabled()) {
			view->Refresh(true);
		}
	}
}

//----------------------------------------------------------------
void ViceRunning( bool abTF )
{
	for ( const auto &view : Views ) {
		if (view->QEnabled()) {
			view->ViceRunning(abTF);
		}
	}
}

//----------------------------------------------------------------
bool FromResponse( const Response &arResponse )
{
	for ( const auto &view : Views ) {
		//Right now we process only responses for specific requests
		// but we may want to change this and attept to process
		// responses that change memory we are viewing
		if (view->FromResponse(arResponse)) {
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------
void DisplayOn( uint32_t aView )
{
	if (aView < NUMVIEWS) {
		Views[aView]->SetEnabled(true);
	}
}

//----------------------------------------------------------------
void Display( bool abInputEnabled )
{
	for ( const auto &view : Views ) {
		view->Display(abInputEnabled);
	}
}

}	//namespace Memory
