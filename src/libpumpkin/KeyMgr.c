#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "mem.h"
#include "pumpkin.h"
#include "xalloc.h"
#include "debug.h"

typedef struct {
  UInt16 initDelay;
  UInt16 period;
  UInt16 doubleTapDelay;
  Boolean queueAhead;
  UInt32 keyMask;
} key_module_t;

extern thread_key_t *key_key;

int KeyInitModule(void) {
  key_module_t *module;

  if ((module = xcalloc(1, sizeof(key_module_t))) == NULL) {
    return -1;
  }

  module->initDelay = 10;
  module->period = 10;
  module->doubleTapDelay = 10;
  module->queueAhead = false;
  module->keyMask = 0xFFFFFFFF;
  thread_set(key_key, module);

  return 0;
}

int KeyFinishModule(void) {
  key_module_t *module = (key_module_t *)thread_get(key_key);

  if (module) {
    xfree(module);
  }

  return 0;
}

// Returns a UInt32 with bits set for keys that are depressed.
// See keyBitPower, keyBitPageUp, keyBitPageDown, etc., in KeyMgr.h

UInt32 KeyCurrentState(void) {
  uint32_t keyMask;

  pumpkin_status(NULL, NULL, &keyMask, NULL, NULL, NULL);

  return keyMask;
}

Err KeyRates(Boolean set, UInt16 *initDelayP, UInt16 *periodP, UInt16 *doubleTapDelayP, Boolean *queueAheadP) {
  key_module_t *module = (key_module_t *)thread_get(key_key);

  if (set) {
    debug(DEBUG_INFO, "KeyMgr", "KeyRates set %d,%d,%d,%d", initDelayP ? *initDelayP : 0, periodP ? *periodP : 0, doubleTapDelayP ? *doubleTapDelayP : 0, queueAheadP ? *queueAheadP : 0);
    if (initDelayP) module->initDelay = *initDelayP;
    if (periodP) module->period = *periodP;
    if (doubleTapDelayP) module->doubleTapDelay = *doubleTapDelayP;
    if (queueAheadP) module->queueAhead = *queueAheadP;
  } else {
    if (initDelayP) *initDelayP = module->initDelay;
    if (periodP) *periodP = module->period;
    if (doubleTapDelayP) *doubleTapDelayP = module->doubleTapDelay;
    if (queueAheadP) *queueAheadP = module->queueAhead;
    debug(DEBUG_INFO, "KeyMgr", "KeyRates get %d,%d,%d,%d", initDelayP ? *initDelayP : 0, periodP ? *periodP : 0, doubleTapDelayP ? *doubleTapDelayP : 0, queueAheadP ? *queueAheadP : 0);
  }

  return errNone;
}

// Specify which keys generate keyDownEvents.
// Returns the old key Mask.

UInt32 KeySetMask(UInt32 keyMask) {
  key_module_t *module = (key_module_t *)thread_get(key_key);
  UInt32 old;

  debug(DEBUG_INFO, "KeyMgr", "KeySetMask 0x%08X", keyMask);
  old = module->keyMask;
  module->keyMask = keyMask;
  pumpkin_keymask(keyMask);

  return old;
}
