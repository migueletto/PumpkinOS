#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "mutex.h"
#include "storage.h"
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_EvtSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapEvtEnableGraffiti: {
      // void EvtEnableGraffiti(Boolean enable)
      uint8_t enable = ARG8;
      EvtEnableGraffiti(enable);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtEnableGraffiti(%d)", enable);
    }
    break;
    case sysTrapEvtResetAutoOffTimer: {
      // Err EvtResetAutoOffTimer(void)
      Err err = EvtResetAutoOffTimer();
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtResetAutoOffTimer(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapEvtAddUniqueEventToQueue: {
      // void EvtAddUniqueEventToQueue(const EventType *eventP, UInt32 id, Boolean inPlace)
      uint32_t eventP = ARG32;
      uint32_t id = ARG32;
      uint8_t inPlace = ARG8;
      emupalmos_trap_in(eventP, trap, 0);
      EventType event;
      if (eventP) decode_event(eventP, &event);
      EvtAddUniqueEventToQueue(eventP ? &event : NULL, id, inPlace);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtAddUniqueEventToQueue(0x%08X [0x%04X], %d, %d)", eventP, event.eType, id, inPlace);
    }
    break;
    case sysTrapEvtAddEventToQueue: {
      // void EvtAddEventToQueue(const EventType *event)
      uint32_t eventP = ARG32;
      uint8_t *p = emupalmos_trap_in(eventP, trap, 0);
      debug(DEBUG_INFO, "EmuPalmOS", "EvtAddEventToQueue event:");
      debug_bytes(DEBUG_INFO, "EmuPalmOS", p, 24);
      EventType event;
      if (eventP) decode_event(eventP, &event);
      EvtAddEventToQueue(eventP ? &event : NULL);
      char *eventName = EvtGetEventName(event.eType);
      if (eventName) {
        debug(DEBUG_TRACE, "EmuPalmOS", "EvtAddEventToQueue(0x%08X [%s])", eventP, eventName);
      } else {
        debug(DEBUG_TRACE, "EmuPalmOS", "EvtAddEventToQueue(0x%08X [0x%04X])", eventP, event.eType);
      }
      if (eventP) encode_event(eventP, &event);
    }
    break;
    case sysTrapEvtEnqueueKey: {
      // Err EvtEnqueueKey(WChar ascii, UInt16 keycode, UInt16 modifiers)
      uint16_t ascii = ARG16;
      uint16_t keycode = ARG16;
      uint16_t modifiers = ARG16;
      Err err = EvtEnqueueKey(ascii, keycode, modifiers);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtEnqueueKey(0x%04X, 0x%04X, 0x%04X): %d", ascii, keycode, modifiers, err);
    }
    break;
    case sysTrapEvtEventAvail: {
      // Boolean EvtEventAvail(void)
      Boolean res = EvtEventAvail();
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtEventAvail(): %d", res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapEvtWakeup: {
      // Err EvtWakeup(void)
      Err err = EvtWakeup();
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtWakeup(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapEvtGetEvent: {
      // void EvtGetEvent(EventType *event, Int32 timeout)
      uint32_t eventP = ARG32;
      int32_t timeout = ARG32;
      timeout = pumpkin_event_timeout(timeout);
      emupalmos_trap_in(eventP, trap, 0);
      EventType event;
      MemSet(&event, sizeof(EventType), 0);
      EvtGetEvent(eventP ? &event : NULL, timeout);
      char *eventName = EvtGetEventName(event.eType);
      if (eventName) {
        debug(DEBUG_TRACE, "EmuPalmOS", "EvtGetEvent(0x%08X [%s], %d)", eventP, eventName, timeout);
      } else {
        debug(DEBUG_TRACE, "EmuPalmOS", "EvtGetEvent(0x%08X [0x%04X], %d)", eventP, event.eType, timeout);
      }
      if (eventP) encode_event(eventP, &event);
    }
    break;
    case sysTrapEvtCopyEvent: {
      // void EvtCopyEvent(const EventType *source, EventType *dest)
      uint32_t sourceP = ARG32;
      uint32_t destP = ARG32;
      emupalmos_trap_in(sourceP, trap, 0);
      emupalmos_trap_in(destP, trap, 1);
      EventType source, dest;
      if (sourceP) decode_event(sourceP, &source);
      EvtCopyEvent(&source, &dest);
      if (destP) encode_event(destP, &dest);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtCopyEvent(0x%08X [0x%04X], 0x%08X)", sourceP, source.eType, destP);
    }
    break;
    case sysTrapEvtGetPen: {
      // void EvtGetPen(Int16 *pScreenX, Int16 *pScreenY, Boolean *pPenDown)
      uint32_t pScreenX = ARG32;
      uint32_t pScreenY = ARG32;
      uint32_t pPenDown = ARG32;
      emupalmos_trap_in(pScreenX, trap, 0);
      emupalmos_trap_in(pScreenY, trap, 1);
      emupalmos_trap_in(pPenDown, trap, 2);
      Int16 screenX, screenY;
      Boolean penDown;
      EvtGetPen(pScreenX ? &screenX : NULL, pScreenY ? &screenY : NULL, pPenDown ? &penDown : NULL);
      if (pScreenX) m68k_write_memory_16(pScreenX, screenX);
      if (pScreenY) m68k_write_memory_16(pScreenY, screenY);
      if (pPenDown) m68k_write_memory_8(pPenDown, penDown);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtGetPen(0x%08X, 0x%08X, 0x%08X)", pScreenX, pScreenY, pPenDown);
    }
    break;
    case sysTrapEvtSysEventAvail: {
      // Boolean EvtSysEventAvail(Boolean ignorePenUps)
      Boolean ignorePenUps = ARG8;
      Boolean res = EvtSysEventAvail(ignorePenUps);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtSysEventAvail(): %d", res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapEvtFlushKeyQueue: {
      // Err EvtFlushKeyQueue(void)
      Err err = EvtFlushKeyQueue();
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtFlushKeyQueue(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapEvtFlushPenQueue: {
      // Err EvtFlushPenQueue(void)
      Err err = EvtFlushPenQueue();
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtFlushPenQueue(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapEvtSetNullEventTick: {
      // Boolean EvtSetNullEventTick(UInt32 tick)
      uint32_t tick = ARG32;
      Boolean res = EvtSetNullEventTick(tick);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtSetNullEventTick(%u): %d", tick, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapEvtFlushNextPenStroke: {
      // Err EvtFlushNextPenStroke(void)
      Err err = EvtFlushNextPenStroke();
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtFlushNextPenStroke(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapEvtKeyQueueEmpty: {
      // Boolean EvtKeyQueueEmpty(void)
      Boolean res = EvtKeyQueueEmpty();
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtKeyQueueEmpty(): %d", res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapEvtGetSilkscreenAreaList:
    case sysTrapEvtGetPenBtnList: {
      // const SilkscreenAreaType *EvtGetSilkscreenAreaList(UInt16* numAreas)
      // const PenBtnInfoType *EvtGetPenBtnList(UInt16* numButtons)
      uint32_t numP = ARG32;
      emupalmos_trap_in(numP, trap, 0);
      if (numP) m68k_write_memory_16(numP, 0);
      m68k_set_reg(M68K_REG_A0, 0);
    }
    break;
  }
}
