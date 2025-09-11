#include "sys.h"
#include "ht.h"
#include "type.h"

static char *tname[] = {
  "bool",
  "int8_t",
  "uint8_t",
  "int16_t",
  "uint16_t",
  "int32_t",
  "uint32_t",
  "float",
  "string"
};

uint32_t type_size(type_t type) {
  uint32_t size = 0;

  switch (type) {
    case TYPE_BOOL:   size = sizeof(uint8_t); break;
    case TYPE_INT8:   size = sizeof(int8_t); break;
    case TYPE_UINT8:  size = sizeof(uint8_t); break;
    case TYPE_INT16:  size = sizeof(int16_t); break;
    case TYPE_UINT16: size = sizeof(uint16_t); break;
    case TYPE_INT32:  size = sizeof(int32_t); break;
    case TYPE_UINT32: size = sizeof(uint32_t); break;
    case TYPE_FLOAT:  size = sizeof(float); break;
    case TYPE_STRING: size = sizeof(uint32_t); break;
    default: break;
  }

  return size;
}

uint32_t type_align(uint32_t size, uint32_t offset) {
  uint32_t rem = offset % size;
  if (rem) offset += size - rem;
  return offset;
}

char *type_name(type_t type) {
  if (type >= TYPE_BOOL && type <= TYPE_STRING) {
    return tname[type];
  }
  if (type >= TYPE_STRUCT) {
    return "struct";
  }
  return "?";
}

int type_signed(type_t t) {
  switch (t) {
    case TYPE_INT8:
    case TYPE_INT16:
    case TYPE_INT32:
    case TYPE_BOOL:
      return 1;
    default:
      break;
  }

  return 0;
}

int type_unsigned(type_t t) {
  switch (t) {
    case TYPE_UINT8:
    case TYPE_UINT16:
    case TYPE_UINT32:
    case TYPE_BOOL:
      return 1;
    default:
      break;
  }

  return 0;
}

int type_integer(type_t t) {
  switch (t) {
    case TYPE_INT8:
    case TYPE_UINT8:
    case TYPE_INT16:
    case TYPE_UINT16:
    case TYPE_INT32:
    case TYPE_UINT32:
    case TYPE_BOOL:
      return 1;
    default:
      break;
  }

  return 0;
}

int type_numeric(type_t t) {
  return t == TYPE_FLOAT || type_integer(t);
}

int type_contains(type_t t1, type_t t2) {
  switch (t1) {
    case TYPE_STRING: return t2 == TYPE_STRING;
    case TYPE_FLOAT:  return t2 == TYPE_FLOAT || type_integer(t2);
    case TYPE_BOOL:   return t2 == TYPE_BOOL;
    case TYPE_INT32:  return type_signed(t2);
    case TYPE_UINT32: return type_unsigned(t2);
    case TYPE_INT16:  return t2 == TYPE_INT16  || t2 == TYPE_INT8 || t2 == TYPE_BOOL;
    case TYPE_UINT16: return t2 == TYPE_UINT16 || t2 == TYPE_UINT8 || t2 == TYPE_BOOL;
    case TYPE_INT8:   return t2 == TYPE_INT8 || t2 == TYPE_BOOL;
    case TYPE_UINT8:  return t2 == TYPE_UINT8 || t2 == TYPE_BOOL;
    default: break;
  }

  return 0;
}

void string_value(value_t *value, char *buf, int len) {
  switch (value->type) {
    case TYPE_STRING: sys_strncpy(buf, value->value.s, len-1); break;
    case TYPE_FLOAT:  sys_snprintf(buf, len-1, "%.8f",  value->value.d);   break;
    case TYPE_INT32:  sys_snprintf(buf, len-1, "%d",    value->value.i32); break;
    case TYPE_INT16:  sys_snprintf(buf, len-1, "%d",    value->value.i16); break;
    case TYPE_INT8:   sys_snprintf(buf, len-1, "%d",    value->value.i8);  break;
    case TYPE_UINT32: sys_snprintf(buf, len-1, "%u",    value->value.u32); break;
    case TYPE_UINT16: sys_snprintf(buf, len-1, "%u",    value->value.u16); break;
    case TYPE_UINT8:  sys_snprintf(buf, len-1, "%u",    value->value.u8);  break;
    case TYPE_BOOL :  sys_snprintf(buf, len-1, "%u",    value->value.b);   break;
    default: break;
  }
}

