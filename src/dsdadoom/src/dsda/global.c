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

#include <stdlib.h>
#include <string.h>

#include "info.h"
#include "d_items.h"
#include "p_inter.h"
#include "p_spec.h"
#include "p_map.h"
#include "sounds.h"
#include "d_main.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "heretic/def.h"
#include "stricmp.h"

#include "global.h"

#include "dsda/args.h"
#include "dsda/map_format.h"
#include "dsda/mobjinfo.h"
#include "dsda/music.h"
#include "dsda/sfx.h"
#include "dsda/sprite.h"
#include "dsda/state.h"

#define IGNORE_VALUE -1

const demostate_t (*demostates)[4];
extern const demostate_t doom_demostates[][4];
extern const demostate_t heretic_demostates[][4];
extern const demostate_t hexen_demostates[][4];

weaponinfo_t* weaponinfo;

int g_maxplayers = 4;
int g_viewheight = 41 * FRACUNIT;
int g_numammo;

int g_mt_player;
int g_mt_tfog;
int g_mt_blood;
int g_skullpop_mt;
int g_s_bloodyskullx1;
int g_s_bloodyskullx2;
int g_s_play_fdth20;

int g_wp_fist;
int g_wp_chainsaw;
int g_wp_pistol;

int g_telefog_height;
int g_thrust_factor;
int g_fuzzy_aim_shift;

int g_s_null;

int g_mt_bloodsplatter;
int g_bloodsplatter_shift;
int g_bloodsplatter_weight;
int g_mons_look_range;
int g_hide_state;
int g_lava_type;

int g_mntr_charge_speed;
int g_mntr_atk1_sfx;
int g_mntr_decide_range;
int g_mntr_charge_rng;
int g_mntr_fire_rng;
int g_mntr_charge_state;
int g_mntr_fire_state;
int g_mntr_charge_puff;
int g_mntr_atk2_sfx;
int g_mntr_atk2_dice;
int g_mntr_atk2_missile;
int g_mntr_atk3_sfx;
int g_mntr_atk3_dice;
int g_mntr_atk3_missile;
int g_mntr_atk3_state;
int g_mntr_fire;

int g_arti_health;
int g_arti_superhealth;
int g_arti_fly;
int g_arti_limit;

int g_sfx_sawup;
int g_sfx_telept;
int g_sfx_stnmov;
int g_sfx_stnmov_plats;
int g_sfx_swtchn;
int g_sfx_swtchx;
int g_sfx_dorcls;
int g_sfx_doropn;
int g_sfx_dorlnd;
int g_sfx_pstart;
int g_sfx_pstop;
int g_sfx_itemup;
int g_sfx_pistol;
int g_sfx_oof;
int g_sfx_menu;
int g_sfx_respawn;
int g_sfx_secret;
int g_sfx_revive;
int g_sfx_console;

int g_door_normal;
int g_door_raise_in_5_mins;
int g_door_open;

int g_st_height;
int g_border_offset;
int g_mf_translucent;
int g_mf_shadow;

const char* g_menu_flat;
int g_menu_save_page_size;
int g_menu_font_spacing;

const char* g_skyflatname;

dboolean hexen = false;
dboolean heretic = false;
dboolean raven = false;

