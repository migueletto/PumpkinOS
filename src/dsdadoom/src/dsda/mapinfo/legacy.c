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
//	DSDA MapInfo Legacy
//

#include "doomstat.h"
#include "g_game.h"
#include "m_misc.h"
#include "p_setup.h"
#include "r_data.h"
#include "s_sound.h"
#include "sounds.h"
#include "w_wad.h"

#include "dsda/map_format.h"
#include "dsda/mapinfo.h"

#include "legacy.h"

int dsda_LegacyNameToMap(int* found, const char* name, int* episode, int* map) {
  char name_upper[9];
  int episode_from_name = -1;
  int map_from_name = -1;

  if (strlen(name) > 8) {
    *found = false;

    return true;
  }

  strncpy(name_upper, name, 8);
  name_upper[8] = 0;
  M_Strupr(name_upper);

  if (gamemode != commercial) {
    if (sscanf(name_upper, "E%dM%d", &episode_from_name, &map_from_name) != 2) {
      *found = false;

      return true;
    }
  }
  else {
    if (sscanf(name_upper, "MAP%d", &map_from_name) != 1) {
      *found = false;

      return true;
    }

    episode_from_name = 1;
  }

  *found = true;

  *episode = episode_from_name;
  *map = map_from_name;

  return true;
}

int dsda_LegacyFirstMap(int* episode, int* map) {
  int i, j, lump;

  *episode = 1;
  *map = 1;

  if (gamemode == commercial) {
    for (i = 1; i < 33; i++) {
      lump = W_CheckNumForName(VANILLA_MAP_LUMP_NAME(1, i));

      if (lump != LUMP_NOT_FOUND && lumpinfo[lump].source == source_pwad) {
        *map = i;

        return true;
      }
    }
  }
  else
    for (i = 1; i < 5; i++)
      for (j = 1; j < 10; j++) {
        lump = W_CheckNumForName(VANILLA_MAP_LUMP_NAME(i, j));

        if (lump != LUMP_NOT_FOUND && lumpinfo[lump].source == source_pwad) {
          *episode = i;
          *map = j;

          return true;
        }
      }

  return true;
}

int dsda_LegacyNewGameMap(int* episode, int* map) {
  return true;
}

int dsda_LegacyResolveWarp(int* args, int arg_count, int* episode, int* map) {
  if (gamemode == commercial)
  {
    if (arg_count) {
      *episode = 1;
      *map = args[0];
    }
  }
  else if (arg_count) {
    *episode = args[0];

    if (arg_count > 1)
      *map = args[1];
    else
      *map = 1;
  }

  return true;
}

int dsda_LegacyNextMap(int* episode, int* map) {
  static byte doom2_next[33] = {
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 31, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 1,
    32, 16, 3
  };
  static byte doom_next[4][9] = {
    { 12, 13, 19, 15, 16, 17, 18, 21, 14 },
    { 22, 23, 24, 25, 29, 27, 28, 31, 26 },
    { 32, 33, 34, 35, 36, 39, 38, 41, 37 },
    { 42, 49, 44, 45, 46, 47, 48, 11, 43 }
  };
  static byte heretic_next[6][9] = {
    { 12, 13, 14, 15, 16, 19, 18, 21, 17 },
    { 22, 23, 24, 29, 26, 27, 28, 31, 25 },
    { 32, 33, 34, 39, 36, 37, 38, 41, 35 },
    { 42, 43, 44, 49, 46, 47, 48, 51, 45 },
    { 52, 53, 59, 55, 56, 57, 58, 61, 54 },
    { 62, 63, 11, 11, 11, 11, 11, 11, 11 }, // E6M4-E6M9 shouldn't be accessible
  };

  // next arrays are 0-based, unlike gameepisode and gamemap
  *episode = gameepisode - 1;
  *map = gamemap - 1;

  if (heretic) {
    int next;

    if (gamemode == shareware)
      heretic_next[0][7] = 11;

    if (gamemode == registered)
      heretic_next[2][7] = 11;

    next = heretic_next[BETWEEN(0, 5, *episode)][BETWEEN(0, 8, *map)];
    *episode = next / 10;
    *map = next % 10;
  }
  else if (gamemode == commercial) {
    // secret level
    doom2_next[14] = (haswolflevels ? 31 : 16);

    if (bfgedition && allow_incompatibility) {
      if (gamemission == pack_nerve) {
        doom2_next[3] = 9;
        doom2_next[7] = 1;
        doom2_next[8] = 5;
      }
      else
        doom2_next[1] = 33;
    }

    *episode = 1;
    *map = doom2_next[BETWEEN(0, 32, *map)];
  }
  else {
    int next;

    // shareware doom has only episode 1
    doom_next[0][7] = (gamemode == shareware ? 11 : 21);

    doom_next[2][7] = // the fourth episode for pre-ultimate complevels is not allowed
      ((gamemode == registered) || (compatibility_level < ultdoom_compatibility) ? 11 : 41);

    next = doom_next[BETWEEN(0, 3, *episode)][BETWEEN(0, 9, *map)];
    *episode = next / 10;
    *map = next % 10;
  }

  return true;
}

