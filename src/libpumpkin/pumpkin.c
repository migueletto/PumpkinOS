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
#include "logtrap.h"
#include "emupalmosinc.h"
#include "RegistryMgr.h"
#include "deploy.h"
#include "language.h"
#include "DDm.h"
#include "storage.h"
#include "pumpkin.h"
#include "media.h"
#include "timeutc.h"
#include "heap.h"
#include "grail.h"
#include "dia.h"
#include "taskbar.h"
#include "wman.h"
#include "calibrate.h"
#include "color.h"
#include "rgb.h"
#include "notif_serde.h"
#include "launch_serde.h"
#include "debug.h"

#define MAX_SEARCH_ORDER   16
#define MAX_NOTIF_QUEUE    8
#define MAX_NOTIF_REGISTER 256
#define MAX_TASKS          32
#define MAX_HOST           256

#define TAG_SCREEN  "screen"
#define TAG_SERIAL  "serial"
#define TAG_APP     "App"
#define TAG_NOTIF   "notif"

#if defined(ANDROID)
#define REGISTRY_DB   "/data/data/com.pit.pit/app_registry/"
#elif defined(EMSCRIPTEN)
#define REGISTRY_DB   "/vfs/app_registry/"
#else
#define REGISTRY_DB   "registry/"
#endif

#define VFS_CARD      "/app_card/"
#define VFS_INSTALL   "/app_install/"

#define HEAP_SIZE (8*1024*1024)
#define SMALL_HEAP_SIZE (128*1024)

#define APP_STORAGE "/app_storage/"

#define MAX_PLUGINS 256

#define FONT_DOUBLE_BASE 9000
#define FONT_LOW_BASE    9100

#define CRASH_LOG   "crash.log"

#define PUMPKIN_USER_AGENT  PUMPKINOS
#define PUMPKIN_SERVER_NAME PUMPKINOS

#define MAX_EVENTS 16

#define SCREEN_DB "ScreenDB"

#define BORDER_SIZE 4

#define TASKBAR_HEIGHT 24

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
  uint32_t taskId;
  int index, width, height, x, y;
  UInt32 creator;
} launch_data_t;

typedef struct {
  int task_index;
  int active;
  int removed;
  int paused;
  int reserved;
  int screen_ptr;
  int dx, dy;
  int width, height;
  int new_width, new_height;
  int handle;
  int penX, penY, buttons;
  int nativeKeys;
  int lockable;
  int osversion;
  int tracing;
  int m68k;
  char name[dmDBNameLength];
  uint16_t density, depth;
  uint32_t alarm_time;
  uint32_t alarm_data;
  uint32_t eventKeyMask;
  uint64_t lastMotion;
  int dirty_level;
  texture_t *texture;
  LocalID dbID;
  UInt32 creator;
  uint32_t (*pilot_main)(uint16_t code, void *param, uint16_t flags);
  uint32_t paramBlockSize;
  DmOpenRef bootRef;
  language_t *lang;
  ErrJumpBuf jmpbuf;
  char *serial[MAX_SERIAL];
  syslib_t syslibs[MAX_SYSLIBS];
  int num_syslibs;
  uint32_t taskId;
  void *exception;
  uint64_t evtTimeoutLast;
  uint32_t evtTimeoutCount;
  Boolean evtTimeoutWarning;
  Err lastErr;
  heap_t *heap;
  int num_notifs;
  SysNotifyParamType notify[MAX_NOTIF_QUEUE]; // for SysNotifyBroadcastDeferred
  void *data;
  void *subdata;
  int (*getchar)(void *iodata);
  int (*haschar)(void *iodata);
  void (*putchar)(void *iodata, char c);
  void (*putstr)(void *iodata, char *s, uint32_t len);
  void (*setcolor)(void *iodata, uint32_t fg, uint32_t bg);
  void *iodata;
  void *table;
  void *local_storage[last_key];
} pumpkin_task_t;

typedef struct {
  char *tag;
  surface_t *surface;
  surface_t *msurface;
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
  uint32_t callback68k;
  void *userData;
} notif_ptr_t;

typedef struct {
  uint32_t msg;
  SysNotifyParamType notify;
  SysNotifyProcPtr callback;
  uint32_t callback68k;
} notif_msg_t;

typedef struct {
  Int32 taskId;
  UInt32 appCreator;
  UInt32 notifyType;
  UInt32 priority;
  int ptr;
} notif_registration_t;

typedef struct {
  uint32_t msg;
  UInt16 cmd;
  UInt16 flags;
} launch_msg_t;

typedef struct {
  int ev, arg1, arg2, arg3;
} event_t;

typedef struct {
  int pe;
  script_ref_t obj;
  script_engine_t *engine;
  window_provider_t *wp;
  audio_provider_t *ap;
  secure_provider_t *secure;
  bt_provider_t *bt;
  gps_parse_line_f gps_parse_line;
  window_t *w;
  heap_t *heap;
  mutex_t *fs_mutex;
  vfs_session_t *session;
  //DataMgrType *dm;
  int battery;
  int finish;
  int paused;
  int launched;
  int spawner;
  int encoding, abgr;
  int width, height, full_height;
  int density, depth, hdepth, mono;
  int fullrefresh;
  int osversion;
  int locked;
  int dragging;
  int lastX, lastY;
  int current_task;
  uint32_t keyMask;
  uint32_t buttonMask;
  uint32_t modMask;
  uint64_t extKeyMask[2];
  uint32_t lockKey, lockModifiers;
  int64_t lastUpdate;
  int num_used;
  int task_order[MAX_TASKS];
  pumpkin_task_t tasks[MAX_TASKS];
  int num_tasks;
  int mode;
  language_t *lang;
  dia_t *dia;
  int dia_kbd;
  taskbar_t *taskbar;
  Boolean taskbar_enabled;
  wman_t *wm;
  int dragged, render;
  int refresh;
  pumpkin_serial_t serial[MAX_SERIAL];
  int num_serial;
  char clipboardAux[cbdMaxTextLength*2];
  uint32_t nextTaskId;
  pumpkin_plugin_t *plugin[MAX_PLUGINS];
  int num_plugins;
  RegMgrType *rm;
  notif_registration_t notif[MAX_NOTIF_REGISTER];
  int num_notif;
  FontType *fontPtrV1[128];
  FontType *fontPtrV2[128];
  event_t events[MAX_EVENTS];
  int nev, iev, oev;
  calibration_t calibration;
  surface_t *background;
  int audio, pcm, channels, rate;
  void *local_storage[last_key];
  char *mount;
  Int32 widget_id;
  logtrap_def ltdef;
  char *vertex_shader;
  int vlen;
  char *fragment_shader;
  int flen;
  float (*shader_getvar)(char *name, void *data);
  void *shader_data;
  int shader_inited;
} pumpkin_module_t;

typedef union {
  uint32_t t;
  uint8_t c[4];
} creator_id_t;

typedef struct {
  surface_t *surface;
  UInt32 creator;
} save_screen_t;

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
  tos8x16Font, tos8x8Font,
  -1
};

static const RGBColorType monoBackground        = { 0, 0xFF, 0xFF, 0xFF };
static const RGBColorType colorBackground       = { 0, 0x50, 0x80, 0xB0 };

static const RGBColorType monoSelectedBorder    = { 0, 0x90, 0x90, 0x90 };
static const RGBColorType monoUnselectedBorder  = { 0, 0xD0, 0xD0, 0xD0 };
static const RGBColorType monoLockedBorder      = { 0, 0xF0, 0xC0, 0x80 };

static const RGBColorType colorSelectedBorder   = { 0, 0xA0, 0xC0, 0xF0 };
static const RGBColorType colorUnselectedBorder = { 0, 0xA0, 0xA0, 0xA0 };
static const RGBColorType colorLockedBorder     = { 0, 0xF0, 0xC0, 0x80 };

static mutex_t *mutex;
static pumpkin_module_t pumpkin_module;
static thread_key_t *task_key;

static void pumpkin_make_current(int i);
static uint32_t pumpkin_launch_request(LocalID dbID, char *name, UInt16 cmd, launch_union_t *param, UInt16 flags, PilotMainF pilotMain, UInt16 opendb);

void *pumpkin_heap_base(void) {
  return heap_base(heap_get());
}

uint32_t pumpkin_heap_size(void) {
  return heap_size(heap_get());
}

void pumpkin_generic_error(char *msg, int code) {
  if (pumpkin_is_m68k()) {
    emupalmos_panic(msg, code);
  } else {
   SysFatalAlert(msg);
   pumpkin_fatal_error(0);
  }
}

void heap_exhausted_error(void) {
  pumpkin_generic_error("Heap exhausted", EMUPALMOS_HEAP_EXHAUSTED);
}

void heap_assertion_error(char *msg) {
  pumpkin_fatal_error(0);
}

void pumpkin_test_exception(int fatal) {
  debug(DEBUG_ERROR, PUMPKINOS, "pumpkin_test_exception fatal=%d", fatal);
  ErrDisplayFileLineMsgEx("test.c", "test", 1, "test exception", fatal);
  debug(DEBUG_ERROR, PUMPKINOS, "pumpkin_test_exception should not be here!");
}

void *pumpkin_heap_alloc(uint32_t size, char *tag) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  void *p;

  p = heap_alloc(task ? task->heap : pumpkin_module.heap, size);
  debug(DEBUG_TRACE, "Heap", "pumpkin_heap_alloc %s %u : %p", tag, size, p);
  if (p) {
    sys_memset(p, 0, size);
  }

  return p;
}

void *pumpkin_heap_realloc(void *p, uint32_t size, char *tag) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  void *q = NULL;

  if (p) {
    q = size ? heap_realloc(task ? task->heap : pumpkin_module.heap, p, size) : NULL;
  }
  debug(DEBUG_TRACE, "Heap", "pumpkin_heap_realloc %s %u %p : %p", tag, size, p, q);

  return q;
}

void pumpkin_heap_free(void *p, char *tag) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  debug(DEBUG_TRACE, "Heap", "pumpkin_heap_free %s %p", tag, p);
  if (p) {
    heap_free(task ? task->heap : pumpkin_module.heap, p);
  }
}

void *pumpkin_heap_dup(void *p, uint32_t size, char *tag) {
  void *q = NULL;

  if (p && size) {
    q = pumpkin_heap_alloc(size, tag);
    if (q) sys_memcpy(q, p, size);
  }
  debug(DEBUG_TRACE, "Heap", "pumpkin_heap_dup %s %u %p : %p", tag, size, p, q);

  return q;
}

void pumpkin_heap_dump(void) {
  if (mutex_lock(mutex) == 0) {
    heap_dump(pumpkin_module.heap);
    mutex_unlock(mutex);
  }
}

#if defined(HEAP_DEBUG)
int pumpkin_heap_debug_access(uint32_t offset, uint32_t size, int read) {
  return heap_debug_access(heap_get(), offset, size, read);
}
#endif

heap_t *heap_get(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  return task ? task->heap : pumpkin_module.heap;
}

static int pumpkin_register_plugin(UInt32 type, UInt32 id, pluginMainF pluginMain) {
  pumpkin_plugin_t *plugin;
  char sType[8], sId[8];
  int r = -1;

  if (pumpkin_module.num_plugins < MAX_PLUGINS) {
    if ((plugin = sys_calloc(1, sizeof(pumpkin_plugin_t))) != NULL) {
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

/*
static void SysNotifyLoadCallback(UInt32 creator, UInt16 seq, UInt16 index, UInt16 id, void *p, UInt16 size, void *data) {
  AppRegistryNotification *n = (AppRegistryNotification *)p;
  char stype[8], screator[8];

  pumpkin_id2s(n->appCreator, screator);
  pumpkin_id2s(n->notifyType, stype);

  if (pumpkin_module.num_notif < MAX_NOTIF_REGISTER) {
    pumpkin_module.notif[pumpkin_module.num_notif].taskId = -1;
    pumpkin_module.notif[pumpkin_module.num_notif].appCreator = n->appCreator;
    pumpkin_module.notif[pumpkin_module.num_notif].notifyType = n->notifyType;
    pumpkin_module.notif[pumpkin_module.num_notif].priority = n->priority;
    pumpkin_module.notif[pumpkin_module.num_notif].ptr = 0;
    pumpkin_module.num_notif++;
    debug(DEBUG_INFO, PUMPKINOS, "load notification type '%s' creator '%s' priority %d: added", stype, screator, n->priority);
  } else {
    debug(DEBUG_ERROR, PUMPKINOS, "load notification type '%s' creator '%s' priority %d: ignored (%d)", stype, screator, n->priority, pumpkin_module.num_notif);
  }
}
*/

FontType *pumpkin_get_font(FontID fontId, UInt16 density) {
  FontType *font = NULL;

  if (fontId >= 0 && fontId < 128) {
    font = density == kDensityLow ? pumpkin_module.fontPtrV1[fontId] : pumpkin_module.fontPtrV2[fontId];
  }

  return font;
}

static void pumpkin_load_fonts(void) {
  UInt16 index;
  FontID fontId;
  MemHandle handle;
  FontPtr f;

  for (index = 0; systemFonts[index] >= 0; index++) {
    fontId = systemFonts[index];

    if ((handle = DmGetResource(fontRscType, FONT_LOW_BASE + fontId)) != NULL) {
      if ((f = MemHandleLock(handle)) != NULL) {
        pumpkin_module.fontPtrV1[fontId] = FntCopyFont(f);
        MemHandleUnlock(handle);
      }
      DmReleaseResource(handle);
    } else {
      debug(DEBUG_ERROR, PUMPKINOS, "built-in V1 font resource id %d not found", fontId);
    }

    if ((handle = DmGetResource(fontExtRscType, FONT_DOUBLE_BASE + fontId)) != NULL) {
      if ((f = MemHandleLock(handle)) != NULL) {
        pumpkin_module.fontPtrV2[fontId] = FntCopyFont(f);
        MemHandleUnlock(handle);
      }
      DmReleaseResource(handle);
    } else {
      debug(DEBUG_ERROR, PUMPKINOS, "built-in V2 font resource id %d not found", fontId);
    }
  }
}

static void pumpkin_unload_fonts(void) {
  UInt16 fontId;

  for (fontId = 0; fontId < 128; fontId++) {
    if (pumpkin_module.fontPtrV1[fontId]) {
      FntFreeFont((FontPtr)pumpkin_module.fontPtrV1[fontId]);
    }
    if (pumpkin_module.fontPtrV2[fontId]) {
      FntFreeFont((FontPtr)pumpkin_module.fontPtrV2[fontId]);
    }
  }
}

int pumpkin_set_local_storage(local_storage_key_t key, void *p) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  int r = -1;

  if (key >= 0 && key < last_key) {
    if (task && pumpkin_module.mode != 1) {
      task->local_storage[key] = p;
    } else {
      pumpkin_module.local_storage[key] = p;
    }
    r = 0;
  }

  return r;
}

void *pumpkin_get_local_storage(local_storage_key_t key) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  void *p = NULL;

  if (key >= 0 && key < last_key) {
    if (task && pumpkin_module.mode != 1) {
      p = task->local_storage[key];
    } else {
      p = pumpkin_module.local_storage[key];
    }
  }

  return p;
}

int pumpkin_global_init(script_engine_t *engine, window_provider_t *wp, audio_provider_t *ap, bt_provider_t *bt, gps_parse_line_f gps_parse_line) {
  int fd;

  sys_memset(&pumpkin_module, 0, sizeof(pumpkin_module_t));

  if ((mutex = mutex_create(PUMPKINOS)) == NULL) {
    return -1;
  }

  if ((pumpkin_module.fs_mutex = mutex_create("fs")) == NULL) {
    return -1;
  }
  pumpkin_module.session = vfs_open_session();

  task_key = thread_key();

  pumpkin_module.engine = engine;
  pumpkin_module.wp = wp;
  pumpkin_module.ap = ap;
  pumpkin_module.bt = bt;
  pumpkin_module.gps_parse_line = gps_parse_line;
  pumpkin_module.current_task = -1;
  pumpkin_module.dragging = -1;
  pumpkin_module.nextTaskId = 1;
  pumpkin_module.battery = 100;
  pumpkin_module.osversion = 54;

  pumpkin_module.pcm = PCM_S16;
  pumpkin_module.channels = 1;
  pumpkin_module.rate = 44100;

  pumpkin_remove_locks(pumpkin_module.session, APP_STORAGE);

  pumpkin_module.heap = heap_init(NULL, HEAP_SIZE*8, SMALL_HEAP_SIZE, wp);
  StoInit(APP_STORAGE, pumpkin_module.fs_mutex);
  //pumpkin_module.dm = DataMgrInit("/app_data/");
  //DataMgrInitModule(pumpkin_module.dm);

  SysUInitModule(); // sto calls SysQSortP

  pumpkin_module.rm = RegInit();
  pumpkin_module.num_notif = 0;

  emupalmos_init(&pumpkin_module.ltdef);

  if (ap && ap->mixer_init) ap->mixer_init();

  if ((fd = sys_open(CRASH_LOG, SYS_WRITE)) == -1) {
    fd = sys_create(CRASH_LOG, SYS_WRITE, 0644);
  }
  if (fd != -1) {
    sys_close(fd);
  }

  return 0;
}

logtrap_def *logtrap_get_def(void) {
  return &pumpkin_module.ltdef;
}

void pumpkin_deploy_files(char *path) {
  if (mutex_lock(pumpkin_module.fs_mutex) == 0) {
    pumpkin_deploy_files_session(pumpkin_module.session, path);
    mutex_unlock(pumpkin_module.fs_mutex);
  }
}

void pumpkin_init_misc(void) {
  DmOpenRef dbRef;
  LocalID dbID;
  PumpkinPreferencesType prefs;

  dbID = DmFindDatabase(0, BOOT_NAME);
  dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly);
  PrefInitModule();
  pumpkin_load_fonts();
  DmCloseDatabase(dbRef);

  if (pumpkin_get_preference(BOOT_CREATOR, PUMPKINOS_PREFS_ID, &prefs, sizeof(PumpkinPreferencesType), true) == 0) {
    MemSet(&prefs, sizeof(PumpkinPreferencesType), 0);
    prefs.version = PUMPKINOS_PREFS_VERSION;
    prefs.value[pLockKey] = WINDOW_KEY_F10;
    prefs.value[pLockModifiers] = WINDOW_MOD_SHIFT;
    prefs.value[pBorderWidth] = BORDER_SIZE;
    prefs.value[pBackgroundImage] = 0;
    prefs.color[pMonoBackground] = monoBackground;
    prefs.color[pMonoSelectedBorder] = monoSelectedBorder;
    prefs.color[pMonoUnselectedBorder] = monoUnselectedBorder;
    prefs.color[pMonoLockedBorder] = monoLockedBorder;
    prefs.color[pColorBackground] = colorBackground;
    prefs.color[pColorSelectedBorder] = colorSelectedBorder;
    prefs.color[pColorUnselectedBorder] = colorUnselectedBorder;
    prefs.color[pColorLockedBorder] = colorLockedBorder;
    pumpkin_set_preference(BOOT_CREATOR, PUMPKINOS_PREFS_ID, &prefs, sizeof(PumpkinPreferencesType), true);
  }

  pumpkin_module.lockKey = prefs.value[pLockKey];
  pumpkin_module.lockModifiers = prefs.value[pLockModifiers];
}

