#!/bin/sh

# 73 int32_t Mfree 1 void@* block

#    case 73: { // int32_t Mfree(void *block)
#        uint32_t addr = ARG32;
#        int32_t res = -1;
#        m68k_set_reg(M68K_REG_D0, res);
#        debug(DEBUG_TRACE, "TOS", "GEMDOS Mfree(0x%08X): %d", addr, res);
#      }
#      break;

prefix=$1

if [ -z "$prefix" ]; then
  echo "usage: $0 <prefix>"
  exit 0
fi

rm -f $prefix"_proto1.h" $prefix"_case1.c" $prefix"_impl1.c"

awk -v prefix=$prefix '
BEGIN {
  filename_proto = prefix "_proto1.h";
  filename_case  = prefix "_case1.c";
  filename_impl  = prefix "_impl1.c";

  print "#include <PalmOS.h>" >> filename_impl;
  print "" >> filename_impl;
  print "#include \"plibc.h\"" >> filename_impl;
  print "#include \"gemdos_proto.h\"" >> filename_impl;
  print "" >> filename_impl;
}
/^#/ {
  next;
}
{
  num = $1;
  rtype = $2;
  fname = $3;
  nargs = 0 + $4;

  gsub("@", " ", rtype);
  s = rtype;
  if (!(rtype ~ /[*]$/)) s = s " ";
  s = s fname "(";
  if (nargs > 0) {
    for (i = 0; i < nargs; i++) {
      if (i > 0) s = s ", ";
      atype = $(5 + i*2);
      gsub("@", " ", atype);
      s = s atype;
      if (atype == "...") break;
      if (!(atype ~ /[*]$/)) s = s " ";
      arg = $(5 + i*2 + 1);
      s = s arg;
    }
  } else {
    s = s "void";
  }
  s = s ")";

  print s ";" >> filename_proto;

  print s " {" >> filename_impl;
  print "  debug(DEBUG_ERROR, \"TOS\", \"" fname " not implemented\");" >> filename_impl;
  if (rtype != "void") {
    print "  return 0;" >> filename_impl;
  }
  print "}" >> filename_impl;
  print "" >> filename_impl;

  needs_valid = 0;
  for (i = 0; i < nargs; i++) {
    atype = $(5 + i*2);
    if (atype ~ /[*]$/) needs_valid = 1;
  }

  print "    case " num ": { // " s >> filename_case;
  if (needs_valid == 1) {
    print "        int valid = 0;" >> filename_case;
  }

  for (i = 0; i < nargs; i++) {
    atype = $(5 + i*2);
    gsub("@", " ", atype);
    if (atype == "...") break;
    arg = $(5 + i*2 + 1);

    if (atype ~ /[*]$/) {
      print "        uint32_t a" arg " = ARG32;" >> filename_case;
      print "        " atype arg " = (" atype ")(ram + a" arg ");" >> filename_case;
      print "        valid |= (uint8_t *)" arg " >= data->block && (uint8_t *)" arg " < data->block + data->blockSize;" >> filename_case;

    } else {
      s = "        " atype;
      if (!(atype ~ /[*]$/)) s = s " ";
      if (atype == "int32_t" || atype == "uint32_t") {
        argn = "ARG32";
      } else if (atype == "int16_t" || atype == "uint16_t") {
        argn = "ARG16";
      } else {
        argn = "ARG8";
      }
      s = s arg " = " argn ";";
      print s >> filename_case;
    }
  }

  if (rtype != "void") {
    if (rtype ~ /[*]$/) {
      print "        " rtype "res = NULL;" >> filename_case;
    } else {
      print "        " rtype " res = 0;" >> filename_case;
    }
  }

  if (needs_valid == 1) {
    print "        if (valid) {" >> filename_case;
  }

  s = "        ";
  if (needs_valid == 1) s = s "  ";
  if (rtype != "void") {
    s = s "res = ";
  }
  s = s fname "(";
  for (i = 0; i < nargs; i++) {
    arg = $(5 + i*2 + 1);
    if (i > 0) s = s ", ";
    s = s arg;
  }
  s = s ");";
  print s >> filename_case;

  if (needs_valid == 1) {
    print "        }" >> filename_case;
  }

  if (rtype != "void") {
    if (rtype ~ /[*]$/) {
      print "        uint32_t ares = res ? ((uint8_t *)res - ram) : 0;" >> filename_case;
      print "        m68k_set_reg(M68K_REG_D0, ares);" >> filename_case;
    } else {
      print "        m68k_set_reg(M68K_REG_D0, res);" >> filename_case;
    }
  }

  s = "        debug(DEBUG_TRACE, \"TOS\", \"GEMDOS " fname "(";
  for (i = 0; i < nargs; i++) {
    atype = $(5 + i*2);
    if (i > 0) s = s ", ";
    if (atype ~ /[*]$/) s = s "0x%08X"; else s = s "%d";
  }
  s = s ")";
  if (rtype != "void") {
    if (rtype ~ /[*]$/) s = s ": 0x%08X"; else s = s ": %d";
  }
  s = s "\"";
  for (i = 0; i < nargs; i++) {
    atype = $(5 + i*2);
    arg = $(5 + i*2 + 1);
    if (atype ~ /[*]$/) {
      s = s ", a" arg;
    } else {
      s = s ", " arg;
    }
  }
  if (rtype != "void") {
    if (rtype ~ /[*]$/) s = s ", ares"; else s = s ", res";
  }
  s = s ");";
  print s >> filename_case;

  print "      }" >> filename_case;
  print "      break;" >> filename_case;
  print "" >> filename_case;
}
'

exit 0
