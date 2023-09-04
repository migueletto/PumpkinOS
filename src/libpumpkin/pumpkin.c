#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#include "script.h"
#include "thread.h"
#include "mutex.h"
#include "pwindow.h"
#include "audio.h"
#include "bytes.h"
#include "vfs.h"
#include "ptr.h"
#include "pit_io.h"
#include "gps.h"
#include "secure.h"
#include "filter.h"
#include "httpc.h"
#include "httpd.h"
#include "template.h"
#include "iterator.h"
#include "loadfile.h"
#include "emupalmosinc.h"
#include "AppRegistry.h"
#include "language.h"
#include "storage.h"
#include "pumpkin.h"
#include "media.h"
#include "timeutc.h"
#include "heap.h"
#include "grail.h"
#include "dia.h"
#include "wman.h"
#include "color.h"
#include "dbg.h"
#include "debug.h"
#include "xalloc.h"

#define DEFAULT_DENSITY kDensityDouble

#define MAX_SEARCH_ORDER   16
#define MAX_NOTIF_QUEUE    8
#define MAX_NOTIF_REGISTER 256
#define MAX_TASKS          32
#define MAX_HOST           256

#define TAG_SCREEN  "screen"
#define TAG_SERIAL  "serial"
#define TAG_APP     "App"
#define TAG_NOTIF   "notif"

#ifdef ANDROID
#define REGISTRY_DB   "/data/data/com.pit.pit/app_registry/"
#else
#define REGISTRY_DB   "registry/"
#endif

#define VFS_CARD      "/app_card/"
#define VFS_INSTALL   "/app_install/"

#define HEAP_SIZE (8*1024*1024)

#define APP_STORAGE "/app_storage/"

#define MAX_PLUGINS 256

#define FONT_BASE   9000

#define CRASH_LOG   "crash.log"
#define COMPAT_LIST "log/compat.txt"

#define PUMPKIN_USER_AGENT  PUMPKINOS
#define PUMPKIN_SERVER_NAME PUMPKINOS

typedef struct {
  uint16_t refNum;
  uint32_t type;
  uint32_t creator;
  uint8_t *code;
  uint32_t codeSize;
  LocalID dbID;
  UInt16 *dispatchTbl;
  UInt8 *globals;
  char name[dmDBNameLength];
  uint8_t *tbl;
} syslib_t;

typedef struct {
  launch_request_t request;
  texture_t *texture;
  int index, width, height, x, y;
  UInt32 creator;
} launch_data_t;

typedef struct {
  int task_index;
  int active;
  int paused;
  int reserved;
  script_ref_t obj;
  int screen_ptr;
  int dx, dy;
  int width, height;
  int new_width, new_height;
  int handle;
  int penX, penY, buttons;
  int v10;
  int m68k;
  char name[dmDBNameLength];
  uint32_t alarm_time;
  uint32_t alarm_data;
  uint32_t eventKeyMask;
  texture_t *texture;
  LocalID dbID;
  UInt32 creator;
  DmOpenRef bootRef;
  language_t *lang;
  ErrJumpBuf jmpbuf;
  char *serial[MAX_SERIAL];
  syslib_t syslibs[MAX_SYSLIBS];
  int num_syslibs;
  uint32_t taskId;
  void *exception;
  heap_t *heap;
  int num_notifs;
  SysNotifyParamType notify[MAX_NOTIF_QUEUE]; // for SysNotifyBroadcastDeferred
  void *data;
  char (*getchar)(void *iodata);
  void (*putchar)(void *iodata, char c);
  void (*setcolor)(void *iodata, uint32_t fg, uint32_t bg);
  void *iodata;
} pumpkin_task_t;

typedef struct {
  char *tag;
  surface_t *surface;
  int dirty; // app screen changed
  int x0, y0, x1, y1;
} task_screen_t;

typedef struct {
  char *descr;
  uint32_t creator;
  uint32_t port;
  char host[MAX_HOST];
  int fd;
} pumpkin_serial_t;

typedef struct {
  char *tag;
  SysNotifyProcPtr callback;
  void *userData;
  uint32_t callback68k;
  uint32_t userData68k;
} notif_ptr_t;

typedef struct {
  UInt32 appCreator;
  UInt32 notifyType;
  UInt32 priority;
  int ptr;
} notif_registration_t;

typedef struct {
  script_engine_t *engine;
  window_provider_t *wp;
  audio_provider_t *ap;
  secure_provider_t *secure;
  bt_provider_t *bt;
  gps_parse_line_f gps_parse_line;
  window_t *w;
  heap_t *heap;
  mutex_t *fs_mutex;
  int battery;
  int finish;
  int paused;
  int launched;
  int spawner;
  int encoding;
  int width, height, depth, border;
  int dragging;
  int lastX, lastY;
  int current_task;
  uint32_t keyMask;
  uint32_t buttonMask;
  uint32_t modMask;
  uint64_t extKeyMask[2];
  int64_t lastUpdate;
  int num_used;
  int task_order[MAX_TASKS];
  pumpkin_task_t tasks[MAX_TASKS];
  int num_tasks;
  int single;
  dia_t *dia;
  wman_t *wm;
  int dragged, render;
  pumpkin_serial_t serial[MAX_SERIAL];
  int num_serial;
  char clipboardAux[cbdMaxTextLength*2];
  uint32_t nextTaskId;
  pumpkin_plugin_t *plugin[MAX_PLUGINS];
  int num_plugins;
  AppRegistryType *registry;
  notif_registration_t notif[MAX_NOTIF_REGISTER];
  int num_notif;
  FontTypeV2 *fontPtr[128];
  MemHandle fontHandle[128];
} pumpkin_module_t;

typedef union {
  uint32_t t;
  uint8_t c[4];
} creator_id_t;

struct pumpkin_httpd_t {
  char prefix[256];
  int pe;
  int status;
  char *script;
  script_ref_t ref;
  Boolean (*idle)(void *data);
  void *data;
};

static const int systemFonts[] = {
  stdFont, boldFont, largeFont, symbolFont, symbol11Font, symbol7Font, ledFont, largeBoldFont,
  mono6x10Font, mono8x14Font, mono16x16Font, mono8x16Font,
  -1
};

static mutex_t *mutex;
static pumpkin_module_t pumpkin_module;

static void pumpkin_make_current(int i);

thread_key_t *task_key;
thread_key_t *sto_key;
thread_key_t *dm_key;
thread_key_t *evt_key;
thread_key_t *fnt_key;
thread_key_t *frm_key;
thread_key_t *ins_key;
thread_key_t *fld_key;
thread_key_t *menu_key;
thread_key_t *sys_key;
thread_key_t *sysu_key;
thread_key_t *uic_key;
thread_key_t *bmp_key;
thread_key_t *win_key;
thread_key_t *pref_key;
thread_key_t *gps_key;
thread_key_t *vfs_key;
thread_key_t *kbd_key;
thread_key_t *clp_key;
thread_key_t *srm_key;
thread_key_t *ftr_key;
thread_key_t *key_key;
thread_key_t *snd_key;
thread_key_t *seltime_key;
thread_key_t *fatal_key;

void *pumpkin_heap_base(void) {
  return heap_base(heap_get());
}

uint32_t pumpkin_heap_size(void) {
  return heap_size(heap_get());
}

void heap_exhausted_error(void) {
  if (pumpkin_is_m68k()) {
    emupalmos_panic("Heap exhausted", EMUPALMOS_HEAP_EXHAUSTED);
  } else {
   SysFatalAlert("Heap exhausted");
   pumpkin_fatal_error(0);
  }
}

void heap_assertion_error(char *msg) {
  pumpkin_fatal_error(0);
}

void pumpkin_test_exception(int fatal) {
  debug(DEBUG_ERROR, PUMPKINOS, "pumpkin_test_exception fatal=%d", fatal);
  ErrDisplayFileLineMsgEx("test.c", "test", 1, "test exception", fatal);
  debug(DEBUG_ERROR, PUMPKINOS, "pumpkin_test_exception should not be here!");
}

void pumpkin_debug_init(void) {
}

void pumpkin_debug_check(void) {
}

void *pumpkin_heap_alloc(uint32_t size, char *tag) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  void *p;

  p = heap_alloc(task ? task->heap : pumpkin_module.heap, size);
  if (p) {
    debug(DEBUG_TRACE, "Heap", "ALLOC %p %s %u", p, tag, size);
    xmemset(p, 0, size);
  }

  return p;
}

void *pumpkin_heap_realloc(void *p, uint32_t size, char *tag) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  void *q = NULL;

  if (p) {
    q = size ? heap_realloc(task ? task->heap : pumpkin_module.heap, p, size) : NULL;
    debug(DEBUG_TRACE, "Heap", "FREE %p %s", p, tag);
    debug(DEBUG_TRACE, "Heap", "ALLOC %p %s %u", q, tag, size);
  }

  return q;
}

void pumpkin_heap_free(void *p, char *tag) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  if (p) {
    debug(DEBUG_TRACE, "Heap", "FREE %p %s", p, tag);
    heap_free(task ? task->heap : pumpkin_module.heap, p);
  }
}

void *pumpkin_heap_dup(void *p, uint32_t size, char *tag) {
  void *q = NULL;

  if (p && size) {
    q = pumpkin_heap_alloc(size, tag);
    if (q) xmemcpy(q, p, size);
  }

  return q;
}

void pumpkin_heap_dump(void) {
  if (mutex_lock(mutex) == 0) {
    heap_dump(pumpkin_module.heap);
    mutex_unlock(mutex);
  }
}

void pumpkin_heap_walk(int global) {
  if (global) {
    heap_walk(pumpkin_module.heap, StoHeapWalk, 0);
  } else {
    pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
    heap_walk(pumpkin_module.heap, StoHeapWalk, task->task_index+1);
  }
}

heap_t *heap_get(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  return task ? task->heap : pumpkin_module.heap;
}

static int pumpkin_register_plugin(UInt32 type, UInt32 id, pluginMainF pluginMain) {
  pumpkin_plugin_t *plugin;
  char sType[8], sId[8];
  int r = -1;

  if (pumpkin_module.num_plugins < MAX_PLUGINS) {
    if ((plugin = xcalloc(1, sizeof(pumpkin_plugin_t))) != NULL) {
      pumpkin_id2s(type, sType);
      pumpkin_id2s(id, sId);
      debug(DEBUG_INFO, PUMPKINOS, "registering plugin type '%s' id '%s'", sType, sId);
      plugin->type = type;
      plugin->id = id;
      plugin->pluginMain = pluginMain;
      pumpkin_module.plugin[pumpkin_module.num_plugins++] = plugin;
      r = 0;
    }
  }

  return r;
}

pumpkin_plugin_t *pumpkin_get_plugin(UInt32 type, UInt32 id) {
  pumpkin_plugin_t *plugin = NULL;
  char sType[8], sId[8];
  int i;

  for (i = 0; i < pumpkin_module.num_plugins; i++) {
    if (pumpkin_module.plugin[i]->type == type && (id == sysAnyPluginId || pumpkin_module.plugin[i]->id == id)) {
      plugin = pumpkin_module.plugin[i];
      id = pumpkin_module.plugin[i]->id;
      break;
    }
  }

  if (plugin == NULL) {
    pumpkin_id2s(type, sType);
    pumpkin_id2s(id, sId);
    debug(DEBUG_INFO, PUMPKINOS, "plugin type '%s' id '%s' not found", sType, sId);
  }

  return plugin;
}

void pumpkin_enum_plugins(UInt32 type, void (*callback)(pumpkin_plugin_t *plugin, void *data), void *data) {
  int i;

  for (i = 0; i < pumpkin_module.num_plugins; i++) {
    if (pumpkin_module.plugin[i]->type == type) {
      callback(pumpkin_module.plugin[i], data);
    }
  }
}

void pumpkin_load_plugins(void) {
  DmSearchStateType stateInfo;
  UInt32 type, id;
  UInt16 cardNo;
  LocalID dbID;
  DmOpenRef dbRef;
  Boolean newSearch, firstLoad;
  pluginMainF (*pluginInit)(UInt32 *type, UInt32 *id);
  pluginMainF pluginMain;
  void *lib;

  for (newSearch = true;; newSearch = false) {
    if (DmGetNextDatabaseByTypeCreator(newSearch, &stateInfo, sysFileTypePlugin, 0, false, &cardNo, &dbID) != errNone) break;

    if ((dbRef = DmOpenDatabase(cardNo, dbID, dmModeReadOnly)) != NULL) {
      if ((lib = DmResourceLoadLib(dbRef, sysRsrcTypeDlib, &firstLoad)) != NULL) {
        debug(DEBUG_INFO, PUMPKINOS, "plugin dlib resource loaded (first %d)", firstLoad ? 1 : 0);
        pluginInit = sys_lib_defsymbol(lib, "PluginInit", 1);
        if (pluginInit) {
          if ((pluginMain = pluginInit(&type, &id)) != NULL) {
            pumpkin_register_plugin(type, id, pluginMain);
          }
        } else {
          debug(DEBUG_ERROR, PUMPKINOS, "PluginInit not found in plugin dlib");
        }
        // sys_lib_close is not being called
      }
      DmCloseDatabase(dbRef);
    }
  }
}

static void SysNotifyLoadCallback(UInt32 creator, UInt16 index, UInt16 id, void *p, void *data) {
  AppRegistryNotification *n = (AppRegistryNotification *)p;
  char stype[8], screator[8];

  pumpkin_id2s(n->appCreator, screator);
  pumpkin_id2s(n->notifyType, stype);
  debug(DEBUG_INFO, PUMPKINOS, "load notification type '%s' creator '%s' priority %d: added", stype, screator, n->priority);

  pumpkin_module.notif[pumpkin_module.num_notif].appCreator = n->appCreator;
  pumpkin_module.notif[pumpkin_module.num_notif].notifyType = n->notifyType;
  pumpkin_module.notif[pumpkin_module.num_notif].priority = n->priority;
  pumpkin_module.notif[pumpkin_module.num_notif].ptr = 0;
  pumpkin_module.num_notif++;
}

FontTypeV2 *pumpkin_get_font(FontID fontId) {
  FontTypeV2 *fontv2 = NULL;

  if (fontId >= 0 && fontId < 128 && pumpkin_module.fontHandle[fontId] && pumpkin_module.fontPtr[fontId]) {
    fontv2 = pumpkin_module.fontPtr[fontId];
  }

  return fontv2;
}

static void pumpkin_load_fonts(void) {
  UInt16 index, fontId, resId;

  for (index = 0; systemFonts[index] >= 0; index++) {
    fontId = systemFonts[index];
    resId = FONT_BASE + fontId;

    if ((pumpkin_module.fontHandle[fontId] = DmGetResource(fontExtRscType, resId)) != NULL) {
      if ((pumpkin_module.fontPtr[fontId] = MemHandleLock(pumpkin_module.fontHandle[fontId])) == NULL) {
        debug(DEBUG_ERROR, PUMPKINOS, "error locking built-in nfnt %d font resource", resId);
        DmReleaseResource(pumpkin_module.fontHandle[fontId]);
        pumpkin_module.fontHandle[fontId] = NULL;
      }
    } else {
      debug(DEBUG_ERROR, PUMPKINOS, "built-in nfnt %d font resource not found", fontId);
    }
  }
}

static void pumpkin_unload_fonts(void) {
  UInt16 fontId;

  for (fontId = 0; fontId < 128; fontId++) {
    if (pumpkin_module.fontPtr[fontId]) {
      MemHandleUnlock(pumpkin_module.fontHandle[fontId]);
      pumpkin_module.fontPtr[fontId] = NULL;
    }
    if (pumpkin_module.fontHandle[fontId]) {
      DmReleaseResource(pumpkin_module.fontHandle[fontId]);
      pumpkin_module.fontHandle[fontId] = NULL;
    }
  }
}

int pumpkin_global_init(script_engine_t *engine, window_provider_t *wp, audio_provider_t *ap, bt_provider_t *bt, gps_parse_line_f gps_parse_line) {
  int fd;

  xmemset(&pumpkin_module, 0, sizeof(pumpkin_module_t));

  if ((mutex = mutex_create(PUMPKINOS)) == NULL) {
    return -1;
  }

  if ((pumpkin_module.fs_mutex = mutex_create("fs")) == NULL) {
    return -1;
  }

  task_key = thread_key();
  sto_key = thread_key();
  dm_key = thread_key();
  evt_key = thread_key();
  fnt_key = thread_key();
  frm_key = thread_key();
  ins_key = thread_key();
  fld_key = thread_key();
  menu_key = thread_key();
  sys_key = thread_key();
  sysu_key = thread_key();
  uic_key = thread_key();
  bmp_key = thread_key();
  win_key = thread_key();
  pref_key = thread_key();
  gps_key = thread_key();
  vfs_key = thread_key();
  kbd_key = thread_key();
  clp_key = thread_key();
  srm_key = thread_key();
  ftr_key = thread_key();
  key_key = thread_key();
  snd_key = thread_key();
  seltime_key = thread_key();
  fatal_key = thread_key();

  pumpkin_module.engine = engine;
  pumpkin_module.wp = wp;
  pumpkin_module.ap = ap;
  pumpkin_module.bt = bt;
  pumpkin_module.gps_parse_line = gps_parse_line;
  pumpkin_module.current_task = -1;
  pumpkin_module.dragging = -1;
  pumpkin_module.nextTaskId = 1;
  pumpkin_module.battery = 100;

  StoRemoveLocks(APP_STORAGE);

  pumpkin_module.heap = heap_init(HEAP_SIZE*4, wp);
  StoInit(APP_STORAGE, pumpkin_module.fs_mutex);

  SysUInitModule(); // sto calls SysQSortP

  pumpkin_module.registry = AppRegistryInit(REGISTRY_DB);

  pumpkin_module.num_notif = 0;
  AppRegistryEnum(pumpkin_module.registry, SysNotifyLoadCallback, 0, appRegistryNotification, NULL);

  emupalmos_init();
  if (ap && ap->mixer_init) ap->mixer_init();

  if ((fd = sys_open(CRASH_LOG, SYS_WRITE)) == -1) {
    fd = sys_create(CRASH_LOG, SYS_WRITE, 0644);
  }
  if (fd != -1) {
    sys_close(fd);
  }

  return 0;
}

