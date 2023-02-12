#include <PalmOS.h>

#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "thread.h"
#include "mutex.h"
#include "sys.h"
#include "pwindow.h"
#include "vfs.h"
#include "mem.h"
#include "bytes.h"
#include "pumpkin.h"
#include "AppRegistry.h"
#include "storage.h"
#include "debug.h"
#include "xalloc.h"

typedef struct {
  MenuBarType *currentMenu;
} menu_module_t;

extern thread_key_t *menu_key;

int MenuInitModule(void) {
  menu_module_t *module;

  if ((module = xcalloc(1, sizeof(menu_module_t))) == NULL) {
    return -1;
  }

  thread_set(menu_key, module);

  return 0;
}

void *MenuReinitModule(void *module) {
  menu_module_t *old = NULL;

  if (module) {
    MenuFinishModule();
    thread_set(menu_key, module);
  } else {
    old = (menu_module_t *)thread_get(menu_key);
    MenuInitModule();
  }

  return old;
}

int MenuFinishModule(void) {
  menu_module_t *module = (menu_module_t *)thread_get(menu_key);

  if (module) {
    xfree(module);
  }

  return 0;
}

MenuBarType *MenuInit(UInt16 resourceId) {
  MenuBarType *mbar = NULL;
  MemHandle menuH;
  MenuPullDownType *pd;
  UInt32 swidth;
  Coord w, h;
  FontID old;
  Err err;
  Int16 height, i;

  debug(DEBUG_TRACE, "Menu", "MenuInit %d", resourceId);
  if ((menuH = DmGetResource(MenuRscType, resourceId)) != NULL_HANDLE) {
    mbar = MemHandleLock(menuH);
    old = FntSetFont(boldFont);
    height = FntCharHeight() + 4;
    WinScreenMode(winScreenModeGet, &swidth, NULL, NULL, NULL);
    w = swidth;
    h = height;
    mbar->barWin = WinCreateOffscreenWindow(w, h, nativeFormat, &err);
    mbar->bitsBehind = WinCreateOffscreenWindow(w, h, nativeFormat, &err);
    FntSetFont(old);

    for (i = 0; i < mbar->numMenus; i++) {
      pd = &mbar->menus[i];
      w = pd->bounds.extent.x;
      h = pd->bounds.extent.y;
      pd->menuWin = WinCreateOffscreenWindow(w, h, nativeFormat, &err);
      pd->menuWin->windowBounds.topLeft.x = pd->bounds.topLeft.x;
      pd->menuWin->windowBounds.topLeft.y = pd->bounds.topLeft.y;
      pd->bitsBehind = WinCreateOffscreenWindow(w, h, nativeFormat, &err);
      pd->hidden = 1;
      pd->selectedItem = -1;
    }
    mbar->menus[0].hidden = 0;
    mbar->curMenu = 0;
    mbar->selectedMenu = -1;
    //MemHandleUnlock(menuH);
  }

  return mbar;
}

MenuBarType *MenuGetActiveMenu(void) {
  menu_module_t *module = (menu_module_t *)thread_get(menu_key);
  return module->currentMenu;
}

MenuBarType *MenuSetActiveMenu(MenuBarType *menuP) {
  menu_module_t *module = (menu_module_t *)thread_get(menu_key);

  debug(DEBUG_TRACE, "Menu", "MenuSetActiveMenu %p", menuP);
  MenuBarType *prev = module->currentMenu;
  module->currentMenu = menuP;

  return prev;
}

void MenuDispose(MenuBarType *menuP) {
  MenuPullDownType *pd;
  MemHandle h;
  UInt16 lockCount;
  int i;

  if (menuP) {
    debug(DEBUG_TRACE, "Menu", "MenuDispose %p", menuP);

    if ((h = MemPtrRecoverHandle(menuP)) != NULL) {
      if (MemHandleUnlockEx(h, &lockCount) == errNone && lockCount == 0) {
        if (menuP->barWin) WinDeleteWindow(menuP->barWin, false);
        if (menuP->bitsBehind) WinDeleteWindow(menuP->bitsBehind, false);

        for (i = 0; i < menuP->numMenus; i++) {
          pd = &menuP->menus[i];
          if (pd->menuWin) WinDeleteWindow(pd->menuWin, false);
          if (pd->bitsBehind) WinDeleteWindow(pd->bitsBehind, false);
        }
      }
      DmReleaseResource(h);
    }
  }
}