void pumpkin_set_spawner(int handle) {
  debug(DEBUG_INFO, PUMPKINOS, "spawner set to port %d", handle);
  pumpkin_module.spawner = handle;
  wman_clear(pumpkin_module.wm);
}

int pumpkin_is_spawner(void) {
  return thread_get_handle() == pumpkin_module.spawner;
}

int pumpkin_get_spawner(void) {
  return pumpkin_module.spawner;
}

static int drop_get_name(char *src, char *dst, int max) {
  int len, i;

  if (!src) return -1;
  if (sys_strchr(src, '%')) return -1;
  if (sys_strstr(src, "..")) return -1;

  len = sys_strlen(src);
  if (len < 2) return -1;
  i = len-1;
  if (src[i] == FILE_SEP) return -1;

  for (;;) {
    if (i == 0) break;
    if (src[i] == FILE_SEP) break;
    i--;
  }

  if (src[i] != FILE_SEP) return -1;

  sys_snprintf(dst, max, "%s%s%s", pumpkin_module.mount, &VFS_INSTALL[1], &src[i+1]);
  return 0;
}

static void drop_deploy(char *file, void *data) {
  client_request_t creq;
  uint8_t *buf;
  char name[256];
  int nr, nw, rfd, wfd = 0;
  int r = 0;

  debug(DEBUG_INFO, PUMPKINOS, "reading \"%s\"", file);
  if (pumpkin_module.mount && drop_get_name(file, name, sizeof(name)-1) == 0) {
    debug(DEBUG_INFO, PUMPKINOS, "writing \"%s\"", name);
    if ((rfd = sys_open(file, SYS_READ)) != -1) {
      if ((wfd = sys_create(name, SYS_WRITE | SYS_TRUNC, 0644)) != -1) {
        if ((buf = (uint8_t *)sys_malloc(65536)) != NULL) {
          for (;;) {
            nr = sys_read(rfd, buf, 65536);
            if (nr <= 0) break;
            nw = sys_write(wfd, buf, nr);
            if (nw != nr) {
              nr = -1;
              break;
            }
          }
          r = (nr == 0);
          sys_free(buf);
        }
        sys_close(wfd);
      }
      sys_close(rfd);
    }
  } else {
    debug(DEBUG_ERROR, PUMPKINOS, "invalid name \"%s\"", file);
  }

  if (r) {
    debug(DEBUG_TRACE, PUMPKINOS, "send MSG_DEPLOY");
    sys_memset(&creq, 0, sizeof(client_request_t));
    creq.type = MSG_DEPLOY;
    thread_client_write(pumpkin_get_spawner(), (uint8_t *)&creq, sizeof(client_request_t));
  } else {
    debug(DEBUG_ERROR, PUMPKINOS, "deploy \"%s\" failed", file);
  }
}

void pumpkin_set_window(window_t *w, int width, int height, int full_height) {
  debug(DEBUG_INFO, PUMPKINOS, "set window %dx%d (%dx%d)", width, height, width, full_height);
  pumpkin_module.w = w;
  pumpkin_module.width = width;
  pumpkin_module.height = height;
  pumpkin_module.full_height = full_height;

  pumpkin_module.wm = wman_init(pumpkin_module.wp, w, width, height);

  if (pumpkin_module.wp->drop_file) {
    pumpkin_module.wp->drop_file(w, drop_deploy, NULL);
  }
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
      pumpkin_module.encoding = pumpkin_module.abgr ? SURFACE_ENCODING_ABGR : SURFACE_ENCODING_ARGB;
      break;
    default:
      pumpkin_module.encoding = SURFACE_ENCODING_RGB565;
      break;
  }
}

int pumpkin_get_encoding(void) {
  return pumpkin_module.encoding;
}

void pumpkin_set_secure(void *secure) {
  pumpkin_module.secure = secure;
}

static void pumpkin_set_host_depth(int depth) {
  PumpkinPreferencesType prefs;
  UInt32 border;

  if (depth > 0) {
    pumpkin_module.hdepth = depth;
    pumpkin_set_encoding(depth);
    debug(DEBUG_INFO, PUMPKINOS, "host screen depth is %d", pumpkin_module.hdepth);
  }
  pumpkin_get_preference(BOOT_CREATOR, PUMPKINOS_PREFS_ID, &prefs, sizeof(PumpkinPreferencesType), true);
  border = prefs.value[pBorderWidth];
  if (pumpkin_module.density == kDensityLow) border /= 2;

  if (pumpkin_module.mono) {
    wman_set_border(pumpkin_module.wm, pumpkin_module.hdepth, border,
      prefs.color[pMonoSelectedBorder].r,   prefs.color[pMonoSelectedBorder].g,   prefs.color[pMonoSelectedBorder].b,
      prefs.color[pMonoLockedBorder].r,     prefs.color[pMonoLockedBorder].g,     prefs.color[pMonoLockedBorder].b,
      prefs.color[pMonoUnselectedBorder].r, prefs.color[pMonoUnselectedBorder].g, prefs.color[pMonoUnselectedBorder].b);

    wman_set_background(pumpkin_module.wm, pumpkin_module.hdepth,
      prefs.color[pMonoBackground].r, prefs.color[pMonoBackground].g, prefs.color[pMonoBackground].b);

  } else {
    if (pumpkin_module.abgr) {
      wman_set_border(pumpkin_module.wm, pumpkin_module.hdepth, border,
        prefs.color[pColorSelectedBorder].b,   prefs.color[pColorSelectedBorder].g,   prefs.color[pColorSelectedBorder].r,
        prefs.color[pColorLockedBorder].b,     prefs.color[pColorLockedBorder].g,     prefs.color[pColorLockedBorder].r,
        prefs.color[pColorUnselectedBorder].b, prefs.color[pColorUnselectedBorder].g, prefs.color[pColorUnselectedBorder].r);
      
      wman_set_background(pumpkin_module.wm, pumpkin_module.hdepth,
        prefs.color[pColorBackground].b, prefs.color[pColorBackground].g, prefs.color[pColorBackground].r);
    } else {
      wman_set_border(pumpkin_module.wm, pumpkin_module.hdepth, border,
        prefs.color[pColorSelectedBorder].r,   prefs.color[pColorSelectedBorder].g,   prefs.color[pColorSelectedBorder].b,
        prefs.color[pColorLockedBorder].r,     prefs.color[pColorLockedBorder].g,     prefs.color[pColorLockedBorder].b,
        prefs.color[pColorUnselectedBorder].r, prefs.color[pColorUnselectedBorder].g, prefs.color[pColorUnselectedBorder].b);

      wman_set_background(pumpkin_module.wm, pumpkin_module.hdepth,
        prefs.color[pColorBackground].r, prefs.color[pColorBackground].g, prefs.color[pColorBackground].b);
    }
  }
}

void pumpkin_set_abgr(int abgr) {
  pumpkin_module.abgr = abgr;
}

static void pumpkin_dia_kbd(void) {
  WinHandle lower_wh, upper_wh, number_wh;
  UInt16 prev;
  int dw, dh;
  Err err;
  RectangleType bounds[256];

  dia_get_graffiti_dimension(pumpkin_module.dia, &dw, &dh);
  prev = WinSetCoordinateSystem(pumpkin_module.density == kDensityDouble ? kCoordinatesDouble : kCoordinatesStandard);
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

void pumpkin_set_obj(int pe, script_ref_t obj) {
  pumpkin_module.pe = pe;
  pumpkin_module.obj = obj;
}

static int pumpkin_get_option(char *name, script_arg_t *value) {
  script_arg_t key;

  key.type = SCRIPT_ARG_STRING;
  key.value.s = name;

  return script_object_get(pumpkin_module.pe, pumpkin_module.obj, &key, value);
}

int pumpkin_get_boolean_option(char *name) {
  script_arg_t value;
  int n, r = 0;

  if (pumpkin_get_option(name, &value) == 0) {
    switch (value.type) {
      case SCRIPT_ARG_LSTRING:
        n = value.value.l.n < 4 ? value.value.l.n : 4;
        r = sys_strncmp(value.value.l.s, "true", n) == 0 ? 1 : 0;
        break;
      case SCRIPT_ARG_INTEGER:
        r = value.value.i ? 1 : 0;
        break;
      case SCRIPT_ARG_BOOLEAN:
        r = value.value.i;
        break;
    }
  }

  return r;
}

int pumpkin_get_integer_option(char *name) {
  script_arg_t value;
  char buf[32];
  int n, r = 0;

  if (pumpkin_get_option(name, &value) == 0) {
    switch (value.type) {
      case SCRIPT_ARG_LSTRING:
        n = value.value.l.n < sizeof(buf) ? value.value.l.n : sizeof(buf);
        sys_strncpy(buf, value.value.l.s, n - 1);
        r = sys_atoi(buf);
        break;
      case SCRIPT_ARG_INTEGER:
        r = value.value.i;
        break;
      case SCRIPT_ARG_BOOLEAN:
        r = value.value.i;
        break;
    }
  }

  return r;
}

uint32_t pumpkin_get_id_option(char *name) {
  script_arg_t value;
  uint32_t r = 0;

  if (pumpkin_get_option(name, &value) == 0) {
    switch (value.type) {
      case SCRIPT_ARG_LSTRING:
        if (value.value.l.n == 4) {
          r = pumpkin_s2id(&r, value.value.l.s);
        }
        break;
    }
  }

  return r;
}

char *pumpkin_get_string_option(char *name) {
  script_arg_t value;
  char *r = NULL;

  if (pumpkin_get_option(name, &value) == 0) {
    switch (value.type) {
      case SCRIPT_ARG_LSTRING:
        r = value.value.l.s;
        break;
    }
  }

  return r;
}

void pumpkin_set_mode(int mode, int dia, int depth) {
  LocalID dbID;
  DmOpenRef dbRef;
  uint32_t language;

  if (mode < 0 || mode > 2) mode = 0;
  pumpkin_module.mode = mode;

  if (mode == 0) {
    pumpkin_set_host_depth(depth);
    dia = 0;
  } else {
    pumpkin_module.hdepth = depth;
    pumpkin_set_encoding(depth);
  }

  if (dia) {
    pumpkin_module.dia = dia_init(pumpkin_module.wp, pumpkin_module.w, pumpkin_module.encoding, depth, pumpkin_module.density == kDensityDouble);
  }

  if (pumpkin_module.mode == 2) {
    dbID = DmFindDatabase(0, BOOT_NAME);
    dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly);
    language = PrefGetPreference(prefLanguage);
    pumpkin_module.lang = LanguageInit(language);
    UicInitModule();
    WinInitModule(pumpkin_module.density, pumpkin_module.width, pumpkin_module.height, pumpkin_module.depth, NULL);
    FntInitModule(pumpkin_module.density);
    FrmInitModule();
    KeyboardInitModule();
    if (dia) {
      pumpkin_dia_kbd();
    }
    DmCloseDatabase(dbRef);
    debug(DEBUG_INFO, PUMPKINOS, "mode 2 inited");

    if (pumpkin_module.wp->average) {
      UInt16 size = 0;
      if (PrefGetAppPreferences('toch', 1, &pumpkin_module.calibration, &size, true) == noPreferenceFound) {
        pumpkin_calibrate(0);
      }
    }
  }
}

int pumpkin_get_mode(void) {
  return pumpkin_module.mode;
}

int pumpkin_dia_enabled(void) {
  return pumpkin_module.dia ? 1 : 0;
}

static void pumpkin_image_background(RGBColorType *rgb, UInt16 id, UInt16 mode) {
  LocalID dbID;
  DmOpenRef dbRef;
  BitmapType *bmp;
  MemHandle h;
  Coord width, height;
  UInt32 x, y, color;

  if ((dbID = DmFindDatabase(0, "Background")) != 0) {
    if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
      if ((h = DmGet1Resource(bitmapRsc, id)) != NULL) {
        if ((bmp = MemHandleLock(h)) != NULL) {
          if (pumpkin_module.background == NULL) {
            pumpkin_module.background = surface_create(pumpkin_module.width, pumpkin_module.height, pumpkin_module.encoding);
          }
          if (pumpkin_module.background != NULL) {
            color = surface_color_rgb(pumpkin_module.encoding, NULL, 0, rgb->r, rgb->g, rgb->b, 0xFF);
            surface_rectangle(pumpkin_module.background, 0, 0, pumpkin_module.width-1, pumpkin_module.height-1, 1, color);
            BmpGetDimensions(bmp, &width, &height, NULL);

            switch (mode) {
              case 0: // top left
                BmpDrawSurface(bmp, 0, 0, width, height, pumpkin_module.background, 0, 0, true, false);
                break;
              case 1: // top right
                BmpDrawSurface(bmp, 0, 0, width, height, pumpkin_module.background, pumpkin_module.width - width, 0, true, false);
                break;
              case 2: // bottom left
                BmpDrawSurface(bmp, 0, 0, width, height, pumpkin_module.background, 0, pumpkin_module.height - height, true, false);
                break;
              case 3: // bottom right
                BmpDrawSurface(bmp, 0, 0, width, height, pumpkin_module.background, pumpkin_module.width - width, pumpkin_module.height - height, true, false);
                break;
              case 4: // center
                BmpDrawSurface(bmp, 0, 0, width, height, pumpkin_module.background, (pumpkin_module.width - width) / 2, (pumpkin_module.height - height) / 2, true, false);
                break;
              case 5: // tiled
                for (y = 0; y < pumpkin_module.height; y += height) {
                  for (x = 0; x < pumpkin_module.width; x += width) {
                    BmpDrawSurface(bmp, 0, 0, width, height, pumpkin_module.background, x, y, true, false);
                  }
                }
                break;
            }
          }
          MemHandleUnlock(h);
        }
        DmReleaseResource(h);
      }
      DmCloseDatabase(dbRef);
    }
  }
}

void pumpkin_set_density(int density) {
  switch (density) {
    case kDensityLow:
      pumpkin_module.density = density;
      break;
    default:
      pumpkin_module.density = kDensityDouble;
      break;
  }
  debug(DEBUG_INFO, PUMPKINOS, "screen density is %d", pumpkin_module.density);
}

int pumpkin_get_density(void) {
  return pumpkin_module.density;
}

void pumpkin_set_depth(int depth) {
  switch (depth) {
    case   1:
    case   2:
    case   4:
    case   8:
    case  16:
    case  32:
      pumpkin_module.depth = depth;
      break;
    default:
      pumpkin_module.depth = 16;
      break;
  }
  debug(DEBUG_INFO, PUMPKINOS, "screen depth is %d", pumpkin_module.depth);
}

int pumpkin_get_depth(void) {
  return pumpkin_module.depth;
}

void pumpkin_refresh_desktop(void) {
  PumpkinPreferencesType prefs;

  if (mutex_lock(mutex) == 0) {
    pumpkin_get_preference(BOOT_CREATOR, PUMPKINOS_PREFS_ID, &prefs, sizeof(PumpkinPreferencesType), true);
    if (prefs.value[pBackgroundImage] & 0xFFFF) {
      pumpkin_image_background(&prefs.color[pColorBackground], prefs.value[pBackgroundImage] & 0xFFFF, prefs.value[pBackgroundImage] >> 16);
    } else if (pumpkin_module.background) {
      surface_destroy(pumpkin_module.background);
      pumpkin_module.background = NULL;
    }
    pumpkin_module.refresh = 1;
    pumpkin_module.render = 1;
    mutex_unlock(mutex);
  }
}

void pumpkin_set_mono(int mono) {
  pumpkin_module.mono = mono;
}

void pumpkin_set_fullrefresh(int fullrefresh) {
  pumpkin_module.fullrefresh = fullrefresh;
}

void pumpkin_set_taskbar(int enabled) {
  pumpkin_module.taskbar_enabled = enabled;
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

void pumpkin_taskbar_create(void) {
  if (pumpkin_module.mode == 0 && pumpkin_module.taskbar_enabled) {
    pumpkin_module.taskbar = taskbar_create(pumpkin_module.wp, pumpkin_module.w,
        pumpkin_module.density, 0, pumpkin_module.height - TASKBAR_HEIGHT, pumpkin_module.width, TASKBAR_HEIGHT, pumpkin_module.encoding);
  }
}

Int32 pumpkin_taskbar_add_widget(UInt16 bmpID) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  Int32 id = -1;

  if (task && bmpID) {
    if (mutex_lock(mutex) == 0) {
      if (pumpkin_module.taskbar) {
        if (taskbar_add_widget(pumpkin_module.taskbar, task->taskId, pumpkin_module.widget_id, pumpkin_get_app_localid(), bmpID)) {
          id = pumpkin_module.widget_id++;
          debug(DEBUG_INFO, PUMPKINOS, "added widget %u to taskbar", id);
        }
      }
      mutex_unlock(mutex);
    }
  }

  return id;
}

Boolean pumpkin_taskbar_remove_widget(Int32 id) {
  Boolean r = false;

  if (id >= 0) {
    if (mutex_lock(mutex) == 0) {
      if (pumpkin_module.taskbar) {
        r = taskbar_remove_widget(pumpkin_module.taskbar, id);
        if (r) {
          debug(DEBUG_INFO, PUMPKINOS, "removed widget %d from taskbar", id);
        }
      }
      mutex_unlock(mutex);
    }
  }

  return r;
}

void pumpkin_taskbar_add(LocalID dbID, UInt32 creator, char *name) {
  int i;

  if (dbID && creator && name) {
    if (mutex_lock(mutex) == 0) {
      if (pumpkin_module.taskbar) {
        for (i = 0; i < pumpkin_module.num_tasks; i++) {
          if (pumpkin_module.tasks[i].creator == creator) {
            taskbar_add(pumpkin_module.taskbar, pumpkin_module.tasks[i].taskId, dbID, creator, name);
            break;
          }
        }
      }
      mutex_unlock(mutex);
    }
  }
}

void pumpkin_taskbar_remove(LocalID dbID) {
  if (dbID) {
    if (mutex_lock(mutex) == 0) {
      if (pumpkin_module.taskbar) {
        taskbar_remove(pumpkin_module.taskbar, dbID);
      }
      mutex_unlock(mutex);
    }
  }
}

void pumpkin_taskbar_update(void) {
  if (mutex_lock(mutex) == 0) {
    if (pumpkin_module.taskbar) {
      taskbar_update(pumpkin_module.taskbar);
    }
    mutex_unlock(mutex);
  }
}

