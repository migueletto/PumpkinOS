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
//  DSDA MapInfo Hexen
//

#include "doomstat.h"
#include "g_game.h"
#include "lprintf.h"
#include "m_misc.h"
#include "p_setup.h"
#include "r_data.h"
#include "s_sound.h"
#include "sc_man.h"
#include "sounds.h"
#include "w_wad.h"

#include "hexen/p_acs.h"
#include "hexen/sv_save.h"

#include "dsda/map_format.h"
#include "dsda/mapinfo.h"
#include "dsda/sndinfo.h"

#include "hexen.h"

#define MAPINFO_SCRIPT_NAME "MAPINFO"
#define MCMD_SKY1 1
#define MCMD_SKY2 2
#define MCMD_LIGHTNING 3
#define MCMD_FADETABLE 4
#define MCMD_DOUBLESKY 5
#define MCMD_CLUSTER 6
#define MCMD_WARPTRANS 7
#define MCMD_NEXT 8
#define MCMD_CDTRACK 9
#define MCMD_CD_STARTTRACK 10
#define MCMD_CD_END1TRACK 11
#define MCMD_CD_END2TRACK 12
#define MCMD_CD_END3TRACK 13
#define MCMD_CD_INTERTRACK 14
#define MCMD_CD_TITLETRACK 15

#define UNKNOWN_MAP_NAME "DEVELOPMENT MAP"
#define DEFAULT_SKY_NAME "SKY1"
#define DEFAULT_FADE_TABLE "COLORMAP"

typedef struct mapInfo_s {
  short cluster;
  short warpTrans;
  short nextMap;
  char name[32];
  short sky1Texture;
  short sky2Texture;
  fixed_t sky1ScrollDelta;
  fixed_t sky2ScrollDelta;
  dboolean doubleSky;
  dboolean lightning;
  int fadetable;
  char songLump[10];
} mapInfo_t;

static int MapCount = 98;

static mapInfo_t MapInfo[99];

static mapInfo_t *CurrentMap = MapInfo;

static const char *MapCmdNames[] = {
  "SKY1",
  "SKY2",
  "DOUBLESKY",
  "LIGHTNING",
  "FADETABLE",
  "CLUSTER",
  "WARPTRANS",
  "NEXT",
  "CDTRACK",
  "CD_START_TRACK",
  "CD_END1_TRACK",
  "CD_END2_TRACK",
  "CD_END3_TRACK",
  "CD_INTERMISSION_TRACK",
  "CD_TITLE_TRACK",
  NULL
};

static int MapCmdIDs[] = {
  MCMD_SKY1,
  MCMD_SKY2,
  MCMD_DOUBLESKY,
  MCMD_LIGHTNING,
  MCMD_FADETABLE,
  MCMD_CLUSTER,
  MCMD_WARPTRANS,
  MCMD_NEXT,
  MCMD_CDTRACK,
  MCMD_CD_STARTTRACK,
  MCMD_CD_END1TRACK,
  MCMD_CD_END2TRACK,
  MCMD_CD_END3TRACK,
  MCMD_CD_INTERTRACK,
  MCMD_CD_TITLETRACK
};

static int QualifyMap(int map) {
  return (map < 1 || map > MapCount) ? 0 : map;
}

static int P_TranslateMap(int map) {
  int i;

  for (i = 1; i < 99; i++)
    if (MapInfo[i].warpTrans == map)
      return i;

  return -1;
}

int dsda_HexenNameToMap(int* found, const char* name, int* episode, int* map) {
  return false;
}

int dsda_HexenFirstMap(int* episode, int* map) {
  if (!hexen)
    return false;

  *episode = 1;
  *map = P_TranslateMap(1);

  if (*map == -1)
    I_Error("Unable to detect default first map");

  return true;
}

int dsda_HexenNewGameMap(int* episode, int* map) {
  if (!hexen)
    return false;

  *episode = 1;
  *map = P_TranslateMap(*map);

  if (*map == -1)
    *map = 1;

  return true;
}

int dsda_HexenResolveWarp(int* args, int arg_count, int* episode, int* map) {
  if (!hexen)
    return false;

  *episode = 1;

  if (arg_count)
    *map = P_TranslateMap(args[0]);
  else
    *map = P_TranslateMap(1);

  if (*map == -1)
    I_Error("-warp: Invalid map number.\n");

  return true;
}

int dsda_HexenNextMap(int* episode, int* map) {
  if (!hexen)
    return false;

  *episode = 1;
  *map = CurrentMap->nextMap;

  return true;
}

int dsda_HexenShowNextLocBehaviour(int* behaviour) {
  return false; // TODO
}

int dsda_HexenSkipDrawShowNextLoc(int* skip) {
  return false; // TODO
}

void dsda_HexenUpdateMapInfo(void) {
  CurrentMap = &MapInfo[QualifyMap(gamemap)];
}

void dsda_HexenUpdateLastMapInfo(void) {
  // TODO
}

void dsda_HexenUpdateNextMapInfo(void) {
  // TODO
}

