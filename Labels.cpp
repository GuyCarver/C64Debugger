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
// FILE    Labels.cpp
//----------------------------------------------------------------------

#include "Labels.h"
#include "6502.h"	//For Upper()
#include "BreakPoints.h"
#include "Code.h"								//For SetAddress
#include "ImGuiUtils.h"
#include "Numbers.h"

#include <imgui.h>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>

/*{
[ ] write unittest
[ ] Handle selection of a value

}*/

namespace Labels
{

constexpr float LABELLINES = 25.0f;

using LabelMap = std::unordered_map<uint16_t, std::string>;

//----------------------------------------------------------------
///Check if apSource starts with apCmp (case-insensitive)
///Return apSource adjusted to skip apString or nullptr if no match
const char *StartsWith( const char *apSource, const char *apCmp )
{
	const char *pres = nullptr;
	while (1) {
		char c = Upper(*apCmp++);
		//If we reached the end of compare string, exit with match
		if (c == 0) {
			pres = apSource;
			break;
		}
		if (c != Upper(*apSource++)) {		//If chars don't match, exit
			break;
		}
	}
	return pres;
}

//----------------------------------------------------------------
///Manage label map.
/// Load labels from a .vs file.
/// Since breakpoints are also stored in the .vs file we also
///  parse those and send them to the Checkpoint module
class LabelView
{
public:
	//----------------------------------------------------------------
	explicit LabelView(  ) = default;
//	{
//		Load("Data/wip.vs");
//	}

	//----------------------------------------------------------------
	///Returned enabled (visible) state
	bool QEnabled(  ) const { return Enabled; }

	//----------------------------------------------------------------
	///Set enabled state
	void SetEnabled( bool abTF )
	{
		Enabled = abTF;
		//TODO: If turned on, refresh?
	}

	//----------------------------------------------------------------
	///Load labels from given file
	bool Load( const char *apFile )
	{
		bool res = false;
		std::ifstream labelFile(apFile);
		char inputStream[128];

		Labels.clear();							//Clear any existing labels

		if (labelFile.is_open()) {
			res = true;							//Assume good data

			//Loop until end of file or error
			while(res && !labelFile.eof()) {
				labelFile.getline(inputStream, sizeof(inputStream));
				res = Parse(inputStream);
			}
			labelFile.close();
		}
		return res;
	}

	//----------------------------------------------------------------
	///Find label matching the given address, else nullptr
	const char *Find( uint16_t aValue )
	{
		const char *plabel = nullptr;
		if (auto entry = Labels.find(aValue); entry != Labels.end()) {
			plabel = entry->second.c_str();
		}
		return plabel;
	}

	//----------------------------------------------------------------
	///Iterate over each label in map and call function
	///Func = bool ( uin16_t aKey, const char *aLabel )
	/// return true to continue, false to stop
	template<class FN> void ForEachLabel( FN aFunc )
	{
		for ( const auto &keyval : Labels ) {
			uint16_t key = keyval.first;
			const char *plabel = keyval.second.c_str();
			if (!aFunc(key, plabel)) {
				break;
			}
		}
	}

	//----------------------------------------------------------------
	void Display(  )
	{
		if (!Enabled) return;					//Early out if view not visible

		ImGui::SetNextWindowSize(ImVec2(230, 485), ImGuiCond_FirstUseEver);
		ImGui::Begin("Labels", &Enabled);
		Filter.Display();						//Only thing in this window is the label list
		ImGui::End();
	}

private:
	LabelMap Labels;							//Map of address/label name
	LabelCombo Filter;							//Filtered list of labels
	bool Enabled = false;						//Window enabled

	//----------------------------------------------------------------
	const char *ParseAddress( const char *apSource, uint16_t &arAddress )
	{
		//Loop for up to 4 chars for address + space "???? "
		// If no space hit within 5 chars, it's an error
		for (uint32_t i = 0 ; i < 5; ++i) {
			char c = *apSource++;
			//If we ran out of data, error
			if (!c) {
				return nullptr;
			}

			//We are done when we hit a space
			if (c == ' ') {
				return apSource;
			}

			arAddress <<= 4;
			arAddress += static_cast<uint16_t>(Numbers::ToNum(c));
		}

		//If we hit here we didn't hit a space within 5 characters, so error
		arAddress = 0;
		return nullptr;
	}

