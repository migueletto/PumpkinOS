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

#include <stdio.h>
#include <string.h>

#include "doomstat.h"
#include "m_misc.h"

#include "dsda/args.h"
#include "dsda/episode.h"
#include "dsda/map_format.h"
#include "dsda/mapinfo/doom.h"
#include "dsda/mapinfo/hexen.h"
#include "dsda/mapinfo/u.h"
#include "dsda/mapinfo/legacy.h"

#include "mapinfo.h"

map_info_t map_info;

int dsda_NameToMap(const char* name, int* episode, int* map) {
  int found;

  if (dsda_DoomNameToMap(&found, name, episode, map))
    return found;

  if (dsda_HexenNameToMap(&found, name, episode, map))
    return found;

  if (dsda_UNameToMap(&found, name, episode, map))
    return found;

  dsda_LegacyNameToMap(&found, name, episode, map);

  return found;
}

void dsda_FirstMap(int* episode, int* map) {
  if (dsda_DoomFirstMap(episode, map))
    return;

  if (dsda_HexenFirstMap(episode, map))
    return;

  if (dsda_UFirstMap(episode, map))
    return;

  dsda_LegacyFirstMap(episode, map);
}

void dsda_NewGameMap(int* episode, int* map) {
  if (dsda_DoomNewGameMap(episode, map))
    return;

  if (dsda_HexenNewGameMap(episode, map))
    return;

  if (dsda_UNewGameMap(episode, map))
    return;

  dsda_LegacyNewGameMap(episode, map);
}

void dsda_ResolveWarp(int* args, int arg_count, int* episode, int* map) {
  if (dsda_DoomResolveWarp(args, arg_count, episode, map))
    return;

  if (dsda_HexenResolveWarp(args, arg_count, episode, map))
    return;

  if (dsda_UResolveWarp(args, arg_count, episode, map))
    return;

  dsda_LegacyResolveWarp(args, arg_count, episode, map);
}

void dsda_NextMap(int* episode, int* map) {
  if (dsda_DoomNextMap(episode, map))
    return;

  if (dsda_HexenNextMap(episode, map))
    return;

  if (dsda_UNextMap(episode, map))
    return;

  dsda_LegacyNextMap(episode, map);
}

void dsda_ShowNextLocBehaviour(int* behaviour) {
  if (dsda_DoomShowNextLocBehaviour(behaviour))
    return;

  if (dsda_HexenShowNextLocBehaviour(behaviour))
    return;

  if (dsda_UShowNextLocBehaviour(behaviour))
    return;

  dsda_LegacyShowNextLocBehaviour(behaviour);
}

int dsda_SkipDrawShowNextLoc(void) {
  int skip;

  if (dsda_DoomSkipDrawShowNextLoc(&skip))
    return skip;

  if (dsda_HexenSkipDrawShowNextLoc(&skip))
    return skip;

  if (dsda_USkipDrawShowNextLoc(&skip))
    return skip;

  dsda_LegacySkipDrawShowNextLoc(&skip);

  return skip;
}

static fixed_t dsda_Gravity(void) {
  fixed_t gravity;

  if (dsda_DoomGravity(&gravity))
    return gravity;

  if (dsda_HexenGravity(&gravity))
    return gravity;

  if (dsda_UGravity(&gravity))
    return gravity;

  dsda_LegacyGravity(&gravity);

  return gravity;
}

static fixed_t dsda_AirControl(void) {
  fixed_t air_control;

  if (dsda_DoomAirControl(&air_control))
    return air_control;

  if (dsda_HexenAirControl(&air_control))
    return air_control;

  if (dsda_UAirControl(&air_control))
    return air_control;

  dsda_LegacyAirControl(&air_control);

  return air_control;
}

static map_info_flags_t dsda_MapFlags(void) {
  map_info_flags_t flags;

  if (dsda_DoomMapFlags(&flags))
    return flags;

  if (dsda_HexenMapFlags(&flags))
    return flags;

  if (dsda_UMapFlags(&flags))
    return flags;

  dsda_LegacyMapFlags(&flags);

  return flags;
}

