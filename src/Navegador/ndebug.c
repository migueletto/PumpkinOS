#include <PalmOS.h>

#include "gps.h"
#include "app.h"
#include "ndebug.h"
#include "log.h"
#include "ddb.h"

static char buf[256];
static int debug_on = 0;

void Debug(int on)
{
  if (on == -1)
    DbDelete(DebugName);
  else
    debug_on = on;
}

void DebugMsg(const char *fmt, ...)
{
  va_list arg;
  FileHand debug;
  DateTimeType dt;

  if (!debug_on)
    return;

  va_start(arg, fmt);
  debug = OpenLog(DebugName, AppID, DebugType, fileModeUpdate);

  TimSecondsToDateTime(TimGetSeconds(), &dt);
  StrPrintF(buf, "%02d:%02d:%02d ", dt.hour, dt.minute, dt.second);
  WriteLog(debug, buf, StrLen(buf));

  StrVPrintF(buf, fmt, arg);
  WriteLog(debug, buf, StrLen(buf));
  WriteLog(debug, "\n", 1);

  CloseLog(debug);
  va_end(arg);
}

void DebugBytes(unsigned char *packet, int n)
{
  Int16 i;
  char hex[8];
  FileHand debug;

  if (!debug_on)
    return;

  debug = OpenLog(DebugName, AppID, DebugType, fileModeUpdate);

  MemSet(buf, sizeof(buf), 0);
  for (i = 0; i < n; i++) {
    if (i && ((i % 8) == 0)) {
      StrCat(buf, "\n");
      WriteLog(debug, buf, StrLen(buf));
      MemSet(buf, sizeof(buf), 0);
    }
    StrPrintF(hex, "%04X ", packet[i]);
    StrCat(buf, &hex[2]);
  }
  if (buf[0]) {
    StrCat(buf, "\n");
    WriteLog(debug, buf, StrLen(buf));
  }

  CloseLog(debug);
}
