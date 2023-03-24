#include <PalmOS.h>

#include "gui.h"
#include "gps.h"
#include "misc.h"
#include "protocol.h"

static Int16 app_protocol[4], data_protocol[4][3];
static Int16 ok = 0;

void InitProtocol(Int16 _ok) {
  ok = _ok;
}

void SetProtocol(int type, int app, int *data) {
  Int16 i;

  app_protocol[type] = app;
  for (i = 0; i < 3; i++)
    data_protocol[type][i] = (data && data[i] > 0) ? data[i] : 0;

  ok = 1;
}

void ShowProtocol(void) {
  FormPtr frm;
  Int16 p, i;
  char aux[32];
  static char buf[256];

  if (ok) {
    StrPrintF(buf, "L%03d\n", app_protocol[LINK_PROTOCOL]);

    for (p = POINT_PROTOCOL; p <= TRACK_PROTOCOL; p++) {
      StrPrintF(aux, "A%03d:", app_protocol[p]);
      StrCat(buf, aux);

      for (i = 0; i < 3 && data_protocol[p][i]; i++) {
        StrPrintF(aux, " D%03d", data_protocol[p][i]);
        StrCat(buf, aux);
      }
      StrCat(buf, "\n");
    }
  } else
    StrCopy(buf, "Not set");

  frm = FrmInitForm(ProtocolForm);
  FldInsertStr(frm, detailFld, buf);
  FrmDoDialog(frm);
  FrmDeleteForm(frm);
}