static void menu_show_pd_title(MenuBarType *menu, MenuPullDownType *pd, int i) {
  RectangleType rect;
  IndexedColorType mFrame, mFill, mTitle, mFore, oldb, oldt;
  WinHandle oldw;
  FontID old;

  oldw = WinGetActiveWindow();
  WinSetActiveWindow(menu->barWin);
  WinSetDrawWindow(menu->barWin);

  mFrame = UIColorGetTableEntryIndex(UIFormFrame);
  mTitle = UIColorGetTableEntryIndex(UIFormTitle);
  mFill = UIColorGetTableEntryIndex(UIMenuFill);
  mFore = UIColorGetTableEntryIndex(UIMenuForeground);
  oldb = WinSetBackColor(mFill);
  oldt = WinSetTextColor(mFore);
  old = FntSetFont(boldFont);

  if (i == menu->curMenu) {
    WinSetBackColor(mFrame);
    WinSetTextColor(mTitle);
  } else {
    WinSetBackColor(mFill);
    WinSetTextColor(mFore);
  }
  RctSetRectangle(&rect, pd->titleBounds.topLeft.x, 1, 3+FntCharsWidth(pd->title, strlen(pd->title))+3, menu->barWin->windowBounds.extent.y-2);
  WinEraseRectangle(&rect, 0);
  WinDrawChars(pd->title, strlen(pd->title), pd->titleBounds.topLeft.x+3, pd->titleBounds.topLeft.y+1);

  FntSetFont(old);
  WinSetBackColor(oldb);
  WinSetTextColor(oldt);
  WinSetActiveWindow(oldw);
}

static void menu_draw_item(MenuPullDownType *pd, MenuItemType *item, Boolean inverted) {
  WinHandle oldw, oldd;
  char cmd[8];
  int mFrame, mTitle, mFill, mFore, old, oldb, oldt, px, y;

  oldw = WinGetActiveWindow();
  WinSetActiveWindow(pd->menuWin);
  oldd = WinSetDrawWindow(pd->menuWin);

  cmd[0] = 22;
  cmd[1] = 'W';
  cmd[3] = 0;
  old = FntSetFont(boldFont);
  //px = FntCharsWidth(cmd, 2) + FntCharWidth('A');
  px = FntCharsWidth(cmd, 2) - 1;
  y = item->localBounds.topLeft.y;

  mFrame = UIColorGetTableEntryIndex(UIFormFrame);
  mTitle = UIColorGetTableEntryIndex(UIFormTitle);
  mFill = UIColorGetTableEntryIndex(UIMenuFill);
  mFore = UIColorGetTableEntryIndex(UIMenuForeground);

  if (inverted) {
    oldb = WinSetBackColor(mFrame);
    oldt = WinSetTextColor(mTitle);
  } else {
    oldb = WinSetBackColor(mFill);
    oldt = WinSetTextColor(mFore);
  }

  WinEraseRectangle(&item->localBounds, 0);
  WinDrawChars(item->itemStr, strlen(item->itemStr), 2, y);
  if (item->command) {
    cmd[1] = item->command;
    WinDrawChars(cmd, 2, item->localBounds.extent.x - px, y);
  }

  FntSetFont(old);
  WinSetBackColor(oldb);
  WinSetTextColor(oldt);
  WinSetDrawWindow(oldd);
  WinSetActiveWindow(oldw);
}

