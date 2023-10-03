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

#include <math.h>

#include "d_player.h"
#include "d_ticcmd.h"
#include "doomstat.h"
#include "lprintf.h"
#include "m_random.h"
#include "r_state.h"

#include "dsda/build.h"
#include "dsda/demo.h"
#include "dsda/features.h"
#include "dsda/key_frame.h"
#include "dsda/skip.h"
#include "dsda/time.h"
#include "dsda/utility.h"

#include "brute_force.h"

#define MAX_BF_DEPTH 35
#define MAX_BF_CONDITIONS 16

typedef struct {
  int min;
  int max;
  int i;
} bf_range_t;

typedef struct {
  dsda_key_frame_t key_frame;
  bf_range_t forwardmove;
  bf_range_t sidemove;
  bf_range_t angleturn;
  byte buttons;
} bf_t;

typedef struct {
  dsda_bf_attribute_t attribute;
  dsda_bf_operator_t operator;
  fixed_t value;
  fixed_t secondary_value;
} bf_condition_t;

typedef struct {
  dsda_bf_attribute_t attribute;
  dsda_bf_limit_t limit;
  fixed_t value;
  dboolean enabled;
  dboolean evaluated;
  fixed_t best_value;
  int best_depth;
  bf_t best_bf[MAX_BF_DEPTH];
} bf_target_t;

static bf_t brute_force[MAX_BF_DEPTH];
static int bf_depth;
static int bf_logictic;
static int bf_condition_count;
static bf_condition_t bf_condition[MAX_BF_CONDITIONS];
static long long bf_volume;
static long long bf_volume_max;
static dboolean bf_mode;
static dboolean bf_nomonsters;
static dsda_key_frame_t nomo_key_frame;
static bf_target_t bf_target;
static ticcmd_t bf_result[MAX_BF_DEPTH];

const char* dsda_bf_attribute_names[dsda_bf_attribute_max] = {
  [dsda_bf_x] = "x",
  [dsda_bf_y] = "y",
  [dsda_bf_z] = "z",
  [dsda_bf_momx] = "vx",
  [dsda_bf_momy] = "vy",
  [dsda_bf_speed] = "spd",
  [dsda_bf_damage] = "dmg",
  [dsda_bf_rng] = "rng",
  [dsda_bf_arm] = "arm",
  [dsda_bf_hp] = "hp",
  [dsda_bf_ammo_0] = "am0",
  [dsda_bf_ammo_1] = "am1",
  [dsda_bf_ammo_2] = "am2",
  [dsda_bf_ammo_3] = "am3",
  [dsda_bf_ammo_4] = "am4",
  [dsda_bf_ammo_5] = "am5",
  [dsda_bf_bmapwidth] = "bmw",
};

const char* dsda_bf_misc_names[dsda_bf_misc_max] = {
  "line skip",
  "line activation",
  "have item",
};

const char* dsda_bf_operator_names[dsda_bf_operator_max] = {
  [dsda_bf_less_than] = "<",
  [dsda_bf_less_than_or_equal_to] = "<=",
  [dsda_bf_greater_than] = ">",
  [dsda_bf_greater_than_or_equal_to] = ">=",
  [dsda_bf_equal_to] = "==",
  [dsda_bf_not_equal_to] = "!="
};

const char* dsda_bf_limit_names[dsda_bf_limit_max] = {
  "acap",
  "max",
  "min",
};

const char* dsda_bf_item_names[dsda_bf_item_max] = {
  [dsda_bf_red_key_card] = "rkc",
  [dsda_bf_yellow_key_card] = "ykc",
  [dsda_bf_blue_key_card] = "bkc",
  [dsda_bf_red_skull_key] = "rsk",
  [dsda_bf_yellow_skull_key] = "ysk",
  [dsda_bf_blue_skull_key] = "bsk",

  [dsda_bf_fist] = "f",
  [dsda_bf_pistol] = "p",
  [dsda_bf_shotgun] = "sg",
  [dsda_bf_chaingun] = "cg",
  [dsda_bf_rocket_launcher] = "rl",
  [dsda_bf_plasma_gun] = "pg",
  [dsda_bf_bfg] = "bfg",
  [dsda_bf_chainsaw] = "cs",
  [dsda_bf_super_shotgun] = "ssg",
};