void pumpkin_deploy_files(char *path) {
  DmOpenRef dbRef;
  LocalID dbID;

  StoDeployFiles(path, pumpkin_module.registry);

  dbID = DmFindDatabase(0, BOOT_NAME);
  dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly);
  PrefInitModule();
  pumpkin_load_fonts();
  DmCloseDatabase(dbRef);
}

void pumpkin_set_spawner(int handle) {
  debug(DEBUG_INFO, PUMPKINOS, "spawner set to port %d", handle);
  pumpkin_module.spawner = handle;
}

int pumpkin_is_spawner(void) {
  return thread_get_handle() == pumpkin_module.spawner;
}

int pumpkin_get_spawner(void) {
  return pumpkin_module.spawner;
}

void pumpkin_set_window(window_t *w, int width, int height) {
  debug(DEBUG_INFO, PUMPKINOS, "set window %dx%d", width, height);
  pumpkin_module.w = w;
  pumpkin_module.width = width;
  pumpkin_module.height = height;

  pumpkin_module.wm = wman_init(pumpkin_module.wp, w, width, height);
}

void pumpkin_get_window(int *width, int *height) {
  *width = pumpkin_module.width;
  *height = pumpkin_module.height;
}

static void pumpkin_set_encoding(int depth) {
  switch (depth) {
    case 8:
      pumpkin_module.encoding = SURFACE_ENCODING_PALETTE;
      break;
    case 16:
      pumpkin_module.encoding = SURFACE_ENCODING_RGB565;
      break;
    case 32:
      pumpkin_module.encoding = SURFACE_ENCODING_ARGB;
      break;
    default:
      pumpkin_module.encoding = SURFACE_ENCODING_RGB565;
      break;
  }
}

void pumpkin_set_secure(void *secure) {
  pumpkin_module.secure = secure;
}

int pumpkin_http_get(char *url, int (*callback)(int ptr, void *_data), void *data) {
  return pit_http_get(PUMPKIN_USER_AGENT, url, pumpkin_module.secure, callback, data);
}

int pumpkin_set_single(int depth) {
  pumpkin_set_encoding(depth);
  pumpkin_module.depth = depth;
  pumpkin_module.single = 1;

  return 0;
}

int pumpkin_is_single(void) {
  return pumpkin_module.single;
}

int pumpkin_set_dia(int depth) {
  pumpkin_set_encoding(depth);
  pumpkin_module.dia = dia_init(pumpkin_module.wp, pumpkin_module.w, pumpkin_module.encoding, depth, DEFAULT_DENSITY == kDensityDouble);
  pumpkin_module.depth = depth;

  return pumpkin_module.dia ? 0 : -1;
}

void pumpkin_set_background(int depth, uint8_t r, uint8_t g, uint8_t b) {
  pumpkin_module.depth = depth;
  pumpkin_set_encoding(depth);
  wman_set_background(pumpkin_module.wm, depth, r, g, b);
}

void pumpkin_set_border(int depth, int size, uint8_t rsel, uint8_t gsel, uint8_t bsel, uint8_t r, uint8_t g, uint8_t b) {
  pumpkin_module.depth = depth;
  pumpkin_module.border = size;
  pumpkin_set_encoding(depth);
  wman_set_border(pumpkin_module.wm, depth, size, rsel, gsel, bsel, r, g, b);
}

int pumpkin_dia_enabled(void) {
  return pumpkin_module.dia ? 1 : 0;
}

int pumpkin_dia_get_trigger(void) {
  int r = -1;

  if (pumpkin_module.dia) {
    if (mutex_lock(mutex) == 0) {
      r = dia_get_trigger(pumpkin_module.dia);
      mutex_unlock(mutex);
    }
  }

  return r;
}

int pumpkin_dia_set_trigger(int trigger) {
  int r = -1;

  if (pumpkin_module.dia) {
    if (mutex_lock(mutex) == 0) {
      r = dia_set_trigger(pumpkin_module.dia, trigger);
      mutex_unlock(mutex);
    }
  }

  return r;
}

int pumpkin_dia_get_state(void) {
  int r = -1;

  if (pumpkin_module.dia) {
    if (mutex_lock(mutex) == 0) {
      r = dia_get_state(pumpkin_module.dia);
      mutex_unlock(mutex);
    }
  }

  return r;
}

int pumpkin_dia_set_state(int state) {
  int r = -1;

  if (pumpkin_module.dia) {
    if (mutex_lock(mutex) == 0) {
      r = dia_set_state(pumpkin_module.dia, state);
      mutex_unlock(mutex);
    }
  }

  return r;
}

int pumpkin_dia_set_graffiti_state(int state) {
  int r = -1;

  if (pumpkin_module.dia) {
    if (mutex_lock(mutex) == 0) {
      dia_set_graffiti_state(pumpkin_module.dia, state);
      r = 0;
      mutex_unlock(mutex);
    }
  }

  return r;
}

int pumpkin_dia_get_taskbar_dimension(int *width, int *height) {
  int r = -1;

  if (pumpkin_module.dia) {
    if (mutex_lock(mutex) == 0) {
      r = dia_get_taskbar_dimension(pumpkin_module.dia, width, height);
      mutex_unlock(mutex);
    }
  }

  return r;
}

int pumpkin_dia_kbd(void) {
  WinHandle lower_wh, upper_wh, number_wh;
  UInt16 prev;
  int dw, dh;
  Err err;
  RectangleType bounds[256];
  int r = -1;

  if (pumpkin_module.dia) {
    dia_get_graffiti_dimension(pumpkin_module.dia, &dw, &dh);
    prev = WinSetCoordinateSystem(DEFAULT_DENSITY == kDensityDouble ? kCoordinatesDouble : kCoordinatesStandard);
    lower_wh  = WinCreateOffscreenWindow(dw, dh, nativeFormat, &err);
    upper_wh  = WinCreateOffscreenWindow(dw, dh, nativeFormat, &err);
    number_wh = WinCreateOffscreenWindow(dw, dh, nativeFormat, &err);
    WinSetCoordinateSystem(prev);

    if (KbdDrawKeyboard(kbdAlpha, false, lower_wh, bounds) == errNone) {
      dia_set_wh(pumpkin_module.dia, DIA_MODE_LOWER, lower_wh, bounds);
    } else {
      WinDeleteWindow(lower_wh, false);
    }
    if (KbdDrawKeyboard(kbdAlpha, true, upper_wh, bounds) == errNone) {
      dia_set_wh(pumpkin_module.dia, DIA_MODE_UPPER, upper_wh, bounds);
    } else {
      WinDeleteWindow(upper_wh, false);
    }
    if (KbdDrawKeyboard(kbdNumbersAndPunc, false, number_wh, bounds) == errNone) {
      dia_set_wh(pumpkin_module.dia, DIA_MODE_NUMBER, number_wh, bounds);
    } else {
      WinDeleteWindow(number_wh, false);
    }
  }

  return r;
}

int pumpkin_global_finish(void) {
  int i;

  pumpkin_unload_fonts();
  PrefFinishModule();

  if (pumpkin_module.wm) {
    wman_finish(pumpkin_module.wm);
  }

  for (i = 0; i < MAX_TASKS; i++) {
    if (pumpkin_module.tasks[i].texture) {
      pumpkin_module.wp->destroy_texture(pumpkin_module.w, pumpkin_module.tasks[i].texture);
    }
  }

  if (pumpkin_module.dia) {
    dia_finish(pumpkin_module.dia);
  }

  if (pumpkin_module.w) {
    if (pumpkin_module.wp->erase) pumpkin_module.wp->erase(pumpkin_module.w, 0);
    pumpkin_module.wp->destroy(pumpkin_module.w);
  }

  for (i = 0; i < pumpkin_module.num_serial; i++) {
    xfree(pumpkin_module.serial[i].descr);
  }

  for (i = 0; i < pumpkin_module.num_plugins; i++) {
    xfree(pumpkin_module.plugin[i]);
  }

  AppRegistryFinish(pumpkin_module.registry);

  SysUFinishModule();
  StoFinish();
  heap_finish(pumpkin_module.heap);
  mutex_destroy(pumpkin_module.fs_mutex);
  mutex_destroy(mutex);

  return 0;
}

static void task_destructor(void *p) {
  task_screen_t *screen;

  if (p) {
    screen = (task_screen_t *)p;
    if (screen->surface) {
      surface_destroy(screen->surface);
    }
    xfree(screen);
  }
}

int pumpkin_ps(int (*ps_callback)(int i, char *name, int m68k, void *data), void *data) {
  int i, r = -1;

  if (ps_callback) {
    if (mutex_lock(mutex) == 0) {
      for (i = 0; i < MAX_TASKS; i++) {
        if (pumpkin_module.tasks[i].active) {
          if (ps_callback(i, pumpkin_module.tasks[i].name, pumpkin_module.tasks[i].m68k, data) != 0) break;
        }
      }
      mutex_unlock(mutex);
      r = 0;
    }
  }

  return r;
}

int pumpkin_kill(char *name) {
  int i, r = -1;

  if (name) {
    if (mutex_lock(mutex) == 0) {
      for (i = 0; i < MAX_TASKS; i++) {
        if (pumpkin_module.tasks[i].active && !sys_strcmp(pumpkin_module.tasks[i].name, name)) {
          thread_end(TAG_APP, pumpkin_module.tasks[i].handle);
          r = 0;
          break;
        }
      }
      mutex_unlock(mutex);
      r = 0;
    }
  }

  return r;
}

static int pumpkin_pilotmain(char *name, PilotMainF pilotMain, uint16_t code, void *param, uint16_t flags) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  LocalID oldDbID, dbID;
  DmOpenRef dbRef;
  UInt32 oldCreator, creator;

  if ((dbID = DmFindDatabase(0, name)) != 0) {
    if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
      if (DmDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creator) == errNone) {
        oldDbID = task->dbID;
        oldCreator = task->creator;
        task->dbID = dbID;
        task->creator = creator;

        pilotMain(code, param, flags);

        if (code != sysAppLaunchCmdNormalLaunch || pumpkin_module.dia) {
          task->dbID = oldDbID;
          task->creator = oldCreator;
        }
      }
      DmCloseDatabase(dbRef);
    }
  }

  return 0;
}

static uint32_t pumpkin_launch_sub(launch_request_t *request, int opendb) {
  uint32_t (*pilot_main)(uint16_t code, void *param, uint16_t flags);
  uint32_t r = 0;
  LocalID dbID;
  DmOpenRef dbRef;
  MemHandle h;
  Boolean firstLoad;
  void *lib;
  int m68k;

  if (request) {
    pilot_main = request->pilot_main;
    lib = NULL;

    if (pilot_main) {
      debug(DEBUG_INFO, PUMPKINOS, "using provided PilotMain");
    } else {
      debug(DEBUG_INFO, PUMPKINOS, "searching PilotMain in dlib");
      if ((dbID = DmFindDatabase(0, request->name)) != 0) {
        if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
          if ((lib = DmResourceLoadLib(dbRef, sysRsrcTypeDlib, &firstLoad)) != NULL) {
            debug(DEBUG_INFO, PUMPKINOS, "dlib resource loaded (first %d)", firstLoad ? 1 : 0);
            pilot_main = sys_lib_defsymbol(lib, "PilotMain", 1);
            if (pilot_main == NULL) {
              debug(DEBUG_ERROR, PUMPKINOS, "PilotMain not found in dlib");
            }
          } else {
            debug(DEBUG_INFO, PUMPKINOS, "dlib resource not loaded");
            if ((h = DmGet1Resource(pumpkin_script_engine_id(), 1)) != NULL) {
              pilot_main = pumpkin_script_main;
              opendb = 1;
              DmReleaseResource(h);
            }
          }
          DmCloseDatabase(dbRef);
        } else {
          debug(DEBUG_ERROR, PUMPKINOS, "error opening database '%s'", request->name);
        }
      } else {
        debug(DEBUG_ERROR, PUMPKINOS, "database '%s' not found", request->name);
      }
    }

    if (pilot_main) {
      pumpkin_set_m68k(0);
      if (opendb) {
        debug(DEBUG_INFO, PUMPKINOS, "calling pilot_main for \"%s\" with code %d as subroutine (opendb)", request->name, request->code);
        r = pumpkin_pilotmain(request->name, pilot_main, request->code, request->param, request->flags);
      } else {
        debug(DEBUG_INFO, PUMPKINOS, "calling pilot_main for \"%s\" with code %d as subroutine", request->name, request->code);
        r = pilot_main(request->code, request->param, request->flags);
      }
      debug(DEBUG_INFO, PUMPKINOS, "pilot_main returned %u", r);

    } else {
      m68k = pumpkin_is_m68k();
      pumpkin_set_m68k(1);
      if (opendb) {
        debug(DEBUG_INFO, PUMPKINOS, "calling emupalmos_main for \"%s\" with code %d as subroutine (opendb)", request->name, request->code);
        r = pumpkin_pilotmain(request->name, emupalmos_main, request->code, request->param, request->flags);
      } else {
        debug(DEBUG_INFO, PUMPKINOS, "calling emupalmos_main for \"%s\" with code %d as subroutine", request->name, request->code);
        r = emupalmos_main(request->code, request->param, request->flags);
      }
      debug(DEBUG_INFO, PUMPKINOS, "emupalmos_main returned %u", r);
      pumpkin_set_m68k(m68k);
    }

    if (lib) {
      sys_lib_close(lib);
    }
  }

  return r;
}

static int pumpkin_local_init(int i, texture_t *texture, char *name, int width, int height, int x, int y) {
  pumpkin_task_t *task;
  task_screen_t *screen;
  LocalID dbID;
  UInt32 language;
  uint32_t color;
  int j, ptr;

  if (mutex_lock(mutex) == -1) {
    return -1;
  }

  if ((task = xcalloc(1, sizeof(pumpkin_task_t))) == NULL) {
    pumpkin_module.tasks[i].reserved = 0;
    mutex_unlock(mutex);
    return -1;
  }

  if ((screen = xcalloc(1, sizeof(task_screen_t))) == NULL) {
    pumpkin_module.tasks[i].reserved = 0;
    xfree(task);
    mutex_unlock(mutex);
    return -1;
  }

  if ((screen->surface = surface_create(width, height, pumpkin_module.encoding)) == NULL) {
    pumpkin_module.tasks[i].reserved = 0;
    xfree(task);
    xfree(screen);
    mutex_unlock(mutex);
    return -1;
  }

  screen->x0 = pumpkin_module.width;
  screen->y0 = pumpkin_module.height;
  screen->x1 = -1;
  screen->y1 = -1;

  screen->tag = TAG_SCREEN;
  if ((ptr = ptr_new(screen, task_destructor)) == -1) {
    pumpkin_module.tasks[i].reserved = 0;
    surface_destroy(screen->surface);
    xfree(task);
    xfree(screen);
    mutex_unlock(mutex);
    return -1;
  }

  color = screen->surface->color_rgb(screen->surface->data, 255, 255, 255, 255);
  surface_rectangle(screen->surface, 0, 0, width-1, height-1, 1, color);

  pumpkin_module.tasks[i].taskId = pumpkin_module.nextTaskId++;
  pumpkin_module.tasks[i].task_index = i;
  pumpkin_module.tasks[i].active = 1;
  pumpkin_module.tasks[i].reserved = 0;
  pumpkin_module.tasks[i].screen_ptr = ptr;
  pumpkin_module.tasks[i].width = width;
  pumpkin_module.tasks[i].height = height;
  pumpkin_module.tasks[i].new_width = width;
  pumpkin_module.tasks[i].new_height = height;
  pumpkin_module.tasks[i].handle = thread_get_handle();
  pumpkin_module.tasks[i].texture = texture;
  pumpkin_module.tasks[i].eventKeyMask = 0xFFFFFF;
  sys_strncpy(pumpkin_module.tasks[i].name, name, dmDBNameLength-1);
  pumpkin_module.tasks[i].num_notifs = 0;

  pumpkin_module.task_order[pumpkin_module.num_tasks] = i;
  pumpkin_module.current_task = i;
  pumpkin_module.num_tasks++;

  task->taskId = pumpkin_module.tasks[i].taskId;
  task->task_index = i;
  task->active = 1;
  task->screen_ptr = ptr;
  task->width = width;
  task->height = height;
  task->new_width = width;
  task->new_height = height;
  sys_strncpy(task->name, name, dmDBNameLength-1);

  thread_set(task_key, task);
  if (!pumpkin_module.dia && !pumpkin_module.single) {
    task->heap = heap_init(HEAP_SIZE, NULL);
    StoInit(APP_STORAGE, pumpkin_module.fs_mutex);
  } else {
    task->heap = pumpkin_module.heap;
  }
  task->exception = pumpkin_heap_alloc(sizeof(void *), "Exception");

  for (j = 0; j < pumpkin_module.num_serial; j++) {
    task->serial[j] = pumpkin_heap_alloc(sys_strlen(pumpkin_module.serial[j].descr) + 1, "serial_descr");
    if (task->serial[j]) sys_strcpy(task->serial[j], pumpkin_module.serial[j].descr);
  }

  SysUInitModule();

  dbID = DmFindDatabase(0, BOOT_NAME);
  pumpkin_module.tasks[i].bootRef = DmOpenDatabase(0, dbID, dmModeReadOnly);

  language = PrefGetPreference(prefLanguage);
  task->lang = LanguageInit(language);

  UicInitModule();
  BmpInitModule(DEFAULT_DENSITY);
  WinInitModule(DEFAULT_DENSITY, pumpkin_module.tasks[i].width, pumpkin_module.tasks[i].height, 16, NULL);
  FntInitModule(DEFAULT_DENSITY);
  FrmInitModule();
  InsPtInitModule();
  FldInitModule();
  MenuInitModule();
  EvtInitModule();
  SysInitModule();
  GPSInitModule(pumpkin_module.gps_parse_line, pumpkin_module.bt);
  VFSInitModule(VFS_CARD);
  KeyboardInitModule();
  ClpInitModule();
  SrmInitModule();
  FtrInitModule();
  KeyInitModule();
  SndInitModule(pumpkin_module.ap);
  SelTimeInitModule();
  SysFatalAlertInit();

  if (pumpkin_module.wm) {
    wman_add(pumpkin_module.wm, pumpkin_module.tasks[i].taskId, pumpkin_module.tasks[i].texture, x, y, width, height);
    pumpkin_module.render = 1;
  }

  mutex_unlock(mutex);

  pumpkin_forward_event(0, MSG_KEY, WINDOW_KEY_CUSTOM, vchrAppStarted, 0);

  return 0;
}

