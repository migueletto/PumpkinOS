#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "xalloc.h"
#include "debug.h"

#define MAX_EVENTS 64

#define PALMOS_MODULE "Event"

#define TIMEOUT 50000

typedef struct {
  EventType events[MAX_EVENTS];
  EventType aux[MAX_EVENTS];
  int numEvents;
  int idxIn;
  int idxOut;
  int penDown;
  Coord screenX, screenY;
  Int32 needNullTickCount;
  Int32 lastPenMove;
  Boolean insideRepeatingButtonCtl;
  UInt32 repeatingButtonTime;
  UInt16 repeatingButtonID;
  void *repeatingButtonP;
  Boolean penMove;
} evt_module_t;

extern thread_key_t *evt_key;

#define lastRegularEvent attnIndicatorSelectEvent

static char *eventName[] = {
  "nilEvent",
  "penDownEvent",
  "penUpEvent",
  "penMoveEvent",
  "keyDownEvent",
  "winEnterEvent",
  "winExitEvent",
  "ctlEnterEvent",
  "ctlExitEvent",
  "ctlSelectEvent",
  "ctlRepeatEvent",
  "lstEnterEvent",
  "lstSelectEvent",
  "lstExitEvent",
  "popSelectEvent",
  "fldEnterEvent",
  "fldHeightChangedEvent",
  "fldChangedEvent",
  "tblEnterEvent",
  "tblSelectEvent",
  "daySelectEvent",
  "menuEvent",
  "appStopEvent",
  "frmLoadEvent",
  "frmOpenEvent",
  "frmGotoEvent",
  "frmUpdateEvent",
  "frmSaveEvent",
  "frmCloseEvent",
  "frmTitleEnterEvent",
  "frmTitleSelectEvent",
  "tblExitEvent",
  "sclEnterEvent",
  "sclExitEvent",
  "sclRepeatEvent",
  "tsmConfirmEvent",
  "tsmFepButtonEvent",
  "tsmFepModeEvent",
  "attnIndicatorEnterEvent",
  "attnIndicatorSelectEvent"
};

int EvtInitModule(void) {
  evt_module_t *module;

  if ((module = xcalloc(1, sizeof(evt_module_t))) == NULL) {
    return -1;
  }

  thread_set(evt_key, module);

  return 0;
}

int EvtFinishModule(void) {
  evt_module_t *module = (evt_module_t *)thread_get(evt_key);

  if (module) {
    xfree(module);
  }

  return 0;
}

void EvtReturnPenMove(Boolean penMove) {
  evt_module_t *module = (evt_module_t *)thread_get(evt_key);
  module->penMove = penMove;
}

char *EvtGetEventName(UInt16 eType) {
  if (eType <= lastRegularEvent) {
    return eventName[eType];
  }
  return NULL;
}

void EvtPrintEvent(char *op, EventType *event) {
  int level = DEBUG_TRACE;

  if (event->eType <= lastRegularEvent) {
    switch (event->eType) {
      case nilEvent:
      case penMoveEvent:
        break;
      default:
        debug(level, PALMOS_MODULE, "%s Event %s", op, eventName[event->eType]);
        break;
    }
  } else switch (event->eType) {
    case menuCmdBarOpenEvent: debug(level, PALMOS_MODULE, "%s Event %s", op, "menuCmdBarOpenEvent"); break;
    case menuOpenEvent: debug(level, PALMOS_MODULE, "%s Event %s", op, "menuOpenEvent"); break;
    case menuCloseEvent: debug(level, PALMOS_MODULE, "%s Event %s", op, "menuCloseEvent"); break;
    case frmGadgetEnterEvent: debug(level, PALMOS_MODULE, "%s Event %s", op, "frmGadgetEnterEvent"); break;
    case frmGadgetMiscEvent: debug(level, PALMOS_MODULE, "%s Event %s", op, "frmGadgetMiscEvent"); break;
    case keyUpEvent: debug(level, PALMOS_MODULE, "%s Event %s", op, "keyUpEven"); break;
    case keyHoldEvent: debug(level, PALMOS_MODULE, "%s Event %s", op, "keyHoldEvent"); break;
    case frmObjectFocusTakeEvent: debug(level, PALMOS_MODULE, "%s Event %s", op, "frmObjectFocusTakeEvent"); break;
    case frmObjectFocusLostEvent: debug(level, PALMOS_MODULE, "%s Event %s", op, "frmObjectFocusLostEvent"); break;
    case winDisplayChangedEvent: debug(level, PALMOS_MODULE, "%s Event %s", op, "winDisplayChangedEvent"); break;
    default:
      if (event->eType >= firstLicenseeEvent && event->eType <= lastLicenseeEvent) {
        debug(level, PALMOS_MODULE, "%s Event licenseeEvent %d", op, event->eType - firstLicenseeEvent);
      } else if (event->eType >= firstUserEvent && event->eType <= lastUserEvent) {
        debug(level, PALMOS_MODULE, "%s Event userEvent %d", op, event->eType - firstUserEvent);
      } else {
        debug(level, PALMOS_MODULE, "%s Event unknown %d (0x%04X)", op, event->eType, event->eType);
      }
      break;
  }
}

