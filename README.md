C64Debugger
=====

C64Debugger is a Windows only binary monitor for the [VICE emulator](https://sourceforge.net/projects/vice-emu/).
This is a Visual Studio 2022 Windows project building with the C++20 toolset.

## Submodules/includes
 - [ImGui](https://github.com/ocornut/imgui) - Now cloned as I made modifications to support Full Ascii char set.
 - [Nlohmann Json](https://github.com/nlohmann/json)
 - [ImGui File Browser](https://github.com/AirGuanZ/imgui-filebrowser)

## Usage
 - I use [KickAssembler](http://www.theweb.dk/KickAssembler/Main.html#frontpage) for my projects so symbols are loaded from a .vs file.
 - **VICE** must be started with the **-binarymonitor** command line option.
 - Currently only supports localhost connection.

### Command Line Options
 - **-p pathto/file.prg** - Open and run given file on VICE and load file.vs symbols file. *VICE must already be running.*

### Key Commands
Key commands reflect the default commands in Visual Studio
 - **F5** - Run
 - **F8** - Toggle Follow IP - The Code View may either follow the IP address or remain where the user sets it.
 - **F9** - Add/Remove Breakpoint
 - **Ctrl+F9** - Toggle Breakpoint
 - **F10** - Step Over
 - **F11** - Step Into
 - **Shift+F11** - Step Out

### TODO:
 - Remote connection support
 - Memory Breakpoints
 - Label support in Assembler
