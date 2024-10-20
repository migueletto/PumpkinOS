#include <PalmOS.h>

#include "sys.h"
#include "script.h"
#include "thread.h"
#include "media.h"
#include "pumpkin.h"
#include "secure.h"
#include "debug.h"

#define MAX_STR 64

typedef struct {
  int width, height, depth, mono, xfactor, yfactor, rotate;
  int software, fullscreen, fullrefresh, dia, single;
  char launcher[MAX_STR];
  char driver[MAX_STR];
  window_provider_t *wp;
  audio_provider_t *ap;
  window_t *w;
  texture_t *t;
  secure_provider_t *secure;
} libos_t;

static void EventLoop(libos_t *data) {
  client_request_t *creq;
  launch_request_t request;
  SysNotifyParamType notify;
  unsigned char *buf;
  unsigned int len;
  int r;

  debug(DEBUG_INFO, PUMPKINOS, "starting launcher");
  sys_memset(&request, 0, sizeof(request));
  sys_strncpy(request.name, data->launcher, dmDBNameLength-1);
  request.code = sysAppLaunchCmdNormalLaunch;
  request.opendb = 1;
  pumpkin_launch(&request);

  for (; !thread_get_flags(FLAG_FINISH);) {
    if ((r = thread_server_read_timeout(1000, &buf, &len)) == -1) break;
    if (r == 1) {
      if (buf) {
        if (len == sizeof(client_request_t)) {
          creq = (client_request_t *)buf;
          switch (creq->type) {
            case MSG_LAUNCH:
              debug(DEBUG_INFO, PUMPKINOS, "received launch message");
              pumpkin_local_refresh();
              pumpkin_launch(&creq->data.launch);
              break;
            case MSG_DEPLOY:
              debug(DEBUG_INFO, PUMPKINOS, "received deploy message");
              MemSet(&notify, sizeof(notify), 0);
              notify.notifyType = sysNotifySyncFinishEvent;
              notify.broadcaster = sysNotifyBroadcasterCode;
              SysNotifyBroadcast(&notify);
              break;
            default:
              debug(DEBUG_ERROR, PUMPKINOS, "invalid client request type %d", creq->type);
              break;
          }
        } else {
          debug(DEBUG_ERROR, PUMPKINOS, "invalid client request size %d", len);
        }
        sys_free(buf);
      }
    }

    if (pumpkin_sys_event() == -1) break;
  }
}

static int libos_action(void *arg) {
  libos_t *data;
  int encoding, height;

  data = (libos_t *)arg;

  if (data->wp) {
    debug(DEBUG_INFO, PUMPKINOS, "creating window");

    encoding = data->depth == 16 ? ENC_RGB565 : ENC_RGBA;
    height = data->dia ? ((data->height - BUTTONS_HEIGHT) * 2) / 3 : data->height;
    if ((data->w = data->wp->create(encoding, &data->width, &data->height, data->xfactor, data->yfactor, data->rotate,
          data->fullscreen, data->software, data->driver, data->wp->data)) == NULL) {
      thread_end(PUMPKINOS, thread_get_handle());
      sys_free(data);
      return 0;
    }
    pumpkin_set_window(data->w, data->width, height, data->height);
    if (data->wp->title) {
      data->wp->title(data->w, data->single ? data->launcher : PUMPKINOS);
    }
    if (data->dia) {
      //dbg_init(data->wp, encoding);
    }
  }

  debug(DEBUG_INFO, PUMPKINOS, "deploying applications");
  pumpkin_deploy_files("/app_install");
  pumpkin_load_plugins();

  pumpkin_set_secure(data->secure);
  pumpkin_set_mono(data->mono);

  if (data->dia) {
    pumpkin_set_dia(data->depth);
  } else if (data->single) {
    pumpkin_set_single(data->depth);
  } else {
    pumpkin_set_depth(data->depth);
  }

  pumpkin_set_spawner(thread_get_handle());
  pumpkin_set_fullrefresh(data->fullrefresh);

  if (data->dia || data->single) {
    debug(DEBUG_INFO, PUMPKINOS, "starting \"%s\"", data->launcher);
    pumpkin_launcher(data->launcher, data->width, height);
  } else {
    debug(DEBUG_INFO, PUMPKINOS, "event loop begin");
    EventLoop(data);
    debug(DEBUG_INFO, PUMPKINOS, "event loop end");
  }
  //dbg_finish();

  sys_free(data);
  thread_end(PUMPKINOS, thread_get_handle());
  sys_set_finish(0);

  return 0;
}

