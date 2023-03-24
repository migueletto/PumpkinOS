#include <PalmOS.h>

#include "gps.h"
#include "app.h"
#include "gui.h"
#include "main.h"
#include "scroll.h"
#include "misc.h"
#include "network.h"
#include "http.h"
#include "error.h"

static char inbuf[256];
static char outbuf[256];
static Boolean quiet;

static Boolean PrivNetworkFormHandleEvent(EventPtr event);
static void NetReceive(char *buf, UInt16 n);

static Boolean PrivNetworkFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  FieldPtr fld;
  char *s;
  Boolean handled;
  static AppPrefs *prefs;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      prefs = LoadPrefs();

      frm = FrmGetActiveForm();
      FldInsertStr(frm, hostFld, prefs->nethost);
      FldInsertStr(frm, urlFld, prefs->neturl);
      StrPrintF(inbuf, "%d", prefs->netport);
      FldInsertStr(frm, portFld, inbuf);

      FrmSetFocus(frm, FrmGetObjectIndex(frm, hostFld));

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
          frm = FrmGetActiveForm();

          if (!GetUInt(frm, portFld, &prefs->netport, 0, 65535)) {
            InfoDialog(INFO, "Invalid port number");
            break;
          }

          fld = (FieldPtr)FrmGetObjectPtr(frm,
                    FrmGetObjectIndex(frm, hostFld));
          if (fld && (s = FldGetTextPtr(fld)) != NULL && s[0])
            StrNCopy(prefs->nethost, s, TAM_HOST-1);

          fld = (FieldPtr)FrmGetObjectPtr(frm,
                    FrmGetObjectIndex(frm, urlFld));
          if (fld && (s = FldGetTextPtr(fld)) != NULL && s[0])
            StrNCopy(prefs->neturl, s, TAM_URL-1);

          SavePrefs();
        case cancelBtn:
          PopForm();
          handled = true;
	  break;
      }
      break;

    case fldChangedEvent:
      if (event->data.fldChanged.fieldID == urlFld) {
        UpdateScrollBar(event->data.fldChanged.pField,
           FrmGetActiveForm(), urlScl);
        handled = true;
      }
      break;

    case sclRepeatEvent:
      ScrollField(FrmGetActiveForm(), urlFld, urlScl,
                  event->data.sclRepeat.newValue -
                  event->data.sclRepeat.value, false);
      break;

    default:
      break;
  }

  return handled;
}

Boolean NetworkFormHandleEvent(EventPtr event)
{
  return PrivNetworkFormHandleEvent(event);
}

static void NetReceive(char *buf, UInt16 n)
{
  buf[n] = 0;
  StrNCat(outbuf, buf, sizeof(outbuf)-StrLen(outbuf)-1);
}

Int16 NetSendPoint(WaypointType *p, Boolean _quiet)
{
  Err err;
  UInt32 t;
  AppPrefs *prefs = GetPrefs();

  quiet = _quiet;

  if (!prefs->nethost[0] || !prefs->netport || !prefs->neturl[0]) {
    if (!quiet)
      InfoDialog(ERROR, "Network options are not configured");
    return -1;
  }

  t = SysTicksPerSecond();

  if (http_init(5*t, 5*t, 5*t) != HTTP_OK) {
    if (!quiet)
      InfoDialog(ERROR, "Network init failed");
    return -1;
  }

  MemSet(inbuf, sizeof(inbuf), 0);
  MemSet(outbuf, sizeof(outbuf), 0);
  ExpandText(p, prefs->neturl, inbuf, sizeof(inbuf)-1);
  err = http_get(prefs->nethost, prefs->netport, inbuf, NetReceive);

  if (!quiet) {
    if (err == HTTP_OK)
      InfoDialog(INFO, outbuf[0] ? "Point sent: %s" : "Point sent", outbuf);
    else
      InfoDialog(INFO, "Error sending point: %s",
        outbuf[0] ? outbuf : http_error(err));
  }

  http_finish();

  return 0;
}
