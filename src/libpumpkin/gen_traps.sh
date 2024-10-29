#!/bin/sh

# SYS_TRAP sysTrapStrIToA 0 Char_* StrIToA 2 Char_* s Int32 i
# SYS_TRAP sysTrapStrLen 0 UInt16 StrLen 1 const_Char_* src
# SEL_TRAP sysTrapIntlDispatch intlTxtCharAttr UInt16 TxtCharAttr 1 WChar inChar

#    case sysTrapDmGetResource: {
#      UInt32 type = sys_va_arg(ap, uint32_t);
#      UInt16 resID = sys_va_arg(ap, uint32_t);
#      MemHandle h = DmGetResource(type, resID);
#      *pret = h;
#      }
#      break;

awk -v trap=$1 '
BEGIN {
  insel = "";
}
/^#/ {
  next;
}
$1 == trap || ($1 !~ /LIB$/ && trap == "0") {
  if ($1 == "SEL_TRAP") {
    if (insel && insel != $2) {
      print "    }";
      print "    }";
      print "    break;";
      print "";
      insel = "";
    }
    if (!insel) {
      print "    case " $2 ": {";
      print "    switch (sel) {" 
      insel = $2;
    }
    print "    case " $3 ": {";
  } else {
    if (insel) {
      print "    }";
      print "    }";
      print "    break;";
      print "";
      insel = "";
    }
    print "    case " $2 ": {";
  }

  nargs = 0 + $6;
  for (i = 0; i < nargs; i++) {
    atype = $(7 + i*2);
    if (atype == "sys_va_list" || atype == "...") break;
    if (atype == "ErrJumpBuf") atype = "ErrJumpBufP";
    if (atype != "sys_va_list") {
      gsub("_", " ", atype);
    }
    s = "      " atype;
    if (!(atype ~ /[*]$/)) s = s " ";
    arg = $(7 + i*2 + 1);
    pos = index(arg, "[");
    if (pos > 0) {
      arg = "*" substr(arg, 1, pos-1);
      atype = atype " *";
    }
    if (substr(atype, 1, 6) == "const ") {
      atype = substr(atype, 7);
    }
    if (atype == "GetCharF" || atype == "PutStringF") {
      arg = "*" arg;
      atype = "void *";
    } else if (atype == "UInt8" || atype == "UInt16" || atype == "Boolean" || atype == "IndexedColorType" || atype == "FrameType" || atype == "WChar" || atype == "Err" || atype == "UIPickColorStartType" || atype == "SndSysBeepType" || atype == "SndSmfCmdEnum" || atype == "DmResID" || atype == "AttnLevelType" || atype == "FileOrigin" || atype == "UDABufferSize" || atype == "TsmFepModeType" || atype == "OmSelector" || atype == "LmLocaleSettingChoice" || atype == "TranslitOpType" || atype == "CharEncodingType" || atype == "IntlSelector") {
      atype = "UInt32";
    } else if (atype == "Char" || atype == "Int8" || atype == "Int16" || atype == "Coord" || atype == "SndStreamWidth" || atype == "SndSampleType" || atype == "SndStreamMode") {
      atype = "Int32";
    }
    s = s arg " = sys_va_arg(ap, " atype ");";
    print s;
  }
  rtype = $4;
  gsub("_", " ", rtype);
  if (rtype != "void") {
    s = "      " rtype;
    if (!(rtype ~ /[*]$/)) s = s " ";
    s = s "ret = " $5 "(";
  } else {
    s = "      " $5 "(";
  }

  if (nargs > 0) {
    for (i = 0; i < nargs; i++) {
      arg = $(7 + i*2 + 1);
      pos = index(arg, "[");
      if (pos > 0) arg = substr(arg, 1, pos-1);
      if (i > 0) s = s ", ";
      s = s arg;
    }
  }
  s = s ");";
  print s;
  if (rtype != "void") {
    if (rtype ~ /[*]$/ || rtype == "MemHandle" || rtype == "MemPtr" || rtype == "WinHandle" || rtype == "DmOpenRef" || rtype == "FileHand" || rtype == "NetHostInfoPtr" || rtype == "NetServInfoPtr") {
      print "      *pret = (void *)ret;";
    } else if (rtype == "FlpDouble") {
      print "      uint64_t *d = (uint64_t *)(&ret);";
      print "      *iret = *d;";
    } else {
      print "      *iret = ret;";
    }
  }
  print "      }";
  print "      break;";
  print "";
}
' traps.txt

exit 0
