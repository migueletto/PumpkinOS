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

void palmos_PrefSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapPrefGetPreferences: {
      // void PrefGetPreferences(SystemPreferencesPtr p)
      uint32_t p = ARG32;
      emupalmos_trap_in(p, trap, 0);
      SystemPreferencesType prefs;
      PrefGetPreferences(p ? &prefs : NULL);
      // XXX decode prefs into p
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetPreferences(0x%08X)", p);
    }
    break;
    case sysTrapPrefSetPreferences: {
      // void PrefSetPreferences(SystemPreferencesPtr p)
      uint32_t p = ARG32;
      emupalmos_trap_in(p, trap, 0);
      SystemPreferencesType prefs;
      // XXX encode p into prefs
      PrefSetPreferences(p ? &prefs : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetPreferences(0x%08X)", p);
    }
    break;
    case sysTrapPrefGetPreference: {
      // UInt32 PrefGetPreference(SystemPreferencesChoice choice)
      uint8_t choice = ARG8;
      uint32_t value = PrefGetPreference(choice);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetPreference(%d): %d", choice, value);
      m68k_set_reg(M68K_REG_D0, value);
    }
    break;
    case sysTrapPrefSetPreference: {
      //void PrefSetPreference(SystemPreferencesChoice choice, UInt32 value)
      uint8_t choice = ARG8;
      uint32_t value = ARG32;
      PrefSetPreference(choice, value);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetPreference(%d, %d)", choice, value);
    }
    break;
    case sysTrapPrefOpenPreferenceDB: {
      // DmOpenRef PrefOpenPreferenceDB(Boolean saved)
      uint8_t saved = ARG8;
      DmOpenRef dbRef = PrefOpenPreferenceDB(saved);
      uint32_t a = emupalmos_trap_out(dbRef);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefOpenPreferenceDB(%d): 0x%08X", saved, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
    case sysTrapPrefOpenPreferenceDBV10: {
      // DmOpenRef PrefOpenPreferenceDBV10(void)
      DmOpenRef dbRef = PrefOpenPreferenceDBV10();
      uint32_t a = emupalmos_trap_out(dbRef);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefOpenPreferenceDBV10(): 0x%08X", a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
    case sysTrapPrefSetAppPreferences: {
      // void PrefSetAppPreferences(UInt32 creator, UInt16 id, Int16 version, const void *prefs, UInt16 prefsSize, Boolean saved)
      uint32_t creator = ARG32;
      uint16_t id = ARG16;
      int16_t version = ARG16;
      uint32_t prefsP = ARG32;
      uint16_t prefsSize = ARG16;
      uint8_t saved = ARG8;
      PrefSetAppPreferences(creator, id, version, emupalmos_trap_in(prefsP, trap, 3), prefsSize, saved);
      char screator[8];
      pumpkin_id2s(creator, screator);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetAppPreferences('%s', %d, %d, 0x%08X, %d, %d)", screator, id, version, prefsP, prefsSize, saved);
    }
    break;
    case sysTrapPrefSetAppPreferencesV10: {
      // void PrefSetAppPreferencesV10(UInt32 creator, Int16 version, void *prefs, UInt16 prefsSize)
      uint32_t creator = ARG32;
      int16_t version = ARG16;
      uint32_t prefsP = ARG32;
      uint16_t prefsSize = ARG16;
      PrefSetAppPreferencesV10(creator, version, emupalmos_trap_in(prefsP, trap, 2), prefsSize);
      char screator[8];
      pumpkin_id2s(creator, screator);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetAppPreferencesV10('%s', %d, 0x%08X, %d)", screator, version, prefsP, prefsSize);
    }
    break;
    case sysTrapPrefGetAppPreferences: {
      // Int16 PrefGetAppPreferences(UInt32 creator, UInt16 id, void *prefs, UInt16 *prefsSize, Boolean saved)
      uint32_t creator = ARG32;
      uint16_t id = ARG16;
      uint32_t prefsP = ARG32;
      uint32_t prefsSizeP = ARG32;
      uint8_t saved = ARG8;
      emupalmos_trap_in(prefsSizeP, trap, 3);
      UInt16 prefsSize = prefsSizeP ? m68k_read_memory_16(prefsSizeP) : 0;
      UInt16 version = PrefGetAppPreferences(creator, id, emupalmos_trap_in(prefsP, trap, 2), prefsSizeP ? &prefsSize : NULL, saved);
      char screator[8];
      pumpkin_id2s(creator, screator);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetAppPreferences('%s', %d, 0x%08X, 0x%08X, %d): %d", screator, id, prefsP, prefsSizeP, saved, version);
      if (prefsSizeP) m68k_write_memory_16(prefsSizeP, prefsSize);
      m68k_set_reg(M68K_REG_D0, version);
    }
    break;
    case sysTrapPrefGetAppPreferencesV10: {
      // Boolean PrefGetAppPreferencesV10(UInt32 type, Int16 version, void *prefs, UInt16 prefsSize)
      uint32_t type = ARG32;
      uint16_t version = ARG16;
      uint32_t prefsP = ARG32;
      uint16_t prefsSize = ARG16;
      Boolean b = PrefGetAppPreferencesV10(type, version, emupalmos_trap_in(prefsP, trap, 2), prefsSize);
      char screator[8];
      pumpkin_id2s(type, screator);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetAppPreferencesV10('%s', %d, 0x%08X, %d): %d", screator, version, prefsP, prefsSize, b);
      m68k_set_reg(M68K_REG_D0, b);
    }
    break;
  }
}
