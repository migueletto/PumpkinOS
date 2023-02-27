#ifndef PIT_SCRIPT_H
#define PIT_SCRIPT_H

#ifdef __cplusplus
extern "C" {
#endif

#define SCRIPT_ARG_ANY      '*'
#define SCRIPT_ARG_NULL     'N'
#define SCRIPT_ARG_INTEGER  'I'
#define SCRIPT_ARG_BOOLEAN  'B'
#define SCRIPT_ARG_REAL     'D'
#define SCRIPT_ARG_STRING   'S'
#define SCRIPT_ARG_LSTRING  'L'
#define SCRIPT_ARG_OBJECT   'O'
#define SCRIPT_ARG_FUNCTION 'F'
#define SCRIPT_ARG_POINTER  'P'
#define SCRIPT_ARG_FILE     'E'

#define MAX_ARGC  32

typedef struct script_priv_t script_priv_t;

typedef int32_t script_int_t;
typedef double  script_real_t;
typedef int64_t script_ref_t;

typedef struct {
  char *s;
  int n;
} script_lstring_t;

typedef struct {
  char type;
  union {
    script_int_t i;
    script_real_t d;
    char *s;
    script_lstring_t l;
    script_ref_t r;
    void *p;
  } value;
} script_arg_t;

// used by main.c

int script_load_engine_static(void);
int script_load_engine(char *libname);
int script_init(void);
int script_finish(void);
int script_create(void);
int script_destroy(int ptr);
void script_idle_loop(int pe);

// used by script.c / builtin.c

int script_create_builtins(int pe, script_ref_t obj);
script_ref_t script_loadlib(int pe, char *libname);
int script_run(int pe, char *filename, int argc, char *argv[]);
int script_set_cleanup(int pe, script_ref_t ref);
int script_set_idle(int pe, script_ref_t ref, uint32_t t);

// used by libraries

script_ref_t script_create_object(int pe);
int script_global_set(int pe, char *name, script_arg_t *value);
int script_global_get(int pe, char *name, script_arg_t *value);
int script_object_get(int pe, script_ref_t obj, script_arg_t *key, script_arg_t *value);
int script_object_set(int pe, script_ref_t obj, script_arg_t *key, script_arg_t *value);
script_ref_t script_create_function(int pe, int (*f)(int pe));
int script_set_pointer(int pe, char *name, void *p);
void *script_get_pointer(int pe, char *name);
int script_add_function(int pe, script_ref_t obj, char *name, int (*f)(int pe));
int script_add_iconst(int pe, script_ref_t obj, char *name, script_int_t value);
int script_add_rconst(int pe, script_ref_t obj, char *name, script_real_t value);
int script_add_sconst(int pe, script_ref_t obj, char *name, char *value);
script_ref_t script_dup_ref(int pe, script_ref_t ref);
int script_remove_ref(int pe, script_ref_t ref);

int script_get_named_value(int pe, int index, int type, char *lib, char *func, char *param, int opt, script_arg_t *arg);
int script_get_value(int pe, int index, int type, script_arg_t *arg);
int script_get_boolean(int pe, int index, int *b);
int script_get_integer(int pe, int index, script_int_t *i);
int script_get_real(int pe, int index, script_real_t *d);
int script_get_string(int pe, int index, char **s);
int script_get_lstring(int pe, int index, char **s, int *len);
int script_get_function(int pe, int index, script_ref_t *r);
int script_get_object(int pe, int index, script_ref_t *r);
int script_opt_integer(int pe, int index, script_int_t *i);
int script_opt_boolean(int pe, int index, int *b);
int script_opt_string(int pe, int index, char **s);

int script_push_value(int pe, script_arg_t *value);
int script_push_boolean(int pe, int b);
int script_push_integer(int pe, script_int_t i);
int script_push_real(int pe, script_real_t d);
int script_push_string(int pe, char *s);
int script_push_lstring(int pe, char *s, int len);
int script_push_function(int pe, script_ref_t r);
int script_push_object(int pe, script_ref_t r);
int script_push_null(int pe);

#define mkint(i) (script_int_t)(i)
int script_call_args(int pe, script_ref_t ref, script_arg_t *ret, int n, script_arg_t *args);
int script_call(int pe, script_ref_t ref, script_arg_t *ret, char *types, ...);
int script_returned_value(script_arg_t *ret);

// symbols implemented by engine

int ext_script_init(void);
script_priv_t *ext_script_create(void);
int ext_script_run(script_priv_t *priv, char *filename, int argc, char *argv[]);
int ext_script_destroy(script_priv_t *priv);
int ext_script_call(script_priv_t *priv, script_ref_t ref, script_arg_t *ret, int n, script_arg_t *args);
int ext_script_get_value(script_priv_t *priv, int i, int type, script_arg_t *arg);
script_ref_t ext_script_create_object(script_priv_t *priv);
int ext_script_global_set(script_priv_t *priv, char *name, script_arg_t *value);
int ext_script_global_get(script_priv_t *priv, char *name, script_arg_t *value);
int ext_script_object_get(script_priv_t *priv, script_ref_t obj, script_arg_t *key, script_arg_t *value);
int ext_script_object_set(script_priv_t *priv, script_ref_t obj, script_arg_t *key, script_arg_t *value);
script_ref_t ext_script_create_function(script_priv_t *priv, int pe, int (*f)(int pe));
script_ref_t ext_script_dup_ref(script_priv_t *priv, script_ref_t ref);
int ext_script_remove_ref(script_priv_t *priv, script_ref_t ref);
int ext_script_push_value(script_priv_t *priv, script_arg_t *value);

#define PIT_LIB_FUNCTION(lname,name) static int lib_function_##name(int pe) { char *lib = #lname; char *func = #name; int iarg = 0, err = 0, r = -1;
#define PIT_LIB_PARAM_F(name)        script_arg_t arg##name; script_ref_t name; err += script_get_named_value(pe, iarg++, SCRIPT_ARG_FUNCTION, lib, func, #name, 0, &arg##name); name = err ? 0 : arg##name.value.r;
#define PIT_LIB_PARAM_O(name)        script_arg_t arg##name; script_ref_t name; err += script_get_named_value(pe, iarg++, SCRIPT_ARG_OBJECT, lib, func, #name, 0, &arg##name); name = err ? 0 : arg##name.value.r;
#define PIT_LIB_PARAM_I(name)        script_arg_t arg##name; script_int_t name; err += script_get_named_value(pe, iarg++, SCRIPT_ARG_INTEGER, lib, func, #name, 0, &arg##name); name = err ? 0 : arg##name.value.i;
#define PIT_LIB_PARAM_B(name)        script_arg_t arg##name; int name; err += script_get_named_value(pe, iarg++, SCRIPT_ARG_BOOLEAN, lib, func, #name, 0, &arg##name); name = err ? 0 : arg##name.value.i;
#define PIT_LIB_PARAM_R(name)        script_arg_t arg##name; script_real_t name; err += script_get_named_value(pe, iarg++, SCRIPT_ARG_REAL, lib, func, #name, 0, &arg##name); name = err ? 0 : arg##name.value.r;
#define PIT_LIB_PARAM_S(name)        script_arg_t arg##name; char *name = NULL; err += script_get_named_value(pe, iarg++,  SCRIPT_ARG_STRING, lib, func, #name, 0, &arg##name); name = err ? 0 : arg##name.value.s;
#define PIT_LIB_PARAM_L(name,len)    script_arg_t arg##name; char *name = NULL; int len; err += script_get_named_value(pe, iarg++, SCRIPT_ARG_LSTRING, lib, func, #name, 0, &arg##name); if (err) { name = NULL; len = 0;} else { name = arg##name.value.l.s; len = arg##name.value.l.n; }
#define PIT_LIB_FREE_S(name)         if (name) xfree(name);
#define PIT_LIB_ENTER_F              debug(DEBUG_TRACE, "SCRIPT", "enter funcion %s.%s", lib, func)
#define PIT_LIB_CODE                 (void)iarg; if (err == 0) { PIT_LIB_ENTER_F;
#define PIT_LIB_PE                   pe
#define PIT_LIB_ERR                  debug(DEBUG_ERROR, "SCRIPT", "function %s.%s not called because of missing parameters", lib, func)
#define PIT_LIB_FAIL                 debug(DEBUG_ERROR, "SCRIPT", "function %s.%s failed", lib, func)
#define PIT_LIB_EXIT_F               debug(DEBUG_TRACE, "SCRIPT", "exit funcion %s.%s", lib, func)
#define PIT_LIB_END_B                } else { PIT_LIB_ERR; } if (r == -1) PIT_LIB_FAIL; PIT_LIB_EXIT_F; return script_push_boolean(pe, r == 0); }
#define PIT_LIB_END_I(i)             } else { PIT_LIB_ERR; } if (r == -1) PIT_LIB_FAIL; PIT_LIB_EXIT_F; return r != -1 ? script_push_integer(pe, i) : -1; }
#define PIT_LIB_END_R(n)             } else { PIT_LIB_ERR; } r = script_push_real(pe, n); PIT_LIB_EXIT_F; return r; }
#define PIT_LIB_END_S(s)             } else { PIT_LIB_ERR; } if (s) { r = script_push_string(pe, s); } else { r = -1; } PIT_LIB_EXIT_F; return r; }
#define PIT_LIB_END_O(obj)           } else { PIT_LIB_ERR; } if (obj != -1) { r = script_push_object(pe, obj); script_remove_ref(pe, obj); } else { r = -1; } PIT_LIB_EXIT_F; return r; }

#define PIT_LIB_BEGIN(name)          int lib##name##_init(int pe, script_ref_t obj) { int err = 0;
#define PIT_LIB_EXPORT_F(name)       err += script_add_function(pe, obj, #name, lib_function_##name);
#define PIT_LIB_EXPORT_I(name,value) err += script_add_iconst(pe, obj, #name, value)
#define PIT_LIB_EXPORT_R(name,value) err += script_add_rconst(pe, obj, #name, value)
#define PIT_LIB_EXPORT_S(name,value) err += script_add_sconst(pe, obj, #name, value)

#define PIT_LIB_END                  return err == 0 ? 0 : -1; }

int libbimage_load(void);
int libbimage_init(int pe, script_ref_t obj);

#ifdef __cplusplus
}
#endif

#endif
