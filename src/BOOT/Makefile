include ../common.mak
include ../commonp.mak

BOOT=BOOT.prc
TARGET=$(VFS)/app_install/BOOT.prc

$(BOOT): MBAR2710.bin
	$(PRCDUILD) -f $(BOOT) -t boot -c BOOT -n BOOT *.bin
	cp $(BOOT) $(TARGET)

MBAR2710.bin: boot.rcp
	$(PILRC) boot.rcp

clean:
	rm -f $(TARGET) $(BOOT) *.bin
