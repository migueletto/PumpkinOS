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
//	DSDA Thing ID
//

#include "p_tick.h"
#include "z_zone.h"

#include "thing_id.h"

#define THING_ID_HASH_MAX 128

static thing_id_list_t* thing_id_list_hash[THING_ID_HASH_MAX];

static thing_id_list_t* dsda_NewThingIDList(short thing_id) {
  thing_id_list_t* result;

  result = Z_CallocLevel(1, sizeof(*result));
  result->thing_id = thing_id;
  return result;
}

static thing_id_list_t* dsda_ThingIDList(short thing_id) {
  thing_id_list_t* result;
  short index;

  index = thing_id % THING_ID_HASH_MAX;

  result = thing_id_list_hash[index];
  while (result && result->next && result->thing_id != thing_id)
    result = result->next;

  if (result) {
    if (result->thing_id != thing_id) {
      result->next = dsda_NewThingIDList(thing_id);
      result = result->next;
    }

    return result;
  }

  thing_id_list_hash[index] = dsda_NewThingIDList(thing_id);
  return thing_id_list_hash[index];
}

static thing_id_list_entry_t* dsda_NewThingIDListEntry(mobj_t* mo) {
  thing_id_list_entry_t* result;

  result = Z_CallocLevel(1, sizeof(*result));
  P_SetTarget(&result->mo, mo);
  return result;
}

void dsda_AddMobjThingID(mobj_t* mo, short thing_id) {
  thing_id_list_t* list;

  list = dsda_ThingIDList(thing_id);

  // Note that there is no uniqueness check - this is consistent with hexen
  if (!list->last) {
    list->first = dsda_NewThingIDListEntry(mo);
    list->last = list->first;
  }
  else {
    list->last->next = dsda_NewThingIDListEntry(mo);
    list->last = list->last->next;
  }

  mo->tid = thing_id;
}

void dsda_RemoveMobjThingID(mobj_t* mo) {
  thing_id_list_t* list;
  thing_id_list_entry_t* entry;
  thing_id_list_entry_t* next;

  list = dsda_ThingIDList(mo->tid);

  if (list->first && list->first->mo == mo) {
    next = list->first->next;
    P_SetTarget(&list->first->mo, NULL);
    // Z_Free(list->first); // TODO: might be freed during iteration, need solution
    list->first = next;
    if (!list->first)
      list->last = NULL;
  }
  else {
    for (entry = list->first; entry; entry = entry->next)
      if (entry->next && entry->next->mo == mo) {
        next = entry->next->next;
        P_SetTarget(&entry->next->mo, NULL);
        // Z_Free(entry->next); // TODO: might be freed during iteration, need solution
        entry->next = next;
        if (!entry->next)
          list->last = entry;
      }
  }

  mo->tid = 0;
}

// The allocated memory is automatically removed (zone memory)
void dsda_WipeMobjThingIDList(void) {
  int i;

  for (i = 0; i < THING_ID_HASH_MAX; ++i)
    thing_id_list_hash[i] = NULL;
}

void dsda_BuildMobjThingIDList(void) {
  mobj_t *mo;
  thinker_t *th;

  dsda_WipeMobjThingIDList();

  for (th = thinkercap.next; th != &thinkercap; th = th->next) {
    if (th->function != P_MobjThinker)
      continue;

    mo = (mobj_t *) th;

    if (mo->tid != 0)
      dsda_AddMobjThingID(mo, mo->tid);
  }
}

void dsda_ResetThingIDSearch(thing_id_search_t* search) {
  search->done = false;
  search->start = NULL;
}

static void dsda_FinishThingIDSearch(thing_id_search_t* search) {
  search->done = true;
  search->start = NULL;
}

mobj_t* dsda_FindMobjFromThingID(short thing_id, thing_id_search_t* search) {
  thing_id_list_entry_t* p;

  p = search->start;

  if (p)
    p = p->next;
  else {
    thing_id_list_t* list = dsda_ThingIDList(thing_id);
    p = list->first;
  }

  if (p) {
    search->start = p;
    return p->mo;
  }

  dsda_FinishThingIDSearch(search);
  return NULL;
}

mobj_t* dsda_FindMobjFromThingIDOrMobj(short thing_id, mobj_t* mo, thing_id_search_t* search) {
  if (search->done)
    return NULL;

  if (!thing_id) {
    dsda_FinishThingIDSearch(search);
    return mo;
  }

  return dsda_FindMobjFromThingID(thing_id, search);
}
