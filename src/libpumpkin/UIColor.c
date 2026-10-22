#include <PalmOS.h>

#include "thread.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

/*
     0 UIObjectFrame
     1 UIObjectFill
     2 UIObjectForeground
     3 UIObjectSelectedFill
     4 UIObjectSelectedForeground

     5 UIMenuFrame
     6 UIMenuFill
     7 UIMenuForeground
     8 UIMenuSelectedFill
     9 UIMenuSelectedForeground

    10 UIFieldBackground
    11 UIFieldText
    12 UIFieldTextLines
    13 UIFieldCaret
    14 UIFieldTextHighlightBackground

    15 UIFieldTextHighlightForeground
    16 UIFieldFepRawText
    17 UIFieldFepRawBackground
    18 UIFieldFepConvertedText
    19 UIFieldFepConvertedBackground

    20 UIFieldFepUnderline
    21 UIFormFrame
    22 UIFormFill
    23 UIDialogFrame
    24 UIDialogFill

    25 UIAlertFrame
    26 UIAlertFill
    27 UIOK
    28 UICaution
    29 UIWarning

    30 UIFieldFepConvertedUnderline
*/

static const IndexedColorType table1[UILastColorTableEntry] = {
  0x01, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01,
  0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00
};

static const IndexedColorType table2[UILastColorTableEntry] = {
  0x03, 0x00, 0x03, 0x03, 0x00, 0x03, 0x00, 0x03,
  0x03, 0x00, 0x00, 0x03, 0x01, 0x03, 0x01, 0x03,
  0x03, 0x00, 0x03, 0x01, 0x03, 0x03, 0x00, 0x03,
  0x00, 0x03, 0x00, 0x03, 0x02, 0x02, 0x00
};

static const IndexedColorType table4[UILastColorTableEntry] = {
  0x0F, 0x00, 0x0F, 0x0B, 0x00, 0x0F, 0x00, 0x0F,
  0x0B, 0x00, 0x00, 0x0F, 0x03, 0x0F, 0x05, 0x0F,
  0x0F, 0x00, 0x0F, 0x05, 0x0B, 0x0B, 0x00, 0x0B,
  0x00, 0x0B, 0x00, 0x0B, 0x06, 0x0A, 0x00
};

static const IndexedColorType table8[UILastColorTableEntry] = {
  0xff, 0x00, 0xff, 0x59, 0x00, 0xff, 0x00, 0xff,
  0x59, 0x00, 0x00, 0xff, 0x19, 0xff, 0x78, 0xff,
  0xff, 0x00, 0xff, 0x5a, 0x59, 0x59, 0x00, 0x59,
  0x00, 0x59, 0x00, 0xd3, 0x79, 0x7d, 0x00
};

typedef struct {
  IndexedColorType table[UILastColorTableEntry];
  IndexedColorType old[UILastColorTableEntry];
} uic_module_t;

int UicInitModule(void) {
  uic_module_t *module;
  int i;

  if ((module = xcalloc(1, sizeof(uic_module_t))) == NULL) {
    return -1;
  }

  for (i = 0; i < UILastColorTableEntry; i++) {
    module->table[i] = table8[i];
  }

  pumpkin_set_local_storage(uic_key, module);

  return 0;
}

int UicFinishModule(void) {
  uic_module_t *module = (uic_module_t *)pumpkin_get_local_storage(uic_key);

  if (module) {
    xfree(module);
  }

  return 0;
}

void UIColorResetTable(void) {
  uic_module_t *module = (uic_module_t *)pumpkin_get_local_storage(uic_key);
  int i;

  for (i = 0; i < UILastColorTableEntry; i++) {
    module->table[i] = table8[i];
  }
}

IndexedColorType UIColorGetTableEntryIndex(UIColorTableEntries which) {
  uic_module_t *module = (uic_module_t *)pumpkin_get_local_storage(uic_key);

  if (which >= UIObjectFrame && which < UILastColorTableEntry) {
    return module->table[which];
  }

  return 0;
}

void UIColorGetDefaultTableEntryRGB(UInt16 depth, UIColorTableEntries which, RGBColorType *rgbP) {
  RGBColorType *palette;
  UInt8 index;

  if (which >= UIObjectFrame && which < UILastColorTableEntry && rgbP) {
    palette = WinGetPalette(depth);
    switch (depth) {
      case 1:  index = table1[which]; break;
      case 2:  index = table2[which]; break;
      case 4:  index = table4[which]; break;
      case 8:  index = table8[which]; break;
      default: index = table8[which]; break;
    }
    rgbP->r = palette[index].r;
    rgbP->g = palette[index].g;
    rgbP->b = palette[index].b;
  }
}

void UIColorGetTableEntryRGB(UIColorTableEntries which, RGBColorType *rgbP) {
  uic_module_t *module = (uic_module_t *)pumpkin_get_local_storage(uic_key);

  if (which >= UIObjectFrame && which < UILastColorTableEntry && rgbP) {
    WinIndexToRGB(module->table[which], rgbP);
  }
}

// Sets the value of a UI color entry to the passed RGB value. Updates
// the index for that UI color entry to the current best fit for that RGB
// value according to the palette used by the current draw window.

Err UIColorSetTableEntry(UIColorTableEntries which, const RGBColorType *rgbP) {
  uic_module_t *module = (uic_module_t *)pumpkin_get_local_storage(uic_key);

  if (which >= UIObjectFrame && which < UILastColorTableEntry && rgbP) {
    module->table[which] = WinRGBToIndex(rgbP);
    debug(DEBUG_TRACE, "Window", "UIColorSetTableEntry entry %2d RGB(%02X,%02X,%02X) -> %d", which, rgbP->r, rgbP->g, rgbP->b, module->table[which]);
  }

  return 0;
}

void *UIColorSaveTable(void) {
  uic_module_t *module = (uic_module_t *)pumpkin_get_local_storage(uic_key);
  IndexedColorType *old;
  Int16 i;

  old = MemPtrNew(UILastColorTableEntry * sizeof(IndexedColorType));
  for (i = 0; i < UILastColorTableEntry; i++) {
    old[i] = module->table[i];
  }

  return old;
}

void UIColorRestoreTable(void *p) {
  uic_module_t *module = (uic_module_t *)pumpkin_get_local_storage(uic_key);
  IndexedColorType *old = (IndexedColorType *)p;
  Int16 i;

  if (old) {
    for (i = 0; i < UILastColorTableEntry; i++) {
      module->table[i] = old[i];
    }
  }
}

Err UIColorPushTable(void) {
  uic_module_t *module = (uic_module_t *)pumpkin_get_local_storage(uic_key);
  Int16 i;

  for (i = 0; i < UILastColorTableEntry; i++) {
    module->old[i] = module->table[i];
  }

  return 0;
}

Err UIColorPopTable(void) {
  uic_module_t *module = (uic_module_t *)pumpkin_get_local_storage(uic_key);
  Int16 i;

  for (i = 0; i < UILastColorTableEntry; i++) {
    module->table[i] = module->old[i];
  }

  return 0;
}
