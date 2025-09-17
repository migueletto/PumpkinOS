#include <PalmOS.h>
#include <VFSMgr.h>

#include "ColorTable.h"

#define PALMOS_MODULE "Bitmap"

#include "sys.h"
#include "thread.h"
#include "mutex.h"
#include "pwindow.h"
#include "rgb.h"
#include "vfs.h"
#include "bytes.h"
#include "AppRegistry.h"
#include "storage.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"
#include "emupalmosinc.h"

typedef enum {
  BitmapFlagCompressed,
  BitmapFlagHasColorTable,
  BitmapFlagHasTransparency,
  BitmapFlagIndirect,
  BitmapFlagForScreen,
  BitmapFlagDirectColor,
  BitmapFlagIndirectColorTable,
  BitmapFlagNoDither,
  BitmapFlagLittleEndian
} BitmapFlagSelector;

#define BitmapFlagAll 0xFFFF

typedef enum {
  BitmapFieldWidth = 0,
  BitmapFieldHeight = 2,
  BitmapFieldRowBytes = 4,
  BitmapFieldFlags = 6,
  BitmapFieldPixelSize = 8,
  BitmapFieldVersion = 9
} BitmapSelector;

typedef enum {
  BitmapV0FieldWidth = BitmapFieldWidth,
  BitmapV0FieldHeight = BitmapFieldHeight,
  BitmapV0FieldRowBytes = BitmapFieldRowBytes,
  BitmapV0FieldFlags = BitmapFieldFlags,
  BitmapV0FieldReserved1 = 8,
  BitmapV0FieldReserved2 = 10,
  BitmapV0FieldReserved3 = 12,
  BitmapV0FieldReserved4 = 14,
  BitmapV0HeaderSize = 16
} BitmapV0Selector;

typedef enum {
  BitmapV1FieldWidth = BitmapFieldWidth,
  BitmapV1FieldHeight = BitmapFieldHeight,
  BitmapV1FieldRowBytes = BitmapFieldRowBytes,
  BitmapV1FieldFlags = BitmapFieldFlags,
  BitmapV1FieldPixelSize = BitmapFieldPixelSize,
  BitmapV1FieldVersion = BitmapFieldVersion,
  BitmapV1FieldNextDepthOffset = 10,
  BitmapV1FieldReserved = 12,
  BitmapV1HeaderSize = 16
} BitmapV1Selector;

#define BitmapV1FieldColorTable BitmapV1HeaderSize

typedef enum {
  BitmapV2FieldWidth = BitmapFieldWidth,
  BitmapV2FieldHeight = BitmapFieldHeight,
  BitmapV2FieldRowBytes = BitmapFieldRowBytes,
  BitmapV2FieldFlags = BitmapFieldFlags,
  BitmapV2FieldPixelSize = BitmapFieldPixelSize,
  BitmapV2FieldVersion = BitmapFieldVersion,
  BitmapV2FieldNextDepthOffset = 10,
  BitmapV2FieldTransparentIndex = 12,
  BitmapV2FieldCompressionType = 13,
  BitmapV2FieldReserved = 14,
  BitmapV2HeaderSize = 16
} BitmapV2Selector;

#define BitmapV2FieldColorTable BitmapV2HeaderSize

typedef enum {
  BitmapV3FieldWidth = BitmapFieldWidth,
  BitmapV3FieldHeight = BitmapFieldHeight,
  BitmapV3FieldRowBytes = BitmapFieldRowBytes,
  BitmapV3FieldFlags = BitmapFieldFlags,
  BitmapV3FieldPixelSize = BitmapFieldPixelSize,
  BitmapV3FieldVersion = BitmapFieldVersion,
  BitmapV3FieldSize = 10,
  BitmapV3FieldPixelFormat = 11,
  BitmapV3FieldUnused = 12,
  BitmapV3FieldCompressionType = 13,
  BitmapV3FieldDensity = 14,
  BitmapV3FieldTransparentValue = 16,
  BitmapV3FieldNextBitmapOffset = 20,
  BitmapV3HeaderSize = 24
} BitmapV3Selector;

#define BitmapV3FieldColorTable BitmapV3HeaderSize

#define BmpGetCommonField(bmp, selector) BmpGetSetCommonField(bmp, selector, 0, 0, false)
#define BmpSetCommonField(bmp, selector, value) BmpGetSetCommonField(bmp, selector, 0, value, true)

#define BmpGetCommonFlag(bmp, flagSelector) BmpGetSetCommonField(bmp, BitmapFieldFlags, flagSelector, 0, false)
#define BmpSetCommonFlag(bmp, flagSelector, value) BmpGetSetCommonField(bmp, BitmapFieldFlags, flagSelector, value, true)

#define BmpV0GetField(bmp, selector) BmpV0GetSetField(bmp, selector, 0, 0, false)
#define BmpV0SetField(bmp, selector, value) BmpV0GetSetField(bmp, selector, 0, value, true)

#define BmpV1GetField(bmp, selector) BmpV1GetSetField(bmp, selector, 0, 0, false)
#define BmpV1SetField(bmp, selector, value) BmpV1GetSetField(bmp, selector, 0, value, true)

#define BmpV2GetField(bmp, selector) BmpV2GetSetField(bmp, selector, 0, 0, false)
#define BmpV2SetField(bmp, selector, value) BmpV2GetSetField(bmp, selector, 0, value, true)

#define BmpV3GetField(bmp, selector) BmpV3GetSetField(bmp, selector, 0, 0, false)
#define BmpV3SetField(bmp, selector, value) BmpV3GetSetField(bmp, selector, 0, value, true)

typedef struct {
  UInt16 encoding;
  MemHandle h;
  BitmapType *bitmapP;
} bmp_surface_t;

static UInt8 BmpRGBToIndex(UInt8 red, UInt8 green, UInt8 blue, ColorTableType *colorTable);
static void BmpIndexToRGB(UInt8 i, UInt8 *red, UInt8 *green, UInt8 *blue, ColorTableType *colorTable);
static UInt8 rgbToGray1(UInt16 r, UInt16 g, UInt16 b);
static UInt8 rgbToGray2(UInt16 r, UInt16 g, UInt16 b);
static UInt8 rgbToGray4(UInt16 r, UInt16 g, UInt16 b);

static const UInt8 gray1[2]       = {0x00, 0xe6};
static const UInt8 gray1values[2] = {0xff, 0x00};

                                    /* white light-gray dark-gray black */
static const UInt8 gray2[4]       = {0x00, 0xdd, 0xda, 0xe6};
static const UInt8 gray2values[4] = {0xff, 0xaa, 0x55, 0x00};

static const UInt8 gray4[16]       = {0x00, 0xe0, 0xdf, 0x19, 0xde, 0xdd, 0x32, 0xdc, 0xdb, 0xa5, 0xda, 0xd9, 0xbe, 0xd8, 0xd7, 0xe6};
static const UInt8 gray4values[16] = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};

static UInt8 emptySlot[BitmapV1HeaderSize] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static Boolean isEmptySlot(BitmapType *bitmapP) {
  UInt8 *bmp = (UInt8 *)bitmapP;
  UInt32 i;

  for (i = 0; i < BitmapV1HeaderSize; i++) {
    if (bmp[i] != emptySlot[i]) return false;
  }

  return true;
}

static BitmapType *skipEmptySlot(BitmapType *bitmapP) {
  return isEmptySlot(bitmapP) ? (BitmapType *)((UInt8 *)bitmapP + BitmapV1HeaderSize) : bitmapP;
}

UInt8 BmpGetVersion(const BitmapType *bitmapP) {
  UInt8 version = 0;
  BitmapType *bmp;

  if (bitmapP) {
    bmp = skipEmptySlot((BitmapType *)bitmapP);
    get1(&version, (UInt8 *)bmp, BitmapFieldVersion);
    version &= 0x7F;
  }

  return version;
}

Boolean BmpLittleEndian(const BitmapType *bitmapP) {
  UInt8 version;
  BitmapType *bmp;
  Boolean le = false;

  if (bitmapP) {
    bmp = skipEmptySlot((BitmapType *)bitmapP);
    get1(&version, (UInt8 *)bmp, BitmapFieldVersion);
    le = (version & 0x80) == 0x80;
  }

  return le;
}

#define get2(a,p,i) le ? get2l(a, p, i) : get2b(a, p, i)
#define get4(a,p,i) le ? get4l(a, p, i) : get4b(a, p, i)
#define put2(a,p,i) { if (le) put2l(a, p, i); else put2b(a, p, i); }
#define put4(a,p,i) { if (le) put4l(a, p, i); else put4b(a, p, i); }

#define get2_16(a,p,i) (le || leBits) ? get2l(a, p, i) : get2b(a, p, i)
#define put2_16(a,p,i) { if (le || leBits) put2l(a, p, i); else put2b(a, p, i); }

UInt32 BmpGetSetCommonField(BitmapType *bmp, BitmapSelector selector, BitmapFlagSelector flagSelector, UInt32 value, Boolean set) {
  UInt8 v8, version;
  UInt16 v16;
  Boolean le;

  if (bmp) {
    bmp = skipEmptySlot(bmp);
    version = BmpGetVersion(bmp);
    le = BmpLittleEndian(bmp);

    switch (version) {
      case 0:
      case 1:
      case 2:
      case 3:
          switch (selector) {
            case BitmapFieldWidth:
            case BitmapFieldHeight:
            case BitmapFieldRowBytes:
              if (set) {
                put2(value, (UInt8 *)bmp, selector);
              } else {
                get2(&v16, (UInt8 *)bmp, selector);
                value = v16;
              }
              break;
            case BitmapFieldPixelSize:
              if (set) {
                put1(value, (UInt8 *)bmp, selector);
              } else {
                if (version == 0) {
                  value = 1;
                } else {
                  get1(&v8, (UInt8 *)bmp, selector);
                  value = v8;
                }
              }
              break;
            case BitmapFieldVersion:
              if (set) {
                put1(value, (UInt8 *)bmp, selector);
              } else {
                get1(&v8, (UInt8 *)bmp, selector);
                value = v8;
              }
              break;
            case BitmapFieldFlags:
              get2(&v16, (UInt8 *)bmp, selector);
              if (set) {
                if (flagSelector != BitmapFlagAll) switch (flagSelector) {
                  case BitmapFlagCompressed:         v16 &= 0x7FFF; v16 |= value ? 0x8000 : 0x0000; value = v16; break;
                  case BitmapFlagHasColorTable:      v16 &= 0xBFFF; v16 |= value ? 0x4000 : 0x0000; value = v16; break;
                  case BitmapFlagHasTransparency:    v16 &= 0xDFFF; v16 |= value ? 0x2000 : 0x0000; value = v16; break;
                  case BitmapFlagIndirect:           v16 &= 0xEFFF; v16 |= value ? 0x1000 : 0x0000; value = v16; break;
                  case BitmapFlagForScreen:          v16 &= 0xF7FF; v16 |= value ? 0x0800 : 0x0000; value = v16; break;
                  case BitmapFlagDirectColor:        v16 &= 0xFBFF; v16 |= value ? 0x0400 : 0x0000; value = v16; break;
                  case BitmapFlagIndirectColorTable: v16 &= 0xFDFF; v16 |= value ? 0x0200 : 0x0000; value = v16; break;
                  case BitmapFlagNoDither:           v16 &= 0xFEFF; v16 |= value ? 0x0100 : 0x0000; value = v16; break;
                  case BitmapFlagLittleEndian:       v16 &= 0xFF7F; v16 |= value ? 0x0080 : 0x0000; value = v16; break;
                }
                put2(value, (UInt8 *)bmp, selector);
              } else {
                if (flagSelector != BitmapFlagAll) switch (flagSelector) {
                  case BitmapFlagCompressed:         value = (v16 & 0x8000) ? 1 : 0; break;
                  case BitmapFlagHasColorTable:      value = (v16 & 0x4000) ? 1 : 0; break;
                  case BitmapFlagHasTransparency:    value = (v16 & 0x2000) ? 1 : 0; break;
                  case BitmapFlagIndirect:           value = (v16 & 0x1000) ? 1 : 0; break;
                  case BitmapFlagForScreen:          value = (v16 & 0x0800) ? 1 : 0; break;
                  case BitmapFlagDirectColor:        value = (v16 & 0x0400) ? 1 : 0; break;
                  case BitmapFlagIndirectColorTable: value = (v16 & 0x0200) ? 1 : 0; break;
                  case BitmapFlagNoDither:           value = (v16 & 0x0100) ? 1 : 0; break;
                  case BitmapFlagLittleEndian:       value = (v16 & 0x0080) ? 1 : 0; break;
                } else {
                  value = v16;
                }
              }
              break;
            default:
              debug(DEBUG_ERROR, "Bitmap", "invalid bitmap selector %u", selector);
              break;
          }
        break;
      default:
        debug(DEBUG_ERROR, "Bitmap", "invalid bitmap version %u", version);
        break;
    }
  }

  return value;
}

Boolean BmpGetLittleEndianBits(const BitmapType *bitmapP) {
  return BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagLittleEndian);
}

void BmpSetLittleEndianBits(const BitmapType *bitmapP, Boolean le) {
  BmpSetCommonFlag((BitmapType *)bitmapP, BitmapFlagLittleEndian, le);
}

UInt32 BmpV0GetSetField(BitmapType *bmp, BitmapSelector selector, BitmapFlagSelector flagSelector, UInt32 value, Boolean set) {
  UInt8 version;

  if (bmp) {
    bmp = skipEmptySlot(bmp);
    version = BmpGetVersion(bmp);

    if (version == 0) {
      switch (selector) {
        case BitmapFieldWidth:
        case BitmapFieldHeight:
        case BitmapFieldRowBytes:
        case BitmapFieldFlags:
        case BitmapFieldPixelSize:
        case BitmapFieldVersion:
          value = BmpGetSetCommonField(bmp, selector, flagSelector, value, set);
          break;
        default:
          debug(DEBUG_ERROR, "Bitmap", "invalid bitmap selector %u", selector);
          value = 0;
          break;
      }
    } else {
      debug(DEBUG_ERROR, "Bitmap", "bitmap %p is not V0 (%u)", bmp, version);
      value = 0;
    }
  } else {
    debug(DEBUG_ERROR, "Bitmap", "null bitmap");
    value = 0;
  }

  return value;
}

UInt32 BmpV1GetSetField(BitmapType *bmp, BitmapV1Selector selector, BitmapFlagSelector flagSelector, UInt32 value, Boolean set) {
  UInt8 version;
  UInt16 v16;
  Boolean le;

  if (bmp) {
    bmp = skipEmptySlot(bmp);
    version = BmpGetVersion(bmp);
    le = BmpLittleEndian(bmp);

    if (version == 1) {
      switch (selector) {
        case BitmapV1FieldWidth:
        case BitmapV1FieldHeight:
        case BitmapV1FieldRowBytes:
        case BitmapV1FieldFlags:
        case BitmapV1FieldPixelSize:
        case BitmapV1FieldVersion:
          value = BmpGetSetCommonField(bmp, (BitmapSelector)selector, flagSelector, value, set);
          break;
        case BitmapV1FieldNextDepthOffset:
          if (set) {
            put2(value, (UInt8 *)bmp, selector);
          } else {
            get2(&v16, (UInt8 *)bmp, selector);
            value = v16;
          }
          break;
        default:
          debug(DEBUG_ERROR, "Bitmap", "invalid bitmap selector %u", selector);
          value = 0;
          break;
      }
    } else {
      debug(DEBUG_ERROR, "Bitmap", "bitmap %p is not V1 (%u)", bmp, version);
      value = 0;
    }
  } else {
    debug(DEBUG_ERROR, "Bitmap", "null bitmap");
    value = 0;
  }

  return value;
}

UInt32 BmpV2GetSetField(BitmapType *bmp, BitmapV2Selector selector, BitmapFlagSelector flagSelector, UInt32 value, Boolean set) {
  UInt8 v8, version;
  UInt16 v16;
  Boolean le;

  if (bmp) {
    bmp = skipEmptySlot(bmp);
    version = BmpGetVersion(bmp);
    le = BmpLittleEndian(bmp);

    if (version == 2) {
        switch (selector) {
          case BitmapV2FieldWidth:
          case BitmapV2FieldHeight:
          case BitmapV2FieldRowBytes:
          case BitmapV2FieldFlags:
          case BitmapV2FieldPixelSize:
          case BitmapV2FieldVersion:
            value = BmpGetSetCommonField(bmp, (BitmapSelector)selector, flagSelector, value, set);
            break;
          case BitmapV2FieldNextDepthOffset:
            if (set) {
              put2(value, (UInt8 *)bmp, selector);
            } else {
              get2(&v16, (UInt8 *)bmp, selector);
              value = v16;
            }
            break;
          case BitmapV2FieldTransparentIndex:
          case BitmapV2FieldCompressionType:
            if (set) {
              put1(value, (UInt8 *)bmp, selector);
            } else {
              get1(&v8, (UInt8 *)bmp, selector);
              value = v8;
            }
            break;
         default:
            debug(DEBUG_ERROR, "Bitmap", "invalid bitmap selector %u", selector);
            value = 0;
            break;
        }
    } else {
      debug(DEBUG_ERROR, "Bitmap", "bitmap %p is not V2 (%u)", bmp, version);
      value = 0;
    }
  } else {
    debug(DEBUG_ERROR, "Bitmap", "null bitmap");
    value = 0;
  }

  return value;
}

