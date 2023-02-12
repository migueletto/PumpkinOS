#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "script.h"
#include "thread.h"
#include "sys.h"
#include "timeutc.h"
#include "pit_io.h"
#include "vfs.h"
#include "vfslocal.h"
#include "xalloc.h"
#include "debug.h"

static int64_t clock0;

static int pit_sprintf(int pe) {
  char c, fmt[256], buf[1024], lfmt[16], *s;
  int ifmt, ibuf, iarg, ilfmt, escape;
  script_int_t integer;
  script_real_t d;

  iarg = 0;
  if (script_get_string(pe, iarg++, &s) == -1) return -1;
  xmemset(fmt, 0, sizeof(fmt));
  xmemcpy(fmt, s, sizeof(fmt)-1);
  xfree(s);
  xmemset(buf, 0, sizeof(buf));
  ilfmt = 0;

  for (ifmt = 0, ibuf = 0, escape = 0; fmt[ifmt]; ifmt++) {
    c = fmt[ifmt];

    if (escape) {
      switch (c) {
        case '%':
          if (ibuf < sizeof(buf)-1) buf[ibuf++] = c;
          escape = 0;
          break;
        case 'c':
          if (ilfmt < sizeof(lfmt)-1) {
            lfmt[ilfmt++] = c;
            lfmt[ilfmt++] = 0;
            if (script_get_integer(pe, iarg++, &integer) == -1) return -1;
            snprintf(buf+ibuf, sizeof(buf)-ibuf-1, lfmt, (char)integer);
            ibuf = strlen(buf);
          }
          escape = 0;
          break;
        case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
          if (ilfmt < sizeof(lfmt)-1) {
            lfmt[ilfmt++] = c;
            lfmt[ilfmt++] = 0;
            if (script_get_integer(pe, iarg++, &integer) == -1) return -1;
            snprintf(buf+ibuf, sizeof(buf)-ibuf-1, lfmt, integer);
            ibuf = strlen(buf);
          }
          escape = 0;
          break;
        case 'e': case 'f': case 'g':
        case 'E': case 'F': case 'G':
          if (ilfmt < sizeof(lfmt)-1) {
            lfmt[ilfmt++] = c;
            lfmt[ilfmt++] = 0;
            if (script_get_real(pe, iarg++, &d) == -1) return -1;
            snprintf(buf+ibuf, sizeof(buf)-ibuf-1, lfmt, d);
            ibuf = strlen(buf);
          }
          escape = 0;
          break;
        case 's':
          if (ilfmt < sizeof(lfmt)-1) {
            lfmt[ilfmt++] = c;
            lfmt[ilfmt++] = 0;
            if (script_get_string(pe, iarg++, &s) == -1) return -1;
            snprintf(buf+ibuf, sizeof(buf)-ibuf-1, lfmt, s);
            xfree(s);
            ibuf = strlen(buf);
          }
          escape = 0;
          break;
        case '-': case '+': case ' ': case '#': case '.':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 'l':
          if (ilfmt < sizeof(lfmt)-1) {
            lfmt[ilfmt++] = c;
          }
          break;
        default:
          escape = 0;
          break;
      }
    } else {
      if (c == '%') {
        escape = 1;
        ilfmt = 0;
        lfmt[ilfmt++] = c;
      } else {
        if (ibuf < sizeof(buf)-1) buf[ibuf++] = c;
      }
    }
  }

  return script_push_string(pe, buf);
}

PIT_LIB_FUNCTION(builtin,debug)
  PIT_LIB_PARAM_I(level)
  PIT_LIB_PARAM_S(text)
PIT_LIB_CODE
  debug(level, "RUN", "%s", text);
  r = 0;
  PIT_LIB_FREE_S(text);
PIT_LIB_END_B

PIT_LIB_FUNCTION(builtin,debugbytes)
  PIT_LIB_PARAM_I(level)
  PIT_LIB_PARAM_L(buf,blen)
PIT_LIB_CODE
  debug_bytes(level, "RUN", (unsigned char *)buf, blen);
  r = 0;
  PIT_LIB_FREE_S(buf);
PIT_LIB_END_B

PIT_LIB_FUNCTION(builtin,istrue)
  PIT_LIB_PARAM_B(cond)
PIT_LIB_CODE
  r = cond ? 0 : 1;
PIT_LIB_END_B

PIT_LIB_FUNCTION(builtin,loadlib)
  PIT_LIB_PARAM_S(libname)
  script_ref_t obj;
PIT_LIB_CODE
  obj = script_loadlib(PIT_LIB_PE, libname);
  r = 0;
PIT_LIB_END_O(obj)

PIT_LIB_FUNCTION(builtin,include)
  PIT_LIB_PARAM_S(filename)
PIT_LIB_CODE
  r = script_run(PIT_LIB_PE, filename, -1, NULL);
  PIT_LIB_FREE_S(filename);
PIT_LIB_END_B

PIT_LIB_FUNCTION(builtin,mount)
  PIT_LIB_PARAM_S(local)
  PIT_LIB_PARAM_S(path)
PIT_LIB_CODE
  r = vfs_local_mount(local, path);
  PIT_LIB_FREE_S(local);
  PIT_LIB_FREE_S(path);
PIT_LIB_END_B

