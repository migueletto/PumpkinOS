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
//	DSDA Settings
//

#include "doomstat.h"
#include "m_menu.h"
#include "e6y.h"
#include "r_things.h"
#include "w_wad.h"
#include "g_game.h"
//#include "gl_struct.h"
#include "lprintf.h"
#include "i_main.h"

#include "dsda/args.h"
#include "dsda/build.h"
#include "dsda/configuration.h"
#include "dsda/exhud.h"
#include "dsda/features.h"
#include "dsda/key_frame.h"
#include "dsda/map_format.h"

#include "settings.h"

int dsda_tas;
int dsda_skip_next_wipe;

void dsda_InitSettings(void) {
  void G_UpdateMouseSensitivity(void);
  void dsda_InitQuickstartCache(void);
  void dsda_InitParallelSFXFilter(void);
  //void gld_ResetAutomapTransparency(void);

  dsda_UpdateStrictMode();
  G_UpdateMouseSensitivity();
  dsda_InitQuickstartCache();
  dsda_InitParallelSFXFilter();
  //gld_ResetAutomapTransparency();
}

static int dsda_WadCompatibilityLevel(void) {
  static int complvl = -1;
  static int last_numwadfiles = -1;

  // This might be called before all wads are loaded
  if (numwadfiles != last_numwadfiles) {
    int num;

    last_numwadfiles = numwadfiles;
    num = W_CheckNumForName("COMPLVL");

    if (num != LUMP_NOT_FOUND) {
      int length;
      const char* data;

      length = W_LumpLength(num);
      data = W_LumpByNum(num);

      if (length == 7 && !strncasecmp("vanilla", data, 7)) {
        if (gamemode == commercial) {
          if (gamemission == pack_plut || gamemission == pack_tnt)
            complvl = 4;
          else
            complvl = 2;
        }
        else
          complvl = 3;
      }
      else if (length == 4 && !strncasecmp("boom", data, 4))
        complvl = 9;
      else if (length == 3 && !strncasecmp("mbf", data, 3))
        complvl = 11;
      else if (length == 5 && !strncasecmp("mbf21", data, 5))
        complvl = 21;

      lprintf(LO_INFO, "Detected COMPLVL lump: %i\n", complvl);
    }
  }

  return complvl;
}

int dsda_CompatibilityLevel(void) {
  int level;
  dsda_arg_t* complevel_arg;

  if (raven) return doom_12_compatibility;

  if (map_format.zdoom) return mbf21_compatibility;

  complevel_arg = dsda_Arg(dsda_arg_complevel);

  if (complevel_arg->count)
    return complevel_arg->value.v_int;

  if (!demoplayback) {
    level = dsda_WadCompatibilityLevel();

    if (level >= 0)
      return level;
  }

  return UNSPECIFIED_COMPLEVEL;
}

void dsda_SetTas(void) {
  dsda_tas = true;
}

dboolean dsda_ViewBob(void) {
  return dsda_IntConfig(dsda_config_viewbob);
}

dboolean dsda_WeaponBob(void) {
  return dsda_IntConfig(dsda_config_weaponbob);
}

dboolean dsda_ShowMessages(void) {
  return dsda_IntConfig(dsda_config_show_messages);
}

dboolean dsda_AutoRun(void) {
  return dsda_IntConfig(dsda_config_autorun);
}

dboolean dsda_MouseLook(void) {
  return dsda_IntConfig(dsda_config_freelook);
}

dboolean dsda_VertMouse(void) {
  return dsda_IntConfig(dsda_config_vertmouse);
}

dboolean dsda_StrictMode(void) {
  return dsda_IntConfig(dsda_config_strict_mode) && demorecording && !dsda_tas;
}

dboolean dsda_MuteSfx(void) {
  return dsda_IntConfig(dsda_config_mute_sfx);
}

dboolean dsda_MuteMusic(void) {
  return dsda_IntConfig(dsda_config_mute_music);
}

dboolean dsda_ProcessCheatCodes(void) {
  return dsda_IntConfig(dsda_config_cheat_codes);
}

dboolean dsda_CycleGhostColors(void) {
  return dsda_IntConfig(dsda_config_cycle_ghost_colors);
}

dboolean dsda_AlwaysSR50(void) {
  return dsda_IntConfig(dsda_config_movement_strafe50);
}

dboolean dsda_HideHorns(void) {
  return dsda_IntConfig(dsda_config_hide_horns);
}