static dboolean fixed_point_attribute[dsda_bf_attribute_max] = {
  [dsda_bf_x] = true,
  [dsda_bf_y] = true,
  [dsda_bf_z] = true,
  [dsda_bf_momx] = true,
  [dsda_bf_momy] = true,
  [dsda_bf_speed] = true,
  [dsda_bf_damage] = true,
  [dsda_bf_rng] = false,
  [dsda_bf_arm] = false,
  [dsda_bf_hp] = false,
  [dsda_bf_ammo_0] = false,
  [dsda_bf_ammo_1] = false,
  [dsda_bf_ammo_2] = false,
  [dsda_bf_ammo_3] = false,
  [dsda_bf_ammo_4] = false,
  [dsda_bf_ammo_5] = false,
  [dsda_bf_bmapwidth] = false,
};

static dboolean dsda_AdvanceBFRange(bf_range_t* range) {
  ++range->i;

  if (range->i > range->max) {
    range->i = range->min;
    return false;
  }

  return true;
}

static dboolean dsda_AdvanceBruteForceFrame(int frame) {
  if (!dsda_AdvanceBFRange(&brute_force[frame].angleturn))
    if (!dsda_AdvanceBFRange(&brute_force[frame].sidemove))
      if (!dsda_AdvanceBFRange(&brute_force[frame].forwardmove))
        return false;

  return true;
}

static int dsda_AdvanceBruteForce(void) {
  int i;

  for (i = bf_depth - 1; i >= 0; --i)
    if (dsda_AdvanceBruteForceFrame(i))
      break;

  return i;
}

static void dsda_CopyBFCommandDepth(ticcmd_t* cmd, bf_t* bf) {
  memset(cmd, 0, sizeof(*cmd));

  cmd->angleturn = bf->angleturn.i << 8;
  cmd->forwardmove = bf->forwardmove.i;
  cmd->sidemove = bf->sidemove.i;
  cmd->buttons = bf->buttons;
}

static void dsda_CopyBFResult(bf_t* bf, int depth) {
  int i;

  for (i = 0; i < depth; ++i)
    dsda_CopyBFCommandDepth(&bf_result[i], &bf[i]);

  if (i != MAX_BF_DEPTH)
    memset(&bf_result[i], 0, sizeof(ticcmd_t) * (MAX_BF_DEPTH - i));
}

static void dsda_RestoreBFKeyFrame(int frame) {
  dsda_RestoreKeyFrame(&brute_force[frame].key_frame, true);
}

static void dsda_StoreBFKeyFrame(int frame) {
  dsda_StoreKeyFrame(&brute_force[frame].key_frame, true, false);
}

static void dsda_PrintBFProgress(void) {
  int percent;
  unsigned long long elapsed_time;

  percent = 100 * bf_volume / bf_volume_max;
  elapsed_time = dsda_ElapsedTimeMS(dsda_timer_brute_force);

  lprintf(LO_INFO, "  %lld / %lld sequences tested (%d%%) in %.2f seconds!\n",
          bf_volume, bf_volume_max, percent, (float) elapsed_time / 1000);
}

#define BF_FAILURE 0
#define BF_SUCCESS 1

static const char* bf_result_text[2] = { "FAILURE", "SUCCESS" };
static dboolean brute_force_ended;

dboolean dsda_BruteForceEnded(void) {
  return brute_force_ended;
}

static void dsda_EndBF(int result) {
  brute_force_ended = true;

  lprintf(LO_INFO, "Brute force complete (%s)!\n", bf_result_text[result]);
  dsda_PrintBFProgress();

  if (bf_nomonsters)
    dsda_RestoreKeyFrame(&nomo_key_frame, true);
  else
    dsda_RestoreBFKeyFrame(0);

  bf_mode = false;

  if (result == BF_SUCCESS)
    dsda_QueueBuildCommands(bf_result, bf_depth);
  else
    dsda_ExitSkipMode();
}

