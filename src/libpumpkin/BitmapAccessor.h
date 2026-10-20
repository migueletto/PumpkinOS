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

UInt32 BmpGetSetCommonField(BitmapType *bmp, BitmapSelector selector, BitmapFlagSelector flagSelector, UInt32 value, Boolean set);
UInt32 BmpV0GetSetField(BitmapType *bmp, BitmapSelector selector, BitmapFlagSelector flagSelector, UInt32 value, Boolean set);
UInt32 BmpV1GetSetField(BitmapType *bmp, BitmapV1Selector selector, BitmapFlagSelector flagSelector, UInt32 value, Boolean set);
UInt32 BmpV2GetSetField(BitmapType *bmp, BitmapV2Selector selector, BitmapFlagSelector flagSelector, UInt32 value, Boolean set);
UInt32 BmpV3GetSetField(BitmapType *bmp, BitmapV3Selector selector, BitmapFlagSelector flagSelector, UInt32 value, Boolean set);
