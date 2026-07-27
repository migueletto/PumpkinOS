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

void palmos_CategorySysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapCategoryCreateListV10: {
      // void CategoryCreateListV10(DmOpenRef db, in ListType *lst, UInt16 currentCategory, Boolean showAll)
      uint32_t db = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
      uint32_t lst = ARG32;
      ListType *s_lst = lst ? (ListType *)(ram + lst) : NULL;
      uint16_t currentCategory = ARG16;
      uint8_t showAll = ARG8;
      CategoryCreateListV10(db ? l_db : 0, lst ? s_lst : NULL, currentCategory, showAll);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategoryCreateListV10(db=0x%08X, lst=0x%08X, currentCategory=%d, showAll=%d)", db, lst, currentCategory, showAll);
    }
    break;
    case sysTrapCategoryCreateList: {
      // void CategoryCreateList(DmOpenRef db, in ListType *listP, UInt16 currentCategory, Boolean showAll, Boolean showUneditables, UInt8 numUneditableCategories, UInt32 editingStrID, Boolean resizeList)
      uint32_t db = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      uint16_t currentCategory = ARG16;
      uint8_t showAll = ARG8;
      uint8_t showUneditables = ARG8;
      uint8_t numUneditableCategories = ARG8;
      uint32_t editingStrID = ARG32;
      uint8_t resizeList = ARG8;
      CategoryCreateList(db ? l_db : 0, listP ? s_listP : NULL, currentCategory, showAll, showUneditables, numUneditableCategories, editingStrID, resizeList);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategoryCreateList(db=0x%08X, listP=0x%08X, currentCategory=%d, showAll=%d, showUneditables=%d, numUneditableCategories=%d, editingStrID=%d, resizeList=%d)", db, listP, currentCategory, showAll, showUneditables, numUneditableCategories, editingStrID, resizeList);
    }
    break;
    case sysTrapCategoryFreeListV10: {
      // void CategoryFreeListV10(DmOpenRef db, in ListType *lst)
      uint32_t db = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
      uint32_t lst = ARG32;
      ListType *s_lst = lst ? (ListType *)(ram + lst) : NULL;
      CategoryFreeListV10(db ? l_db : 0, lst ? s_lst : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategoryFreeListV10(db=0x%08X, lst=0x%08X)", db, lst);
    }
    break;
    case sysTrapCategoryFreeList: {
      // void CategoryFreeList(DmOpenRef db, in ListType *listP, Boolean showAll, UInt32 editingStrID)
      uint32_t db = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
      uint32_t listP = ARG32;
      ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
      uint8_t showAll = ARG8;
      uint32_t editingStrID = ARG32;
      CategoryFreeList(db ? l_db : 0, listP ? s_listP : NULL, showAll, editingStrID);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategoryFreeList(db=0x%08X, listP=0x%08X, showAll=%d, editingStrID=%d)", db, listP, showAll, editingStrID);
    }
    break;
    case sysTrapCategoryFind: {
      // UInt16 CategoryFind(DmOpenRef db, in Char *name)
      uint32_t db = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
      uint32_t name = ARG32;
      char *s_name = name ? (char *)(ram + name) : NULL;
      UInt16 res = CategoryFind(db ? l_db : 0, name ? s_name : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategoryFind(db=0x%08X, name=0x%08X [%s]): %d", db, name, s_name, res);
    }
    break;
    case sysTrapCategoryGetName: {
      // void CategoryGetName(DmOpenRef db, UInt16 index, out Char *name)
      uint32_t db = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
      uint16_t index = ARG16;
      uint32_t name = ARG32;
      char *s_name = name ? (char *)(ram + name) : NULL;
      CategoryGetName(db ? l_db : 0, index, name ? s_name : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategoryGetName(db=0x%08X, index=%d, name=0x%08X [%s])", db, index, name, s_name);
    }
    break;
    case sysTrapCategoryEditV10: {
      // Boolean CategoryEditV10(DmOpenRef db, inout UInt16 *category)
      uint32_t db = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
      uint32_t category = ARG32;
      UInt16 l_category = 0;
      if (category) l_category = m68k_read_memory_16(category);
      Boolean res = CategoryEditV10(db ? l_db : 0, category ? &l_category : NULL);
      if (category) m68k_write_memory_16(category, l_category);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategoryEditV10(db=0x%08X, category=0x%08X [%d]): %d", db, category, l_category, res);
    }
    break;
    case sysTrapCategoryEditV20: {
      // Boolean CategoryEditV20(DmOpenRef db, inout UInt16 *category, UInt32 titleStrID)
      uint32_t db = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
      uint32_t category = ARG32;
      UInt16 l_category = 0;
      if (category) l_category = m68k_read_memory_16(category);
      uint32_t titleStrID = ARG32;
      Boolean res = CategoryEditV20(db ? l_db : 0, category ? &l_category : NULL, titleStrID);
      if (category) m68k_write_memory_16(category, l_category);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategoryEditV20(db=0x%08X, category=0x%08X [%d], titleStrID=%d): %d", db, category, l_category, titleStrID, res);
    }
    break;
    case sysTrapCategoryEdit: {
      // Boolean CategoryEdit(DmOpenRef db, inout UInt16 *category, UInt32 titleStrID, UInt8 numUneditableCategories)
      uint32_t db = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
      uint32_t category = ARG32;
      UInt16 l_category = 0;
      if (category) l_category = m68k_read_memory_16(category);
      uint32_t titleStrID = ARG32;
      uint8_t numUneditableCategories = ARG8;
      Boolean res = CategoryEdit(db ? l_db : 0, category ? &l_category : NULL, titleStrID, numUneditableCategories);
      if (category) m68k_write_memory_16(category, l_category);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategoryEdit(db=0x%08X, category=0x%08X [%d], titleStrID=%d, numUneditableCategories=%d): %d", db, category, l_category, titleStrID, numUneditableCategories, res);
    }
    break;
    case sysTrapCategorySelectV10: {
      // Boolean CategorySelectV10(DmOpenRef db, in FormType *frm, UInt16 ctlID, UInt16 lstID, Boolean title, out UInt16 *categoryP, out Char *categoryName)
      uint32_t db = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
      uint32_t frm = ARG32;
      FormType *s_frm = frm ? (FormType *)(ram + frm) : NULL;
      uint16_t ctlID = ARG16;
      uint16_t lstID = ARG16;
      uint8_t title = ARG8;
      uint32_t categoryP = ARG32;
      UInt16 l_categoryP = 0;
      uint32_t categoryName = ARG32;
      char *s_categoryName = categoryName ? (char *)(ram + categoryName) : NULL;
      Boolean res = CategorySelectV10(db ? l_db : 0, frm ? s_frm : NULL, ctlID, lstID, title, categoryP ? &l_categoryP : NULL, categoryName ? s_categoryName : NULL);
      if (categoryP) m68k_write_memory_16(categoryP, l_categoryP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategorySelectV10(db=0x%08X, frm=0x%08X, ctlID=%d, lstID=%d, title=%d, categoryP=0x%08X [%d], categoryName=0x%08X [%s]): %d", db, frm, ctlID, lstID, title, categoryP, l_categoryP, categoryName, s_categoryName, res);
    }
    break;
    case sysTrapCategorySelect: {
      // Boolean CategorySelect(DmOpenRef db, in FormType *frm, UInt16 ctlID, UInt16 lstID, Boolean title, out UInt16 *categoryP, out Char *categoryName, UInt8 numUneditableCategories, UInt32 editingStrID)
      uint32_t db = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
      uint32_t frm = ARG32;
      FormType *s_frm = frm ? (FormType *)(ram + frm) : NULL;
      uint16_t ctlID = ARG16;
      uint16_t lstID = ARG16;
      uint8_t title = ARG8;
      uint32_t categoryP = ARG32;
      UInt16 l_categoryP = 0;
      uint32_t categoryName = ARG32;
      char *s_categoryName = categoryName ? (char *)(ram + categoryName) : NULL;
      uint8_t numUneditableCategories = ARG8;
      uint32_t editingStrID = ARG32;
      Boolean res = CategorySelect(db ? l_db : 0, frm ? s_frm : NULL, ctlID, lstID, title, categoryP ? &l_categoryP : NULL, categoryName ? s_categoryName : NULL, numUneditableCategories, editingStrID);
      if (categoryP) m68k_write_memory_16(categoryP, l_categoryP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategorySelect(db=0x%08X, frm=0x%08X, ctlID=%d, lstID=%d, title=%d, categoryP=0x%08X [%d], categoryName=0x%08X [%s], numUneditableCategories=%d, editingStrID=%d): %d", db, frm, ctlID, lstID, title, categoryP, l_categoryP, categoryName, s_categoryName, numUneditableCategories, editingStrID, res);
    }
    break;
    case sysTrapCategoryGetNext: {
      // UInt16 CategoryGetNext(DmOpenRef db, UInt16 index)
      uint32_t db = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
      uint16_t index = ARG16;
      UInt16 res = CategoryGetNext(db ? l_db : 0, index);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategoryGetNext(db=0x%08X, index=%d): %d", db, index, res);
    }
    break;
    case sysTrapCategorySetTriggerLabel: {
      // void CategorySetTriggerLabel(in ControlType *ctl, Char *name)
      uint32_t ctl = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      ControlType *s_ctl = ctl ? (ControlType *)(ram + ctl) : NULL;
      uint32_t name = ARG32;
      char *s_name = name ? (char *)(ram + name) : NULL;
      CategorySetTriggerLabel(ctl ? s_ctl : NULL, name ? s_name : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategorySetTriggerLabel(ctl=0x%08X, name=0x%08X [%s])", ctl, name, s_name);
    }
    break;
    case sysTrapCategoryTruncateName: {
      // void CategoryTruncateName(inout Char *name, UInt16 maxWidth)
      uint32_t name = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      char *s_name = name ? (char *)(ram + name) : NULL;
      uint16_t maxWidth = ARG16;
      CategoryTruncateName(name ? s_name : NULL, maxWidth);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategoryTruncateName(name=0x%08X [%s], maxWidth=%d)", name, s_name, maxWidth);
    }
    break;
    case sysTrapCategoryInitialize: {
      // void CategoryInitialize(inout AppInfoType *appInfoP, UInt16 localizedAppInfoStrID)
      uint32_t appInfoP = ARG32;
      AppInfoType *l_appInfoP;
      if ((l_appInfoP = MemPtrNew(sizeof(AppInfoType))) != NULL) {
        decode_appinfo(appInfoP, l_appInfoP);
        uint16_t localizedAppInfoStrID = ARG16;
        CategoryInitialize(appInfoP ? l_appInfoP : NULL, localizedAppInfoStrID);
        encode_appinfo(appInfoP, l_appInfoP);
        MemPtrFree(l_appInfoP);
        debug(DEBUG_TRACE, "EmuPalmOS", "CategoryInitialize(appInfoP=0x%08X, localizedAppInfoStrID=%d)", appInfoP, localizedAppInfoStrID);
      }
    }
    break;
    case sysTrapCategorySetName: {
      // void CategorySetName(DmOpenRef db, UInt16 index, in Char *nameP)
      uint32_t db = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
      uint16_t index = ARG16;
      uint32_t nameP = ARG32;
      char *s_nameP = nameP ? (char *)(ram + nameP) : NULL;
      CategorySetName(db ? l_db : 0, index, nameP ? s_nameP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "CategorySetName(db=0x%08X, index=%d, nameP=0x%08X [%s])", db, index, nameP, s_nameP);
    }
    break;
  }
}
