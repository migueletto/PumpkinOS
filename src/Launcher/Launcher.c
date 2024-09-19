#include <PalmOS.h>
#include <Control.h>
#include <BmpGlue.h>
#include <VFSMgr.h>

#include "sys.h"
#include "resedit.h"
#include "script.h"
#include "thread.h"
#include "mutex.h"
#include "pwindow.h"
#include "media.h"
#include "ptr.h"
#include "vfs.h"
#include "bytes.h"
#include "AppRegistry.h"
#include "storage.h"
#include "pumpkin.h"
#include "emulation/emupalmosinc.h"
#include "debug.h"
#include "xalloc.h"

#include "resource.h"

#define AppName     "Launcher"
#define AppID       'Lnch'

#define MAX_ITEMS   1024
#define MAX_NAME    256

#define PARENT_DIR  ".."

#define APP_CELL_WIDTH   50
#define APP_CELL_HEIGHT  35

#define BLACK   1
#define WHITE   0

#define BATTERY_WIDTH 40

// When DIA is closed: Title (15) + 6*35 + Taskbar (15) = 240 = 160 + 80

typedef enum {
  launcher_app,
  launcher_app_small,
  launcher_db,
  launcher_rsrc,
  launcher_rec,
  launcher_file,
  launcher_task,
  launcher_registry
} launcher_mode_t;

typedef struct {
  LocalID dbID;
  UInt32 creator;
  UInt32 type;
  UInt32 size;
  UInt16 id, index;
  UInt16 numRecs;
  UInt16 compat, code, width, height;
  Boolean rsrc, dir, m68k, uptodate;
  char name[dmDBNameLength];
  char *info;
  WinHandle iconWh;
  WinHandle invIconWh;
  UInt16 bmpHeight;
  uint32_t (*pilot_main)(uint16_t code, void *param, uint16_t flags);
} launcher_item_t;

typedef struct {
  mutex_t *mutex;
  launcher_mode_t mode;
  launcher_item_t item[MAX_ITEMS];
  int numItems, topItem;
  UInt16 cellWidth, cellHeight;
  char title[MAX_NAME];
  Boolean finish, top, deploy, reload;
  Int16 lastMinute;
  RectangleType gadRect, sclRect;
  UInt32 filterDbType, filterCreator, filterResType, filterId;
  Boolean filterRsrc;
  LocalID dbID;
  char path[MAX_NAME];
  char name[MAX_NAME];
  int sort, dir;
  int  prev, prevX, prevY;
  int num, x[16];
  MenuBarType *mainMenu, *appListMenu;
} launcher_data_t;

static const dynamic_form_item_t dbFilterItems[] = {
  { "Type:",        alphaItem,    4 },
  { "Creator:",     alphaItem,    4 },
  { "Resource DB",  checkboxItem, 0 },
  { NULL,           alphaItem,    0 }
};

static const dynamic_form_item_t rsrcFilterItems[] = {
  { "Type:", alphaItem, 4 },
  { "ID:",   alphaItem, 5 },
  { NULL,    alphaItem, 0 }
};

static Boolean ItemsGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
static void UpdateStatus(FormPtr frm, launcher_data_t *data, Boolean title);
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);

static void ErrorDialog(char *msg, Err err, UInt16 num) {
  char cod[64], s[8];

  if (err) {
    sys_snprintf(cod, sizeof(cod)-1, "\nError code: %d", err);
    s[0] = 0;
    if (num) sys_snprintf(s, sizeof(s)-1, "(%x)", num);
    FrmCustomAlert(ErrorAlert, msg, cod, s);
  } else {
    FrmCustomAlert(InfoAlert, msg, "", "");
  }
}

static void OpenErrorDialog(char *name, Err err) {
  char buf[256];

  sys_snprintf(buf, sizeof(buf)-1, "Could not open '%s' for writing.", name);
  ErrorDialog(buf, err, 0);
}

static void setTextColor(UInt16 black) {
  IndexedColorType c = UIColorGetTableEntryIndex(black ? UIFieldText : UIObjectSelectedForeground);
  WinSetTextColor(c);
}

static void setBackColor(UInt16 black) {
  IndexedColorType c = UIColorGetTableEntryIndex(black ? UIFormFrame : UIFormFill);
  WinSetBackColor(c);
}

static void updateCounts(launcher_item_t *item) {
  DmOpenRef dbRef;
  UInt32 numRecords, dataBytes;

  if (item && !item->uptodate) {
    if ((dbRef = DmOpenDatabase(0, item->dbID, dmModeReadOnly)) != NULL) {
      DmCloseDatabase(dbRef);
      DmDatabaseSize(0, item->dbID, &numRecords, NULL, &dataBytes);
      item->numRecs = numRecords;
      item->size = dataBytes;
      item->uptodate = true;
    }
  }
}

static Int16 compareItemName(void *e1, void *e2, Int32 other) {
  launcher_item_t *item1, *item2;

  item1 = (launcher_item_t *)(other ? e2 : e1);
  item2 = (launcher_item_t *)(other ? e1 : e2);

  if (item1 && item2) {
    return StrCaselessCompare(item1->name, item2->name);
  }

  return 0;
}

static Int16 compareItemTypeCreator(void *e1, void *e2, Int32 other) {
  launcher_item_t *item1, *item2;

  item1 = (launcher_item_t *)(other ? e2 : e1);
  item2 = (launcher_item_t *)(other ? e1 : e2);

  if (item1 && item2) {
    if (item1->type < item2->type) return -1;
    if (item1->type > item2->type) return 1;
    if (item1->creator < item2->creator) return -1;
    if (item1->creator > item2->creator) return 1;
  }

  return 0;
}

static Int16 compareItemTypeId(void *e1, void *e2, Int32 other) {
  launcher_item_t *item1, *item2;

  item1 = (launcher_item_t *)(other ? e2 : e1);
  item2 = (launcher_item_t *)(other ? e1 : e2);

  if (item1 && item2) {
    if (item1->type < item2->type) return -1;
    if (item1->type > item2->type) return 1;
    if (item1->id < item2->id) return -1;
    if (item1->id > item2->id) return 1;
  }

  return 0;
}

static Int16 compareItemCreatorType(void *e1, void *e2, Int32 other) {
  launcher_item_t *item1, *item2;

  item1 = (launcher_item_t *)(other ? e2 : e1);
  item2 = (launcher_item_t *)(other ? e1 : e2);

  if (item1 && item2) {
    if (item1->creator < item2->creator) return -1;
    if (item1->creator > item2->creator) return 1;
    if (item1->type < item2->type) return -1;
    if (item1->type > item2->type) return 1;
  }

  return 0;
}

static Int16 compareItemIdType(void *e1, void *e2, Int32 other) {
  launcher_item_t *item1, *item2;

  item1 = (launcher_item_t *)(other ? e2 : e1);
  item2 = (launcher_item_t *)(other ? e1 : e2);

  if (item1 && item2) {
    if (item1->id < item2->id) return -1;
    if (item1->id > item2->id) return 1;
    if (item1->type < item2->type) return -1;
    if (item1->type > item2->type) return 1;
  }

  return 0;
}

static Int16 compareItemId(void *e1, void *e2, Int32 other) {
  launcher_item_t *item1, *item2;

  item1 = (launcher_item_t *)(other ? e2 : e1);
  item2 = (launcher_item_t *)(other ? e1 : e2);

  if (item1 && item2) {
    if (item1->id < item2->id) return -1;
    if (item1->id > item2->id) return 1;
  }

  return 0;
}

static Int16 compareItemRecs(void *e1, void *e2, Int32 other) {
  launcher_item_t *item1, *item2;

  item1 = (launcher_item_t *)(other ? e2 : e1);
  item2 = (launcher_item_t *)(other ? e1 : e2);

  if (item1 && item2) {
    updateCounts(item1);
    updateCounts(item2);
    if (item1->numRecs < item2->numRecs) return -1;
    if (item1->numRecs > item2->numRecs) return 1;
  }

  return 0;
}

static Int16 compareItemIndex(void *e1, void *e2, Int32 other) {
  launcher_item_t *item1, *item2;

  item1 = (launcher_item_t *)(other ? e2 : e1);
  item2 = (launcher_item_t *)(other ? e1 : e2);

  if (item1 && item2) {
    if (item1->index < item2->index) return -1;
    if (item1->index > item2->index) return 1;
  }

  return 0;
}

static Int16 compareItemSize(void *e1, void *e2, Int32 other) {
  launcher_item_t *item1, *item2;

  item1 = (launcher_item_t *)(other ? e2 : e1);
  item2 = (launcher_item_t *)(other ? e1 : e2);

  if (item1 && item2) {
    updateCounts(item1);
    updateCounts(item2);
    if (item1->size < item2->size) return -1;
    if (item1->size > item2->size) return 1;
  }

  return 0;
}

static Int16 compareItemCompat(void *e1, void *e2, Int32 other) {
  launcher_item_t *item1, *item2;

  item1 = (launcher_item_t *)(other ? e2 : e1);
  item2 = (launcher_item_t *)(other ? e1 : e2);

  if (item1 && item2) {
    if (item1->compat < item2->compat) return -1;
    if (item1->compat > item2->compat) return 1;
    if (item1->code < item2->code) return -1;
    if (item1->code > item2->code) return 1;
  }

  return 0;
}

static Int16 compareItemWidthHeight(void *e1, void *e2, Int32 other) {
  launcher_item_t *item1, *item2;

  item1 = (launcher_item_t *)(other ? e2 : e1);
  item2 = (launcher_item_t *)(other ? e1 : e2);

  if (item1 && item2) {
    if (item1->width < item2->width) return -1;
    if (item1->width > item2->width) return 1;
    if (item1->height < item2->height) return -1;
    if (item1->height > item2->height) return 1;
  }

  return 0;
}

static void sortItems(launcher_data_t *data) {
  switch (data->mode) {
    case launcher_app:
      break;
    case launcher_app_small:
      switch (data->sort) {
        case 0:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemName, data->dir);
          break;
        case 1:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemCreatorType, data->dir);
          break;
        case 2:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemSize, data->dir);
          break;
      }
      break;
    case launcher_db:
      switch (data->sort) {
        case 0:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemName, data->dir);
          break;
        case 1:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemTypeCreator, data->dir);
          break;
        case 2:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemCreatorType, data->dir);
          break;
        case 3:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemRecs, data->dir);
          break;
        case 4:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemSize, data->dir);
          break;
      }
      break;
    case launcher_rsrc:
      switch (data->sort) {
        case 0:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemTypeId, data->dir);
          break;
        case 1:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemIdType, data->dir);
          break;
        case 2:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemSize, data->dir);
          break;
      }
      break;
    case launcher_rec:
      switch (data->sort) {
        case 0:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemIndex, data->dir);
          break;
        case 1:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemSize, data->dir);
          break;
      }
      break;
    case launcher_file:
      switch (data->sort) {
        case 0:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemName, data->dir);
          break;
        case 1:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemSize, data->dir);
          break;
      }
      break;
    case launcher_task:
      switch (data->sort) {
        case 0:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemId, data->dir);
          break;
        case 1:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemName, data->dir);
          break;
      }
      break;
    case launcher_registry:
      switch (data->sort) {
        case 0:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemTypeCreator, data->dir);
          break;
        case 1:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemWidthHeight, data->dir);
          break;
        case 2:
          SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemCompat, data->dir);
          break;
      }
      break;
  }
}

static void launcherResetItems(launcher_data_t *data) {
  int i;

  for (i = 0; i < data->numItems; i++) {
    if (data->item[i].iconWh) WinDeleteWindow(data->item[i].iconWh, false);
    if (data->item[i].invIconWh) WinDeleteWindow(data->item[i].invIconWh, false);
    if (data->item[i].info) xfree(data->item[i].info);
  }

  MemSet(data->item, sizeof(data->item), 0);
  data->numItems = 0;
}

