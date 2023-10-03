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
//	DSDA Tools
//

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "doomstat.h"
#include "p_inter.h"
#include "p_tick.h"
#include "g_game.h"
#include "sounds.h"
#include "s_sound.h"
#include "am_map.h"

#include "dsda/analysis.h"
#include "dsda/args.h"
#include "dsda/build.h"
#include "dsda/demo.h"
#include "dsda/exhud.h"
#include "dsda/features.h"
#include "dsda/ghost.h"
#include "dsda/key_frame.h"
#include "dsda/mouse.h"
#include "dsda/settings.h"
#include "dsda/split_tracker.h"
#include "dsda/tracker.h"
#include "dsda/wad_stats.h"
#include "dsda.h"

#define TELEFRAG_DAMAGE 10000

// command-line toggles
int dsda_track_pacifist;
int dsda_track_100k;

int dsda_last_leveltime;
int dsda_last_gamemap;
int dsda_startmap;
int dsda_movie_target;
dboolean dsda_any_map_completed;

// other
int dsda_max_kill_requirement;
static int dsda_session_attempts = 1;

static int turbo_scale;
static int start_in_build_mode;

static int line_activation[2][LINE_ACTIVATION_INDEX_MAX + 1];
static int line_activation_frame;
static int line_activation_index;

static int dsda_time_keys;
static int dsda_time_use;
static int dsda_time_secrets;

dboolean dsda_IsWeapon(mobj_t* thing);
void dsda_DisplayNotification(const char* msg);
void dsda_ResetMapVariables(void);

dboolean dsda_ILComplete(void) {
  return dsda_any_map_completed && dsda_last_gamemap == dsda_startmap && !dsda_movie_target;
}

dboolean dsda_MovieComplete(void) {
  return dsda_any_map_completed && dsda_last_gamemap == dsda_movie_target && dsda_movie_target;
}

void dsda_WatchLineActivation(line_t* line, mobj_t* mo) {
  if (mo && mo->player) {
    if (line_activation_index < LINE_ACTIVATION_INDEX_MAX) {
      line_activation[line_activation_frame][line_activation_index] = line->iLineID;
      ++line_activation_index;
      line_activation[line_activation_frame][line_activation_index] = -1;
    }

    ++line->player_activations;
  }
}

int* dsda_PlayerActivatedLines(void) {
  return line_activation[!line_activation_frame];
}

static void dsda_FlipLineActivationTracker(void) {
  line_activation_frame = !line_activation_frame;
  line_activation_index = 0;

  line_activation[line_activation_frame][line_activation_index] = -1;
}

static void dsda_ResetLineActivationTracker(void) {
  line_activation[0][0] = -1;
  line_activation[1][0] = -1;
}

static void dsda_HandleTurbo(void) {
  dsda_arg_t* arg;

  arg = dsda_Arg(dsda_arg_turbo);

  if (arg->found)
    turbo_scale = arg->value.v_int;

  if (turbo_scale > 100)
    dsda_ToggleBuildTurbo();
}

int dsda_TurboScale(void) {
  return turbo_scale;
}

static dboolean frozen_mode;

dboolean dsda_FrozenMode(void) {
  return frozen_mode;
}

void dsda_ToggleFrozenMode(void) {
  if (demorecording || demoplayback)
    return;

  frozen_mode = !frozen_mode;
}

static void dsda_HandleBuild(void) {
  start_in_build_mode = dsda_Flag(dsda_arg_build);
}

int dsda_StartInBuildMode(void) {
  return start_in_build_mode;
}

