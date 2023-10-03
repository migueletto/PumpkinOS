//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Mobj Info
//

#include <stdlib.h>
#include <string.h>

#include "sounds.h"
#include "p_map.h"

#include "dsda/map_format.h"

#include "mobjinfo.h"

mobjinfo_t* mobjinfo;
int num_mobj_types;
int mobj_types_zero;
byte* edited_mobjinfo_bits;

static void dsda_ResetMobjInfo(int from, int to) {
  int i;

  for (i = from; i < to; ++i) {
    mobjinfo[i].droppeditem = MT_NULL;
    mobjinfo[i].infighting_group = IG_DEFAULT;
    mobjinfo[i].projectile_group = PG_DEFAULT;
    mobjinfo[i].splash_group = SG_DEFAULT;
    mobjinfo[i].altspeed = NO_ALTSPEED;
    mobjinfo[i].meleerange = MELEERANGE;
    mobjinfo[i].visibility = VF_DOOM;
  }
}

static void dsda_EnsureCapacity(int limit) {
  while (limit >= num_mobj_types) {
    int old_num_mobj_types = num_mobj_types;

    num_mobj_types *= 2;

    mobjinfo = Z_Realloc(mobjinfo, num_mobj_types * sizeof(*mobjinfo));
    memset(mobjinfo + old_num_mobj_types, 0,
      (num_mobj_types - old_num_mobj_types) * sizeof(*mobjinfo));

    edited_mobjinfo_bits =
      Z_Realloc(edited_mobjinfo_bits, num_mobj_types * sizeof(*edited_mobjinfo_bits));
    memset(edited_mobjinfo_bits + old_num_mobj_types, 0,
      (num_mobj_types - old_num_mobj_types) * sizeof(*edited_mobjinfo_bits));

    dsda_ResetMobjInfo(old_num_mobj_types, num_mobj_types);
  }
}

static deh_index_hash_t deh_mobj_index_hash;

int dsda_FindDehMobjIndex(int index) {
  return dsda_FindDehIndex(index, &deh_mobj_index_hash);
}

int dsda_GetDehMobjIndex(int index) {
  return dsda_GetDehIndex(index, &deh_mobj_index_hash);
}

// Dehacked has the index off by 1
int dsda_TranslateDehMobjIndex(int index) {
  return dsda_GetDehMobjIndex(index - 1) + 1;
}

dsda_deh_mobjinfo_t dsda_GetDehMobjInfo(int index) {
  dsda_deh_mobjinfo_t deh_mobjinfo;

  dsda_EnsureCapacity(index);

  deh_mobjinfo.info = &mobjinfo[index];
  deh_mobjinfo.edited_bits = &edited_mobjinfo_bits[index];

  return deh_mobjinfo;
}

void dsda_InitializeMobjInfo(int zero, int max, int count) {
  extern dboolean raven;

  num_mobj_types = count;
  mobj_types_zero = zero;

  mobjinfo = Z_Calloc(num_mobj_types, sizeof(*mobjinfo));

  if (raven) return;

  deh_mobj_index_hash.start_index = num_mobj_types;
  deh_mobj_index_hash.end_index = num_mobj_types;
  edited_mobjinfo_bits = Z_Calloc(num_mobj_types, sizeof(*edited_mobjinfo_bits));
}

// Changing the renderer causes a reset that accesses this list,
//   so we can't free it.
void dsda_FreeDehMobjInfo(void) {
  // free(edited_mobjinfo_bits);
}

int ZMT_MAPSPOT = ZMT_UNDEFINED;
int ZMT_MAPSPOT_GRAVITY = ZMT_UNDEFINED;
int ZMT_TELEPORTDEST2 = ZMT_UNDEFINED;
int ZMT_TELEPORTDEST3 = ZMT_UNDEFINED;

static mobjinfo_t zmt_mapspot_info = {
  .doomednum = 9001,
  .spawnstate = S_NULL,
  .spawnhealth = 1000,
  .seestate = S_NULL,
  .seesound = sfx_None,
  .reactiontime = 8,
  .attacksound = sfx_None,
  .painstate = S_NULL,
  .painchance = 0,
  .painsound = sfx_None,
  .meleestate = S_NULL,
  .missilestate = S_NULL,
  .deathstate = S_NULL,
  .xdeathstate = S_NULL,
  .deathsound = sfx_None,
  .speed = 0,
  .radius = 20 * FRACUNIT,
  .height = 16 * FRACUNIT,
  .mass = 100,
  .damage = 0,
  .activesound = sfx_None,
  .flags = MF_NOBLOCKMAP | MF_NOSECTOR | MF_NOGRAVITY,
  .raisestate = S_NULL,
  .droppeditem = MT_NULL,
  .crashstate = S_NULL,
  .flags2 = 0,
  .infighting_group = IG_DEFAULT,
  .projectile_group = PG_DEFAULT,
  .splash_group = SG_DEFAULT,
  .ripsound = sfx_None,
  .altspeed = NO_ALTSPEED,
  .meleerange = MELEERANGE,
  .bloodcolor = 0,
  .visibility = VF_ZDOOM,
};

