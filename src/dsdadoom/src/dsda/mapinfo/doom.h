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
//  DSDA MapInfo Doom
//

#ifndef __DSDA_MAPINFO_DOOM__
#define __DSDA_MAPINFO_DOOM__

#include "p_mobj.h"

#include "dsda/mapinfo.h"
#include "dsda/utility.h"

int dsda_DoomNameToMap(int* found, const char* name, int* episode, int* map);
int dsda_DoomFirstMap(int* episode, int* map);
int dsda_DoomNewGameMap(int* episode, int* map);
int dsda_DoomResolveWarp(int* args, int arg_count, int* episode, int* map);
int dsda_DoomNextMap(int* episode, int* map);
int dsda_DoomShowNextLocBehaviour(int* behaviour);
int dsda_DoomSkipDrawShowNextLoc(int* skip);
void dsda_DoomUpdateMapInfo(void);
void dsda_DoomUpdateLastMapInfo(void);
void dsda_DoomUpdateNextMapInfo(void);
int dsda_DoomResolveCLEV(int* clev, int* episode, int* map);
int dsda_DoomResolveINIT(int* init);
int dsda_DoomMusicIndexToLumpNum(int* lump, int music_index);
int dsda_DoomMapMusic(int* music_index, int* music_lump);
int dsda_DoomIntermissionMusic(int* music_index, int* music_lump);
int dsda_DoomInterMusic(int* music_index, int* music_lump);
int dsda_DoomStartFinale(void);
int dsda_DoomFTicker(void);
void dsda_DoomFDrawer(void);
int dsda_DoomBossAction(mobj_t* mo);
int dsda_DoomMapLumpName(const char** name, int episode, int map);
int dsda_DoomMapAuthor(const char** author);
int dsda_DoomHUTitle(dsda_string_t* str);
int dsda_DoomSkyTexture(int* sky);
int dsda_DoomPrepareInitNew(void);
int dsda_DoomPrepareIntermission(int* result);
int dsda_DoomPrepareFinale(int* result);
void dsda_DoomLoadMapInfo(void);
int dsda_DoomExitPic(const char** exit_pic);
int dsda_DoomEnterPic(const char** enter_pic);
int dsda_DoomBorderTexture(const char** border_texture);
int dsda_DoomPrepareEntering(void);
int dsda_DoomPrepareFinished(void);
int dsda_DoomMapLightning(int* lightning);
int dsda_DoomApplyFadeTable(void);
int dsda_DoomMapCluster(int* cluster, int map);
int dsda_DoomSky1Texture(short* texture);
int dsda_DoomSky2Texture(short* texture);
int dsda_DoomGravity(fixed_t* gravity);
int dsda_DoomAirControl(fixed_t* air_control);
int dsda_DoomInitSky(void);
int dsda_DoomMapFlags(map_info_flags_t* flags);

#endif
