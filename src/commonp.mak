OSNAME    = PumpkinOS
PCC       = m68k-palmos-gcc
PILRC     = $(TOOLS)/pilrc
PRCDUILD  = $(TOOLS)/prcbuild
BUILDPRC  = build-prc
DLIBID    = $(shell expr $(SYS_OS) \* 64 + $(SYS_CPU) \* 8 + $(SYS_SIZE))
DLIB      = $(shell printf "dlib%04x.bin" $(DLIBID))

PALMOS    = $(SRC)/PalmOS
CUSTOMFLAGS=-DUNIX -DOSNAME=\"$(OSNAME)\" -I$(SRC)/libpumpkin -I$(SRC)/font -I$(PALMOS) -I$(PALMOS)/Core -I$(PALMOS)/Core/System -I$(PALMOS)/Dynamic -I$(PALMOS)/Core/UI -I$(PALMOS)/Libraries -I$(PALMOS)/Libraries/PalmOSGlue -I$(PALMOS)/Libraries/Pdi -I$(PALMOS)/Libraries/CPMLib -I$(PALMOS)/Libraries/INet -I$(PALMOS)/Libraries/SSL -I$(PALMOS)/Extensions/ExpansionMgr -I$(PALMOS)/Extensions/Bluetooth -I$(PALMOS)/Core/Hardware -I$(PALMOS)/Garmin $(CUSTOMPFLAGS)