static void dsda_UpdateMapInfo(void) {
  dsda_DoomUpdateMapInfo();
  dsda_HexenUpdateMapInfo();
  dsda_UUpdateMapInfo();
  dsda_LegacyUpdateMapInfo();

  map_info.flags = dsda_MapFlags();
  map_info.gravity = dsda_Gravity();
  map_info.air_control = dsda_AirControl();
}

void dsda_UpdateGameMap(int episode, int map) {
  gameepisode = episode;
  gamemap = map;
  dsda_UpdateMapInfo();
}

void dsda_ResetAirControl(void) {
  map_info.air_control = dsda_AirControl();
}

void dsda_ResetLeaveData(void) {
  memset(&leave_data, 0, sizeof(leave_data));
}

void dsda_UpdateLeaveData(int map, int position, int flags, angle_t angle) {
  leave_data.map = map;
  leave_data.position = position;
  leave_data.flags = flags;
  leave_data.angle = angle;
}

dboolean dsda_FinaleShortcut(void) {
  return map_format.zdoom && leave_data.map == LEAVE_VICTORY;
}

void dsda_UpdateLastMapInfo(void) {
  dsda_DoomUpdateLastMapInfo();
  dsda_HexenUpdateLastMapInfo();
  dsda_UUpdateLastMapInfo();
  dsda_LegacyUpdateLastMapInfo();
}

void dsda_UpdateNextMapInfo(void) {
  dsda_DoomUpdateNextMapInfo();
  dsda_HexenUpdateNextMapInfo();
  dsda_UUpdateNextMapInfo();
  dsda_LegacyUpdateNextMapInfo();
}

int dsda_ResolveCLEV(int* episode, int* map) {
  int clev;

  if (dsda_DoomResolveCLEV(&clev, episode, map))
    return clev;

  if (dsda_HexenResolveCLEV(&clev, episode, map))
    return clev;

  if (dsda_UResolveCLEV(&clev, episode, map))
    return clev;

  dsda_LegacyResolveCLEV(&clev, episode, map);

  return clev;
}

int dsda_ResolveINIT(void) {
  int init;

  if (dsda_DoomResolveINIT(&init))
    return init;

  if (dsda_HexenResolveINIT(&init))
    return init;

  if (dsda_UResolveINIT(&init))
    return init;

  dsda_LegacyResolveINIT(&init);

  return init;
}

int dsda_MusicIndexToLumpNum(int music_index) {
  int lump;

  if (dsda_DoomMusicIndexToLumpNum(&lump, music_index))
    return lump;

  if (dsda_HexenMusicIndexToLumpNum(&lump, music_index))
    return lump;

  if (dsda_UMusicIndexToLumpNum(&lump, music_index))
    return lump;

  dsda_LegacyMusicIndexToLumpNum(&lump, music_index);

  return lump;
}

void dsda_MapMusic(int* music_index, int* music_lump) {
  if (dsda_DoomMapMusic(music_index, music_lump))
    return;

  if (dsda_HexenMapMusic(music_index, music_lump))
    return;

  if (dsda_UMapMusic(music_index, music_lump))
    return;

  dsda_LegacyMapMusic(music_index, music_lump);
}

void dsda_IntermissionMusic(int* music_index, int* music_lump) {
  if (dsda_DoomIntermissionMusic(music_index, music_lump))
    return;

  if (dsda_HexenIntermissionMusic(music_index, music_lump))
    return;

  if (dsda_UIntermissionMusic(music_index, music_lump))
    return;

  dsda_LegacyIntermissionMusic(music_index, music_lump);
}

void dsda_InterMusic(int* music_index, int* music_lump) {
  if (dsda_DoomInterMusic(music_index, music_lump))
    return;

  if (dsda_HexenInterMusic(music_index, music_lump))
    return;

  if (dsda_UInterMusic(music_index, music_lump))
    return;

  dsda_LegacyInterMusic(music_index, music_lump);
}