static void dsda_InitDoom(void) {
  int i;
  doom_mobjinfo_t* mobjinfo_p;

  dsda_InitializeMobjInfo(DOOM_MT_ZERO, DOOM_NUMMOBJTYPES, DOOM_NUMMOBJTYPES);
  dsda_InitializeStates(doom_states, DOOM_NUMSTATES);
  dsda_InitializeSprites(doom_sprnames, DOOM_NUMSPRITES);
  dsda_InitializeSFX(doom_S_sfx, DOOM_NUMSFX);
  dsda_InitializeMusic(doom_S_music, DOOM_NUMMUSIC);

  demostates = doom_demostates;

  weaponinfo = doom_weaponinfo;

  g_maxplayers = 4;
  g_viewheight = 41 * FRACUNIT;
  g_numammo = DOOM_NUMAMMO;

  g_mt_player = MT_PLAYER;
  g_mt_tfog = MT_TFOG;
  g_mt_blood = MT_BLOOD;
  g_skullpop_mt = MT_GIBDTH;

  g_wp_fist = wp_fist;
  g_wp_chainsaw = wp_chainsaw;
  g_wp_pistol = wp_pistol;

  g_telefog_height = 0;
  g_thrust_factor = 100;
  g_fuzzy_aim_shift = 20;

  g_s_null = S_NULL;

  g_sfx_sawup = sfx_sawup;
  g_sfx_telept = sfx_telept;
  g_sfx_stnmov = sfx_stnmov;
  g_sfx_stnmov_plats = sfx_stnmov;
  g_sfx_swtchn = sfx_swtchn;
  g_sfx_swtchx = sfx_swtchx;
  g_sfx_dorcls = sfx_dorcls;
  g_sfx_doropn = sfx_doropn;
  g_sfx_dorlnd = sfx_dorcls;
  g_sfx_pstart = sfx_pstart;
  g_sfx_pstop = sfx_pstop;
  g_sfx_itemup = sfx_itemup;
  g_sfx_pistol = sfx_pistol;
  g_sfx_oof = sfx_oof;
  g_sfx_menu = sfx_pstop;
  g_sfx_secret = sfx_secret;
  g_sfx_revive = sfx_slop;
  g_sfx_console = sfx_radio;

  g_door_normal = normal;
  g_door_raise_in_5_mins = waitRaiseDoor;
  g_door_open = openDoor;

  g_st_height = 32;
  g_border_offset = 8;
  g_mf_translucent = MF_TRANSLUCENT;
  g_mf_shadow = MF_SHADOW;

  g_menu_flat = "FLOOR4_6";
  g_menu_save_page_size = 7;
  g_menu_font_spacing = -1;

  g_skyflatname = "F_SKY1";

  // convert doom mobj types to shared type
  for (i = 0; i < DOOM_NUMMOBJTYPES; ++i) {
    mobjinfo_p = &doom_mobjinfo[i];

    mobjinfo[i].doomednum    = mobjinfo_p->doomednum;
    mobjinfo[i].spawnstate   = mobjinfo_p->spawnstate;
    mobjinfo[i].spawnhealth  = mobjinfo_p->spawnhealth;
    mobjinfo[i].seestate     = mobjinfo_p->seestate;
    mobjinfo[i].seesound     = mobjinfo_p->seesound;
    mobjinfo[i].reactiontime = mobjinfo_p->reactiontime;
    mobjinfo[i].attacksound  = mobjinfo_p->attacksound;
    mobjinfo[i].painstate    = mobjinfo_p->painstate;
    mobjinfo[i].painchance   = mobjinfo_p->painchance;
    mobjinfo[i].painsound    = mobjinfo_p->painsound;
    mobjinfo[i].meleestate   = mobjinfo_p->meleestate;
    mobjinfo[i].missilestate = mobjinfo_p->missilestate;
    mobjinfo[i].deathstate   = mobjinfo_p->deathstate;
    mobjinfo[i].xdeathstate  = mobjinfo_p->xdeathstate;
    mobjinfo[i].deathsound   = mobjinfo_p->deathsound;
    mobjinfo[i].speed        = mobjinfo_p->speed;
    mobjinfo[i].radius       = mobjinfo_p->radius;
    mobjinfo[i].height       = mobjinfo_p->height;
    mobjinfo[i].mass         = mobjinfo_p->mass;
    mobjinfo[i].damage       = mobjinfo_p->damage;
    mobjinfo[i].activesound  = mobjinfo_p->activesound;
    mobjinfo[i].flags        = mobjinfo_p->flags;
    mobjinfo[i].raisestate   = mobjinfo_p->raisestate;
    mobjinfo[i].droppeditem  = MT_NULL;
    mobjinfo[i].crashstate   = 0; // not in doom
    mobjinfo[i].flags2       = 0; // not in doom

    // mbf21
    mobjinfo[i].infighting_group = IG_DEFAULT;
    mobjinfo[i].projectile_group = PG_DEFAULT;
    mobjinfo[i].splash_group = SG_DEFAULT;
    mobjinfo[i].ripsound = sfx_None;
    mobjinfo[i].altspeed = NO_ALTSPEED;
    mobjinfo[i].meleerange = MELEERANGE;

    // misc
    mobjinfo[i].bloodcolor = 0; // default
    mobjinfo[i].visibility = VF_DOOM;
  }

  // don't want to reorganize info.c structure for a few tweaks...
  mobjinfo[MT_WOLFSS].droppeditem    = MT_CLIP;
  mobjinfo[MT_POSSESSED].droppeditem = MT_CLIP;
  mobjinfo[MT_SHOTGUY].droppeditem   = MT_SHOTGUN;
  mobjinfo[MT_CHAINGUY].droppeditem  = MT_CHAINGUN;

  mobjinfo[MT_VILE].flags2    = MF2_SHORTMRANGE | MF2_DMGIGNORED | MF2_NOTHRESHOLD;
  mobjinfo[MT_CYBORG].flags2  = MF2_NORADIUSDMG | MF2_HIGHERMPROB | MF2_RANGEHALF |
                                MF2_FULLVOLSOUNDS | MF2_E2M8BOSS | MF2_E4M6BOSS;
  mobjinfo[MT_SPIDER].flags2  = MF2_NORADIUSDMG | MF2_RANGEHALF | MF2_FULLVOLSOUNDS |
                                MF2_E3M8BOSS | MF2_E4M8BOSS;
  mobjinfo[MT_SKULL].flags2   = MF2_RANGEHALF;
  mobjinfo[MT_FATSO].flags2   = MF2_MAP07BOSS1;
  mobjinfo[MT_BABY].flags2    = MF2_MAP07BOSS2;
  mobjinfo[MT_BRUISER].flags2 = MF2_E1M8BOSS;
  mobjinfo[MT_UNDEAD].flags2  = MF2_LONGMELEE | MF2_RANGEHALF;

  mobjinfo[MT_BRUISER].projectile_group = PG_BARON;
  mobjinfo[MT_KNIGHT].projectile_group = PG_BARON;

  mobjinfo[MT_BRUISERSHOT].altspeed = 20 * FRACUNIT;
  mobjinfo[MT_HEADSHOT].altspeed = 20 * FRACUNIT;
  mobjinfo[MT_TROOPSHOT].altspeed = 20 * FRACUNIT;

  for (i = S_SARG_RUN1; i <= S_SARG_PAIN2; ++i)
    states[i].flags |= STATEF_SKILL5FAST;
}

