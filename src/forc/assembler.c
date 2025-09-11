#include "sys.h"
#include "plibc.h"
#include "findargs.h"
#include "buffer.h"
#include "section.h"
#include "ht.h"
#include "error.h"
#include "debug.h"

#define MAX_PARTS   16

typedef struct {
  char *name;
  uint32_t code;
} opcode_t;

typedef struct {
  uint32_t addr;
  int defined;
  int absolute;
  uint32_t refs[16];
  uint32_t nrefs;
} label_t;

typedef struct {
  buffer_t *symb;
  buffer_t *code;
  int linenum;
  int fdout;
  int verbose;
  ht *labels;
} assemble_t;

static opcode_t opcodes[] = {
  { "bool",       0x10 },
  { "not",        0x11 },
  { "mod",        0x12 },
  { "and",        0x13 },
  { "or",         0x14 },
  { "xor",        0x15 },
  { "eq",         0x16 },
  { "neq",        0x17 },
  { "lsh",        0x18 },
  { "rsh",        0x19 },
  { "sign",       0x1a },
  { "swap",       0x1b },
  { "drop",       0x1c },
  { "dup",        0x1d },
  { "over",       0x1e },

  { "push32",     0x20 },
  { "fetch32",    0x21 },
  { "link",       0x22 },
  { "unlink",     0x23 },
  { "ret",        0x24 },
  { "new",        0x25 },
  { "hinc",       0x26 },
  { "hdec",       0x27 },
  { "floati",     0x28 },
  { "floatu",     0x29 },

  { "add",        0x30 },
  { "sub",        0x31 },
  { "mul",        0x32 },
  { "div",        0x33 },
  { "gt",         0x34 },
  { "lt",         0x35 },
  { "gte",        0x36 },
  { "lte",        0x37 },

  { "push",       0x40 },
  { "fetch",      0x50 },
  { "store",      0x60 },

  { "faddr",      0x80 },

  { "jpa",        0x90 },

  { "jp",         0xa0 },
  { "jz",         0xa1 },
  { "call",       0xa2 },

  { "frame",      0xb0 },
  { "unframe",    0xb1 },

  { NULL,         0x00 }
};

static int error(assemble_t *assemble, char *fmt, ...) {
  sys_va_list ap;
  char buf[1024];

  sys_va_start(ap, fmt);
  sys_vsnprintf(buf, sizeof(buf)-1, fmt, ap);
  sys_va_end(ap);
  system_error("Error at line %u: %s", assemble->linenum, buf);

  return -1;
}

static void info(assemble_t *assemble, char *msg, ...) {
  sys_va_list ap;

  if (assemble->verbose) {
    sys_va_start(ap, msg);
    debugva(DEBUG_INFO, APPNAME, msg, ap);
    sys_va_end(ap);
  }
}

static uint32_t number(char *s) {
  int h = !sys_strncmp(s, "0x", 2);
  return sys_strtoul(s, NULL, h ? 16 : 10);
}

static int fix_label(assemble_t *assemble, uint32_t addr, uint32_t dest, int absolute) {
  uint32_t b;
  int32_t d;
  int r = 0;

  if (absolute) {
    // 32 bits absolute addressing
    b = dest;
    info(assemble, "Fix 32 bits absolute address at 0x%08X: 0x%08X", addr, b);
    assemble->code->buf[addr]   = b;
    assemble->code->buf[addr+1] = b >> 8;
    assemble->code->buf[addr+2] = b >> 16;
    assemble->code->buf[addr+3] = b >> 24;

  } else {
    // 24 bits relative addressing
    d = (int32_t)dest - (int32_t)addr;
    b = d;
    info(assemble, "Fix 24 bits relative address at 0x%08X: 0x%08X", addr, b);

    if ((b & 0x80000000) && (b & 0xff000000) != 0xff000000) {
      r = error(assemble, "Backward reference too far (0x%08X)", b);
    } else if (!(b & 0x80000000) && (b & 0xff000000) != 0x00000000) {
      r = error(assemble, "Forward reference too far (0x%08X)", b);
    } else {
      assemble->code->buf[addr]   = b;
      assemble->code->buf[addr+1] = b >> 8;
      assemble->code->buf[addr+2] = b >> 16;
    }
  }

  return r;
}

static int add_label(assemble_t *assemble, char *label, int absolute) {
  label_t *lbl;
  int r = -1;

  if ((lbl = ht_get(assemble->labels, label)) == NULL) {
    lbl = sys_calloc(1, sizeof(label_t));
    lbl->absolute = absolute;
    lbl->refs[lbl->nrefs++] = assemble->code->size-4;
    ht_set(assemble->labels, label, lbl);
    r = 0;
  } else if (lbl->defined) {
    r = fix_label(assemble, assemble->code->size-4, lbl->addr, absolute ? absolute : lbl->absolute);
  } else {
    lbl->refs[lbl->nrefs++] = assemble->code->size-4;
    r = 0;
  }

  return r;
}

