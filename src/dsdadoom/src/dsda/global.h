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
//	DSDA Global - define top level globals for doom vs heretic
//

#ifndef __DSDA_GLOBAL__
#define __DSDA_GLOBAL__

#include "doomtype.h"

extern int g_maxplayers;
extern int g_viewheight;
extern int g_numammo;

extern int g_mt_player;
extern int g_mt_tfog;
extern int g_mt_blood;
extern int g_skullpop_mt;
extern int g_s_bloodyskullx1;
extern int g_s_bloodyskullx2;
extern int g_s_play_fdth20;

extern int g_wp_fist;
extern int g_wp_chainsaw;
extern int g_wp_pistol;

extern int g_telefog_height;
extern int g_thrust_factor;
extern int g_fuzzy_aim_shift;

extern int g_s_null;

extern int g_mt_bloodsplatter;
extern int g_bloodsplatter_shift;
extern int g_bloodsplatter_weight;
extern int g_mons_look_range;
extern int g_hide_state;
extern int g_lava_type;

extern int g_mntr_charge_speed;
extern int g_mntr_atk1_sfx;
extern int g_mntr_decide_range;
extern int g_mntr_charge_rng;
extern int g_mntr_fire_rng;
extern int g_mntr_charge_state;
extern int g_mntr_fire_state;
extern int g_mntr_charge_puff;
extern int g_mntr_atk2_sfx;
extern int g_mntr_atk2_dice;
extern int g_mntr_atk2_missile;
extern int g_mntr_atk3_sfx;
extern int g_mntr_atk3_dice;
extern int g_mntr_atk3_missile;
extern int g_mntr_atk3_state;
extern int g_mntr_fire;

extern int g_arti_health;
extern int g_arti_superhealth;
extern int g_arti_fly;
extern int g_arti_limit;

extern int g_sfx_telept;
extern int g_sfx_sawup;
extern int g_sfx_stnmov;
extern int g_sfx_stnmov_plats;
extern int g_sfx_swtchn;
extern int g_sfx_swtchx;
extern int g_sfx_dorcls;
extern int g_sfx_doropn;
extern int g_sfx_dorlnd;
extern int g_sfx_pstart;
extern int g_sfx_pstop;
extern int g_sfx_itemup;
extern int g_sfx_pistol;
extern int g_sfx_oof;
extern int g_sfx_menu;
extern int g_sfx_respawn;
extern int g_sfx_secret;
extern int g_sfx_revive;
extern int g_sfx_console;

extern int g_door_normal;
extern int g_door_raise_in_5_mins;
extern int g_door_open;

extern int g_st_height;
extern int g_border_offset;
extern int g_mf_translucent;
extern int g_mf_shadow;

extern const char* g_skyflatname;

extern dboolean heretic;

void dsda_InitGlobal(void);

#endif
