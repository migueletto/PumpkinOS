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
//	DSDA Music
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "doomtype.h"
#include "p_saveg.h"
#include "s_advsound.h"
#include "s_sound.h"
#include "stricmp.h"

#include "music.h"

musicinfo_t* S_music;
int num_music;
int mus_musinfo;
static int deh_musicnames_size;
static char** deh_musicnames;
static byte* music_state;

#if 0
static void dsda_EnsureCapacity(int limit) {
  while (limit >= num_music) {
    int old_num_music = num_music;

    num_music *= 2;

    S_music = realloc(S_music, num_music * sizeof(*S_music));
    memset(S_music + old_num_music, 0, (num_music - old_num_music) * sizeof(*S_music));

    music_state = realloc(music_state, num_music * sizeof(*music_state));
    memset(music_state + old_num_music, 0,
      (num_music - old_num_music) * sizeof(*music_state));
  }
}
#endif

int dsda_GetDehMusicIndex(const char* key, size_t length) {
  int i;

  for (i = 1; i < num_music; ++i)
    if (
      S_music[i].name &&
      strlen(S_music[i].name) == length &&
      !strnicmp(S_music[i].name, key, length) &&
      !music_state[i]
    ) {
      music_state[i] = true; // music has been edited
      return i;
    }

  return -1;
}

int dsda_GetOriginalMusicIndex(const char* key) {
  int i;
  // const char* c;

  for (i = 1; deh_musicnames[i]; ++i)
    if (!strncasecmp(deh_musicnames[i], key, 6))
      return i;

  return -1;

  // is it a number?
  // for (c = key; *c; c++)
  //   if (!isdigit(*c))
  //     return -1;
  //
  // i = atoi(key);
  // dsda_EnsureCapacity(i);
  //
  // return i;
}

void dsda_InitializeMusic(musicinfo_t* source, int count) {
  int i;
  extern int raven;

  num_music = count;
  deh_musicnames_size = num_music + 1;
  mus_musinfo = num_music - 1;

  S_music = source;

  // S_music = malloc(num_music * sizeof(*S_music));
  // memcpy(S_music, source, num_music * sizeof(*S_music));

  if (raven) return;

  deh_musicnames = malloc(deh_musicnames_size * sizeof(*deh_musicnames));
  for (i = 1; i < num_music; i++)
    if (S_music[i].name != NULL)
      deh_musicnames[i] = strdup(S_music[i].name);
    else
      deh_musicnames[i] = NULL;
  deh_musicnames[0] = NULL;
  deh_musicnames[num_music] = NULL;

  music_state = calloc(num_music, sizeof(*music_state));
}

void dsda_FreeDehMusic(void) {
  int i;

  if (deh_musicnames)
    for (i = 0; i < deh_musicnames_size; i++)
      if (deh_musicnames[i])
        free(deh_musicnames[i]);

  free(deh_musicnames);
  free(music_state);
}

static int music_queue = -1;

void dsda_ArchiveMusic(void) {
  P_SAVE_X(musinfo.current_item);
}

void dsda_UnArchiveMusic(void) {
  P_LOAD_X(music_queue);
}

dboolean dsda_StartQueuedMusic(void) {
  if (music_queue == -1)
    return false;

  S_ChangeMusInfoMusic(music_queue, true);
  music_queue = -1;

  return true;
}