static void launcherScanApps(launcher_data_t *data) {
  DmSearchStateType stateInfo;
  Boolean newSearch;
  UInt32 creator;
  UInt16 cardNo, attr;
  Coord bw, bh;
  DmOpenRef dbRef;
  MemHandle iconRes, defaultIconRes;
  RectangleType rect;
  BitmapType *iconBmp;
  WinHandle old;
  int i;

  launcherResetItems(data);

  data->cellWidth = APP_CELL_WIDTH;
  data->cellHeight = APP_CELL_HEIGHT;
  defaultIconRes = DmGetResource(iconType, 10000);

  for (newSearch = true, i = 0; i < MAX_ITEMS; newSearch = false) {
    if (DmGetNextDatabaseByTypeCreator(newSearch, &stateInfo, sysFileTApplication, 0, false, &cardNo, &data->item[i].dbID) != errNone) {
      break;
    }
    if (DmDatabaseInfo(cardNo, data->item[i].dbID, data->item[i].name, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creator) == errNone) {
      if (StrCompare(data->item[i].name, AppName) && (attr & dmHdrAttrResDB)) {
        debug(DEBUG_INFO, "Launcher", "found app \"%s\"", data->item[i].name);
        data->item[i].type = sysFileTApplication;
        data->item[i].creator = creator;
        data->item[i].rsrc = true;
        if ((dbRef = DmOpenDatabase(cardNo, data->item[i].dbID, dmModeReadOnly)) != NULL) {
          iconRes = DmGet1Resource(iconType, 1000);
          if (iconRes == NULL) iconRes = defaultIconRes;
          if (iconRes != NULL) {
            if ((iconBmp = MemHandleLock(iconRes)) != NULL) {
              BmpGetDimensions(iconBmp, &bw, &bh, NULL);
              data->item[i].bmpHeight = bh < 22 ? bh : 22; // max allowed icon height
              RctSetRectangle(&rect, 0, 0, data->cellWidth, bh);

              data->item[i].iconWh = WinCreateOffscreenWindow(data->cellWidth, bh, nativeFormat, NULL);
              old = WinSetDrawWindow(data->item[i].iconWh);
              WinEraseRectangle(&rect, 0);
              WinPaintBitmap(iconBmp, (data->cellWidth - bw) / 2, 0);
              WinSetDrawWindow(old);

              data->item[i].invIconWh = WinCreateOffscreenWindow(data->cellWidth, bh, nativeFormat, NULL);
              old = WinSetDrawWindow(data->item[i].invIconWh);
              setBackColor(BLACK);
              WinEraseRectangle(&rect, 0);
              setBackColor(WHITE);
              WinPaintBitmap(iconBmp, (data->cellWidth - bw) / 2, 0);
              WinSetDrawWindow(old);

              MemHandleUnlock(iconRes);
            }
            DmReleaseResource(iconRes);
          }
          i++;
          DmCloseDatabase(dbRef);
        }
      }
    }
  }
  DmReleaseResource(defaultIconRes);

  data->numItems = i;
  SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemName, 0);
  debug(DEBUG_INFO, "Launcher", "found %d apps", data->numItems);
}

static void launcherScanAppsSmall(launcher_data_t *data) {
  DmSearchStateType stateInfo;
  Boolean newSearch;
  UInt16 cardNo, attr;
  UInt32 creator;
  Coord bw, bh;
  DmOpenRef dbRef;
  MemHandle iconRes;
  RectangleType rect;
  BitmapType *iconBmp;
  WinHandle oldw;
  FontID old;
  int i;

  launcherResetItems(data);

  old = FntSetFont(stdFont);
  data->cellHeight = FntCharHeight() + 2;
  data->cellWidth = data->gadRect.extent.x;
  FntSetFont(old);
  data->prev = -1;

  for (newSearch = true, i = 0; i < MAX_ITEMS; newSearch = false) {
    if (DmGetNextDatabaseByTypeCreator(newSearch, &stateInfo, sysFileTApplication, 0, false, &cardNo, &data->item[i].dbID) != errNone) {
      break;
    }
    if (DmDatabaseInfo(cardNo, data->item[i].dbID, data->item[i].name, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creator) == errNone) {
      if (StrCompare(data->item[i].name, AppName) && (attr & dmHdrAttrResDB)) {
        debug(DEBUG_INFO, "Launcher", "found small app \"%s\"", data->item[i].name);
        data->item[i].type = sysFileTApplication;
        data->item[i].creator = creator;
        data->item[i].rsrc = true;
        if ((dbRef = DmOpenDatabase(cardNo, data->item[i].dbID, dmModeReadOnly)) != NULL) {
          iconRes = DmGet1Resource(iconType, 1001);
          if (iconRes == NULL) {
            iconRes = DmGetResource(iconType, 10001);
          }
          if (iconRes != NULL) {
            if ((iconBmp = MemHandleLock(iconRes)) != NULL) {
              data->item[i].iconWh = WinCreateOffscreenWindow(data->cellHeight, data->cellHeight, nativeFormat, NULL);
              BmpGetDimensions(iconBmp, &bw, &bh, NULL);
              oldw = WinSetDrawWindow(data->item[i].iconWh);
              RctSetRectangle(&rect, 0, 0, data->cellHeight, data->cellHeight);
              WinEraseRectangle(&rect, 0);
              WinPaintBitmap(iconBmp, (data->cellHeight - bw)/2, 0);
              WinSetDrawWindow(oldw);
              MemHandleUnlock(iconRes);
            }
            DmReleaseResource(iconRes);
          }
          i++;
          DmCloseDatabase(dbRef);
        }
      }
    }
  }

  data->numItems = i;
  SysQSort(data->item, data->numItems, sizeof(launcher_item_t), compareItemName, 0);
  debug(DEBUG_INFO, "Launcher", "found %d small apps", data->numItems);
}

static void launcherScanDBs(launcher_data_t *data) {
  DmSearchStateType stateInfo;
  Boolean newSearch;
  UInt16 cardNo, attr;
  UInt32 creator, type;
  FontID old;
  int i;

  launcherResetItems(data);

  old = FntSetFont(stdFont);
  data->cellHeight = FntCharHeight() + 2;
  data->cellWidth = data->gadRect.extent.x;
  FntSetFont(old);

  for (newSearch = true, i = 0; i < MAX_ITEMS; newSearch = false) {
    if (DmGetNextDatabaseByTypeCreator(newSearch, &stateInfo, data->filterDbType, data->filterCreator, false, &cardNo, &data->item[i].dbID) != errNone) {
      break;
    }
    if (DmDatabaseInfo(cardNo, data->item[i].dbID, data->item[i].name, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, &creator) == errNone) {
      if (!data->filterRsrc || (attr & dmHdrAttrResDB)) {
        debug(DEBUG_INFO, "Launcher", "found DB \"%s\"", data->item[i].name);
        data->item[i].type = type;
        data->item[i].creator = creator;
        data->item[i].rsrc = (attr & dmHdrAttrResDB);
        data->item[i].uptodate = false;
        data->item[i].numRecs = 0;
        data->item[i].size = 0;
        i++;
      }
    }
  }

  data->numItems = i;
  sortItems(data);
  debug(DEBUG_INFO, "Launcher", "found %d DBs", data->numItems);
}


static void launcherScanResources(launcher_data_t *data) {
  MemHandle h;
  UInt16 num, index, i;
  FontID old;
  DmOpenRef dbRef;
  DmResType type;
  DmResID id;
  AlertTemplateType *alert;
  MenuBarType *menu;
  BitmapType *bmp;
  FontType *font;
  LocalID localId;
  UInt16 sys_os, sys_cpu, sys_size, x, y, width, height;
  UInt8 version, depth, *b;
  Boolean chain;
  UInt32 *value, size;
  char *s, buf[256];
  void *p;

  index = 0;
  if ((dbRef = DmOpenDatabase(0, data->dbID, dmModeReadOnly)) != NULL) {
    launcherResetItems(data);

    old = FntSetFont(stdFont);
    data->cellHeight = FntCharHeight() + 2;
    data->cellWidth = data->gadRect.extent.x;
    FntSetFont(old);

    num = DmNumResources(dbRef);
    for (i = 0; i < num && index < MAX_ITEMS; i++) {
      if (DmResourceInfo(dbRef, i, &type, &id, NULL) != errNone) break;
      if (data->filterResType != 0 && data->filterResType != type) continue;
      if (data->filterId != 0xffffffff && data->filterId != id) continue;
      if ((h = DmGetResourceIndex(dbRef, i)) == NULL) continue;
      size = MemHandleSize(h);
      p = MemHandleLock(h);
      data->item[index].type = type;
      data->item[index].id = id;
      data->item[index].index = i;
      data->item[index].size = size;
      pumpkin_id2s(type, buf);

      switch (type) {
        case formRscType:
          localId = MemHandleToLocalID(h);
          b = MemLocalIDToPtr(localId, 0);
          if (b) {
            union {
              uint16_t u16flags;
              WindowFlagsType wflags;
            } flags;
            get2b(&flags.u16flags, b, 8);
            get2b(&x, b, 10);
            get2b(&y, b, 12);
            get2b(&width, b, 14);
            get2b(&height, b, 16);
            sys_snprintf(buf, sizeof(buf)-1, "Form %dx%d at %d,%d%s", width, height, x, y, flags.wflags.modal ? " modal" : "");
          } else {
            sys_snprintf(buf, sizeof(buf)-1, "Form");
          }
          data->item[index].info = xstrdup(buf);
          break;
        case alertRscType:
          alert = (AlertTemplateType *)p;
          switch (alert->alertType) {
            case informationAlert:  s = "Information";  break;
            case confirmationAlert: s = "Confirmation"; break;
            case warningAlert:      s = "Warning";      break;
            case errorAlert:        s = "Error";        break;
            default:                s = "Unknown";      break;
          }
          sys_snprintf(buf, sizeof(buf)-1, "%s alert", s);
          data->item[index].info = xstrdup(buf);
          break;
        case MenuRscType:
          menu = (MenuBarType *)p;
          sys_snprintf(buf, sizeof(buf)-1, "Menu with %d pulldown(s)", menu->numMenus);
          data->item[index].info = xstrdup(buf);
          break;
        case fontRscType:
          font = (FontType *)p;
          sys_snprintf(buf, sizeof(buf)-1, "Font V%d", font->v);
          data->item[index].info = xstrdup(buf);
          break;
        case fontExtRscType:
          font = (FontType *)p;
          sys_snprintf(buf, sizeof(buf)-1, "Font V%d", font->v);
          data->item[index].info = xstrdup(buf);
          break;
        case iconType:
        case bitmapRsc:
        case 'abmp':
          bmp = (BitmapType *)p;
          version = BmpGetVersion(bmp);
          depth = BmpGetBitDepth(bmp);
          BmpGetDimensions(bmp, (Coord *)&width, (Coord *)&height, NULL);
          s = (type == iconType) ? "Icon" : "Bitmap";
          chain = BmpGetNextBitmapAnyDensity(bmp) != NULL;
          sys_snprintf(buf, sizeof(buf)-1, "%s V%d, %dx%d, %d bpp%s", s, version, width, height, depth, chain ? " (chain)" : "");
          data->item[index].info = xstrdup(buf);
          break;
        case scriptEngineLua:
          data->item[index].info = xstrdup("Lua script");
          break;
        case scriptEngineJS:
          data->item[index].info = xstrdup("JS script");
          break;
        case strRsc:
          s = (char *)p;
          sys_snprintf(buf, sizeof(buf)-1, "\"%s\"", s);
          data->item[index].info = xstrdup(buf);
          break;
        case constantRscType:
          value = (UInt32 *)p;
          sys_snprintf(buf, sizeof(buf)-1, "Constant %d (0x%08X)", *value, *value);
          data->item[index].info = xstrdup(buf);
          break;
        case wrdListRscType:
          sys_snprintf(buf, sizeof(buf)-1, "Word list");
          data->item[index].info = xstrdup(buf);
          break;
        case strListRscType:
          sys_snprintf(buf, sizeof(buf)-1, "String list");
          data->item[index].info = xstrdup(buf);
          break;
        case ainRsc:
          s = (char *)p;
          sys_snprintf(buf, sizeof(buf)-1, "App name \"%s\"", s);
          data->item[index].info = xstrdup(buf);
          break;
        case verRsc:
          s = (char *)p;
          sys_snprintf(buf, sizeof(buf)-1, "Version \"%s\"", s);
          data->item[index].info = xstrdup(buf);
          break;
        case defaultCategoryRscType:
          sys_snprintf(buf, sizeof(buf)-1, "Default category");
          data->item[index].info = xstrdup(buf);
          break;
        case sysRsrcTypeDlib:
          // id = SYS_OS * 64 + SYS_CPU * 8 + SYS_SIZE;
          sys_size = id & 0x07;
          sys_cpu = (id & 0x38) >> 3;
          sys_os  = (id & 0x1C0) >> 6;
          sys_snprintf(buf, sizeof(buf)-1, "Code (%d bits %s CPU for %s)", (sys_size == 1) ? 32 : 64, (sys_cpu == 1) ? "ARM" : "x86", (sys_os == 1) ? "Linux" : "Windows");
          data->item[index].info = xstrdup(buf);
          break;
        case sysFileTLibrary:
          sys_snprintf(buf, sizeof(buf)-1, "Library (68K)");
          data->item[index].info = xstrdup(buf);
          break;
        case sysRsrcTypeWinD:
          get2b(&width, p, 0);
          get2b(&height, p, 2);
          sys_snprintf(buf, sizeof(buf)-1, "Absolute window dimensions %dx%d", width, height);
          data->item[index].info = xstrdup(buf);
          break;
        case sysResTAppCode:
          if (id == 0) {
            sys_snprintf(buf, sizeof(buf)-1, "App information (68K)");
          } else {
            sys_snprintf(buf, sizeof(buf)-1, "Code (68K)");
          }
          data->item[index].info = xstrdup(buf);
          break;
        case sysResTAppGData:
          sys_snprintf(buf, sizeof(buf)-1, "Global data (68K)");
          data->item[index].info = xstrdup(buf);
          break;
        case sysResTAppPrefs:
          sys_snprintf(buf, sizeof(buf)-1, "App prefs (68K)");
          data->item[index].info = xstrdup(buf);
          break;
        case 'rloc':
          sys_snprintf(buf, sizeof(buf)-1, "Relocation (68K)");
          data->item[index].info = xstrdup(buf);
          break;
        case 'locs':
          sys_snprintf(buf, sizeof(buf)-1, "Localization");
          data->item[index].info = xstrdup(buf);
          break;
      }

      MemHandleUnlock(h);
      DmReleaseResource(h);
      index++;
    }

    DmCloseDatabase(dbRef);
  }

  data->numItems = index;
  sortItems(data);
  debug(DEBUG_INFO, "Launcher", "found %d resources", data->numItems);
}