PIT_LIB_FUNCTION(builtin,listdir)
  PIT_LIB_PARAM_S(path)
  PIT_LIB_PARAM_F(callback)
PIT_LIB_CODE
  script_arg_t ret;
  sys_dir_t *dir;
  char name[256];

  r = -1;
  if ((dir = sys_opendir(path)) != NULL) {
    for (;;) {
      if (sys_readdir(dir, name, sizeof(name)) != 0) break;
      if (!strcmp(name, ".") || !strcmp(name, "..")) continue;
      script_call(pe, callback, &ret, "S", name);
    }
    sys_closedir(dir);
    r = 0;
  }

  PIT_LIB_FREE_S(path);
  r = 0;
PIT_LIB_END_B

PIT_LIB_FUNCTION(builtin,sysname)
  char *name = SYSTEM_NAME;
PIT_LIB_CODE
  r = 0;
PIT_LIB_END_S(name)

PIT_LIB_FUNCTION(builtin,osname)
  char *name = SYSTEM_OS;
PIT_LIB_CODE
  r = 0;
PIT_LIB_END_S(name)

PIT_LIB_FUNCTION(builtin,getenv)
  PIT_LIB_PARAM_S(name)
  char *value;
PIT_LIB_CODE
  value = sys_getenv(name);
  r = 0;
  PIT_LIB_FREE_S(name);
PIT_LIB_END_S(value)

PIT_LIB_FUNCTION(builtin,clock)
  int64_t c;
PIT_LIB_CODE
  if (clock0 != -1) {
    c = sys_get_clock();
    if (c > clock0) {
      c -= clock0;
      r = 0;
    }
  }
PIT_LIB_END_I(c)

PIT_LIB_FUNCTION(builtin,time)
  time_t t = sys_time();
PIT_LIB_CODE
  r = 0;
PIT_LIB_END_I(t)

PIT_LIB_FUNCTION(builtin,proc_time)
  int64_t t = sys_get_process_time();
PIT_LIB_CODE
  r = 0;
PIT_LIB_END_R(t)

PIT_LIB_FUNCTION(builtin,thread_time)
  int64_t t = sys_get_thread_time();
PIT_LIB_CODE
  r = 0;
PIT_LIB_END_R(t)

PIT_LIB_FUNCTION(builtin,pid)
  script_int_t pid = sys_get_pid();
PIT_LIB_CODE
  r = 0;
PIT_LIB_END_I(pid)

PIT_LIB_FUNCTION(builtin,tid)
  script_int_t tid = sys_get_tid();
PIT_LIB_CODE
  r = 0;
PIT_LIB_END_I(tid)

PIT_LIB_FUNCTION(builtin,usleep)
  PIT_LIB_PARAM_I(t)
PIT_LIB_CODE
  if (t > 0) {
    sys_usleep(t);
    r = 0;
  }
PIT_LIB_END_B

PIT_LIB_FUNCTION(builtin,cleanup)
  PIT_LIB_PARAM_F(callback)
PIT_LIB_CODE
  r = script_set_cleanup(PIT_LIB_PE, callback);
PIT_LIB_END_B

PIT_LIB_FUNCTION(builtin,idle)
  PIT_LIB_PARAM_F(callback)
  PIT_LIB_PARAM_I(t)
PIT_LIB_CODE
  r = script_set_idle(PIT_LIB_PE, callback, t);
PIT_LIB_END_B

PIT_LIB_FUNCTION(builtin,finish)
  PIT_LIB_PARAM_I(code)
PIT_LIB_CODE
  sys_set_finish(code);
  r = 0;
PIT_LIB_END_B

PIT_LIB_FUNCTION(builtin,must_end)
  int must_end = thread_must_end();
PIT_LIB_CODE
  r = must_end;
PIT_LIB_END_B

PIT_LIB_BEGIN(builtin)
  PIT_LIB_EXPORT_F(debug);
  PIT_LIB_EXPORT_F(debugbytes);
  PIT_LIB_EXPORT_F(istrue);
  PIT_LIB_EXPORT_F(loadlib);
  PIT_LIB_EXPORT_F(include);
  PIT_LIB_EXPORT_F(mount);
  PIT_LIB_EXPORT_F(listdir);
  PIT_LIB_EXPORT_F(sysname);
  PIT_LIB_EXPORT_F(osname);
  PIT_LIB_EXPORT_F(getenv);
  PIT_LIB_EXPORT_F(clock);
  PIT_LIB_EXPORT_F(time);
  PIT_LIB_EXPORT_F(proc_time);
  PIT_LIB_EXPORT_F(thread_time);
  PIT_LIB_EXPORT_F(pid);
  PIT_LIB_EXPORT_F(tid);
  PIT_LIB_EXPORT_F(usleep);
  PIT_LIB_EXPORT_F(cleanup);
  PIT_LIB_EXPORT_F(idle);
  PIT_LIB_EXPORT_F(finish);
  PIT_LIB_EXPORT_F(must_end);
PIT_LIB_END

int script_create_builtins(int pe, script_ref_t obj) {
  clock0 = sys_get_clock();

  libbuiltin_init(pe, obj);
  script_add_function(pe, obj, "sprintf", pit_sprintf);
  script_add_sconst(pe, obj, "SEP", SFILE_SEP);

  return 0;
}
