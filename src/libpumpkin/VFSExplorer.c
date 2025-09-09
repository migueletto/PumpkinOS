#include <PalmOS.h>
#include <VFSMgr.h>

#include "pumpkin.h"
#include "VFSExplorer.h"
#include "debug.h"

#define MAX_NAME 64
#define MAX_PATH 256

struct VFSExplorer {
  UInt16 tableID;
  FormType *frm;
  TableType *tbl;
  FileRef d;
  Int16 selectedRow;
  char *selectedItem;
  char root[MAX_PATH];
  char path[MAX_PATH];
};

typedef struct {
  VFSExplorer *vfse;
  char *name;
  UInt32 iterator;
  Boolean dir;
} item_t;

static void item_draw(void *t, Int16 row, Int16 column, RectangleType *rect) {
  TableType *tbl = (TableType *)t;
  IndexedColorType fg, bg;
  item_t *item;
  Int16 len;

  if (tbl && rect && (item = TblGetItemPtr(tbl, row, column)) != NULL) {
    if (item->name) {
      if (row == item->vfse->selectedRow) {
        fg = UIColorGetTableEntryIndex(UIObjectSelectedForeground);
        bg = UIColorGetTableEntryIndex(UIFormFrame);
      } else {
        fg = UIColorGetTableEntryIndex(UIFieldText);
        bg = UIColorGetTableEntryIndex(UIFormFill);
      }
      WinSetBackColor(bg);
      WinSetTextColor(fg);

      len = StrLen(item->name);
      WinPaintChars(item->name, len, rect->topLeft.x, rect->topLeft.y);
      rect->topLeft.x += FntCharsWidth(item->name, len);
      rect->extent.x -= rect->topLeft.x;
    }
  }

  WinEraseRectangle(rect, 0);
}

VFSExplorer *VFSExplorerCreate(FormType *frm, UInt16 tableID, char *root) {
  VFSExplorer *vfse;

  if (root == NULL) {
    debug(DEBUG_ERROR, "VFSExplorer", "null root path");
    return NULL;
  }

  if ((vfse = MemPtrNew(sizeof(VFSExplorer))) == NULL) {
    return NULL;
  }

  vfse->tableID = tableID;
  StrNCopy(vfse->root, root, MAX_PATH-1);
  StrNCopy(vfse->path, root, MAX_PATH-1);

  VFSExplorerRefresh(frm, vfse);

  return vfse;
}

void VFSExplorerRefresh(FormType *frm, VFSExplorer *vfse) {
  FileInfoType info;
  TableType *tbl;
  UInt32 iterator;
  FileRef d;
  UInt16 index, row, rows;
  item_t *item, *oldItem;
  char name[MAX_NAME];

  if (frm == NULL) {
    frm = FrmGetActiveForm();
  }

  if ((index = FrmGetObjectIndex(frm, vfse->tableID)) == 0xffff) {
    debug(DEBUG_ERROR, "VFSExplorer", "invalid object ID %u", vfse->tableID);
    return;
  }

  if (FrmGetObjectType(frm, index) != frmTableObj) {
    debug(DEBUG_ERROR, "VFSExplorer", "object ID %u is not a table", vfse->tableID);
    return;
  }

  if ((tbl = FrmGetObjectPtr(frm, index)) == NULL) {
    debug(DEBUG_ERROR, "VFSExplorer", "table ID %u not found", vfse->tableID);
    return;
  }

  if (VFSFileOpen(1, vfse->path, vfsModeRead, &d) != errNone) {
    return;
  }

  vfse->frm = frm;
  vfse->tbl = tbl;
  vfse->d = d;
  vfse->selectedRow = -1;

  TblSetColumnUsable(vfse->tbl, 0, true);
  TblSetCustomDrawProcedure(vfse->tbl, 0, item_draw);
  rows = TblGetNumberOfRows(vfse->tbl);

  iterator = vfsIteratorStart;
  for (row = 0; row < rows; row++) {
    MemSet(&info, sizeof(info), 0);
    info.nameP = name;
    info.nameBufLen = MAX_NAME - 1;
    if (VFSDirEntryEnumerate(vfse->d, &iterator, &info) != errNone) break;
    if (iterator == vfsIteratorStop) break;
    if ((item = MemPtrNew(sizeof(item_t))) == NULL) break;
    oldItem = TblGetItemPtr(vfse->tbl, row, 0);
    if (oldItem) {
      if (oldItem->name) MemPtrFree(oldItem->name);
      MemPtrFree(oldItem);
    }
    TblSetItemStyle(vfse->tbl, row, 0, customTableItem);
    item->vfse = vfse;
    item->name = StrDup(info.nameP);
    item->dir = (info.attributes == vfsFileAttrDirectory);
    item->iterator = iterator;
    TblSetItemPtr(vfse->tbl, row, 0, item);
    TblSetRowUsable(vfse->tbl, row, true);
  }
  for (; row < rows; row++) {
    oldItem = TblGetItemPtr(vfse->tbl, row, 0);
    if (oldItem) {
      if (oldItem->name) MemPtrFree(oldItem->name);
      MemPtrFree(oldItem);
    }
    TblSetItemPtr(vfse->tbl, row, 0, NULL);
    TblSetRowUsable(vfse->tbl, row, false);
  }
  TblMarkTableInvalid(vfse->tbl);
}

