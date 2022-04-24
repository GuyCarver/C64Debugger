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
// FILE    Registers.cpp
//----------------------------------------------------------------------

#include "Registers.h"
#include "Code.h"
#include "Monitor.h"
#include <imgui.h>

namespace Registers
{

/*
GetRegisters:
0a 00 - 10 entries
03 03 d1 e5 - ip
03 00 00 00 - ar
03 01 00 00 - xr
03 02 0a 00 - yr
03 04 f3 00 - sp
03 37 2f 00 --00
03 38 37 00 - 01
03 05 22 00 - Status
03 35 0c 00 - Lin
03 36 01 00 - Cyc

Registers Available
02 02 3d 00 00 00 83 00 02 01 00 00
0a 00
05 03 10 02 50 43 ....PC
04 00 08 01 41 ....A
04 01 08 01 58 ....X
04 02 08 01 59 ....Y
05 04 08 02 53 50 ....SP
05 37 08 02 30 30 .7..00
05 38 08 02 30 31 .8..01
05 05 08 02 46 4c ....FL
06 35 10 03 4c 49 4e .5..LIN
06 36 10 03 43 59 43 .6..CYC

*/

//----------------------------------------------------------------
///Register IDs returned by the response
enum REGID
{
	AR,
	XR,
	YR,
	IP,
	SP,
	ST,
	LIN,			//0x35
	CYC,			//0x36
	R00,			//0x37
	R01,			//0x38
	COUNT
};

//----------------------------------------------------------------
///Array of values given by response
uint16_t RegValueA[REGID::COUNT] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//----------------------------------------------------------------
///Ids for the ImGui buttons
char STStringA[8][5] =
{
	"1##c",			//Carry
	"1##z",			//Zero
	"0##i",			//IRQ Disable
	"1##d",			//Decimal
	"1##b",			//BRK Command
	"    ",
	"1##v",			//Overflow
	"1##n",			//Negative
};

//----------------------------------------------------------------
///Bit values for the status register
enum STBIT
{
	C,
	Z,
	I,
	D,
	B,
	//Bit 5 is not used
	N = 6,
	V
};

//----------------------------------------------------------------
///Update ImGui status strings from value
void SetStatus( uint8_t aValue )
{
	RegValueA[REGID::ST] = aValue;

	auto set = [&aValue]( uint32_t B ) {
		STStringA[B][0] = aValue & (1 << B) ? '1' : '0';
	};

	set(STBIT::C);
	set(STBIT::Z);
	set(STBIT::I);
	set(STBIT::D);
	set(STBIT::B);
	set(STBIT::N);
	set(STBIT::V);
}

//----------------------------------------------------------------
///Send 1 register in set command
CommandPtr BuildCommand( uint8_t aRegister )
{
	//Set to no ID as we always respond to these commands and don't need
	// a specific ID.
	CommandPtr pcmd(new Command(COMMAND::REGISTERS_SET, NOID));

	//This lambda is useful for setting more than one register
//	auto addit = [&]( uint8_t aID ) {
//		pcmd->Add(3_u8);							// 3 bytes, Register ID, 16bit value
//		pcmd->Add(aID);
//		pcmd->Add(RegValueA[aID]);
//	};
//
	pcmd->Add(0_u8);							// Main memory
	pcmd->Add(1_u16);							// Sending 10 registers
	pcmd->Add(3_u8);							// 3 bytes, Register ID, 16bit value
	pcmd->Add(aRegister);
	pcmd->Add(RegValueA[aRegister]);
	return pcmd;
}

//----------------------------------------------------------------
///Send the given register update to VICE
void UpdateRegister( uint8_t aRegister )
{
	CommandPtr pcmd = BuildCommand(aRegister);
	Monitor::Send(pcmd);
}

//----------------------------------------------------------------
///Update the status register from the button bits and send to VICE
void UpdateStatusRegister(  )
{
	uint16_t bit = 0x01;
	uint16_t v = 0;
	//Build register value from the individual ImGui display values
	for ( uint32_t i = 0; i < 8; ++i) {
		if (STStringA[i][0] == '1') {
			v |= bit;
		}
		bit <<= 1;
	}
	RegValueA[REGID::ST] = v;
	UpdateRegister(REGID::ST);
}

//----------------------------------------------------------------
void FromResponse( const Response &arResponse )
{
	//Parse reponse data into data
	RegValueA[REGID::IP] = arResponse.Get16(0x4);
	RegValueA[REGID::AR] = arResponse.Get8(0x8);
	RegValueA[REGID::XR] = arResponse.Get8(0xc);
	RegValueA[REGID::YR] = arResponse.Get8(0x10);
	RegValueA[REGID::SP] = arResponse.Get8(0x14);
	SetStatus(arResponse.Get8(0x20));
	//We store these values as we have to pass them when setting registers
	RegValueA[REGID::R00] = arResponse.Get8(0x18);
	RegValueA[REGID::R01] = arResponse.Get8(0x1c);
	RegValueA[REGID::LIN] = arResponse.Get8(0x24);
	RegValueA[REGID::CYC] = arResponse.Get8(0x28);

	Code::NewIP(RegValueA[REGID::IP]);
}

//----------------------------------------------------------------
void Display( bool abInputEnabled )
{
	ImGui::Begin("Registers", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

	float fs = ImGui::GetFontSize() - 3.0f;

	//Funky spacing to try and align with the buttons which are at minimum width but still
	// wider than the characters.  Could change display to non-interactive buttons to match
	// but that might look a bit confusing
	ImGui::Text(" IP   AR XR YR SP  N V - B D  I Z C");

	//Add scalar input for a register
	auto regbutton = []( const char *aID, uint8_t aRegister, bool abSameLine = true, int32_t aDataType = ImGuiDataType_U8 ) {
		if (abSameLine) {
			ImGui::SameLine(0.0f, 1.0f);
		}
		if (ImGui::InputScalar(aID, aDataType, &RegValueA[aRegister],
			nullptr, nullptr,
			aDataType == ImGuiDataType_U8 ? "%02X" : "%04X",
			ImGuiInputTextFlags_CharsHexadecimal
			| ImGuiInputTextFlags_EnterReturnsTrue
			| ImGuiInputTextFlags_AlwaysOverwrite)) {
			UpdateRegister(aRegister);
		}
	};

	ImGui::PushItemWidth(fs * 4.0f - 2.0f);
	regbutton("##ip", REGID::IP, false, ImGuiDataType_U16);
	ImGui::PopItemWidth();

	if (!abInputEnabled)
		ImGui::BeginDisabled(true);

	ImGui::PushItemWidth(fs * 2.0f);
	regbutton("##ar", REGID::AR);
	regbutton("##xr", REGID::XR);
	regbutton("##yr", REGID::YR);
	regbutton("##sp", REGID::SP);
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(fs);

	//Add button for the given status register
	auto stbutton = []( char *aID, float aSpacing = 1.0f ) {
		ImGui::SameLine(0.0f, aSpacing);
		if (ImGui::Button(aID)) {
			aID[0] = (aID[0] == '1' ? '0' : '1');
			UpdateStatusRegister();				// Send register set command
		}
	};

	stbutton(STStringA[STBIT::N], 6.0f);
	ImGui::SameLine(0.0f, 1.0f);
	stbutton(STStringA[STBIT::V]);
	stbutton(STStringA[STBIT::B], fs + 3.0f);
	stbutton(STStringA[STBIT::D]);
	stbutton(STStringA[STBIT::I]);
	stbutton(STStringA[STBIT::Z]);
	stbutton(STStringA[STBIT::C]);

	ImGui::PopItemWidth();
	if (!abInputEnabled)
		ImGui::EndDisabled();

	ImGui::End();
}

}	//namespace Registers
