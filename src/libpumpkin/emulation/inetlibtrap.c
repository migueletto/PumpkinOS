#include <PalmOS.h>
#include <VFSMgr.h>
#include <INetMgr.h>

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

void palmos_inetlibtrap(uint16_t trap) {
  uint32_t sp;
  uint16_t idx;
  char buf[256];
  Err err;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
    case sysLibTrapOpen: {
      // Err INetLibOpen(UInt16 libRefnum, UInt16 config, UInt32 flags, DmOpenRef cacheRef, UInt32 cacheSize, MemHandle *inetHP)
      uint16_t refNum = ARG16;
      uint16_t config = ARG16;
      uint32_t flags = ARG32; // unused by PalmOS
      uint32_t cacheRefP = ARG32;
      uint32_t cacheSize = ARG32;
      uint32_t inetHP = ARG32;
      DmOpenRef cacheRef = (DmOpenRef)emupalmos_trap_in(cacheRefP, trap, 3);
      emupalmos_trap_in(inetHP, trap, 5);
      MemHandle inetH;
      err = INetLibOpen(refNum, config, flags, cacheRef, cacheSize, &inetH);
      if (inetHP) m68k_write_memory_32(inetHP, emupalmos_trap_out(inetH));
      debug(DEBUG_INFO, "EmuPalmOS", "INetLibOpen(refNum=%u, config=%u, flags=0x%08X, cacheRef=0x%08X, cacheSize=%u, inetHP=0x%08X): %d",
        refNum, config, flags, cacheRefP, cacheSize, inetHP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapClose: {
      // Err INetLibClose(UInt16 libRefnum, MemHandle inetH)
      uint16_t refNum = ARG16;
      uint32_t inetHP = ARG32;
      MemHandle inetH = emupalmos_trap_in(inetHP, trap, 1);
      err = INetLibClose(refNum, inetH);
      debug(DEBUG_INFO, "EmuPalmOS", "INetLibClose(refNum=%d, inetH=0x%08X): %d", refNum, inetHP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapSleep: {
      uint16_t refNum = ARG16;
      err = NetLibSleep(refNum);
      debug(DEBUG_INFO, "EmuPalmOS", "INetLibSleep(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapWake: {
      uint16_t refNum = ARG16;
      err = NetLibWake(refNum);
      debug(DEBUG_INFO, "EmuPalmOS", "INetLibWake(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case inetLibTrapSettingGet: {
      // Err INetLibSettingGet(UInt16 libRefnum, MemHandle inetH, UInt16 /*INetSettingEnum */ setting, void *bufP, UInt16 *bufLenP)
      uint16_t refNum = ARG16;
      uint32_t inetHP = ARG32;
      uint16_t setting = ARG16;
      uint32_t bufP = ARG32;
      uint32_t bufLenP = ARG32;
      MemHandle inetH = emupalmos_trap_in(inetHP, trap, 1);
      void *buf = emupalmos_trap_in(bufP, trap, 3);
      emupalmos_trap_in(bufLenP, trap, 4);
      UInt16 bufLen = 0; 
      if (bufLenP) bufLen = m68k_read_memory_16(bufLenP);
      err = INetLibSettingGet(refNum, inetH, setting, buf, &bufLen);
      if (bufLenP) m68k_write_memory_16(bufLenP, bufLen);
      debug(DEBUG_INFO, "EmuPalmOS", "INetLibSettingGet(refNum=%d, inetH=0x%08X, setting=%u, bufP=0x%08X, bufLenP=0x%08X): %d",
        refNum, inetHP, setting, bufP, bufLenP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case inetLibTrapSettingSet: {
      // Err INetLibSettingSet(UInt16 libRefnum, MemHandle inetH, UInt16 /*INetSettingEnum*/ setting, void *bufP, UInt16 bufLen)
      uint16_t refNum = ARG16;
      uint32_t inetHP = ARG32;
      uint16_t setting = ARG16;
      uint32_t bufP = ARG32;
      uint16_t bufLen = ARG16;
      MemHandle inetH = emupalmos_trap_in(inetHP, trap, 1);
      void *buf = emupalmos_trap_in(inetHP, trap, 3);
      err = INetLibSettingSet(refNum, inetH, setting, buf, bufLen);
      debug(DEBUG_INFO, "EmuPalmOS", "INetLibSettingSet(refNum=%d, inetH=0x%08X, setting=%u, bufP=0x%08X, bufLen=%u): %d",
        refNum, inetHP, setting, bufP, bufLen, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case inetLibTrapGetEvent: {
      // void INetLibGetEvent(UInt16 libRefnum, MemHandle inetH, INetEventType *eventP, Int32 timeout)
      uint16_t refNum = ARG16;
      uint32_t inetHP = ARG32;
      uint32_t eventP = ARG32;
      int32_t timeout = ARG32;
      MemHandle inetH = emupalmos_trap_in(inetHP, trap, 1);
      emupalmos_trap_in(eventP, trap, 2);
      INetEventType event;
      MemSet(&event, sizeof(INetEventType), 0);
      INetLibGetEvent(refNum, inetH, &event, timeout);
      char *eventName = EvtGetEventName(event.eType);
      if (eventName) {
        debug(DEBUG_INFO, "EmuPalmOS", "INetLibGetEvent(refNum=%d, inetH=0x%08X, eventP=0x%08X [%s], timeout=%d)",
          refNum, inetHP, eventP, eventName, timeout);
      } else {
        debug(DEBUG_INFO, "EmuPalmOS", "INetLibGetEvent(refNum=%d, inetH=0x%08X, eventP=0x%08X [0x%04X], timeout=%d)",
          refNum, inetHP, eventP, event.eType, timeout);
      }
      if (eventP) encode_event(eventP, (EventType *)&event);
      }
      break;
    case inetLibTrapWirelessIndicatorCmd: {
      // Boolean INetLibWirelessIndicatorCmd(UInt16 refNum, MemHandle inetH, UInt16 /*WiCmdEnum*/ cmd, int enableOrX, int y)
      uint16_t refNum = ARG16;
      uint32_t inetHP = ARG32;
      uint16_t cmd = ARG16;
      int16_t enableOrX = ARG16;
      int16_t y = ARG16;
      MemHandle inetH = emupalmos_trap_in(inetHP, trap, 1);
      Boolean res = INetLibWirelessIndicatorCmd(refNum, inetH, cmd, enableOrX, y);
      debug(DEBUG_INFO, "EmuPalmOS", "INetLibWirelessIndicatorCmd(refNum=%d, inetH=0x%08X, cmd=%d, enableOrX=%d, y=%d): %d",
        refNum, inetHP, cmd, enableOrX, y, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case inetLibTrapURLsAdd: {
      // Err INetLibURLsAdd(UInt16 libRefnum, Char *baseURLStr, Char *embeddedURLStr, Char *resultURLStr, UInt16 *resultLenP)
      uint16_t refNum = ARG16;
      uint32_t baseURLStrP = ARG32;
      uint32_t embeddedURLStrP = ARG32;
      uint32_t resultURLStrP = ARG32;
      uint32_t resultLenP = ARG32;
      char *baseURLStr = (char *)emupalmos_trap_in(baseURLStrP, trap, 1);
      char *embeddedURLStr = (char *)emupalmos_trap_in(embeddedURLStrP, trap, 2);
      char *resultURLStr = (char *)emupalmos_trap_in(resultURLStrP, trap, 3);
      emupalmos_trap_in(resultLenP, trap, 4);
      UInt16 resultLen = 0;
      if (resultLenP) resultLen = m68k_read_memory_16(resultLenP);
      err = INetLibURLsAdd(refNum, baseURLStr, embeddedURLStr, resultURLStr, &resultLen);
      if (resultLenP) m68k_write_memory_16(resultLenP, resultLen);
      debug(DEBUG_INFO, "EmuPalmOS", "INetLibURLsAdd(refNum=%d, base=\"%s\", embedded=\"%s\", result=\"%s\", resultLen=%u): %d",
        refNum, baseURLStr, embeddedURLStr, resultURLStr, resultLen, err);
      resultLenP = ARG32;
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case inetLibConfigIndexFromName: {
      // Err INetLibConfigIndexFromName(UInt16 refNum, INetConfigNamePtr nameP, UInt16 *indexP)
      // typedef struct {
      //   Char name[inetConfigNameSize];
      // } INetConfigNameType, *INetConfigNamePtr;
      uint16_t refNum = ARG16;
      uint32_t nameP = ARG32;
      uint32_t indexP = ARG32;
      char *name = (char *)emupalmos_trap_in(nameP, trap, 1);
      emupalmos_trap_in(indexP, trap, 2);
      INetConfigNameType configName;
      StrNCopy(configName.name, name, inetConfigNameSize-1);
      UInt16 index = 0;
      err = INetLibConfigIndexFromName(refNum, &configName, &index);
      if (err == errNone && indexP) m68k_write_memory_16(indexP, index);
      debug(DEBUG_INFO, "EmuPalmOS", "INetLibConfigIndexFromName(refNum=%d, name=\"%s\", index=%d): %d", refNum, name, index, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case inetLibTrapURLCrack: {
      // Err INetLibURLCrack(UInt16 libRefnum, UInt8 *urlTextP, INetURLType *urlP)
      uint16_t refNum = ARG16;
      uint32_t urlTextP = ARG32;
      uint32_t urlP = ARG32;
      uint8_t *urlText = (uint8_t *)emupalmos_trap_in(urlTextP, trap, 1);
      INetURLType url;
      decode_INetURLType(urlP, &url);
      err = INetLibURLCrack(refNum, urlText, &url);
      encode_INetURLType(urlP, &url);
      debug(DEBUG_INFO, "EmuPalmOS", "INetLibURLCrack(refNum=%d, urlText=\"%s\", urlP=0x%08X): %d", refNum, (char *)urlText, urlP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "INetLib trap 0x%04X (%u) not mapped", trap, trap - sysLibTrapCustom);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