void pumpkin_local_refresh(void) {
  StoRefresh();
}

static int pumpkin_local_finish(UInt32 creator) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  AppRegistryPosition p;
  int i, x, y;

  if (mutex_lock(mutex) == -1) {
    return -1;
  }

  if (task->lang) LanguageFinish(task->lang);

  if (pumpkin_module.wm) {
    if (creator) {
      if (wman_xy(pumpkin_module.wm, pumpkin_module.tasks[task->task_index].taskId, &x, &y) == 0) {
        p.x = x;
        p.y = y;
        AppRegistrySet(pumpkin_module.registry, creator, appRegistryPosition, 0, &p);
      }
    }
    wman_remove(pumpkin_module.wm, pumpkin_module.tasks[task->task_index].taskId, 1);
    pumpkin_module.render = 1;
  }

  SysFatalAlertFinish();

  pumpkin_module.tasks[task->task_index].task_index = 0;
  pumpkin_module.tasks[task->task_index].active = 0;
  pumpkin_module.tasks[task->task_index].screen_ptr = 0;

  for (i = 0; i < pumpkin_module.num_tasks; i++) {
    if (pumpkin_module.task_order[i] == task->task_index) {
      pumpkin_module.task_order[i] = 0;
      break;
    }
  }

  pumpkin_module.num_tasks--;

  for (; i < pumpkin_module.num_tasks; i++) {
    pumpkin_module.task_order[i] = pumpkin_module.task_order[i+1];
  }

  if (task->task_index == pumpkin_module.current_task) {
    if (pumpkin_module.num_tasks) {
      pumpkin_make_current(0);
    }
  }

  ptr_free(task->screen_ptr, TAG_SCREEN);
  pumpkin_heap_free(task->exception, "Exception");

  SelTimeFinishModule();
  SndFinishModule();
  KeyFinishModule();
  FtrFinishModule();
  SrmFinishModule();
  ClpFinishModule();
  KeyboardFinishModule();
  VFSFinishModule();
  GPSFinishModule();
  SysFinishModule();
  if (!pumpkin_module.dia && !pumpkin_module.single) {
    SysUFinishModule();
  }
  EvtFinishModule();
  MenuFinishModule();
  FldFinishModule();
  InsPtFinishModule();
  FrmFinishModule();
  UicFinishModule();
  FntFinishModule();
  WinFinishModule(true);
  BmpFinishModule();

  if (pumpkin_module.tasks[task->task_index].bootRef) {
    DmCloseDatabase(pumpkin_module.tasks[task->task_index].bootRef);
    pumpkin_module.tasks[task->task_index].bootRef = NULL;
  }

  for (i = 0; i < pumpkin_module.num_tasks; i++) {
    if (task->serial[i]) pumpkin_heap_free(task->serial[i], "serial_descr");
  }

  if (!pumpkin_module.dia && !pumpkin_module.single) {
    StoFinish();
    heap_finish(task->heap);
  }

  thread_set(task_key, NULL);
  xfree(task);

  if (pumpkin_module.num_tasks == 0) {
    debug(DEBUG_INFO, PUMPKINOS, "last task finishing");
    sys_set_finish(0);
  }

  mutex_unlock(mutex);

  pumpkin_forward_event(0, MSG_KEY, WINDOW_KEY_CUSTOM, vchrAppFinished, 0);

  return 0;
}

void pumpkin_set_data(void *data) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  task->data = data;
}

void *pumpkin_get_data(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task->data;
}

// called directly by libos if DIA or single app mode
int pumpkin_launcher(char *name, int width, int height) {
  LocalID dbID;
  UInt32 creator;
  pumpkin_task_t *task;
  texture_t *texture;
  launch_request_t request;

  texture = pumpkin_module.wp->create_texture(pumpkin_module.w, width, height);

  if (pumpkin_local_init(0, texture, name, width, height, pumpkin_module.border, pumpkin_module.border) == 0) {
    dbID = DmFindDatabase(0, name);
    DmDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creator);
    pumpkin_set_compat(creator, appCompatOk, 0);

    task = (pumpkin_task_t *)thread_get(task_key);
    if (ErrSetJump(task->jmpbuf) != 0) {
      debug(DEBUG_ERROR, PUMPKINOS, "ErrSetJump not zero");
    } else {
      MemSet(&request, sizeof(launch_request_t), 0);
#ifdef ANDROID
      extern UInt32 LauncherPilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);
      request.pilot_main = LauncherPilotMain;
#endif
      StrNCopy(request.name, name, dmDBNameLength);
      request.code = sysAppLaunchCmdNormalLaunch;
      pumpkin_launch_sub(&request, 1);
    }

    pumpkin_local_finish(creator);
  }

  return 0;
}

static int pumpkin_launch_action(void *arg) {
  pumpkin_task_t *task;
  launch_data_t *data;
  launch_request_t launch;
  char name[dmDBNameLength];
  int i, cont = 0;

  data = (launch_data_t *)arg;
  debug(DEBUG_INFO, PUMPKINOS, "starting \"%s\"", data->request.name);

  sys_strncpy(name, data->request.name, dmDBNameLength);
  for (i = 0; name[i]; i++) {
    if (name[i] <= 32) name[i] = '_';
  }
  thread_set_name(name);

  if (pumpkin_local_init(data->index, data->texture, data->request.name, data->width, data->height, data->x, data->y) == 0) {
    task = (pumpkin_task_t *)thread_get(task_key);
    pumpkin_set_compat(data->creator, appCompatOk, 0);
    if (ErrSetJump(task->jmpbuf) != 0) {
      debug(DEBUG_ERROR, PUMPKINOS, "ErrSetJump not zero");
      pumpkin_forward_event(0, MSG_KEY, WINDOW_KEY_CUSTOM, vchrAppCrashed, 0);
    } else {
      pumpkin_launch_sub(&data->request, data->request.opendb);
      cont = SysUIAppSwitchCont(&launch);
    }
    pumpkin_local_finish(data->creator);
  }

  if (data->request.code == sysAppLaunchCmdGoTo && data->request.param) {
    pumpkin_heap_free(data->request.param, "GoToParams");
  }

  thread_set_name(TAG_APP);
  debug(DEBUG_INFO, PUMPKINOS, "thread exiting");
  xfree(data);

  if (cont) {
    debug(DEBUG_INFO, PUMPKINOS, "switching to \"%s\"", launch.name);
    pumpkin_launch_request(launch.name, launch.code, launch.param, launch.flags, NULL, 1);
  }

  return 0;
}

static int pumpkin_wait_ack(int port, uint32_t *reply) {
  uint8_t *buf;
  unsigned int len;
  uint32_t *p;
  int ack, client, i, r = -1;

  for (i = 0, ack = 0; i < 10 && !ack; i++) {
    if ((r = thread_server_read_timeout_from(100000, &buf, &len, &client)) == -1) {
      debug(DEBUG_ERROR, PUMPKINOS, "error receiving ack from %d", port);
      break;
    }
    if (r == 0) {
      r = -1;
      continue;
    }
    r = -1;

    if (buf) {
      if (client == port) {
        ack = 1;
        if (len == sizeof(uint32_t)) {
          p = (uint32_t *)buf;
          if (reply) *reply = *p;
          r = 0;
          debug(DEBUG_INFO, PUMPKINOS, "reply %u from %d", *p, port);
        } else {
          debug(DEBUG_ERROR, PUMPKINOS, "received %d bytes from %d but was expecting %d bytes", len, port, sizeof(uint32_t));
        }
      } else {
        if (len == sizeof(uint32_t)) {
          p = (uint32_t *)buf;
          debug(DEBUG_ERROR, PUMPKINOS, "received reply %u from %d but was expecting %d", p, client, port);
        } else {
          debug(DEBUG_ERROR, PUMPKINOS, "received reply from %d but was expecting %d", client, port);
        }
      }
      xfree(buf);
    } else {
      debug(DEBUG_ERROR, PUMPKINOS, "received nothing from %d but was expecting %d bytes", port, sizeof(uint32_t));
    }
  }

  return r;
}

int pumpkin_launch(launch_request_t *request) {
  LocalID dbID;
  AppRegistrySize s;
  AppRegistryPosition p;
  UInt32 creator;
  launch_data_t *data;
  client_request_t creq;
  int i, running, index, wait_ack, r = -1;

  running = -1;
  index = -1;

  if (mutex_lock(mutex) == 0) {
    for (i = 0; i < MAX_TASKS; i++) {
      if (pumpkin_module.tasks[i].active || pumpkin_module.tasks[i].reserved) {
        if (!sys_strncmp(pumpkin_module.tasks[i].name, request->name, dmDBNameLength-1)) {
          if (!(request->flags & sysAppLaunchFlagFork)) {
            running = i;
            break;
          }
        }
      } else {
        if (pumpkin_module.tasks[i].texture) {
          pumpkin_module.wp->destroy_texture(pumpkin_module.w, pumpkin_module.tasks[i].texture);
          pumpkin_module.tasks[i].texture = NULL;
        }
        if (index == -1) {
          debug(DEBUG_INFO, PUMPKINOS, "task %d reserved for application \"%s\"", i, request->name);
          sys_strncpy(pumpkin_module.tasks[i].name, request->name, dmDBNameLength-1);
          pumpkin_module.tasks[i].reserved = 1;
          index = i;
        }
      }
    }

    if (running != -1) {
      debug(DEBUG_INFO, PUMPKINOS, "application \"%s\" is already running", request->name);
      if (index != -1) {
        pumpkin_module.tasks[index].reserved = 0;
      }
      wait_ack = -1;
      if (request->code == sysAppLaunchCmdNormalLaunch) {
        // if application is already running and launch code is sysAppLaunchCmdNormalLaunch, just make the application current
        pumpkin_make_current(running);
      } else if (request->code == sysAppLaunchCmdGoTo) {
        // if application is already running and launch code is sysAppLaunchCmdGoTo, send request to it
        xmemset(&creq, 0, sizeof(client_request_t));
        creq.type = MSG_LAUNCH;
        xmemcpy(&creq.data.launch, request, sizeof(launch_request_t));
        if (thread_client_write(pumpkin_module.tasks[running].handle, (uint8_t *)&creq, sizeof(client_request_t)) == sizeof(client_request_t)) {
          wait_ack = pumpkin_module.tasks[running].handle;
        }
      }
      mutex_unlock(mutex);
      if (wait_ack != -1) {
        debug(DEBUG_INFO, PUMPKINOS, "waiting ack from \"%s\"", request->name);
        if (pumpkin_wait_ack(wait_ack, NULL) == 0) {
          if (request->param) {
            pumpkin_heap_free(request->param, "request_param");
          }
        }
      }
      return 0;
    }

    if (index == -1) {
      debug(DEBUG_ERROR, PUMPKINOS, "no more threads");
      mutex_unlock(mutex);
      return -1;
    }

    if ((data = xcalloc(1, sizeof (launch_data_t))) != NULL) {
      dbID = DmFindDatabase(0, request->name);
      DmDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creator);
      data->creator = creator;
      data->width = APP_SCREEN_WIDTH;
      data->height = APP_SCREEN_HEIGHT;

      if (!pumpkin_module.dia && !pumpkin_module.single) {
        data->x = (pumpkin_module.width - data->width) / 2;
        data->y = (pumpkin_module.height - data->height) / 2;

        if (AppRegistryGet(pumpkin_module.registry, creator, appRegistrySize, 0, &s)) {
          data->width = s.width;
          data->height = s.height;
          debug(DEBUG_INFO, PUMPKINOS, "using size %dx%d from registry", data->width, data->height);
        }
        if (AppRegistryGet(pumpkin_module.registry, creator, appRegistryPosition, 0, &p)) {
          data->x = p.x;
          data->y = p.y;
          debug(DEBUG_INFO, PUMPKINOS, "using position %d,%d from registry", data->x, data->y);
        }
      }

      if (data->width == 0 || data->height == 0) {
        data->width = pumpkin_module.width;
        data->height = pumpkin_module.height;
      }
      if (data->width >= pumpkin_module.width || data->height >= pumpkin_module.height) {
        //data->width = pumpkin_module.width;
        //data->height = pumpkin_module.height;
        if (pumpkin_module.dia) data->height -= (DEFAULT_DENSITY == kDensityDouble) ? 160 : 80; // XXX
      }

      data->index = index;
      xmemcpy(&data->request, request, sizeof(launch_request_t));
      data->texture = pumpkin_module.wp->create_texture(pumpkin_module.w, data->width, data->height);
      debug(DEBUG_INFO, PUMPKINOS, "starting \"%s\" with launchCode %d", request->name, request->code);
      r = thread_begin(TAG_APP, pumpkin_launch_action, data);
      if (r == -1) {
        pumpkin_module.tasks[index].reserved = 0;
      }
    }

    mutex_unlock(mutex);
  }

  return r;
}

static void pumpkin_launched(int launched) {
  if (mutex_lock(mutex) == 0) {
    pumpkin_module.launched = launched;
    mutex_unlock(mutex);
  }
}

int pumpkin_is_launched(void) {
  int r = 0;

  if (mutex_lock(mutex) == 0) {
    r = (pumpkin_module.dia || pumpkin_module.single) ? pumpkin_module.launched : 1;
    mutex_unlock(mutex);
  }

  return r;
}

static int pumpkin_pause_task(char *name, int *call_sub) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  int i, handle = -1;
  uint32_t msg;

  if (name && task && !sys_strcmp(name, task->name)) {
    // not pausing itself...
    debug(DEBUG_INFO, PUMPKINOS, "no need to pause myself");
    *call_sub = 1;

  } else {
    *call_sub = 0;

    if (mutex_lock(mutex) == 0) {
      for (i = 0; i < MAX_TASKS; i++) {
        if (pumpkin_module.tasks[i].active) {
          if (!sys_strncmp(pumpkin_module.tasks[i].name, name, dmDBNameLength-1)) {
            handle = pumpkin_module.tasks[i].handle;
            break;
          }
        }
      }

      if (handle > 0) {
        debug(DEBUG_INFO, PUMPKINOS, "sending pause request to \"%s\" on port %d", name, handle);
        msg = MSG_PAUSE;
        if (thread_client_write(handle, (uint8_t *)&msg, sizeof(uint32_t)) == sizeof(uint32_t)) {
          *call_sub = 1;
        }
      } else {
        *call_sub = 1;
      }
      mutex_unlock(mutex);

      if (handle > 0) {
        debug(DEBUG_INFO, PUMPKINOS, "waiting pause reply from \"%s\"", name);
        if (pumpkin_wait_ack(handle, NULL) == -1) {
          debug(DEBUG_ERROR, PUMPKINOS, "pause reply error");
          *call_sub = 0;
          handle = -1;
        } else {
          debug(DEBUG_INFO, PUMPKINOS, "pause reply received");
        }
      }
    }
  }

  return handle;
}

static int pumpkin_resume_task(int handle) {
  uint32_t msg;
  int r = 0;

  if (handle > 0) {
    debug(DEBUG_INFO, PUMPKINOS, "sending resume request to port %d", handle);
    msg = MSG_RESUME;
    r = thread_client_write(handle, (uint8_t *)&msg, sizeof(uint32_t));
  }

  return r;
}

uint32_t pumpkin_launch_request(char *name, UInt16 cmd, UInt8 *param, UInt16 flags, PilotMainF pilotMain, UInt16 opendb) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  client_request_t creq;
  GoToParamsType *gotoParam;
  ErrJumpBuf jmpbuf;
  void *win_module;
  void *frm_module;
  void *fld_module;
  void *menu_module;
  int handle, call_sub;
  uint32_t r = 0;
  void *data;

  xmemset(&creq, 0, sizeof(client_request_t));
  creq.type = MSG_LAUNCH;
  sys_strncpy(creq.data.launch.name, name, dmDBNameLength-1);
  creq.data.launch.code = cmd;
  creq.data.launch.flags = flags;
  creq.data.launch.param = param;
  creq.data.launch.pilot_main = pilotMain;
  creq.data.launch.opendb = opendb;

  switch (cmd) {
    case sysAppLaunchCmdNormalLaunch:
      break;
    case sysAppLaunchCmdGoTo:
      if ((gotoParam = pumpkin_heap_alloc(sizeof(GoToParamsType), "GoToParams")) != NULL) {
        xmemcpy(gotoParam, param, sizeof(GoToParamsType));
      }
      creq.data.launch.param = gotoParam;
      break;
    default:
      debug(DEBUG_INFO, PUMPKINOS, "sending launch code %d to \"%s\"", cmd, name);
      handle = pumpkin_pause_task(name, &call_sub);
      if (call_sub) {
        debug(DEBUG_INFO, PUMPKINOS, "calling \"%s\" as subroutine", name);
        creq.data.launch.flags |= sysAppLaunchFlagSubCall;
        r = pumpkin_launch_sub(&creq.data.launch, 1);
      }
      if (handle > 0) {
        pumpkin_resume_task(handle);
      }
      return r;
  }

  if (thread_get_handle() == pumpkin_module.spawner) {
    debug(DEBUG_INFO, PUMPKINOS, "spawning \"%s\" with launchCode %d", name, cmd);
    win_module = WinReinitModule(NULL);
    frm_module = FrmReinitModule(NULL);
    fld_module = FldReinitModule(NULL);
    menu_module = MenuReinitModule(NULL);
    UIColorPushTable();
    pumpkin_launched(1);
    xmemcpy(jmpbuf, task->jmpbuf, sizeof(ErrJumpBuf));
    if (ErrSetJump(task->jmpbuf) != 0) {
      debug(DEBUG_ERROR, PUMPKINOS, "ErrSetJump not zero");
    } else {
      data = pumpkin_get_data();
      pumpkin_set_data(NULL);
      r = pumpkin_launch_sub(&creq.data.launch, 1);
      pumpkin_set_data(data);
    }
    xmemcpy(task->jmpbuf, jmpbuf, sizeof(ErrJumpBuf));
    pumpkin_set_m68k(0);
    pumpkin_launched(0);
    UIColorPopTable();
    MenuReinitModule(menu_module);
    FldReinitModule(fld_module);
    FrmReinitModule(frm_module);
    WinReinitModule(win_module);
    FrmSetDIAPolicyAttr(FrmGetActiveForm(), frmDIAPolicyCustom);
    PINSetInputTriggerState(pinInputTriggerEnabled);
    FrmDrawForm(FrmGetActiveForm());
  } else {
    debug(DEBUG_INFO, PUMPKINOS, "sending launch code %d to \"%s\" via spawner on port %d", cmd, name, pumpkin_module.spawner);
    thread_client_write(pumpkin_module.spawner, (uint8_t *)&creq, sizeof(client_request_t));
  }

  return r;
}

