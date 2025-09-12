#include "sys.h"
#include "plibc.h"
#include "ht.h"
#include "parser.h"
#include "lexer.h"
#include "assembler.h"
#include "type.h"
#include "symbol.h"
#include "section.h"
#include "buffer.h"
#include "error.h"
#include "debug.h"

#define next() parser_next(p)

#define BACK p->back = p->t

#define opt_match(i) \
  (next(), p->t.id == i)

#define match(i) \
  parser_next(p); \
  if (p->t.id != i) return syntax_error(p, "expecting %s, got %s", lexer_stoken(i), lexer_stoken(p->t.id))

#define EXPR(depth) \
  depth = 0; \
  if ((type = parse_expr(p, &depth)) == -1) return -1

#define TYPE(type) \
  if ((type = parse_type(p)) == -1) return -1

#define insection(s) \
  if (section > s) return syntax_error(p, "wrong section"); \
  section = s

#define OPENP   match(TOKEN_OPENP)
#define CLOSEP  match(TOKEN_CLOSEP)
#define OPENB   match(TOKEN_OPENB)
#define CLOSEB  match(TOKEN_CLOSEB)
#define OPENSB  match(TOKEN_OPENSB)
#define CLOSESB match(TOKEN_CLOSESB)

#define CONDITION \
  OPENP; \
  EXPR(depth); \
  CLOSEP

#define STATEMENTS(last, pre_loop, post_loop) \
  if (parse_statements(p, last, pre_loop, post_loop) != 0) return -1

#define BLOCK \
  OPENB; \
  STATEMENTS(NULL, pre_loop, post_loop); \
  CLOSEB

#define IDENT(s) \
  match(TOKEN_IDENT); \
  s = p->t.value

//#define ASSIGN match(TOKEN_ASSIGN)
#define CASE   match(TOKEN_CASE)
#define COMMA  match(TOKEN_COMMA)
#define opt(id) opt_match(TOKEN_##id)

#define FORMAL_PARAMS(lt) if ((nparams = parse_formal_params(p, lt)) == -1) return -1;
#define ACTUAL_PARAMS(f)  if (parse_actual_params(p, f) == -1) return -1;
#define LOCAL_VARS(lt)    if (parse_local_vars(p, lt) != 0) return -1;

#define getvalue p->t.value

#define MAX_STACK   64
#define NUM_SYMBOLS 64

typedef struct {
  PLIBC_FILE *fdout;
  lexer_t *l;
  symbol_table_t *st;
  token_t back;
  token_t t;
  int eval;
  symbol_function_t *f;
  value_t stack[MAX_STACK];
  uint32_t stackp;
  uint32_t ilabel;
  uint32_t param_size;
  int verbose;
} parser_t;

static void info(parser_t *p, char *msg, ...) {
  sys_va_list ap;

  if (p->verbose) {
    sys_va_start(ap, msg);
    debugva(DEBUG_INFO, APPNAME, msg, ap);
    sys_va_end(ap);
  }
}

static int syntax_error(parser_t *p, char *msg, ...) {
  sys_va_list ap;
  char buf[1024];

  sys_va_start(ap, msg);
  sys_vsnprintf(buf, sizeof(buf)-1, msg, ap);
  sys_va_end(ap);
  system_error("Syntax error at line %u: %s", lexer_line(p->l), buf);

  return -1;
}

static symbol_table_t *new_symbol_table(int global) {
  symbol_table_t *st;

  if ((st = sys_calloc(1, sizeof(symbol_table_t))) != NULL) {
    st->global = global;
  }

  return st;
}

static symbol_struct_t *get_struct(parser_t *p, type_t type) {
  symbol_t *s;
  uint32_t index;

  index = type - TYPE_STRUCT;
  if (index < p->st->num_symbols) {
    s = &p->st->symbol[index];
    if (s->type == SYMBOL_STRUCT) {
      return &s->symbol.struc;
    }
  }

  syntax_error(p, "invalid type");
  return NULL;
}

static symbol_table_t *copy_symbol_table(parser_t *p, symbol_table_t *st) {
  symbol_table_t *nst;
  symbol_struct_t *struc;
  symbol_t *ns, *s;
  int i;

  if ((nst = sys_calloc(1, sizeof(symbol_table_t))) != NULL) {
    nst->size_symbols = st->size_symbols;
    nst->num_symbols = st->num_symbols;
    nst->global = st->global;
    nst->frame_size = st->frame_size;
    nst->symbol = sys_calloc(st->size_symbols, sizeof(symbol_t));

    for (i = 0; i < st->num_symbols; i++) {
      ns = &nst->symbol[i];
      s = &st->symbol[i];
      ns->index = s->index;
      ns->type = s->type;
      ns->name = s->name;
      ns->st = nst;

      if (s->type == SYMBOL_VAR) {
        ns->symbol.var.type = s->symbol.var.type;
        ns->symbol.var.array = s->symbol.var.array;
        ns->symbol.var.offset = s->symbol.var.offset;

        if (s->symbol.var.type >= TYPE_STRUCT) {
          struc = get_struct(p, s->symbol.var.type);
          ns->symbol.var.st = copy_symbol_table(p, struc->st);
        }
      }
    }
  }

  return nst;
}

static parser_t *parser_init(lexer_t *l, int fdout) {
  parser_t *p;
  PLIBC_FILE *f;

  if ((f = plibc_fdopen(fdout, "w")) != NULL) {
    if ((p = sys_calloc(1, sizeof(parser_t))) != NULL) {
      p->fdout = f;
      p->st = new_symbol_table(1);
      p->l = l;
      p->ilabel = 1;
      p->verbose = 1;
    }
  }

  return p;
}

static void parser_destroy(parser_t *p) {
  if (p) sys_free(p);
}

static uint32_t next_label(parser_t *p) {
  return p->ilabel++;
}

static uint16_t parser_next(parser_t *p) {
  if (p->back.id) {
    p->t = p->back;
    p->back.id = 0;
    return p->t.id;
  }

  lexer_next(p->l, &p->t);

  return p->t.id;
}

#define booln \
  case TOKEN_BOOL: \
    etype[*depth] = gen_bool(p, p->t.value); \
    (*depth)++; \
    break

#define integer(ti, base, sig) \
  case TOKEN_##ti: \
    etype[*depth] = gen_integer(p, p->t.value, base, sig); \
    (*depth)++; \
    break

#define floatn \
  case TOKEN_FLOAT: \
    etype[*depth] = gen_float(p, p->t.value); \
    (*depth)++; \
    break

