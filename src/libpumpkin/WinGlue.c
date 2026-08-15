#include <PalmOS.h>

#include "WindowAccessor.h"

void WinGlueDrawTruncChars(const Char* pChars, UInt16 length, Int16 x, Int16 y, Int16 maxWidth) {
  WinDrawTruncChars(pChars, length, x, y, maxWidth);
}

FrameType WinGlueGetFrameType(const WinHandle winH) {
  //return winH ? winH->frameType.word : 0;
  return winH ? WinGetField(winH, WindowFieldFrameType) : 0;
}

void WinGlueSetFrameType(WinHandle winH, FrameType frame) {
  if (winH) {
    //winH->frameType.word = frame;
    WinSetField(winH, WindowFieldFrameType, frame);
  }
}