int dsda_LegacyShowNextLocBehaviour(int* behaviour) {
  if (
    gamemode != commercial &&
    (gamemap == 8 || (gamemission == chex && gamemap == 5))
  )
    *behaviour = WI_SHOW_NEXT_DONE;
  else
    *behaviour = WI_SHOW_NEXT_LOC;

  if (dsda_FinaleShortcut())
    *behaviour = WI_SHOW_NEXT_DONE;

  return true;
}

int dsda_LegacySkipDrawShowNextLoc(int* skip) {
  *skip = false;

  if (dsda_FinaleShortcut())
    *skip = true;

  return true;
}

void dsda_LegacyUpdateMapInfo(void) {
  // nothing to do right now
}

void dsda_LegacyUpdateLastMapInfo(void) {
  // nothing to do right now
}

void dsda_LegacyUpdateNextMapInfo(void) {
  // nothing to do right now
}

static int dsda_CannotCLEV(int episode, int map) {
  char* next;

  if (
    episode < 1 ||
    map < 0 ||
    ((gamemode == retail || gamemode == registered) && (episode > 9 || map > 9)) ||
    (gamemode == shareware && (episode > 1 || map > 9)) ||
    (gamemode == commercial && (episode > 1 || map > 99)) ||
    (gamemission == pack_nerve && map > 9)
  ) return true;

  // Catch invalid maps
  next = VANILLA_MAP_LUMP_NAME(episode, map);
  if (!W_LumpNameExists(next)) {
    doom_printf("IDCLEV target not found: %s", next);
    return true;
  }

  return false;
}

int dsda_LegacyResolveCLEV(int* clev, int* episode, int* map) {
  if (dsda_CannotCLEV(*episode, *map))
    *clev = false;
  else {
    if (gamemission == chex)
      *episode = 1;

    *clev = true;
  }

  return true;
}

int dsda_LegacyResolveINIT(int* init) {
  *init = false;

  return true;
}

int dsda_LegacyMusicIndexToLumpNum(int* lump, int music_index) {
  char name[9];
  const char* format;

  format = raven ? "%s" : "d_%s";

  sprintf(name, format, S_music[music_index].name);

  *lump = W_GetNumForName(name);

  return true;
}

static inline int WRAP(int i, int w)
{
  while (i < 0)
    i += w;

  return i % w;
}

int dsda_LegacyMapMusic(int* music_index, int* music_lump) {
  *music_lump = -1;

  if (idmusnum != -1)
    *music_index = idmusnum; //jff 3/17/98 reload IDMUS music if not -1
  else {
    if (gamemode == commercial)
      *music_index = mus_runnin + WRAP(gamemap - 1, DOOM_MUSINFO - mus_runnin);
    else {
      static const int spmus[] = {
        mus_e3m4,
        mus_e3m2,
        mus_e3m3,
        mus_e1m5,
        mus_e2m7,
        mus_e2m4,
        mus_e2m6,
        mus_e2m5,
        mus_e1m9
      };

      if (heretic)
        *music_index = heretic_mus_e1m1 +
                       WRAP((gameepisode - 1) * 9 + gamemap - 1,
                            HERETIC_NUMMUSIC - heretic_mus_e1m1);
      else if (gameepisode < 4)
        *music_index = mus_e1m1 +
                       WRAP((gameepisode - 1) * 9 + gamemap - 1, mus_runnin - mus_e1m1);
      else
        *music_index = spmus[WRAP(gamemap - 1, 9)];
    }
  }

  return true;
}

int dsda_LegacyIntermissionMusic(int* music_index, int* music_lump) {
  *music_lump = -1;

  if (gamemode == commercial)
    *music_index = mus_dm2int;
  else
    *music_index = mus_inter;

  return true;
}

int dsda_LegacyInterMusic(int* music_index, int* music_lump) {
  *music_lump = -1;

  switch (gamemode) {
    case shareware:
    case registered:
    case retail:
      *music_index = mus_victor;
      break;
    default:
      *music_index = mus_read_m;
      break;
  }

  return true;
}