dboolean dsda_HideWeapon(void) {
  return dsda_IntConfig(dsda_config_hide_weapon);
}

dboolean dsda_SwitchWhenAmmoRunsOut(void) {
  return dsda_IntConfig(dsda_config_switch_when_ammo_runs_out);
}

dboolean dsda_SkipQuitPrompt(void) {
  return dsda_IntConfig(dsda_config_skip_quit_prompt);
}

dboolean dsda_TrackSplits(void) {
  return demorecording;
}

dboolean dsda_ShowSplitData(void) {
  return dsda_IntConfig(dsda_config_show_split_data);
}

dboolean dsda_CommandDisplay(void) {
  return dsda_IntConfig(dsda_config_command_display) || dsda_BuildMode();
}

dboolean dsda_CoordinateDisplay(void) {
  return dsda_IntConfig(dsda_config_coordinate_display);
}

dboolean dsda_ShowFPS(void) {
  return dsda_IntConfig(dsda_config_show_fps);
}

dboolean dsda_ShowMinimap(void) {
  return dsda_IntConfig(dsda_config_show_minimap);
}

dboolean dsda_ShowLevelSplits(void) {
  return dsda_IntConfig(dsda_config_show_level_splits);
}

dboolean dsda_ShowDemoAttempts(void) {
  return dsda_IntConfig(dsda_config_show_demo_attempts) && demorecording;
}

dboolean dsda_MapCoordinates(void) {
  return dsda_IntConfig(dsda_config_map_coordinates);
}

dboolean dsda_MapTotals(void) {
  return dsda_IntConfig(dsda_config_map_totals);
}

dboolean dsda_MapTime(void) {
  return dsda_IntConfig(dsda_config_map_time);
}

dboolean dsda_MapTitle(void) {
  return dsda_IntConfig(dsda_config_map_title);
}

dboolean dsda_PainPalette(void) {
  return dsda_IntConfig(dsda_config_palette_ondamage);
}

dboolean dsda_BonusPalette(void) {
  return dsda_IntConfig(dsda_config_palette_onbonus);
}

dboolean dsda_PowerPalette(void) {
  return dsda_IntConfig(dsda_config_palette_onpowers);
}

dboolean dsda_ShowHealthBars(void) {
  //return dsda_IntConfig(dsda_config_gl_health_bar);
  return 0;
}

dboolean dsda_WipeAtFullSpeed(void) {
  return dsda_IntConfig(dsda_config_wipe_at_full_speed);
}

int dsda_ShowAliveMonsters(void) {
  return dsda_IntConfig(dsda_config_show_alive_monsters);
}

int dsda_reveal_map;

int dsda_RevealAutomap(void) {
  if (dsda_StrictMode()) return 0;

  return dsda_reveal_map;
}

void dsda_ResetRevealMap(void) {
  dsda_reveal_map = 0;
}

int dsda_GameSpeed(void) {
  return dsda_IntConfig(dsda_config_game_speed);
}

void dsda_UpdateGameSpeed(int value) {
  dsda_UpdateIntConfig(dsda_config_game_speed, value, true);
}

void dsda_SkipNextWipe(void) {
  dsda_skip_next_wipe = 1;
}

// In raven, strict mode does not affect this setting
dboolean dsda_RenderWipeScreen(void) {
  return raven ? dsda_TransientIntConfig(dsda_config_render_wipescreen) :
                 dsda_IntConfig(dsda_config_render_wipescreen);
}

dboolean dsda_PendingSkipWipe(void) {
  return dsda_skip_next_wipe || !dsda_RenderWipeScreen();
}

dboolean dsda_SkipWipe(void) {
  if (dsda_skip_next_wipe) {
    dsda_skip_next_wipe = 0;
    return true;
  }

  return !dsda_RenderWipeScreen() || hexen;
}

static dboolean game_controller_used;
static dboolean mouse_used;

dboolean dsda_AllowGameController(void) {
  return !dsda_StrictMode() || !mouse_used;
}

dboolean dsda_AllowMouse(void) {
  return !dsda_StrictMode() || !game_controller_used;
}

void dsda_WatchGameControllerEvent(void) {
  game_controller_used = true;

  if (mouse_used)
    dsda_TrackFeature(uf_mouse_and_controller);
}

void dsda_WatchMouseEvent(void) {
  mouse_used = true;

  if (game_controller_used)
    dsda_TrackFeature(uf_mouse_and_controller);
}

void dsda_LiftInputRestrictions(void) {
  game_controller_used = false;
  mouse_used = false;
}