static int add_rel_reloc(assemble_t *assemble, uint32_t addr) {
  emit32(assemble->symb, SYM_RELR);
  emit32(assemble->symb, addr);

  return 0;
}

static int assemble_data_instruction(assemble_t *assemble, char *parts[], int nparts) {
  uint32_t data;
  int size, nitems, label, i, r = 0;

  if (nparts <= 1) {
    return error(assemble, "Invalid data instruction");
  }

  size = sys_atoi(&parts[0][1]);
  nitems = nparts - 1;

  for (i = 0; i < nitems && r == 0; i++) {
    if (parts[i+1][0] == ':') {
      // label
      data = 0;
      label = 1;
    } else if (parts[i+1][0] >= '0' && parts[i+1][0] <= '9') {
      // numeric value
      data = number(parts[i+1]);
      label = 0;
    } else {
      return error(assemble, "Invalid data argument %s", parts[i+1]);
    }

    switch (size) {
      case 8:
        emit8(assemble->code, data);
        break;
      case 16:
        emit16(assemble->code, data);
        break;
      case 32:
        if (label) r = add_rel_reloc(assemble, assemble->code->size);
        if (r == 0) {
          emit32(assemble->code, data);
          if (label) r = add_label(assemble, &parts[i+1][1], 1);
        }
        break;
      default:
        r = error(assemble, "Invalid data size %d", size);
        break;
    }
  }

  return r;
}

static int assemble_res_instruction(assemble_t *assemble, char *parts[], int nparts) {
  int size, i, r = -1;

  if (nparts == 2 && (size = sys_atoi(parts[1])) > 0) {
    for (i = 0; i < size; i++) emit8(assemble->code, 0);
    r = 0;
  } else {
    r = error(assemble, "Invalid res instruction");
  }

  return r;
}

static int add_optype(assemble_t *assemble, char optype, uint32_t *data, int shift) {
  int r = 0;

  switch (optype) {
    case 'f':
      *data |= 0x01 << shift;
      break;
    case 0:
      break;
    default:
      r = error(assemble, "Invalid modifier");
      break;
  }

  return r;
}

static void add_modifiers(assemble_t *assemble, char sign, int size, uint32_t *data, int shift) {
  switch (sign) {
    case 'u': break;
    case 'i': *data |= 0x80 << shift; break;
    default: break;
  }

  switch (size) {
    case  8: break;
    case 16: *data |= 0x01 << shift; break;
    case 32: *data |= 0x02 << shift; break;
  }
}

static int add_arg(assemble_t *assemble, int size, uint32_t *data, char *s) {
  uint32_t arg;
  int r = -1;

  arg = number(s);

  switch (size) {
    case  8: *data |= arg & 0xff;   r = 0; break;
    case 16: *data |= arg & 0xffff; r = 0; break;
    default: error(assemble, "Invalid size in opcode"); break;
  }

  return r;
}

static int add_abs_reloc(assemble_t *assemble, uint32_t addr, char *name) {
  emit32(assemble->symb, SYM_ABSR);
  emit32(assemble->symb, addr);
  emit_string(assemble->symb, name);

  return 0;
}

static int get_modifiers(assemble_t *assemble, char *opcode, int i, char *sign, uint32_t *size) {
  int r = -1;

  switch (opcode[i]) {
    case 'i':
    case 'u':
      *sign = opcode[i++];
      break;
    case '1':
    case '3':
    case '8':
      *sign = 0;
      break;
    default:
      return error(assemble, "Invalid sign modifier in opcode %s", opcode);
  }

  *size = sys_atoi(&opcode[i]);
  switch (*size) {
    case 8:
    case 16:
      if (!*sign) {
        error(assemble, "Missing sign modifier in opcode %s", opcode);
      } else {
        r = 0;
      }
      break;
    case 32:
      if (*sign) {
        error(assemble, "Invalid sign modifier in opcode %s", opcode);
      } else {
        r = 0;
      }
      break;
    default:
      error(assemble, "Invalid size modifier in opcode %s", opcode);
      break;
  }

  return r;
}