static void launcherScanFiles(launcher_data_t *data) {
  UInt32 dirEntryIterator;
  FileInfoType info;
  FileRef dirRef;
  FileRef fileRef;
  FontID old;
  UInt16 index;
  UInt32 size;
  char name[MAX_NAME];
  char aux[MAX_NAME];

  launcherResetItems(data);

  old = FntSetFont(stdFont);
  data->cellHeight = FntCharHeight() + 2;
  data->cellWidth = data->gadRect.extent.x;
  FntSetFont(old);

  if (VFSFileOpen(1, data->path, vfsModeRead, &dirRef) == errNone) {
    dirEntryIterator = vfsIteratorStart;
    for (index = 0; index < MAX_ITEMS; index++) {
      MemSet(&info, sizeof(info), 0);
      MemSet(name, MAX_NAME, 0);
      info.nameP = name;
      info.nameBufLen = MAX_NAME - 1;
      if (VFSDirEntryEnumerate(dirRef, &dirEntryIterator, &info) != errNone) break;
      data->item[index].size = 0;
      if (info.attributes & vfsFileAttrDirectory) {
        data->item[index].dir = true;
      } else {
        data->item[index].dir = false;
        if (StrLen(data->path) + StrLen(name) < MAX_NAME - 1) {
          sys_strncpy(aux, data->path, sizeof(aux)-1);
          sys_strncat(aux, name, sizeof(aux)-sys_strlen(aux)-1);
          if (VFSFileOpen(1, aux, vfsModeRead, &fileRef) == errNone) {
            VFSFileSize(fileRef, &size);
            data->item[index].size = size;
            VFSFileClose(fileRef);
          }
        }
      }
      StrNCopy(data->item[index].name, name, MAX_NAME-1);
    }
    VFSFileClose(dirRef);
  }

  if (StrCompare(data->path, "/") != 0 && index < MAX_ITEMS) {
    data->item[index].size = 0;
    data->item[index].dir = true;
    StrCopy(data->item[index].name, PARENT_DIR);
    index++;
  }

  data->numItems = index;
  sortItems(data);
  debug(DEBUG_INFO, "Launcher", "found %d files", data->numItems);
}

static int ps_callback(int i, char *name, int m68k, void *_data) {
  launcher_data_t *data = (launcher_data_t *)_data;

  if (data->numItems < MAX_ITEMS) {
    StrNCopy(data->item[data->numItems].name, name, dmDBNameLength-1);
    data->item[data->numItems].id = i;
    data->item[data->numItems].m68k = m68k;
    data->numItems++;
  }

  return 0;
}

static void launcherScanTasks(launcher_data_t *data) {
  FontID old;

  launcherResetItems(data);

  old = FntSetFont(stdFont);
  data->cellHeight = FntCharHeight() + 2;
  data->cellWidth = data->gadRect.extent.x;
  FntSetFont(old);

  data->numItems = 0;
  pumpkin_ps(ps_callback, data);

  sortItems(data);
  debug(DEBUG_INFO, "Launcher", "found %d tasks", data->numItems);
}

static void compat_callback(UInt32 creator, UInt16 seq, UInt16 index, UInt16 id, void *p, UInt16 size, void *_data) {
  launcher_data_t *data = (launcher_data_t *)_data;
  AppRegistryCompat *c;
  AppRegistrySize *s;

  if (data->numItems < MAX_ITEMS) {
    switch (id) {
      case appRegistryCompat:
        data->numItems++;
        c = (AppRegistryCompat *)p;
        data->item[data->numItems-1].type = sysFileTApplication;
        data->item[data->numItems-1].creator = creator;
        data->item[data->numItems-1].compat = c->compat;
        data->item[data->numItems-1].code = c->code;
        break;
      case appRegistrySize:
        if (data->numItems > 0) {
          s = (AppRegistrySize *)p;
          data->item[data->numItems-1].type = sysFileTApplication;
          data->item[data->numItems-1].creator = creator;
          data->item[data->numItems-1].width = s->width;
          data->item[data->numItems-1].height = s->height;
        }
        break;
    }
  }
}

static void launcherScanRegistry(launcher_data_t *data) {
  FontID old;

  launcherResetItems(data);

  old = FntSetFont(stdFont);
  data->cellHeight = FntCharHeight() + 2;
  data->cellWidth = data->gadRect.extent.x;
  FntSetFont(old);

  data->numItems = 0;
  pumpkin_enum_compat(compat_callback, data);

  sortItems(data);
  debug(DEBUG_INFO, "Launcher", "found %d registries", data->numItems);
}

static void launcherScanRecords(launcher_data_t *data) {
  MemHandle h;
  UInt16 num, index;
  FontID old;
  DmOpenRef dbRef;

  if ((dbRef = DmOpenDatabase(0, data->dbID, dmModeReadOnly)) != NULL) {
    launcherResetItems(data);

    old = FntSetFont(stdFont);
    data->cellHeight = FntCharHeight() + 2;
    data->cellWidth = data->gadRect.extent.x;
    FntSetFont(old);

    num = DmNumRecords(dbRef);
    for (index = 0; index < num && index < MAX_ITEMS; index++) {
      if ((h = DmGetRecord(dbRef, index)) == NULL) break;
      data->item[index].index = index;
      data->item[index].size = MemHandleSize(h);
      DmReleaseRecord(dbRef, index, false);
    }

    DmCloseDatabase(dbRef);
  }

  data->numItems = index;
  sortItems(data);
  debug(DEBUG_INFO, "Launcher", "found %d records", data->numItems);
}

static void launcherScan(launcher_data_t *data) {
  switch (data->mode) {
    case launcher_app:
      launcherScanApps(data);
      break;
    case launcher_app_small:
      launcherScanAppsSmall(data);
      break;
    case launcher_db:
      launcherScanDBs(data);
      break;
    case launcher_rsrc:
      launcherScanResources(data);
      break;
    case launcher_rec:
      launcherScanRecords(data);
      break;
    case launcher_file:
      launcherScanFiles(data);
      break;
    case launcher_task:
      launcherScanTasks(data);
      break;
    case launcher_registry:
      launcherScanRegistry(data);
      break;
  }
}

static void refresh(FormPtr frm, launcher_data_t *data) {
  FormGadgetTypeInCallback *gad;
  ScrollBarType *scl;
  UInt16 objIndex;
  Int16 cols, rows, totalRows;

  data->topItem = 0;
  objIndex = FrmGetObjectIndex(frm, iconsGad);
  gad = (FormGadgetTypeInCallback *)FrmGetObjectPtr(frm, objIndex);
  ItemsGadgetCallback(gad, formGadgetDrawCmd, NULL);

  rows = data->gadRect.extent.y / data->cellHeight;
  if (data->mode != launcher_app) rows--;
  cols = data->gadRect.extent.x / data->cellWidth;
  totalRows = (data->numItems + cols - 1) / cols;

  objIndex = FrmGetObjectIndex(frm, iconsScl);
  if (totalRows > rows) {
    scl = (ScrollBarType *)FrmGetObjectPtr(frm, objIndex);
    SclSetScrollBar(scl, 0, 0, totalRows - rows, (totalRows - rows) >= rows ? rows - 1 : totalRows - rows);
    FrmShowObject(frm, objIndex);
  } else {
    FrmHideObject(frm, objIndex);
  }
}

static int adjustName(char *buf, int size, char *name, int *width, int max) {
  int len;

  StrNCopy(buf, name, size-1);
  len = StrLen(buf);
  *width = FntCharsWidth(buf, len);
  if (*width >= max) {
    len = FntWidthToOffset(buf, len, max - FntCharWidth(chrEllipsis), NULL, NULL);
    buf[len++] = chrEllipsis;
    buf[len] = 0;
    *width = FntCharsWidth(buf, len);
  }

  return len;
}

static void printApp(launcher_data_t *data, launcher_item_t *item, int x, int y, Boolean inverted) {
  RectangleType rect;
  int len, nameWidth;
  char buf[dmDBNameLength];

  RctSetRectangle(&rect, x, y, data->cellWidth, data->cellHeight);
  setBackColor(inverted ? BLACK : WHITE);
  WinEraseRectangle(&rect, 0);
  setBackColor(WHITE);

  if (item->iconWh) {
    RctSetRectangle(&rect, 0, 0, data->cellWidth, item->bmpHeight);
    WinCopyRectangle(inverted && item->invIconWh ? item->invIconWh : item->iconWh, WinGetDrawWindow(), &rect, x, y, winPaint);
    WinCopyRectangle(inverted && item->invIconWh ? item->invIconWh : item->iconWh, WinGetDisplayWindow(), &rect, x, y, winPaint);
  }

  len = adjustName(buf, dmDBNameLength, item->name, &nameWidth, data->cellWidth-2);
  setTextColor(inverted ? WHITE : BLACK);
  WinPaintChars(buf, len, x + (data->cellWidth - nameWidth) / 2, y + data->cellHeight - FntCharHeight() - 4);
  setTextColor(BLACK);
}

static void firstColumn(launcher_data_t *data, int x, int y, Boolean inverted) {
  RectangleType rect;

  if (inverted) {
    setTextColor(WHITE);
    setBackColor(BLACK);
  } else {
    setTextColor(BLACK);
    setBackColor(WHITE);
  }

  RctSetRectangle(&rect, x, y+data->cellHeight-2, data->cellWidth, 2);
  WinEraseRectangle(&rect, 0);
  data->num = 0;
}

static int printColumn(launcher_data_t *data, launcher_item_t *item, char *label, char *value, int x, int y, int max, Boolean rightAlign, Boolean inverted) {
  RectangleType rect;
  char buf[MAX_NAME];
  int width, len;

  data->x[data->num++] = x;
  FntSetFont(item ? stdFont : boldFont);
  len = adjustName(buf, MAX_NAME, item ? value : label, &width, max);
  if (rightAlign) {
    RctSetRectangle(&rect, x, y, max-width, data->cellHeight);
    WinEraseRectangle(&rect, 0);
    WinPaintChars(buf, len, x+max-width, y);
  } else {
    WinPaintChars(buf, len, x, y);
    RctSetRectangle(&rect, x+width, y, max-width, data->cellHeight);
    WinEraseRectangle(&rect, 0);
  }
  FntSetFont(stdFont);
  x += max;

  return x;
}

static void paintBitmap(UInt16 id, Coord x, Coord y) {
  BitmapType *bmp;
  MemHandle h;

  if ((h = DmGet1Resource(bitmapRsc, id)) != NULL) {
    if ((bmp = MemHandleLock(h)) != NULL) {
      WinPaintBitmap(bmp, x, y);
      MemHandleUnlock(h);
    }
    DmReleaseResource(h);
  }
}

static int printBmpColumn(launcher_data_t *data, launcher_item_t *item, char *label, char *value, UInt16 bmpId, int x, int y, int max, Boolean inverted) {
  RectangleType rect;
  char buf[MAX_NAME];
  int width, len;

  data->x[data->num++] = x;

  if (item) {
    FntSetFont(stdFont);
    RctSetRectangle(&rect, x, y, 11, data->cellHeight);
    WinEraseRectangle(&rect, 0);
    setBackColor(WHITE);
    RctSetRectangle(&rect, x, y, data->cellHeight, data->cellHeight);
    WinEraseRectangle(&rect, 0);
    if (bmpId == 0xffff) {
      if (item->iconWh) {
        RctSetRectangle(&rect, 0, 0, data->cellHeight, data->cellHeight);
        WinCopyRectangle(item->iconWh, WinGetDrawWindow(), &rect, x, y+1, winPaint);
        WinCopyRectangle(item->iconWh, WinGetDisplayWindow(), &rect, x, y+1, winPaint);
      }
    } else if (bmpId > 0) {
      paintBitmap(bmpId, x, y);
    }
    if (inverted) {
      setBackColor(BLACK);
    } else {
      setBackColor(WHITE);
    }
    RctSetRectangle(&rect, x+11, y, 4, data->cellHeight);
    WinEraseRectangle(&rect, 0);
    len = adjustName(buf, MAX_NAME, value, &width, max);
    width += 15;
    WinPaintChars(buf, len, x+15, y);
  } else {
    FntSetFont(boldFont);
    width = FntCharsWidth(label, StrLen(label));
    WinPaintChars(label, StrLen(label), x, y);
    FntSetFont(stdFont);
  }

  RctSetRectangle(&rect, x+width, y, max-width, data->cellHeight);
  WinEraseRectangle(&rect, 0);
  x += max;

  return x;
}

