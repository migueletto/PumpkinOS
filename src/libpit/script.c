#include "sys.h"
#include "script.h"
#include "ptr.h"
#include "thread.h"
#include "mutex.h"
#include "pit_io.h"
#include "timeutc.h"
#include "debug.h"
#include "xalloc.h"

#define TAG_ENV    "env"

#define MAX_MODULE_NAME  256
#define MAX_LIB_UNLOAD   256
#define MAX_FILE_NAME    256

struct script_engine_t {
  mutex_t *unload_mutex;
  int num_lib_unload;
  int (*lib_unload_f[MAX_LIB_UNLOAD])(void);

  int (*dl_ext_script_init)(void);
  script_priv_t *(*dl_ext_script_create)(void);
  uint32_t (*dl_ext_script_engine_id)(void);
  char *(*dl_ext_script_engine_ext)(void);
  int (*dl_ext_script_run)(script_priv_t *priv, char *filename, int argc, char *argv[], int str);
  int (*dl_ext_script_get_last_error)(script_priv_t *priv, char *buf, int max);
  int (*dl_ext_script_destroy)(script_priv_t *priv);
  int (*dl_ext_script_call)(script_priv_t *priv, script_ref_t ref, script_arg_t *ret, int n, script_arg_t *args);
  int (*dl_ext_script_get_value)(script_priv_t *priv, int i, int type, script_arg_t *arg);
  script_ref_t (*dl_ext_script_create_object)(script_priv_t *priv);
  int (*dl_ext_script_global_set)(script_priv_t *priv, char *name, script_arg_t *value);
  int (*dl_ext_script_global_get)(script_priv_t *priv, char *name, script_arg_t *value);
  int (*dl_ext_script_object_get)(script_priv_t *priv, script_ref_t obj, script_arg_t *key, script_arg_t *value);
  int (*dl_ext_script_object_set)(script_priv_t *priv, script_ref_t obj, script_arg_t *key, script_arg_t *value);
  script_ref_t (*dl_ext_script_create_function)(script_priv_t *priv, int pe, int (*f)(int pe));
  script_ref_t (*dl_ext_script_dup_ref)(script_priv_t *priv, script_ref_t ref);
  int (*dl_ext_script_remove_ref)(script_priv_t *priv, script_ref_t ref);
  int (*dl_ext_script_push_value)(script_priv_t *priv, script_arg_t *value);
  int (*dl_ext_script_get_stack)(script_priv_t *priv);
  int (*dl_ext_script_set_stack)(script_priv_t *priv, int index);
};

typedef struct {
  char *tag;
  script_engine_t *engine;
  script_priv_t *priv;
  script_ref_t cleanup;
  script_ref_t idle;
  uint32_t idle_period;
} script_env_t;

int script_init(script_engine_t *engine) {
  if (engine->dl_ext_script_init() == -1) {
    return -1;
  }

  engine->num_lib_unload = 0;
  sys_memset(engine->lib_unload_f, 0, sizeof(engine->lib_unload_f));

  if ((engine->unload_mutex = mutex_create("unload")) == NULL) {
    return -1;
  }

  return 0;
}

int script_finish(script_engine_t *engine) {
  int i;

  if (engine) {
    for (i = 0; i < engine->num_lib_unload; i++) {
      engine->lib_unload_f[i]();
    }

    if (engine->unload_mutex) {
      mutex_destroy(engine->unload_mutex);
    }

    xfree(engine);
  }

  return 0;
}

script_engine_t *script_get_engine(int pe) {
  script_engine_t *engine;
  script_env_t *env;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    engine = env->engine;
    ptr_unlock(pe, TAG_ENV);
  }

  return engine;
}

static void script_destructor(void *p) {
  script_env_t *env;

  env = (script_env_t *)p;

  if (env) {
    env->engine->dl_ext_script_destroy(env->priv);
    xfree(env);
  }
}

