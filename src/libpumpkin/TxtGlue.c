#include <PalmOS.h>

#include "sys.h"

UInt16 TxtGlueCharAttr(WChar inChar) {
  return TxtCharAttr(inChar);
}

WChar TxtGlueUpperChar(WChar inChar) {
  return sys_toupper(inChar);
}

WChar TxtGlueLowerChar(WChar inChar) {
  return sys_tolower(inChar);
}

Boolean TxtGlueCharIsValid(WChar inChar) {
  return TxtCharIsValid(inChar);
}

Boolean TxtGlueFindString(const Char* inSourceStr, const Char* inTargetStr, UInt32* outPos, UInt16* outLength) {
  return TxtFindString(inSourceStr, inTargetStr, outPos, outLength);
}

UInt16 TxtGlueGetPreviousChar(const Char* inText, UInt32 inOffset, WChar* outChar) {
  return TxtGetPreviousChar(inText, inOffset, outChar);
}

UInt16 TxtGlueGetNextChar(const Char* inText, UInt32 inOffset, WChar* outChar) {
  return TxtGetNextChar(inText, inOffset, outChar);
}

UInt16 TxtGlueSetNextChar(Char* ioText, UInt32 inOffset, WChar inChar) {
  return TxtGlueSetNextChar(ioText, inOffset, inChar);
}

UInt16 TxtGlueCharSize(WChar inChar) {
  return TxtCharSize(inChar);
}
