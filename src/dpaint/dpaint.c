#include <PalmOS.h>

#include "tos.h"

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  if (cmd == sysAppLaunchCmdNormalLaunch) {
    TOSMain();
  }

  return 0;
}
