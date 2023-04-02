#include <stdint.h>

typedef uint32_t UInt32;
typedef uint16_t UInt16;
typedef uint8_t UInt8;
typedef int32_t Int32;
typedef int16_t Int16;
typedef int8_t Int8;
typedef uint8_t Boolean;

typedef char Char;
typedef UInt16 WChar;
typedef UInt16 Err;
typedef Int16 Coord;
typedef UInt16 DmResID;

UInt32 pumpkin_get_app_creator(void);

uint8_t *pumpkin_heap_alloc(uint32_t size, const char *tag);
void pumpkin_heap_free(void *p, const char *tag);

void debug_full(const char *file, const char *func, int line, int level, const char *sys, const char *fmt, ...);
void pumpkin_puts(const char *s);

void FrmCenterDialogs(Boolean center);
void FrmSetUsable(void *formP, UInt16 objIndex, Boolean usable);
void FrmGotoForm(UInt16 formId);
void *FrmInitForm(UInt16 formId);
void FrmSetActiveForm(void *formP);
void FrmCloseAllForms(void);
UInt16 FrmGetActiveFormID(void);
void *FrmGetActiveForm(void);
void FrmSetEventHandler(void *formP, void *handler);
Boolean FrmDispatchEvent(void *event);
void FrmDrawForm(void *formP);
UInt16 FrmAlert(UInt16 alertId);
UInt16 FrmCustomAlert(UInt16 alertId, const char *s1, const char *s2, const char *s3);
UInt16 FrmDoDialog(void *formP);
void FrmPopupForm(UInt16 formId);
void FrmReturnToForm(UInt16 formId);
void FrmUpdateForm(UInt16 formId, UInt16 updateCode);
void FrmHelp(UInt16 helpMsgId);
const Char *FrmGetLabel(void *formP, UInt16 labelID);
void FrmCopyLabel(void *formP, UInt16 labelID, const Char *newLabel);
void FrmSetControlValue(void *formP, UInt16 objIndex, Int16 newValue);
void FrmSetControlGroupSelection(void *formP, UInt8 groupNum, UInt16 controlID);
UInt16 FrmGetControlGroupSelection(void *formP, UInt8 groupNum);
UInt16 FrmGetObjectIndex(void *formP, UInt16 objID);
void *FrmGetObjectPtr(void *formP, UInt16 objIndex);
UInt8 FrmGetObjectType(void *formP, UInt16 objIndex);

void CtlShowControl(void *controlP);
void CtlHideControl(void *controlP);
Int16 CtlGetValue(void *controlP);
void CtlSetValue(void *controlP, Int16 newValue);
void CtlSetSliderValues(void *controlP, const UInt16 *minValueP, const UInt16 *maxValueP, const UInt16 *pageSizeP, const UInt16 *valueP);
const Char *CtlGetLabel(void *controlP);
void CtlSetLabel(void *controlP, const Char *newLabel);
void CtlUpdateGroup(void *controlP, Boolean value);
void CtlSetGraphics(void *ctlP, DmResID newBitmapID, DmResID newSelectedBitmapID);

void FldSetText(void *fldP, void *textHandle, UInt16 offset, UInt16 size);
Boolean FldInsert(void *fldP, const Char *insertChars, UInt16 insertLen);
void FldDelete(void *fldP, UInt16 start, UInt16 end);

void EvtGetEvent(void *event, Int32 timeout);
void EvtAddEventToQueue(void *event);
Boolean EvtEventAvail(void);
void EvtCopyEvent(void *source, void *dest);

Boolean SysHandleEvent(void *event);
Boolean MenuHandleEvent(void *menuP, void *event, UInt16 *error);
void AbtShowAboutPumpkin(UInt32 creator);