static void menu_show_pd(MenuBarType *menu, MenuPullDownType *pd) {
  RectangleType rect;
  MenuItemType *item;
  WinHandle oldw;
  IndexedColorType mFrame, mFill, mFore, oldf, oldb, oldt;
  FontID old;
  int j, y;

  oldw = WinGetActiveWindow();
  WinSetActiveWindow(pd->menuWin);
  WinSetDrawWindow(pd->menuWin);

  // save pulldown box
  WinSaveRectangle(pd->bitsBehind, &pd->bounds);

  mFrame = UIColorGetTableEntryIndex(UIMenuFrame);
  mFill = UIColorGetTableEntryIndex(UIMenuFill);
  mFore = UIColorGetTableEntryIndex(UIMenuForeground);
  oldf = WinSetForeColor(mFrame);
  oldb = WinSetBackColor(mFill);
  oldt = WinSetTextColor(mFore);
  old = FntSetFont(boldFont);

  RctSetRectangle(&rect, 0, 0, pd->bounds.extent.x, pd->bounds.extent.y);
  WinSetBackColor(mFrame);
  WinEraseRectangle(&rect, 0);
  RctSetRectangle(&rect, 1, 1, pd->bounds.extent.x-2, pd->bounds.extent.y-2);
  WinSetBackColor(mFill);
  WinEraseRectangle(&rect, 0);

  for (j = 0, y = 1; j < pd->numItems; j++) {
    item = &pd->items[j];
    if (item->itemStr) {
      if (item->itemStr[0] == '-') {
        WinDrawGrayLine(1, y+2, pd->bounds.extent.x-2, y+2);
        y += 4;
      } else {
        RctSetRectangle(&item->localBounds, 1, y, pd->bounds.extent.x-2, FntCharHeight());
        RctSetRectangle(&item->bounds, pd->bounds.topLeft.x+1, pd->bounds.topLeft.y+y, pd->bounds.extent.x-2, FntCharHeight());
        menu_draw_item(pd, item, false);
        y += FntCharHeight();
      }
    }
  }

  FntSetFont(old);
  WinSetForeColor(oldf);
  WinSetBackColor(oldb);
  WinSetTextColor(oldt);
  WinSetActiveWindow(oldw);
}

static void menu_hide_pd(MenuBarType *menu, MenuPullDownType *pd) {
  WinRestoreRectangle(pd->bitsBehind, &pd->bounds);
}