static int assemble_instruction(assemble_t *assemble, char *instruction) {
  char *parts[MAX_PARTS];
  uint32_t data, size;
  int i, nparts, len, r = -1;
  char optype, sign;

  nparts = findargs(instruction, parts, MAX_PARTS, NULL, NULL);

  if (!sys_strcmp(parts[0], "res")) {
    r = assemble_res_instruction(assemble, parts, nparts);
  } else if (!sys_strcmp(parts[0], "d8") || !sys_strcmp(parts[0], "d16") || !sys_strcmp(parts[0], "d32")) {
    r = assemble_data_instruction(assemble, parts, nparts);
  } else if (nparts >= 1) {
    for (i = 0; opcodes[i].name; i++) {
      len = sys_strlen(opcodes[i].name);
      if (!sys_strncmp(parts[0], opcodes[i].name, len)) {
        break;
      }
    }
    if (opcodes[i].name) {
      switch (opcodes[i].code & 0xf0) {
        case 0x10: // bool, not, ...
        case 0x20: // push32, fetch32, link, unlink, ret
          data = opcodes[i].code << 24;
          emit32(assemble->code, data);
          r = 0;
          break;

        case 0x30: // add, addf, ...
          data = opcodes[i].code << 24;
          optype = parts[0][len];
          r = add_optype(assemble, optype, &data, 16);
          if (r == 0) emit32(assemble->code, data);
          break;

        case 0x40: // pushi8, pushu8, pushi16, pushu16
          if (nparts == 2) {
            data = opcodes[i].code << 24;
            if (get_modifiers(assemble, parts[0], len, &sign, &size) == 0) {
              add_modifiers(assemble, sign, size, &data, 16);
              r = add_arg(assemble, size, &data, parts[1]);
              if (r == 0) emit32(assemble->code, data);
            }
          } else {
            error(assemble, "Missing operand");
          }
          break;

        case 0x50: // fetchi8, fetchu8, fetchi16, fetchu16
          if (nparts == 1) {
            data = opcodes[i].code << 24;
            if (get_modifiers(assemble, parts[0], len, &sign, &size) == 0) {
              add_modifiers(assemble, sign, size, &data, 16);
              emit32(assemble->code, data);
              r = 0;
            }
          } else {
            error(assemble, "Extra operand");
          }
          break;

        case 0x60: // store8, store16, store32
          if (nparts == 1) {
            data = opcodes[i].code << 24;
            if (get_modifiers(assemble, parts[0], len, &sign, &size) == 0 && sign == 0) {
              add_modifiers(assemble, 0, size, &data, 16);
              emit32(assemble->code, data);
              r = 0;
            }
          } else {
            error(assemble, "Extra operand");
          }
          break;

        case 0x80: // faddr
          if (nparts == 2) {
            data = opcodes[i].code << 24;
            r = add_arg(assemble, 16, &data, parts[1]);
            if (r == 0) emit32(assemble->code, data);
          } else {
            error(assemble, "Missing operand");
          }
          break;

        case 0x90: // jpa
          if (nparts == 2 && parts[1][0] == '#') {
            data = opcodes[i].code << 24;
            emit32(assemble->code, data);
            r = add_abs_reloc(assemble, assemble->code->size, &parts[1][1]);
            emit32(assemble->code, 0);
          } else {
            error(assemble, "Invalid jpa argument");
          }
          break;

        case 0xa0: // jp, jz, call
          if (nparts == 2 && parts[1][0] == ':') {
            data = opcodes[i].code << 24;
            emit32(assemble->code, data);
            r = add_label(assemble, &parts[1][1], 0);
          } else {
            error(assemble, "Missing or invalid label");
          }
          break;

        case 0xb0: // frame, unframe
          if (nparts == 2) {
            data = opcodes[i].code << 24;
            r = add_arg(assemble, 16, &data, parts[1]);
            if (r == 0) emit32(assemble->code, data);
          } else {
            error(assemble, "Missing operand");
          }
          break;
      }
    } else {
      error(assemble, "Invalid opcode '%s'", parts[0]);
    }
  }

  return r;
}

static int assemble_label(assemble_t *assemble, char *label) {
  label_t *lbl;
  uint32_t i;
  int r = 0;

  if (label[0]) {
    if ((lbl = ht_get(assemble->labels, label)) == NULL) {
      lbl = sys_calloc(1, sizeof(label_t));
      lbl->addr = assemble->code->size;
      lbl->defined = 1;
      ht_set(assemble->labels, label, lbl);
      for (i = 0; i < lbl->nrefs && r == 0; i++) {
        r = fix_label(assemble, lbl->refs[i], lbl->addr, lbl->absolute);
      }
    } else if (!lbl->defined) {
      lbl->addr = assemble->code->size;
      lbl->defined = 1;
      for (i = 0; i < lbl->nrefs && r == 0; i++) {
        r = fix_label(assemble, lbl->refs[i], lbl->addr, lbl->absolute);
      }
    } else {
      r = error(assemble, "Duplicate label '%s'", label);
    }
  } else {
    r = error(assemble, "Invalid empty label");
  }

  return r;
}