int script_create(script_engine_t *engine) {
  script_env_t *env;
  script_arg_t value;
  int pe;

  if ((env = xcalloc(1, sizeof(script_env_t))) == NULL) {
    return -1;
  }

  env->engine = engine;

  if ((env->priv = env->engine->dl_ext_script_create()) == NULL) {
    xfree(env);
    return -1;
  }

  env->tag = TAG_ENV;

  if ((pe = ptr_new(env, script_destructor)) == -1) {
    env->engine->dl_ext_script_destroy(env->priv);
    xfree(env);
    return -1;
  }

  value.type = SCRIPT_ARG_OBJECT;

  if ((value.value.r = script_create_object(pe)) == -1 ||
       script_global_set(pe, SYSTEM_NAME, &value) == -1) {
    ptr_free(pe, TAG_ENV);
    return -1;
  }

  if (script_create_builtins(pe, value.value.r) == -1) {
    ptr_free(pe, TAG_ENV);
    return -1;
  }

  return pe;
}

uint32_t script_engine_id(script_engine_t *engine) {
  return engine->dl_ext_script_engine_id();
}

char *script_engine_ext(script_engine_t *engine) {
  return engine->dl_ext_script_engine_ext();
}

int script_run(int pe, char *filename, int argc, char *argv[], int str) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r = env->engine->dl_ext_script_run(env->priv, filename, argc, argv, str);
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

int script_get_last_error(int pe, char *buf, int max) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r = env->engine->dl_ext_script_get_last_error(env->priv, buf, max);
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

int script_set_idle(int pe, script_ref_t ref, uint32_t t) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    debug(DEBUG_INFO, "SCRIPT", "idle period set to %u us", t);
    env->idle = ref;
    env->idle_period = t;
    ptr_unlock(pe, TAG_ENV);
    r = 0;
  }

  return r;
}

void script_idle_loop(int pe) {
  script_env_t *env;
  script_arg_t ret;
  script_ref_t ref = 0;
  unsigned char *buf;
  unsigned int len;
  uint32_t d = 100000;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    ref = env->idle;
    if (ref) d = env->idle_period;
    ptr_unlock(pe, TAG_ENV);
  }

  for (; !thread_must_end();) {
    if (ref) {
      if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
        env->engine->dl_ext_script_call(env->priv, ref, &ret, 0, NULL);
        ptr_unlock(pe, TAG_ENV);
      }
    }
    if (thread_server_read_timeout(d, &buf, &len) == -1) break;
    if (buf) xfree(buf);
  }
}

int script_destroy(int pe) {
  script_env_t *env;
  script_arg_t ret;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    if (env->cleanup) {
      debug(DEBUG_INFO, "SCRIPT", "calling cleanup function");
      env->engine->dl_ext_script_call(env->priv, env->cleanup, &ret, 0, NULL);
    }
    ptr_unlock(pe, TAG_ENV);
  }

  return ptr_free(pe, TAG_ENV);
}

int script_send(int handle, char *buf, int len) {
  return thread_client_write(handle, (unsigned char *)buf, len) == -1 ? -1 : 0;
}

int script_set_cleanup(int pe, script_ref_t ref) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    env->cleanup = ref;
    ptr_unlock(pe, TAG_ENV);
    r = 0;
  }

  return r;
}

static int script_dup_string(script_arg_t *arg) {
  char *s;
  int r = 0;

  if (arg->type == SCRIPT_ARG_STRING) {
    arg->value.s = xstrdup(arg->value.s);
    if (arg->value.s == NULL) r = -1;
  } else if (arg->type == SCRIPT_ARG_LSTRING) {
    s = arg->value.l.s;
    arg->value.l.s = xmalloc(arg->value.l.n);
    if (arg->value.l.s == NULL) {
      r = -1;
    } else {
      sys_memcpy(arg->value.l.s, s, arg->value.l.n);
    }
  }

  return r;
}

int script_call_args(int pe, script_ref_t ref, script_arg_t *ret, int n, script_arg_t *args) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r = env->engine->dl_ext_script_call(env->priv, ref, ret, n, args);
    if (r == 0) {
      r = script_dup_string(ret);
    }
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

