#include "doomgeneric.h"
#include "m_misc.h"
#include "w_file.h"
#include "z_zone.h"

typedef struct {
  wad_file_t wad;
  dg_file_t *f;
} dg_wad_file_t;

extern wad_file_class_t dg_wad_file;

static wad_file_t *W_DG_OpenFile(char *path) {
  dg_wad_file_t *result;
  dg_file_t *f;

  if ((f = DG_open(path, 0)) != NULL) {
    result = Z_Malloc(sizeof(dg_wad_file_t), PU_STATIC, 0);
    result->wad.file_class = &dg_wad_file;
    result->wad.mapped = NULL;
    result->wad.length = DG_filesize(f);
    result->f = f;
    return &result->wad;
  }

  return NULL;
}

static void W_DG_CloseFile(wad_file_t *wad) {
  dg_wad_file_t *dg_wad = (dg_wad_file_t *)wad;
  DG_close(dg_wad->f);
  Z_Free(dg_wad);
}

size_t W_DG_Read(wad_file_t *wad, unsigned int offset, void *buffer, size_t buffer_len) {
  dg_wad_file_t *dg_wad = (dg_wad_file_t *)wad;
  size_t result = 0;

  DG_seek(dg_wad->f, offset);
  if (DG_read(dg_wad->f, buffer, buffer_len) == buffer_len) {
    result = buffer_len;
  }

  return result;
}

wad_file_class_t dg_wad_file = {
  W_DG_OpenFile,
  W_DG_CloseFile,
  W_DG_Read,
};
