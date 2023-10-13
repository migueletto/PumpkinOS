#include "script.h"
#include "pit_io.h"
#include "pwindow.h"
#include "audio.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "gps.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BOOT_CREATOR  'BOOT'
#define BOOT_NAME     "BOOT"

#define APP_SCREEN_WIDTH  320
#define APP_SCREEN_HEIGHT 320

#define MAX_SERIAL  8
#define MAX_SYSLIBS 32

#define MISSING_SYMBOL_WIDTH 5  // XXX

#define NetLibRefNum (MAX_SYSLIBS+1)
#define NetLibName   "Net.lib"

#define GPDLibRefNum (MAX_SYSLIBS+2)
#define GPDLibName   GPD_LIB_NAME

#define GPSLibRefNum (MAX_SYSLIBS+3)

#define BUTTONS_HEIGHT 64

#define MSG_KEY     1
#define MSG_BUTTON  2
#define MSG_MOTION  3
#define MSG_LAUNCH  4
#define MSG_PAUSE   5
#define MSG_RESUME  6
#define MSG_DISPLAY 7
#define MSG_RAISE   8
#define MSG_DEPLOY  9
#define MSG_KEYDOWN 10
#define MSG_KEYUP   11

#define oemErrNotImplemented (oemErrorClass | 0x7EFF)

#ifndef IN
#define IN        debug(DEBUG_TRACE, PALMOS_MODULE, "%s begin", __FUNCTION__)
#define OUTV      debug(DEBUG_TRACE, PALMOS_MODULE, "%s end", __FUNCTION__)
#define OUTE(err) debug(err ? DEBUG_ERROR : DEBUG_TRACE, PALMOS_MODULE, "%s end err=0x%X", __FUNCTION__, err); PALMOS_MODERR(err)
#define OUT(err)  debug(err ? DEBUG_ERROR : DEBUG_TRACE, PALMOS_MODULE, "%s end err=0x%X", __FUNCTION__, err); return err
#define NOTIMPLV  debug(DEBUG_ERROR, PALMOS_MODULE, "%s not implemented", __FUNCTION__); PALMOS_MODERR(oemErrNotImplemented)
#define NOTIMPLP  NOTIMPLV; return NULL
#define NOTIMPLI  NOTIMPLV; return 0
#define NOTIMPL   NOTIMPLV; return oemErrNotImplemented
#endif

#define GRAFFITI_CAPS   13
#define GRAFFITI_NUMBER 14
#define GRAFFITI_SHIFT  15
#define GRAFFITI_PUNCT  16
#define GRAFFITI_SYMBOL 17

#define font6x10Id  9010
#define font8x14Id  9011
#define font16x16Id 9012
#define font8x16Id  9013

#define sysFileTypePlugin 'plgi'

#define sysRsrcTypeDlib   'dlib'
#define sysRsrcTypeWinD   'wind'

#define sysAnyPluginId  0xFFFFFFFF

#define vchrPumpkinMin     0x3000
#define vchrPumpkinMax     0x30FF
#define vchrRefreshState   vchrPumpkinMin
#define vchrAppStarted     vchrPumpkinMin+1
#define vchrAppFinished    vchrPumpkinMin+2
#define vchrAppCrashed     vchrPumpkinMin+3
#define vchrReloadState    vchrPumpkinMin+4

#define sysAppLaunchFlagFork 0x8000

#define sysNotifyMaximumPriority 0x01
#define sysNotifyMediumPriority  0x80
#define sysNotifyMinimumPriority 0xFF

#define PasswordForm  17000
#define passwordFld   17000

#define BITMAP_MAGIC 'Bitm'

#define PUMPKINOS "PumpkinOS"

#define minValue(a,b) (a) < (b) ? (a) : (b)
#define maxValue(a,b) (a) > (b) ? (a) : (b)

