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
//	DSDA Skip Mode
//

#include "d_main.h"
#include "doomstat.h"
#include "e6y.h"
#include "i_main.h"
#include "i_sound.h"
#include "s_sound.h"
#include "smooth.h"
#include "v_video.h"
#include "stricmp.h"
//#include "gl_struct.h"

#include "dsda/args.h"
#include "dsda/build.h"
#include "dsda/features.h"
#include "dsda/pause.h"
#include "dsda/playback.h"

#include "skip.h"

static dboolean skip_mode;

static int demo_skiptics;
static dboolean skip_until_next_map;
static dboolean skip_until_end_of_map;
static dboolean skip_until_logictic;
static dboolean demo_warp_reached;
static int skip_until_map;
static int skip_until_episode;

dboolean dsda_SkipMode(void) {
  return skip_mode;
}

static void dsda_CacheSkipSetting(dboolean* old, dboolean* current) {
  *old = *current;
  *current = true;
}

static dboolean old_fastdemo, old_nodrawers, old_nosfxparm, old_nomusicparm;

static void dsda_ApplySkipSettings(void) {
  dsda_CacheSkipSetting(&old_fastdemo, &fastdemo);
  dsda_CacheSkipSetting(&old_nodrawers, &nodrawers);
  dsda_CacheSkipSetting(&old_nosfxparm, &nosfxparm);
  dsda_CacheSkipSetting(&old_nomusicparm, &nomusicparm);
}

static void dsda_ResetSkipSettings(void) {
  fastdemo = old_fastdemo;
  nodrawers = old_nodrawers;
  nosfxparm = old_nosfxparm;
  nomusicparm = old_nomusicparm;
}

void dsda_EnterSkipMode(void) {
  extern void M_ClearMenus(void);

  dsda_TrackFeature(uf_skip);

  skip_mode = true;

  M_ClearMenus();
  dsda_ApplySkipSettings();
  dsda_ResetPauseMode();
  S_StopMusic();
  I_Init2();

  if (dsda_BuildMode())
    dsda_ApplyPauseMode(PAUSE_BUILDMODE);
}

void dsda_ExitSkipMode(void) {
  skip_mode = false;

  dsda_ResetSkipSettings();

  skip_until_next_map = false;
  skip_until_end_of_map = false;
  skip_until_logictic = 0;
  skip_until_map = -1;
  skip_until_episode = -1;
  demo_warp_reached = false;
  demo_skiptics = 0;

  I_Init2();
  I_InitSound();
  S_Init();
  S_RestartMusic();

#if 0
  if (V_IsOpenGLMode())
    gld_PreprocessLevel();
#endif
}

void dsda_ToggleSkipMode(void) {
  dsda_SkipMode() ? dsda_ExitSkipMode() : dsda_EnterSkipMode();
}

void dsda_SkipToNextMap(void) {
  skip_until_next_map = true;
  dsda_EnterSkipMode();
}

void dsda_SkipToEndOfMap(void) {
  skip_until_end_of_map = true;
  dsda_EnterSkipMode();
}

void dsda_SkipToLogicTic(int tic) {
  skip_until_logictic = tic;
  dsda_EnterSkipMode();
}

void dsda_EvaluateSkipModeGTicker(void) {
  if (dsda_SkipMode() && skip_until_logictic && skip_until_logictic <= true_logictic)
    dsda_ExitSkipMode();
}

void dsda_EvaluateSkipModeInitNew(void) {
  if (dsda_SkipMode() && skip_until_map == gamemap && skip_until_episode == gameepisode)
    demo_warp_reached = true;
}

void dsda_EvaluateSkipModeBuildTiccmd(void) {
  if (dsda_SkipMode() && gametic > 0)
    if (
      (
        !skip_until_logictic &&
        skip_until_map == -1 &&
        demo_skiptics &&
        (
          demo_skiptics > 0 ?
            gametic > demo_skiptics :
            dsda_PlaybackTics() - demo_skiptics >= demo_tics_count
        )
      ) ||
      (
        demo_warp_reached && gametic - levelstarttic > demo_skiptics
      )
    )
      dsda_ExitSkipMode();
}

void dsda_EvaluateSkipModeDoCompleted(void) {
  if (dsda_SkipMode() && (skip_until_end_of_map || demo_warp_reached))
    dsda_ExitSkipMode();
}

void dsda_EvaluateSkipModeDoTeleportNewMap(void) {
  if (dsda_SkipMode()) {
    static int firstmap = 1;

    demo_warp_reached = skip_until_next_map ||
      (
        gamemode == commercial ?
          (skip_until_map == gamemap) :
          (skip_until_episode == gameepisode && skip_until_map == gamemap)
      );

    if (demo_warp_reached && demo_skiptics == 0 && !firstmap)
      dsda_ExitSkipMode();

    firstmap = 0;
  }
}

void dsda_EvaluateSkipModeDoWorldDone(void) {
  if (dsda_SkipMode()) {
    static int firstmap = 1;

    demo_warp_reached = skip_until_next_map ||
      (
        gamemode == commercial ?
          (skip_until_map == gamemap) :
          (skip_until_episode == gameepisode && skip_until_map == gamemap)
      );

    if (demo_warp_reached && demo_skiptics == 0 && !firstmap)
      dsda_ExitSkipMode();

    firstmap = 0;
  }
}

void dsda_EvaluateSkipModeCheckDemoStatus(void) {
  if (dsda_SkipMode() && (skip_until_end_of_map || skip_until_next_map))
    dsda_ExitSkipMode();
}

void dsda_HandleSkip(void) {
  extern int warpmap;
  extern int warpepisode;

  dsda_arg_t* arg;

  arg = dsda_Arg(dsda_arg_skipsec);
  if (arg->found) {
    float min, sec;

    if (sscanf(arg->value.v_string, "%f:%f", &min, &sec) == 2)
      demo_skiptics = (int) ((60 * min + sec) * TICRATE);
    else if (sscanf(arg->value.v_string, "%f", &sec) == 1)
      demo_skiptics = (int) (sec * TICRATE);
  }

  arg = dsda_Arg(dsda_arg_skiptic);
  if (arg->found)
    demo_skiptics = arg->value.v_int;

  if (dsda_PlaybackName() && (warpmap != -1 || demo_skiptics)) {
    skip_until_map = warpmap;
    skip_until_episode = warpepisode;

    dsda_EnterSkipMode();
  }
}
