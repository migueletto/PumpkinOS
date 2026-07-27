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

void palmos_TblSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapTblSetCustomDrawProcedure: {
      // void TblSetCustomDrawProcedure(TableType *tableP, Int16 column, TableDrawItemFuncPtr drawCallback)
      uint32_t tableP = ARG32;
      int16_t column = ARG16;
      uint32_t funcP = ARG32;
      TableType *table = (TableType *)emupalmos_trap_in(tableP, trap, 0);
      emupalmos_trap_in(funcP, trap, 2);
      if (table && column >= 0 && column < table->numColumns) {
        table->columnAttrs[column].m68k_drawfunc = funcP;
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetCustomDrawProcedure(0x%08X, %d, 0x%08X)", tableP, column, funcP);
    }
    break;
    case sysTrapTblSetLoadDataProcedure: {
      // void TblSetLoadDataProcedure(TableType *tableP, Int16 column, TableLoadDataFuncPtr loadDataCallback)
      uint32_t tableP = ARG32;
      int16_t column = ARG16;
      uint32_t funcP = ARG32;
      TableType *table = (TableType *)emupalmos_trap_in(tableP, trap, 0);
      emupalmos_trap_in(funcP, trap, 2);
      if (table && column >= 0 && column < table->numColumns) {
        table->columnAttrs[column].m68k_loadfunc = funcP;
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetLoadDataProcedure(0x%08X, %d, 0x%08X)", tableP, column, funcP);
    }
    break;
    case sysTrapTblSetSaveDataProcedure: {
      // void TblSetSaveDataProcedure(TableType *tableP, Int16 column, TableSaveDataFuncPtr saveDataCallback)
      uint32_t tableP = ARG32;
      int16_t column = ARG16;
      uint32_t funcP = ARG32;
      TableType *table = (TableType *)emupalmos_trap_in(tableP, trap, 0);
      emupalmos_trap_in(funcP, trap, 2);
      if (table && column >= 0 && column < table->numColumns) {
        table->columnAttrs[column].m68k_savefunc = funcP;
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetSaveDataProcedure(0x%08X, %d, 0x%08X)", tableP, column, funcP);
    }
    break;
    case sysTrapTblDrawTable: {
      // void TblDrawTable(in TableType *tableP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      TblDrawTable(tableP ? s_tableP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblDrawTable(tableP=0x%08X)", tableP);
    }
    break;
    case sysTrapTblRedrawTable: {
      // void TblRedrawTable(in TableType *tableP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      TblRedrawTable(tableP ? s_tableP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblRedrawTable(tableP=0x%08X)", tableP);
    }
    break;
    case sysTrapTblEraseTable: {
      // void TblEraseTable(in TableType *tableP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      TblEraseTable(tableP ? s_tableP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblEraseTable(tableP=0x%08X)", tableP);
    }
    break;
    case sysTrapTblHandleEvent: {
      // Boolean TblHandleEvent(in TableType *tableP, in EventType *event)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      uint32_t event = ARG32;
      EventType l_event;
      decode_event(event, &l_event);
      Boolean res = TblHandleEvent(tableP ? s_tableP : NULL, event ? &l_event : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblHandleEvent(tableP=0x%08X, event=0x%08X): %d", tableP, event, res);
    }
    break;
    case sysTrapTblGetItemBounds: {
      // void TblGetItemBounds(in TableType *tableP, Int16 row, Int16 column, out RectangleType *rP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      int16_t column = ARG16;
      uint32_t rP = ARG32;
      RectangleType l_rP;
      TblGetItemBounds(tableP ? s_tableP : NULL, row, column, rP ? &l_rP : NULL);
      encode_rectangle(rP, &l_rP);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetItemBounds(tableP=0x%08X, row=%d, column=%d, rP=0x%08X [%d,%d,%d,%d])", tableP, row, column, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
    }
    break;
    case sysTrapTblSelectItem: {
      // void TblSelectItem(in TableType *tableP, Int16 row, Int16 column)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      int16_t column = ARG16;
      TblSelectItem(tableP ? s_tableP : NULL, row, column);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSelectItem(tableP=0x%08X, row=%d, column=%d)", tableP, row, column);
    }
    break;
    case sysTrapTblGetItemInt: {
      // Int16 TblGetItemInt(in TableType *tableP, Int16 row, Int16 column)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      int16_t column = ARG16;
      Int16 res = TblGetItemInt(tableP ? s_tableP : NULL, row, column);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetItemInt(tableP=0x%08X, row=%d, column=%d): %d", tableP, row, column, res);
    }
    break;
    case sysTrapTblSetItemInt: {
      // void TblSetItemInt(in TableType *tableP, Int16 row, Int16 column, Int16 value)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      int16_t column = ARG16;
      int16_t value = ARG16;
      TblSetItemInt(tableP ? s_tableP : NULL, row, column, value);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetItemInt(tableP=0x%08X, row=%d, column=%d, value=%d)", tableP, row, column, value);
    }
    break;
    case sysTrapTblSetItemPtr: {
      // void TblSetItemPtr(in TableType *tableP, Int16 row, Int16 column, in void *value)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      int16_t column = ARG16;
      uint32_t value = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      void *s_value = value ? (void *)(ram + value) : NULL;
      TblSetItemPtr(tableP ? s_tableP : NULL, row, column, value ? s_value : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetItemPtr(tableP=0x%08X, row=%d, column=%d, value=0x%08X)", tableP, row, column, value);
    }
    break;
    case sysTrapTblSetItemStyle: {
      // void TblSetItemStyle(in TableType *tableP, Int16 row, Int16 column, TableItemStyleType type)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      int16_t column = ARG16;
      uint8_t type = ARG8;
      TblSetItemStyle(tableP ? s_tableP : NULL, row, column, type);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetItemStyle(tableP=0x%08X, row=%d, column=%d, type=%d)", tableP, row, column, type);
    }
    break;
    case sysTrapTblUnhighlightSelection: {
      // void TblUnhighlightSelection(in TableType *tableP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      TblUnhighlightSelection(tableP ? s_tableP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblUnhighlightSelection(tableP=0x%08X)", tableP);
    }
    break;
    case sysTrapTblRowUsable: {
      // Boolean TblRowUsable(in TableType *tableP, Int16 row)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      Boolean res = TblRowUsable(tableP ? s_tableP : NULL, row);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblRowUsable(tableP=0x%08X, row=%d): %d", tableP, row, res);
    }
    break;
    case sysTrapTblSetRowUsable: {
      // void TblSetRowUsable(in TableType *tableP, Int16 row, Boolean usable)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      uint8_t usable = ARG8;
      TblSetRowUsable(tableP ? s_tableP : NULL, row, usable);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowUsable(tableP=0x%08X, row=%d, usable=%d)", tableP, row, usable);
    }
    break;
    case sysTrapTblGetLastUsableRow: {
      // Int16 TblGetLastUsableRow(in TableType *tableP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      Int16 res = TblGetLastUsableRow(tableP ? s_tableP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetLastUsableRow(tableP=0x%08X): %d", tableP, res);
    }
    break;
    case sysTrapTblSetColumnUsable: {
      // void TblSetColumnUsable(in TableType *tableP, Int16 column, Boolean usable)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t column = ARG16;
      uint8_t usable = ARG8;
      TblSetColumnUsable(tableP ? s_tableP : NULL, column, usable);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnUsable(tableP=0x%08X, column=%d, usable=%d)", tableP, column, usable);
    }
    break;
    case sysTrapTblSetRowSelectable: {
      // void TblSetRowSelectable(in TableType *tableP, Int16 row, Boolean selectable)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      uint8_t selectable = ARG8;
      TblSetRowSelectable(tableP ? s_tableP : NULL, row, selectable);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowSelectable(tableP=0x%08X, row=%d, selectable=%d)", tableP, row, selectable);
    }
    break;
    case sysTrapTblRowSelectable: {
      // Boolean TblRowSelectable(in TableType *tableP, Int16 row)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      Boolean res = TblRowSelectable(tableP ? s_tableP : NULL, row);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblRowSelectable(tableP=0x%08X, row=%d): %d", tableP, row, res);
    }
    break;
    case sysTrapTblGetNumberOfRows: {
      // Int16 TblGetNumberOfRows(in TableType *tableP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      Int16 res = TblGetNumberOfRows(tableP ? s_tableP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetNumberOfRows(tableP=0x%08X): %d", tableP, res);
    }
    break;
    case sysTrapTblGetBounds: {
      // void TblGetBounds(in TableType *tableP, out RectangleType *rP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      uint32_t rP = ARG32;
      RectangleType l_rP;
      TblGetBounds(tableP ? s_tableP : NULL, rP ? &l_rP : NULL);
      encode_rectangle(rP, &l_rP);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetBounds(tableP=0x%08X, rP=0x%08X [%d,%d,%d,%d])", tableP, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
    }
    break;
    case sysTrapTblSetBounds: {
      // void TblSetBounds(in TableType *tableP, in RectangleType *rP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      uint32_t rP = ARG32;
      RectangleType l_rP;
      decode_rectangle(rP, &l_rP);
      TblSetBounds(tableP ? s_tableP : NULL, rP ? &l_rP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetBounds(tableP=0x%08X, rP=0x%08X [%d,%d,%d,%d])", tableP, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
    }
    break;
    case sysTrapTblGetRowHeight: {
      // Coord TblGetRowHeight(in TableType *tableP, Int16 row)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      Coord res = TblGetRowHeight(tableP ? s_tableP : NULL, row);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetRowHeight(tableP=0x%08X, row=%d): %d", tableP, row, res);
    }
    break;
    case sysTrapTblSetRowHeight: {
      // void TblSetRowHeight(in TableType *tableP, Int16 row, Coord height)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      int16_t height = ARG16;
      TblSetRowHeight(tableP ? s_tableP : NULL, row, height);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowHeight(tableP=0x%08X, row=%d, height=%d)", tableP, row, height);
    }
    break;
    case sysTrapTblGetColumnWidth: {
      // Coord TblGetColumnWidth(in TableType *tableP, Int16 column)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t column = ARG16;
      Coord res = TblGetColumnWidth(tableP ? s_tableP : NULL, column);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetColumnWidth(tableP=0x%08X, column=%d): %d", tableP, column, res);
    }
    break;
    case sysTrapTblSetColumnWidth: {
      // void TblSetColumnWidth(in TableType *tableP, Int16 column, Coord width)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t column = ARG16;
      int16_t width = ARG16;
      TblSetColumnWidth(tableP ? s_tableP : NULL, column, width);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnWidth(tableP=0x%08X, column=%d, width=%d)", tableP, column, width);
    }
    break;
    case sysTrapTblGetColumnSpacing: {
      // Coord TblGetColumnSpacing(in TableType *tableP, Int16 column)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t column = ARG16;
      Coord res = TblGetColumnSpacing(tableP ? s_tableP : NULL, column);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetColumnSpacing(tableP=0x%08X, column=%d): %d", tableP, column, res);
    }
    break;
    case sysTrapTblSetColumnSpacing: {
      // void TblSetColumnSpacing(in TableType *tableP, Int16 column, Coord spacing)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t column = ARG16;
      int16_t spacing = ARG16;
      TblSetColumnSpacing(tableP ? s_tableP : NULL, column, spacing);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnSpacing(tableP=0x%08X, column=%d, spacing=%d)", tableP, column, spacing);
    }
    break;
    case sysTrapTblFindRowID: {
      // Boolean TblFindRowID(in TableType *tableP, UInt16 id, out Int16 *rowP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      uint16_t id = ARG16;
      uint32_t rowP = ARG32;
      Int16 l_rowP = 0;
      Boolean res = TblFindRowID(tableP ? s_tableP : NULL, id, rowP ? &l_rowP : NULL);
      if (rowP) m68k_write_memory_16(rowP, l_rowP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblFindRowID(tableP=0x%08X, id=%d, rowP=0x%08X [%d]): %d", tableP, id, rowP, l_rowP, res);
    }
    break;
    case sysTrapTblFindRowData: {
      // Boolean TblFindRowData(in TableType *tableP, UInt32 data, out Int16 *rowP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      uint32_t data = ARG32;
      uint32_t rowP = ARG32;
      Int16 l_rowP = 0;
      Boolean res = TblFindRowData(tableP ? s_tableP : NULL, data, rowP ? &l_rowP : NULL);
      if (rowP) m68k_write_memory_16(rowP, l_rowP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblFindRowData(tableP=0x%08X, data=%d, rowP=0x%08X [%d]): %d", tableP, data, rowP, l_rowP, res);
    }
    break;
    case sysTrapTblGetRowID: {
      // UInt16 TblGetRowID(in TableType *tableP, Int16 row)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      UInt16 res = TblGetRowID(tableP ? s_tableP : NULL, row);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetRowID(tableP=0x%08X, row=%d): %d", tableP, row, res);
    }
    break;
    case sysTrapTblSetRowID: {
      // void TblSetRowID(in TableType *tableP, Int16 row, UInt16 id)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      uint16_t id = ARG16;
      TblSetRowID(tableP ? s_tableP : NULL, row, id);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowID(tableP=0x%08X, row=%d, id=%d)", tableP, row, id);
    }
    break;
    case sysTrapTblGetRowData: {
      // UInt32 TblGetRowData(in TableType *tableP, Int16 row)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      uint32_t res = TblGetRowData(tableP ? s_tableP : NULL, row);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetRowData(tableP=0x%08X, row=%d): 0x%08X", tableP, row, res);
    }
    break;
    case sysTrapTblSetRowData: {
      // void TblSetRowData(in TableType *tableP, Int16 row, UInt32 data)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      uint32_t data = ARG32;
      TblSetRowData(tableP ? s_tableP : NULL, row, data);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowData(tableP=0x%08X, row=%d, data=0x%08X)", tableP, row, data);
    }
    break;
    case sysTrapTblRowInvalid: {
      // Boolean TblRowInvalid(in TableType *tableP, Int16 row)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      Boolean res = TblRowInvalid(tableP ? s_tableP : NULL, row);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblRowInvalid(tableP=0x%08X, row=%d): %d", tableP, row, res);
    }
    break;
    case sysTrapTblMarkRowInvalid: {
      // void TblMarkRowInvalid(in TableType *tableP, Int16 row)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      TblMarkRowInvalid(tableP ? s_tableP : NULL, row);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblMarkRowInvalid(tableP=0x%08X, row=%d)", tableP, row);
    }
    break;
    case sysTrapTblMarkTableInvalid: {
      // void TblMarkTableInvalid(in TableType *tableP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      TblMarkTableInvalid(tableP ? s_tableP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblMarkTableInvalid(tableP=0x%08X)", tableP);
    }
    break;
    case sysTrapTblGetSelection: {
      // Boolean TblGetSelection(in TableType *tableP, out Int16 *rowP, out Int16 *columnP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      uint32_t rowP = ARG32;
      Int16 l_rowP = 0;
      uint32_t columnP = ARG32;
      Int16 l_columnP = 0;
      Boolean res = TblGetSelection(tableP ? s_tableP : NULL, rowP ? &l_rowP : NULL, columnP ? &l_columnP : NULL);
      if (rowP) m68k_write_memory_16(rowP, l_rowP);
      if (columnP) m68k_write_memory_16(columnP, l_columnP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetSelection(tableP=0x%08X, rowP=0x%08X [%d], columnP=0x%08X [%d]): %d", tableP, rowP, l_rowP, columnP, l_columnP, res);
    }
    break;
    case sysTrapTblInsertRow: {
      // void TblInsertRow(in TableType *tableP, Int16 row)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      TblInsertRow(tableP ? s_tableP : NULL, row);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblInsertRow(tableP=0x%08X, row=%d)", tableP, row);
    }
    break;
    case sysTrapTblRemoveRow: {
      // void TblRemoveRow(in TableType *tableP, Int16 row)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      TblRemoveRow(tableP ? s_tableP : NULL, row);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblRemoveRow(tableP=0x%08X, row=%d)", tableP, row);
    }
    break;
    case sysTrapTblReleaseFocus: {
      // void TblReleaseFocus(in TableType *tableP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      TblReleaseFocus(tableP ? s_tableP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblReleaseFocus(tableP=0x%08X)", tableP);
    }
    break;
    case sysTrapTblEditing: {
      // Boolean TblEditing(in TableType *tableP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      Boolean res = TblEditing(tableP ? s_tableP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblEditing(tableP=0x%08X): %d", tableP, res);
    }
    break;
    case sysTrapTblGetCurrentField: {
      // FieldType *TblGetCurrentField(in TableType *tableP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      FieldType *res = TblGetCurrentField(tableP ? s_tableP : NULL);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetCurrentField(tableP=0x%08X): 0x%08X", tableP, r_res);
    }
    break;
    case sysTrapTblGrabFocus: {
      // void TblGrabFocus(in TableType *tableP, Int16 row, Int16 column)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      int16_t column = ARG16;
      TblGrabFocus(tableP ? s_tableP : NULL, row, column);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGrabFocus(tableP=0x%08X, row=%d, column=%d)", tableP, row, column);
    }
    break;
    case sysTrapTblSetColumnEditIndicator: {
      // void TblSetColumnEditIndicator(in TableType *tableP, Int16 column, Boolean editIndicator)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t column = ARG16;
      uint8_t editIndicator = ARG8;
      TblSetColumnEditIndicator(tableP ? s_tableP : NULL, column, editIndicator);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnEditIndicator(tableP=0x%08X, column=%d, editIndicator=%d)", tableP, column, editIndicator);
    }
    break;
    case sysTrapTblSetRowStaticHeight: {
      // void TblSetRowStaticHeight(in TableType *tableP, Int16 row, Boolean staticHeight)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      uint8_t staticHeight = ARG8;
      TblSetRowStaticHeight(tableP ? s_tableP : NULL, row, staticHeight);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowStaticHeight(tableP=0x%08X, row=%d, staticHeight=%d)", tableP, row, staticHeight);
    }
    break;
    case sysTrapTblHasScrollBar: {
      // void TblHasScrollBar(in TableType *tableP, Boolean hasScrollBar)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      uint8_t hasScrollBar = ARG8;
      TblHasScrollBar(tableP ? s_tableP : NULL, hasScrollBar);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblHasScrollBar(tableP=0x%08X, hasScrollBar=%d)", tableP, hasScrollBar);
    }
    break;
    case sysTrapTblGetItemFont: {
      // FontID TblGetItemFont(in TableType *tableP, Int16 row, Int16 column)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      int16_t column = ARG16;
      FontID res = TblGetItemFont(tableP ? s_tableP : NULL, row, column);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetItemFont(tableP=0x%08X, row=%d, column=%d): %d", tableP, row, column, res);
    }
    break;
    case sysTrapTblSetItemFont: {
      // void TblSetItemFont(in TableType *tableP, Int16 row, Int16 column, FontID fontID)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      int16_t column = ARG16;
      uint8_t fontID = ARG8;
      TblSetItemFont(tableP ? s_tableP : NULL, row, column, fontID);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetItemFont(tableP=0x%08X, row=%d, column=%d, fontID=%d)", tableP, row, column, fontID);
    }
    break;
    case sysTrapTblGetItemPtr: {
      // void *TblGetItemPtr(in TableType *tableP, Int16 row, Int16 column)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      int16_t column = ARG16;
      void *res = TblGetItemPtr(tableP ? s_tableP : NULL, row, column);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetItemPtr(tableP=0x%08X, row=%d, column=%d): 0x%08X", tableP, row, column, r_res);
    }
    break;
    case sysTrapTblRowMasked: {
      // Boolean TblRowMasked(in TableType *tableP, Int16 row)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      Boolean res = TblRowMasked(tableP ? s_tableP : NULL, row);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblRowMasked(tableP=0x%08X, row=%d): %d", tableP, row, res);
    }
    break;
    case sysTrapTblSetRowMasked: {
      // void TblSetRowMasked(in TableType *tableP, Int16 row, Boolean masked)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      uint8_t masked = ARG8;
      TblSetRowMasked(tableP ? s_tableP : NULL, row, masked);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowMasked(tableP=0x%08X, row=%d, masked=%d)", tableP, row, masked);
    }
    break;
    case sysTrapTblSetColumnMasked: {
      // void TblSetColumnMasked(in TableType *tableP, Int16 column, Boolean masked)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t column = ARG16;
      uint8_t masked = ARG8;
      TblSetColumnMasked(tableP ? s_tableP : NULL, column, masked);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnMasked(tableP=0x%08X, column=%d, masked=%d)", tableP, column, masked);
    }
    break;
    case sysTrapTblGetNumberOfColumns: {
      // Int16 TblGetNumberOfColumns(in TableType *tableP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      Int16 res = TblGetNumberOfColumns(tableP ? s_tableP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetNumberOfColumns(tableP=0x%08X): %d", tableP, res);
    }
    break;
    case sysTrapTblGetTopRow: {
      // Int16 TblGetTopRow(in TableType *tableP)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      Int16 res = TblGetTopRow(tableP ? s_tableP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGetTopRow(tableP=0x%08X): %d", tableP, res);
    }
    break;
    case sysTrapTblSetSelection: {
      // void TblSetSelection(in TableType *tableP, Int16 row, Int16 column)
      uint32_t tableP = ARG32;
      TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
      int16_t row = ARG16;
      int16_t column = ARG16;
      TblSetSelection(tableP ? s_tableP : NULL, row, column);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetSelection(tableP=0x%08X, row=%d, column=%d)", tableP, row, column);
    }
    break;
  }
}