static void dsda_InitHeretic(void) {
  int i, j;
  raven_mobjinfo_t* mobjinfo_p;

  dsda_InitializeMobjInfo(HERETIC_MT_ZERO, HERETIC_NUMMOBJTYPES, HERETIC_NUMMOBJTYPES);
  dsda_InitializeStates(heretic_states, HERETIC_NUMSTATES);
  dsda_InitializeSprites(heretic_sprnames, HERETIC_NUMSPRITES);
  dsda_InitializeSFX(heretic_S_sfx, HERETIC_NUMSFX);
  dsda_InitializeMusic(heretic_S_music, HERETIC_NUMMUSIC);

  demostates = heretic_demostates;

  weaponinfo = wpnlev1info;

  g_maxplayers = 4;
  g_viewheight = 41 * FRACUNIT;
  g_numammo = HERETIC_NUMAMMO;

  g_mt_player = HERETIC_MT_PLAYER;
  g_mt_tfog = HERETIC_MT_TFOG;
  g_mt_blood = HERETIC_MT_BLOOD;
  g_skullpop_mt = HERETIC_MT_BLOODYSKULL;
  g_s_bloodyskullx1 = HERETIC_S_BLOODYSKULLX1;
  g_s_bloodyskullx2 = HERETIC_S_BLOODYSKULLX2;
  g_s_play_fdth20 = HERETIC_S_PLAY_FDTH20;

  g_wp_fist = wp_staff;
  g_wp_chainsaw = wp_gauntlets;
  g_wp_pistol = wp_goldwand;

  g_telefog_height = TELEFOGHEIGHT;
  g_thrust_factor = 150;
  g_fuzzy_aim_shift = 21;

  g_s_null = HERETIC_S_NULL;

  g_mt_bloodsplatter = HERETIC_MT_BLOODSPLATTER;
  g_bloodsplatter_shift = 9;
  g_bloodsplatter_weight = 2;
  g_mons_look_range = 20 * 64 * FRACUNIT;
  g_hide_state = HERETIC_S_HIDESPECIAL1;
  g_lava_type = HERETIC_MT_PHOENIXFX2;

  g_mntr_atk1_sfx = heretic_sfx_stfpow;
  g_mntr_charge_speed = 13 * FRACUNIT;
  g_mntr_decide_range = 8;
  g_mntr_charge_rng = 150;
  g_mntr_charge_state = HERETIC_S_MNTR_ATK4_1;
  g_mntr_fire_rng = 220;
  g_mntr_fire_state = HERETIC_S_MNTR_ATK3_1;
  g_mntr_charge_puff = HERETIC_MT_PHOENIXPUFF;
  g_mntr_atk2_sfx = heretic_sfx_minat2;
  g_mntr_atk2_dice = 5;
  g_mntr_atk2_missile = HERETIC_MT_MNTRFX1;
  g_mntr_atk3_sfx = heretic_sfx_minat1;
  g_mntr_atk3_dice = 5;
  g_mntr_atk3_missile = HERETIC_MT_MNTRFX2;
  g_mntr_atk3_state = HERETIC_S_MNTR_ATK3_4;
  g_mntr_fire = HERETIC_MT_MNTRFX3;

  g_arti_health = arti_health;
  g_arti_superhealth = arti_superhealth;
  g_arti_fly = arti_fly;
  g_arti_limit = 16;

  g_sfx_sawup = heretic_sfx_gntact;
  g_sfx_telept = heretic_sfx_telept;
  g_sfx_stnmov = heretic_sfx_dormov;
  g_sfx_stnmov_plats = heretic_sfx_stnmov;
  g_sfx_swtchn = heretic_sfx_switch;
  g_sfx_swtchx = heretic_sfx_switch;
  g_sfx_dorcls = heretic_sfx_doropn;
  g_sfx_doropn = heretic_sfx_doropn;
  g_sfx_dorlnd = heretic_sfx_dorcls;
  g_sfx_pstart = heretic_sfx_pstart;
  g_sfx_pstop = heretic_sfx_pstop;
  g_sfx_itemup = heretic_sfx_itemup;
  g_sfx_pistol = heretic_sfx_gldhit;
  g_sfx_oof = heretic_sfx_plroof;
  g_sfx_menu = heretic_sfx_dorcls;
  g_sfx_respawn = heretic_sfx_respawn;
  g_sfx_secret = heretic_sfx_chat;
  g_sfx_revive = heretic_sfx_telept;
  g_sfx_console = heretic_sfx_chat;

  g_door_normal = vld_normal;
  g_door_raise_in_5_mins = vld_raiseIn5Mins;
  g_door_open = vld_open;

  g_st_height = 42;
  g_border_offset = 4;
  g_mf_translucent = MF_SHADOW;
  g_mf_shadow = 0; // doesn't exist in heretic

  g_menu_flat = "FLOOR30";
  g_menu_save_page_size = 5;
  g_menu_font_spacing = 0;

  g_skyflatname = "F_SKY1";

  // convert heretic mobj types to shared type
  for (i = 0; i < HERETIC_NUMMOBJTYPES - HERETIC_MT_ZERO; ++i) {
    mobjinfo_p = &heretic_mobjinfo[i];

    j = i + HERETIC_MT_ZERO;
    mobjinfo[j].doomednum    = mobjinfo_p->doomednum;
    mobjinfo[j].spawnstate   = mobjinfo_p->spawnstate;
    mobjinfo[j].spawnhealth  = mobjinfo_p->spawnhealth;
    mobjinfo[j].seestate     = mobjinfo_p->seestate;
    mobjinfo[j].seesound     = mobjinfo_p->seesound;
    mobjinfo[j].reactiontime = mobjinfo_p->reactiontime;
    mobjinfo[j].attacksound  = mobjinfo_p->attacksound;
    mobjinfo[j].painstate    = mobjinfo_p->painstate;
    mobjinfo[j].painchance   = mobjinfo_p->painchance;
    mobjinfo[j].painsound    = mobjinfo_p->painsound;
    mobjinfo[j].meleestate   = mobjinfo_p->meleestate;
    mobjinfo[j].missilestate = mobjinfo_p->missilestate;
    mobjinfo[j].deathstate   = mobjinfo_p->deathstate;
    mobjinfo[j].xdeathstate  = mobjinfo_p->xdeathstate;
    mobjinfo[j].deathsound   = mobjinfo_p->deathsound;
    mobjinfo[j].speed        = mobjinfo_p->speed;
    mobjinfo[j].radius       = mobjinfo_p->radius;
    mobjinfo[j].height       = mobjinfo_p->height;
    mobjinfo[j].mass         = mobjinfo_p->mass;
    mobjinfo[j].damage       = mobjinfo_p->damage;
    mobjinfo[j].activesound  = mobjinfo_p->activesound;
    mobjinfo[j].flags        = mobjinfo_p->flags;
    mobjinfo[j].raisestate   = 0; // not in heretic
    mobjinfo[j].droppeditem  = 0; // not in heretic
    mobjinfo[j].crashstate   = mobjinfo_p->crashstate;
    mobjinfo[j].flags2       = mobjinfo_p->flags2;

    // mbf21
    mobjinfo[j].infighting_group = IG_DEFAULT;
    mobjinfo[j].projectile_group = PG_DEFAULT;
    mobjinfo[j].splash_group = SG_DEFAULT;
    mobjinfo[j].ripsound = heretic_sfx_None;
    mobjinfo[j].altspeed = NO_ALTSPEED;
    mobjinfo[j].meleerange = MELEERANGE;

    // misc
    mobjinfo[j].bloodcolor = 0; // default
    mobjinfo[j].visibility = VF_HERETIC;
  }

  // heretic doesn't use "clip" concept
  for (i = 0; i < NUMAMMO; ++i) clipammo[i] = 1;

  // so few it's not worth implementing a pointer swap
  maxammo[0] = 100; // gold wand
  maxammo[1] = 50;  // crossbow
  maxammo[2] = 200; // blaster
  maxammo[3] = 200; // skull rod
  maxammo[4] = 20;  // phoenix rod
  maxammo[5] = 150; // mace
}

