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
//	DSDA Data Organizer
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "doomtype.h"
#include "lprintf.h"
#include "m_file.h"
#include "w_wad.h"
#include "i_system.h"
#include "z_zone.h"
#include "e6y.h"

#include "host.h"
#include "debug.h"

#include "dsda/args.h"
#include "dsda/utility.h"

#include "data_organizer.h"

#define DATA_DIR_LIMIT 9
static const char* dsda_data_root = "dsda_doom_data";
static char* dsda_data_dir_strings[DATA_DIR_LIMIT];
static char* dsda_base_data_dir;
static char* dsda_wad_data_dir;

// Remove trailing slashes, translate backslashes to slashes
// The string to normalize is passed and returned in str
//
// jff 4/19/98 Make killoughs slash fixer a subroutine
//
static void dsda_NormalizeSlashes(char *str)
{
  size_t l;

  // killough 1/18/98: Neater / \ handling.
  // Remove trailing / or \ to prevent // /\ \/ \\, and change \ to /

  if (!str || !(l = strlen(str)))
    return;
  if (str[--l] == '/' || str[l] == '\\')     // killough 1/18/98
    str[l] = 0;
  while (l--)
    if (str[l] == '\\')
      str[l] = '/';
}

char* dsda_DetectDirectory(const char* env_key, int arg_id) {
  dsda_arg_t* arg;
  char* result = NULL;
  const char* default_directory;

  default_directory = M_getenv(env_key);

  if (!default_directory)
    default_directory = I_DoomExeDir();

  arg = dsda_Arg(arg_id);
  if (arg->found) {
    if (M_IsDir(arg->value.v_string)) {
      if (result) Z_Free(result);
      result = Z_Strdup(arg->value.v_string);
    }
    else
      lprintf(LO_ERROR, "Error: path %s does not exist. Using %s\n",
              arg->value.v_string, default_directory);
  }

  if (!result)
    result = Z_Strdup(default_directory);

  dsda_NormalizeSlashes(result);

  return result;
}

void dsda_InitDataDir(void) {
  char* parent_directory;
  dsda_string_t str;

  parent_directory = dsda_DetectDirectory("DOOMDATADIR", dsda_arg_data);

  dsda_StringPrintF(&str, "%s/%s", parent_directory, dsda_data_root);

  dsda_base_data_dir = str.string;
  M_MakeDir(dsda_base_data_dir, true);

  Z_Free(parent_directory);
}

static void dsda_InitWadDataDir(void) {
  int i;
  const int iwad_index = 0;
  int pwad_index = 1;
  dsda_string_t str;

  for (i = 0; i < numwadfiles; ++i) {
    const char* start;
    char* result;
    int length;

    start = PathFindFileName(wadfiles[i].name);

    length = strlen(start) - 4;

    if (length > 0 && !strcasecmp(start + length, ".wad")) {
      int dir_index;

      if (wadfiles[i].src == 0)
        dir_index = iwad_index;
      else if (wadfiles[i].src == 3)
        dir_index = pwad_index;
      else
        dir_index = -1;

      if (dir_index >= 0 && dir_index < DATA_DIR_LIMIT) {
        dsda_data_dir_strings[dir_index] = Z_Malloc(length + 1);
        strncpy(dsda_data_dir_strings[dir_index], start, length);
        dsda_data_dir_strings[dir_index][length] = '\0';

        for (result = dsda_data_dir_strings[dir_index]; *result; ++result)
          *result = tolower(*result);

        if (dir_index == pwad_index)
          pwad_index++;
      }
    }
  }

  dsda_InitString(&str, dsda_base_data_dir);

  for (i = 0; i < DATA_DIR_LIMIT; ++i)
    if (dsda_data_dir_strings[i]) {
      dsda_StringCatF(&str, "/%s", dsda_data_dir_strings[i]);
      M_MakeDir(str.string, true);
    }

  dsda_wad_data_dir = str.string;

  DG_debug(DEBUG_INFO,"Using data file directory: %s", dsda_wad_data_dir);
}

char* dsda_DataDir(void) {
  if (!dsda_wad_data_dir)
    dsda_InitWadDataDir();

  return dsda_wad_data_dir;
}

const char* dsda_DataRoot(void) {
  return dsda_base_data_dir;
}
