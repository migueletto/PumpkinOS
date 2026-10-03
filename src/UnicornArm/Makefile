# On Linux: install the Unicorn development package (on Debian it is called libunicorn-dev)
# Call this Makefile

# You can also download the Unicorn source code and compile it. In this case, you can specify just the "arm" architecture
# and the resulting library will be much smaller. The commands to compile Unicorn from source are:

# mkdir build
# cd build
# cmake -DUNICORN_ARCH="arm" -DCMAKE_BUILD_TYPE=Release ..

# After that, call this Makefile setting the UNICORN variable to the Unicorn source directory (without the 'build' subdirectory):
# make UNICORN=/some/path/Unicorn
# And it will use libunicorn.a, the static library you just compiled, avoiding the dependency on any other external library.

# On Windows: download the Unicorn binary release from the official repository on github.
# At the time of this writing, the latest release is https://github.com/unicorn-engine/unicorn/releases/download/2.1.4/windows-mingw64-shared.7z
# I have tested with 2.1.4 and 2.0.1, newer versions may or may not work.
# Extract the contents of the 7z archive on a local folder, for example /some/path/Unicorn.
# Call this Makefile with the command line:
# make UNICORN=/some/path/Unicorn
# Copy /some/path/Unicorn/bin/libunicorn.dll to PumpkinOS/lib

include ../common.mak

ifeq ($(OSNAME),GNU/Linux)
ifeq ($(UNICORN),)
UNICORN_LIB=-lunicorn
else
UNICORN_INC=-I$(UNICORN)/include
UNICORN_LIB=$(UNICORN)/build/libunicorn.a
endif
endif

ifeq ($(OSNAME),Msys)
ifeq ($(UNICORN),)
$(error Missing UNICORN parameter (UNICORN must point to your Unicorn root directory))
else
UNICORN_INC=-I$(UNICORN)/include
UNICORN_LIB=-L$(UNICORN)/bin -lunicorn
endif
endif

ifeq ($(OSNAME),Haiku)
ifeq ($(UNICORN),)
$(error Missing UNICORN parameter (UNICORN must point to your Unicorn root directory))
else
UNICORN_INC=-I$(UNICORN)/include
UNICORN_LIB=-L$(UNICORN)/build -lunicorn
endif
endif

SOURCE=ucarm.c
APPNAME=UnicornArm
APPID=ucAr
APPTYPE=plgi

CUSTOMPFLAGS=-I$(SRC)/libpumpkin/emulation -I$(SRC)/libpumpkin/emulation/arm $(UNICORN_INC)
EXTRALIBS=$(UNICORN_LIB)

include ../commonp.mak
include ../app.mak