Boolean MenuHandleEvent(MenuBarType *menuP, EventType *event, UInt16 *error) {
  EventType ev;
  MenuPullDownType *pd, *prev;
  Boolean handled = false;
  int i, j;

  *error = 0;

  if (menuP == NULL) {
    menuP = MenuGetActiveMenu();
  }

  if (menuP) {
    switch (event->eType) {
      case penDownEvent:
        if (menuP->attr.visible) {
          debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penDown menu is visible");
          for (i = 0; i < menuP->numMenus && !handled; i++) {
            if (RctPtInRectangle(event->screenX, event->screenY, &menuP->menus[i].titleBounds)) {
              debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penDown selectedMenu %d", i);
              menuP->selectedMenu = i;
              handled = true;
            }
          }
          if (!handled) {
            pd = &menuP->menus[menuP->curMenu];
            if (!pd->hidden) {
              if (RctPtInRectangle(event->screenX, event->screenY, &pd->bounds)) {
                debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penDown inside pullDown %d", menuP->curMenu);
                for (j = 0; j < pd->numItems && !handled; j++) {
                  if (RctPtInRectangle(event->screenX, event->screenY, &pd->items[j].bounds)) {
                    debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penDown inside item %d", j);
                    pd->selectedItem = j;
                    menu_draw_item(pd, &pd->items[j], true);
                    handled = true;
                  }
                }
              }
            }
            if (!handled) {
              debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penDown hide pullDown %d", menuP->curMenu);
              if (!pd->hidden) {
                menu_hide_pd(menuP, pd);
              }
              WinSetActiveWindow(menuP->savedActiveWin);
              WinSetDrawWindow(menuP->savedActiveWin);
              WinRestoreRectangle(menuP->bitsBehind, &menuP->barWin->windowBounds);
              menuP->attr.visible = 0;
              handled = true;
            }
          }
        }
        break;
      case penUpEvent:
        if (menuP->attr.visible) {
          debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penUp menu is visible");
          if (menuP->selectedMenu != -1) {
            debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penUp selectedMenu %d", menuP->selectedMenu);
            pd = &menuP->menus[menuP->selectedMenu];
            if (RctPtInRectangle(event->screenX, event->screenY, &pd->titleBounds)) {
              debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penUp inside pullDown %d", menuP->selectedMenu);
              if (menuP->selectedMenu == menuP->curMenu) {
                debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penUp selected is current");
                if (pd->hidden) {
                  debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penUp show pullDown %d", menuP->selectedMenu);
                  menu_show_pd_title(menuP, pd, menuP->curMenu);
                  menu_show_pd(menuP, pd);
                  pd->hidden = 0;
                } else {
                  debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penUp hide pullDown %d", menuP->selectedMenu);
                  menu_show_pd_title(menuP, pd, -1);
                  menu_hide_pd(menuP, pd);
                  pd->hidden = 1;
                }
              } else {
                debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penUp outside pullDown %d", menuP->selectedMenu);
                prev = &menuP->menus[menuP->curMenu];
                menu_show_pd_title(menuP, prev, -1);
                menu_hide_pd(menuP, prev);
                prev->hidden = 1;
                menuP->curMenu = menuP->selectedMenu;
                menu_show_pd_title(menuP, pd, menuP->curMenu);
                menu_show_pd(menuP, pd);
                pd->hidden = 0;
              }
              handled = true;
            }
            menuP->selectedMenu = -1;
          }
          if (!handled) {
            pd = &menuP->menus[menuP->curMenu];
            if (pd->selectedItem != -1) {
              debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penUp current pullDown %d item %d", menuP->curMenu, pd->selectedItem);
              if (RctPtInRectangle(event->screenX, event->screenY, &pd->items[pd->selectedItem].bounds)) {
                debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penUp current pullDown %d inside item %d", menuP->curMenu, pd->selectedItem);
                menu_draw_item(pd, &pd->items[pd->selectedItem], false);
                menu_hide_pd(menuP, pd);
                pd->hidden = 0;
                WinSetActiveWindow(menuP->savedActiveWin);
                WinSetDrawWindow(menuP->savedActiveWin);
                WinRestoreRectangle(menuP->bitsBehind, &menuP->barWin->windowBounds);
                menuP->attr.visible = 0;

                debug(DEBUG_TRACE, "Menu", "MenuHandleEvent menuEvent item %d", pd->items[pd->selectedItem].id);
                MemSet(&ev, sizeof(EventType), 0);
                ev.eType = menuEvent;
                ev.data.menu.itemID = pd->items[pd->selectedItem].id;
                EvtAddEventToQueue(&ev);
                handled = true;
              } else {
                debug(DEBUG_TRACE, "Menu", "MenuHandleEvent penUp current pullDown %d outside item %d", menuP->curMenu, pd->selectedItem);
              }
              pd->selectedItem = -1;
            }
          }
        }
        break;
      case keyDownEvent:
        if (event->data.keyDown.chr == vchrMenu) {
          debug(DEBUG_TRACE, "Menu", "MenuHandleEvent keyDown vchrMenu");
          if (menuP->attr.visible) {
            pd = &menuP->menus[menuP->curMenu];
            if (!pd->hidden) {
              debug(DEBUG_TRACE, "Menu", "MenuHandleEvent keyDown hide pullDown %d", menuP->curMenu);
              menu_hide_pd(menuP, pd);
            }
            debug(DEBUG_TRACE, "Menu", "MenuHandleEvent keyDown hide menu");
            WinSetActiveWindow(menuP->savedActiveWin);
            WinSetDrawWindow(menuP->savedActiveWin);
            WinRestoreRectangle(menuP->bitsBehind, &menuP->barWin->windowBounds);
            menuP->attr.visible = 0;
          } else {
            debug(DEBUG_TRACE, "Menu", "MenuHandleEvent keyDown show menu");
            MenuDrawMenu(menuP);
            menuP->attr.visible = 1;
          }
        }
        break;
      default:
        break;
    }
  }

  return handled;
}

void MenuDrawMenu(MenuBarType *menuP) {
  RectangleType rect;
  MenuPullDownType *pd;
  IndexedColorType mFrame, mFill, oldb;
  int i;

  if (!menuP) return;

  debug(DEBUG_TRACE, "Menu", "MenuDrawMenu");
  menuP->savedActiveWin = WinGetActiveWindow();

  // save menu box
  WinSaveRectangle(menuP->bitsBehind, &menuP->barWin->windowBounds);

  mFrame = UIColorGetTableEntryIndex(UIMenuFrame);
  mFill = UIColorGetTableEntryIndex(UIMenuFill);
  oldb = WinSetBackColor(mFill);

  WinSetDrawWindow(menuP->barWin);
  WinSetActiveWindow(menuP->barWin);

  WinSetBackColor(mFrame);
  WinEraseRectangle(&menuP->barWin->windowBounds, 0);

  WinSetBackColor(mFill);
  RctSetRectangle(&rect, 1, 1, menuP->barWin->windowBounds.extent.x-2, menuP->barWin->windowBounds.extent.y-2);
  WinEraseRectangle(&rect, 0);

  for (i = 0; i < menuP->numMenus; i++) {
    pd = &menuP->menus[i];
    menu_show_pd_title(menuP, pd, i);
  }

  for (i = 0; i < menuP->numMenus; i++) {
    pd = &menuP->menus[i];
    if (!pd->hidden) {
      menu_show_pd(menuP, pd);
    }
  }

  WinSetBackColor(oldb);
  WinSetActiveWindow(menuP->barWin);
}

