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
//	DSDA Options Lump
//

#include "doomstat.h"
#include "w_wad.h"
#include "g_game.h"
#include "m_random.h"
#include "lprintf.h"

#include "dsda/configuration.h"

#include "options.h"

static const dsda_options_t default_vanilla_options = {
  .weapon_recoil = 0,
  .monsters_remember = 0,
  .monster_infighting = 1,
  .monster_backing = 0,
  .monster_avoid_hazards = 0,
  .monkeys = 0,
  .monster_friction = 0,
  .help_friends = 0,
  .player_helpers = 0,
  .friend_distance = 0,
  .dog_jumping = 0
};

static const dsda_options_t default_boom_options = {
  .weapon_recoil = 0,
  .monsters_remember = 1,
  .monster_infighting = 1,
  .monster_backing = 0,
  .monster_avoid_hazards = 0,
  .monkeys = 0,
  .monster_friction = 0,
  .help_friends = 0,
  .player_helpers = 0,
  .friend_distance = 0,
  .dog_jumping = 0
};

static const dsda_options_t default_mbf_options = {
  .weapon_recoil = 0,
  .monsters_remember = 1,
  .monster_infighting = 1,
  .monster_backing = 0,
  .monster_avoid_hazards = 1,
  .monkeys = 0,
  .monster_friction = 1,
  .help_friends = 0,
  .player_helpers = 0,
  .friend_distance = 128,
  .dog_jumping = 1,

  .comp_telefrag = 0,
  .comp_dropoff = 0,
  .comp_vile = 0,
  .comp_pain = 0,
  .comp_skull = 0,
  .comp_blazing = 0,
  .comp_doorlight = 0,
  .comp_model = 0,
  .comp_god = 0,
  .comp_falloff = 0,
  .comp_floors = 0,
  .comp_skymap = 0,
  .comp_pursuit = 0,
  .comp_doorstuck = 0,
  .comp_staylift = 0,
  .comp_zombie = 1,
  .comp_stairs = 0,
  .comp_infcheat = 0,
  .comp_zerotags = 0

  // These are not configurable:
  // .comp_moveblock = 0,
  // .comp_respawn = 1,
  // .comp_sound = 0,
  // .comp_666 = 0,
  // .comp_soul = 1,
  // .comp_maskedanim = 0,
  // .comp_ouchface = 1,
  // .comp_maxhealth = 0,
  // .comp_translucency = 0,
  // .comp_ledgeblock = 0,
  // .comp_friendlyspawn = 1,
  // .comp_voodooscroller = 1,
  // .comp_reservedlineflag = 1,
};

static const dsda_options_t default_latest_options = {
  .weapon_recoil = 0,
  .monsters_remember = 1,
  .monster_infighting = 1,
  .monster_backing = 0,
  .monster_avoid_hazards = 1,
  .monkeys = 0,
  .monster_friction = 1,
  .help_friends = 0,
  .player_helpers = 0,
  .friend_distance = 128,
  .dog_jumping = 1,

  .comp_telefrag = 0,
  .comp_dropoff = 0,
  .comp_vile = 0,
  .comp_pain = 0,
  .comp_skull = 0,
  .comp_blazing = 0,
  .comp_doorlight = 0,
  .comp_model = 0,
  .comp_god = 0,
  .comp_falloff = 0,
  .comp_floors = 0,
  .comp_skymap = 0,
  .comp_pursuit = 1,
  .comp_doorstuck = 0,
  .comp_staylift = 0,
  .comp_zombie = 1,
  .comp_stairs = 0,
  .comp_infcheat = 0,
  .comp_zerotags = 0,

  .comp_moveblock = 0,
  .comp_respawn = 0,
  .comp_sound = 0,
  .comp_666 = 0,
  .comp_soul = 0,
  .comp_maskedanim = 0,
  .comp_ouchface = 0,
  .comp_maxhealth = 0,
  .comp_translucency = 0,
  .comp_ledgeblock = 1,
  .comp_friendlyspawn = 1,
  .comp_voodooscroller = 0,
  .comp_reservedlineflag = 1,
};

static dsda_options_t mbf_options;

