//
// Copyright(C) 2020 by Ryan Krafnick
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
//	DSDA Options Lump
//

#ifndef __DSDA_OPTIONS__
#define __DSDA_OPTIONS__

typedef struct dsda_options {
  int weapon_recoil;
  int monsters_remember;
  int monster_infighting;
  int monster_backing;
  int monster_avoid_hazards;
  int monkeys;
  int monster_friction;
  int help_friends;
  int player_helpers;
  int friend_distance;
  int dog_jumping;
  int comp_telefrag;
  int comp_dropoff;
  int comp_vile;
  int comp_pain;
  int comp_skull;
  int comp_blazing;
  int comp_doorlight;
  int comp_model;
  int comp_god;
  int comp_falloff;
  int comp_floors;
  int comp_skymap;
  int comp_pursuit;
  int comp_doorstuck;
  int comp_staylift;
  int comp_zombie;
  int comp_stairs;
  int comp_infcheat;
  int comp_zerotags;
  int comp_moveblock;
  int comp_respawn;
  int comp_sound;
  int comp_666;
  int comp_soul;
  int comp_maskedanim;
  int comp_ouchface;
  int comp_maxhealth;
  int comp_translucency;
  int comp_ledgeblock;
  int comp_friendlyspawn;
  int comp_voodooscroller;
  int comp_reservedlineflag;
} dsda_options_t;

void dsda_ParseOptionsLump(void);
const dsda_options_t* dsda_Options(void);
int dsda_GameOptionSize(void);
byte* dsda_WriteOptions21(byte* demo_p);
const byte *dsda_ReadOptions21(const byte *demo_p);

#endif
