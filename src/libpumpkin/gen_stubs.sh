#!/bin/sh

awk '
BEGIN {
  print "#include <PalmOS.h>";
  print "#include <PalmCompatibility.h>";
  print "#include <VFSMgr.h>";
  print "#include <ExpansionMgr.h>";
  print "#include <DLServer.h>";
  print "#include <SerialMgrOld.h>";
  print "#include <UDAMgr.h>";
  print "#include <PceNativeCall.h>";
  print "#include <FixedMath.h>";
  print "#include <FntGlue.h>";
  print "#include <TxtGlue.h>";
  print "#include <CPMLib.h>";
  print "#include <GPSLib68K.h>";
  print "#include <GPDLib.h>";
  print "#include <PdiLib.h>";
  print "#include <BtLib.h>";
  print "#include <FSLib.h>";
  print "#include <SslLib.h>";
  print "#include <INetMgr.h>";
  print "#include <SlotDrvrLib.h>";
  print "";
}
FILENAME == "syscalls_stubs.txt" {
  symbol[$1] = 1;
  next;
}
/^#/ {
  next;
} 
symbol[$5] {
  rtype = $4;
  gsub("_", " ", rtype);
  s = rtype;
  if (!(rtype ~ /[*]$/)) {
    s = s " ";
  }
  s = s $5 "(";
  nargs = 0 + $6;
  varargs = 0;
  lastarg = "";
  if (nargs == 0) {
    s = s "void";
  } else {
    for (i = 0; i < nargs; i++) {
      if (i > 0) s = s ", ";
      atype = $(7 + i*2);
      if (atype != "sys_va_list") {
        gsub("_", " ", atype);
      }
      s = s atype;
      if (atype == "...") {
        varargs = 1;
        break;
      }
      if (!(atype ~ /[*]$/)) {
        s = s " ";
      }
      lastarg = $(7 + i*2 + 1);
      s = s lastarg;
    }
  }
  s = s ") {";
  print s;
  if ($1 ~ /LIB$/) lib = $1; else lib = "0";
  if ($1 == "SYS_TRAP") sel = 0; else sel = $3;

  if (varargs == 1) {
    print "  sys_va_list ap;";
    print "  sys_va_start(ap, " lastarg ");";
  }

  if (rtype ~ /[*]$/ || rtype == "MemHandle" || rtype == "MemPtr" || rtype == "WinHandle" || rtype == "DmOpenRef" || rtype == "FileHand") {
    print "  void *pret;";
    s = "  pumpkin_system_call_p(" lib ", " $2 ", " sel ", NULL, &pret";
    if (nargs > 0) {
      for (i = 0; i < nargs; i++) {
        arg = $(7 + i*2 + 1);
        pos = index(arg, "[");
        if (pos > 0) arg = substr(arg, 1, pos-1);
        s = s ", " arg;
      }
    }
    s = s ");";
    print s;
    if (varargs == 1) {
      print "  sys_va_end(ap);";
    }
    print "  return (" rtype ")pret;";
  } else {
    if (rtype != "void") {
      print "  uint64_t iret;";
      s = "  pumpkin_system_call_p(" lib ", " $2 ", " sel ", &iret, NULL";
    } else {
      s = "  pumpkin_system_call_p(" lib ", " $2 ", " sel ", NULL, NULL";
    }
    if (nargs > 0) {
      for (i = 0; i < nargs; i++) {
        arg = $(7 + i*2 + 1);
        pos = index(arg, "[");
        if (pos > 0) arg = substr(arg, 1, pos-1);
        s = s ", " arg;
      }
    }
    s = s ");";
    print s;
    if (varargs == 1) {
      print "  sys_va_end(ap);";
    }
    if (rtype != "void") {
      print "  return (" rtype ")iret;";
    }
  }
  print "}";
  print "";
}
' $1 $2

exit 0