void pumpkin_taskbar_destroy(void) {
  if (mutex_lock(mutex) == 0) {
    if (pumpkin_module.taskbar) {
      taskbar_destroy(pumpkin_module.taskbar);
      pumpkin_module.taskbar = NULL;
    }
    mutex_unlock(mutex);
  }
}

void pumpkin_taskbar_ui(int show) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  int x,y;

  if (mutex_lock(mutex) == 0) {
    if (show) {
      x = (pumpkin_module.width - task->width) / 2;
      y = (pumpkin_module.height - task->height) / 2;
      wman_add(pumpkin_module.wm, task->taskId, task->texture, x, y, task->width, task->height);
    } else {
      wman_remove(pumpkin_module.wm, task->taskId, 0);
    }
    mutex_unlock(mutex);
  }
}

int pumpkin_global_finish(void) {
  int i;

  pumpkin_unload_fonts();
  PrefFinishModule();

  if (pumpkin_module.ap && pumpkin_module.ap->finish && pumpkin_module.audio > 0) {
    pumpkin_module.ap->finish(pumpkin_module.audio);
  }

  if (pumpkin_module.background) {
    surface_destroy(pumpkin_module.background);
  }

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
    sys_free(pumpkin_module.serial[i].descr);
  }

  for (i = 0; i < pumpkin_module.num_plugins; i++) {
    sys_free(pumpkin_module.plugin[i]);
  }

  RegFinish(pumpkin_module.rm);

  logtrap_global_finish(&pumpkin_module.ltdef);

  SysUFinishModule();
  StoFinish();
  //DataMgrFinishModule();
  //DataMgrFinish(pumpkin_module.dm);
  heap_finish(pumpkin_module.heap);
  thread_key_delete(task_key);
  vfs_close_session(pumpkin_module.session);
  mutex_destroy(pumpkin_module.fs_mutex);
  mutex_destroy(mutex);

  return 0;
}

void *pumpkin_reg_get(DmResType type, UInt16 id, UInt32 *size) {
  return RegGet(pumpkin_module.rm, type, id, size);
}

Err pumpkin_reg_set(DmResType type, UInt16 id, void *p, UInt32 size) {
  return RegSet(pumpkin_module.rm, type, id, p, size);
}

static void task_destructor(void *p) {
  task_screen_t *screen;

  if (p) {
    screen = (task_screen_t *)p;
    if (screen->surface) surface_destroy(screen->surface);
    if (screen->msurface) surface_destroy(screen->msurface);
    sys_free(screen);
  }
}

int pumpkin_ps(int (*ps_callback)(int i, uint32_t id, char *name, int m68k, void *data), void *data) {
  int i, r = -1;

  if (ps_callback) {
    if (mutex_lock(mutex) == 0) {
      for (i = 0; i < MAX_TASKS; i++) {
        if (pumpkin_module.tasks[i].active) {
          if (ps_callback(i, pumpkin_module.tasks[i].taskId, pumpkin_module.tasks[i].name, pumpkin_module.tasks[i].m68k, data) != 0) break;
        }
      }
      mutex_unlock(mutex);
      r = 0;
    }
  }

  return r;
}

int pumpkin_kill(uint32_t tid) {
  int i, r = -1;

  if (mutex_lock(mutex) == 0) {
    for (i = 0; i < MAX_TASKS; i++) {
      if (pumpkin_module.tasks[i].active && tid == pumpkin_module.tasks[i].taskId) {
        thread_end(TAG_APP, pumpkin_module.tasks[i].handle);
        r = 0;
        break;
      }
    }
    mutex_unlock(mutex);
  }

  return r;
}

static int pumpkin_normal_launch(uint16_t cmd) {
  switch (cmd) {
    case sysAppLaunchCmdNormalLaunch:
    case sysAppLaunchCmdPanelCalledFromApp:
    case sysAppLaunchCmdReturnFromPanel:
      return 1;
  }

  return 0;
}

static int pumpkin_pilotmain(char *name, PilotMainF pilotMain, uint16_t code, launch_union_t *param, uint16_t flags) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  SysAppLaunchCmdSystemResetType reset;
  RegFlagsType *regFlagsP, regFlags;
  RegOsType regOS;
  LocalID oldDbID, dbID;
  DmOpenRef dbRef;
  Boolean callNormal;
  UInt32 oldCreator, creator, regSize;

  if ((dbID = DmFindDatabase(0, name)) != 0) {
    if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
      if (DmDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creator) == errNone) {
        oldDbID = task->dbID;
        oldCreator = task->creator;
        task->dbID = dbID;
        task->creator = creator;

        callNormal = true;

        if (code == sysAppLaunchCmdNormalLaunch) {
          if (mutex_lock(mutex) == 0) {
            if ((regFlagsP = pumpkin_reg_get(creator, regFlagsID, &regSize)) != NULL) {
              regFlags.flags = regFlagsP->flags;
              MemPtrFree(regFlagsP);
            } else {
              regFlags.flags = regFlagReset;
            }

            if (pumpkin_reg_get(creator, regOsID, &regSize) == NULL) {
              regOS.version = pumpkin_get_default_osversion();
              pumpkin_reg_set(creator, regOsID, &regOS, sizeof(RegOsType));
            }
            mutex_unlock(mutex);

            if (regFlags.flags & regFlagReset) {
              regFlags.flags &= ~regFlagReset;
              pumpkin_reg_set(creator, regFlagsID, &regFlags, sizeof(RegFlagsType));

              // Defer a sysAppLaunchCmdSystemReset for when the app is called for the first time.
              // It is not exactly a "reset", but it allows the app to initialize itself after being installed.
              // The Note Pad app uses this launch code to create its database with a sample record.
              debug(DEBUG_INFO, PUMPKINOS, "calling sysAppLaunchCmdSystemReset for \"%s\"", name);
              reset.hardReset = false;
              reset.createDefaultDB = true;
              if (pilotMain(sysAppLaunchCmdSystemReset, &reset, 0) != 0) {
                callNormal = false;
              }
            }
          }
        }

        if (callNormal) {
          pilotMain(code, param, flags);
        }

        if (!pumpkin_normal_launch(code) || pumpkin_module.mode != 0) {
          task->dbID = oldDbID;
          task->creator = oldCreator;
        }
      }
      DmCloseDatabase(dbRef);
    }
  }

  return 0;
}

static void sendLaunchNotification(UInt32 notifyType, UInt32 dbID) {
  SysNotifyParamType notify;
  SysNotifyAppLaunchOrQuitType launch;

  launch.version = 0;
  launch.dbID = dbID;
  launch.cardNo = 0;

  MemSet(&notify, sizeof(notify), 0);
  notify.notifyType = notifyType;
  notify.broadcaster = 0;
  notify.notifyDetailsP = &launch;
  SysNotifyBroadcast(&notify);
}

