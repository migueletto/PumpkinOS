#include <PalmOS.h>

#include "thread.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

/*
    UIObjectFrame
    UIObjectFill
    UIObjectForeground
    UIObjectSelectedFill
    UIObjectSelectedForeground

    UIMenuFrame
    UIMenuFill
    UIMenuForeground
    UIMenuSelectedFill
    UIMenuSelectedForeground

    UIFieldBackground
    UIFieldText
    UIFieldTextLines
    UIFieldCaret
    UIFieldTextHighlightBackground

    UIFieldTextHighlightForeground
    UIFieldFepRawText
    UIFieldFepRawBackground
    UIFieldFepConvertedText
    UIFieldFepConvertedBackground

    UIFieldFepUnderline
    UIFormFrame
    UIFormFill
    UIDialogFrame
    UIDialogFill

    UIAlertFrame
    UIAlertFill
    UIOK
    UICaution
    UIWarning

    UIFieldFepConvertedUnderline
*/

static const IndexedColorType table[UILastColorTableEntry] = {
  0xff, 0x00, 0xff, 0x59, 0x00,
  0xff, 0x00, 0xff, 0x59, 0x00, 
  0x00, 0xff, 0x19, 0xff, 0x78, 
  0xff, 0xff, 0x00, 0xff, 0x5a, 
  0x59, 0x59, 0x00, 0x59, 0x00, 
  0x59, 0x00, 0xd3, 0x79, 0x7d,
  0x00
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
    module->table[i] = table[i];
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

IndexedColorType UIColorGetTableEntryIndex(UIColorTableEntries which) {
  uic_module_t *module = (uic_module_t *)pumpkin_get_local_storage(uic_key);

  if (which >= UIObjectFrame && which < UILastColorTableEntry) {
    return module->table[which];
  }

  return 0;
}

void UIColorGetDefaultTableEntryRGB(UIColorTableEntries which, RGBColorType *rgbP) {
  uic_module_t *module = (uic_module_t *)pumpkin_get_local_storage(uic_key);
  RGBColorType *palette;

  if (which >= UIObjectFrame && which < UILastColorTableEntry && rgbP) {
    // always take color from 8bpp palette, regardless of current color depth
    palette = WinGetPalette(8);

    rgbP->r = palette[module->table[which]].r;
    rgbP->g = palette[module->table[which]].g;
    rgbP->b = palette[module->table[which]].b;
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
    debug(DEBUG_TRACE, "Window", "UIColorSetTableEntry entry %d RGB(%02X,%02X,%02X) -> %d", which, rgbP->r, rgbP->g, rgbP->b, module->table[which]);
  }

  return 0;
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
