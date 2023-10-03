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
//	DSDA Build Mode
//

#include "doomstat.h"
#include "g_game.h"

#include "dsda/brute_force.h"
#include "dsda/demo.h"
#include "dsda/exhud.h"
#include "dsda/features.h"
#include "dsda/input.h"
#include "dsda/key_frame.h"
#include "dsda/pause.h"
#include "dsda/playback.h"
#include "dsda/settings.h"
#include "dsda/skip.h"

#include "build.h"

typedef struct {
  ticcmd_t* cmds;
  int depth;
  int original_depth;
} build_cmd_queue_t;

static dboolean allow_turbo;
static dboolean build_mode;
static dboolean advance_frame;
static ticcmd_t build_cmd;
static ticcmd_t overwritten_cmd;
static int overwritten_logictic;
static int build_cmd_tic = -1;
static dboolean replace_source = true;
static build_cmd_queue_t cmd_queue;

static signed char forward50(void) {
  return pclass[players[consoleplayer].pclass].forwardmove[1];
}

#if 0
static signed char strafe40(void) {
  return pclass[players[consoleplayer].pclass].sidemove[1];
}
#endif

static signed char strafe50(void) {
  return forward50();
}

static signed short shortTic(void) {
  return (1 << 8);
}

static signed char maxForward(void) {
  return allow_turbo ? 127 : forward50();
}

static signed char minBackward(void) {
  return allow_turbo ? -127 : -forward50();
}

static signed char maxStrafeRight(void) {
  return allow_turbo ? 127 : strafe50();
}

static signed char minStrafeLeft(void) {
  return allow_turbo ? -128 : -strafe50();
}

void dsda_ChangeBuildCommand(void) {
  if (demoplayback)
    dsda_JoinDemo(NULL);

  replace_source = true;
  build_cmd_tic = true_logictic - 1;
  dsda_JumpToLogicTicFrom(true_logictic, true_logictic - 1);
}

dboolean dsda_BuildMF(int x) {
  if (x < 0 || x > 127)
    return false;

  build_cmd.forwardmove = x;

  dsda_ChangeBuildCommand();

  return true;
}

dboolean dsda_BuildMB(int x) {
  if (x < 0 || x > 127)
    return false;

  build_cmd.forwardmove = -x;

  dsda_ChangeBuildCommand();

  return true;
}

dboolean dsda_BuildSR(int x) {
  if (x < 0 || x > 127)
    return false;

  build_cmd.sidemove = x;

  dsda_ChangeBuildCommand();

  return true;
}

dboolean dsda_BuildSL(int x) {
  if (x < 0 || x > 128)
    return false;

  build_cmd.sidemove = -x;

  dsda_ChangeBuildCommand();

  return true;
}

dboolean dsda_BuildTR(int x) {
  if (x < 0 || x > 128)
    return false;

  build_cmd.angleturn = (-x << 8);

  dsda_ChangeBuildCommand();

  return true;
}

dboolean dsda_BuildTL(int x) {
  if (x < 0 || x > 127)
    return false;

  build_cmd.angleturn = (x << 8);

  dsda_ChangeBuildCommand();

  return true;
}

dboolean dsda_BuildFU(int x) {
  if (x < 0 || x > 7)
    return false;

  build_cmd.lookfly &= 0x0f;
  build_cmd.lookfly |= (x << 4);

  dsda_ChangeBuildCommand();

  return true;
}

dboolean dsda_BuildFD(int x) {
  if (x < 0 || x > 7)
    return false;

  if (x)
    x = 16 - x;

  build_cmd.lookfly &= 0x0f;
  build_cmd.lookfly |= (x << 4);

  dsda_ChangeBuildCommand();

  return true;
}

dboolean dsda_BuildFC(void) {
  build_cmd.lookfly &= 0x0f;
  build_cmd.lookfly |= 0x80;

  dsda_ChangeBuildCommand();

  return true;
}

dboolean dsda_BuildLU(int x) {
  if (x < 0 || x > 7)
    return false;

  build_cmd.lookfly &= 0xf0;
  build_cmd.lookfly |= x;

  dsda_ChangeBuildCommand();

  return true;
}

dboolean dsda_BuildLD(int x) {
  if (x < 0 || x > 7)
    return false;

  if (x)
    x = 16 - x;

  build_cmd.lookfly &= 0xf0;
  build_cmd.lookfly |= x;

  dsda_ChangeBuildCommand();

  return true;
}

dboolean dsda_BuildLC(void) {
  build_cmd.lookfly &= 0xf0;
  build_cmd.lookfly |= 0x08;

  dsda_ChangeBuildCommand();

  return true;
}

dboolean dsda_BuildUA(int x) {
  if (x < 0 || x > (heretic ? 10 : 15))
    return false;

  build_cmd.arti = x;

  dsda_ChangeBuildCommand();

  return true;
}