uint32_t pumpkin_fork(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  return pumpkin_launch_request(task->name, sysAppLaunchCmdNormalLaunch, NULL, sysAppLaunchFlagFork, NULL, 1);
}

/*
  switch (cmd) {
    case sysAppLaunchCmdNormalLaunch:
      len = 0;
      break;
    case sysAppLaunchCmdFind:
      len = sizeof(FindParamsType);
      break;
    case sysAppLaunchCmdGoTo:
      len = sizeof(GoToParamsType);
      break;
    case sysAppLaunchCmdSyncNotify:
      len = 0;
      break;
    case sysAppLaunchCmdTimeChange:
      len = 0;
      break;
    case sysAppLaunchCmdSystemReset:
      len = sizeof(SysAppLaunchCmdSystemResetType);
      break;
    case sysAppLaunchCmdAlarmTriggered:
      len = sizeof(SysAlarmTriggeredParamType);
      break;
    case sysAppLaunchCmdDisplayAlarm:
      len = sizeof(SysDisplayAlarmParamType);
      break;
    case sysAppLaunchCmdCountryChange:
      len = 0;
      break;
    case sysAppLaunchCmdSyncRequest:
    case sysAppLaunchCmdSaveData:
    case sysAppLaunchCmdInitDatabase:
    case sysAppLaunchCmdSyncCallApplicationV10:
    case sysAppLaunchCmdPanelCalledFromApp:
    case sysAppLaunchCmdReturnFromPanel:
    case sysAppLaunchCmdLookup:
    case sysAppLaunchCmdSystemLock:
    case sysAppLaunchCmdSyncRequestRemote:
    case sysAppLaunchCmdHandleSyncCallApp:
    case sysAppLaunchCmdAddRecord:
    case sysSvcLaunchCmdSetServiceID:
    case sysSvcLaunchCmdGetServiceID:
    case sysSvcLaunchCmdGetServiceList:
    case sysSvcLaunchCmdGetServiceInfo:
    case sysAppLaunchCmdFailedAppNotify:
    case sysAppLaunchCmdEventHook:
    case sysAppLaunchCmdExgReceiveData:
    case sysAppLaunchCmdExgAskUser:
    case sysDialLaunchCmdDial:
    case sysDialLaunchCmdHangUp:
    case sysDialLaunchCmdLast:
    case sysSvcLaunchCmdGetQuickEditLabel:
    case sysSvcLaunchCmdLast:
    case sysAppLaunchCmdURLParams:
      break;
    case sysAppLaunchCmdNotify:
      len = sizeof(SysNotifyParamType);
      break;
    case sysAppLaunchCmdOpenDB:
    case sysAppLaunchCmdAntennaUp:
    case sysAppLaunchCmdGoToURL:
    case sysAppLaunchNppiNoUI:
    case sysAppLaunchNppiUI:
    case sysAppLaunchCmdExgPreview:
    case sysAppLaunchCmdCardLaunch:
    case sysAppLaunchCmdExgGetData:
    case sysAppLaunchCmdAttention:
    case sysAppLaunchPnpsPreLaunch:
    case sysAppLaunchCmdMultimediaEvent:
    case sysAppLaunchCmdFepPanelAddWord:
    case sysAppLaunchCmdLookupWord:
      break;
  }
*/

Err SysAppLaunch(UInt16 cardNo, LocalID dbID, UInt16 launchFlags, UInt16 cmd, MemPtr cmdPBP, UInt32 *resultP) {
  return SysAppLaunchEx(cardNo, dbID, launchFlags, cmd, cmdPBP, resultP, NULL);
}

Err SysAppLaunchEx(UInt16 cardNo, LocalID dbID, UInt16 launchFlags, UInt16 cmd, MemPtr cmdPBP, UInt32 *resultP, PilotMainF pilotMain) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  launch_request_t request;
  char name[dmDBNameLength];
  UInt32 type;
  int r = -1;

  debug(DEBUG_INFO, PUMPKINOS, "SysAppLaunch dbID 0x%08X flags 0x%04X cmd %d param %p", dbID, launchFlags, cmd, cmdPBP);

  if (DmDatabaseInfo(0, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, NULL) == errNone) {
    if (dbID != task->dbID) {
      *resultP = pumpkin_launch_request(name, cmd, cmdPBP, launchFlags, pilotMain, 1);
      r = 0;
    } else {
      debug(DEBUG_INFO, PUMPKINOS, "calling self with launchCode %d", cmd);
      xmemset(&request, 0, sizeof(launch_request_t));
      sys_strncpy(request.name, name, dmDBNameLength-1);
      request.code = cmd;
      request.flags = sysAppLaunchFlagSubCall;
      request.param = cmdPBP;
      request.pilot_main = pilotMain;
      request.opendb = 1;
      *resultP = pumpkin_launch_sub(&request, 0);
      r = 0;
    }
  }

  return (r == 0) ? errNone : sysErrParamErr;
}

int pumpkin_change_display(int width, int height) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  int r = -1;

  if (mutex_lock(mutex) == 0) {
    pumpkin_module.tasks[task->task_index].new_width = width;
    pumpkin_module.tasks[task->task_index].new_height = height;
    mutex_unlock(mutex);
    r = 0;
  }

  return r;
}

static int pumpkin_changed_display(pumpkin_task_t *task, task_screen_t *screen, int width, int height) {
  surface_t *surface;
  texture_t *texture, *old;
  int r = -1;

  if ((surface = surface_create(width, height, pumpkin_module.encoding)) != NULL) {
    surface_draw(surface, 0, 0, screen->surface, 0, 0, task->width, task->height);
    surface_destroy(screen->surface);
    screen->surface = surface;

    screen->x0 = 0;
    screen->y0 = 0;
    screen->x1 = width - 1;
    screen->y1 = height - 1;
    screen->dirty = 1;

    old = task->texture;
    if ((texture = pumpkin_module.wp->create_texture(pumpkin_module.w, width, height)) != NULL) {
      wman_remove(pumpkin_module.wm, task->taskId, 0);
      wman_texture(pumpkin_module.wm, task->taskId, texture, width, height);
      task->texture = texture;
      task->width = width;
      task->height = height;
      if (old) pumpkin_module.wp->destroy_texture(pumpkin_module.w, old);
      wman_raise(pumpkin_module.wm, task->taskId);
      r = 0;
    }
  }

  return r;
}

static int draw_task(int i, int *x, int *y, int *w, int *h) {
  task_screen_t *screen;
  uint8_t *raw;
  int width, height, len, updated = 0;

  if ((screen = ptr_lock(pumpkin_module.tasks[i].screen_ptr, TAG_SCREEN))) {
    if (pumpkin_module.dia) {
      if (dia_get_main_dimension(pumpkin_module.dia, &width, &height) == 0) {
        if (width != pumpkin_module.tasks[i].width || height != pumpkin_module.tasks[i].height) {
          debug(DEBUG_INFO, PUMPKINOS, "task %d (%s) display changed to %dx%d", i, pumpkin_module.tasks[i].name, width, height);
          if (pumpkin_changed_display(&pumpkin_module.tasks[i], screen, width, height) == 0) {
            pumpkin_forward_event(i, MSG_DISPLAY, width, height, 0);
          }
        }
      }
    } else if (pumpkin_module.tasks[i].width != pumpkin_module.tasks[i].new_width ||
               pumpkin_module.tasks[i].height != pumpkin_module.tasks[i].new_height) {
      width = pumpkin_module.tasks[i].new_width;
      height = pumpkin_module.tasks[i].new_height;
      debug(DEBUG_INFO, PUMPKINOS, "task %d (%s) display changed to %dx%d", i, pumpkin_module.tasks[i].name, width, height);
      if (pumpkin_changed_display(&pumpkin_module.tasks[i], screen, width, height) == 0) {
        pumpkin_forward_event(i, MSG_DISPLAY, width, height, 0);
      }
    }

    if (screen->dirty) {
      raw = (uint8_t *)screen->surface->getbuffer(screen->surface->data, &len);
      *x = screen->x0;
      *y = screen->y0;
      *w = screen->x1 - screen->x0 + 1;
      *h = screen->y1 - screen->y0 + 1;
      debug(DEBUG_TRACE, PUMPKINOS, "task %d (%s) update texture %d,%d %d,%d", i, pumpkin_module.tasks[i].name, *x, *y, *w, *h);
      pumpkin_module.wp->update_texture_rect(pumpkin_module.w, pumpkin_module.tasks[i].texture, raw, *x, *y, *w, *h);
      screen->x0 = pumpkin_module.width;
      screen->y0 = pumpkin_module.height;
      screen->x1 = -1;
      screen->y1 = -1;
      screen->dirty = 0;
      updated = 1;
    }
    ptr_unlock(pumpkin_module.tasks[i].screen_ptr, TAG_SCREEN);
  }

  return updated;
}

// XXX this can block if the pipe is full, which can happen if the task is not reading quite often
static void task_forward_event(int i, int ev, int a1, int a2, int a3) {
  unsigned char *buf;
  unsigned int len;
  uint32_t carg[4];

  carg[0] = ev;
  carg[1] = a1;
  carg[2] = a2;
  carg[3] = a3;
  buf = (unsigned char *)&carg[0];
  len = sizeof(uint32_t)*4;

  thread_client_write(pumpkin_module.tasks[i].handle, buf, len);
}

void pumpkin_forward_event(int i, int ev, int a1, int a2, int a3) {
  task_forward_event(i, ev, a1, a2, a3);
}

void grail_draw_stroke(int x1, int y1, int x2, int y2, int alpha) {
  dia_draw_stroke(pumpkin_module.dia, x1, y1, x2, y2, alpha);
}

static int task_clicked(int x, int y, int *tx, int *ty) {
  int id, i, j;

  id = wman_clicked(pumpkin_module.wm, x, y, tx, ty);
  if (id != -1) {
    for (i = pumpkin_module.num_tasks-1; i >= 0; i--) {
      j = pumpkin_module.task_order[i];
      if (pumpkin_module.tasks[j].taskId == id) {
        return j;
      }
    }
  }

  return -1;
}

int pumpkin_get_current(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task ? task->task_index+1 : 0;
}

static void pumpkin_make_current(int i) {
  int k;

  if (i != pumpkin_module.current_task) {
    pumpkin_module.current_task = i;

    for (k = 0; k < pumpkin_module.num_tasks; k++) {
      if (pumpkin_module.task_order[k] == i) break;
    }
    for (; k < pumpkin_module.num_tasks-1; k++) {
      pumpkin_module.task_order[k] = pumpkin_module.task_order[k+1];
    }
    pumpkin_module.task_order[k] = i;

    if (pumpkin_module.wm) {
      wman_raise(pumpkin_module.wm, pumpkin_module.tasks[i].taskId);
      pumpkin_forward_event(i, MSG_RAISE, 0, 0, 0);
      pumpkin_module.render = 1;
    }
  }
}

int pumpkin_pause(int pause) {
  int r = -1;

  if (mutex_lock(mutex) == 0) {
    if (pumpkin_module.paused != pause) {
      debug(DEBUG_INFO, PUMPKINOS, "pause %d", pause);
      pumpkin_module.paused = pause;
    }
    mutex_unlock(mutex);
    r = 0;
  }

  return r;
}

int pumpkin_is_paused(void) {
  int r = 0;

  if (mutex_lock(mutex) == 0) {
    r = pumpkin_module.paused;
    mutex_unlock(mutex);
  }

  return r;
}

void pumpkin_set_finish(int finish) {
  if (mutex_lock(mutex) == 0) {
    pumpkin_module.finish = finish;
    mutex_unlock(mutex);
  }
}

int pumpkin_must_finish(void) {
  int r = 0;

  if (mutex_lock(mutex) == 0) {
    r = pumpkin_module.finish;
    mutex_unlock(mutex);
  }

  return r;
}

static void pumpkin_set_key(int key) {
  switch (key) {
    case WINDOW_KEY_DOWN:  pumpkin_module.keyMask |= keyBitPageDown;   break;
    case WINDOW_KEY_UP:    pumpkin_module.keyMask |= keyBitPageUp;     break;
    case WINDOW_KEY_LEFT:  pumpkin_module.keyMask |= keyBitLeft;       break;
    case WINDOW_KEY_RIGHT: pumpkin_module.keyMask |= keyBitRight;      break;
    case WINDOW_KEY_F1:    pumpkin_module.keyMask |= keyBitHard1;      break;
    case WINDOW_KEY_F2:    pumpkin_module.keyMask |= keyBitHard2;      break;
    case WINDOW_KEY_F3:    pumpkin_module.keyMask |= keyBitHard3;      break;
    case WINDOW_KEY_F4:    pumpkin_module.keyMask |= keyBitHard4;      break;
    case WINDOW_KEY_SHIFT: pumpkin_module.modMask |= WINDOW_MOD_SHIFT; break;
    case WINDOW_KEY_CTRL:  pumpkin_module.modMask |= WINDOW_MOD_CTRL;  break;
    case WINDOW_KEY_RCTRL: pumpkin_module.modMask |= WINDOW_MOD_RCTRL; break;
    case WINDOW_KEY_LALT:  pumpkin_module.modMask |= WINDOW_MOD_LALT;  break;
    case WINDOW_KEY_RALT:  pumpkin_module.modMask |= WINDOW_MOD_RALT;  break;
    default:
      if (key >= 0 && key < 128) {
        pumpkin_module.extKeyMask[key >> 6] |= ((uint64_t)1) << (key & 63);
      }
      break;
  }
}

static int pumpkin_reset_key(int key) {
  switch (key) {
    case WINDOW_KEY_DOWN:  pumpkin_module.keyMask &= ~keyBitPageDown; break;
    case WINDOW_KEY_UP:    pumpkin_module.keyMask &= ~keyBitPageUp;   break;
    case WINDOW_KEY_LEFT:  pumpkin_module.keyMask &= ~keyBitLeft;     break;
    case WINDOW_KEY_RIGHT: pumpkin_module.keyMask &= ~keyBitRight;    break;
    case WINDOW_KEY_F1:    pumpkin_module.keyMask &= ~keyBitHard1;    break;
    case WINDOW_KEY_F2:    pumpkin_module.keyMask &= ~keyBitHard2;    break;
    case WINDOW_KEY_F3:    pumpkin_module.keyMask &= ~keyBitHard3;    break;
    case WINDOW_KEY_F4:    pumpkin_module.keyMask &= ~keyBitHard4;    break;
    case WINDOW_KEY_SHIFT: pumpkin_module.modMask &= ~WINDOW_MOD_SHIFT; key = 0; break;
    case WINDOW_KEY_CTRL:  pumpkin_module.modMask &= ~WINDOW_MOD_CTRL;  key = 0; break;
    case WINDOW_KEY_RCTRL: pumpkin_module.modMask &= ~WINDOW_MOD_RCTRL; key = 0; break;
    case WINDOW_KEY_LALT:  pumpkin_module.modMask &= ~WINDOW_MOD_LALT;  key = 0; break;
    case WINDOW_KEY_RALT:  pumpkin_module.modMask &= ~WINDOW_MOD_RALT;  key = 0; break;
    default:
      if (key >= 0 && key < 128) {
        pumpkin_module.extKeyMask[key >> 6] &= ~(((uint64_t)1) << (key & 63));
      }
      break;
  }

  return key;
}

