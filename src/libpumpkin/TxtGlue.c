#include <PalmOS.h>

#include <ctype.h>

UInt16 TxtGlueCharAttr(WChar inChar) {
  return TxtCharAttr(inChar);
}

WChar TxtGlueUpperChar(WChar inChar) {
  return toupper(inChar);
}

WChar TxtGlueLowerChar(WChar inChar) {
  return tolower(inChar);
}

Boolean TxtGlueCharIsValid(WChar inChar) {
  return TxtCharIsValid(inChar);
}

Boolean TxtGlueFindString(const Char* inSourceStr, const Char* inTargetStr, UInt32* outPos, UInt16* outLength) {
  return TxtFindString(inSourceStr, inTargetStr, outPos, outLength);
}