Boolean VFSExplorerHandleEvent(VFSExplorer *vfse, EventType *event) {
  Int16 row;
  item_t *item;
  Boolean handled = false;

  if (vfse && event) {
    switch (event->eType) {
      case tblSelectEvent:
        if (event->data.tblSelect.tableID == vfse->tableID) {
          row = event->data.tblSelect.row;
          item = TblGetItemPtr(vfse->tbl, row, 0);
          debug(DEBUG_INFO, "VFSExplorer", "VFSExplorerHandleEvent tblSelectEvent row %d [%s]", row, item->name);
          if (vfse->selectedRow != -1) {
            TblMarkRowInvalid(vfse->tbl, vfse->selectedRow);
          }
          TblMarkRowInvalid(vfse->tbl, row);
          vfse->selectedRow = row;
          vfse->selectedItem = item->name;
          TblRedrawTable(vfse->tbl);
          if (item->dir) {
            StrNCat(vfse->path, item->name, MAX_PATH-1);
            StrNCat(vfse->path, "/", MAX_PATH-1);
            debug(DEBUG_INFO, "VFSExplorer", "change to subdir [%s]", vfse->path);
            VFSExplorerRefresh(vfse->frm, vfse);
            TblRedrawTable(vfse->tbl);
            handled = true;
          }
        }
        break;
      case tblExitEvent:
        if (event->data.tblExit.tableID == vfse->tableID) {
          debug(DEBUG_INFO, "VFSExplorer", "VFSExplorerHandleEvent tblExitEvent");
          vfse->selectedRow = -1;
          handled = true;
        }
        break;
      default:
        break;
    }
  }

  return handled;
}

char *VFSExplorerCurrentPath(VFSExplorer *vfse) {
  return vfse ? vfse->path : NULL;
}

char *VFSExplorerSelectedItem(VFSExplorer *vfse) {
  char *name = NULL;

  if (vfse && vfse->selectedItem) {
    name = vfse->selectedItem;
  }

  return name;
}

void VFSExplorerDestroy(VFSExplorer *vfse) {
  UInt16 row, rows;
  item_t *item;

  if (vfse) {
    if (vfse->d) {
      VFSFileClose(vfse->d);
    }
    if (vfse->tbl) {
      rows = TblGetNumberOfRows(vfse->tbl);
      for (row = 0; row < rows; row++) {
        if ((item = TblGetItemPtr(vfse->tbl, row, 0)) != NULL) {
          TblSetItemPtr(vfse->tbl, row, 0, NULL);
          if (item->name) MemPtrFree(item->name);
          MemPtrFree(item);
        }
      }
      TblMarkTableInvalid(vfse->tbl);
    }
    MemPtrFree(vfse);
  }
}