int script_call(int pe, script_ref_t ref, script_arg_t *ret, char *types, ...) {
  sys_va_list ap;
  script_arg_t args[MAX_ARGC];
  script_lstring_t *lstr;
  int i, r = 0;

  sys_va_start(ap, types);

  for (i = 0; types[i] && i < MAX_ARGC; i++) {
    args[i].type = types[i];

    switch (types[i]) {
      case SCRIPT_ARG_INTEGER:
        args[i].value.i = sys_va_arg(ap, script_int_t);
        break;
      case SCRIPT_ARG_BOOLEAN:
        args[i].value.i = sys_va_arg(ap, int);
        break;
      case SCRIPT_ARG_REAL:
        args[i].value.d = sys_va_arg(ap, script_real_t);
        break;
      case SCRIPT_ARG_STRING:
        args[i].value.s = sys_va_arg(ap, char *);
        break;
      case SCRIPT_ARG_LSTRING:
        lstr = sys_va_arg(ap, script_lstring_t *);
        args[i].value.l.s = lstr->s;
        args[i].value.l.n = lstr->n;
        break;
      default:
        debug(DEBUG_ERROR, "SCRIPT", "invalid argument type '%c'", types[i]);
        r = -1;
    }
  }

  sys_va_end(ap);

  if (r == 0) {
    r = script_call_args(pe, ref, ret, i, args);
  }

  return r;
}

int script_get_named_value(int pe, int index, int type, char *lib, char *func, char *param, int opt, script_arg_t *arg) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r = env->engine->dl_ext_script_get_value(env->priv, index, type, arg);

    if (r == 0) {
      r = script_dup_string(arg);
    }

    ptr_unlock(pe, TAG_ENV);
  }

  if (r == -1 && !opt) {
    if (lib && func && param) {
      debug(DEBUG_ERROR, "SCRIPT", "param %s.%s(%s), index %d, type '%c' not found", lib, func, param, index, type);
    } else {
      debug(DEBUG_ERROR, "SCRIPT", "param index %d, type '%c' not found", index, type);
    }
  }

  return r;
}

int script_get_value(int pe, int index, int type, script_arg_t *arg) {
  return script_get_named_value(pe, index, type, NULL, NULL, NULL, 0, arg);
}

int script_get_boolean(int pe, int index, int *b) {
  script_arg_t arg;
  int r;

  r = script_get_named_value(pe, index, SCRIPT_ARG_BOOLEAN, NULL, NULL, NULL, 0, &arg);
  *b = r == 0 ? (arg.value.i ? 1 : 0) : 0;

  return r;
}

int script_opt_boolean(int pe, int index, int *b) {
  script_arg_t arg;
  int r;

  r = script_get_named_value(pe, index, SCRIPT_ARG_BOOLEAN, NULL, NULL, NULL, 1, &arg);
  *b = r == 0 ? (arg.value.i ? 1 : 0) : 0;

  return r;
}

int script_get_integer(int pe, int index, script_int_t *i) {
  script_arg_t arg;
  int r;

  r = script_get_named_value(pe, index, SCRIPT_ARG_INTEGER, NULL, NULL, NULL, 0, &arg);
  *i = r == 0 ? arg.value.i : 0;

  return r;
}

int script_opt_integer(int pe, int index, script_int_t *i) {
  script_arg_t arg;
  int r;

  r = script_get_named_value(pe, index, SCRIPT_ARG_INTEGER, NULL, NULL, NULL, 1, &arg);
  *i = r == 0 ? arg.value.i : 0;

  return r;
}

int script_get_real(int pe, int index, script_real_t *d) {
  script_arg_t arg;
  int r;

  r = script_get_named_value(pe, index, SCRIPT_ARG_REAL, NULL, NULL, NULL, 0, &arg);
  *d = r == 0 ? arg.value.d : 0;

  return r;
}

int script_get_string(int pe, int index, char **s) {
  script_arg_t arg;
  int r;

  r = script_get_named_value(pe, index, SCRIPT_ARG_STRING, NULL, NULL, NULL, 0, &arg);
  *s = r == 0 ? arg.value.s : NULL;

  return r;
}

int script_opt_string(int pe, int index, char **s) {
  script_arg_t arg;
  int r;

  r = script_get_named_value(pe, index, SCRIPT_ARG_STRING, NULL, NULL, NULL, 1, &arg);
  *s = r == 0 ? arg.value.s : NULL;

  return r;
}