int dsda_LegacyStartFinale(void) {
  return true;
}

int dsda_LegacyFTicker(void) {
  return true;
}

void dsda_LegacyFDrawer(void) {
  return;
}

int dsda_LegacyBossAction(mobj_t* mo) {
  return false;
}

int dsda_LegacyMapLumpName(const char** name, int episode, int map) {
  *name = VANILLA_MAP_LUMP_NAME(episode, map);

  return true;
}

int dsda_LegacyMapAuthor(const char** author) {
  *author = NULL;

  return true;
}

int dsda_LegacyHUTitle(dsda_string_t* str) {
  extern char** mapnames[];
  extern char** mapnames2[];
  extern char** mapnamesp[];
  extern char** mapnamest[];
  extern const char* LevelNames[];

  dsda_InitString(str, NULL);

  if (gamestate == GS_LEVEL && gamemap > 0 && gameepisode > 0) {
    if (heretic) {
      if (gameepisode < 6 && gamemap < 10)
        dsda_StringCat(str, LevelNames[(gameepisode - 1) * 9 + gamemap - 1]);
    }
    else {
      switch (gamemode) {
        case shareware:
        case registered:
        case retail:
          // Chex.exe always uses the episode 1 level title
          // eg. E2M1 gives the title for E1M1
          if (gamemission == chex && gamemap < 10)
            dsda_StringCat(str, *mapnames[gamemap - 1]);
          else if (gameepisode < 6 && gamemap < 10)
            dsda_StringCat(str, *mapnames[(gameepisode - 1) * 9 + gamemap - 1]);
          break;

        default:  // Ty 08/27/98 - modified to check mission for TNT/Plutonia
          if (gamemission == pack_tnt && gamemap < 33)
            dsda_StringCat(str, *mapnamest[gamemap - 1]);
          else if (gamemission == pack_plut && gamemap < 33)
            dsda_StringCat(str, *mapnamesp[gamemap - 1]);
          else if (gamemap < 34)
            dsda_StringCat(str, *mapnames2[gamemap - 1]);
          break;
      }
    }
  }

  if (!str->string)
    dsda_StringCat(str, VANILLA_MAP_LUMP_NAME(gameepisode, gamemap));

  return true;
}

int dsda_LegacySkyTexture(int* sky) {
  if (map_format.doublesky)
    *sky = Sky1Texture;
  else if (heretic) {
    static const char *sky_lump_names[5] = {
        "SKY1", "SKY2", "SKY3", "SKY1", "SKY3"
    };

    if (gameepisode < 6)
      *sky = R_TextureNumForName(sky_lump_names[gameepisode - 1]);
    else
      *sky = R_TextureNumForName("SKY1");
  }
  else if (gamemode == commercial) {
    *sky = R_TextureNumForName ("SKY3");
    if (gamemap < 12)
      *sky = R_TextureNumForName ("SKY1");
    else
      if (gamemap < 21)
        *sky = R_TextureNumForName ("SKY2");
  }
  else {
    switch (gameepisode) {
      case 1:
        *sky = R_TextureNumForName ("SKY1");
        break;
      case 2:
        *sky = R_TextureNumForName ("SKY2");
        break;
      case 3:
        *sky = R_TextureNumForName ("SKY3");
        break;
      case 4: // Special Edition sky
        *sky = R_TextureNumForName ("SKY4");
        break;
    }
  }

  return true;
}

int dsda_LegacyPrepareInitNew(void) {
  return true;
}

void dsda_LegacyParTime(int* partime, dboolean* modified) {
  extern int deh_pars;

  if (gamemode == commercial) {
    if (gamemap >= 1 && gamemap <= 34) {
      *partime = TICRATE * cpars[gamemap - 1];
      *modified = deh_pars;
    }
  }
  else {
    if (gameepisode >= 1 && gameepisode <= 4 && gamemap >= 1 && gamemap <= 9) {
      *partime = TICRATE * pars[gameepisode][gamemap];
      *modified = deh_pars;
    }
  }
}

