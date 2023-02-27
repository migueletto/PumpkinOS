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
