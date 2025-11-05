#!/bin/sh

if [ $# -ne 2 ]; then
  echo "usage: $0 kind file.struct"
  exit 1
fi

awk -v kind=$1 -v src=$2 '
BEGIN {
  sizes["UInt32"] = 4;
  sizes["UInt16"] = 2;
  sizes["UInt8"] = 1;
  sizes["Int32"] = 4;
  sizes["Int16"] = 2;
  sizes["Int8"] = 1;
  sizes["Boolean"] = 1;
  sizes["LocalID"] = 4;
  sizes["Coord"] = 2;
  sizes["CString"] = 0;
  put[4] = "put4b";
  put[2] = "put2b";
  put[1] = "put1";
  get[4] = "get4b";
  get[2] = "get2b";
  get[1] = "get1";
  readm[4] = "m68k_read_memory_32";
  readm[2] = "m68k_read_memory_16";
  readm[1] = "m68k_read_memory_8";
  writem[4] = "m68k_write_memory_32";
  writem[2] = "m68k_write_memory_16";
  writem[1] = "m68k_write_memory_8";
  put_type[4] = "uint32_t";
  put_type[2] = "uint16_t";
  put_type[1] = "uint8_t";
  num_names = 0;
  NOTICE = "/* This file was generated from \"" src "\". Do not edit. */";

  header = kind "_serde.h"
  source = kind "_serde.c"
  emuhdr = "emulation/emu_" kind "_serde.h"
  emusrc = "emulation/emu_" kind "_serde.c"

  print NOTICE > header;
  print "#ifndef " kind "_serde_h" > header;
  print "#define " kind "_serde_h" > header;

  print NOTICE > source;
  print "#include <PalmOS.h>" >> source;
  print "#include <VFSMgr.h>" >> source;
  print "#include <GPSLib.h>" >> source;
  print "" >> source;
  print "#include \"bytes.h\"" >> source;
  print "#include \"" kind "_serde.h\"" >> source;
  print "" >> source;

  print NOTICE > emuhdr;

  print NOTICE > emusrc;
  print "#include <PalmOS.h>" >> emusrc;
  print "#include <VFSMgr.h>" >> emusrc;
  print "#include <GPSLib.h>" >> emusrc;
  print "" >> emusrc;
  print "#ifdef ARMEMU" >> emusrc
  print "#include \"armemu.h\"" >> emusrc
  print "#endif" >> emusrc
  print "#include \"logtrap.h\"" >> emusrc;
  print "#include \"m68k/m68k.h\"" >> emusrc;
  print "#include \"m68k/m68kcpu.h\"" >> emusrc;
  print "#include \"emupalmosinc.h\"" >> emusrc;
  print "#include \"emupalmos.h\"" >> emusrc;
  print "#include \"" kind "_serde.h\"" >> emusrc;
  print "#include \"emu_" kind "_serde.h\"" >> emusrc;
  print "" >> emusrc;
}
$1 ~ /^#/ {
  next;
}
$1 == "name" {
  name = $2;
  names[num_names] = name;
  next;
}
$1 == "pointer" {
  pointer = $2;
  scalar = "";
  struct = "";
  structs[num_names] = pointer;
  next;
}
$1 == "scalar" {
  pointer = "";
  scalar = $2;
  struct = "";
  size = sizes[scalar];
  structs[num_names] = scalar;
  next;
}
$1 == "struct" {
  pointer = "";
  scalar = "";
  struct = $2;
  num_fields = 0;
  size = 0;
  extra_size = "";
  has_cstring = "";
  structs[num_names] = struct;
  next;
}
$1 == "field" {
  type = $2;
  field = $3;
  len = $4;
  field_types[num_fields] = type;
  fields[num_fields] = field;
  lengths[num_fields] = len;
  size += sizes[type];
  if (len) extra_size = extra_size " + " len;
  if (type == "CString") {
    extra_size = extra_size " + 2 + sys_strlen(param->" field ") + 2";
    has_cstring = 1;
  }
  num_fields++;
  next;
}
$1 == "end" {
  print "uint8_t *serialize_" name "(void *p, UInt32 *size);" >> header;
  print "uint8_t *serialize_" name "(void *p, UInt32 *size) {" >> source;
  if (pointer) {
    print "  uint8_t *buf = (uint8_t *)p;" >> source;
    print "  UInt32 i = 0;" >> source;
  } else if (scalar) {
    print "  " scalar " *param = p;" >> source;
    print "  uint8_t *buf = sys_calloc(1, *size + " size ");" >> source;
    print "  UInt32 i = *size;" >> source;
    print "  i += " put[size] "((" put_type[size] ")*param, buf, i);" >> source;
  } else {
    print "  " struct " *param = p;" >> source;
    print "  uint8_t *buf = sys_calloc(1, *size + " size extra_size ");" >> source;
    if (has_cstring) print "  UInt16 len;" >> source;
    print "  UInt32 i = *size;" >> source;
    for (i = 0; i < num_fields; i++) {
      if (field_types[i] == "CString") {
        print "  len = sys_strlen(param->" fields[i] ") + 1;" >> source;
        print "  i += " put[2] "(len, buf, i);" >> source;
        print "  sys_memcpy(buf + i, param->" fields[i] ", len);" >> source;
        print "  i += len;" >> source;
        print "  if (len % 2) i += put1(0, buf, i);" >> source;
      } else if (lengths[i]) {
        print "  sys_memcpy(buf + i, param->" fields[i] ", " lengths[i] ");" >> source;
        print "  i += " lengths[i] ";" >> source;
      } else {
        sz = sizes[field_types[i]];
        print "  i += " put[sz] "(param->" fields[i] ", buf, i);" >> source;
      }
    }
  }
  print "  *size = i;" >> source;
  print "  return buf;" >> source;
  print "}" >> source;
  print "" >> source;

  if (pointer) {
    print "int deserialize_" name "(uint8_t *buf, UInt32 size, " pointer " *param);" >> header;
    print "int deserialize_" name "(uint8_t *buf, UInt32 size, " pointer " *param) {" >> source;
    print "  *param = (" pointer ")((uintptr_t)buf);" >> source;
    print "  return 0;" >> source;
    print "}" >> source;
  } else if (scalar) {
    print "int deserialize_" name "(uint8_t *buf, UInt32 size, " scalar " *param);" >> header;
    print "int deserialize_" name "(uint8_t *buf, UInt32 size, " scalar " *param) {" >> source;
    print "  int r = -1;" >> source;
    print "  if (size == " size ") {" >> source;
    print "    " get[size] "((" put_type[size] " *)param, buf, 0);" >> source;
    print "    r = 0;" >> source;
    print "  }" >> source;
    print "  return r;" >> source;
    print "}" >> source;
    print "" >> source;
  } else {
    print "int deserialize_" name "(uint8_t *buf, UInt32 size, " struct " *param);" >> header;
    print "int deserialize_" name "(uint8_t *buf, UInt32 size, " struct " *param) {" >> source;
    print "  int r = -1;" >> source;
    if (has_cstring) {
      print "  if (size >= " size ") {" >> source;
    } else {
      print "  if (size >= " size extra_size ") {" >> source;
    }
    print "    UInt32 i = 0;" >> source;
    if (has_cstring) print "    UInt16 len;" >> source;
    for (i = 0; i < num_fields; i++) {
      if (field_types[i] == "CString") {
        print "    i += " get[2] "(&len, buf, i);" >> source;
        print "    param->" fields[i] " = (char *)(buf + i);" >> source;
        print "    if (len % 2) i++;" >> source;
        print "    i += len;" >> source;
      } else if (lengths[i]) {
        print "    sys_memcpy(param->" fields[i] ", buf + i, " lengths[i] ");" >> source;
        print "    i += " lengths[i] ";" >> source;
      } else {
        sz = sizes[field_types[i]];
        print "    i += " get[sz] "((" put_type[sz] " *)&param->" fields[i] ", buf, i);" >> source;
      }
    }
    print "    r = 0;" >> source;
    print "  }" >> source;
    print "  return r;" >> source;
    print "}" >> source;
    print "" >> source;
  }

  if (pointer) {
    print "void decode_" name "(UInt32 buf, " pointer " *param);" >> emuhdr;
    print "void decode_" name "(UInt32 buf, " pointer " *param) {" >> emusrc;
    print "  *param = (" pointer ")buf;" >> emusrc;
    print "}" >> emusrc;
    print "" >> emusrc;

    print "void encode_" name "(UInt32 buf, " pointer " *param);" >> emuhdr;
    print "void encode_" name "(UInt32 buf, " pointer " *param) {" >> emusrc;
    print "}" >> emusrc;
    print "" >> emusrc;

  } else if (scalar) {
    print "void decode_" name "(UInt32 buf, " scalar " *param);" >> emuhdr;
    print "void decode_" name "(UInt32 buf, " scalar " *param) {" >> emusrc;
    print "  *param = " readm[size] "(buf);" >> emusrc;
    print "}" >> emusrc;
    print "" >> emusrc;

    print "void encode_" name "(UInt32 buf, " scalar " *param);" >> emuhdr;
    print "void encode_" name "(UInt32 buf, " scalar " *param) {" >> emusrc;
    print "  " writem[size] "(buf, *param);" >> emusrc;
    print "}" >> emusrc;
    print "" >> emusrc;

  } else {
    print "void decode_" name "(UInt32 buf, " struct " *param);" >> emuhdr;
    print "void decode_" name "(UInt32 buf, " struct " *param) {" >> emusrc;
    print "  MemSet(param, sizeof(" struct "), 0);" >> emusrc;
    if (has_cstring) print "  UInt32 addr;" >> emusrc;
    print "  UInt32 offset = 0;" >> emusrc;
    for (i = 0; i < num_fields; i++) {
      if (field_types[i] == "CString") {
        print "  addr = " readm[4] "(buf + offset);" >> emusrc;
        print "  param->" fields[i] " = addr ? (char *)pumpkin_heap_base() + addr : NULL;" >> emusrc;
        print "  offset += 4;" >> emusrc;
      } else if (lengths[i]) {
        print "  for (UInt32 i = 0; i < " lengths[i] "; i++) {" >> emusrc;
        print "    param->" fields[i] "[i] = " readm[1] "(buf + offset + i);" >> emusrc;
        print "  }" >> emusrc;
        print "  offset += " lengths[i] ";" >> emusrc;
      } else {
        sz = sizes[field_types[i]];
        print "  param->" fields[i] " = " readm[sz] "(buf + offset);" >> emusrc;
        print "  offset += " sz ";" >> emusrc;
      }
    }
    print "}" >> emusrc;
    print "" >> emusrc;

    print "void encode_" name "(UInt32 buf, " struct " *param);" >> emuhdr;
    print "void encode_" name "(UInt32 buf, " struct " *param) {" >> emusrc;
    print "  UInt32 offset = 0;" >> emusrc;
    for (i = 0; i < num_fields; i++) {
      if (field_types[i] == "CString") {
        print "  " writem[4] "(buf + offset, param->" fields[i] " ? param->" fields[i] " - (char *)pumpkin_heap_base() : 0);" >> emusrc;
        print "  offset += 4;" >> emusrc;
      } else if (lengths[i]) {
        print "  for (UInt32 i = 0; i < " lengths[i] "; i++) {" >> emusrc;
        print "  " writem[1] "(buf + offset + i, param->" fields[i] "[i]);" >> emusrc;
        print "  }" >> emusrc;
        print "  offset += " lengths[i] ";" >> emusrc;
      } else {
        sz = sizes[field_types[i]];
        print "  " writem[sz] "(buf + offset, param->" fields[i] ");" >> emusrc;
        print "  offset += " sz ";" >> emusrc;
      }
    }
    print "}" >> emusrc;
    print "" >> emusrc;
  }

  num_names++;
  next;
}
END {
  print "" >> header;
  print "typedef union {" >> header;
  for (i = 0; i < num_names; i++) {
    print "  " structs[i] " p" i ";" >> header;
  }
  print "} " kind "_union_t;" >> header;
  print "" >> header;

  print "uint8_t *serialize_" kind "(UInt32 paramType, void *param, UInt32 *size);" >> header;
  print "uint8_t *serialize_" kind "(UInt32 paramType, void *param, UInt32 *size) {" >> source;
  print "  switch (paramType) {" >> source;
  for (i = 0; i < num_names; i++) {
    print "    case " names[i] ":" >> source;
    print "      return serialize_" names[i] "(param, size);" >> source;
  }
  print "    default:" >> source;
  print "      break;" >> source;
  print "  }" >> source;
  print "  return NULL;" >> source;
  print "}" >> source;
  print "" >> source;

  print "int deserialize_" kind "(UInt32 paramType, void *buf, UInt32 size, " kind "_union_t *param);" >> header;
  print "int deserialize_" kind "(UInt32 paramType, void *buf, UInt32 size, " kind "_union_t *param) {" >> source;
  print "  switch (paramType) {" >> source;
  for (i = 0; i < num_names; i++) {
    print "    case " names[i] ":" >> source;
    print "      return deserialize_" names[i] "(buf, size, &param->p" i ");" >> source;
  }
  print "    default:" >> source;
  print "      break;" >> source;
  print "  }" >> source;
  print "  return -1;" >> source;
  print "}" >> source;

  print "#endif" >> header;

  print "" >> emuhdr;
  print "int decode_" kind "(UInt32 paramType, UInt32 buf, " kind "_union_t *param);" >> emuhdr;
  print "int decode_" kind "(UInt32 paramType, UInt32 buf, " kind "_union_t *param) {" >> emusrc;
  print "  int r = 0;" >> emusrc;
  print "  switch (paramType) {" >> emusrc;
  for (i = 0; i < num_names; i++) {
    print "    case " names[i] ":" >> emusrc;
    print "      decode_" names[i] "(buf, &param->p" i ");" >> emusrc;
    print "      break;" >> emusrc;
  }
  print "    default:" >> emusrc;
  print "      r = -1;" >> emusrc;
  print "      break;" >> emusrc;
  print "  }" >> emusrc;
  print "  return r;" >> emusrc;
  print "}" >> emusrc;

  print "" >> emusrc;
  print "int encode_" kind "(UInt32 paramType, UInt32 buf, " kind "_union_t *param);" >> emuhdr;
  print "int encode_" kind "(UInt32 paramType, UInt32 buf, " kind "_union_t *param) {" >> emusrc;
  print "  int r = 0;" >> emusrc;
  print "  switch (paramType) {" >> emusrc;
  for (i = 0; i < num_names; i++) {
    print "    case " names[i] ":" >> emusrc;
    print "      encode_" names[i] "(buf, &param->p" i ");" >> emusrc;
    print "      break;" >> emusrc;
  }
  print "    default:" >> emusrc;
  print "      r = -1;" >> emusrc;
  print "      break;" >> emusrc;
  print "  }" >> emusrc;
  print "  return r;" >> emusrc;
  print "}" >> emusrc;

}
' $2

exit 0

