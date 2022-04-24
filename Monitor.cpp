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
// FILE    Monitor.cpp
//----------------------------------------------------------------------

#include "monitor.h"
#include "BreakPoints.h"
#include "Code.h"
#include "Diagnostics.h"
#include "imfilebrowser.h"
#include "Labels.h"
#include "Memory.h"
#include "Program.h"
#include "Registers.h"
#include "Response.h"

#include <assert.h>
#include <chrono>
#include <fstream>
#include <imgui.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <semaphore>
#include <thread>
#include <vector>

//Use old winsock2 API with C++20 because the new API errors
// on experimental code and I want the <semaphore>
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>

/* Notes:
	The binary monitor has a checkpoint system that may be used to report
	when memory is accessed for read/write or execute. However if it is
	used to monitor non-halting access, it may cause a large amount of
	responses to be spammed. Also, checkpoints reside in VICE and if they
	are not cleaned up before debugger disconnect, they remain.
	Therefore I've chosen to use a more restrained polling system to request
	information only when we are ready for it. Checkpoints will only be
	used for breakpoints.

	This may change in the future by always using checkpoints that break allowing
	us to throttle the input stream.
*/

/*{

[*] Shore up connection loss/reconnect. Mostly done except after reconnection VICE will stop communicating
[ ] Need some sort of callback for new connection so we can refresh displays

}*/

namespace Monitor
{

constexpr const char SettingFile[] = "settings.json";
constexpr float FONTSIZE = 13.0;

ImFont *pC64Font = nullptr;
uint32_t CommDelay = 0;							//Counter for no response
VICESTATE eState = VICESTATE::DISCONNECTED;		//Current known state of VICE
bool Stopped = false;							//Flag to indicate we want VICE stopped

const char HelpText[] =
"F5 = Run\n"
"F8 = Toggle Follow IP\n"
"F9 = Add/Remove BreakPoint\n"
"Ctrl+F9 = Toggle BreakPoint\n"
"F10 = Step over\n"
"F11 = Step into\n"
"Shift+F11 = Step out\n";

//----------------------------------------------------------------
const char *StateString[] =
{
	"DISCONNECTED",
	"RUNNING     ",
	"STOPPED     "
};

#include "MonitorThread.ipp"

ImGui::FileBrowser FileDialog;
bool OpenProg = false;							//Hack to determine what type of file the Dialog is opening

//----------------------------------------------------------------
///Save settings to json file
void SaveSettings(  )
{
	auto strm = std::ofstream(SettingFile);
	//If file open good
	if (strm.good()) {
		nlohmann::json data;

		//Save file dialog path
		auto dir = FileDialog.GetPwd();
		data["path"] = dir.c_str();

		//Save window active states (size/position is handled in imgui.ini)
		// and whatever other data
		Code::ToJson(data);
		Memory::ToJson(data);
		Labels::ToJson(data);
		BreakPoints::ToJson(data);
		Diagnostics::ToJson(data);

		strm << data.dump(2);					//Save json data to stream
	}
}

//----------------------------------------------------------------
///Load settings from json file
void LoadSettings(  )
{
	auto strm = std::ifstream(SettingFile);
	//If file open success, read data
	if (strm.good()) {
		auto data = nlohmann::json::parse(strm);
		auto p = data["path"];					//Read path for file open dialog
		if (!p.empty()) {
			FileDialog.SetPwd(p);
		}
		//Read settings for modules
		Code::FromJson(data);
		Memory::FromJson(data);
		Labels::FromJson(data);
		BreakPoints::FromJson(data);
		Diagnostics::FromJson(data);
	}
}

//----------------------------------------------------------------
///Update the State value
void UpdateStatus(  )
{
	if (Thread::QInstance().QConnected()) {
		auto oldState = eState;
		eState = Stopped ? VICESTATE::STOPPED : VICESTATE::RUNNING;
		if (oldState == VICESTATE::DISCONNECTED) {
			Memory::Refresh();
			Code::Refresh();
		}
	}
	else {
		eState = VICESTATE::DISCONNECTED;
	}
}

//----------------------------------------------------------------
bool Init( void *apFontData, int32_t aFontDataSize )
{
	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	if (apFontData) {
		pC64Font = io.Fonts->AddFontFromMemoryTTF(apFontData, aFontDataSize, FONTSIZE);
	}
	io.Fonts->Build();

	//If load failed, just use the default font
	if (pC64Font) {
		//Change fallback char used for unrecognized char to '.'
		pC64Font->FallbackGlyph = pC64Font->FindGlyph(static_cast<ImWchar>('.'));
	}
	else {
		pC64Font = ImGui::GetFont();
	}

	[[maybe_unused]] auto p = new Thread();

	LoadSettings();								//Load settings

	return true;
}

//----------------------------------------------------------------
VICESTATE ViceState(  )
{
	return eState;
}

//----------------------------------------------------------------
void Resume(  )
{
	Stopped = false;
	if (ViceState() == VICESTATE::STOPPED) {
		Send(Command::ExitCommand);
	}
}

//----------------------------------------------------------------
void FlushCommands( bool abWait )
{
	Thread::QInstance().Flush(abWait);			//Flush any pending commands
}

#include "MonitorMenus.ipp"

//----------------------------------------------------------------
void Display(  )
{
	//Early out of not initialize
	if (!Thread::QInitialized()) return;

	MainMenu();

	Registers::Display(Stopped);
	Code::Display(Stopped);
	Memory::Display(Stopped);
	Diagnostics::Display();
	Labels::Display();
	BreakPoints::Display();

	ImGui::SetNextWindowPos(ImVec2(268, 18), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(247, 78), ImGuiCond_FirstUseEver);
	ImGui::Begin("Monitor", nullptr
		, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize
		| ImGuiWindowFlags_NoResize);

	ImGui::Text(StateString[ViceState()]);
	ImGui::SameLine();
	if (ImGui::Checkbox("Stop", &Stopped)) {
		Send(Stopped ? Command::GetRegsCommand : Command::ExitCommand);
	}
	ImGui::SameLine(ImGui::GetWindowWidth() - 20.0f);;
	ImGui::Text("?");
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip(HelpText);
	}