typedef struct {
  char name[dmDBNameLength];
  UInt16 code;
  UInt16 flags;
  void *param;
  uint32_t (*pilot_main)(uint16_t code, void *param, uint16_t flags);
  UInt16 opendb;
} launch_request_t;

typedef struct {
  uint32_t type;
  union {
    launch_request_t launch;
  } data;
} client_request_t;

typedef void *(*pluginMainF)(void *p);

typedef struct {
  UInt32 type;
  UInt32 id;
  pluginMainF pluginMain;
} pumpkin_plugin_t;

typedef struct pumpkin_httpd_t pumpkin_httpd_t;

typedef UInt32 PilotMainF(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);
Err SysAppLaunchEx(UInt16 cardNo, LocalID dbID, UInt16 launchFlags, UInt16 cmd, MemPtr cmdPBP, UInt32 *resultP, PilotMainF pilotMain);
uint32_t pumpkin_launch_request(char *name, UInt16 cmd, UInt8 *param, UInt16 flags, PilotMainF pilotMain, UInt16 opendb);
uint32_t pumpkin_fork(void);

void *pumpkin_heap_base(void);
uint32_t pumpkin_heap_size(void);
void *pumpkin_heap_alloc(uint32_t size, char *tag);
void *pumpkin_heap_realloc(void *p, uint32_t size, char *tag);
void pumpkin_heap_free(void *p, char *tag);
void *pumpkin_heap_dup(void *p, uint32_t size, char *tag);
void pumpkin_heap_dump(void);
void pumpkin_heap_walk(int global);

int pumpkin_global_init(script_engine_t *engine, window_provider_t *wp, audio_provider_t *ap, bt_provider_t *bt, gps_parse_line_f gps_parse_line);
int pumpkin_global_finish(void);
void pumpkin_deploy_files(char *path);
void pumpkin_local_refresh(void);
void pumpkin_set_spawner(int handle);
int pumpkin_is_spawner(void);
int pumpkin_get_spawner(void);
void pumpkin_set_window(window_t *w, int width, int height, int full_height);
void pumpkin_get_window(int *width, int *height);
int pumpkin_set_single(int depth);
int pumpkin_is_single(void);
int pumpkin_set_dia(int depth);
int pumpkin_is_dia(void);
void pumpkin_set_display(int ptr, int width, int height);
void pumpkin_set_input(int num, int width, int height);
void pumpkin_set_background(int depth, uint8_t r, uint8_t g, uint8_t b);
void pumpkin_set_border(int depth, int size, uint8_t rsel, uint8_t gsel, uint8_t bsel, uint8_t r, uint8_t g, uint8_t b);
void pumpkin_set_mono(int mono);
int pumpkin_get_current(void);

void pumpkin_set_secure(void *secure);
int pumpkin_http_get(char *url, int (*callback)(int ptr, void *_data), void *data);

int pumpkin_dia_enabled(void);
int pumpkin_dia_set_trigger(int trigger);
int pumpkin_dia_get_trigger(void);
int pumpkin_dia_set_state(int state);
int pumpkin_dia_get_state(void);
int pumpkin_dia_set_graffiti_state(int state);
int pumpkin_dia_get_taskbar_dimension(int *width, int *height);
int pumpkin_dia_kbd(void);

int pumpkin_launcher(char *name, int width, int height);
int pumpkin_launch(launch_request_t *request);
int pumpkin_ps(int (*ps_callback)(int i, char *name, int m68k, void *data), void *data);
int pumpkin_kill(char *name);
uint32_t pumpkin_get_taskid(void);
LocalID pumpkin_get_app_localid(void);
UInt32 pumpkin_get_app_creator(void);
void *pumpkin_get_exception(void);
void pumpkin_error_dialog(char *msg);
void pumpkin_fatal_error(int finish);
void pumpkin_set_size(uint32_t creator, uint16_t width, uint16_t height);
void pumpkin_create_compat(uint32_t creator);
void pumpkin_set_compat(uint32_t creator, int compat, int code);
void pumpkin_enum_compat(void (*callback)(UInt32 creator, UInt16 index, UInt16 id, void *p, void *data), void *data);
void pumpkin_compat_log(void);

