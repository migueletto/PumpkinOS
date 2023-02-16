#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "script.h"
#include "debug.h"
#include "xalloc.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

struct script_priv_t {
  lua_State *L;
};

int ext_script_init(void) {
  debug(DEBUG_INFO, "LUA", "%s", LUA_RELEASE);
  debug(DEBUG_INFO, "LUA", "integer type is %d bytes", sizeof(lua_Integer));
  debug(DEBUG_INFO, "LUA", "floating point type is %d bytes", sizeof(lua_Number));

  return 0;
}

static void script_hook(lua_State *L, lua_Debug *ar) {
  char *what, *fname, *vname, *value, buf[64];
  int i;

  if (ar) {
    if (lua_getinfo(L, "nS", ar)) {
      what = (char *)ar->namewhat;
      fname = (char *)ar->name;
      if (what && fname) {
        debug(DEBUG_TRACE, "LUA", "hook call %s '%s' (%s:%d)", what, fname, ar->short_src, ar->linedefined);
        for (i = 1;; i++) {
          vname = (char *)lua_getlocal(L, ar, i);
          if (vname == NULL) break;
          if (vname[0] == '(' && strcmp(what, "field")) break; // XXX find another way to filter these
          if (lua_isnoneornil(L, i)) {
            value = "nil";
          } else if (lua_isboolean(L, i)) {
            value = lua_toboolean(L, i) ? "true" : "false";
          } else if (lua_isnumber(L, i)) {
            lua_Number number = luaL_checknumber(L, i);
            snprintf(buf, sizeof(buf), "%.6f", number);
            value = buf;
          } else if (lua_isstring(L, i)) {
            value = (char *)luaL_checkstring(L, i);
          } else if (lua_istable(L, i)) {
            value = "<table>";
          } else if (lua_isfunction(L, i)) {
            value = "<function>";
          } else if (lua_islightuserdata(L, i) || luaL_testudata(L, i, LUA_FILEHANDLE) != NULL) {
            value = "<userdata>";
          } else {
            value = "???";
          }
          debug(DEBUG_TRACE, "LUA", "hook var %d: %s = '%s'", i, vname, value);
          lua_pop(L, 1);
        }
      }
    }
  }
}

script_priv_t *ext_script_create(void) {
  script_priv_t *priv;

  if ((priv = xcalloc(1, sizeof(script_priv_t))) == NULL) {
    return NULL;
  }

  if ((priv->L = luaL_newstate()) == NULL) {
    debug(DEBUG_ERROR, "LUA", "luaL_newstate failed");
    xfree(priv);
    return NULL;
  }

  luaL_openlibs(priv->L);
  lua_settop(priv->L, 0);

  if (debug_getsyslevel("LUA") == DEBUG_TRACE) {
    lua_sethook(priv->L, script_hook, LUA_MASKCALL, 0);
  }

  return priv;
}

static int traceback(lua_State *L) {
  const char *msg = lua_tostring(L, 1);

  if (msg) {
    debug(DEBUG_ERROR, "LUA", "execution error: %s", msg);
  } else if (!lua_isnoneornil(L, 1)) {  // is there an error object?
    if (!luaL_callmeta(L, 1, "__tostring"))  { // try its 'tostring' metamethod
      lua_pushliteral(L, "(no error message)");
    }
  }

  return 1;
}

static int lua_docall(lua_State *L, int narg, int nres) {
  int status;
  int base = lua_gettop(L) - narg;  // function index
  lua_pushcfunction(L, traceback);  // push traceback function
  lua_insert(L, base);              // put it under chunk and args
  status = lua_pcall(L, narg, nres, base);
  lua_remove(L, base);              // remove traceback function
  return status;
}

static void get_table(lua_State *L, char *name) {
  lua_getglobal(L, name);

  if (lua_isnoneornil(L, -1)) {
    lua_newtable(L);
    lua_setglobal(L, name);
    lua_getglobal(L, name);
  }
}

int ext_script_run(script_priv_t *priv, char *filename, int argc, char *argv[]) {
  int i, str, r = -1;

  if (filename) {
    lua_settop(priv->L, 0);
    str = filename[0] == '-';
    r = str ?  luaL_loadstring(priv->L, filename) : luaL_loadfile(priv->L, filename);

    if (r == 0) {
      if (argc == 0 && !str) {
        get_table(priv->L, "argv");
        lua_settop(priv->L, 1);
      } else if (argc > 0) {
        for (i = 0; i < argc; i++) {
          get_table(priv->L, "argv");
          lua_pushinteger(priv->L, i+1);
          lua_pushstring(priv->L, argv[i]);
          lua_settable(priv->L, -3);
          lua_settop(priv->L, 1);
        }
      }

      if (lua_docall(priv->L, 0, 0) != 0) {
        r = -1;
      }
      lua_settop(priv->L, 0);

    } else {
      debug(DEBUG_ERROR, "LUA", "compilation error: %s ", lua_tostring(priv->L, -1));
      r = -1;
    }
  }

  return r;
}

