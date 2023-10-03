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

#include "z_zone.h"

#include "deh_hash.h"

int dsda_FindDehIndex(int index, deh_index_hash_t* hash) {
  deh_index_entry_t* entry;

  // this index is not in the hash
  if (index < hash->start_index)
    return index;

  entry = &hash->table[index % DEH_INDEX_HASH_SIZE];

  while (entry->next && entry->index_in != index)
    entry = entry->next;

  if (entry->index_in != index)
    return DEH_INDEX_NOT_FOUND;

  return entry->index_out;
}

int dsda_GetDehIndex(int index, deh_index_hash_t* hash) {
  deh_index_entry_t* entry;

  // this index is not in the hash
  if (index < hash->start_index)
    return index;

  entry = &hash->table[index % DEH_INDEX_HASH_SIZE];

  while (entry->next && entry->index_in != index)
    entry = entry->next;

  if (entry->index_in != index) {
    entry->next = Z_Calloc(1, sizeof(*entry));
    entry = entry->next;
    entry->index_in = index;
    entry->index_out = hash->end_index;
    ++hash->end_index;
  }

  return entry->index_out;
}
