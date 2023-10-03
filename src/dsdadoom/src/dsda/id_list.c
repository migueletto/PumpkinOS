//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA ID List
//

#include "z_zone.h"

#include "id_list.h"

typedef struct {
  int id;
  int size;
  int allocated_size;
  int* data;
} id_list_t;

typedef struct {
  int size;
  id_list_t* lists;
} id_index_t;

typedef struct {
  int size;
  id_index_t* data;
} id_hash_t;

static id_hash_t line_id_hash;
static id_hash_t sector_id_hash;

static id_list_t* dsda_NewListForIndex(id_index_t* index, int id) {
  id_list_t* new_list;

  index->lists = Z_ReallocLevel(index->lists, sizeof(*index->lists) * (index->size + 1));

  new_list = &index->lists[index->size++];
  new_list->id = id;
  new_list->size = 0;
  new_list->allocated_size = 1;
  new_list->data = Z_CallocLevel(new_list->allocated_size, sizeof(*new_list->data));
  new_list->data[0] = -1;

  return new_list;
}

static id_list_t* dsda_GetIDList(id_hash_t* hash, int id) {
  int i;
  id_index_t* index;

  index = &hash->data[(unsigned int)id % hash->size];

  for (i = 0; i < index->size; ++i)
    if (index->lists[i].id == id)
      return &index->lists[i];

  return dsda_NewListForIndex(index, id);
}

static void dsda_AddToIDHash(id_hash_t* hash, int id, int value) {
  id_list_t* list;

  list = dsda_GetIDList(hash, id);
  if (list->allocated_size < list->size + 2) {
    while (list->allocated_size < list->size + 2)
      list->allocated_size *= 2;
    list->data = Z_ReallocLevel(list->data, list->allocated_size * sizeof(*list->data));
  }
  list->data[list->size++] = value;
  list->data[list->size] = -1;
}

void dsda_AddLineID(int id, int value) {
  dsda_AddToIDHash(&line_id_hash, id, value);
}

void dsda_AddSectorID(int id, int value) {
  dsda_AddToIDHash(&sector_id_hash, id, value);
}

static int empty_list[] = { -1 };
static int missing_id_list[] = { -1, -1 };

const int* dsda_FindLinesFromID(int id) {
  return dsda_GetIDList(&line_id_hash, id)->data;
}

const int* dsda_FindSectorsFromID(int id) {
  return dsda_GetIDList(&sector_id_hash, id)->data;
}

const int* dsda_FindSectorsFromIDOrLine(int id, const line_t* line)
{
  if (id == 0) {
    if (!line || !line->backsector)
      return empty_list;

    missing_id_list[0] = line->backsector->iSectorID;
    return missing_id_list;
  }
  else
    return dsda_FindSectorsFromID(id);
}

const int hash_factor = 10;

void dsda_ResetLineIDList(int size) {
  line_id_hash.size = (size > hash_factor ? size / hash_factor : size);
  line_id_hash.data = Z_CallocLevel(line_id_hash.size, sizeof(*line_id_hash.data));
}

void dsda_ResetSectorIDList(int size) {
  sector_id_hash.size = (size > hash_factor ? size / hash_factor : size);
  sector_id_hash.data = Z_CallocLevel(sector_id_hash.size, sizeof(*sector_id_hash.data));
}
