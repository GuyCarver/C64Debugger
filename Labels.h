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
// FILE    Labels.h
//----------------------------------------------------------------------

#pragma once

#include "types.h"
#include "json/json.hpp"

#include <filesystem>

namespace Labels
{

//----------------------------------------------------------------
///Displays a combo box with a text filter.
/// pSelected is set to the selected label
struct LabelCombo
{
	LabelCombo( ) = default;
	explicit LabelCombo( float aColumns, float aLines, bool abAlwaysOn = false );

	char Filter[64];
	const char *pSelected = nullptr;			//Pointer to selected label
	int32_t Selected = -1;						//Current list index
	int32_t Active = 0;							//Internal use countdown for deactivation
	float Columns = -1.0f;						//Number of columns to display, -1 = adaptive
	float Lines = -1.0f;						//Number of lines to display, -1 = adaptive
	uint16_t Value = 0;							//Value of selected label
	bool AlwaysOn = true;						//True if always on, false if dropdown

	//----------------------------------------------------------------
	///Display the combo box
	///Return 1 if selected, 2 if ctrl+selected, otherwise 0
	uint32_t Display(  );
};

//----------------------------------------------------------------
///Save data to Json
void ToJson( nlohmann::json &arData );

//----------------------------------------------------------------
///Load data from Json
void FromJson( nlohmann::json &arData );

//----------------------------------------------------------------
///Load labels from given file
bool Load( std::filesystem::path aPath );

//----------------------------------------------------------------
///Enable window
void DisplayOn(  );

//----------------------------------------------------------------
///Draw window
void Display(  );

//----------------------------------------------------------------
///Look up label name for given value, return nullptr if not found
const char *Find( uint16_t aValue );

}	//namespace Labels