static int spaceColumn(launcher_data_t *data, int x, int y, int dx) {
  RectangleType rect;

  RctSetRectangle(&rect, x, y, dx, data->cellHeight);
  WinEraseRectangle(&rect, 0);

  return x+dx;
}

static void lastColumn(launcher_data_t *data, int x, int y) {
  RectangleType rect;

  data->x[data->num] = x;
  if (x < data->cellWidth) {
    RctSetRectangle(&rect, x, y, data->cellWidth - x, data->cellHeight);
    WinEraseRectangle(&rect, 0);
  }

  setTextColor(BLACK);
  setBackColor(WHITE);
}

static void printAppSmall(launcher_data_t *data, launcher_item_t *item, int x, int y, Boolean inverted) {
  char buf[32];

  updateCounts(item);

  firstColumn(data, x, y, inverted);
  x = printBmpColumn(data, item, "Name", item ? item->name : NULL, 0xffff, x, y, 28 * FntCharWidth('a'), inverted);
  if (!pumpkin_dia_enabled() && !pumpkin_is_single()) {
    x = printColumn(data, item, "Creat.", item ? pumpkin_id2s(item->creator, buf) : NULL, x, y, 7 * FntCharWidth('w'), false, inverted);
    if (item) sys_snprintf(buf, sizeof(buf)-1, "%u", item->size);
    x = printColumn(data, item, "Size", item ? buf : NULL, x, y, 10 * FntCharWidth('0'), true, inverted);
  }
  lastColumn(data, x, y);
}

static void printDatabase(launcher_data_t *data, launcher_item_t *item, int x, int y, Boolean inverted) {
  char buf[32];

  updateCounts(item);

  firstColumn(data, x, y, inverted);
  x = printColumn(data, item, "Name", item ? item->name : NULL, x, y, 28 * FntCharWidth('a'), false, inverted);
  if (!pumpkin_dia_enabled() && !pumpkin_is_single()) {
    x = printColumn(data, item, "Type", item ? pumpkin_id2s(item->type, buf) : NULL, x, y, 6 * FntCharWidth('w'), false, inverted);
    x = printColumn(data, item, "Creat.", item ? pumpkin_id2s(item->creator, buf) : NULL, x, y, 7 * FntCharWidth('w'), false, inverted);
    if (item) sys_snprintf(buf, sizeof(buf)-1, "%u", item->numRecs);
    x = printColumn(data, item, "Recs", item ? buf : NULL, x, y, 5 * FntCharWidth('0'), true, inverted);
    if (item) sys_snprintf(buf, sizeof(buf)-1, "%u", item->size);
    x = printColumn(data, item, "Size", item ? buf : NULL, x, y, 10 * FntCharWidth('0'), true, inverted);
  }
  lastColumn(data, x, y);
}

static void printFile(launcher_data_t *data, launcher_item_t *item, int x, int y, Boolean inverted) {
  char buf[MAX_NAME];

  firstColumn(data, x, y, inverted);
  x = printBmpColumn(data, item, "Name", item ? item->name : NULL, item && item->dir ? folderBmp : 0, x, y, 40 * FntCharWidth('a'), inverted);
  if (item) {
    if (item->dir) {
      StrCopy(buf, " ");
    } else {
      sys_snprintf(buf, sizeof(buf)-1, "%u", item->size);
    }
  }
  x = printColumn(data, item, "Size", item ? buf : NULL, x, y, 10 * FntCharWidth('0'), true, inverted);
  lastColumn(data, x, y);
}

static void printTask(launcher_data_t *data, launcher_item_t *item, int x, int y, Boolean inverted) {
  char buf[32];

  firstColumn(data, x, y, inverted);
  if (item) sys_snprintf(buf, sizeof(buf)-1, "%u", item->id);
  x = printColumn(data, item, "ID", item ? buf : NULL, x, y, 5 * FntCharWidth('0'), true, inverted);
  x = spaceColumn(data, x, y, 10);
  x = printBmpColumn(data, item, "Name", item ? item->name : NULL, item ? (item->m68k ? m68kBmp : pumpkinBmp) : 0, x, y, 28 * FntCharWidth('a'), inverted);
  lastColumn(data, x, y);
}

static void printRegistry(launcher_data_t *data, launcher_item_t *item, int x, int y, Boolean inverted) {
  char buf[128], *s;
  UInt16 bmpId;
  int dia;

  dia = pumpkin_dia_enabled() || pumpkin_is_single();
  firstColumn(data, x, y, inverted);
  x = printColumn(data, item, "Creator", item ? pumpkin_id2s(item->creator, buf) : NULL, x, y, 8 * FntCharWidth('w'), false, inverted);
  if (item) sys_snprintf(buf, sizeof(buf)-1, "%ux%u", item->width, item->height);
  x = printColumn(data, item, "Size", item ? buf : NULL, x, y, 10 * FntCharWidth('0'), false, inverted);

  if (item) {
    switch (item->compat) {
      case appCompatUnknown:
        StrCopy(buf, "Unknown");
        bmpId = questionBmp;
        break;
      case appCompatOk:
        StrCopy(buf, "Good");
        bmpId = okBmp;
        break;
      case appCompatMinor:
        StrCopy(buf, "Minor problem");
        bmpId = warningBmp;
        break;
      case appCompatMajor:
        StrCopy(buf, "Major problem");
        bmpId = warningBmp;
        break;
      case appCompatCrash:
        switch (item->code) {
          case EMUPALMOS_INVALID_ADDRESS:
            s = "invalid address";
            break;
          case EMUPALMOS_INVALID_INSTRUCTION:
            s = "invalid instruction";
            break;
          case EMUPALMOS_INVALID_TRAP:
            s = "invalid trap";
            break;
          case EMUPALMOS_INVALID_XREF:
            s = "invalid xref";
            break;
          case EMUPALMOS_GENERIC_ERROR:
            s = "other error";
            break;
        }
        sys_snprintf(buf, sizeof(buf)-1, "Crash: %s", s);
        bmpId = errorBmp;
        break;
    }
    x = printBmpColumn(data, item, NULL, buf, bmpId, x, y, 32 * FntCharWidth('w'), inverted);

  } else {
    x = printColumn(data, item, dia ? "Compat." : "Compatibility", NULL, x, y, 32 * FntCharWidth('w'), false, inverted);
  }

  lastColumn(data, x, y);
}

static void printResource(launcher_data_t *data, launcher_item_t *item, int x, int y, Boolean inverted) {
  char buf[32];

  firstColumn(data, x, y, inverted);
  x = printColumn(data, item, "Type", item ? pumpkin_id2s(item->type, buf) : NULL, x, y, 6 * FntCharWidth('w'), false, inverted);
  if (item) sys_snprintf(buf, sizeof(buf)-1, "%u", item->id);
  x = printColumn(data, item, "ID", item ?  buf : NULL, x, y, 6 * FntCharWidth('0'), true, inverted);
  if (item) sys_snprintf(buf, sizeof(buf)-1, "%u", item->size);
  x = printColumn(data, item, "Size", item ? buf : NULL, x, y, 10 * FntCharWidth('0'), true, inverted);
  if (!pumpkin_dia_enabled() && !pumpkin_is_single()) {
    x = spaceColumn(data, x, y, 10);
    x = printColumn(data, item, "Contents", item ? (item->info ? item->info : "Unknown") : NULL, x, y, data->cellWidth - (x - data->x[0]), false, inverted);
  }
  lastColumn(data, x, y);
}