typedef struct {
  const char* key;
  int* value;
  int min;
  int max;
  int config_key;
} dsda_option_t;

static dsda_option_t option_list[] = {
  { "weapon_recoil", &mbf_options.weapon_recoil, 0, 1 },
  { "monsters_remember", &mbf_options.monsters_remember, 0, 1 },
  { "monster_infighting", &mbf_options.monster_infighting, 0, 1 },
  { "monster_backing", &mbf_options.monster_backing, 0, 1 },
  { "monster_avoid_hazards", &mbf_options.monster_avoid_hazards, 0, 1 },
  { "monkeys", &mbf_options.monkeys, 0, 1 },
  { "monster_friction", &mbf_options.monster_friction, 0, 1 },
  { "help_friends", &mbf_options.help_friends, 0, 1 },
  { "player_helpers", &mbf_options.player_helpers, 0, 3 },
  { "friend_distance", &mbf_options.friend_distance, 0, 999 },
  { "dog_jumping", &mbf_options.dog_jumping, 0, 1 },
  { "comp_telefrag", &mbf_options.comp_telefrag, 0, 1 },
  { "comp_dropoff", &mbf_options.comp_dropoff, 0, 1 },
  { "comp_vile", &mbf_options.comp_vile, 0, 1 },
  { "comp_pain", &mbf_options.comp_pain, 0, 1 },
  { "comp_skull", &mbf_options.comp_skull, 0, 1 },
  { "comp_blazing", &mbf_options.comp_blazing, 0, 1 },
  { "comp_doorlight", &mbf_options.comp_doorlight, 0, 1 },
  { "comp_model", &mbf_options.comp_model, 0, 1 },
  { "comp_god", &mbf_options.comp_god, 0, 1 },
  { "comp_falloff", &mbf_options.comp_falloff, 0, 1 },
  { "comp_floors", &mbf_options.comp_floors, 0, 1 },
  { "comp_skymap", &mbf_options.comp_skymap, 0, 1 },
  { "comp_pursuit", &mbf_options.comp_pursuit, 0, 1 },
  { "comp_doorstuck", &mbf_options.comp_doorstuck, 0, 1 },
  { "comp_staylift", &mbf_options.comp_staylift, 0, 1 },
  { "comp_zombie", &mbf_options.comp_zombie, 0, 1 },
  { "comp_stairs", &mbf_options.comp_stairs, 0, 1 },
  { "comp_infcheat", &mbf_options.comp_infcheat, 0, 1 },
  { "comp_zerotags", &mbf_options.comp_zerotags, 0, 1 },
  { "comp_respawn", &mbf_options.comp_respawn, 0, 1 },
  { "comp_respawnfix", &mbf_options.comp_respawn, 0, 1 },
  { "comp_soul", &mbf_options.comp_soul, 0, 1 },
  { "comp_ledgeblock", &mbf_options.comp_ledgeblock, 0, 1 },
  { "comp_friendlyspawn", &mbf_options.comp_friendlyspawn, 0, 1 },
  { "comp_voodooscroller", &mbf_options.comp_voodooscroller, 0, 1 },
  { "comp_reservedlineflag", &mbf_options.comp_reservedlineflag, 0, 1 },

  { "mapcolor_back", NULL, 0, 255, dsda_config_mapcolor_back },
  { "mapcolor_grid", NULL, 0, 255, dsda_config_mapcolor_grid },
  { "mapcolor_wall", NULL, 0, 255, dsda_config_mapcolor_wall },
  { "mapcolor_fchg", NULL, 0, 255, dsda_config_mapcolor_fchg },
  { "mapcolor_cchg", NULL, 0, 255, dsda_config_mapcolor_cchg },
  { "mapcolor_clsd", NULL, 0, 255, dsda_config_mapcolor_clsd },
  { "mapcolor_rkey", NULL, 0, 255, dsda_config_mapcolor_rkey },
  { "mapcolor_bkey", NULL, 0, 255, dsda_config_mapcolor_bkey },
  { "mapcolor_ykey", NULL, 0, 255, dsda_config_mapcolor_ykey },
  { "mapcolor_rdor", NULL, 0, 255, dsda_config_mapcolor_rdor },
  { "mapcolor_bdor", NULL, 0, 255, dsda_config_mapcolor_bdor },
  { "mapcolor_ydor", NULL, 0, 255, dsda_config_mapcolor_ydor },
  { "mapcolor_tele", NULL, 0, 255, dsda_config_mapcolor_tele },
  { "mapcolor_secr", NULL, 0, 255, dsda_config_mapcolor_secr },
  { "mapcolor_revsecr", NULL, 0, 255, dsda_config_mapcolor_revsecr },
  { "mapcolor_exit", NULL, 0, 255, dsda_config_mapcolor_exit },
  { "mapcolor_unsn", NULL, 0, 255, dsda_config_mapcolor_unsn },
  { "mapcolor_flat", NULL, 0, 255, dsda_config_mapcolor_flat },
  { "mapcolor_sprt", NULL, 0, 255, dsda_config_mapcolor_sprt },
  { "mapcolor_item", NULL, 0, 255, dsda_config_mapcolor_item },
  { "mapcolor_enemy", NULL, 0, 255, dsda_config_mapcolor_enemy },
  { "mapcolor_frnd", NULL, 0, 255, dsda_config_mapcolor_frnd },
  { "mapcolor_hair", NULL, 0, 255, dsda_config_mapcolor_hair },
  { "mapcolor_sngl", NULL, 0, 255, dsda_config_mapcolor_sngl },
  { "mapcolor_me", NULL, 0, 255, dsda_config_mapcolor_me },
  { 0 }
};

