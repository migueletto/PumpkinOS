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

void palmos_FtrSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapFtrPtrNew: {
      // Err FtrPtrNew(UInt32 creator, UInt16 featureNum, UInt32 size, void **newPtrP)
      uint32_t creator = ARG32;
      uint16_t featureNum = ARG16;
      uint32_t size = ARG32;
      uint32_t newPtrP = ARG32;
      emupalmos_trap_in(newPtrP, trap, 3);
      uint8_t *p = MemPtrNew(size);
      Err err;
      if (p) {
        uint32_t a = emupalmos_trap_out(p);
        if (newPtrP) m68k_write_memory_32(newPtrP, a);
        err = FtrSet(creator, featureNum, a);
      } else {
        err = memErrNotEnoughSpace;
      }
      char screator[8];
      pumpkin_id2s(creator, screator);
      debug(DEBUG_TRACE, "EmuPalmOS", "FtrPtrNew('%s', %d, %d, 0x%08X): %d", screator, featureNum, size, newPtrP, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapFtrPtrFree: {
      // Err FtrPtrFree(UInt32 creator, UInt16 featureNum)
      uint32_t creator = ARG32;
      uint16_t featureNum = ARG16;
      uint32_t a;
      Err err = FtrGet(creator, featureNum, &a);
      if (err == errNone && a) {
        uint8_t *p = emupalmos_trap_in(a, trap, -1);
        MemPtrFree(p);
      }
      char screator[8];
      pumpkin_id2s(creator, screator);
      debug(DEBUG_TRACE, "EmuPalmOS", "FtrPtrFree('%s', %d): %d", screator, featureNum, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapFtrUnregister: {
      // Err FtrUnregister(UInt32 creator, UInt16 featureNum)
      uint32_t creator = ARG32;
      uint16_t featureNum = ARG16;
      Err err = FtrUnregister(creator, featureNum);
      char screator[8];
      pumpkin_id2s(creator, screator);
      debug(DEBUG_TRACE, "EmuPalmOS", "FtrUnregister('%s', %d): %d", screator, featureNum, err);
    }
    break;
    case sysTrapFtrGet: {
      // Err FtrGet(UInt32 creator, UInt16 featureNum, UInt32 *valueP)
      uint32_t creator = ARG32;
      uint16_t featureNum = ARG16;
      uint32_t valueP = ARG32;
      emupalmos_trap_in(valueP, trap, 2);
      uint32_t value;
      char screator[8];
      pumpkin_id2s(creator, screator);
      Err err = FtrGet(creator, featureNum, &value);
    
      if (creator == sysFileCSystem && featureNum == sysFtrNumProcessorID && err == errNone) {
    #ifdef ARMEMU
        // If the processor is 68K, Cubis writes directly to the display bitmap. It works ONLY if the display is 8bpp.
        //value = sysFtrNumProcessorEZ;
    
        // If the processor is ARM, Cubis does not write directly to the display bitmap. It works both on 8pp and 16bpp. No hooks are necessary.
        value = sysFtrNumProcessorARM720T;
    #else
        value = sysFtrNumProcessorEZ;
    #endif
    }
    
      debug(DEBUG_TRACE, "EmuPalmOS", "FtrGet('%s', %d, 0x%08X [0x%08X]): %d", screator, featureNum, valueP, value, err);
      m68k_write_memory_32(valueP, value);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapFtrSet: {
      // Err FtrSet(UInt32 creator, UInt16 featureNum, UInt32 newValue)
      uint32_t creator = ARG32;
      uint16_t featureNum = ARG16;
      uint32_t newValue = ARG32;
      char screator[8];
      pumpkin_id2s(creator, screator);
      Err err = FtrSet(creator, featureNum, newValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "FtrSet('%s', %d, %d): %d", screator, featureNum, newValue, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
  }
}
