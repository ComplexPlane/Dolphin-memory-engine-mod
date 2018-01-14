# Dolphin Memory Engine

![Screenshot](https://raw.githubusercontent.com/aldelaro5/Dolphin-memory-engine/master/Docs/screenshot.png)

A RAM search specifically made to search, track, and edit the emulated memory of [the Dolphin emulator](https://github.com/dolphin-emu/dolphin) at runtime. The primary goal is to make research, tool-assisted speedruns, and reverse engineering of GameCube & Wii games more convenient and easier than with the alternative solution, Cheat Engine. Its name is derived from Cheat Engine to symbolize that.

It uses Qt 5 for the GUI and was made with convenience in mind, without disrupting the performance of the emulation.

For binary releases of this program, refer to [the "releases" page](https://github.com/aldelaro5/Dolphin-memory-engine/releases) on [the Github repository](https://github.com/aldelaro5/Dolphin-memory-engine).


## System requirements
Any x86_64 based system should theoretically work, however, please note that Mac OS is currently _not_ supported. Additionally, 32-bit x86 based systems are unsupported as Dolphin does not support them either. (Support for them was dropped a while ago.)

You absolutely need to have Dolphin running ***and*** _have the emulation started_ for this program to be of any use. As such, your system needs to meet Dolphin's [system requirements](https://github.com/dolphin-emu/dolphin#system-requirements). Additionally, have at least 250 MB of memory free.

On Linux, you need to install the Qt 5 package of your respective distribution.


## How to Build
### Microsoft Windows
> *You will need Microsoft Visual Studio 2015 or 2017 with the Visual C++ tools installed.  Previous versions may work, but are untested so preferably, make sure you use either of these versions.  The sources works best with the 2017 version, but it is possible to have the 2015 version work with a small adjustement.*

Before proceeding, make sure you have initialised the Qt submodule by running the command `git submodule update --init` at the repository's root.  The files should appear at the `Externals\Qt` directory.

Once this is done, open Visual Studio and open the solution located in the `Source` directory.  If you are using Visual Studio 2017, this is all you need to do, simply select the build configuration and build the solution.  If you are using Visual Studio 2015 however, you may have to change the toolset of the project to the one that comes with Visual Studio 2015.  To do so, right click on the project from the Solution Explorer and click properties.  From there, change the "Platform Toolset" to the one that you have installed.  Please note that this will change the settings in the `vcxproj` file so if you plan to submit a Pull Request, make sure to not stage this change.


### Linux
> _You will need CMake and Qt 5. Please refer to your distribution's documentation for specific instructions on how to install them if necessary._

To build for your system, simply run the following commands from the `Source` directory:

	mkdir build && cd build
	cmake ..
	make

The compiled binaries should be appear in the directory named `build`.


## General usage
First, open Dolphin and start a game, then run this program. Make sure that it reports that the Wii-only extra memory is present for Wii games and absent for GameCube games.

>_The Wii-only extra memory is an extra memory region exclusive to the Wii. Its presence affects what are considered valid watch addresses as well as where the scanner will look._

If the hooking process is successful, the UI should be enabled, otherwise, you will need to click the hook button before the program can be of any use.

Once hooked, you can do scans just like Cheat Engine as well as manage your watch list. You can save and load your watch list to disk by using the file menu. Note, the watch list files uses JSON internally so you can edit them manually with a text editor.

If the program unhooks itself from Dolphin, it means a read/write failed which normally means that the emulation has stopped in some way. Just boot a game again to solve this; your watch list and scan will be retained if this happens.

Finally, the program also includes a memory viewer which shows an hexadecimal view and an ASCII view of the memory.  Simply click on the coresponding button or right click on a watch to browse the memory using the memory viewer.


## License
This program is licensed under the MIT license which grants you the permission to anything with the software as long as you preserve all copyright notices. (See the file LICENSE for the legal text.)
