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
//	DSDA Keys HUD Component
//

#include "base.h"

#include "keys.h"

#define PATCH_DELTA 10

typedef struct {
  dsda_patch_component_t component;
  dboolean horizontal;
} local_component_t;

static local_component_t* local;

static int key_patch_num[NUMCARDS];

static const char* dsda_Key1Name(player_t* player) {
  if (heretic) {
    if (player->cards[key_yellow])
      return "ykeyicon";
  }
  else {
    if (player->cards[0] && player->cards[3])
      return "STKEYS6";
    else if (player->cards[0])
      return "STKEYS0";
    else if (player->cards[3])
      return "STKEYS3";
  }

  return NULL;
}

static const char* dsda_Key2Name(player_t* player) {
  if (heretic) {
    if (player->cards[key_green])
      return "gkeyicon";
  }
  else {
    if (player->cards[1] && player->cards[4])
      return "STKEYS7";
    else if (player->cards[1])
      return "STKEYS1";
    else if (player->cards[4])
      return "STKEYS4";
  }

  return NULL;
}

static const char* dsda_Key3Name(player_t* player) {
  if (heretic) {
    if (player->cards[key_blue])
      return "bkeyicon";
  }
  else {
    if (player->cards[2] && player->cards[5])
      return "STKEYS8";
    else if (player->cards[2])
      return "STKEYS2";
    else if (player->cards[5])
      return "STKEYS5";
  }

  return NULL;
}

void drawKey(player_t* player, int* x, int* y, const char* (*key)(player_t*)) {
  const char* name;

  name = key(player);

  if (name)
    V_DrawNamePatch(*x, *y, FG, name, CR_DEFAULT, local->component.vpt);

  if (local->horizontal)
    *x += PATCH_DELTA;
  else
    *y += PATCH_DELTA;
}

static void dsda_DrawComponent(void) {
  player_t* player;
  int x, y;

  player = &players[displayplayer];

  x = local->component.x;
  y = local->component.y;

  if (hexen) {
    int i;

    for (i = 0; i < NUMCARDS; ++i)
      if (player->cards[i]) {
        V_DrawNumPatch(x, y, 0, key_patch_num[i], CR_DEFAULT, local->component.vpt);
        x += R_NumPatchWidth(key_patch_num[i]) + 4;
      }

    return;
  }

  drawKey(player, &x, &y, dsda_Key1Name);
  drawKey(player, &x, &y, dsda_Key2Name);
  drawKey(player, &x, &y, dsda_Key3Name);
}

void dsda_InitKeysHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->horizontal = arg_count > 0 ? !!args[0] : false;

  if (hexen) {
    int i;

    for (i = 0; i < NUMCARDS; ++i)
      key_patch_num[i] = R_NumPatchForSpriteIndex(HEXEN_SPR_KEY1 + i);
  }

  dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateKeysHC(void* data) {
  local = data;
}

void dsda_DrawKeysHC(void* data) {
  local = data;

  dsda_DrawComponent();
}
