#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "mem.h"
#include "pumpkin.h"
#include "xalloc.h"
#include "debug.h"

#define MAX_MSG 1024

typedef struct {
  FormType *formP;
  FieldType *fldP;
} fatal_module_t;

extern thread_key_t *fatal_key;

void SysFatalAlertInit(void) {
/*
  fatal_module_t *module;
  FieldAttrType attr;
  UInt16 index;

  if ((module = xcalloc(1, sizeof(fatal_module_t))) != NULL) {
    if ((module->formP = FrmInitForm(10400)) != NULL) {
      FrmSetTitle(module->formP, "Fatal Exception");
      index = FrmGetObjectIndex(module->formP, 10404);
      FrmHideObject(module->formP, index);
      index = FrmGetObjectIndex(module->formP, 10405);
      FrmHideObject(module->formP, index);

      index = FrmGetObjectIndex(module->formP, 10402);
      module->fldP = FrmGetObjectPtr(module->formP, index);
      if (module->fldP != NULL) {
        FldGetAttributes(module->fldP, &attr);
        attr.editable = false;
        FldSetAttributes(module->fldP, &attr);
        FldSetFont(module->fldP, boldFont);
      }
    }
    thread_set(fatal_key, module);
  }
*/
}

void SysFatalAlertFinish(void) {
/*
  fatal_module_t *module = (fatal_module_t *)thread_get(fatal_key);

  if (module) {
    thread_set(fatal_key, NULL);
    xfree(module);
  }
*/
}

/*
UInt16 SysFatalAlert(const Char *msg) {
  fatal_module_t *module = (fatal_module_t *)thread_get(fatal_key);

  debug(DEBUG_ERROR, "Fatal", "%s", msg);

  if (module && module->formP && module->fldP && msg) {
    FldSetTextPtr(module->fldP, (char *)msg);
    FrmDoDialog(module->formP);
  }

  return 0;
}
*/

UInt16 SysFatalAlert(const Char *msg) {
  FormType *formP, *previous;
  FieldType *fldP;
  FieldAttrType attr;
  UInt16 index;

  debug(DEBUG_ERROR, "System", "SysFatalAlert: %s", msg);
  WinSetCoordinateSystem(kCoordinatesStandard);

  if ((formP = FrmInitForm(10400)) != NULL) {
    FrmSetTitle(formP, "Fatal Exception");
    index = FrmGetObjectIndex(formP, 10404);
    FrmHideObject(formP, index);
    index = FrmGetObjectIndex(formP, 10405);
    FrmHideObject(formP, index);

    index = FrmGetObjectIndex(formP, 10402);
    fldP = FrmGetObjectPtr(formP, index);

    if (fldP != NULL) {
      FldGetAttributes(fldP, &attr);
      attr.editable = false;
      FldSetAttributes(fldP, &attr);
      FldSetFont(fldP, boldFont);
      FldSetTextPtr(fldP, (char *)msg);
    } else {
      debug(DEBUG_ERROR, "System", "field not found on form");
    }
    previous = FrmGetActiveForm();

    FrmDoDialog(formP);
    FrmSetTitle(formP, NULL);
    if (fldP != NULL) {
      FldSetTextPtr(fldP, NULL);
    }

    FrmDeleteForm(formP);
    FrmSetActiveForm(previous);
  }

  return 0;
}
