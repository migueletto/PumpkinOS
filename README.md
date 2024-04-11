# PumpkinOS
PumpkinOS is a re-implementation of PalmOS that runs on modern architectures (x86, ARM, etc).
It is not your average PalmOS emulator (it does NOT require a PalmOS ROM), but it can run m68K PalmOS applications.
For a series of articles describing various aspects of PumpkinOS, look here: https://pmig96.wordpress.com/category/palmos/.

![](/screenshots/pumpkin.png)

Launcher is the first application that runs when PumpkinOS starts. It shows a panel from which you can start other applications.
Preferences will eventually contain all preference options for configuring PumpkinOS.
Command is a command shell, still experimental.

This release contains the four PIM applications found on PalmOS: AddressBook, MemoPad, ToDoList and DateBook. The source code for these applications
were distributed in one or more PalmOS SDks and were adapted for correct compilation on PumpkinOS.
Records created by AddressBook and MemoPad should be compatible with their PalmOS counterparts. Because of differences in
word size en endianness, however, records created by ToDoList and DateBook are not compatible.
These applications were tested just to the point where I could create and edit a few records. There are still some quirks, and some functions were not tested at all.
The goal here is just to offer a view of what to expect from PumpkinOS in the future.

I am planing to setup a bug tracker to document enhancements and bugs.

## Licensing
PumpkinOS is licensed under the GPL v3.
The license directory contains information on specific licenses of the various components used in PumpkinOS.
If you think something is missing and/or incorrect, please let me know.

## Building
You have to build PumpkinOS from source. No IDE is required, you can build from the command line.
If you use 64-bits Windows, you can use MSYS2 (https://www.msys2.org/). Download the installer and follow the instructions there.
Open a MINGW64 terminal (the one with the blue 'M' icon) and install these additional packages:

    pacman -S gcc binutils make git

Next clone the PumpkinOS repository:

    git clone https://github.com/migueletto/PumpkinOS.git

Finally go to the source directory of the PumpkinOS repository you have just cloned and run the make script:

    cd PumpkinOS/src
    ./mk.sh Msys 64

If everything goes well, you will have a pumpkin.exe in the root directory, some DLLs in the bin directory, and some PRC files in the vfs/app_install directory.

There is also experimental support for 32-bits Windows (Vista or later. It will not work on Windows XP).
Open a MINGW32 terminal (the one with the gray 'M' icon) and install this additional package:

    pacman -S mingw-w64-i686-gcc

From there, compile using (note that now argument is 32, for 32-bits):

    cd PumpkinOS/src
    ./mk.sh Msys 32

If you are using a 64-bits Linux-based OS (like Debian, Ubuntu, etc), you also need gcc, binutils, make and git. If you are a developer,
there is a chance you already have those. If they are not installed, follow the instructions to download additional packages on your specific Linux distribution.
You must also install the SDL2 development package (the package that contains the libraries and the headers). On a Debian distribution, it is probably something like:

    sudo apt install gcc binutils make git libsdl2-dev

Again, you must clone the repository and compile it using:

    cd PumpkinOS/src
    ./mk.sh GNU/Linux 64

On Windows 11 and recent releases of Windows 10, it is also possible to build PumpkinOS on WSL (Windows Subsystem for Linux, version 2). 
Open a WSL terminal and follow the same instructions for a Linux build.

If you are using an Apple Silicon-based Mac (anything with an M1, M2, etc, processor), you will need Xcode and the SDL2 libraries. You can install them using Homebrew with something like:

    brew install sdl2

Then, clone the respository and compile it using:

    cd PumpkinOS/src
    ./mk.sh Darwin 64

## Running
On 64-bits Windows, run pumpkin.bat. On 32-bits Windows, run pumpkin32.bat. On Linux or WSL, run pumpkin.sh. On macOS, run pumpkin_macos.sh. PumpkinOS will open on a new window.
On WSL you may need to run a X-Window Manager, otherwise the PumpkinOS window will not have a border.

When you run PumpkinOS, all PRCs inside vfs/app_install will be removed and expanded into folders inside vfs/app_storage.
Please keep in mind that everything is pretty much experimental at this stage, so expect a few issues here and there.
After either a successful or an unsuccessful run, you will find a pumpkin.log file on the root directory.
If something goes wrong, look for lines marked with an "E" on the third column of this file.
You can reach me for questions (and send me your log file if you wish).

The Windows version implements Drag & Drop functionality. You can drag a PalmOS PRC over the PumpkinOS window and hopefully
it will be installed and show up in the Launcher. The Linux version lacks this functionality. For now, you have to manually copy PRCs
to the vfs/app_install directory and restart PumpkinOS.

If you really want to, you can debug PumpkinOS with gdb on Windows, Linux and WSL. On Windows, edit pumpkin.bat and change the last line to (you should also add the Windows equivalent of the /usr/bin directory of your MSYS2 installation the the PATH):

    gdb.exe --args .\pumpkin.exe -d 1 -f pumpkin.log -s libscriptlua.dll script\pumpkin_windows.lua

On Linux and WSL edit pumpkin.sh and change the last line to:

    gdb --args ./pumpkin -d 1 -f pumpkin.log -s libscriptlua.so ./script/pumpkin_linux.lua

I am writing a full Wiki article on source level debuging PumpkinOS.
