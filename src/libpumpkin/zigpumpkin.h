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

typedef struct WindowType WindowType;
typedef WindowType *WinHandle;

typedef struct PointType {
  Coord x;
  Coord y;
} PointType;

typedef struct RectangleType {
  PointType  topLeft;
  PointType  extent;
} RectangleType;

void debug_full(const char *file, const char *func, int line, int level, const char *sys, const char *fmt, ...);
void pumpkin_puts(const char *s);
UInt16 FrmCustomAlert(UInt16 alertId, const char *s1, const char *s2, const char *s3);

void EvtGetEvent(void *event, Int32 timeout);
Boolean SysHandleEvent(void *event);
Boolean MenuHandleEvent(void *menuP, void *event, UInt16 *error);
Boolean FrmDispatchEvent(void *event);
void FrmGotoForm(UInt16 formId);
void *FrmInitForm(UInt16 formId);
void FrmSetActiveForm(void *formP);
void FrmCloseAllForms(void);
void FrmSetEventHandler(void *formP, void *handler);
void *FrmGetActiveForm(void);
void FrmDrawForm(void *formP);
void AbtShowAboutPumpkin(UInt32 creator);
