#!/bin/sh

#case sysTrapSysAppExit:
#  // Err SysAppExit(SysAppInfoPtr appInfoP, MemPtr prevGlobalsP, MemPtr globalsP)
#  debug(DEBUG_INFO, "EmuPalmOS", "SysAppExit called");
#  m68k_set_reg(M68K_REG_D0, 0);
#  m68k_pulse_halt();
#  emupalmos_finish(1);
#break;

#    case sysTrapSysAppExit:
#      palmos_FrmSysTrap(sp, idx, trap);
#      break;

awk '
BEGIN {
  ng = 0;
  casefile = "emulation/sc_case.c"
  protfile = "emulation/sc_prot.h"
  print "" > casefile;
  print "" > protfile;
}
$1 == "case" && $2 ~ /^sysTrap/ {
  len = length($2);
  trap = substr($2, 1, len-1);
  name = substr(trap, 8);
  len = length(name);
  group = substr(name, 1, 1);
  if (group == "U") {
    group = group substr(name, 2, 1);
  } else {
    for (i = 2; i <= len; i++) {
      c = substr(name, i, 1);
      if (c < "a" || c > "z") break;
      group = group c;
    }
  }
  if (group == "Sys") next;

  filename = "emulation/" group "SysTrap.c";

  if (!groups[group]) {
    idx[ng] = group;
    ng++;
    groups[group] = 1;
    items[group][0] = trap;
    nitems[group] = 1;
    print "#include <PalmOS.h>" > filename;
    print "#include <VFSMgr.h>" >> filename;
    print "#include <DLServer.h>" >> filename;
    print "#include <Helper.h>" >> filename;
    print "#include <CharAttr.h>" >> filename;
    print "#include <HsNavCommon.h>" >> filename;
    print "" >> filename;
    print "#include \"sys.h\"" >> filename;
    print "#ifdef ARMEMU" >> filename;
    print "#include \"armemu.h\"" >> filename;
    print "#include \"armp.h\"" >> filename;
    print "#endif" >> filename;
    print "#include \"pumpkin.h\"" >> filename;
    print "#include \"mutex.h\"" >> filename;
    print "#include \"storage.h\"" >> filename;
    print "#include \"logtrap.h\"" >> filename;
    print "#include \"m68k/m68k.h\"" >> filename;
    print "#include \"m68k/m68kcpu.h\"" >> filename;
    print "#include \"emupalmos.h\"" >> filename;
    print "#include \"debug.h\"" >> filename;
    print "" >> filename;
    print "void palmos_" group "SysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {" >> filename;
    print "" >> filename;
    print "  switch (trap) {" >> filename;
    print "    " $0 >> filename;
  } else {
    items[group][nitems[group]] = trap;
    nitems[group]++;
    print "    " $0 >> filename;
  }
  next;
}
group != "Sys" {
  print "    " $0 >> filename;
}
END {
  for (i = 0; i < ng; i++) {
    group = idx[i];
    print "extern void palmos_" group "SysTrap(uint32_t sp, uint16_t idx, uint32_t trap);" >> protfile;
  }
  for (i = 0; i < ng; i++) {
    group = idx[i];
    filename = "emulation/" group "SysTrap.c";
    print "  }" >> filename;
    print "}" >> filename;
    for (j = 0; j < nitems[group]; j++) {
      trap = items[group][j];
      print "    case " trap ":" >> casefile;
    }
    print "      palmos_" group "SysTrap(sp, idx, trap);" >> casefile;
    print "      break;" >> casefile;
  }
}
' emulation/switch.c

exit 0