	//----------------------------------------------------------------
	///Parse string for label/breakpoint and add to map
	bool Parse( const char *apSource )
	{
		bool bres = false;
		uint16_t address = 0;
		const char *piter;

		//parse "al C:
		if (piter = StartsWith(apSource, "al C:"); piter) {
			//parse ???? into destination string
			if (piter = ParseAddress(piter, address); piter) {
				//Parse .name into std::string?
				if (bres = (*piter++ == '.'); bres) {
					//Add to map
					Labels[address] = piter;
				}
			}
		}
		//Check for break ????
		else if (piter = StartsWith(apSource, "break "); piter) {
			if (bres = ParseAddress(piter, address); bres) {
				BreakPoints::Add(address);
			}
		}

		return bres;
	}
};

using LabelViewPtr = std::unique_ptr<LabelView>;

LabelViewPtr pView(new LabelView());

//----------------------------------------------------------------
///LabelCombo methods
//----------------------------------------------------------------

//----------------------------------------------------------------
LabelCombo::LabelCombo( float aColumns, float aLines, bool abAlwaysOn )
	: Columns(aColumns)
	, Lines(aLines)
	, AlwaysOn(abAlwaysOn)
{

}

//----------------------------------------------------------------
uint32_t LabelCombo::Display(  )
{
	uint32_t activated = 0;						//Flag indicating if item was selected
	if (!pView) return activated;				//Early out

	//Calculate height of frame, if -1.0 then adaptive
	const float h = Lines > 0.0f ? ImGui::GetTextLineHeightWithSpacing() * Lines : Lines;
	const float w = Columns > 0.0f ? (ImGui::GetTextLineHeight() - 5.4f) * Columns : Columns;

	//Display filter text input
	if (ImGui::InputText("##LF", Filter, sizeof(Filter), ImGuiInputTextFlags_EnterReturnsTrue)) {
		activated = CtrlDown()
		? 2
		: 1;
	}

	//If item is activated reset the active countdown timer
	if (ImGui::IsItemActivated()) {
		Active = 1;
	}

	//If item isn't active and countdown timer is set start countdown
	// unless AlwaysOn
	//NOTE: This causes the dropdown to remain active for a few frames
	// to allow for item selection to occur. This takes about 10-14 frames
	// for some reason. If we don't do this countdown, selection
	// of an item in the list is not possible as we stop displaying it
	// before selection can happen
	else if(!ImGui::IsItemActive() && Active && !AlwaysOn) {
		--Active;
	}

	//If dropdown is active then display it and accept input
	if (Active) {
		ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(200, 255, 255, 100));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

		//If AlwaysOn use a frame, otherwise use a ToolTip
		if (AlwaysOn) {
			//Draw frame around the list
			ImGui::BeginChild("##LabelViewFrame", ImVec2(w + 1.0f, h + 1.0f),
				true, ImGuiWindowFlags_NoScrollbar);
		}
		else {
			auto currentPos = ImGui::GetWindowPos();
			currentPos.x += ImGui::GetCursorPosX();
			currentPos.y += ImGui::GetCursorPosY();
			ImGui::SetNextWindowPos(currentPos);
			//TODO: Figure out how to give this focus as it loses focus after 1st use
//			ImGui::SetNextWindowFocus();
			ImGui::BeginTooltip();
		}

		static const ImVec4 reg(0.0f, 0.4f, 0.6f, 0.5f);
		static const ImVec4 hov(0.0f, 0.6f, 0.7f, 0.5f);
		static const ImVec4 act(0.0f, 0.7f, 0.9f, 0.5f);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
		ImGui::PushStyleColor(ImGuiCol_Button, reg);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hov);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, act);
		int index = 0;

		if (ImGui::BeginListBox("##LABELS", ImVec2(w, h))) {
			//Add a line for each label entry

			//Call function on each label in the map
			pView->ForEachLabel([&]( uint16_t aKey, const char *apLabel) {
				//If label starts with filter then add it to the list
				if (StartsWith(apLabel, Filter)) {
					ImGui::Text("$%04X", aKey);	//Add address
					ImGui::SameLine();
					const bool bsel = (Selected == index);
					if (bsel) {
						pSelected = apLabel;	//Set pointer to selected string
						Value = aKey;
					}
					//When pressed, set Selected index
					if (ImGui::Selectable(apLabel, bsel)) {
						//Set activated value
						pSelected = apLabel;
						Selected = index;
						Value = aKey;
						activated = CtrlDown() ? 2 : 1;
						//If not always on clear active count
						// so dropdown will go away
						if (!AlwaysOn) {
							Active = 0;
						}
						//Copy selected string to Filter TextInput
//						strncpy_s(Filter, apLabel, sizeof(Filter));
					}
					++index;
				}
				return true;					//Always do next label
			});
			if (Selected >= index) {			//Clamp selected to last index
				Selected = index - 1;
			}
			ImGui::EndListBox();
		}

		ImGui::PopStyleColor();					//Active
		ImGui::PopStyleColor();					//Hover
		ImGui::PopStyleColor();					//Button
		ImGui::PopStyleColor();					//Bg color

		//Terminate whichever frame type we used at the beginning
		AlwaysOn ? ImGui::EndChild() : ImGui::EndTooltip();
		ImGui::PopStyleVar();					//FramePadding
		ImGui::PopStyleColor();					//Border color

		//Do input processing
		if (ImGui::IsWindowFocused())
		{
			if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
				if (Selected > 0) {
					--Selected;
				}
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
				if (Selected < index) {
					++Selected;
				}
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				Active = 0;						//Close the popup
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
				activated = CtrlDown() ? 2 : 1;
				Active = 0;
			}
		}
	}

	return activated;
}

//----------------------------------------------------------------
void ToJson( nlohmann::json &arData )
{
	if (pView) {
		arData["Labels"] = {
			{"On", pView->QEnabled() }
		};
	}
}

//----------------------------------------------------------------
void FromJson( nlohmann::json &arData )
{
	if (pView) {
		auto obj = arData["Labels"];
		if (!obj.is_null()) {
			pView->SetEnabled(obj["On"]);
		}
	}
}

//----------------------------------------------------------------
bool Load( const char *apFile )
{
	return pView ? pView->Load(apFile) : false;
}

//----------------------------------------------------------------
void DisplayOn(  )
{
	if (pView) pView->SetEnabled(true);
}

//----------------------------------------------------------------
void Display(  )
{
	if (pView) pView->Display();
}

//----------------------------------------------------------------
const char *Find( uint16_t aValue )
{
	return pView ? pView->Find(aValue) : nullptr;
}

}	//namespace Labels