int pumpkin_sys_event(void) {
  uint64_t now;
  int arg1, arg2, w, h;
  int i, j, x, y, tx, ty, ev, tmp;
  int paused, wait, r = -1;

  for (;;) {
    if (thread_must_end()) return -1;
    dbg_poll();
    paused = pumpkin_is_paused();
    wait = paused ? 100 : 1;
    ev = pumpkin_module.wp->event2(pumpkin_module.w, wait, &arg1, &arg2);
    if (ev == -1) return -1;
    if (!paused) break;
  }

  if (mutex_lock(mutex) == 0) {
    now = sys_get_clock();

    if ((now - pumpkin_module.lastUpdate) > 50000) {
      if (pumpkin_module.num_tasks > 0) {
        for (j = 0; j < pumpkin_module.num_tasks; j++) {
          i = pumpkin_module.task_order[j];
          if (draw_task(i, &x, &y, &w, &h) && pumpkin_module.wm) {
            wman_update(pumpkin_module.wm, pumpkin_module.tasks[i].taskId, x, y, w, h);
            pumpkin_module.render = 1;
          }
        }
        if (pumpkin_module.dragged) {
          for (j = 0; j < pumpkin_module.num_tasks; j++) {
            i = pumpkin_module.task_order[j];
            if (pumpkin_module.wm && (pumpkin_module.tasks[i].dx != 0 || pumpkin_module.tasks[i].dy != 0)) {
              wman_move(pumpkin_module.wm, pumpkin_module.tasks[i].taskId, pumpkin_module.tasks[i].dx, pumpkin_module.tasks[i].dy);
            }
            pumpkin_module.tasks[i].dx = 0;
            pumpkin_module.tasks[i].dy = 0;
          }
          pumpkin_module.dragged = 0;
          pumpkin_module.render = 1;
        }
      }

      if (pumpkin_module.dia && dia_update(pumpkin_module.dia)) {
        pumpkin_module.render = 1;
      }

      if (pumpkin_module.render) {
        if (pumpkin_module.wp->render) {
          pumpkin_module.wp->render(pumpkin_module.w);
        }
        pumpkin_module.render = 0;
      }
      pumpkin_module.lastUpdate = now;
    }

    i = pumpkin_module.current_task;

    if (ev) {
      debug(DEBUG_TRACE, PUMPKINOS, "event ev=%d arg1=%d arg2=%d", ev, arg1, arg2);
    }

    switch (ev) {
      case WINDOW_KEYDOWN:
        if (arg1 && i != -1 && pumpkin_module.tasks[i].active) {
          task_forward_event(i, MSG_KEYDOWN, arg1, 0, 0);
        }
        pumpkin_set_key(arg1);
        break;
      case WINDOW_KEYUP:
        if (arg1 && i != -1 && pumpkin_module.tasks[i].active) {
          task_forward_event(i, MSG_KEYUP, arg1, 0, 0);
        }
        arg1 = pumpkin_reset_key(arg1);
        if (arg1 && i != -1 && pumpkin_module.tasks[i].active) {
          uint32_t ok = 1;
          switch (arg1) {
            case WINDOW_KEY_DOWN: ok = pumpkin_module.tasks[i].eventKeyMask & keyBitPageDown; break;
            case WINDOW_KEY_UP:   ok = pumpkin_module.tasks[i].eventKeyMask & keyBitPageUp; break;
            case WINDOW_KEY_F1:   ok = pumpkin_module.tasks[i].eventKeyMask & keyBitHard1; break;
            case WINDOW_KEY_F2:   ok = pumpkin_module.tasks[i].eventKeyMask & keyBitHard2; break;
            case WINDOW_KEY_F3:   ok = pumpkin_module.tasks[i].eventKeyMask & keyBitHard3; break;
            case WINDOW_KEY_F4:   ok = pumpkin_module.tasks[i].eventKeyMask & keyBitHard4; break;
          }
          if (ok) {
            arg2 = pumpkin_module.modMask;
            task_forward_event(i, MSG_KEY, arg1, arg2, 0);
          }
        }
        break;
      case WINDOW_BUTTONDOWN:
        if (pumpkin_module.dia) {
          pumpkin_module.wp->status(pumpkin_module.w, &pumpkin_module.lastX, &pumpkin_module.lastY, &tmp);
          if (dia_clicked(pumpkin_module.dia, pumpkin_module.current_task, pumpkin_module.lastX, pumpkin_module.lastY, 1) == 0) break;
        }

        pumpkin_module.buttonMask |= arg1;

        i = task_clicked(pumpkin_module.lastX, pumpkin_module.lastY, &tx, &ty);

        if (arg1 == 2) {
          if (i != -1) {
            if (pumpkin_module.tasks[i].width < pumpkin_module.width) {
              pumpkin_module.dragging = i;
            }
            pumpkin_make_current(i);

          } else {
            pumpkin_module.current_task = -1;
          }
        } else if (arg1 == 1) {
          pumpkin_module.dragging = -1;
          if (i != -1 && i == pumpkin_module.current_task) {
            if (pumpkin_module.dia || pumpkin_module.single) {
              task_forward_event(i, MSG_MOTION, tx, ty, 0);
              pumpkin_module.tasks[i].penX = tx;
              pumpkin_module.tasks[i].penY = ty;
            }
            task_forward_event(i, MSG_BUTTON, 0, 0, 1);
          }
        }
        break;
      case WINDOW_BUTTONUP:
        if (pumpkin_module.dia) {
          pumpkin_module.wp->status(pumpkin_module.w, &pumpkin_module.lastX, &pumpkin_module.lastY, &tmp);
          if (dia_clicked(pumpkin_module.dia, pumpkin_module.current_task, pumpkin_module.lastX, pumpkin_module.lastY, 0) == 0) break;
        }

        pumpkin_module.buttonMask &= ~arg1;
        pumpkin_module.dragging = -1;

        if (arg1 == 1) {
          i = task_clicked(pumpkin_module.lastX, pumpkin_module.lastY, &tx, &ty);
          if (i != -1  && i == pumpkin_module.current_task) {
            task_forward_event(i, MSG_BUTTON, 0, 0, 0);
          }
        }
        break;
      case WINDOW_MOTION:
        x = arg1;
        y = arg2;

        if (pumpkin_module.dragging != -1) {
          pumpkin_module.tasks[pumpkin_module.dragging].dx += x - pumpkin_module.lastX;
          pumpkin_module.tasks[pumpkin_module.dragging].dy += y - pumpkin_module.lastY;
          pumpkin_module.dragged = 1;

        } else {
          if (i != -1 && pumpkin_module.tasks[i].active) {
            if (!pumpkin_module.dia || !dia_stroke(pumpkin_module.dia, x, y)) {
              if (wman_xy(pumpkin_module.wm, pumpkin_module.tasks[i].taskId, &tx, &ty) == 0) {
                if (x >= tx && x < tx + pumpkin_module.tasks[i].width &&
                    y >= ty && y < ty + pumpkin_module.tasks[i].height) {
                  task_forward_event(i, MSG_MOTION, x - tx, y - ty, 0);
                  pumpkin_module.tasks[i].penX = x - tx;
                  pumpkin_module.tasks[i].penY = y - ty;
                }
              }
            }
          }
        }

        pumpkin_module.lastX = x;
        pumpkin_module.lastY = y;
        break;
    }

    mutex_unlock(mutex);
    r = 0;
  }

  return r;
}

void pumpkin_keymask(uint32_t keyMask) {
  int i;

  if (mutex_lock(mutex) == 0) {
    i = pumpkin_module.current_task;
    if (i != -1) {
      pumpkin_module.tasks[i].eventKeyMask = keyMask;
    }
    mutex_unlock(mutex);
  }
}

void pumpkin_status(int *x, int *y, uint32_t *keyMask, uint32_t *modMask, uint32_t *buttonMask, uint64_t *extKeyMask) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  int i;

  if (thread_get_handle() == pumpkin_module.spawner) {
    if (pumpkin_sys_event() == -1) {
      pumpkin_set_finish(1);
      sys_set_finish(1);
    }
  }

  if (x) *x = 0;
  if (y) *y = 0;
  if (keyMask) *keyMask = 0;
  if (modMask) *modMask = 0;
  if (buttonMask) *buttonMask = 0;
  if (extKeyMask) {
    extKeyMask[0] = 0;
    extKeyMask[1] = 0;
  }

  if (mutex_lock(mutex) == 0) {
    i = pumpkin_module.current_task;
    if (i != -1 && i == task->task_index) {
      if (x) *x = pumpkin_module.tasks[i].penX;
      if (y) *y = pumpkin_module.tasks[i].penY;
      if (keyMask) *keyMask = pumpkin_module.keyMask;
      if (modMask) *modMask = pumpkin_module.modMask;
      if (buttonMask) *buttonMask = pumpkin_module.buttonMask;
      if (extKeyMask) {
        extKeyMask[0] = pumpkin_module.extKeyMask[0];
        extKeyMask[1] = pumpkin_module.extKeyMask[1];
      }
    }
    mutex_unlock(mutex);
  }
}

int pumpkin_extkey_down(int key, uint64_t *extKeyMask) {
  int r = 0;

  if (key >= 0 && key < 128 && extKeyMask) {
    r = (extKeyMask[key >> 6] & ((uint64_t)1) << (key & 63)) ? 1 : 0;
  }

  return r;
}

int pumpkin_event_peek(void) {
  return thread_server_peek();
}

int pumpkin_event(int *key, int *mods, int *buttons, uint8_t *data, uint32_t *n, uint32_t usec) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  client_request_t *creq;
  EventType event;
  UInt16 prev;
  unsigned char *buf;
  unsigned int len;
  uint32_t *arg, reply;
  int r, client, ev = 0;

  if (thread_get_handle() == pumpkin_module.spawner) {
    if (pumpkin_sys_event() == -1) {
      pumpkin_set_finish(1);
      sys_set_finish(1);
    }
  }
  pumpkin_alarm_check();

  if ((r = thread_server_read_timeout_from(usec, &buf, &len, &client)) == 1) {
    arg = (uint32_t *)buf;

    if (len >= sizeof(uint32_t)) {
      ev = arg[0];
      switch (ev) {
        case MSG_KEY:
        case MSG_KEYDOWN:
        case MSG_KEYUP:
        case MSG_MOTION:
        case MSG_BUTTON:
          if (len == sizeof(uint32_t)*4) {
            *key = arg[1];
            *mods = arg[2];
            *buttons = arg[3];
          } else {
            ev = 0;
          }
          break;
        case MSG_LAUNCH:
          debug(DEBUG_INFO, PUMPKINOS, "pumpkin_event received launch request from %d", client);
          if (len == sizeof(client_request_t)) {
            creq = (client_request_t *)buf;
            reply = pumpkin_launch_sub(&creq->data.launch, 0);
            debug(DEBUG_INFO, PUMPKINOS, "pumpkin_event sending reply %u to %d", reply, client);
            thread_client_write(client, (uint8_t *)&reply, sizeof(uint32_t));
          }
          break;
        case MSG_PAUSE:
          debug(DEBUG_INFO, PUMPKINOS, "pumpkin_event pausing");
          task->paused = client;
          reply = 0;
          thread_client_write(client, (uint8_t *)&reply, sizeof(uint32_t));
          break;
        case MSG_DISPLAY:
          debug(DEBUG_INFO, PUMPKINOS, "pumpkin_event change display to %dx%d", arg[1], arg[2]);
          task->width = arg[1];
          task->height = arg[2];

          prev = WinSetCoordinateSystem(kCoordinatesDouble);
          WinSetDisplayExtent(arg[1], arg[2]);
          WinSetCoordinateSystem(prev);

          MemSet(&event, sizeof(EventType), 0);
          event.eType = winDisplayChangedEvent;
          RctSetRectangle(&event.data.winDisplayChanged.newBounds, 0, 0, arg[1], arg[2]);
          EvtAddEventToQueue(&event);
          break;
        case MSG_RAISE:
          debug(DEBUG_INFO, PUMPKINOS, "pumpkin_event raise");
          MemSet(&event, sizeof(EventType), 0);
          event.eType = appRaiseEvent;
          EvtAddEventToQueue(&event);
          break;
        default:
          len -= sizeof(uint32_t);
          if (len <= *n) {
            *n = len;
          } else {
            debug(DEBUG_ERROR, PUMPKINOS, "pumpkin_event event %u len %u > %u", ev, len, *n);
            len = *n;
          }
          if (data) xmemcpy(data, &arg[1], len);
          break;
      }
    } else {
      debug(DEBUG_ERROR, PUMPKINOS, "pumpkin_event invalid len %u", len);
    }
    if (buf) xfree(buf);

  } else if (r == -1) {
    ev = -1;
  }

  while (task->paused) {
    if ((r = thread_server_read_timeout_from(10000, &buf, &len, &client)) == -1) break;
    if (r == 0) continue;
    if (buf == NULL) continue;
    if (client != task->paused) {
      xfree(buf);
      continue;
    }
    arg = (uint32_t *)buf;
    if (len != sizeof(uint32_t) || *arg != MSG_RESUME) {
      xfree(buf);
      continue;
    }
    debug(DEBUG_INFO, PUMPKINOS, "pumpkin_event resuming");
    task->paused = 0;
  }

  return ev;
}

uint32_t pumpkin_get_taskid(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task->taskId;
}

LocalID pumpkin_get_app_localid(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task->dbID;
}

UInt32 pumpkin_get_app_creator(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task ? task->creator : 0;
}

int pumpkin_script_global_function(int pe, char *name, int (*f)(int pe)) {
  script_arg_t value;
  int r = -1;

  if (pe > 0) {
    value.type = SCRIPT_ARG_FUNCTION;
    value.value.r = script_create_function(pe, f);
    r = script_global_set(pe, name, &value);
  }

  return r;
}

int pumpkin_script_global_function_data(int pe, char *name, int (*f)(int pe, void *data), void *data) {
  script_arg_t value;
  int r = -1;

  if (pe > 0) {
    value.type = SCRIPT_ARG_FUNCTION;
    value.value.r = script_create_function_data(pe, f, data);
    r = script_global_set(pe, name, &value);
  }

  return r;
}

int pumpkin_script_obj_function(int pe, script_ref_t obj, char *name, int (*f)(int pe)) {
  int r = -1;

  if (pe > 0 && obj > 0) {
    r = script_add_function(pe, obj, name, f);
  }

  return r;
}

int pumpkin_script_global_iconst(int pe, char *name, int n) {
  script_arg_t value;
  int r = -1;

  if (pe > 0) {
    value.type = SCRIPT_ARG_INTEGER;
    value.value.i = n;
    r = script_global_set(pe, name, &value);
  }

  return r;
}

int pumpkin_script_global_iconst_value(int pe, char *name) {
  script_arg_t value;
  int val = 0;

  if (pe > 0) {
    if (script_global_get(pe, name, &value) == 0) {
      if (value.type == SCRIPT_ARG_INTEGER) {
        val = value.value.i;
      }
    }
  }

  return val;
}

void *pumpkin_script_global_pointer_value(int pe, char *name) {
  script_arg_t value;
  void *val = NULL;

  if (pe > 0) {
    if (script_global_get(pe, name, &value) == 0) {
      if (value.type == SCRIPT_ARG_POINTER) {
        val = value.value.p;
      }
    }
  }

  return val;
}

int pumpkin_script_obj_boolean(int pe, script_ref_t obj, char *name, int value) {
  int r = -1;

  if (pe > 0 && obj > 0) {
    r = script_add_boolean(pe, obj, name, value);
  }

  return r;
}

int pumpkin_script_obj_iconst(int pe, script_ref_t obj, char *name, int value) {
  int r = -1;

  if (pe > 0 && obj > 0) {
    r = script_add_iconst(pe, obj, name, value);
  }

  return r;
}

int pumpkin_script_obj_sconst(int pe, script_ref_t obj, char *name, char *value) {
  int r = -1;

  if (pe > 0 && obj > 0) {
    r = script_add_sconst(pe, obj, name, value);
  }

  return r;
}

int pumpkin_script_create_obj(int pe, char *name) {
  script_arg_t value;
  int r = -1;

  if (pe > 0) {
    r = script_create_object(pe);
    if (name) {
      value.type = SCRIPT_ARG_OBJECT;
      value.value.r = r;
      if (script_global_set(pe, name, &value) != 0) {
        r = -1;
      }
    }
  }

  return r;
}

int pumpkin_script_create(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  script_arg_t value;
  script_ref_t obj, pe = -1;

  debug(DEBUG_TRACE, PUMPKINOS, "creating script environment");
  if ((pe = script_create(pumpkin_module.engine)) != -1) {
    value.type = SCRIPT_ARG_OBJECT;
    if ((value.value.r = script_create_object(pe)) != -1 && script_global_set(pe, "app", &value) != -1) {
      obj = value.value.r;
      if (task) {
        script_add_iconst(pe, obj, "width", task->width);
        script_add_iconst(pe, obj, "height", task->height);
      }
    } else {
      script_destroy(pe);
      pe = -1;
    }
  } else {
    pe = -1;
  }

  return pe;
}

uint32_t pumpkin_script_engine_id(void) {
  return script_engine_id(pumpkin_module.engine);
}

int pumpkin_script_init(int pe, uint32_t type, uint16_t id) {
  MemHandle h;
  UInt32 len;
  char *s, *p;
  int r = -1;

  if (pe > 0) {
    if ((h = DmGet1Resource(type, id)) != 0) {
      if ((s = MemHandleLock(h)) != NULL) {
        len = MemHandleSize(h);
        if ((p = MemPtrNew(len+1)) != NULL) {
          MemMove(p, s, len);
          debug(DEBUG_TRACE, PUMPKINOS, "running script resource");
          r = script_run(pe, p, 0, NULL, 1);
          MemPtrFree(p);
        }
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    }
  }

  return r;
}

int pumpkin_script_destroy(int pe) {
  int r = -1;

  if (pe > 0) {
    debug(DEBUG_TRACE, PUMPKINOS, "destroying script environment");
    script_destroy(pe);
    r = 0;
  }

  return r;
}

int pumpkin_getstr(char **s, uint8_t *p, int i) {
  *s = (char *)&p[i];
  return sys_strlen(*s) + 1;
}

void pumpkin_screen_copy(uint16_t *src, uint16_t y0, uint16_t y1) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  task_screen_t *screen;
  uint32_t offset, size;
  uint16_t *dst;
  int len;

  if ((screen = ptr_lock(task->screen_ptr, TAG_SCREEN))) {
    dst = surface_buffer(screen->surface, &len);
    offset = y0 * task->width;
    size = (y1 - y0) * task->width * sizeof(uint16_t);
    sys_memcpy(dst + offset, src, size);
    screen->x0 = 0;
    screen->x1 = task->width-1;
    if (y0 < screen->y0) screen->y0 = y0;
    if (y1 - 1 > screen->y1) screen->y1 = y1 - 1;
    screen->dirty = 1;
    ptr_unlock(task->screen_ptr, TAG_SCREEN);
  }
}

void pumpkin_screen_dirty(WinHandle wh, int x, int y, int w, int h) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  task_screen_t *screen;
  RectangleType *r;
  Int16 sx, sy;

  if (!task) return;

