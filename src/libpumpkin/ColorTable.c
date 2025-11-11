#include <PalmOS.h>

#include "ColorTable.h"
#include "bytes.h"
#include "debug.h"

UInt16 CtbGetNumEntries(ColorTableType *colorTableP) {
  UInt16 numEntries = 0;

  if (colorTableP) {
    get2b(&numEntries, (UInt8 *)colorTableP, 0);
  }

  return numEntries;
}

void CtbSetNumEntries(ColorTableType *colorTableP, UInt16 numEntries) {
  if (colorTableP) {
    put2b(numEntries, (UInt8 *)colorTableP, 0);
  }
}

void CtbGetEntry(ColorTableType *colorTableP, UInt8 index, RGBColorType *rgbP) {
  UInt16 numEntries;
  UInt8 *p;

  if (colorTableP && rgbP) {
    get2b(&numEntries, (UInt8 *)colorTableP, 0);
    if (index < numEntries) {
      p = (UInt8 *)colorTableP + 2 + index * 4;
      rgbP->index = index;
      rgbP->r = p[1];
      rgbP->g = p[2];
      rgbP->b = p[3];
    }
  }
}

void CtbSetEntry(ColorTableType *colorTableP, UInt8 index, RGBColorType *rgbP) {
  UInt16 numEntries;
  UInt8 *p;

  if (colorTableP && rgbP) {
    get2b(&numEntries, (UInt8 *)colorTableP, 0);
    if (index < numEntries) {
      p = (UInt8 *)colorTableP + 2 + index * 4;
      p[1] = rgbP->r;
      p[2] = rgbP->g;
      p[3] = rgbP->b;
    }
  }
}

Boolean CtbCompare(ColorTableType *colorTableP1, ColorTableType *colorTableP2) {
  UInt16 numEntries1, numEntries2;

  if (!colorTableP1 && !colorTableP2) return false;
  if (!colorTableP1 &&  colorTableP2) return false;
  if ( colorTableP1 && !colorTableP2) return false;

  get2b(&numEntries1, (UInt8 *)colorTableP1, 0);
  get2b(&numEntries2, (UInt8 *)colorTableP2, 0);

  if (numEntries1 != numEntries2) return false;

  return sys_memcmp(colorTableP1, colorTableP2, 2 + numEntries1 * 4) == 0;
}