UInt32 BmpV3GetSetField(BitmapType *bmp, BitmapV3Selector selector, BitmapFlagSelector flagSelector, UInt32 value, Boolean set) {
  UInt8 v8, version;
  UInt16 v16;
  UInt32 v32;
  Boolean le;

  if (bmp) {
    bmp = skipEmptySlot(bmp);
    version = BmpGetVersion(bmp);
    le = BmpLittleEndian(bmp);
    BmpGetCommonFlag(bmp, BitmapFlagLittleEndian);

    if (version == 3) {
        switch (selector) {
          case BitmapV3FieldWidth:
          case BitmapV3FieldHeight:
          case BitmapV3FieldRowBytes:
          case BitmapV3FieldFlags:
          case BitmapV3FieldPixelSize:
          case BitmapV3FieldVersion:
            value = BmpGetSetCommonField(bmp, (BitmapSelector)selector, flagSelector, value, set);
            break;
          case BitmapV3FieldSize:
          case BitmapV3FieldPixelFormat:
          case BitmapV3FieldUnused:
          case BitmapV3FieldCompressionType:
            if (set) {
              put1(value, (UInt8 *)bmp, selector);
            } else {
              get1(&v8, (UInt8 *)bmp, selector);
              value = v8;
            }
            break;
          case BitmapV3FieldDensity:
            if (set) {
              put2(value, (UInt8 *)bmp, selector);
            } else {
              get2(&v16, (UInt8 *)bmp, selector);
              value = v16;
            }
            break;
          case BitmapV3FieldTransparentValue:
            if (set) {
              put4(value, (UInt8 *)bmp, selector);
            } else {
              get4(&v32, (UInt8 *)bmp, selector);
              value = v32;
            }
            break;
          case BitmapV3FieldNextBitmapOffset:
            if (set) {
              put4(value, (UInt8 *)bmp, selector);
            } else {
              get4(&v32, (UInt8 *)bmp, selector);
              value = v32;
            }
            break;
         default:
            debug(DEBUG_ERROR, "Bitmap", "invalid bitmap selector %u", selector);
            value = 0;
            break;
        }
    } else {
      debug(DEBUG_ERROR, "Bitmap", "bitmap %p is not V3 (%u)", bmp, version);
      value = 0;
    }
  } else {
    debug(DEBUG_ERROR, "Bitmap", "null bitmap");
    value = 0;
  }

  return value;
}

BitmapTypeV3 *BmpCreateBitmapV3(const BitmapType *bitmapP, UInt16 density, const void *bitsP, const ColorTableType *colorTableP) {
  BitmapType *newBmp = NULL;
  Coord width, height;
  UInt32 colorTableSize, newSize, index, entry, transparentValue, addr;
  UInt16 rowBytes, numEntries, i;
  UInt8 version, depth;
  UInt8 *ram, *bitmapColorTable;
  Boolean le, hasColorTable, isDirectColor, indirectColorTable, hasTransparency;

  if (bitmapP && bitsP) {
    bitmapP = skipEmptySlot((BitmapType *)bitmapP);
    version = BmpGetVersion(bitmapP);
    le = BmpLittleEndian(bitmapP);
    hasColorTable = BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagHasColorTable);
    isDirectColor = BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagDirectColor);
    indirectColorTable = BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagIndirectColorTable);
    hasTransparency = BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagHasTransparency);

    newSize = BitmapV3HeaderSize;
    numEntries = 0;
    colorTableSize = 0;
    bitmapColorTable = NULL;
    transparentValue = 0;
    ram = pumpkin_heap_base();

    if (hasColorTable) {
      switch (version) {
        case 1:
          numEntries = BmpV1GetField(newBmp, BitmapV1FieldColorTable);
          colorTableSize = sizeof(UInt16) + numEntries * 4;
          bitmapColorTable = (UInt8 *)bitmapP + BitmapV1FieldColorTable;
          break;
        case 2:
          numEntries = BmpV2GetField(newBmp, BitmapV2FieldColorTable);
          colorTableSize = sizeof(UInt16) + numEntries * 4;
          bitmapColorTable = (UInt8 *)bitmapP + BitmapV2FieldColorTable;
          if (isDirectColor) {
            // BitmapDirectInfoType.transparentColor
            get4(&transparentValue, (UInt8 *)bitmapP, BitmapV2FieldColorTable + colorTableSize + 4);
          }
          break;
        case 3:
          if (indirectColorTable) {
            colorTableSize = 4; // pointer to color table
            get4(&addr, (UInt8 *)bitmapP, BitmapV3FieldColorTable);
            bitmapColorTable = addr ? ram + addr : NULL;
          } else {
            numEntries = BmpV3GetField(newBmp, BitmapV3FieldColorTable);
            colorTableSize = sizeof(UInt16) + numEntries * 4;
            bitmapColorTable = (UInt8 *)bitmapP + BitmapV3FieldColorTable;
          }
          break;
      }
    }

    if (colorTableP) {
      get2(&numEntries, (UInt8 *)colorTableP, 0);
      bitmapColorTable = (UInt8 *)colorTableP;
      hasColorTable = 1;
      indirectColorTable = 1;
      newSize += 4; // pointer to color table
    } else {
      newSize += colorTableSize;
    }

    BmpGetDimensions(bitmapP, &width, &height, &rowBytes);
    newSize += rowBytes * height;

    if ((newBmp = MemPtrNew(newSize)) != NULL) {
      // density: if 0, the returned bitmap's density is set to the default value of kDensityLow
      if (density == 0) density = kDensityLow;
      depth = BmpGetBitDepth(bitmapP);
    
      BmpSetCommonField(newBmp, BitmapFieldWidth, width);
      BmpSetCommonField(newBmp, BitmapFieldHeight, height);
      BmpSetCommonField(newBmp, BitmapFieldRowBytes, rowBytes);
      BmpSetCommonFlag(newBmp, BitmapFlagAll, BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagAll));
      BmpSetCommonFlag(newBmp, BitmapFlagIndirect, 0);
      BmpSetCommonFlag(newBmp, BitmapFlagHasColorTable, hasColorTable);
      BmpSetCommonFlag(newBmp, BitmapFlagIndirectColorTable, indirectColorTable);
      BmpSetCommonField(newBmp, BitmapFieldPixelSize, depth);
      BmpSetCommonField(newBmp, BitmapFieldVersion, 3);

      BmpV3SetField(newBmp, BitmapV3FieldSize, BitmapV3HeaderSize);
      BmpV3SetField(newBmp, BitmapV3FieldCompressionType, BmpGetCompressionType(bitmapP));
      BmpV3SetField(newBmp, BitmapV3FieldDensity, density);
      BmpV3SetField(newBmp, BitmapV3FieldNextBitmapOffset, 0); // XXX what if the source bitmap has a next value ?

      debug(DEBUG_TRACE, "Bitmap", "BmpCreateBitmapV3 %p %dx%d density %d rowBytes %d", newBmp, width, height, density, rowBytes);

      switch (version) {
        case 0:
          debug(DEBUG_TRACE, "Bitmap", "BmpCreateBitmapV3 create from V0 %p", bitmapP);
          BmpV3SetField(newBmp, BitmapV3FieldPixelFormat, pixelFormatIndexed);
          BmpSetCommonField(newBmp, BitmapFieldPixelSize, 1);
          break;
        case 1:
          debug(DEBUG_TRACE, "Bitmap", "BmpCreateBitmapV3 create from V1 %p", bitmapP);
          BmpV3SetField(newBmp, BitmapV3FieldPixelFormat, pixelFormatIndexed);
          BmpSetCommonField(newBmp, BitmapFieldPixelSize, BmpGetCommonField((BitmapType *)bitmapP, BitmapFieldPixelSize));
          break;
        case 2:
          debug(DEBUG_TRACE, "Bitmap", "BmpCreateBitmapV3 create from V2 %p", bitmapP);
          BmpV3SetField(newBmp, BitmapV3FieldPixelFormat, pixelFormatIndexed);
          BmpSetCommonField(newBmp, BitmapFieldPixelSize, BmpGetCommonField((BitmapType *)bitmapP, BitmapFieldPixelSize));
          if (hasTransparency) {
            if (isDirectColor) {
              BmpV3SetField(newBmp, BitmapV3FieldTransparentValue, transparentValue);
            } else {
              BmpV3SetField(newBmp, BitmapV3FieldTransparentValue, BmpV2GetField((BitmapType *)bitmapP, BitmapV2FieldTransparentIndex));
            }
          }
          break;
        case 3:
          debug(DEBUG_TRACE, "Bitmap", "BmpCreateBitmapV3 create from V3 %p", bitmapP);
          BmpV3SetField(newBmp, BitmapV3FieldPixelFormat, BmpV3GetField(newBmp, BitmapV3FieldPixelFormat));
          BmpSetCommonField(newBmp, BitmapFieldPixelSize, BmpGetCommonField((BitmapType *)bitmapP, BitmapFieldPixelSize));
          if (hasTransparency) {
            BmpV3SetField(newBmp, BitmapV3FieldTransparentValue, BmpV3GetField(newBmp, BitmapV3FieldTransparentValue));
          }
          break;
        default:
          debug(DEBUG_ERROR, "Bitmap", "BmpCreateBitmapV3 create from invalid V%d", version);
          break;
      }

      index = BitmapV3HeaderSize;

      if (hasColorTable) {
        if (indirectColorTable) {
          // indirect color table: pointer
          index += put4b(bitmapColorTable - ram, (UInt8 *)newBmp, index);
        } else {
          // direct color table: numEntries followed by entries
          index += put2b(numEntries, (UInt8 *)newBmp, index);
          for (i = 0; i < numEntries; i++) {
            get4(&entry, bitmapColorTable, 2 + i * 4);
            index += put4b(entry, (UInt8 *)newBmp, index);
          }
        }
      }

      // copy the pixels
      MemMove((UInt8 *)newBmp + index, bitsP, rowBytes * height);
    }
  }

  debug(DEBUG_TRACE, "Bitmap", "BmpCreateBitmapV3 returning bitmap %p", newBmp);
  return (BitmapTypeV3 *)newBmp;
}

// This function creates an uncompressed, non-transparent Version Two bitmap with the width, height, and depth that you specify.
BitmapType *BmpCreate(Coord width, Coord height, UInt8 depth, ColorTableType *colorTableP, UInt16 *error) {
  BitmapType *bitmapP;
  UInt16 numEntries, rowBytes, v16, i;
  UInt32 newSize, index, v32;

  if (error) *error = sysErrParamErr;

  if (width <= 0 || height <= 0) {
    debug(DEBUG_ERROR, "Bitmap", "BmpCreate invalid width %d or height %d", width, height);
    return NULL;
  }

  switch (depth) {
    case  1: numEntries = 2;   rowBytes = (width + 7) / 8; break;
    case  2: numEntries = 4;   rowBytes = (width + 3) / 4; break;
    case  4: numEntries = 16;  rowBytes = (width + 1) / 2; break;
    case  8: numEntries = 256; rowBytes = width; break;
    case 16: numEntries = 256; rowBytes = width * 2; break;
    case 24: numEntries = 256; rowBytes = width * 3; break;
    case 32: numEntries = 256; rowBytes = width * 4; break;
    default:
      debug(DEBUG_ERROR, "Bitmap", "BmpCreate invalid depth %d", depth);
      return NULL;
  }

  newSize = BitmapV2HeaderSize;

  if (colorTableP) {
    get2b(&v16, (UInt8 *)colorTableP, 0);
    if (v16 != numEntries) {
      debug(DEBUG_ERROR, "Bitmap", "BmpCreate wrong colorTable numEntries %d for depth %d", v16, depth);
      return NULL;
    }
    newSize += sizeof(UInt16) + numEntries * 4;
  }

  newSize += rowBytes * height;
  if ((bitmapP = MemPtrNew(newSize)) == NULL) {
    if (error) *error = sysErrNoFreeResource;
    return NULL;
  }

  BmpSetCommonField(bitmapP, BitmapFieldWidth, width);
  BmpSetCommonField(bitmapP, BitmapFieldHeight, height);
  BmpSetCommonField(bitmapP, BitmapFieldRowBytes, rowBytes);
  BmpSetCommonFlag(bitmapP, BitmapFlagAll, 0);
  BmpSetCommonFlag(bitmapP, BitmapFlagHasColorTable, colorTableP != NULL);
  BmpSetCommonField(bitmapP, BitmapFieldPixelSize, depth);
  BmpSetCommonField(bitmapP, BitmapFieldVersion, 2);

  if (colorTableP) {
    // direct color table: numEntries followed by entries
    index = BitmapV2HeaderSize;
    index += put2b(numEntries, (UInt8 *)bitmapP, index);
    for (i = 0; i < numEntries; i++) {
      get4b(&v32, (UInt8 *)colorTableP, 2 + i * 4);
      index += put4b(v32, (UInt8 *)bitmapP, index);
    }
  }

  if (error) *error = errNone;

  return bitmapP;
}

BitmapType *BmpCreate3(Coord width, Coord height, UInt16 rowBytes, UInt16 density, UInt8 depth, Boolean hasTransparency, UInt32 transparentValue, ColorTableType *colorTableP, UInt16 *error) {
  BitmapType *bitmapP;
  UInt16 numEntries, v16, i;
  UInt32 newSize, index, v32;

  if (error) *error = sysErrParamErr;

  if (width <= 0 || height <= 0) {
    debug(DEBUG_ERROR, "Bitmap", "BmpCreate3 invalid width %d or height %d", width, height);
    return NULL;
  }

  switch (depth) {
    case  1: numEntries = 2;   if (rowBytes == 0) rowBytes = (width + 7) / 8; break;
    case  2: numEntries = 4;   if (rowBytes == 0) rowBytes = (width + 3) / 4; break;
    case  4: numEntries = 16;  if (rowBytes == 0) rowBytes = (width + 1) / 2; break;
    case  8: numEntries = 256; if (rowBytes == 0) rowBytes = width; break;
    case 16: numEntries = 256; if (rowBytes == 0) rowBytes = width * 2; break;
    case 24: numEntries = 256; if (rowBytes == 0) rowBytes = width * 3; break;
    case 32: numEntries = 256; if (rowBytes == 0) rowBytes = width * 4; break;
    default:
      debug(DEBUG_ERROR, "Bitmap", "BmpCreate3 invalid depth %d", depth);
      return NULL;
  }

  switch (density) {
    case kDensityLow:
    case kDensityDouble:
      break;
    default:
      debug(DEBUG_ERROR, "Bitmap", "BmpCreate3 invalid density %d", density);
      return NULL;
  }

  newSize = BitmapV3HeaderSize;

  if (colorTableP) {
    get2b(&v16, (UInt8 *)colorTableP, 0);
    if (v16 != numEntries) {
      debug(DEBUG_ERROR, "Bitmap", "BmpCreate3 wrong colorTable numEntries %d for depth %d", v16, depth);
      return NULL;
    }
    newSize += sizeof(UInt16) + numEntries * 4;
  }

  newSize += rowBytes * height;
  if ((bitmapP = MemPtrNew(newSize)) == NULL) {
    if (error) *error = sysErrNoFreeResource;
    return NULL;
  }

  BmpSetCommonField(bitmapP, BitmapFieldWidth, width);
  BmpSetCommonField(bitmapP, BitmapFieldHeight, height);
  BmpSetCommonField(bitmapP, BitmapFieldRowBytes, rowBytes);
  BmpSetCommonFlag(bitmapP, BitmapFlagAll, 0);
  BmpSetCommonFlag(bitmapP, BitmapFlagHasColorTable, colorTableP != NULL);
  BmpSetCommonFlag(bitmapP, BitmapFlagHasTransparency, hasTransparency);
  BmpSetCommonField(bitmapP, BitmapFieldPixelSize, depth);
  BmpSetCommonField(bitmapP, BitmapFieldVersion, 3);

  BmpV3SetField(bitmapP, BitmapV3FieldSize, BitmapV3HeaderSize);
  BmpV3SetField(bitmapP, BitmapV3FieldPixelFormat, pixelFormatIndexed);
  BmpV3SetField(bitmapP, BitmapV3FieldCompressionType, BitmapCompressionTypeNone);
  BmpV3SetField(bitmapP, BitmapV3FieldDensity, density);
  BmpV3SetField(bitmapP, BitmapV3FieldTransparentValue, transparentValue);

  if (colorTableP) {
    // direct color table: numEntries followed by entries
    index = BitmapV3HeaderSize;
    index += put2b(numEntries, (UInt8 *)bitmapP, index);
    for (i = 0; i < numEntries; i++) {
      get4b(&v32, (UInt8 *)colorTableP, 2 + i * 4);
      index += put4b(v32, (UInt8 *)bitmapP, index);
    }
  }

  if (error) *error = errNone;

  return bitmapP;
}

