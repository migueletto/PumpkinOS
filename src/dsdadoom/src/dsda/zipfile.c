//
// Copyright(C) 2023 by Pierre Wendling
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	DSDA zipfile support using libzip
//

#include <stdio.h>

#include "i_system.h"
#include "lprintf.h"
#include "m_file.h"
#include "z_zone.h"
#include "zip.h"
#include "zip_source_file.h"

#include "sys.h"
#include "host.h"

#include "dsda/utility.h"

static char **temp_dirs;

/* Allow a maximum of 1GB to be uncompressed to prevent zip-bombs */
#define UNZIPPED_BYTES_LIMIT 1000000000ULL

static zip_uint64_t total_bytes_read;

#define CHUNK_SIZE 4 * 1024U

static void dsda_WriteContentToFile(zip_file_t *input_file, dg_file_t *dest_file, zip_uint64_t data_size) {
  byte buffer[CHUNK_SIZE];

  while (data_size != 0) {
    zip_uint64_t chunk_size;
    zip_int64_t bytes_read;

    chunk_size = MIN(data_size, CHUNK_SIZE);
    bytes_read = zip_fread(input_file, buffer, chunk_size);
    if (bytes_read == -1)
      I_Error("dsda_WriteContentToFile: Unable to read data from archive.");

    if (DG_write(dest_file, buffer, bytes_read) != bytes_read)
      I_Error("dsda_WriteContentToFile: Failed to write data to file.");

    data_size -= bytes_read;
    total_bytes_read += bytes_read;
    if (total_bytes_read >= UNZIPPED_BYTES_LIMIT)
      I_Error("dsda_WriteContentToFile: Too much data to decompress.");
  }
}

static void dsda_WriteZippedFilesToDest(zip_t *archive, const char *destination_directory) {
  zip_int64_t i;

  for (i = 0; i < zip_get_num_entries(archive, ZIP_FL_UNCHANGED); i++) {
    dsda_string_t full_path;
    zip_file_t *zipped_file;
    zip_stat_t stat;
    dg_file_t *dest_file;
    const char *file_name = dsda_BaseName(zip_get_name(archive, i, ZIP_FL_UNCHANGED));

    /* Intermediate directories have a trailing '/', so their base name is empty */
    if (*file_name == '\0') {
      continue;
    }

    dsda_StringPrintF(&full_path, "%s/%s", destination_directory, file_name);

    zip_stat_index(archive, i, ZIP_FL_UNCHANGED, &stat);
    if ((stat.valid & ZIP_STAT_SIZE) == 0)
      I_Error("dsda_WriteZippedFilesToDest: Failed to read size of zipped file %s.", file_name);

    zipped_file = zip_fopen_index(archive, i, ZIP_FL_UNCHANGED);
    if (zipped_file == NULL)
      I_Error("dsda_WriteZippedFilesToDest: Failed to open zipped file %s.", file_name);

    dest_file = M_OpenFile(full_path.string, "wb");
    if (dest_file == NULL)
      I_Error("dsda_WriteZippedFilesToDest: Failed to open destination file %s.", full_path.string);

    dsda_WriteContentToFile(zipped_file, dest_file, stat.size);

    zip_fclose(zipped_file);
    DG_close(dest_file);
    dsda_FreeString(&full_path);
  }
}

bool zip_op_open(zip_source_file_context_t *ctx) {
  ctx->f = DG_open(ctx->fname, 0);
  return ctx->f != NULL;;
}

static void zip_op_close(zip_source_file_context_t *ctx) {
  if (ctx->f) {
    DG_close(ctx->f);
    ctx->f = NULL;
  }
}

static zip_int64_t zip_op_read(zip_source_file_context_t *ctx, void *buf, zip_uint64_t len) {
  zip_int64_t r = 0;

  if (ctx->f) {
    r = DG_read(ctx->f, buf, len);
  }

  return r;
}

static bool zip_op_seek(zip_source_file_context_t *ctx, void *f, zip_int64_t offset, int whence) {
  dg_seek_t w;
  bool r = false;

  if (ctx->f) {
    switch (whence) {
      case SEEK_SET: w = DG_SEEK_SET; break;
      case SEEK_CUR: w = DG_SEEK_CUR; break;
      case SEEK_END: w = DG_SEEK_END; break;
      default: w = DG_SEEK_SET;
    }
    r = DG_seek(ctx->f, offset, w) == 0;
  }

  return r;
}

static bool zip_op_stat(zip_source_file_context_t *ctx, zip_source_file_stat_t *st) {
  dg_file_t *f = DG_open(ctx->fname, 0);

  if ((f = DG_open(ctx->fname, 0)) != NULL) {
    st->size = DG_filesize(f);
    st->exists = true;
    st->regular_file = true; // XXX always set
    DG_close(f);
  } else {
    st->size = 0;
    st->exists = false;
    st->regular_file = false;
  }
  // XXX mtime is left unset

  // XXX always return true
  return true;
}

static char *zip_op_string_duplicate(zip_source_file_context_t *ctx, const char *s) {
  return sys_strdup(s);
}

static zip_source_file_operations_t ops = {
    zip_op_close,
    NULL,
    NULL,
    NULL,
    zip_op_open,
    zip_op_read,
    NULL,
    NULL,
    zip_op_seek,
    zip_op_stat,
    zip_op_string_duplicate,
    NULL,
    NULL,
};

static void dsda_UnzipFileToDestination(const char *zipped_file_name, const char *destination_directory) {
  int error_code;
  zip_t *archive_handle;
  zip_source_t *src;
  zip_error_t error;

  total_bytes_read = 0;
  //archive_handle = zip_open(zipped_file_name, ZIP_RDONLY, &error_code);
  src = zip_source_file_common_new(zipped_file_name, NULL, 0, 0, NULL, &ops, NULL, &error);
  archive_handle = zip_open_src(src, zipped_file_name, ZIP_RDONLY, &error_code);
  if (archive_handle == NULL) {
    zip_error_t error;
    zip_error_init_with_code(&error, error_code);
    I_Error("dsda_UnzipFileToDestination: Unable to open %s: %s.\n", zipped_file_name, zip_error_strerror(&error));
  }

  dsda_WriteZippedFilesToDest(archive_handle, destination_directory);

  zip_close(archive_handle);
}

const char* dsda_UnzipFile(const char *zipped_file_name) {
  dsda_string_t temporary_directory;
  static unsigned int file_counter = 0;

  dsda_StringPrintF(&temporary_directory, "%s/%u-%s", I_GetTempDir(), file_counter, dsda_BaseName(zipped_file_name));
  if (M_IsDir(temporary_directory.string))
    if (!M_RemoveFilesAtPath(temporary_directory.string))
      I_Error("dsda_UnzipFile: unable to clear tempdir %s\n", temporary_directory.string);
  M_MakeDir(temporary_directory.string, true);

  dsda_UnzipFileToDestination(zipped_file_name, temporary_directory.string);

  temp_dirs = Z_Realloc(temp_dirs, (file_counter + 2) * sizeof(*temp_dirs));
  temp_dirs[file_counter] = temporary_directory.string;
  temp_dirs[file_counter + 1] = NULL;
  file_counter++;

  return temporary_directory.string;
}

void dsda_CleanZipTempDirs(void) {
  int i;

  if(temp_dirs == NULL)
    return;

  for (i = 0; temp_dirs[i] != NULL; i++) {
    M_RemoveFilesAtPath(temp_dirs[i]);
    M_remove(temp_dirs[i]);
    Z_Free(temp_dirs[i]);
  }
  Z_Free(temp_dirs);
}