typedef enum {
  finale_owner_legacy,
  finale_owner_u,
  finale_owner_hexen,
  finale_owner_doom,
} finale_owner_t;

static finale_owner_t finale_owner = finale_owner_legacy;

void dsda_StartFinale(void) {
  if (dsda_DoomStartFinale()) {
    finale_owner = finale_owner_doom;
    return;
  }

  if (dsda_HexenStartFinale()) {
    finale_owner = finale_owner_hexen;
    return;
  }

  if (dsda_UStartFinale()) {
    finale_owner = finale_owner_u;
    return;
  }

  dsda_LegacyStartFinale();
  finale_owner = finale_owner_legacy;
}

int dsda_FTicker(void) {
  if (finale_owner == finale_owner_doom) {
    if (!dsda_DoomFTicker())
      finale_owner = finale_owner_legacy;

    return true;
  }

  if (finale_owner == finale_owner_hexen) {
    if (!dsda_HexenFTicker())
      finale_owner = finale_owner_legacy;

    return true;
  }

  if (finale_owner == finale_owner_u) {
    if (!dsda_UFTicker())
      finale_owner = finale_owner_legacy;

    return true;
  }

  dsda_LegacyFTicker();
  return false;
}

int dsda_FDrawer(void) {
  if (finale_owner == finale_owner_doom) {
    dsda_DoomFDrawer();

    return true;
  }

  if (finale_owner == finale_owner_hexen) {
    dsda_HexenFDrawer();

    return true;
  }

  if (finale_owner == finale_owner_u) {
    dsda_UFDrawer();

    return true;
  }

  dsda_LegacyFDrawer();
  return false;
}

int dsda_BossAction(mobj_t* mo) {
  if (dsda_DoomBossAction(mo))
    return true;

  if (dsda_HexenBossAction(mo))
    return true;

  if (dsda_UBossAction(mo))
    return true;

  dsda_LegacyBossAction(mo);
  return false;
}

const char* dsda_MapLumpName(int episode, int map) {
  const char* name;

  if (dsda_DoomMapLumpName(&name, episode, map))
    return name;

  if (dsda_HexenMapLumpName(&name, episode, map))
    return name;

  if (dsda_UMapLumpName(&name, episode, map))
    return name;

  dsda_LegacyMapLumpName(&name, episode, map);

  return name;
}

void dsda_HUTitle(dsda_string_t* str) {
  if (dsda_DoomHUTitle(str))
    return;

  if (dsda_HexenHUTitle(str))
    return;

  if (dsda_UHUTitle(str))
    return;

  dsda_LegacyHUTitle(str);
}

const char* dsda_MapAuthor(void) {
  const char* author;

  if (dsda_DoomMapAuthor(&author))
    return author;

  if (dsda_HexenMapAuthor(&author))
    return author;

  if (dsda_UMapAuthor(&author))
    return author;

  dsda_LegacyMapAuthor(&author);

  return author;
}

int dsda_SkyTexture(void) {
  int sky;

  if (dsda_DoomSkyTexture(&sky))
    return sky;

  if (dsda_HexenSkyTexture(&sky))
    return sky;

  if (dsda_USkyTexture(&sky))
    return sky;

  dsda_LegacySkyTexture(&sky);

  return sky;
}

void dsda_PrepareInitNew(void) {
  if (dsda_DoomPrepareInitNew())
    return;

  if (dsda_HexenPrepareInitNew())
    return;

  if (dsda_UPrepareInitNew())
    return;

  dsda_LegacyPrepareInitNew();
}

void dsda_PrepareIntermission(int* behaviour) {
  if (dsda_DoomPrepareIntermission(behaviour))
    return;

  if (dsda_HexenPrepareIntermission(behaviour))
    return;

  if (dsda_UPrepareIntermission(behaviour))
    return;

  dsda_LegacyPrepareIntermission(behaviour);
}

void dsda_PrepareFinale(int* behaviour) {
  if (dsda_DoomPrepareFinale(behaviour))
    return;

  if (dsda_HexenPrepareFinale(behaviour))
    return;

  if (dsda_UPrepareFinale(behaviour))
    return;

  dsda_LegacyPrepareFinale(behaviour);
}