typedef enum {
  PARAM_WIDTH = 1, PARAM_HEIGHT, PARAM_DEPTH, PARAM_XFACTOR, PARAM_YFACTOR, PARAM_ROTATE,
  PARAM_FULLSCREEN, PARAM_DIA, PARAM_SINGLE, PARAM_SOFTWARE, PARAM_FULLREFRESH,
  PARAM_DRIVER, PARAM_LAUNCHER
} param_id_t;

typedef struct {
  param_id_t id;
  int type;
  char *name;
} param_t;

static param_t params[] = {
  { PARAM_WIDTH,       SCRIPT_ARG_INTEGER, "width"       },
  { PARAM_HEIGHT,      SCRIPT_ARG_INTEGER, "height"      },
  { PARAM_DEPTH,       SCRIPT_ARG_INTEGER, "depth"       },
  { PARAM_XFACTOR,     SCRIPT_ARG_INTEGER, "xfactor"     },
  { PARAM_YFACTOR,     SCRIPT_ARG_INTEGER, "yfactor"     },
  { PARAM_ROTATE,      SCRIPT_ARG_INTEGER, "rotate"      },
  { PARAM_FULLSCREEN,  SCRIPT_ARG_BOOLEAN, "fullscreen"  },
  { PARAM_DIA,         SCRIPT_ARG_BOOLEAN, "dia"         },
  { PARAM_SINGLE,      SCRIPT_ARG_BOOLEAN, "single"      },
  { PARAM_SOFTWARE,    SCRIPT_ARG_BOOLEAN, "software"    },
  { PARAM_FULLREFRESH, SCRIPT_ARG_BOOLEAN, "fullrefresh" },
  { PARAM_DRIVER,      SCRIPT_ARG_LSTRING, "driver"      },
  { PARAM_LAUNCHER,    SCRIPT_ARG_LSTRING, "launcher"    },
  { 0, 0, NULL }
};

