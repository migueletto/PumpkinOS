#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>
#include <INetMgr.h>

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

void palmos_TimSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapTimSetSeconds: {
      // void TimSetSeconds(UInt32 seconds)
      uint32_t seconds = ARG32;
      TimSetSeconds(seconds);
      debug(DEBUG_TRACE, "EmuPalmOS", "TimSetSeconds(%u)", seconds);
    }
    break;
    case sysTrapTimAdjust: {
      // void TimAdjust(DateTimeType *dateTimeP, Int32 adjustment)
      uint32_t dateTimeP = ARG32;
      int32_t adjustment = ARG32;
      emupalmos_trap_in(dateTimeP, trap, 0);
      DateTimeType dateTime;
      decode_datetime(dateTimeP, &dateTime);
      TimAdjust(&dateTime, adjustment);
      encode_datetime(dateTimeP, &dateTime);
      debug(DEBUG_TRACE, "EmuPalmOS", "TimAdjust(0x%08X, %d)", dateTimeP, adjustment);
    }
    break;
    case sysTrapTimDateTimeToSeconds: {
      // UInt32 TimDateTimeToSeconds(const DateTimeType *dateTimeP)
      uint32_t dateTimeP = ARG32;
      emupalmos_trap_in(dateTimeP, trap, 0);
      DateTimeType dateTime;
      decode_datetime(dateTimeP, &dateTime);
      UInt32 seconds = TimDateTimeToSeconds(&dateTime);
      debug(DEBUG_TRACE, "EmuPalmOS", "TimDateTimeToSeconds(0x%08X [%04d-%02d-%02d %02d:%02d:%02d]): %u", dateTimeP, dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second, seconds);
      m68k_set_reg(M68K_REG_D0, seconds);
    }
    break;
    case sysTrapTimSecondsToDateTime: {
      // void TimSecondsToDateTime(UInt32 seconds, DateTimeType *dateTimeP)
      uint32_t seconds = ARG32;
      uint32_t dateTimeP = ARG32;
      emupalmos_trap_in(dateTimeP, trap, 1);
      DateTimeType dateTime;
      TimSecondsToDateTime(seconds, &dateTime);
      encode_datetime(dateTimeP, &dateTime);
      debug(DEBUG_TRACE, "EmuPalmOS", "TimSecondsToDateTime(%u, 0x%08X [%04d-%02d-%02d %02d:%02d:%02d])", seconds, dateTimeP, dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second);
    }
    break;
    case sysTrapTimGetSeconds: {
      // UInt32 TimGetSeconds(void)
      UInt32 t = TimGetSeconds();
      debug(DEBUG_TRACE, "EmuPalmOS", "TimGetSeconds(): %u", t);
      m68k_set_reg(M68K_REG_D0, t);
    }
    break;
    case sysTrapTimGetTicks: {
      // UInt32 TimGetTicks(void)
      UInt32 t = TimGetTicks();
      debug(DEBUG_TRACE, "EmuPalmOS", "TimGetTicks(): %u", t);
      m68k_set_reg(M68K_REG_D0, t);
    }
    break;
  }
}
