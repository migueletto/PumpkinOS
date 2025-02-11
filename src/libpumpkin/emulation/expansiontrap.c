#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

/*
#define expCapabilityHasStorage   0x00000001  // card supports reading (& maybe writing) sectors
#define expCapabilityReadOnly     0x00000002  // card is read only
#define expCapabilitySerial       0x00000004  // card supports dumb serial interface

#define expCardInfoStringMaxLen   31

typedef struct {
  UInt32  capabilityFlags;  // bits for different stuff the card supports
  Char    manufacturerStr[expCardInfoStringMaxLen+1]; // Manufacturer, e.g., "Palm", "Motorola", etc...
  Char    productStr[expCardInfoStringMaxLen+1];      // Name of product, e.g., "SafeBackup 32MB"
  Char    deviceClassStr[expCardInfoStringMaxLen+1];  // Type of product, e.g., "Backup", "Ethernet", etc.
  Char    deviceUniqueIDStr[expCardInfoStringMaxLen+1];// Unique identifier for product, e.g., a serial number.  Set to "" if no such identifier exists.
} ExpCardInfoType;
*/

static void encode_expcardinfo(uint32_t infoP, ExpCardInfoType *info) {
  int i;

  if (infoP && info) {
    m68k_write_memory_32(infoP +  0, info->capabilityFlags);

    for (i = 0; info->manufacturerStr[i]; i++) {
      m68k_write_memory_8(infoP + 4 + i, info->manufacturerStr[i]);
    }
    m68k_write_memory_8(infoP + 4 + i, 0);

    for (i = 0; info->productStr[i]; i++) {
      m68k_write_memory_8(infoP + 4 + expCardInfoStringMaxLen + 1 + i, info->productStr[i]);
    }
    m68k_write_memory_8(infoP + 4 + i, 0);

    for (i = 0; info->deviceClassStr[i]; i++) {
      m68k_write_memory_8(infoP + 4 + 2 * (expCardInfoStringMaxLen + 1) + i, info->deviceClassStr[i]);
    }
    m68k_write_memory_8(infoP + 4 + i, 0);

    for (i = 0; info->deviceUniqueIDStr[i]; i++) {
      m68k_write_memory_8(infoP + 4 + 3 * (expCardInfoStringMaxLen + 1) + i, info->deviceUniqueIDStr[i]);
    }
    m68k_write_memory_8(infoP + 4 + i, 0);
  }
}

void palmos_expansiontrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];

  switch (sel) {
    case 9: {
      // Err ExpCardInfo(UInt16 slotRefNum, ExpCardInfoType *infoP)
      uint16_t slotRefNum = ARG16;
      uint32_t infoP = ARG32;
      if (slotRefNum == 0) slotRefNum = 1;
      ExpCardInfoType info;
      emupalmos_trap_sel_in(infoP, sysTrapExpansionDispatch, sel, 1);
      Err res = ExpCardInfo(slotRefNum, infoP ? &info : NULL);
      encode_expcardinfo(infoP, &info);
      debug(DEBUG_TRACE, "EmuPalmOS", "ExpCardInfo(%d, 0x%08X): %d", slotRefNum, infoP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "ExpansionDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
