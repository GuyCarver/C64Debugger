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
// FILE    MonitorMenus.ipp
// Compile: Monitor.cpp
//----------------------------------------------------------------------

//----------------------------------------------------------------
void FileMenu(  )
{
	if (ImGui::MenuItem("Vice Path", "Ctrl+V")) {
		pFileDialogResult = []( const std::filesystem::path aSelected ) {
			VicePath = aSelected.string();
		};
		FileDialog.SetTitle("Vice Exe");
		FileDialog.SetTypeFilters({".exe"});
		std::filesystem::path vp(VicePath);

		FileDialog.SetPwd(vp.remove_filename());
		FileDialog.Open();
	}

	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip(VicePath.c_str());
	}
	ImGui::MenuItem("AutoStart", "", &bAutoStartVice);

	ImGui::Separator();

	if (ImGui::MenuItem("Open Prg", "Ctrl+O")) {
		pFileDialogResult = []( const std::filesystem::path aSelected ) {
			Program::Load(aSelected);
		};
		FileDialog.SetTitle("Open Program");
		FileDialog.SetTypeFilters({".prg"});
		FileDialog.Open();
	}

	if (ImGui::MenuItem("Open Labels", "Ctrl+L")) {
		pFileDialogResult = []( const std::filesystem::path aSelected ) {
			Labels::Load(aSelected.string().c_str());
			Code::UpdateDisView();
		};
		FileDialog.SetTitle("Open Labels");
		FileDialog.SetTypeFilters({".vs"});
		FileDialog.Open();
	}

	ImGui::Separator();

	if (ImGui::MenuItem("Soft Reset", "Ctrl+R")) {
		Send(Command::SoftResetCommand);
	}
	if (ImGui::MenuItem("Hard Reset", "Ctrl+Shift+R")) {
		Send(Command::HardResetCommand);
	}
}

//----------------------------------------------------------------
void ViewMenu(  )
{
	if (ImGui::MenuItem("Diagnostics", "Ctrl+D")) {
		Diagnostics::DisplayOn();
	}
}

//----------------------------------------------------------------
void MainMenu(  )
{
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Vice")) {
			FileMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			ViewMenu();
			ImGui::EndMenu();
		}
	}
	ImGui::EndMainMenuBar();
}