static uint32_t pumpkin_launch_sub(launch_request_t *request, int opendb) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  uint32_t (*pilot_main)(uint16_t code, void *param, uint16_t flags);
  uint32_t r = 0;
  LocalID dbID = 0;
  DmOpenRef dbRef;
  UInt32 creator;
  MemHandle h;
  Boolean firstLoad;
  void *lib;
  void **pumpkin_system_call_p;
  int m68k;

  if (request) {
    pilot_main = request->pilot_main;
    lib = NULL;

    if (pilot_main) {
      debug(DEBUG_INFO, PUMPKINOS, "using provided PilotMain");
    } else {
      debug(DEBUG_INFO, PUMPKINOS, "searching PilotMain in dlib");
      if ((dbID = DmFindDatabase(0, request->name)) != 0) {
        DmDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creator);
        if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
          if ((lib = DmResourceLoadLib(dbRef, sysRsrcTypeDlib, &firstLoad)) != NULL) {
            debug(DEBUG_INFO, PUMPKINOS, "dlib resource loaded (first %d)", firstLoad ? 1 : 0);
            pilot_main = sys_lib_defsymbol(lib, "PilotMain", 1);
            if (pilot_main == NULL) {
              debug(DEBUG_ERROR, PUMPKINOS, "PilotMain not found in dlib");
            } else {
              pumpkin_system_call_p = sys_lib_defsymbol(lib, "pumpkin_system_call_p", 0);
              if (pumpkin_system_call_p) {
                debug(DEBUG_INFO, PUMPKINOS, "setting syscall address for \"%s\"", request->name);
                *pumpkin_system_call_p = (void *)pumpkin_system_call;
              }
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

    if (pumpkin_normal_launch(request->code) && dbID) {
      sendLaunchNotification(sysNotifyAppLaunchingEvent, dbID);
    }

    if (pilot_main) {
      if (pumpkin_normal_launch(request->code)) {
        task->pilot_main = pilot_main;
      }
      pumpkin_set_m68k(0);
      if (opendb) {
        debug(DEBUG_INFO, PUMPKINOS, "calling pilot_main for \"%s\" with code %d as subroutine (opendb)", request->name, request->code);
        r = pumpkin_pilotmain(request->name, pilot_main, request->code, request->hasParam ? &request->param : NULL, request->flags);
      } else {
        debug(DEBUG_INFO, PUMPKINOS, "calling pilot_main for \"%s\" with code %d as subroutine", request->name, request->code);
        r = pilot_main(request->code, request->hasParam ? &request->param : NULL, request->flags);
      }
      debug(DEBUG_INFO, PUMPKINOS, "pilot_main \"%s\" returned %u", request->name, r);

    } else {
      if (pumpkin_normal_launch(request->code)) {
        task->pilot_main = emupalmos_main;
      }
      m68k = pumpkin_is_m68k();
      pumpkin_set_m68k(1);
      //task->tracing = 1;
      if (opendb) {
        debug(DEBUG_INFO, PUMPKINOS, "calling emupalmos_main for \"%s\" with code %d as subroutine (opendb)", request->name, request->code);
        r = pumpkin_pilotmain(request->name, emupalmos_main, request->code, request->hasParam ? &request->param : NULL, request->flags);
      } else {
        debug(DEBUG_INFO, PUMPKINOS, "calling emupalmos_main for \"%s\" with code %d as subroutine", request->name, request->code);
        r = emupalmos_main(request->code, request->hasParam ? &request->param : NULL, request->flags);
      }
      //task->tracing = 0;
      debug(DEBUG_INFO, PUMPKINOS, "emupalmos_main \"%s\" returned %u", request->name, r);
      pumpkin_set_m68k(m68k);
    }

    if (pumpkin_normal_launch(request->code) && dbID) {
      sendLaunchNotification(sysNotifyAppQuittingEvent, dbID);
    }

    if (lib) {
      sys_lib_close(lib);
    }
  }

  return r;
}

static uint32_t calibrate_x(int x, int y) {
  calibration_t *c = &pumpkin_module.calibration;
  x = ((c->a * x) + (c->b * y) + c->c) / c->div;
  if (x < 0) x = 0;
  else if (x >= pumpkin_module.width) x = pumpkin_module.width - 1;
  return x;
}

static uint32_t calibrate_y(int x, int y) {
  calibration_t *c = &pumpkin_module.calibration;
  y = ((c->d * x) + (c->e * y) + c->f) / c->div;
  if (y < 0) y = 0;
  else if (y >= pumpkin_module.full_height) y = pumpkin_module.full_height - 1;
  return y;
}

static void calibrate_test(char *label, int x, int y) {
  int cx, cy;

  cx = calibrate_x(x, y);
  cy = calibrate_y(x, y);
  debug(DEBUG_INFO, "TOUCH", "%-12s (%3d,%3d) mapped to (%3d,%3d)", label, x, y, cx, cy);
}

void pumpkin_calibrate(int restore) {
  int dx, dy;

  if (pumpkin_module.wp->average) {
    debug(DEBUG_INFO, "TOUCH", "calibrating touch screen");
    dx = pumpkin_module.width;
    dy = pumpkin_module.full_height;
    calibrate(pumpkin_module.wp, pumpkin_module.w, pumpkin_module.depth, dx, dy, &pumpkin_module.calibration);

    debug(DEBUG_INFO, "TOUCH", "calibration parameters a=%d b=%d c=%d d=%d e=%d f=%d div=%d",
      pumpkin_module.calibration.a, pumpkin_module.calibration.b, pumpkin_module.calibration.c,
      pumpkin_module.calibration.d, pumpkin_module.calibration.e, pumpkin_module.calibration.f,
      pumpkin_module.calibration.div);
    calibrate_test("top left    ", 0, 0);
    calibrate_test("top right   ", dx-1, 0);
    calibrate_test("bottom left ", 0, dy-1);
    calibrate_test("bottom right", dx-1, dy-1);
    calibrate_test("center      ", dx/2, dy/2);

    PrefSetAppPreferences('toch', 1, 1, &pumpkin_module.calibration, sizeof(calibration_t), true);
    if (restore) {
      FrmUpdateForm(FrmGetActiveFormID(), 0);
      dia_refresh(pumpkin_module.dia);
      dia_update(pumpkin_module.dia);
    }
  } else {
    debug(DEBUG_ERROR, "TOUCH", "average function is not implemented in window provider");
  }
}

static void pumpkin_init_midi(void) {
  DmOpenRef dbRef;
  DmResID id;
  MemHandle h;
  UInt32 size;
  UInt16 at;
  void *p;

  if ((dbRef = DmOpenDatabaseByTypeCreator(sysFileTMidi, sysFileCSystem, dmModeWrite)) == NULL) {
    debug(DEBUG_INFO, PUMPKINOS, "creating system MIDI database");
    if (DmCreateDatabase(0, "System MIDI Sounds", sysFileCSystem, sysFileTMidi, false) == errNone) {
      if ((dbRef = DmOpenDatabaseByTypeCreator(sysFileTMidi, sysFileCSystem, dmModeWrite)) != NULL) {
        for (id = 0; ; id++) {
          if ((h = DmGetResource('pmid', id)) == NULL) break;
          if ((p = MemHandleLock(h)) != NULL) {
            size = MemHandleSize(h);
            at = 0xFFFF;
            DmNewRecordEx(dbRef, &at, size, p, 0, 0, false);
            debug(DEBUG_INFO, PUMPKINOS, "adding MIDI sound %u at %u", id, at);
            MemHandleUnlock(h);
          }
          DmReleaseResource(h);
        }
        DmCloseDatabase(dbRef);
      }
    }
  } else {
    DmCloseDatabase(dbRef);
  }
}

static void pumpkin_init_icon(void) {
  surface_t *icon;
  BitmapType *bmp;
  MemHandle h;
  Coord width, height;
  UInt32 transparentValue;
  uint32_t *raw;
  int density, depth, len;

  if (pumpkin_module.wp->icon) {
    if ((h = DmGetResource(bitmapRsc, 10000)) != NULL) {
      if ((bmp = MemHandleLock(h)) != NULL) {
        bmp = BmpGetBestBitmap(bmp, pumpkin_module.density, pumpkin_module.depth);
        BmpGetDimensions(bmp, &width, &height, NULL);
        depth = BmpGetBitDepth(bmp);
        density = BmpGetDensity(bmp);
        BmpGetTransparentValue(bmp, &transparentValue);
        debug(DEBUG_INFO, PUMPKINOS, "set icon %dx%d, density %d, depth %d, transparent 0x%08X", width, height, density, depth, transparentValue);
        icon = surface_create(width, height, pumpkin_module.encoding);
        BmpDrawSurface(bmp, 0, 0, width, height, icon, 0, 0, true, false);
        raw = (uint32_t *)icon->getbuffer(icon->data, &len);
        pumpkin_module.wp->icon(pumpkin_module.w, raw, width, height);
        surface_destroy(icon);
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    }
  }
}

static int pumpkin_local_init(int i, uint32_t taskId, texture_t *texture, uint32_t creator, char *name, int width, int height, int x, int y) {
  pumpkin_task_t *task;
  task_screen_t *screen;
  PumpkinPreferencesType prefs;
  RegDisplayType *regDisplay;
  RegOsType *regOS;
  LocalID dbID;
  UInt32 language, regSize;
  UInt16 size;
  uint32_t color;
  int j, ptr;

  if (mutex_lock(mutex) == -1) {
    return -1;
  }

  if ((task = sys_calloc(1, sizeof(pumpkin_task_t))) == NULL) {
    pumpkin_module.tasks[i].reserved = 0;
    mutex_unlock(mutex);
    return -1;
  }

  if ((screen = sys_calloc(1, sizeof(task_screen_t))) == NULL) {
    pumpkin_module.tasks[i].reserved = 0;
    sys_free(task);
    mutex_unlock(mutex);
    return -1;
  }

  if ((screen->surface = surface_create(width, height, pumpkin_module.encoding)) == NULL) {
    pumpkin_module.tasks[i].reserved = 0;
    sys_free(task);
    sys_free(screen);
    mutex_unlock(mutex);
    return -1;
  }

  if (pumpkin_module.mono) {
    screen->msurface = surface_create(width, height, pumpkin_module.encoding);
  }

  screen->x0 = width;
  screen->y0 = height;
  screen->x1 = -1;
  screen->y1 = -1;

  screen->tag = TAG_SCREEN;
  if ((ptr = ptr_new(screen, task_destructor)) == -1) {
    pumpkin_module.tasks[i].reserved = 0;
    surface_destroy(screen->surface);
    if (screen->msurface) surface_destroy(screen->msurface);
    sys_free(task);
    sys_free(screen);
    mutex_unlock(mutex);
    return -1;
  }

  color = screen->surface->color_rgb(screen->surface->data, 255, 255, 255, 255);
  surface_rectangle(screen->surface, 0, 0, width-1, height-1, 1, color);

  pumpkin_module.tasks[i].taskId = taskId;
  pumpkin_module.tasks[i].creator = creator;
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
  pumpkin_module.tasks[i].lockable = 0;

  pumpkin_module.task_order[pumpkin_module.num_tasks] = i;
  pumpkin_module.current_task = i;
  pumpkin_module.num_tasks++;

  task->taskId = pumpkin_module.tasks[i].taskId;
  task->creator = pumpkin_module.tasks[i].creator;
  task->texture = pumpkin_module.tasks[i].texture;
  task->task_index = i;
  task->active = 1;
  task->screen_ptr = ptr;
  task->width = width;
  task->height = height;
  task->new_width = width;
  task->new_height = height;
  sys_strncpy(task->name, name, dmDBNameLength-1);

  thread_set(task_key, task);
  if (pumpkin_module.mode != 1) {
    task->heap = heap_init(NULL, HEAP_SIZE, SMALL_HEAP_SIZE, NULL);
    StoInit(APP_STORAGE, pumpkin_module.fs_mutex);
    //DataMgrInitModule(pumpkin_module.dm);
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

  if ((regOS = pumpkin_reg_get(creator, regOsID, &regSize)) != NULL) {
    task->osversion = regOS->version;
    debug(DEBUG_INFO, PUMPKINOS, "overriding OS version=%d for \"%s\"", regOS->version, name);
    MemPtrFree(regOS);
  } else {
    task->osversion = pumpkin_module.osversion;
  }

  if ((regDisplay = pumpkin_reg_get(creator, regDisplayID, &regSize)) != NULL) {
    task->depth = regDisplay->depth <= pumpkin_module.depth ? regDisplay->depth : pumpkin_module.depth;
    task->density = regDisplay->density <= pumpkin_module.density ? regDisplay->density : pumpkin_module.density;
    MemPtrFree(regDisplay);
  } else {
    task->depth = pumpkin_module.depth;
    task->density = pumpkin_module.density;
  }

  width = task->width;
  height = task->height;

  if (task->density != pumpkin_module.density) {
    debug(DEBUG_INFO, PUMPKINOS, "overriding display density=%d for \"%s\"", task->density, name);
    if (task->density == kDensityLow) {
      width >>= 1;
      height >>= 1;
    }
  }
  if (task->depth != pumpkin_module.depth) {
    debug(DEBUG_INFO, PUMPKINOS, "overriding display depth=%d for \"%s\"", task->depth, name);
  }

  pumpkin_module.tasks[i].density = task->density;
  pumpkin_module.tasks[i].depth = task->depth;

  UicInitModule();
  WinInitModule(task->density, width, height, task->depth, NULL);
  FntInitModule(task->density);
  FrmInitModule();
  InsPtInitModule();
  FldInitModule();
  MenuInitModule();
  EvtInitModule();
  SysInitModule();
  GPSInitModule(pumpkin_module.gps_parse_line, pumpkin_module.bt);
  VFSInitModule();
  VFSAddVolume(VFS_CARD);
  KeyboardInitModule();
  ClpInitModule();
  SrmInitModule();
  FtrInitModule();
  KeyInitModule();
  SndInitModule(pumpkin_module.ap);
  SelTimeInitModule();
  CharAttrInitModule();
  SysFatalAlertInit();

  if (i == 0) {
    pumpkin_module.mount = VFSGetMount(1);
    debug(DEBUG_INFO, PUMPKINOS, "VFS mount \"%s\"", pumpkin_module.mount);

    pumpkin_get_preference(BOOT_CREATOR, PUMPKINOS_PREFS_ID, &prefs, sizeof(PumpkinPreferencesType), true);
    if (prefs.value[pBackgroundImage] & 0xFFFF) {
      pumpkin_image_background(&prefs.color[pColorBackground], prefs.value[pBackgroundImage] & 0xFFFF, prefs.value[pBackgroundImage] >> 16);
    } else if (pumpkin_module.background) {
      surface_destroy(pumpkin_module.background);
      pumpkin_module.background = NULL;
    }
    pumpkin_module.refresh = 1;

    pumpkin_init_midi();
    pumpkin_init_icon();
  }

  pumpkin_module.render = 1;
  mutex_unlock(mutex);

  if (pumpkin_module.mode == 1 && pumpkin_module.wp->average) {
    size = 0;
    if (PrefGetAppPreferences('toch', 1, &pumpkin_module.calibration, &size, true) == noPreferenceFound) {
      pumpkin_calibrate(0);
    }
  }

  return 0;
}

void pumpkin_local_refresh(void) {
  StoRefresh();
}

static int pumpkin_local_finish(UInt32 creator) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  RegPositionType regPos;
  int i, x, y;

  if (mutex_lock(mutex) == -1) {
    return -1;
  }

  if (task->lang) LanguageFinish(task->lang);

  if (pumpkin_module.wm) {
    if (creator) {
      if (wman_xy(pumpkin_module.wm, pumpkin_module.tasks[task->task_index].taskId, &x, &y) == 0) {
        regPos.x = x;
        regPos.y = y;
        pumpkin_reg_set(creator, regPositionID, &regPos, sizeof(RegPositionType));
      }
    }
  }

  SysFatalAlertFinish();

  pumpkin_module.tasks[task->task_index].creator = 0;
  pumpkin_module.tasks[task->task_index].task_index = 0;
  pumpkin_module.tasks[task->task_index].active = 0;
  pumpkin_module.tasks[task->task_index].removed = 1;
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
  if (pumpkin_module.mode != 1) {
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
  CharAttrFinishModule();

  if (pumpkin_module.tasks[task->task_index].bootRef) {
    DmCloseDatabase(pumpkin_module.tasks[task->task_index].bootRef);
    pumpkin_module.tasks[task->task_index].bootRef = NULL;
  }

  if (task->table) {
    sys_free(task->table);
  }

  for (i = 0; i < pumpkin_module.num_tasks; i++) {
    if (task->serial[i]) pumpkin_heap_free(task->serial[i], "serial_descr");
  }

  for (i = 0; i < pumpkin_module.num_notif; i++) {
    if (pumpkin_module.notif[i].appCreator == creator) {
      pumpkin_module.notif[i].taskId = -1;
    }
  }

  if (pumpkin_module.mode != 1) {
    StoFinish();
    //DataMgrFinishModule();
    heap_finish(task->heap);
  }

  if (pumpkin_module.wp->show_cursor) pumpkin_module.wp->show_cursor(pumpkin_module.w, 1);
  wman_choose_border(pumpkin_module.wm, 0);
  pumpkin_module.locked = 0;

  thread_set(task_key, NULL);
  sys_free(task);

  if (pumpkin_module.num_tasks == 0) {
    debug(DEBUG_INFO, PUMPKINOS, "last task finishing");
    sys_set_finish(0);
  }

  mutex_unlock(mutex);

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

void pumpkin_set_subdata(void *subdata) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  task->subdata = subdata;
}

void *pumpkin_get_subdata(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task->subdata;
}

int pumpkin_shader(char *vertex_shader, int vlen, char *fragment_shader, int flen, float (*getvar)(char *name, void *data), void *data) {
  int r = -1;

  if (pumpkin_module.wp->shader) {
    if (pumpkin_module.mode == 0) {
      if (pumpkin_get_boolean_option("opengl")) {
        if (mutex_lock(mutex) == 0) {
          if (pumpkin_module.vertex_shader) sys_free(pumpkin_module.vertex_shader);
          if (pumpkin_module.fragment_shader) sys_free(pumpkin_module.fragment_shader);
          pumpkin_module.vertex_shader = vertex_shader ? sys_strdup(vertex_shader) : NULL;
          pumpkin_module.fragment_shader = fragment_shader ? sys_strdup(fragment_shader) : NULL;
          pumpkin_module.vlen = vlen;
          pumpkin_module.flen = flen;
          pumpkin_module.shader_getvar = getvar;
          pumpkin_module.shader_data = data;
          r = 0;
          mutex_unlock(mutex);
        }
      }
    } else {
      r = pumpkin_module.wp->shader(pumpkin_module.w, 0, vertex_shader, vlen, fragment_shader, flen, getvar, data);
    }
  }

  return r;
}

// called directly by libos if DIA or single app mode
int pumpkin_launcher(char *name, int width, int height) {
  pumpkin_task_t *task;
  LocalID dbID;
  UInt32 creator;
  texture_t *texture;
  launch_request_t request;

  texture = pumpkin_module.wp->create_texture(pumpkin_module.w, width, height);

  if (pumpkin_local_init(0, 0, texture, 0, name, width, height, 0, 0) == 0) {
    dbID = DmFindDatabase(0, name);
    DmDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creator);

    task = (pumpkin_task_t *)thread_get(task_key);
    if (ErrSetJump(task->jmpbuf) != 0) {
      debug(DEBUG_ERROR, PUMPKINOS, "ErrSetJump not zero");
    } else {
      wman_add(pumpkin_module.wm, 0, texture, 0, 0, width, height);
      MemSet(&request, sizeof(launch_request_t), 0);
#if defined(ANDROID) || defined(KERNEL)
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

void pumpkin_app_crashed(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  if (task && task->dbID) {
    sendLaunchNotification(sysNotifyAppCrashedEvent, task->dbID);
  }
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

  if (pumpkin_local_init(data->index, data->taskId, data->texture, data->creator, data->request.name, data->width, data->height, data->x, data->y) == 0) {
    task = (pumpkin_task_t *)thread_get(task_key);
    if (ErrSetJump(task->jmpbuf) != 0) {
      debug(DEBUG_ERROR, PUMPKINOS, "ErrSetJump not zero");
      pumpkin_app_crashed();
    } else {
      pumpkin_launch_sub(&data->request, data->request.opendb);
      cont = SysUIAppSwitchCont(&launch);
    }
    pumpkin_local_finish(data->creator);
  }

  thread_set_name(TAG_APP);
  debug(DEBUG_INFO, PUMPKINOS, "thread exiting");
  StrNCopy(name, data->request.name, dmDBNameLength - 1);
  sys_free(data);

  if (cont) {
    debug(DEBUG_INFO, PUMPKINOS, "switching from \"%s\" to \"%s\" (SysUIAppSwitch)", name, launch.name);
    // In PumpkinOS, the parameter of sysAppLaunchCmdPanelCalledFromApp is set to the calling app's name ...
    if (launch.code == sysAppLaunchCmdPanelCalledFromApp) {
      launch.hasParam = true;
      StrNCopy(launch.param.p4.name, name, dmDBNameLength - 1);
    }
    pumpkin_launch_request(0, launch.name, launch.code, launch.hasParam ? &launch.param : NULL, launch.flags, NULL, 1);
  } else if (data->request.code == sysAppLaunchCmdPanelCalledFromApp) {
    // ... so that the panel knows which app to call on sysAppLaunchCmdReturnFromPanel
    StrNCopy(launch.name, data->request.param.p4.name, dmDBNameLength - 1);
    debug(DEBUG_INFO, PUMPKINOS, "switching from \"%s\" to \"%s\" (sysAppLaunchCmdReturnFromPanel)", name, launch.name);
    pumpkin_launch_request(0, launch.name, sysAppLaunchCmdReturnFromPanel, NULL, 0, NULL, 1);
  }

  return 0;
}

static int pumpkin_wait_ack(int port, uint32_t *reply) {
  uint8_t *buf;
  unsigned int len;
  uint32_t *p;
  int ack, client, r = -1;

  for (ack = 0; !ack && !thread_must_end();) {
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
          debug(DEBUG_ERROR, PUMPKINOS, "received %d bytes from %d but was expecting %u bytes", len, port, (uint32_t)sizeof(uint32_t));
        }
      } else {
        if (len == sizeof(uint32_t)) {
          p = (uint32_t *)buf;
          debug(DEBUG_ERROR, PUMPKINOS, "received reply %u from %d but was expecting %d", *p, client, port);
        } else {
          debug(DEBUG_ERROR, PUMPKINOS, "received reply from %d but was expecting %d", client, port);
        }
      }
      sys_free(buf);
    } else {
      debug(DEBUG_ERROR, PUMPKINOS, "received nothing from %d but was expecting %u bytes", port, (uint32_t)sizeof(uint32_t));
    }
  }

  return r;
}

void pumpkin_send_deploy(void) {
  client_request_t creq;
  SysNotifyParamType notify;

  if (pumpkin_module.mode == 0) {
    debug(DEBUG_TRACE, PUMPKINOS, "send MSG_DEPLOY");
    sys_memset(&creq, 0, sizeof(client_request_t));
    creq.type = MSG_DEPLOY;
    thread_client_write(pumpkin_get_spawner(), (uint8_t *)&creq, sizeof(client_request_t));
  } else {
    MemSet(&notify, sizeof(notify), 0);
    notify.notifyType = sysNotifySyncFinishEvent;
    notify.broadcaster = sysNotifyBroadcasterCode;
    SysNotifyBroadcast(&notify);
  }
}

int pumpkin_launch(launch_request_t *request) {
  LocalID dbID;
  RegPositionType *regPos;
  RegDimensionType *regDim;
  RegDisplayEndianType *regEnd;
  UInt32 type, creator, regSize;
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
      } else if (!pumpkin_module.tasks[i].removed) {
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
      if (pumpkin_normal_launch(request->code)) {
        // if application is already running and launch code is sysAppLaunchCmdNormalLaunch, just make the application current
        pumpkin_make_current(running);
      } else if (request->code == sysAppLaunchCmdGoTo) {
        // if application is already running and launch code is sysAppLaunchCmdGoTo, send request to it
        pumpkin_make_current(running);
        sys_memset(&creq, 0, sizeof(client_request_t));
        creq.type = MSG_LAUNCH;
        sys_memcpy(&creq.data.launch, request, sizeof(launch_request_t));
        creq.data.launch.flags &= ~sysAppLaunchFlagNewGlobals;
        if (thread_client_write(pumpkin_module.tasks[running].handle, (uint8_t *)&creq, sizeof(client_request_t)) == sizeof(client_request_t)) {
          wait_ack = pumpkin_module.tasks[running].handle;
        }
      }
      mutex_unlock(mutex);
      if (wait_ack != -1) {
        debug(DEBUG_INFO, PUMPKINOS, "waiting ack from \"%s\"", request->name);
        pumpkin_wait_ack(wait_ack, NULL);
      }
      return 0;
    }

    if (index == -1) {
      debug(DEBUG_ERROR, PUMPKINOS, "no more threads");
      mutex_unlock(mutex);
      return -1;
    }

    if ((data = sys_calloc(1, sizeof (launch_data_t))) != NULL) {
      dbID = DmFindDatabase(0, request->name);
      DmDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, &creator);
      data->creator = creator;
      data->width = APP_SCREEN_WIDTH;
      data->height = APP_SCREEN_HEIGHT;
      if (pumpkin_module.density == kDensityLow) {
        data->width /= 2;
        data->height /= 2;
      }

      if (pumpkin_module.mode == 0) {
        data->x = (pumpkin_module.width - data->width) / 2;
        data->y = (pumpkin_module.height - data->height) / 2;

        if ((regPos = pumpkin_reg_get(creator, regPositionID, &regSize)) == NULL) {
          debug(DEBUG_INFO, PUMPKINOS, "recreating registry");
          pumpkin_registry_create(creator);
        }

        if ((regPos = pumpkin_reg_get(creator, regPositionID, &regSize)) != NULL) {
          data->x = regPos->x;
          data->y = regPos->y;
          debug(DEBUG_INFO, PUMPKINOS, "using position %d,%d from registry", data->x, data->y);
          MemPtrFree(regPos);
        }

        if ((regDim = pumpkin_reg_get(creator, regDimensionID, &regSize)) != NULL) {
          data->width = regDim->width;
          data->height = regDim->height;
          debug(DEBUG_INFO, PUMPKINOS, "using size %dx%d from registry", data->width, data->height);
          MemPtrFree(regDim);
        }
      }

      if (data->width == 0 || data->height == 0) {
        data->width = pumpkin_module.width;
        data->height = pumpkin_module.height;
      }

      if ((regEnd = pumpkin_reg_get(creator, regEndianID, &regSize)) != NULL) {
        debug(DEBUG_INFO, PUMPKINOS, "using display endianness %d from registry", regEnd->littleEndian);
        MemPtrFree(regEnd);
      }

      data->index = index;
      data->taskId = pumpkin_module.nextTaskId++;
      sys_memcpy(&data->request, request, sizeof(launch_request_t));
      data->texture = pumpkin_module.wp->create_texture(pumpkin_module.w, data->width, data->height);
      if (type == sysFileTApplication || type == sysFileTPanel) {
        wman_add(pumpkin_module.wm, data->taskId, data->texture, data->x, data->y, data->width, data->height);
      }
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
    r = pumpkin_module.mode == 1 ? pumpkin_module.launched : 1;
    mutex_unlock(mutex);
  }

  return r;
}

static int pumpkin_pause_task(char *name, int *call_sub) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  int i, handle = -1;
  uint32_t msg;

  if ((name && task && !sys_strcmp(name, task->name))) {
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

/*
static void pumpkin_forward_launch_code(UInt32 creator, char *name, UInt16 cmd, launch_union_t *param, UInt16 flags) {
  launch_request_t request;
  launch_msg_t msg;
  UInt32 size, i;
  uint8_t *buf;

  for (i = 0; i < pumpkin_module.num_tasks; i++) {
    if (pumpkin_module.tasks[i].active && pumpkin_module.tasks[i].creator == creator) break;
  }

  if (i == pumpkin_module.num_tasks) {
    debug(DEBUG_INFO, PUMPKINOS, "sending launch code %d to \"%s\" using call", cmd, name);
    MemSet(&request, sizeof(launch_request_t), 0);
    StrNCopy(request.name, name, dmDBNameLength-1);
    request.code = cmd;
    request.flags = flags;
    sys_memcpy(&request.param, param, sizeof(launch_union_t));
    pumpkin_launch_sub(&request, 1);
    return;
  }

  msg.msg = MSG_LAUNCHC;
  msg.cmd = cmd;
  msg.flags = flags;

  if (param) {
    size = sizeof(launch_msg_t);
    if ((buf = serialize_launch(cmd, param, &size)) != NULL) {
      sys_memcpy(buf, &msg, sizeof(launch_msg_t));
      debug(DEBUG_INFO, PUMPKINOS, "sending launch code %d to \"%s\" using message (%u bytes)", cmd, name, size);
      debug_bytes(DEBUG_TRACE, PUMPKINOS, buf, sizeof(launch_msg_t));
      debug_bytes(DEBUG_TRACE, PUMPKINOS, buf + sizeof(launch_msg_t), size - sizeof(launch_msg_t));
      thread_client_write(pumpkin_module.tasks[i].handle, buf, size);
      sys_free(buf);
    }
  } else {
    size = sizeof(launch_msg_t);
    debug(DEBUG_INFO, PUMPKINOS, "sending launch code %d (NO param) to \"%s\" using message (%u bytes)", cmd, name, size);
    buf = (uint8_t *)&msg;
    debug_bytes(DEBUG_TRACE, PUMPKINOS, buf, sizeof(launch_msg_t));
    thread_client_write(pumpkin_module.tasks[i].handle, buf, size);
  }
}
*/

static uint32_t pumpkin_launch_request(LocalID dbID, char *name, UInt16 cmd, launch_union_t *param, UInt16 flags, PilotMainF pilotMain, UInt16 opendb) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  client_request_t creq;
  ErrJumpBuf jmpbuf;
  BitmapType *bmp;
  WinHandle wh, draw;
  void *old;
  void *win_module;
  void *fnt_module;
  void *frm_module;
  void *fld_module;
  void *menu_module;
  int handle, call_sub, m68k, launched;
  uint32_t r = 0;
  void *data;

  sys_memset(&creq, 0, sizeof(client_request_t));
  creq.type = MSG_LAUNCH;
  sys_strncpy(creq.data.launch.name, name, dmDBNameLength-1);
  creq.data.launch.code = cmd;
  creq.data.launch.flags = flags;
  if (param) {
    creq.data.launch.hasParam = true;
    sys_memcpy(&creq.data.launch.param, param, sizeof(launch_union_t));
  }
  creq.data.launch.pilot_main = pilotMain;
  creq.data.launch.opendb = opendb;

  switch (cmd) {
    case sysAppLaunchCmdNormalLaunch:
    case sysAppLaunchCmdReturnFromPanel:
    case sysAppLaunchCmdPanelCalledFromApp:
      break;
    case sysAppLaunchCmdGoTo:
      // XXX set sysAppLaunchFlagNewGlobals so that the app initializes properly
      creq.data.launch.flags |= sysAppLaunchFlagNewGlobals;
      break;
/*
    case sysAppLaunchCmdSystemReset:
      debug(DEBUG_INFO, PUMPKINOS, "sending launch code %d to \"%s\" using call or message", cmd, name);
      if (DmDatabaseInfo(0, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creator) == errNone) {
        if (mutex_lock(mutex) == 0) {
          pumpkin_forward_launch_code(creator, name, cmd, param, flags);
          mutex_unlock(mutex);
        }
      }
      return r;
*/
    default:
      if (cmd >= sysAppLaunchCmdCustomBase || task == NULL) {
        break;
      }
      debug(DEBUG_INFO, PUMPKINOS, "sending launch code %d to \"%s\" using pause/resume", cmd, name);
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

  if (pumpkin_module.mode != 0) {
    debug(DEBUG_INFO, PUMPKINOS, "spawning \"%s\" with launchCode %d", name, cmd);
    old = UIColorSaveTable();
    win_module = WinReinitModule(NULL);
    fnt_module = FntReinitModule(NULL);
    frm_module = FrmReinitModule(NULL);
    fld_module = FldReinitModule(NULL);
    menu_module = MenuReinitModule(NULL);
    InsPtEnable(false);
    m68k = pumpkin_is_m68k();
    launched = pumpkin_is_launched();
    pumpkin_launched(1);
    data = pumpkin_get_data();
    sys_memcpy(jmpbuf, task->jmpbuf, sizeof(ErrJumpBuf));
    if (ErrSetJump(task->jmpbuf) != 0) {
      debug(DEBUG_ERROR, PUMPKINOS, "ErrSetJump not zero");
    } else {
      pumpkin_set_data(NULL);
      r = pumpkin_launch_sub(&creq.data.launch, 1);
    }
    sys_memcpy(task->jmpbuf, jmpbuf, sizeof(ErrJumpBuf));
    pumpkin_set_data(data);
    pumpkin_set_m68k(m68k);
    pumpkin_launched(launched);
    InsPtEnable(false);
    UIColorRestoreTable(old);
    WinReinitModule(win_module);
    FntReinitModule(fnt_module);
    MenuReinitModule(menu_module);
    FldReinitModule(fld_module);
    FrmReinitModule(frm_module);
    FrmSetDIAPolicyAttr(FrmGetActiveForm(), frmDIAPolicyCustom);
    PINSetInputTriggerState(pinInputTriggerEnabled);
    // restore the previous screen
    wh = WinGetDisplayWindow();
    bmp = WinGetBitmap(wh);
    draw = WinSetDrawWindow(wh);
    WinDrawBitmap(bmp, 0, 0);
    WinSetDrawWindow(draw);
  } else {
    debug(DEBUG_INFO, PUMPKINOS, "sending launch code %d to \"%s\" via spawner on port %d", cmd, name, pumpkin_module.spawner);
    thread_client_write(pumpkin_module.spawner, (uint8_t *)&creq, sizeof(client_request_t));
  }

  return r;
}

uint32_t pumpkin_fork(void) {
  UInt32 result;

  return SysAppLaunch(0, pumpkin_get_app_localid(), sysAppLaunchFlagFork, sysAppLaunchCmdNormalLaunch, NULL, &result);
}

// Launch a specified application as a subroutine of the caller.
Err SysAppLaunch(UInt16 cardNo, LocalID dbID, UInt16 launchFlags, UInt16 cmd, MemPtr cmdPBP, UInt32 *resultP) {
  return SysAppLaunchEx(cardNo, dbID, launchFlags, cmd, cmdPBP, resultP, NULL);
}

Err SysAppLaunchEx(UInt16 cardNo, LocalID dbID, UInt16 launchFlags, UInt16 cmd, MemPtr cmdPBP, UInt32 *resultP, PilotMainF pilotMain) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  launch_request_t request;
  char name[dmDBNameLength];
  int r = -1;

  debug(DEBUG_INFO, PUMPKINOS, "SysAppLaunch dbID 0x%08X flags 0x%04X cmd %d param %p", dbID, launchFlags, cmd, cmdPBP);

  if (DmDatabaseInfo(0, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == errNone) {
    if (!task || (task && dbID != task->dbID) || (launchFlags & sysAppLaunchFlagFork)) {
      *resultP = pumpkin_launch_request(dbID, name, cmd, cmdPBP, launchFlags, pilotMain, 1);
      r = 0;
    } else {
      debug(DEBUG_INFO, PUMPKINOS, "calling self with launchCode %d", cmd);
      sys_memset(&request, 0, sizeof(launch_request_t));
      sys_strncpy(request.name, name, dmDBNameLength-1);
      request.code = cmd;
      request.flags = sysAppLaunchFlagSubCall;
      if (cmdPBP) {
        request.hasParam = true;
        sys_memcpy(&request.param, cmdPBP, sizeof(launch_union_t));
      }
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
  pumpkin_task_t *task;
  uint8_t *raw;
  uint16_t prev;
  int width, height, len, updated = 0;
  EventType event;

  if ((screen = ptr_lock(pumpkin_module.tasks[i].screen_ptr, TAG_SCREEN))) {
    if (pumpkin_module.dia) {
      if (dia_get_main_dimension(pumpkin_module.dia, &width, &height) == 0) {
        if (width != pumpkin_module.tasks[i].width || height != pumpkin_module.tasks[i].height) {
          debug(DEBUG_INFO, PUMPKINOS, "task %d (%s) display changed from %dx%d to %dx%d", i, pumpkin_module.tasks[i].name,
              pumpkin_module.tasks[i].width, pumpkin_module.tasks[i].height, width, height);
          if (pumpkin_changed_display(&pumpkin_module.tasks[i], screen, width, height) == 0) {
            if (pumpkin_module.mode == 2) {
              pumpkin_forward_msg(i, MSG_DISPLAY, width, height, 0);
            } else {
              task = (pumpkin_task_t *)thread_get(task_key);
              task->width = width;
              task->height = height;
              prev = WinSetCoordinateSystem(kCoordinatesDouble);
              WinSetDisplayExtent(width, height);
              WinSetCoordinateSystem(prev);
              MemSet(&event, sizeof(EventType), 0);
              event.eType = winDisplayChangedEvent;
              RctSetRectangle(&event.data.winDisplayChanged.newBounds, 0, 0, width, height);
              EvtAddEventToQueue(&event);
            }
          }
        }
      }
    } else if (pumpkin_module.tasks[i].width != pumpkin_module.tasks[i].new_width ||
               pumpkin_module.tasks[i].height != pumpkin_module.tasks[i].new_height) {
      width = pumpkin_module.tasks[i].new_width;
      height = pumpkin_module.tasks[i].new_height;
      debug(DEBUG_INFO, PUMPKINOS, "task %d (%s) display changed from %dx%d to %dx%d", i, pumpkin_module.tasks[i].name,
          pumpkin_module.tasks[i].width, pumpkin_module.tasks[i].height, width, height);
      if (pumpkin_changed_display(&pumpkin_module.tasks[i], screen, width, height) == 0) {
        pumpkin_forward_msg(i, MSG_DISPLAY, width, height, 0);
      }
    }

    if (screen->dirty) {
      if (pumpkin_module.mono) {
        surface_dither(screen->msurface, 0, 0, screen->surface, 0, 0, screen->surface->width, screen->surface->height, pumpkin_module.mono);
        raw = (uint8_t *)screen->surface->getbuffer(screen->msurface->data, &len);
      } else {
        raw = (uint8_t *)screen->surface->getbuffer(screen->surface->data, &len);
      }
      *x = screen->x0;
      *y = screen->y0;
      *w = screen->x1 - screen->x0 + 1;
      *h = screen->y1 - screen->y0 + 1;
      debug(DEBUG_TRACE, PUMPKINOS, "task %d (%s) update texture %d,%d %d,%d", i, pumpkin_module.tasks[i].name, *x, *y, *w, *h);
      pumpkin_module.wp->update_texture_rect(pumpkin_module.w, pumpkin_module.tasks[i].texture, raw, *x, *y, *w, *h);
      screen->x0 = pumpkin_module.tasks[i].width;
      screen->y0 = pumpkin_module.tasks[i].height;
      screen->x1 = -1;
      screen->y1 = -1;
      screen->dirty = 0;
      updated = 1;
    }
    ptr_unlock(pumpkin_module.tasks[i].screen_ptr, TAG_SCREEN);
  }

  return updated;
}

static void put_event(int ev, int arg1, int arg2, int arg3) {
  if (pumpkin_module.nev < MAX_EVENTS) {
    pumpkin_module.events[pumpkin_module.iev].ev = ev;
    pumpkin_module.events[pumpkin_module.iev].arg1 = arg1;
    pumpkin_module.events[pumpkin_module.iev].arg2 = arg2;
    pumpkin_module.events[pumpkin_module.iev].arg3 = arg3;
    pumpkin_module.iev++;
    if (pumpkin_module.iev == MAX_EVENTS) pumpkin_module.iev = 0;
    pumpkin_module.nev++;
  }
}

static int get_event(int *arg1, int *arg2, int *arg3) {
  int ev = 0;

  if (pumpkin_module.nev) {
    ev = pumpkin_module.events[pumpkin_module.oev].ev;
    *arg1 = pumpkin_module.events[pumpkin_module.oev].arg1;
    *arg2 = pumpkin_module.events[pumpkin_module.oev].arg2;
    *arg3 = pumpkin_module.events[pumpkin_module.oev].arg3;
    pumpkin_module.oev++;
    if (pumpkin_module.oev == MAX_EVENTS) pumpkin_module.oev = 0;
    pumpkin_module.nev--;
  }

  return ev;
}

void pumpkin_forward_msg(int i, int ev, int a1, int a2, int a3) {
  unsigned char *buf;
  unsigned int len;
  uint32_t carg[4];

  if (pumpkin_module.mode == 1) {
    put_event(ev, a1, a2, a3);
  } else {
    carg[0] = ev;
    carg[1] = a1;
    carg[2] = a2;
    carg[3] = a3;
    buf = (unsigned char *)&carg[0];
    len = sizeof(uint32_t)*4;

    // XXX this can block if the pipe is full, which can happen if the task is not reading quite often
    thread_client_write(pumpkin_module.tasks[i].handle, buf, len);
  }
}

void pumpkin_forward_event(int i, EventType *event) {
  uint32_t buf[64];

  if (mutex_lock(mutex) == 0) {
    buf[0] = MSG_USER;
    sys_memcpy(&buf[1], event, sizeof(EventType));
    thread_client_write(pumpkin_module.tasks[i].handle, (uint8_t *)buf, 4 + sizeof(EventType));
    mutex_unlock(mutex);
  }
}

static void pumpkin_forward_notif(Int32 taskId, UInt32 creator, SysNotifyParamType *notify, SysNotifyProcPtr callback, UInt32 callback68k) {
  notif_msg_t msg;
  DmSearchStateType stateInfo;
  LocalID dbID;
  char snotify[8], screator[8];
  uint8_t *buf;
  UInt32 size, result, i;

  if (taskId == -1) {
    for (i = 0; i < pumpkin_module.num_tasks; i++) {
      if (pumpkin_module.tasks[i].active && pumpkin_module.tasks[i].creator == creator) break;
    }
    if (i < pumpkin_module.num_tasks) {
      taskId = pumpkin_module.tasks[i].taskId;
    } else {
      pumpkin_id2s(notify->notifyType, snotify);
      pumpkin_id2s(creator, screator);
      debug(DEBUG_INFO, PUMPKINOS, "application '%s' not available for receiving notification '%s'", screator, snotify);
      if (DmGetNextDatabaseByTypeCreator(true, &stateInfo, sysFileTApplication, creator, false, NULL, &dbID) == errNone) {
        SysAppLaunch(0, dbID, sysAppLaunchFlagSubCall, sysAppLaunchCmdNotify, notify, &result);
      }
    }
    return;
  }

  if (taskId != -1) {
    for (i = 0; i < pumpkin_module.num_tasks; i++) {
      if (pumpkin_module.tasks[i].taskId == taskId) break;
    }
    if (i < pumpkin_module.num_tasks) {
      MemSet(&msg, sizeof(notif_msg_t), 0);
      msg.msg = MSG_NOTIFY;
      msg.callback = callback;
      msg.callback68k = callback68k;
      sys_memcpy(&msg.notify, notify, sizeof(SysNotifyParamType));
      size = sizeof(notif_msg_t);
      if ((buf = serialize_notif(notify->notifyType, msg.notify.notifyDetailsP, &size)) != NULL) {
        msg.notify.notifyDetailsP = NULL;
        sys_memcpy(buf, &msg, sizeof(notif_msg_t));
        pumpkin_id2s(notify->notifyType, snotify);
        debug(DEBUG_INFO, PUMPKINOS, "sending notification '%s' (%u bytes)", snotify, size);
        debug_bytes(DEBUG_TRACE, PUMPKINOS, buf, sizeof(notif_msg_t));
        if (size > sizeof(notif_msg_t)) {
          debug_bytes(DEBUG_TRACE, PUMPKINOS, buf + sizeof(notif_msg_t), size - sizeof(notif_msg_t));
        }
        thread_client_write(pumpkin_module.tasks[i].handle, buf, size);
        sys_free(buf);
      }
    } else {
      debug(DEBUG_ERROR, PUMPKINOS, "send notification to invalid taskId %d", taskId);
    }
  }
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
  return task ? task->task_index : 0;
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
      pumpkin_forward_msg(i, MSG_RAISE, 0, 0, 0);
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
    case WINDOW_KEY_END:   pumpkin_module.keyMask |= keyBitPower;      break;
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
    case WINDOW_KEY_END:   pumpkin_module.keyMask &= ~keyBitPower;    break;
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

void pumpkin_set_native_keys(int active) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  task->nativeKeys = active;
}

int pumpkin_get_native_keys(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  int locked = 0;

  if (mutex_lock(mutex) == 0) {
    locked = pumpkin_module.locked;
    mutex_unlock(mutex);
  }

  return task->nativeKeys && locked;
}

int pumpkin_set_lockable(int lockable) {
  int r = -1;

  if (mutex_lock(mutex) == 0) {
    if (pumpkin_module.current_task != -1) {
      pumpkin_module.tasks[pumpkin_module.current_task].lockable = lockable;
      r = 0;
    }
    mutex_unlock(mutex);
  }

  return r;
}

static void save_screen_callback(void *context, void *screen, int size) {
  save_screen_t *scr = (save_screen_t *)context;
  LocalID dbID;
  DmOpenRef dbRef;
  char buf[8];
        
  pumpkin_id2s(scr->creator, buf);
          
  debug(DEBUG_INFO, PUMPKINOS, "save screen app '%s', dimension %dx%d, size %d", buf, scr->surface->width, scr->surface->height, size);
        
  if ((dbID = DmFindDatabase(0, SCREEN_DB)) == 0) {
    DmCreateDatabase(0, SCREEN_DB, 'Scrn', 'Data', true);
  }     
        
  if ((dbID = DmFindDatabase(0, SCREEN_DB)) != 0) {
    if ((dbRef = DmOpenDatabase(0, dbID, dmModeWrite)) != NULL) {
      DmNewResourceEx(dbRef, scr->creator, 1, size, screen);
      DmCloseDatabase(dbRef);
    }       
  }         
}           
            
static void save_screen(void) {   
  task_screen_t *screen;
  save_screen_t scr;

  if ((screen = ptr_lock(pumpkin_module.tasks[pumpkin_module.current_task].screen_ptr, TAG_SCREEN))) {
    scr.surface = screen->surface;
    scr.creator = pumpkin_module.tasks[pumpkin_module.current_task].creator;
    surface_save_mem(screen->surface, 0, &scr, save_screen_callback);
    ptr_unlock(pumpkin_module.tasks[pumpkin_module.current_task].screen_ptr, TAG_SCREEN);
  }
}

int pumpkin_sys_event(void) {
  uint64_t now;
  int arg1, arg2, w, h;
  int i, j, x, y, tx, ty, ev, tmp, len, mult;
  int paused, wait, r = -1;
  void *bits;
  Int32 taskId;
  UInt32 widgetId;

  if (pumpkin_module.vertex_shader != NULL && pumpkin_module.shader_inited == 0) {
    pumpkin_module.shader_inited = pumpkin_module.wp->shader(pumpkin_module.w, 0, pumpkin_module.vertex_shader, pumpkin_module.vlen,
      pumpkin_module.fragment_shader, pumpkin_module.flen, pumpkin_module.shader_getvar, pumpkin_module.shader_data) == 0 ? 1 : -1;
  } else if (pumpkin_module.vertex_shader == NULL && pumpkin_module.shader_inited == 1) {
    pumpkin_module.wp->shader(pumpkin_module.w, 0, NULL, 0, NULL, 0, NULL, NULL);
    pumpkin_module.shader_inited = 0;
  }

  for (;;) {
    if (thread_must_end()) {
      if (pumpkin_module.fullrefresh) {
        wman_clear(pumpkin_module.wm);
        pumpkin_module.wp->render(pumpkin_module.w);
      }
      return -1;
    }
    paused = pumpkin_is_paused();
    wait = paused ? 100 : 1;
    ev = pumpkin_module.wp->event2(pumpkin_module.w, wait, &arg1, &arg2);
    if (ev == -1) return -1;
    if (!paused) break;
  }

  if (ev == WINDOW_EXPOSE) {
    pumpkin_module.refresh = 1;
    ev = 0;
  }

  if (mutex_lock(mutex) == 0) {
    now = sys_get_clock();

    if ((now - pumpkin_module.lastUpdate) > 50000) {
      if (pumpkin_module.num_tasks > 0) {
        if (pumpkin_module.refresh) {
          if (pumpkin_module.background) {
            bits = surface_buffer(pumpkin_module.background, &len);
            wman_set_image_background(pumpkin_module.wm, pumpkin_module.hdepth, bits);
          } else {
            pumpkin_set_host_depth(0);
          }
        }

        if (pumpkin_module.fullrefresh || pumpkin_module.refresh) {
          wman_clear(pumpkin_module.wm);
        }

        for (j = 0; j < MAX_TASKS; j++) {
          if (pumpkin_module.tasks[j].removed) {
            wman_remove(pumpkin_module.wm, pumpkin_module.tasks[j].taskId, 1);
            pumpkin_module.tasks[j].taskId = -1;
            pumpkin_module.tasks[j].removed = 0;
            pumpkin_module.render = 1;
          }
        }

        if (pumpkin_module.fullrefresh || pumpkin_module.refresh) {
          for (j = 0; j < pumpkin_module.num_tasks; j++) {
            i = pumpkin_module.task_order[j];
            draw_task(i, &x, &y, &w, &h);
          }
          wman_draw_all(pumpkin_module.wm);
          pumpkin_module.render = 1;

        } else {
          for (j = 0; j < pumpkin_module.num_tasks; j++) {
            i = pumpkin_module.task_order[j];
            if (draw_task(i, &x, &y, &w, &h) && pumpkin_module.wm) {
              wman_update(pumpkin_module.wm, pumpkin_module.tasks[i].taskId, x, y, w, h);
              pumpkin_module.render = 1;
            }
          }
        }
        pumpkin_module.refresh = 0;

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

      if (pumpkin_module.dia) {
        if (dia_update(pumpkin_module.dia)) {
          if (pumpkin_module.mode == 2) dia_refresh(pumpkin_module.dia);
          pumpkin_module.render = 1;
        }
      }

      if (pumpkin_module.render) {
        if (pumpkin_module.taskbar && pumpkin_module.taskbar_enabled) {
          taskbar_draw(pumpkin_module.taskbar);
        } else {
}
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
          if (arg1 == pumpkin_module.lockKey && (pumpkin_module.modMask & pumpkin_module.lockModifiers)) {
            if (pumpkin_module.locked) {
              if (pumpkin_module.wp->show_cursor) pumpkin_module.wp->show_cursor(pumpkin_module.w, 1);
              wman_choose_border(pumpkin_module.wm, 0);
              wman_raise(pumpkin_module.wm, pumpkin_module.tasks[i].taskId);
              pumpkin_module.locked = 0;
              pumpkin_module.render = 1;
            } else if (pumpkin_module.tasks[i].lockable) {
              wman_choose_border(pumpkin_module.wm, 1);
              wman_raise(pumpkin_module.wm, pumpkin_module.tasks[i].taskId);
              pumpkin_module.locked = 1;
              pumpkin_module.render = 1;
            }
          }

          if (arg1 == WINDOW_KEY_F12) {
            save_screen();
          } else {
            pumpkin_forward_msg(i, MSG_KEYDOWN, arg1, 0, 0);
          }
        }
        pumpkin_set_key(arg1);
        break;
      case WINDOW_KEYUP:
        if (arg1 == WINDOW_KEY_F12) {
          pumpkin_reset_key(arg1);
          break;
        }
        if (arg1 && i != -1 && pumpkin_module.tasks[i].active) {
          pumpkin_forward_msg(i, MSG_KEYUP, arg1, 0, 0);
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
            pumpkin_forward_msg(i, MSG_KEY, arg1, arg2, 0);
          }
        }
        break;
      case WINDOW_BUTTONDOWN:
        if (pumpkin_module.dia) {
          pumpkin_module.wp->status(pumpkin_module.w, &pumpkin_module.lastX, &pumpkin_module.lastY, &tmp);
          if (dia_clicked(pumpkin_module.dia, pumpkin_module.current_task, pumpkin_module.lastX, pumpkin_module.lastY, 1) == 0) break;
        }

        if (pumpkin_module.taskbar && pumpkin_module.taskbar_enabled) {
          pumpkin_module.wp->status(pumpkin_module.w, &pumpkin_module.lastX, &pumpkin_module.lastY, &tmp);
          if (pumpkin_module.lastY >= pumpkin_module.height - TASKBAR_HEIGHT && pumpkin_module.lastY < pumpkin_module.height) {
            break;
          }
        }

        if (i != -1 && pumpkin_module.locked) {
          pumpkin_module.buttonMask |= arg1;
          pumpkin_forward_msg(i, MSG_BUTTON, 0, 0, arg1);
        } else {
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
            pumpkin_module.buttonMask |= arg1;
            pumpkin_module.dragging = -1;
            if (i != -1 && i == pumpkin_module.current_task) {
              if (pumpkin_module.mode == 1) {
                pumpkin_forward_msg(i, MSG_MOTION, tx, ty, 0);
                pumpkin_module.tasks[i].penX = tx;
                pumpkin_module.tasks[i].penY = ty;
              }
              pumpkin_forward_msg(i, MSG_BUTTON, 0, 0, arg1);
            }
          }
        }
        break;
      case WINDOW_BUTTONUP:
        if (pumpkin_module.dia) {
          pumpkin_module.wp->status(pumpkin_module.w, &pumpkin_module.lastX, &pumpkin_module.lastY, &tmp);
          if (dia_clicked(pumpkin_module.dia, pumpkin_module.current_task, pumpkin_module.lastX, pumpkin_module.lastY, 0) == 0) break;
        }

        if (pumpkin_module.taskbar && pumpkin_module.taskbar_enabled) {
          pumpkin_module.wp->status(pumpkin_module.w, &pumpkin_module.lastX, &pumpkin_module.lastY, &tmp);

          if (pumpkin_module.lastY >= pumpkin_module.height - TASKBAR_HEIGHT && pumpkin_module.lastY < pumpkin_module.height) {
            if ((taskId = taskbar_clicked(pumpkin_module.taskbar, pumpkin_module.lastX)) != -1) {
              for (i = 0; i < MAX_TASKS; i++) {
                if (pumpkin_module.tasks[i].active && pumpkin_module.tasks[i].taskId == taskId) {
                  pumpkin_make_current(i);
                }
              }
            } else if ((taskId = taskbar_widget_clicked(pumpkin_module.taskbar, pumpkin_module.lastX, &widgetId)) != -1) {
              for (i = 0; i < MAX_TASKS; i++) {
                if (pumpkin_module.tasks[i].active && pumpkin_module.tasks[i].taskId == taskId) {
                  debug(DEBUG_INFO, PUMPKINOS, "forward MSG_WIDGET %u", widgetId);
                  pumpkin_forward_msg(i, MSG_WIDGET, widgetId, 0, 0);
                }
              }
            }
            break;
          }
        }

        pumpkin_module.buttonMask &= ~arg1;
        pumpkin_module.dragging = -1;

        if (i != -1 && (pumpkin_module.locked || arg1 == 1)) {
          pumpkin_forward_msg(i, MSG_BUTTON, 0, 0, 0);
        }
        break;
      case WINDOW_MOTION:
        x = arg1;
        y = arg2;

        if (pumpkin_module.dragging != -1) {
          pumpkin_module.tasks[pumpkin_module.dragging].dx += x - pumpkin_module.lastX;
          pumpkin_module.tasks[pumpkin_module.dragging].dy += y - pumpkin_module.lastY;
          pumpkin_module.dragged = 1;

        } else if (i != -1 && pumpkin_module.tasks[i].active) {
          mult = 1;
          if (pumpkin_module.tasks[i].density == kDensityLow && pumpkin_module.density == kDensityDouble) {
            mult = 2;
          }

          if (pumpkin_module.locked) {
            if (wman_xy(pumpkin_module.wm, pumpkin_module.tasks[i].taskId, &tx, &ty) == 0) {
              x -= tx;
              y -= ty;
              if (x < 0) x = 0; else if (x >= pumpkin_module.tasks[i].width) x = pumpkin_module.tasks[i].width - 1;
              if (y < 0) y = 0; else if (y >= pumpkin_module.tasks[i].height) y = pumpkin_module.tasks[i].height - 1;
              if (pumpkin_module.tasks[i].penX != x || pumpkin_module.tasks[i].penY != y) {
                pumpkin_forward_msg(i, MSG_MOTION, x/mult, y/mult, 0);
              }
              pumpkin_module.tasks[i].penX = x;
              pumpkin_module.tasks[i].penY = y;
              x += tx;
              y += ty;
            }

          } else if (!pumpkin_module.dia || !dia_stroke(pumpkin_module.dia, x, y)) {
            if (wman_xy(pumpkin_module.wm, pumpkin_module.tasks[i].taskId, &tx, &ty) == 0) {
              x -= tx;
              y -= ty;
              if (x >= 0 && x < pumpkin_module.tasks[i].width && y >= 0 && y < pumpkin_module.tasks[i].height) {
                // try not to flood the task with penMove events
                if ((pumpkin_module.tasks[i].penX != x || pumpkin_module.tasks[i].penY != y) &&
                    (now - pumpkin_module.tasks[i].lastMotion) > 5000) {
                  pumpkin_forward_msg(i, MSG_MOTION, x/mult, y/mult, 0);
                  pumpkin_module.tasks[i].lastMotion = now;
                }
                pumpkin_module.tasks[i].penX = x;
                pumpkin_module.tasks[i].penY = y;
              }
              x += tx;
              y += ty;
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

  thread_yield(0);

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
      if (x) {
        *x = pumpkin_module.tasks[i].penX;
        if (task->density == kDensityLow && pumpkin_module.density == kDensityDouble) *x = *x / 2;
      }
      if (y) {
        *y = pumpkin_module.tasks[i].penY;
        if (task->density == kDensityLow && pumpkin_module.density == kDensityDouble) *y = *y / 2;
      }
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

static void pumpkin_update_single_app(void) {
  int x, y, w, h;
  uint64_t now;

  now = sys_get_clock();

  if ((now - pumpkin_module.lastUpdate) > 50000) {
    if (pumpkin_module.fullrefresh) {
      draw_task(0, &x, &y, &w, &h);
      wman_update(pumpkin_module.wm, 0, 0, 0, pumpkin_module.tasks[0].width, pumpkin_module.tasks[0].height);
      pumpkin_module.render = 1;
    } else if (draw_task(0, &x, &y, &w, &h)) {
      wman_update(pumpkin_module.wm, 0, x, y, w, h);
      pumpkin_module.render = 1;
    }

    if (pumpkin_module.dia && dia_update(pumpkin_module.dia)) {
      if (pumpkin_module.fullrefresh) dia_refresh(pumpkin_module.dia);
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
}

static int pumpkin_event_single_app(int *key, int *mods, int *buttons, uint8_t *data, uint32_t *n, uint32_t usec) {
  int ev, arg1, arg2, wait;
  int x, y, tmp;

  if ((ev = get_event(&arg1, &arg2, &tmp)) != 0) {
    *key = arg1;
    *mods = arg2;
    *buttons = tmp;

  } else {
    wait = usec/1000;
    if (usec && !wait) wait = 1;

    if ((ev = pumpkin_module.wp->event2(pumpkin_module.w, wait, &arg1, &arg2)) > 0) {
      switch (ev) {
        case WINDOW_KEYDOWN:
          pumpkin_set_key(arg1);
          if (pumpkin_module.dia) {
            ev = 0;
          } else {
            *key = arg1;
            *mods = pumpkin_module.modMask;
            *buttons = 0;
            ev = MSG_KEYDOWN;
          }
          break;
        case WINDOW_KEYUP:
          if (arg1 == WINDOW_KEY_F12) {
            save_screen();
            ev = 0;
          } else {
            if (pumpkin_module.dia) {
              *key = pumpkin_reset_key(arg1);
              *mods = pumpkin_module.modMask;
              *buttons = 0;
              ev = MSG_KEY;
            } else {
              *key = arg1;
              *mods = 0;
              *buttons = 0;
              ev = MSG_KEYUP;
              put_event(MSG_KEY, pumpkin_reset_key(arg1), pumpkin_module.modMask, 0);
            }
          }
          break;
        case WINDOW_BUTTONDOWN:
          pumpkin_module.wp->status(pumpkin_module.w, &arg1, &arg2, &tmp);
          pumpkin_module.lastX = pumpkin_module.wp->average ? calibrate_x(arg1, arg2) : arg1;
          pumpkin_module.lastY = pumpkin_module.wp->average ? calibrate_y(arg1, arg2) : arg2;
          if (pumpkin_module.dia && dia_clicked(pumpkin_module.dia, 0, pumpkin_module.lastX, pumpkin_module.lastY, 1) == 0) {
            ev = 0;
            break;
          }
          pumpkin_module.buttonMask |= arg1;
          pumpkin_module.tasks[0].penX = pumpkin_module.lastX;
          pumpkin_module.tasks[0].penY = pumpkin_module.lastY;
          put_event(MSG_BUTTON, 0, 0, 1);
          *key = pumpkin_module.lastX;
          *mods = pumpkin_module.lastY;
          *buttons = 0;
          ev = MSG_MOTION;
          break;
        case WINDOW_BUTTONUP:
          pumpkin_module.wp->status(pumpkin_module.w, &arg1, &arg2, &tmp);
          pumpkin_module.lastX = pumpkin_module.wp->average ? calibrate_x(arg1, arg2) : arg1;
          pumpkin_module.lastY = pumpkin_module.wp->average ? calibrate_y(arg1, arg2) : arg2;
          if (pumpkin_module.dia && dia_clicked(pumpkin_module.dia, 0, pumpkin_module.lastX, pumpkin_module.lastY, 0) == 0) {
            ev = 0;
            break;
          }
          pumpkin_module.buttonMask &= ~arg1;
          *key = *mods = 0;
          *buttons = 0;
          ev = MSG_BUTTON;
          break;
        case WINDOW_MOTION:
          ev = 0;
          x = arg1;
          y = arg2;
          x = pumpkin_module.wp->average ? calibrate_x(arg1, arg2) : arg1;
          y = pumpkin_module.wp->average ? calibrate_y(arg1, arg2) : arg2;
          if (!pumpkin_module.dia || !dia_stroke(pumpkin_module.dia, x, y)) {
            if (x >= 0 && x < pumpkin_module.tasks[0].width && y >= 0 && y < pumpkin_module.tasks[0].height) {
              pumpkin_module.tasks[0].penX = x;
              pumpkin_module.tasks[0].penY = y;
              *key = x;
              *mods = y;
              ev = MSG_MOTION;
            }
          }
          pumpkin_module.lastX = x;
          pumpkin_module.lastY = y;
          break;
        default:
          ev = 0;
          break;
      }
    }
  }

  if (pumpkin_module.dia && !pumpkin_module.dia_kbd) {
    pumpkin_dia_kbd();
    pumpkin_module.dia_kbd = 1;
  }
  pumpkin_update_single_app();

  return ev;
}

static int pumpkin_event_multi_thread(int *key, int *mods, int *buttons, uint8_t *data, uint32_t *n, uint32_t usec) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  client_request_t *creq;
  notif_msg_t *notif;
  notif_union_t nparam;
  //launch_msg_t *launch;
  //launch_union_t lparam;
  EventType event;
  UInt16 prev;
  unsigned char *buf;
  unsigned int len;
  uint32_t *arg, reply;
  char snotify[8];
  int r, client, ev = 0;

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
          debug(DEBUG_INFO, PUMPKINOS, "pumpkin_event pausing, sending ack to %d", client);
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
        case MSG_WIDGET:
          MemSet(&event, sizeof(EventType), 0);
          event.eType = appWidgetEvent;
          event.data.widget.id = arg[1];
          EvtAddEventToQueue(&event);
          ev = 0;
          break;
        case MSG_NOTIFY:
          if (len >= sizeof(notif_msg_t)) {
            notif = (notif_msg_t *)buf;
            pumpkin_id2s(notif->notify.notifyType, snotify);
            debug_bytes(DEBUG_TRACE, PUMPKINOS, buf, sizeof(notif_msg_t));
            debug_bytes(DEBUG_TRACE, PUMPKINOS, buf + sizeof(notif_msg_t), len - sizeof(notif_msg_t));
            deserialize_notif(notif->notify.notifyType, buf + sizeof(notif_msg_t), len - sizeof(notif_msg_t), &nparam);
            debug(DEBUG_INFO, PUMPKINOS, "received notification '%s' (%u bytes)", snotify, len);
            notif->notify.notifyDetailsP = &nparam;
            if (notif->callback) {
              debug(DEBUG_INFO, PUMPKINOS, "delivering notification via callback");
              notif->callback(&notif->notify);
            } else if (notif->callback68k) {
              debug(DEBUG_INFO, PUMPKINOS, "delivering notification via 68k callback");
              CallNotifyProc(notif->callback68k, &notif->notify, len - sizeof(notif_msg_t));
            } else if (task->pilot_main) {
              debug(DEBUG_INFO, PUMPKINOS, "delivering notification via PilotMain");
              task->paramBlockSize = 18 + len - sizeof(notif_msg_t);
              task->pilot_main(sysAppLaunchCmdNotify, &notif->notify, 0);
            } else {
              debug(DEBUG_ERROR, PUMPKINOS, "task has no callback or PilotMain for delivering notifications");
            }
          } else {
            debug(DEBUG_ERROR, PUMPKINOS, "wrong notification message length %u", len);
          }
          ev = 0;
          break;
/*
        case MSG_LAUNCHC:
          if (len >= sizeof(launch_msg_t)) {
            launch = (launch_msg_t *)buf;
            debug_bytes(DEBUG_TRACE, PUMPKINOS, buf, sizeof(launch_msg_t));
            if (len > sizeof(launch_msg_t)) {
              debug_bytes(DEBUG_TRACE, PUMPKINOS, buf + sizeof(launch_msg_t), len - sizeof(launch_msg_t));
              deserialize_launch(launch->cmd, buf + sizeof(launch_msg_t), len - sizeof(launch_msg_t), &lparam);
            }
            debug(DEBUG_INFO, PUMPKINOS, "received launch code %d (%u bytes)", launch->cmd, len);
            if (task->pilot_main) {
              debug(DEBUG_INFO, PUMPKINOS, "delivering launch code via PilotMain");
              task->paramBlockSize = len - sizeof(launch_msg_t);
              task->pilot_main(launch->cmd, task->paramBlockSize ? &lparam : NULL, launch->flags);
            } else {
              debug(DEBUG_ERROR, PUMPKINOS, "task has no PilotMain for delivering launch codes");
            }
          }
          ev = 0;
          break;
*/
        default:
          len -= sizeof(uint32_t);
          if (len <= *n) {
            *n = len;
          } else {
            debug(DEBUG_ERROR, PUMPKINOS, "pumpkin_event event %u len %u > %u", ev, len, *n);
            len = *n;
          }
          if (data) sys_memcpy(data, &arg[1], len);
          break;
      }
    } else {
      debug(DEBUG_ERROR, PUMPKINOS, "pumpkin_event invalid len %u", len);
    }
    if (buf) sys_free(buf);

  } else if (r == -1) {
    ev = -1;
  }

  while (task->paused) {
    if ((r = thread_server_read_timeout_from(10000, &buf, &len, &client)) == -1) break;
    if (r == 0) continue;
    if (buf == NULL) continue;
    if (client != task->paused) {
      sys_free(buf);
      continue;
    }
    arg = (uint32_t *)buf;
    if (len != sizeof(uint32_t) || *arg != MSG_RESUME) {
      sys_free(buf);
      continue;
    }
    debug(DEBUG_INFO, PUMPKINOS, "pumpkin_event resuming");
    task->paused = 0;
  }

  return ev;
}

int pumpkin_event(int *key, int *mods, int *buttons, uint8_t *data, uint32_t *n, uint32_t usec) {
  return pumpkin_module.mode == 1 ?
    pumpkin_event_single_app(key, mods, buttons, data, n, usec) :
    pumpkin_event_multi_thread(key, mods, buttons, data, n, usec);
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

uint32_t pumpkin_get_param_size(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task ? task->paramBlockSize : 0;
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

surface_t *pumpkin_screen_lock(void **scr) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  task_screen_t *screen;
  surface_t *surface = NULL;

  if (scr && (screen = ptr_lock(task->screen_ptr, TAG_SCREEN))) {
    surface = screen->surface;
    *scr = screen;
  }

  return surface;
}

void pumpkin_screen_unlock(void *scr, int x0, int y0, int x1, int y1) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  task_screen_t *screen = (task_screen_t *)scr;

  if (x0 < screen->x0) screen->x0 = x0;
  if (y0 < screen->y0) screen->y0 = y0;
  if (x1 > screen->x1) screen->x1 = x1;
  if (y1 > screen->y1) screen->y1 = y1;

  if (task->density == kDensityLow && pumpkin_module.density == kDensityDouble) {
    if (screen->x0 > 0) screen->x0 <<= 1;
    if (screen->x1 > 0) screen->x1 = (screen->x1 << 1) + 1;
    if (screen->y0 > 0) screen->y0 <<= 1;
    if (screen->y1 > 0) screen->y1 = (screen->y1 << 1) + 1;
  }

  if (screen->x0 < 0) screen->x0 = 0;
  else if (screen->x0 >= task->width) screen->x0 = task->width-1;
  if (screen->x1 < 0) screen->x1 = 0;
  else if (screen->x1 >= task->width) screen->x1 = task->width-1;

  if (screen->y0 < 0) screen->y0 = 0;
  else if (screen->y0 >= task->height) screen->y0 = task->height-1;
  if (screen->y1 < 0) screen->y1 = 0;
  else if (screen->y1 >= task->height) screen->y1 = task->height-1;

  screen->dirty = 1;

  ptr_unlock(task->screen_ptr, TAG_SCREEN);
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

void pumpkin_dirty_region_mode(dirty_region_e d) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  task_screen_t *screen;

  switch (d) {
    case dirtyRegionBegin:
      task->dirty_level++;
      break;
    case dirtyRegionEnd:
      if (task->dirty_level > 0) task->dirty_level--;
      if (task->dirty_level == 0) {
        if ((screen = ptr_lock(task->screen_ptr, TAG_SCREEN))) {
          if (screen->x0 <= screen->x1 && screen->y0 <= screen->y1) {
            screen->dirty = 1;
          }
          ptr_unlock(task->screen_ptr, TAG_SCREEN);
        }
      }
      break;
    case dirtyRegionReset:
      task->dirty_level = 0;
      break;
  }
}

void pumpkin_screen_dirty(WinHandle wh, int x, int y, int w, int h) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  task_screen_t *screen;
  BitmapType *bmp;
  Boolean dbl;
  Coord sx, sy;
  int xd, yd, wd, hd;

  if (!task) return;

//debug(1, "XXX", "pumpkin_screen_dirty (%d,%d,%d,%d) ...", x, y, w, h);
  if (wh) {
    switch (task->density) {
      case kDensityLow:
        dbl = pumpkin_module.density == kDensityDouble;
        break;
      case kDensityDouble:
        dbl = false;
        break;
      default:
        dbl = false;
        break;
    }

    if ((screen = ptr_lock(task->screen_ptr, TAG_SCREEN))) {
      WinGetPosition(wh, &sx, &sy);
      if (pumpkin_module.density == kDensityDouble) {
        sx <<= 1;
        sy <<= 1;
      }

      if (dbl) {
        xd = x << 1;
        yd = y << 1;
        wd = w << 1;
        hd = h << 1;
      } else {
        xd = x;
        yd = y;
        wd = w;
        hd = h;
      }
//debug(1, "XXX", "pumpkin_screen_dirty BmpDrawSurface (%d,%d,%d,%d) %d,%d", x, y, w, h, sx+x, sy+y);
      xd += sx;
      yd += sy;
      bmp = WinGetBitmap(wh);
      BmpDrawSurface(bmp, x, y, w, h, screen->surface, xd, yd, true, dbl);
//debug(1, "XXX", "pumpkin_screen_dirty x=%d y=%d", x, y);
//debug(1, "XXX", "pumpkin_screen_dirty dirty before (%d,%d,%d,%d)", screen->x0, screen->y0, screen->x1, screen->y1);

      if (xd < screen->x0) screen->x0 = xd;
      if (xd+wd-1 > screen->x1) screen->x1 = xd+wd-1;

      if (screen->x0 < 0) screen->x0 = 0;
      else if (screen->x0 >= task->width) screen->x0 = task->width-1;
      if (screen->x1 < 0) screen->x1 = 0;
      else if (screen->x1 >= task->width) screen->x1 = task->width-1;

      if (yd < screen->y0) screen->y0 = yd;
      if (yd+hd-1 > screen->y1) screen->y1 = yd+hd-1;

      if (screen->y0 < 0) screen->y0 = 0;
      else if (screen->y0 >= task->height) screen->y0 = task->height-1;
      if (screen->y1 < 0) screen->y1 = 0;
      else if (screen->y1 >= task->height) screen->y1 = task->height-1;

//debug(1, "XXX", "pumpkin_screen_dirty dirty (%d,%d,%d,%d)", screen->x0, screen->y0, screen->x1, screen->y1);

      ptr_unlock(task->screen_ptr, TAG_SCREEN);
    }

    if (pumpkin_module.mode == 1 && pumpkin_module.launched) {
      pumpkin_update_single_app();
    }
  }

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
        sys_memcpy(pumpkin_module.clipboardAux, s, len);
      }
      if (len + length > cbdMaxTextLength*2) length = cbdMaxTextLength*2 - len;
      if (length > 0) {
        sys_memcpy(&pumpkin_module.clipboardAux[len], text, length);
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
        sys_memcpy(text, s, len);
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
  launch_request_t request;
  int r = 0;

  if (task->alarm_time) {
    // there is an alarm set
    if (sys_time() >= task->alarm_time) {
      debug(DEBUG_INFO, PUMPKINOS, "pumpkin_alarm_check: alarm time has arrived");

      sys_memset(&request, 0, sizeof(request));
      request.code = sysAppLaunchCmdAlarmTriggered;
      request.hasParam = true;
      request.param.p1.alarmSeconds = task->alarm_time;
      request.param.p1.ref = task->alarm_data;

      if (pumpkin_launch_sub(&request, 0) == 0) {
        if (request.param.p1.purgeAlarm) {
          // application requested to cancel the alarm
          debug(DEBUG_INFO, PUMPKINOS, "pumpkin_alarm_check: purge alarm");
          task->alarm_time = 0;
          task->alarm_data = 0;

        } else {
          // go ahead and send the alarm
          task->alarm_time = 0;
          task->alarm_data = 0;

          sys_memset(&request, 0, sizeof(request));
          request.code = sysAppLaunchCmdDisplayAlarm;
          request.param.p2.alarmSeconds = task->alarm_time;
          request.param.p2.ref = task->alarm_data;
          request.param.p2.soundAlarm = false; // not used

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
      pumpkin_module.serial[pumpkin_module.num_serial].descr = sys_calloc(1, sys_strlen(descr) + 1);
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
      val = sys_strdup(buf);
      break;
    case SCRIPT_ARG_REAL:
      sys_snprintf(buf, sizeof(buf)-1, "%f", ret->value.d);
      val = sys_strdup(buf);
      break;
    case SCRIPT_ARG_BOOLEAN:
      sys_strcpy(buf, ret->value.i ? "true" : "false");
      val = sys_strdup(buf);
      break;
    case SCRIPT_ARG_STRING:
      val = ret->value.s;
      break;
    case SCRIPT_ARG_LSTRING:
      val = sys_calloc(1, ret->value.l.n + 1);
      if (val) {
        sys_memcpy(val, ret->value.l.s, ret->value.l.n);
      }
      sys_free(ret->value.l.s);
      break;
    case SCRIPT_ARG_OBJECT:
      sys_strcpy(buf, "<object>");
      val = sys_strdup(buf);
      script_remove_ref(pe, ret->value.r);
      break;
    case SCRIPT_ARG_FUNCTION:
      sys_strcpy(buf, "<function>");
      val = sys_strdup(buf);
      script_remove_ref(pe, ret->value.r);
      break;
    case SCRIPT_ARG_NULL:
      val = NULL;
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "type(%c)", ret->type);
      val = sys_strdup(buf);
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

      debug(DEBUG_INFO, PUMPKINOS, "calling function %s(\"%s\")", function, s);
      if (script_call_args(pe, f.value.r, &ret, 1, &arg) == 0) {
        val = pumpkin_script_ret(pe, &ret);
      } else {
        debug(DEBUG_ERROR, PUMPKINOS, "function call error");
      }
    } else {
      debug(DEBUG_ERROR, PUMPKINOS, "attempt to call non function \"%s\"", function);
    }
  } else {
    debug(DEBUG_ERROR, PUMPKINOS, "function \"%s\" not found", function);
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
  FrmCustomAlert(ErrOKAlert, msg, "", "");
}

void pumpkin_fatal_error(int finish) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  debug(DEBUG_ERROR, PUMPKINOS, "pumpkin_fatal_error finish=%d", finish);
  if (finish || pumpkin_module.mode == 1) {
    sys_set_finish(finish);
    sys_exit(finish);
  }

  debug(DEBUG_ERROR, PUMPKINOS, "calling ErrLongJump");
  ErrLongJump(task->jmpbuf, 1);
  debug(DEBUG_ERROR, PUMPKINOS, "after ErrLongJump!");
}

void pumpkin_set_size(uint32_t creator, uint16_t width, uint16_t height) {
  RegDimensionType regDim;

  regDim.width = width;
  regDim.width = height;
  pumpkin_reg_set(creator, regDimensionID, &regDim, sizeof(RegDimensionType));
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
          sys_memcpy(task->syslibs[i].code, code, size);
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
      sys_memset(&task->syslibs[i], 0, sizeof(syslib_t));
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

void pumpkin_trace(uint16_t trap) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  if (task && task->tracing) {
    switch (trap) {
      case sysTrapWinCopyRectangle:
      case sysTrapWinDrawBitmap:
      case sysTrapWinDrawChar:
      case sysTrapWinDrawChars:
      case sysTrapWinDrawGrayLine:
      case sysTrapWinDrawGrayRectangleFrame:
      case sysTrapWinDrawInvertedChars:
      case sysTrapWinDrawLine:
      case sysTrapWinDrawPixel:
      case sysTrapWinDrawRectangle:
      case sysTrapWinDrawRectangleFrame:
      case sysTrapWinDrawTruncChars:
      case sysTrapWinDrawWindowFrame:
      case sysTrapWinEraseChars:
      case sysTrapWinEraseLine:
      case sysTrapWinErasePixel:
      case sysTrapWinEraseRectangle:
      case sysTrapWinEraseRectangleFrame:
      case sysTrapWinEraseWindow:
      case sysTrapWinFillLine:
      case sysTrapWinFillRectangle:
      case sysTrapWinInvertChars:
      case sysTrapWinInvertLine:
      case sysTrapWinInvertPixel:
      case sysTrapWinInvertRectangle:
      case sysTrapWinInvertRectangleFrame:
      case sysTrapWinPaintBitmap:
      case sysTrapWinPaintChar:
      case sysTrapWinPaintChars:
      case sysTrapWinPaintLine:
      case sysTrapWinPaintLines:
      case sysTrapWinPaintPixel:
      case sysTrapWinPaintPixels:
      case sysTrapWinPaintRectangle:
      case sysTrapWinPaintRectangleFrame:
      case sysTrapWinRestoreBits:
      case sysTrapWinScrollRectangle:
        debug(DEBUG_INFO, PUMPKINOS, "tracing 0x%04X", trap);
        SysTaskDelay(40);
        break;
    }
  }
}

void pumpkin_set_osversion(int version) {
  debug(DEBUG_INFO, PUMPKINOS, "setting global OS version=%d", version);
  pumpkin_module.osversion = version;
}

int pumpkin_get_default_osversion(void) {
  return pumpkin_module.osversion;
}

int pumpkin_get_osversion(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task ? task->osversion : pumpkin_module.osversion;
}

void pumpkin_set_m68k(int m68k) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  task->m68k = m68k;

  if (mutex_lock(mutex) == 0) {
    pumpkin_module.tasks[task->task_index].m68k = m68k;
    mutex_unlock(mutex);
  }

  pumpkin_forward_msg(0, MSG_KEY, WINDOW_KEY_CUSTOM, vchrRefreshState, 0);
}

int pumpkin_is_m68k(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task ? task->m68k : 0;
}

void pumpkin_set_preference(UInt32 creator, UInt16 id, void *p, UInt16 size, Boolean saved) {
  DmOpenRef dbRef;
  MemHandle h;
  UInt16 currentSize, index;
  UInt8 *pref;

  if (mutex_lock(mutex) == 0) {
    if ((dbRef = PrefOpenPreferenceDB(saved)) != NULL) {
      if ((index = DmFindResource(dbRef, creator, id, NULL)) == 0xFFFF) {
        h = DmNewResourceEx(dbRef, creator, id, size, p);
        index = DmFindResource(dbRef, 0, 0, h);
      }
      if ((h = DmGetResourceIndex(dbRef, index)) != NULL) {
        currentSize = MemHandleSize(h);
        if (size != currentSize) {
          DmResizeResource(h, size);
        }
        if ((pref = MemHandleLock(h)) != NULL) {
          DmWrite(pref, 0, p, size);
          MemHandleUnlock(h);
        }
        DmReleaseResource(h);
      }
      DmCloseDatabase(dbRef);
    }
    mutex_unlock(mutex);
  }
}

UInt16 pumpkin_get_preference(UInt32 creator, UInt16 id, void *p, UInt16 size, Boolean saved) {
  DmOpenRef dbRef;
  MemHandle h;
  UInt16 currentSize, index;
  UInt8 *pref;

  if (mutex_lock(mutex) == 0) {
    if ((dbRef = PrefOpenPreferenceDB(saved)) != NULL) {
      if ((index = DmFindResource(dbRef, creator, id, NULL)) != 0xFFFF) {
        if ((h = DmGetResourceIndex(dbRef, index)) != NULL) {
          if ((pref = MemHandleLock(h)) != NULL) {
            currentSize = MemHandleSize(h);
            if (p == NULL || size == 0) {
              size = currentSize;
            } else {
              if (size > currentSize) size = currentSize;
              MemMove(p, pref, size);
            }
            MemHandleUnlock(h);
          } else {
            size = 0;
          }
          DmReleaseResource(h);
        } else {
          size = 0;
        }
      } else {
        size = 0;
      }
      DmCloseDatabase(dbRef);
    } else {
      size = 0;
    }
    mutex_unlock(mutex);
  }

  return size;
}

void pumpkin_delete_preferences(UInt32 creator, Boolean saved) {
  DmOpenRef dbRef;
  UInt16 i, index;

  if (mutex_lock(mutex) == 0) {
    if ((dbRef = PrefOpenPreferenceDB(saved)) != NULL) {
      for (i = 0; ; i++) {
        if ((index = DmFindResourceType(dbRef, creator, i)) == 0xFFFF) break;
        DmRemoveResource(dbRef, index);
      }
      DmCloseDatabase(dbRef);
    }
    mutex_unlock(mutex);
  }
}

void pumpkin_delete_registry(UInt32 creator) {
  RegDelete(pumpkin_module.rm, creator);
}

void pumpkin_save_bitmap(BitmapType *bmp, UInt16 density, Coord wWidth, Coord wHeight, Coord width, Coord height, char *filename) {
  surface_t *surface;
  UInt16 oldDensity;
  char *card, buf[256];

  if (width == 0 || height == 0) {
    BmpGetDimensions(bmp, &width, &height, NULL);
  }
  if (wWidth == 0 || wHeight == 0) {
    wWidth = width;
    wHeight = height;
  }

  if ((surface = surface_create(wWidth, wHeight, SURFACE_ENCODING_ARGB)) != NULL) {
    oldDensity = BmpGetDensity(bmp);
    BmpSetDensity(bmp, kDensityLow);
    BmpDrawSurface(bmp, 0, 0, wWidth, wHeight, surface, 0, 0, false, false);
    BmpSetDensity(bmp, oldDensity);

    card = VFS_CARD;
    if (card[0] == '/') card++;
    sys_snprintf(buf, sizeof(buf)-1, "%s%s%s", VFSGetMount(1), card, filename);
    debug(DEBUG_INFO, PUMPKINOS, "saving %dx%d bitmap as %s", width, height, buf);
    surface_save(surface, buf, 0);

    surface_destroy(surface);
  }
}

void pumpkin_save_surface(surface_t *surface, char *filename) {
  char *card, buf[256];

  card = VFS_CARD;
  if (card[0] == '/') card++;
  sys_snprintf(buf, sizeof(buf)-1, "%s%s%s", VFSGetMount(1), card, filename);
  debug(DEBUG_INFO, PUMPKINOS, "saving %dx%d surface as %s", surface->width, surface->height, buf);
  surface_save(surface, buf, 0);
}

void LongToRGB(UInt32 c, RGBColorType *rgb) {
  rgb->r = r32(c);
  rgb->g = g32(c);
  rgb->b = b32(c);
}

UInt32 RGBToLong(RGBColorType *rgb) {
  return rgba32(rgb->r, rgb->g, rgb->b, 0xFF);
}

#if !defined(KERNEL)
int pumpkin_http_get(char *url, int timeout, int (*callback)(int ptr, void *_data), void *data) {
  return pit_http_get(PUMPKIN_USER_AGENT, url, pumpkin_module.secure, timeout, callback, data);
}

void pumpkin_http_abort(int handle) {
  pit_http_abort(handle);
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

    if (str) sys_free(str);
    if (mime) sys_free(mime);
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

    if (filename) sys_free(filename);
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

  if (s) sys_free(s);

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
      if (mutex_lock(pumpkin_module.fs_mutex) == 0) {
        r = pumpkin_deploy_from_image(pumpkin_module.session, p, len);
      }
      mutex_unlock(pumpkin_module.fs_mutex);
      MemPtrFree(p);
    }
  }

  if (s) sys_free(s);

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
      prefix = sys_malloc(value.value.l.n + 1);
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
      sys_free(prefix);
    }
  }

  if (filename) sys_free(filename);
  if (mimetype) sys_free(mimetype);

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

  if (root && (h = sys_calloc(1, sizeof(pumpkin_httpd_t))) != NULL) {
    h->idle = idle;
    h->data = data;

    if ((hscript = DmGet1Resource(pumpkin_script_engine_id(), scriptId)) != 0) {
      if ((s = MemHandleLock(hscript)) != NULL) {
        len = MemHandleSize(hscript);
        if ((h->script = sys_malloc(len+1)) != NULL) {
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
      sys_snprintf(h->prefix, sizeof(h->prefix)-1, "%s%s", VFSGetMount(1), card);

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
      sys_snprintf(buf, sizeof(buf)-1, "%s%s%s", VFSGetMount(1), card, root);

      if (httpd_create("0.0.0.0", port, PUMPKIN_SERVER_NAME, buf, NULL, NULL, NULL, NULL, NULL, pumpkin_httpd_callback, h, 0) == -1) {
        sys_free(h->script);
        sys_free(h);
        h = NULL;
      }
    } else {
      sys_free(h->script);
      sys_free(h);
      h = NULL;
    }
  }

  return h;
}

int pumpkin_httpd_destroy(pumpkin_httpd_t *h) {
  int r = -1;

  if (h) {
    if (h->script) sys_free(h->script);
    if (h->pe > 0) pumpkin_script_destroy(h->pe);
    sys_free(h);
    r = 0;
  }

  return r;
}
#endif

void pumpkin_save_bmp(char *dbname, UInt32 type, UInt16 id, char *filename) {
  LocalID dbID;
  DmOpenRef dbRef;
  BitmapType *bmp;
  MemHandle h;

  if ((dbID = DmFindDatabase(0, dbname)) != 0) {
    if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
      if ((h = DmGet1Resource(type, id)) != NULL) {
        if ((bmp = MemHandleLock(h)) != NULL) {
          pumpkin_save_bitmap(bmp, BmpGetDensity(bmp), 0, 0, 0, 0, filename);
          MemHandleUnlock(h);
        }
        DmReleaseResource(h);
      }
      DmCloseDatabase(dbRef);
    }
  }
}

language_t *LanguageGet(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task->lang;
}

language_t *LanguageSelect(language_t *lang) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  language_t *old;

  old = task->lang;
  task->lang = lang;

  return old;
}

UInt32 pumpkin_next_char(UInt8 *s, UInt32 i, UInt32 len, UInt32 *w) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  language_t *lang;

  lang = task && task->lang ? task->lang : pumpkin_module.lang;

  if (lang && lang->nextChar) {
    i = lang->nextChar(s, i, len, w, lang->data);
  } else {
   *w = s[i];
    i = 1;
  }

  return i;
}

UInt8 pumpkin_map_char(UInt32 w, FontType **f) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  language_t *lang;

  lang = task && task->lang ? task->lang : pumpkin_module.lang;

  if (lang && lang->mapChar) {
    return lang->mapChar(w, f, lang->data);
  }

  *f = FntGetFontPtr();
  return w;
}