	if (ImGui::Button("Mem0")) {
		Memory::DisplayOn(0);
	}
	ImGui::SameLine();
	if (ImGui::Button("Mem1")) {
		Memory::DisplayOn(1);
	}
	ImGui::SameLine();
	if (ImGui::Button("Labels")) {
		Labels::DisplayOn();
	}
	ImGui::SameLine();
	if (ImGui::Button("BreakPoints")) {
		BreakPoints::DisplayOn();
	}
	ImGui::End();

	FileDialog.Display();

	//File Dalog selection
	if (FileDialog.HasSelected()) {
		if (OpenProg) {
			Program::Load(FileDialog.GetSelected());
		}
		else {
			Labels::Load(FileDialog.GetSelected().string().c_str());
			Code::UpdateDisView();
		}
		FileDialog.ClearSelected();
	}

	Thread::QInstance().Flush();				//Flush any pending commands
}

//----------------------------------------------------------------
void ProcessResponses(  )
{
	bool needStart = false;
	uint32_t processed = 0;

	//Process all new responses in the queue
	Thread::QInstance().ProcessResponses([&]( const Response &arResponse ) {
		++processed;
		switch (arResponse.QCommand()) {
			//Memory responses go to either Memory View or Code View
			case COMMAND::MEMORY_GET:
				//NOTE: If we decide to have all views process responses
				// that fall within it's memory range, the if statement should
				// be removed.
				if (!Memory::FromResponse(arResponse)) {
					Code::FromResponse(arResponse);
				}
				break;
			//Checkpoints all go to BreakPoint module
			case COMMAND::CHECKPOINT_INFO:
				BreakPoints::ProcessInfo(arResponse);
				break;
			//Registers all go to Register View
			case COMMAND::REGISTERS_GET:
				Registers::FromResponse(arResponse);
				break;
			//All commands prompt a stopped response in addition to
			// the response to the command, so we resume if we
			// didn't wish to actually stop
			case COMMAND::STOPPED: {
				//Check if stopped because of a breakpoint
				auto ip = arResponse.Get16(0);
				//If hit breakpoint then stop
				if (BreakPoints::CheckHit(ip)) {
					Stopped = true;
				}
				else {
					//If we want to be running don't set the stopped state
					// as the monitor thread is going to start it back up
					needStart = !Stopped;
				}
				if (!needStart) {
					//If we request a stop the memory view needs to refresh
					// and enable the checkpoint to send responses on memory
					// change while stopped. We disable this checkpoint when
					// running to avoid potential spam of memory responses.
					// We could handle the responses alone if they didn't also
					// halt VICE, but we don't want that
					Memory::ViceRunning(false);
				}
				break;
			}
			case COMMAND::RESUMED:
				needStart = false;
				break;
			default:
				break;
		}
	});

	if (needStart) {
		Send(Command::ExitCommand);
	}
	if (processed) {
		CommDelay = 0;
	}
	else if (!Stopped) {
		auto c = ++CommDelay & 0xff;
		//This is about every 2 seconds
		if (c == 0x80) {
			CommDelay & 0x200
			? Thread::QInstance().ClearConnected()
			: Send(Command::PingCommand);		//Send a Ping so we get some sort of response
		}
	}
	UpdateStatus();								//Update connection/running status
}

//----------------------------------------------------------------
ImFont *C64Font(  )
{
	return pC64Font;
}

//----------------------------------------------------------------
void Close(  )
{
	SaveSettings();
//	Checkpoints::Close();
	BreakPoints::Close();						//Shut down BreakPoint tracking system
	Resume();									//Resume VICE before we exit or it will lock up
	Thread::ShutDown();
}

//----------------------------------------------------------------
void Send( CommandPtr apCommand )
{
	Thread::QInstance().PushCommand(apCommand);
}

}	//namespace Monitor




