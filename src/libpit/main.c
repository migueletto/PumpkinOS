#include "sys.h"
#include "main.h"
#include "sig.h"
#include "script.h"
#include "vfs.h"
#include "ptr.h"
#include "thread.h"
#include "vfont.h"
#include "endianness.h"
#include "debug.h"

int pit_main(int argc, char *argv[], void (*callback)(int pe, void *data), void *data) {
  script_engine_t *engine;
  char *script_engine, *debugfile;
  int pe, background, dlevel, err, i;
  int script_argc, status;
  char **script_argv, *d, *s;

  script_engine = NULL;
  script_argc = 0;
  script_argv = NULL;
  background = 0;
  debugfile = NULL;
  err = 0;

  for (i = 1; i < argc && !err; i++) {
    if (argv[i][0] == '-') {
      if (i < argc-1) {
        switch (argv[i][1]) {
          case 'b':
            background = 1;
            break;
          case 'f':
            debugfile = argv[++i];
            break;
          case 'd':
            d = argv[++i];
            dlevel = sys_atoi(d);
            s = sys_strchr(d, ':');
            debug_setsyslevel(s ? s+1 : NULL, dlevel);
            break;
          case 's':
            if (script_engine == NULL) {
              script_engine = argv[++i];
            } else {
              err = 1;
            }
            break;
          case 't':
            debug_scope(1);
            break;
          default:
            err = 1;
        }
      } else {
        err = 1;
      }
    } else {
      script_argc = argc - i;
      script_argv = &argv[i];
      break;
    }
  }

  if (err || script_engine == NULL || script_argv == NULL) {
    return STATUS_ERROR;
  }

  sys_init();
  debug_init(debugfile);
  ptr_init();
  thread_init();

  debug(DEBUG_INFO, "MAIN", "%s starting on %s (%s endian)", SYSTEM_NAME, SYSTEM_OS, little_endian() ? "little" : "big");

  if (background && sys_daemonize() != 0) {
    debug_close();
    return STATUS_ERROR;
  }
  thread_setmain();

  sys_unblock_signals();
  signal_install_handlers();

  if ((engine = script_load_engine(script_engine)) == NULL) {
    debug(DEBUG_ERROR, "MAIN", "error exit");
    thread_close();
    debug_close();
    return STATUS_ERROR;
  }

  if (script_init(engine) == -1) {
    debug(DEBUG_ERROR, "MAIN", "error exit");
    script_finish(engine);
    thread_close();
    debug_close();
    return STATUS_ERROR;
  }

  if ((pe = script_create(engine)) == -1) {
    debug(DEBUG_ERROR, "MAIN", "error exit");
    script_finish(engine);
    thread_close();
    debug_close();
    return STATUS_ERROR;
  }

  vfs_init();
  vfont_init(pe);

  if (callback) {
    callback(pe, data);
  }

  if (script_run(pe, script_argv[0], script_argc-1, &script_argv[1], 0) == 0) {
    debug(DEBUG_INFO, "MAIN", "idle loop begin");
    script_idle_loop(pe);
    debug(DEBUG_INFO, "MAIN", "idle loop end");
  } else {
    debug(DEBUG_ERROR, "MAIN", "script_run failed");
    sys_set_finish(STATUS_ERROR);
  }

  sys_usleep(1000); // give threads a chance to start
  thread_wait_all();
  vfont_finish(pe);
  script_destroy(pe);
  script_finish(engine);
  vfs_finish();
  status = thread_get_status();
  thread_close();
  debug(DEBUG_INFO, "MAIN", "%s stopping", SYSTEM_NAME);
  debug_close();

  return status;
}
