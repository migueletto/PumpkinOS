#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "emupalmosinc.h"
#include "debug.h"

#define PALMOS_MODULE "Error"

Int16 ErrSetJump(ErrJumpBuf buf) {
  return sys_setjmp(buf);
}

void ErrLongJump(ErrJumpBuf buf, Int16 result) {
  sys_longjmp(buf, result);
}

MemPtr *ErrExceptionList(void) {
  return pumpkin_get_exception();
}

void ErrThrow(Int32 err) {
  MemPtr *p;
  ErrExceptionType *e;

  if ((p = ErrExceptionList()) != NULL) {
    if ((e = (ErrExceptionType *)(*p)) != NULL) {
      *p = e->nextP;
      ErrLongJump(e->state, err);
    }
  }
}

void ErrDisplayFileLineMsg(const Char * const filename, UInt16 lineNo, const Char * const msg) {
  ErrDisplayFileLineMsgEx(filename, NULL, lineNo, msg, 0);
}

void ErrDisplayFileLineMsgEx(const Char * const filename, const Char * const function, UInt16 lineNo, const Char * const msg, int finish) {
  char buf[256];
  char *fn, *m;
  int i;

  fn = filename ? (char *)filename : "unknown";
  for (i = StrLen(fn)-1; i >= 0; i--) {
    if (fn[i] == '/') {
      fn = &fn[i+1];
      break;
    }
  }
  m = msg ? (char *)msg : "no message";
  if (function) {
    sys_snprintf(buf, sizeof(buf) - 1, "Fatal Alert %s, %s, Line:%d: %s", fn, function, lineNo, m);
  } else {
    sys_snprintf(buf, sizeof(buf) - 1, "Fatal Alert %s, Line:%d: %s", fn, lineNo, m);
  }
  SysFatalAlert(buf);

  if (pumpkin_is_m68k()) {
    // emupalmos_finish() is safer because it does not rely on ErrLongJump.
    emupalmos_finish(1);
  } else {
    pumpkin_fatal_error(finish);
  }
}

UInt16 ErrAlertCustom(Err errCode, Char *errMsgP, Char *preMsgP, Char *postMsgP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "ErrAlertCustom not implemented");
  return 0;
}

