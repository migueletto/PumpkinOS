#include <PalmOS.h>

#include "gps.h"
#include "gui.h"
#include "app.h"
#include "main.h"
#include "ddb.h"
#include "error.h"

static void SetButtonValue(FormPtr frm, AppPrefs *prefs, Int16 i)
{
  ListPtr lst;

  lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, action1List+i));
  LstSetSelection(lst, prefs->action[i]);
  CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm,
    action1Ctl+i)), LstGetSelectionText(lst, prefs->action[i]));
}

Boolean ButtonsFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  UInt16 id;
  Boolean handled;
  static AppPrefs *prefs;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      prefs = LoadPrefs();
      frm = FrmGetActiveForm();

      SetButtonValue(frm, prefs, 0);
      SetButtonValue(frm, prefs, 1);
      SetButtonValue(frm, prefs, 2);
      SetButtonValue(frm, prefs, 3);

      FrmDrawForm(frm);
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case okBtn:
          handled = true;
          SavePrefs();
        case cancelBtn:
          PopForm();
          handled = true;
      }
      break;

    case popSelectEvent:
      switch (id = event->data.popSelect.listID) {
        case action1List:
        case action2List:
        case action3List:
        case action4List:
          prefs->action[id - action1List] = event->data.popSelect.selection;
      }
      break;

    default:
      break;
  }

  return handled;
}

/*
Err ButtonsInit(void)
{
  DmSearchStateType stateInfo;
  UInt16 cardNo;
  LocalID dbID;
  DmOpenRef dbRef;
  DmResType type;
  DmResID id;
  MemHandle h;
  BitmapPtr bmp;
  UInt16 i, index;
  Err err;

  if (DmGetNextDatabaseByTypeCreator(true, &stateInfo, sysFileTPanel,
        sysFileCButtons, true, &cardNo, &dbID) != 0)
    return -1;

  if ((dbRef = DbOpen(dbID, dmModeReadOnly, &err)) == NULL)
    return err;

  for (i = 0; ; i++) {
    if ((index =  DmFindResourceType(dbRef, bitmapRsc, i)) == 0xFFFF)
      break;

    if ((err = DmResourceInfo(dbRef, index, &type, &id, NULL)) != 0)
      break;

    if ((h = DmGet1Resource(type, id)) == NULL)
      break;

    if ((bmp = (BitmapPtr)MemHandleLock(h)) == NULL) {
      DmReleaseResource(h);
      break;
    }

    MemHandleUnlock(h);
    DmReleaseResource(h);
  }

  DbClose(dbRef);

  return 0;
}
*/
