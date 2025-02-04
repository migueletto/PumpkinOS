#!/bin/sh

# SYS_TRAP sysTrapWinRGBToIndex 0 IndexedColorType WinRGBToIndex 1 const_RGBColorType_* rgbP
# SEL_TRAP sysTrapAccessorDispatch 2 Boolean FrmGlueGetObjectUsable 2 const_FormType_* formP UInt16 objIndex

# uint16_t trap, selector;
# char *name;
# char *rtype;
# int nargs;
# char *atype[16];
# char *aname[16];
# char *ptr;
# struct trap_t *dispatch;

awk '
BEGIN {
  OFS = ", ";
}
/^#/ {
  next;
}
$1 == "SYS_TRAP" || $1 == "SEL_TRAP" {
  trap = $2;
  selector = $3;
  t = $4;
  if (substr(t,1,6) == "const_") t = substr(t,7);
  rtype = "\"" t "\"";
  name = "\"" $5 "\"";
  nargs = 0 + $6;
  ptr = "";
  j = 0;
  for (i = 0; i < nargs; i++) {
    t = $(7+j);
    if (substr(t,1,6) == "const_") t = substr(t,7);
    len = length(t);
    if (substr(t,len-1) == "_*") {
      t = substr(t,1,len-2);
      ptr = ptr "*";
    } else {
      ptr = ptr " ";
    }
    n = $(7+j+1);
    atype[i] = "\"" t "\"";
    aname[i] = "\"" n "\"";
    j += 2;
  }
  if ($1 == "SEL_TRAP") {
    dispatch = $2 "Table";
  } else {
    dispatch = "NULL";
  }
  atypes = "{";
  for (i = 0; i < nargs; i++) {
    if (i > 0) atypes = atypes ",";
    atypes = atypes " " atype[i];
  }
  atypes = atypes " }";
  anames = "{";
  for (i = 0; i < nargs; i++) {
    if (i > 0) anames = anames ",";
    anames = anames " " aname[i];
  }
  anames = anames " }";
  ptr = "\"" ptr "\"";
  print "  { " trap, selector, name, rtype, nargs, atypes, anames, ptr, dispatch " },";
}
' traps.txt

exit 0