int ext_script_destroy(script_priv_t *priv) {
  if (priv != NULL) {
    lua_close(priv->L);
    xfree(priv);
  }

  return 0;
}

static script_ref_t new_ref(lua_State *L, int index) {
  script_ref_t ref;

  lua_pushvalue(L, index);
  ref = luaL_ref(L, LUA_REGISTRYINDEX);
  debug(DEBUG_TRACE, "LUA", "creating ref %d (index %d)", (int)ref, index);

  return ref;
}

static void get_ref(lua_State *L, script_ref_t ref) {
  lua_rawgeti(L, LUA_REGISTRYINDEX, (lua_Integer)ref);
}

static script_ref_t dup_ref(lua_State *L, script_ref_t ref) {
  script_ref_t nref;

  lua_rawgeti(L, LUA_REGISTRYINDEX, (lua_Integer)ref);
  nref = luaL_ref(L, LUA_REGISTRYINDEX);
  debug(DEBUG_TRACE, "LUA", "duplicating ref %d to %d", (int)ref, (int)nref);

  return nref;
}

static int del_ref(lua_State *L, script_ref_t ref) {
  debug(DEBUG_TRACE, "LUA", "removing ref %d", (int)ref);
  luaL_unref(L, LUA_REGISTRYINDEX, (lua_Integer)ref);

  return 0;
}

int ext_script_call(script_priv_t *priv, script_ref_t ref, script_arg_t *ret, int n, script_arg_t *args) {
  int i, err;

  lua_settop(priv->L, 0);
  get_ref(priv->L, ref);

  if (lua_isnoneornil(priv->L, 1)) {
    debug(DEBUG_ERROR, "LUA", "attempt to call nil");
    return -1;
  }

  if (!lua_isfunction(priv->L, 1)) {
    debug(DEBUG_ERROR, "LUA", "attempt to call a '%s'", lua_typename(priv->L, lua_type(priv->L, 1)));
    if (lua_isstring(priv->L, 1)) {
      debug(DEBUG_ERROR, "LUA", "string value '%s'", (char *)luaL_checkstring(priv->L, 1));
    }
    return -1;
  }

  for (i = 0, err = 0; i < n; i++) {
    if (ext_script_push_value(priv, &args[i]) == -1) {
      err = 1;
    }
  }

  if (!err) {
    if (lua_docall(priv->L, n, 1) == 0) {
      if (ext_script_get_value(priv, 0, SCRIPT_ARG_ANY, ret) == -1) {
        err = 1;
      }
    } else {
      err = 1;
    }
  }

  lua_settop(priv->L, 0);

  return err ? -1 : 0;
}

static int close_file(lua_State *L) {
  (void)L;
  return 0;
}

static int push_value(lua_State *L, script_arg_t *arg) {
  luaL_Stream *p;

  switch(arg->type) {
    case SCRIPT_ARG_BOOLEAN:
      lua_pushboolean(L, arg->value.i);
      break;

    case SCRIPT_ARG_INTEGER:
      lua_pushinteger(L, arg->value.i);
      break;

    case SCRIPT_ARG_REAL:
      lua_pushnumber(L, arg->value.d);
      break;

    case SCRIPT_ARG_STRING:
      lua_pushstring(L, arg->value.s);
      break;

    case SCRIPT_ARG_LSTRING:
      lua_pushlstring(L, arg->value.l.s, arg->value.l.n);
      break;

    case SCRIPT_ARG_NULL:
      lua_pushnil(L);
      break;

    case SCRIPT_ARG_OBJECT:
      get_ref(L, arg->value.r);
      break;

    case SCRIPT_ARG_FUNCTION:
      get_ref(L, arg->value.r);
      break;

    case SCRIPT_ARG_POINTER:
      lua_pushlightuserdata(L, arg->value.p);
      break;

    case SCRIPT_ARG_FILE:
      p = (luaL_Stream *)lua_newuserdata(L, sizeof(luaL_Stream));
      p->f = arg->value.f;
      p->closef = close_file;
      luaL_setmetatable(L, LUA_FILEHANDLE);
      break;

    default:
      debug(DEBUG_ERROR, "LUA", "invalid pushed value type '%c'", arg->type);
      return -1;
  }

  return 1;
}