void dsda_ReadCommandLine(void) {
  dsda_arg_t* arg;
  int dsda_time_all;

  dsda_track_pacifist = dsda_Flag(dsda_arg_track_pacifist);
  dsda_track_100k = dsda_Flag(dsda_arg_track_100k);
  dsda_analysis = dsda_Flag(dsda_arg_analysis);
  dsda_time_keys = dsda_SimpleIntArg(dsda_arg_time_keys);
  dsda_time_use = dsda_SimpleIntArg(dsda_arg_time_use);
  dsda_time_secrets = dsda_SimpleIntArg(dsda_arg_time_secrets);
  dsda_time_all = dsda_SimpleIntArg(dsda_arg_time_all);

  if ((arg = dsda_Arg(dsda_arg_movie))->found)
    dsda_movie_target = arg->value.v_int;

  if (dsda_time_all) {
    dsda_time_keys = dsda_time_all;
    dsda_time_use = dsda_time_all;
    dsda_time_secrets = dsda_time_all;
  }

  arg = dsda_Arg(dsda_arg_export_ghost);
  if (arg->found)
    dsda_InitGhostExport(arg->value.v_string);

  dsda_HandleTurbo();
  dsda_HandleBuild();

  arg = dsda_Arg(dsda_arg_import_ghost);
  if (arg->found)
    dsda_InitGhostImport(arg->value.v_string_array, arg->count);

  if (dsda_Flag(dsda_arg_tas) || dsda_Flag(dsda_arg_build)) dsda_SetTas();

  dsda_InitKeyFrame();
  dsda_InitCommandHistory();
}

static int dsda_shown_attempt = 0;

int dsda_SessionAttempts(void) {
  return dsda_session_attempts;
}

void dsda_DisplayNotifications(void) {
  if (dsda_ShowDemoAttempts() && dsda_session_attempts > dsda_shown_attempt) {
    doom_printf("Attempt %d / %d", dsda_session_attempts, dsda_DemoAttempts());

    dsda_shown_attempt = dsda_session_attempts;
  }

  if (!dsda_pacifist && dsda_track_pacifist && !dsda_pacifist_note_shown) {
    dsda_pacifist_note_shown = true;
    dsda_DisplayNotification("Not pacifist!");
  }

  if (dsda_100k_on_map && dsda_track_100k && !dsda_100k_note_shown) {
    dsda_TrackFeature(uf_100k);

    dsda_100k_note_shown = true;
    dsda_DisplayNotification("100K achieved!");
  }
}

void dsda_DecomposeILTime(dsda_level_time_t* level_time) {
  level_time->m = dsda_last_leveltime / 35 / 60;
  level_time->s = (dsda_last_leveltime % (60 * 35)) / 35;
  level_time->t = round(100.f * (dsda_last_leveltime % 35) / 35);
}

void dsda_DecomposeMovieTime(dsda_movie_time_t* total_time) {
  extern int totalleveltimes;

  total_time->h = totalleveltimes / 35 / 60 / 60;
  total_time->m = (totalleveltimes % (60 * 60 * 35)) / 35 / 60;
  total_time->s = (totalleveltimes % (60 * 35)) / 35;
}

void dsda_DisplayNotification(const char* msg) {
  S_StartVoidSound(gamemode == commercial ? sfx_radio : sfx_itmbk);
  doom_printf("%s", msg);
}

void dsda_WatchReborn(int playernum) {
  dsda_reborn = true;
}

void dsda_WatchCard(card_t card) {
  if (dsda_time_keys)
    switch (card) {
      case it_bluecard:
      case it_blueskull:
        dsda_AddSplit(DSDA_SPLIT_BLUE_KEY, dsda_time_keys);
        break;
      case it_yellowcard:
      case it_yellowskull:
        dsda_AddSplit(DSDA_SPLIT_YELLOW_KEY, dsda_time_keys);
        break;
      case it_redcard:
      case it_redskull:
        dsda_AddSplit(DSDA_SPLIT_RED_KEY, dsda_time_keys);
        break;
      default:
        break;
    }
}

static int player_damage_leveltime;
int player_damage_last_tic;

void dsda_WatchDamage(mobj_t* target, mobj_t* inflictor, mobj_t* source, int damage) {
  if (
    ((source && source->player) || (inflictor && inflictor->intflags & MIF_PLAYER_DAMAGED_BARREL)) \
    && damage != TELEFRAG_DAMAGE
  ) {
    if (!target->player) {
      if (leveltime != player_damage_leveltime) {
        player_damage_leveltime = leveltime;
        player_damage_last_tic = 0;
      }

      player_damage_last_tic += damage;
    }

    if (target->type == MT_BARREL || (heretic && target->type == HERETIC_MT_POD))
      target->intflags |= MIF_PLAYER_DAMAGED_BARREL;
    else if (!target->player)
      dsda_pacifist = false;
  }

  if (target->player) {
    dsda_reality = false;

    // "almost reality" means allowing nukage damage
    // we cannot differentiate between crushers and nukage in this scope
    // we account for crushers in dsda_WatchCrush instead
    if (inflictor) dsda_almost_reality = false;
  }
}

