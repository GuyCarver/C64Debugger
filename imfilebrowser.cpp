
///From https://github.com/AirGuanZ/imgui-filebrowser

#include "imfilebrowser.h"
#include <windows.h>

//----------------------------------------------------------------
uint32_t ImGui::FileBrowser::GetDrivesBitMask()
{
	DWORD mask = GetLogicalDrives();
	uint32_t ret = 0;
	for(int i = 0; i < 26; ++i)
	{
		if(!(mask & (1 << i)))
		{
			continue;
		}
		char rootName[4] = { static_cast<char>('A' + i), ':', '\\', '\0' };
		UINT type = GetDriveTypeA(rootName);
		if(type == DRIVE_REMOVABLE || type == DRIVE_FIXED ||  type == DRIVE_REMOTE)
		{
			ret |= (1 << i);
		}
	}
	return ret;
}
