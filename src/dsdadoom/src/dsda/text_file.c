//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Text File
//

#include "doomstat.h"
#include "lprintf.h"
#include "m_file.h"
#include "e6y.h"
#include "stricmp.h"

#include "dsda.h"
#include "dsda/analysis.h"
#include "dsda/args.h"
#include "dsda/configuration.h"
#include "dsda/mapinfo.h"
#include "dsda/playback.h"

#include "text_file.h"

extern int dsda_last_leveltime, dsda_last_gamemap, dsda_startmap;

static char* dsda_TextFileName(void) {
  int name_length;
  char* name;
  char* playdemo;
  const char* playback_name;

  playback_name = dsda_PlaybackName();

  if (!playback_name)
    return NULL;

  playdemo = Z_Strdup(playback_name);
  name_length = strlen(playdemo);

  if (name_length > 4 && !stricmp(playdemo + name_length - 4, ".lmp")) {
    name = Z_Strdup(playdemo);
    name[name_length - 4] = '\0';
  }
  else {
    name = Z_Calloc(name_length + 4, 1);
    strcat(name, playdemo);
  }

  strcat(name, ".txt");

  Z_Free(playdemo);

  return name;
}

static int dsda_IL(void) {
  extern int dsda_startmap;

  return dsda_startmap == dsda_last_gamemap;
}

static const char* dsda_Movie(void) {
  if (gamemode == commercial) {
    if (dsda_startmap == 1 && dsda_last_gamemap == 10)
      return "Episode 1";
    if (dsda_startmap == 11 && dsda_last_gamemap == 20)
      return "Episode 2";
    if (dsda_startmap == 21 && dsda_last_gamemap == 30)
      return "Episode 3";
    if (dsda_startmap == 1 && dsda_last_gamemap == 30)
      return "D2All";
  }
  else {
    if (dsda_startmap == 1 && dsda_last_gamemap == 8) {
      if (gameepisode == 1)
        return "Episode 1";
      if (gameepisode == 2)
        return "Episode 2";
      if (gameepisode == 3)
        return "Episode 3";
      if (gameepisode == 4)
        return "Episode 4";
    }
  }

  return NULL;
}

static char* dsda_TextFileTime(void) {
  char* text_file_time;

  text_file_time = Z_Malloc(16);

  if (dsda_IL())
    snprintf(
      text_file_time,
      16,
      "%d:%05.2f",
      dsda_last_leveltime / 35 / 60,
      (float)(dsda_last_leveltime % (60 * 35)) / 35
    );
  else
    snprintf(
      text_file_time,
      16,
      "%d:%02d",
      totalleveltimes / 35 / 60,
      (totalleveltimes / 35) % 60
    );

  return text_file_time;
}

void dsda_ExportTextFile(void) {
  dsda_arg_t* arg;
  char* name;
  const char* iwad = NULL;
  const char* pwad = NULL;
  const char* dsda_player_name;
  dg_file_t* file;

  if (!dsda_Flag(dsda_arg_export_text_file))
    return;

  name = dsda_TextFileName();

  if (!name)
    return;

  file = M_OpenFile(name, "wb");
  Z_Free(name);

  if (!file)
    I_Error("Unable to export text file!");

  arg = dsda_Arg(dsda_arg_iwad);
  if (arg->found)
    iwad = PathFindFileName(arg->value.v_string);

  arg = dsda_Arg(dsda_arg_file);
  if (arg->found)
    pwad = PathFindFileName(arg->value.v_string_array[0]);

  DG_printf(file, "Doom Speed Demo Archive\n");
  DG_printf(file, "https://dsdarchive.com/\n");
  DG_printf(file, "\n");
  if (iwad)
    DG_printf(file, "Iwad:      %s\n", iwad);
  if (pwad)
    DG_printf(file, "Pwad:      %s\n", pwad);

  if (dsda_IL())
    DG_printf(file, "Map:       %s\n", dsda_MapLumpName(gameepisode, dsda_startmap));
  else {
    const char* movie;

    movie = dsda_Movie();

    if (movie)
      DG_printf(file, "Movie:     %s\n", movie);
    else {
      DG_printf(file, "Movie:     %s", dsda_MapLumpName(gameepisode, dsda_startmap));
      DG_printf(file, " - %s\n",       dsda_MapLumpName(gameepisode, dsda_last_gamemap));
    }
  }

  DG_printf(file, "Skill:     %i\n", gameskill + 1);
  DG_printf(file, "Category:  %s\n", dsda_DetectCategory());
  DG_printf(file, "Exe:       %s -complevel %i\n", PACKAGE_STRING, compatibility_level);
  DG_printf(file, "\n");

  name = dsda_TextFileTime();
  DG_printf(file, "Time:      %s\n", name);
  Z_Free(name);

  dsda_player_name = dsda_StringConfig(dsda_config_player_name);

  DG_printf(file, "\n");
  DG_printf(file, "Author:    %s\n", dsda_player_name);
  DG_printf(file, "\n");
  DG_printf(file, "Comments:\n");

  DG_close(file);
}
