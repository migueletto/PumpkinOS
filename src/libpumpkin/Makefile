include ../common.mak
include ../commonp.mak

PROGRAM=$(BIN)/libpumpkin$(SOEXT)

CRDATE := $(shell date +%s)

GPSLIB=GPSLib68K.o

GPDLIB=GPDLib.o

PALMOS=$(SRC)/PalmOS

CUSTOMPFLAGS=-DCRDATE=$(CRDATE) -I./emulation -I./emulation/arm -I./emulation/darm -Wno-multichar -I./dlmalloc -DARMEMU

ARMOBJS=emulation/arm/armem.o emulation/arm/armemu.o emulation/arm/CPU.o emulation/arm/icache.o emulation/arm/MMU.o emulation/arm/RAM.o \
        emulation/arm/pxa_IC.o emulation/arm/cp15.o

DARMOBJS=emulation/darm/armv7.o emulation/darm/armv7-tbl.o emulation/darm/darm.o emulation/darm/darm-tbl.o emulation/darm/thumb.o \
         emulation/darm/thumb2.o emulation/darm/thumb2-decoder.o emulation/darm/thumb2-tbl.o emulation/darm/thumb-tbl.o

EMUOBJS=emulation/emupalmos.o emulation/omtrap.o emulation/pinstrap.o emulation/hdtrap.o emulation/serialtrap.o emulation/fstrap.o emulation/intltrap.o \
        emulation/flpemtrap.o emulation/flptrap.o emulation/accessortrap.o emulation/netlibtrap.o emulation/gpdlibtrap.o emulation/systrap.o \
        emulation/trapnames.o emulation/m68kcpu.o emulation/m68kdasm.o emulation/softfloat/softfloat.o emulation/m68kops.o emulation/disasm.o \
        $(ARMOBJS) $(DARMOBJS)

OBJS=pumpkin.o storage.o script.o fill.o AboutBox.o AddressSortLib.o AlarmMgr.o AttentionMgr.o Bitmap.o BmpGlue.o BtLib.o Category.o Clipboard.o ConnectionMgr.o ConsoleMgr.o Control.o CtlGlue.o CPMLib68KInterface.o Crc.o DateGlue.o DateTime.o Day.o DebugMgr.o DLServer.o Encrypt.o ErrorBase.o Event.o ExgLib.o ExgMgr.o ExpansionMgr.o FatalAlert.o FeatureMgr.o Field.o FileStream.o Find.o FloatMgr.o Font.o FontSelect.o FntGlue.o Form.o FrmGlue.o FSLib.o Graffiti.o GraffitiReference.o GraffitiShift.o HAL.o HostControl.o IMCUtils.o INetMgr.o InsPoint.o IntlMgr.o IrLib.o Keyboard.o KeyMgr.o Launcher.o List.o LstGlue.o LocaleMgr.o Localize.o Lz77Mgr.o Menu.o ModemMgr.o NetBitUtils.o NetMgr.o OverlayMgr.o Password.o PceNativeCall.o PdiLib.o PenInputMgr.o PenMgr.o PhoneLookup.o Preferences.o PrivateRecords.o Progress.o Rect.o ScrollBar.o SelTime.o SelDay.o SelTimeZone.o SerialLinkMgr.o SerialMgr.o SerialMgrOld.o SerialSdrv.o SerialVdrv.o SlotDrvrLib.o SoundMgr.o SslLib.o StringMgr.o SysEvtMgr.o SystemMgr.o SysUtils.o Table.o TblGlue.o TelephonyMgr.o TextMgr.o TxtGlue.o TextServicesMgr.o TimeMgr.o UDAMgr.o UIColor.o UIControls.o UIResources.o VFSMgr.o Window.o Chat.o dlheap.o dlmalloc/dlmalloc.o grail.o rgb.o dia.o wman.o dbg.o peditor.o syntax.o edit.o AppRegistry.o language.o rcpexport.o $(GPSLIB) $(GPDLIB) $(EMUOBJS)

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit -lm

emulation/systrap.o: emulation/systrap.c emulation/switch.c
	$(CC) $(CFLAGS) -c emulation/systrap.c -o emulation/systrap.o

clean:
	rm -f $(PROGRAM) $(OBJS)
