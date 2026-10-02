#include <PalmOS.h>
#include <HsNavCommon.h>
#include <HsExtTraps.h>
#include <HsExt.h>
#include <HsKeyTypes.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "xalloc.h"
#include "debug.h"

typedef struct {
  UInt16 initDelay;
  UInt16 period;
  UInt16 doubleTapDelay;
  Boolean queueAhead;
  UInt32 keyMask[3];
} key_module_t;

int KeyInitModule(void) {
  key_module_t *module;

  if ((module = xcalloc(1, sizeof(key_module_t))) == NULL) {
    return -1;
  }

  module->initDelay = 10;
  module->period = 10;
  module->doubleTapDelay = 10;
  module->queueAhead = false;
  module->keyMask[0] = 0xFFFFFFFF;
  module->keyMask[1] = 0xFFFFFFFF;
  module->keyMask[2] = 0xFFFFFFFF;
  pumpkin_set_local_storage(key_key, module);

  return 0;
}

int KeyFinishModule(void) {
  key_module_t *module = (key_module_t *)pumpkin_get_local_storage(key_key);

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
  key_module_t *module = (key_module_t *)pumpkin_get_local_storage(key_key);

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
  key_module_t *module = (key_module_t *)pumpkin_get_local_storage(key_key);
  UInt32 old;

  debug(DEBUG_INFO, "KeyMgr", "KeySetMask 0x%08X", keyMask);
  old = module->keyMask[0];
  module->keyMask[0] = keyMask;
  pumpkin_keymask(keyMask);

  return old;
}

/*
#define keyBitNumPower         0      // Power key
#define keyBitNumPageUp        1      // Page-up
#define keyBitNumPageDown      2      // Page-down
#define keyBitNumHard1         3      // App #1
#define keyBitNumHard2         4      // App #2
#define keyBitNumHard3         5      // App #3
#define keyBitNumHard4         6      // App #4
#define keyBitNumCradle        7      // Button on cradle
#define keyBitNumAntenna       8      // Antenna "key" <chg 3-31-98 RM>
#define keyBitNumContrast      9      // Contrast key


#define keyBitNumJogUp        12      // jog wheel up
#define keyBitNumJogDown      13      // jog wheel down
#define keyBitNumJogPress     14      // press/center on jog wheel
#define keyBitNumJogBack      15      // jog wheel back button
#define keyBitNumRockerUp     16      // 5-way rocker up
#define keyBitNumRockerDown   17      // 5-way rocker down
#define keyBitNumRockerLeft   18      // 5-way rocker left
#define keyBitNumRockerRight  19      // 5-way rocker right
#define keyBitNumRockerCenter 20      // 5-way rocker center/press
*/

void HsKeyCurrentStateExt(UInt32 keys[3]) {
  uint32_t keyMask;

  pumpkin_status(NULL, NULL, &keyMask, NULL, NULL, NULL);
  keys[0] = keyMask & 0x3FF; // mask out everything above keyBitNumContrast
  keys[1] = 0; // XXX not supported yet
  keys[2] = 0; // XXX not supported yet

  // map left key to keyBitNumRockerLeft instead of keyBitLeft
  if (keyMask & keyBitLeft) {
    keys[0] &= ~keyBitLeft;
    keys[0] |= 1 << keyBitNumRockerLeft;
  }

  // map right key to keyBitNumRockerRight instead of keyBitRight
  if (keyMask & keyBitRight) {
    keys[0] &= ~keyBitRight;
    keys[0] |= 1 << keyBitNumRockerRight;
  }
}

void HsKeySetMaskExt(const UInt32 keyMaskNew[3], UInt32 keyMaskOld[3]) {
  key_module_t *module = (key_module_t *)pumpkin_get_local_storage(key_key);

  debug(DEBUG_INFO, "KeyMgr", "HsKeySetMaskExt 0x%08X 0x%08X 0x%08X", keyMaskNew[0], keyMaskNew[1], keyMaskNew[2]);
  if (keyMaskOld) keyMaskOld[0] = module->keyMask[0];
  if (keyMaskOld) keyMaskOld[1] = module->keyMask[1];
  if (keyMaskOld) keyMaskOld[2] = module->keyMask[2];
  module->keyMask[0] = keyMaskNew[0];
  module->keyMask[1] = keyMaskNew[1];
  module->keyMask[2] = keyMaskNew[2];
  pumpkin_keymask(keyMaskNew[0]);
}