void pumpkin_set_finish(int finish);
int pumpkin_must_finish(void);
void pumpkin_set_battery(int level);
int pumpkin_get_battery(void);
int pumpkin_is_launched(void);
int pumpkin_pause(int pause);
int pumpkin_is_paused(void);
int pumpkin_sys_event(void);
int pumpkin_event(int *key, int *mods, int *buttons, uint8_t *data, uint32_t *n, uint32_t usec);
void pumpkin_forward_event(int i, int ev, int a1, int a2, int a3);
int pumpkin_event_peek(void);
void pumpkin_keymask(uint32_t keyMask);
void pumpkin_status(int *x, int *y, uint32_t *keyMask, uint32_t *modMask, uint32_t *buttonMask, uint64_t *extKeyMask);
int pumpkin_extkey_down(int key, uint64_t *extKeyMask);
void pumpkin_screen_dirty(WinHandle win, int x, int y, int w, int h);
void pumpkin_screen_copy(uint16_t *src, uint16_t y0, uint16_t y1);
int pumpkin_change_display(int width, int height);
int pumpkin_default_density(void);
void pumpkin_calibrate(int restore);
ColorTableType *pumpkin_defaultcolorTable(void);

void pumpkin_set_data(void *data);
void *pumpkin_get_data(void);

AlertTemplateType *pumpkin_create_alert(void *h, uint8_t *p, uint32_t *dsize);
MenuBarType *pumpkin_create_menu(void *h, uint8_t *p, uint32_t *dsize);
BitmapType *pumpkin_create_bitmap(void *h, uint8_t *p, uint32_t size, uint32_t type, int chain, uint32_t *dsize);
FontType *pumpkin_create_font(void *h, uint8_t *p, uint32_t size, uint32_t *dsize);
FontTypeV2 *pumpkin_create_fontv2(void *h, uint8_t *p, uint32_t size, uint32_t *dsize);

void pumpkin_destroy_alert(void *p);
void pumpkin_destroy_menu(void *p);
void pumpkin_destroy_bitmap(void *p);
void pumpkin_destroy_font(void *p);
void pumpkin_destroy_fontv2(void *p);

void *pumpkin_encode_bitmap(void *p, uint32_t *size);

int pumpkin_script_create(void);
int pumpkin_script_init(int pe, uint32_t type, uint16_t id);
int pumpkin_script_destroy(int pe);
int pumpkin_script_create_obj(int pe, char *name);
int pumpkin_script_global_function(int pe, char *name, int (*f)(int pe));
int pumpkin_script_global_function_data(int pe, char *name, int (*f)(int pe, void *data), void *data);
int pumpkin_script_global_iconst(int pe, char *name, int value);
int pumpkin_script_global_iconst_value(int pe, char *name);
void *pumpkin_script_global_pointer_value(int pe, char *name);
int pumpkin_script_obj_function(int pe, script_ref_t obj, char *name, int (*f)(int pe));
int pumpkin_script_obj_boolean(int pe, script_ref_t obj, char *name, int value);
int pumpkin_script_obj_iconst(int pe, script_ref_t obj, char *name, int value);
int pumpkin_script_obj_sconst(int pe, script_ref_t obj, char *name, char *value);
int pumpkin_script_obj_function(int pe, script_ref_t obj, char *name, int (*f)(int pe));
char *pumpkin_script_call(int pe, char *function, char *s);
int pumpkin_script_run_string(int pe, char *s);
int pumpkin_script_run_file(int pe, char *s);
int pumpkin_script_get_last_error(int pe, char *buf, int max);
uint32_t pumpkin_script_engine_id(void);

