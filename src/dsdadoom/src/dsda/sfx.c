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
//	DSDA SFX
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "doomtype.h"
#include "stricmp.h"

#include "dsda/configuration.h"

#include "sfx.h"

sfxinfo_t* S_sfx;
int num_sfx;
static int deh_soundnames_size;
static char** deh_soundnames;
static byte* sfx_state;

static void dsda_ResetSFX(int from, int to) {
  int i;

  for (i = from; i < to; ++i) {
    S_sfx[i].priority = 127;
    S_sfx[i].pitch = -1;
    S_sfx[i].volume = -1;
  }
}

static void dsda_PrepAllocation(void) {
  static int first_allocation = true;

  if (first_allocation) {
    sfxinfo_t* source = S_sfx;

    first_allocation = false;
    S_sfx = malloc(num_sfx * sizeof(*S_sfx));
    memcpy(S_sfx, source, num_sfx * sizeof(*S_sfx));
  }
}

static void dsda_EnsureCapacity(int limit) {
  while (limit >= num_sfx) {
    int old_num_sfx = num_sfx;

    dsda_PrepAllocation();

    num_sfx *= 2;

    S_sfx = realloc(S_sfx, num_sfx * sizeof(*S_sfx));
    memset(S_sfx + old_num_sfx, 0, (num_sfx - old_num_sfx) * sizeof(*S_sfx));

    sfx_state = realloc(sfx_state, num_sfx * sizeof(*sfx_state));
    memset(sfx_state + old_num_sfx, 0,
      (num_sfx - old_num_sfx) * sizeof(*sfx_state));

    dsda_ResetSFX(old_num_sfx, num_sfx);
  }
}

int dsda_GetDehSFXIndex(const char* key, size_t length) {
  int i;

  for (i = 1; i < num_sfx; ++i)
    if (
      S_sfx[i].name &&
      strlen(S_sfx[i].name) == length &&
      !strnicmp(S_sfx[i].name, key, length) &&
      !sfx_state[i]
    ) {
      sfx_state[i] = true; // sfx has been edited
      return i;
    }

  return -1;
}

int dsda_GetOriginalSFXIndex(const char* key) {
  int i;
  const char* c;

  for (i = 1; deh_soundnames[i]; ++i)
    if (!strncasecmp(deh_soundnames[i], key, 6))
      return i;

  // is it a number?
  for (c = key; *c; c++)
    if (!isdigit(*c))
      return -1;

  i = atoi(key);
  dsda_EnsureCapacity(i);

  return i;
}

sfxinfo_t* dsda_GetDehSFX(int index) {
  dsda_EnsureCapacity(index);

  return &S_sfx[index];
}

void dsda_InitializeSFX(sfxinfo_t* source, int count) {
  int i;
  extern int raven;

  num_sfx = count;
  deh_soundnames_size = num_sfx + 1;

  S_sfx = source;

  if (raven) return;

  deh_soundnames = malloc(deh_soundnames_size * sizeof(*deh_soundnames));
  for (i = 1; i < num_sfx; i++)
    if (S_sfx[i].name != NULL)
      deh_soundnames[i] = strdup(S_sfx[i].name);
    else
      deh_soundnames[i] = NULL;
  deh_soundnames[0] = NULL;
  deh_soundnames[num_sfx] = NULL;

  sfx_state = calloc(num_sfx, sizeof(*sfx_state));
}

void dsda_FreeDehSFX(void) {
  int i;

  if (deh_soundnames)
    for (i = 0; i < deh_soundnames_size; i++)
      if (deh_soundnames[i])
        free(deh_soundnames[i]);

  free(deh_soundnames);
  free(sfx_state);
}

static int dsda_parallel_sfx_limit;
static int dsda_parallel_sfx_window;

void dsda_InitParallelSFXFilter(void) {
  dsda_parallel_sfx_limit = dsda_IntConfig(dsda_config_parallel_sfx_limit);
  dsda_parallel_sfx_window = dsda_IntConfig(dsda_config_parallel_sfx_window);
}

dboolean dsda_BlockSFX(sfxinfo_t *sfx) {
  extern int gametic;

  if (!dsda_parallel_sfx_limit) return false;

  if (gametic - sfx->parallel_tic >= dsda_parallel_sfx_window) {
    sfx->parallel_tic = gametic;
    sfx->parallel_count = 0;
  }

  ++sfx->parallel_count;

  return sfx->parallel_count > dsda_parallel_sfx_limit;
}
