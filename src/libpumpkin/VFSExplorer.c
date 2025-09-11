#include <PalmOS.h>
#include <VFSMgr.h>

#include "pumpkin.h"
#include "VFSExplorer.h"
#include "debug.h"

#define ITEMS_CAP 32
#define MAX_NAME 64
#define MAX_PATH 256
#define PARENT_DIR ".."

struct VFSExplorer {
  UInt16 tableID, upID, downID;
  FormType *frm;
  TableType *tbl;
  FileRef d;
  struct item_t *items;
  int itemsCap;
  int itemsLen;
  int topItem;
  Int16 selectedRow;
  Int16 selectedItem;
  char root[MAX_PATH];
  char path[MAX_PATH];
};

typedef struct item_t {
  VFSExplorer *vfse;
  char *name;
  UInt32 size;
  Boolean dir;
} item_t;

static void item_draw(void *t, Int16 row, Int16 column, RectangleType *rect) {
  TableType *tbl = (TableType *)t;
  IndexedColorType fg, bg;
  RGBColorType fgRGB, bgRGB;
  item_t *item;
  Int16 len, width;
  char *s, buf[32];

  if (tbl && rect && (item = TblGetItemPtr(tbl, row, column)) != NULL) {
    switch (column) {
      case 0:
        s = item->name;
        break;
      case 1:
        StrNPrintF(buf, sizeof(buf)-1, "%u", item->size);
        s = buf;
        break;
      default:
        s = "";
        break;
    }

    if (row == item->vfse->selectedRow) {
      bg = UIColorGetTableEntryIndex(UIFormFrame);
      WinSetBackColor(bg);
      fg = UIColorGetTableEntryIndex(UIObjectSelectedForeground);
      WinSetTextColor(fg);
    } else {
      if (row % 2) {
        bgRGB.r = 0xf0;
        bgRGB.g = 0xf0;
        bgRGB.b = 0xff;
      } else {
        bgRGB.r = 0xf8;
        bgRGB.g = 0xf8;
        bgRGB.b = 0xff;
      }
      WinSetBackColorRGB(&bgRGB, NULL);
      fg = UIColorGetTableEntryIndex(UIFieldText);
      if (item->dir) {
        fgRGB.r = 0x20;
        fgRGB.g = 0x20;
        fgRGB.b = 0xff;
      } else {
        fgRGB.r = 0x00;
        fgRGB.g = 0x00;
        fgRGB.b = 0x00;
      }
      WinSetTextColorRGB(&fgRGB, NULL);
    }

    if (s && s[0]) {
      len = StrLen(s);
      WinPaintChars(s, len, rect->topLeft.x, rect->topLeft.y);
      width = FntCharsWidth(s, len);
      rect->topLeft.x += width;
      rect->extent.x -= width;
    }

    WinEraseRectangle(rect, 0);
  }
}

VFSExplorer *VFSExplorerCreate(FormType *frm, UInt16 tableID, UInt16 upID, UInt16 downID, char *root) {
  VFSExplorer *vfse;

  if (root == NULL) {
    debug(DEBUG_ERROR, "VFSExplorer", "null root path");
    return NULL;
  }

  if ((vfse = MemPtrNew(sizeof(VFSExplorer))) == NULL) {
    return NULL;
  }

  vfse->tableID = tableID;
  vfse->upID = upID;
  vfse->downID = downID;
  StrNCopy(vfse->root, root, MAX_PATH-1);
  StrNCopy(vfse->path, root, MAX_PATH-1);

  VFSExplorerRefresh(frm, vfse, false);

  return vfse;
}

static void item_insert(VFSExplorer *vfse, char *name, Boolean dir, UInt32 i) {
  if (i == vfse->itemsCap) {
    if (vfse->itemsCap == 0) {
      vfse->itemsCap = ITEMS_CAP;
      vfse->items = sys_calloc(vfse->itemsCap, sizeof(item_t));
    } else {
      vfse->itemsCap += ITEMS_CAP;
      vfse->items = sys_realloc(vfse->items, vfse->itemsCap * sizeof(item_t));
    }
  }

  if (vfse->items[i].name) {
    MemPtrFree(vfse->items[i].name);
  }
  vfse->items[i].name = StrDup(name);
  vfse->items[i].dir = dir;
  vfse->items[i].vfse = vfse;
}