int pumpkin_add_serial(char *descr, uint32_t creator, char *host, uint32_t port);
int pumpkin_info_serial(uint32_t id, char *host, int hlen, uint32_t *port);
int pumpkin_get_serial(uint32_t id, char **descr, uint32_t *creator);
int pumpkin_get_serial_by_creator(uint32_t *id, char **descr, uint32_t creator);
int pumpkin_num_serial(void);
int pumpkin_open_serial(uint32_t id);
int pumpkin_close_serial(uint32_t id);
int pumpkin_baud_serial(uint32_t id, uint32_t baud);
int pumpkin_word_serial(uint32_t id, char *word);

void pumpkin_load_plugins(void);
pumpkin_plugin_t *pumpkin_get_plugin(UInt32 type, UInt32 id);
void pumpkin_enum_plugins(UInt32 type, void (*callback)(pumpkin_plugin_t *plugin, void *data), void *data);

FontTypeV2 *pumpkin_get_font(FontID font);

int pumpkin_clipboard_add_text(char *text, int length);
int pumpkin_clipboard_append_text(char *text, int length);
int pumpkin_clipboard_get_text(char *text, int *length);
int pumpkin_clipboard_add_bitmap(BitmapType *bmp, int size);

int pumpkin_alarm_check(void);
int pumpkin_alarm_set(LocalID dbID, uint32_t t, uint32_t data);
int pumpkin_alarm_get(LocalID dbID, uint32_t *t, uint32_t *data);

void pumpkin_set_preference(UInt32 creator, UInt16 seq, void *p, UInt16 size, Boolean saved);
UInt16 pumpkin_get_preference(UInt32 creator, UInt16 seq, void *p, UInt16 size, Boolean saved);

void pumpkin_set_v10(void);
void pumpkin_set_m68k(int m68k);
int pumpkin_is_v10(void);
int pumpkin_is_m68k(void);

pumpkin_httpd_t *pumpkin_httpd_create(UInt16 port, UInt16 scriptId, char *worker, char *root, void *data, Boolean (*idle)(void *data));
int pumpkin_httpd_destroy(pumpkin_httpd_t *h);
int pumpkin_httpd_status(pumpkin_httpd_t *h);

UInt32 pumpkin_next_char(UInt8 *s, UInt32 i, UInt16 *w);
UInt8 pumpkin_map_char(UInt16 w, FontType **f);

UInt32 pumpkin_s2id(UInt32 *ID, char *s);
char *pumpkin_id2s(UInt32 ID, char *s);
uint32_t pumpkin_dt(void);
int pumpkin_getstr(char **s, uint8_t *p, int i);

void pumpkin_save_bitmap(BitmapType *bmp, UInt16 density, Coord wWidth, Coord wHeight, Coord width, Coord height, char *filename);
void pumpkin_save_icon(char *name);

MemPtr MemHandleLockEx(MemHandle h, Boolean decoded);
Err MemHandleUnlockEx(MemHandle h, UInt16 *lockCount);
Err DmResourceType(MemHandle resourceH, DmResType *resType, DmResID *resID);
MemHandle DmNewRecordEx(DmOpenRef dbP, UInt16 *atP, UInt32 size, void *p);
MemHandle MemLocalIDToHandle(LocalID local);

Int16 StrNPrintF(Char *s, UInt16 size, const Char *formatStr, ...);
Int16 StrVNPrintF(Char *s, UInt16 size, const Char *formatStr, sys_va_list arg);