static uint32_t BmpSurfaceGetPixel(void *data, int x, int y) {
  bmp_surface_t *bsurf = (bmp_surface_t *)data;
  return BmpGetPixelValue(bsurf->bitmapP, x, y);
}

static void BmpSurfaceSetPixel(void *data, int x, int y, uint32_t color) {
  bmp_surface_t *bsurf = (bmp_surface_t *)data;
  BmpSetPixel(bsurf->bitmapP, x, y, color);
}

static int BmpSurfaceGetTransparent(void *data, uint32_t *transp) {
  bmp_surface_t *bsurf = (bmp_surface_t *)data;
  UInt32 transparentValue;
  Boolean transparent;

  transparent = BmpGetTransparentValue(bsurf->bitmapP, &transparentValue);
  *transp = transparentValue;

  return transparent;
}

static uint32_t BmpSurfaceColorRgb(void *data, int red, int green, int blue, int alpha) {
  bmp_surface_t *bsurf = (bmp_surface_t *)data;
  UInt8 depth = BmpGetBitDepth(bsurf->bitmapP);
  ColorTableType *colorTable;
  UInt32 transparentValue;
  Boolean transp;
  uint32_t color = 0;

  transp = BmpGetTransparentValue(bsurf->bitmapP, &transparentValue);

  switch (depth) {
    case 1:
      color = rgbToGray1(red, green, blue);
      break;
    case 2:
      color = rgbToGray2(red, green, blue);
      break;
    case 4:
      color = rgbToGray4(red, green, blue);
      break;
    case 8:
      if (transp && alpha == 0x00) {
        color = transparentValue;
      } else {
        colorTable = BmpGetColortable(bsurf->bitmapP);
        if (colorTable == NULL) colorTable = pumpkin_defaultcolorTable();
        color = BmpRGBToIndex(red, green, blue, colorTable);
      }
      break;
    case 16:
      if (transp && alpha == 0x00) {
        color = transparentValue;
      } else {
        color = rgb565(red, green, blue);
      }
      break;
    case 32:
      color = rgba32(red, green, blue, alpha);
      break;
  }

  return color;
}

static void BmpSurfaceRgbColor(void *data, uint32_t color, int *red, int *green, int *blue, int *alpha) {
  bmp_surface_t *bsurf = (bmp_surface_t *)data;
  UInt8 depth = BmpGetBitDepth(bsurf->bitmapP);
  ColorTableType *colorTable;
  UInt32 transparentValue;
  UInt8 r, g, b;
  Boolean transp;

  transp = BmpGetTransparentValue(bsurf->bitmapP, &transparentValue);

  switch (depth) {
    case 1:
      *red = *green = *blue = gray1values[color & 0x01];
      *alpha = 0xFF;
      break;
    case 2:
      *red = *green = *blue = gray2values[color & 0x03];
      *alpha = 0xFF;
      break;
    case 4:
      *red = *green = *blue = gray4values[color & 0x0F];
      *alpha = 0xFF;
      break;
    case 8:
      colorTable = BmpGetColortable(bsurf->bitmapP);
      if (colorTable == NULL) colorTable = pumpkin_defaultcolorTable();
      BmpIndexToRGB(color & 0xFF, &r, &g, &b, colorTable);
      *red   = r;
      *green = g;
      *blue  = b;
      *alpha = transp && color == transparentValue ? 0x00 : 0xFF;
      break;
    case 16:
      *red   = r565(color);
      *green = g565(color);
      *blue  = b565(color);
      *alpha = transp && color == transparentValue ? 0x00 : 0xFF;
      break;
    case 32:
      *red   = r32(color);
      *green = g32(color);
      *blue  = b32(color);
      *alpha = a32(color);
      break;
  }
}

static void BmpSurfaceDestroy(void *data) {
  bmp_surface_t *bsurf = (bmp_surface_t *)data;

  if (bsurf) {
    if (bsurf->h) MemHandleUnlock(bsurf->h);
    if (bsurf->h) DmReleaseResource(bsurf->h);
    xfree(bsurf);
  }
}

surface_t *BmpCreateSurfaceBitmap(BitmapType *bitmapP) {
  surface_t *surface = NULL;
  bmp_surface_t *bsurf;
  ColorTableType *colorTable;
  RGBColorType rgb, *palette;
  Coord width, height;
  UInt16 i, numEntries;
  Int16 encoding;
  UInt8 depth;

  bitmapP = skipEmptySlot(bitmapP);
  depth = BmpGetBitDepth(bitmapP);

  switch (depth) {
    case  1:
    case  2:
    case  4:
    case  8: encoding = SURFACE_ENCODING_PALETTE; break;
    case 16: encoding = SURFACE_ENCODING_RGB565;  break;
    case 32: encoding = SURFACE_ENCODING_ARGB;    break;
    default:
      debug(DEBUG_ERROR, "Bitmap", "BmpBitmapCreateSurface unsupported depth %d", depth);
      encoding = -1;
      break;
  }

  if (encoding >= 0) {
    if ((bsurf = xcalloc(1, sizeof(bmp_surface_t))) != NULL) {
      if ((surface = xcalloc(1, sizeof(surface_t))) != NULL) {
        bsurf->bitmapP = bitmapP;

        BmpGetDimensions(bitmapP, &width, &height, NULL);
        surface->tag = TAG_SURFACE;
        surface->encoding = encoding;
        surface->width = width;
        surface->height = height;
        surface->getpixel = BmpSurfaceGetPixel;
        surface->setpixel = BmpSurfaceSetPixel;
        surface->gettransp = BmpSurfaceGetTransparent;
        surface->rgb_color = BmpSurfaceRgbColor;
        surface->color_rgb = BmpSurfaceColorRgb;
        surface->destroy = BmpSurfaceDestroy;
        surface->data = bsurf;

        if (encoding == SURFACE_ENCODING_PALETTE) {
          colorTable = BmpGetColortable(bitmapP);

          if (colorTable) {
            numEntries = CtbGetNumEntries(colorTable);
            for (i = 0; i < numEntries; i++) {
              CtbGetEntry(colorTable, i, &rgb);
              surface_palette(surface, i, rgb.r, rgb.g, rgb.b);
            }
          } else {
            palette = WinGetPalette(depth);
            switch (depth) {
              case 1:
                for (i = 0; i < 2; i++) {
                  surface_palette(surface, i, palette[i].r, palette[i].g, palette[i].b);
                }
                break;
              case 2:
                for (i = 0; i < 4; i++) {
                  surface_palette(surface, i, palette[i].r, palette[i].g, palette[i].b);
                }
                break;
              case 4:
                for (i = 0; i < 16; i++) {
                  surface_palette(surface, i, palette[i].r, palette[i].g, palette[i].b);
                }
                break;
              case 8:
                for (i = 0; i < 256; i++) {
                  surface_palette(surface, i, palette[i].r, palette[i].g, palette[i].b);
                }
                break;
            }
          }
        }
      } else {
        xfree(bsurf);
      }
    }
  }

  return surface;
}

surface_t *BmpCreateSurface(UInt16 id) {
  surface_t *surface = NULL;
  bmp_surface_t *bsurf;
  MemHandle h;
  BitmapType *bitmapP;

  if ((h = DmGetResource(bitmapRsc, id)) != NULL) {
    if ((bitmapP = MemHandleLock(h)) != NULL) {
      surface = BmpCreateSurfaceBitmap(bitmapP);
      bsurf = (bmp_surface_t *)surface->data;
      bsurf->h = h;
    } else {
      DmReleaseResource(h);
    }
  }

  return surface;
}

Err BmpDelete(BitmapType *bitmapP) {
  if (bitmapP) {
    debug(DEBUG_TRACE, "Bitmap", "MemChunkFree %p", bitmapP);
    MemChunkFree(bitmapP); // caused a fault in BookWorm once
  }

  return errNone;
}

void BmpGetDimensions(const BitmapType *bitmapP, Coord *widthP, Coord *heightP, UInt16 *rowBytesP) {
  BitmapType *bmp;

  if (bitmapP) {
    bmp = skipEmptySlot((BitmapType *)bitmapP);
    if (widthP) *widthP = BmpGetCommonField(bmp, BitmapFieldWidth);
    if (heightP) *heightP = BmpGetCommonField(bmp, BitmapFieldHeight);
    if (rowBytesP) *rowBytesP = BmpGetCommonField(bmp, BitmapFieldRowBytes);
  }
}

BitmapType *BmpGetBestBitmapEx(BitmapPtr bitmapP, UInt16 density, UInt8 depth, Boolean checkAddr) {
  UInt16 best_depth, best_density;
  Boolean exact_depth;
  Boolean exact_density;
  WinHandle display;
  BitmapType *displayBmp;
  BitmapType *best = NULL;
  Coord width, height;
  UInt8 version, bitmapDepth, displayDepth;
  UInt16 bitmapDensity, displayDensity, rowBytes;
  UInt32 size, offset;
  UInt8 *bmp, *last, *base, *end;

  if (bitmapP) {
    debug(DEBUG_TRACE, "Bitmap", "BmpGetBestBitmap %p begin", bitmapP);
    base = (uint8_t *)pumpkin_heap_base();
    end = base + pumpkin_heap_size();
    size = MemPtrSize(bitmapP);
    last = size ? (UInt8 *)bitmapP + size : NULL;

    display = WinGetDisplayWindow();
    displayBmp = WinGetBitmap(display);
    displayDepth = BmpGetBitDepth(displayBmp);
    displayDensity = BmpGetDensity(displayBmp);

    for (best = NULL, best_depth = 0, best_density = 0, exact_depth = false, exact_density = false; bitmapP;) {
      bmp = (uint8_t *)bitmapP;
      if (checkAddr && (bmp < base || bmp >= end)) {
        debug(DEBUG_ERROR, "Bitmap", "BmpGetBestBitmap invalid bitmap %p", bitmapP);
        break;
      }

      if (last && bmp >= last) break;

      bitmapP = skipEmptySlot(bitmapP);
      version = BmpGetVersion(bitmapP);
      BmpGetDimensions(bitmapP, &width, &height, &rowBytes);
      bitmapDepth = BmpGetBitDepth(bitmapP);

      switch (version) {
        case 0:
          debug(DEBUG_TRACE, "Bitmap", "BmpGetBestBitmap candidate V%d, %dx%d, bpp %d, density %d", version, width, height, 1, kDensityLow);
          if (best == NULL) {
            best = bitmapP;
            best_depth = 1;
            best_density = kDensityLow;
            exact_depth = best_depth == depth;
            exact_density = best_density == density;
          }
          //bitmapP = (BitmapType *)((UInt8 *)bitmapP + size);
          bitmapP = NULL; // XXX is it possible to have a chain of version 0 bitmaps ? how ?
          break;
        case 1:
          debug(DEBUG_TRACE, "Bitmap", "BmpGetBestBitmap candidate V%d, %dx%d, bpp %d, density %d", version, width, height, bitmapDepth, kDensityLow);
          if (best == NULL || (bitmapDepth > best_depth && bitmapDepth <= displayDepth && !exact_depth)) {
            best = bitmapP;
            best_depth = bitmapDepth;
            best_density = kDensityLow;
            exact_depth = best_depth == depth;
            exact_density = best_density == density;
          }
          offset = BmpV1GetField(bitmapP, BitmapV1FieldNextDepthOffset);
          bitmapP = offset ? (BitmapType *)((UInt8 *)bitmapP + offset * 4) : NULL;
          debug(DEBUG_TRACE, "Bitmap", "BmpGetBestBitmap next offset %u", offset * 4);
          break;
        case 2:
          debug(DEBUG_TRACE, "Bitmap", "BmpGetBestBitmap candidate V%d, %dx%d, bpp %d, density %d", version, width, height, bitmapDepth, kDensityLow);
          if (best == NULL || (bitmapDepth > best_depth && bitmapDepth <= displayDepth && !exact_depth)) {
            best = bitmapP;
            best_depth = bitmapDepth;
            best_density = kDensityLow;
            exact_depth = best_depth == depth;
            exact_density = best_density == density;
          }
          offset = BmpV2GetField(bitmapP, BitmapV2FieldNextDepthOffset);
          bitmapP = offset ? (BitmapType *)((UInt8 *)bitmapP + offset * 4) : NULL;
          debug(DEBUG_TRACE, "Bitmap", "BmpGetBestBitmap next offset %u", offset * 4);
          break;
        case 3:
          bitmapDensity = BmpGetDensity(bitmapP);
          debug(DEBUG_TRACE, "Bitmap", "BmpGetBestBitmap candidate V%d, %dx%d, bpp %d, density %d", version, width, height, bitmapDepth, bitmapDensity);
          if (best == NULL || (bitmapDepth >= best_depth && bitmapDensity >= best_density &&
              bitmapDepth <= displayDepth && bitmapDensity <= displayDensity && (!exact_density || exact_density))) {
            best = bitmapP;
            best_depth = bitmapDepth;
            best_density = bitmapDensity;
            exact_depth = best_depth == depth;
            exact_density = best_density == density;
          }
          offset = BmpV3GetField(bitmapP, BitmapV3FieldNextBitmapOffset);
          bitmapP = offset ? (BitmapType *)((UInt8 *)bitmapP + offset) : NULL;
          debug(DEBUG_TRACE, "Bitmap", "BmpGetBestBitmap next offset %u", offset);
          break;
        default:
          debug(DEBUG_ERROR, "Bitmap", "BmpGetBestBitmap invalid version %d", version);
          debug_bytes(DEBUG_ERROR, "Bitmap", (UInt8 *)bitmapP, 256);
          return NULL;
      }
    }

    if (best) {
      version = BmpGetVersion(best);
      BmpGetDimensions(best, &width, &height, NULL);
      debug(DEBUG_TRACE, "Bitmap", "BmpGetBestBitmap V%d, %dx%d, bpp %d (%d), density %d",
        version, width, height, best_depth, exact_depth ? 1 : 0, best_density);
    } else {
      debug(DEBUG_ERROR, "Bitmap", "BmpGetBestBitmap no suitable bitmap for bpp %d, density %d", depth, density);
    }
  }

  return best;
}

BitmapType *BmpGetBestBitmap(BitmapPtr bitmapP, UInt16 density, UInt8 depth) {
  return BmpGetBestBitmapEx(bitmapP, density, depth, true);
}

Err BmpCompress(BitmapType *bitmapP, BitmapCompressionType compType) {
  debug(DEBUG_ERROR, "Bitmap", "BmpCompress not implemented");
  return sysErrParamErr;
}

void *BmpGetBits(BitmapType *bitmapP) {
  UInt32 headerSize, addr;
  void *bits = NULL;
  Boolean le;

  if (bitmapP) {
    bitmapP = skipEmptySlot(bitmapP);
    le = BmpLittleEndian(bitmapP);

    switch (BmpGetVersion(bitmapP)) {
      case 0:
        headerSize = BitmapV0HeaderSize;
        bits = (UInt8 *)bitmapP + headerSize;
        break;
      case 1:
        headerSize = BitmapV1HeaderSize + BmpColortableSize(bitmapP);
        bits = (UInt8 *)bitmapP + headerSize;
        break;
      case 2:
        headerSize = BitmapV2HeaderSize + BmpColortableSize(bitmapP);
        if (BmpGetCommonFlag(bitmapP, BitmapFlagDirectColor)) {
          headerSize += 8;
        }
        if (BmpGetCommonFlag(bitmapP, BitmapFlagIndirect)) {
          get4(&addr, (UInt8 *)bitmapP, headerSize);
          bits = addr ? pumpkin_heap_base() + addr : NULL;
        } else {
          bits = (UInt8 *)bitmapP + headerSize;
        }
        break;
      case 3:
        headerSize = BitmapV3HeaderSize;
        if (BmpGetCommonFlag(bitmapP, BitmapFlagIndirectColorTable)) {
          headerSize += 4;
        } else {
          headerSize += BmpColortableSize(bitmapP);
        }
        if (BmpGetCommonFlag(bitmapP, BitmapFlagIndirect)) {
          get4(&addr, (UInt8 *)bitmapP, headerSize);
          bits = addr ? pumpkin_heap_base() + addr : NULL;
        } else {
          bits = (UInt8 *)bitmapP + headerSize;
        }
        break;
    }
  }

  return bits;
}