int dsda_HexenResolveCLEV(int* clev, int* episode, int* map) {
  char* next;

  if (!hexen)
    return false;

  // Catch invalid maps
  next = VANILLA_MAP_LUMP_NAME(*episode, P_TranslateMap(*map));
  if (!W_LumpNameExists(next)) {
    doom_printf("IDCLEV target not found: %s", next);
    *clev = false;
  }
  else
    *clev = true;

  return true;
}

dboolean partial_reset = false;

int dsda_HexenResolveINIT(int* init) {
  if (!hexen)
    return false;

  partial_reset = true;

  G_DeferedInitNew(gameskill, gameepisode, CurrentMap->warpTrans);

  *init = true;

  return true;
}

int dsda_HexenMusicIndexToLumpNum(int* lump, int music_index) {
  const char* lump_name;

  if (!hexen)
    return false;

  if (music_index >= hexen_mus_hub)
    return false;

  if (!map_format.sndinfo)
    return false;

  lump_name = dsda_SndInfoMapSongLumpName(music_index);
  if (!*lump_name)
    switch (music_index)
    {
    case hexen_mus_hexen:
    case hexen_mus_hub:
    case hexen_mus_hall:
    case hexen_mus_orb:
    case hexen_mus_chess:
      lump_name = S_music[music_index].name;
      break;
    }

  if (!*lump_name)
    *lump = 0;
  else
    *lump = W_GetNumForName(lump_name);

  return true;
}

int dsda_HexenMapMusic(int* music_index, int* music_lump) {
  if (!hexen)
    return false;

  if (!map_format.sndinfo)
    return false;

  *music_lump = -1;
  *music_index = gamemap;

  return true;
}

int dsda_HexenIntermissionMusic(int* music_index, int* music_lump) {
  return false; // TODO
}

int dsda_HexenInterMusic(int* music_index, int* music_lump) {
  return false; // TODO
}

int dsda_HexenStartFinale(void) {
  return false; // TODO
}

int dsda_HexenFTicker(void) {
  return false; // TODO
}

void dsda_HexenFDrawer(void) {
  return; // TODO
}

int dsda_HexenBossAction(mobj_t* mo) {
  return false; // TODO
}

int dsda_HexenMapLumpName(const char** name, int episode, int map) {
  return false; // TODO
}

int dsda_HexenMapAuthor(const char** author) {
  return false;
}

int dsda_HexenHUTitle(dsda_string_t* str) {
  if (!hexen)
    return false;

  dsda_InitString(str, NULL);

  if (gamestate == GS_LEVEL && gamemap > 0 && gameepisode > 0)
    dsda_StringCat(str, CurrentMap->name);

  if (!str->string)
    dsda_StringCat(str, VANILLA_MAP_LUMP_NAME(gameepisode, gamemap));

  return true;
}

int dsda_HexenSkyTexture(int* sky) {
  return false; // TODO
}

int dsda_HexenPrepareInitNew(void) {
  extern int RebornPosition;

  if (!hexen)
    return false;

  SV_Init();

  if (partial_reset) {
    partial_reset = false;
    return true;
  }

  if (map_format.acs)
    P_ACSInitNewGame();

  // Default the player start spot group to 0
  RebornPosition = 0;

  return true;
}

int dsda_HexenPrepareIntermission(int* result) {
  if (!hexen)
    return false;

  if (leave_data.map == LEAVE_VICTORY && leave_data.position == LEAVE_VICTORY)
    *result = DC_VICTORY;
  else
    *result = 0;

  return true;
}

int dsda_HexenPrepareFinale(int* result) {
  return false; // TODO
}