void EvtAddEventToQueue(const EventType *event) {
  evt_module_t *module = (evt_module_t *)thread_get(evt_key);

  if (event->eType == penUpEvent && module->insideRepeatingButtonCtl) {
    module->insideRepeatingButtonCtl = false;
  } else if (event->eType == ctlRepeatEvent && event->data.ctlRepeat.pControl->style == repeatingButtonCtl && !module->insideRepeatingButtonCtl) {
    module->insideRepeatingButtonCtl = true;
    module->repeatingButtonTime = event->data.ctlRepeat.time;
    module->repeatingButtonID = event->data.ctlRepeat.controlID;
    module->repeatingButtonP = event->data.ctlRepeat.pControl;
  }

  if (module->numEvents < MAX_EVENTS) {
    module->numEvents++;
    xmemcpy(&module->events[module->idxIn++], event, sizeof(EventType));
    EvtPrintEvent("Put", (EventType *)event);
    if (module->idxIn == MAX_EVENTS) module->idxIn = 0;
  } else {
    debug(DEBUG_ERROR, PALMOS_MODULE, "EvtAddEventToQueue event queue overflow");
  }
}

void EvtAddUniqueEventToQueue(const EventType *eventP, UInt32 id, Boolean inPlace) {
  evt_module_t *module = (evt_module_t *)thread_get(evt_key);
  int i, j;

  if (id == 0) {
    for (i = 0; i < module->numEvents; i++) {
      j = (module->idxOut + i) % MAX_EVENTS;
      if (module->events[j].eType == eventP->eType) {
        if (inPlace) {
          xmemcpy(&module->events[j], eventP, sizeof(EventType));
          break;
        } else {
          module->events[j].eType = nilEvent; // XXX not correct, event should be deleted
        }
      }
    }
    if (i == module->numEvents) {
      EvtAddEventToQueue(eventP);
    }
  } else {
    debug(DEBUG_ERROR, PALMOS_MODULE, "EvtAddUniqueEventToQueue id=%d not implemented", id);
  }
}

static void adjustCoords(Coord *x, Coord *y) {
  UInt32 density;
  WinHandle wh;
  Coord x0, y0;

   WinScreenGetAttribute(winScreenDensity, &density);

   switch (density) {
     case kDensityLow:
       switch (WinGetRealCoordinateSystem()) {
         case kCoordinatesDouble:    *x *= 2; *y *= 2; break;
         case kCoordinatesQuadruple: *x *= 4; *y *= 4; break;
       }
       break;
     case kDensityDouble:
       switch (WinGetRealCoordinateSystem()) {
         case kCoordinatesStandard:  *x /= 2; *y /= 2; break;
         case kCoordinatesQuadruple: *x *= 2; *y *= 2; break;
       }
       break;
     case kDensityQuadruple:
       switch (WinGetRealCoordinateSystem()) {
         case kCoordinatesStandard:  *x /= 4; *y /= 4; break;
         case kCoordinatesDouble:    *x /= 2; *y /= 2; break;
       }
       break;
   }

   if ((wh = WinGetActiveWindow()) != NULL) {
     WinGetPosition(wh, &x0, &y0);
     *x -= x0;
     *y -= y0;
   }
}

