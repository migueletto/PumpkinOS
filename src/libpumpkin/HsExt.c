#include <PalmOS.h>
#include <VFSMgr.h>
#include <INetMgr.h>
#include <HsNavCommon.h>
#include <HsExtTraps.h>
#include <HsExt.h>
#include <HsKeyTypes.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

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

  keys[0] = 0;
  keys[1] = 0;
  keys[2] = 0;

  if (keyMask & keyBitNumRockerDown) {
    keys[0] |= 1 << keyBitNumPageDown;
  }
  if (keyMask & keyBitNumRockerUp) {
    keys[0] |= 1 << keyBitNumPageUp;
  }
  if (keyMask & keyBitLeft) {
    keys[0] |= 1 << keyBitNumRockerLeft;
  }
  if (keyMask & keyBitRight) {
    keys[0] |= 1 << keyBitNumRockerRight;
  }
  if (keyMask & keyBitHard1) {
    keys[0] |= 1 << keyBitNumHard1;
  }
  if (keyMask & keyBitHard2) {
    keys[0] |= 1 << keyBitNumHard2;
  }
  if (keyMask & keyBitHard3) {
    keys[0] |= 1 << keyBitNumHard3;
  }
  if (keyMask & keyBitHard4) {
    keys[0] |= 1 << keyBitNumHard4;
  }
}
