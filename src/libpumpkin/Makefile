include ../common.mak
include ../commonp.mak

CRDATE := $(shell date +%s)

CUSTOMPFLAGS=-DCRDATE=$(CRDATE) -I./emulation -I./emulation/arm -I./emulation/darm -I./tos -Wno-multichar -I../libpit/custom_malloc/ -DARMEMU -ffreestanding -DHEAP_DEBUG

GPSLIB_SOURCE=GPSLib68K.c

GPDLIB_SOURCE=GPDLib.c

ARM_SOURCE=emulation/arm/armem.c emulation/arm/armemu.c emulation/arm/CPU.c emulation/arm/icache.c emulation/arm/MMU.c emulation/arm/RAM.c \
        emulation/arm/pxa_IC.c emulation/arm/cp15.c

DARM_SOURCE=emulation/darm/armv7.c emulation/darm/armv7-tbl.c emulation/darm/darm.c emulation/darm/darm-tbl.c emulation/darm/thumb.c \
         emulation/darm/thumb2.c emulation/darm/thumb2-decoder.c emulation/darm/thumb2-tbl.c emulation/darm/thumb-tbl.c

EMU_SOURCE=emulation/emupalmos.c emulation/omtrap.c emulation/pinstrap.c emulation/hdtrap.c emulation/serialtrap.c emulation/fstrap.c emulation/intltrap.c \
        emulation/flpemtrap.c emulation/flptrap.c emulation/accessortrap.c emulation/expansiontrap.c emulation/tsmtrap.c emulation/netlibtrap.c emulation/gpdlibtrap.c \
        emulation/systrap.c emulation/trapnames.c emulation/m68k/m68kcpu.c emulation/m68k/m68kdasm.c emulation/softfloat/softfloat.c emulation/m68k/m68kops.c emulation/disasm.c \
        $(ARM_SOURCE) $(DARM_SOURCE)

TOS_SOURCE=tos/tos.c tos/gemdos_impl.c tos/bios_impl.c tos/xbios_impl.c tos/vdi.c tos/aes.c

GLUE_SOURCE=BmpGlue.c CtlGlue.c DateGlue.c FldGlue.c FntGlue.c FrmGlue.c LstGlue.c MemGlue.c TblGlue.c TxtGlue.c WinGlue.c

SOURCE=pumpkin.c pumpkin_syscall.c storage.c script.c fill.c AboutBox.c AddressSortLib.c AlarmMgr.c AttentionMgr.c Bitmap.c CharAttr.c ColorTable.c BtLib.c Category.c Clipboard.c ConnectionMgr.c ConsoleMgr.c Control.c CPMLib68KInterface.c Crc.c DateTime.c Day.c DebugMgr.c DLServer.c Encrypt.c md5.c sha1.c ErrorBase.c Event.c ExgLib.c ExgMgr.c ExpansionMgr.c FatalAlert.c FeatureMgr.c Field.c FileStream.c Find.c FixedMath.c FloatMgr.c Font.c FontSelect.c Form.c FSLib.c Graffiti.c GraffitiReference.c GraffitiShift.c HAL.c HostControl.c IMCUtils.c INetMgr.c InsPoint.c IntlMgr.c IrLib.c Keyboard.c KeyMgr.c Launcher.c List.c LocaleMgr.c Localize.c Lz77Mgr.c Menu.c ModemMgr.c NetBitUtils.c NetMgr.c OverlayMgr.c Password.c PceNativeCall.c PdiLib.c PenInputMgr.c PenMgr.c PhoneLookup.c Preferences.c PrivateRecords.c Progress.c Rect.c ScrollBar.c SelTime.c SelDay.c SelTimeZone.c SerialLinkMgr.c SerialMgr.c SerialMgrOld.c SerialSdrv.c SerialVdrv.c SlotDrvrLib.c SoundMgr.c SslLib.c StringMgr.c SysEvtMgr.c SystemMgr.c SysUtils.c Table.c TelephonyMgr.c TextMgr.c TextServicesMgr.c TimeMgr.c UDAMgr.c UIColor.c UIControls.c UIResources.c VFSMgr.c VFSExplorer.c Window.c Chat.c cmheap.c grail.c wav.c dia.c wman.c peditor.c syntax.c edit.c AppRegistry.c language.c calibrate.c unzip.c junzip.c puff.c plibc.c dosbox.c ssfn_font.c $(GLUE_SOURCE) $(GPSLIB_SOURCE) $(GPDLIB_SOURCE) $(EMU_SOURCE) $(TOS_SOURCE)

ifeq ($(OSNAME),Emscripten)

LAUNCHER=$(SRC)/Launcher
LAUNCHER_SOURCE=$(LAUNCHER)/Launcher.c $(LAUNCHER)/editstr.c $(LAUNCHER)/editbmp.c $(LAUNCHER)/editform.c $(LAUNCHER)/editbin.c $(LAUNCHER)/editsurf.c
LAUNCHER_OBJS=$(LAUNCHER_SOURCE:%.c=%.wasm)

PROGRAM=$(BIN)/libpumpkin.a
OBJS=$(SOURCE:%.c=%.wasm)
$(PROGRAM): $(OBJS) $(LAUNCHER_OBJS)
	@echo Creating libpumpkin.a
	@$(ARCMD) $(PROGRAM) $(OBJS) $(LAUNCHER_OBJS)

emulation/systrap.wasm: emulation/systrap.c emulation/switch.c
	@echo Compiling $<
	@$(EM_CC) $(CFLAGS) -c emulation/systrap.c -o emulation/systrap.wasm

else ifeq ($(OSNAME),Kernel)

LAUNCHER=$(SRC)/Launcher
LAUNCHER_SOURCE=$(LAUNCHER)/Launcher.c $(LAUNCHER)/editstr.c $(LAUNCHER)/editbmp.c $(LAUNCHER)/editform.c $(LAUNCHER)/editbin.c $(LAUNCHER)/editsurf.c
LAUNCHER_OBJS=$(LAUNCHER_SOURCE:%.c=%.o)

PROGRAM=$(BIN)/libpumpkin.a
OBJS=$(SOURCE:%.c=%.o)
$(PROGRAM): $(OBJS) $(LAUNCHER_OBJS)
	@echo Creating libpumpkin.a
	@$(ARCMD) $(PROGRAM) $(OBJS) $(LAUNCHER_OBJS)

emulation/systrap.o: emulation/systrap.c emulation/switch.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c emulation/systrap.c -o emulation/systrap.o

else

PROGRAM=$(BIN)/libpumpkin$(SOEXT)
OBJS=$(SOURCE:%.c=%.o)
$(PROGRAM): $(OBJS)
	@echo Linking libpumpkin$(SOEXT)
	@$(CC) -shared -o $(PROGRAM) $(OBJS) $(EXTLIBS) -L$(BIN) -lpit -lm

emulation/systrap.o: emulation/systrap.c emulation/switch.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c emulation/systrap.c -o emulation/systrap.o

endif

clean:
	@rm -f $(PROGRAM) $(OBJS) $(LAUNCHER_OBJS)
