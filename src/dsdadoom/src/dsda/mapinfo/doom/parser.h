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
//	DSDA MAPINFO Doom Parser
//

#ifndef __DSDA_MAPINFO_DOOM_PARSER__
#define __DSDA_MAPINFO_DOOM_PARSER__

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

typedef enum {
  dmi_end_null,
  dmi_end_game_scroll,
  dmi_end_game_cast,
} dmi_end_t;

typedef struct {
  char* map;
  char* end_pic;
  char* end_pic_b;
  char* music;
  dboolean loop_music;
  dmi_end_t end;
} doom_mapinfo_map_next_t;

typedef struct {
  char* lump;
  // float scrollspeed;
} doom_mapinfo_sky_t;

typedef struct {
  int monster_type;
  int action_special;
  int special_args[5];
} doom_mapinfo_special_action_t;

#define DMI_INTERMISSION                   0x00000001ul
#define DMI_ALLOW_MONSTER_TELEFRAGS        0x00000002ul
#define DMI_ACTIVATE_OWN_DEATH_SPECIALS    0x00000004ul
#define DMI_LAX_MONSTER_ACTIVATION         0x00000008ul
#define DMI_MISSILES_ACTIVATE_IMPACT_LINES 0x00000010ul
#define DMI_FILTER_STARTS                  0x00000020ul
#define DMI_ALLOW_RESPAWN                  0x00000040ul
#define DMI_ALLOW_JUMP                     0x00000080ul
#define DMI_CHECK_SWITCH_RANGE             0x00000200ul
#define DMI_RESET_HEALTH                   0x00000400ul
#define DMI_RESET_INVENTORY                0x00000800ul
#define DMI_USE_PLAYER_START_Z             0x00001000ul
#define DMI_SHOW_AUTHOR                    0x00008000ul
#define DMI_PASSOVER                       0x00010000ul
#define DMI_EVEN_LIGHTING                  0x00020000ul
#define DMI_SMOOTH_LIGHTING                0x00040000ul

typedef uint32_t doom_mapinfo_map_flags_t;

typedef struct {
  char* lump_name;
  char* nice_name;
  char* author;
  int level_num;
  int cluster;
  doom_mapinfo_map_next_t next;
  doom_mapinfo_map_next_t secret_next;
  doom_mapinfo_sky_t sky1;
  char* title_patch;
  char* exit_pic;
  char* enter_pic;
  char* border_texture;
  char* music;
  char* inter_music;
  int par;
  char* gravity;
  char* air_control;
  size_t num_special_actions;
  doom_mapinfo_special_action_t* special_actions;
  doom_mapinfo_map_flags_t flags;
} doom_mapinfo_map_t;

#define DSI_SPAWN_MULTI      0x0001
#define DSI_FAST_MONSTERS    0x0002
#define DSI_INSTANT_REACTION 0x0004
#define DSI_NO_PAIN          0x0010
#define DSI_DEFAULT_SKILL    0x0020
#define DSI_PLAYER_RESPAWN   0x0080
#define DSI_EASY_BOSS_BRAIN  0x0100
#define DSI_MUST_CONFIRM     0x0200
#define DSI_AUTO_USE_HEALTH  0x0400 // not applicable to doom

typedef uint16_t doom_mapinfo_skill_flags_t;

typedef struct {
  char* unique_id;
  char* ammo_factor;
  char* damage_factor;
  char* armor_factor;
  char* health_factor;
  char* monster_health_factor;
  char* friend_health_factor;
  int respawn_time;
  int spawn_filter;
  char key;
  char* must_confirm;
  char* name;
  char* pic_name;
  char* text_color;
  doom_mapinfo_skill_flags_t flags;
} doom_mapinfo_skill_t;

typedef struct {
  int id;
  char* enter_text;
  char* exit_text;
  char* music;
  char* flat;
  char* pic;
} doom_mapinfo_cluster_t;

typedef struct {
  char* map_lump;
  char* name;
  char* pic_name;
  char key;
} doom_mapinfo_episode_t;

typedef struct {
  size_t num_maps;
  doom_mapinfo_map_t* maps;
  size_t num_skills;
  doom_mapinfo_skill_t* skills;
  size_t num_clusters;
  doom_mapinfo_cluster_t* clusters;
  size_t num_episodes;
  doom_mapinfo_episode_t* episodes;
  int loaded;
  int skills_cleared;
  int episodes_cleared;
} doom_mapinfo_t;

extern doom_mapinfo_t doom_mapinfo;

typedef void (*doom_mapinfo_errorfunc)(const char *fmt, ...);	// this must not return!

void dsda_ParseDoomMapInfo(const unsigned char* buffer, size_t length, doom_mapinfo_errorfunc err);

#ifdef __cplusplus
}
#endif

#endif
