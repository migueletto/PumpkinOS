#include <PalmOS.h>

#include "emulator.h"
#include "debug.h"

static Err read_bin(uint8_t *bin, uint8_t *memory) {
  UInt16 start, len, exec;
  UInt8 buf[16];
  UInt32 pos;
  int fd;

  fd = sys_create("trooper.bin", SYS_WRITE | SYS_TRUNC, 0644);

  pos = 0;
  for (;;) {
    sys_memcpy(buf, &bin[pos], 5);
    pos += 5;

    if (buf[0] != 0x00) break;

    len = buf[1]*256 + buf[2];
    start = buf[3]*256 + buf[4];

    if (start >= 0x8000 || len > 0x8000)
      return -2;

    if ((start + len) >= 0x8000)
      len = 0x8000 - start;

    debug(1, "XXX", "read 0x%04X bytes at 0x%04X", len, start);
    sys_memcpy(&memory[start], &bin[pos], len);
    sys_write(fd, &memory[start], len);
    pos += len;
  }

  if (buf[1] != 0x00 || buf[2] != 0x00)
    return -4;

  sys_close(fd);

  exec = buf[3]*256 + buf[4];
  debug(1, "XXX", "exec 0x%04X", exec);

  return 0;
}

static uint8_t memory[65536];

static void read_bin_res(void) {
  MemHandle h;
  uint8_t *bin;

  if ((h = DmGet1Resource('pBIN', 1)) != NULL) {
    if ((bin = MemHandleLock(h)) != NULL) {
      if (read_bin(bin, memory) != 0) {
        debug(1, "XXX", "error reading bin");
      }
      MemHandleUnlock(h);
    }
    MemHandleUnlock(h);
  }
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  if (cmd == sysAppLaunchCmdNormalLaunch) {
    //read_bin_res();
    EmulatorMain();
  }

  return 0;
}