//debug(1, "XXX", "pumpkin_screen_dirty (%d,%d,%d,%d) ...", x, y, w, h);
  if (wh) {
    r = &wh->windowBounds;
    sx = r->topLeft.x;
    sy = r->topLeft.y;
    switch (DEFAULT_DENSITY) {
      case kDensityDouble: sx *= 2; sy *= 2; break;
      default: break;
    }

    if ((screen = ptr_lock(task->screen_ptr, TAG_SCREEN))) {
//debug(1, "XXX", "pumpkin_screen_dirty BmpDrawSurface (%d,%d,%d,%d) %d,%d", x, y, w, h, sx+x, sy+y);
      BmpDrawSurface(wh->bitmapP, x, y, w, h, screen->surface, sx+x, sy+y, true);

      x += sx;
      y += sy;
//debug(1, "XXX", "pumpkin_screen_dirty x=%d y=%d", x, y);
//debug(1, "XXX", "pumpkin_screen_dirty dirty before (%d,%d,%d,%d)", screen->x0, screen->y0, screen->x1, screen->y1);

      if (x < screen->x0) screen->x0 = x;
      if (x+w-1 > screen->x1) screen->x1 = x+w-1;
//debug(1, "XXX", "pumpkin_screen_dirty range x0=%d x1=%d", screen->x0, screen->x1);

      if (screen->x0 < 0) screen->x0 = 0;
      else if (screen->x0 >= task->width) screen->x0 = task->width-1;
//debug(1, "XXX", "pumpkin_screen_dirty limit x0=%d", screen->x0);
      if (screen->x1 < 0) screen->x1 = 0;
      else if (screen->x1 >= task->width) screen->x1 = task->width-1;
//debug(1, "XXX", "pumpkin_screen_dirty limit x1=%d", screen->x1);

      if (y < screen->y0) screen->y0 = y;
      if (y+h-1 > screen->y1) screen->y1 = y+h-1;
//debug(1, "XXX", "pumpkin_screen_dirty range y0=%d y1=%d", screen->y0, screen->y1);

      if (screen->y0 < 0) screen->y0 = 0;
      else if (screen->y0 >= task->height) screen->y0 = task->height-1;
//debug(1, "XXX", "pumpkin_screen_dirty limit y0=%d", screen->y0);
      if (screen->y1 < 0) screen->y1 = 0;
      else if (screen->y1 >= task->height) screen->y1 = task->height-1;
//debug(1, "XXX", "pumpkin_screen_dirty limit y1=%d", screen->y1);

//debug(1, "XXX", "pumpkin_screen_dirty dirty (%d,%d,%d,%d)", screen->x0, screen->y0, screen->x1, screen->y1);

      screen->dirty = 1;
      ptr_unlock(task->screen_ptr, TAG_SCREEN);
    }
  }

  /*
  // this code is not necessary for DIA
  if ((pumpkin_module.dia || pumpkin_module.single) && mutex_lock(mutex) == 0) {
    uint64_t now = sys_get_clock();
    if ((now - pumpkin_module.lastUpdate) > 50000) {
      if (draw_task(0, &x, &y, &w, &h)) {
        wman_update(pumpkin_module.wm, pumpkin_module.tasks[0].taskId, x, y, w, h);
        if (pumpkin_module.wp->render) {
          pumpkin_module.wp->render(pumpkin_module.w);
        }
      }
      pumpkin_module.lastUpdate = now;

    }
    mutex_unlock(mutex);
    dbg_poll();
  }
  */

//debug(1, "XXX", "pumpkin_screen_dirty done");
}

// From: Friday, 1 January 1904, 00:00:00
// To: Thursday, 1 January 1970, 00:00:00
// 2,082,844,800 seconds

uint32_t pumpkin_dt(void) {
  return 2082844800;
}

int pumpkin_clipboard_add_text(char *text, int length) {
  int r = -1;

  if (text && length && pumpkin_module.wp && pumpkin_module.wp->clipboard) {
    if (mutex_lock(mutex) == 0) {
      pumpkin_module.wp->clipboard(pumpkin_module.w, text, length);
      mutex_unlock(mutex);
      r = 0;
    }
  }

  return r;
}

int pumpkin_clipboard_append_text(char *text, int length) {
  char *s;
  int len, r = -1;

  if (text && length && pumpkin_module.wp && pumpkin_module.wp->clipboard) {
    if (mutex_lock(mutex) == 0) {
      len = 0;
      if ((s = pumpkin_module.wp->clipboard(pumpkin_module.w, NULL, 0)) != NULL) {
        len = sys_strlen(s);
        if (len > cbdMaxTextLength*2) len = cbdMaxTextLength*2;
        xmemcpy(pumpkin_module.clipboardAux, s, len);
      }
      if (len + length > cbdMaxTextLength*2) length = cbdMaxTextLength*2 - len;
      if (length > 0) {
        xmemcpy(&pumpkin_module.clipboardAux[len], text, length);
        len += length;
      }
      pumpkin_module.wp->clipboard(pumpkin_module.w, pumpkin_module.clipboardAux, len);
      mutex_unlock(mutex);
      r = 0;
    }
  }

  return r;
}

int pumpkin_clipboard_get_text(char *text, int *length) {
  char *s;
  int len, r = -1;

  if (text && length && pumpkin_module.wp && pumpkin_module.wp->clipboard) {
    if (mutex_lock(mutex) == 0) {
      if ((s = pumpkin_module.wp->clipboard(pumpkin_module.w, NULL, 0)) != NULL) {
        len = sys_strlen(s);
        if (len > *length) len = *length;
        xmemcpy(text, s, len);
        *length = len;
        r = 0;
      }
      mutex_unlock(mutex);
    }
  }

  return r;
}

int pumpkin_clipboard_add_bitmap(BitmapType *bmp, int size) {
  int r = -1;

  if (bmp && size > 0) {
    // XXX not implemented
  }

  return r;
}

int pumpkin_alarm_check(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  SysAlarmTriggeredParamType triggerAlarm;
  SysDisplayAlarmParamType displayAlarm;
  launch_request_t request;
  int r = 0;

  if (task->alarm_time) {
    // there is an alarm set
    if (sys_time() >= task->alarm_time) {
      debug(DEBUG_INFO, PUMPKINOS, "pumpkin_alarm_check: alarm time has arrived");
      triggerAlarm.alarmSeconds = task->alarm_time;
      triggerAlarm.ref = task->alarm_data;

      xmemset(&request, 0, sizeof(request));
      request.code = sysAppLaunchCmdAlarmTriggered;
      request.param = &triggerAlarm;

      if (pumpkin_launch_sub(&request, 0) == 0) {
        xmemcpy(&triggerAlarm, request.param, sizeof(SysAlarmTriggeredParamType));
        if (triggerAlarm.purgeAlarm) {
          // application requested to cancel the alarm
          debug(DEBUG_INFO, PUMPKINOS, "pumpkin_alarm_check: purge alarm");
          task->alarm_time = 0;
          task->alarm_data = 0;

        } else {
          // go ahead and send the alarm
          displayAlarm.alarmSeconds = task->alarm_time;
          displayAlarm.ref = task->alarm_data;
          displayAlarm.soundAlarm = false; // not used
          task->alarm_time = 0;
          task->alarm_data = 0;

          xmemset(&request, 0, sizeof(request));
          request.code = sysAppLaunchCmdDisplayAlarm;
          request.param = &displayAlarm;

          debug(DEBUG_INFO, PUMPKINOS, "pumpkin_alarm_check: display alarm");
          if (pumpkin_launch_sub(&request, 0) == 0) {
            r = 1;
          }
        }
      }
    }
  }

  return r;
}

int pumpkin_alarm_set(LocalID dbID, uint32_t t, uint32_t data) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  int r = -1;

  if (dbID == task->dbID) {
    task->alarm_time = t;
    task->alarm_data = data;
    r = 0;
  }

  return r;
}

int pumpkin_alarm_get(LocalID dbID, uint32_t *t, uint32_t *data) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  int r = -1;

  if (dbID == task->dbID) {
    if (t) *t = task->alarm_time;
    if (data) *data = task->alarm_data;
    r = 0;
  }

  return r;
}

int pumpkin_add_serial(char *descr, uint32_t creator, char *host, uint32_t port) {
  int r = -1;

  if (mutex_lock(mutex) == 0) {
    if (pumpkin_module.num_serial < MAX_SERIAL) {
      pumpkin_module.serial[pumpkin_module.num_serial].descr = xcalloc(1, sys_strlen(descr) + 1);
      if (pumpkin_module.serial[pumpkin_module.num_serial].descr) {
        sys_strcpy(pumpkin_module.serial[pumpkin_module.num_serial].descr, descr);
      }
      pumpkin_module.serial[pumpkin_module.num_serial].creator = creator;
      pumpkin_module.serial[pumpkin_module.num_serial].port = port;
      sys_strncpy(pumpkin_module.serial[pumpkin_module.num_serial].host, host, MAX_HOST);
      pumpkin_module.num_serial++;
      r = 0;
    }
    mutex_unlock(mutex);
  }

  return r;
}

int pumpkin_info_serial(uint32_t id, char *host, int hlen, uint32_t *port) {
  int r = -1;

  if (mutex_lock(mutex) == 0) {
    if (id < pumpkin_module.num_serial) {
      if (host && hlen) sys_strncpy(host, pumpkin_module.serial[id].host, hlen);
      if (port) *port = pumpkin_module.serial[id].port;
      r = 0;
    }
    mutex_unlock(mutex);
  }

  return r;
}

int pumpkin_get_serial(uint32_t id, char **descr, uint32_t *creator) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  int r = -1;

  if (mutex_lock(mutex) == 0) {
    if (id < pumpkin_module.num_serial) {
      if (descr) *descr = task->serial[id];
      if (creator) *creator = pumpkin_module.serial[id].creator;
      r = 0;
    }
    mutex_unlock(mutex);
  }

  return r;
}

int pumpkin_get_serial_by_creator(uint32_t *id, char **descr, uint32_t creator) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  uint32_t i;
  int r = -1;

  if (mutex_lock(mutex) == 0) {
    for (i = 0; i < pumpkin_module.num_serial; i++) {
      if (pumpkin_module.serial[i].creator == creator) {
        if (id) *id = i;
        if (descr) *descr = task->serial[i];
        r = 0;
        break;
      }
    }
    mutex_unlock(mutex);
  }

  return r;
}

int pumpkin_open_serial(uint32_t id) {
  int fd = -1;

  if (mutex_lock(mutex) == 0) {
    if (id < pumpkin_module.num_serial && !pumpkin_module.serial[id].fd) {
      fd = io_connect(pumpkin_module.serial[id].host, pumpkin_module.serial[id].port, pumpkin_module.bt);
      if (fd != -1) {
        pumpkin_module.serial[id].fd = fd;
      }
    }
    mutex_unlock(mutex);
  }

  return fd;
}

int pumpkin_close_serial(uint32_t id) {
  int r = -1;

  if (mutex_lock(mutex) == 0) {
    if (id < pumpkin_module.num_serial && pumpkin_module.serial[id].fd) {
      r = sys_close(pumpkin_module.serial[id].fd);
      pumpkin_module.serial[id].fd = 0;
    }
    mutex_unlock(mutex);
  }

  return r;
}

int pumpkin_baud_serial(uint32_t id, uint32_t baud) {
  int r = -1;

  if (mutex_lock(mutex) == 0) {
    if (id < pumpkin_module.num_serial && pumpkin_module.serial[id].fd) {
      if (pumpkin_module.serial[id].host[0] == '/') {
        r = sys_serial_baud(pumpkin_module.serial[id].fd, baud);
      } else {
        r = 0;
      }
    }
    mutex_unlock(mutex);
  }

  return r;
}

int pumpkin_word_serial(uint32_t id, char *word) {
  int r = -1;

  if (mutex_lock(mutex) == 0) {
    if (id < pumpkin_module.num_serial && pumpkin_module.serial[id].fd) {
      if (pumpkin_module.serial[id].host[0] == '/') {
        r = sys_serial_word(pumpkin_module.serial[id].fd, word);
      } else {
        r = 0;
      }
    }
    mutex_unlock(mutex);
  }

  return r;
}

int pumpkin_num_serial(void) {
  int r = -1;

  if (mutex_lock(mutex) == 0) {
    r = pumpkin_module.num_serial;
    mutex_unlock(mutex);
  }

  return r;
}

void pumpkin_set_battery(int level) {
  if (mutex_lock(mutex) == 0) {
    pumpkin_module.battery = level;
    mutex_unlock(mutex);
  }
}

int pumpkin_get_battery(void) {
  int level = 0;

  if (mutex_lock(mutex) == 0) {
    level = pumpkin_module.battery;
    mutex_unlock(mutex);
  }

  return level;
}

static char *pumpkin_script_ret(int pe, script_arg_t *ret) {
  char buf[64], *val;

  switch (ret->type) {
    case SCRIPT_ARG_INTEGER:
      sys_snprintf(buf, sizeof(buf)-1, "%d", ret->value.i);
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_REAL:
      sys_snprintf(buf, sizeof(buf)-1, "%f", ret->value.d);
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_BOOLEAN:
      sys_strcpy(buf, ret->value.i ? "true" : "false");
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_STRING:
      val = ret->value.s;
      break;
    case SCRIPT_ARG_LSTRING:
      val = xcalloc(1, ret->value.l.n + 1);
      if (val) {
        xmemcpy(val, ret->value.l.s, ret->value.l.n);
      }
      xfree(ret->value.l.s);
      break;
    case SCRIPT_ARG_OBJECT:
      sys_strcpy(buf, "<object>");
      val = xstrdup(buf);
      script_remove_ref(pe, ret->value.r);
      break;
    case SCRIPT_ARG_FUNCTION:
      sys_strcpy(buf, "<function>");
      val = xstrdup(buf);
      script_remove_ref(pe, ret->value.r);
      break;
    case SCRIPT_ARG_NULL:
      val = NULL;
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "type(%c)", ret->type);
      val = xstrdup(buf);
      break;
  }

  return val;
}

char *pumpkin_script_call(int pe, char *function, char *s) {
  script_arg_t f, arg, ret;
  char *val = NULL;

  if (script_global_get(pe, function, &f) == 0) {
    if (f.type == SCRIPT_ARG_FUNCTION) {
      arg.type = SCRIPT_ARG_LSTRING;
      arg.value.l.s = (char *)s;
      arg.value.l.n = sys_strlen(s);

      debug(DEBUG_INFO, PUMPKINOS, "calling function \'%s\'", function);
      if (script_call_args(pe, f.value.r, &ret, 1, &arg) == 0) {
        val = pumpkin_script_ret(pe, &ret);
      } else {
        debug(DEBUG_ERROR, PUMPKINOS, "function call error");
      }
    } else {
      debug(DEBUG_ERROR, PUMPKINOS, "attempt to call non function \'%s\'", function);
    }
  } else {
    debug(DEBUG_ERROR, PUMPKINOS, "function \'%s\' not found", function);
  }

  return val;
}

int pumpkin_script_run_string(int pe, char *s) {
  int r = -1;

  if (s && pe > 0) {
    r = script_run(pe, s, 0, NULL, 1);
  }

  return r;
}

int pumpkin_script_run_file(int pe, char *s) {
  int r = -1;

  if (s && pe > 0) {
    r = script_run(pe, s, 0, NULL, 0);
  }

  return r;
}

int pumpkin_script_get_last_error(int pe, char *buf, int max) {
  return script_get_last_error(pe, buf, max);
}

void *pumpkin_get_exception(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task->exception;
}

void pumpkin_error_dialog(char *msg) {
  FrmCustomAlert(10021, msg, "", "");
}

void pumpkin_fatal_error(int finish) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  debug(DEBUG_ERROR, PUMPKINOS, "pumpkin_fatal_error finish=%d", finish);
  if (finish || pumpkin_module.dia || pumpkin_module.single) {
    sys_set_finish(finish);
    sys_exit(finish);
  }

  debug(DEBUG_ERROR, PUMPKINOS, "calling ErrLongJump");
  ErrLongJump(task->jmpbuf, 1);
  debug(DEBUG_ERROR, PUMPKINOS, "after ErrLongJump!");
}

void pumpkin_set_size(uint32_t creator, uint16_t width, uint16_t height) {
  AppRegistrySize s;
  s.width = width;
  s.height = height;
  AppRegistrySet(pumpkin_module.registry, creator, appRegistrySize, 0, &s);
}

void pumpkin_set_compat(uint32_t creator, int compat, int code) {
  AppRegistryCompat c;
  c.compat = compat;
  c.code = code;
  AppRegistrySet(pumpkin_module.registry, creator, appRegistryCompat, 0, &c);
}

void pumpkin_enum_compat(void (*callback)(UInt32 creator, UInt16 index, UInt16 id, void *p, void *data), void *data) {
  AppRegistryEnum(pumpkin_module.registry, callback, 0, 0, data);
}

static void compat_callback(UInt32 creator, UInt16 index, UInt16 id, void *p, void *data) {
  AppRegistryCompat *c;
  char buf[256], st[8];
  int fd;

  if (id == appRegistryCompat) {
    c = (AppRegistryCompat *)p;
    fd = *((int *)data);
    pumpkin_id2s(creator, st);
    sys_snprintf(buf, sizeof(buf)-1, "%s;%d;%d\n", st, c->compat, c->code);
    sys_write(fd, (uint8_t *)buf, sys_strlen(buf));
  }
}

void pumpkin_compat_log(void) {
  int fd;

  if ((fd = sys_create(COMPAT_LIST, SYS_WRITE | SYS_TRUNC, 0622)) != -1) {
    AppRegistryEnum(pumpkin_module.registry, compat_callback, 0, 0, &fd);
    sys_close(fd);
  }
}

void pumpkin_crash_log(UInt32 creator, int code, char *msg) {
  char buf[256], st[8];
  int fd;

  pumpkin_id2s(creator, st);
  debug(DEBUG_INFO, "CRASH", "%s;%d;%s", st, code, msg);

  if (mutex_lock(mutex) == 0) {
    if ((fd = sys_open(CRASH_LOG, SYS_WRITE)) != -1) {
      sys_seek(fd, 0, SYS_SEEK_END);
      sys_snprintf(buf, sizeof(buf)-1, "%s;%d;%s\n", st, code, msg);
      sys_write(fd, (uint8_t *)buf, sys_strlen(buf));
      sys_close(fd);
    }
    mutex_unlock(mutex);
  }
}