int EvtPumpEvents(Int32 timeoutUs) {
  evt_module_t *module = (evt_module_t *)thread_get(evt_key);
  EventType event;
  //notify_broadcast_t *nb;
  //SysNotifyProcPtr f;
  //char tbuf[8], bbuf[8];
  UInt8 buf[1024];
  UInt32 n;
  //UInt32 result;
  int32_t wait;
  uint64_t t0, t, dt;
  UInt32 ticks;
  int native;
  int ev, key, mods, buttons, forever, r = 0;

  t0 = sys_get_clock();
  MemSet(&event, sizeof(EventType), 0);

  if (module->needNullTickCount > 0 && TimGetTicks() > module->needNullTickCount) {
    debug(DEBUG_INFO, PALMOS_MODULE, "EvtPumpEvents needNullTickCount reached");
    module->needNullTickCount = 0;
    event.eType = nilEvent;
    EvtAddEventToQueue(&event);
    return 1;
  }

  if (timeoutUs < 0) {
    // wait forever
    wait = TIMEOUT;
    forever = 1;
  } else if (timeoutUs == 0) {
    // no wait
    wait = 0;
    forever = 0;
  } else if (timeoutUs <= TIMEOUT) {
    // wait at most timeoutUs
    wait = timeoutUs;
    forever = 0;
  } else {
    // wait at most TIMEOUT us each time
    wait = TIMEOUT;
    forever = 0;
  }

  for (; !thread_must_end() && !pumpkin_must_finish();) {
    InsPtCheckBlink();
    n = sizeof(buf);
    ev = pumpkin_event(&key, &mods, &buttons, buf, &n, wait);
    if (ev == -1) {
      debug(DEBUG_ERROR, PALMOS_MODULE, "EvtPumpEvents pumpkin_event failed");
      return -1;
    }

    if (ev == 0 && module->insideRepeatingButtonCtl) {
      ticks = TimGetTicks();
      if ((ticks - module->repeatingButtonTime) >= SysTicksPerSecond() / 2) {
        module->repeatingButtonTime = ticks;
        event.eType = ctlRepeatEvent;
        event.data.ctlRepeat.controlID = module->repeatingButtonID;
        event.data.ctlRepeat.pControl = (ControlType *)module->repeatingButtonP;
        event.data.ctlRepeat.time = ticks;
        EvtAddEventToQueue(&event);
        return 1;
      }
    }

    if (ev == MSG_KEYDOWN || ev == MSG_KEYUP) {
      if (key != WINDOW_KEY_SHIFT && key != WINDOW_KEY_CTRL && key != WINDOW_KEY_LALT) continue;
    }

    if (ev == MSG_MOTION) {
      module->screenX = key;
      module->screenY = mods;
      adjustCoords(&module->screenX, &module->screenY);
      if (!module->penDown && !module->penMove) {
        ev = 0;
      } else {
        ticks = TimGetTicks();
        if (module->lastPenMove == 0 || (ticks - module->lastPenMove) > 0) {
          module->lastPenMove = ticks;
        } else {
          ev = 0;
        }
      }

/*
    } else if (ev == MSG_NOTIFY) {
      // notifications
      nb = (notify_broadcast_t *)buf;
      if (nb && nb->notify) {
        pumpkin_id2s(nb->notify->notifyType, tbuf);
        pumpkin_id2s(nb->notify->broadcaster, bbuf);
        debug(DEBUG_INFO, PALMOS_MODULE, "EvtPumpEvents notification '%s' from '%s'", tbuf, bbuf);
        if (nb->taskId == pumpkin_get_taskid()) {
          if (nb->callback) {
            debug(DEBUG_INFO, PALMOS_MODULE, "EvtPumpEvents notification callback %p", nb->callback);
            f = (SysNotifyProcPtr)nb->callback;
            f(nb->notify);
          } else if (nb->callback_m68k) {
            debug(DEBUG_INFO, PALMOS_MODULE, "EvtPumpEvents notification 68K callback 0x%08X", nb->callback_m68k);
            CallNotifyProc(nb->callback_m68k, nb->notify);
          } else {
            debug(DEBUG_INFO, PALMOS_MODULE, "EvtPumpEvents send sysAppLaunchCmdNotify to dbID 0x%08X", nb->dbID);
            SysAppLaunch(nb->cardNo, nb->dbID, 0, sysAppLaunchCmdNotify, nb->notify, &result);
          }
        } else {
          debug(DEBUG_ERROR, PALMOS_MODULE, "EvtPumpEvents notification to wrong task %u != %u", nb->taskId, pumpkin_get_taskid());
        }
      }
      ev = 0;
*/
    }

    if (ev) break;
    if (forever) continue;
    t = sys_get_clock();
    dt = t - t0;
    if (timeoutUs <= dt) break;
    timeoutUs -= dt;
    t0 = t;
    wait = (timeoutUs < TIMEOUT) ? timeoutUs : TIMEOUT;
  }

  //event->tapCount = ???; // XXX

  switch (ev) {
    case MSG_KEYDOWN:
    case MSG_KEYUP:
      switch (key) {
        case WINDOW_KEY_SHIFT: event.data.keyDown.modifiers |= shiftKeyMask; break;
        case WINDOW_KEY_CTRL:  event.data.keyDown.modifiers |= controlKeyMask; break;
        case WINDOW_KEY_LALT:  event.data.keyDown.modifiers |= optionKeyMask; break;
      }
      if (event.data.keyDown.modifiers) {
        event.eType = ev == MSG_KEYDOWN ? modKeyDownEvent : modKeyUpEvent;
        EvtAddEventToQueue(&event);
      }
      break;
    case MSG_KEY:
      native = pumpkin_get_native_keys();

      switch (key) {
        case WINDOW_KEY_F1:
          key = vchrHard1;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_F2:
          key = vchrHard2;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_F3:
          key = vchrHard3;
           event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_F4:
          key = vchrHard4;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_F5:
          if (native) {
            key = vchrHard5;
          } else {
            key = vchrMenu;
          }
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_F6:
          key = vchrHard6;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_F7:
          key = vchrHard7;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_F8:
          key = vchrHard8;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_F9:
          key = vchrHard9;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_F10:
          key = vchrHard10;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_UP:
          key = vchrPageUp;
          break;
        case WINDOW_KEY_DOWN:
          key = vchrPageDown;
          break;
        case WINDOW_KEY_LEFT:
          key = vchrRockerLeft;
          break;
        case WINDOW_KEY_RIGHT:
          key = vchrRockerRight;
          break;
        case WINDOW_KEY_INS:
          key = vchrNativeInsert;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_DEL:
          key = vchrNativeDelete;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_HOME:
          if (native) {
            key = vchrNativeHome;
            event.data.keyDown.modifiers |= commandKeyMask;
          } else {
            debug(DEBUG_INFO, PALMOS_MODULE, "EvtPumpEvents keyDownEvent vchrLaunch");
            EvtEnqueueKey(vchrLaunch, 0, commandKeyMask);
            return 1;
          }
          break;
        case WINDOW_KEY_END:
          key = vchrNativeEnd;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_PGUP:
          key = vchrNativePgUp;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_PGDOWN:
          key = vchrNativePgDown;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
        case WINDOW_KEY_CUSTOM:
          key = mods;
          event.data.keyDown.modifiers |= commandKeyMask;
          break;
      }

      event.eType = keyDownEvent;
      if (key == 13) key = 10;
      event.data.keyDown.chr = key;   // ascii code
      event.data.keyDown.keyCode = 0; // XXX virtual key code
      if (mods & WINDOW_MOD_SHIFT) event.data.keyDown.modifiers |= shiftKeyMask;
      if (mods & WINDOW_MOD_CTRL)  event.data.keyDown.modifiers |= controlKeyMask;
      if (mods & WINDOW_MOD_LALT)  event.data.keyDown.modifiers |= optionKeyMask;
      EvtAddEventToQueue(&event);
      r = 1;
      break;

    case MSG_BUTTON:
      event.screenX = module->screenX;
      event.screenY = module->screenY;

      if ((buttons & 0x03)) {
        module->penDown = 1;
        event.eType = (buttons == 1) ? penDownEvent : penDownRightEvent;
        event.penDown = true;
        EvtAddEventToQueue(&event);
        r = 1;
      } else {
        if (module->penDown) {
          module->penDown = 0;
          FrmTrackPenUp(module->screenX, module->screenY);
          event.eType = penUpEvent;
          // Display-relative start point of the stroke.
          event.data.penUp.start.x = event.screenX;
          event.data.penUp.start.y = event.screenY;
          // Display-relative end point of the stroke.
          event.data.penUp.end.x = event.data.penUp.start.x; // XXX
          event.data.penUp.end.y = event.data.penUp.start.y; // XXX
          EvtAddEventToQueue(&event);
          r = 1;
        }
      }
      break;

    case MSG_MOTION:
      event.eType = penMoveEvent;
      event.screenX = module->screenX;
      event.screenY = module->screenY;
      event.penDown = module->penDown;
      EvtAddEventToQueue(&event);
      r = 1;
      break;

    case MSG_USER:
      if (n <= sizeof(EventType)) {
        sys_memcpy(&event, buf, n);
        EvtAddEventToQueue(&event);
        r = 1;
      }
      break;
  }

  return r;
}

