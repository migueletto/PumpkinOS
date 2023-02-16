#include <PalmOS.h>

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include "script.h"
#include "thread.h"
#include "mutex.h"
#include "media.h"
#include "sys.h"
#include "pumpkin.h"
#include "secure.h"
#include "dbg.h"
#include "debug.h"
#include "xalloc.h"

typedef struct {
  int width, height, depth, fullscreen, dia, single;
  char launcher[256];
  window_provider_t *wp;
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

  debug(DEBUG_INFO, OSNAME, "starting launcher");
  xmemset(&request, 0, sizeof(request));
  strncpy(request.name, data->launcher, dmDBNameLength-1);
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
              debug(DEBUG_INFO, OSNAME, "received launch message");
              pumpkin_local_refresh();
              pumpkin_launch(&creq->data.launch);
              break;
            case MSG_DEPLOY:
              debug(DEBUG_INFO, OSNAME, "received deploy message");
              MemSet(&notify, sizeof(notify), 0);
              notify.notifyType = sysNotifySyncFinishEvent;
              notify.broadcaster = sysNotifyBroadcasterCode;
              SysNotifyBroadcast(&notify);
              break;
            default:
              debug(DEBUG_ERROR, OSNAME, "invalid client request type %d", creq->type);
              break;
          }
        } else {
          debug(DEBUG_ERROR, OSNAME, "invalid client request size %d", len);
        }
        xfree(buf);
      }
    }

    if (pumpkin_sys_event() == -1) break;
  }
}

static int libos_action(void *arg) {
  libos_t *data;
  int encoding, height;

  data = (libos_t *)arg;
  height = data->dia ? (data->height * 2) / 3 : data->height;

  if (data->wp) {
    debug(DEBUG_INFO, OSNAME, "creating window");
    encoding = data->depth == 16 ? ENC_RGB565 : ENC_RGBA;
    debug(1, "XXX", "wp=%p", data->wp);
    debug(1, "XXX", "wp->create=%p", data->wp ? data->wp->create : NULL);
    debug(1, "XXX", "encoding=%d width=%d height=%d full=%d", encoding, data->width, data->height, data->fullscreen);
    if ((data->w = data->wp->create(encoding, &data->width, &data->height, 1, 1, 0, data->fullscreen, 0, data->wp->data)) == NULL) {
    debug(1, "XXX", "create failed");
      thread_end(OSNAME, thread_get_handle());
      xfree(data);
      return 0;
    }
    debug(1, "XXX", "create ok");
    pumpkin_set_window(data->w, data->width, height);
    debug(1, "XXX", "set_windows ok");
    if (data->wp->title) {
      data->wp->title(data->w, data->single ? data->launcher : OSNAME);
    }
    if (data->dia) {
      //dbg_init(data->wp, encoding);
    }
  }

  pumpkin_set_secure(data->secure);

  if (data->dia) {
    pumpkin_set_dia(data->depth);
  } else if (data->single) {
    pumpkin_set_single(data->depth);
  } else {
    pumpkin_set_background(data->depth, 0x58, 0x8C, 0xB4);
    pumpkin_set_border(data->depth, 4, 0xA0, 0xC0, 0xFF, 0xA0, 0xA0, 0xA0);
  }

  pumpkin_set_spawner(thread_get_handle());

  debug(DEBUG_INFO, OSNAME, "deploying applications");
  pumpkin_deploy_files("/app_install");
  pumpkin_load_plugins();

  if (data->dia || data->single) {
    debug(DEBUG_INFO, OSNAME, "starting \"%s\"", data->launcher);
    pumpkin_launcher(data->launcher, data->width, height);
  } else {
    debug(DEBUG_INFO, OSNAME, "event loop begin");
    EventLoop(data);
    debug(DEBUG_INFO, OSNAME, "event loop end");
  }
  //dbg_finish();

  xfree(data);
  thread_end(OSNAME, thread_get_handle());
  sys_set_finish(0);

  return 0;
}

static int libos_start_direct(window_provider_t *wp, secure_provider_t *secure, int width, int height, int depth, int fullscreen, int dia, int single, char *launcher) {
  libos_t *data;
  int r = -1;

  if (dia) {
    width = pumpkin_default_density() == kDensityDouble ?  APP_SCREEN_WIDTH : APP_SCREEN_WIDTH / 2;
    height = (width * 3 ) / 2;
  } else if (single) {
    width = pumpkin_default_density() == kDensityDouble ?  APP_SCREEN_WIDTH : APP_SCREEN_WIDTH / 2;
    height = width;
  }

  if (width > 0 && height > 0 && (depth == 16 || depth == 32)) {
    if ((data = xcalloc(1, sizeof(libos_t))) != NULL) {
      data->wp = wp;
      data->secure = secure;
      data->width = width;
      data->height = height;
      data->depth = depth;
      data->fullscreen = fullscreen;
      data->dia = dia;
      data->single = single;
      strncpy(data->launcher, launcher, 256);

      // Calling in the same thread. As a result, the script engine will remain locked.
      // This could be a problem only if an application calls the engine, which is unlikely.
      libos_action(data);
    }
  }

  return r;
}

static int libos_start(int pe) {
  char *launcher = NULL;
  script_int_t width, height, depth;
  secure_provider_t *secure;
  window_provider_t *wp;
  int fullscreen, dia, single, r = -1;

  if (script_get_integer(pe, 0, &width) == 0 &&
      script_get_integer(pe, 1, &height) == 0 &&
      script_get_integer(pe, 2, &depth) == 0 &&
      script_get_boolean(pe, 3, &fullscreen) == 0 &&
      script_get_boolean(pe, 4, &dia) == 0 &&
      script_get_boolean(pe, 5, &single) == 0 &&
      script_get_string(pe,  6, &launcher) == 0) {

    wp = script_get_pointer(pe, WINDOW_PROVIDER);
    secure = script_get_pointer(pe, SECURE_PROVIDER);
    r = libos_start_direct(wp, secure, width, height, depth, fullscreen, dia, single, launcher);
  }

  if (launcher) xfree(launcher);

  return r;
}

static int libos_app_init(int pe) {
  window_provider_t *wp;
  bt_provider_t *bt;
  gps_parse_line_f gps_parse_line;

  wp = script_get_pointer(pe, WINDOW_PROVIDER);
  bt = script_get_pointer(pe, BT_PROVIDER);
  gps_parse_line = script_get_pointer(pe, GPS_PARSE_LINE_PROVIDER);

  return script_push_boolean(pe, pumpkin_global_init(wp, bt, gps_parse_line) == 0);
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
      debug(DEBUG_ERROR, OSNAME, "invalid creator for serial");
    }
  }

  if (host) xfree(host);
  if (screator) xfree(screator);
  if (descr) xfree(descr);

  return r;
}

int libos_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, OSNAME, "libos_init");

  script_add_function(pe, obj, "init",   libos_app_init);
  script_add_function(pe, obj, "finish", libos_app_finish);
  script_add_function(pe, obj, "serial", libos_serial);
  script_add_function(pe, obj, "start",  libos_start);

  return 0;
}
