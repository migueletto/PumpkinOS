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
//	DSDA Playback
//

#include "doomstat.h"
#include "g_game.h"
#include "i_system.h"
#include "p_saveg.h"
#include "w_wad.h"

#include "dsda/args.h"
#include "dsda/demo.h"
#include "dsda/exdemo.h"
#include "dsda/input.h"
#include "dsda/key_frame.h"
#include "dsda/skip.h"

#include "playback.h"

static const byte* playback_origin_p;
static const byte* playback_p;
static int playback_length;
static int playback_behaviour;
static int playback_tics;

static dsda_arg_t* playdemo_arg;
static dsda_arg_t* fastdemo_arg;
static dsda_arg_t* timedemo_arg;
static dsda_arg_t* recordfromto_arg;
static char* playback_name;
static char* playback_filename;

dboolean demoplayback;
dboolean userdemo;

void dsda_RestartPlayback(void) {
  G_StartDemoPlayback(playback_origin_p, playback_length, playback_behaviour);
}

dboolean dsda_JumpToLogicTic(int tic) {
  if (tic < 0)
    return false;

  if (tic > true_logictic)
    dsda_SkipToLogicTic(tic);
  else if (tic < true_logictic) {
    if (!dsda_RestoreClosestKeyFrame(tic))
      return false;

    if (tic != true_logictic)
      dsda_SkipToLogicTic(tic);
  }

  return true;
}

dboolean dsda_JumpToLogicTicFrom(int tic, int from_tic) {
  if (tic < 0 || tic > true_logictic)
    return false;

  if (!dsda_RestoreClosestKeyFrame(from_tic))
    return false;

  if (tic != true_logictic)
    dsda_SkipToLogicTic(tic);

  return true;
}

const char* dsda_PlaybackName(void) {
  return playback_name;
}

void dsda_ExecutePlaybackOptions(void) {
  if (playdemo_arg)
  {
    G_DeferedPlayDemo(playback_name);
    userdemo = true;
  }
  else if (fastdemo_arg) {
    G_DeferedPlayDemo(playback_name);
    fastdemo = true;
    timingdemo = true;
    userdemo = true;
  }
  else if (timedemo_arg)
  {
    G_DeferedPlayDemo(playback_name);
    singletics = true;
    timingdemo = true;
    userdemo = true;
  }
  else if (recordfromto_arg) {
    userdemo = true;
    G_ContinueDemo(playback_name);
  }
}

static void dsda_UpdatePlaybackName(const char* name) {
  if (playback_name)
    Z_Free(playback_name);

  if (playback_filename)
    Z_Free(playback_filename);

  playback_name = Z_Strdup(name);
  playback_filename = I_RequireFile(playback_name, ".lmp");
}

const char* dsda_ParsePlaybackOptions(void) {
  dsda_arg_t* arg;

  arg = dsda_Arg(dsda_arg_playdemo);
  if (arg->found) {
    playdemo_arg = arg;
    dsda_UpdatePlaybackName(arg->value.v_string);
    return playback_filename;
  }

  arg = dsda_Arg(dsda_arg_fastdemo);
  if (arg->found) {
    fastdemo_arg = arg;
    fastdemo = true;
    dsda_UpdatePlaybackName(arg->value.v_string);
    return playback_filename;
  }

  arg = dsda_Arg(dsda_arg_timedemo);
  if (arg->found) {
    timedemo_arg = arg;
    dsda_UpdatePlaybackName(arg->value.v_string);
    return playback_filename;
  }

  arg = dsda_Arg(dsda_arg_recordfromto);
  if (arg->found) {
    recordfromto_arg = arg;
    dsda_SetDemoBaseName(arg->value.v_string_array[1]);
    dsda_UpdatePlaybackName(arg->value.v_string_array[0]);
    return playback_filename;
  }

  return NULL;
}

void dsda_InitDemoPlayback(void) {
  demoplayback = true;
}

void dsda_AttachPlaybackStream(const byte* demo_p, int length, int behaviour) {
  playback_origin_p = demo_p;
  playback_p = demo_p;
  playback_length = length;
  playback_behaviour = behaviour;
  playback_tics = 0;
}

int dsda_PlaybackTics(void) {
  return playback_tics;
}

void dsda_StorePlaybackPosition(void) {
  P_SAVE_X(playback_tics);
  P_SAVE_X(playback_p);
}

void dsda_RestorePlaybackPosition(void) {
  P_LOAD_X(playback_tics);
  P_LOAD_X(playback_p);
}

void dsda_ClearPlaybackStream(void) {
  playback_origin_p = NULL;
  playback_p = NULL;
  playback_length = 0;
  playback_behaviour = 0;
  playback_tics = 0;

  demoplayback = false;
  userdemo = false;
}

static dboolean dsda_EndOfPlaybackStream(void) {
  return *playback_p == DEMOMARKER ||
         playback_p + dsda_BytesPerTic() > playback_origin_p + playback_length;
}

void dsda_JoinDemo(ticcmd_t* cmd) {
  if (!demoplayback)
    return;

  if (dsda_SkipMode())
    dsda_ExitSkipMode();

  if (demorecording)
    dsda_WriteQueueToDemo(playback_p, playback_length - (playback_p - playback_origin_p));

  dsda_ClearPlaybackStream();

  if (cmd)
    dsda_JoinDemoCmd(cmd);
  else
    dsda_QueueJoin();

  dsda_MergeExDemoFeatures();
}

void dsda_TryPlaybackOneTick(ticcmd_t* cmd) {
  dboolean ended = false;

  if (!playback_p)
    return;

  if (dsda_EndOfPlaybackStream())
    ended = true;
  else {
    G_ReadOneTick(cmd, &playback_p);

    ++playback_tics;
  }

  if (ended) {
    if (playback_behaviour & PLAYBACK_JOIN_ON_END)
      dsda_JoinDemo(cmd);
    else
      G_CheckDemoStatus();
  }
  else if (dsda_InputActive(dsda_input_join_demo) || dsda_InputJoyBActive(dsda_input_use))
    dsda_JoinDemo(cmd);
}
