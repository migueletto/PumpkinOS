#include "sys.h"
#include "plibc.h"
#include "section.h"
#include "error.h"

uint8_t *section_read(uint32_t id, char *name, uint32_t *size, PLIBC_FILE *f) {
  uint8_t *section = NULL;
  uint32_t aux;
  int ioerr = 0;

  if (plibc_fseek(f, 0, PLIBC_SEEK_SET) != 0) {
    ioerr = 1;
  }

  if (plibc_fread(&aux, sizeof(uint32_t), 1, f) != 1) {
    ioerr = 1;
  } else if (aux != OBJ_MAGIC) {
    system_error("invalid module \"%s\"", name);
  } else {
    for (; !ioerr;) {
      if (plibc_fread(&aux, sizeof(uint32_t), 1, f) != 1) {
        break;
      }
      if (aux != SEC_CODE && aux != SEC_SYMB) {
        system_error("invalid section in module \"%s\"", name);
        break;
      }
      if (plibc_fread(size, sizeof(uint32_t), 1, f) != 1) {
        ioerr = 1;
        break;
      }
      if (aux != id) {
        if (plibc_fseek(f, *size, PLIBC_SEEK_CUR) != 0) {
          ioerr = 1;
          break;
        }
        continue;
      }
      if ((section = sys_calloc(1, *size)) == NULL) {
        break;
      }
      if (plibc_fread(section, 1, *size, f) != *size) {
        sys_free(section);
        section = NULL;
        ioerr = 1;
      }
      break;
    }
  }

  if (ioerr) {
    system_error("error reading module \"%s\"", name);
  }

  return section;
}
