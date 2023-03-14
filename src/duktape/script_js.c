#include "duktape.h"
#include "duk_v1_compat.h"

#include "sys.h"
#include "script.h"
#include "thread.h"
#include "xalloc.h"
#include "debug.h"

#define LAST_ERROR "last_error"

struct script_priv_t {
  duk_context *ctx;
  script_ref_t iref;
  char *ref_mask;
};

static script_ref_t new_ref(script_priv_t *priv, int index);
static void get_ref(script_priv_t *priv, script_ref_t ref);

static int push_value(script_priv_t *priv, script_arg_t *arg) {
  switch(arg->type) {
    case SCRIPT_ARG_BOOLEAN:
      duk_push_boolean(priv->ctx, arg->value.i ? 1 : 0);
      break;

    case SCRIPT_ARG_INTEGER:
      duk_push_int(priv->ctx, (duk_int_t)arg->value.i);
      break;

    case SCRIPT_ARG_REAL:
      duk_push_number(priv->ctx, arg->value.d);
      break;

    case SCRIPT_ARG_STRING:
      duk_push_string(priv->ctx, arg->value.s);
      break;

    case SCRIPT_ARG_LSTRING:
      duk_push_lstring(priv->ctx, arg->value.l.s, arg->value.l.n);
      break;

    case SCRIPT_ARG_NULL:
      duk_push_null(priv->ctx);
      break;

    case SCRIPT_ARG_OBJECT:
      get_ref(priv, arg->value.r);
      break;

    case SCRIPT_ARG_FUNCTION:
      get_ref(priv, arg->value.r);
      break;

    case SCRIPT_ARG_POINTER:
      duk_push_pointer(priv->ctx, arg->value.p);
      break;

    default:
      debug(DEBUG_ERROR, "JS", "invalid pushed value type '%c'", arg->type);
      return -1;
  }

  return 1;
}