int script_get_lstring(int pe, int index, char **s, int *len) {
  script_arg_t arg;
  int r;

  r = script_get_named_value(pe, index, SCRIPT_ARG_LSTRING, NULL, NULL, NULL, 0, &arg);
  if (r == 0) {
    *s = arg.value.l.s;
    *len = arg.value.l.n;
  } else {
    *s = NULL;
    *len = 0;
  }

  return r;
}

int script_get_function(int pe, int index, script_ref_t *ref) {
  script_arg_t arg;
  int r;

  r = script_get_named_value(pe, index, SCRIPT_ARG_FUNCTION, NULL, NULL, NULL, 0, &arg);
  *ref = r == 0 ? arg.value.r : 0;

  return r;
}

int script_get_object(int pe, int index, script_ref_t *ref) {
  script_arg_t arg;
  int r;

  r = script_get_named_value(pe, index, SCRIPT_ARG_OBJECT, NULL, NULL, NULL, 0, &arg);
  *ref = r == 0 ? arg.value.r : 0;

  return r;
}

int script_add_pointer(int pe, script_ref_t obj, char *name, void *p) {
  script_arg_t key, value;

  key.type = SCRIPT_ARG_STRING;
  key.value.s = name;
  value.type = SCRIPT_ARG_POINTER;
  value.value.p = p;

  return script_object_set(pe, obj, &key, &value);
}

int script_add_boolean(int pe, script_ref_t obj, char *name, int b) {
  script_arg_t key, value;

  key.type = SCRIPT_ARG_STRING;
  key.value.s = name;
  value.type = SCRIPT_ARG_BOOLEAN;
  value.value.i = b;

  return script_object_set(pe, obj, &key, &value);
}

int script_add_iconst(int pe, script_ref_t obj, char *name, script_int_t i) {
  script_arg_t key, value;

  key.type = SCRIPT_ARG_STRING;
  key.value.s = name;
  value.type = SCRIPT_ARG_INTEGER;
  value.value.i = i;

  return script_object_set(pe, obj, &key, &value);
}

int script_add_rconst(int pe, script_ref_t obj, char *name, script_real_t d) {
  script_arg_t key, value;

  key.type = SCRIPT_ARG_STRING;
  key.value.s = name;
  value.type = SCRIPT_ARG_REAL;
  value.value.d = d;

  return script_object_set(pe, obj, &key, &value);
}

int script_add_sconst(int pe, script_ref_t obj, char *name, char *s) {
  script_arg_t key, value;

  key.type = SCRIPT_ARG_STRING;
  key.value.s = name;
  value.type = SCRIPT_ARG_STRING;
  value.value.s = s;

  return script_object_set(pe, obj, &key, &value);
}

int script_add_function(int pe, script_ref_t obj, char *name, int (*f)(int pe)) {
  script_arg_t key, value;
  script_ref_t function;

  function = script_create_function(pe, f);

  key.type = SCRIPT_ARG_STRING;
  key.value.s = name;
  value.type = SCRIPT_ARG_FUNCTION;
  value.value.r = function;

  return script_object_set(pe, obj, &key, &value);
}

int script_set_pointer(int pe, char *name, void *p) {
  script_arg_t value;

  value.type = SCRIPT_ARG_POINTER;
  value.value.p = p;

  return script_global_set(pe, name, &value);
}

void *script_get_pointer(int pe, char *name) {
  script_arg_t value;
  void *p = NULL;

  if (script_global_get(pe, name, &value) == 0) {
    p = value.type == SCRIPT_ARG_POINTER ? value.value.p : NULL;
  }

  return p;
}

script_ref_t script_create_object(int pe) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r = env->engine->dl_ext_script_create_object(env->priv);
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

int script_global_set(int pe, char *name, script_arg_t *value) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r = env->engine->dl_ext_script_global_set(env->priv, name, value);
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

int script_global_get(int pe, char *name, script_arg_t *value) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r = env->engine->dl_ext_script_global_get(env->priv, name, value);
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

int script_object_get(int pe, script_ref_t obj, script_arg_t *key, script_arg_t *value) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r = env->engine->dl_ext_script_object_get(env->priv, obj, key, value);
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

int script_object_set(int pe, script_ref_t obj, script_arg_t *key, script_arg_t *value) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r = env->engine->dl_ext_script_object_set(env->priv, obj, key, value);
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