static void dsda_InitHexen(void) {
  int i, j;
  raven_mobjinfo_t* mobjinfo_p;

  dsda_InitializeMobjInfo(HEXEN_MT_ZERO, HEXEN_NUMMOBJTYPES, TOTAL_NUMMOBJTYPES);
  dsda_InitializeStates(hexen_states, HEXEN_NUMSTATES);
  dsda_InitializeSprites(hexen_sprnames, HEXEN_NUMSPRITES);
  dsda_InitializeSFX(hexen_S_sfx, HEXEN_NUMSFX);
  dsda_InitializeMusic(hexen_S_music, HEXEN_NUMMUSIC);

  demostates = hexen_demostates;

  // weaponinfo = wpnlev1info;

  g_maxplayers = 8;
  g_viewheight = 48 * FRACUNIT;
  g_numammo = NUMMANA;

  // g_mt_player = HERETIC_MT_PLAYER;
  g_mt_tfog = HEXEN_MT_TFOG;
  g_mt_blood = HEXEN_MT_BLOOD;
  g_skullpop_mt = HEXEN_MT_BLOODYSKULL;
  g_s_bloodyskullx1 = HEXEN_S_BLOODYSKULLX1;
  g_s_bloodyskullx2 = HEXEN_S_BLOODYSKULLX2;
  g_s_play_fdth20 = HEXEN_S_PLAY_FDTH20;

  // g_wp_fist = wp_staff;
  // g_wp_chainsaw = wp_gauntlets;
  // g_wp_pistol = wp_goldwand;

  g_telefog_height = TELEFOGHEIGHT;
  g_thrust_factor = 150;
  g_fuzzy_aim_shift = 21;

  g_s_null = HEXEN_S_NULL;

  g_mt_bloodsplatter = HEXEN_MT_BLOODSPLATTER;
  g_bloodsplatter_shift = 10;
  g_bloodsplatter_weight = 3;
  g_mons_look_range = 16 * 64 * FRACUNIT;
  g_hide_state = HEXEN_S_HIDESPECIAL1;
  g_lava_type = HEXEN_MT_CIRCLEFLAME;

  g_mntr_atk1_sfx = hexen_sfx_maulator_hammer_swing;
  g_mntr_charge_speed = 23 * FRACUNIT;
  g_mntr_decide_range = 16;
  g_mntr_charge_rng = 230;
  g_mntr_charge_state = HEXEN_S_MNTR_ATK4_1;
  g_mntr_fire_rng = 100;
  g_mntr_fire_state = HEXEN_S_MNTR_ATK3_1;
  g_mntr_charge_puff = HEXEN_MT_PUNCHPUFF;
  g_mntr_atk2_sfx = hexen_sfx_maulator_hammer_swing;
  g_mntr_atk2_dice = 3;
  g_mntr_atk2_missile = HEXEN_MT_MNTRFX1;
  g_mntr_atk3_sfx = hexen_sfx_maulator_hammer_hit;
  g_mntr_atk3_dice = 3;
  g_mntr_atk3_missile = HEXEN_MT_MNTRFX2;
  g_mntr_atk3_state = HEXEN_S_MNTR_ATK3_4;
  g_mntr_fire = HEXEN_MT_MNTRFX3;

  g_arti_health = hexen_arti_health;
  g_arti_superhealth = hexen_arti_superhealth;
  g_arti_fly = hexen_arti_fly;
  g_arti_limit = 25;

  g_sfx_telept = hexen_sfx_teleport;
  g_sfx_stnmov = hexen_sfx_door_light_close;
  g_sfx_swtchn = hexen_sfx_fighter_hammer_hitwall;
  g_sfx_swtchx = hexen_sfx_fighter_hammer_hitwall;
  g_sfx_dorcls = hexen_sfx_door_light_close;
  g_sfx_doropn = hexen_sfx_door_open;
  g_sfx_dorlnd = hexen_sfx_door_light_close;
  g_sfx_itemup = hexen_sfx_pickup_key;
  g_sfx_pistol = hexen_sfx_fighter_hammer_hitwall;
  g_sfx_oof = hexen_sfx_player_fighter_grunt;
  g_sfx_menu = hexen_sfx_door_light_close;
  g_sfx_respawn = hexen_sfx_respawn;
  g_sfx_secret = hexen_sfx_chat;
  g_sfx_revive = hexen_sfx_teleport;
  g_sfx_console = hexen_sfx_chat;

  g_st_height = 39;
  g_border_offset = 4;
  g_mf_translucent = MF_SHADOW; // hexen_note: how does ALTSHADOW fit in?
  g_mf_shadow = 0; // doesn't exist in hexen

  g_menu_flat = "F_032";
  g_menu_save_page_size = 5;
  g_menu_font_spacing = 0;

  g_skyflatname = "F_SKY";

  // convert hexen mobj types to shared type
  for (i = 0; i < HEXEN_NUMMOBJTYPES - HEXEN_MT_ZERO; ++i) {
    mobjinfo_p = &hexen_mobjinfo[i];

    j = i + HEXEN_MT_ZERO;
    mobjinfo[j].doomednum    = mobjinfo_p->doomednum;
    mobjinfo[j].spawnstate   = mobjinfo_p->spawnstate;
    mobjinfo[j].spawnhealth  = mobjinfo_p->spawnhealth;
    mobjinfo[j].seestate     = mobjinfo_p->seestate;
    mobjinfo[j].seesound     = mobjinfo_p->seesound;
    mobjinfo[j].reactiontime = mobjinfo_p->reactiontime;
    mobjinfo[j].attacksound  = mobjinfo_p->attacksound;
    mobjinfo[j].painstate    = mobjinfo_p->painstate;
    mobjinfo[j].painchance   = mobjinfo_p->painchance;
    mobjinfo[j].painsound    = mobjinfo_p->painsound;
    mobjinfo[j].meleestate   = mobjinfo_p->meleestate;
    mobjinfo[j].missilestate = mobjinfo_p->missilestate;
    mobjinfo[j].deathstate   = mobjinfo_p->deathstate;
    mobjinfo[j].xdeathstate  = mobjinfo_p->xdeathstate;
    mobjinfo[j].deathsound   = mobjinfo_p->deathsound;
    mobjinfo[j].speed        = mobjinfo_p->speed;
    mobjinfo[j].radius       = mobjinfo_p->radius;
    mobjinfo[j].height       = mobjinfo_p->height;
    mobjinfo[j].mass         = mobjinfo_p->mass;
    mobjinfo[j].damage       = mobjinfo_p->damage;
    mobjinfo[j].activesound  = mobjinfo_p->activesound;
    mobjinfo[j].flags        = mobjinfo_p->flags;
    mobjinfo[j].raisestate   = 0; // not in hexen
    mobjinfo[j].droppeditem  = 0; // not in hexen
    mobjinfo[j].crashstate   = mobjinfo_p->crashstate;
    mobjinfo[j].flags2       = mobjinfo_p->flags2;

    // mbf21
    mobjinfo[j].infighting_group = IG_DEFAULT;
    mobjinfo[j].projectile_group = PG_DEFAULT;
    mobjinfo[j].splash_group = SG_DEFAULT;
    mobjinfo[j].ripsound = hexen_sfx_None;
    mobjinfo[j].altspeed = NO_ALTSPEED;
    mobjinfo[j].meleerange = MELEERANGE;

    // misc
    mobjinfo[j].bloodcolor = 0; // default
    mobjinfo[j].visibility = VF_HEXEN;
  }

  {
    extern void P_UseHexenRNG(void);

    P_UseHexenRNG();
  }
}

static dboolean dsda_AutoDetectHeretic(void)
{
  dsda_arg_t* arg;
  int length;
  arg = dsda_Arg(dsda_arg_iwad);
  if (arg->found) {
    length = strlen(arg->value.v_string);
    if (length >= 11 && !strnicmp(arg->value.v_string + length - 11, "heretic.wad", 11))
      return true;
  }

  return false;
}

static dboolean dsda_AutoDetectHexen(void)
{
  dsda_arg_t* arg;
  int length;
  arg = dsda_Arg(dsda_arg_iwad);
  if (arg->found) {
    length = strlen(arg->value.v_string);
    if (length >= 9 && !strnicmp(arg->value.v_string + length - 9, "hexen.wad", 9))
      return true;
  }

  return false;
}

extern void dsda_ResetNullPClass(void);

void dsda_InitGlobal(void) {
  heretic = dsda_Flag(dsda_arg_heretic) || dsda_AutoDetectHeretic();
  hexen = dsda_Flag(dsda_arg_hexen) || dsda_AutoDetectHexen();
  raven = heretic || hexen;

  if (hexen)
    dsda_InitHexen();
  else if (heretic)
    dsda_InitHeretic();
  else
    dsda_InitDoom();

  dsda_ResetNullPClass();
}
