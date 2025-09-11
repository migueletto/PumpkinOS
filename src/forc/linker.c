#include "sys.h"
#include "plibc.h"
#include "section.h"
#include "buffer.h"
#include "ht.h"
#include "linker.h"
#include "debug.h"

typedef struct {
  char *name;
  uint32_t *code;
  uint32_t size;
  uint32_t base;
  buffer_t arelocs;
  buffer_t rrelocs;
} module_t;

typedef struct {
  uint32_t addr;
  char *name;
  module_t *module;
} linker_symbol_t;

typedef struct {
  ht *defined;
  ht *modules;
  int fd;
} linker_t;

static void linker_error(char *fmt, ...) {
  sys_va_list ap;

  debug(DEBUG_ERROR, APPNAME,  "Linker error");
  sys_va_start(ap, fmt);
  debugva(DEBUG_ERROR, APPNAME, fmt, ap);
  sys_va_end(ap);
}

static int add_symbols(linker_t *link, module_t *module, uint8_t *symb, uint32_t size) {
  buffer_t buffer;
  linker_symbol_t *sym;
  char *name;
  uint32_t stype, addr, nparams;
  int i, r = 0;

  buffer.size = size;
  buffer.alloc_size = size;
  buffer.i = 0;
  buffer.buf = symb;

  for (; buffer.i < buffer.size && r == 0;) {
    stype = get32(&buffer);
    addr = get32(&buffer);

    switch (stype) {
      case SYM_RELR:
        emit32(&module->rrelocs, addr);
        break;

      case SYM_ABSR:
        name = get_string(&buffer);
        emit32(&module->arelocs, addr);
        emit_string(&module->arelocs, name);
        sys_free(name);
        break;

      case SYM_FUNC:
        get32(&buffer); // skip type
        nparams = get32(&buffer);
        if (nparams) {
          for (i = 0; i < nparams; i++) {
            get32(&buffer); // skip ith param
          }
        }
        name = get_string(&buffer);
        if ((sym = ht_get(link->defined, name)) == NULL) {
          if ((sym = sys_calloc(1, sizeof(linker_symbol_t))) != NULL) {
            sym->addr = addr;
            sym->name = name;
            sym->module = module;
            ht_set(link->defined, name, sym);
          } else {
            r = -1;
          }
        } else {
          linker_error("symbol '%s' already defined", name);
          r = -1;
        }
    }
  }

  return r;
}

static int linker_load(linker_t *link, char *filename) {
  module_t *module;
  uint32_t symb_size, code_size;
  uint8_t *symb, *code;
  PLIBC_FILE *f;
  int r = -1;

  if ((f = plibc_fopen(1, filename, "rb")) != NULL) {
    if ((module = ht_get(link->modules, filename)) == NULL) {
      if ((module = sys_calloc(1, sizeof(module_t))) != NULL) {
        code = section_read(SEC_CODE, filename, &code_size, f);
        symb = section_read(SEC_SYMB, filename, &symb_size, f);
        module->name = filename;
        module->code = (uint32_t *)code;
        module->size = code_size;
        if (symb) {
          r = add_symbols(link, module, symb, symb_size);
          sys_free(symb);
        } else {
          r = 0;
        }
        if (r == 0) {
          ht_set(link->modules, filename, module);
        } else {
          buffer_free(&module->rrelocs);
          buffer_free(&module->arelocs);
          sys_free(module);
        }
      }
    } else {
      linker_error("module '%s' already loaded", filename);
    }
    plibc_fclose(f);
  } else {
    linker_error("could not load module '%s'", filename);
  }

  return r;
}

static void save_header(linker_t *link, linker_symbol_t *main) {
  uint32_t aux;

  aux = EXE_MAGIC;
  plibc_write(link->fd, &aux, sizeof(uint32_t));
  aux = main->module->base + main->addr;
  plibc_write(link->fd, &aux, sizeof(uint32_t));
}

static void save_obj(linker_t *link, uint8_t *buf, uint32_t size) {
  plibc_write(link->fd, buf, size);
}

static int linker_link1(linker_t *link) {
  linker_symbol_t *main, *defined;
  module_t *module;
  hti it;
  char *name;
  uint32_t base, addr;
  int r = 0;

  if ((main = ht_get(link->defined, "main")) == NULL) {
    linker_error("main function is not defined in any module");
    return -1;
  }

  base = 0;
  it = ht_iterator(link->modules);
  while (ht_next(&it)) {
    module = (module_t *)it.value;
    module->base = base;
    base += module->size;
  }

  it = ht_iterator(link->modules);
  while (ht_next(&it)) {
    module = (module_t *)it.value;
    for (; module->rrelocs.i < module->rrelocs.size;) {
      addr = get32(&module->rrelocs);
      module->code[addr >> 2] += module->base;
    }
    for (; module->arelocs.i < module->arelocs.size;) {
      addr = get32(&module->arelocs);
      name = get_string(&module->arelocs);
      if ((defined = ht_get(link->defined, name)) != NULL) {
        module->code[addr >> 2] = defined->module->base + defined->addr;
      } else {
        linker_error("symbol '%s' used in module '%s' is not defined in any module", name, module->name);
        r = -1;
      }
    }
  }

  if (r == 0) {
    save_header(link, main);
    it = ht_iterator(link->modules);
    while (ht_next(&it) && r == 0) {
      module = (module_t *)it.value;
      save_obj(link, (uint8_t *)module->code, module->size);
    }
  }

  return r;
}

int linker_link(int n, char *obj[], char *exe) {
  linker_t link;
  int i, r = 0;

  if ((link.fd = plibc_open(1, exe, PLIBC_WRONLY | PLIBC_CREAT | PLIBC_TRUNC)) == -1) {
    plibc_close(link.fd);
    debug(DEBUG_ERROR, APPNAME, "Error creating executable file '%s'", exe);
    return -1;
  }

  link.defined = ht_create();
  link.modules = ht_create();

  for (i = 0; i < n && r == 0; i++) {
    r = linker_load(&link, obj[i]);
  }

  if (r == 0) {
    r = linker_link1(&link);
  }
  plibc_close(link.fd);

  ht_destroy(link.defined);
  ht_destroy(link.modules);

  if (r != 0) {
    plibc_remove(1, exe);
  }

  return r;
}