static int assemble_symbol(assemble_t *assemble, char *symbol) {
  label_t *lbl;
  char *parts[MAX_PARTS];
  int nparts, nparams, i, r = 0;

  switch (symbol[0]) {
    case 'c':
      // c b 6 0
      nparts = findargs(symbol, parts, MAX_PARTS, NULL, NULL);
      if (nparts == 4) {
        if ((lbl = ht_get(assemble->labels, parts[1])) != NULL) {
          emit32(assemble->symb, SYM_CONS);
          emit32(assemble->symb, lbl->addr);        // address (relative to module)
          emit32(assemble->symb, sys_atoi(parts[2]));   // const type
          emit32(assemble->symb, sys_atoi(parts[3]));   // array size (or 0)
          emit_string(assemble->symb, parts[1]);    // name
          r = 0;
        }
      }
      break;
    case 'f':
      // f main 6 1 6
      nparts = findargs(symbol, parts, MAX_PARTS, NULL, NULL);
      if (nparts >= 4) {
        if ((lbl = ht_get(assemble->labels, parts[1])) != NULL) {
          nparams = sys_atoi(parts[3]);
          emit32(assemble->symb, SYM_FUNC);
          emit32(assemble->symb, lbl->addr);      // address (relative to module)
          emit32(assemble->symb, sys_atoi(parts[2])); // function return type
          emit32(assemble->symb, nparams);        // number of parameters
          for (i = 0; i < nparams; i++) {
            emit32(assemble->symb, sys_atoi(parts[4+i])); // ith parameter type
          }
          emit_string(assemble->symb, parts[1]);  // name
          r = 0;
        }
      }
      break;
    default:
      r = error(assemble, "Invalid symbol '%s'", symbol);
      break;
  }

  return r;
}

static int assemble_line(assemble_t *assemble, char *line) {
  int r = -1;

  if (line[0] == ':') {
    r = assemble_label(assemble, &line[1]);
  } else if (line[0] == '!') {
    r = assemble_symbol(assemble, &line[1]);
  } else {
    r = assemble_instruction(assemble, line);
  }

if (r == -1) {
  return -1;
}
  return r;
}

static void save_header(assemble_t *assemble) {
  uint32_t magic = OBJ_MAGIC;
  info(assemble, "Saving header");
  plibc_write(assemble->fdout, &magic, sizeof(uint32_t));
}

static void save_section(assemble_t *assemble, uint32_t id, buffer_t *buffer, char *name) {
  uint32_t aux;

  if (buffer->buf && buffer->size) {
    info(assemble, "Saving %s section", name);
    aux = id;
    plibc_write(assemble->fdout, &aux, sizeof(uint32_t));
    aux = buffer->size;
    plibc_write(assemble->fdout, &aux, sizeof(uint32_t));
    plibc_write(assemble->fdout, buffer->buf, buffer->size);
  }
}

static int assemble1(PLIBC_FILE *fdin, int fdout) {
  assemble_t *assemble;
  hti it;
  label_t *lbl;
  char line[256];
  int i, r = 0;

  assemble = sys_calloc(1, sizeof(assemble_t));
  assemble->labels = ht_create();
  assemble->code = sys_calloc(1, sizeof(buffer_t));
  assemble->symb = sys_calloc(1, sizeof(buffer_t));
  assemble->fdout = fdout;
  assemble->verbose = 1;

  for (; r == 0;) {
    if (plibc_fgets(line, sizeof(line)-1, fdin) == NULL) break;
    assemble->linenum++;

    for (i = 0; line[i]; i++) {
      if (line[i] == ';' || line[i] == '\n') {
        line[i] = 0;
        break;
      }
    }
    for (i = 0; line[i] == ' ' || line[i] == '\t'; i++);
    if (line[i] == 0) continue;

    r = assemble_line(assemble, &line[i]);
  }

  if (r == 0) {
    save_header(assemble);

    it = ht_iterator(assemble->labels);
    while (ht_next(&it)) {
      lbl = (label_t *)it.value;
      if (!lbl->defined) {
        error(assemble, "Label '%s' is not defined", it.key);
        r = -1;
      }
      sys_free(it.value);
    }
  }
  ht_destroy(assemble->labels);

  if (r == 0) {
    save_section(assemble, SEC_SYMB, assemble->symb, "symbol");
    save_section(assemble, SEC_CODE, assemble->code, "code");
  }

  buffer_free(assemble->code);
  buffer_free(assemble->symb);
  sys_free(assemble);

  return r;
}

int assembler_assemble(char *asmm, char *obj) {
  PLIBC_FILE *fdin;
  int fdout, r;

  if ((fdin = plibc_fopen(1, asmm, "r")) == NULL) {
    return system_error("Error opening input file '%s'", asmm);
  }

  if ((fdout = plibc_open(1, obj, PLIBC_WRONLY | PLIBC_CREAT | PLIBC_TRUNC)) == -1) {
    plibc_fclose(fdin);
    return system_error("Error opening output file '%s'", obj);
  }

  r = assemble1(fdin, fdout);
  plibc_fclose(fdin);
  plibc_close(fdout);

  return r;
}
