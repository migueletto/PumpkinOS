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

#include "info.h"

#include "pclass.h"

dsda_pclass_t pclass[NUMCLASSES] = {
  [PCLASS_NULL] = {
    .armor_increment = { 0 },
    .auto_armor_save = 0,
    .armor_max = 0,

    .forwardmove = { 0x19, 0x32 },
    .sidemove = { 0x18, 0x28 },
    .stroller_threshold = 0x19,
    .turbo_threshold = 0x32,
  },

  [PCLASS_FIGHTER] = {
    .armor_increment = { 25 * FRACUNIT, 20 * FRACUNIT, 15 * FRACUNIT, 5 * FRACUNIT },
    .auto_armor_save = 15 * FRACUNIT,
    .armor_max = 100 * FRACUNIT,

    .forwardmove = { 0x1D, 0x3C },
    .sidemove = { 0x1B, 0x3B },
    .stroller_threshold = 0x1D,
    .turbo_threshold = 0x3C,

    .normal_state = HEXEN_S_FPLAY,
    .run_state = HEXEN_S_FPLAY_RUN1,
    .fire_weapon_state = HEXEN_S_FPLAY_ATK1,
    .attack_state = HEXEN_S_FPLAY_ATK1,
    .attack_end_state = HEXEN_S_FPLAY_ATK2,
  },

  [PCLASS_CLERIC] = {
    .armor_increment = { 10 * FRACUNIT, 25 * FRACUNIT, 5 * FRACUNIT, 20 * FRACUNIT },
    .auto_armor_save = 10 * FRACUNIT,
    .armor_max = 90 * FRACUNIT,

    .forwardmove = { 0x19, 0x32 },
    .sidemove = { 0x18, 0x28 },
    .stroller_threshold = 0x19,
    .turbo_threshold = 0x32,

    .normal_state = HEXEN_S_CPLAY,
    .run_state = HEXEN_S_CPLAY_RUN1,
    .fire_weapon_state = HEXEN_S_CPLAY_ATK1,
    .attack_state = HEXEN_S_CPLAY_ATK1,
    .attack_end_state = HEXEN_S_CPLAY_ATK3,
  },

  [PCLASS_MAGE] = {
    .armor_increment = { 5 * FRACUNIT, 15 * FRACUNIT, 10 * FRACUNIT, 25 * FRACUNIT },
    .auto_armor_save = 5 * FRACUNIT,
    .armor_max = 80 * FRACUNIT,

    .forwardmove = { 0x16, 0x2E },
    .sidemove = { 0x15, 0x25 },
    .stroller_threshold = 0x16,
    .turbo_threshold = 0x2D,

    .normal_state = HEXEN_S_MPLAY,
    .run_state = HEXEN_S_MPLAY_RUN1,
    .fire_weapon_state = HEXEN_S_MPLAY_ATK1,
    .attack_state = HEXEN_S_MPLAY_ATK1,
    .attack_end_state = HEXEN_S_MPLAY_ATK2,
  },

  [PCLASS_PIG] = {
    .armor_increment = { 0 },
    .auto_armor_save = 0,
    .armor_max = 5 * FRACUNIT,

    .forwardmove = { 0x18, 0x31 },
    .sidemove = { 0x17, 0x27 },
    .stroller_threshold = 0x18,
    .turbo_threshold = 0x31,

    .normal_state = HEXEN_S_PIGPLAY,
    .run_state = HEXEN_S_PIGPLAY_RUN1,
    .fire_weapon_state = HEXEN_S_PIGPLAY_ATK1,
    .attack_state = HEXEN_S_PIGPLAY_ATK1,
    .attack_end_state = HEXEN_S_PIGPLAY_ATK1,
  },
};

void dsda_ResetNullPClass(void) {
  if (heretic)
  {
    pclass[PCLASS_NULL].normal_state = HERETIC_S_PLAY;
    pclass[PCLASS_NULL].run_state = HERETIC_S_PLAY_RUN1;
    pclass[PCLASS_NULL].fire_weapon_state = HERETIC_S_PLAY_ATK2;
    pclass[PCLASS_NULL].attack_state = HERETIC_S_PLAY_ATK1;
    pclass[PCLASS_NULL].attack_end_state = HERETIC_S_PLAY_ATK2;
  }
  else
  {
    pclass[PCLASS_NULL].normal_state = S_PLAY;
    pclass[PCLASS_NULL].run_state = S_PLAY_RUN1;
    pclass[PCLASS_NULL].fire_weapon_state = S_PLAY_ATK1;
    pclass[PCLASS_NULL].attack_state = S_PLAY_ATK1;
    pclass[PCLASS_NULL].attack_end_state = S_PLAY_ATK2;
  }
}