ColorTableType *BmpGetColortable(BitmapType *bitmapP) {
  ColorTableType *colorTable = NULL;
  UInt32 addr;
  Boolean le;

  if (bitmapP) {
    bitmapP = skipEmptySlot(bitmapP);

    if (BmpGetCommonFlag(bitmapP, BitmapFlagHasColorTable)) {
      le = BmpLittleEndian(bitmapP);

      switch (BmpGetVersion(bitmapP)) {
        case 1:
          colorTable = (ColorTableType *)((UInt8 *)bitmapP + BitmapV1FieldColorTable);
          break;
        case 2:
          colorTable = (ColorTableType *)((UInt8 *)bitmapP + BitmapV2FieldColorTable);
          break;
        case 3:
          if (BmpGetCommonFlag(bitmapP, BitmapFlagIndirectColorTable)) {
            get4(&addr, (UInt8 *)bitmapP, BitmapV3FieldColorTable);
            colorTable = addr ? (ColorTableType *)(pumpkin_heap_base() + addr) : NULL;
          } else {
            colorTable = (ColorTableType *)((UInt8 *)bitmapP + BitmapV3FieldColorTable);
          }
          break;
      }
    }
  }

  return colorTable;
}

UInt16 BmpSize(const BitmapType *bitmapP) {
  UInt32 headerSize, dataSize;

  BmpGetSizes(bitmapP, &dataSize, &headerSize);

  return headerSize + dataSize;
}

UInt16 BmpBitsSize(const BitmapType *bitmapP) {
  UInt32 bitsSize;

  BmpGetSizes(bitmapP, &bitsSize, NULL);

  return bitsSize;
}

void BmpGetSizes(const BitmapType *bitmapP, UInt32 *dataSizeP, UInt32 *headerSizeP) {
  UInt32 v32, headerSize = 0, dataSize = 0;
  UInt16 rowBytes, v16;
  UInt8 *bits;
  Boolean le, compressed;
  Coord width, height;

  if (bitmapP) {
    bitmapP = skipEmptySlot((BitmapType *)bitmapP);
    le = BmpLittleEndian(bitmapP);
    BmpGetDimensions(bitmapP, &width, &height, &rowBytes);
    bits = BmpGetBits((BitmapType *)bitmapP);
    compressed = BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagCompressed);
    dataSize = rowBytes * height;

    switch (BmpGetVersion(bitmapP)) {
      case 0:
        headerSize = BitmapV0HeaderSize;
        if (compressed) {
          get2(&v16, bits, 0);
          dataSize = v16;
        }
        break;
      case 1:
        headerSize = BitmapV1HeaderSize + BmpColortableSize(bitmapP);
        if (compressed) {
          get2(&v16, bits, 0);
          dataSize = v16;
        }
        break;
      case 2:
        headerSize = BitmapV2HeaderSize + BmpColortableSize(bitmapP);
        if (BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagDirectColor)) {
          headerSize += 8;
        }
        if (compressed) {
          get2(&v16, bits, 0);
          dataSize = v16;
        }
        break;
      case 3:
        headerSize = BitmapV3HeaderSize;
        if (BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagIndirectColorTable)) {
          headerSize += 4;
        } else {
          headerSize += BmpColortableSize(bitmapP);
        }
        if (compressed) {
          get4(&v32, bits, 0);
          dataSize = v32;
        }
        break;
    }
  }

  if (dataSizeP) *dataSizeP = dataSize;
  if (headerSizeP) *headerSizeP = headerSize;
}

UInt16 BmpColortableSize(const BitmapType *bitmapP) {
  ColorTableType *colorTable;
  UInt16 numEntries, size = 0;
  Boolean le;

  if (bitmapP) {
    le = BmpLittleEndian(bitmapP);
    if ((colorTable = BmpGetColortable((BitmapType *)bitmapP)) != NULL) {
      get2(&numEntries, (UInt8 *)colorTable, 0);
      size = sizeof(UInt16) + numEntries * 4;
    }
  }

  return size;
}

UInt8 BmpGetBitDepth(const BitmapType *bitmapP) {
  UInt8 version, depth = 0;

  if (bitmapP) {
    version = BmpGetVersion((BitmapType *)bitmapP);
    depth = version == 0 ? 1 : BmpGetCommonField((BitmapType *)bitmapP, BitmapFieldPixelSize);
  }

  return depth;
}

Err BmpSetBitDepth(BitmapType *bitmapP, UInt8 depth) {
  Coord width, height;
  Err err = sysErrParamErr;

  if (bitmapP && BmpGetVersion(bitmapP) == 3) {
    BmpGetDimensions(bitmapP, &width, &height, NULL);

    switch (depth) {
      case 1:
        BmpSetCommonField(bitmapP, BitmapFieldPixelSize, 1);
        BmpSetCommonField(bitmapP, BitmapFieldRowBytes, (width + 7) / 8);
        err = errNone;
        break;
      case 2:
        BmpSetCommonField(bitmapP, BitmapFieldPixelSize, 2);
        BmpSetCommonField(bitmapP, BitmapFieldRowBytes, (width + 3) / 4);
        err = errNone;
        break;
      case 4:
        BmpSetCommonField(bitmapP, BitmapFieldPixelSize, 4);
        BmpSetCommonField(bitmapP, BitmapFieldRowBytes, (width + 1) / 2);
        err = errNone;
        break;
      case 8:
        BmpSetCommonField(bitmapP, BitmapFieldPixelSize, 8);
        BmpSetCommonField(bitmapP, BitmapFieldRowBytes, width);
        err = errNone;
        break;
      case 16:
        BmpSetCommonField(bitmapP, BitmapFieldPixelSize, 16);
        BmpSetCommonField(bitmapP, BitmapFieldRowBytes, width * 2);
        err = errNone;
        break;
    }
  }

  return err;
}

BitmapType *BmpGetNextBitmapAnyDensity(BitmapType *bitmapP) {
  UInt32 offset;

  if (bitmapP) {
    bitmapP = skipEmptySlot(bitmapP);

    switch (BmpGetVersion(bitmapP)) {
      case 0:
        bitmapP = NULL;
        break;
      case 1:
        offset = BmpV1GetField(bitmapP, BitmapV1FieldNextDepthOffset);
        bitmapP = offset ? (BitmapType *)((UInt8 *)bitmapP + offset * 4) : NULL;
        break;
      case 2:
        offset = BmpV2GetField(bitmapP, BitmapV2FieldNextDepthOffset);
        bitmapP = offset ? (BitmapType *)((UInt8 *)bitmapP + offset * 4) : NULL;
        break;
      case 3:
        offset = BmpV3GetField(bitmapP, BitmapV3FieldNextBitmapOffset);
        bitmapP = offset ? (BitmapType *)((UInt8 *)bitmapP + offset) : NULL;
        break;
      default:
        debug(DEBUG_ERROR, "Bitmap", "BmpGetNextBitmap invalid version %d", BmpGetVersion(bitmapP));
        debug_bytes(DEBUG_ERROR, "Bitmap", (uint8_t *)bitmapP, 256);
        bitmapP = NULL;
        break;
    }
  }

  return bitmapP;
}

BitmapType *BmpGetNextBitmap(BitmapType *bitmapP) {
  for (;;) {
    bitmapP = BmpGetNextBitmapAnyDensity(bitmapP);
    if (bitmapP == NULL) break;
    if (BmpGetDensity(bitmapP) == kDensityDouble) continue;
  }

  return bitmapP;
}

BitmapCompressionType BmpGetCompressionType(const BitmapType *bitmapP) {
  BitmapCompressionType ct = BitmapCompressionTypeNone;

  if (bitmapP && BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagCompressed)) {
    switch (BmpGetVersion(bitmapP)) {
      case 0:
      case 1: ct = BitmapCompressionTypeScanLine; break;
      case 2: ct = BmpV2GetField((BitmapType *)bitmapP, BitmapV2FieldCompressionType); break;
      case 3: ct = BmpV3GetField((BitmapType *)bitmapP, BitmapV3FieldCompressionType); break;
    }
  }

  return ct;
}

Boolean BmpGetNoDither(const BitmapType *bitmapP) {
  return bitmapP ? BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagNoDither) != 0 : false;
}

UInt16 BmpGetDensity(const BitmapType *bitmapP) {
  UInt16 d = kDensityLow;

  if (bitmapP) {
    if (BmpGetVersion(bitmapP) == 3) {
      d = BmpV3GetField((BitmapType *)bitmapP, BitmapV3FieldDensity);
    }
  }

  return d;
}

Err BmpSetDensity(BitmapType *bitmapP, UInt16 density) {
  Err err = sysErrParamErr;

  if (bitmapP) {
    if (BmpGetVersion(bitmapP) >= 3) {
      switch (density) {
        case kDensityLow:
        case kDensityDouble:
          BmpV3SetField((BitmapType *)bitmapP, BitmapV3FieldDensity, density);
          err = errNone;
          break;
       }
    }
  }

  return err;
}

Boolean BmpGetTransparentValue(const BitmapType *bitmapP, UInt32 *transparentValueP) {
  Boolean hasTransparentValue, r = false;
  UInt16 colorTableSize;
  Boolean le;

  if (transparentValueP) *transparentValueP = 0;

  if (bitmapP) {
    bitmapP = skipEmptySlot((BitmapType *)bitmapP);
    le = BmpLittleEndian(bitmapP);

    switch (BmpGetVersion(bitmapP)) {
      case 2:
        if (BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagHasTransparency)) {
          if (transparentValueP) {
            hasTransparentValue = BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagDirectColor);
            if (hasTransparentValue) {
              colorTableSize =  BmpColortableSize(bitmapP);
              get4(transparentValueP, (UInt8 *)bitmapP, BitmapV2FieldColorTable + colorTableSize + 4);
            } else {
              *transparentValueP = BmpV2GetField((BitmapType *)bitmapP, BitmapV2FieldTransparentIndex);
            }
          }
          r = true;
        }
        break;
      case 3:
        if (BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagHasTransparency)) {
          if (transparentValueP) {
            *transparentValueP = BmpV3GetField((BitmapType *)bitmapP, BitmapV3FieldTransparentValue);
          }
          r = true;
        }
        break;
    }
  }

  return r;
}

void BmpSetTransparentValue(BitmapType *bitmapP, UInt32 transparentValue) {
  Boolean hasTransparentValue;
  UInt16 colorTableSize;
  Boolean le;

  if (bitmapP) {
    bitmapP = skipEmptySlot(bitmapP);
    le = BmpLittleEndian(bitmapP);

    switch (BmpGetVersion(bitmapP)) {
      case 2:
        if (transparentValue == kTransparencyNone) {
          BmpSetCommonFlag(bitmapP, BitmapFlagHasTransparency, 0);
          BmpV2SetField(bitmapP, BitmapV2FieldTransparentIndex, 0);
        } else {
          hasTransparentValue = BmpGetCommonFlag((BitmapType *)bitmapP, BitmapFlagDirectColor);
          BmpSetCommonFlag(bitmapP, BitmapFlagHasTransparency, 1);
          if (hasTransparentValue) {
            colorTableSize =  BmpColortableSize(bitmapP);
            put4(transparentValue, (UInt8 *)bitmapP, BitmapV2FieldColorTable + colorTableSize + 4);
          } else {
            BmpV2SetField(bitmapP, BitmapV2FieldTransparentIndex, transparentValue);
          }
        }
        break;
      case 3:
        if (transparentValue == kTransparencyNone) {
          BmpSetCommonFlag(bitmapP, BitmapFlagHasTransparency, 0);
          BmpV3SetField(bitmapP, BitmapV3FieldTransparentValue, 0);
        } else {
          BmpSetCommonFlag(bitmapP, BitmapFlagHasTransparency, 1);
          BmpV3SetField(bitmapP, BitmapV3FieldTransparentValue, transparentValue);
        }
        break;
      default:
        debug(DEBUG_ERROR, "Bitmap", "BmpSetTransparentValue V%d index %d not supported", BmpGetVersion(bitmapP), transparentValue);
        break;
    }
  }
}

static UInt8 BmpRGBToIndex(UInt8 red, UInt8 green, UInt8 blue, ColorTableType *colorTable) {
  Int32 dr, dg, db;
  UInt16 numEntries;
  UInt32 i, d, dmin, imin;
  RGBColorType rgb;

  dmin = 0xffffffff;
  imin = 0;
  numEntries = CtbGetNumEntries(colorTable);

  for (i = 0; i < numEntries; i++) {
    CtbGetEntry(colorTable, i, &rgb);

    if (red == rgb.r && green == rgb.g && blue == rgb.b) {
      return i;
    }
    dr = (Int32)red - (Int32)rgb.r;
    dr = dr * dr;
    dg = (Int32)green - (Int32)rgb.g;
    dg = dg * dg;
    db = (Int32)blue - (Int32)rgb.b;
    db = db * db;
    d = dr + dg + db;
    if (d < dmin) {
      dmin = d;
      imin = i;
    }
  }

  return imin;
}

static void BmpIndexToRGB(UInt8 i, UInt8 *red, UInt8 *green, UInt8 *blue, ColorTableType *colorTable) {
  RGBColorType rgb;

  CtbGetEntry(colorTable, i, &rgb);
  *red   = rgb.r;
  *green = rgb.g;
  *blue  = rgb.b;
}

IndexedColorType BmpGetPixel(BitmapType *bitmapP, Coord x, Coord y) {
  UInt8 red, green, blue;
  UInt32 value;
  IndexedColorType b = 0;

  if (bitmapP) {
    switch (BmpGetBitDepth(bitmapP)) {
      case  1:
        b = BmpGetPixelValue(bitmapP, x, y);
        b = gray1[b];
        break;
      case  2:
        b = BmpGetPixelValue(bitmapP, x, y);
        b = gray2[b];
        break;
      case  4:
        b = BmpGetPixelValue(bitmapP, x, y);
        b = gray4[b];
        break;
      case  8:
        b = BmpGetPixelValue(bitmapP, x, y);
        break;
      case 16:
        value = BmpGetPixelValue(bitmapP, x, y);
        red   = r565(value);
        green = g565(value);
        blue  = b565(value);
        b = BmpRGBToIndex(red, green, blue, pumpkin_defaultcolorTable());
        break;
      case 24:
        value = BmpGetPixelValue(bitmapP, x, y);
        red   = r24(value);
        green = g24(value);
        blue  = b24(value);
        b = BmpRGBToIndex(red, green, blue, pumpkin_defaultcolorTable());
        break;
      case 32:
        value = BmpGetPixelValue(bitmapP, x, y);
        red   = r32(value);
        green = g32(value);
        blue  = b32(value);
        b = BmpRGBToIndex(red, green, blue, pumpkin_defaultcolorTable());
        break;
    }
  }

  return b;
}

UInt32 BmpGetPixelValue(BitmapType *bitmapP, Coord x, Coord y) {
  UInt8 b, *bits;
  UInt16 w, rowBytes;
  UInt32 offset;
  UInt32 value = 0;
  Boolean le, leBits;

  if (bitmapP) {
    le = BmpLittleEndian(bitmapP);
    leBits = BmpGetCommonFlag(bitmapP, BitmapFlagLittleEndian);
    bits = BmpGetBits(bitmapP);
    BmpGetDimensions(bitmapP, NULL, NULL, &rowBytes);

    switch (BmpGetBitDepth(bitmapP)) {
      case  1:
        offset = y * rowBytes + (x >> 3);
        b = bits[offset];
        value = (b >> (7 - (x & 0x07))) & 0x01;
        break;
      case  2:
        offset = y * rowBytes + (x >> 2);
        b = bits[offset];
        switch (x & 0x03) {
          case 0: value = (b >> 6) & 0x03; break;
          case 1: value = (b >> 4) & 0x03; break;
          case 2: value = (b >> 2) & 0x03; break;
          case 3: value =  b       & 0x03; break;
        }
        //value = (b >> (3 - (x & 0x03))) & 0x03;
        //value = (b >> ((x & 0x03) << 1)) & 0x03;
        break;
      case  4:
        offset = y * rowBytes + (x >> 1);
        b = bits[offset];
        value = !(x & 0x01) ? b >> 4 : b & 0x0F;
        break;
      case  8:
        offset = y * rowBytes + x;
        value = bits[offset];
        break;
      case 16:
        offset = y * rowBytes + x*2;
        get2_16(&w, bits, offset);
        value = w;
        break;
      case 24:
        offset = y * rowBytes + x*3;
        value = rgb24(bits[offset], bits[offset+1], bits[offset+2]);
        break;
      case 32:
        offset = y * rowBytes + x*4;
        get4b(&value, bits, offset);
        break;
    }
  }

  return value;
}

