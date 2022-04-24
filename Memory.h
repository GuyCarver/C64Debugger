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
// FILE    Memory.h
//----------------------------------------------------------------------

#pragma once

#include "types.h"
#include "json/json.hpp"

class Response;

namespace Memory
{

//----------------------------------------------------------------
///Save data to Json
void ToJson( nlohmann::json &arData );

//----------------------------------------------------------------
///Load data from Json
void FromJson( nlohmann::json &arData );

//----------------------------------------------------------------
/// Display the memory views and enable input if desired
void Display( bool abInputEnabled );

//----------------------------------------------------------------
/// Refresh contents of views by requesting new data
void Refresh(  );

//----------------------------------------------------------------
/// Handle vice state
void ViceRunning( bool abTF );

//----------------------------------------------------------------
///Update views from a response
bool FromResponse( const Response &arResponse );

//----------------------------------------------------------------
/// Enable the view indicated by given index
void DisplayOn( uint32_t aView );

}	//namespace Memory