int dsda_LegacyPrepareIntermission(int* result) {
  if (gamemode != commercial)
    if (gamemap == 9) {
      int i;

      for (i = 0; i < g_maxplayers; i++)
        players[i].didsecret = true;
    }

  wminfo.didsecret = players[consoleplayer].didsecret;

  // wminfo.next is 0 biased, unlike gamemap
  if (gamemode == commercial) {
    if (secretexit)
      switch(gamemap) {
        case 15:
          wminfo.next = 30;
          break;
        case 31:
          wminfo.next = 31;
          break;
        case 2:
          if (bfgedition && allow_incompatibility)
            wminfo.next = 32;
          break;
        case 4:
          if (gamemission == pack_nerve && allow_incompatibility)
            wminfo.next = 8;
          break;
      }
    else
      switch(gamemap) {
        case 31:
        case 32:
          wminfo.next = 15;
          break;
        case 33:
          if (bfgedition && allow_incompatibility)
          {
            wminfo.next = 2;
            break;
          }
          // fallthrough
        default:
          wminfo.next = gamemap;
      }

    if (gamemission == pack_nerve && allow_incompatibility && gamemap == 9)
      wminfo.next = 4;
  }
  else {
    if (secretexit)
      wminfo.next = 8; // go to secret level
    else if (gamemap == 9) {
      // returning from secret level
      if (heretic)
      {
        static int after_secret[5] = { 6, 4, 4, 4, 3 };
        wminfo.next = after_secret[gameepisode - 1];
      }
      else
        switch (gameepisode) {
          case 1:
            wminfo.next = 3;
            break;
          case 2:
            wminfo.next = 5;
            break;
          case 3:
            wminfo.next = 6;
            break;
          case 4:
            wminfo.next = 2;
            break;
        }
    }
    else
      wminfo.next = gamemap; // go to next level
  }

  dsda_LegacyParTime(&wminfo.partime, &wminfo.modified_partime);

  if (map_format.zdoom)
    if (leave_data.map > 0)
      wminfo.next = leave_data.map - 1;

  *result = 0;

  return true;
}

int dsda_LegacyPrepareFinale(int* result) {
  *result = 0;

  if (gamemode == commercial && gamemission != pack_nerve) {
    switch (gamemap) {
      case 15:
      case 31:
        if (!secretexit)
          break;
        // fallthrough
      case 6:
      case 11:
      case 20:
      case 30:
        *result = WD_START_FINALE;
        break;
    }
  }
  else if (gamemission == pack_nerve && allow_incompatibility && gamemap == 8)
    *result = WD_START_FINALE;
  else if (gamemap == 8)
    *result = WD_VICTORY;
  else if (gamemap == 5 && gamemission == chex)
    *result = WD_VICTORY;

  if (dsda_FinaleShortcut())
    *result = WD_START_FINALE;

  return true;
}

void dsda_LegacyLoadMapInfo(void) {
  return;
}

int dsda_LegacyExitPic(const char** exit_pic) {
  *exit_pic = NULL;

  return true;
}

int dsda_LegacyEnterPic(const char** enter_pic) {
  *enter_pic = NULL;

  return true;
}

int dsda_LegacyBorderTexture(const char** border_texture) {
  *border_texture = heretic ? "FLOOR30" :
                    gamemode == commercial ? "GRNROCK" : "FLOOR7_2";

  return true;
}

int dsda_LegacyPrepareEntering(void) {
  extern const char *el_levelname;
  extern const char *el_levelpic;
  extern const char *el_author;

  el_levelname = NULL;
  el_levelpic = NULL;
  el_author = NULL;

  return true;
}

int dsda_LegacyPrepareFinished(void) {
  extern const char *lf_levelname;
  extern const char *lf_levelpic;
  extern const char *lf_author;

  lf_levelname = NULL;
  lf_levelpic = NULL;
  lf_author = NULL;

  return true;
}

int dsda_LegacyMapLightning(int* lightning) {
  *lightning = false;

  return true;
}

int dsda_LegacyApplyFadeTable(void) {
  return true;
}

int dsda_LegacyMapCluster(int* cluster, int map) {
  *cluster = -1;

  return true;
}

int dsda_LegacySky1Texture(short* texture) {
  *texture = -1;

  return true;
}

int dsda_LegacySky2Texture(short* texture) {
  *texture = -1;

  return true;
}

int dsda_LegacyGravity(fixed_t* gravity) {
  *gravity = FRACUNIT;

  return true;
}

int dsda_LegacyAirControl(fixed_t* air_control) {
  dboolean dsda_AllowJumping(void);

  *air_control = dsda_AllowJumping() ? (FRACUNIT >> 8) : 0;

  return true;
}

int dsda_LegacyInitSky(void) {
  return true;
}

int dsda_LegacyMapFlags(map_info_flags_t* flags) {
  *flags = MI_INTERMISSION |
           MI_ACTIVATE_OWN_DEATH_SPECIALS |
           MI_LAX_MONSTER_ACTIVATION |
           MI_MISSILES_ACTIVATE_IMPACT_LINES |
           MI_SHOW_AUTHOR;

  return true;
}