static void editResource(launcher_data_t *data, launcher_item_t *item) {
  LocalID dbID;
  DmOpenRef myDbRef;
  DmOpenRef dbRef;
  UInt32 swidth, sheight;
  FormType *frm;
  MemHandle h;
  Boolean (*editor)(FormType *frm, char *title, MemHandle h) = NULL;
  char st[8], title[32];
  UInt16 formId = 0;

  if ((dbRef = DmOpenDatabase(0, data->dbID, dmModeReadWrite)) != NULL) {
    if ((h = DmGetResourceIndex(dbRef, item->index)) != NULL) {
      pumpkin_id2s(item->type, st);
      sys_snprintf(title, sizeof(title)-1, "%s %d", st, item->id);
      formId = EditBinForm;
      editor = editBinary;

      switch (item->type) {
        case formRscType:
          formId = EditFormForm;
          editor = editForm;
          break;
        case alertRscType:
          break;
        case MenuRscType:
          break;
        case fontRscType:
          break;
        case fontExtRscType:
          break;
        case iconType:
        case bitmapRsc:
        case 'abmp':
          formId = EditBmpForm;
          editor = editBitmap;
          break;
        case strRsc:
          formId = EditStrForm;
          editor = editString;
          break;
        case constantRscType:
          break;
        case wrdListRscType:
          break;
        case strListRscType:
          break;
        case ainRsc:
          formId = EditStrForm;
          editor = editString;
          break;
        case verRsc:
          formId = EditStrForm;
          editor = editString;
          break;
        case defaultCategoryRscType:
          formId = EditStrForm;
          editor = editString;
          break;
        case sysRsrcTypeWinD:
          break;
      }

      dbID = pumpkin_get_app_localid();
      if ((myDbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
        if ((frm = FrmInitForm(formId)) != NULL) {
          if (item->type == formRscType) {
            DmCloseDatabase(myDbRef);
          }
          WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
          frm->window.windowBounds.topLeft.x = (swidth - frm->window.windowBounds.extent.x) / 2;
          frm->window.windowBounds.topLeft.y = (sheight - frm->window.windowBounds.extent.y) / 2;
          editor(frm, title, h);
          FrmDeleteForm(frm);
        }
        if (frm == NULL || item->type != formRscType) {
          DmCloseDatabase(myDbRef);
        }
      }

      DmReleaseResource(h);
    }
    DmCloseDatabase(dbRef);
  } else {
    OpenErrorDialog(data->name, DmGetLastErr());
  }
}

static void printRecord(launcher_data_t *data, launcher_item_t *item, int x, int y, Boolean inverted) {
  char buf[32];

  firstColumn(data, x, y, inverted);
  if (item) sys_snprintf(buf, sizeof(buf)-1, "%u", item->index);
  x = printColumn(data, item, "Index", item ?  buf : NULL, x, y, 6 * FntCharWidth('0'), true, inverted);
  if (item) sys_snprintf(buf, sizeof(buf)-1, "%u", item->size);
  x = printColumn(data, item, "Size", item ? buf : NULL, x, y, 10 * FntCharWidth('0'), true, inverted);
  lastColumn(data, x, y);
}

static void editRecord(launcher_data_t *data, launcher_item_t *item) {
  LocalID dbID;
  DmOpenRef myDbRef;
  DmOpenRef dbRef;
  UInt32 swidth, sheight;
  FormType *frm;
  MemHandle h;
  Boolean changed;
  char title[32];

  if ((dbRef = DmOpenDatabase(0, data->dbID, dmModeReadWrite)) != NULL) {
    if ((h = DmGetRecord(dbRef, item->index)) != NULL) {
      dbID = pumpkin_get_app_localid();
      changed = false;

      if ((myDbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
        if ((frm = FrmInitForm(EditBinForm)) != NULL) {
          WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
          frm->window.windowBounds.topLeft.x = (swidth - frm->window.windowBounds.extent.x) / 2;
          frm->window.windowBounds.topLeft.y = (sheight - frm->window.windowBounds.extent.y) / 2;
          sys_snprintf(title, sizeof(title)-1, "Record %d", item->index);
          changed = editBinary(frm, title, h);
          FrmDeleteForm(frm);
        }
        DmCloseDatabase(myDbRef);
      }

      DmReleaseRecord(h, item->index, changed);
    }
    DmCloseDatabase(dbRef);
  } else {
    OpenErrorDialog(data->name, DmGetLastErr());
  }
}

static void columnClicked(launcher_data_t *data, int x, int y, Boolean enter) {
  RectangleType rect;
  FormType *frm;
  int i;

  for (i = 0; i < data->num; i++) {
    if (x >= data->x[i] && x < data->x[i+1]) {
      if (enter) {
        if (data->sort == i) {
          data->dir = 1-data->dir;
        } else {
          data->sort = i;
          data->dir = 0;
        }
      }
      break;
    }
  }

  RctSetRectangle(&rect, data->x[i], y, data->x[i+1] - data->x[i], data->cellHeight);

  if (enter) {
    sortItems(data);
    frm = FrmGetActiveForm();
    refresh(frm, data);
    WinInvertRectangle(&rect, 0);
  } else {
    WinInvertRectangle(&rect, 0);
  }
}

static Boolean ItemsGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  launcher_data_t *data;
  EventType *event;
  RectangleType rect;
  WinDrawOperation mode;
  FormType *frm;
  FontID old;
  UInt16 fc, tc, bc;
  UInt16 flags;
  UInt32 result;
  LocalID dbID;
  DmOpenRef myDbRef;
  int i, x, y, iw, ih, num, ncols, nrows, col, row, wgad, hgad, plen, len;

  debug(DEBUG_TRACE, "Launcher", "ItemsGadgetCallback %d", cmd);

  if (cmd == formGadgetDeleteCmd) {
    return true;
  }

  data = pumpkin_get_data();
  bc = WinSetBackColor(UIColorGetTableEntryIndex(UIFormFill));
  tc = WinSetTextColor(UIColorGetTableEntryIndex(UIFieldText));
  fc = WinSetForeColor(UIColorGetTableEntryIndex(UIFieldText));
  mode = WinSetDrawMode(winPaint);
  old = FntSetFont(stdFont);
  iw = data->cellWidth;
  ih = data->cellHeight;
  wgad = gad->rect.extent.x;
  hgad = gad->rect.extent.y;
  ncols = wgad / iw;
  wgad = ncols * iw;
  nrows = hgad / ih;
  num = ncols * nrows;

  if (data->mode == launcher_file) {
    dbID = pumpkin_get_app_localid();
    myDbRef = DmOpenDatabase(0, dbID, dmModeReadOnly);
  } else {
    myDbRef = NULL;
  }

  switch (cmd) {
    case formGadgetDrawCmd:
      debug(DEBUG_TRACE, "Launcher", "draw items cols=%d rows=%d numItems=%d", ncols, nrows, data->numItems);
      x = gad->rect.topLeft.x;
      y = gad->rect.topLeft.y + 2;

      for (i = 0; i < num; i++) {
        if (x >= gad->rect.topLeft.x + wgad) {
          x = gad->rect.topLeft.x;
          y += ih;
        } else if (x + iw > gad->rect.topLeft.x + wgad) {
          RctSetRectangle(&rect, x, y, gad->rect.topLeft.x + wgad - x, ih);
          WinEraseRectangle(&rect, 0);
          x = gad->rect.topLeft.x;
          y += ih;
        }

        switch (data->mode) {
          case launcher_app:
            if (i + data->topItem < data->numItems) {
              WinSetDrawMode(winOverlay);
              printApp(data, &data->item[i + data->topItem], x, y, false);
            } else {
              RctSetRectangle(&rect, x, y, iw, ih);
              WinEraseRectangle(&rect, 0);
            }
            break;
          case launcher_app_small:
            if (i == 0) {
              printAppSmall(data, NULL, x, y, false);
            } else if (i-1 + data->topItem < data->numItems) {
              printAppSmall(data, &data->item[i-1 + data->topItem], x, y, false);
            } else {
              RctSetRectangle(&rect, x, y, iw, ih);
              WinEraseRectangle(&rect, 0);
            }
            break;
          case launcher_db:
            if (i == 0) {
              printDatabase(data, NULL, x, y, false);
            } else if (i-1 + data->topItem < data->numItems) {
              printDatabase(data, &data->item[i-1 + data->topItem], x, y, false);
            } else {
              RctSetRectangle(&rect, x, y, iw, ih);
              WinEraseRectangle(&rect, 0);
            }
            break;
          case launcher_rsrc:
            if (i == 0) {
              printResource(data, NULL, x, y, false);
            } else if (i-1 + data->topItem < data->numItems) {
              printResource(data, &data->item[i-1 + data->topItem], x, y, false);
            } else {
              RctSetRectangle(&rect, x, y, iw, ih);
              WinEraseRectangle(&rect, 0);
            }
            break;
          case launcher_rec:
            if (i == 0) {
              printRecord(data, NULL, x, y, false);
            } else if (i-1 + data->topItem < data->numItems) {
              printRecord(data, &data->item[i-1 + data->topItem], x, y, false);
            } else {
              RctSetRectangle(&rect, x, y, iw, ih);
              WinEraseRectangle(&rect, 0);
            }
            break;
          case launcher_file:
            if (i == 0) {
              printFile(data, NULL, x, y, false);
            } else if (i-1 + data->topItem < data->numItems) {
              printFile(data, &data->item[i-1 + data->topItem], x, y, false);
            } else {
              RctSetRectangle(&rect, x, y, iw, ih);
              WinEraseRectangle(&rect, 0);
            }
            break;
          case launcher_task:
            if (i == 0) {
              printTask(data, NULL, x, y, false);
            } else if (i-1 + data->topItem < data->numItems) {
              printTask(data, &data->item[i-1 + data->topItem], x, y, false);
            } else {
              RctSetRectangle(&rect, x, y, iw, ih);
              WinEraseRectangle(&rect, 0);
            }
            break;
          case launcher_registry:
            if (i == 0) {
              printRegistry(data, NULL, x, y, false);
            } else if (i-1 + data->topItem < data->numItems) {
              printRegistry(data, &data->item[i-1 + data->topItem], x, y, false);
            } else {
              RctSetRectangle(&rect, x, y, iw, ih);
              WinEraseRectangle(&rect, 0);
            }
            break;
        }

        x += iw;
      }

      y = nrows * ih;
      RctSetRectangle(&rect, gad->rect.topLeft.x, gad->rect.topLeft.y + y, gad->rect.extent.x, gad->rect.extent.y - y);
      WinEraseRectangle(&rect, 0);
      x = ncols * iw;
      RctSetRectangle(&rect, gad->rect.topLeft.x + x, gad->rect.topLeft.y, gad->rect.extent.x - x, gad->rect.extent.y);
      WinEraseRectangle(&rect, 0);

      debug(DEBUG_TRACE, "Launcher", "draw items end");
      break;

    case formGadgetHandleEventCmd:
      event = (EventType *)param;
      col = (event->screenX - gad->rect.topLeft.x) / iw;
      row = (event->screenY - gad->rect.topLeft.y) / ih;

      switch (data->mode) {
        case launcher_app:
          i = row * ncols + col + data->topItem;
          if (i >= 0 && i < data->numItems) {
            WinSetDrawMode(winOverlay);
            x = gad->rect.topLeft.x + col * iw;
            y = gad->rect.topLeft.y + 2 + row * ih;
            if (event->eType == frmGadgetEnterEvent) {
              printApp(data, &data->item[i], x, y, true);
            } else {
              data->top = false;
              flags = sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp;
              SysAppLaunchEx(0, data->item[i].dbID, flags, sysAppLaunchCmdNormalLaunch, NULL, &result, data->item[i].pilot_main);
              printApp(data, &data->item[i], x, y, false);
            }
          }
          break;
        case launcher_app_small:
          if (row == 0) {
            x = event->screenX - gad->rect.topLeft.x;
            y = gad->rect.topLeft.y + 2;
            columnClicked(data, x, y, event->eType == frmGadgetEnterEvent);
          } else {
            i = (row-1) * ncols + col + data->topItem;
            if (i < data->numItems) {
              x = gad->rect.topLeft.x + col * iw;
              y = gad->rect.topLeft.y + 2 + row * ih;
              if (event->eType == frmGadgetEnterEvent) {
                if (i != data->prev) {
                  if (data->prev >= 0) {
                    printAppSmall(data, &data->item[data->prev], data->prevX, data->prevY, false);
                  }
                  printAppSmall(data, &data->item[i], x, y, true);
                  data->prev = i;
                  data->prevX = x;
                  data->prevY = y;
                }
              }
            }
          }
          break;
        case launcher_db:
          if (row == 0) {
            x = event->screenX - gad->rect.topLeft.x;
            y = gad->rect.topLeft.y + 2;
            columnClicked(data, x, y, event->eType == frmGadgetEnterEvent);
          } else {
            i = (row-1) * ncols + col + data->topItem;
            if (i < data->numItems) {
              x = gad->rect.topLeft.x + col * iw;
              y = gad->rect.topLeft.y + 2 + row * ih;
              if (event->eType == frmGadgetEnterEvent) {
                printDatabase(data, &data->item[i], x, y, true);
              } else {
                printDatabase(data, &data->item[i], x, y, false);
                StrNCopy(data->name, data->item[i].name, dmDBNameLength-1);
                data->dbID = data->item[i].dbID;
                data->sort = 0;
                data->dir = 0;
                frm = FrmGetActiveForm();
                data->mode = data->item[i].rsrc ? launcher_rsrc : launcher_rec;
                launcherScan(data);
                refresh(frm, data);
                UpdateStatus(frm, data, true);
              }
            }
          }
          break;
        case launcher_rsrc:
          if (row == 0) {
            x = event->screenX - gad->rect.topLeft.x;
            y = gad->rect.topLeft.y + 2;
            columnClicked(data, x, y, event->eType == frmGadgetEnterEvent);
          } else {
            i = (row-1) * ncols + col + data->topItem;
            if (i < data->numItems) {
              x = gad->rect.topLeft.x + col * iw;
              y = gad->rect.topLeft.y + 2 + row * ih;
              if (event->eType == frmGadgetEnterEvent) {
                printResource(data, &data->item[i], x, y, true);
              } else {
                printResource(data, &data->item[i], x, y, false);
                if (!pumpkin_dia_enabled() && !pumpkin_is_single()) {
                  editResource(data, &data->item[i]);
                }
              }
            }
          }
          break;
        case launcher_rec:
          if (row == 0) {
            x = event->screenX - gad->rect.topLeft.x;
            y = gad->rect.topLeft.y + 2;
            columnClicked(data, x, y, event->eType == frmGadgetEnterEvent);
          } else {
            i = (row-1) * ncols + col + data->topItem;
            if (i < data->numItems) {
              x = gad->rect.topLeft.x + col * iw;
              y = gad->rect.topLeft.y + 2 + row * ih;
              if (event->eType == frmGadgetEnterEvent) {
                printRecord(data, &data->item[i], x, y, true);
              } else {
                printRecord(data, &data->item[i], x, y, false);
                if (!pumpkin_dia_enabled() && !pumpkin_is_single()) {
                  editRecord(data, &data->item[i]);
                }
              }
            }
          }
          break;
        case launcher_file:
          if (row == 0) {
            x = event->screenX - gad->rect.topLeft.x;
            y = gad->rect.topLeft.y + 2;
            columnClicked(data, x, y, event->eType == frmGadgetEnterEvent);
          } else {
            i = (row-1) * ncols + col + data->topItem;
            if (i < data->numItems) {
              x = gad->rect.topLeft.x + col * iw;
              y = gad->rect.topLeft.y + 2 + row * ih;
              if (event->eType == frmGadgetEnterEvent) {
                printFile(data, &data->item[i], x, y, true);
              } else {
                printFile(data, &data->item[i], x, y, false);
                frm = FrmGetActiveForm();
                plen = StrLen(data->path);
                if (StrCompare(data->item[i].name, PARENT_DIR) == 0) {
                  for (plen--; data->path[plen-1] != '/'; plen--);
                  data->path[plen] = 0;
                  launcherScan(data);
                  refresh(frm, data);
                  UpdateStatus(frm, data, true);
                } else if (data->item[i].dir) {
                  len = StrLen(data->item[i].name);
                  if (plen + len + 1 < MAX_NAME-1) {
                    StrCat(data->path, data->item[i].name);
                    StrCat(data->path, "/");
                    launcherScan(data);
                    refresh(frm, data);
                    UpdateStatus(frm, data, true);
                  }
                }
              }
            }
          }
          break;
        case launcher_task:
          if (row == 0) {
            x = event->screenX - gad->rect.topLeft.x;
            y = gad->rect.topLeft.y + 2;
            columnClicked(data, x, y, event->eType == frmGadgetEnterEvent);
          } else {
            i = (row-1) * ncols + col + data->topItem;
            if (i < data->numItems) {
              x = gad->rect.topLeft.x + col * iw;
              y = gad->rect.topLeft.y + 2 + row * ih;
              if (event->eType == frmGadgetEnterEvent) {
                printTask(data, &data->item[i], x, y, true);
              } else {
                printTask(data, &data->item[i], x, y, false);
              }
            }
          }
          break;
        case launcher_registry:
          if (row == 0) {
            x = event->screenX - gad->rect.topLeft.x;
            y = gad->rect.topLeft.y + 2;
            columnClicked(data, x, y, event->eType == frmGadgetEnterEvent);
          } else {
            i = (row-1) * ncols + col + data->topItem;
            if (i < data->numItems) {
              x = gad->rect.topLeft.x + col * iw;
              y = gad->rect.topLeft.y + 2 + row * ih;
              if (event->eType == frmGadgetEnterEvent) {
                printRegistry(data, &data->item[i], x, y, true);
              } else {
                printRegistry(data, &data->item[i], x, y, false);
              }
            }
          }
          break;
      }
      break;
  }

  if (myDbRef) DmCloseDatabase(myDbRef);

  FntSetFont(old);
  WinSetDrawMode(mode);
  WinSetBackColor(bc);
  WinSetTextColor(tc);
  WinSetTextColor(fc);

  return true;
}

static void find(launcher_data_t *data) {
  FindParamsType find;
  FormType *frm, *previous;
  FieldType *fld;
  EventType event;
  RectangleType rect;
  TableType *tbl;
  GoToParamsType gotoParam;
  MemHandle h;
  char *s;
  char label[32];
  UInt16 index, row, i, j;
  UInt32 result, wait;
  Boolean ok, stop, paused;
  Err err;

  MemSet(&find, sizeof(FindParamsType), 0);
  ok = false;

  if ((frm = FrmInitForm(10500)) != NULL) {
    previous = FrmGetActiveForm();
    index = FrmGetObjectIndex(frm, 10503);
    FrmSetFocus(frm, index);

    debug(DEBUG_INFO, "Launcher", "find query");
    if (FrmDoDialog(frm) == 10504) {
      if ((fld = (FieldType *)FrmGetObjectPtr(frm, index)) != NULL) {
        if ((h = FldGetTextHandle(fld)) != NULL) {
          if ((s = MemHandleLock(h)) != NULL) {
            StrNCopy(find.strAsTyped, s, maxFindStrLen);
            MemHandleUnlock(h);
            ok = true;
          }
        }
      }
    }
    FrmDeleteForm(frm);
    FrmSetActiveForm(previous);
  }

  if (ok) {
    ok = false;
    if ((frm = FrmInitForm(10600)) != NULL) {
      previous = FrmGetActiveForm();
      FrmSetActiveForm(frm);
      FrmDrawForm(frm);

      // set label
      index = FrmGetObjectIndex(frm, 10602);
      FrmHideObject(frm, index);
      StrPrintF(label, "Matches for \"%s\"", find.strAsTyped); // XXX check size limit
      FrmCopyLabel(frm, 10602, label);
      FrmShowObject(frm, index);

      index = FrmGetObjectIndex(frm, 10603);
      FrmGetObjectBounds(frm, index, &find.rect);
      tbl = (TableType *)FrmGetObjectPtr(frm, index);

      // table: 10603
      // Cancel: 10605, not usable
      // Find more: 10606, not usable
      // Stop: 10607, usable, overlaps Cancel

      find.dbAccesMode = dmModeReadOnly;
      StrNCopy(find.strToFind, find.strAsTyped, maxFindStrLen);
      for (i = 0; i < maxFinds; i++) {
        find.idx[i] = 0xFFFF;
      }

      debug(DEBUG_INFO, "Launcher", "find begin");
      for (i = 0, stop = false, paused = false, wait = 0;;) {
        EvtGetEvent(&event, wait);
        if (SysHandleEvent(&event)) continue;
        if (MenuHandleEvent(NULL, &event, &err)) continue;

        if (!FrmDispatchEvent(&event)) {
          switch (event.eType) {
            case ctlSelectEvent:
              if (event.data.ctlSelect.pControl->style == buttonCtl) {
                switch (event.data.ctlSelect.controlID) {
                  case 10606:  // Find more
                    debug(DEBUG_INFO, "Launcher", "find more");
                    WinEraseRectangle(&find.rect, 0);
                    find.numMatches = 0;
                    find.lineNumber = 0;
                    for (i = 0; i < maxFinds; i++) {
                      find.idx[i] = 0xFFFF;
                    }
                    paused = false;
                    // hide "Find more" button
                    index = FrmGetObjectIndex(frm, 10606);
                    FrmHideObject(frm, index);
                    break;
                  case 10605:  // Cancel
                  case 10607:  // Stop
                    debug(DEBUG_INFO, "Launcher", "find cancel");
                    stop = true;
                    break;
                }
              }
              break;
            case tblEnterEvent:
              row = event.data.tblSelect.row;
              if (row < maxFinds && find.idx[row] < maxFinds) {
                row = find.idx[row];
                gotoParam.searchStrLen = StrLen(find.strAsTyped);        // length of search string.
                gotoParam.dbCardNo = find.match[row].dbCardNo;           // card number of the database
                gotoParam.dbID = find.match[row].dbID;                   // LocalID of the database
                gotoParam.recordNum = find.match[row].recordNum;         // index of record that contain a match
                gotoParam.matchPos = find.match[row].matchPos;           // postion in record of the match.
                gotoParam.matchFieldNum = find.match[row].matchFieldNum; // field number string was found int
                gotoParam.matchCustom = find.match[row].matchCustom;     // application specific info
                debug(DEBUG_INFO, "Launcher", "find selected item %d", row);
                ok = true;
              } else {
                debug(DEBUG_ERROR, "Launcher", "find invalid item %d", row);
              }
              stop = true;
              break;
            case appStopEvent:
              debug(DEBUG_INFO, "Launcher", "find stop");
              stop = true;
              break;
            default:
              break;
          }
        }
        if (stop) break;

        if (i < data->numItems) {
          if (find.numMatches < maxFinds) {
            debug(DEBUG_INFO, "Launcher", "find launch \"%s\"", data->item[i].name);
            if (SysAppLaunch(0, data->item[i].dbID, 0, sysAppLaunchCmdFind, &find, &result) == errNone) {
              debug(DEBUG_INFO, "Launcher", "find %d matches", find.numMatches);
              if (find.more) {
                debug(DEBUG_INFO, "Launcher", "find more");
              } else {
                // next application
                debug(DEBUG_INFO, "Launcher", "find next");
                find.recordNum = 0;
                i++;
              }
            } else {
              // next application
              debug(DEBUG_ERROR, "Launcher", "find launch error");
              find.recordNum = 0;
              i++;
            }
          } else if (!paused) {
            debug(DEBUG_INFO, "Launcher", "find pause");
            for (j = 0; j < maxFinds; j++) {
              TblSetRowUsable(tbl, j, false);
            }
            for (j = 0; j < maxFinds; j++) {
              if (find.idx[j] < maxFinds) TblSetRowUsable(tbl, j, true);
            }
            TblSetColumnUsable(tbl, 0, true);
            paused = true;
            wait = evtWaitForever;
            // show "Find more" button
            index = FrmGetObjectIndex(frm, 10606);
            FrmShowObject(frm, index);
          }
        } else if (i == data->numItems) {
          debug(DEBUG_INFO, "Launcher", "find done");
          for (j = 0; j < maxFinds; j++) {
            TblSetRowUsable(tbl, j, false);
          }
          for (j = 0; j < maxFinds; j++) {
            if (find.idx[j] < maxFinds) TblSetRowUsable(tbl, j, true);
          }
          TblSetColumnUsable(tbl, 0, true);
          wait = evtWaitForever;
          // hide "Stop" button
          index = FrmGetObjectIndex(frm, 10607);
          FrmHideObject(frm, index);
          // show "Cancel" button
          index = FrmGetObjectIndex(frm, 10605);
          FrmShowObject(frm, index);
          i++;
        }
      }

      MemMove(&rect, &frm->window.windowBounds, sizeof(RectangleType));
      if (frm->window.windowFlags.modal) {
        rect.topLeft.x -= 2;
        rect.topLeft.y -= 2;
        rect.extent.x += 4;
        rect.extent.y += 4;
      }
      WinRestoreRectangle(frm->bitsBehindForm, &rect);

      FrmDeleteForm(frm);
      FrmSetActiveForm(previous);
    }
  }

  if (ok) {
    for (i = 0; i < data->numItems; i++) {
      if (data->item[i].dbID == find.match[row].appDbID) {
        // If your application was the current
        // application before the user selected the Find command, the launch
        // flag is clear to indicate that your globals should not be re-initialized.
        debug(DEBUG_INFO, "Launcher", "find goto launch \"%s\"", data->item[i].name);
        SysAppLaunch(0, data->item[i].dbID, 0, sysAppLaunchCmdGoTo, &gotoParam, &result);
        break;
      }
    }
    if (i == data->numItems) {
      debug(DEBUG_ERROR, "Launcher", "find goto appID %d not found", gotoParam.dbID);
    }
  }
}

static void LauncherOpenForm(UInt16 formId) {
  FormType *frm, *previous;

  if ((frm = FrmInitForm(formId)) != NULL) {
    previous = FrmGetActiveForm();
    FrmDoDialog(frm);
    FrmDeleteForm(frm);
    FrmSetActiveForm(previous);
  }
}

static void DrawBattery(void) {
  FormType *frm;
  RectangleType rect;
  RGBColorType rgb, old;
  UInt16 battery;

  frm = FrmGetActiveForm();
  FrmGetFormBounds(frm, &rect);

  battery = pumpkin_get_battery();

  rgb.r = 0x00;
  rgb.g = 0x40;
  rgb.b = 0xff;
  WinSetBackColorRGB(&rgb, &old);
  RctSetRectangle(&rect, (rect.extent.x - BATTERY_WIDTH) / 2, 4, (battery * BATTERY_WIDTH) / 100, 5);
  WinEraseRectangle(&rect, 0);

  if (rect.extent.x < BATTERY_WIDTH) {
    rgb.r = 0x80;
    rgb.g = 0x80;
    rgb.b = 0x80;
    WinSetBackColorRGB(&rgb, NULL);
    rect.topLeft.x += rect.extent.x;
    rect.extent.x = BATTERY_WIDTH - rect.extent.x;
    WinEraseRectangle(&rect, 0);
  }

  WinSetBackColorRGB(&old, NULL);
}

static void UpdateStatus(FormPtr frm, launcher_data_t *data, Boolean title) {
  RectangleType rect;
  DateTimeType dt;
  UInt32 t, tf;
  FontID old;
  UInt16 objIndex;
  Boolean update;
  int width;

  update = false;
  t = TimGetSeconds();
  TimSecondsToDateTime(t, &dt);
  if (dt.minute != data->lastMinute) {
    update = true;
    data->lastMinute = dt.minute;
  }

  objIndex = FrmGetObjectIndex(frm, filterCtl);

  switch (data->mode) {
    case launcher_app:
    case launcher_app_small:
      if (update) {
        tf = PrefGetPreference(prefTimeFormat);
        TimeToAscii(dt.hour, dt.minute, tf, data->title);
        FrmSetTitle(frm, data->title);
      }
      FrmHideObject(frm, objIndex);
      break;
    case launcher_db:
      if (title) FrmSetTitle(frm, "Databases");
      FrmShowObject(frm, objIndex);
      break;
    case launcher_rsrc:
      if (title) FrmSetTitle(frm, data->name);
      FrmShowObject(frm, objIndex);
      break;
    case launcher_rec:
      if (title) FrmSetTitle(frm, data->name);
      FrmHideObject(frm, objIndex);
      break;
    case launcher_file:
      if (title) {
        FrmGetObjectBounds(frm, objIndex, &rect);
        old = FntSetFont(boldFont);
        adjustName(data->title, MAX_NAME, data->path, &width, rect.topLeft.x - 8);
        FntSetFont(old);
        FrmSetTitle(frm, data->title);
      }
      FrmHideObject(frm, objIndex);
      break;
    case launcher_task:
      if (title) FrmSetTitle(frm, "Tasks");
      FrmHideObject(frm, objIndex);
      break;
    case launcher_registry:
      if (title) FrmSetTitle(frm, "Registry");
      FrmHideObject(frm, objIndex);
      break;
  }

  if (update) DrawBattery();
}

void setField(FormType *frm, UInt16 fieldId, char *s, Boolean focus) {
  FieldType *fld;
  UInt16 objIndex, len;
  FieldAttrType attr;
  Boolean old;

  objIndex = FrmGetObjectIndex(frm, fieldId);
  fld = (FieldPtr)FrmGetObjectPtr(frm, objIndex);

  FldGetAttributes(fld, &attr);
  old = attr.editable;
  attr.editable = true;
  FldSetAttributes(fld, &attr);

  len = FldGetTextLength(fld);
  if (len) FldDelete(fld, 0, len);
  FldInsert(fld, s, StrLen(s));

  attr.editable = old;
  FldSetAttributes(fld, &attr);

  if (focus) fld->attr.hasFocus = true;
}

void setField4char(FormType *frm, UInt16 fieldId, UInt32 value, Boolean focus) {
  char buf[8];

  pumpkin_id2s(value, buf);
  setField(frm, fieldId, buf, focus);
}

void setFieldNum(FormType *frm, UInt16 fieldId, UInt32 value, Boolean focus) {
  char buf[8];

  if (value != 0xffffffff) {
    sys_snprintf(buf, sizeof(buf)-1, "%d", value);
    setField(frm, fieldId, buf, focus);
  }
}

Boolean getField(FormType *frm, UInt16 fieldId, char *buf, int size) {
  FieldType *fld;
  UInt16 objIndex;
  char *s;
  Boolean r = false;

  objIndex = FrmGetObjectIndex(frm, fieldId);
  if ((fld = (FieldType *)FrmGetObjectPtr(frm, objIndex)) != NULL) {
    s = FldGetTextPtr(fld);
    if (s && s[0]) {
      StrNCopy(buf, s, size-1);
      r = true;
    }
  }

  return r;
}

UInt32 getField4Char(FormType *frm, UInt16 fieldId) {
  char buf[8];
  UInt32 value = 0;

  if (getField(frm, fieldId, buf, sizeof(buf))) {
    pumpkin_s2id(&value, buf);
  }

  return value;
}

UInt32 getFieldNum(FormType *frm, UInt16 fieldId) {
  char buf[8];
  UInt32 value = 0xffffffff;

  if (getField(frm, fieldId, buf, sizeof(buf))) {
    value = sys_atoi(buf);
  }

  return value;
}

void setControlValue(FormType *frm, UInt16 ctlId, Int16 value) {
  ControlType *ctl;
  UInt16 objIndex;

  objIndex = FrmGetObjectIndex(frm, ctlId);
  ctl = (ControlType *)FrmGetObjectPtr(frm, objIndex);
  CtlSetValue(ctl, value);
}

Int16 getControlValue(FormType *frm, UInt16 ctlId) {
  ControlType *ctl;
  UInt16 objIndex;

  objIndex = FrmGetObjectIndex(frm, ctlId);
  ctl = (ControlType *)FrmGetObjectPtr(frm, objIndex);

  return CtlGetValue(ctl);
}

void showDynamicForm(const dynamic_form_item_t *items, char *title, void (*callback)(FormType *frm, dynamic_form_phase_t phase, void *data), void *data) {
  FormType *previous, *formP;
  UInt16 formW, formH, x, y, width, height, id;
  UInt8 group;
  FontID old;
  int i, r = 0;

  height = FntCharHeight();
  formW = 156;
  formH = 15 + 2;
  y = formH;
  for (i = 0; items[i].label; i++) {
    formH += height + 2;
  }
  formH += 8 + height + 4;
  group = 0;
  id = 0;

  if ((formP = FrmNewForm(1000, title, 2, 158-formH, formW, formH, true, 1000, 0, 0)) != NULL) {
    for (i = 0; items[i].label; i++) {
      switch (items[i].type) {
        case alphaItem:
        case numericItem:
          old = FntSetFont(boldFont);
          width = FntCharsWidth(items[i].label, StrLen(items[i].label));
          FntSetFont(old);
          x = 65 - width - 4;
          FrmNewLabel(&formP, 2000+id, items[i].label, x, y, boldFont);
          x = 65;
          width = FntCharWidth(items[i].type == numericItem ? '0' : 'w') * items[i].maxChars;
          if (x + width >= formW-4) width = formW - 4 - x;
          FldNewField((void **)&formP, 1000+id, x, y, width, height+2, stdFont, items[i].maxChars, true, true,
            true, false, leftAlign, false, false, items[i].type == numericItem);
          id++;
          break;
        case numPairItem:
          old = FntSetFont(boldFont);
          width = FntCharsWidth(items[i].label, StrLen(items[i].label));
          FntSetFont(old);
          x = 65 - width - 4;
          FrmNewLabel(&formP, 2000+id, items[i].label, x, y, boldFont);
          x = 65;
          width = FntCharWidth('0') * 4;
          FldNewField((void **)&formP, 1000+id, x, y, width, height+2, stdFont, 3, true, true, true, false, leftAlign, false, false, true);
          x += width + 8;
          id++;
          FldNewField((void **)&formP, 1000+id, x, y, width, height+2, stdFont, 3, true, true, true, false, leftAlign, false, false, true);
          id++;
          break;
        case checkboxItem:
          old = FntSetFont(stdFont);
          width = FntCharWidth('w') * 2 + FntCharsWidth(items[i].label, StrLen(items[i].label));
          FntSetFont(old);
          x = 4;
          if (x + width >= formW-4) width = formW - 4 - x;
          CtlNewControl((void **)&formP, 1000+id, checkboxCtl, items[i].label, x, y, width, height+2, stdFont, group, true);
          id++;
          break;
      }
      y += height + 2;
    }

    x = 4;
    y += 6;
    width = FntCharsWidth("<Ok>", 4);
    CtlNewControl((void **)&formP, 3001, buttonCtl, "Ok", x, y, width, height+2, stdFont, 0, true);
    x += width + 6;
    width = FntCharsWidth("<Cancel>", 8);
    CtlNewControl((void **)&formP, 3002, buttonCtl, "Cancel", x, y, width, height+2, stdFont, 0, true);
     
    previous = FrmGetActiveForm();
    callback(formP, setProperties, data);
    r = FrmDoDialog(formP);
    if (r == 3001) {
      callback(formP, getProperties, data);
    }
    FrmDeleteForm(formP);
    FrmSetActiveForm(previous);
    callback(formP, finishForm, data);
  }
}

static void dbFiltercallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  launcher_data_t *data = (launcher_data_t *)_data;

  switch (phase) {
    case setProperties:
      setField4char(frm, 1000, data->filterDbType, true);
      setField4char(frm, 1001, data->filterCreator, false);
      setControlValue(frm, 1002, data->filterRsrc);
      break;
    case getProperties:
      data->filterDbType = getField4Char(frm, 1000);
      data->filterCreator = getField4Char(frm, 1001);
      data->filterRsrc = getControlValue(frm, 1002);
      break;
    case finishForm:
      frm = FrmGetActiveForm();
      launcherScan(data);
      refresh(frm, data);
      break;
  }
}