void EvtEmptyQueue(void) {
  evt_module_t *module = (evt_module_t *)thread_get(evt_key);
  int i, j;

  for (i = 0; i < module->numEvents; i++) {
    j = (module->idxOut + i) % MAX_EVENTS;
    debug(DEBUG_TRACE, "Event", "Event %d/%d: EvtEmptyQueue %s", i+1, module->numEvents, EvtGetEventName(module->events[j].eType));
  }

  module->numEvents = 0;
  module->idxIn = 0;
  module->idxOut = 0;
}

Err EvtFlushPenQueue(void) {
  evt_module_t *module = (evt_module_t *)thread_get(evt_key);
  int i, j, k;

  for (i = 0, k = 0; i < module->numEvents; i++) {
    j = (module->idxOut + i) % MAX_EVENTS;
    switch (module->events[j].eType) {
      case penDownEvent:
      case penUpEvent:
      case penMoveEvent:
        break;
      default:
        MemMove(&module->aux[k++], &module->events[j], sizeof(EventType));
        break;
    }
  }

  debug(DEBUG_TRACE, PALMOS_MODULE, "EvtFlushPenQueue flushed %d event(s), %d remaining", module->numEvents - k, k);
  if (module->numEvents > k) {
    MemMove(&module->events[0], &module->aux[0], k*sizeof(EventType));
    module->numEvents = k;
    module->idxOut = 0;
    module->idxIn = k;
  }

  return errNone;
}