int ext_script_get_value(script_priv_t *priv, int i, int type, script_arg_t *arg) {
  duk_double_t number;
  duk_int_t integer;
  duk_size_t len;
  char *s;

  if (i >= duk_get_top(priv->ctx)) {
    return -1;
  }

  switch (type) {
    case SCRIPT_ARG_ANY:
      if (duk_is_null_or_undefined(priv->ctx, i)) {
        arg->type = SCRIPT_ARG_NULL;
        arg->value.i = 0;
      } else if (duk_is_boolean(priv->ctx, i)) {
        arg->type = SCRIPT_ARG_BOOLEAN;
        arg->value.i = duk_to_boolean(priv->ctx, i);
      } else if (duk_is_pointer(priv->ctx, i)) {
        arg->type = SCRIPT_ARG_POINTER;
        arg->value.p = duk_to_pointer(priv->ctx, i);
      } else if (duk_is_function(priv->ctx, i)) {
        arg->type = SCRIPT_ARG_FUNCTION;
        arg->value.r = new_ref(priv, i);
      } else if (duk_is_object(priv->ctx, i)) {
        arg->type = SCRIPT_ARG_OBJECT;
        arg->value.r = new_ref(priv, i);
      } else if (duk_is_number(priv->ctx, i)) {
        number = duk_to_number(priv->ctx, i);
        integer = (duk_int_t)number;
        if (number == integer) {
          arg->type = SCRIPT_ARG_INTEGER;
          arg->value.i = integer;
        } else {
          arg->type = SCRIPT_ARG_REAL;
          arg->value.d = number;
        }
      } else if (duk_is_string(priv->ctx, i)) {
        s = (char *)duk_to_lstring(priv->ctx, i, &len);
        if (s) {
          arg->type = SCRIPT_ARG_LSTRING;
          arg->value.l.s = s;
          arg->value.l.n = len;
        } else {
          return -1;
        }
      } else {
        return -1;
      }
      break;
    case SCRIPT_ARG_BOOLEAN:
      if (duk_is_string(priv->ctx, i)) {
        s = (char *)duk_to_string(priv->ctx, i);
        if (s && !strcmp(s, "true")) {
          arg->type = SCRIPT_ARG_BOOLEAN;
          arg->value.i = 1;
        } else {
          arg->type = SCRIPT_ARG_BOOLEAN;
          arg->value.i = duk_to_int(priv->ctx, i) ? 1 : 0;
        }
      } else {
        arg->type = SCRIPT_ARG_BOOLEAN;
        arg->value.i = duk_to_boolean(priv->ctx, i);
      }
      break;
    case SCRIPT_ARG_INTEGER:
      arg->type = SCRIPT_ARG_INTEGER;
      arg->value.i = duk_to_int(priv->ctx, i);
      break;
    case SCRIPT_ARG_REAL:
      arg->type = SCRIPT_ARG_REAL;
      arg->value.d = duk_to_number(priv->ctx, i);
      break;
    case SCRIPT_ARG_STRING:
      if (duk_is_null_or_undefined(priv->ctx, i)) {
        arg->type = SCRIPT_ARG_STRING;
        arg->value.s = NULL;
      } else if (duk_is_boolean(priv->ctx, i)) {
        arg->type = SCRIPT_ARG_STRING;
        arg->value.s = duk_to_boolean(priv->ctx, i) ? "true" : "false";
      } else {
        s = (char *)duk_to_string(priv->ctx, i);
        arg->type = SCRIPT_ARG_STRING;
        arg->value.s = s;
      }
      break;
    case SCRIPT_ARG_LSTRING:
      if (duk_is_null_or_undefined(priv->ctx, i)) {
        arg->type = SCRIPT_ARG_LSTRING;
        arg->value.l.s = NULL;
        arg->value.l.n = 0;
      } else if (duk_is_boolean(priv->ctx, i)) {
        s = duk_to_boolean(priv->ctx, i) ? "true" : "false";
        arg->type = SCRIPT_ARG_LSTRING;
        arg->value.l.s = s;
        arg->value.l.n = strlen(s);
      } else {
        s = (char *)duk_to_lstring(priv->ctx, i, &len);
        arg->type = SCRIPT_ARG_LSTRING;
        arg->value.l.s = s;
        arg->value.l.n = len;
      }
      break;
    case SCRIPT_ARG_FUNCTION:
      if (duk_is_function(priv->ctx, i)) {
        arg->type = SCRIPT_ARG_FUNCTION;
        arg->value.r = new_ref(priv, i);
      } else {
        debug(DEBUG_ERROR, "JS", "invalid argument type");
        return -1;
      }
      break;
    case SCRIPT_ARG_POINTER:
      if (duk_is_null_or_undefined(priv->ctx, i)) {
        arg->type = SCRIPT_ARG_POINTER;
        arg->value.p = 0;
      } else if (duk_is_pointer(priv->ctx, i)) {
        arg->type = SCRIPT_ARG_POINTER;
        arg->value.p = duk_to_pointer(priv->ctx, i);
      } else {
        debug(DEBUG_ERROR, "JS", "invalid argument type");
        return -1;
      }
      break;
    case SCRIPT_ARG_OBJECT:
      if (duk_is_null_or_undefined(priv->ctx, i)) {
        arg->type = SCRIPT_ARG_OBJECT;
        arg->value.r = 0;
      } else if (duk_is_object(priv->ctx, i)) {
        arg->type = SCRIPT_ARG_OBJECT;
        arg->value.r = new_ref(priv, i);
      } else {
        debug(DEBUG_ERROR, "JS", "invalid argument type");
        return -1;
      }
  }

  return 0;
}

static void set_last_error(script_priv_t *priv, char *msg) {
  script_arg_t value;

  value.type = SCRIPT_ARG_STRING;
  value.value.s = msg;
  ext_script_global_set(priv, LAST_ERROR, &value);
}

int ext_script_get_last_error(script_priv_t *priv, char *buf, int max) {
  script_arg_t value;
  int n, r = -1;

  if (ext_script_global_get(priv, LAST_ERROR, &value) == 0) {
    if (value.type == SCRIPT_ARG_LSTRING) {
      n = value.value.l.n < max ? value.value.l.n : max;
      if (n > 0) {
        sys_strncpy(buf, value.value.l.s, n-1);
      } else {
        buf[0] = 0;
      }
    }
    r = 0;
  }

  return r;
}

static void fatal_error(void *udata, const char *msg) {
  debug(DEBUG_ERROR, "JS", "fatal error: %s", (msg ? msg : "no message"));
}

static void *alloc_function(void *udata, duk_size_t size) {
  return xmalloc(size);
}

static void *realloc_function(void *udata, void *ptr, duk_size_t size) {
  return xrealloc(ptr, size);
}

static void free_function(void *udata, void *ptr) {
  if (ptr) xfree(ptr);
}

