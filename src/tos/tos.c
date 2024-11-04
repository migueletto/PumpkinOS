#include <PalmOS.h>

#include "heap.h"
#include "tos.h"
#include "debug.h"

int CommandMain(int argc, char *argv[]) {
  LocalID dbID;
  DmOpenRef dbRef;
  int r = -1;

  if (argc >= 2) {
    if ((dbID = DmFindDatabase(0, argv[1])) != 0) {
      if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
        debug(DEBUG_INFO, "TOS", "running TOS command \"%s\" with %d argument(s)", argv[1], argc - 2);
        r = tos_main(argc - 1, &argv[1]);
        debug(DEBUG_INFO, "TOS", "TOS command \"%s\" returned %d", argv[1], r);
        DmCloseDatabase(dbRef);
      }
    }
  }

  return r;
}