UInt32 pumpkin_s2id(UInt32 *ID, char *s) {
  creator_id_t id;

  id.c[3] = s[0];
  id.c[2] = s[1];
  id.c[1] = s[2];
  id.c[0] = s[3];

  *ID = id.t;
  return *ID;
}

char *pumpkin_id2s(UInt32 ID, char *s) {
  creator_id_t id;

  id.t = ID;
  s[0] = id.c[3];
  s[1] = id.c[2];
  s[2] = id.c[1];
  s[3] = id.c[0];
  s[4] = 0;

  return s;
}

int pumpkin_default_density(void) {
  return DEFAULT_DENSITY;
}

Boolean SysLibNewRefNum68K(UInt32 type, UInt32 creator, UInt16 *refNum) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  char buf[8], buf2[8];
  UInt16 i, first;
  Boolean exists = false;

  *refNum = 0;

  for (i = 0, first = 0xFFFF; i < MAX_SYSLIBS; i++) {
    if (first == 0xFFFF && task->syslibs[i].refNum == 0) {
      first = i;
    }
    if (task->syslibs[i].refNum > 0 && task->syslibs[i].type == type && task->syslibs[i].creator == creator) {
      *refNum = task->syslibs[i].refNum;
      exists = true;
      break;
    }
  }

  if (!exists && first != 0xFFFF) {
    task->syslibs[first].refNum = first+1;
    task->syslibs[first].type = type;
    task->syslibs[first].creator = creator;
    *refNum = task->syslibs[first].refNum;
    pumpkin_id2s(type, buf);
    pumpkin_id2s(creator, buf2);
    debug(DEBUG_INFO, PUMPKINOS, "reserving syslib type '%s' creator '%s' at %d", buf, buf2, *refNum);
  }

  return exists;
}

Err SysLibRegister68K(UInt16 refNum, LocalID dbID, uint8_t *code, UInt32 size, UInt16 *dispatchTblP, UInt8 *globalsP) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  char buf[8], buf2[8];
  UInt16 nameOffset, firstOffset, numFunctions;
  UInt32 d, i;
  Err err = 0;

  for (i = 0; i < MAX_SYSLIBS; i++) {
    if (task->syslibs[i].refNum == refNum) {
      if (DmDatabaseInfo(0, dbID, task->syslibs[i].name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == errNone) {
        task->syslibs[i].code = pumpkin_heap_alloc(size, "syslib_code");
        task->syslibs[i].tbl = pumpkin_heap_alloc(sizeof(SysLibTblEntryType), "syslib_tbl");
        if (task->syslibs[i].code) {
          xmemcpy(task->syslibs[i].code, code, size);
          d = (uint8_t *)dispatchTblP - code;
          task->syslibs[i].dispatchTbl = (UInt16 *)(task->syslibs[i].code + d);
          task->syslibs[i].codeSize = size;
          task->syslibs[i].dbID = dbID;
          task->syslibs[i].globals = globalsP;
          get2b(&nameOffset, (uint8_t *)task->syslibs[i].dispatchTbl, 0);
          get2b(&firstOffset, (uint8_t *)task->syslibs[i].dispatchTbl, 2);
          numFunctions = firstOffset / 2;
          debug(DEBUG_INFO, PUMPKINOS, "SysLibRegister68K nameOffset %d firstOffset %d numFunctions %d", nameOffset, firstOffset, numFunctions);
          debug_bytes(DEBUG_TRACE, PUMPKINOS, (uint8_t *)task->syslibs[i].dispatchTbl, numFunctions*2);
          pumpkin_id2s(task->syslibs[i].type, buf);
          pumpkin_id2s(task->syslibs[i].creator, buf2);
          debug(DEBUG_INFO, PUMPKINOS, "registering syslib \"%s\" type '%s' creator '%s' at %d", task->syslibs[i].name, buf, buf2, refNum);
        }
      }
      break;
    }
  }

  return err;
}

uint8_t *SysLibTblEntry68K(UInt16 refNum, SysLibTblEntryType *tbl) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  UInt16 i;
  uint8_t *r = NULL;

  for (i = 0; i < MAX_SYSLIBS; i++) {
    if (task->syslibs[i].refNum == refNum) {
      tbl->dispatchTblP = (MemPtr *)task->syslibs[i].dispatchTbl;
      tbl->globalsP = task->syslibs[i].globals;
      tbl->dbID = task->syslibs[i].dbID;
      tbl->codeRscH = NULL;
      r = task->syslibs[i].tbl;
      break;
    }
  }

  return r;
}

void SysLibCancelRefNum68K(UInt16 refNum) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  char buf[8], buf2[8];
  UInt16 i;

  for (i = 0; i < MAX_SYSLIBS; i++) {
    if (task->syslibs[i].refNum == refNum) {
      if (task->syslibs[i].code) {
        pumpkin_heap_free(task->syslibs[i].code, "syslib_code");
      }
      if (task->syslibs[i].tbl) {
        pumpkin_heap_free(task->syslibs[i].tbl, "syslib_tbl");
      }
      pumpkin_id2s(task->syslibs[i].type, buf);
      pumpkin_id2s(task->syslibs[i].creator, buf2);
      debug(DEBUG_INFO, PUMPKINOS, "unregistering syslib \"%s\" type '%s' creator '%s' at %d", task->syslibs[i].name, buf, buf2, refNum);
      xmemset(&task->syslibs[i], 0, sizeof(syslib_t));
      break;
    }
  }
}

UInt16 SysLibFind68K(char *name) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  char buf[8], buf2[8];
  UInt16 i, refNum = 0;

  for (i = 0; i < MAX_SYSLIBS; i++) {
    if (task->syslibs[i].refNum > 0 && !sys_strcmp(task->syslibs[i].name, name)) {
      refNum = task->syslibs[i].refNum;
      pumpkin_id2s(task->syslibs[i].type, buf);
      pumpkin_id2s(task->syslibs[i].creator, buf2);
      debug(DEBUG_INFO, PUMPKINOS, "found syslib \"%s\" type '%s' creator '%s' at %d", name, buf, buf2, refNum);
      break;
    }
  }

  return refNum;
}

UInt16 *SysLibGetDispatch68K(UInt16 refNum) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  UInt16 i, *dispatch = NULL;

  for (i = 0; i < MAX_SYSLIBS; i++) {
    if (task->syslibs[i].refNum == refNum) {
      dispatch = task->syslibs[i].dispatchTbl;
      break;
    }
  }

  return dispatch;
}

void pumpkin_set_v10(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  task->v10 = 1;
}

int pumpkin_is_v10(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task->v10;
}

void pumpkin_set_m68k(int m68k) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  task->m68k = m68k;

  if (mutex_lock(mutex) == 0) {
    pumpkin_module.tasks[task->task_index].m68k = m68k;
    mutex_unlock(mutex);
  }

  pumpkin_forward_event(0, MSG_KEY, WINDOW_KEY_CUSTOM, vchrRefreshState, 0);
}

int pumpkin_is_m68k(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task ? task->m68k : 0;
}

void pumpkin_set_preference(UInt32 creator, UInt16 seq, void *p, UInt16 size, Boolean saved) {
  if (mutex_lock(mutex) == 0) {
    AppRegistrySetPreference(pumpkin_module.registry, creator, seq, p, size, saved);
    mutex_unlock(mutex);
  }
}

UInt16 pumpkin_get_preference(UInt32 creator, UInt16 seq, void *p, UInt16 size, Boolean saved) {
  if (mutex_lock(mutex) == 0) {
    size = AppRegistryGetPreference(pumpkin_module.registry, creator, seq, p, size, saved);
    mutex_unlock(mutex);
  }

  return size;
}

void pumpkin_save_bitmap(BitmapType *bmp, UInt16 density, Coord wWidth, Coord wHeight, Coord width, Coord height, char *filename) {
  surface_t *surface;
  RectangleType rect;
  WinHandle wh, old;
  char *card, buf[256];

  if (width == 0 || height == 0) {
    BmpGetDimensions(bmp, &width, &height, NULL);
  }
  if (wWidth == 0 || wHeight == 0) {
    wWidth = width;
    wHeight = height;
  }
  debug(DEBUG_INFO, PUMPKINOS, "saving %dx%d bitmap as %s", width, height, filename);
  if ((surface = surface_create(wWidth*2, wHeight*2, SURFACE_ENCODING_ARGB)) != NULL) {
    wh = WinCreateOffscreenWindow(wWidth, wHeight, nativeFormat, NULL);
    old = WinSetDrawWindow(wh);
    RctSetRectangle(&rect, 0, 0, wWidth, wHeight);
    WinEraseRectangle(&rect, 0);
    WinPaintBitmap(bmp, (wWidth - width)/2, 0);
    WinSetDrawWindow(old);
    BmpDrawSurface(wh->bitmapP, 0, 0, wWidth*2, wHeight*2, surface, 0, 0, false);

    card = VFS_CARD;
    if (card[0] == '/') card++;
    sys_snprintf(buf, sizeof(buf)-1, "%s%s%s", VFSGetMount(), card, filename);
    surface_save(surface, buf, 0);

    WinDeleteWindow(wh, false);
    surface_destroy(surface);
  }
}

void LongToRGB(UInt32 c, RGBColorType *rgb) {
  rgb->r = (c & 0xff0000) >> 16;
  rgb->g = (c & 0x00ff00) >> 8;
  rgb->b = (c & 0x0000ff);
}

UInt32 RGBToLong(RGBColorType *rgb) {
  return (((UInt32)rgb->r) << 16) | (((UInt32)rgb->g) << 8) | ((UInt32)rgb->b);
}

static int pumpkin_httpd_string(int pe) {
  http_connection_t *con;
  script_int_t ptr, code;
  char *str = NULL;
  char *mime = NULL;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &code) == 0) {

    script_opt_string(pe, 2, &str);
    script_opt_string(pe, 3, &mime);

    if ((con = (http_connection_t *)ptr_lock(ptr, TAG_CONN)) != NULL) {
      if (str && mime) {
        httpd_string(con, code, str, mime);
      } else {
        httpd_reply(con, code);
      }
      ptr_unlock(ptr, TAG_CONN);
      r = 0;
    }

    if (str) xfree(str);
    if (mime) xfree(mime);
  }

  return script_push_boolean(pe, r == 0);
}

static int pumpkin_httpd_file(int pe) {
  http_connection_t *con;
  script_int_t ptr, code;
  char *filename = NULL;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &code) == 0 &&
      script_get_string(pe, 2, &filename) == 0) {

    if ((con = (http_connection_t *)ptr_lock(ptr, TAG_CONN)) != NULL) {
      httpd_file(con, filename);
      ptr_unlock(ptr, TAG_CONN);
      r = 0;
    }

    if (filename) xfree(filename);
  }

  return script_push_boolean(pe, r == 0);
}

static int pumpkin_httpd_read(int pe) {
  http_connection_t *con;
  script_int_t ptr, len;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &len) == 0) {

    if (len > 0) {
      if ((con = (http_connection_t *)ptr_lock(ptr, TAG_CONN)) != NULL) {
        if (con->body_fd > 0) {
          if (len > MAX_PACKET) len = MAX_PACKET;
          len = sys_read(con->body_fd, (uint8_t *)con->body_buf, len);
          if (len >= 0) {
            r = script_push_lstring(pe, con->body_buf, len);
          }
        }
        ptr_unlock(ptr, TAG_CONN);
      }
    }
  }

  return r;
}

static int stream_write(int pe) {
  char *s = NULL;
  int fd, len;

  if (script_get_lstring(pe, 0, &s, &len) == 0) {
    fd = pumpkin_script_global_iconst_value(pe, "_fout");
    if (fd > 0) {
      sys_write(fd, (uint8_t *)s, len);
    }
  }

  if (s) xfree(s);

  return 0;
}

static int it_pos(int pe) {
  script_int_t i, index;
  iterator_t *it;
  int r = 0;

  if (script_get_integer(pe, 0, &i) == 0 &&
      script_get_integer(pe, 1, &index) == 0) {

    if ((it = (iterator_t *)ptr_lock(i, TAG_ITERATOR)) != NULL) {
      if (it->position && it->position(it, index)) {
        r = script_push_boolean(pe, 1);
      }
      ptr_unlock(i, TAG_ITERATOR);
    }
  }

  return r;
}

static int it_next(int pe) {
  script_int_t i;
  iterator_t *it;
  int r = 0;

  if (script_get_integer(pe, 0, &i) == 0) {
    if ((it = (iterator_t *)ptr_lock(i, TAG_ITERATOR)) != NULL) {
      if (it->next && it->next(it)) {
        r = script_push_boolean(pe, 1);
      }
      ptr_unlock(i, TAG_ITERATOR);
    }
  }

  return r;
}

static int it_obj(int pe) {
  script_int_t i;
  iterator_t *it;
  char *name, *value;
  script_ref_t obj, j, k, n, r = 0;

  if (script_get_integer(pe, 0, &i) == 0) {
    if ((it = (iterator_t *)ptr_lock(i, TAG_ITERATOR)) != NULL) {
      obj = pumpkin_script_create_obj(pe, NULL);
      n = it->num_properties(it);
      for (j = 1; j <= n; j++) {
        name = it->property_name(it, j);
        value = it->get_property_value(it, j);
        if (value) {
          for (k = 0; value[k]; k++) {
            if (value[k] < '0' || value[k] > '9') break;
          }
          if (k && !value[k]) {
            pumpkin_script_obj_iconst(pe, obj, name, sys_atoi(value));
          } else {
            pumpkin_script_obj_sconst(pe, obj, name, value);
          }
        }
      }
      ptr_unlock(i, TAG_ITERATOR);
    }
  }

  return r;
}

static int httpd_template(int pe, http_connection_t *con, template_t *t) {
  char *script;
  int fd, r = -1;

  if ((fd = sys_mkstemp()) != -1) {
    script = template_getscript(t);
    pumpkin_script_global_iconst(pe, "_fout", fd);

    if (pumpkin_script_run_string(pe, script) != -1) {
      sys_seek(fd, 0, SYS_SEEK_SET);
      r = httpd_file_stream(con, fd, template_gettype(t), 0);
    }
    sys_close(fd);
  } else {
    r = httpd_reply(con, 500);
  }

  return r;
}

static int pumpkin_httpd_template(int pe) {
  http_connection_t *con;
  template_t *t;
  script_int_t ptr1, ptr2;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr1) == 0 &&
      script_get_integer(pe, 1, &ptr2) == 0) {

    con = (http_connection_t *)ptr_lock(ptr1, TAG_CONN);
    if (con) {
      t = (template_t *)ptr_lock(ptr2, TAG_TEMPLATE);
      if (t) {
        r = script_push_boolean(pe, httpd_template(pe, con, t) == 0);
        ptr_unlock(ptr2, TAG_TEMPLATE);
      }
      ptr_unlock(ptr1, TAG_CONN);
    }
  }

  return r;
}

static int pumpkin_httpd_install(int pe) {
  char *s = NULL;
  uint8_t *p;
  int len, r = -1;

  if (script_get_lstring(pe, 0, &s, &len) == 0) {
    if ((p = MemPtrNew(len)) != NULL) {
      MemMove(p, s, len);
      r = StoDeployFileFromImage(p, len, pumpkin_module.registry);
      MemPtrFree(p);
    }
  }

  if (s) xfree(s);

  return r;
}

static void template_destructor(void *p) {
  template_t *t;

  t = (template_t *)p;
  if (t) {
    template_destroy(t);
  }
}

static int pumpkin_template_create(int pe) {
  char *filename = NULL;
  char *mimetype = NULL;
  script_arg_t value;
  template_t *t;
  char *prefix = NULL;
  int ptr, r = -1;

  if (script_get_string(pe, 0, &filename) == 0 &&
      script_get_string(pe, 1, &mimetype) == 0) {

    if (script_global_get(pe, "prefix", &value) == 0 && value.type == SCRIPT_ARG_LSTRING) {
      prefix = xmalloc(value.value.l.n + 1);
      sys_strncpy(prefix, value.value.l.s, value.value.l.n);
    }

    if (prefix) {
      if ((t = template_create(prefix, filename, mimetype)) != NULL) {
        if (template_compile(t) != -1) {
          if ((ptr = ptr_new(t, template_destructor)) != -1) {
            r = script_push_integer(pe, ptr);
          } else {
            template_destroy(t);
          }
        } else {
          template_destroy(t);
        }
      }
      xfree(prefix);
    }
  }

  if (filename) xfree(filename);
  if (mimetype) xfree(mimetype);

  return r;
}

static int pumpkin_template_destroy(int pe) {
  script_int_t ptr;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    return script_push_boolean(pe, ptr_free(ptr, TAG_TEMPLATE) == 0);
  }

  return -1;
}

static int pumpkin_httpd_callback(http_connection_t *con) {
  pumpkin_httpd_t *h;
  script_arg_t value, ret;
  int i, r, obj, ptr;

  h = (pumpkin_httpd_t *)con->data;

  switch (con->status) {
    case HTTPD_START:
      h->status = 1;
      return 0;
    case HTTPD_STOP:
      h->status = 0;
      return 0;
    case HTTPD_IDLE:
      return h->idle(h->data) ? -1 : 0;
    default:
      break;
  }

  if (h->ref) {
    obj = pumpkin_script_create_obj(h->pe, "header");
    for (i = 0; i < con->num_headers; i++) {
      script_add_sconst(h->pe, obj, con->header_name[i], con->header_value[i]);
    }

    obj = pumpkin_script_create_obj(h->pe, "param");
    for (i = 0; i < con->num_params; i++) {
      script_add_sconst(h->pe, obj, con->param_name[i], con->param_value[i]);
    }

    value.type = SCRIPT_ARG_STRING;
    value.value.s = con->method;
    script_global_set(h->pe, "method", &value);
    value.value.s = con->uri;
    script_global_set(h->pe, "path", &value);

    if ((ptr = ptr_new(con, NULL)) != -1) {
      r = script_call(h->pe, h->ref, &ret, "I", ptr);
      ptr_free(ptr, TAG_CONN);
    } else {
      r = -1;
    }

    if (r == 0) {
      if (!script_returned_value(&ret)) {
        httpd_reply(con, 404);
      }
    } else {
      httpd_reply(con, 500);
    }
  } else {
    httpd_reply(con, 404);
  }

  return 0;
}