script_ref_t script_create_function(int pe, int (*f)(int pe)) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r =  env->engine->dl_ext_script_create_function(env->priv, pe, f);
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

script_ref_t script_dup_ref(int pe, script_ref_t ref) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r =  env->engine->dl_ext_script_dup_ref(env->priv, ref);
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

int script_remove_ref(int pe, script_ref_t ref) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r =  env->engine->dl_ext_script_remove_ref(env->priv, ref);
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

int script_get_stack(int pe) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r = env->engine->dl_ext_script_get_stack(env->priv);
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

int script_set_stack(int pe, int index) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r = env->engine->dl_ext_script_set_stack(env->priv, index);
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

int script_push_value(int pe, script_arg_t *value) {
  script_env_t *env;
  int r = -1;

  if ((env = ptr_lock(pe, TAG_ENV)) != NULL) {
    r = env->engine->dl_ext_script_push_value(env->priv, value);
    ptr_unlock(pe, TAG_ENV);
  }

  return r;
}

int script_push_boolean(int pe, int b) {
  script_arg_t ret;

  ret.type = SCRIPT_ARG_BOOLEAN;
  ret.value.i = b ? 1 : 0;

  return script_push_value(pe, &ret);
}

int script_push_integer(int pe, script_int_t i) {
  script_arg_t ret;

  ret.type = SCRIPT_ARG_INTEGER;
  ret.value.i = i;

  return script_push_value(pe, &ret);
}

int script_push_real(int pe, script_real_t d) {
  script_arg_t ret;

  ret.type = SCRIPT_ARG_REAL;
  ret.value.d = d;

  return script_push_value(pe, &ret);
}

int script_push_string(int pe, char *s) {
  script_arg_t ret;

  ret.type = SCRIPT_ARG_STRING;
  ret.value.s = s;

  return script_push_value(pe, &ret);
}

int script_push_lstring(int pe, char *s, int len) {
  script_arg_t ret;

  ret.type = SCRIPT_ARG_LSTRING;
  ret.value.l.s = s;
  ret.value.l.n = len;
    
  return script_push_value(pe, &ret);
}

int script_push_function(int pe, script_ref_t r) {
  script_arg_t ret;

  ret.type = SCRIPT_ARG_FUNCTION;
  ret.value.r = r;

  return script_push_value(pe, &ret);
}

int script_push_object(int pe, script_ref_t r) {
  script_arg_t ret;

  ret.type = SCRIPT_ARG_OBJECT;
  ret.value.r = r;

  return script_push_value(pe, &ret);
}

int script_push_null(int pe) {
  script_arg_t ret;

  ret.type = SCRIPT_ARG_NULL;
  ret.value.i = 0;

  return script_push_value(pe, &ret);
}

int script_returned_value(script_arg_t *ret) {
  int r = 0;

  switch (ret->type) {
    case SCRIPT_ARG_BOOLEAN:
      r = ret->value.i ? 1 : 0;
      break;
    case SCRIPT_ARG_INTEGER:
      r = ret->value.i;
      break;
  }

  return r;
}