static fixed_t dsda_BFAttribute(int attribute) {
  extern int bmapwidth;

  player_t* player;

  player = &players[displayplayer];

  switch (attribute) {
    case dsda_bf_x:
      return player->mo->x;
    case dsda_bf_y:
      return player->mo->y;
    case dsda_bf_z:
      return player->mo->z;
    case dsda_bf_momx:
      return player->mo->momx;
    case dsda_bf_momy:
      return player->mo->momy;
    case dsda_bf_speed:
      return P_PlayerSpeed(player);
    case dsda_bf_damage:
      {
        extern int player_damage_last_tic;

        return player_damage_last_tic;
      }
    case dsda_bf_rng:
      return rng.rndindex;
    case dsda_bf_arm:
      return player->armorpoints[ARMOR_ARMOR];
    case dsda_bf_hp:
      return player->health;
    case dsda_bf_ammo_0:
      return player->ammo[0];
    case dsda_bf_ammo_1:
      return player->ammo[1];
    case dsda_bf_ammo_2:
      return player->ammo[2];
    case dsda_bf_ammo_3:
      return player->ammo[3];
    case dsda_bf_ammo_4:
      return player->ammo[4];
    case dsda_bf_ammo_5:
      return player->ammo[5];
    case dsda_bf_bmapwidth:
      return bmapwidth;
    default:
      return 0;
  }
}

static dboolean dsda_BFMiscConditionReached(int i) {
  player_t* player;

  player = &players[displayplayer];

  switch (bf_condition[i].attribute) {
    case dsda_bf_line_skip:
      return lines[bf_condition[i].value].player_activations == bf_condition[i].secondary_value;
    case dsda_bf_line_activation:
      return lines[bf_condition[i].value].player_activations > bf_condition[i].secondary_value;
    case dsda_bf_have_item:
      switch (bf_condition[i].value) {
        case dsda_bf_red_key_card:
          return player->cards[it_redcard];
        case dsda_bf_yellow_key_card:
          return player->cards[it_yellowcard];
        case dsda_bf_blue_key_card:
          return player->cards[it_bluecard];
        case dsda_bf_red_skull_key:
          return player->cards[it_redskull];
        case dsda_bf_yellow_skull_key:
          return player->cards[it_yellowskull];
        case dsda_bf_blue_skull_key:
          return player->cards[it_blueskull];
        case dsda_bf_fist:
          return player->weaponowned[wp_fist];
        case dsda_bf_pistol:
          return player->weaponowned[wp_pistol];
        case dsda_bf_shotgun:
          return player->weaponowned[wp_shotgun];
        case dsda_bf_chaingun:
          return player->weaponowned[wp_chaingun];
        case dsda_bf_rocket_launcher:
          return player->weaponowned[wp_missile];
        case dsda_bf_plasma_gun:
          return player->weaponowned[wp_plasma];
        case dsda_bf_bfg:
          return player->weaponowned[wp_bfg];
        case dsda_bf_chainsaw:
          return player->weaponowned[wp_chainsaw];
        case dsda_bf_super_shotgun:
          return player->weaponowned[wp_supershotgun];
        default:
          return false;
      }
    default:
      return false;
  }
}

static dboolean dsda_BFConditionReached(int i) {
  fixed_t value;

  if (bf_condition[i].operator == dsda_bf_operator_misc)
    return dsda_BFMiscConditionReached(i);

  value = dsda_BFAttribute(bf_condition[i].attribute);

  switch (bf_condition[i].operator) {
    case dsda_bf_less_than:
      return value < bf_condition[i].value;
    case dsda_bf_less_than_or_equal_to:
      return value <= bf_condition[i].value;
    case dsda_bf_greater_than:
      return value > bf_condition[i].value;
    case dsda_bf_greater_than_or_equal_to:
      return value >= bf_condition[i].value;
    case dsda_bf_equal_to:
      return value == bf_condition[i].value;
    case dsda_bf_not_equal_to:
      return value != bf_condition[i].value;
    default:
      return false;
  }
}

static void dsda_BFUpdateBestResult(fixed_t value) {
  int i;
  char str[FIXED_STRING_LENGTH];
  char cmd_str[COMMAND_MOVEMENT_STRING_LENGTH];

  bf_target.evaluated = true;
  bf_target.best_value = value;
  bf_target.best_depth = true_logictic - bf_logictic;

  for (i = 0; i < bf_target.best_depth; ++i)
    bf_target.best_bf[i] = brute_force[i];

  dsda_CopyBFResult(bf_target.best_bf, bf_target.best_depth);

  if (fixed_point_attribute[bf_target.attribute])
    dsda_FixedToString(str, value);
  else
    snprintf(str, FIXED_STRING_LENGTH, "%i", value);

  lprintf(LO_INFO, "New best: %s = %s\n", dsda_bf_attribute_names[bf_target.attribute], str);

  for (i = 0; i < bf_target.best_depth; ++i) {
    dsda_PrintCommandMovement(cmd_str, &bf_result[i]);
    lprintf(LO_INFO, "    %s\n", cmd_str);
  }

  lprintf(LO_INFO, "\n");
}

