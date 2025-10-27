#!/bin/sh

# SYS_TRAP sysTrapStrIToA 0 Char_* StrIToA 2 Char_* s Int32 i
# SYS_TRAP sysTrapStrLen 0 UInt16 StrLen 1 const_Char_* src
# SEL_TRAP sysTrapIntlDispatch intlTxtCharAttr UInt16 TxtCharAttr 1 WChar inChar

rm -f trapArgs.c

awk -v tt=$1 '
BEGIN {
  trapArgs = "trapArgs.c";

  ptr["DmOpenRef"]   = 1;
  ptr["FileRef"]     = 1;
  ptr["FileHand"]    = 1;
  ptr["ErrJumpBuf"]  = 1;
  ptr["GetCharF"]    = 1;
  ptr["PutStringF"]  = 1;
  ptr["sys_va_list"] = 1;
  ptr["..."]         = 1;

  tchar["Char"]      = 1;
  tchar["char"]      = 1;

  wchar["WChar"]     = 1;

  isize["Int8"]      = 1;
  isize["Int16"]     = 2;
  isize["Int32"]     = 4;
  isize["Coord"]     = 2;
  isize["FixedType"] = 4;
  isize["long"]      = 4;

  usize["Boolean"]   = 1;
  usize["UInt8"]     = 1;
  usize["UInt16"]    = 2;
  usize["UInt32"]    = 4;
  usize["AttnFlagsType"] = 4;
  usize["AttnLevelType"] = 2;
  usize["BitmapCompressionType"] = 1;
  usize["ClipboardFormatType"] = 1;
  usize["ControlStyleType"] = 1;
  usize["FontID"] = 1;
  usize["DmResID"] = 2;
  usize["DateFormatType"] = 1;
  usize["DateType"] = 2;
  usize["DlkCtlEnum"] = 1;
  usize["Err"] = 2;
  usize["EvtSetAutoOffCmd"] = 1;
  usize["FileOpEnum"] = 1;
  usize["FileOriginEnum"] = 1;
  usize["JustificationType"] = 1;
  usize["WinDirectionType"] = 1;
  usize["FormObjectKind"] = 1;
  usize["NumberFormatType"] = 1;
  usize["MenuCmdBarResultType"] = 1;
  usize["SystemPreferencesChoice"] = 1;
  usize["privateRecordViewEnum"] = 1;
  usize["SelectDayType"] = 1;
  usize["SndSmfCmdEnum"] = 1;
  usize["SndSysBeepType"] = 1;
  usize["SndStreamMode"] = 1;
  usize["SndStreamRef"] = 4;
  usize["SndSampleType"] = 2;
  usize["SndFormatType"] = 4;
  usize["SndStreamWidth"] = 1;
  usize["ReferenceType"] = 2;
  usize["KeyboardType"] = 1;
  usize["TableItemStyleType"] = 1;
  usize["TimeFormatType"] = 1;
  usize["UIColorTableEntries"] = 1;
  usize["UIPickColorStartType"] = 2;
  usize["WinDrawOperation"] = 1;
  usize["WindowFormatType"] = 1;
  usize["FrameType"] = 2;
  usize["IndexedColorType"] = 1;
  usize["WinLockInitType"] = 1;
  usize["WinScreenModeOperation"] = 1;
  usize["PatternType"] = 1;
  usize["UnderlineModeType"] = 1;
  usize["ButtonFrameType"] = 1;
  usize["CncProfileID"] = 1;
  usize["FontDefaultType"] = 1;
  usize["WinScreenAttrType"] = 1;
  usize["HostBoolType"] = 4;
  usize["HostSignalType"] = 4;
  usize["HostControlSelectorType"] = 2;
  usize["HostBoolType"] = 4;
  usize["HostClockType"] = 4;
  usize["HostErrType"] = 4;
  usize["HostIDType"] = 4;
  usize["HostPlatformType"] = 4;
  usize["HostSignalType"] = 4;
  usize["HostSizeType"] = 4;
  usize["HostTimeType"] = 4;
  usize["IntlSelector"] = 2;
  usize["CharEncodingType"] = 1;
  usize["TranslitOpType"] = 2;
  usize["LmLocaleSettingChoice"] = 2;
  usize["OmSelector"] = 2;
  usize["TsmFepModeType"] = 2;
  usize["UDABufferSize"] = 2;
  usize["FileOrigin"] = 2;
  usize["LocalIDKind"] = 1;
  usize["HostErr"] = 4;
  usize["FlpFloat"] = 4;
  usize["FlpDouble"] = 8;

  dmid["DmResType"] = 1;
  rgb["RGBColorType"] = 1;
  evt["EventType"] = 1;
  rct["RectangleType"] = 1;
  datetime["DateTimeType"] = 1;
  bmp["BitmapType"] = 1;
  localid["LocalID"] = 1;

  accessor[0] = "CtlGlueGetControlStyle";
  accessor[1] = "FldGlueGetLineInfo";
  accessor[2] = "FrmGlueGetObjectUsable";
  accessor[3] = "BmpGlueGetCompressionType";
  accessor[4] = "BmpGlueGetTransparentValue";
  accessor[5] = "BmpGlueSetTransparentValue";
  accessor[6] = "CtlGlueGetFont";
  accessor[7] = "CtlGlueSetFont";
  accessor[8] = "CtlGlueGetGraphics";
  accessor[9] = "CtlGlueNewSliderControl";
  accessor[10] = "CtlGlueSetLeftAnchor";
  accessor[11] = "FrmGlueGetDefaultButtonID";
  accessor[12] = "FrmGlueSetDefaultButtonID";
  accessor[13] = "FrmGlueGetHelpID";
  accessor[14] = "FrmGlueSetHelpID";
  accessor[15] = "FrmGlueGetMenuBarID";
  accessor[16] = "FrmGlueGetLabelFont";
  accessor[17] = "FrmGlueSetLabelFont";
  accessor[18] = "FrmGlueGetEventHandler";
  accessor[19] = "LstGlueGetFont";
  accessor[20] = "LstGlueSetFont";
  accessor[21] = "LstGlueGetItemsText";
  accessor[22] = "LstGlueSetIncrementalSearch";
  accessor[23] = "TblGlueGetColumnMasked";
  accessor[24] = "WinGlueGetFrameType";
  accessor[25] = "WinGlueSetFrameType";
  accessor[26] = "FrmGlueGetObjIDFromObjPtr";
  accessor[27] = "LstGlueGetDrawFunction";
  accessor[28] = "CtlGlueIsGraphical";
  accessor[29] = "CtlGlueSetFrameStyle";

  print "#define T_VOID  0" >> trapArgs;
  print "#define T_SIG   1" >> trapArgs;
  print "#define T_USIG  2" >> trapArgs;
  print "#define T_CHAR  3" >> trapArgs;
  print "#define T_WCHR  4" >> trapArgs;
  print "#define T_HEX   5" >> trapArgs;
  print "#define T_ID    6" >> trapArgs;
  print "#define T_STR   7" >> trapArgs;
  print "#define T_RGB   8" >> trapArgs;
  print "#define T_EVT   9" >> trapArgs;
  print "#define T_RCT  10" >> trapArgs;
  print "#define T_LOC  11" >> trapArgs;
  print "#define T_DATE 12" >> trapArgs;
  print "#define T_BMP  13" >> trapArgs;
  print "" >> trapArgs;

  print "#define OMIT_IN  1" >> trapArgs;
  print "#define OMIT_OUT 2" >> trapArgs;
  print "" >> trapArgs;

  print "static trap_t trapArgs[] = {" >> trapArgs;
}
/^#/ {
  next;
}
$1 == tt || ($1 !~ /LIB$/ && tt == "0") {
  trap = $2;
  nargs = 0 + $6;
  selector = $3;

  if (selector == "0" && $2 !~ /Dispatch$/) {
    if (trap ~ /^sysTrap/) name = substr(trap, 8);
    else if (trap ~ /^sysLib/) name = substr(trap, 4);
    else name = "ERROR_" trap;
  } else {
    name = selector;
    if (name ~ /^sysTrap/) name = substr(name, 8);
    else if (trap == "sysTrapExpansionMgr")  name = "E" substr(name, 2);
    else if (trap == "sysTrapFlpEmDispatch") name = substr(name, 11);
    else if (trap == "sysTrapFlpDispatch")   name = substr(name, 4);
    else if (trap == "sysTrapHighDensityDispatch") name = substr(name, 11);
    else if (trap == "sysTrapHostControl")   name = "Host" substr(name, 13);
    else if (trap == "sysTrapIntlDispatch")  name = substr(name, 5);
    else if (trap == "sysTrapLmDispatch")    name = "L" substr(name, 2);
    else if (trap == "sysTrapOmDispatch")    name = "O" substr(name, 2);
    else if (trap == "sysTrapPinsDispatch")  name = substr(name, 4);
    else if (trap == "sysTrapSerialDispatch") name = substr(name, 4);
    else if (trap == "sysTrapTsmDispatch")   name = "T" substr(name, 2);
    else if (trap == "sysTrapUdaMgrDispatch") name = substr(name, 4);
    else if (trap == "sysTrapVFSMgr") name = "VFS" substr(name, 8);
    else if (trap == "sysTrapAccessorDispatch") {
      if (accessor[name]) name = accessor[name];
      else name = "Accessor" name;
    }
  }

  rtype = $4;
  if (substr(rtype, 1, 6) == "const_") {
    rtype = substr(rtype, 7);
  }
  rtyp = "T_SIG";
  isptr = 0;

  if (rtype ~ /[*]$/) {
    rtype = substr(rtype, 1, length(rtype) - 2);
    if (bmp[rtype]) {
      rtyp = "T_BMP";
    } else {
      rtyp = "T_VOID";
    }
    size = 4;
    isptr = 1;
  } else if (rtype ~ /Ptr$/ || rtype ~ /Handle$/ || rtype ~ /Callback/) {
    size = 4;
    isptr = 1;
    rtyp = "T_VOID";
  } else if (ptr[rtype]) {
    size = 4;
    isptr = 1;
    rtyp = "T_VOID";
  } else if (tchar[rtype]) {
    size = 1;
    rtyp = "T_CHAR";
  } else if (wchar[rtype]) {
    size = 2;
    rtyp = "T_WCHR";
  } else if (isize[rtype]) {
    size = isize[rtype];
    rtyp = "T_SIG";
  } else if (usize[rtype]) {
    size = usize[rtype];
    rtyp = "T_USIG";
  } else if (rtype == "void") {
    size = 0;
    rtyp = "T_VOID";
  } else if (dmid[rtype]) {
    size = 4;
    rtyp = "T_ID";
  } else if (localid[rtype]) {
    size = 4;
    rtyp = "T_LOC";
  } else {
    size = "ERROR_" rtype;
  }

  if (selector == "0" && trap !~ /Dispatch$/) selector = "-1";
  s = "  { .trap=" trap ", .selector=" selector ", .name=\"" name "\", .rtype=" rtyp ", .rptr=" isptr ", .rsize=" size ", .nargs=" nargs ", .args={ ";

  for (i = 0; i < nargs; i++) {
    atype = $(7 + i*2);
    arg = $(7 + i*2 + 1);
    isptr = 0;

    io = substr(arg, 1, 1);
    if (io ~ /[012]/) {
      arg = substr(arg, 2);
      if (io == "1") io = "OMIT_IN";
      else if (io == "2") io = "OMIT_OUT";
    } else {
      io = "0";
    }

    if (substr(atype, 1, 6) == "const_") {
      atype = substr(atype, 7);
    }
 
    j = index(atype, "_*");
    if (j > 0) {
      atype = substr(atype, 1, j-1);
      atyp = "T_VOID";
      size = 4;
      isptr = 1;
    } else if (index(arg, "[")) {
      atyp = "T_VOID";
      size = 4;
      isptr = 1;
    } else {
      atyp = "T_SIG";
    }

    if (atype ~ /Ptr$/ || atype ~ /Handle$/ || atype ~ /Callback/) {
      atyp = "T_VOID";
      size = 4;
      isptr = 1;
    } else if (ptr[atype]) {
      size = 4;
      isptr = 1;
      atyp = "T_VOID";
    } else if (rgb[atype]) {
      atyp = "T_RGB";
    } else if (evt[atype]) {
      atyp = "T_EVT";
    } else if (rct[atype]) {
      atyp = "T_RCT";
    } else if (datetime[atype]) {
      atyp = "T_DATE";
    } else if (bmp[atype]) {
      atyp = "T_BMP";
    } else if (localid[atype]) {
      atyp = "T_LOC";
      size = 4;
    } else if (tchar[atype]) {
      if (isptr) {
        atyp = "T_STR";
      } else {
        atyp = "T_CHAR";
        size = 1;
      }
    } else if (wchar[atype]) {
      atyp = "T_WCHR";
      size = 2;
    } else if (isize[atype]) {
      atyp = "T_SIG";
      size = isize[atype];
    } else if (usize[atype]) {
      atyp = "T_USIG";
      size = usize[atype];
    } else if (dmid[atype]) {
      atyp = "T_ID";
      size = 4;
    } else if (!isptr) {
      size = "ERROR_" atype;
    }

    if (atyp == "T_USIG" && (arg == "flags" || arg ~ /Flags$/ || arg ~ /^attributes/)) {
      atyp = "T_HEX";
    }

    if (i > 0) s = s ", ";
    s = s "{ .type=" atyp ", .ptr=" isptr ", .size=" size ", .io=" io ", .name=\"" arg "\" }";
  }

  s = s " } },";
  print s >> trapArgs;
}
END {
  print "  { .name = NULL } " >> trapArgs;
  print "};" >> trapArgs;
}
' traps.txt

exit 0
