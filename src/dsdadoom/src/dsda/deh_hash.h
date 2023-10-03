//
// Copyright(C) 2022 by Ryan Krafnick
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
//	DSDA Dehacked Hash
//

#ifndef __DSDA_DEH_HASH__
#define __DSDA_DEH_HASH__

#define DEH_INDEX_HASH_SIZE 128
#define DEH_INDEX_NOT_FOUND -1

typedef struct deh_index_entry_s {
  int index_in;
  int index_out;
  struct deh_index_entry_s* next;
} deh_index_entry_t;

typedef struct {
  deh_index_entry_t table[DEH_INDEX_HASH_SIZE];
  int start_index;
  int end_index;
} deh_index_hash_t;

int dsda_FindDehIndex(int index, deh_index_hash_t* hash);
int dsda_GetDehIndex(int index, deh_index_hash_t* hash);

#endif
