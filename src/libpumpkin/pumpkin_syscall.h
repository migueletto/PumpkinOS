#include "pumpkin_syscall_id.h"

inline void FrmCenterDialogs(Boolean center) {
  pumpkin_system_call(sysCallFrmCenterDialogs, NULL, NULL);
}

inline WinHandle WinCreateOffscreenWindow(Coord width, Coord height, WindowFormatType format, UInt16 *error) {
  void *pret;
  pumpkin_system_call(sysTrapWinCreateOffscreenWindow, NULL, &pret, width, height, format, error);
  return (WinHandle)pret;
}
