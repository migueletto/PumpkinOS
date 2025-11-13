#include <PalmOS.h>

#include "emulator.h"

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  if (cmd == sysAppLaunchCmdNormalLaunch) {
    EmulatorMain();
  }

  return 0;
}