int type_coerce(value_t *value, type_t t, value_t *arg, int cast) {
  value->type = t;
  char buf[64];
  int r = 0;

  switch (t) {
    case TYPE_STRING:
      if (arg->type == TYPE_STRING) {
        value->value.s = arg->value.s;
      } else if (cast) {
        string_value(arg, buf, sizeof(buf));
        value->value.s = sys_strdup(buf);
      } else {
        r = -1;
      }
      break;
    case TYPE_FLOAT:
      switch (arg->type) {
        case TYPE_FLOAT:  value->value.d = arg->value.d; break;
        case TYPE_INT32:  value->value.d = arg->value.i32; break;
        case TYPE_INT16:  value->value.d = arg->value.i16; break;
        case TYPE_INT8:   value->value.d = arg->value.i8; break;
        case TYPE_UINT32: value->value.d = arg->value.u32; break;
        case TYPE_UINT16: value->value.d = arg->value.u16; break;
        case TYPE_UINT8:  value->value.d = arg->value.u8; break;
        case TYPE_BOOL:   value->value.d = arg->value.b; break;
        case TYPE_STRING: if (cast) value->value.d = sys_atof(arg->value.s); break;
        default: r = -1; break;
      }
      break;
    case TYPE_BOOL:
      switch (arg->type) {
        case TYPE_BOOL:   value->value.b = arg->value.b; break;
        default: r = -1; break;
      }
      break;
    case TYPE_INT32:
      switch (arg->type) {
        case TYPE_INT32:  value->value.i32 = arg->value.i32; break;
        case TYPE_INT16:  value->value.i32 = arg->value.i16; break;
        case TYPE_INT8:   value->value.i32 = arg->value.i8; break;
        case TYPE_BOOL:   value->value.i32 = arg->value.b; break;
        case TYPE_UINT32: if (cast) value->value.i32 = arg->value.u32; break;
        case TYPE_UINT16: if (cast) value->value.i32 = arg->value.u16; break;
        case TYPE_UINT8:  if (cast) value->value.i32 = arg->value.u8; break;
        case TYPE_FLOAT:  if (cast) value->value.i32 = arg->value.d; break;
        default: r = -1; break;
      }
      break;
    case TYPE_UINT32:
      switch (arg->type) {
        case TYPE_UINT32: value->value.u32 = arg->value.u32; break;
        case TYPE_UINT16: value->value.u32 = arg->value.u16; break;
        case TYPE_UINT8:  value->value.u32 = arg->value.u8; break;
        case TYPE_BOOL:   value->value.u32 = arg->value.b; break;
        case TYPE_INT32:  if (cast) value->value.u32 = arg->value.i32; break;
        case TYPE_INT16:  if (cast) value->value.u32 = arg->value.i16; break;
        case TYPE_INT8:   if (cast) value->value.u32 = arg->value.i8; break;
        case TYPE_FLOAT:  if (cast) value->value.u32 = arg->value.d; break;
        default: r = -1; break;
      }
      break;
    case TYPE_INT16:
      switch (arg->type) {
        case TYPE_INT16:  value->value.i16 = arg->value.i16; break;
        case TYPE_INT8:   value->value.i16 = arg->value.i8; break;
        case TYPE_BOOL:   value->value.i16 = arg->value.b; break;
        case TYPE_INT32:  if (cast) value->value.i16 = arg->value.i32; break;
        case TYPE_UINT32: if (cast) value->value.i16 = arg->value.u32; break;
        case TYPE_UINT16: if (cast) value->value.i16 = arg->value.u16; break;
        case TYPE_UINT8:  if (cast) value->value.i16 = arg->value.u8; break;
        case TYPE_FLOAT:  if (cast) value->value.i16 = arg->value.d; break;
        default: r = -1; break;
      }
      break;
    case TYPE_UINT16:
      switch (arg->type) {
        case TYPE_UINT16: value->value.u16 = arg->value.u16; break;
        case TYPE_UINT8:  value->value.u16 = arg->value.u8; break;
        case TYPE_BOOL:   value->value.u16 = arg->value.b; break;
        case TYPE_UINT32: if (cast) value->value.u16 = arg->value.u32; break;
        case TYPE_INT32:  if (cast) value->value.u16 = arg->value.i32; break;
        case TYPE_INT16:  if (cast) value->value.u16 = arg->value.i16; break;
        case TYPE_INT8:   if (cast) value->value.u16 = arg->value.i8; break;
        case TYPE_FLOAT:  if (cast) value->value.u16 = arg->value.d; break;
        default: r = -1; break;
      }
      break;
    case TYPE_INT8:
      switch (arg->type) {
        case TYPE_INT8:   value->value.i8 = arg->value.i8; break;
        case TYPE_BOOL:   value->value.i8 = arg->value.b; break;
        case TYPE_INT32:  if (cast) value->value.i8 = arg->value.i32; break;
        case TYPE_INT16:  if (cast) value->value.i8 = arg->value.i16; break;
        case TYPE_UINT32: if (cast) value->value.i8 = arg->value.u32; break;
        case TYPE_UINT16: if (cast) value->value.i8 = arg->value.u16; break;
        case TYPE_UINT8:  if (cast) value->value.i8 = arg->value.u8; break;
        case TYPE_FLOAT:  if (cast) value->value.i8 = arg->value.d; break;
        default: r = -1; break;
      }
      break;
    case TYPE_UINT8:
      switch (arg->type) {
        case TYPE_UINT8:  value->value.u8 = arg->value.u8; break;
        case TYPE_BOOL:   value->value.u8 = arg->value.b; break;
        case TYPE_UINT32: if (cast) value->value.u8 = arg->value.u32; break;
        case TYPE_UINT16: if (cast) value->value.u8 = arg->value.u16; break;
        case TYPE_INT32:  if (cast) value->value.u8 = arg->value.i32; break;
        case TYPE_INT16:  if (cast) value->value.u8 = arg->value.i16; break;
        case TYPE_INT8:   if (cast) value->value.u8 = arg->value.i8; break;
        case TYPE_FLOAT:  if (cast) value->value.u8 = arg->value.d; break;
        default: r = -1; break;
      }
      break;
    default:
      break;
  }

  return r;
}