int pumpkin_httpd_status(pumpkin_httpd_t *h) {
  return h ? h->status : 0;
}

pumpkin_httpd_t *pumpkin_httpd_create(UInt16 port, UInt16 scriptId, char *worker, char *root, void *data, Boolean (*idle)(void *data)) {
  pumpkin_httpd_t *h = NULL;
  MemHandle hscript;
  script_arg_t value;
  char *s, *card, buf[256];
  script_ref_t obj, len;

  if (root && (h = xcalloc(1, sizeof(pumpkin_httpd_t))) != NULL) {
    h->idle = idle;
    h->data = data;

    if ((hscript = DmGet1Resource(pumpkin_script_engine_id(), scriptId)) != 0) {
      if ((s = MemHandleLock(hscript)) != NULL) {
        len = MemHandleSize(hscript);
        if ((h->script = xmalloc(len+1)) != NULL) {
          MemMove(h->script, s, len);
        }
        MemHandleUnlock(hscript);
      }
      DmReleaseResource(hscript);
    }

    if (h->script && (h->pe = pumpkin_script_create()) != -1) {
      obj = pumpkin_script_create_obj(h->pe, "http");
      script_add_function(h->pe, obj, "file",     pumpkin_httpd_file);
      script_add_function(h->pe, obj, "string",   pumpkin_httpd_string);
      script_add_function(h->pe, obj, "read",     pumpkin_httpd_read);
      script_add_function(h->pe, obj, "template", pumpkin_httpd_template);
      script_add_function(h->pe, obj, "install",  pumpkin_httpd_install);

      obj = pumpkin_script_create_obj(h->pe, "template");
      script_add_function(h->pe, obj, "create",   pumpkin_template_create);
      script_add_function(h->pe, obj, "destroy",  pumpkin_template_destroy);

      pumpkin_script_global_function(h->pe, "_stream_write", stream_write);
      pumpkin_script_global_function(h->pe, "_it_pos", it_pos);
      pumpkin_script_global_function(h->pe, "_it_next", it_next);
      pumpkin_script_global_function(h->pe, "_it_obj", it_obj);

      card = VFS_CARD;
      if (card[0] == '/') card++;
      sys_snprintf(h->prefix, sizeof(h->prefix)-1, "%s%s", VFSGetMount(), card);

      value.type = SCRIPT_ARG_STRING;
      value.value.s = h->prefix;
      script_global_set(h->pe, "prefix", &value);

      pumpkin_script_run_string(h->pe, h->script);

      if (script_global_get(h->pe, worker, &value) == 0) {
        if (value.type == SCRIPT_ARG_FUNCTION) {
          h->ref = value.value.r;
        }
      }

      if (root[0] == '/') root++;
      sys_snprintf(buf, sizeof(buf)-1, "%s%s%s", VFSGetMount(), card, root);

      if (httpd_create("0.0.0.0", port, PUMPKIN_SERVER_NAME, buf, NULL, NULL, NULL, NULL, NULL, pumpkin_httpd_callback, h, 0) == -1) {
        xfree(h->script);
        xfree(h);
        h = NULL;
      }
    } else {
      xfree(h->script);
      xfree(h);
      h = NULL;
    }
  }

  return h;
}

int pumpkin_httpd_destroy(pumpkin_httpd_t *h) {
  int r = -1;

  if (h) {
    if (h->script) xfree(h->script);
    if (h->pe > 0) pumpkin_script_destroy(h->pe);
    xfree(h);
    r = 0;
  }

  return r;
}

void pumpkin_save_icon(char *name) {
  DmOpenRef dbRef;
  LocalID dbID;
  BitmapType *bmp;
  MemHandle h;
  Coord cellWidth, cellHeight;
  char filename[256];

  cellWidth = 50;
  cellHeight = 35;

  if ((dbID = DmFindDatabase(0, name)) != 0) {
    if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
      if ((h = DmGet1Resource(iconType, 1000)) != NULL) {
        if ((bmp = MemHandleLock(h)) != NULL) {
          sys_snprintf(filename, sizeof(filename)-1, "%s.png", name);
          pumpkin_save_bitmap(bmp, BmpGetDensity(bmp), cellWidth, cellHeight, 0, 0, filename);
          MemHandleUnlock(h);
        }
        DmReleaseResource(h);
      }
      DmCloseDatabase(dbRef);
    }
  }
}

UInt32 pumpkin_next_char(UInt8 *s, UInt32 i, UInt16 *w) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  if (task->lang && task->lang->nextChar) {
    i = task->lang->nextChar(s, i, w, task->lang->data);
  } else {
   *w = s[i];
    i = 1;
  }

  return i;
}

UInt8 pumpkin_map_char(UInt16 w, FontType **f) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  if (task->lang && task->lang->mapChar) {
    return task->lang->mapChar(w, f, task->lang->data);
  }

  *f = FntGetFontPtr();
  return w;
}

static void notif_ptr_destructor(void *p) {
  if (p) xfree(p);
}

Err SysNotifyRegister(UInt16 cardNo, LocalID dbID, UInt32 notifyType, SysNotifyProcPtr callbackP, Int8 priority, void *userDataP) {
  AppRegistryNotification n;
  UInt32 type, creator;
  notif_ptr_t *np;
  char screator[8], stype[8];
  int ptr, i;
  Err err;

  if ((err = DmDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, &creator)) == errNone) {
    if (type == sysFileTApplication) {
      if (mutex_lock(mutex) == 0) {
        pumpkin_id2s(creator, screator);
        pumpkin_id2s(notifyType, stype);

        if (pumpkin_module.num_notif < MAX_NOTIF_REGISTER) {
          for (i = 0; i < pumpkin_module.num_notif; i++) {
            if (pumpkin_module.notif[i].appCreator == creator && pumpkin_module.notif[i].notifyType == notifyType) {
              debug(DEBUG_ERROR, PUMPKINOS, "register notification type '%s' creator '%s' priority %d: duplicate", stype, screator, priority);
              err = sysNotifyErrDuplicateEntry;
              break;
            }
          }

          if (i == pumpkin_module.num_notif) {
            debug(DEBUG_INFO, PUMPKINOS, "register notification type '%s' creator '%s' priority %d: added", stype, screator, priority);
            if (callbackP || userDataP) {
              np = xcalloc(1, sizeof(notif_ptr_t));
              np->tag = TAG_NOTIF;
              if (pumpkin_is_m68k()) {
                np->callback68k = callbackP ? (uint8_t *)callbackP - (uint8_t *)pumpkin_heap_base() : 0;
                np->userData68k = userDataP ? (uint8_t *)userDataP - (uint8_t *)pumpkin_heap_base() : 0;
              } else {
                np->callback = callbackP;
                np->userData = userDataP;
              }
              ptr = ptr_new(np, notif_ptr_destructor);
            } else {
              n.appCreator = creator;
              n.notifyType = notifyType;
              n.priority = priority;
              AppRegistrySet(pumpkin_module.registry, creator, appRegistryNotification, 0, &n);
              ptr = 0;
            }
            pumpkin_module.notif[i].appCreator = creator;
            pumpkin_module.notif[i].notifyType = notifyType;
            pumpkin_module.notif[i].priority = priority;
            pumpkin_module.notif[i].ptr = ptr;
            pumpkin_module.num_notif++;
            err = errNone;
          }
        } else {
          debug(DEBUG_ERROR, PUMPKINOS, "register notification type '%s' creator '%s' priority %d: max reached", stype, screator, priority);
          err = sysErrParamErr;
        }
        mutex_unlock(mutex);
      }
    } else {
      // XXX different behavior: in PalmOS any code resource is ok, not only applications
      debug(DEBUG_ERROR, PUMPKINOS, "register notification type '%s' creator '%s' priority %d: dbID is not an application", stype, screator, priority);
      err = sysErrParamErr;
    }
  }

  return err;
}

Err SysNotifyUnregister(UInt16 cardNo, LocalID dbID, UInt32 notifyType, Int8 priority) {
  AppRegistryNotification n;
  UInt32 creator;
  char screator[8], stype[8];
  int i;
  Err err;

  if ((err = DmDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creator)) == errNone) {
    if (mutex_lock(mutex) == 0) {
      pumpkin_id2s(creator, screator);
      pumpkin_id2s(notifyType, stype);

      for (i = 0; i < pumpkin_module.num_notif; i++) {
        if (pumpkin_module.notif[i].appCreator == creator && pumpkin_module.notif[i].notifyType == notifyType) {
          debug(DEBUG_INFO, PUMPKINOS, "unregister notification type '%s' creator '%s' priority %d: removed", stype, screator, pumpkin_module.notif[i].priority);
          if (pumpkin_module.notif[i].ptr) {
            ptr_free(pumpkin_module.notif[i].ptr, TAG_NOTIF);
          } else {
            n.appCreator = creator;
            n.notifyType = notifyType;
            n.priority = 0xFFFF; // invalid priority: remove (appCreator,notifyType)
            AppRegistrySet(pumpkin_module.registry, creator, appRegistryNotification, 0, &n);
          }
          for (i++; i < pumpkin_module.num_notif; i++) {
            MemMove(&pumpkin_module.notif[i-1], &pumpkin_module.notif[i], sizeof(notif_registration_t));
          }
          pumpkin_module.num_notif--;
          err = errNone;
          break;
        }
      }

      if (i == pumpkin_module.num_notif) {
        debug(DEBUG_ERROR, PUMPKINOS, "unregister notification type '%s' creator '%s': not found", stype, screator);
        err = sysNotifyErrEntryNotFound;
      }
      mutex_unlock(mutex);
    }
  }

  return err;
}

static Int32 compare_registration(void *e1, void *e2, void *otherP) {
  notif_registration_t *r1, *r2;
  Int32 p1, p2, r = 0;

  r1 = (notif_registration_t *)e1;
  r2 = (notif_registration_t *)e2;
  p1 = r1->priority;
  p2 = r2->priority;

  // XXX lower numbers mean higher priorities, but sysNotifyNormalPriority is 0.
  // XXX adjust it so that priority 1 is higher than sysNotifyNormalPriority
  if (p1 == sysNotifyNormalPriority) p1 = sysNotifyMediumPriority;
  if (p2 == sysNotifyNormalPriority) p2 = sysNotifyMediumPriority;

  if (p1 < p2) {
    r = -1;
  } else if (p1 > p2) {
    r = 1;
  }

  return r;
}

Err SysNotifyBroadcast(SysNotifyParamType *notify) {
  notif_registration_t *selected;
  DmSearchStateType stateInfo;
  notif_ptr_t *np;
  LocalID dbID;
  char name[dmDBNameLength], stype[8];
  int i, j, n, handle, call;

  if (notify == NULL) {
    return sysErrParamErr;
  }

  pumpkin_id2s(notify->notifyType, stype);
  debug(DEBUG_INFO, PUMPKINOS, "broadcast notification type '%s' begin", stype);

  selected = NULL;
  n = 0;

  if (mutex_lock(mutex) == 0) {
    for (i = 0; i < pumpkin_module.num_notif; i++) {
      if (pumpkin_module.notif[i].notifyType == notify->notifyType) {
        n++;
      }
    }

    if (n > 0) {
      if ((selected = xcalloc(n, sizeof(notif_registration_t))) != NULL) {
        for (i = 0, j = 0; i < pumpkin_module.num_notif && j < n; i++) {
          if (pumpkin_module.notif[i].notifyType == notify->notifyType) {
            MemMove(&selected[j++], &pumpkin_module.notif[i], sizeof(notif_registration_t));
          }
        }
      }
    } else {
      debug(DEBUG_INFO, PUMPKINOS, "no app registered for this notification type");
    }
    mutex_unlock(mutex);
  }

  if (selected != NULL) {
    notify->handled = false;
    notify->reserved2 = 0;

    if (notify->broadcaster == 0) {
      notify->broadcaster = pumpkin_get_app_creator();
    }

    SysQSortP(selected, n, sizeof(notif_registration_t *), compare_registration, NULL);

    for (j = 0; j < n; j++) {
      if (DmGetNextDatabaseByTypeCreator(true, &stateInfo, sysFileTApplication, selected[j].appCreator, false, NULL, &dbID) != errNone) continue;
      if (DmDatabaseInfo(0, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != errNone) continue;

      if (selected[j].ptr) {
        if ((np = ptr_lock(selected[j].ptr, TAG_NOTIF)) != NULL) {
          notify->userDataP = np->userData68k ? (uint8_t *)pumpkin_heap_base() + np->userData68k : np->userData;

          if (np->callback) {
            debug(DEBUG_INFO, PUMPKINOS, "send notification type '%s' priority %d to \"%s\" using callback %p", stype, selected[j].priority, name, np->callback);
            handle = pumpkin_pause_task(name, &call);
            if (call) np->callback(notify);
            pumpkin_resume_task(handle);
          } else if (np->callback68k) {
            debug(DEBUG_INFO, PUMPKINOS, "send notification type '%s' priority %d to \"%s\" using 68k callback 0x%08X", stype, selected[j].priority, name, np->callback68k);
            handle = pumpkin_pause_task(name, &call);
            if (call) CallNotifyProc(np->callback68k, notify);
            pumpkin_resume_task(handle);
          } else {
            debug(DEBUG_INFO, PUMPKINOS, "send notification type '%s' priority %d to \"%s\" using launch code", stype, selected[j].priority, name);
            pumpkin_launch_request(name, sysAppLaunchCmdNotify, (UInt8 *)notify, 0, NULL, 1);
          }
          ptr_unlock(selected[j].ptr, TAG_NOTIF);
        }
      } else {
        notify->userDataP = NULL;
        debug(DEBUG_INFO, PUMPKINOS, "send notification type '%s' priority %d to \"%s\" using launch code", stype, selected[j].priority, name);
        pumpkin_launch_request(name, sysAppLaunchCmdNotify, (UInt8 *)notify, 0, NULL, 1);
      }
    }
    xfree(selected);
  }

  debug(DEBUG_INFO, PUMPKINOS, "broadcast notification type '%s' end", stype);

  return errNone;
}

Err SysNotifyBroadcastDeferred(SysNotifyParamType *notify, Int16 paramSize) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  char stype[8];
  Err err = sysErrParamErr;

  if (notify) {
    pumpkin_id2s(notify->notifyType, stype);

    if (task->num_notifs < MAX_NOTIF_QUEUE) {
      debug(DEBUG_INFO, PUMPKINOS, "defer notification type '%s'", stype);
      MemMove(&task->notify[task->num_notifs], notify, sizeof(SysNotifyParamType));
      if (notify->notifyDetailsP && paramSize) {
        task->notify[task->num_notifs].notifyDetailsP = MemPtrNew(paramSize);
        MemMove(task->notify[task->num_notifs].notifyDetailsP, notify->notifyDetailsP, paramSize);
      }
      task->num_notifs++;
      err = errNone;
    } else {
      debug(DEBUG_ERROR, PUMPKINOS, "defer notification type '%s' max reached", stype);
      err = sysNotifyErrQueueFull;
    }
  }

  return err;
}

void SysNotifyBroadcastQueued(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  int i;

  if (task->num_notifs > 0) {
    debug(DEBUG_INFO, PUMPKINOS, "flush notification queue begin");

    for (i = 0; i < task->num_notifs; i++) {
      SysNotifyBroadcast(&task->notify[i]);
      if (task->notify[i].notifyDetailsP) {
        MemPtrFree(task->notify[i].notifyDetailsP);
      }
    }

    MemSet(task->notify, sizeof(SysNotifyParamType) * MAX_NOTIF_QUEUE, 0);
    task->num_notifs = 0;
    debug(DEBUG_INFO, PUMPKINOS, "flush notification queue end");
  }
}

void pumpkin_setio(char (*getchar)(void *iodata), void (*putchar)(void *iodata, char c), void (*setcolor)(void *iodata, uint32_t fg, uint32_t bg), void *iodata) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  task->getchar = getchar;
  task->putchar = putchar;
  task->setcolor = setcolor;
  task->iodata = iodata;
}

char pumpkin_getchar(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  return task->getchar ? task->getchar(task->iodata) : 0;
}

void pumpkin_putchar(char c) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  if (task->putchar) {
    task->putchar(task->iodata, c);
  }
}

void pumpkin_puts(char *s) {
  uint32_t i;

  for (i = 0; s[i]; i++) {
    if (s[i] == '\n' && (i == 0 || s[i-1] != '\r')) {
      pumpkin_putchar('\r');
    }
    pumpkin_putchar(s[i]);
  }
}

uint32_t pumpkin_gets(char *buf, uint32_t max) {
  uint32_t i = 0;
  char c;

  if (buf && max > 0) {
    for (; i < max-1;) {
      c = pumpkin_getchar();
      if (c == 0) break;
      if (c == '\n') {
        pumpkin_putchar('\r');
        pumpkin_putchar('\n');
        break;
      }
      if (c == '\b') {
        if (i > 0) {
          pumpkin_putchar('\b');
          i--;
        }
        continue;
      }
      pumpkin_putchar(c);
      buf[i++] = c;
    }
    buf[i] = 0;
  }

  return i;
}

void pumpkin_setcolor(uint32_t fg, uint32_t bg) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  if (task->setcolor) {
    task->setcolor(task->iodata, fg, bg);
  }
}

void pumpkin_sound_init(void) {
  pumpkin_task_t *task;

  if ((task = xcalloc(1, sizeof(pumpkin_task_t))) != NULL) {
    thread_set(task_key, task);
    task->heap = heap_init(256*1024, NULL);
    StoInit(APP_STORAGE, pumpkin_module.fs_mutex);
    VFSInitModule(VFS_CARD);
    SndInitModule(pumpkin_module.ap);
  }
}

void pumpkin_sound_finish(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  if (task) {
    VFSFinishModule();
    SndFinishModule();
    StoFinish();
    heap_finish(task->heap);
    xfree(task);
  }
}