static void rsrcFiltercallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  launcher_data_t *data = (launcher_data_t *)_data;

  switch (phase) {
    case setProperties:
      setField4char(frm, 1000, data->filterResType, true);
      setFieldNum(frm, 1001, data->filterId, false);
      break;
    case getProperties:
      data->filterResType = getField4Char(frm, 1000);
      data->filterId = getFieldNum(frm, 1001);
      break;
    case finishForm:
      frm = FrmGetActiveForm();
      launcherScan(data);
      refresh(frm, data);
      break;
  }
}

static void showDbFilter(launcher_data_t *data) {
  showDynamicForm(dbFilterItems, "DB Filter", dbFiltercallback, data);
}

static void showRsrcFilter(launcher_data_t *data) {
  showDynamicForm(rsrcFilterItems, "RSRC Filter", rsrcFiltercallback, data);
}

static void deleteApplication(launcher_item_t *item) {
  DmSearchStateType stateInfo;
  char name[dmDBNameLength], screator[8], stype[8];
  Boolean newSearch;
  LocalID dbID;
  UInt32 type;

  pumpkin_id2s(item->creator, screator);
  debug(DEBUG_INFO, "Launcher", "deleting application '%s'", screator);

  // if deleting the application was successful, then delete the other databases
  if (DmDeleteDatabase(0, item->dbID) == errNone) {
    for (newSearch = true;; newSearch = false) {
      if (DmGetNextDatabaseByTypeCreator(newSearch, &stateInfo, 0, item->creator, false, NULL, &dbID) != errNone) break;
      if (DmDatabaseInfo(0, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, NULL) == errNone) {
        pumpkin_id2s(type, stype);
        debug(DEBUG_INFO, "Launcher", "deleting database \"%s\" type '%s' creator '%s'", name, stype, screator);
        DmDeleteDatabase(0, dbID);
      }
    }
    pumpkin_delete_preferences(item->creator, true);
    pumpkin_delete_preferences(item->creator, false);
  } else {
    FrmCustomAlert(ErrorAlert, "Could not delete application. Is it running?", "", "");
  }
}

