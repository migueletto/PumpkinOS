#include <PalmOS.h>

#include "bytes.h"
#include "debug.h"

void NetLibBitMove(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, UInt8 *srcP, UInt32 *srcBitOffsetP, UInt32 numBits) {
  debug(DEBUG_ERROR, "NetMgr", "NetLibBitMove not implemented");
}

void NetLibBitPutUIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, UInt32 value) {
  put4b(value, dstP, *dstBitOffsetP);
  *dstBitOffsetP += 4;
}

void NetLibBitPutIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, Int32 value) {
  put4b(value, dstP, *dstBitOffsetP);
  *dstBitOffsetP += 4;
}

UInt32 NetLibBitGetUIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP) {
  UInt32 value;
  get4b(&value, dstP, *dstBitOffsetP);
  *dstBitOffsetP += 4;
  return value;
}

Int32 NetLibBitGetIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP) {
  Int32 value;
  get4b((UInt32 *)&value, dstP, *dstBitOffsetP);
  *dstBitOffsetP += 4;
  return value;
}

void NetLibBitPutFixed(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, UInt32 value, UInt16 numBits) {
  debug(DEBUG_ERROR, "NetMgr", "NetLibBitPutFixed not implemented");
}

static UInt8 getBit(UInt8 *dstP, UInt32 *offset) {
  UInt32 byteIndex, bitPos;
  UInt8 bit;

  byteIndex = *offset >> 3;
  bitPos = *offset & 7;
  bit = dstP[byteIndex];
  bit >>= 7 - bitPos; // big endian bits
  *offset += 1;

  return bit & 1;
}

UInt32 NetLibBitGetFixed(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, UInt16 numBits) {
  UInt32 v32, old, index, i;
  UInt16 v16;
  UInt8 v8, bit;

  switch (numBits) {
    case 0:
      return 0;
    case 8:
      if ((*dstBitOffsetP & 0x07) == 0) {
        index = *dstBitOffsetP >> 3;
        v8 = dstP[index];
        debug(DEBUG_INFO, "NetMgr", "NetLibBitGettFixed byte pos=%u 0x%02X", *dstBitOffsetP, v8);
        *dstBitOffsetP += 8;
        return v8;
      }
      break;
    case 16:
      if ((*dstBitOffsetP & 0x0F) == 0) {
        index = *dstBitOffsetP >> 3;
        get2b(&v16, dstP, index);
        debug(DEBUG_INFO, "NetMgr", "NetLibBitGettFixed word pos=%u 0x%04X", *dstBitOffsetP, v16);
        *dstBitOffsetP += 16;
        return v16;
      }
      if ((*dstBitOffsetP & 0x07) == 0) {
        index = *dstBitOffsetP >> 3;
        v16 = dstP[index++];
        v16 <<= 8;
        v16 |= dstP[index];
        debug(DEBUG_INFO, "NetMgr", "NetLibBitGettFixed word (middle) pos=%u 0x%04X", *dstBitOffsetP, v16);
        *dstBitOffsetP += 16;
        return v16;
      }
      break;
    default:
      if (numBits > 32) {
        debug(DEBUG_ERROR, "NetMgr", "NetLibBitGettFixed numBits %u > 32", numBits);
        return 0;
      }
      break;
  }

  old = *dstBitOffsetP;
  v32 = 0;

  for (i = 0; i < numBits; i++) {
    bit = getBit(dstP, dstBitOffsetP);
    v32 |= bit << (numBits - i - 1);
  }

  debug(DEBUG_INFO, "NetMgr", "NetLibBitGettFixed numBits=%u pos=%u 0x%X", numBits, old, v32);
  return v32;
}