void dsda_LoadMapInfo(void) {
  dsda_AddOriginalEpisodes();

  dsda_DoomLoadMapInfo();
  dsda_HexenLoadMapInfo();
  dsda_ULoadMapInfo();
  dsda_LegacyLoadMapInfo();

  dsda_AddCustomEpisodes();
}

const char* dsda_ExitPic(void) {
  const char* exit_pic;

  if (dsda_DoomExitPic(&exit_pic))
    return exit_pic;

  if (dsda_HexenExitPic(&exit_pic))
    return exit_pic;

  if (dsda_UExitPic(&exit_pic))
    return exit_pic;

  dsda_LegacyExitPic(&exit_pic);
  return exit_pic;
}

const char* dsda_EnterPic(void) {
  const char* enter_pic;

  if (dsda_DoomEnterPic(&enter_pic))
    return enter_pic;

  if (dsda_HexenEnterPic(&enter_pic))
    return enter_pic;

  if (dsda_UEnterPic(&enter_pic))
    return enter_pic;

  dsda_LegacyEnterPic(&enter_pic);
  return enter_pic;
}

const char* dsda_BorderTexture(void) {
  const char* border_texture;

  if (dsda_DoomBorderTexture(&border_texture))
    return border_texture;

  if (dsda_HexenBorderTexture(&border_texture))
    return border_texture;

  if (dsda_UBorderTexture(&border_texture))
    return border_texture;

  dsda_LegacyBorderTexture(&border_texture);
  return border_texture;
}

void dsda_PrepareEntering(void) {
  if (dsda_DoomPrepareEntering())
    return;

  if (dsda_HexenPrepareEntering())
    return;

  if (dsda_UPrepareEntering())
    return;

  dsda_LegacyPrepareEntering();
}

void dsda_PrepareFinished(void) {
  if (dsda_DoomPrepareFinished())
    return;

  if (dsda_HexenPrepareFinished())
    return;

  if (dsda_UPrepareFinished())
    return;

  dsda_LegacyPrepareFinished();
}

int dsda_MapLightning(void) {
  int lightning;

  if (dsda_DoomMapLightning(&lightning))
    return lightning;

  if (dsda_HexenMapLightning(&lightning))
    return lightning;

  if (dsda_UMapLightning(&lightning))
    return lightning;

  dsda_LegacyMapLightning(&lightning);

  return lightning;
}

void dsda_ApplyFadeTable(void) {
  if (dsda_DoomApplyFadeTable())
    return;

  if (dsda_HexenApplyFadeTable())
    return;

  if (dsda_UApplyFadeTable())
    return;

  dsda_LegacyApplyFadeTable();
}

int dsda_MapCluster(int map) {
  int cluster;

  if (dsda_DoomMapCluster(&cluster, map))
    return cluster;

  if (dsda_HexenMapCluster(&cluster, map))
    return cluster;

  if (dsda_UMapCluster(&cluster, map))
    return cluster;

  dsda_LegacyMapCluster(&cluster, map);

  return cluster;
}

short dsda_Sky1Texture(void) {
  short texture;

  if (dsda_DoomSky1Texture(&texture))
    return texture;

  if (dsda_HexenSky1Texture(&texture))
    return texture;

  if (dsda_USky1Texture(&texture))
    return texture;

  dsda_LegacySky1Texture(&texture);

  return texture;
}

short dsda_Sky2Texture(void) {
  short texture;

  if (dsda_DoomSky2Texture(&texture))
    return texture;

  if (dsda_HexenSky2Texture(&texture))
    return texture;

  if (dsda_USky2Texture(&texture))
    return texture;

  dsda_LegacySky2Texture(&texture);

  return texture;
}

void dsda_InitSky(void) {
  if (dsda_DoomInitSky())
    return;

  if (dsda_HexenInitSky())
    return;

  if (dsda_UInitSky())
    return;

  dsda_LegacyInitSky();
}
