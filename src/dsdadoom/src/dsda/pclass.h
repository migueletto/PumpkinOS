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
//	DSDA Player Class
//

#ifndef __DSDA_PCLASS__
#define __DSDA_PCLASS__

#include "m_fixed.h"
#include "doomdef.h"

typedef struct dsda_pclass_s {
  int armor_increment[NUMARMOR];
  int auto_armor_save;
  int armor_max;

  fixed_t forwardmove[2];
  fixed_t sidemove[2];
  fixed_t stroller_threshold;
  fixed_t turbo_threshold;

  int normal_state;
  int run_state;
  int fire_weapon_state;
  int attack_state;
  int attack_end_state;
} dsda_pclass_t;

extern dsda_pclass_t pclass[NUMCLASSES];

#endif
