#include <PalmOS.h>

#include "resource.h"
#include "debug.h"

static const UInt16 fonts[] = { 11903, 11904, 11905, 11908, 11909, 11910, 11911, 0 };

typedef struct {
  UInt16 font;
} widgets_data_t;

void *WidgetInit(void) {
  widgets_data_t *widgetData;

  debug(DEBUG_INFO, "Widgets", "WidgetInit");
  widgetData = MemPtrNew(sizeof(widgets_data_t));
  widgetData->font = 0;

  return widgetData;
}

void WidgetFinish(void *data) {
  debug(DEBUG_INFO, "Widgets", "WidgetFinish");
  if (data) MemPtrFree(data);
}

void WidgetDialogPre(UInt16 formId, UInt16 panel, FormType *frm, void *data) {
  widgets_data_t *widgetData = data;
  ControlType *ctl;
  UInt16 index;

  debug(DEBUG_INFO, "Widgets", "WidgetDialogPre formId %d, panel %d", formId, panel);

  switch (formId) {
    case SelectFontForm:
      index = FrmGetObjectIndex(frm, fonts[widgetData->font]);
      ctl = FrmGetObjectPtr(frm, index);
      CtlSetValue(ctl, 1);
      break;
  }
}

Boolean WidgetDialogEventHandler(UInt16 formId, UInt16 panel, FormType *frm, EventType *event, void *data) {
  debug(DEBUG_TRACE, "Widgets", "WidgetDialogEventHandler formId %d, panel %d, event 0x%04X (%s)",
    formId, panel, event->eType, EvtGetEventName(event->eType));

  return false;
}

void WidgetDialogPost(UInt16 formId, UInt16 panel, FormType *frm, UInt16 button, void *data) {
  widgets_data_t *widgetData = data;
  ControlType *ctl;
  UInt16 index, i;

  debug(DEBUG_INFO, "Widgets", "WidgetDialogPost formId %d, panel %d, button %d", formId, panel, button);

  switch (formId) {
    case SelectFontForm:
      switch (button) {
        case 11906:
          debug(DEBUG_INFO, "Widgets", "user pressed OK");
          for (i = 0; fonts[i]; i++) {
            index = FrmGetObjectIndex(frm, fonts[i]);
            ctl = FrmGetObjectPtr(frm, index);
            if (CtlGetValue(ctl)) {
              debug(DEBUG_INFO, "Widgets", "font %d selected", i);
              widgetData->font = i;
              break;
            }
          }
          break;

        case 11907:
          debug(DEBUG_INFO, "Widgets", "user pressed Cancel");
          break;
      }
      break;
  }
}