typedef struct {
  dboolean found;
  int value;
} dsda_parsed_option_t;

static dsda_parsed_option_t parsed_option_list[arrlen(option_list)];

#define OPTIONS_LINE_LENGTH 80

typedef struct {
  const char* data;
  int length;
} options_lump_t;

static const char* dsda_ReadOption(char* buf, size_t size, options_lump_t* lump) {
  if (lump->length <= 0)
    return NULL;

  while (size > 1 && *lump->data && lump->length) {
    size--;
    lump->length--;
    if ((*buf++ = *lump->data++) == '\n')
      break;
  }

  *buf = '\0';

  return lump->data;
}

void dsda_ParseOptionsLump(void) {
  options_lump_t lump;
  char buf[OPTIONS_LINE_LENGTH];
  char key[OPTIONS_LINE_LENGTH];
  char* scan;
  int lumpnum;
  int i, value, count;

  lumpnum = W_CheckNumForName("OPTIONS");

  if (lumpnum == LUMP_NOT_FOUND)
    return;

  lump.length = W_LumpLength(lumpnum);
  lump.data = W_LumpByNum(lumpnum);

  while (dsda_ReadOption(buf, OPTIONS_LINE_LENGTH, &lump)) {
    if (buf[0] == '#')
      continue;

    scan = buf;
    count = sscanf(scan, "%79s %d", key, &value);

    if (count != 2)
      continue;

    for (i = 0; option_list[i].key; ++i) {
      if (!strncmp(key, option_list[i].key, OPTIONS_LINE_LENGTH)) {
        if (option_list[i].value) {
          parsed_option_list[i].found = true;
          parsed_option_list[i].value = BETWEEN(option_list[i].min, option_list[i].max, value);
        }
        else
          dsda_UpdateIntConfig(option_list[i].config_key, value, false);

        lprintf(LO_INFO, "dsda_LumpOptions: %s = %d\n", key, value);

        break;
      }
    }
  }

  return;
}

static const dsda_options_t* dsda_MBFOptions(void) {
  int i;

  if (compatibility_level == mbf_compatibility)
    mbf_options = default_mbf_options;
  else
    mbf_options = default_latest_options;

  for (i = 0; option_list[i].value; ++i)
    if (parsed_option_list[i].found)
      *option_list[i].value = parsed_option_list[i].value;

  return &mbf_options;
}

const dsda_options_t* dsda_Options(void) {
  if (demo_compatibility)
    return &default_vanilla_options;

  if (!mbf_features)
    return &default_boom_options;

  return dsda_MBFOptions();
}

#define MBF21_COMP_TOTAL 25