void MenuEraseStatus(MenuBarType *menuP) {
  debug(DEBUG_ERROR, "Menu", "MenuEraseStatus not implemented");
}

void MenuSetActiveMenuRscID(UInt16 resourceId) {
  debug(DEBUG_ERROR, "Menu", "MenuSetActiveMenuRscID not implemented");
}

void MenuCmdBarDisplay(void) {
  debug(DEBUG_ERROR, "Menu", "MenuCmdBarDisplay not implemented");
}

Boolean MenuShowItem(UInt16 id) {
  debug(DEBUG_ERROR, "Menu", "MenuShowItem not implemented");
  return 0;
}

Boolean MenuHideItem(UInt16 id) {
  debug(DEBUG_ERROR, "Menu", "MenuHideItem not implemented");
  return 0;
}

Err MenuAddItem(UInt16 positionId, UInt16 id, Char cmd, const Char *textP) {
  debug(DEBUG_ERROR, "Menu", "MenuAddItem not implemented");
  return 0;
}

Err MenuCmdBarAddButton(UInt8 where, UInt16 bitmapId, MenuCmdBarResultType resultType, UInt32 result, Char *nameP) {
  debug(DEBUG_ERROR, "Menu", "MenuCmdBarAddButton not implemented");
  return 0;
}

Boolean MenuCmdBarGetButtonData(Int16 buttonIndex, UInt16 *bitmapIdP, MenuCmdBarResultType *resultTypeP, UInt32 *resultP, Char *nameP) {
  debug(DEBUG_ERROR, "Menu", "MenuCmdBarGetButtonData not implemented");
  return 0;
}