static void notif_ptr_destructor(void *p) {
  if (p) sys_free(p);
}

Err SysNotifyRegister(UInt16 cardNo, LocalID dbID, UInt32 notifyType, SysNotifyProcPtr callbackP, Int8 priority, void *userDataP) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
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
            if (pumpkin_module.notif[i].appCreator == creator && pumpkin_module.notif[i].notifyType == notifyType && pumpkin_module.notif[i].ptr == 0) {
              debug(DEBUG_INFO, PUMPKINOS, "register notification type '%s' creator '%s' priority %d: duplicate", stype, screator, priority);
              err = sysNotifyErrDuplicateEntry;
              break;
            }
          }

          if (i == pumpkin_module.num_notif) {
            debug(DEBUG_INFO, PUMPKINOS, "register notification type '%s' creator '%s' priority %d: added", stype, screator, priority);
            if (callbackP || userDataP) {
              np = sys_calloc(1, sizeof(notif_ptr_t));
              np->tag = TAG_NOTIF;
              if (pumpkin_is_m68k()) {
                np->callback68k = callbackP ? (uint8_t *)callbackP - (uint8_t *)pumpkin_heap_base() : 0;
                np->userData = userDataP;
                debug(DEBUG_INFO, PUMPKINOS, "notification callback callback68k 0x%08X userData %p", np->callback68k, np->userData);
              } else {
                np->callback = callbackP;
                np->userData = userDataP;
                debug(DEBUG_INFO, PUMPKINOS, "notification callback %p usedData %p", np->callback, np->userData);
              }
              ptr = ptr_new(np, notif_ptr_destructor);
            } else {
/*
              debug(DEBUG_INFO, PUMPKINOS, "notification has no callback and no userData");
              n.appCreator = creator;
              n.notifyType = notifyType;
              n.priority = priority;
              AppRegistrySet(pumpkin_module.registry, creator, appRegistryNotification, 0, &n);
*/
              ptr = 0;
            }
            pumpkin_module.notif[i].taskId = task ? task->taskId : -1;
            pumpkin_module.notif[i].appCreator = creator;
            pumpkin_module.notif[i].notifyType = notifyType;
            pumpkin_module.notif[i].priority = priority;
            pumpkin_module.notif[i].ptr = ptr; // (callback + userdata)
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
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  UInt32 creator;
  char screator[8], stype[8];
  int i;
  Err err;

  if ((err = DmDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creator)) == errNone) {
    if (mutex_lock(mutex) == 0) {
      pumpkin_id2s(creator, screator);
      pumpkin_id2s(notifyType, stype);

      for (i = 0; i < pumpkin_module.num_notif; i++) {
        if (pumpkin_module.notif[i].taskId == task->taskId && pumpkin_module.notif[i].appCreator == creator && pumpkin_module.notif[i].notifyType == notifyType) {
          debug(DEBUG_INFO, PUMPKINOS, "unregister notification type '%s' creator '%s' priority %d: removed", stype, screator, pumpkin_module.notif[i].priority);
          if (pumpkin_module.notif[i].ptr) {
            ptr_free(pumpkin_module.notif[i].ptr, TAG_NOTIF);
          } else {
/*
            n.appCreator = creator;
            n.notifyType = notifyType;
            n.priority = 0xFFFF; // invalid priority: remove (appCreator,notifyType)
            AppRegistrySet(pumpkin_module.registry, creator, appRegistryNotification, 0, &n);
*/
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

/*
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
*/

Err SysNotifyBroadcast(SysNotifyParamType *notify) {
  notif_ptr_t *np;
  char stype[8], screator[8];
  int i, n = 0;

  if (notify == NULL) {
    return sysErrParamErr;
  }

  notify->handled = false;
  notify->reserved2 = 0;
  if (notify->broadcaster == 0) {
    notify->broadcaster = pumpkin_get_app_creator();
  }

  pumpkin_id2s(notify->notifyType, stype);
  debug(DEBUG_INFO, PUMPKINOS, "broadcast notification type '%s' begin", stype);

  if (mutex_lock(mutex) == 0) {
    for (i = 0, n = 0; i < pumpkin_module.num_notif; i++) {
      if (pumpkin_module.notif[i].notifyType == notify->notifyType && pumpkin_module.notif[i].appCreator != pumpkin_get_app_creator()) {
        pumpkin_id2s(pumpkin_module.notif[i].appCreator, screator);

        if (pumpkin_module.notif[i].ptr) {
          if ((np = ptr_lock(pumpkin_module.notif[i].ptr, TAG_NOTIF)) != NULL) {
            notify->userDataP = np->userData;
            debug(DEBUG_INFO, PUMPKINOS, "send notification type '%s' priority %d to '%s' taskId %d userData %p",
              stype, pumpkin_module.notif[i].priority, screator, pumpkin_module.notif[i].taskId, notify->userDataP);
            if (pumpkin_module.mode == 0) {
              pumpkin_forward_notif(pumpkin_module.notif[i].taskId, pumpkin_module.notif[i].appCreator, notify, np->callback, np->callback68k);
            } else {
              np->callback(notify);
            }
            ptr_unlock(pumpkin_module.notif[i].ptr, TAG_NOTIF);
          }
        } else {
          debug(DEBUG_INFO, PUMPKINOS, "send notification type '%s' priority %d to '%s' taskId %d NO userData",
            stype, pumpkin_module.notif[i].priority, screator, pumpkin_module.notif[i].taskId);
          notify->userDataP = NULL;
          pumpkin_forward_notif(pumpkin_module.notif[i].taskId, pumpkin_module.notif[i].appCreator, notify, NULL, 0);
        }
        n++;
      }
    }
    mutex_unlock(mutex);
  }

  if (n == 0) {
    debug(DEBUG_INFO, PUMPKINOS, "no app registered for notification type '%s", stype);
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

// XXX this may be incorrect
Err SysNotifyBroadcastFromInterrupt(UInt32 notifyType, UInt32 broadcaster, void *notifyDetailsP) {
  SysNotifyParamType notify;

  MemSet(&notify, sizeof(SysNotifyParamType), 0);
  notify.notifyType = notifyType;
  notify.broadcaster = broadcaster;
  notify.notifyDetailsP = notifyDetailsP;

  return SysNotifyBroadcast(&notify);
}

void *pumpkin_gettable(uint32_t n) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  if (task->table == NULL) {
    task->table = sys_calloc(n, sizeof(void *));
  }

  return task->table;
}

void pumpkin_setio(
  int (*getchar)(void *iodata),
  int (*haschar)(void *iodata),
  void (*putchar)(void *iodata, char c),
  void (*putstr)(void *iodata, char *s, uint32_t len),
  void (*setcolor)(void *iodata, uint32_t fg, uint32_t bg), void *iodata) {

  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  task->getchar = getchar;
  task->haschar = haschar;
  task->putchar = putchar;
  task->putstr = putstr;
  task->setcolor = setcolor;
  task->iodata = iodata;
}

int pumpkin_getchar(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  return task->getchar ? task->getchar(task->iodata) : 0;
}

int pumpkin_haschar(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  return task->haschar ? task->haschar(task->iodata) : 0;
} 

void pumpkin_putchar(char c) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  if (task->putchar) {
    task->putchar(task->iodata, c);
  }
}

void pumpkin_write(char *s, uint32_t len) {
  uint32_t i;

  for (i = 0; i < len; i++) {
    if (s[i] == '\n' && (i == 0 || s[i-1] != '\r')) {
      pumpkin_putchar('\r');
    }
    pumpkin_putchar(s[i]);
  }
}

void pumpkin_puts(char *s) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  if (task->putstr) {
    task->putstr(task->iodata, s, sys_strlen(s));
  } else {
    pumpkin_write(s, sys_strlen(s));
  }
}

uint32_t pumpkin_vprintf(const char *format, sys_va_list ap) {
  char *buf;
  uint32_t n = 0;

  if (format) {
    if ((buf = sys_malloc(16384)) != NULL) {
      n = sys_vsnprintf(buf, 16384, format, ap);
      pumpkin_puts(buf);
      sys_free(buf);
    }
  }

  return n;
}

uint32_t pumpkin_printf(const char *format, ...) {
  sys_va_list ap;
  uint32_t n = 0;

  if (format) {
    sys_va_start(ap, format);
    n = pumpkin_vprintf(format, ap);
    sys_va_end(ap);
  }

  return n;
}

int32_t pumpkin_gets(char *buf, uint32_t max, int echo) {
  uint32_t i = 0;
  int c;

  if (buf && max > 0) {
    for (; i < max-1;) {
      c = pumpkin_getchar();
      if (c == -1 && i == 0) return -1;
      if (c <= 0) break;
      if (c == '\r') continue;
      if (c == '\n') {
        if (echo) pumpkin_putchar('\r');
        if (echo) pumpkin_putchar('\n');
        break;
      }
      if (c == '\b') {
        if (i > 0) {
          if (echo) pumpkin_putchar('\b');
          i--;
        }
        continue;
      }
      if (echo) pumpkin_putchar(c);
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

int pumpkin_audio_get(int *pcm, int *channels, int *rate) {
  int r = -1;

  if (mutex_lock(mutex) == 0) {
    if (pcm) *pcm = pumpkin_module.pcm;
    if (channels) *channels = pumpkin_module.channels;
    if (rate) *rate = pumpkin_module.rate;

    if (pumpkin_module.ap && pumpkin_module.ap->init) {
      if (pumpkin_module.audio == 0) {
        pumpkin_module.audio = pumpkin_module.ap->init(pumpkin_module.pcm, pumpkin_module.channels, pumpkin_module.rate);
      }
      r = pumpkin_module.audio;
    }

    mutex_unlock(mutex);
  }

  return r;
}

int pumpkin_audio_set(int pcm, int channels, int rate) {
  int r = -1;

  if (mutex_lock(mutex) == 0) {
    pumpkin_module.pcm = pcm;
    pumpkin_module.channels = channels;
    pumpkin_module.rate = rate;
    r = 0;
    mutex_unlock(mutex);
  }

  return r;
}

void pumpkin_audio_task_init(void) {
  pumpkin_task_t *task;

  if ((task = sys_calloc(1, sizeof(pumpkin_task_t))) != NULL) {
    thread_set(task_key, task);
    task->heap = heap_init(NULL, 256*1024, 0, NULL);
    StoInit(APP_STORAGE, pumpkin_module.fs_mutex);
    VFSInitModule();
    VFSAddVolume(VFS_CARD);
    SndInitModule(pumpkin_module.ap);
  }
}

void pumpkin_audio_task_finish(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);

  if (task) {
    VFSFinishModule();
    SndFinishModule();
    StoFinish();
    heap_finish(task->heap);
    sys_free(task);
  }
}

int32_t pumpkin_event_timeout(int32_t t) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  uint64_t now;
  uint16_t r;

  if (t == 0) {
    now = sys_get_clock();
    if (task->evtTimeoutLast > 0 && now - task->evtTimeoutLast > 1000000) {
      task->evtTimeoutCount = 0;
    }
    task->evtTimeoutLast = now;

    if (task->evtTimeoutCount >= 20) {
      if (!task->evtTimeoutWarning) {
        //r = FrmCustomAlert(ConfirmationOKCancelAlert, "App is calling EvtGetEvent(0) repeatedly (busy waiting). Click OK to continue execution or Cancel to terminate it.", "", "");
r = 0;
        task->evtTimeoutWarning = true;

        if (r == 1) {
          // Cancel button was clicked
          if (pumpkin_is_m68k()) {
            emupalmos_finish(1);
          } else {
            pumpkin_fatal_error(1);
          }
        }
      }
    }
    if (task->evtTimeoutCount < 100) task->evtTimeoutCount++;
  } else {
    task->evtTimeoutCount = 0;
  }

  return t;
}

void pumpkin_set_lasterr(Err err) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  if (task) task->lastErr = err;
}

Err pumpkin_get_lasterr(void) {
  pumpkin_task_t *task = (pumpkin_task_t *)thread_get(task_key);
  return task ? task->lastErr : 0;
}

static const char *memErrorMsg[] = {
  "memErrChunkLocked",
  "memErrNotEnoughSpace",
  "memErrInvalidParam",
  "memErrChunkNotLocked",
  "memErrCardNotPresent",
  "memErrNoCardHeader",
  "memErrInvalidStoreHeader",
  "memErrRAMOnlyCard",
  "memErrWriteProtect",
  "memErrNoRAMOnCard",
  "memErrNoStore",
  "memErrROMOnlyCard"
};

static const char *dmErrorMsg[] = {
  "dmErrMemError",
  "dmErrIndexOutOfRange",
  "dmErrInvalidParam",
  "dmErrReadOnly",
  "dmErrDatabaseOpen",
  "dmErrCantOpen",
  "dmErrCantFind",
  "dmErrRecordInWrongCard",
  "dmErrCorruptDatabase",
  "dmErrRecordDeleted",
  "dmErrRecordArchived",
  "dmErrNotRecordDB",
  "dmErrNotResourceDB",
  "dmErrROMBased",
  "dmErrRecordBusy",
  "dmErrResourceNotFound",
  "dmErrNoOpenDatabase",
  "dmErrInvalidCategory",
  "dmErrNotValidRecord",
  "dmErrWriteOutOfBounds",
  "dmErrSeekFailed",
  "dmErrAlreadyOpenForWrites",
  "dmErrOpenedByAnotherTask",
  "dmErrUniqueIDNotFound",
  "dmErrAlreadyExists",
  "dmErrInvalidDatabaseName",
  "dmErrDatabaseProtected",
  "dmErrDatabaseNotProtected"
};

static const char *sysErrorMsg[] = {
  "sysErrTimeout",
  "sysErrParamErr",
  "sysErrNoFreeResource",
  "sysErrNoFreeRAM",
  "sysErrNotAllowed",
  "sysErrSemInUse",
  "sysErrInvalidID",
  "sysErrOutOfOwnerIDs",
  "sysErrNoFreeLibSlots",
  "sysErrLibNotFound",
  "sysErrDelayWakened",
  "sysErrRomIncompatible",
  "sysErrBufTooSmall",
  "sysErrPrefNotFound"
};

static const char *vfsErrorMsg[] = {
  "vfsErrBufferOverflow",
  "vfsErrFileGeneric",
  "vfsErrFileBadRef",
  "vfsErrFileStillOpen",
  "vfsErrFilePermissionDenied",
  "vfsErrFileAlreadyExists",
  "vfsErrFileEOF",
  "vfsErrFileNotFound",
  "vfsErrVolumeBadRef",
  "vfsErrVolumeStillMounted",
  "vfsErrNoFileSystem",
  "vfsErrBadData",
  "vfsErrDirNotEmpty",
  "vfsErrBadName",
  "vfsErrVolumeFull",
  "vfsErrUnimplemented",
  "vfsErrNotADirectory",
  "vfsErrIsADirectory",
  "vfsErrDirectoryNotFound",
  "vfsErrNameShortened"
};

const char *pumpkin_error_msg(Err err) {
  const char *s;

  if (err > dmErrorClass && err <= dmErrDatabaseNotProtected) {
    s = dmErrorMsg[err - dmErrorClass - 1];
  } else if (err > sysErrorClass && err <= sysErrPrefNotFound) {
    s = sysErrorMsg[err - sysErrorClass - 1];
  } else if (err > memErrorClass && err <= memErrROMOnlyCard) {
    s = memErrorMsg[err - memErrorClass - 1];
  } else if (err > vfsErrorClass && err <= vfsErrNameShortened) {
    s = vfsErrorMsg[err - vfsErrorClass - 1];
  } else {
    s = "unknown";
  }

  return s;
}