void dsda_WatchDeath(mobj_t* thing) {
  if (thing->flags & MF_COUNTKILL) {
    ++dsda_kills_on_map;

    if (dsda_kills_on_map >= totalkills) dsda_100k_on_map = true;
  }
}

void dsda_WatchKill(player_t* player, mobj_t* target) {
  player->killcount++;
  if (target->intflags & MIF_SPAWNED_BY_ICON) player->maxkilldiscount++;
  dsda_WadStatsKill();
}

void dsda_WatchResurrection(mobj_t* target, mobj_t* raiser) {
  int i;

  if (raiser && raiser->intflags & MIF_SPAWNED_BY_ICON)
    target->intflags |= MIF_SPAWNED_BY_ICON;

  if (
    (
      (target->flags ^ MF_COUNTKILL) &
      (MF_FRIEND | MF_COUNTKILL)
    ) || target->intflags & MIF_SPAWNED_BY_ICON
  ) return;

  for (i = 0; i < g_maxplayers; ++i) {
    if (!playeringame[i] || players[i].killcount == 0) continue;

    if (players[i].killcount > 0) {
      players[i].maxkilldiscount++;
      return;
    }
  }
}

void dsda_WatchCrush(mobj_t* thing, int damage) {
  player_t *player;

  player = thing->player;
  if (!player) return;

  // invincible
  if (
    (damage < 1000 || (!comp[comp_god] && (player->cheats&CF_GODMODE))) \
    && (player->cheats&CF_GODMODE || player->powers[pw_invulnerability])
  ) return;

  dsda_almost_reality = false;
}

