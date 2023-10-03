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
//	DSDA Sprite
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "info.h"

#include "sprite.h"
#include "stricmp.h"

const char** sprnames;
int num_sprites;
static int deh_spritenames_size;
static char** deh_spritenames;
static byte* sprnames_state;

static void dsda_PrepAllocation(void) {
  static int first_allocation = true;

  if (first_allocation) {
    const char** source = sprnames;

    first_allocation = false;
    sprnames = malloc(num_sprites * sizeof(*sprnames));
    memcpy(sprnames, source, num_sprites * sizeof(*sprnames));
  }
}

static void dsda_EnsureCapacity(int limit) {
  while (limit >= num_sprites) {
    int old_num_sprites = num_sprites;

    dsda_PrepAllocation();

    num_sprites *= 2;

    sprnames = realloc(sprnames, num_sprites * sizeof(*sprnames));
    memset(sprnames + old_num_sprites, 0, (num_sprites - old_num_sprites) * sizeof(*sprnames));

    sprnames_state = realloc(sprnames_state, num_sprites * sizeof(*sprnames_state));
    memset(sprnames_state + old_num_sprites, 0,
      (num_sprites - old_num_sprites) * sizeof(*sprnames_state));
  }
}

int dsda_GetDehSpriteIndex(const char* key) {
  int i;

  for (i = 0; i < num_sprites; ++i)
    if (sprnames[i] && !strnicmp(sprnames[i], key, 4) && !sprnames_state[i]) {
      sprnames_state[i] = true; // sprite has been edited
      return i;
    }

  return -1;
}

int dsda_GetOriginalSpriteIndex(const char* key) {
  int i;
  const char* c;

  for (i = 0; deh_spritenames[i]; ++i)
    if (!strncasecmp(deh_spritenames[i], key, 4))
      return i;

  // is it a number?
  for (c = key; *c; c++)
    if (!isdigit(*c))
      return -1;

  i = atoi(key);
  dsda_EnsureCapacity(i);

  return i;
}

void dsda_InitializeSprites(const char** source, int count) {
  int i;
  extern int raven;

  num_sprites = count;
  deh_spritenames_size = num_sprites + 1;

  sprnames = source;

  if (raven) return;

  deh_spritenames = malloc(deh_spritenames_size * sizeof(*deh_spritenames));
  for (i = 0; i < num_sprites; i++)
    deh_spritenames[i] = strdup(sprnames[i]);
  deh_spritenames[num_sprites] = NULL;

  sprnames_state = calloc(num_sprites, sizeof(*sprnames_state));
}

void dsda_FreeDehSprites(void) {
  int i;

  if (deh_spritenames)
    for (i = 0; i < deh_spritenames_size; i++)
      if (deh_spritenames[i])
        free(deh_spritenames[i]);

  free(deh_spritenames);
  free(sprnames_state);
}