static void buildForward(void) {
  if (allow_turbo) {
    if (build_cmd.forwardmove == 127)
      build_cmd.forwardmove = 0;
    else if (build_cmd.forwardmove == forward50())
      build_cmd.forwardmove = 127;
    else
      build_cmd.forwardmove = forward50();
  }
  else {
    if (build_cmd.forwardmove == forward50())
      build_cmd.forwardmove = 0;
    else
      build_cmd.forwardmove = forward50();
  }

  dsda_ChangeBuildCommand();
}

static void buildBackward(void) {
  if (allow_turbo) {
    if (build_cmd.forwardmove == -127)
      build_cmd.forwardmove = 0;
    else if (build_cmd.forwardmove == -forward50())
      build_cmd.forwardmove = -127;
    else
      build_cmd.forwardmove = -forward50();
  }
  else {
    if (build_cmd.forwardmove == -forward50())
      build_cmd.forwardmove = 0;
    else
      build_cmd.forwardmove = -forward50();
  }

  dsda_ChangeBuildCommand();
}

static void buildFineForward(void) {
  if (build_cmd.forwardmove < maxForward())
    ++build_cmd.forwardmove;

  dsda_ChangeBuildCommand();
}

static void buildFineBackward(void) {
  if (build_cmd.forwardmove > minBackward())
    --build_cmd.forwardmove;

  dsda_ChangeBuildCommand();
}

static void buildStrafeRight(void) {
  if (allow_turbo) {
    if (build_cmd.sidemove == 127)
      build_cmd.sidemove = 0;
    else if (build_cmd.sidemove == strafe50())
      build_cmd.sidemove = 127;
    else
      build_cmd.sidemove = strafe50();
  }
  else {
    if (build_cmd.sidemove == strafe50())
      build_cmd.sidemove = 0;
    else
      build_cmd.sidemove = strafe50();
  }

  dsda_ChangeBuildCommand();
}

static void buildStrafeLeft(void) {
  if (allow_turbo) {
    if (build_cmd.sidemove == -128)
      build_cmd.sidemove = 0;
    else if (build_cmd.sidemove == -strafe50())
      build_cmd.sidemove = -128;
    else
      build_cmd.sidemove = -strafe50();
  }
  else {
    if (build_cmd.sidemove == -strafe50())
      build_cmd.sidemove = 0;
    else
      build_cmd.sidemove = -strafe50();
  }

  dsda_ChangeBuildCommand();
}

static void buildFineStrafeRight(void) {
  if (build_cmd.sidemove < maxStrafeRight())
    ++build_cmd.sidemove;

  dsda_ChangeBuildCommand();
}

static void buildFineStrafeLeft(void) {
  if (build_cmd.sidemove > minStrafeLeft())
    --build_cmd.sidemove;

  dsda_ChangeBuildCommand();
}

static void buildTurnRight(void) {
  build_cmd.angleturn -= shortTic();

  dsda_ChangeBuildCommand();
}

static void buildTurnLeft(void) {
  build_cmd.angleturn += shortTic();

  dsda_ChangeBuildCommand();
}

static void buildUse(void) {
  build_cmd.buttons ^= BT_USE;

  dsda_ChangeBuildCommand();
}

static void buildFire(void) {
  build_cmd.buttons ^= BT_ATTACK;

  dsda_ChangeBuildCommand();
}

static void buildWeapon(int weapon) {
  int cmdweapon;

  cmdweapon = weapon << BT_WEAPONSHIFT;

  if (build_cmd.buttons & BT_CHANGE && (build_cmd.buttons & BT_WEAPONMASK) == cmdweapon)
    build_cmd.buttons &= ~BT_CHANGE;
  else
    build_cmd.buttons |= BT_CHANGE;

  build_cmd.buttons &= ~BT_WEAPONMASK;
  if (build_cmd.buttons & BT_CHANGE)
    build_cmd.buttons |= cmdweapon;

  dsda_ChangeBuildCommand();
}

static void resetCmd(void) {
  memset(&build_cmd, 0, sizeof(build_cmd));
}

dboolean dsda_AllowBuilding(void) {
  return !dsda_StrictMode();
}

dboolean dsda_BuildMode(void) {
  return build_mode;
}

void dsda_QueueBuildCommands(ticcmd_t* cmds, int depth) {
  cmd_queue.original_depth = depth;
  cmd_queue.depth = depth;

  if (cmd_queue.cmds)
    Z_Free(cmd_queue.cmds);

  cmd_queue.cmds = Z_Malloc(depth * sizeof(*cmds));
  memcpy(cmd_queue.cmds, cmds, depth * sizeof(*cmds));
}

static void dsda_PopCommandQueue(ticcmd_t* cmd) {
  *cmd = cmd_queue.cmds[cmd_queue.original_depth - cmd_queue.depth];
  --cmd_queue.depth;

  if (!cmd_queue.depth)
    dsda_ExitSkipMode();
}

dboolean dsda_BuildPlayback(void) {
  return !replace_source;
}

void dsda_CopyBuildCmd(ticcmd_t* cmd) {
  *cmd = build_cmd;
}

