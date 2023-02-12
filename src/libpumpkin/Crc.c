#include <PalmOS.h>

#include "debug.h"

UInt16 Crc16CalcBlock(const void *bufP, UInt16 count, UInt16 crc) {
  return Crc16CalcBigBlock((void *)bufP, count, crc);
}

// XXX validate

UInt16 Crc16CalcBigBlock(void *bufP, UInt32 count, UInt16 crc) {
  UInt8 x, *p;

  p = (UInt8 *)bufP;

  while (count--) {
    x = crc >> 8 ^ *p++;
    x ^= x >> 4;
    crc = (crc << 8) ^ ((UInt16)(x << 12)) ^ ((UInt16)(x <<5)) ^ ((UInt16)x);
  }

  return crc;
}
