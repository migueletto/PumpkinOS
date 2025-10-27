#include <PalmOS.h>
  
#include "sys.h"
#include "pwindow.h"
#include "vfs.h"
#include "bytes.h"
#include "logtrap.h"
#include "emupalmosinc.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

/*
This function displays a dialog that allows users to change the
preference prefShowPrivateRecords, which controls how
private records are displayed.
When the user taps the OK button in this dialog, SecVerifyPW is
called to see if the user changed the preference setting and, if so, to
prompt the user to enter the appropriate password.
After calling this function, your code should check the return value
or the value of prefShowPrivateRecords and mask, display, or
hide the private records accordingly. See the description of
TblSetRowMasked for a partial example.
*/

// Preferences:
// prefShowPrivateRecords: returns privateRecordViewEnum

privateRecordViewEnum SecSelectViewStatus(void) {
  FormType *frm, *previous;
  ListType *lst;
  ControlType *ctl;
  UInt16 index, sel;
  uint32_t *aux32, w;
  char *s;
  privateRecordViewEnum secLevel;

  if ((frm = FrmInitForm(13200)) != NULL) {
    previous = FrmGetActiveForm();

    index = FrmGetObjectIndex(frm, 13202);
    ctl = (ControlType *)FrmGetObjectPtr(frm, index);
    index = FrmGetObjectIndex(frm, 13203);
    lst = (ListType *)FrmGetObjectPtr(frm, index);

    if (lst != NULL && ctl != NULL) {
      secLevel = PrefGetPreference(prefShowPrivateRecords);
      switch (secLevel) {
        case showPrivateRecords: sel = 0; break;
        case maskPrivateRecords: sel = 1; break;
        case hidePrivateRecords: sel = 2; break;
        default: sel = 0;
      }
      LstSetSelection(lst, sel);

      if (pumpkin_is_m68k()) {                                                                                                                                                            
        aux32 = (uint32_t *)lst->itemsText;                                                                                                                                               
        if (aux32[sel]) {                                                                                                                                                                     
          get4b(&w, (uint8_t *)(&aux32[sel]), 0);                                                                                                                                             
          s = (char *)(emupalmos_ram() + w);                                                                                                                                                
          CtlSetLabel(ctl, s);
        }                                                                                                                                                                                   
      } else {                                                                                                                                                                              
        if (lst->itemsText[sel]) {                                                                                                                                                          
          CtlSetLabel(ctl, lst->itemsText[sel]);
        }                                                                                                                                                                                   
      }                                                                                                                                                                                     


      if (FrmDoDialog(frm) == 13204) { // "OK" button
        sel = LstGetSelection(lst);
        switch (sel) {
          case 0: secLevel = showPrivateRecords; break;
          case 1: secLevel = maskPrivateRecords; break;
          case 2: secLevel = hidePrivateRecords; break;
          default: secLevel = PrefGetPreference(prefShowPrivateRecords);
        }
        SecVerifyPW(secLevel);
      }
    }

    FrmDeleteForm(frm);
    FrmSetActiveForm(previous);
  }

  return PrefGetPreference(prefShowPrivateRecords);
}

Boolean SecVerifyPW(privateRecordViewEnum newSecLevel) {
  privateRecordViewEnum currentSecLevel;
  Boolean ok, r = false;

  debug(DEBUG_ERROR, "PALMOS", "SecVerifyPW not implemented");
  currentSecLevel = PrefGetPreference(prefShowPrivateRecords);

  if (newSecLevel != currentSecLevel) {
    switch (newSecLevel) {
      case maskPrivateRecords:
        // XXX Show "mask" alert message and if OK is clicked continue
        ok = true;
        break;
      case hidePrivateRecords:
        // XXX Show "hide" alert message and if OK is clicked continue
        ok = true;
        break;
      default:
        ok = true;
        break;
    }

    if (ok) {
      if (newSecLevel < currentSecLevel) {
        switch (newSecLevel) {
          case showPrivateRecords:
            // XXX display "show" password dialog
            ok = true;
            break;
          case maskPrivateRecords:
            // XXX display "mask" password dialog
            ok = true;
            break;
          default:
            ok = true;
            break;
        }

        if (ok) {
          PrefSetPreference(prefShowPrivateRecords, newSecLevel);
          r = true;
        }
      } else {
        PrefSetPreference(prefShowPrivateRecords, newSecLevel);
        r = true;
      }
    }
  }

  return r;
}

