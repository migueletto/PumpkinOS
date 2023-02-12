#include <PalmOS.h>

MemHandle SysFormPointerArrayToStrings(Char *c, Int16 stringCount) {
  MemHandle h = NULL;
  Char **p, *s;
  Int16 i, j;

  if (c) {
    if ((h = MemHandleNew(stringCount * sizeof(Char *))) != NULL) {
      if ((p = MemHandleLock(h)) != NULL) {
        s = c;
        for (i = 0, j = 0; j < stringCount;) {
          if (s[i] == 0) {
            p[j] = s;
            s = &s[i+1];
            i = 0;
            j++;
          } else {
            i++;
          }
        }
        MemHandleUnlock(h);
      }
    }
  }

  return h;
}
