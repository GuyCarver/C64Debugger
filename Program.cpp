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
// FILE    Program.cpp
//----------------------------------------------------------------------

#include "Program.h"
#include "Command.h"
#include "Labels.h"
#include "Monitor.h"

namespace Program
{

//byte 0: Run after loading? 0x01: true, 0x00: false
//byte 1-2: File index of the file to execute, if a disk image. 0x00 is the default value.
//byte 3: Length of filename
//byte 4+: Filename to autoload.

constexpr uint32_t FILELEN = 0xff;				//Maximum file name size

CommandPtr LoadFileCommand = Command::Create(COMMAND::AUTOSTART, FILELEN);

//----------------------------------------------------------------
bool Load( std::filesystem::path aPath )
{
	bool bres;
	const auto &rname = aPath.string();
	auto len = rname.length() + 1;				//+ 1 to include null termination
	if (bres = (len <= FILELEN); bres) {
		LoadFileCommand->Reset();
		LoadFileCommand->Add(1_u8);				//Run after loading
		LoadFileCommand->Add(0_u16);			//File index
		LoadFileCommand->Add(static_cast<uint8_t>(len));
		LoadFileCommand->Add(rname.c_str());	//Add file name
		Monitor::Send(LoadFileCommand);			//Send command

		aPath.replace_extension(".vs");			//Change extension to .vs
		Labels::Load(aPath);					//Attempt to load .vs file

		//NOTE: We could wait for response to load .vs file
	}

	return bres;
}


}	//namespace Program