// Return true if a low-level system event (such as a pen or key event) is available.
Boolean EvtSysEventAvail(Boolean ignorePenUps) {
  evt_module_t *module = (evt_module_t *)thread_get(evt_key);
  int i, j;

  for (;;) {
    if (EvtPumpEvents(0) != 1) break;
  }

  for (i = 0; i < module->numEvents; i++) {
    j = (module->idxOut + i) % MAX_EVENTS;

    switch (module->events[j].eType) {
      case penUpEvent:
        if (!ignorePenUps) {
          debug(DEBUG_TRACE, PALMOS_MODULE, "EvtSysEventAvail %s available", eventName[module->events[j].eType]);
          return true;
        }
        break;
      case penDownEvent:
      case penMoveEvent:
      case keyDownEvent:
        debug(DEBUG_TRACE, PALMOS_MODULE, "EvtSysEventAvail %s available", eventName[module->events[j].eType]);
        return true;
     default:
        break;
    }
  }

  return false;
}

Boolean EvtEventAvail(void) {
  evt_module_t *module = (evt_module_t *)thread_get(evt_key);

  for (;;) {
    if (EvtPumpEvents(0) != 1) break;
  }

  return module->numEvents > 0;
}

// timeout: time in us to wait before an event is returned (evtWaitForever means wait indefinitely).
void EvtGetEventUs(EventType *event, Int32 timeoutUs) {
  evt_module_t *module = (evt_module_t *)thread_get(evt_key);

  MemSet(event, sizeof(EventType), 0);
  EvtPumpEvents(0);

  if (module->numEvents == 0) {
    if (EvtPumpEvents(timeoutUs) == -1) {
      debug(DEBUG_INFO, PALMOS_MODULE, "EvtGetEvent appStopEvent (event failed)");
      event->eType = appStopEvent;
      EvtPrintEvent("Get", event);
      return;
    }
  }

  if (thread_must_end()) {
    debug(DEBUG_INFO, PALMOS_MODULE, "EvtGetEvent appStopEvent (thread_must_end)");
    event->eType = appStopEvent;
    EvtPrintEvent("Get", event);
    return;
  }

  if (pumpkin_must_finish()) {
    debug(DEBUG_INFO, PALMOS_MODULE, "EvtGetEvent appStopEvent (pumpkin_must_finish)");
    event->eType = appStopEvent;
    EvtPrintEvent("Get", event);
    return;
  }

  if (module->numEvents > 0) {
    // get the next event
    xmemcpy(event, &module->events[module->idxOut++], sizeof(EventType));
    if (module->idxOut == MAX_EVENTS) module->idxOut = 0;
    module->numEvents--;
    EvtPrintEvent("Get", event);
    return;
  }

  event->eType = nilEvent;
  EvtPrintEvent("Get", event);
}

