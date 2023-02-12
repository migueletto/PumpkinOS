#include <PalmOS.h>

#include "debug.h"

void BmpGlueGetDimensions(const BitmapType *bitmapP, Coord *widthP, Coord *heightP, UInt16 *rowBytesP) {
  BmpGetDimensions(bitmapP, widthP, heightP, rowBytesP);
}

UInt8 BmpGlueGetBitDepth(const BitmapType *bitmapP) {
  return BmpGetBitDepth(bitmapP);
}

BitmapType *BmpGlueGetNextBitmap(BitmapType *bitmapP) {
  return BmpGetNextBitmap(bitmapP);
}

BitmapCompressionType BmpGlueGetCompressionType(const BitmapType *bitmapP) {
  return BmpGetCompressionType(bitmapP);
}

Boolean BmpGlueGetTransparentValue(const BitmapType *bitmapP, UInt32 *transparentValueP) {
  return BmpGetTransparentValue(bitmapP, transparentValueP);
}

void BmpGlueSetTransparentValue(BitmapType *bitmapP, UInt32 transparentValue) {
  BmpSetTransparentValue(bitmapP, transparentValue);
}

void *BmpGlueGetBits(BitmapType *bitmapP) {
  return BmpGetBits(bitmapP);
}