static void get_context(script_priv_t *priv, int i, int type) {
  lua_Debug ar;

  if (lua_getstack(priv->L, 0, &ar)) {
    if (lua_getinfo(priv->L, "nSlu", &ar)) {
      debug(DEBUG_INFO, "LUA", "parameter context: %s '%s', index %d, type '%c'", ar.namewhat, ar.name, i, type);
    }
  }
}

int ext_script_get_value(script_priv_t *priv, int i, int type, script_arg_t *arg) {
  lua_Number number;
  lua_Integer integer;
  luaL_Stream *p;
  size_t len;
  char *s;

  if (i < 0) {
    debug(DEBUG_ERROR, "LUA", "invalid argument index %d", i);
    return -1;
  }

  if (lua_isnone(priv->L, i+1)) {
    get_context(priv, i, type);
    return -1;
  }

  switch (type) {
    case SCRIPT_ARG_ANY:
      if (lua_isnoneornil(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_NULL;
        arg->value.i = 0;
      } else if (lua_isboolean(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_BOOLEAN;
        arg->value.i = lua_toboolean(priv->L, i+1);
      } else if (lua_istable(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_OBJECT;
        arg->value.r = new_ref(priv->L, i+1);
      } else if (lua_islightuserdata(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_POINTER;
        arg->value.p = lua_touserdata(priv->L, i+1);
      } else if (lua_isfunction(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_FUNCTION;
        arg->value.r = new_ref(priv->L, i+1);
      } else if (lua_isnumber(priv->L, i+1)) {
        number = luaL_checknumber(priv->L, i+1);
        integer = (lua_Integer)number;
        if (number == integer) {
          arg->type = SCRIPT_ARG_INTEGER;
          arg->value.i = integer;
        } else {
          arg->type = SCRIPT_ARG_REAL;
          arg->value.d = number;
        }
      } else if (lua_isstring(priv->L, i+1)) {
        s = (char *)luaL_checklstring(priv->L, i+1, &len);
        if (s) {
          arg->type = SCRIPT_ARG_LSTRING;
          arg->value.l.s = s;
          arg->value.l.n = len;
        } else {
          return -1;
        }
      } else if (luaL_testudata(priv->L, i+1, LUA_FILEHANDLE) != NULL) {
        if ((p = luaL_checkudata(priv->L, i+1, LUA_FILEHANDLE)) != NULL) {
          arg->type = SCRIPT_ARG_FILE;
          arg->value.f = p->f;
        } else {
          return -1;
        }
      } else {
        return -1;
      }
      break;

    case SCRIPT_ARG_BOOLEAN:
      if (lua_isnoneornil(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_BOOLEAN;
        arg->value.i = 0;
      } else if (lua_isboolean(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_BOOLEAN;
        arg->value.i = lua_toboolean(priv->L, i+1);
      } else {
        s = (char *)lua_tostring(priv->L, i+1);
        if (s && !strcmp(s, "true")) {
          arg->type = SCRIPT_ARG_BOOLEAN;
          arg->value.i = 1;
        } else {
          integer = s ? atoi(s) : lua_tointeger(priv->L, i+1);
          arg->type = SCRIPT_ARG_BOOLEAN;
          arg->value.i = lua_isnumber(priv->L, i+1) ? integer : 0;
        }
      }
      break;

    case SCRIPT_ARG_INTEGER:
      if (lua_isnoneornil(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_INTEGER;
        arg->value.i = 0;
      } else if (lua_isboolean(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_INTEGER;
        arg->value.i = lua_toboolean(priv->L, i+1) ? 1 : 0;
      } else {
        number = luaL_checknumber(priv->L, i+1);
        arg->type = SCRIPT_ARG_INTEGER;
        arg->value.i = (script_int_t)number;
      }
      break;

    case SCRIPT_ARG_REAL:
      if (lua_isnoneornil(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_REAL;
        arg->value.d = 0;
      } else if (lua_isboolean(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_REAL;
        arg->value.d = lua_toboolean(priv->L, i+1) ? 1 : 0;
      } else {
        arg->type = SCRIPT_ARG_REAL;
        arg->value.d = luaL_checknumber(priv->L, i+1);
      }
      break;

    case SCRIPT_ARG_STRING:
      if (lua_isnoneornil(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_STRING;
        arg->value.s = NULL;
      } else if (lua_isboolean(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_STRING;
        s = lua_toboolean(priv->L, i+1) ? "true" : "false";
        arg->value.s = s;
      } else {
        arg->type = SCRIPT_ARG_STRING;
        s = (char *)luaL_checkstring(priv->L, i+1);
        arg->value.s = s;
      }
      break;

    case SCRIPT_ARG_LSTRING:
      if (lua_isnoneornil(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_LSTRING;
        arg->value.l.s = NULL;
        arg->value.l.n = 0;
      } else if (lua_isboolean(priv->L, i+1)) {
        s = lua_toboolean(priv->L, i+1) ? "true" : "false";
        arg->type = SCRIPT_ARG_LSTRING;
        arg->value.l.s = s;
        arg->value.l.n = strlen(s);
      } else {
        s = (char *)luaL_checklstring(priv->L, i+1, &len);
        arg->type = SCRIPT_ARG_LSTRING;
        arg->value.l.s = s;
        arg->value.l.n = len;
      }
      break;

    case SCRIPT_ARG_FUNCTION:
      if (lua_isfunction(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_FUNCTION;
        arg->value.r = new_ref(priv->L, i+1);
      } else {
        debug(DEBUG_ERROR, "LUA", "invalid argument type for function");
        get_context(priv, i, type);
        return -1;
      }
      break;

    case SCRIPT_ARG_POINTER:
      if (lua_isnoneornil(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_POINTER;
        arg->value.p = 0;
      } else if (lua_islightuserdata(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_POINTER;
        arg->value.p = lua_touserdata(priv->L, i+1);
      } else {
        debug(DEBUG_ERROR, "LUA", "invalid argument type for pointer");
        get_context(priv, i, type);
        return -1;
      }
      break;

    case SCRIPT_ARG_OBJECT:
      if (lua_isnoneornil(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_OBJECT;
        arg->value.r = 0;
      } else if (lua_istable(priv->L, i+1)) {
        arg->type = SCRIPT_ARG_OBJECT;
        arg->value.r = new_ref(priv->L, i+1);
      } else {
        debug(DEBUG_ERROR, "LUA", "invalid argument type for object");
        get_context(priv, i, type);
        return -1;
      }
      break;
  }

  return 0;
}

script_ref_t ext_script_create_object(script_priv_t *priv) {
  script_ref_t obj;

  lua_newtable(priv->L);
  obj = new_ref(priv->L, lua_gettop(priv->L));
  lua_settop(priv->L, 0);

  return obj;
}

int ext_script_global_set(script_priv_t *priv, char *name, script_arg_t *value) {
  push_value(priv->L, value);
  lua_setglobal(priv->L, name);
  lua_settop(priv->L, 0);

  return 0;
}

int ext_script_global_get(script_priv_t *priv, char *name, script_arg_t *value) {
  int r;

  lua_getglobal(priv->L, name);
  r = ext_script_get_value(priv, lua_gettop(priv->L)-1, SCRIPT_ARG_ANY, value);
  lua_pop(priv->L, 1);

  return r;
}

int ext_script_object_set(script_priv_t *priv, script_ref_t obj, script_arg_t *key, script_arg_t *value) {
  get_ref(priv->L, obj);

  if (!lua_istable(priv->L, -1)) {
    debug(DEBUG_ERROR, "LUA", "object %lld not found or not a table", obj);
    return -1;
  }

  push_value(priv->L, key);
  push_value(priv->L, value);
  lua_settable(priv->L, -3);
  lua_settop(priv->L, 0);

  return 0;
}

int ext_script_object_get(script_priv_t *priv, script_ref_t obj, script_arg_t *key, script_arg_t *value) {
  int r;

  get_ref(priv->L, obj);

  if (!lua_istable(priv->L, -1)) {
    debug(DEBUG_ERROR, "LUA", "object %lld not found or not a table", obj);
    return -1;
  }

  push_value(priv->L, key);
  lua_gettable(priv->L, -2);
  r = ext_script_get_value(priv, lua_gettop(priv->L)-1, SCRIPT_ARG_ANY, value);
  lua_settop(priv->L, 0);

  return r;
}

static int call_function(lua_State *L) {
  int pe;
  int (*f)(int pe);
  int r;

  f = lua_touserdata(L, lua_upvalueindex(1));
  pe = lua_tointeger(L, lua_upvalueindex(2));
  r = f(pe);

  return r >= 0 ? r : 0;
}

script_ref_t ext_script_create_function(script_priv_t *priv, int pe, int (*f)(int pe)) {
  script_ref_t obj;

  lua_pushlightuserdata(priv->L, f);
  lua_pushinteger(priv->L, pe);
  lua_pushcclosure(priv->L, call_function, 2);
  obj = new_ref(priv->L, lua_gettop(priv->L));
  lua_settop(priv->L, 0);

  return obj;
}

script_ref_t ext_script_dup_ref(script_priv_t *priv, script_ref_t ref) {
  return dup_ref(priv->L, ref);
}

int ext_script_remove_ref(script_priv_t *priv, script_ref_t ref) {
  return del_ref(priv->L, ref);
}

int ext_script_push_value(script_priv_t *priv, script_arg_t *value) {
  return push_value(priv->L, value);
}
