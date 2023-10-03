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
//	DSDA Save
//

#include <stdlib.h>

#include "doomstat.h"
#include "g_game.h"
#include "lprintf.h"
#include "z_zone.h"
#include "p_saveg.h"
#include "p_map.h"
#include "s_sound.h"

#include "dsda/args.h"
#include "dsda/configuration.h"
#include "dsda/data_organizer.h"
#include "dsda/excmd.h"
#include "dsda/features.h"
#include "dsda/mapinfo.h"
#include "dsda/music.h"
#include "dsda/options.h"

#include "save.h"

static char* dsda_base_save_dir;
static char* dsda_wad_save_dir;

extern int dsda_max_kill_requirement;
extern int player_damage_last_tic;

static void dsda_ArchiveInternal(void) {
  uint64_t features;

  P_SAVE_X(dsda_max_kill_requirement);
  P_SAVE_X(player_damage_last_tic);

  features = dsda_UsedFeatures();
  P_SAVE_X(features);
}

static void dsda_UnArchiveInternal(void) {
  uint64_t features;

  P_LOAD_X(dsda_max_kill_requirement);
  P_LOAD_X(player_damage_last_tic);

  P_LOAD_X(features);
  dsda_MergeFeatures(features);
}

static void dsda_ArchiveContext(void) {
  int i;
  int boom_logictic_value;
  int true_logictic_value;

  P_SAVE_BYTE(compatibility_level);
  P_SAVE_BYTE(gameskill);
  P_SAVE_BYTE(gameepisode);
  P_SAVE_BYTE(gamemap);

  for (i = 0; i < g_maxplayers; ++i)
    P_SAVE_BYTE(playeringame[i]);

  for (; i < FUTURE_MAXPLAYERS; ++i)
    P_SAVE_BYTE(0);

  P_SAVE_BYTE(idmusnum);

  dsda_ArchiveMusic();

  CheckSaveGame(dsda_GameOptionSize());
  save_p = G_WriteOptions(save_p);

  P_SAVE_X(leveltime);
  P_SAVE_X(totalleveltimes);
  P_SAVE_X(levels_completed);

  boom_logictic_value = boom_logictic;
  P_SAVE_X(boom_logictic_value);

  true_logictic_value = true_logictic;
  P_SAVE_X(true_logictic_value);
}

static void dsda_UnArchiveContext(void) {
  int i;
  int epi, map;
  int boom_logictic_value;
  int true_logictic_value;

  P_LOAD_BYTE(compatibility_level);
  P_LOAD_BYTE(gameskill);

  P_LOAD_BYTE(epi);
  P_LOAD_BYTE(map);
  dsda_UpdateGameMap(epi, map);

  for (i = 0; i < g_maxplayers; ++i)
    P_LOAD_BYTE(playeringame[i]);
  save_p += FUTURE_MAXPLAYERS - g_maxplayers;

  P_LOAD_BYTE(idmusnum);
  if (idmusnum == 255)
    idmusnum = -1;

  dsda_UnArchiveMusic();

  save_p += (G_ReadOptions(save_p) - save_p);

  G_InitNew(gameskill, gameepisode, gamemap, false);

  P_LOAD_X(leveltime);
  P_LOAD_X(totalleveltimes);
  P_LOAD_X(levels_completed);

  P_LOAD_X(boom_logictic_value);
  boom_basetic = gametic - boom_logictic_value;

  P_LOAD_X(true_logictic_value);
  true_basetic = gametic - true_logictic_value;
}

void dsda_ArchiveAll(void) {
  dsda_ArchiveContext();

  P_ArchiveACS();
  P_ArchivePlayers();
  P_ThinkerToIndex();
  P_ArchiveWorld();
  P_ArchivePolyobjs();
  P_TrueArchiveThinkers();
  P_ArchiveScripts();
  P_ArchiveSounds();
  P_ArchiveAmbientSound();
  P_ArchiveMisc();
  P_IndexToThinker();
  P_ArchiveRNG();
  P_ArchiveMap();

  dsda_ArchiveInternal();
}

void dsda_UnArchiveAll(void) {
  dsda_UnArchiveContext();

  P_MapStart();
  P_UnArchiveACS();
  P_UnArchivePlayers();
  P_UnArchiveWorld();
  P_UnArchivePolyobjs();
  P_TrueUnArchiveThinkers();
  P_UnArchiveScripts();
  P_UnArchiveSounds();
  P_UnArchiveAmbientSound();
  P_UnArchiveMisc();
  P_UnArchiveRNG();
  P_UnArchiveMap();
  P_MapEnd();

  dsda_UnArchiveInternal();
}

void dsda_InitSaveDir(void) {
  dsda_base_save_dir = dsda_DetectDirectory("DOOMSAVEDIR", dsda_arg_save);
}

static char* dsda_SaveDir(void) {
  if (dsda_IntConfig(dsda_config_organized_saves)) {
    if (!dsda_wad_save_dir)
      dsda_wad_save_dir = dsda_DataDir();

    return dsda_wad_save_dir;
  }

  return dsda_base_save_dir;
}

extern const char* savegamename;

char* dsda_SaveGameName(int slot, dboolean via_cmd) {
  int length;
  char* name;
  const char* save_dir;
  const char* save_type;

  if (slot > 9999 || slot < 0)
    I_Error("dsda_SaveGameName: bad save slot %d", slot);

  save_dir = dsda_SaveDir();

  save_type = (via_cmd && demoplayback) ? "demosav" : savegamename;

  length = strlen(save_type) + strlen(save_dir) + 10; // "/" + "9999.dsg\0"

  name = Z_Malloc(length);

  snprintf(name, length, "%s/%s%d.dsg", save_dir, save_type, slot);

  return name;
}

static int* demo_save_slots;
static int allocated_save_slot_count;
static int demo_save_slot_count;

void dsda_ResetDemoSaveSlots(void) {
  demo_save_slot_count = 0;
}

static void dsda_MarkSaveSlotUsed(int slot) {
  int i;

  if (!demorecording) return;

  for (i = 0; i < demo_save_slot_count; ++i)
    if (demo_save_slots[i] == slot)
      return;

  ++demo_save_slot_count;
  if (demo_save_slot_count > allocated_save_slot_count) {
    allocated_save_slot_count = allocated_save_slot_count ? allocated_save_slot_count * 2 : 8;
    demo_save_slots =
      Z_Realloc(demo_save_slots, allocated_save_slot_count * sizeof(*demo_save_slots));
  }

  demo_save_slots[demo_save_slot_count - 1] = slot;
}

int dsda_AllowMenuLoad(int slot) {
  int i;

  if (!demorecording) return true;
  if (!dsda_AllowCasualExCmdFeatures()) return false;

  for (i = 0; i < demo_save_slot_count; ++i)
    if (demo_save_slots[i] == slot)
      return true;

  return false;
}

int dsda_AllowAnyMenuLoad(void) {
  return !demorecording || dsda_AllowCasualExCmdFeatures();
}

static int last_save_file_slot = -1;

void dsda_SetLastLoadSlot(int slot) {
  last_save_file_slot = slot;
}

void dsda_SetLastSaveSlot(int slot) {
  dsda_MarkSaveSlotUsed(slot);
  last_save_file_slot = slot;
}

int dsda_LastSaveSlot(void) {
  return last_save_file_slot;
}

void dsda_ResetLastSaveSlot(void) {
  last_save_file_slot = -1;
}
