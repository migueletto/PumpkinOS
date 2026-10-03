# PumpkinOS
PumpkinOS is a re-implementation of PalmOS that runs on modern architectures (x86, ARM, etc).
It is not your average PalmOS emulator (it does NOT require a PalmOS ROM), but it can run m68K PalmOS applications.
For a series of articles describing various aspects of PumpkinOS, look here: https://pmig96.wordpress.com/category/palmos/.

![](/screenshots/pumpkinos.png)

Launcher is the first application that runs when PumpkinOS starts. It shows a panel from which you can start other applications.
Preferences will eventually contain all preference options for configuring PumpkinOS.
Command is a command shell, still experimental. This release does not contain the four classic PIM applications from PalmOS.
They can still be compiled from source.

## Licensing
PumpkinOS is licensed under the GPL v3.
The license directory contains information on specific licenses of the various components used in PumpkinOS.
If you think something is missing and/or incorrect, please let me know.

## Building
You have to build PumpkinOS from source. No IDE is required, you can build from the command line.
If you use 64-bits Windows, you can use MSYS2 (https://www.msys2.org/). Download the installer and follow the instructions there.
Open a MINGW64 terminal (the one with the blue 'M' icon) and install these additional packages:

    pacman -S gcc binutils make git

If you are using a 64-bits Linux-based OS (like Debian, Ubuntu, etc), you also need gcc, binutils, make and git. If you are a developer,
there is a chance you already have those. If they are not installed, follow the instructions to download additional packages on your specific Linux distribution.
You must also install the SDL2 development package (the package that contains the libraries and the headers). On a Debian distribution the command is:

    sudo apt install gcc binutils make git libsdl2-dev

On Windows 11 and recent releases of Windows 10, it is also possible to build PumpkinOS on WSL2 (Windows Subsystem for Linux, version 2). 
Open a WSL2 terminal and follow the same instructions for a Linux build.

From here on, the instructions are the same for Linux, WSL2 and MINGW64. Clone the PumpkinOS repository:

    git clone https://github.com/migueletto/PumpkinOS.git

Go to the source directory of the PumpkinOS repository you have just cloned and run make:

    cd PumpkinOS/src
    make

If everything goes well, you will have a pumpkin executable in the root directory, some dynamic libraries in the bin directory, and some PRC files in the vfs/app_install directory.

## Running
On 64-bits Windows, run pumpkin.bat. On Linux or WSL2, run pumpkin.sh. PumpkinOS will open on a new window.

When you run PumpkinOS, all PRCs inside vfs/app_install will be removed and expanded into folders inside vfs/app_storage.
Please keep in mind that everything is pretty much experimental at this stage, so expect a few issues here and there.
On debug compilations, after either a successful or an unsuccessful run you will find a pumpkin.log file on the root directory.
If something goes wrong, look for lines marked with an "E" on the third column of this file.
You can reach me for questions (and send me your log file if you wish).

PumpkinOS supports Drag & Drop functionality. You can drag a PalmOS PRC file over its window and 
it will be installed and show up in the Launcher. You can also manually copy PRCs to the vfs/app_install directory and restart PumpkinOS.

If you really want to, you can debug PumpkinOS with gdb on Windows, Linux and WSL2. On Windows, edit pumpkin.bat and change the last line to:

    gdb.exe --args .\pumpkin.exe -d 1 -f pumpkin.log -s libscriptlua .\script\pumpkin.lua

On Linux and WSL2 edit pumpkin.sh and change the last line to:

    gdb --args ./pumpkin -d 1 -f pumpkin.log -s libscriptlua ./script/pumpkin.lua