int ext_script_init(void) {
  debug(DEBUG_INFO, "JS", "duktape version %d", DUK_VERSION);
  debug(DEBUG_INFO, "JS", "integer type is %d bytes", sizeof(duk_int_t));
  debug(DEBUG_INFO, "JS", "floating point type is %d bytes", sizeof(duk_double_t));

  return 0;
}

script_priv_t *ext_script_create(void) {
  script_priv_t *priv;

  if ((priv = xcalloc(1, sizeof(script_priv_t))) == NULL) {
    return NULL;
  }

  if ((priv->ctx = duk_create_heap(alloc_function, realloc_function, free_function, NULL, fatal_error)) == NULL) {
    debug(DEBUG_ERROR, "JS", "duk_create_heap failed");
    xfree(priv);
    return NULL;
  }

  priv->iref = 1;

  if (sizeof(script_ref_t) == sizeof(long int)) {
    priv->ref_mask = "ref%lx";
  } else {
    priv->ref_mask = "ref%llx";
  }

  return priv;
}

int ext_script_destroy(script_priv_t *priv) {
  if (priv != NULL) {
    if (priv->ctx) {
      duk_destroy_heap(priv->ctx);
    }
    xfree(priv);
  }

  return 0;
}

uint32_t ext_script_engine_id(void) {
  return scriptEngineJS;
}

int ext_script_run(script_priv_t *priv, char *filename, int argc, char *argv[]) {
  char buf[32], *msg;
  int i, str, r = -1;

  if (priv->ctx && filename) {
    set_last_error(priv, "");
    if (argc) {
      duk_push_global_object(priv->ctx);
      duk_push_array(priv->ctx);
      duk_dup_top(priv->ctx);
      duk_put_prop_string(priv->ctx, -3, "argv");
      for (i = 0; i < argc; i++) {
        snprintf(buf, sizeof(buf), "%d", i);
        duk_push_string(priv->ctx, argv[i]);
        duk_put_prop_string(priv->ctx, -2, buf);
      }
      duk_set_top(priv->ctx, 0);
    }

    str = filename[0] == '/';
    r = str ? duk_pcompile_string(priv->ctx, 0, filename) : duk_pcompile_file(priv->ctx, 0, filename);

    if (r == 0) {
      if ((r = duk_pcall(priv->ctx, 0)) != 0) {
        msg = (char *)duk_safe_to_string(priv->ctx, -1);
        debug(DEBUG_ERROR, "JS", "execution error (%s): %s", filename, msg);
        set_last_error(priv, msg);
      }
    } else {
      msg = (char *)duk_safe_to_string(priv->ctx, -1);
      debug(DEBUG_ERROR, "JS", "compilation error (%s): %s", filename, msg);
      set_last_error(priv, msg);
    }
    duk_set_top(priv->ctx, 0);
  }

  return r;
}

static int call_js(script_priv_t *priv, script_arg_t *ret, int n, script_arg_t *args) {
  int i, err;

  if (priv->ctx == NULL) {
    return -1;
  }

  if (!duk_is_function(priv->ctx, -1)) {
    debug(DEBUG_ERROR, "JS", "function not found or not a function");
    return -1;
  }

  for (i = 0, err = 0; i < n && !err; i++) {
    if (push_value(priv, &args[i]) == -1) {
      err = 1;
    }
  }

  if (!err) {
    if (duk_pcall(priv->ctx, n) == 0) {
      if (ext_script_get_value(priv, -1, SCRIPT_ARG_ANY, ret) == 0) {
        return 0;
      }
      debug(DEBUG_ERROR, "JS", "could not extract result");
    } else {
      debug(DEBUG_ERROR, "JS", "execution error: %s", duk_safe_to_string(priv->ctx, -1));
    }
  }

  return -1;
}

script_ref_t ext_script_dup_ref(script_priv_t *priv, script_ref_t ref) {
  get_ref(priv, ref);
  ref = new_ref(priv, -1);

  return ref;
}

int ext_script_remove_ref(script_priv_t *priv, script_ref_t ref) {
  char buf[32];

  snprintf(buf, sizeof(buf), priv->ref_mask, ref);
  duk_push_heap_stash(priv->ctx);
  duk_del_prop_string(priv->ctx, -1, buf);
  duk_pop(priv->ctx);

  return 0;
}