void WinCopyWindow(WinHandle src, WinHandle dst, RectangleType *rect, Coord dstX, Coord dstY);
void WinBlitBitmap(BitmapType *bitmapP, WinHandle wh, const RectangleType *rect, Coord x, Coord y, WinDrawOperation mode, Boolean text);
void WinSaveRectangle(WinHandle dstWin, const RectangleType *srcRect);
void WinRestoreRectangle(WinHandle srcWin, const RectangleType *dstRect);
void WinSetClipingBounds(WinHandle win, const RectangleType *rP);
UInt8 WinSetBackAlpha(UInt8 alpha);
UInt8 WinGetBackAlpha(void);
void WinSendWindowEvents(WinHandle wh);
void WinSetDisplayExtent(Coord extentX, Coord extentY);
void WinAdjustCoords(Coord *x, Coord *y);
void WinAdjustCoordsInv(Coord *x, Coord *y);
void WinLegacyGetAddr(UInt32 *startAddr, UInt32 *endAddr);
void WinLegacyWriteByte(UInt32 offset, UInt8 value);
void WinLegacyWriteWord(UInt32 offset, UInt16 value);
void WinLegacyWriteLong(UInt32 offset, UInt32 value);
Int16 WinGetBorderRect(WinHandle wh, RectangleType *rect);
surface_t *WinCreateSurface(WinHandle wh, RectangleType *rect);

int PrefInitModule(void);
int PrefFinishModule(void);
char *PrefCountryName(UInt32 i);
char *PrefLanguageName(UInt32 i);
Err LmTimeZoneToIndex(Int16 timeZone, UInt16 *ioLocaleIndex);

char *EvtGetEventName(UInt16 eType);
void EvtEmptyQueue(void);
int EvtPumpEvents(Int32 timeoutUs);
void EvtGetEventUs(EventType *event, Int32 timeoutUs);
void EvtPrintEvent(char *op, EventType *event);

#define ErrFatalDisplayEx(msg, finish) ErrDisplayFileLineMsgEx(__FILE__, __FUNCTION__, (UInt16) __LINE__, msg, finish)
void ErrDisplayFileLineMsgEx(const Char * const filename, const Char * const function, UInt16 lineNo, const Char * const msg, int finish);
void SysFatalAlertFinish(void);

BitmapType *BmpGetBestBitmapEx(BitmapPtr bitmapP, UInt16 density, UInt8 depth, Boolean checkAddr);
void BmpPutBit(UInt32 b, Boolean transp, BitmapType *dst, Coord dx, Coord dy, WinDrawOperation mode, Boolean dbl);
void BmpCopyBit(BitmapType *src, Coord sx, Coord sy, BitmapType *dst, Coord dx, Coord dy, WinDrawOperation mode, Boolean dbl, Boolean text, UInt32 tc, UInt32 bc);
BitmapType *BmpCreate3(Coord width, Coord height, UInt16 density, UInt8 depth, Boolean hasTransparency, UInt32 transparentValue, ColorTableType *colorTableP, UInt16 *error);
void BmpDrawSurface(BitmapType *bitmapP, Coord sx, Coord sy, Coord w, Coord h, surface_t *surface, Coord x, Coord y, Boolean useTransp);
IndexedColorType BmpGetPixel(BitmapType *bitmapP, Coord x, Coord y);
UInt32 BmpGetPixelValue(BitmapType *bitmapP, Coord x, Coord y);
Err BmpGetPixelRGB(BitmapType *bitmapP, Coord x, Coord y, RGBColorType *rgbP);
void BmpSetPixel(BitmapType *bitmapP, Coord x, Coord y, UInt32 value);
BitmapType *BmpGetBestBitmap(BitmapPtr bitmapP, UInt16 density, UInt8 depth);
void BmpFillData(BitmapType *bitmapP);
void BmpPrintChain(BitmapType *bitmapP, DmResType type, DmResID resID, char *label);
const UInt8 *BmpGetGray(UInt8 depth);
surface_t *BmpBitmapCreateSurface(UInt16 id);
UInt32 BmpConvertFrom24Bits(UInt32 b, UInt8 depth, ColorTableType *dstColorTable);