// timeout: maximum number of ticks to wait before an event is returned (evtWaitForever means wait indefinitely).
void EvtGetEvent(EventType *event, Int32 timeout) {
  // 1 tick = 10 ms = 10000 us
  //uint64_t t = sys_get_clock();
  EvtGetEventUs(event, timeout * 10000);
  //t = sys_get_clock() - t;
  //char *s = event->eType <= lastRegularEvent ? eventName[event->eType] : "unknown";
  //debug(DEBUG_INFO, PALMOS_MODULE, "EvtGetEvent(%d): %s (%d) in %d us", timeout, s, event->eType, (int32_t)t);

//debug(1, "XXX", "EvtGetEvent offset eType    = %d", OffsetOf(EventType, eType));
//debug(1, "XXX", "EvtGetEvent offset penDown  = %d", OffsetOf(EventType, penDown));
//debug(1, "XXX", "EvtGetEvent offset tapCount = %d", OffsetOf(EventType, tapCount));
//debug(1, "XXX", "EvtGetEvent offset screenX  = %d", OffsetOf(EventType, screenX));
//debug(1, "XXX", "EvtGetEvent offset screenY  = %d", OffsetOf(EventType, screenY));
//debug(1, "XXX", "EvtGetEvent offset data     = %d", OffsetOf(EventType, data));
  SysNotifyBroadcastQueued();
}

Err EvtEnqueueKey(WChar ascii, UInt16 keycode, UInt16 modifiers) {
  EventType event;

  xmemset(&event, 0, sizeof(EventType));
  event.eType = keyDownEvent;
  event.data.keyDown.chr = ascii;
  event.data.keyDown.keyCode = keycode;
  event.data.keyDown.modifiers = modifiers;
  EvtAddEventToQueue(&event);

  return 0;
}

void EvtCopyEvent(const EventType *source, EventType *dest) {
  if (dest && source) {
    MemMove(dest, source, sizeof(EventType));
  }
}

void EvtGetPenEx(Int16 *pScreenX, Int16 *pScreenY, Boolean *pPenDown, Boolean *pRight) {
  int x, y;
  uint32_t buttonMask;

  pumpkin_status(&x, &y, NULL, NULL, &buttonMask, NULL);
  *pScreenX = x;
  *pScreenY = y;
  *pPenDown = (buttonMask & 3) ? true : false;
  if (pRight) *pRight = (buttonMask & 2) ? true : false;
}

void EvtGetPen(Int16 *pScreenX, Int16 *pScreenY, Boolean *pPenDown) {
  EvtGetPenEx(pScreenX, pScreenY, pPenDown, NULL);
  adjustCoords(pScreenX, pScreenY);
}

Boolean EvtSetNullEventTick(UInt32 tick) {
  evt_module_t *module = (evt_module_t *)thread_get(evt_key);
  Boolean r = false;

  if (module->needNullTickCount == 0 || module->needNullTickCount > tick || module->needNullTickCount <= TimGetTicks()) {
    debug(DEBUG_INFO, PALMOS_MODULE, "EvtSetNullEventTick %d", tick);
    module->needNullTickCount = tick;
    r = true;
  }

  return r;
}