MenuBarType *pumpkin_create_menu(void *h, uint8_t *p, uint32_t *dsize) {
  MenuBarType *mbar;
  UInt32 dummy32;
  UInt16 totalItems, numStrs, curMenu, curItem, numMenus, numItems, itemId;
  UInt16 pullDownX, pullDownY, pullDownW, pullDownH, titleX, titleY, titleW, titleH;
  UInt8 itemCmd, itemFlags;
  char *menuText;
  uint16_t attr;
  int i, j, k, l;

  if ((mbar = StoNewDecodedResource(h, sizeof(MenuBarType), 0, 0)) != NULL) {
    totalItems = 0;
    numStrs = 0;

    // szRCMenuBarBA16 "zl,zl,zl,zl,uzu3zu12,zw,w,zl,w,zl"
    i = 0;
    i = get4b(&dummy32, p, i);
    i += get4b(&dummy32, p, i);
    i += get4b(&dummy32, p, i);
    i += get4b(&dummy32, p, i);
    i += get2b(&attr, p, i);
    i += get2b(&curMenu, p, i);
    i += get2b(&curItem, p, i);
    i += get4b(&dummy32, p, i);
    i += get2b(&numMenus, p, i);
    i += get4b(&dummy32, p, i);
    debug(DEBUG_TRACE, "Form", "menu pulldowns %d", numMenus);

    mbar->attr.visible                = (attr & 0x8000) ? 1 : 0;
    mbar->attr.commandPending         = (attr & 0x4000) ? 1 : 0;
    mbar->attr.insPtEnabled           = (attr & 0x2000) ? 1 : 0;
    mbar->attr.needsRecalc            = (attr & 0x1000) ? 1 : 0;
    mbar->attr.attnIndicatorIsAllowed = (attr & 0x0800) ? 1 : 0;
    mbar->attr.reserved               = 0;
    mbar->curMenu = curMenu;
    mbar->curItem = curItem;
    mbar->numMenus = numMenus;
    mbar->menus = pumpkin_heap_alloc(numMenus * sizeof(MenuPullDownType), "Menu");

    for (j = 0; j < numMenus; j++) {
      // szRCMenuPullDownBA16 "zl,w4,zl,w4,l,uu15,l"
      i += get4b(&dummy32, p, i);
      i += get2b(&pullDownX, p, i);
      i += get2b(&pullDownY, p, i);
      i += get2b(&pullDownW, p, i);
      i += get2b(&pullDownH, p, i);
      i += get4b(&dummy32, p, i);
      i += get2b(&titleX, p, i);
      i += get2b(&titleY, p, i);
      i += get2b(&titleW, p, i);
      i += get2b(&titleH, p, i);
      i += get4b(&dummy32, p, i);
      i += get2b(&numItems, p, i);
      i += get4b(&dummy32, p, i);
      totalItems += numItems;
      numStrs++;
      debug(DEBUG_TRACE, "Form", "pulldown %d bounds (%d,%d,%d,%d), title (%d,%d,%d,%d), items %d",
        j, pullDownX, pullDownY, pullDownW, pullDownH,  titleX, titleY, titleW, titleH, numItems);

      mbar->menus[j].bounds.topLeft.x = pullDownX;
      mbar->menus[j].bounds.topLeft.y = pullDownY;
      mbar->menus[j].bounds.extent.x = pullDownW;
      mbar->menus[j].bounds.extent.y = pullDownH+2;
      mbar->menus[j].titleBounds.topLeft.x = titleX;
      mbar->menus[j].titleBounds.topLeft.y = titleY;
      mbar->menus[j].titleBounds.extent.x = titleW;
      mbar->menus[j].titleBounds.extent.y = titleH;
      mbar->menus[j].numItems = numItems;
      mbar->menus[j].items = pumpkin_heap_alloc(numItems * sizeof(MenuItemType), "MenuItem");
    }

    for (j = 0; j < numMenus; j++) {
      for (k = 0; k < mbar->menus[j].numItems; k++) {
        // szRCMENUITEM "w,b,tzt7,l"
        i += get2b(&itemId, p, i);
        i += get1(&itemCmd, p, i);
        i += get1(&itemFlags, p, i);
        i += get4b(&dummy32, p, i);
        numStrs++;
        debug(DEBUG_TRACE, "Form", "item %d.%d, id %d, cmd %d, flags 0x%04x", j, k, itemId, itemCmd, itemFlags);
        mbar->menus[j].items[k].id = itemId;
        mbar->menus[j].items[k].command = itemCmd;
        // XXX set itemFlags
      }
    }

    for (j = 0; j < numMenus; j++) {
      i += pumpkin_getstr(&menuText, p, i);
      debug(DEBUG_TRACE, "Form", "pulldown %d text \"%s\"", j, menuText);
      mbar->menus[j].title = menuText;

      for (k = 0; k < mbar->menus[j].numItems; k++) {
        i += pumpkin_getstr(&menuText, p, i);
        for (l = StrLen(menuText)-1; l > 0 && menuText[l] == ' '; l--) {
          menuText[l] = 0;
        }
        debug(DEBUG_TRACE, "Form", "item %d.%d text \"%s\"", j, k, menuText);
        mbar->menus[j].items[k].itemStr = menuText;
      }
    }
  }

  *dsize = sizeof(MenuBarType);
  return mbar;
}

void pumpkin_destroy_menu(void *p) {
  MenuBarType* mbar;
  int j, k;

  mbar = (MenuBarType *)p;
  if (mbar) {
    if (mbar->menus) {
      for (j = 0; j < mbar->numMenus; j++) {
        //if (mbar->menus[j].title) xfree(mbar->menus[j].title); // XXX
        if (mbar->menus[j].items) {
          for (k = 0; k < mbar->menus[j].numItems; k++) {
            //if (mbar->menus[j].items[k].itemStr) xfree(mbar->menus[j].items[k].itemStr); // XXX
          }
          pumpkin_heap_free(mbar->menus[j].items, "MenuItem");
        }
      }
      pumpkin_heap_free(mbar->menus, "Menu");
    }
    MemChunkFree(mbar);
  }
}
