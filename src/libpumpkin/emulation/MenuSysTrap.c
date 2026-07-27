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

void palmos_MenuSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapMenuInit: {
      // MenuBarType *MenuInit(UInt16 resourceId)
      uint16_t resourceId = ARG16;
      MenuBarType *res = MenuInit(resourceId);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuInit(resourceId=%d): 0x%08X", resourceId, r_res);
    }
    break;
    case sysTrapMenuGetActiveMenu: {
      // MenuBarType *MenuGetActiveMenu(void)
      MenuBarType *res = MenuGetActiveMenu();
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuGetActiveMenu(): 0x%08X", r_res);
    }
    break;
    case sysTrapMenuSetActiveMenu: {
      // MenuBarType *MenuSetActiveMenu(in MenuBarType *menuP)
      uint32_t menuP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      MenuBarType *s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
      MenuBarType *res = MenuSetActiveMenu(menuP ? s_menuP : NULL);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuSetActiveMenu(menuP=0x%08X): 0x%08X", menuP, r_res);
    }
    break;
    case sysTrapMenuDispose: {
      // void MenuDispose(in MenuBarType *menuP)
      uint32_t menuP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      MenuBarType *s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
      MenuDispose(menuP ? s_menuP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuDispose(menuP=0x%08X)", menuP);
    }
    break;
    case sysTrapMenuHandleEvent: {
      // Boolean MenuHandleEvent(in MenuBarType *menuP, in EventType *event, out UInt16 *error)
      uint32_t menuP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      MenuBarType *s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
      uint32_t event = ARG32;
      EventType l_event;
      decode_event(event, &l_event);
      uint32_t error = ARG32;
      UInt16 l_error = 0;
      Boolean res = MenuHandleEvent(menuP ? s_menuP : NULL, event ? &l_event : NULL, error ? &l_error : NULL);
      if (error) m68k_write_memory_16(error, l_error);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuHandleEvent(menuP=0x%08X, event=0x%08X, error=0x%08X [%d]): %d", menuP, event, error, l_error, res);
    }
    break;
    case sysTrapMenuDrawMenu: {
      // void MenuDrawMenu(in MenuBarType *menuP)
      uint32_t menuP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      MenuBarType *s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
      MenuDrawMenu(menuP ? s_menuP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuDrawMenu(menuP=0x%08X)", menuP);
    }
    break;
    case sysTrapMenuEraseStatus: {
      // void MenuEraseStatus(in MenuBarType *menuP)
      uint32_t menuP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      MenuBarType *s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
      MenuEraseStatus(menuP ? s_menuP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuEraseStatus(menuP=0x%08X)", menuP);
    }
    break;
    case sysTrapMenuSetActiveMenuRscID: {
      // void MenuSetActiveMenuRscID(UInt16 resourceId)
      uint16_t resourceId = ARG16;
      MenuSetActiveMenuRscID(resourceId);
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuSetActiveMenuRscID(resourceId=%d)", resourceId);
    }
    break;
    case sysTrapMenuCmdBarAddButton: {
      // Err MenuCmdBarAddButton(UInt8 where, UInt16 bitmapId, MenuCmdBarResultType resultType, UInt32 result, in Char *nameP)
      uint8_t where = ARG8;
      uint16_t bitmapId = ARG16;
      uint8_t resultType = ARG8;
      uint32_t result = ARG32;
      uint32_t nameP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      char *s_nameP = nameP ? (char *)(ram + nameP) : NULL;
      Err res = MenuCmdBarAddButton(where, bitmapId, resultType, result, nameP ? s_nameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuCmdBarAddButton(where=%d, bitmapId=%d, resultType=%d, result=%d, nameP=0x%08X [%s]): %d", where, bitmapId, resultType, result, nameP, s_nameP, res);
    }
    break;
    case sysTrapMenuCmdBarGetButtonData: {
      // Boolean MenuCmdBarGetButtonData(Int16 buttonIndex, out UInt16 *bitmapIdP, out MenuCmdBarResultType *resultTypeP, out UInt32 *resultP, out Char *nameP)
      int16_t buttonIndex = ARG16;
      uint32_t bitmapIdP = ARG32;
      UInt16 l_bitmapIdP = 0;
      uint32_t resultTypeP = ARG32;
      MenuCmdBarResultType l_resultTypeP;
      uint32_t resultP = ARG32;
      UInt32 l_resultP = 0;
      uint32_t nameP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      char *s_nameP = nameP ? (char *)(ram + nameP) : NULL;
      Boolean res = MenuCmdBarGetButtonData(buttonIndex, bitmapIdP ? &l_bitmapIdP : NULL, resultTypeP ? &l_resultTypeP : NULL, resultP ? &l_resultP : NULL, nameP ? s_nameP : NULL);
      if (bitmapIdP) m68k_write_memory_16(bitmapIdP, l_bitmapIdP);
      if (resultTypeP) m68k_write_memory_8(resultTypeP, l_resultTypeP);
      if (resultP) m68k_write_memory_32(resultP, l_resultP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuCmdBarGetButtonData(buttonIndex=%d, bitmapIdP=0x%08X [%d], resultTypeP=0x%08X, resultP=0x%08X [%d], nameP=0x%08X [%s]): %d", buttonIndex, bitmapIdP, l_bitmapIdP, resultTypeP, resultP, l_resultP, nameP, s_nameP, res);
    }
    break;
    case sysTrapMenuCmdBarDisplay: {
      // void MenuCmdBarDisplay(void)
      MenuCmdBarDisplay();
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuCmdBarDisplay()");
    }
    break;
    case sysTrapMenuShowItem: {
      // Boolean MenuShowItem(UInt16 id)
      uint16_t id = ARG16;
      Boolean res = MenuShowItem(id);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuShowItem(id=%d): %d", id, res);
    }
    break;
    case sysTrapMenuHideItem: {
      // Boolean MenuHideItem(UInt16 id)
      uint16_t id = ARG16;
      Boolean res = MenuHideItem(id);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuHideItem(id=%d): %d", id, res);
    }
    break;
    case sysTrapMenuAddItem: {
      // Err MenuAddItem(UInt16 positionId, UInt16 id, Char cmd, in Char *textP)
      uint16_t positionId = ARG16;
      uint16_t id = ARG16;
      int8_t cmd = ARG8;
      uint32_t textP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      char *s_textP = textP ? (char *)(ram + textP) : NULL;
      Err res = MenuAddItem(positionId, id, cmd, textP ? s_textP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "MenuAddItem(positionId=%d, id=%d, cmd=%d, textP=0x%08X [%s]): %d", positionId, id, cmd, textP, s_textP, res);
    }
    break;
  }
}
