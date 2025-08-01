#include <PalmOS.h>
#include <CharAttr.h>

#include "pumpkin.h"

typedef struct {
  UInt8 *charCaselessValue;
  UInt8 *charSortValue;
} charattr_module_t;

static const UInt8 charCaselessValue[256] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x2a, 0x2b, 0x2c,
  0x2d, 0x2e, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12,
  0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x28, 0x2f, 0x30, 0x31,
  0x32, 0x33, 0x34, 0x23, 0x35, 0x36, 0x37, 0x5a, 0x38, 0x24, 0x39, 0x3a,
  0x75, 0x79, 0x7b, 0x7d, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x3b, 0x3c,
  0x5b, 0x5c, 0x5d, 0x3d, 0x3e, 0x85, 0x96, 0x98, 0x9c, 0xa0, 0xaa, 0xad,
  0xaf, 0xb1, 0xbb, 0xbd, 0xbf, 0xc1, 0xc3, 0xc7, 0xd8, 0xda, 0xdc, 0xde,
  0xe3, 0xe8, 0xf2, 0xf4, 0xf6, 0xf8, 0xfe, 0x3f, 0x40, 0x41, 0x42, 0x44,
  0x45, 0x85, 0x96, 0x98, 0x9c, 0xa0, 0xaa, 0xad, 0xaf, 0xb1, 0xbb, 0xbd,
  0xbf, 0xc1, 0xc3, 0xc7, 0xd8, 0xda, 0xdc, 0xde, 0xe3, 0xe8, 0xf2, 0xf4,
  0xf6, 0xf8, 0xfe, 0x46, 0x47, 0x48, 0x49, 0x1b, 0x67, 0x1c, 0x54, 0xaa,
  0x57, 0x73, 0x70, 0x71, 0x43, 0x74, 0xde, 0x58, 0xd6, 0x1d, 0x1e, 0x1f,
  0x20, 0x52, 0x53, 0x55, 0x56, 0x72, 0x26, 0x27, 0x51, 0xe7, 0xde, 0x59,
  0xd6, 0x21, 0x22, 0xf8, 0x29, 0x4a, 0x63, 0x64, 0x65, 0x66, 0x4b, 0x68,
  0x4c, 0x69, 0x85, 0x5f, 0x6a, 0x25, 0x6b, 0x4d, 0x6c, 0x5e, 0x7c, 0x7e,
  0x4e, 0x6d, 0x6e, 0x6f, 0x4f, 0x7a, 0xc7, 0x60, 0x76, 0x77, 0x78, 0x50,
  0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x94, 0x98, 0xa0, 0xa0, 0xa0, 0xa0,
  0xb1, 0xb1, 0xb1, 0xb1, 0x9e, 0xc3, 0xc7, 0xc7, 0xc7, 0xc7, 0xc7, 0x61,
  0xc7, 0xe8, 0xe8, 0xe8, 0xe8, 0xf8, 0xe5, 0xe2, 0x85, 0x85, 0x85, 0x85,
  0x85, 0x85, 0x94, 0x98, 0xa0, 0xa0, 0xa0, 0xa0, 0xb1, 0xb1, 0xb1, 0xb1,
  0x9e, 0xc3, 0xc7, 0xc7, 0xc7, 0xc7, 0xc7, 0x62, 0xc7, 0xe8, 0xe8, 0xe8,
  0xe8, 0xf8, 0xe5, 0xf8
};

int CharAttrInitModule(void) {
  charattr_module_t *module;
  Int16 i;

  if ((module = sys_calloc(1, sizeof(charattr_module_t))) == NULL) {
    return -1;
  }

  if ((module->charCaselessValue = MemPtrNew(256)) != NULL) {
    MemMove(module->charCaselessValue, charCaselessValue, 256);
  }

  if ((module->charSortValue = MemPtrNew(256)) != NULL) {
    for (i = 0; i < 256; i++) {
      module->charSortValue[i] = i;
    }
  }

  pumpkin_set_local_storage(charattr_key, module);

  return 0;
}

int CharAttrFinishModule(void) {
  charattr_module_t *module = (charattr_module_t *)pumpkin_get_local_storage(charattr_key);

  if (module) {
    if (module->charCaselessValue) MemPtrFree(module->charCaselessValue);
    if (module->charSortValue) MemPtrFree(module->charSortValue);
    sys_free(module);
  }

  return 0;
}

const UInt8 *GetCharCaselessValue(void) {
  charattr_module_t *module = (charattr_module_t *)pumpkin_get_local_storage(charattr_key);

  return module->charCaselessValue;
}

const UInt8 *GetCharSortValue(void) {
  charattr_module_t *module = (charattr_module_t *)pumpkin_get_local_storage(charattr_key);

  return module->charSortValue;
}
