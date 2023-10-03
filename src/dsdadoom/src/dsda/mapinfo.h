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
//	DSDA MapInfo
//

#ifndef __DSDA_MAPINFO__
#define __DSDA_MAPINFO__

#include "p_mobj.h"

#include "dsda/utility.h"

#define WI_SHOW_NEXT_LOC      0x01
#define WI_SHOW_NEXT_DONE     0x02
#define WI_SHOW_NEXT_EPISODAL 0x04

#define DC_VICTORY 0x01

#define WD_VICTORY      0x01
#define WD_START_FINALE 0x02

#define MI_INTERMISSION                   0x00000001ul
#define MI_ALLOW_MONSTER_TELEFRAGS        0x00000002ul
#define MI_ACTIVATE_OWN_DEATH_SPECIALS    0x00000004ul
#define MI_LAX_MONSTER_ACTIVATION         0x00000008ul
#define MI_MISSILES_ACTIVATE_IMPACT_LINES 0x00000010ul
#define MI_FILTER_STARTS                  0x00000020ul
#define MI_ALLOW_RESPAWN                  0x00000040ul
#define MI_ALLOW_JUMP                     0x00000080ul
#define MI_CHECK_SWITCH_RANGE             0x00000200ul
#define MI_RESET_HEALTH                   0x00000400ul
#define MI_RESET_INVENTORY                0x00000800ul
#define MI_USE_PLAYER_START_Z             0x00001000ul
#define MI_SHOW_AUTHOR                    0x00008000ul
#define MI_PASSOVER                       0x00010000ul
#define MI_EVEN_LIGHTING                  0x00020000ul
#define MI_SMOOTH_LIGHTING                0x00040000ul

typedef uint32_t map_info_flags_t;

typedef struct {
  fixed_t gravity;
  fixed_t air_control;
  map_info_flags_t flags;
} map_info_t;

extern map_info_t map_info;

void dsda_FirstMap(int* episode, int* map);
void dsda_NewGameMap(int* episode, int* map);
void dsda_ResolveWarp(int* args, int arg_count, int* episode, int* map);
int dsda_NameToMap(const char* name, int* episode, int* map);
void dsda_NextMap(int* episode, int* map);
void dsda_ShowNextLocBehaviour(int* behaviour);
int dsda_SkipDrawShowNextLoc(void);
void dsda_UpdateGameMap(int episode, int map);
void dsda_ResetLeaveData(void);
void dsda_UpdateLeaveData(int map, int position, int flags, angle_t angle);
dboolean dsda_FinaleShortcut(void);
void dsda_UpdateLastMapInfo(void);
void dsda_UpdateNextMapInfo(void);
int dsda_ResolveCLEV(int* episode, int* map);
int dsda_ResolveINIT(void);
int dsda_MusicIndexToLumpNum(int music_index);
void dsda_MapMusic(int* music_index, int* music_lump);
void dsda_IntermissionMusic(int* music_index, int* music_lump);
void dsda_InterMusic(int* music_index, int* music_lump);
void dsda_StartFinale(void);
int dsda_FTicker(void);
int dsda_FDrawer(void);
int dsda_BossAction(mobj_t* mo);
const char* dsda_MapLumpName(int episode, int map);
const char* dsda_MapAuthor(void);
void dsda_HUTitle(dsda_string_t* str);
int dsda_SkyTexture(void);
void dsda_PrepareInitNew(void);
void dsda_PrepareIntermission(int* behaviour);
void dsda_PrepareFinale(int* behaviour);
void dsda_LoadMapInfo(void);
const char* dsda_ExitPic(void);
const char* dsda_EnterPic(void);
const char* dsda_BorderTexture(void);
void dsda_PrepareEntering(void);
void dsda_PrepareFinished(void);
int dsda_MapLightning(void);
void dsda_ApplyFadeTable(void);
int dsda_MapCluster(int map);
short dsda_Sky1Texture(void);
short dsda_Sky2Texture(void);
void dsda_InitSky(void);

#endif
