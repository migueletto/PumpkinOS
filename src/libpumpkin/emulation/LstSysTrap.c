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

void palmos_LstSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapLstSetDrawFunction: {
      // void LstSetDrawFunction(ListType *listP, ListDrawDataFuncPtr func)
      uint32_t listP = ARG32;
      uint32_t funcP = ARG32;
      ListType *list = (ListType *)emupalmos_trap_in(listP, trap, 0);
      emupalmos_trap_in(funcP, trap, 1);
      if (list) list->m68k_drawfunc = funcP;
      debug(DEBUG_TRACE, "EmuPalmOS", "LstSetDrawFunction(0x%08X, 0x%08X)", listP, funcP);
    }
    break;
    case sysTrapLstDrawList: {
      // void LstDrawList(ListType *listP)
      uint32_t listP = ARG32;
      ListType *list = (ListType *)emupalmos_trap_in(listP, trap, 0);
      LstDrawList(list);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstDrawList(0x%08X)", listP);
      }
    break;
    case sysTrapLstEraseList: {
      // void LstEraseList(in ListType *listP)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      LstEraseList(listP ? s_listP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstEraseList(listP=0x%08X)", listP);
    }
    break;
    case sysTrapLstGetSelection: {
      // Int16 LstGetSelection(in ListType *listP)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      Int16 res = LstGetSelection(listP ? s_listP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGetSelection(listP=0x%08X): %d", listP, res);
    }
    break;
    case sysTrapLstGetSelectionText: {
      // Char *LstGetSelectionText(in ListType *listP, Int16 itemNum)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      int16_t itemNum = ARG16;
      Char *res = LstGetSelectionText(listP ? s_listP : NULL, itemNum);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGetSelectionText(listP=0x%08X, itemNum=%d): 0x%08X", listP, itemNum, r_res);
    }
    break;
    case sysTrapLstHandleEvent: {
      // Boolean LstHandleEvent(in ListType *listP, in EventType *eventP)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      uint32_t eventP = ARG32;
      EventType l_eventP;
      decode_event(eventP, &l_eventP);
      Boolean res = LstHandleEvent(listP ? s_listP : NULL, eventP ? &l_eventP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstHandleEvent(listP=0x%08X, eventP=0x%08X): %d", listP, eventP, res);
    }
    break;
    case sysTrapLstSetHeight: {
      // void LstSetHeight(in ListType *listP, Int16 visibleItems)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      int16_t visibleItems = ARG16;
      LstSetHeight(listP ? s_listP : NULL, visibleItems);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstSetHeight(listP=0x%08X, visibleItems=%d)", listP, visibleItems);
    }
    break;
    case sysTrapLstSetPosition: {
      // void LstSetPosition(in ListType *listP, Coord x, Coord y)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      int16_t x = ARG16;
      int16_t y = ARG16;
      LstSetPosition(listP ? s_listP : NULL, x, y);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstSetPosition(listP=0x%08X, x=%d, y=%d)", listP, x, y);
    }
    break;
    case sysTrapLstSetSelection: {
      // void LstSetSelection(in ListType *listP, Int16 itemNum)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      int16_t itemNum = ARG16;
      LstSetSelection(listP ? s_listP : NULL, itemNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstSetSelection(listP=0x%08X, itemNum=%d)", listP, itemNum);
    }
    break;
    case sysTrapLstSetListChoices: {
      // void LstSetListChoices(in ListType *listP, in Char **itemsText, Int16 numItems)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      uint32_t itemsText = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      char **s_itemsText = itemsText ? (char **)(ram + itemsText) : NULL;
      int16_t numItems = ARG16;
      LstSetListChoices(listP ? s_listP : NULL, itemsText ? s_itemsText : NULL, numItems);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstSetListChoices(listP=0x%08X, itemsText=0x%08X, numItems=%d)", listP, itemsText, numItems);
    }
    break;
    case sysTrapLstSetTopItem: {
      // void LstSetTopItem(in ListType *listP, Int16 itemNum)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      int16_t itemNum = ARG16;
      LstSetTopItem(listP ? s_listP : NULL, itemNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstSetTopItem(listP=0x%08X, itemNum=%d)", listP, itemNum);
    }
    break;
    case sysTrapLstMakeItemVisible: {
      // void LstMakeItemVisible(in ListType *listP, Int16 itemNum)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      int16_t itemNum = ARG16;
      LstMakeItemVisible(listP ? s_listP : NULL, itemNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstMakeItemVisible(listP=0x%08X, itemNum=%d)", listP, itemNum);
    }
    break;
    case sysTrapLstGetNumberOfItems: {
      // Int16 LstGetNumberOfItems(in ListType *listP)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      Int16 res = LstGetNumberOfItems(listP ? s_listP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGetNumberOfItems(listP=0x%08X): %d", listP, res);
    }
    break;
    case sysTrapLstPopupList: {
      // Int16 LstPopupList(in ListType *listP)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      Int16 res = LstPopupList(listP ? s_listP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstPopupList(listP=0x%08X): %d", listP, res);
    }
    break;
    case sysTrapLstScrollList: {
      // Boolean LstScrollList(in ListType *listP, WinDirectionType direction, Int16 itemCount)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      uint8_t direction = ARG8;
      int16_t itemCount = ARG16;
      Boolean res = LstScrollList(listP ? s_listP : NULL, direction, itemCount);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstScrollList(listP=0x%08X, direction=%d, itemCount=%d): %d", listP, direction, itemCount, res);
    }
    break;
    case sysTrapLstGetVisibleItems: {
      // Int16 LstGetVisibleItems(in ListType *listP)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      Int16 res = LstGetVisibleItems(listP ? s_listP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGetVisibleItems(listP=0x%08X): %d", listP, res);
    }
    break;
    case sysTrapLstGetTopItem: {
      // Int16 LstGetTopItem(in ListType *listP)
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      Int16 res = LstGetTopItem(listP ? s_listP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGetTopItem(listP=0x%08X): %d", listP, res);
    }
    break;
  }
}
