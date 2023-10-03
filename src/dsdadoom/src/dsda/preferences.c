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
//	DSDA Preferences
//

#include "lprintf.h"
#include "w_wad.h"
#include "v_video.h"

#include "dsda/messenger.h"
#include "dsda/utility.h"

#include "preferences.h"

typedef struct {
  dboolean opengl;
  dboolean software;
  dboolean mapinfo;
} preferences_t;

static preferences_t map_preferences;
static preferences_t wad_preferences;

void dsda_LoadWadPreferences(void) {
  char* lump;
  char** lines;
  const char* line;
  int line_i;
  int value;
  char key[33] = { 0 };

  lump = W_ReadLumpToString(W_CheckNumForName("DSDAPREF"));

  if (!lump)
    return;

  lines = dsda_SplitString(lump, "\n\r");

  for (line_i = 0; lines[line_i]; ++line_i) {
    line = lines[line_i];

    if (!line[0] || line[0] == '/')
      continue;

    value = 1;
    if (!sscanf(line, "%32s %d", key, &value))
      I_Error("DSDAPREF lump has unknown format! (%s)", line);

    if (!strcasecmp(key, "prefer_opengl"))
      wad_preferences.opengl = !!value;
    else if (!strcasecmp(key, "prefer_software"))
      wad_preferences.software = !!value;
    else if (!strcasecmp(key, "use_mapinfo"))
      wad_preferences.mapinfo = !!value;
    else
      lprintf(LO_WARN, "Unknown DSDAPREF key: %s\n", key);
  }

  Z_Free(lines);
  Z_Free(lump);
}

static void dsda_HandleWadPreferences(void) {
  DO_ONCE
    if (wad_preferences.opengl && V_IsSoftwareMode())
      dsda_AddAlert("This wad may have rendering errors\nin software mode!");

    if (wad_preferences.software && V_IsOpenGLMode())
      dsda_AddAlert("This wad may have rendering errors\nin opengl mode!");
  END_ONCE
}

void dsda_HandleMapPreferences(void) {
  dsda_HandleWadPreferences();

  if (map_preferences.opengl && V_IsSoftwareMode())
    dsda_AddAlert("This level may have rendering errors\nin software mode!");

  if (map_preferences.software && V_IsOpenGLMode())
    dsda_AddAlert("This level may have rendering errors\nin opengl mode!");

  memset(&map_preferences, 0, sizeof(map_preferences));
}

void dsda_PreferOpenGL(void) {
  map_preferences.opengl = true;
}

void dsda_PreferSoftware(void) {
  map_preferences.software = true;
}

dboolean dsda_UseMapinfo(void) {
  return wad_preferences.mapinfo;
}