static dboolean dsda_BFNewBestResult(fixed_t value) {
  if (!bf_target.evaluated)
    return true;

  switch (bf_target.limit) {
    case dsda_bf_acap:
      return abs(value - bf_target.value) < abs(bf_target.best_value - bf_target.value);
    case dsda_bf_max:
      return value > bf_target.best_value;
    case dsda_bf_min:
      return value < bf_target.best_value;
    default:
      return false;
  }
}

static void dsda_BFEvaluateTarget(void) {
  fixed_t value;

  value = dsda_BFAttribute(bf_target.attribute);

  if (dsda_BFNewBestResult(value))
    dsda_BFUpdateBestResult(value);
}

static dboolean dsda_BFConditionsReached(void) {
  int i, reached;

  reached = 0;
  for (i = 0; i < bf_condition_count; ++i)
    reached += dsda_BFConditionReached(i);

  if (reached == bf_condition_count)
    if (bf_target.enabled) {
      dsda_BFEvaluateTarget();

      return false;
    }

  return reached == bf_condition_count;
}

dboolean dsda_BruteForce(void) {
  return bf_mode;
}

void dsda_ResetBruteForceConditions(void) {
  bf_condition_count = 0;
  memset(&bf_target, 0, sizeof(bf_target));
}

void dsda_AddMiscBruteForceCondition(dsda_bf_attribute_t attribute, fixed_t value) {
  if (bf_condition_count == MAX_BF_CONDITIONS)
    return;

  bf_condition[bf_condition_count].attribute = attribute;
  bf_condition[bf_condition_count].operator = dsda_bf_operator_misc;
  bf_condition[bf_condition_count].value = value;

  switch (attribute) {
    case dsda_bf_line_skip:
    case dsda_bf_line_activation:
      bf_condition[bf_condition_count].secondary_value = lines[value].player_activations;
      break;
    default:
      break;
  }

  ++bf_condition_count;

  lprintf(LO_INFO, "Added brute force condition: %s %d\n",
                   dsda_bf_misc_names[attribute],
                   value);
}

void dsda_AddBruteForceCondition(dsda_bf_attribute_t attribute,
                                 dsda_bf_operator_t operator, fixed_t value) {
  if (bf_condition_count == MAX_BF_CONDITIONS)
    return;

  bf_condition[bf_condition_count].attribute = attribute;
  bf_condition[bf_condition_count].operator = operator;
  bf_condition[bf_condition_count].value = value;

  if (fixed_point_attribute[attribute])
    bf_condition[bf_condition_count].value <<= FRACBITS;

  ++bf_condition_count;

  lprintf(LO_INFO, "Added brute force condition: %s %s %d\n",
                   dsda_bf_attribute_names[attribute],
                   dsda_bf_operator_names[operator],
                   value);
}

void dsda_SetBruteForceTarget(dsda_bf_attribute_t attribute,
                              dsda_bf_limit_t limit, fixed_t value, dboolean has_value) {
  bf_target.attribute = attribute;
  bf_target.limit = limit;
  bf_target.value = value;
  bf_target.best_value = dsda_BFAttribute(attribute);
  bf_target.enabled = true;

  if (has_value)
    lprintf(LO_INFO, "Set brute force target: %s %s %d\n",
                    dsda_bf_attribute_names[attribute],
                    dsda_bf_limit_names[limit],
                    value);
  else
    lprintf(LO_INFO, "Set brute force target: %s %s\n",
                    dsda_bf_attribute_names[attribute],
                    dsda_bf_limit_names[limit]);
}

static void dsda_SortIntPair(int* a, int* b) {
  if (*a > *b) {
    int temp;

    temp = *a;
    *a = *b;
    *b = temp;
  }
}

int dsda_KeepBruteForceFrame(int i) {
  ticcmd_t cmd;

  if (!dsda_CopyPendingCmd(&cmd, i))
    return false;

  brute_force[i].forwardmove.min = cmd.forwardmove;
  brute_force[i].forwardmove.max = cmd.forwardmove;

  brute_force[i].sidemove.min = cmd.sidemove;
  brute_force[i].sidemove.max = cmd.sidemove;

  brute_force[i].angleturn.min = cmd.angleturn >> 8;
  brute_force[i].angleturn.max = cmd.angleturn >> 8;

  brute_force[i].buttons = cmd.buttons;

  return true;
}