void dsda_WatchSpawn(mobj_t* spawned) {
  if (
    (spawned->flags & MF_COUNTKILL) \
    || spawned->type == MT_SKULL \
    || spawned->type == MT_BOSSBRAIN
  ) dsda_any_monsters = true;

  if (!dsda_any_weapons) dsda_any_weapons = dsda_IsWeapon(spawned);

  if (!((spawned->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    ++dsda_max_kill_requirement;
}

void dsda_WatchFailedSpawn(mobj_t* spawned) {
  // Fix count from dsda_WatchSpawn
  if (!((spawned->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    --dsda_max_kill_requirement;
}

void dsda_WatchMorph(mobj_t* morphed) {
  // Fix count from dsda_WatchSpawn
  if (!((morphed->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    --dsda_max_kill_requirement;
}

void dsda_WatchUnMorph(mobj_t* morphed) {
  // Fix count from dsda_WatchSpawn
  if (!((morphed->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    --dsda_max_kill_requirement;
}

void dsda_WatchIconSpawn(mobj_t* spawned) {
  spawned->intflags |= MIF_SPAWNED_BY_ICON;

  // Fix count from dsda_WatchSpawn
  // We can't know inside P_SpawnMobj what the source is
  // This is less invasive than introducing a spawn source concept
  if (!((spawned->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    --dsda_max_kill_requirement;
}

int dsda_MaxKillRequirement() {
  return dsda_max_kill_requirement;
}

void dsda_WatchPTickCompleted(void) {
  dsda_FlipLineActivationTracker();
}

void dsda_WatchCommand(void) {
  int i;
  ticcmd_t* cmd;
  dsda_pclass_t *player_class;

  for (i = 0; i < g_maxplayers; ++i) {
    if (!playeringame[i]) continue;

    cmd = &players[i].cmd;
    player_class = &pclass[players[i].pclass];

    if (cmd->buttons & BT_USE && dsda_time_use)
      dsda_AddSplit(DSDA_SPLIT_USE, dsda_time_use);

    if (cmd->sidemove != 0 || abs(cmd->forwardmove) > player_class->stroller_threshold)
      dsda_stroller = false;

    if (
      abs(cmd->sidemove) > player_class->turbo_threshold ||
      abs(cmd->forwardmove) > player_class->turbo_threshold
    )
      dsda_turbo = true;
  }

  dsda_AddCommandToCommandDisplay(&players[displayplayer].cmd);

  dsda_ExportGhostFrame();
}

void dsda_WatchBeforeLevelSetup(void) {
  dsda_100k_on_map = false;
  dsda_kills_on_map = 0;
  dsda_100k_note_shown = false;
  dsda_max_kill_requirement = 0;
}

void dsda_WatchAfterLevelSetup(void) {
  dsda_SpawnGhost();
  dsda_ResetTrackers();
  dsda_ResetLineActivationTracker();
  dsda_WadStatsEnterMap();
  player_damage_last_tic = 0;
  player_damage_leveltime = 0;
}

void dsda_WatchNewLevel(void) {
  dsda_ResetAutoKeyFrameTimeout();
}

void dsda_WatchLevelCompletion(void) {
  thinker_t *th;
  mobj_t *mobj;
  int i;
  int secret_count = 0;
  int kill_count = 0;
  int missed_monsters = 0;

  for (th = thinkercap.next; th != &thinkercap; th = th->next) {
    if (th->function != P_MobjThinker) continue;

    mobj = (mobj_t *)th;

    // max rules: everything dead that affects kill counter except icon spawns
    if (
      !((mobj->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)) \
      && !(mobj->intflags & MIF_SPAWNED_BY_ICON) \
      && mobj->health > 0
    ) {
      ++missed_monsters;
    }

    if (dsda_IsWeapon(mobj)) {
      ++dsda_missed_weapons;
      dsda_weapon_collector = false;
    }
  }

  dsda_missed_monsters += missed_monsters;

  for (i = 0; i < g_maxplayers; ++i) {
    if (!playeringame[i]) continue;

    kill_count += players[i].killcount;
    secret_count += players[i].secretcount;
  }

  dsda_missed_secrets += (totalsecret - secret_count);

  if (kill_count < totalkills) dsda_100k = false;
  if (secret_count < totalsecret) dsda_100s = false;
  if (totalkills > 0) dsda_any_counted_monsters = true;
  if (totalsecret > 0) dsda_any_secrets = true;

  dsda_last_leveltime = leveltime;
  dsda_last_gamemap = gamemap;
  dsda_any_map_completed = true;

  dsda_RecordSplit();
  dsda_WadStatsExitMap(missed_monsters);
}

dboolean dsda_IsWeapon(mobj_t* thing) {
  switch (thing->sprite) {
    case SPR_BFUG:
    case SPR_MGUN:
    case SPR_CSAW:
    case SPR_LAUN:
    case SPR_PLAS:
    case SPR_SHOT:
    case SPR_SGN2:
      return true;
    default:
      return false;
  }
}

void dsda_WatchWeaponFire(weapontype_t weapon) {
  if (weapon == wp_fist || weapon == wp_pistol || weapon == wp_chainsaw) return;

  dsda_tyson_weapons = false;
}

void dsda_WatchSecret(void) {
  if (dsda_time_secrets)
    dsda_AddSplit(DSDA_SPLIT_SECRET, dsda_time_secrets);
}

static void dsda_ResetTracking(void) {
  dsda_ResetAnalysis();

  dsda_pacifist_note_shown = false;
}

void dsda_WatchDeferredInitNew(int skill, int episode, int map) {
  if (!demorecording) return;

  ++dsda_session_attempts;

  dsda_ResetTracking();
  dsda_QueueQuickstart();

  dsda_ResetRevealMap();
  G_CheckDemoStatus();

  dsda_last_gamemap = 0;
  dsda_last_leveltime = 0;
  dsda_any_map_completed = false;

  dsda_InitDemoRecording();

  boom_basetic = gametic;
  true_basetic = gametic;
}

void dsda_WatchNewGame(void) {
  if (!demorecording) return;

  G_BeginRecording();
}

void dsda_WatchLevelReload(int* reloaded) {
  extern int startmap;

  if (!demorecording || *reloaded) return;

  G_DeferedInitNew(gameskill, gameepisode, startmap);
  *reloaded = 1;
}
