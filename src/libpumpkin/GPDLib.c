#include <PalmOS.h>
#include <PalmCompatibility.h>
#include <time.h>
#include <GPDLib.h>

#include "pumpkin.h"
#include "debug.h"

Err GPDOpen(UInt16 uRefNum) {
  debug(DEBUG_INFO, "GPDLib", "GPDOpen");

  return errNone;
}

Err GPDClose(UInt16 uRefNum, UInt32 *dwRefCountP) {
  debug(DEBUG_INFO, "GPDLib", "GPDClose");
  if (dwRefCountP) *dwRefCountP = 0;

  return errNone;
}

Err GPDGetVersion(UInt16 uRefNum, UInt32 *dwVerP) {
  // version 1.2 according to GPDLib.h
  if (dwVerP) *dwVerP = sysMakeROMVersion(1, 2, 0, 0, 0);

  return errNone;
}

Err GPDReadInstant(UInt16 uRefNum, UInt8 *resultP) {
  uint32_t keyMask, modMask, buttonMask;
  uint64_t extKeyMask[2];
  Err err = GPDErrParam;

  if (resultP) {
    pumpkin_status(NULL, NULL, &keyMask, &modMask, &buttonMask, extKeyMask);
    // buttonMask: button1 = 0x01, button2 = 0x02
    // modMask: WINDOW_MOD_CTRL
    // keyMask:
    //   keyBitPageDown
    //   keyBitPageUp
    //   keyBitLeft
    //   keyBitRight
    //   keyBitHard1
    //   keyBitHard2
    //   keyBitHard3
    //   keyBitHard4

    *resultP = 0;

    if (keyMask & keyBitPageDown) *resultP |= GAMEPAD_DOWN;
    if (keyMask & keyBitPageUp)   *resultP |= GAMEPAD_UP;
    if (keyMask & keyBitLeft)     *resultP |= GAMEPAD_LEFT;
    if (keyMask & keyBitRight)    *resultP |= GAMEPAD_RIGHT;
    if (buttonMask & 0x01)        *resultP |= GAMEPAD_LEFTFIRE;
    if (buttonMask & 0x02)        *resultP |= GAMEPAD_RIGHTFIRE;

    if (pumpkin_extkey_down('w', extKeyMask)) *resultP |= GAMEPAD_UP;
    if (pumpkin_extkey_down('s', extKeyMask)) *resultP |= GAMEPAD_DOWN;
    if (pumpkin_extkey_down('a', extKeyMask)) *resultP |= GAMEPAD_LEFT;
    if (pumpkin_extkey_down('d', extKeyMask)) *resultP |= GAMEPAD_RIGHT;
    if (modMask & WINDOW_MOD_CTRL)  *resultP |= GAMEPAD_LEFTFIRE;
    if (modMask & WINDOW_MOD_RCTRL) *resultP |= GAMEPAD_RIGHTFIRE;

    // not mapped:
    // GAMEPAD_START
    // GAMEPAD_SELECT

    err = errNone;
  }

  return err;
}