static void MenuEvent(UInt16 id, launcher_data_t *data) {
  FormPtr frm;
  UInt16 flags;
  UInt32 result;

  switch (id) {
    case shutdownCmd:
      data->finish = true;
      break;
    case findCmd:
      find(data);
      break;
      break;
    case appCmd:
      frm = FrmGetActiveForm();
      if (data->mode != launcher_app) {
        data->mode = launcher_app;
        data->sort = 0;
        data->dir = 0;
      }
      data->lastMinute = -1;
      launcherScan(data);
      refresh(frm, data);
      UpdateStatus(frm, data, true);
      frm->mbar = data->mainMenu;
      MenuSetActiveMenu(frm->mbar);
      break;
    case appSmallCmd:
      frm = FrmGetActiveForm();
      if (data->mode != launcher_app_small) {
        data->mode = launcher_app_small;
        data->sort = 0;
        data->dir = 0;
      }
      data->lastMinute = -1;
      launcherScan(data);
      refresh(frm, data);
      UpdateStatus(frm, data, true);
      frm->mbar = data->appListMenu;
      MenuSetActiveMenu(frm->mbar);
      break;
    case dbCmd:
      if (data->mode != launcher_db) {
        data->mode = launcher_db;
        data->sort = 0;
        data->dir = 0;
        frm = FrmGetActiveForm();
        launcherScan(data);
        refresh(frm, data);
        UpdateStatus(frm, data, true);
        frm->mbar = data->mainMenu;
        MenuSetActiveMenu(frm->mbar);
      }
      break;
    case fileCmd:
      if (data->mode != launcher_file) {
        data->mode = launcher_file;
        data->sort = 0;
        data->dir = 0;
        frm = FrmGetActiveForm();
        launcherScan(data);
        refresh(frm, data);
        UpdateStatus(frm, data, true);
        frm->mbar = data->mainMenu;
        MenuSetActiveMenu(frm->mbar);
      }
      break;
    case taskCmd:
      data->mode = launcher_task;
      data->sort = 0;
      data->dir = 0;
      frm = FrmGetActiveForm();
      launcherScan(data);
      refresh(frm, data);
      UpdateStatus(frm, data, true);
      frm->mbar = data->mainMenu;
      MenuSetActiveMenu(frm->mbar);
      break;
    case registryCmd:
      data->mode = launcher_registry;
      data->sort = 0;
      data->dir = 0;
      frm = FrmGetActiveForm();
      launcherScan(data);
      refresh(frm, data);
      UpdateStatus(frm, data, true);
      frm->mbar = data->mainMenu;
      MenuSetActiveMenu(frm->mbar);
      break;
    case runCmd:
      if (data->mode == launcher_app_small && data->prev >= 0 && data->item[data->prev].creator != AppID) {
        data->top = false;
        flags = sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp;
        SysAppLaunchEx(0, data->item[data->prev].dbID, flags, sysAppLaunchCmdNormalLaunch, NULL, &result, data->item[data->prev].pilot_main);
      }
      break;
    case delCmd:
      if (data->mode == launcher_app_small && data->prev >= 0) {
        if (data->item[data->prev].creator == AppID || data->item[data->prev].creator == 'Pref') {
          FrmCustomAlert(ErrorAlert, "This application can not be deleted.", "", "");
        } else {
          if (FrmCustomAlert(QuestionAlert, "Delete application?", "", "") == 0) {
            deleteApplication(&data->item[data->prev]);
            data->prev = -1;
            frm = FrmGetActiveForm();
            launcherScan(data);
            refresh(frm, data);
            UpdateStatus(frm, data, true);
          }
        }
      }
      break;
    case forkCmd:
      pumpkin_fork();
      break;
    case aboutCmd:
      LauncherOpenForm(AboutForm);
      break;
  }
}

static void resize(FormType *frm, launcher_data_t *data) {
  WinHandle wh;
  RectangleType rect;
  ScrollBarType *scl;
  UInt32 swidth, sheight;
  UInt16 objIndex, rows, cols, totalRows;

  WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
  wh = FrmGetWindowHandle(frm);
  RctSetRectangle(&rect, 0, 0, swidth, sheight);
  WinSetBounds(wh, &rect); 

  // filter button
  objIndex = FrmGetObjectIndex(frm, filterCtl);
  FrmGetObjectBounds(frm, objIndex, &rect);
  rect.topLeft.x = swidth - rect.extent.x - 1;
  FrmSetObjectBounds(frm, objIndex, &rect);

  // gadget
  objIndex = FrmGetObjectIndex(frm, iconsGad);
  rect.topLeft.x = data->gadRect.topLeft.x;
  rect.topLeft.y = data->gadRect.topLeft.y;
  rect.extent.x = data->gadRect.extent.x + (swidth  - 160);
  rect.extent.y = data->gadRect.extent.y + (sheight - 160);
  FrmSetObjectBounds(frm, objIndex, &rect);
  FrmGetObjectBounds(frm, objIndex, &data->gadRect);

  // scrollBar
  objIndex = FrmGetObjectIndex(frm, iconsScl);
  scl = (ScrollBarType *)FrmGetObjectPtr(frm, objIndex);
  rows = rect.extent.y / data->cellHeight;
  if (data->mode != launcher_app) rows--;
  cols = rect.extent.x / data->cellWidth;
  totalRows = (data->numItems + cols - 1) / cols;
  if (totalRows > rows) {
    SclSetScrollBar(scl, 0, 0, totalRows - rows, (totalRows - rows) >= rows ? rows - 1 : totalRows - rows);
  }
  rect.topLeft.x = swidth - data->sclRect.extent.x;
  rect.topLeft.y = data->sclRect.topLeft.y;
  rect.extent.x = data->sclRect.extent.x;
  rect.extent.y = data->sclRect.extent.y + (sheight - 160);
  FrmSetObjectBounds(frm, objIndex, &rect);

  FrmSetUsable(frm, objIndex, totalRows > rows);
}