Err BmpGetPixelRGB(BitmapType *bitmapP, Coord x, Coord y, RGBColorType *rgbP) {
  ColorTableType *colorTable;
  UInt8 b, *bits;
  UInt16 w, rowBytes;
  UInt32 offset;
  Err err = sysErrParamErr;

  if (bitmapP && rgbP) {
    switch (BmpGetBitDepth(bitmapP)) {
      case  1:
        b = BmpGetPixelValue(bitmapP, x, y);
        rgbP->r = rgbP->g = rgbP->b = gray1values[b];
        break;
      case  2:
        b = BmpGetPixelValue(bitmapP, x, y);
        rgbP->r = rgbP->g = rgbP->b = gray2values[b];
        break;
      case  4:
        b = BmpGetPixelValue(bitmapP, x, y);
        rgbP->r = rgbP->g = rgbP->b = gray4values[b];
        break;
      case  8:
        b = BmpGetPixelValue(bitmapP, x, y);
        colorTable = BmpGetColortable(bitmapP);
        if (colorTable == NULL) colorTable = pumpkin_defaultcolorTable();
        BmpIndexToRGB(b, &rgbP->r, &rgbP->g, &rgbP->b, colorTable);
        break;
      case 16:
        w = BmpGetPixelValue(bitmapP, x, y);
        rgbP->r = r565(w);
        rgbP->g = g565(w);
        rgbP->b = b565(w);
        break;
      case 24:
        BmpGetDimensions(bitmapP, NULL, NULL, &rowBytes);
        bits = BmpGetBits(bitmapP);
        offset = y * rowBytes + x*3;
        rgbP->r = bits[offset];
        rgbP->g = bits[offset + 1];
        rgbP->b = bits[offset + 2];
        break;
      case 32:
        BmpGetDimensions(bitmapP, NULL, NULL, &rowBytes);
        bits = BmpGetBits(bitmapP);
        offset = y * rowBytes + x*4;
        rgbP->r = bits[offset + 1];
        rgbP->g = bits[offset + 2];
        rgbP->b = bits[offset + 3];
        break;
    }

    err = errNone;
  }

  return err;
}

void BmpDrawSurface(BitmapType *bitmapP, Coord sx, Coord sy, Coord w, Coord h, surface_t *surface, Coord x, Coord y, Boolean useTransp) {
  ColorTableType *colorTable;
  UInt32 offset, transparentValue, c;
  Int32 offsetb;
  UInt8 *bits, b, alpha, red, green, blue, gray;
  UInt16 depth, rowBytes, rgb;
  Coord width, height, i, j, k;
  Boolean le, leBits, transp;

  if (bitmapP && surface && w > 0 && h > 0) {
    le = BmpLittleEndian(bitmapP);
    leBits = BmpGetCommonFlag(bitmapP, BitmapFlagLittleEndian);
    BmpGetDimensions(bitmapP, &width, &height, &rowBytes);

  if (sx < width && sy < height) {
    bits = BmpGetBits(bitmapP);

    if (bits) {
      if (sx < 0) {
        w += sx;
        x -= sx;
        sx = 0;
      }
      if ((sx + w) >= width) {
        w = width - sx;
      }
      if (sy < 0) {
        h += sy;
        y -= sy;
        sy = 0;
      }
      if ((sy + h) >= height) {
        h = height - sy;
      }

      if (w > 0 && h > 0) {
        transp = BmpGetTransparentValue(bitmapP, &transparentValue);
        depth = BmpGetBitDepth(bitmapP);

        switch (depth) {
          case 1:
            offset = sy * rowBytes + sx / 8;
            for (i = 0; i < h; i++, offset += rowBytes) {
              offsetb = 7 - (sx % 8);
              k = 0;
              for (j = 0; j < w; j++) {
                b = bits[offset + k] & (1 << offsetb);
                gray = gray1values[b ? 1 : 0];
                c = surface_color_rgb(surface->encoding, surface->palette, surface->npalette, gray, gray, gray, 0xff);
                surface->setpixel(surface->data, x+j, y+i, c);
                offsetb--;
                if (offsetb == -1) {
                  offsetb = 7;
                  k++;
                }
              }
            }
            break;
          case 2:
            offset = sy * rowBytes + sx / 4;
            for (i = 0; i < h; i++, offset += rowBytes) {
              offsetb = (3 - (sx % 4)) << 1;
              k = 0;
              for (j = 0; j < w; j++) {
                b = (bits[offset + k] & (3 << offsetb)) >> offsetb;
                gray = gray2values[b];
                c = surface_color_rgb(surface->encoding, surface->palette, surface->npalette, gray, gray, gray, 0xff);
                surface->setpixel(surface->data, x+j, y+i, c);
                offsetb -= 2;
                if (offsetb == -2) {
                  offsetb = 6;
                  k++;
                }
              }
            }
            break;
          case 4:
            colorTable = BmpGetColortable(bitmapP);
            if (colorTable == NULL) colorTable = pumpkin_defaultcolorTable();
            offset = sy * rowBytes + sx / 2;
            for (i = 0; i < h; i++, offset += rowBytes) {
              offsetb = (sx % 2) << 2;
              k = 0;
              for (j = 0; j < w; j++) {
                b = (bits[offset + k] & (0xf << (4-offsetb))) >> (4-offsetb);
                BmpIndexToRGB(b, &red, &green, &blue, colorTable);
                c = surface_color_rgb(surface->encoding, surface->palette, surface->npalette, red, green, blue, 0xff);
                surface->setpixel(surface->data, x+j, y+i, c);
                offsetb += 4;
                if (offsetb == 8) {
                  offsetb = 0;
                  k++;
                }
              }
            }
            break;
          case 8:
            colorTable = BmpGetColortable(bitmapP);
            if (colorTable == NULL) colorTable = pumpkin_defaultcolorTable();
            offset = sy * rowBytes + sx;
            for (i = 0; i < h; i++, offset += rowBytes) {
              for (j = 0; j < w; j++) {
                b = bits[offset + j];
                if (!useTransp || !transp || b != transparentValue) {
                  BmpIndexToRGB(b, &red, &green, &blue, colorTable);
                  c = surface_color_rgb(surface->encoding, surface->palette, surface->npalette, red, green, blue, 0xff);
                  surface->setpixel(surface->data, x+j, y+i, c);
                }
              }
            }
            break;
          case 16:
            offset = sy * rowBytes + sx*2;
            // XXX transparentValue is 24-bits RGB, but pixel values are 16-bits 565, so convert transparentValue to 16-bits
            //transparentValue = rgb565(r32(transparentValue), g32(transparentValue), b32(transparentValue));
            // XXX is this correct? the logo bitmap is 16 bits and the white transparentValue is 0x0000FFFF, not 0x00FFFFFF
            for (i = 0; i < h; i++, offset += rowBytes) {
              for (j = 0, k = 0; j < w; j++, k += 2) {
                get2_16(&rgb, bits, offset + k);
                if (!useTransp || !transp || rgb != transparentValue) {
                  red   = r565(rgb);
                  green = g565(rgb);
                  blue  = b565(rgb);
                  c = surface_color_rgb(surface->encoding, surface->palette, surface->npalette, red, green, blue, 0xff);
                  surface->setpixel(surface->data, x+j, y+i, c);
                }
              }
            }
            break;
          case 24:
            offset = sy * rowBytes + sx*3;
            for (i = 0; i < h; i++, offset += rowBytes) {
              for (j = 0, k = 0; j < w; j++, k += 3) {
                red   = bits[offset + k];
                green = bits[offset + k + 1];
                blue  = bits[offset + k + 2];
                if (!useTransp || !transp || rgb24(red, green, blue) != transparentValue) {
                  c = surface_color_rgb(surface->encoding, surface->palette, surface->npalette, red, green, blue, 0xff);
                  surface->setpixel(surface->data, x+j, y+i, c);
                }
              }
            }
            break;
          case 32:
            offset = sy * rowBytes + sx*4;
            for (i = 0; i < h; i++, offset += rowBytes) {
              for (j = 0, k = 0; j < w; j++, k += 4) {
                alpha = bits[offset + k];
                red   = bits[offset + k + 1];
                green = bits[offset + k + 2];
                blue  = bits[offset + k + 3];
                c = surface_color_rgb(surface->encoding, surface->palette, surface->npalette, red, green, blue, useTransp ? alpha : 0xff);
                surface->setpixel(surface->data, x+j, y+i, c);
              }
            }
            break;
        }
      }
    }
  }
  }
}

UInt32 BmpConvertFrom1Bit(UInt32 b, UInt8 depth, ColorTableType *colorTable, Boolean isDefault) {
  switch (depth) {
    case 1: break;
    case 2: b = b ? 0x03 : 0x00; break;
    case 4: b = b ? 0x0F : 0x00; break;
    case 8:
      if (isDefault) {
        b = b ? 0xE6 : 0x00;
      } else {
        b = b ? BmpRGBToIndex(0x00, 0x00, 0x00, colorTable) : BmpRGBToIndex(0xFF, 0xFF, 0xFF, colorTable);
      }
      break;
    case 16:
      b = b ? 0x0000 : 0xFFFF;
      break;
    case 24:
      b = b ? 0x000000 : 0xFFFFFF;
      break;
    case 32:
      b = b ? 0xFF000000 : 0xFFFFFFFF;
      break;
  }

  return b;
}

/*
0x00: 0xff,0xff,0xff
0xe0: 0xee,0xee,0xee
0xdf: 0xdd,0xdd,0xdd
0x19: 0xcc,0xcc,0xcc
0xde: 0xbb,0xbb,0xbb
0xdd: 0xaa,0xaa,0xaa
0x32: 0x99,0x99,0x99
0xdc: 0x88,0x88,0x88
0xdb: 0x77,0x77,0x77
0xa5: 0x66,0x66,0x66
0xda: 0x55,0x55,0x55
0xd9: 0x44,0x44,0x44
0xbe: 0x33,0x33,0x33
0xd8: 0x22,0x22,0x22
0xd7: 0x11,0x11,0x11
0xe6: 0x00,0x00,0x00
*/

UInt32 BmpConvertFrom2Bits(UInt32 b, UInt8 depth, ColorTableType *colorTable, Boolean isDefault) {
  switch (depth) {
    case 1: b = b ? 1 : 0; break;
    case 2: break;
    case 4: b = b << 2; break;
    case 8:
      if (isDefault) {
        b = gray2[b];
      } else {
        b = BmpRGBToIndex(gray2values[b], gray2values[b], gray2values[b], colorTable);
      }
      break;
    case 16:
      b = rgb565(gray2values[b], gray2values[b], gray2values[b]);
      break;
    case 24:
      b = rgb24(gray2values[b], gray2values[b], gray2values[b]);
      break;
    case 32:
      b = rgba32(gray2values[b], gray2values[b], gray2values[b], 0xff);
      break;
  }

  return b;
}

UInt32 BmpConvertFrom4Bits(UInt32 b, UInt8 depth, ColorTableType *colorTable, Boolean isDefault) {
  switch (depth) {
    case 1: b = b ? 1 : 0; break;
    case 2: b = b >> 2; break;
    case 4: break;
    case 8:
      if (isDefault) {
        b = gray4[b];
      } else {
        b = BmpRGBToIndex(gray4values[b], gray4values[b], gray4values[b], colorTable);
      }
      break;
    case 16:
      b = rgb565(gray4values[b], gray4values[b], gray4values[b]);
      break;
    case 24:
      b = rgb24(gray4values[b], gray4values[b], gray4values[b]);
      break;
    case 32:
      b = rgba32(gray4values[b], gray4values[b], gray4values[b], 0xff);
      break;
  }

  return b;
}

static UInt8 rgbToGray1(UInt16 r, UInt16 g, UInt16 b) {
  return (r > 127 && g > 127 && b > 127) ? 0 : 1;
}

static UInt8 rgbToGray2(UInt16 r, UInt16 g, UInt16 b) {
  UInt16 c = (r + g + b) / 3;
  return 3 - (c >> 6);
}

static UInt8 rgbToGray4(UInt16 r, UInt16 g, UInt16 b) {
  UInt16 c = (r + g + b) / 3;
  return 15 - (c >> 4);
}

UInt32 BmpConvertFrom8Bits(UInt32 b, ColorTableType *srcColorTable, Boolean isSrcDefault, UInt8 depth, ColorTableType *dstColorTable, Boolean isDstDefault) {
  RGBColorType rgb;

  CtbGetEntry(srcColorTable, b, &rgb);

  switch (depth) {
    case  1: b = rgbToGray1(rgb.r, rgb.g, rgb.b); break;
    case  2: b = rgbToGray2(rgb.r, rgb.g, rgb.b); break;
    case  4: b = rgbToGray4(rgb.r, rgb.g, rgb.b); break;
    case  8: b = BmpRGBToIndex(rgb.r, rgb.g, rgb.b, dstColorTable); break;
    case 16: b = rgb565(rgb.r, rgb.g, rgb.b); break;
    case 24: b = rgb24(rgb.r, rgb.g, rgb.b); break;
    case 32: b = rgba32(rgb.r, rgb.g, rgb.b, 0xff); break;
  }

  return b;
}

UInt32 BmpConvertFrom16Bits(UInt32 b, UInt8 depth, ColorTableType *dstColorTable) {
  switch (depth) {
    case  1: b = rgbToGray1(r565(b), g565(b), b565(b)); break;
    case  2: b = rgbToGray2(r565(b), g565(b), b565(b)); break;
    case  4: b = rgbToGray4(r565(b), g565(b), b565(b)); break;
    case  8: b = BmpRGBToIndex(r565(b), g565(b), b565(b), dstColorTable); break;
    case 24: b = rgb24(r565(b), g565(b), b565(b)); break;
    case 32: b = rgba32(r565(b), g565(b), b565(b), 0xff); break;
  }

  return b;
}

UInt32 BmpConvertFrom24Bits(UInt32 b, UInt8 depth, ColorTableType *dstColorTable) {
  switch (depth) {
    case  1: b = rgbToGray1(r24(b), g24(b), b24(b)); break;
    case  2: b = rgbToGray2(r24(b), g24(b), b24(b)); break;
    case  4: b = rgbToGray4(r24(b), g24(b), b24(b)); break;
    case  8: b = BmpRGBToIndex(r24(b), g24(b), b24(b), dstColorTable); break;
    case 16: b = rgb565(r24(b), g24(b), b24(b)); break;
    case 32: b = rgba32(r24(b), g24(b), b24(b), 0xff); break;
  }

  return b;
}

static UInt32 BmpConvertFrom32Bits(UInt32 b, UInt8 depth, ColorTableType *dstColorTable) {
  switch (depth) {
    case  1: b = rgbToGray1(r32(b), g32(b), b32(b)); break;
    case  2: b = rgbToGray2(r32(b), g32(b), b32(b)); break;
    case  4: b = rgbToGray4(r32(b), g32(b), b32(b)); break;
    case  8: b = BmpRGBToIndex(r32(b), g32(b), b32(b), dstColorTable); break;
    case 16: b = rgb565(r32(b), g32(b), b32(b)); break;
    case 24: b = rgb24(r32(b), g32(b), b32(b)); break;
  }

  return b;
}

#define BmpSetBit1p(offset, mask, dataSize, b) \
  if (offset < dataSize) { \
    bits[offset] &= ~(mask); \
    bits[offset] |= (b); \
  }

#define BmpSetBit1(offset, mask, dataSize, b, dbl) \
  BmpSetBit1p(offset, mask, dataSize, b); \
  if (dbl) { \
    BmpSetBit1p(offset, mask>>1, dataSize, b>>1); \
    BmpSetBit1p(offset+rowBytes, mask, dataSize, b); \
    BmpSetBit1p(offset+rowBytes, mask>>1, dataSize, b>>1); \
  }

static void BmpCopyBit1(UInt8 b, Boolean transp, BitmapType *dst, Coord dx, Coord dy, WinDrawOperation mode, Boolean dbl) {
  UInt8 *bits, mask, old, fg, bg;
  UInt32 offset, shift, dataSize;
  UInt16 rowBytes;
  RGBColorType rgb;

  BmpGetDimensions(dst, NULL, NULL, &rowBytes);
  BmpGetSizes(dst, &dataSize, NULL);
  bits = BmpGetBits(dst);
  offset = dy * rowBytes + (dx >> 3);
  shift = 7 - (dx & 0x07);
  b = b << shift;
  mask = (1 << shift);

  switch (mode) {
    case winPaint:        // write color-matched source pixels to the destination
                          // If a bitmap’s hasTransparency flag is set, winPaint behaves like winOverlay instead.
      if (!transp) {
        BmpSetBit1(offset, mask, dataSize, b, dbl);
      }
      break;
    case winErase:        // write backColor if the source pixel is transparent
      if (transp) {
        WinSetBackColorRGB(NULL, &rgb);
        b = ((rgb.r == 0 && rgb.g == 0 && rgb.b == 0) ? 0 : 1) << shift;
        BmpSetBit1(offset, mask, dataSize, b, dbl);
      }
      break;
    case winMask:         // write backColor if the source pixel is not transparent
      if (!transp) {
        WinSetBackColorRGB(NULL, &rgb);
        b = ((rgb.r == 0 && rgb.g == 0 && rgb.b == 0) ? 0 : 1) << shift;
        BmpSetBit1(offset, mask, dataSize, b, dbl);
      }
      break;
    case winInvert:       // bitwise XOR the color-matched source pixel onto the destination (this mode does not honor the transparent color in any way)
      old = bits[offset] & mask;
      BmpSetBit1(offset, mask, dataSize, (old ^ b), dbl);
      break;
    case winOverlay:      // write color-matched source pixel to the destination if the source pixel is not transparent. Transparent pixels are skipped.
                          // For a 1-bit display, the "off" bits are considered to be the transparent color
      if (!transp) {
        BmpSetBit1(offset, mask, dataSize, b, dbl);
      }
      break;
    case winPaintInverse: // invert the source pixel color and then proceed as with winPaint
      BmpSetBit1(offset, mask, dataSize, b ^ mask, dbl);
      break;
    case winSwap:         // Swap the backColor and foreColor destination colors if the source is a pattern (the type of pattern is disregarded).
                          // If the source is a bitmap, then the bitmap is transferred using winPaint mode instead.
      b = bits[offset] & mask ? 1 : 0;
      bg = WinGetBackColor() ? 1 : 0;
      fg = WinGetForeColor()? 1 : 0;
      if (b == bg) {
        BmpSetBit1(offset, mask, dataSize, fg << shift, dbl);
      } else if (b == fg) {
        BmpSetBit1(offset, mask, dataSize, bg << shift, dbl);
      }
      break;
  }
}