void dsda_HexenLoadMapInfo(void) {
  int map;
  int mapMax;
  int mcmdValue;
  mapInfo_t* info;
  const char* default_sky_name = DEFAULT_SKY_NAME;

  if (!hexen)
    return;

  mapMax = 1;

  if (gamemode == shareware)
    default_sky_name = "SKY2";

  // Put defaults into MapInfo[0]
  info = MapInfo;
  info->cluster = 0;
  info->warpTrans = 0;
  info->nextMap = 1; // Always go to map 1 if not specified
  info->sky1Texture = R_TextureNumForName(default_sky_name);
  info->sky2Texture = info->sky1Texture;
  info->sky1ScrollDelta = 0;
  info->sky2ScrollDelta = 0;
  info->doubleSky = false;
  info->lightning = false;
  info->fadetable = W_GetNumForName(DEFAULT_FADE_TABLE);
  M_StringCopy(info->name, UNKNOWN_MAP_NAME, sizeof(info->name));

  SC_OpenLump(MAPINFO_SCRIPT_NAME);
  while (SC_GetString()) {
    if (SC_Compare("MAP") == false)
      SC_ScriptError(NULL);

    SC_MustGetNumber();
    if (sc_Number < 1 || sc_Number > 99)
      SC_ScriptError(NULL);

    map = sc_Number;

    info = &MapInfo[map];

    // Copy defaults to current map definition
    memcpy(info, &MapInfo[0], sizeof(*info));

    // The warp translation defaults to the map number
    info->warpTrans = map;

    // Map name must follow the number
    SC_MustGetString();
    M_StringCopy(info->name, sc_String, sizeof(info->name));

    // Process optional tokens
    while (SC_GetString()) {
      if (SC_Compare("MAP")) { // Start next map definition
        SC_UnGet();
        break;
      }

      mcmdValue = MapCmdIDs[SC_MustMatchString(MapCmdNames)];
      switch (mcmdValue) {
        case MCMD_CLUSTER:
          SC_MustGetNumber();
          info->cluster = sc_Number;
          break;
        case MCMD_WARPTRANS:
          SC_MustGetNumber();
          info->warpTrans = sc_Number;
          break;
        case MCMD_NEXT:
          SC_MustGetNumber();
          info->nextMap = sc_Number;
          break;
        case MCMD_CDTRACK:
          SC_MustGetNumber();
          // not used
          break;
        case MCMD_SKY1:
          SC_MustGetString();
          info->sky1Texture = R_TextureNumForName(sc_String);
          SC_MustGetNumber();
          info->sky1ScrollDelta = sc_Number << 8;
          break;
        case MCMD_SKY2:
          SC_MustGetString();
          info->sky2Texture = R_TextureNumForName(sc_String);
          SC_MustGetNumber();
          info->sky2ScrollDelta = sc_Number << 8;
          break;
        case MCMD_DOUBLESKY:
          info->doubleSky = true;
          break;
        case MCMD_LIGHTNING:
          info->lightning = true;
          break;
        case MCMD_FADETABLE:
          SC_MustGetString();
          info->fadetable = W_GetNumForName(sc_String);
          break;
        case MCMD_CD_STARTTRACK:
        case MCMD_CD_END1TRACK:
        case MCMD_CD_END2TRACK:
        case MCMD_CD_END3TRACK:
        case MCMD_CD_INTERTRACK:
        case MCMD_CD_TITLETRACK:
          SC_MustGetNumber();
          // not used
          break;
      }
    }
    mapMax = map > mapMax ? map : mapMax;
  }

  SC_Close();

  MapCount = mapMax;
}

int dsda_HexenExitPic(const char** exit_pic) {
  return false; // TODO
}

int dsda_HexenEnterPic(const char** enter_pic) {
  return false; // TODO
}

int dsda_HexenBorderTexture(const char** border_texture) {
  if (!hexen)
    return false;

  *border_texture = "F_022";

  return true;
}

int dsda_HexenPrepareEntering(void) {
  return false; // TODO
}

int dsda_HexenPrepareFinished(void) {
  return false; // TODO
}

int dsda_HexenMapLightning(int* lightning) {
  if (!hexen)
    return false;

  *lightning = CurrentMap->lightning;

  return true;
}

int dsda_HexenApplyFadeTable(void) {
  extern dboolean LevelUseFullBright;
  extern const lighttable_t** colormaps;

  int fade_lump;

  if (!hexen)
    return false;

  fade_lump = CurrentMap->fadetable;

  colormaps[0] = (const lighttable_t *) W_LumpByNum(fade_lump);

  if (fade_lump == W_GetNumForName("COLORMAP"))
    LevelUseFullBright = true;
  else
    LevelUseFullBright = false; // Probably fog ... don't use fullbright sprites

  return true;
}

int dsda_HexenMapCluster(int* cluster, int map) {
  if (!hexen)
    return false;

  *cluster = MapInfo[QualifyMap(map)].cluster;

  return true;
}

int dsda_HexenSky1Texture(short* texture) {
  if (!hexen)
    return false;

  *texture = CurrentMap->sky1Texture;

  return true;
}

int dsda_HexenSky2Texture(short* texture) {
  if (!hexen)
    return false;

  *texture = CurrentMap->sky2Texture;

  return true;
}

int dsda_HexenGravity(fixed_t* gravity) {
  return false;
}

int dsda_HexenAirControl(fixed_t* air_control) {
  if (!hexen)
    return false;

  *air_control = (FRACUNIT >> 8);

  return true;
}

int dsda_HexenInitSky(void) {
  extern fixed_t Sky1ScrollDelta;
  extern fixed_t Sky2ScrollDelta;

  if (!hexen)
    return false;

  Sky1Texture = CurrentMap->sky1Texture;
  Sky2Texture = CurrentMap->sky2Texture;
  Sky1ScrollDelta = CurrentMap->sky1ScrollDelta;
  Sky2ScrollDelta = CurrentMap->sky2ScrollDelta;
  Sky1ColumnOffset = 0;
  Sky2ColumnOffset = 0;
  DoubleSky = CurrentMap->doubleSky;

  return true;
}

int dsda_HexenMapFlags(map_info_flags_t* flags) {
  if (!hexen)
    return false;

  *flags = MI_INTERMISSION |
           MI_ACTIVATE_OWN_DEATH_SPECIALS |
           MI_MISSILES_ACTIVATE_IMPACT_LINES |
           MI_SHOW_AUTHOR;

  return true;
}