void FrmCenterDialogs(Boolean center);
Boolean FrmTrackPenUp(UInt32 x, UInt32 y);
void FrmSetFormBounds(const FormType *formP, RectangleType *rP);
void FrmSetUsable(FormType *formP, UInt16 objIndex, Boolean usable);
void FrmSetObjectPtr(const FormType *formP, UInt16 objIndex, void *p);
void FrmDrawObject(FormType *formP, UInt16 objIndex, Boolean setUsable);
void FrmEraseObject(FormType *formP, UInt16 objIndex, Boolean setUsable);
UInt16 FrmGrfGetState(void);
void FrmGrfSetState(UInt16 state);
UInt16 FrmDoDialogEx(FormType *formP, Int32 timeout);
void FrmDrawEmptyDialog(FormType *formP, RectangleType *rect, Int16 margin, WinHandle wh);
char *FrmAskPassword(UInt16 maxLength);

void pumpkin_fix_popups(FormType *form);
FormType *pumpkin_create_form(uint8_t *p, uint32_t formSize);
void pumpkin_destroy_form(FormType *formP);

ListType *LstNewListEx(void **formPP, UInt16 id, Coord x, Coord y, Coord width, Coord height, FontID font, Int16 visibleItems, Int16 triggerId, Boolean usable);
void LstFreeListChoices(ListType *listP);

void CtlUpdateGroup(ControlType *controlP, Boolean value);
FieldType *FldGetActiveField(void);
void FldSetActiveField(FieldType *fldP);
void FldBlinkCursor(void);
void FldSetPassword(FieldType *fldP, Boolean password);
void FldReplaceText(FieldType *fldP, char *s, Boolean focus);
IndexedColorType WinGetForeColor(void);
IndexedColorType WinGetBackColor(void);
void WinDrawCharBox(Char *text, UInt16 len, FontID font, RectangleType *bounds, Boolean draw, UInt16 *drawnLines, UInt16 *totalLines, UInt16 *maxWidth, LineInfoType *lineInfo, UInt16 maxLines);
void WinInvertRect(RectangleType *rect, UInt16 corner, Boolean isInverted);
void RctRectToAbs(const RectangleType *rP, AbsRectType *arP);
void RctAbsToRect(const AbsRectType *arP, RectangleType *rP);
UInt16 RctGetDifference(const RectangleType *a, const RectangleType *b, RectangleType *r);
void RctGetUnion(const RectangleType *a, const RectangleType *b, RectangleType *r);

void FntSaveFonts(void);

Err HwrGetROMToken(UInt16 cardNo, UInt32 token, UInt8 **dataP, UInt16 *sizeP);

int SysUIAppSwitchCont(launch_request_t *request);

void SysNotifyBroadcastQueued(void);

Boolean SysLibNewRefNum68K(UInt32 type, UInt32 creator, UInt16 *refNum);
Err SysLibRegister68K(UInt16 refNum, LocalID dbID, uint8_t *code, UInt32 size, UInt16 *dispatchTblP, UInt8 *globalsP);
uint8_t *SysLibTblEntry68K(UInt16 refNum, SysLibTblEntryType *tbl);
void SysLibCancelRefNum68K(UInt16 refNum);
UInt16 SysLibFind68K(char *name);
UInt16 *SysLibGetDispatch68K(UInt16 refNum);
Boolean SysCreateDataBaseList68K(UInt32 type, UInt32 creator, UInt16 *dbCount, MemHandle *dbIDs, Boolean lookupName);

Boolean CallFormHandler(UInt32 addr, EventType *event);
Boolean CallGadgetHandler(UInt32 addr, FormGadgetTypeInCallback *gadgetP, UInt8 cmd, EventType *eventP);
void CallListDrawItem(UInt32 addr, Int16 i, RectangleType *rect, char **text);
void CallTableDrawItem(UInt32 addr, TableType *tableP, Int16 row, Int16 column, RectangleType *rect);
Boolean CallTableSaveData(UInt32 addr, TableType *tableP, Int16 row, Int16 column);
Err CallTableLoadData(UInt32 addr, TableType *tableP, Int16 row, Int16 column, Boolean editable, MemHandle *dataH, Int16 *dataOffset, Int16 *dataSize, FieldPtr fld);
Int16 CallDmCompare(UInt32 addr, UInt32 rec1, UInt32 rec2, Int16 other, UInt32 rec1SortInfo, UInt32 rec2SortInfo, UInt32 appInfoH);
Err CallNotifyProc(UInt32 addr, SysNotifyParamType *notifyParamsP);