static Boolean MainFormHandleEvent(EventPtr event) {
  launcher_data_t *data;
  FormType *frm;
  RectangleType rect;
  FormGadgetTypeInCallback *gad;
  UInt16 index, sclIndex, gadIndex, cols;
  Boolean handled;

  data = pumpkin_get_data();
  handled = false;

  switch (event->eType) {
    case appRaiseEvent: 
      PINSetInputTriggerState(pinInputTriggerEnabled);
      data->top = true;
      frm = FrmGetActiveForm();
      UpdateStatus(frm, data, true);
      handled = true; 
      break;

    case frmOpenEvent:
      frm = FrmGetActiveForm();
      data->mainMenu = frm->mbar;
      gadIndex = FrmGetObjectIndex(frm, iconsGad);
      FrmGetObjectBounds(frm, gadIndex, &data->gadRect);
      sclIndex = FrmGetObjectIndex(frm, iconsScl);
      FrmGetObjectBounds(frm, sclIndex, &data->sclRect);
      FrmSetDIAPolicyAttr(frm, frmDIAPolicyCustom);
      PINSetInputTriggerState(pinInputTriggerEnabled);
      PINSetInputAreaState(pinInputAreaOpen);
      launcherScan(data);
      resize(frm, data);
      FrmSetGadgetHandler(frm, gadIndex, ItemsGadgetCallback);

      index = FrmGetObjectIndex(frm, filterCtl);
      switch (data->mode) {
        case launcher_db:
        case launcher_rsrc:
          break;
        default:
          FrmHideObject(frm, index);
          break;
      }
      FrmDrawForm(frm);

      UpdateStatus(frm, data, true);
      handled = true;
      break;

    case winDisplayChangedEvent:
      frm = FrmGetActiveForm();
      resize(frm, data);
      FrmDrawForm(frm);
      handled = true;
      break;

    case nilEvent:
      if (data->top) {
        frm = FrmGetActiveForm();
        UpdateStatus(frm, data, false);
      }
      handled = true;
      break;

    case keyDownEvent:
      if (event->data.keyDown.modifiers & commandKeyMask) {
        switch (event->data.keyDown.chr) {
          case vchrAppCrashed:
            FrmCustomAlert(ErrorAlert, "An app has crashed and was terminated.", "", "");
            // fall through
          case vchrAppFinished:
          case vchrRefreshState:
            if (data->mode == launcher_task) {
              MenuEvent(taskCmd, data);
            }
            handled = true;
            break;
        }
      }
      break;

    case menuEvent:
      MenuEvent(event->data.menu.itemID, data);
      handled = true;
      break;

    case ctlSelectEvent:
      if (event->data.ctlSelect.controlID == filterCtl) {
        switch (data->mode) {
          case launcher_db:
            showDbFilter(data);
            break;
          case launcher_rsrc:
            showRsrcFilter(data);
            break;
          default:
            break;
        }
        handled = true;
      }
      break;

    case sclRepeatEvent:
      frm = FrmGetActiveForm();
      gadIndex = FrmGetObjectIndex(frm, iconsGad);
      FrmGetObjectBounds(frm, gadIndex, &rect);
      cols = rect.extent.x / data->cellWidth;
      data->topItem = event->data.sclRepeat.newValue * cols;
      if (data->topItem >= data->numItems) {
        data->topItem = data->numItems-1;
      }
      gad = (FormGadgetTypeInCallback *)FrmGetObjectPtr(frm, gadIndex);
      ItemsGadgetCallback(gad, formGadgetDrawCmd, NULL);
      break;

    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventPtr event) {
  FormPtr frm;
  UInt16 form;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      form = event->data.frmLoad.formID;
      frm = FrmInitForm(form);
      FrmSetActiveForm(frm);
      FrmSetEventHandler(frm, MainFormHandleEvent);
      handled = true;
      break;
    case frmCloseEvent:
      break;
    default:
      break;
  }

  return handled;
}

static void CheckNotifications(void) {
  launcher_data_t *data = pumpkin_get_data();
  Boolean deploy = false, reload = false;

  if (mutex_lock(data->mutex) == 0) {
    deploy = data->deploy;
    reload = data->reload;
    data->deploy = false;
    data->reload = false;
    mutex_unlock(data->mutex);
  }

  if (deploy) {
    debug(DEBUG_INFO, "Launcher", "CheckNotifications deploy");
    pumpkin_deploy_files("/app_install");
    reload = true;
  }

  if (reload) {
    debug(DEBUG_INFO, "Launcher", "CheckNotifications reload");
    pumpkin_local_refresh();

    switch (data->mode) {
      case launcher_app:
        MenuEvent(appCmd, data);
        break;
      case launcher_app_small:
        MenuEvent(appSmallCmd, data);
        break;
      default:
        break;
    }
  }
}

static void EventLoop(launcher_data_t *data) {
  EventType event;
  Err err;

  data->finish = false;
  data->top = true;
  debug(DEBUG_INFO, "Launcher", "event loop begin");
  do {
    CheckNotifications();
    EvtGetEvent(&event, 30);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);
  } while (event.eType != appStopEvent && !data->finish);
  debug(DEBUG_INFO, "Launcher", "event loop end");
}

static Err LauncherNotificationHandler(SysNotifyParamType *notifyParamsP) {
  launcher_data_t *data = (launcher_data_t *)notifyParamsP->userDataP;
  SysNotifyDBCreatedType *dbCreated;
  SysNotifyDBDeletedType *dbDeleted;
  Boolean reload;
  char stype[8], screator[8];

  if (data && mutex_lock(data->mutex) == 0) {
    pumpkin_id2s(notifyParamsP->notifyType, stype);
    debug(DEBUG_INFO, "Launcher", "LauncherNotificationHandler notification type '%s' received", stype);
    reload = false;

    switch (notifyParamsP->notifyType) {
      case sysNotifyDisplayChangeEvent:
        debug(DEBUG_INFO, "Launcher", "sysNotifyDisplayChangeEvent ok");
        break;
      case sysNotifySyncFinishEvent:
        if (!data->deploy) {
          debug(DEBUG_INFO, "Launcher", "sysNotifySyncFinishEvent deploy scheduled");
          data->deploy = true;
        }
        break;
      case sysNotifyDBCreatedEvent:
        dbCreated = (SysNotifyDBCreatedType *)notifyParamsP->notifyDetailsP;
        if (dbCreated) {
          pumpkin_id2s(dbCreated->type, stype);
          pumpkin_id2s(dbCreated->creator, screator);
          debug(DEBUG_INFO, "Launcher", "sysNotifyDBCreatedEvent database type '%s' creator '%s'", stype, screator);
          reload = dbCreated->type == sysFileTApplication;
        }
        notifyParamsP->handled = true;
        break;
      case sysNotifyDBDeletedEvent:
        dbDeleted = (SysNotifyDBDeletedType *)notifyParamsP->notifyDetailsP;
        if (dbDeleted) {
          pumpkin_id2s(dbDeleted->type, stype);
          pumpkin_id2s(dbDeleted->creator, screator);
          debug(DEBUG_INFO, "Launcher", "sysNotifyDBDeletedEvent database type '%s' creator '%s'", stype, screator);
          reload = dbDeleted->type == sysFileTApplication;
        }
        notifyParamsP->handled = true;
        break;
    }

    if (reload && !data->reload) {
      debug(DEBUG_INFO, "Launcher", "reload scheduled");
      data->reload = true;
    }
    mutex_unlock(data->mutex);
  }

  return errNone;
}

#ifdef ANDROID
UInt32 LauncherPilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
#else
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
#endif
{
  launcher_data_t *data;
  MemHandle fontHandle;
  FontType *font;
  UInt32 value;

  switch (cmd) {
    case sysAppLaunchCmdNormalLaunch:
      break;
    case sysAppLaunchCmdNotify:
      debug(DEBUG_INFO, "Launcher", "launch code sysAppLaunchCmdNotify received");
      LauncherNotificationHandler((SysNotifyParamType *)cmdPBP);
      return 0;
    default:
      debug(DEBUG_INFO, "Launcher", "launch code %d ignored", cmd);
      return 0;
  }

  if (pumpkin_dia_enabled() || pumpkin_is_single()) {
    pumpkin_set_spawner(thread_get_handle());
    pumpkin_dia_kbd();
  }

  switch (SYS_OS) {
    case 1:
      debug(DEBUG_INFO, PUMPKINOS, "Host OS is Linux based");
      break;
    case 2:
      debug(DEBUG_INFO, PUMPKINOS, "Host OS is Windows based");
      break;
    case 3:
      debug(DEBUG_INFO, PUMPKINOS, "Host OS is SerenityOS");
      break;
    case 4:
      debug(DEBUG_INFO, PUMPKINOS, "Host OS is Android");
      break;
    default:
      debug(DEBUG_ERROR, PUMPKINOS, "Host OS is unknown");
      break;
  }

  if (FtrGet(sysFileCSystem, sysFtrNumProcessorID, &value) == errNone) {
    if (sysFtrNumProcessorIsARM(value)) {
      debug(DEBUG_INFO, PUMPKINOS, "Host CPU is ARM");
    } else if ((value & sysFtrNumProcessorMask) == sysFtrNumProcessorx86) {
      debug(DEBUG_INFO, PUMPKINOS, "Host CPU is x86");
    } else {
      debug(DEBUG_ERROR, PUMPKINOS, "Host CPU is unknown");
    }
  }

  switch (SYS_SIZE) {
    case 1:
      debug(DEBUG_INFO, PUMPKINOS, "Host word size is 32");
      break;
    case 2:
      debug(DEBUG_INFO, PUMPKINOS, "Host word size is 64");
      break;
    default:
      debug(DEBUG_ERROR, PUMPKINOS, "Host word size is unknown");
      break;
  }

  data = xcalloc(1, sizeof(launcher_data_t));
  pumpkin_set_data(data);
  data->mutex = mutex_create("launcher");
  data->lastMinute = -1;
  data->mode = launcher_app;
  data->filterId = 0xffffffff;

  fontHandle = DmGetResource(fontExtRscType, fakeFnt);
  font = MemHandleLock(fontHandle);
  FntDefineFont(fakeMonoFont, font);
  MemHandleUnlock(fontHandle);
  DmReleaseResource(fontHandle);

  StrCopy(data->path, "/");

  SysNotifyRegister(0, pumpkin_get_app_localid(), sysNotifySyncFinishEvent, LauncherNotificationHandler, sysNotifyNormalPriority, data);
  SysNotifyRegister(0, pumpkin_get_app_localid(), sysNotifyDBCreatedEvent,  LauncherNotificationHandler, sysNotifyNormalPriority, data);
  SysNotifyRegister(0, pumpkin_get_app_localid(), sysNotifyDBDeletedEvent,  LauncherNotificationHandler, sysNotifyNormalPriority, data);
  SysNotifyRegister(0, pumpkin_get_app_localid(), sysNotifyDisplayChangeEvent, NULL, sysNotifyNormalPriority, data);

  data->appListMenu = MenuInit(AppListMenu);

  FrmCenterDialogs(true);
  FrmGotoForm(MainForm);
  EventLoop(data);
  FrmCloseAllForms();

  MenuDispose(data->appListMenu);

  SysNotifyUnregister(0, pumpkin_get_app_localid(), sysNotifySyncFinishEvent, sysNotifyNormalPriority);
  SysNotifyUnregister(0, pumpkin_get_app_localid(), sysNotifyDBCreatedEvent,  sysNotifyNormalPriority);
  SysNotifyUnregister(0, pumpkin_get_app_localid(), sysNotifyDBDeletedEvent,  sysNotifyNormalPriority);

  launcherResetItems(data);
  pumpkin_set_data(NULL);
  mutex_destroy(data->mutex);
  xfree(data);
  if (!(launchFlags & sysAppLaunchFlagFork)) pumpkin_set_finish(1);

  return 0;
}