static script_ref_t new_ref(script_priv_t *priv, int index) {
  script_ref_t ref;
  char buf[32];

  ref = priv->iref++;
  snprintf(buf, sizeof(buf), priv->ref_mask, ref);
  duk_dup(priv->ctx, index);
  duk_push_heap_stash(priv->ctx);
  duk_swap(priv->ctx, -2, -1);
  duk_put_prop_string(priv->ctx, -2, buf);
  duk_pop(priv->ctx);

  return ref;
}

static void get_ref(script_priv_t *priv, script_ref_t ref) {
  char buf[32];

  snprintf(buf, sizeof(buf), priv->ref_mask, ref);
  duk_push_heap_stash(priv->ctx);
  duk_get_prop_string(priv->ctx, -1, buf);
  duk_swap(priv->ctx, -2, -1);
  duk_pop(priv->ctx);
}

int ext_script_call(script_priv_t *priv, script_ref_t ref, script_arg_t *ret, int n, script_arg_t *args) {
  char buf[32];
  int r;

  snprintf(buf, sizeof(buf), priv->ref_mask, ref);
  duk_push_heap_stash(priv->ctx);
  duk_get_prop_string(priv->ctx, -1, buf);
  //duk_swap(priv->ctx, -2, -1);
  //duk_pop(priv->ctx);

  r = call_js(priv, ret, n, args);
  duk_set_top(priv->ctx, 0);

  return r;
}

int ext_script_global_set(script_priv_t *priv, char *name, script_arg_t *value) {
  duk_push_global_object(priv->ctx);
  duk_push_string(priv->ctx, name);
  push_value(priv, value);
  duk_put_prop(priv->ctx, -3);
  duk_set_top(priv->ctx, 0);

  return 0;
}

int ext_script_global_get(script_priv_t *priv, char *name, script_arg_t *value) {
  int r;

  duk_push_global_object(priv->ctx);
  duk_push_string(priv->ctx, name);
  duk_get_prop(priv->ctx, -2);
  r = ext_script_get_value(priv, -1, SCRIPT_ARG_ANY, value);
  duk_set_top(priv->ctx, 0);

  return r;
}

script_ref_t ext_script_create_object(script_priv_t *priv) {
  script_ref_t obj;

  duk_push_object(priv->ctx);
  obj = new_ref(priv, -1);

  return obj;
}

int ext_script_object_set(script_priv_t *priv, script_ref_t obj, script_arg_t *key, script_arg_t *value) {
  get_ref(priv, obj);

  if (!duk_is_object(priv->ctx, -1)) {
    debug(DEBUG_ERROR, "JS", "object %lld not found or not an object", obj);
    return -1;
  }

  push_value(priv, key);
  push_value(priv, value);
  duk_put_prop(priv->ctx, -3);
  duk_pop(priv->ctx);

  return 0;
}

int ext_script_object_get(script_priv_t *priv, script_ref_t obj, script_arg_t *key, script_arg_t *value) {
  int r;

  get_ref(priv, obj);

  if (!duk_is_object(priv->ctx, -1)) {
    debug(DEBUG_ERROR, "JS", "object %lld not found or not an object", obj);
    return -1;
  }

  push_value(priv, key);
  duk_get_prop(priv->ctx, -2);
  r = ext_script_get_value(priv, -2, SCRIPT_ARG_ANY, value);
  duk_pop(priv->ctx);

  return r;
}

static int call_function(duk_context *ctx) {
  int (*f)(int pe);
  int pe, r;

  duk_push_current_function(ctx);

  duk_get_prop_string(ctx, -1, "f");
  f = duk_get_pointer(ctx, -1);
  duk_pop(ctx);

  duk_get_prop_string(ctx, -1, "pe");
  pe = duk_to_int(ctx, -1);
  duk_pop(ctx);

  duk_pop(ctx);

  r = f ? f(pe) : -1;

  return r >= 0 ? r : 0;
}

script_ref_t ext_script_create_function(script_priv_t *priv, int pe, int (*f)(int pe)) {
  script_ref_t obj;

  duk_push_c_function(priv->ctx, call_function, DUK_VARARGS);
  duk_push_pointer(priv->ctx, f);
  duk_put_prop_string(priv->ctx, -2, "f");
  duk_push_int(priv->ctx, pe);
  duk_put_prop_string(priv->ctx, -2, "pe");
  obj = new_ref(priv, -1);

  return obj;
}

int ext_script_push_value(script_priv_t *priv, script_arg_t *value) {
  return push_value(priv, value);
}
