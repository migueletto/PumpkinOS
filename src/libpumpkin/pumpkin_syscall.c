#include <PalmOS.h>

#include "pumpkin.h"
#include "pumpkin_syscall_id.h"
#include "debug.h"

int pumpkin_system_call(uint32_t id, uint64_t *iret, void **pret, ...) {
  sys_va_list ap;
  int r = -1;

  sys_va_start(ap, pret);
  switch (id) {
    case sysCallFrmCenterDialogs:
      Boolean center = sys_va_arg(ap, int);
      FrmCenterDialogs(center);
      r = 0;
      break;
    case sysTrapWinCreateOffscreenWindow: {
      Coord width = sys_va_arg(ap, int);
      Coord height = sys_va_arg(ap, int);
      WindowFormatType format = sys_va_arg(ap, unsigned int);
      UInt16 *error = sys_va_arg(ap, void *);
      WinHandle wh = WinCreateOffscreenWindow(width, height, format, error);
      *pret = wh;
      r = 0;
      }
      break;
    default:
      debug(DEBUG_ERROR, PUMPKINOS, "invalid system call 0x%04X", id);
      break;
  }
  sys_va_end(ap);

  return r;
}
