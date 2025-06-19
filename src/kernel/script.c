#include "sys.h"
#include "script.h"
#include "pwindow.h"

static void *wp;

int script_set_pointer(int pe, char *name, void *p) {
  if (!sys_strcmp(name, WINDOW_PROVIDER)) {
    wp = p;
  }

  return 0;
}

void *script_get_pointer(int pe, char *name) {
  if (!sys_strcmp(name, WINDOW_PROVIDER)) {
     return wp;
  }

  return NULL;
}

int script_get_object(int pe, int index, script_ref_t *ref) {
  sys_memset(ref, 0, sizeof(script_ref_t));
  return 0;
}

#define OBJ_INTEGER(k,v) \
    if (!sys_strcmp(key->value.s, k)) { \
      value->type = SCRIPT_ARG_INTEGER; \
      value->value.i = v; \
      return 0; \
    }

#define OBJ_BOOLEAN(k,v) \
    if (!sys_strcmp(key->value.s, k)) { \
      value->type = SCRIPT_ARG_BOOLEAN; \
      value->value.i = v; \
      return 0; \
    }

int script_object_get(int pe, script_ref_t obj, script_arg_t *key, script_arg_t *value) {
  if (key->type == SCRIPT_ARG_STRING) {
    OBJ_INTEGER("density", 144);
    OBJ_INTEGER("width",   320);
    OBJ_INTEGER("height",  480);
    OBJ_INTEGER("hdepth",  16);
    OBJ_INTEGER("depth",   16);
    OBJ_INTEGER("mode",    1);
    OBJ_BOOLEAN("dia",     1);
  }

  return -1;
}

int script_get_integer(int pe, int index, script_int_t *i) {
  return 0;
}

int script_get_string(int pe, int index, char **s) {
  return 0;
}

int script_get_lstring(int pe, int index, char **s, int *len){
  return 0;
}

int script_get_boolean(int pe, int index, int *b) {
  return 0;
}

int script_opt_integer(int pe, int index, script_int_t *i) {
  return 0;
}

int script_opt_boolean(int pe, int index, int *b) {
  return 0;
}

int script_opt_string(int pe, int index, char **s) {
  return 0;
}

int script_push_boolean(int pe, int b) {
  return 0;
}

int script_push_integer(int pe, script_int_t i) {
  return 0;
}

int script_push_string(int pe, char *s) {
  return 0;
}

int script_push_lstring(int pe, char *s, int len) {
  return 0;
}

int script_push_object(int pe, script_ref_t r) {
  return 0;
}

int script_add_function(int pe, script_ref_t obj, char *name, int (*f)(int pe)) {
  return 0;
}

int script_add_boolean(int pe, script_ref_t obj, char *name, int b) {
  return 0;
}

int script_add_iconst(int pe, script_ref_t obj, char *name, script_int_t value) {
  return 0;
}

int script_add_sconst(int pe, script_ref_t obj, char *name, char *value) {
  return 0;
}

uint32_t script_engine_id(script_engine_t *engine) {
  return 0;
}

script_engine_t *script_get_engine(int pe) {
  return NULL;
}

script_ref_t script_create_function(int pe, int (*f)(int pe)) {
  return 0;
}

script_ref_t script_create_function_data(int pe, int (*f)(int pe, void *data), void *data) {
  return 0;
}

script_ref_t script_create_object(int pe) {
  return 0;
}

int script_global_set(int pe, char *name, script_arg_t *value) {
  return 0;
}

int script_global_get(int pe, char *name, script_arg_t *value) {
  return 0;
}

int script_returned_value(script_arg_t *ret) {
  return 0;
}

int script_get_last_error(int pe, char *buf, int max) {
  return 0;
}

int script_create(script_engine_t *engine) {
  return 0;
}

int script_destroy(int ptr) {
  return 0;
}

int script_call_args(int pe, script_ref_t ref, script_arg_t *ret, int n, script_arg_t *args) {
  return 0;
}

int script_call(int pe, script_ref_t ref, script_arg_t *ret, char *types, ...) {
  return 0;
}

int script_run(int pe, char *filename, int argc, char *argv[], int str) {
  return 0;
}

int script_remove_ref(int pe, script_ref_t ref) {
  return 0;
}
