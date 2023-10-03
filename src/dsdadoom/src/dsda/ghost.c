//
// Copyright(C) 2020 by Ryan Krafnick
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
//	DSDA Ghost
//

#include <stdio.h>

#include "lprintf.h"
#include "doomtype.h"
#include "doomstat.h"
#include "info.h"
#include "m_file.h"
#include "p_maputl.h"
#include "p_tick.h"
#include "sounds.h"
#include "z_zone.h"
#include "w_wad.h"

#include "dsda/features.h"
#include "dsda/settings.h"

#include "ghost.h"

#define DSDA_GHOST_MIN_VERSION 1
#define DSDA_GHOST_VERSION 2

typedef struct {
  fixed_t x;
  fixed_t y;
  fixed_t z;
  angle_t angle;
  spritenum_t sprite;
  int frame;
  int map;
  int episode;
  int tic;
} dsda_ghost_frame_t;

typedef struct {
  dsda_ghost_frame_t frame;
  mobj_t* mobj;
  dg_file_t* fstream;
} dsda_ghost_t;

typedef struct {
  thinker_t* thinker;
  dsda_ghost_t* ghosts;
  int count;
} dsda_ghost_import_t;

typedef struct {
  dg_file_t* fstream;
  int version;
  int count;
} dsda_ghost_file_t;

mobjinfo_t dsda_ghost_info = {
  -1,            // doomednum
  S_PLAY,        // spawnstate
  0,             // spawnhealth
  S_PLAY_RUN1,   // seestate
  sfx_None,      // seesound
  0,             // reactiontime
  sfx_None,      // attacksound
  S_PLAY_PAIN,   // painstate
  0,             // painchance
  sfx_None,      // painsound
  S_NULL,        // meleestate
  S_PLAY_ATK1,   // missilestate
  S_PLAY_DIE1,   // deathstate
  S_PLAY_XDIE1,  // xdeathstate
  sfx_None,      // deathsound
  0,             // speed
  0,             // radius
  0,             // height
  0,             // mass
  0,             // damage
  sfx_None,      // activesound
  MF_NOBLOCKMAP | MF_TRANSLUCENT, // flags
  S_NULL         // raisestate
};

dg_file_t *dsda_ghost_export;
dsda_ghost_import_t dsda_ghost_import;

void dsda_InitGhostExport(const char* name) {
  int version;
  char* filename;
  filename = Z_Malloc(strlen(name) + 4 + 1);
  AddDefaultExtension(strcpy(filename, name), ".gst");

  dsda_ghost_export = M_OpenFile(filename, "wb");

  if (dsda_ghost_export == NULL)
    I_Error("dsda_InitGhostExport: failed to open %s", name);

  version = DSDA_GHOST_VERSION;
  DG_write(dsda_ghost_export, &version, sizeof(int));

  Z_Free(filename);
}

static void dsda_OpenGhostFile(const char* ghost_name, dsda_ghost_file_t* ghost_file) {
  char* filename;
  int read_result;

  memset(ghost_file, 0, sizeof(dsda_ghost_file_t));

  filename = Z_Malloc(strlen(ghost_name) + 4 + 1);
  AddDefaultExtension(strcpy(filename, ghost_name), ".gst");

  ghost_file->fstream = M_OpenFile(filename, "rb");

  if (ghost_file->fstream == NULL)
    I_Error("dsda_OpenGhostImport: failed to open %s", ghost_name);

  read_result = DG_read(ghost_file->fstream, &ghost_file->version, sizeof(int));
  if (ghost_file->version < DSDA_GHOST_MIN_VERSION ||
      ghost_file->version > DSDA_GHOST_VERSION ||
      read_result != sizeof(int))
    I_Error("dsda_OpenGhostImport: unsupported ghost version %s", ghost_name);

  if (ghost_file->version == 1)
    ghost_file->count = 1;
  else {
    read_result = DG_read(ghost_file->fstream, &ghost_file->count, sizeof(int));
    if (read_result != sizeof(int))
      I_Error("dsda_OpenGhostImport: error reading ghost count %s", ghost_name);
  }

  Z_Free(filename);
}

static int dsda_GhostCount(const char* ghost_name) {
  dsda_ghost_file_t ghost_file;

  dsda_OpenGhostFile(ghost_name, &ghost_file);

  DG_close(ghost_file.fstream);

  return ghost_file.count;
}

void dsda_InitGhostImport(const char** ghost_names, int count) {
  int arg_i;
  int ghost_i;
  int i;
  dsda_ghost_file_t ghost_file;

  ghost_i = 0;

  for (arg_i = 0; arg_i < count; ++arg_i)
    dsda_ghost_import.count += dsda_GhostCount(ghost_names[arg_i]);

  dsda_ghost_import.ghosts = Z_Calloc(dsda_ghost_import.count, sizeof(dsda_ghost_t));

  for (arg_i = 0; arg_i < count; ++arg_i) {
    dsda_OpenGhostFile(ghost_names[arg_i], &ghost_file);

    for (i = 0; i < ghost_file.count; ++i) {
      dsda_ghost_import.ghosts[ghost_i].fstream = ghost_file.fstream;
      ++ghost_i;
    }
  }
}