#define binop(op) \
  case TOKEN_##op: \
    if (*depth < 2) return syntax_error(p, "insuficient arguments on stack for binary operator (%d)", *depth); \
    if (gen_binop(p, TOKEN_##op, etype, *depth) == -1) return -1; \
    (*depth)--; \
    break

#define unop(op) \
  case TOKEN_##op: \
    if (*depth < 1) return syntax_error(p, "insuficient arguments on stack for unary operator (%d)", *depth); \
    if (gen_unop(p, TOKEN_##op, etype, *depth) == -1) return -1; \
    break

static void emit(parser_t *p, char *fmt, ...) {
  sys_va_list ap;

  sys_va_start(ap, fmt);
  plibc_vfprintf(p->fdout, fmt, ap);
  plibc_fflush(p->fdout);
  sys_va_end(ap);
}

static void push(parser_t *p, value_t *v) {
  if (p->stackp < MAX_STACK) {
    p->stack[p->stackp++] = *v;
  }
}

static void pop(parser_t *p, value_t *v) {
  if (p->stackp > 0) {
    *v = p->stack[--p->stackp];
  }
}

static symbol_t *find_symbol(symbol_table_t *st, char *name, uint32_t type_mask) {
  int i;

  for (i = 0; i < st->num_symbols; i++) {
    if (!sys_strcmp(st->symbol[i].name, name)) {
      if (st->symbol[i].type & type_mask) return &st->symbol[i];
      break;
    }
  }

  return NULL;
}

static type_t parse_type(parser_t *p) {
  symbol_t *s;
  token_e t;
  char *name;

  switch (t = parser_next(p)) {
    case TOKEN_UINT8:  return TYPE_UINT8;
    case TOKEN_INT8:   return TYPE_INT8;
    case TOKEN_UINT16: return TYPE_UINT16;
    case TOKEN_INT16:  return TYPE_INT16;
    case TOKEN_UINT32: return TYPE_UINT32;
    case TOKEN_INT32:  return TYPE_INT32;
    case TOKEN_FLOAT:  return TYPE_FLOAT;
    case TOKEN_STRING: return TYPE_STRING;
    case TOKEN_STRUCT:
      IDENT(name);
      if ((s = find_symbol(p->st, name, SYMBOL_STRUCT)) == NULL) {
        return syntax_error(p, "struct '%s' is not defined", name);
      }
      return TYPE_STRUCT + s->index;
    default: break;
  }

  return syntax_error(p, "invalid type %s", lexer_stoken(t));
}

static symbol_t *new_symbol(parser_t *p, symbol_table_t *st, char *name, uint32_t type) {
  if (find_symbol(st, name, SYMBOL_ALL)) {
    syntax_error(p, "symbol '%s' already defined", name);
    return NULL;
  }

  if (st->symbol == NULL) {
    st->size_symbols = NUM_SYMBOLS;
    st->symbol = sys_calloc(st->size_symbols, sizeof(symbol_t));
  } else if (st->num_symbols == st->size_symbols) {
    st->size_symbols += NUM_SYMBOLS;
    st->symbol = sys_realloc(st->symbol, st->size_symbols * sizeof(symbol_t));
  }

  st->symbol[st->num_symbols].index = st->num_symbols;
  st->symbol[st->num_symbols].type = type;
  st->symbol[st->num_symbols].name = name;
  st->symbol[st->num_symbols].st = st;

  return &st->symbol[st->num_symbols++];
}

static int set_value(uint8_t *p, uint32_t i, value_t *value) {
  int r = -1;

  switch (value->type) {
    case TYPE_BOOL:   ((uint8_t *)p)[i]  = value->value.b;   r = 0; break;
    case TYPE_INT8:   ((int8_t *)p)[i]   = value->value.i8;  r = 0; break;
    case TYPE_UINT8:  ((uint8_t *)p)[i]  = value->value.u8;  r = 0; break;
    case TYPE_INT16:  ((int16_t *)p)[i]  = value->value.i16; r = 0; break;
    case TYPE_UINT16: ((uint16_t *)p)[i] = value->value.u16; r = 0; break;
    case TYPE_INT32:  ((int32_t *)p)[i]  = value->value.i32; r = 0; break;
    case TYPE_UINT32: ((uint32_t *)p)[i] = value->value.u32; r = 0; break;
    case TYPE_FLOAT:  ((float  *)p)[i]   = value->value.d;   r = 0; break;
    //case TYPE_STRING: ((uint32_t *)p)[i] = value->value.s;   r = 0; break;
    default: break;
  }

  return r;
}

static int get_value(type_t type, uint8_t *p, uint32_t i, value_t *value) {
  int r = -1;

  value->type = type;

  switch (type) {
    case TYPE_BOOL:   value->value.b   = ((uint8_t  *)p)[i]; r = 0; break;
    case TYPE_INT8:   value->value.i8  = ((int8_t   *)p)[i]; r = 0; break;
    case TYPE_UINT8:  value->value.u8  = ((uint8_t  *)p)[i]; r = 0; break;
    case TYPE_INT16:  value->value.i16 = ((int16_t  *)p)[i]; r = 0; break;
    case TYPE_UINT16: value->value.u16 = ((uint16_t *)p)[i]; r = 0; break;
    case TYPE_INT32:  value->value.i32 = ((int32_t  *)p)[i]; r = 0; break;
    case TYPE_UINT32: value->value.u32 = ((uint32_t *)p)[i]; r = 0; break;
    case TYPE_FLOAT:  value->value.d   = ((float    *)p)[i]; r = 0; break;
    //case TYPE_STRING: value->value.s   = ((char    **)p)[i]; r = 0; break;
    default: break;
  }

  return r;
}

static type_t gen_bool(parser_t *p, char *s) {
  value_t v;

  v.type = TYPE_BOOL;
  v.value.b = sys_strtoul(s, NULL, 10) ? 1 : 0;

  if (p->eval) {
    push(p, &v);
  } else {
    emit(p, "pushu8 %d\n", v.value.b);
  }

  return v.type;
}

static type_t gen_integer(parser_t *p, char *s, int base, int issigned) {
  value_t v;
  uint64_t u;
  int64_t i;

  v.value.u64 = 0;

  if (issigned) {
    i = sys_strtol(s, NULL, base);
    v.type = TYPE_INT32;
    v.value.i32 = i;
    if (p->eval) {
      push(p, &v);
    } else {
      if (i >= -0x80 && i < 0x7F) {
        emit(p, "pushi8 %d\n", i);
      } else if (i >= -0x8000 && i < 0x7FFF) {
        emit(p, "pushi16 %d\n", i);
      } else {
        emit(p, "push32\n");
        emit(p, "d32 %d\n", i);
      }
    }
  } else {
    u = sys_strtoul(s, NULL, base);
    v.type = TYPE_UINT32;
    v.value.u32 = u;
    if (p->eval) {
      push(p, &v);
    } else {
      if (u < 0x100) {
        emit(p, "pushu8 %u\n", u);
      } else if (u < 0x10000) {
        emit(p, "pushu16 %u\n", u);
      } else {
        emit(p, "push32\n");
        emit(p, "d32 %u\n", u);
      }
    }
  }

  return v.type;
}

static type_t gen_float(parser_t *p, char *s) {
  value_t v;

  v.type = TYPE_FLOAT;
  v.value.d = sys_atof(s);

  if (p->eval) {
    push(p, &v);
  } else {
    emit(p, "push32\n");
    emit(p, "d32 0x%08X ; float %.8f\n", v.value.u32, v.value.d);
  }

  return v.type;
}

static symbol_t *add_new_string(parser_t *p, char *s) {
  symbol_t *symbol;
  char name[16];
  uint32_t id;

  id = next_label(p);
  sys_snprintf(name, sizeof(name)-1, "%u", id);

  if ((symbol = new_symbol(p, p->st, sys_strdup(name), SYMBOL_STRING)) != NULL) {
    symbol->symbol.string.id = id;
    symbol->symbol.string.s = s;
    emit(p, "d32 :%u ; string \"%s\"\n", id, s);
  }

  return symbol;
}

static type_t gen_string(parser_t *p, char *s) {
  value_t v;

  v.type = TYPE_STRING;
  v.value.s = s;

  if (p->eval) {
    push(p, &v);
  } else {
    emit(p, "push32\n");
    add_new_string(p, s);
  }

  return v.type;
}

#define icases(op) \
    case TYPE_UINT32: v1.value.u32 = v1.value.u32 op v2.value.u32; break; \
    case TYPE_UINT16: v1.value.u16 = v1.value.u16 op v2.value.u16; break; \
    case TYPE_UINT8:  v1.value.u8  = v1.value.u8  op v2.value.u8;  break; \
    case TYPE_INT32:  v1.value.i32 = v1.value.i32 op v2.value.i32; break; \
    case TYPE_INT16:  v1.value.i16 = v1.value.i16 op v2.value.i16; break; \
    case TYPE_INT8:   v1.value.i8  = v1.value.i8  op v2.value.i8;  break; \
    case TYPE_BOOL:   v1.value.u8  = v1.value.u8  op v2.value.u8;  break;

#define ibinop(op) \
  switch (v1.type) { \
    icases(op) \
    default: return -1; \
  }

#define ifbinop(op) \
  switch (v1.type) { \
    icases(op) \
    case TYPE_FLOAT: v1.value.d = v1.value.d op v2.value.d; break; \
    default: return -1; \
  }

#define eval_binop(op,bop) \
  pop(p, &v1); \
  pop(p, &v2); \
  if (v1.type != v2.type) { \
    if (type_contains(v1.type, v2.type)) { \
      if (type_coerce(&v2, v1.type, &v2, 0) != 0) return -1; \
    } else if (type_contains(v2.type, v1.type)) { \
      if (type_coerce(&v1, v2.type, &v1, 0) != 0) return -1; \
    } else { \
      return -1; \
    } \
  } \
  bop(op) \
  push(p, &v1);

#define iucases(op) \
  case TYPE_UINT32: v1.value.u32 = op v1.value.u32; break; \
  case TYPE_UINT16: v1.value.u16 = op v1.value.u16; break; \
  case TYPE_UINT8:  v1.value.u8  = op v1.value.u8;  break; \
  case TYPE_INT32:  v1.value.i32 = op v1.value.i32; break; \
  case TYPE_INT16:  v1.value.i16 = op v1.value.i16; break; \
  case TYPE_INT8:   v1.value.i8  = op v1.value.i8;  break; \
  case TYPE_BOOL:   v1.value.u8  = op v1.value.u8;  break;

#define iunop(op) \
  switch (v1.type) { \
    iucases(op) \
    default: return -1; \
  }

#define ifunop(op) \
  switch (v1.type) { \
    iucases(op) \
    case TYPE_FLOAT: v1.value.d = op v1.value.d; break; \
    default: return -1; \
  }

#define eval_unop(op,bop) \
  pop(p, &v1); \
  bop(op) \
  push(p, &v1);

static int gen_binop(parser_t *p, uint16_t op, type_t *etype, int depth) {
  value_t v1, v2;
  int dofloat = 0, dobool = 0, suffix = 0;
  char *s, sig1, sig2;

  if (p->eval) {
    switch (op) {
      case TOKEN_PLUS:   eval_binop(+,  ifbinop); break;
      case TOKEN_MINUS:  eval_binop(-,  ifbinop); break;
      case TOKEN_TIMES:  eval_binop(*,  ifbinop); break;
      case TOKEN_DIV:    eval_binop(/,  ifbinop); break;
      case TOKEN_MOD:    eval_binop(%,  ibinop);  break;
      case TOKEN_AND:    eval_binop(&&, ifbinop); break;
      case TOKEN_BAND:   eval_binop(&,  ibinop);  break;
      case TOKEN_OR:     eval_binop(||, ifbinop); break;
      case TOKEN_BOR:    eval_binop(|,  ibinop);  break;
      case TOKEN_BXOR:   eval_binop(^,  ibinop);  break;
      case TOKEN_EQ:     eval_binop(==, ifbinop); break;
      case TOKEN_GT:     eval_binop(>,  ifbinop); break;
      case TOKEN_LT:     eval_binop(<,  ifbinop); break;
      case TOKEN_NEQ:    eval_binop(!=, ifbinop); break;
      case TOKEN_GTE:    eval_binop(>=, ifbinop); break;
      case TOKEN_LTE:    eval_binop(<=, ifbinop); break;
      case TOKEN_LSHIFT: eval_binop(<<, ibinop);  break;
      case TOKEN_RSHIFT: eval_binop(>>, ibinop);  break;
    }
  } else {
    switch (op) {
      // accept integer or float arguments
      case TOKEN_PLUS:   s = "add"; dofloat = 1; suffix = 1; break;
      case TOKEN_MINUS:  s = "sub"; dofloat = 1; suffix = 1; break;
      case TOKEN_TIMES:  s = "mul"; dofloat = 1; suffix = 1; break;
      case TOKEN_DIV:    s = "div"; dofloat = 1; suffix = 1; break;
      case TOKEN_EQ:     s = "eq";  dofloat = 1; suffix = 0; break;
      case TOKEN_NEQ:    s = "neq"; dofloat = 1; suffix = 0; break;
      case TOKEN_GT:     s = "gt";  dofloat = 1; suffix = 1; break;
      case TOKEN_GTE:    s = "gte"; dofloat = 1; suffix = 1; break;
      case TOKEN_LT:     s = "lt";  dofloat = 1; suffix = 1; break;
      case TOKEN_LTE:    s = "lte"; dofloat = 1; suffix = 1; break;

      // accept only integer arguments
      case TOKEN_MOD:    s = "mod"; dofloat = 0; dobool = 0; break;
      case TOKEN_BAND:   s = "and"; dofloat = 0; dobool = 0; break;
      case TOKEN_BOR:    s = "or";  dofloat = 0; dobool = 0; break;
      case TOKEN_BXOR:   s = "xor"; dofloat = 0; dobool = 0; break;
      case TOKEN_LSHIFT: s = "lsh"; dofloat = 0; dobool = 0; break;
      case TOKEN_RSHIFT: s = "rsh"; dofloat = 0; dobool = 0; break;
      case TOKEN_AND:    s = "and"; dofloat = 0; dobool = 1; break;
      case TOKEN_OR:     s = "or";  dofloat = 0; dobool = 1; break;
    }

    sig1 = type_signed(etype[depth-1]) ? 'i' : 'u';
    sig2 = type_signed(etype[depth-2]) ? 'i' : 'u';

    if (etype[depth-1] == TYPE_FLOAT || etype[depth-2] == TYPE_FLOAT) {
      if (!dofloat) return syntax_error(p, "operator does not support float arguments");
      if (etype[depth-1] != TYPE_FLOAT) {
        emit(p, "float%c\n", sig1);
      } else if (etype[depth-2] != TYPE_FLOAT) {
        emit(p, "swap\n");
        emit(p, "float%c\n", sig2);
        emit(p, "swap\n");
      }
      emit(p, "%s%s\n", s, suffix ? "f" : "");
      etype[depth-2] = TYPE_FLOAT;
    } else if (sig1 != sig2) {
      if (sig1 == 'u') {
        emit(p, "sign\n");
      } else if (sig2 == 'u') {
        emit(p, "swap\n");
        emit(p, "sign\n");
        emit(p, "swap\n");
      }
      if (dobool) emit(p, "bool\n");
      emit(p, "%s\n", s);
      etype[depth-2] = TYPE_INT32;
    } else {
      if (dobool) emit(p, "bool\n");
      emit(p, "%s\n", s);
      etype[depth-2] = sig1 == 'i' ? TYPE_INT32 : TYPE_UINT32;
    }
  }

  return 0;
}

static int gen_unop(parser_t *p, uint16_t op, type_t *etype, int depth) {
  value_t v1;

  if (p->eval) {
    switch (op) {
      case TOKEN_NOT:  eval_unop(!, ifunop); break;
      case TOKEN_BNOT: eval_unop(~, iunop);  break;
    }
  } else {
    if (etype[depth-1] == TYPE_FLOAT) return syntax_error(p, "operator does not support float argument");

    switch (op) {
      case TOKEN_NOT:
        emit(p, "bool\n");
        emit(p, "not\n");
        etype[depth-1] = TYPE_BOOL;
        break;
      case TOKEN_BNOT:
        emit(p, "not\n");
        break;
    }
  }

  return 0;
}

static symbol_t *find_symbol_any(parser_t *p, char *name) {
  symbol_table_t *lc;
  symbol_t *symbol = NULL;

  lc = p->f ? p->f->st : NULL;

  if (lc == NULL || (symbol = find_symbol(lc, name, SYMBOL_VAR)) == NULL) {
    if ((symbol = find_symbol(p->st, name, SYMBOL_CONST | SYMBOL_STRUCT | SYMBOL_VAR | SYMBOL_FUNCTION)) == NULL) {
      syntax_error(p, "symbol '%s' is not defined", name);
    }
  }

  return symbol;
}

static type_t parse_expr(parser_t *p, int *depth);

static int actual_param(parser_t *p, char *name, type_t param_type, type_t type) {
  if (param_type == TYPE_FLOAT && type != TYPE_FLOAT) {
    emit(p, "float%c\n", type_signed(type) ? 'i' : 'u');
  } else if (param_type != TYPE_FLOAT && type == TYPE_FLOAT) {
    return syntax_error(p, "%s parameter '%s' can not accept float", type_name(param_type), name);
  }

  return 0;
}

static int parse_actual_params(parser_t *p, symbol_function_t *f) {
  type_t type;
  int depth, i;

  emit(p, "pushu8 0\n"); // return value place holder

  for (i = 0; i < f->nparams; i++) {
    if (i > 0) {
      COMMA;
    }
    EXPR(depth);
    if (actual_param(p, f->st->symbol[i].name, f->st->symbol[i].symbol.var.type, type) != 0) return -1;
  }

  return 0;
}

static int function_call(parser_t *p, symbol_function_t *f, int inexpr) {
  emit(p, "pushu16 %u\n", f->param_size - 4);
  emit(p, "link\n");
  emit(p, "call :%s\n", f->name);

  if (!inexpr) {
    emit(p, "drop\n");
  }

  emit(p, "unlink\n");

  return 0;
}

static void emit_addr(parser_t *p, char *name, uint32_t offset, int global) {
  if (global) {
    emit(p, "push32\n");
    emit(p, "d32 :%s\n", name);
  } else {
    emit(p, "faddr 0x%04X\n", offset);
  }
}

static uint32_t ptype_size(parser_t *p, type_t type) {
  if (type < TYPE_STRUCT) {
    return type_size(type);
  }

  if (get_struct(p, type) != NULL) {
    return sizeof(uint32_t);
  }

  return syntax_error(p, "invalid type");
}

static uint32_t struct_size(parser_t *p, type_t type) {
  symbol_struct_t *struc;

  if ((struc = get_struct(p, type)) != NULL) {
    return struc->st->frame_size;
  }

  return syntax_error(p, "invalid type");
}

static void fetch(parser_t *p, type_t type, char *name, uint32_t offset, int global, int array) {
  uint32_t tsize, bsize;
  char *sig;

  tsize = ptype_size(p, type);
  bsize = tsize * 8;
  sig = bsize == 32 ? "" : (type_signed(type) ? "i" : "u");

  if (array && tsize > 1) {
    emit(p, "pushu8 %u\n", tsize);
    emit(p, "mul\n");
  }

  emit_addr(p, name, offset, global);

  if (array) {
    emit(p, "fetch32\n");
    emit(p, "add\n");
  }

  emit(p, "fetch%s%u\n", sig, bsize);
}

/*
static void fetch_field(parser_t *p, symbol_t *symbol, uint32_t offset, type_t vtype, type_t type) {
  symbol_var_t *var;
  uint32_t tsize, bsize;
  char *sig;

  var = &symbol->symbol.var;
  emit_addr(p, symbol->name, var->offset, symbol->st == p->st);
  emit(p, "fetch32\n");

  if (offset) {
    emit(p, "push32\n");
    emit(p, "d32 %u\n", offset);
    emit(p, "add\n");
  }

  tsize = ptype_size(p, vtype);
  bsize = tsize * 8;
  sig = bsize == 32 ? "" : (type_signed(vtype) ? "i" : "u");
  emit(p, "fetch%s%u\n", sig, bsize);
}

static int parse_dotlist(parser_t *p, symbol_t *symbol, uint32_t *offset, uint32_t *type) {
  symbol_struct_t *struc;
  char *field;

  for (*offset = 0;;) {
    if (symbol->symbol.var.type < TYPE_STRUCT) return syntax_error(p, "'%s' is not a struct", symbol->name);
    IDENT(field);
    *type = symbol->symbol.var.type;

    if ((struc = get_struct(p, *type)) == NULL) return syntax_error(p, "struct not found");
    if ((symbol = find_symbol(struc->st, field, SYMBOL_VAR)) == NULL) {
      return syntax_error(p, "field '%s' not defined in struct '%s'", field, struc->name);
    }
    *offset += symbol->symbol.var.offset;
    //fprintf(stderr, "field '%s' at offset %u on struct '%s', total offset %u\n", field, symbol->symbol.var.offset, struc->name, *offset);
    if (opt(DOT)) continue;

    BACK;
    break;
  }

  if (symbol->symbol.var.type >= TYPE_STRUCT) return syntax_error(p, "'%s' is a struct", symbol->name);

  return 0;
}
*/

static void emit_index(parser_t *p, type_t type) {
  uint32_t size;

  size = ptype_size(p, type);
  if (size > 1) {
    emit(p, "pushu8 %u\n", size);
    emit(p, "mul\n");
  }
  emit(p, "add\n");
}

static int parse_index(parser_t *p, symbol_var_t *var) {
  type_t type;
  int depth;

  if (opt(OPENSB)) {
    //fprintf(stderr, "parsing index\n");
    if (!var->array) return syntax_error(p, "variable is not an array");
    EXPR(depth);
    if (!type_integer(type)) return syntax_error(p, "array index must be integer");
    CLOSESB;
    emit_index(p, var->type);
  } else {
    BACK;
  }

  return 0;
}

static int parse_const(parser_t *p, symbol_t *symbol) {
  symbol_const_t *cons;
  value_t value;
  type_t type;
  int depth;

  cons = &symbol->symbol.cons;
  if (opt(OPENSB)) {
    //fprintf(stderr, "parsing index\n");
    if (!cons->array) return syntax_error(p, "constant is not an array");
    EXPR(depth);
    CLOSESB;
    pop(p, &value);
    if (!type_integer(value.type)) return syntax_error(p, "array index must be integer");
    type_coerce(&value, TYPE_INT32, &value, 0);
  } else {
    BACK;
  }

  if (p->eval) {
    get_value(cons->type, cons->value, 0, &value);
    push(p, &value);
  } else {
    fetch(p, cons->type, symbol->name, 0, 1, 0);
  }

  return 0;
}

static int parse_address(parser_t *p, symbol_t *symbol, symbol_var_t **r, int *global) {
  symbol_var_t *var, *v;
  symbol_struct_t *struc;
  char *field_name;

  if (symbol->type != SYMBOL_VAR) return syntax_error(p, "'%s' is not a variable", symbol->name);
  var = &symbol->symbol.var;
  emit_addr(p, symbol->name, var->offset, symbol->st == p->st);
  if (parse_index(p, var) == -1) return -1;
  *global = symbol->st == p->st;

  v = var;
  for (;;) {
    if (!opt(DOT)) {
      BACK;
      break;
    }
    if (v->type < TYPE_STRUCT) return syntax_error(p, "not a struct");
    if ((struc = get_struct(p, v->type)) == NULL) return syntax_error(p, "struct not found");
    //fprintf(stderr, "struct '%s'\n", struc->name);
    IDENT(field_name);
    //fprintf(stderr, "resolving field '%s'\n", field_name);
    if ((symbol = find_symbol(struc->st, field_name, SYMBOL_VAR)) == NULL) {
      return syntax_error(p, "field '%s' not defined in struct '%s'", field_name, struc->name);
    }
    v = &symbol->symbol.var;
    if (v->offset) {
      emit(p, "pushu16 %u\n", v->offset);
      emit(p, "add\n");
    }
    if (parse_index(p, v) == -1) return -1;
  }

  //fprintf(stderr, "resolved: type %s%s, offset %u\n", type_name(v->type), v->array ? " (array)" : "", v->offset);
  *r = v;
  return 0;
}

static type_t parse_expr(parser_t *p, int *depth) {
  symbol_t *symbol;
  symbol_var_t *var;
  symbol_const_t *cons;
  char *name, *sig;
  uint32_t tsize;
  uint16_t t;
  int global, s, i;
  type_t etype[MAX_STACK];

  for (s = 0, i = 0; i < MAX_STACK; i++) {
    t = parser_next(p);

    switch (s) {
      case 0:
        switch (t) {
          booln;
          integer(INTEGER, 10, 1);
          integer(UINTEGER, 10, 0);
          integer(HEXA, 16, 0);
          integer(OCTAL, 8, 0);
          integer(BINARY, 8, 0);
          floatn;

          binop(TIMES);
          binop(DIV);
          binop(MOD);
          binop(PLUS);
          binop(MINUS);
          binop(AND);
          binop(OR);
          binop(BAND);
          binop(BOR);
          binop(BXOR);
          binop(LSHIFT);
          binop(RSHIFT);
          binop(EQ);
          binop(NEQ);
          binop(LT);
          binop(LTE);
          binop(GT);
          binop(GTE);

          unop(NOT);
          unop(BNOT);

          case TOKEN_STRING:
            name = getvalue;
            etype[*depth] = gen_string(p, name);
            (*depth)++;
            break;

          case TOKEN_IDENT:
            BACK;
            IDENT(name);
            if ((symbol = find_symbol_any(p, name)) == NULL) return -1;
            if (symbol->type == SYMBOL_CONST) {
              if (parse_const(p, symbol) != 0) return -1;
              cons = &symbol->symbol.cons;
              etype[*depth] = cons->type;
              (*depth)++;
              break;
            }
            if (p->eval) {
              if (symbol->type == SYMBOL_VAR && symbol->symbol.var.value.type) {
                var = &symbol->symbol.var;
                push(p, &var->value);
                etype[*depth] = var->type;
                (*depth)++;
                break;
              }
              return syntax_error(p, "expression is not constant");
            }
            if (parse_address(p, symbol, &var, &global) != 0) return -1;
            tsize = ptype_size(p, var->type) * 8;
            sig = tsize == 32 ? "" : (type_signed(var->type) ? "i" : "u");
            emit(p, "fetch%s%u\n", sig, tsize);
            etype[*depth] = var->type;
            (*depth)++;
            break;

          case TOKEN_RANGE:
          case TOKEN_COMMA:
          case TOKEN_CLOSEP:
          case TOKEN_CLOSEB:
          case TOKEN_CLOSESB:
            BACK;
            if (*depth == 0) return syntax_error(p, "empty expression");
            if (*depth != 1) return syntax_error(p, "too many arguments on stack (%d)", *depth);
            return etype[0];
        }
        break;
/*
      case 1:
        switch (t) {
          case TOKEN_OPENP:
            // function call
            if (p->eval) return syntax_error(p, "can not use function calls in constant expression");
            if (symbol->type != SYMBOL_FUNCTION) return syntax_error(p, "'%s' is not a function", symbol->name);
            ACTUAL_PARAMS(&symbol->symbol.function);
            CLOSEP;
            if (function_call(p, &symbol->symbol.function, 1) != 0) return -1;
            etype[*depth] = symbol->symbol.function.type;
            break;
          default:
            // scalar variable or constant
            BACK;
            switch (symbol->type) {
              case SYMBOL_VAR:
                if (p->eval) return syntax_error(p, "can not use variables in constant expression");
                var = &symbol->symbol.var;
                if (var->array) return syntax_error(p, "'%s' is an array", symbol->name);
                fetch(p, var->type, symbol->name, var->offset, symbol->st == p->st, 0);
                etype[*depth] = var->type;
                break;
              case SYMBOL_CONST:
                cons = &symbol->symbol.cons;
                if (cons->array) return syntax_error(p, "'%s' is an array", symbol->name);
                if (p->eval) {
                  get_value(cons->type, cons->value, 0, &value);
                  push(p, &value);
                } else {
                  fetch(p, cons->type, symbol->name, 0, 1, 0);
                }
                etype[*depth] = cons->type;
                break;
              default:
                return syntax_error(p, "'%s' is not a variable or constant", symbol->name);
            }
          }
          (*depth)++;
          s = 0;
          break;
*/
    }
  }

  return syntax_error(p, "unreachable");
}

static int return_expr(parser_t *p, type_t type) {
  symbol_t *symbol;
  symbol_var_t *var;
  uint32_t i;

  if (p->f->type == TYPE_FLOAT && type != TYPE_FLOAT) {
    emit(p, "float%c\n", type_signed(type) ? 'i' : 'u');
  } else if (p->f->type != TYPE_FLOAT && type == TYPE_FLOAT) {
    return syntax_error(p, "%s function '%s' can not return float", type_name(p->f->type), p->f->name);
  }

  if ((symbol = find_symbol(p->f->st, "_", SYMBOL_VAR)) != NULL) {
    emit(p, "faddr 0x%04X\n", symbol->symbol.var.offset);
    emit(p, "store32\n");
  }

  for (i = p->f->nparams; i < p->f->st->num_symbols; i++) {
    if (p->f->st->symbol[i].type == SYMBOL_VAR) {
      var = &p->f->st->symbol[i].symbol.var;
      if (var->type >= TYPE_STRUCT) {
        // decrement reference count for local variables of type struct
        emit(p, "faddr 0x%04X\n", var->offset);
        emit(p, "fetch32\n");
        emit(p, "hdec\n");
      }
    }
  }

  return 0;
}

static void struct_alloc(parser_t *p, symbol_var_t *f) {
  //fprintf(stderr, "alloc struct (%u bytes)\n", struct_size(p, f->type));
  emit(p, "push32\n");
  emit(p, "d32 %u\n", struct_size(p, f->type));
  emit(p, "new\n");
  emit(p, "swap\n");
  emit(p, "store32\n");
}

static void array_alloc(parser_t *p, symbol_var_t *f) {
  uint32_t tsize;

  tsize = ptype_size(p, f->type);
  //fprintf(stderr, "alloc array (element type %u bytes)\n", tsize);
  if (tsize > 1) {
    emit(p, "pushu8 %u\n", tsize);
    emit(p, "mul\n");
  }
  emit(p, "new\n");
  emit(p, "swap\n");
  emit(p, "store32\n");
}

static int check_types(parser_t *p, type_t vtype, type_t type) {
  if (type_integer(vtype) && !type_integer(type)) {
    return syntax_error(p, "type mismatch (%s, %s)", type_name(vtype), type_name(type));
  }
  if (vtype == TYPE_FLOAT && !type_numeric(type)) {
    return syntax_error(p, "type mismatch (%s, %s)", type_name(vtype), type_name(type));
  }
  if (vtype == TYPE_STRING && type != TYPE_STRING) {
    return syntax_error(p, "type mismatch (%s, %s)", type_name(vtype), type_name(type));
  }
  if (vtype >= TYPE_STRUCT && type < TYPE_STRUCT) {
    return syntax_error(p, "not a struct");
  }
  if (vtype >= TYPE_STRUCT && type != vtype) {
    return syntax_error(p, "struct mismatch");
  }

  return 0;
}

static int assign(parser_t *p, symbol_var_t *f, type_t type) {
  uint32_t size;
  char *sign;

  //fprintf(stderr, "assign type %s to variable type %s\n", type_name(type), type_name(f->type));
  if (check_types(p, f->type, type) != 0) return -1;

  if (type == TYPE_FLOAT) {
    emit(p, "float%c\n", type_signed(type) ? 'i' : 'u');
    sign = "";
  } else if (type >= TYPE_STRUCT) {
    // vaddr hnewaddr
    emit(p, "over\n");
    // vaddr hnewaddr vaddr
    emit(p, "fetch32\n");
    // vaddr hnewaddr holdaddr
    emit(p, "hdec\n"); // decrement reference count for old address
    // vaddr hnewaddr
    emit(p, "dup\n");
    // vaddr hnewaddr hnewaddr
    emit(p, "hinc\n"); // increment reference count for new address
    // vaddr hnewaddr
    sign = "";
  } else {
    size = ptype_size(p, type);
    sign = size < 4 ? (type_signed(type) ? "i" : "u") : "";
  }

  emit(p, "swap\n");
  emit(p, "store%s%u\n", sign, ptype_size(p, f->type) * 8);

  return 0;
}

static void emit_compare(parser_t *p, type_t expr_type, value_t *value) {
  uint32_t u;
  int32_t i;

  emit(p, "dup\n");

  if (type_signed(expr_type)) {
    i = value->value.i32;
    if (i >= -0x80 && i < 0x7F) {
      emit(p, "pushi8 %d\n", i);
    } else if (i >= -0x8000 && i < 0x7FFF) {
      emit(p, "pushi16 %d\n", i);
    } else {
      emit(p, "push32\n");
      emit(p, "d32 %d\n", i);
    }
  } else {
    u = value->value.u32;
    if (u < 0x100) {
      emit(p, "pushu8 %u\n", u);
    } else if (u < 0x10000) {
      emit(p, "pushu16 %u\n", u);
    } else {
      emit(p, "push32\n");
      emit(p, "d32 %d\n", u);
    }
  }
}

static int parse_statements(parser_t *p, token_e *last, uint32_t pre_loop, uint32_t post_loop);

static int parse_caselist(parser_t *p, type_t expr_type, uint32_t end_label, uint32_t pre_loop, uint32_t post_loop) {
  value_t value1, value2;
  uint32_t label;
  int depth, type, range;
 
  for (;;) {
    if (opt(ELSE)) {
      BLOCK;
      break;
    }
    BACK;
    CASE;
    OPENP;
    p->eval = 1;
    EXPR(depth);
    p->eval = 0;
    pop(p, &value1);
    if (!type_integer(value1.type)) return syntax_error(p, "case expression must be integer");
    if (!type_contains(expr_type, value1.type)) return syntax_error(p, "case expression incompatible with switch expression");
    if (opt(RANGE)) {
      p->eval = 1;
      EXPR(depth);
      p->eval = 0;
      pop(p, &value2);
      if (!type_integer(value2.type)) return syntax_error(p, "case expression must be integer");
      if (!type_contains(expr_type, value2.type)) return syntax_error(p, "case expression incompatible with switch expression");
      range = 1;
    } else {
      BACK;
      range = 0;
    }
    CLOSEP;
    type_coerce(&value1, expr_type, &value1, 0);
    emit_compare(p, expr_type, &value1);
    label = next_label(p);
    if (range) {
      emit(p, "gte\n");
      emit(p, "jz :%u\n", label);
      type_coerce(&value2, expr_type, &value2, 0);
      emit_compare(p, expr_type, &value2);
      emit(p, "lte\n");
      emit(p, "jz :%u\n", label);
    } else {
      emit(p, "eq\n");
      emit(p, "jz :%u\n", label);
    }
    BLOCK;
    emit(p, "jp :%u\n", end_label);
    emit(p, ":%u\n", label);
  }

  return 0;
}

// assign:
//   LET var [ dotlist ] OPENP EXPR CLOSEP |
//   LET var [ dotlist ] NEW [ OPENP EXPR CLOSEP ] ;
// var:
//   IDENT |
//   IDENT OPENSB EXPR CLOSESB ;
// dotlist:
//   { DOT field } ;
// field:
//   IDENT |
//   IDENT OPENSB EXPR CLOSESB ;

static int parse_statements(parser_t *p, token_e *last, uint32_t pre_loop, uint32_t post_loop) {
  symbol_t *symbol;
  symbol_var_t *var;
  type_t type, expr_type;
  char *name;
  uint32_t label1, label2;
  int depth, global;

  if (last) *last = 0;
  for (;;) {
    switch (next()) {
      case TOKEN_CLOSEB:
        BACK;
        return 0;
      case TOKEN_PRINT:
        OPENP;
        EXPR(depth);
        CLOSEP;
        switch (type) {
          case TYPE_UINT8:  emit(p, "printu8\n");  break;
          case TYPE_INT8:   emit(p, "printi8\n");  break;
          case TYPE_UINT16: emit(p, "printu16\n"); break;
          case TYPE_INT16:  emit(p, "printi16\n"); break;
          case TYPE_UINT32: emit(p, "printi32\n"); break;
          case TYPE_INT32:  emit(p, "printu32\n"); break;
          case TYPE_FLOAT:  emit(p, "printf\n");   break;
          case TYPE_STRING: emit(p, "prints\n");   break;
          default: return syntax_error(p, "invalid expression type for print");
        }
        break;
      case TOKEN_LET:
        if (last) *last = TOKEN_LET;
        IDENT(name);
        if ((symbol = find_symbol_any(p, name)) == NULL) return -1;
        if (parse_address(p, symbol, &var, &global) != 0) return -1;

        switch (next()) {
          case TOKEN_OPENP:
            EXPR(depth);
            expr_type = type;
            CLOSEP;
            if (assign(p, var, expr_type) != 0) return -1;
            break;
          case TOKEN_NEW:
            if (opt(OPENP)) {
              if (!var->array) return syntax_error(p, "not an array");
              EXPR(depth);
              expr_type = type;
              CLOSEP;
              if (!type_integer(expr_type)) return syntax_error(p, "array size must be integer");
              array_alloc(p, var);
            } else {
              BACK;
              if (var->type < TYPE_STRUCT) return syntax_error(p, "not a struct");
              struct_alloc(p, var);
            }
            break;
          default:
            return syntax_error(p, "invalid LET statement");
        }
        break;
      case TOKEN_IDENT:
        name = getvalue;
        if ((symbol = find_symbol_any(p, name)) == NULL) return -1;
        if (symbol->type != SYMBOL_FUNCTION) return syntax_error(p, "'%s' is not a function", symbol->name);
        OPENP;
        ACTUAL_PARAMS(&symbol->symbol.function);
        CLOSEP;
        if (function_call(p, &symbol->symbol.function, 0) != 0) return -1;
        break;
      case TOKEN_RETURN:
        if (last) *last = TOKEN_RETURN;
        OPENP;
        EXPR(depth);
        expr_type = type;
        CLOSEP;
        if (return_expr(p, expr_type) != 0) return -1;
        emit(p, "jp :%u\n", p->f->end_label);
        break;
      case TOKEN_IF:
        if (last) *last = TOKEN_IF;
        CONDITION;
        emit(p, "bool\n");
        label1 = next_label(p);
        emit(p, "jz :%u\n", label1);
        BLOCK;
        if (opt(ELSE)) {
          label2 = next_label(p);
          emit(p, "jp :%u\n", label2);
          emit(p, ":%u\n", label1);
          BLOCK;
          emit(p, ":%u\n", label2);
        } else {
          BACK;
          emit(p, ":%u\n", label1);
        }
        break;
      case TOKEN_LOOP:
        if (last) *last = TOKEN_LOOP;
        label1 = next_label(p);
        label2 = next_label(p);
        emit(p, ":%u\n", label1);
        OPENB;
        STATEMENTS(NULL, label1, label2);
        CLOSEB;
        emit(p, "jp :%u\n", label1);
        emit(p, ":%u\n", label2);
        break;
      case TOKEN_BREAK:
        if (!post_loop) return syntax_error(p, "break outside of loop");
        if (opt(OPENP)) {
          BACK;
          CONDITION;
          emit(p, "bool\n");
          emit(p, "not\n");
          emit(p, "jz :%u\n", post_loop);
        } else {
          emit(p, "jp :%u\n", post_loop);
        }
        break;
      case TOKEN_CONTINUE:
        if (!pre_loop) return syntax_error(p, "continue outside of loop");
        if (opt(OPENP)) {
          BACK;
          CONDITION;
          emit(p, "bool\n");
          emit(p, "not\n");
          emit(p, "jz :%u\n", pre_loop);
        } else {
          emit(p, "jp :%u\n", pre_loop);
        }
        break;
      case TOKEN_SWITCH:
        if (last) *last = TOKEN_SWITCH;
        CONDITION;
        expr_type = type;
        OPENB;
        label1 = next_label(p);
        if (parse_caselist(p, expr_type, label1, pre_loop, post_loop) != 0) return -1;
        CLOSEB;
        emit(p, ":%u\n", label1);
        emit(p, "drop\n");
        break;
      default:
        return syntax_error(p, "invalid statement");
    }
  }

  return syntax_error(p, "unreachable");
}

static int check_constant(parser_t *p, type_t type, char *name, value_t *v, value_t *r, int isvar) {
  uint32_t u;
  int32_t i;
  int ok;
  char *s = isvar ? "variable" : "constant";

  if ((type_integer(type) && !type_integer(v->type)) ||
      (type_signed(type) && !type_signed(v->type)) ||
      (type_unsigned(type) && !type_unsigned(v->type)) ||
      (type == TYPE_FLOAT && !type_numeric(v->type)) ||
      (type == TYPE_STRING && v->type != TYPE_STRING)) {
    return syntax_error(p, "%s '%s' of type %s can not receive type %s", s, name, type_name(type), type_name(v->type));
  }

  if (type_signed(type)) {
    type_coerce(r, TYPE_INT32, v, 0);
    i = r->value.i32;
    ok = 1;
    switch (type) {
      case TYPE_INT8:
        if (i < -0x80 || i > 0x7f) ok = 0;
        else type_coerce(r, type, v, 0);
        break;
      case TYPE_INT16:
        if (i < -0x8000 || i > 0x7fff) ok = 0;
        else type_coerce(r, type, v, 0);
        break;
      default:
        break;
    }
    if (!ok) return syntax_error(p, "value %d is out of range for type %s in %s '%s'", i, type_name(type), s, name);

  } else if (type_unsigned(type)) {
    type_coerce(r, TYPE_UINT32, v, 0);
    u = r->value.u32;
    ok = 1;
    switch (type) {
      case TYPE_UINT8:
        if (u > 0xff) ok = 0;
        else type_coerce(r, type, v, 0);
        break;
      case TYPE_UINT16:
        if (u > 0xffff) ok = 0;
        else type_coerce(r, type, v, 0);
        break;
      default:
        break;
    }
    if (!ok) return syntax_error(p, "value %u is out of range for type %s in %s '%s'", u, type_name(type), s, name);

  } else if (type == TYPE_FLOAT) {
    type_coerce(r, TYPE_FLOAT, v, 0);
  } else if (type == TYPE_STRING) {
    r->type = TYPE_STRING;
    r->value.s = v->value.s;
  }

  return 0;
}

static void emit_data(parser_t *p, value_t *v) {
  switch (v->type) {
    case TYPE_INT8:
    case TYPE_UINT8:
      emit(p, "d8 0x%02X\n", v->value.u8);
      break;
    case TYPE_INT16:
    case TYPE_UINT16:
      emit(p, "d16 0x%04X\n", v->value.u16);
      break;
    case TYPE_INT32:
    case TYPE_UINT32:
    case TYPE_FLOAT:
      emit(p, "d32 0x%08X\n", v->value.u32);
      break;
    case TYPE_STRING:
      add_new_string(p, v->value.s);
      break;
    default:
      break;
  }
}

static int add_new_const(parser_t *p, type_t const_type, char *name, value_t *v) {
  value_t value;
  symbol_t *s;
  uint32_t tsize;
  int r = -1;

  info(p, "Generating constant '%s'", name);
  if (check_constant(p, const_type, name, v, &value, 0) != 0) return -1;

  if ((s = new_symbol(p, p->st, name, SYMBOL_CONST)) != NULL) {
    tsize = ptype_size(p, const_type);
    s->symbol.cons.type = const_type;
    s->symbol.cons.value = sys_calloc(1, tsize);
    set_value(s->symbol.cons.value, 0, &value);
    emit(p, ":%s\n", s->name);
    emit_data(p, &value);
    r = 0;
  }

  return r;
}

static int add_new_const_array(parser_t *p, type_t const_type, char *name, uint32_t size, uint32_t n, value_t *v) {
  value_t value;
  symbol_t *s;
  uint32_t tsize, id, i;

  info(p, "Generating array constant '%s'", name);

  if ((s = new_symbol(p, p->st, name, SYMBOL_CONST)) != NULL) {
    tsize = ptype_size(p, const_type);
    s->symbol.cons.type = const_type;
    s->symbol.cons.array = size;
    s->symbol.cons.value = sys_calloc(size, tsize);

    for (i = 0; i < n; i++) {
      if (check_constant(p, const_type, name, &v[i], &value, 0) != 0) return -1;
      set_value(s->symbol.cons.value, i, &value);
    }

    emit(p, ":%s\n", s->name);
    id = next_label(p);
    emit(p, "d32 :%u\n", id);
    emit(p, ":%u\n", id);
    for (i = 0; i < size; i++) {
      get_value(const_type, s->symbol.cons.value, i, &value);
      emit_data(p, &value);
    }
  }

  return 0;
}

static int add_new_var(parser_t *p, symbol_table_t *st, type_t var_type, char *name, value_t *v) {
  value_t value;
  symbol_t *s;
  symbol_struct_t *struc;
  uint32_t tsize;
  int r = -1;

  if (st == p->st) {
    info(p, "Generating variable '%s'", name);
  }

  if (v) {
    if (check_constant(p, var_type, name, v, &value, 1) != 0) return -1;
    info(p, "Variable '%s' has initial value", name);
  }

  if ((s = new_symbol(p, st, name, SYMBOL_VAR)) != NULL) {
    s->symbol.var.type = var_type;
    s->symbol.var.array = 0;
    tsize = ptype_size(p, var_type);
    s->symbol.var.offset = type_align(tsize, st->frame_size);
    st->frame_size = s->symbol.var.offset + tsize;
    //fprintf(stderr, "var '%s' offset %u\n", name, s->symbol.var.offset);

    if (v) {
      sys_memcpy(&s->symbol.var.value, &value, sizeof(value_t));
    }

    if (var_type >= TYPE_STRUCT) {
      struc = get_struct(p, var_type);
      s->symbol.var.st = copy_symbol_table(p, struc->st);
    }
    r = 0;
  }

  return r;
}

static int add_new_var_array(parser_t *p, symbol_table_t *st, type_t var_type, char *name, int32_t size) {
  symbol_t *s;
  uint32_t tsize;
  int r = -1;

  if (st == p->st) {
    info(p, "Generating array variable '%s'", name);
  }

  if ((s = new_symbol(p, st, name, SYMBOL_VAR)) != NULL) {
    s->symbol.var.type = var_type;
    s->symbol.var.array = size;
    tsize = ptype_size(p, var_type);
    s->symbol.var.offset = type_align(tsize, st->frame_size);
    if (s->symbol.var.array == -1) {
      st->frame_size = s->symbol.var.offset + 4;
    } else {
      st->frame_size = s->symbol.var.offset + size * tsize;
    }
    //fprintf(stderr, "var array '%s' offset %u\n", name, s->symbol.var.offset);
    r = 0;
  }

  return r;
}

static int parse_formal_params(parser_t *p, symbol_table_t *lt) {
  type_t type;
  char *name;
  int i;

  for (i = 0;; i++) {
    if (opt(CLOSEP)) {
      BACK;
      return i;
    }
    BACK;
    if (i > 0) {
      COMMA;
    }
    TYPE(type);
    IDENT(name);
    add_new_var(p, lt, type, name, NULL);
  }

  return i;
}

static int parse_local_vars(parser_t *p, symbol_table_t *lt) {
  type_t vtype;
  value_t value;
  int type, array, depth, hasvalue;
  char *name;

  p->param_size = lt->frame_size;
  lt->frame_size += 3*4; // reserve space for param size, saved fp, and return addr

  for (;;) {
    if (!opt(VAR)) {
      BACK;
      return 0;
    }
    TYPE(vtype);
    IDENT(name);
    hasvalue = 0;

    if (opt(OPENSB)) {
      if (opt(CLOSESB)) {
        array = -1;
      } else {
        BACK;
        p->eval = 1;
        EXPR(depth);
        p->eval = 0;
        CLOSESB;
        pop(p, &value);
        if (!type_integer(value.type)) return syntax_error(p, "array size in variable '%s' must be integer", name);
        array = value.type == TYPE_BOOL ? value.value.b : value.value.i32;
        if (array <= 0) return syntax_error(p, "invalid array size %d in variable '%s'", array, name);
      }
    } else {
      BACK;

      if (opt(OPENP)) {
        p->eval = 1;
        EXPR(depth);
        p->eval = 0;
        CLOSEP;
        pop(p, &value);
        hasvalue = 1;
      } else {
        BACK;
      }

      array = 0;
    }

    if (array) {
      add_new_var_array(p, lt, vtype, name, array);
    } else {
      if (add_new_var(p, lt, vtype, name, hasvalue ? &value : NULL) != 0) return -1;
    }
  }

  return 0;
}

static void add_new_function(parser_t *p, symbol_table_t *lt, type_t type, char *name, int nparams) {
  symbol_t *s;
  symbol_var_t *var;
  uint32_t locals_size, size, i;
  char *sign;

  info(p, "Generating function '%s'", name);

  for (i = 0; i < lt->num_symbols; i++) {
    var = &lt->symbol[i].symbol.var;
    if (var->array == -1) {
      size = 4;
    } else {
      size = ptype_size(p, var->type);
      if (var->array) size *= var->array;
    }
    var->offset = lt->frame_size - var->offset - size;
    //fprintf(stderr, "symbol %u: '%s' offset %u\n", i, lt->symbol[i].name, var->offset);
  }

  if ((s = new_symbol(p, p->st, name, SYMBOL_FUNCTION)) != NULL) {
    s->symbol.function.type = type;
    s->symbol.function.name = name;
    s->symbol.function.nparams = nparams;
    s->symbol.function.param_size = p->param_size;
    s->symbol.function.st = lt;
    s->symbol.function.end_label = next_label(p);
    p->f = &s->symbol.function;
    emit(p, ":%s\n", s->name);

    locals_size = lt->frame_size - p->f->param_size - 12;
    if (locals_size) {
      emit(p, "frame %u\n", locals_size);

      for (i = 0; i < lt->num_symbols; i++) {
         var = &lt->symbol[i].symbol.var;
         if (var->value.type && !var->array) {
           switch (var->type) {
             case TYPE_BOOL:   emit(p, "pushu8 %d\n", var->value.value.b); break;
             case TYPE_INT8:   emit(p, "pushi8 %d\n", var->value.value.i8); break;
             case TYPE_UINT8:  emit(p, "pushu8 %u\n", var->value.value.u8); break;
             case TYPE_INT16:  emit(p, "pushi16 %d\n", var->value.value.i16); break;
             case TYPE_UINT16: emit(p, "pushu16 %u\n", var->value.value.u16); break;
             case TYPE_INT32:  emit(p, "push32\n"); emit(p, "d32 %d\n", var->value.value.i32); break;
             case TYPE_UINT32: emit(p, "push32\n"); emit(p, "d32 %u\n", var->value.value.u32); break;
             case TYPE_FLOAT:  emit(p, "push32\n"); emit(p, "d32 0x%08X ; float %.8f\n", var->value.value.u32, var->value.value.d); break;
             default: break; // not possible
           }
           emit(p, "faddr 0x%04X\n", var->offset);
           size = ptype_size(p, var->type);
           sign = size < 4 ? (type_signed(var->type) ? "i" : "u") : "";
           emit(p, "store%s%u\n", sign, ptype_size(p, var->type) * 8);
         }
      }
    }
  }
}

static int add_symbols(parser_t *p, char *module, uint8_t *symb, uint32_t size, uint8_t *code, uint32_t code_size) {
  buffer_t buffer;
  symbol_t *s;
  char *name;
  uint32_t addr, type, array, nparams, *param, tsize;
  int i, r = -1;

  buffer.size = size;
  buffer.alloc_size = size;
  buffer.i = 0;
  buffer.buf = symb;

  for (; buffer.i < buffer.size;) {
    switch (get32(&buffer)) {
      case SYM_CONS:
        addr = get32(&buffer);
        type = get32(&buffer);
        array = get32(&buffer);
        name = get_string(&buffer);
        info(p, "Importing constant '%s' from module '%s'", name, module);

        if ((s = new_symbol(p, p->st, name, SYMBOL_CONST)) != NULL) {
          tsize = ptype_size(p, type);
          s->symbol.cons.type = type;
          s->symbol.cons.array = array;
          s->symbol.cons.external = 1;
          if (array == 0) array = 1;
          s->symbol.cons.value = sys_calloc(array, tsize);
          sys_memcpy(s->symbol.cons.value, &code[addr], array * tsize);
          r = 0;
        }
        //free(name);
        break;

      case SYM_FUNC:
        addr = get32(&buffer);
        type = get32(&buffer);
        nparams = get32(&buffer);
        if (nparams) {
          param = sys_calloc(nparams, sizeof(uint32_t));
          for (i = 0; i < nparams; i++) {
            param[i] = get32(&buffer);
          }
        } else {
          param = NULL;
        }
        name = get_string(&buffer);
        info(p, "Importing function '%s' from module '%s'", name, module);

        if ((s = new_symbol(p, p->st, name, SYMBOL_FUNCTION)) != NULL) {
          s->symbol.function.type = type;
          s->symbol.function.name = name;
          s->symbol.function.nparams = nparams;
          s->symbol.function.st = new_symbol_table(0);
          s->symbol.function.external = 1;

          for (i = 0; i < nparams; i++) {
            if ((s = new_symbol(p, s->symbol.function.st, "", SYMBOL_VAR)) != NULL) {
              s->symbol.var.type = param[i];
            }
          }

          emit(p, ":%s\n", name);
          emit(p, "jpa #%s\n", name);
          r = 0;
        }

        if (param) sys_free(param);
        //free(name);
        break;

      default:
        syntax_error(p, "invalid symbol type in module '%s'", name);
        break;
    }
  }

  return r;
}

static int add_import(parser_t *p, char *name) {
  char obj[MAX_TOKEN + 8], asmm[MAX_TOKEN + 8], src[MAX_TOKEN + 8];
  uint8_t *symb, *code;
  uint32_t symb_size, code_size;
  PLIBC_FILE *f;
  int r = -1;

  info(p, "Importing module '%s'", name);
  sys_snprintf(obj, sizeof(obj)-1, "%s.o", name);

  if ((f = plibc_fopen(1, obj, "rb")) == NULL) {
    sys_snprintf(src, sizeof(src)-1, "%s.fc", name);
    sys_snprintf(asmm, sizeof(asmm)-1, "%s.s", name);
    if (compile(src, asmm) != 0) return -1;
    info(p, "Assembling file '%s' into '%s'", asmm, obj);

    if (assembler_assemble(asmm, obj) != 0) {
      return -1;
    }

    if ((f = plibc_fopen(1, obj, "rb")) == NULL) {
      return syntax_error(p, "error importing module \"%s\"", name);
    }
  }

  symb = section_read(SEC_SYMB, name, &symb_size, f);
  code = section_read(SEC_CODE, name, &code_size, f);
  plibc_fclose(f);

  if (symb && code) {
    r = add_symbols(p, name, symb, symb_size, code, code_size);
  }
  if (symb) sys_free(symb);
  if (code) sys_free(code);

  return r;
}

static int parse_fieldlist(parser_t *p, symbol_table_t *lt) {
  type_t type;
  char *name;

  for (;;) {
    TYPE(type);
    IDENT(name);
    if (find_symbol(lt, name, SYMBOL_VAR)) {
      syntax_error(p, "field '%s' already defined", name);
      return -1;
    }
    add_new_var(p, lt, type, name, NULL);
    if (opt(CLOSEB)) {
      BACK;
      break;
    }
    BACK;
  }

  return 0;
}

static int parser_parse(parser_t *p) {
  value_t value, *varray;
  symbol_table_t *lt;
  symbol_t *struc;
  type_t vtype;
  token_e last;
  uint32_t locals_size;
  int32_t array, count;
  char *name;
  int type, depth, nparams, hasvalue, section = 0;

  for (;;) {
    switch (next()) {
      case TOKEN_IMPORT:
        insection(0);
        IDENT(name);
        if (add_import(p, name) != 0) return -1;
        break;
      case TOKEN_CONST:
        insection(1);
        TYPE(vtype);
        if (type >= TYPE_STRUCT) return syntax_error(p, "structs can not be constants");
        IDENT(name);
        p->eval = 1;
        if (opt(OPENSB)) {
          EXPR(depth);
          CLOSESB;
          pop(p, &value);
          if (!type_integer(value.type)) return syntax_error(p, "array size in constant '%s' must be integer", name);
          array = value.type == TYPE_BOOL ? value.value.b : value.value.i32;
          if (array <= 0) return syntax_error(p, "invalid array size %d in constant '%s'", array, name);
          varray = sys_calloc(array, sizeof(value_t));
          OPENP;
          for (count = 0;;) {
            if (opt(CLOSEP)) break;
            if (count == array) return syntax_error(p, "too many elements in array constant '%s'", name);
            BACK;
            EXPR(depth);
            pop(p, &varray[count]);
            count++;
            if (opt(COMMA)) continue;
            BACK;
          }
          if (add_new_const_array(p, vtype, name, array, count, varray) != 0) return -1;
        } else {
          BACK;
          array = 0;
          varray = NULL;
          BACK;
          OPENP;
          EXPR(depth);
          CLOSEP;
          pop(p, &value);
          if (add_new_const(p, vtype, name, &value) != 0) return -1;
        }
        p->eval = 0;
        break;
      case TOKEN_STRUCT:
        insection(2);
        IDENT(name);
        lt = new_symbol_table(0);
        if ((struc = new_symbol(p, p->st, name, SYMBOL_STRUCT)) == NULL) return -1;
        struc->symbol.struc.name = name;
        struc->symbol.struc.st = lt;
        OPENB;
        parse_fieldlist(p, lt);
        CLOSEB;
        break;
      case TOKEN_VAR:
        insection(3);
        TYPE(vtype);
        IDENT(name);
        hasvalue = 0;
        if (opt(OPENSB)) {
          if (opt(CLOSESB)) {
            array = -1;
          } else {
            BACK;
            p->eval = 1;
            EXPR(depth);
            p->eval = 0;
            CLOSESB;
            pop(p, &value);
            if (!type_integer(value.type)) return syntax_error(p, "array size in variable '%s' must be integer", name);
            array = value.type == TYPE_BOOL ? value.value.b : value.value.i32;
            if (array <= 0) return syntax_error(p, "invalid array size %d in variable '%s'", array, name);
          }
        } else {
          BACK;
          if (opt(OPENP)) {
            p->eval = 1;
            EXPR(depth);
            p->eval = 0;
            CLOSEP;
            pop(p, &value);
            hasvalue = 1;
          } else {
            BACK;
          }
          array = 0;
        }
        if (array) {
          add_new_var_array(p, p->st, vtype, name, array);
        } else {
          if (add_new_var(p, p->st, vtype, name, hasvalue ? &value : NULL) != 0) return -1;
        }
        break;
      case TOKEN_FUNCTION:
        insection(4);
        TYPE(vtype);
        IDENT(name);
        lt = new_symbol_table(0);
        add_new_var(p, lt, vtype, "_", NULL); // return value
        OPENP;
        FORMAL_PARAMS(lt);
        CLOSEP;
        OPENB;
        LOCAL_VARS(lt);
        add_new_function(p, lt, vtype, name, nparams);
        STATEMENTS(&last, 0, 0);
        CLOSEB;
        if (last != TOKEN_RETURN) return syntax_error(p, "function must return a value");
        emit(p, ":%u\n", p->f->end_label);
        locals_size = p->f->st->frame_size - p->f->param_size - 12;
        if (locals_size) emit(p, "unframe %u\n", locals_size);
        emit(p, "ret\n");
        p->f = NULL;
        break;
      case TOKEN_EOF:
        return 0;
      case TOKEN_ERROR:
        return syntax_error(p, "lexical error");
      default:
        return syntax_error(p, "invalid statement");
    }
  }

  return 0;
}

static void emit_strings(parser_t *p) {
  symbol_t *symbol;
  symbol_string_t *string;
  uint32_t i, j, rem;

  for (i = 0; i < p->st->num_symbols; i++) {
    symbol = &p->st->symbol[i];
    if (symbol->type == SYMBOL_STRING) {
      string = &symbol->symbol.string;
      emit(p, ":%u\n", string->id);
      emit(p, "d8");
      for (j = 0; string->s[j]; j++) {
        emit(p, " 0x%02X", string->s[j]);
      }
      emit(p, " 0x00");
      rem = (j+1) % 4;
      if (rem) {
        rem = 4 - rem;
        for (j = 0; j < rem; j++) {
          emit(p, " 0x00");
        }
      }
      emit(p, " ; string \"%s\"\n", string->s);
    }
  }
}

static void emit_vars(parser_t *p) {
  symbol_t *symbol;
  symbol_var_t *var;
  uint32_t id, i;

  for (i = 0; i < p->st->num_symbols; i++) {
    symbol = &p->st->symbol[i];
    if (symbol->type == SYMBOL_VAR) {
      var = &symbol->symbol.var;
      emit(p, ":%s\n", symbol->name);
      if (var->array) {
        id = next_label(p);
        if (var->array > 0) {
          emit(p, "d32 :%u\n", id);
          emit(p, ":%u\n", id);
          emit(p, "res %u\n", var->array * ptype_size(p, var->type));
        } else {
          emit(p, "d32 0 ; unitialized array\n");
        }
      } else {
        if (var->value.type) {
          emit_data(p, &var->value);
        } else {
          emit(p, "res %u\n", ptype_size(p, var->type));
        }
      }
    }
  }
}

static void emit_symbols(parser_t *p) {
  symbol_t *symbol;
  symbol_function_t *f;
  symbol_const_t *cons;
  symbol_var_t *var;
  int i, j;

  for (i = 0; i < p->st->num_symbols; i++) {
    symbol = &p->st->symbol[i];
    if (symbol->type == SYMBOL_CONST) {
      cons = &symbol->symbol.cons;
      if (!cons->external) {
        emit(p, "!c %s %u %u\n", symbol->name, cons->type, cons->array);
      }
    } else if (symbol->type == SYMBOL_FUNCTION) {
      f = &symbol->symbol.function;
      if (!f->external) {
        emit(p, "!f %s %u %u", symbol->name, f->type, f->nparams);
        for (j = 0; j < f->nparams; j++) {
          var = &f->st->symbol[j].symbol.var;
          emit(p, " %u", var->type);
        }
        emit(p, "\n");
      }
    }
  }
}

int compile(char *src, char *obj) {
  lexer_t *l;
  parser_t *p;
  int fd_src, fd_obj, r;

  if ((fd_src = plibc_open(1, src, PLIBC_RDONLY)) == -1) {
    return system_error("Error opening source file '%s'", src);
  }

  if ((fd_obj = plibc_open(1, obj, PLIBC_WRONLY | PLIBC_CREAT | PLIBC_TRUNC)) == -1) {
    plibc_close(fd_src);
    return system_error("Error creating object file '%s'", obj);
  }

  if ((l = lexer_init(fd_src)) != NULL) {
    if ((p = parser_init(l, fd_obj)) != NULL) {
      info(p, "Compiling file '%s' into '%s'", src, obj);
      if ((r = parser_parse(p)) == 0) {
        emit_strings(p);
        emit_vars(p);
        emit_symbols(p);
      }
      parser_destroy(p);
    }
    lexer_destroy(l);
  }

  plibc_close(fd_obj);
  plibc_close(fd_src);

  if (r == 0) {
    info(p, "Success compiling file '%s'", src);
  } else {
    plibc_remove(1, obj);
    info(p, "Error compiling file '%s'", src);
  }

  return r;
}