static Int32 compareItem(void *e1, void *e2, void *p) {
  item_t *item1 = e1;
  item_t *item2 = e2;

  if (item1->dir == item2->dir) {
    return StrCompare(item1->name, item2->name);
  }

  if (item1->dir && !item2->dir) {
    return -1;
  }

  return 1;
}

void VFSExplorerRefresh(FormType *frm, VFSExplorer *vfse, Boolean redraw) {
  FileInfoType info;
  TableType *tbl;
  ControlType *ctl;
  UInt32 iterator;
  FileRef d;
  UInt16 index, row, rows;
  Boolean enabled;
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

  if (vfse->d == NULL) {
    if (VFSFileOpen(1, vfse->path, vfsModeRead, &d) == errNone) {
      vfse->d = d;
    }
  }

  vfse->frm = frm;
  vfse->tbl = tbl;
  vfse->selectedRow = -1;

  vfse->itemsLen = 0;
  if (StrCompare(vfse->path, vfse->root) != 0) {
    item_insert(vfse, PARENT_DIR, true, vfse->itemsLen++);
  }

  if (vfse->d != NULL) {
    iterator = vfsIteratorStart;
    for (;; vfse->itemsLen++) {
      MemSet(&info, sizeof(info), 0);
      MemSet(name, sizeof(MAX_NAME), 0);
      info.nameP = name;
      info.nameBufLen = MAX_NAME - 1;
      if (VFSDirEntryEnumerate(vfse->d, &iterator, &info) != errNone) break;
      if (iterator == vfsIteratorStop) break;
      item_insert(vfse, info.nameP, info.attributes == vfsFileAttrDirectory, vfse->itemsLen);
    }

    SysQSortP(vfse->items, vfse->itemsLen, sizeof(item_t), compareItem, vfse);
  }

  TblSetColumnUsable(vfse->tbl, 0, true);
  TblSetColumnUsable(vfse->tbl, 1, true);
  TblSetCustomDrawProcedure(vfse->tbl, 0, item_draw);
  TblSetCustomDrawProcedure(vfse->tbl, 1, item_draw);
  rows = TblGetNumberOfRows(vfse->tbl);

  for (row = 0; row < rows && (row + vfse->topItem) < vfse->itemsLen; row++) {
    TblMarkRowInvalid(vfse->tbl, row);
    TblSetItemStyle(vfse->tbl, row, 0, customTableItem);
    TblSetItemStyle(vfse->tbl, row, 1, customTableItem);
    TblSetItemInt(vfse->tbl, row, 0, vfse->topItem + row);
    TblSetItemInt(vfse->tbl, row, 1, vfse->topItem + row);
    TblSetItemPtr(vfse->tbl, row, 0, &vfse->items[vfse->topItem + row]);
    TblSetItemPtr(vfse->tbl, row, 1, &vfse->items[vfse->topItem + row]);
    TblSetRowUsable(vfse->tbl, row, true);
  }
  for (; row < rows; row++) {
    TblSetItemStyle(vfse->tbl, row, 0, customTableItem);
    TblSetItemStyle(vfse->tbl, row, 1, customTableItem);
    TblSetItemInt(vfse->tbl, row, 0, -1);
    TblSetItemInt(vfse->tbl, row, 1, -1);
    TblSetItemPtr(vfse->tbl, row, 0, NULL);
    TblSetItemPtr(vfse->tbl, row, 1, NULL);
    TblSetRowUsable(vfse->tbl, row, false);
  }
  TblMarkTableInvalid(vfse->tbl);

  if (redraw) {
    TblRedrawTable(vfse->tbl);
  }

  if ((index = FrmGetObjectIndex(frm, vfse->upID)) != 0xffff) {
    if (FrmGetObjectType(frm, index) == frmControlObj) {
      if ((ctl = FrmGetObjectPtr(frm, index)) != NULL) {
        enabled = vfse->itemsLen > 0 && vfse->itemsLen > rows && vfse->topItem > 0;
        CtlSetLabel(ctl, enabled ? "\x01" : "\x03");
        CtlSetEnabled(ctl, enabled);
      }
    }
  }

  if ((index = FrmGetObjectIndex(frm, vfse->downID)) != 0xffff) {
    if (FrmGetObjectType(frm, index) == frmControlObj) {
      if ((ctl = FrmGetObjectPtr(frm, index)) != NULL) {
        enabled = vfse->itemsLen > 0 && vfse->itemsLen > rows && vfse->topItem < vfse->itemsLen-1;
        CtlSetLabel(ctl, enabled ? "\x02" : "\x04");
        CtlSetEnabled(ctl, enabled);
      }
    }
  }
}

