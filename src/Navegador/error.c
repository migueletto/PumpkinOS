#include <PalmOS.h>

#include "error.h"
#include "gui.h"
#include "pumpkin.h"

void InfoDialog(int type, const char *fmt, ...) {
  char buf[256];
  sys_va_list arg;

  sys_va_start(arg, fmt);
  StrVNPrintF(buf, sizeof(buf)-1, fmt, arg);

  switch (type) {
    case INFO:
      FrmCustomAlert(InfoAlert, buf, "", "");
      break;
    case WARNING:
      FrmCustomAlert(WarningAlert, buf, "", "");
      break;
    case ERROR:
      FrmCustomAlert(ErrorAlert, buf, "", "");
  }
  sys_va_end(arg);
}
