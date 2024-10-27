#include <PalmOS.h>

WChar TxtUpperChar(WChar inChar) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysCallTxtUpperChar, 0, &iret, NULL, inChar);
  return (Boolean)iret;
}

WChar TxtLowerChar(WChar inChar) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysCallTxtLowerChar, 0, &iret, NULL, inChar);
  return (Boolean)iret;
}