void dsda_ReadBuildCmd(ticcmd_t* cmd) {
  if (cmd_queue.depth)
    dsda_PopCommandQueue(cmd);
  else if (dsda_BruteForce())
    dsda_CopyBruteForceCommand(cmd);
  else if (true_logictic == build_cmd_tic) {
    *cmd = build_cmd;
    build_cmd_tic = -1;
  }
  else
    dsda_CopyPendingCmd(cmd, 0);

  dsda_JoinDemoCmd(cmd);
}

void dsda_EnterBuildMode(void) {
  dsda_TrackFeature(uf_build);

  if (!demorecording) {
    if (!build_mode)
      dsda_StoreTempKeyFrame();

    advance_frame = true;
  }

  if (!true_logictic)
    advance_frame = true;

  build_mode = true;
  dsda_ApplyPauseMode(PAUSE_BUILDMODE);

  dsda_RefreshExHudCommandDisplay();
}

void dsda_ExitBuildMode(void) {
  build_mode = false;
  dsda_RemovePauseMode(PAUSE_BUILDMODE);

  dsda_RefreshExHudCommandDisplay();
}

void dsda_RefreshBuildMode(void) {
  if (demoplayback)
    replace_source = false;

  if (!dsda_SkipMode() &&
      overwritten_logictic != true_logictic - 1 &&
      build_cmd_tic == -1 &&
      true_logictic > 0) {
    dsda_CopyPriorCmd(&overwritten_cmd, 1);
    build_cmd = overwritten_cmd;
    overwritten_logictic = true_logictic - 1;
    replace_source = false;
  }
}

dboolean dsda_BuildResponder(event_t* ev) {
  if (!dsda_AllowBuilding())
    return false;

  if (dsda_InputActivated(dsda_input_build)) {
    if (dsda_BuildMode())
      dsda_ExitBuildMode();
    else
      dsda_EnterBuildMode();

    return true;
  }

  if (!build_mode || menuactive)
    return false;

  if (dsda_InputActivated(dsda_input_build_source)) {
    replace_source = !replace_source;

    if (!replace_source) {
      build_cmd = overwritten_cmd;

      dsda_ChangeBuildCommand();
      replace_source = false;
    }

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_advance_frame)) {
    advance_frame = true;
    build_cmd_tic = true_logictic;

    build_cmd.angleturn = 0;
    build_cmd.arti = 0;
    build_cmd.buttons &= ~BT_USE;
    if (build_cmd.buttons & BT_CHANGE)
      build_cmd.buttons &= ~(BT_CHANGE | BT_WEAPONMASK);

    if (dsda_CopyPendingCmd(&overwritten_cmd, 0)) {
       if (!replace_source)
          build_cmd = overwritten_cmd;
    }
    else {
      overwritten_cmd = build_cmd;
      replace_source = true;
    }

    overwritten_logictic = true_logictic;

    if (!demorecording)
      dsda_StoreTempKeyFrame();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_reverse_frame)) {
    if (!demorecording) {
      doom_printf("Cannot reverse outside demo");
      return true;
    }

    if (true_logictic > 1) {
      dsda_CopyPriorCmd(&build_cmd, 2);
      overwritten_cmd = build_cmd;
      overwritten_logictic = true_logictic - 2;
      replace_source = false;

      dsda_JumpToLogicTic(true_logictic - 1);
    }

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_reset_command)) {
    resetCmd();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_forward)) {
    buildForward();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_backward)) {
    buildBackward();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_fine_forward)) {
    buildFineForward();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_fine_backward)) {
    buildFineBackward();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_strafe_right)) {
    buildStrafeRight();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_strafe_left)) {
    buildStrafeLeft();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_fine_strafe_right)) {
    buildFineStrafeRight();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_fine_strafe_left)) {
    buildFineStrafeLeft();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_turn_right)) {
    buildTurnRight();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_turn_left)) {
    buildTurnLeft();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_use)) {
    buildUse();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_fire)) {
    buildFire();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon1)) {
    buildWeapon(0);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon2)) {
    buildWeapon(1);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon3)) {
    buildWeapon(2);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon4)) {
    buildWeapon(3);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon5)) {
    buildWeapon(4);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon6)) {
    buildWeapon(5);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon7)) {
    buildWeapon(6);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon8)) {
    buildWeapon(7);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon9)) {
    if (!demo_compatibility && gamemode == commercial)
      buildWeapon(8);

    return true;
  }

  if (dsda_InputActivated(dsda_input_join_demo))
    dsda_JoinDemo(NULL);

  return false;
}

void dsda_ToggleBuildTurbo(void) {
  allow_turbo = !allow_turbo;

  if (!allow_turbo) {
    if (build_cmd.forwardmove > maxForward())
      build_cmd.forwardmove = maxForward();
    else if (build_cmd.forwardmove < minBackward())
      build_cmd.forwardmove = minBackward();

    if (build_cmd.sidemove > maxStrafeRight())
      build_cmd.sidemove = maxStrafeRight();
    else if (build_cmd.sidemove < minStrafeLeft())
      build_cmd.sidemove = minStrafeLeft();
  }
}

dboolean dsda_AdvanceFrame(void) {
  dboolean result;

  if (dsda_SkipMode())
    advance_frame = true;

  result = advance_frame;
  advance_frame = false;

  return result;
}
