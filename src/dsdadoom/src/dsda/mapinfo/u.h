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
//	DSDA MapInfo U
//

#ifndef __DSDA_MAPINFO_U__
#define __DSDA_MAPINFO_U__

#include "p_mobj.h"

#include "dsda/mapinfo.h"
#include "dsda/utility.h"

int dsda_UNameToMap(int* found, const char* name, int* episode, int* map);
int dsda_UFirstMap(int* episode, int* map);
int dsda_UNewGameMap(int* episode, int* map);
int dsda_UResolveWarp(int* args, int arg_count, int* episode, int* map);
int dsda_UNextMap(int* episode, int* map);
int dsda_UShowNextLocBehaviour(int* behaviour);
int dsda_USkipDrawShowNextLoc(int* skip);
void dsda_UUpdateMapInfo(void);
void dsda_UUpdateLastMapInfo(void);
void dsda_UUpdateNextMapInfo(void);
int dsda_UResolveCLEV(int* clev, int* episode, int* map);
int dsda_UResolveINIT(int* init);
int dsda_UMusicIndexToLumpNum(int* lump, int music_index);
int dsda_UMapMusic(int* music_index, int* music_lump);
int dsda_UIntermissionMusic(int* music_index, int* music_lump);
int dsda_UInterMusic(int* music_index, int* music_lump);
int dsda_UStartFinale(void);
int dsda_UFTicker(void);
void dsda_UFDrawer(void);
int dsda_UBossAction(mobj_t* mo);
int dsda_UMapLumpName(const char** name, int episode, int map);
int dsda_UMapAuthor(const char** author);
int dsda_UHUTitle(dsda_string_t* str);
int dsda_USkyTexture(int* sky);
int dsda_UPrepareInitNew(void);
int dsda_UPrepareIntermission(int* result);
int dsda_UPrepareFinale(int* result);
void dsda_ULoadMapInfo(void);
int dsda_UExitPic(const char** exit_pic);
int dsda_UEnterPic(const char** enter_pic);
int dsda_UBorderTexture(const char** border_texture);
int dsda_UPrepareEntering(void);
int dsda_UPrepareFinished(void);
int dsda_UMapLightning(int* lightning);
int dsda_UApplyFadeTable(void);
int dsda_UMapCluster(int* cluster, int map);
int dsda_USky1Texture(short* texture);
int dsda_USky2Texture(short* texture);
int dsda_UGravity(fixed_t* gravity);
int dsda_UAirControl(fixed_t* air_control);
int dsda_UInitSky(void);
int dsda_UMapFlags(map_info_flags_t* flags);

#endif