void dsda_ExportGhostFrame(void) {
  dsda_ghost_frame_t ghost_frame;
  mobj_t* player;
  int i;

  if (dsda_ghost_export == NULL) return;

  // just write the number of players on the zeroth tic
  if (gametic == 0) {
    int count = 0;

    for (i = 0; i < g_maxplayers; ++i) if (playeringame[i]) ++count;

    DG_write(dsda_ghost_export, &count, sizeof(int));

    return;
  }

  for (i = 0; i < g_maxplayers; ++i) {
    if (!playeringame[i]) break;

    player = players[i].mo;

    if (player == NULL) continue;

    ghost_frame.x = player->x;
    ghost_frame.y = player->y;
    ghost_frame.z = player->z;
    ghost_frame.angle = player->angle;
    ghost_frame.sprite = player->sprite;
    ghost_frame.frame = player->frame;
    ghost_frame.map = gamemap;
    ghost_frame.episode = gameepisode;
    ghost_frame.tic = gametic;

    DG_write(dsda_ghost_export, &ghost_frame, sizeof(dsda_ghost_frame_t));
  }
}

// Stripped down version of P_SpawnMobj
void dsda_SpawnGhost(void) {
  mobj_t* mobj;
  state_t* ghost_state;
  int ghost_i;

  if (dsda_StrictMode())
    return;

  for (ghost_i = 0; ghost_i < dsda_ghost_import.count; ++ghost_i) {
    if (dsda_ghost_import.ghosts[ghost_i].fstream == NULL) {
      dsda_ghost_import.ghosts[ghost_i].mobj = NULL;
      continue;
    }

    mobj = Z_MallocLevel(sizeof(*mobj));
    memset(mobj, 0, sizeof(*mobj));
    mobj->type = MT_NULL;
    mobj->info = &dsda_ghost_info;
    mobj->flags = dsda_ghost_info.flags;

    if (dsda_CycleGhostColors()) {
      switch (ghost_i % 4) {
        case 0:
          break;
        case 1:
          mobj->flags |= MF_TRANSLATION1;
          break;
        case 2:
          mobj->flags |= MF_TRANSLATION2;
          break;
        case 3:
          mobj->flags |= MF_TRANSLATION;
          break;
      }
    }

    mobj->x = players[0].mo->x;
    mobj->y = players[0].mo->y;
    mobj->z = players[0].mo->z;
    mobj->angle = players[0].mo->angle;

    ghost_state = &states[dsda_ghost_info.spawnstate];

    mobj->state  = ghost_state;
    mobj->tics   = ghost_state->tics;
    mobj->sprite = ghost_state->sprite;
    mobj->frame  = ghost_state->frame;
    mobj->touching_sectorlist = NULL;

    P_SetThingPosition(mobj);

    mobj->dropoffz =
    mobj->floorz   = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    mobj->PrevX = mobj->x;
    mobj->PrevY = mobj->y;
    mobj->PrevZ = mobj->z;

    mobj->friction = ORIG_FRICTION;
    mobj->index = -1;

    dsda_ghost_import.ghosts[ghost_i].mobj = mobj;
  }

  if (dsda_ghost_import.count > 0) {
    dsda_TrackFeature(uf_ghost);
    dsda_ghost_import.thinker = Z_MallocLevel(sizeof(*mobj));
    memset(dsda_ghost_import.thinker, 0, sizeof(thinker_t));
    dsda_ghost_import.thinker->function = dsda_UpdateGhosts;
    P_AddThinker(dsda_ghost_import.thinker);
  }
}

void dsda_UpdateGhosts(void* _void) {
  dsda_ghost_t* ghost;
  mobj_t* mobj;
  int ghost_i;
  int read_result;
  dboolean ghost_was_behind;

  for (ghost_i = 0; ghost_i < dsda_ghost_import.count; ++ghost_i) {
    ghost = &dsda_ghost_import.ghosts[ghost_i];

    if (ghost->fstream == NULL) continue;

    mobj = ghost->mobj;

    // Ghost removed from map (finished map already)
    if (mobj->touching_sectorlist == NULL) continue;

    mobj->PrevX = mobj->x;
    mobj->PrevY = mobj->y;
    mobj->PrevZ = mobj->z;

    ghost_was_behind = ghost->frame.map != 0 && ghost->frame.map != gamemap;

    // if the ghost was left behind, catch it up
    do {
      read_result = DG_read(ghost->fstream, &ghost->frame, sizeof(dsda_ghost_frame_t));

      if (read_result != sizeof(dsda_ghost_frame_t)) {
        DG_close(ghost->fstream);
        ghost->fstream = NULL;
        break;
      }
    } while (ghost_was_behind && ghost->frame.map != gamemap);

    if (ghost->fstream == NULL) continue;

    P_UnsetThingPosition(mobj);

    // Roll back one frame and leave position unset until next map
    if (ghost->frame.map != gamemap) {
      DG_seek(ghost->fstream, -(int)sizeof(dsda_ghost_frame_t), DG_SEEK_CUR);
      continue;
    }

    mobj->x = ghost->frame.x;
    mobj->y = ghost->frame.y;
    mobj->z = ghost->frame.z;
    mobj->angle = ghost->frame.angle;
    mobj->sprite = ghost->frame.sprite;
    mobj->frame = ghost->frame.frame;

    P_SetThingPosition(mobj);
  }
}