#define BmpSetBit2p(offset, mask, dataSize, b) \
  if (offset < dataSize) { \
    bits[offset] &= ~(mask); \
    bits[offset] |= (b); \
  }

#define BmpSetBit2(offset, mask, dataSize, b, dbl) \
  BmpSetBit2p(offset, mask, dataSize, b); \
  if (dbl) { \
    BmpSetBit2p(offset, mask>>2, dataSize, b>>2); \
    BmpSetBit2p(offset+rowBytes, mask, dataSize, b); \
    BmpSetBit2p(offset+rowBytes, mask>>2, dataSize, b>>2); \
  }

static void BmpCopyBit2(UInt8 b, Boolean transp, BitmapType *dst, Coord dx, Coord dy, WinDrawOperation mode, Boolean dbl) {
  UInt8 *bits, mask, old, fg, bg;
  UInt32 offset, shift, dataSize;
  UInt16 rowBytes;

  BmpGetDimensions(dst, NULL, NULL, &rowBytes);
  BmpGetSizes(dst, &dataSize, NULL);
  bits = BmpGetBits(dst);
  offset = dy * rowBytes + (dx >> 2);
  shift = (3 - (dx & 0x03)) << 1;
  b = b << shift;
  mask = (0x03 << shift);

  switch (mode) {
    case winPaint:        // write color-matched source pixels to the destination
                          // If a bitmap’s hasTransparency flag is set, winPaint behaves like winOverlay instead.
      if (!transp) {
        BmpSetBit2(offset, mask, dataSize, b, dbl);
      }
      break;
    case winErase:        // write backColor if the source pixel is transparent
      if (transp) {
        bg = WinGetBackColor();
        BmpSetBit2(offset, mask, dataSize, bg, dbl);
      }
      break;
    case winMask:         // write backColor if the source pixel is not transparent
      if (!transp) {
        bg = WinGetBackColor();
        BmpSetBit2(offset, mask, dataSize, bg, dbl);
      }
      break;
    case winInvert:       // bitwise XOR the color-matched source pixel onto the destination (this mode does not honor the transparent color in any way)
      old = bits[offset] & mask;
      BmpSetBit2(offset, mask, dataSize, (old ^ b), dbl);
      break;
    case winOverlay:      // write color-matched source pixel to the destination if the source pixel is not transparent. Transparent pixels are skipped.
      if (!transp) {
        BmpSetBit2(offset, mask, dataSize, b, dbl);
      }
      break;
    case winPaintInverse: // invert the source pixel color and then proceed as with winPaint
      BmpSetBit2(offset, mask, dataSize, b ^ mask, dbl);
      break;
    case winSwap:         // Swap the backColor and foreColor destination colors if the source is a pattern (the type of pattern is disregarded).
                          // If the source is a bitmap, then the bitmap is transferred using winPaint mode instead.
      b = (bits[offset] & mask) >> shift;
      bg = WinGetBackColor();
      fg = WinGetForeColor();
      if (b == bg) b = fg;
      else if (b == fg) b = bg;
      BmpSetBit2(offset, mask, dataSize, b << shift, dbl);
      break;
  }
}

#define BmpSetBit4p(offset, mask, dataSize, b) \
  if (offset < dataSize) { \
    bits[offset] &= ~(mask); \
    bits[offset] |= (b); \
  }

#define BmpSetBit4(offset, mask, dataSize, b, dbl) \
  BmpSetBit4p(offset, mask, dataSize, b); \
  if (dbl) { \
    BmpSetBit4p(offset, mask>>4, dataSize, b>>4); \
    BmpSetBit4p(offset+rowBytes, mask, dataSize, b); \
    BmpSetBit4p(offset+rowBytes, mask>>4, dataSize, b>>4); \
  }

static void BmpCopyBit4(UInt8 b, Boolean transp, BitmapType *dst, Coord dx, Coord dy, WinDrawOperation mode, Boolean dbl) {
  UInt8 *bits, mask, old, fg, bg;
  UInt32 offset, shift, dataSize;
  UInt16 rowBytes;

  BmpGetDimensions(dst, NULL, NULL, &rowBytes);
  BmpGetSizes(dst, &dataSize, NULL);
  bits = BmpGetBits(dst);
  offset = dy * rowBytes + (dx >> 1);
  shift = (dx & 0x01) ? 0 : 4;
  b = b << shift;
  mask = (0x0F << shift);

  switch (mode) {
    case winPaint:        // write color-matched source pixels to the destination
                          // If a bitmap's hasTransparency flag is set, winPaint behaves like winOverlay instead.
      if (!transp) {
        BmpSetBit4(offset, mask, dataSize, b, dbl);
      }
      break;
    case winErase:        // write backColor if the source pixel is transparent
      if (transp) {
        bg = WinGetBackColor();
        BmpSetBit4(offset, mask, dataSize, bg, dbl);
      }
      break;
    case winMask:         // write backColor if the source pixel is not transparent
      if (!transp) {
        bg = WinGetBackColor();
        BmpSetBit4(offset, mask, dataSize, bg, dbl);
      }
      break;
    case winInvert:       // bitwise XOR the color-matched source pixel onto the destination (this mode does not honor the transparent color in any way)
      old = bits[offset] & mask;
      BmpSetBit4(offset, mask, dataSize, (old ^ b), dbl);
      break;
    case winOverlay:      // write color-matched source pixel to the destination if the source pixel is not transparent. Transparent pixels are skipped.
      if (!transp) {
        BmpSetBit4(offset, mask, dataSize, b, dbl);
      }
      break;
    case winPaintInverse: // invert the source pixel color and then proceed as with winPaint
      BmpSetBit4(offset, mask, dataSize, b ^ mask, dbl);
      break;
    case winSwap:         // Swap the backColor and foreColor destination colors if the source is a pattern (the type of pattern is disregarded).
                          // If the source is a bitmap, then the bitmap is transferred using winPaint mode instead.
      b = (bits[offset] & mask) >> shift;
      bg = WinGetBackColor();
      fg = WinGetForeColor();
      if (b == bg) b = fg;
      else if (b == fg) b = bg;
      BmpSetBit4(offset, mask, dataSize, b << shift, dbl);
      break;
  }
}

#define BmpSetBit8p(offset, dataSize, b) if (offset < dataSize) bits[offset] = b

#define BmpSetBit8(offset, dataSize, b, dbl) \
  BmpSetBit8p(offset, dataSize, b); \
  if (dbl) { \
    BmpSetBit8p(offset+1, dataSize, b); \
    BmpSetBit8p(offset+rowBytes, dataSize, b); \
    BmpSetBit8p(offset+rowBytes+1, dataSize, b); \
  }

static void BmpCopyBit8(UInt8 b, Boolean transp, BitmapType *dst, ColorTableType *colorTable, Coord dx, Coord dy, WinDrawOperation mode, Boolean dbl) {
  UInt8 *bits, old, r1, g1, b1, r2, g2, b2, fg, bg;
  UInt32 offset, dataSize;
  UInt16 rowBytes;

  BmpGetDimensions(dst, NULL, NULL, &rowBytes);
  BmpGetSizes(dst, &dataSize, NULL);
  bits = BmpGetBits(dst);
  offset = dy * rowBytes + dx;

  switch (mode) {
    case winPaint:        // write color-matched source pixels to the destination
                          // If a bitmap’s hasTransparency flag is set, winPaint behaves like winOverlay instead.
      if (!transp) {
        BmpSetBit8(offset, dataSize, b, dbl);
      }
      break;
    case winErase:        // write backColor if the source pixel is transparent
      if (transp) {
        bg = WinGetBackColor();
        BmpSetBit8(offset, dataSize, bg, dbl);
      }
      break;
    case winMask:         // write backColor if the source pixel is not transparent
      if (!transp) {
        bg = WinGetBackColor();
        BmpSetBit8(offset, dataSize, bg, dbl);
      }
      break;
    case winInvert:       // bitwise XOR the color-matched source pixel onto the destination (this mode does not honor the transparent color in any way)
      old = bits[offset];
      BmpIndexToRGB(old, &r1, &g1, &b1, colorTable);
      BmpIndexToRGB(b, &r2, &g2, &b2, colorTable);
      r1 ^= r2;
      g1 ^= g2;
      b1 ^= b2;
      b = BmpRGBToIndex(r1, g1, b1, colorTable);
      BmpSetBit8(offset, dataSize, b, dbl);
      break;
    case winOverlay:      // write color-matched source pixel to the destination if the source pixel is not transparent. Transparent pixels are skipped.
      if (!transp) {
        BmpSetBit8(offset, dataSize, b, dbl);
      }
      break;
    case winPaintInverse: // invert the source pixel color and then proceed as with winPaint
      if (!transp) {
        BmpIndexToRGB(b, &r1, &g1, &b1, colorTable);
        r1 ^= 0xff;
        g1 ^= 0xff;
        b1 ^= 0xff;
        b = BmpRGBToIndex(r1, g1, b1, colorTable);
        BmpSetBit8(offset, dataSize, b, dbl);
      }
      break;
    case winSwap:         // Swap the backColor and foreColor destination colors if the source is a pattern (the type of pattern is disregarded).
                          // If the source is a bitmap, then the bitmap is transferred using winPaint mode instead.
      b = bits[offset];
      bg = WinGetBackColor();
      fg = WinGetForeColor();
      if (b == bg) b = fg;
      else if (b == fg) b = bg;
      BmpSetBit8(offset, dataSize, b, dbl);
      break;
  }
}

#define BmpSetBit16p(offset, dataSize, b) if (offset+1 < dataSize) put2_16(b, bits, offset)

#define BmpSetBit16(offset, dataSize, b, dbl) \
  BmpSetBit16p(offset, dataSize, b); \
  if (dbl) { \
    BmpSetBit16p(offset+2, dataSize, b); \
    BmpSetBit16p(offset+rowBytes, dataSize, b); \
    BmpSetBit16p(offset+rowBytes+2, dataSize, b); \
  }

static void BmpCopyBit16(UInt16 b, Boolean transp, BitmapType *dst, Coord dx, Coord dy, WinDrawOperation mode, Boolean dbl) {
  RGBColorType rgb, aux;
  UInt8 *bits;
  UInt16 rowBytes, old, fg, bg;
  UInt32 offset, dataSize;
  Boolean le, leBits;

  le = BmpLittleEndian(dst);
  leBits = BmpGetCommonFlag(dst, BitmapFlagLittleEndian);
  BmpGetDimensions(dst, NULL, NULL, &rowBytes);
  BmpGetSizes(dst, &dataSize, NULL);
  bits = BmpGetBits(dst);
  offset = dy * rowBytes + dx*2;

  switch (mode) {
    case winPaint:        // write color-matched source pixels to the destination
      if (!transp) {
        BmpSetBit16(offset, dataSize, b, dbl);
      }
      break;
    case winErase:        // write backColor if the source pixel is transparent
      if (transp) {
        WinSetBackColorRGB(NULL, &rgb);
        BmpSetBit16(offset, dataSize, rgb565(rgb.r, rgb.g, rgb.b), dbl);
      }
      break;
    case winMask:         // write backColor if the source pixel is not transparent
      if (!transp) {
        WinSetBackColorRGB(NULL, &rgb);
        BmpSetBit16(offset, dataSize, rgb565(rgb.r, rgb.g, rgb.b), dbl);
      }
      break;
    case winInvert:       // bitwise XOR the color-matched source pixel onto the destination (this mode does not honor the transparent color in any way)
      get2_16(&old, bits, offset);
      rgb.r = r565(b);
      rgb.g = g565(b);
      rgb.b = b565(b);
      rgb.r ^= r565(old);
      rgb.g ^= g565(old);
      rgb.b ^= b565(old);
      b = rgb565(rgb.r, rgb.g, rgb.b);
      BmpSetBit16(offset, dataSize, b, dbl);
      break;
    case winOverlay:      // write color-matched source pixel to the destination if the source pixel is not transparent. Transparent pixels are skipped.
      if (!transp) {
        BmpSetBit16(offset, dataSize, b, dbl);
      }
      break;
    case winPaintInverse: // invert the source pixel color and then proceed as with winPaint
      if (!transp) {
        rgb.r = r565(b);
        rgb.g = g565(b);
        rgb.b = b565(b);
        rgb.r ^= 0xff;
        rgb.g ^= 0xff;
        rgb.b ^= 0xff;
        BmpSetBit16(offset, dataSize, rgb565(rgb.r, rgb.g, rgb.b), dbl);
      }
      break;
    case winSwap:         // Swap the backColor and foreColor destination colors if the source is a pattern (the type of pattern is disregarded).
                          // If the source is a bitmap, then the bitmap is transferred using winPaint mode instead.
      get2_16(&old, bits, offset);
      WinSetBackColorRGB(NULL, &rgb);
      WinSetForeColorRGB(NULL, &aux);
      bg = rgb565(rgb.r, rgb.g, rgb.b);
      fg = rgb565(aux.r, aux.g, aux.b);
      if (old == bg) {
        BmpSetBit16(offset, dataSize, fg, dbl);
      } else if (old == fg) {
        BmpSetBit16(offset, dataSize, bg, dbl);
      }
      break;
    default:
      break;
  }
}

#define BmpSetBit24p(offset, dataSize, b) if (offset+2 < dataSize) { \
    bits[offset] = r24(b); \
    bits[offset+1] = g24(b); \
    bits[offset+2] = b24(b); \
  }

#define BmpSetBit24(offset, dataSize, b, dbl) \
  BmpSetBit24p(offset, dataSize, b); \
  if (dbl) { \
    BmpSetBit24p(offset+3, dataSize, b); \
    BmpSetBit24p(offset+rowBytes, dataSize, b); \
    BmpSetBit24p(offset+rowBytes+3, dataSize, b); \
  }

static void BmpCopyBit24(UInt32 b, Boolean transp, BitmapType *dst, Coord dx, Coord dy, WinDrawOperation mode, Boolean dbl) {
  RGBColorType rgb, aux;
  UInt8 *bits;
  UInt16 rowBytes;
  UInt32 old, fg, bg;
  UInt32 offset, dataSize;

  BmpGetDimensions(dst, NULL, NULL, &rowBytes);
  BmpGetSizes(dst, &dataSize, NULL);
  bits = BmpGetBits(dst);
  offset = dy * rowBytes + dx*3;

  switch (mode) {
    case winPaint:        // write color-matched source pixels to the destination
      if (!transp) {
        BmpSetBit24(offset, dataSize, b, dbl);
      }
      break;
    case winErase:        // write backColor if the source pixel is transparent
      if (transp) {
        WinSetBackColorRGB(NULL, &rgb);
        BmpSetBit24(offset, dataSize, rgb24(rgb.r, rgb.g, rgb.b), dbl);
      }
      break;
    case winMask:         // write backColor if the source pixel is not transparent
      if (!transp) {
        WinSetBackColorRGB(NULL, &rgb);
        BmpSetBit24(offset, dataSize, rgb24(rgb.r, rgb.g, rgb.b), dbl);
      }
      break;
    case winInvert:       // bitwise XOR the color-matched source pixel onto the destination (this mode does not honor the transparent color in any way)
      rgb.r = r24(b);
      rgb.g = g24(b);
      rgb.b = b24(b);
      rgb.r ^= bits[offset];
      rgb.g ^= bits[offset+1];
      rgb.b ^= bits[offset+1];
      b = rgb24(rgb.r, rgb.g, rgb.b);
      BmpSetBit24(offset, dataSize, b, dbl);
      break;
    case winOverlay:      // write color-matched source pixel to the destination if the source pixel is not transparent. Transparent pixels are skipped.
      if (!transp) {
        BmpSetBit24(offset, dataSize, b, dbl);
      }
      break;
    case winPaintInverse: // invert the source pixel color and then proceed as with winPaint
      if (!transp) {
        rgb.r = r24(b);
        rgb.g = g24(b);
        rgb.b = b24(b);
        rgb.r ^= 0xff;
        rgb.g ^= 0xff;
        rgb.b ^= 0xff;
        BmpSetBit24(offset, dataSize, rgb24(rgb.r, rgb.g, rgb.b), dbl);
      }
      break;
    case winSwap:         // Swap the backColor and foreColor destination colors if the source is a pattern (the type of pattern is disregarded).
                          // If the source is a bitmap, then the bitmap is transferred using winPaint mode instead.
      old = rgb24(bits[offset], bits[offset+1], bits[offset+2]);
      WinSetBackColorRGB(NULL, &rgb);
      WinSetForeColorRGB(NULL, &aux);
      bg = rgb24(rgb.r, rgb.g, rgb.b);
      fg = rgb24(aux.r, aux.g, aux.b);
      if (old == bg) {
        BmpSetBit24(offset, dataSize, fg, dbl);
      } else if (old == fg) {
        BmpSetBit24(offset, dataSize, bg, dbl);
      }
      break;
    default:
      break;
  }
}

