#!/bin/sh

# SYS_TRAP sysTrapStrIToA 0 Char_* StrIToA 2 Char_* s Int32 i
# SYS_TRAP sysTrapStrLen 0 UInt16 StrLen 1 const_Char_* src

# inline Char *StrIToA(Char *s, Int32 i) {
#   void *pret;
#   pumpkin_system_call_p(0, sysTrapStrIToA, 0, NULL, &pret, s, i);
#   return (Char *)pret;
# }
#
# inline UInt16 StrLen(const Char *src) {
#   uint64_t iret;
#   pumpkin_system_call_p(0, sysTrapStrLen, 0, &iret, NULL, src);
#   return (UInt16)iret;
# }

awk -v trap=$1 '
BEGIN {
  print "#include <PalmOS.h>";
  print "#include <VFSMgr.h>";
  print "#include <ExpansionMgr.h>";
  print "#include <DLServer.h>";
  print "#include <SerialMgrOld.h>";
  print "#include <UDAMgr.h>";
  print "#include <PceNativeCall.h>";
  print "#include <FixedMath.h>";
  print "";
  print "inline void FrmCenterDialogs(Boolean center) {";
  print "  pumpkin_system_call_p(0, sysCallFrmCenterDialogs, 0, NULL, NULL, center);";
  print "}";
  print "";
}
/^#/ {
  next;
} 
$1 == trap || ($1 !~ /LIB$/ && trap == "0") {
  rtype = $4;
  gsub("_", " ", rtype);
  s = "inline " rtype;
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
    print("  sys_va_list ap;");
    print("  sys_va_start(ap, " lastarg ");");
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
      print("  sys_va_end(ap);");
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
      print("  sys_va_end(ap);");
    }
    if (rtype != "void") print "  return (" rtype ")iret;";
  }
  print "}";
  print "";
}
' traps.txt

exit 0