script_ref_t script_loadlib(int pe, char *libname) {
  script_env_t *env;
  script_ref_t obj;
  int (*load_f)(void);
  int (*init_f)(int pe, script_ref_t ref);
  int (*unload_f)(void);
  void *lib;
  char module[MAX_MODULE_NAME];
  char lname[MAX_MODULE_NAME + 8];
  char iname[MAX_MODULE_NAME + 8];
  char uname[MAX_MODULE_NAME + 8];
  char libnameext[FILE_PATH];
  int i, len, idot, islash, first_load;
 
  if (!libname || !libname[0]) {
    debug(DEBUG_ERROR, "SCRIPT", "empty or null library name");
    return -1;
  }

  len = sys_strlen(libname);
  idot = islash = -1;
  for (i = len-1; i > 0; i--) {
    if (libname[i] == '.') {
      idot = i;
      break;
    }
  }
  for (; i > 0; i--) {
    if (libname[i] == FILE_SEP) {
      islash = i;
      break;
    }
  }

  sys_strncpy(libnameext, libname, FILE_PATH);
  if (idot == -1) {
    sys_strncat(libnameext, SOEXT, FILE_PATH - len - sys_strlen(SOEXT));
    idot = len;
  }
  len = idot - islash - 1;
  if (len >= MAX_MODULE_NAME-1) {
    debug(DEBUG_ERROR, "SCRIPT", "library name %s is too long", libname);
    return -1;
  }
  sys_memcpy(module, &libname[islash+1], len);
  module[len] = 0;
  debug(DEBUG_TRACE, "SCRIPT", "module name %s", module);

  if ((lib = sys_lib_load(libnameext, &first_load)) == NULL) {
    return -1;
  }

  sys_snprintf(lname, sizeof(lname)-1, "%s_load", module);
  sys_snprintf(iname, sizeof(iname)-1, "%s_init", module);
  sys_snprintf(uname, sizeof(uname)-1, "%s_unload", module);
  load_f = sys_lib_defsymbol(lib, lname, 0);
  init_f = sys_lib_defsymbol(lib, iname, 0);
  unload_f = sys_lib_defsymbol(lib, uname, 0);

  if (first_load && load_f && load_f() == -1) {
    return -1;
  }

  env = ptr_lock(pe, TAG_ENV);

  if (first_load && unload_f) {
    if (mutex_lock(env->engine->unload_mutex) == 0) {
      if (env->engine->num_lib_unload < MAX_LIB_UNLOAD) {
        debug(DEBUG_INFO, "SCRIPT", "registering finalizer for %s library", libname);
        env->engine->lib_unload_f[env->engine->num_lib_unload++] = unload_f;
      } else {
        debug(DEBUG_ERROR, "SCRIPT", "finalizer for %s library will not be called", libname);
      }
      mutex_unlock(env->engine->unload_mutex);
    }
  }

  ptr_unlock(pe, TAG_ENV);
  obj = -1;

  if (init_f) {
    if ((obj = script_create_object(pe)) == -1) {
      return -1;
    }
    script_add_pointer(pe, obj, "_lib", lib);

    if (init_f(pe, obj) == -1) {
      debug(DEBUG_ERROR, "SCRIPT", "failed to initialize %s library", libname);
      return -1;
    }
    debug(DEBUG_INFO, "SCRIPT", "%s library initialized", libname);

  } else {
    debug(DEBUG_INFO, "SCRIPT", "no initializer found in library %s", libname);
  }

  return obj;
}

#define ENGINE_SYMBOL(sym) engine->dl_##sym = sys_lib_defsymbol(lib, #sym, 1); err += ((engine->dl_##sym) == NULL) ? 1 : 0

script_engine_t *script_load_engine(char *libname) {
  script_engine_t *engine;
  void *lib;
  int first_load, err;

  if ((engine = xcalloc(1, sizeof(script_engine_t))) != NULL) {
    if ((lib = sys_lib_load(libname, &first_load)) != NULL) {
      err = 0;
      ENGINE_SYMBOL(ext_script_init);
      ENGINE_SYMBOL(ext_script_engine_id);
      ENGINE_SYMBOL(ext_script_engine_ext);
      ENGINE_SYMBOL(ext_script_create);
      ENGINE_SYMBOL(ext_script_run);
      ENGINE_SYMBOL(ext_script_get_last_error);
      ENGINE_SYMBOL(ext_script_destroy);
      ENGINE_SYMBOL(ext_script_call);
      ENGINE_SYMBOL(ext_script_get_value);
      ENGINE_SYMBOL(ext_script_create_object);
      ENGINE_SYMBOL(ext_script_global_get);
      ENGINE_SYMBOL(ext_script_global_set);
      ENGINE_SYMBOL(ext_script_object_get);
      ENGINE_SYMBOL(ext_script_object_set);
      ENGINE_SYMBOL(ext_script_create_function);
      ENGINE_SYMBOL(ext_script_dup_ref);
      ENGINE_SYMBOL(ext_script_remove_ref);
      ENGINE_SYMBOL(ext_script_push_value);
      ENGINE_SYMBOL(ext_script_get_stack);
      ENGINE_SYMBOL(ext_script_set_stack);
    }

    if (err) {
      xfree(engine);
      engine = NULL;
    }
  }

  return engine;
}