#define BmpSetBit32p(offset, dataSize, b) if (offset+3 < dataSize) put4b(b, bits, offset)

#define BmpSetBit32(offset, dataSize, b, dbl) \
  BmpSetBit32p(offset, dataSize, b); \
  if (dbl) { \
    BmpSetBit32p(offset+4, dataSize, b); \
    BmpSetBit32p(offset+rowBytes, dataSize, b); \
    BmpSetBit32p(offset+rowBytes+4, dataSize, b); \
  }

static void BmpCopyBit32(UInt32 b, Boolean transp, BitmapType *dst, Coord dx, Coord dy, WinDrawOperation mode, Boolean dbl) {
  RGBColorType rgb, aux;
  UInt8 *bits;
  UInt16 rowBytes;
  UInt32 old, fg, bg;
  UInt32 offset, dataSize;

  BmpGetDimensions(dst, NULL, NULL, &rowBytes);
  BmpGetSizes(dst, &dataSize, NULL);
  bits = BmpGetBits(dst);
  offset = dy * rowBytes + dx*4;

  switch (mode) {
    case winPaint:        // write color-matched source pixels to the destination
      if (!transp) {
        BmpSetBit32(offset, dataSize, b, dbl);
      }
      break;
    case winErase:        // write backColor if the source pixel is transparent
      if (transp) {
        WinSetBackColorRGB(NULL, &rgb);
        BmpSetBit32(offset, dataSize, rgb32(rgb.r, rgb.g, rgb.b), dbl);
      }
      break;
    case winMask:         // write backColor if the source pixel is not transparent
      if (!transp) {
        WinSetBackColorRGB(NULL, &rgb);
        BmpSetBit32(offset, dataSize, rgb32(rgb.r, rgb.g, rgb.b), dbl);
      }
      break;
    case winInvert:       // bitwise XOR the color-matched source pixel onto the destination (this mode does not honor the transparent color in any way)
      rgb.r = r32(b);
      rgb.g = g32(b);
      rgb.b = b32(b);
      rgb.r ^= bits[offset+1];
      rgb.g ^= bits[offset+2];
      rgb.b ^= bits[offset+3];
      b = rgb32(rgb.r, rgb.g, rgb.b);
      BmpSetBit32(offset, dataSize, b, dbl);
      break;
    case winOverlay:      // write color-matched source pixel to the destination if the source pixel is not transparent. Transparent pixels are skipped.
      if (!transp) {
        BmpSetBit32(offset, dataSize, b, dbl);
      }
      break;
    case winPaintInverse: // invert the source pixel color and then proceed as with winPaint
      if (!transp) {
        rgb.r = r32(b);
        rgb.g = g32(b);
        rgb.b = b32(b);
        rgb.r ^= 0xff;
        rgb.g ^= 0xff;
        rgb.b ^= 0xff;
        BmpSetBit32(offset, dataSize, rgb24(rgb.r, rgb.g, rgb.b), dbl);
      }
      break;
    case winSwap:         // Swap the backColor and foreColor destination colors if the source is a pattern (the type of pattern is disregarded).
                          // If the source is a bitmap, then the bitmap is transferred using winPaint mode instead.
      old = rgb32(bits[offset], bits[offset+1], bits[offset+2]);
      WinSetBackColorRGB(NULL, &rgb);
      WinSetForeColorRGB(NULL, &aux);
      bg = rgb32(rgb.r, rgb.g, rgb.b);
      fg = rgb32(aux.r, aux.g, aux.b);
      if (old == bg) {
        BmpSetBit32(offset, dataSize, fg, dbl);
      } else if (old == fg) {
        BmpSetBit32(offset, dataSize, bg, dbl);
      }
      break;
    default:
      break;
  }
}

void BmpPutBit(UInt32 b, Boolean transp, BitmapType *dst, Coord dx, Coord dy, WinDrawOperation mode, Boolean dbl) {
  ColorTableType *dstColorTable, *colorTable;
  Coord width, height;
  UInt8 dstDepth;

  BmpGetDimensions(dst, &width, &height, NULL);

  if (dst && dx >= 0 && dx < width && dy >= 0 && dy < height) {
    colorTable = pumpkin_defaultcolorTable();
    dstColorTable = BmpGetColortable(dst);
    if (dstColorTable == NULL) {
      dstColorTable = colorTable;
    }

    dstDepth = BmpGetBitDepth(dst);

    switch (dstDepth) {
        case 1:
          BmpCopyBit1(b, transp, dst, dx, dy, mode, dbl);
          break;
        case 2:
          BmpCopyBit2(b, transp, dst, dx, dy, mode, dbl);
          break;
        case 4:
          BmpCopyBit4(b, transp, dst, dx, dy, mode, dbl);
          break;
        case 8:
          BmpCopyBit8(b, transp, dst, dstColorTable, dx, dy, mode, dbl);
          break;
        case 16:
          BmpCopyBit16(b, transp, dst, dx, dy, mode, dbl);
          break;
        case 24:
          BmpCopyBit24(b, transp, dst, dx, dy, mode, dbl);
          break;
        case 32:
          BmpCopyBit32(b, transp, dst, dx, dy, mode, dbl);
          break;
    }
  }
}

void BmpSetPixel(BitmapType *bitmapP, Coord x, Coord y, UInt32 value) {
  BmpPutBit(value, false, bitmapP, x, y, winPaint, false);
}

void BmpCopyBit(BitmapType *src, Coord sx, Coord sy, BitmapType *dst, Coord dx, Coord dy, WinDrawOperation mode, Boolean dbl, Boolean text, UInt32 tc, UInt32 bc) {
  ColorTableType *srcColorTable, *dstColorTable, *colorTable;
  UInt32 srcAlpha, srcRed, srcGreen, srcBlue;
  UInt32 dstRed, dstGreen, dstBlue;
  UInt8 srcDepth, dstDepth, *bits;
  UInt32 srcPixel, dstPixel, offset;
  UInt32 srcTransparentValue, dstTransparentValue;
  UInt16 srcRowBytes, dstRowBytes, aux;
  Coord srcWidth, srcHeight, dstWidth, dstHeight;
  Boolean le, leBits, srcTransp, dstTransp, isSrcDefault, isDstDefault;

  BmpGetDimensions(src, &srcWidth, &srcHeight, &srcRowBytes);
  BmpGetDimensions(dst, &dstWidth, &dstHeight, &dstRowBytes);

  if (src && dst && sx >= 0 && sx < srcWidth && sy >= 0 && sy < srcHeight && dx >= 0 && dx < dstWidth && dy >= 0 && dy < dstHeight) {
    colorTable = pumpkin_defaultcolorTable();

    srcColorTable = BmpGetColortable(src);
    if (srcColorTable == NULL) {
      srcColorTable = colorTable;
    }
    isSrcDefault = srcColorTable == colorTable;

    dstColorTable = BmpGetColortable(dst);
    if (dstColorTable == NULL) {
      dstColorTable = colorTable;
    }
    isDstDefault = dstColorTable == colorTable;

    srcTransp = BmpGetTransparentValue(src, &srcTransparentValue);
    srcDepth = BmpGetBitDepth(src);
    dstDepth = BmpGetBitDepth(dst);
    bits = BmpGetBits(src);
    le = BmpLittleEndian(src);
    leBits = BmpGetCommonFlag(src, BitmapFlagLittleEndian);

    switch (srcDepth) {
      case 1:
        srcPixel = bits[sy * srcRowBytes + (sx >> 3)];
        srcPixel = (srcPixel >> (7 - (sx & 0x07))) & 1;
        dstPixel = (dstDepth == 1) ? srcPixel : BmpConvertFrom1Bit(srcPixel, dstDepth, dstColorTable, isDstDefault);
        break;
      case 2:
        srcPixel = bits[sy * srcRowBytes + (sx >> 2)];
        srcPixel = (srcPixel >> ((3 - (sx & 0x03)) << 1)) & 0x03;
        dstPixel = (dstDepth == 2) ? srcPixel : BmpConvertFrom2Bits(srcPixel, dstDepth, dstColorTable, isDstDefault);
        break;
      case 4:
        srcPixel = bits[sy * srcRowBytes + (sx >> 1)];
        srcPixel = !(sx & 0x01) ? srcPixel >> 4 : srcPixel & 0x0F;
        dstPixel = (dstDepth == 4) ? srcPixel : BmpConvertFrom4Bits(srcPixel, dstDepth, dstColorTable, isDstDefault);
        break;
      case 8:
        srcPixel = bits[sy * srcRowBytes + sx];
        if (dstDepth != 8 || !isSrcDefault || !isDstDefault) {
          dstPixel = BmpConvertFrom8Bits(srcPixel, srcColorTable, isSrcDefault, dstDepth, dstColorTable, isDstDefault);
        } else {
          dstPixel = srcPixel;
        }
        break;
      case 16:
        get2_16(&aux, bits, sy * srcRowBytes + sx*2);
        srcPixel = aux;
        dstPixel = (dstDepth == 16) ? srcPixel : BmpConvertFrom16Bits(srcPixel, dstDepth, dstColorTable);
        break;
      case 24:
        offset = sy * srcRowBytes + sx*3;
        srcPixel = rgb24(bits[offset], bits[offset+1], bits[offset+2]);
        dstPixel = (dstDepth == 24) ? srcPixel : BmpConvertFrom24Bits(srcPixel, dstDepth, dstColorTable);
        break;
      case 32:
        get4b(&srcPixel, bits, sy * srcRowBytes + sx*4);
        srcAlpha = a32(srcPixel);
        if (srcAlpha == 255) {
          srcTransp = false;
          dstPixel = (dstDepth == 32) ? srcPixel : BmpConvertFrom32Bits(srcPixel, dstDepth, dstColorTable);
        } else if (srcAlpha == 0) {
          srcTransp = true;
          dstPixel = BmpGetPixelValue(dst, dx, dy);
        } else if (dstDepth >= 16) {
          srcTransp = false;
          srcRed   = r32(srcPixel);
          srcGreen = g32(srcPixel);
          srcBlue  = b32(srcPixel);
          dstPixel = BmpGetPixelValue(dst, dx, dy);
          switch (dstDepth) {
            case 16:
              dstRed   = r565(dstPixel);
              dstGreen = g565(dstPixel);
              dstBlue  = b565(dstPixel);
              break;
            case 24:
              dstRed   = r24(dstPixel);
              dstGreen = g24(dstPixel);
              dstBlue  = b24(dstPixel);
              break;
            case 32:
              dstRed   = r32(dstPixel);
              dstGreen = g32(dstPixel);
              dstBlue  = b32(dstPixel);
              break;
            default:
              dstRed   = 0;
              dstGreen = 0;
              dstBlue  = 0;
              break;
          }
          dstRed   = ((srcRed   * srcAlpha) + (dstRed   * (255 - srcAlpha))) / 255;
          dstGreen = ((srcGreen * srcAlpha) + (dstGreen * (255 - srcAlpha))) / 255;
          dstBlue  = ((srcBlue  * srcAlpha) + (dstBlue  * (255 - srcAlpha))) / 255;
          switch (dstDepth) {
            case 16:
              dstPixel = rgb565(dstRed, dstGreen, dstBlue);
              break;
            case 24:
              dstPixel = rgb24(dstRed, dstGreen, dstBlue);
              break;
            case 32:
              dstPixel = rgba32(dstRed, dstGreen, dstBlue, a32(dstPixel));
              break;
          }
        } else {
          srcTransp = true;
          dstPixel = BmpGetPixelValue(dst, dx, dy);
        }
        break;
      default:
        dstPixel = 0;
        break;
    }

    if (srcDepth != 32) {
      if (srcTransp) {
        srcTransp = (srcPixel == srcTransparentValue);
      } else if (mode == winMask || mode == winOverlay) {
        // source bitmap is not transparent but mode is winMask or winOverlay
        // use the transparent color anyway
        srcTransp = (srcPixel == srcTransparentValue);
      }
    }

    if (text) {
      dstPixel = srcPixel ? tc : bc;
      if (mode == winPaint) srcTransp = false;
    } else {
      dstTransp = BmpGetTransparentValue(dst, &dstTransparentValue);
      if (srcTransp && dstTransp && srcPixel == srcTransparentValue) {
        dstPixel = dstTransparentValue;
        srcTransp = false;
      }
    }
    bits = BmpGetBits(dst);

    switch (dstDepth) {
      case 1:
        BmpCopyBit1(dstPixel, srcTransp, dst, dx, dy, mode, dbl);
        break;
      case 2:
        BmpCopyBit2(dstPixel, srcTransp, dst, dx, dy, mode, dbl);
        break;
      case 4:
        BmpCopyBit4(dstPixel, srcTransp, dst, dx, dy, mode, dbl);
        break;
      case 8:
        BmpCopyBit8(dstPixel, srcTransp, dst, dstColorTable, dx, dy, mode, dbl);
        break;
      case 16:
        BmpCopyBit16(dstPixel, srcTransp, dst, dx, dy, mode, dbl);
        break;
      case 24:
        BmpCopyBit24(dstPixel, srcTransp, dst, dx, dy, mode, dbl);
        break;
      case 32:
        BmpCopyBit32(dstPixel, srcTransp, dst, dx, dy, mode, dbl);
        break;
    }
  }
}

/*
Compression:
BitmapCompressionTypeScanLine : Use scan line compression. Scan line compression is compatible with Palm OS 2.0 and higher.
BitmapCompressionTypeRLE      : Use RLE compression. RLE compression is supported in Palm OS 3.5 only.

Exemplo de bitmap V2 com BitmapCompressionTypeRLE:
00 13 00 10 00 14 a0 00 08 02 00 00 00 01 00 00

00 9e : number of bytes following

row 0:
05 00
04 5f
0b 00

row 1:
05 00
04 5f
0b 00

row 2:
05 00
04 6b
0b 00

row 3:
02 00
03 6b
04 5f
06 6b
05 00

02 1b
0e 5f
04 00

02 1b
02 5f
01 6b
01 5b
01 6b
01 5b
04 6b
01 5b
04 5f
03 00

02 1b
02 5f
09 6b
02 5b
03 5f
02 00

02 1b
02 5f
0b 6b
01 5b
03 5f
01 00

02 1b
02 5f
0b 6b
01 5b
03 5f
01 00

02 1b
02 5f
09 6b
02 5b
03 5f
02 00

02 1b
02 5f
01 6b
01 5b
01 6b
01 5b
04 6b
01 5b
04 5f
03 00

02 1b
0e 5f
04 00

02 00
03 6b
04 5f
06 6b
05 00

05 00
04 6b
0b 00

05 00
04 5f
0b 00

05 00
04 5f
0b 00

00 00 : end marker
*/

static int decompress_bitmap_rle(uint8_t *p, uint8_t *dp, uint32_t dsize) {
  uint8_t len, b;
  int i, j, k;

  debug(DEBUG_TRACE, "Bitmap", "RLE bitmap decompressing");

  for (i = 0, j = 0;;) {
    i += get1(&len, p, i);
    i += get1(&b, p, i);
    debug(DEBUG_TRACE, "Bitmap", "RLE len %d code %d", len, b);

    for (k = 0; k < len && j < dsize; k++) {
      dp[j++] = b;
    }

    if (j == dsize) {
      if (k == len) {
        debug(DEBUG_TRACE, "Bitmap", "RLE bitmap decompressed %d bytes into %d bytes", i, dsize);
        break;
      }
      debug(DEBUG_ERROR, "Bitmap", "RLE bitmap decompression error 3 (%d < %d)", k, len);
      return -1;
    }
  }

  return 0;
}

static int decompress_bitmap_packbits8(uint8_t *p, uint8_t *dp, uint32_t dsize) {
  uint8_t len, b;
  int8_t count;
  int i, j, k;

  debug(DEBUG_TRACE, "Bitmap", "PackBits8 bitmap decompressing");

  for (i = 0, j = 0;;) {
    i += get1((uint8_t *)&count, p, i);
    debug(DEBUG_TRACE, "Bitmap", "PackBits8 index %d code %d", i-1, count);

    if (count >= -127 && count <= -1) {
      // encoded run packet
      len = (uint8_t)(-count) + 1;
      i += get1(&b, p, i);
      debug(DEBUG_TRACE, "Bitmap", "PackBits8 %d bytes encoded run packet 0x%02X", len, b);

      for (k = 0; k < len && j < dsize; k++) {
        dp[j++] = b;
      }

    } else if (count >= 0 && count <= 127) {
      // literal run packet
      len = (uint8_t)count + 1;
      debug(DEBUG_TRACE, "Bitmap", "PackBits8 %d bytes literal run packet", len);

      for (k = 0; k < len && j < dsize; k++) {
        i += get1(&b, p, i);
        dp[j++] = b;
      }

    } else {
      debug(DEBUG_TRACE, "Bitmap", "PackBits8 code -128 ignored");
    }

    if (j == dsize) {
      debug(DEBUG_TRACE, "Bitmap", "PackBits8 bitmap decompressed %d bytes into %d bytes", i, dsize);
      break;
    }
  }

  return i;
}