Boolean VFSExplorerHandleEvent(VFSExplorer *vfse, EventType *event) {
  Int16 row, i;
  item_t *item;
  Boolean handled = false;

  if (vfse && event) {
    switch (event->eType) {
      case tblSelectEvent:
        if (event->data.tblSelect.tableID == vfse->tableID) {
          row = event->data.tblSelect.row;
          i = TblGetItemInt(vfse->tbl, row, 0);
          item = &vfse->items[i];
          debug(DEBUG_INFO, "VFSExplorer", "VFSExplorerHandleEvent tblSelectEvent row %d item %d [%s] dir %d", row, i, item->name, item->dir);
          if (vfse->selectedRow != -1) {
            TblMarkRowInvalid(vfse->tbl, vfse->selectedRow);
          }
          TblMarkRowInvalid(vfse->tbl, row);
          vfse->selectedRow = row;
          vfse->selectedItem = i;
          if (item->dir) {
            if (StrCompare(item->name, PARENT_DIR) == 0) {
              for (i = StrLen(vfse->path) - 2; i >= 0 && vfse->path[i]; i--) {
                if (vfse->path[i] == '/') {
                  vfse->path[i+1] = 0;
                  break;
                }
              }
              if (vfse->d) {
                VFSFileClose(vfse->d);
                vfse->d = NULL;
              }
              debug(DEBUG_INFO, "VFSExplorer", "change to parent dir [%s]", vfse->path);
              vfse->topItem = 0;
              VFSExplorerRefresh(vfse->frm, vfse, false);
            }
            handled = true;
          }
          TblRedrawTable(vfse->tbl);
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

  if (vfse && vfse->selectedItem != -1) {
    name = vfse->items[vfse->selectedItem].name;
  }

  return name;
}

void VFSExplorerEnter(VFSExplorer *vfse) {
  if (vfse && vfse->selectedRow != -1) {
    StrNCat(vfse->path, vfse->items[vfse->selectedItem].name, MAX_PATH-1);
    StrNCat(vfse->path, "/", MAX_PATH-1);
    debug(DEBUG_INFO, "VFSExplorer", "change to sub dir [%s]", vfse->path);
    if (vfse->d) {
      VFSFileClose(vfse->d);
      vfse->d = NULL;
    }
    vfse->topItem = 0;
    VFSExplorerRefresh(vfse->frm, vfse, true);
  }
}

void VFSExplorerPaginate(VFSExplorer *vfse, Int16 n) {
  if (vfse && vfse->itemsLen && n) {
    vfse->topItem += n;
    if (vfse->topItem < 0) vfse->topItem = 0;
    else if (vfse->topItem >= vfse->itemsLen) vfse->topItem = vfse->itemsLen - 1;
    VFSExplorerRefresh(vfse->frm, vfse, true);
  }
}

void VFSExplorerDestroy(VFSExplorer *vfse) {
  UInt16 row, rows, i;

  if (vfse) {
    if (vfse->d) {
      VFSFileClose(vfse->d);
    }

    if (vfse->tbl) {
      rows = TblGetNumberOfRows(vfse->tbl);
      for (row = 0; row < rows; row++) {
        TblSetItemInt(vfse->tbl, row, 0, -1);
        TblSetItemInt(vfse->tbl, row, 1, -1);
        TblSetItemPtr(vfse->tbl, row, 0, NULL);
        TblSetItemPtr(vfse->tbl, row, 1, NULL);
      }
      TblMarkTableInvalid(vfse->tbl);
    }

    for (i = 0; i < vfse->itemsLen; i++) {
      if (vfse->items[i].name) MemPtrFree(vfse->items[i].name);
    }
    sys_free(vfse->items);

    MemPtrFree(vfse);
  }
}