int dsda_AddBruteForceFrame(int i,
                            int forwardmove_min, int forwardmove_max,
                            int sidemove_min, int sidemove_max,
                            int angleturn_min, int angleturn_max,
                            byte buttons) {
  if (i < 0 || i >= MAX_BF_DEPTH)
    return false;

  dsda_SortIntPair(&forwardmove_min, &forwardmove_max);
  dsda_SortIntPair(&sidemove_min, &sidemove_max);
  dsda_SortIntPair(&angleturn_min, &angleturn_max);

  brute_force[i].forwardmove.min = forwardmove_min;
  brute_force[i].forwardmove.max = forwardmove_max;

  brute_force[i].sidemove.min = sidemove_min;
  brute_force[i].sidemove.max = sidemove_max;

  brute_force[i].angleturn.min = angleturn_min;
  brute_force[i].angleturn.max = angleturn_max;

  brute_force[i].buttons = buttons;

  return true;
}

void dsda_BruteForceWithoutMonsters(void) {
  bf_nomonsters = true;
}

void dsda_BruteForceWithMonsters(void) {
  bf_nomonsters = false;
}

dboolean dsda_StartBruteForce(int depth) {
  int i;

  if (!dsda_BuildMode()) {
    lprintf(LO_WARN, "You cannot start brute force outside of build mode!\n");
    return false;
  }

  if (depth <= 0 || depth > MAX_BF_DEPTH)
    return false;

  dsda_TrackFeature(uf_bruteforce);

  lprintf(LO_INFO, "Brute force starting:\n");

  bf_depth = depth;
  bf_logictic = true_logictic;
  bf_volume = 0;
  bf_volume_max = 1;

  for (i = 0; i < bf_depth; ++i) {
    lprintf(LO_INFO, "  %d: F %d:%d S %d:%d T %d:%d B %d\n", i,
            brute_force[i].forwardmove.min, brute_force[i].forwardmove.max,
            brute_force[i].sidemove.min, brute_force[i].sidemove.max,
            brute_force[i].angleturn.min, brute_force[i].angleturn.max,
            brute_force[i].buttons);

    bf_volume_max *= (brute_force[i].forwardmove.max - brute_force[i].forwardmove.min + 1) *
                     (brute_force[i].sidemove.max - brute_force[i].sidemove.min + 1) *
                     (brute_force[i].angleturn.max - brute_force[i].angleturn.min + 1);

    brute_force[i].forwardmove.i = brute_force[i].forwardmove.min;
    brute_force[i].sidemove.i = brute_force[i].sidemove.min;
    brute_force[i].angleturn.i = brute_force[i].angleturn.min;
  }

  lprintf(LO_INFO, "Testing %lld sequences with depth %d\n\n", bf_volume_max, bf_depth);

  bf_mode = true;

  if (bf_nomonsters) {
    lprintf(LO_INFO, "Warning: ignoring monsters! The result may desync with monsters!\n");
    dsda_StoreKeyFrame(&nomo_key_frame, true, false);
    P_RemoveMonsters();
  }

  dsda_EnterSkipMode();

  dsda_StartTimer(dsda_timer_brute_force);

  return true;
}

void dsda_UpdateBruteForce(void) {
  int frame;

  frame = true_logictic - bf_logictic;

  if (frame == bf_depth) {
    if (bf_volume % 10000 == 0)
      dsda_PrintBFProgress();

    frame = dsda_AdvanceBruteForce();

    if (frame >= 0)
      dsda_RestoreBFKeyFrame(frame);
  }
  else
    dsda_StoreBFKeyFrame(frame);
}

void dsda_EvaluateBruteForce(void) {
  if (true_logictic - bf_logictic != bf_depth)
    return;

  ++bf_volume;

  if (dsda_BFConditionsReached()) {
    dsda_CopyBFResult(brute_force, bf_depth);
    dsda_EndBF(BF_SUCCESS);
  }
  else if (bf_volume >= bf_volume_max) {
    if (bf_target.enabled && bf_target.evaluated)
      dsda_EndBF(BF_SUCCESS);
    else
      dsda_EndBF(BF_FAILURE);
  }
}

void dsda_CopyBruteForceCommand(ticcmd_t* cmd) {
  int depth;

  depth = true_logictic - bf_logictic;

  if (depth >= bf_depth) {
    memset(cmd, 0, sizeof(*cmd));

    return;
  }

  dsda_CopyBFCommandDepth(cmd, &brute_force[depth]);
}