typedef Int32 _comparFP (void *, void *, void *otherP);
typedef _comparFP * CmpFuncPPtr;
void SysQSortP(void *baseP, UInt32 numOfElements, Int32 width, CmpFuncPPtr comparFP, void *otherP);

UInt16 KbdGrfGetState(void);
void KbdGrfSetState(UInt16 state);
Err KbdDrawKeyboard(KeyboardType kbd, Boolean upper, WinHandle wh, RectangleType *bounds);

char *VFSGetMount(void);
Err VFSChangeDir(UInt16 volRefNum, char *path);
Err VFSCurrentDir(UInt16 volRefNum, char *path, UInt16 max);

void AbtShowAboutEx(UInt32 creator, UInt16 formID, char *descr);
void AbtShowAboutPumpkin(UInt32 creator);

int WinInitModule(UInt16 density, UInt16 width, UInt16 height, UInt16 depth, WinHandle displayWindow);
void *WinReinitModule(void *module);
int WinFinishModule(Boolean deleteDisplay);
int BmpInitModule(UInt16 density);
int BmpFinishModule(void);
int FntInitModule(UInt16 density);
int FntFinishModule(void);
int UicInitModule(void);
int UicFinishModule(void);
int FrmInitModule(void);
void *FrmReinitModule(void *module);
int FrmFinishModule(void);
int InsPtInitModule(void);
int InsPtFinishModule(void);
int FldInitModule(void);
void *FldReinitModule(void *module);
int FldFinishModule(void);
int MenuInitModule(void);
void *MenuReinitModule(void *module);
int MenuFinishModule(void);
int EvtInitModule(void);
int EvtFinishModule(void);
int SysUInitModule(void);
int SysUFinishModule(void);
int SysInitModule(void);
int SysFinishModule(void);
int NotInitModule(void);
int NotFinishModule(void);
int VFSInitModule(char *card);
int VFSFinishModule(void);
int KeyboardInitModule(void);
int KeyboardFinishModule(void);
int ClpInitModule(void);
int ClpFinishModule(void);
int SrmInitModule(void);
int SrmFinishModule(void);
int FtrInitModule(void);
int FtrFinishModule(void);
int KeyInitModule(void);
int KeyFinishModule(void);
int SndInitModule(audio_provider_t *ap);
int SndFinishModule(void);
int SelTimeInitModule(void);
int SelTimeFinishModule(void);
int GPSInitModule(gps_parse_line_f parse_line, bt_provider_t *bt);
int GPSFinishModule(void);

void pumpkin_crash_log(UInt32 creator, int code, char *msg);
void pumpkin_test_exception(int fatal);
void pumpkin_debug_init(void);
void pumpkin_debug_check(void);

uint32_t pumpkin_script_main(uint16_t code, void *param, uint16_t flags);
int pumpkin_script_appenv(int pe);

void pumpkin_setio(char (*getchar)(void *iodata), void (*putchar)(void *iodata, char c),
       void (*setcolor)(void *iodata, uint32_t fg, uint32_t bg), void *iodata);
char pumpkin_getchar(void);
void pumpkin_putchar(char c);
void pumpkin_puts(char *s);
uint32_t pumpkin_gets(char *buf, uint32_t max);
void pumpkin_setcolor(uint32_t fg, uint32_t bg);

void pumpkin_sound_init(void);
void pumpkin_sound_finish(void);

#ifdef __cplusplus
}
#endif