static int decompress_bitmap_packbits16(uint8_t *p, uint16_t *dp, uint32_t dsize) {
  uint8_t len;
  uint16_t w;
  int8_t count;
  int i, j, k;

  debug(DEBUG_TRACE, "Bitmap", "PackBits16 bitmap decompressing");

  for (i = 0, j = 0;;) {
    i += get1((uint8_t *)&count, p, i);
    debug(DEBUG_TRACE, "Bitmap", "PackBits16 index %d code %d", i-1, count);

    if (count >= -127 && count <= -1) {
      // encoded run packet
      len = (uint8_t)(-count) + 1;
      i += get2l(&w, p, i);
      debug(DEBUG_TRACE, "Bitmap", "PackBits16 %d bytes encoded run packet 0x%04X", len, w);

      for (k = 0; k < len && j < dsize; k++) {
        dp[j++] = w;
      }

    } else if (count >= 0 && count <= 127) {
      // literal run packet
      len = (uint8_t)count + 1;
      debug(DEBUG_TRACE, "Bitmap", "PackBits16 %d bytes literal run packet", len);

      for (k = 0; k < len && j < dsize; k++) {
        i += get2l(&w, p, i);
        dp[j++] = w;
      }

    } else {
      debug(DEBUG_TRACE, "Bitmap", "PackBits16 code -128 ignored");
    }

    if (j == dsize) {
      debug(DEBUG_TRACE, "Bitmap", "PackBits16 bitmap decompressed %d bytes into %d bytes", i, dsize*2);
      break;
    }
  }

  return i;
}

#define MIN(a,b) ((a) < (b) ? (a) : (b))

static int decompress_bitmap_scanline(uint8_t *p, uint8_t *dp, uint16_t rowBytes, uint16_t width, uint16_t height) {
  uint32_t i, j, k, row, byteCount;
  uint16_t dsize;
  uint8_t diffmask, inval;

  dsize = rowBytes * height;
  debug(DEBUG_TRACE, "Bitmap", "ScanLine bitmap decompressing %d bytes", dsize);
  i = 0;

  for (row = 0; row < height; row++) {
    for (j = 0; j < rowBytes; j += 8) {
      i += get1(&diffmask, p, i);
      byteCount = MIN(rowBytes - j, 8);

      for (k = 0; k < byteCount; k++) {
        if (row == 0 || ((diffmask & (1 << (7 - k))) != 0)) {
          i += get1(&inval, p, i);
          dp[row * rowBytes + j + k] = inval;
        } else {
          dp[row * rowBytes + j + k] = dp[(row - 1) * rowBytes + j + k];
        }
      }
    }
  }

  return 0;
}

static int BmpDecompress(UInt16 version, BitmapCompressionType compression, UInt16 depth, Coord width, Coord height, UInt16 rowBytes, UInt8 *compressed, UInt8 *decompressed) {
  int error = -1;

  switch (version) {
    case 0:
    case 1:
      // XXX Starship's tAIB.1000 icon has two leading zeros before the compressed size.
      // Skip the leading zeros here: I am not sure why it is happening and if this is the right fix.
      // Besides, the compressed size is little endian (?), or it is just 1 byte followed by a zero.
      if (compressed[0] == 0x00 && compressed[1] == 0x00) {
        compressed += 2;
      }
      compressed += 2;
      break;
    case 2:
      compressed += 2;
      break;
    case 3:
      compressed += 4;
      break;
  }

  switch (compression) {
    case BitmapCompressionTypeScanLine:
      error = decompress_bitmap_scanline(compressed, decompressed, rowBytes, width, height);
      break;
    case BitmapCompressionTypeRLE:
      error = decompress_bitmap_rle(compressed, decompressed, rowBytes * height);
      break;
    case BitmapCompressionTypePackBits:
      error = depth == 16 ?
        decompress_bitmap_packbits16(compressed, (uint16_t *)decompressed, (rowBytes * height) / 2) :
        decompress_bitmap_packbits8(compressed, decompressed, rowBytes * height);
      break;
    default:
      debug(DEBUG_ERROR, "Bitmap", "invalid compression type %u", compression);
      error = -1;
      break;
  }

  return error;
}

BitmapType *BmpDecompressBitmap(BitmapType *bitmapP) {
  BitmapCompressionType compression;
  Coord width, height;
  UInt32 transparentValue;
  UInt16 density, rowBytes, error;
  UInt8 version, depth, *compressed, *decompressed;
  Boolean hasTransparency;
  ColorTableType *colorTable;
  BitmapType *newBmp = NULL;

  compression = BmpGetCompressionType(bitmapP);

  if (compression != BitmapCompressionTypeNone) {
    BmpGetDimensions(bitmapP, &width, &height, &rowBytes);
    depth = BmpGetBitDepth(bitmapP);
    density = BmpGetDensity(bitmapP);
    hasTransparency = BmpGetTransparentValue(bitmapP, &transparentValue);
    colorTable = BmpGetColortable(bitmapP);

    newBmp = BmpCreate3(width, height, rowBytes, density, depth, hasTransparency, transparentValue, colorTable, &error);

    if (newBmp && error == 0) {
      version = BmpGetVersion(bitmapP);
      BmpGetDimensions(newBmp, &width, &height, &rowBytes);
      compressed = BmpGetBits(bitmapP);
      decompressed = BmpGetBits(newBmp);
      if (BmpDecompress(version, compression, depth, width, height, rowBytes, compressed, decompressed) != 0) {
        BmpDelete(newBmp);
        newBmp = NULL;
      }
    } else {
      debug(DEBUG_ERROR, "Bitmap", "error creating decompressed bitmap");
    }
  }

  return newBmp;
}

void BmpDecompressBitmapChain(MemHandle handle, DmResType resType, DmResID resID) {
  BitmapType *bitmapP, *bmp, *oldBmp[32];
  BitmapCompressionType compression;
  Coord width, height;
  UInt32 i, r, total, compressed, newSize[32], headerSize, dataSize, totalSize;
  UInt16 rowBytes, attr;
  UInt8 version, depth, *p0, *p;
  Boolean invalid;
  char st[8];

  debug(DEBUG_TRACE, "Bitmap", "BmpDecompressBitmapChain %p begin", handle);

  pumpkin_id2s(resType, st);
  debug(DEBUG_TRACE, "Bitmap", "bitmap type %s id %u", st, resID);

  bitmapP = MemHandleLock(handle);

  for (bmp = bitmapP, total = compressed = totalSize = 0; bmp && total < 32; total++) {
    if (isEmptySlot(bmp)) {
      debug(DEBUG_TRACE, "Bitmap", "bitmap index %d empty slot", total);
      oldBmp[total] = bmp;
      newSize[total] = BitmapV1HeaderSize;
      bmp = skipEmptySlot(bmp);

    } else {
      oldBmp[total] = bmp;
      version = BmpGetVersion(bmp);
      BmpGetDimensions(bmp, &width, &height, &rowBytes);
      depth = BmpGetBitDepth(bmp);

      if (resType == 'tRAW') {
        // XXX FreeJongg stores bitmaps as tRAW, but in general not all tRAW resources are bitmaps...
        // Trying my best to detect if a tRAW resource is really a bitmap
        switch (depth) {
          case 1:
          case 2:
          case 4:
            invalid = 0;
            break;
          case 8:
            invalid = (rowBytes < width);
            break;
          case 16:
            invalid = (rowBytes < width*2);
            break;
          default:
            invalid = 1;
            break;
        }
        attr = BmpGetCommonFlag(bmp, BitmapFlagAll);
        if (invalid || version > 3 || width == 0 || height == 0 || (attr & 0x00FF) != 0x0000) {
          debug(DEBUG_INFO, "Bitmap", "resource type %s id %u is probably not a bitmap (%04X)", st, resID, attr);
          debug_bytes(DEBUG_INFO, "Bitmap", (UInt8 *)bmp, 32);
          MemHandleUnlock(handle);
          return;
        }
      }

      BmpGetSizes(bmp, &dataSize, &headerSize);
      newSize[total] = headerSize + rowBytes * height;
      r = newSize[total] % 4;
      if (r != 0) newSize[total] += 4 - r;
      compression = BmpGetCompressionType(bmp);
      if (compression == BitmapCompressionTypeNone) {
        debug(DEBUG_TRACE, "Bitmap", "bitmap index %d V%u not compressed, size %u", total, version, headerSize + dataSize);
      } else {
        debug(DEBUG_TRACE, "Bitmap", "bitmap index %d V%u compression type %u, size %u", total, version, compression, headerSize + dataSize);
        compressed++;
      }
      bmp = BmpGetNextBitmapAnyDensity(bmp);
    }
    totalSize += newSize[total];
  }
  debug(DEBUG_TRACE, "Bitmap", "bitmap chain has %d bitmap(s), %d compressed, size %u", total, compressed, MemHandleSize(handle));

  if (compressed == 0) {
    debug(DEBUG_TRACE, "Bitmap", "BmpDecompressBitmapChain end (nothing to do)");
    MemHandleUnlock(handle);
    return;
  }

  debug(DEBUG_TRACE, "Bitmap", "new bitmap chain size is %u (0x%04X)", totalSize, totalSize);
  if ((p0 = MemPtrNew(totalSize)) == NULL) {
    MemHandleUnlock(handle);
    return;
  }
  p = p0;

  for (i = 0; i < total; i++) {
    if (isEmptySlot(oldBmp[i])) {
      debug(DEBUG_TRACE, "Bitmap", "bitmap index %d empty slot", i);
      MemMove(p, oldBmp[i], newSize[i]);

    } else {
      version = BmpGetVersion(oldBmp[i]);
      BmpGetSizes(oldBmp[i], NULL, &headerSize);
      BmpGetDimensions(oldBmp[i], &width, &height, &rowBytes);
      compression = BmpGetCompressionType(oldBmp[i]);
      depth = BmpGetBitDepth(oldBmp[i]);
      MemMove(p, oldBmp[i], headerSize);

      debug(DEBUG_TRACE, "Bitmap", "bitmap index %d V%u %dx%d, %d bpp, offset 0x%08X", i, version, width, height, depth, (UInt32)(p - p0));
      if (compression == BitmapCompressionTypeNone) {
        MemMove(p + headerSize, (UInt8 *)oldBmp[i] + headerSize, rowBytes * height);
      } else {
        BmpDecompress(version, compression, depth, width, height, rowBytes, (UInt8 *)oldBmp[i] + headerSize, p + headerSize);
        BmpSetCommonFlag((BitmapType *)p, BitmapFlagCompressed, 0);
      }

      switch (version) {
        case 1:
          BmpV1SetField((BitmapType *)p, BitmapV1FieldNextDepthOffset, (i < total - 1) ? newSize[i] / 4 : 0);
          break;
        case 2:
          BmpV2SetField((BitmapType *)p, BitmapV2FieldNextDepthOffset, (i < total - 1) ? newSize[i] / 4 : 0);
          break;
        case 3:
          BmpV3SetField((BitmapType *)p, BitmapV3FieldNextBitmapOffset, (i < total - 1) ? newSize[i] : 0);
          break;
      }
    }
    p += newSize[i];
  }

  MemHandleUnlock(handle);
  MemHandleResize(handle, totalSize);
  bitmapP = MemHandleLock(handle);
  MemMove(bitmapP, p0, totalSize);
  MemHandleUnlock(handle);
  MemPtrFree(p0);

  debug(DEBUG_TRACE, "Bitmap", "BmpDecompressBitmapChain end %p", handle);
}

void BmpExportFont(UInt16 id, UInt16 fw, UInt16 fh) {
  MemHandle hbmp;
  BitmapType *bmp;
  FileRef fileRef;
  UInt16 cols, col, row, x, y;
  Coord w, h;
  char buf[256];
  int i, j, k;

  hbmp = DmGetResource(bitmapRsc, id);
  bmp = MemHandleLock(hbmp);
  BmpGetDimensions(bmp, &w, &h, NULL);
  cols = w / fw;
  StrPrintF(buf, "f%u.txt", id);
  VFSFileCreate(1, buf);
  VFSFileOpen(1, buf, vfsModeWrite, &fileRef);
  VFSFilePrintF(fileRef, "fontType 36864\nascent 16\ndescent 0\n");
  for (i = 1; i < 256; i++) {   
    row = i / cols;
    col = i % cols;
    VFSFilePrintF(fileRef, "\nGLYPH %d\n", i);
    for (j = 0; j < fh; j++) {
      for (k = 0; k < fw; k++) {
        y = row * fh + j;
        x = col * fw + k;
        buf[k] = BmpGetPixelValue(bmp, x, y) ? '#' : '-';
      }
      buf[k++] = '\n';
      buf[k] = 0;
      VFSFilePrintF(fileRef, buf);
    }
  } 
  VFSFileClose(fileRef);
  MemHandleUnlock(hbmp);
  DmReleaseResource(hbmp);
}

BitmapType *BmpRotate(BitmapType *bitmapP, Int16 angle) {
  BitmapType *rotated = NULL;
  Coord width, height, newWidth, newHeight, cx, cy, x, y, m, n, j, k;
  UInt16 density, depth, error;
  UInt32 transparentValue, color;
  Boolean leBits, hasTransparency;
  double radians, dsin, dcos;

  while (angle < 0) angle += 360;
  while (angle >= 360) angle -= 360;

  bitmapP = skipEmptySlot(bitmapP);
  BmpGetDimensions(bitmapP, &width, &height, NULL);
  leBits = BmpGetCommonFlag(bitmapP, BitmapFlagLittleEndian);
  density = BmpGetDensity(bitmapP);
  depth = BmpGetBitDepth(bitmapP);
  hasTransparency = BmpGetTransparentValue(bitmapP, &transparentValue);

  if (angle == 0) {
    rotated = BmpCreate3(width, height, 0, density, depth, hasTransparency, transparentValue, NULL, &error);
    BmpSetCommonFlag(rotated, BitmapFlagLittleEndian, leBits);

    for (x = 0; x < width; x++) {
      for (y = 0; y < height; y++) {
        color = BmpGetPixelValue(bitmapP, x, y);
        BmpSetPixel(rotated, x, y, color);
      }
    }

  } else {
    radians = angle * sys_pi() / 180.0;
    dcos = sys_cos(radians);
    dsin = sys_sin(radians);
    newWidth = width > height ? width : height;
    newHeight = newWidth;
    cx = newWidth / 2;
    cy = newHeight / 2;
    rotated = BmpCreate3(newWidth, newHeight, 0, density, depth, hasTransparency, transparentValue, NULL, &error);
    BmpSetCommonFlag(rotated, BitmapFlagLittleEndian, leBits);

    for (x = 0; x < newWidth; x++) {
      for (y = 0; y < newHeight; y++) {
        m = x - cx;
        n = y - cy;
        j = (Coord)sys_floor((m * dcos - n * dsin) + cx + 0.5);
        k = (Coord)sys_floor((n * dcos + m * dsin) + cy + 0.5);
        if (j >= 0 && j < width && k >= 0 && k < height){
          color = BmpGetPixelValue(bitmapP, j, k);
        } else {
          color = depth == 32 ? rgba32(0, 0, 0, 0) : transparentValue;
        }
        BmpSetPixel(rotated, x, y, color);
      }
    }
  }

  return rotated;
}

BitmapType *BmpFlip(BitmapType *bitmapP, Boolean vertical, Boolean horizontal) {
  BitmapType *flipped = NULL;
  Coord width, height, x, y;
  UInt16 density, depth, error;
  UInt32 transparentValue, color;
  Boolean leBits, hasTransparency;

  bitmapP = skipEmptySlot(bitmapP);
  BmpGetDimensions(bitmapP, &width, &height, NULL);
  leBits = BmpGetCommonFlag(bitmapP, BitmapFlagLittleEndian);
  density = BmpGetDensity(bitmapP);
  depth = BmpGetBitDepth(bitmapP);
  hasTransparency = BmpGetTransparentValue(bitmapP, &transparentValue);
  flipped = BmpCreate3(width, height, 0, density, depth, hasTransparency, transparentValue, NULL, &error);
  BmpSetCommonFlag(flipped, BitmapFlagLittleEndian, leBits);

  for (x = 0; x < width; x++) {
    for (y = 0; y < height; y++) {
      color = BmpGetPixelValue(bitmapP, x, y);
      BmpSetPixel(flipped, horizontal ? width - x - 1 : x, vertical ? height - y - 1 : y, color);
    }
  }

  return flipped;
}
