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
//	DSDA Brute Force
//

#include "doomtype.h"

typedef enum {
  dsda_bf_x,
  dsda_bf_y,
  dsda_bf_z,
  dsda_bf_momx,
  dsda_bf_momy,
  dsda_bf_speed,
  dsda_bf_damage,
  dsda_bf_rng,
  dsda_bf_arm,
  dsda_bf_hp,
  dsda_bf_ammo_0,
  dsda_bf_ammo_1,
  dsda_bf_ammo_2,
  dsda_bf_ammo_3,
  dsda_bf_ammo_4,
  dsda_bf_ammo_5,
  dsda_bf_bmapwidth,
  dsda_bf_attribute_max,

  dsda_bf_line_skip = 0,
  dsda_bf_line_activation,
  dsda_bf_have_item,
  dsda_bf_misc_max,
} dsda_bf_attribute_t;

typedef enum {
  dsda_bf_less_than,
  dsda_bf_less_than_or_equal_to,
  dsda_bf_greater_than,
  dsda_bf_greater_than_or_equal_to,
  dsda_bf_equal_to,
  dsda_bf_not_equal_to,
  dsda_bf_operator_max,

  dsda_bf_operator_misc,
} dsda_bf_operator_t;

typedef enum {
  dsda_bf_red_key_card,
  dsda_bf_yellow_key_card,
  dsda_bf_blue_key_card,
  dsda_bf_red_skull_key,
  dsda_bf_yellow_skull_key,
  dsda_bf_blue_skull_key,
  dsda_bf_fist,
  dsda_bf_pistol,
  dsda_bf_shotgun,
  dsda_bf_chaingun,
  dsda_bf_rocket_launcher,
  dsda_bf_plasma_gun,
  dsda_bf_bfg,
  dsda_bf_chainsaw,
  dsda_bf_super_shotgun,
  dsda_bf_item_max,
} dsda_bf_item_t;

typedef enum {
  dsda_bf_limit_trio_zero,
  dsda_bf_acap = dsda_bf_limit_trio_zero,
  dsda_bf_limit_trio_max,

  dsda_bf_limit_duo_zero = dsda_bf_limit_trio_max,
  dsda_bf_max = dsda_bf_limit_duo_zero,
  dsda_bf_min,
  dsda_bf_limit_duo_max,

  dsda_bf_limit_max = dsda_bf_limit_duo_max
} dsda_bf_limit_t;

extern const char* dsda_bf_attribute_names[dsda_bf_attribute_max];
extern const char* dsda_bf_operator_names[dsda_bf_operator_max];
extern const char* dsda_bf_item_names[dsda_bf_item_max];
extern const char* dsda_bf_limit_names[dsda_bf_limit_max];

dboolean dsda_BruteForce(void);
dboolean dsda_BruteForceEnded(void);
void dsda_ResetBruteForceConditions(void);
void dsda_SetBruteForceTarget(dsda_bf_attribute_t attribute,
                              dsda_bf_limit_t limit, fixed_t value, dboolean has_value);
void dsda_AddMiscBruteForceCondition(dsda_bf_attribute_t attribute, fixed_t value);
void dsda_AddBruteForceCondition(dsda_bf_attribute_t attribute,
                                 dsda_bf_operator_t operator, fixed_t value);
dboolean dsda_StartBruteForce(int depth);
int dsda_KeepBruteForceFrame(int i);
int dsda_AddBruteForceFrame(int i,
                            int forwardmove_min, int forwardmove_max,
                            int sidemove_min, int sidemove_max,
                            int angleturn_min, int angleturn_max,
                            byte buttons);
void dsda_BruteForceWithoutMonsters(void);
void dsda_BruteForceWithMonsters(void);
void dsda_UpdateBruteForce(void);
void dsda_EvaluateBruteForce(void);
void dsda_CopyBruteForceCommand(ticcmd_t* cmd);
