//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
//	WAD I/O functions.
//

#include "w_file.h"

extern wad_file_class_t dg_wad_file;

wad_file_t *W_OpenFile(char *path) {
  return dg_wad_file.OpenFile(path);
}

void W_CloseFile(wad_file_t *wad) {
  wad->file_class->CloseFile(wad);
}

size_t W_Read(wad_file_t *wad, unsigned int offset, void *buffer, size_t buffer_len) {
  return wad->file_class->Read(wad, offset, buffer, buffer_len);
}