static int libos_start(int pe) {
  script_int_t width, height, depth;
  script_arg_t k, v;
  script_ref_t obj;
  libos_t *data;
  char *launcher = NULL;
  int fullscreen, dia, single;
  int i, r = -1;

  if ((data = sys_calloc(1, sizeof(libos_t))) != NULL) {
    data->wp = script_get_pointer(pe, WINDOW_PROVIDER);
    data->secure = script_get_pointer(pe, SECURE_PROVIDER);

    if (script_get_object(pe, 0, &obj) == 0) {
      // using new style parameters

      // set some default values
      data->width = 1024;
      data->height = 680;
      data->depth = 16;
      data->xfactor = 1;
      data->yfactor = 1;
      sys_strncpy(data->launcher, "Launcher", MAX_STR);

      for (i = 0; params[i].id; i++) {
        k.type = SCRIPT_ARG_STRING;
        k.value.s = params[i].name;

        if (script_object_get(pe, obj, &k, &v) == 0) {
          if (v.type == params[i].type) {
            switch (params[i].id) {
              case PARAM_WIDTH:       data->width       = v.value.i; break;
              case PARAM_HEIGHT:      data->height      = v.value.i; break;
              case PARAM_DEPTH:       data->depth       = v.value.i; break;
              case PARAM_XFACTOR:     data->xfactor     = v.value.i; break;
              case PARAM_YFACTOR:     data->yfactor     = v.value.i; break;
              case PARAM_ROTATE:      data->rotate      = v.value.i; break;
              case PARAM_FULLSCREEN:  data->fullscreen  = v.value.i; break;
              case PARAM_DIA:         data->dia         = v.value.i; break;
              case PARAM_SINGLE:      data->single      = v.value.i; break;
              case PARAM_SOFTWARE:    data->software    = v.value.i; break;
              case PARAM_FULLREFRESH: data->fullrefresh = v.value.i; break;
              case PARAM_DRIVER:
                sys_strncpy(data->driver, v.value.l.s, v.value.l.n < MAX_STR ? v.value.l.n : MAX_STR);
                break;
              case PARAM_LAUNCHER:
                sys_strncpy(data->launcher, v.value.l.s, v.value.l.n < MAX_STR ? v.value.l.n : MAX_STR);
                break;
            }
          } else if (v.type != SCRIPT_ARG_NULL) {
            debug(DEBUG_ERROR, PUMPKINOS, "invalid parameter type '%c' for %s", v.type, params[i].name);
          }
        }
      }

    } else if (script_get_integer(pe, 0, &width) == 0 &&
               script_get_integer(pe, 1, &height) == 0 &&
               script_get_integer(pe, 2, &depth) == 0 &&
               script_get_boolean(pe, 3, &fullscreen) == 0 &&
               script_get_boolean(pe, 4, &dia) == 0 &&
               script_get_boolean(pe, 5, &single) == 0 &&
               script_get_string(pe,  6, &launcher) == 0) {

      // using old style parameters
      data->width = width;
      data->height = height;
      data->depth = depth;
      data->fullscreen = fullscreen;
      data->dia = dia;
      data->single = single;
      sys_strncpy(data->launcher, launcher, MAX_STR);
    }

    if (data->dia) {
      data->width = pumpkin_default_density() == kDensityDouble ?  APP_SCREEN_WIDTH : APP_SCREEN_WIDTH / 2;
      data->height = (data->width * 3 ) / 2 + BUTTONS_HEIGHT;
    } else if (data->single) {
      data->width = pumpkin_default_density() == kDensityDouble ?  APP_SCREEN_WIDTH : APP_SCREEN_WIDTH / 2;
      data->height = data->width;
    }

    if (data->depth < 16) {
      data->mono = data->depth;
      data->depth = 16;
    } else {
      data->mono = 0;
    }

    if (data->width > 0 && data->height > 0 && (data->depth == 16 || data->depth == 32)) {
      if (thread_needs_run()) {
        r = thread_begin("LIBOS", libos_action, data);
        thread_run();
      } else {
        // Calling in the same thread. As a result, the script engine will remain locked.
        // This could be a problem only if an application calls the engine, which is unlikely.
        r = libos_action(data);
      }
    }
  }

  return r;
}

static int libos_app_init(int pe) {
  window_provider_t *wp;
  audio_provider_t *ap;
  bt_provider_t *bt;
  gps_parse_line_f gps_parse_line;

  wp = script_get_pointer(pe, WINDOW_PROVIDER);
  ap = script_get_pointer(pe, AUDIO_PROVIDER);
  bt = script_get_pointer(pe, BT_PROVIDER);
  gps_parse_line = script_get_pointer(pe, GPS_PARSE_LINE_PROVIDER);

  return script_push_boolean(pe, pumpkin_global_init(script_get_engine(pe), wp, ap, bt, gps_parse_line) == 0);
}

static int libos_app_finish(int pe) {
  return script_push_boolean(pe, pumpkin_global_finish() == 0);
}

static int libos_serial(int pe) {
  script_int_t port;
  char *descr = NULL;
  char *screator = NULL;
  char *host = NULL;
  UInt32 creator;
  int len, r = -1;

  if (script_get_string(pe,  0, &descr) == 0 &&
      script_get_lstring(pe, 1, &screator, &len) == 0 &&
      script_get_string(pe,  2, &host) == 0 &&
      script_get_integer(pe, 3, &port) == 0) {

    if (len == 4) {
      pumpkin_s2id(&creator, screator);
      r = pumpkin_add_serial(descr, creator, host, port);
      r = script_push_boolean(pe, r == 0);
    } else {
      debug(DEBUG_ERROR, PUMPKINOS, "invalid creator for serial");
    }
  }

  if (host) sys_free(host);
  if (screator) sys_free(screator);
  if (descr) sys_free(descr);

  return r;
}

int libos_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, PUMPKINOS, "libos_init");

  script_add_function(pe, obj, "init",   libos_app_init);
  script_add_function(pe, obj, "finish", libos_app_finish);
  script_add_function(pe, obj, "serial", libos_serial);
  script_add_function(pe, obj, "start",  libos_start);

  return 0;
}
