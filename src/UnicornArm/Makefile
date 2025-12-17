# On Linux: install the Unicorn development package (on Debian it is called libunicorn-dev)
# Call make on this Makefile

# On Windows: download the Unicorn binary release from the official repository on github.
# At the time of this writing, the latest release is https://github.com/unicorn-engine/unicorn/releases/download/2.1.4/windows-mingw64-shared.7z
# I have tested with 2.1.4 and 2.0.1, newer versions may or may not work.
# Extract the contents of the 7z archive on a local folder, for example /some/path/Unicorn.
# Call make on this Makefile with the command line: make UNICORN=/some/path/Unicorn
# Copy /some/path/Unicorn/bin/libunicorn.dll to PumpkinOS/lib

include ../common.mak

ifeq ($(OSNAME),Msys)
ifeq ($(UNICORN),)
$(error Missing UNICORN parameter (UNICORN must point to your Unicorn root directory))
else
UNICORN_INC=-I$(UNICORN)/include
UNICORN_LIB=-L$(UNICORN)/bin
endif
endif

SOURCE=ucarm.c
APPNAME=UnicornArm
APPID=ucAr
APPTYPE=plgi

CUSTOMPFLAGS=-I$(SRC)/libpumpkin/emulation -I$(SRC)/libpumpkin/emulation/arm $(UNICORN_INC)
EXTRALIBS=$(UNICORN_LIB) -lunicorn

include ../commonp.mak
include ../app.mak
