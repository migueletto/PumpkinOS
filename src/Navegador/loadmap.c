#include <PalmOS.h>

#include "gps.h"
#include "app.h"
#include "main.h"
#include "map.h"
#include "mapdecl.h"
#include "misc.h"
#include "list.h"
#include "gui.h"
#include "file.h"

Boolean LoadMapFormHandleEvent(EventPtr event) {
  FormPtr frm;
  ListPtr lst;
  Boolean handled;
  static AppPrefs *prefs;
  static FileType maps;
  static char **fname;
  static UInt16 index;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      prefs = LoadPrefs();
      CreateFileList(AppID, MapFType, &maps, NULL);

      FrmSetControlValue(frm, FrmGetObjectIndex(frm, showCtl),
        prefs->showtrack);

      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, fontList));
      LstSetSelection(lst, prefs->mapfont);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
         FrmGetObjectIndex(frm, fontCtl)),
         LstGetSelectionText(lst, prefs->mapfont));

      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, detailList));
      LstSetSelection(lst, prefs->mapdetail);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
         FrmGetObjectIndex(frm, detailCtl)),
         LstGetSelectionText(lst, prefs->mapdetail));

      index = 0;
      if (maps.n) {
        if (prefs->mapname[0]) {
          for (index = 0; index < maps.n; index++)
            if (!StrCompare(maps.fname[index], prefs->mapname))
              break;
          if (index == maps.n)
            index = 0;
          else
            index++;
        }
        lst = (ListPtr)FrmGetObjectPtr(frm,
           FrmGetObjectIndex(frm, mapnameList));
        LstSetHeight(lst, maps.n+1);
        fname = MemPtrNew((maps.n+1)*sizeof(char *));
        fname[0] = "None";
        if (maps.n)
          MemMove(&fname[1], maps.fname, maps.n*sizeof(char *));
        LstSetListChoices(lst, fname, maps.n+1);
        LstSetSelection(lst, index);
        CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
           FrmGetObjectIndex(frm, mapnameCtl)), LstGetSelectionText(lst,index));
      } else
        fname = NULL;
      FrmDrawForm(frm);
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case showCtl:
          prefs->showtrack = !prefs->showtrack;
          handled = true;
          break;

        case okBtn:
          if (index == 0)
            prefs->mapname[0] = 0;
          else {
            frm = FrmGetActiveForm();
            lst = (ListPtr)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, mapnameList));
            StrNCopy(prefs->mapname, LstGetSelectionText(lst, index),
                     dmDBNameLength);
          }
          SavePrefs();
          // fall-through

        case cancelBtn:
          DestroyFileList(&maps);
          if (fname)
            MemPtrFree(fname);
          PopForm();

          MapFindReset();
          handled = true;
      }
      break;

    case popSelectEvent:
      switch (event->data.popSelect.listID) {
        case mapnameList:
          index = event->data.popSelect.selection;
          break;
        case fontList:
          prefs->mapfont = event->data.popSelect.selection;
          break;
        case detailList:
          prefs->mapdetail = event->data.popSelect.selection;
      }
      handled = false;
      break;

    default:
      break;
  }

  return handled;
}