static mobjinfo_t zmt_mapspot_gravity_info = {
  .doomednum = 9013,
  .spawnstate = S_NULL,
  .spawnhealth = 1000,
  .seestate = S_NULL,
  .seesound = sfx_None,
  .reactiontime = 8,
  .attacksound = sfx_None,
  .painstate = S_NULL,
  .painchance = 0,
  .painsound = sfx_None,
  .meleestate = S_NULL,
  .missilestate = S_NULL,
  .deathstate = S_NULL,
  .xdeathstate = S_NULL,
  .deathsound = sfx_None,
  .speed = 0,
  .radius = 20 * FRACUNIT,
  .height = 16 * FRACUNIT,
  .mass = 100,
  .damage = 0,
  .activesound = sfx_None,
  .flags = 0,
  .raisestate = S_NULL,
  .droppeditem = MT_NULL,
  .crashstate = S_NULL,
  .flags2 = MF2_DONTDRAW,
  .infighting_group = IG_DEFAULT,
  .projectile_group = PG_DEFAULT,
  .splash_group = SG_DEFAULT,
  .ripsound = sfx_None,
  .altspeed = NO_ALTSPEED,
  .meleerange = MELEERANGE,
  .bloodcolor = 0,
  .visibility = VF_ZDOOM,
};

static mobjinfo_t zmt_teleportdest2_info = {
  .doomednum = 9044,
  .spawnstate = S_NULL,
  .spawnhealth = 1000,
  .seestate = S_NULL,
  .seesound = sfx_None,
  .reactiontime = 8,
  .attacksound = sfx_None,
  .painstate = S_NULL,
  .painchance = 0,
  .painsound = sfx_None,
  .meleestate = S_NULL,
  .missilestate = S_NULL,
  .deathstate = S_NULL,
  .xdeathstate = S_NULL,
  .deathsound = sfx_None,
  .speed = 0,
  .radius = 20 * FRACUNIT,
  .height = 16 * FRACUNIT,
  .mass = 100,
  .damage = 0,
  .activesound = sfx_None,
  .flags = MF_NOBLOCKMAP | MF_NOSECTOR | MF_NOGRAVITY,
  .raisestate = S_NULL,
  .droppeditem = MT_NULL,
  .crashstate = S_NULL,
  .flags2 = 0,
  .infighting_group = IG_DEFAULT,
  .projectile_group = PG_DEFAULT,
  .splash_group = SG_DEFAULT,
  .ripsound = sfx_None,
  .altspeed = NO_ALTSPEED,
  .meleerange = MELEERANGE,
  .bloodcolor = 0,
  .visibility = VF_ZDOOM,
};

static mobjinfo_t zmt_teleportdest3_info = {
  .doomednum = 9043,
  .spawnstate = S_NULL,
  .spawnhealth = 1000,
  .seestate = S_NULL,
  .seesound = sfx_None,
  .reactiontime = 8,
  .attacksound = sfx_None,
  .painstate = S_NULL,
  .painchance = 0,
  .painsound = sfx_None,
  .meleestate = S_NULL,
  .missilestate = S_NULL,
  .deathstate = S_NULL,
  .xdeathstate = S_NULL,
  .deathsound = sfx_None,
  .speed = 0,
  .radius = 20 * FRACUNIT,
  .height = 16 * FRACUNIT,
  .mass = 100,
  .damage = 0,
  .activesound = sfx_None,
  .flags = MF_NOBLOCKMAP | MF_NOSECTOR,
  .raisestate = S_NULL,
  .droppeditem = MT_NULL,
  .crashstate = S_NULL,
  .flags2 = 0,
  .infighting_group = IG_DEFAULT,
  .projectile_group = PG_DEFAULT,
  .splash_group = SG_DEFAULT,
  .ripsound = sfx_None,
  .altspeed = NO_ALTSPEED,
  .meleerange = MELEERANGE,
  .bloodcolor = 0,
  .visibility = VF_ZDOOM,
};

typedef struct {
  int* index_p;
  mobjinfo_t* mobjinfo_p;
} append_mobjinfo_t;

static append_mobjinfo_t append_mobjinfo[] = {
  { &ZMT_MAPSPOT, &zmt_mapspot_info },
  { &ZMT_MAPSPOT_GRAVITY, &zmt_mapspot_gravity_info },
  { &ZMT_TELEPORTDEST2, &zmt_teleportdest2_info },
  { &ZMT_TELEPORTDEST3, &zmt_teleportdest3_info },
};

static int append_mobjinfo_count = sizeof(append_mobjinfo) / sizeof(append_mobjinfo[0]);

void dsda_AppendZDoomMobjInfo(void) {
  int i;
  int index;
  dsda_deh_mobjinfo_t mobjinfo;

  index = deh_mobj_index_hash.end_index;
  for (i = 0; i < append_mobjinfo_count; ++i) {
    mobjinfo = dsda_GetDehMobjInfo(index);
    *(append_mobjinfo[i].index_p) = index;
    *(mobjinfo.info) = *(append_mobjinfo[i].mobjinfo_p);
    ++index;
  }
}
