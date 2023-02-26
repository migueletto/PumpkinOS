#include <PalmOS.h>

#include "sys.h"
#include "pumpkin.h"
#include "debug.h"

UInt16 PINGetInputAreaState(void) {
  UInt16 r;

  switch (pumpkin_dia_get_state()) {
    case  0:
      // The dynamic input area is not being displayed.
      r = pinInputAreaClosed;
      break;
    case  1:
      // The dynamic input area is being displayed.
      r = pinInputAreaOpen;
      break;
    default:
      // The input area is not dynamic, or there is no input area.
      r = pinInputAreaNone;
      break;
  }

  return r;
}

Err PINSetInputAreaState(UInt16 state) {
  Err err = pinErrNoSoftInputArea;

  // After opening or closing the input area, this function broadcasts the notification sysNotifyDisplayResizedEvent
  // (and in Pen Input Manager version 1.1, posts the event winDisplayChangedEvent to the event queue).
  // Applications register for this notification or respond to the event if they wish to resize themselves.

  switch (state) {
    case pinInputAreaOpen:
      if (pumpkin_dia_get_state() == 0) {
        if (pumpkin_dia_set_state(1) == 0) {
          err = errNone;
        }
      }
      break;
    case pinInputAreaClosed:
      if (pumpkin_dia_get_state() == 1) {
        if (pumpkin_dia_set_state(0) == 0) {
          err = errNone;
        }
      }
      break;
    case pinInputAreaUser:
      // lets the user decide whether the input area is open or closed, in response to the frmOpenEvent
      if (pumpkin_dia_set_state(2) == 0) {
        err = errNone;
      }
      break;
    default:
      err = pinErrInvalidParam;
      break;
  }

  return err;
}

UInt16 PINGetInputTriggerState(void) {
  UInt16 r;

  switch (pumpkin_dia_get_trigger()) {
    case  0:
      // The input trigger is enabled, meaning that the user is allowed to open and close the dynamic input area.
      r = pinInputTriggerDisabled;
      break;
    case  1:
      // The input trigger is disabled, meaning that the user is not allowed to close the dynamic input area.
      r = pinInputTriggerEnabled;
      break;
    default:
      // There is no dynamic input area.
      r = pinInputTriggerNone;
      break;
  }

  return r;
}

Err PINSetInputTriggerState(UInt16 state) {
  Err err = pinErrNoSoftInputArea;

  // Applications or Palm OS call this function to enable the input area icon in the status bar.
  // Normally, this trigger is enabled and should remain enabled, allowing the user the choice of
  // displaying the input area or not. Legacy applications might disable the trigger on some devices. 

  switch (state) {
    case pinInputTriggerEnabled:
      if (pumpkin_dia_set_trigger(1) == 0) err = errNone;
      break;
    case pinInputTriggerDisabled:
      if (pumpkin_dia_set_trigger(0) == 0) err = errNone;
      break;
    default:
      err = pinErrInvalidParam;
      break;
  }

  return err;
}

Err StatGetAttribute(UInt16 selector, UInt32 *dataP) {
  int width, height;
  Coord w, h;
  Err err = sysErrParamErr;

  switch (selector) {
    case statAttrBarVisible:
      // control bar is always visible
      *dataP = 1;
      err = errNone;
      break;
    case statAttrDimension:
      // gets the control bar bounds. The return data is two UInt16 values,
      // where the first is the width of the control bar and the second is the height.
      // The dimensions use the active coordinate system
      if (pumpkin_dia_get_taskbar_dimension(&width, &height) == 0) {
        WinAdjustCoordsInv(&w, &h);
        *dataP = (w << 16) | h;
        err = errNone;
      }
      break;
  }

  return err;
}

Err StatHide(void) {
  return sysErrNotAllowed;
}

Err StatShow(void) {
  return sysErrNotAllowed;
}