static int mbf21_comp_translation[MBF21_COMP_TOTAL] = {
  comp_telefrag,
  comp_dropoff,
  comp_vile,
  comp_pain,
  comp_skull,
  comp_blazing,
  comp_doorlight,
  comp_model,
  comp_god,
  comp_falloff,
  comp_floors,
  comp_skymap,
  comp_pursuit,
  comp_doorstuck,
  comp_staylift,
  comp_zombie,
  comp_stairs,
  comp_infcheat,
  comp_zerotags,
  comp_respawn,
  comp_soul,
  comp_ledgeblock,
  comp_friendlyspawn,
  comp_voodooscroller,
  comp_reservedlineflag,
};

// killough 5/2/98: number of bytes reserved for saving options
#define MBF_GAME_OPTION_SIZE 64
#define MBF21_GAME_OPTION_SIZE (21 + MBF21_COMP_TOTAL)

int dsda_GameOptionSize(void) {
  return mbf21 ? MBF21_GAME_OPTION_SIZE : MBF_GAME_OPTION_SIZE;
}

byte* dsda_WriteOptions21(byte* demo_p) {
  int i;
  byte *target = demo_p + dsda_GameOptionSize();

  *demo_p++ = monsters_remember;
  *demo_p++ = weapon_recoil;
  *demo_p++ = player_bobbing;

  *demo_p++ = respawnparm;
  *demo_p++ = fastparm;
  *demo_p++ = nomonsters;

  *demo_p++ = (byte)((rngseed >> 24) & 0xff);
  *demo_p++ = (byte)((rngseed >> 16) & 0xff);
  *demo_p++ = (byte)((rngseed >>  8) & 0xff);
  *demo_p++ = (byte)( rngseed        & 0xff);

  *demo_p++ = monster_infighting;
  *demo_p++ = dogs;

  *demo_p++ = (distfriend >> 8) & 0xff;
  *demo_p++ =  distfriend       & 0xff;

  *demo_p++ = monster_backing;
  *demo_p++ = monster_avoid_hazards;
  *demo_p++ = monster_friction;
  *demo_p++ = help_friends;
  *demo_p++ = dog_jumping;
  *demo_p++ = monkeys;

  *demo_p++ = MBF21_COMP_TOTAL;
  for (i = 0; i < MBF21_COMP_TOTAL; i++)
    *demo_p++ = comp[mbf21_comp_translation[i]] != 0;

  if (demo_p != target)
    I_Error("dsda_WriteOptions21: dsda_GameOptionSize is too small");

  return demo_p;
}

const byte *dsda_ReadOptions21(const byte *demo_p) {
  int i, count;

  // not configurable in mbf21
  variable_friction = 1;
  allow_pushers = 1;
  demo_insurance = 0;

  monsters_remember = *demo_p++;
  weapon_recoil = *demo_p++;
  player_bobbing = *demo_p++;

  respawnparm = *demo_p++;
  fastparm = *demo_p++;
  nomonsters = *demo_p++;

  rngseed  = *demo_p++ & 0xff;
  rngseed <<= 8;
  rngseed += *demo_p++ & 0xff;
  rngseed <<= 8;
  rngseed += *demo_p++ & 0xff;
  rngseed <<= 8;
  rngseed += *demo_p++ & 0xff;

  monster_infighting = *demo_p++;
  dogs = *demo_p++;

  distfriend  = *demo_p++ << 8;
  distfriend += *demo_p++;

  monster_backing = *demo_p++;
  monster_avoid_hazards = *demo_p++;
  monster_friction = *demo_p++;
  help_friends = *demo_p++;
  dog_jumping = *demo_p++;
  monkeys = *demo_p++;

  count = *demo_p++;

  if (count > MBF21_COMP_TOTAL)
    I_Error("Encountered unknown mbf21 compatibility options!");

  for (i = 0; i < count; i++)
    comp[mbf21_comp_translation[i]] = *demo_p++;

  // comp_voodooscroller
  if (count < 24)
    comp[mbf21_comp_translation[23]] = 1;

  // comp_reservedlineflag
  if (count < 25)
    comp[mbf21_comp_translation[24]] = 0;

  G_Compatibility();

  return demo_p;
}
