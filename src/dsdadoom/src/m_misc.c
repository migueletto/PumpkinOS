/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Main loop menu stuff.
 *  Default Config File.
 *  PCX Screenshots.
 *
 *-----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "doomstat.h"
#include "g_game.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "s_sound.h"
#include "lprintf.h"
#include "m_file.h"
#include "d_main.h"

#include "m_misc.h"

#include "dsda/args.h"
#include "dsda/game_controller.h"
#include "dsda/settings.h"

// NSM
#include "i_capture.h"

typedef struct
{
  const char* name;
  dsda_config_identifier_t config_id;
} cfg_def_t;

typedef struct
{
  const char* name;
  int identifier;
  dsda_input_default_t input;
} cfg_input_def_t;

#define SETTING_HEADING(str) { str, 0 }
#define INPUT_SETTING(str, id, k, m, j) { str, id, { k, m, j } }
#define MIGRATED_SETTING(id) { NULL, id }

cfg_def_t cfg_defs[] =
{
  //e6y
  SETTING_HEADING("System settings"),
  MIGRATED_SETTING(dsda_config_process_priority),

  SETTING_HEADING("Misc settings"),
  MIGRATED_SETTING(dsda_config_vanilla_keymap),
  MIGRATED_SETTING(dsda_config_menu_background),
  MIGRATED_SETTING(dsda_config_max_player_corpse),
  MIGRATED_SETTING(dsda_config_flashing_hom),
  MIGRATED_SETTING(dsda_config_demo_smoothturns),
  MIGRATED_SETTING(dsda_config_demo_smoothturnsfactor),
  MIGRATED_SETTING(dsda_config_screenshot_dir),
  MIGRATED_SETTING(dsda_config_startup_delay_ms),
  MIGRATED_SETTING(dsda_config_ansi_endoom),
  MIGRATED_SETTING(dsda_config_announce_map),

  SETTING_HEADING("Game settings"),
  MIGRATED_SETTING(dsda_config_default_complevel),
  MIGRATED_SETTING(dsda_config_default_skill),
  MIGRATED_SETTING(dsda_config_weapon_attack_alignment),
  MIGRATED_SETTING(dsda_config_sts_always_red),
  MIGRATED_SETTING(dsda_config_sts_pct_always_gray),
  MIGRATED_SETTING(dsda_config_sts_traditional_keys),
  MIGRATED_SETTING(dsda_config_show_messages),
  MIGRATED_SETTING(dsda_config_autorun),
  MIGRATED_SETTING(dsda_config_deh_apply_cheats),
  MIGRATED_SETTING(dsda_config_movement_strafe50),
  MIGRATED_SETTING(dsda_config_movement_strafe50onturns),
  MIGRATED_SETTING(dsda_config_movement_shorttics),

  SETTING_HEADING("Sound settings"),
  MIGRATED_SETTING(dsda_config_snd_pcspeaker),
  MIGRATED_SETTING(dsda_config_pitched_sounds),
  MIGRATED_SETTING(dsda_config_full_sounds),
  MIGRATED_SETTING(dsda_config_snd_samplerate),
  MIGRATED_SETTING(dsda_config_snd_samplecount),
  MIGRATED_SETTING(dsda_config_sfx_volume),
  MIGRATED_SETTING(dsda_config_music_volume),
  MIGRATED_SETTING(dsda_config_mus_pause_opt),
  MIGRATED_SETTING(dsda_config_snd_channels),
  //MIGRATED_SETTING(dsda_config_snd_midiplayer),
  MIGRATED_SETTING(dsda_config_snd_mididev),
  MIGRATED_SETTING(dsda_config_snd_soundfont),
  MIGRATED_SETTING(dsda_config_mus_fluidsynth_chorus),
  MIGRATED_SETTING(dsda_config_mus_fluidsynth_reverb),
  MIGRATED_SETTING(dsda_config_mus_fluidsynth_gain),
  MIGRATED_SETTING(dsda_config_mus_fluidsynth_chorus_depth),
  MIGRATED_SETTING(dsda_config_mus_fluidsynth_chorus_level),
  MIGRATED_SETTING(dsda_config_mus_fluidsynth_reverb_damp),
  MIGRATED_SETTING(dsda_config_mus_fluidsynth_reverb_level),
  MIGRATED_SETTING(dsda_config_mus_fluidsynth_reverb_width),
  MIGRATED_SETTING(dsda_config_mus_fluidsynth_reverb_room_size),
  MIGRATED_SETTING(dsda_config_mus_opl_gain),
  MIGRATED_SETTING(dsda_config_mus_portmidi_reset_type),
  MIGRATED_SETTING(dsda_config_mus_portmidi_reset_delay),
  MIGRATED_SETTING(dsda_config_mus_portmidi_filter_sysex),
  MIGRATED_SETTING(dsda_config_mus_portmidi_reverb_level),
  MIGRATED_SETTING(dsda_config_mus_portmidi_chorus_level),

  SETTING_HEADING("Video settings"),
  MIGRATED_SETTING(dsda_config_videomode),
  MIGRATED_SETTING(dsda_config_screen_resolution),
  MIGRATED_SETTING(dsda_config_custom_resolution),
  MIGRATED_SETTING(dsda_config_use_fullscreen),
  MIGRATED_SETTING(dsda_config_exclusive_fullscreen),
  MIGRATED_SETTING(dsda_config_render_vsync),
  MIGRATED_SETTING(dsda_config_uncapped_framerate),
  MIGRATED_SETTING(dsda_config_boom_translucent_sprites),
  MIGRATED_SETTING(dsda_config_screenblocks),
  MIGRATED_SETTING(dsda_config_usegamma),
  MIGRATED_SETTING(dsda_config_fps_limit),
  MIGRATED_SETTING(dsda_config_sdl_video_window_pos),
  MIGRATED_SETTING(dsda_config_palette_ondamage),
  MIGRATED_SETTING(dsda_config_palette_onbonus),
  MIGRATED_SETTING(dsda_config_palette_onpowers),
  MIGRATED_SETTING(dsda_config_render_wipescreen),
  MIGRATED_SETTING(dsda_config_render_screen_multiply),
  MIGRATED_SETTING(dsda_config_integer_scaling),
  MIGRATED_SETTING(dsda_config_render_aspect),
  MIGRATED_SETTING(dsda_config_render_doom_lightmaps),
  MIGRATED_SETTING(dsda_config_fake_contrast_mode),
  MIGRATED_SETTING(dsda_config_render_stretch_hud),
  MIGRATED_SETTING(dsda_config_render_patches_scalex),
  MIGRATED_SETTING(dsda_config_render_patches_scaley),
  MIGRATED_SETTING(dsda_config_render_stretchsky),
  MIGRATED_SETTING(dsda_config_freelook),

  //SETTING_HEADING("OpenGL settings"),
  //MIGRATED_SETTING(dsda_config_gl_render_multisampling),
  //MIGRATED_SETTING(dsda_config_gl_render_fov),
  //MIGRATED_SETTING(dsda_config_gl_skymode),
  //MIGRATED_SETTING(dsda_config_gl_health_bar),
  //MIGRATED_SETTING(dsda_config_gl_usevbo),
  //MIGRATED_SETTING(dsda_config_gl_fade_mode),

  SETTING_HEADING("Mouse settings"),
  //MIGRATED_SETTING(dsda_config_use_mouse),
  MIGRATED_SETTING(dsda_config_mouse_stutter_correction),
  MIGRATED_SETTING(dsda_config_mouse_sensitivity_horiz),
  MIGRATED_SETTING(dsda_config_fine_sensitivity),
  MIGRATED_SETTING(dsda_config_mouse_sensitivity_vert),
  MIGRATED_SETTING(dsda_config_mouse_acceleration),
  MIGRATED_SETTING(dsda_config_mouse_sensitivity_mlook),
  MIGRATED_SETTING(dsda_config_mouse_doubleclick_as_use),
  MIGRATED_SETTING(dsda_config_mouse_carrytics),
  MIGRATED_SETTING(dsda_config_vertmouse),
  MIGRATED_SETTING(dsda_config_movement_mousestrafedivisor),
  MIGRATED_SETTING(dsda_config_movement_mouseinvert),

  SETTING_HEADING("Game controller settings"),
  //MIGRATED_SETTING(dsda_config_use_game_controller),
  //MIGRATED_SETTING(dsda_config_left_analog_deadzone),
  //MIGRATED_SETTING(dsda_config_right_analog_deadzone),
  //MIGRATED_SETTING(dsda_config_left_trigger_deadzone),
  //MIGRATED_SETTING(dsda_config_right_trigger_deadzone),
  //MIGRATED_SETTING(dsda_config_left_analog_sensitivity_x),
  //MIGRATED_SETTING(dsda_config_left_analog_sensitivity_y),
  //MIGRATED_SETTING(dsda_config_right_analog_sensitivity_x),
  //MIGRATED_SETTING(dsda_config_right_analog_sensitivity_y),
  MIGRATED_SETTING(dsda_config_analog_look_acceleration),
  //MIGRATED_SETTING(dsda_config_swap_analogs),
  MIGRATED_SETTING(dsda_config_invert_analog_look),

  SETTING_HEADING("Automap settings"),
  MIGRATED_SETTING(dsda_config_mapcolor_back),
  MIGRATED_SETTING(dsda_config_mapcolor_grid),
  MIGRATED_SETTING(dsda_config_mapcolor_wall),
  MIGRATED_SETTING(dsda_config_mapcolor_fchg),
  MIGRATED_SETTING(dsda_config_mapcolor_cchg),
  MIGRATED_SETTING(dsda_config_mapcolor_clsd),
  MIGRATED_SETTING(dsda_config_mapcolor_rkey),
  MIGRATED_SETTING(dsda_config_mapcolor_bkey),
  MIGRATED_SETTING(dsda_config_mapcolor_ykey),
  MIGRATED_SETTING(dsda_config_mapcolor_rdor),
  MIGRATED_SETTING(dsda_config_mapcolor_bdor),
  MIGRATED_SETTING(dsda_config_mapcolor_ydor),
  MIGRATED_SETTING(dsda_config_mapcolor_tele),
  MIGRATED_SETTING(dsda_config_mapcolor_secr),
  MIGRATED_SETTING(dsda_config_mapcolor_revsecr),
  MIGRATED_SETTING(dsda_config_mapcolor_exit),
  MIGRATED_SETTING(dsda_config_mapcolor_unsn),
  MIGRATED_SETTING(dsda_config_mapcolor_flat),
  MIGRATED_SETTING(dsda_config_mapcolor_sprt),
  MIGRATED_SETTING(dsda_config_mapcolor_item),
  MIGRATED_SETTING(dsda_config_mapcolor_hair),
  MIGRATED_SETTING(dsda_config_mapcolor_sngl),
  MIGRATED_SETTING(dsda_config_mapcolor_me),
  MIGRATED_SETTING(dsda_config_mapcolor_enemy),
  MIGRATED_SETTING(dsda_config_mapcolor_frnd),
  MIGRATED_SETTING(dsda_config_map_blinking_locks),
  MIGRATED_SETTING(dsda_config_map_secret_after),
  MIGRATED_SETTING(dsda_config_map_coordinates),
  MIGRATED_SETTING(dsda_config_map_totals),
  MIGRATED_SETTING(dsda_config_map_time),
  MIGRATED_SETTING(dsda_config_map_title),
  MIGRATED_SETTING(dsda_config_automap_overlay),
  MIGRATED_SETTING(dsda_config_automap_rotate),
  MIGRATED_SETTING(dsda_config_automap_follow),
  MIGRATED_SETTING(dsda_config_automap_grid),
  MIGRATED_SETTING(dsda_config_map_grid_size),
  MIGRATED_SETTING(dsda_config_map_scroll_speed),
  MIGRATED_SETTING(dsda_config_map_wheel_zoom),
  MIGRATED_SETTING(dsda_config_map_use_multisamling),
  MIGRATED_SETTING(dsda_config_map_textured),
  //MIGRATED_SETTING(dsda_config_map_textured_trans),
  //MIGRATED_SETTING(dsda_config_map_textured_overlay_trans),
  //MIGRATED_SETTING(dsda_config_map_lines_overlay_trans),
  MIGRATED_SETTING(dsda_config_map_things_appearance),

  SETTING_HEADING("Heads-up display settings"),
  MIGRATED_SETTING(dsda_config_hud_health_red),
  MIGRATED_SETTING(dsda_config_hud_health_yellow),
  MIGRATED_SETTING(dsda_config_hud_health_green),
  MIGRATED_SETTING(dsda_config_hud_ammo_red),
  MIGRATED_SETTING(dsda_config_hud_ammo_yellow),
  MIGRATED_SETTING(dsda_config_hud_displayed),
  MIGRATED_SETTING(dsda_config_hudadd_secretarea),
  MIGRATED_SETTING(dsda_config_hudadd_demoprogressbar),
  MIGRATED_SETTING(dsda_config_hudadd_crosshair),
  MIGRATED_SETTING(dsda_config_hudadd_crosshair_scale),
  MIGRATED_SETTING(dsda_config_hudadd_crosshair_color),
  MIGRATED_SETTING(dsda_config_hudadd_crosshair_health),
  MIGRATED_SETTING(dsda_config_hudadd_crosshair_target),
  MIGRATED_SETTING(dsda_config_hudadd_crosshair_target_color),
  MIGRATED_SETTING(dsda_config_hudadd_crosshair_lock_target),

  SETTING_HEADING("DSDA-Doom settings"),
  MIGRATED_SETTING(dsda_config_strict_mode),
  MIGRATED_SETTING(dsda_config_cycle_ghost_colors),
  MIGRATED_SETTING(dsda_config_auto_key_frame_interval),
  MIGRATED_SETTING(dsda_config_auto_key_frame_depth),
  MIGRATED_SETTING(dsda_config_auto_key_frame_timeout),
  MIGRATED_SETTING(dsda_config_exhud),
  MIGRATED_SETTING(dsda_config_ex_text_scale_x),
  MIGRATED_SETTING(dsda_config_ex_text_ratio_y),
  MIGRATED_SETTING(dsda_config_free_text),
  MIGRATED_SETTING(dsda_config_wipe_at_full_speed),
  MIGRATED_SETTING(dsda_config_show_demo_attempts),
  MIGRATED_SETTING(dsda_config_hide_horns),
  MIGRATED_SETTING(dsda_config_hide_weapon),
  MIGRATED_SETTING(dsda_config_organized_saves),
  MIGRATED_SETTING(dsda_config_command_display),
  MIGRATED_SETTING(dsda_config_command_history_size),
  MIGRATED_SETTING(dsda_config_hide_empty_commands),
  MIGRATED_SETTING(dsda_config_coordinate_display),
  MIGRATED_SETTING(dsda_config_show_fps),
  MIGRATED_SETTING(dsda_config_show_minimap),
  MIGRATED_SETTING(dsda_config_show_level_splits),
  MIGRATED_SETTING(dsda_config_skip_quit_prompt),
  MIGRATED_SETTING(dsda_config_show_split_data),
  MIGRATED_SETTING(dsda_config_player_name),
  MIGRATED_SETTING(dsda_config_quickstart_cache_tics),
  MIGRATED_SETTING(dsda_config_death_use_action),
  MIGRATED_SETTING(dsda_config_mute_sfx),
  MIGRATED_SETTING(dsda_config_mute_music),
  MIGRATED_SETTING(dsda_config_cheat_codes),
  MIGRATED_SETTING(dsda_config_allow_jumping),
  MIGRATED_SETTING(dsda_config_parallel_sfx_limit),
  MIGRATED_SETTING(dsda_config_parallel_sfx_window),
  MIGRATED_SETTING(dsda_config_movement_toggle_sfx),
  MIGRATED_SETTING(dsda_config_switch_when_ammo_runs_out),
  MIGRATED_SETTING(dsda_config_viewbob),
  MIGRATED_SETTING(dsda_config_weaponbob),
  MIGRATED_SETTING(dsda_config_quake_intensity),
  MIGRATED_SETTING(dsda_config_organize_failed_demos),

  SETTING_HEADING("Scripts"),
  MIGRATED_SETTING(dsda_config_script_0),
  MIGRATED_SETTING(dsda_config_script_1),
  MIGRATED_SETTING(dsda_config_script_2),
  MIGRATED_SETTING(dsda_config_script_3),
  MIGRATED_SETTING(dsda_config_script_4),
  MIGRATED_SETTING(dsda_config_script_5),
  MIGRATED_SETTING(dsda_config_script_6),
  MIGRATED_SETTING(dsda_config_script_7),
  MIGRATED_SETTING(dsda_config_script_8),
  MIGRATED_SETTING(dsda_config_script_9),

  // NSM
  SETTING_HEADING("Video capture encoding settings"),
  MIGRATED_SETTING(dsda_config_cap_soundcommand),
  MIGRATED_SETTING(dsda_config_cap_videocommand),
  MIGRATED_SETTING(dsda_config_cap_muxcommand),
  MIGRATED_SETTING(dsda_config_cap_tempfile1),
  MIGRATED_SETTING(dsda_config_cap_tempfile2),
  MIGRATED_SETTING(dsda_config_cap_remove_tempfiles),
  MIGRATED_SETTING(dsda_config_cap_wipescreen),
  MIGRATED_SETTING(dsda_config_cap_fps),

  SETTING_HEADING("Overrun settings"),
  MIGRATED_SETTING(dsda_config_overrun_spechit_warn),
  MIGRATED_SETTING(dsda_config_overrun_spechit_emulate),
  MIGRATED_SETTING(dsda_config_overrun_reject_warn),
  MIGRATED_SETTING(dsda_config_overrun_reject_emulate),
  MIGRATED_SETTING(dsda_config_overrun_intercept_warn),
  MIGRATED_SETTING(dsda_config_overrun_intercept_emulate),
  MIGRATED_SETTING(dsda_config_overrun_playeringame_warn),
  MIGRATED_SETTING(dsda_config_overrun_playeringame_emulate),
  MIGRATED_SETTING(dsda_config_overrun_donut_warn),
  MIGRATED_SETTING(dsda_config_overrun_donut_emulate),
  MIGRATED_SETTING(dsda_config_overrun_missedbackside_warn),
  MIGRATED_SETTING(dsda_config_overrun_missedbackside_emulate),

  SETTING_HEADING("Mapping error compatibility settings"),
  MIGRATED_SETTING(dsda_config_comperr_passuse),
  MIGRATED_SETTING(dsda_config_comperr_hangsolid),
  MIGRATED_SETTING(dsda_config_comperr_blockmap),
  MIGRATED_SETTING(dsda_config_comperr_freeaim),

  SETTING_HEADING("Weapon preferences"),
  MIGRATED_SETTING(dsda_config_weapon_choice_1),
  MIGRATED_SETTING(dsda_config_weapon_choice_2),
  MIGRATED_SETTING(dsda_config_weapon_choice_3),
  MIGRATED_SETTING(dsda_config_weapon_choice_4),
  MIGRATED_SETTING(dsda_config_weapon_choice_5),
  MIGRATED_SETTING(dsda_config_weapon_choice_6),
  MIGRATED_SETTING(dsda_config_weapon_choice_7),
  MIGRATED_SETTING(dsda_config_weapon_choice_8),
  MIGRATED_SETTING(dsda_config_weapon_choice_9),

  SETTING_HEADING("Input settings"),
  MIGRATED_SETTING(dsda_config_input_profile),
};

cfg_input_def_t input_defs[] = {
  INPUT_SETTING("input_forward", dsda_input_forward, KEYD_UPARROW, 2, -1),
  INPUT_SETTING("input_backward", dsda_input_backward, KEYD_DOWNARROW, -1, -1),
  INPUT_SETTING("input_turnleft", dsda_input_turnleft, KEYD_LEFTARROW, -1, -1),
  INPUT_SETTING("input_turnright", dsda_input_turnright, KEYD_RIGHTARROW, -1, -1),
  INPUT_SETTING("input_speed", dsda_input_speed, 0, -1, -1),
  INPUT_SETTING("input_strafeleft", dsda_input_strafeleft, 'a', -1, -1),
  INPUT_SETTING("input_straferight", dsda_input_straferight, 'd', -1, -1),
  INPUT_SETTING("input_strafe", dsda_input_strafe, 0, 1, DSDA_CONTROLLER_BUTTON_LEFTSHOULDER),
  INPUT_SETTING("input_autorun", dsda_input_autorun, KEYD_CAPSLOCK, -1, DSDA_CONTROLLER_BUTTON_LEFTSTICK),
  INPUT_SETTING("input_reverse", dsda_input_reverse, '/', -1, DSDA_CONTROLLER_BUTTON_RIGHTSTICK),
  INPUT_SETTING("input_use", dsda_input_use, ' ', -1, DSDA_CONTROLLER_BUTTON_A),
  INPUT_SETTING("input_flyup", dsda_input_flyup, '.', -1, DSDA_CONTROLLER_BUTTON_DPAD_UP),
  INPUT_SETTING("input_flydown", dsda_input_flydown, ',', -1, DSDA_CONTROLLER_BUTTON_DPAD_DOWN),
  INPUT_SETTING("input_flycenter", dsda_input_flycenter, 0, -1, -1),
  INPUT_SETTING("input_mlook", dsda_input_mlook, '\\', -1, -1),
  INPUT_SETTING("input_novert", dsda_input_novert, 0, -1, -1),

  INPUT_SETTING("input_weapon1", dsda_input_weapon1, '1', -1, -1),
  INPUT_SETTING("input_weapon2", dsda_input_weapon2, '2', -1, -1),
  INPUT_SETTING("input_weapon3", dsda_input_weapon3, '3', -1, -1),
  INPUT_SETTING("input_weapon4", dsda_input_weapon4, '4', -1, -1),
  INPUT_SETTING("input_weapon5", dsda_input_weapon5, '5', -1, -1),
  INPUT_SETTING("input_weapon6", dsda_input_weapon6, '6', -1, -1),
  INPUT_SETTING("input_weapon7", dsda_input_weapon7, '7', -1, -1),
  INPUT_SETTING("input_weapon8", dsda_input_weapon8, '8', -1, -1),
  INPUT_SETTING("input_weapon9", dsda_input_weapon9, '9', -1, -1),
  INPUT_SETTING("input_nextweapon", dsda_input_nextweapon, 0, -1, DSDA_CONTROLLER_BUTTON_Y),
  INPUT_SETTING("input_prevweapon", dsda_input_prevweapon, 0, -1, DSDA_CONTROLLER_BUTTON_X),
  INPUT_SETTING("input_toggleweapon", dsda_input_toggleweapon, '0', -1, -1),
  INPUT_SETTING("input_fire", dsda_input_fire, KEYD_RCTRL, 0, DSDA_CONTROLLER_BUTTON_TRIGGERRIGHT),

  INPUT_SETTING("input_pause", dsda_input_pause, KEYD_PAUSE, -1, -1),
  INPUT_SETTING("input_map", dsda_input_map, KEYD_TAB, -1, DSDA_CONTROLLER_BUTTON_TRIGGERLEFT),
  INPUT_SETTING("input_soundvolume", dsda_input_soundvolume, KEYD_F4, -1, -1),
  INPUT_SETTING("input_hud", dsda_input_hud, KEYD_F5, -1, -1),
  INPUT_SETTING("input_messages", dsda_input_messages, KEYD_F8, -1, -1),
  INPUT_SETTING("input_gamma", dsda_input_gamma, KEYD_F11, -1, -1),
  INPUT_SETTING("input_spy", dsda_input_spy, KEYD_F12, -1, -1),
  INPUT_SETTING("input_zoomin", dsda_input_zoomin, '=', -1, -1),
  INPUT_SETTING("input_zoomout", dsda_input_zoomout, '-', -1, -1),
  INPUT_SETTING("input_screenshot", dsda_input_screenshot, '*', -1, -1),
  INPUT_SETTING("input_savegame", dsda_input_savegame, KEYD_F2, -1, -1),
  INPUT_SETTING("input_loadgame", dsda_input_loadgame, KEYD_F3, -1, -1),
  INPUT_SETTING("input_quicksave", dsda_input_quicksave, KEYD_F6, -1, -1),
  INPUT_SETTING("input_quickload", dsda_input_quickload, KEYD_F9, -1, -1),
  INPUT_SETTING("input_endgame", dsda_input_endgame, KEYD_F7, -1, -1),
  INPUT_SETTING("input_quit", dsda_input_quit, KEYD_F10, -1, -1),

  INPUT_SETTING("input_map_follow", dsda_input_map_follow, 'f', -1, -1),
  INPUT_SETTING("input_map_zoomin", dsda_input_map_zoomin, '=', -1, -1),
  INPUT_SETTING("input_map_zoomout", dsda_input_map_zoomout, '-', -1, -1),
  INPUT_SETTING("input_map_up", dsda_input_map_up, KEYD_UPARROW, -1, -1),
  INPUT_SETTING("input_map_down", dsda_input_map_down, KEYD_DOWNARROW, -1, -1),
  INPUT_SETTING("input_map_left", dsda_input_map_left, KEYD_LEFTARROW, -1, -1),
  INPUT_SETTING("input_map_right", dsda_input_map_right, KEYD_RIGHTARROW, -1, -1),
  INPUT_SETTING("input_map_mark", dsda_input_map_mark, 'm', -1, -1),
  INPUT_SETTING("input_map_clear", dsda_input_map_clear, 'c', -1, -1),
  INPUT_SETTING("input_map_gobig", dsda_input_map_gobig, '0', -1, -1),
  INPUT_SETTING("input_map_grid", dsda_input_map_grid, 'g', -1, -1),
  INPUT_SETTING("input_map_rotate", dsda_input_map_rotate, 'r', -1, -1),
  INPUT_SETTING("input_map_overlay", dsda_input_map_overlay, 'o', -1, -1),
  INPUT_SETTING("input_map_textured", dsda_input_map_textured, 0, -1, -1),

  INPUT_SETTING("input_repeat_message", dsda_input_repeat_message, 0, -1, -1),

  INPUT_SETTING("input_speed_up", dsda_input_speed_up, 0, -1, -1),
  INPUT_SETTING("input_speed_down", dsda_input_speed_down, 0, -1, -1),
  INPUT_SETTING("input_speed_default", dsda_input_speed_default, 0, -1, -1),
  INPUT_SETTING("input_demo_skip", dsda_input_demo_skip, KEYD_INSERT, -1, -1),
  INPUT_SETTING("input_demo_endlevel", dsda_input_demo_endlevel, KEYD_END, -1, -1),
  INPUT_SETTING("input_walkcamera", dsda_input_walkcamera, KEYD_KEYPAD0, -1, -1),
  INPUT_SETTING("input_join_demo", dsda_input_join_demo, 0, -1, -1),
  INPUT_SETTING("input_restart", dsda_input_restart, KEYD_HOME, -1, -1),
  INPUT_SETTING("input_nextlevel", dsda_input_nextlevel, KEYD_PAGEDOWN, -1, -1),
  INPUT_SETTING("input_showalive", dsda_input_showalive, 0, -1, -1),

  INPUT_SETTING("input_menu_down", dsda_input_menu_down, KEYD_DOWNARROW, -1, DSDA_CONTROLLER_BUTTON_DPAD_DOWN),
  INPUT_SETTING("input_menu_up", dsda_input_menu_up, KEYD_UPARROW, -1, DSDA_CONTROLLER_BUTTON_DPAD_UP),
  INPUT_SETTING("input_menu_left", dsda_input_menu_left, KEYD_LEFTARROW, -1, DSDA_CONTROLLER_BUTTON_DPAD_LEFT),
  INPUT_SETTING("input_menu_right", dsda_input_menu_right, KEYD_RIGHTARROW, -1, DSDA_CONTROLLER_BUTTON_DPAD_RIGHT),
  INPUT_SETTING("input_menu_backspace", dsda_input_menu_backspace, KEYD_BACKSPACE, -1, DSDA_CONTROLLER_BUTTON_B),
  INPUT_SETTING("input_menu_enter", dsda_input_menu_enter, KEYD_ENTER, -1, DSDA_CONTROLLER_BUTTON_A),
  INPUT_SETTING("input_menu_escape", dsda_input_menu_escape, KEYD_ESCAPE, -1, DSDA_CONTROLLER_BUTTON_START),
  INPUT_SETTING("input_menu_clear", dsda_input_menu_clear, KEYD_DEL, -1, DSDA_CONTROLLER_BUTTON_BACK),

  INPUT_SETTING("input_iddqd", dsda_input_iddqd, 0, -1, -1),
  INPUT_SETTING("input_idkfa", dsda_input_idkfa, 0, -1, -1),
  INPUT_SETTING("input_idfa", dsda_input_idfa, 0, -1, -1),
  INPUT_SETTING("input_idclip", dsda_input_idclip, 0, -1, -1),
  INPUT_SETTING("input_idbeholdh", dsda_input_idbeholdh, 0, -1, -1),
  INPUT_SETTING("input_idbeholdm", dsda_input_idbeholdm, 0, -1, -1),
  INPUT_SETTING("input_idbeholdv", dsda_input_idbeholdv, 0, -1, -1),
  INPUT_SETTING("input_idbeholds", dsda_input_idbeholds, 0, -1, -1),
  INPUT_SETTING("input_idbeholdi", dsda_input_idbeholdi, 0, -1, -1),
  INPUT_SETTING("input_idbeholdr", dsda_input_idbeholdr, 0, -1, -1),
  INPUT_SETTING("input_idbeholda", dsda_input_idbeholda, 0, -1, -1),
  INPUT_SETTING("input_idbeholdl", dsda_input_idbeholdl, 0, -1, -1),
  INPUT_SETTING("input_idmypos", dsda_input_idmypos, 0, -1, -1),
  INPUT_SETTING("input_idrate", dsda_input_idrate, 0, -1, -1),
  INPUT_SETTING("input_iddt", dsda_input_iddt, 0, -1, -1),
  INPUT_SETTING("input_ponce", dsda_input_ponce, 0, -1, -1),
  INPUT_SETTING("input_shazam", dsda_input_shazam, 0, -1, -1),
  INPUT_SETTING("input_chicken", dsda_input_chicken, 0, -1, -1),

  INPUT_SETTING("input_lookup", dsda_input_lookup, 0, -1, -1),
  INPUT_SETTING("input_lookdown", dsda_input_lookdown, 0, -1, -1),
  INPUT_SETTING("input_lookcenter", dsda_input_lookcenter, 0, -1, -1),
  INPUT_SETTING("input_use_artifact", dsda_input_use_artifact, KEYD_RSHIFT, -1, DSDA_CONTROLLER_BUTTON_RIGHTSHOULDER),
  INPUT_SETTING("input_arti_tome", dsda_input_arti_tome, 0, -1, -1),
  INPUT_SETTING("input_arti_quartz", dsda_input_arti_quartz, 0, -1, -1),
  INPUT_SETTING("input_arti_urn", dsda_input_arti_urn, 0, -1, -1),
  INPUT_SETTING("input_arti_bomb", dsda_input_arti_bomb, 0, -1, -1),
  INPUT_SETTING("input_arti_ring", dsda_input_arti_ring, 0, -1, -1),
  INPUT_SETTING("input_arti_chaosdevice", dsda_input_arti_chaosdevice, 0, -1, -1),
  INPUT_SETTING("input_arti_shadowsphere", dsda_input_arti_shadowsphere, 0, -1, -1),
  INPUT_SETTING("input_arti_wings", dsda_input_arti_wings, 0, -1, -1),
  INPUT_SETTING("input_arti_torch", dsda_input_arti_torch, 0, -1, -1),
  INPUT_SETTING("input_arti_morph", dsda_input_arti_morph, 0, -1, -1),
  INPUT_SETTING("input_invleft", dsda_input_invleft, KEYD_MWHEELDOWN, -1, DSDA_CONTROLLER_BUTTON_DPAD_LEFT),
  INPUT_SETTING("input_invright", dsda_input_invright, KEYD_MWHEELUP, -1, DSDA_CONTROLLER_BUTTON_DPAD_RIGHT),
  INPUT_SETTING("input_store_quick_key_frame", dsda_input_store_quick_key_frame, 0, -1, -1),
  INPUT_SETTING("input_restore_quick_key_frame", dsda_input_restore_quick_key_frame, 0, -1, -1),
  INPUT_SETTING("input_rewind", dsda_input_rewind, 0, -1, -1),
  INPUT_SETTING("input_cycle_profile", dsda_input_cycle_profile, 0, -1, -1),
  INPUT_SETTING("input_cycle_palette", dsda_input_cycle_palette, 0, -1, -1),
  INPUT_SETTING("input_command_display", dsda_input_command_display, 0, -1, -1),
  INPUT_SETTING("input_strict_mode", dsda_input_strict_mode, 0, -1, -1),
  INPUT_SETTING("input_console", dsda_input_console, 0, -1, -1),
  INPUT_SETTING("input_coordinate_display", dsda_input_coordinate_display, 0, -1, -1),
  INPUT_SETTING("input_fps", dsda_input_fps, 0, -1, -1),
  INPUT_SETTING("input_avj", dsda_input_avj, 0, -1, -1),
  INPUT_SETTING("input_exhud", dsda_input_exhud, 0, -1, -1),
  INPUT_SETTING("input_mute_sfx", dsda_input_mute_sfx, 0, -1, -1),
  INPUT_SETTING("input_mute_music", dsda_input_mute_music, 0, -1, -1),
  INPUT_SETTING("input_cheat_codes", dsda_input_cheat_codes, 0, -1, -1),
  INPUT_SETTING("input_notarget", dsda_input_notarget, 0, -1, -1),
  INPUT_SETTING("input_freeze", dsda_input_freeze, 0, -1, -1),

  INPUT_SETTING("input_build", dsda_input_build, 0, -1, -1),
  INPUT_SETTING("input_build_advance_frame", dsda_input_build_advance_frame, KEYD_RIGHTARROW, -1, -1),
  INPUT_SETTING("input_build_reverse_frame", dsda_input_build_reverse_frame, KEYD_LEFTARROW, -1, -1),
  INPUT_SETTING("input_build_reset_command", dsda_input_build_reset_command, KEYD_DEL, -1, -1),
  INPUT_SETTING("input_build_source", dsda_input_build_source, KEYD_RSHIFT, -1, -1),
  INPUT_SETTING("input_build_forward", dsda_input_build_forward, 'w', -1, -1),
  INPUT_SETTING("input_build_backward", dsda_input_build_backward, 's', -1, -1),
  INPUT_SETTING("input_build_fine_forward", dsda_input_build_fine_forward, 't', -1, -1),
  INPUT_SETTING("input_build_fine_backward", dsda_input_build_fine_backward, 'g', -1, -1),
  INPUT_SETTING("input_build_turn_left", dsda_input_build_turn_left, 'q', -1, -1),
  INPUT_SETTING("input_build_turn_right", dsda_input_build_turn_right, 'e', -1, -1),
  INPUT_SETTING("input_build_strafe_left", dsda_input_build_strafe_left, 'a', -1, -1),
  INPUT_SETTING("input_build_strafe_right", dsda_input_build_strafe_right, 'd', -1, -1),
  INPUT_SETTING("input_build_fine_strafe_left", dsda_input_build_fine_strafe_left, 'f', -1, -1),
  INPUT_SETTING("input_build_fine_strafe_right", dsda_input_build_fine_strafe_right, 'h', -1, -1),
  INPUT_SETTING("input_build_use", dsda_input_build_use, KEYD_SPACEBAR, -1, -1),
  INPUT_SETTING("input_build_fire", dsda_input_build_fire, KEYD_RCTRL, -1, -1),
  INPUT_SETTING("input_build_weapon1", dsda_input_build_weapon1, '1', -1, -1),
  INPUT_SETTING("input_build_weapon2", dsda_input_build_weapon2, '2', -1, -1),
  INPUT_SETTING("input_build_weapon3", dsda_input_build_weapon3, '3', -1, -1),
  INPUT_SETTING("input_build_weapon4", dsda_input_build_weapon4, '4', -1, -1),
  INPUT_SETTING("input_build_weapon5", dsda_input_build_weapon5, '5', -1, -1),
  INPUT_SETTING("input_build_weapon6", dsda_input_build_weapon6, '6', -1, -1),
  INPUT_SETTING("input_build_weapon7", dsda_input_build_weapon7, '7', -1, -1),
  INPUT_SETTING("input_build_weapon8", dsda_input_build_weapon8, '8', -1, -1),
  INPUT_SETTING("input_build_weapon9", dsda_input_build_weapon9, '9', -1, -1),

  INPUT_SETTING("input_jump", dsda_input_jump, KEYD_RALT, -1, DSDA_CONTROLLER_BUTTON_B),
  INPUT_SETTING("input_hexen_arti_incant", dsda_input_hexen_arti_incant, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_summon", dsda_input_hexen_arti_summon, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_disk", dsda_input_hexen_arti_disk, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_flechette", dsda_input_hexen_arti_flechette, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_banishment", dsda_input_hexen_arti_banishment, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_boots", dsda_input_hexen_arti_boots, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_krater", dsda_input_hexen_arti_krater, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_bracers", dsda_input_hexen_arti_bracers, 0, -1, -1),

  INPUT_SETTING("input_script_0", dsda_input_script_0, 0, -1, -1),
  INPUT_SETTING("input_script_1", dsda_input_script_1, 0, -1, -1),
  INPUT_SETTING("input_script_2", dsda_input_script_2, 0, -1, -1),
  INPUT_SETTING("input_script_3", dsda_input_script_3, 0, -1, -1),
  INPUT_SETTING("input_script_4", dsda_input_script_4, 0, -1, -1),
  INPUT_SETTING("input_script_5", dsda_input_script_5, 0, -1, -1),
  INPUT_SETTING("input_script_6", dsda_input_script_6, 0, -1, -1),
  INPUT_SETTING("input_script_7", dsda_input_script_7, 0, -1, -1),
  INPUT_SETTING("input_script_8", dsda_input_script_8, 0, -1, -1),
  INPUT_SETTING("input_script_9", dsda_input_script_9, 0, -1, -1),
};

static int input_def_count = sizeof(input_defs) / sizeof(input_defs[0]);
static int def_count = sizeof(cfg_defs) / sizeof(cfg_defs[0]);

static char* defaultfile; // CPhipps - static, const

static dboolean forget_config_file;

void M_ForgetCurrentConfig(void)
{
  forget_config_file = true;
}

void M_RememberCurrentConfig(void)
{
  forget_config_file = false;
}

void M_SaveDefaults (void)
{
  int   i;
  dg_file_t* f;
  int maxlen;

  if (forget_config_file)
    return;

  f = M_OpenFile(defaultfile, "w");
  if (!f)
    return; // can't write the file, but don't complain

  maxlen = dsda_MaxConfigLength();

  for (i = 0; i < input_def_count; i++) {
    int len;

    len = strlen(input_defs[i].name);
    if (len > maxlen && len < 80)
      maxlen = len;
  }

  // 3/3/98 explain format of file

  DG_printf(f, "# Doom config file\n");
  DG_printf(f, "# Format:\n");
  DG_printf(f, "# variable   value\n");

  for (i = 0 ; i < def_count ; i++) {
    if (cfg_defs[i].config_id)
    {
      dsda_WriteConfig(cfg_defs[i].config_id, maxlen, f);
    }
    else if (cfg_defs[i].name)
    {
      DG_printf(f, "\n# %s\n", cfg_defs[i].name);
    }
  }

  DG_printf(f, "\n");

  for (i = 0; i < input_def_count; ++i) {
    int a, j;
    dsda_input_t* input[DSDA_INPUT_PROFILE_COUNT];
    dsda_InputCopy(input_defs[i].identifier, input);

    DG_printf(f, "%-*s", maxlen, input_defs[i].name);

    for (a = 0; a < DSDA_INPUT_PROFILE_COUNT; ++a)
    {
      if (input[a]->num_keys)
      {
        DG_printf(f, " %i", input[a]->key[0]);
        for (j = 1; j < input[a]->num_keys; ++j)
        {
          DG_printf(f, ",%i", input[a]->key[j]);
        }
      }
      else
        DG_printf(f, " 0");

      DG_printf(f, " %i %i", input[a]->mouseb, input[a]->joyb);

      if (a != DSDA_INPUT_PROFILE_COUNT - 1)
        DG_printf(f, " |");
    }

    DG_printf(f, "\n");
  }

  DG_close (f);
}

//
// M_LoadDefaults
//

#define CFG_BUFFERMAX 32000

void M_LoadDefaults (void)
{
  int   i;
  int   len;
  dg_file_t* f;
  char  def[80];
  char* strparm = Z_Malloc(CFG_BUFFERMAX);
  char* cfgline = Z_Malloc(CFG_BUFFERMAX);
  char* newstring = NULL;   // killough
  int   parm;
  dsda_arg_t *arg;

  // set everything to base values

  dsda_InitConfig();

  for (i = 0; i < input_def_count; i++) {
    int c;

    for (c = 0; c < DSDA_INPUT_PROFILE_COUNT; ++c)
      dsda_InputSetSpecific(c, input_defs[i].identifier, input_defs[i].input);
  }

  // special fallback input values
  {
    dsda_input_default_t fallback_help = { KEYD_F1, -1, -1 };
    dsda_input_default_t fallback_escape = { KEYD_ESCAPE, -1, -1 };

    for (i = 0; i < DSDA_INPUT_PROFILE_COUNT; ++i)
    {
      dsda_InputSetSpecific(i, dsda_input_help, fallback_help);
      dsda_InputSetSpecific(i, dsda_input_escape, fallback_escape);
    }
  }

  // check for a custom default file

  arg = dsda_Arg(dsda_arg_config);
  if (arg->found)
  {
    defaultfile = Z_Strdup(arg->value.v_string);
  }
  else
  {
    const char* exedir = I_DoomExeDir();
    int len = snprintf(NULL, 0, "%s/dsda-doom.cfg", exedir);
    defaultfile = Z_Malloc(len + 1);
    snprintf(defaultfile, len + 1, "%s/dsda-doom.cfg", exedir);
  }

  lprintf(LO_DEBUG, " default file: %s\n", defaultfile);

  // read the file in, overriding any set defaults

  f = M_OpenFile(defaultfile, "r");
  if (f)
  {
    while (!DG_eof(f))
    {
      parm = 0;

      cfgline = DG_fgets(f, cfgline, CFG_BUFFERMAX);
      if (!cfgline)
        break;

      if (sscanf (cfgline, "%79s %[^\n]\n", def, strparm) == 2)
      {
        newstring = NULL;

        //jff 3/3/98 skip lines not starting with an alphanum
        if (!isalnum(def[0]))
          continue;

        if (strparm[0] == '"')
        {
          // get a string
          len = strlen(strparm);
          newstring = Z_Malloc(len);
          strparm[len - 1] = 0; // clears trailing double-quote mark
          strcpy(newstring, strparm + 1); // clears leading double-quote mark
        }
        else if ((strparm[0] == '0') && (strparm[1] == 'x'))
        {
          // CPhipps - allow ints to be specified in hex
          sscanf(strparm+2, "%x", &parm);
        }
        else
        {
          sscanf(strparm, "%i", &parm);
        }

        if (dsda_ReadConfig(def, newstring, parm))
        {
          Z_Free(newstring);
        }
        else
        {
          for (i = 0; i < input_def_count; i++)
            if (!strcmp(def, input_defs[i].name))
            {
              int count;
              char keys[80];
              int key, mouseb, joyb;
              int index = 0;
              char* key_scan_p;
              char* config_scan_p;

              config_scan_p = strparm;
              do
              {
                count = sscanf(config_scan_p, "%79s %d %d", keys, &mouseb, &joyb);

                if (count != 3)
                  break;

                dsda_InputResetSpecific(index, input_defs[i].identifier);

                dsda_InputAddSpecificMouseB(index, input_defs[i].identifier, mouseb);
                dsda_InputAddSpecificJoyB(index, input_defs[i].identifier, joyb);

                key_scan_p = strtok(keys, ",");
                do
                {
                  count = sscanf(key_scan_p, "%d,", &key);

                  if (count != 1)
                    break;

                  dsda_InputAddSpecificKey(index, input_defs[i].identifier, key);

                  key_scan_p = strtok(NULL, ",");
                } while (key_scan_p);

                index++;
                config_scan_p = strchr(config_scan_p, '|');
                if (config_scan_p)
                  config_scan_p++;
              } while (config_scan_p && index < DSDA_INPUT_PROFILE_COUNT);

              break;
            }
        }
      }
    }

    DG_close (f);
  }

  Z_Free(strparm);
  Z_Free(cfgline);

  dsda_ApplyAdHocConfiguration();

  dsda_InitSettings();

  //e6y: Check on existence of dsda-doom.wad
  port_wad_file = I_RequireFile(WAD_DATA, "");
}

//
// M_ScreenShot
//
// Modified by Lee Killough so that any number of shots can be taken,
// the code is faster, and no annoying "screenshot" message appears.

// CPhipps - modified to use its own buffer for the image
//         - checks for the case where no file can be created (doesn't occur on POSIX systems, would on DOS)
//         - track errors better
//         - split into 2 functions

//
// M_DoScreenShot
// Takes a screenshot into the names file

void M_DoScreenShot (const char* fname)
{
  if (I_ScreenShot(fname) != 0)
    doom_printf("M_ScreenShot: Error writing screenshot\n");
}

#ifndef SCREENSHOT_DIR
#define SCREENSHOT_DIR "."
#endif

#ifdef HAVE_LIBSDL2_IMAGE
#define SCREENSHOT_EXT ".png"
#else
#define SCREENSHOT_EXT ".bmp"
#endif

const char* M_CheckWritableDir(const char *dir)
{
  static char *base = NULL;
  static int base_len = 0;

  const char *result = NULL;
  int len;

  if (!dir || !(len = strlen(dir)))
  {
    return NULL;
  }

  if (len + 1 > base_len)
  {
    base_len = len + 1;
    base = Z_Malloc(len + 1);
  }

  if (base)
  {
    strcpy(base, dir);

    if (base[len - 1] != '\\' && base[len - 1] != '/')
      strcat(base, "/");

    if (M_ReadWriteAccess(base))
    {
      base[strlen(base) - 1] = 0;
      result = base;
    }
  }

  return result;
}

void M_ScreenShot(void)
{
  static int shot;
  char       *lbmname = NULL;
  int        startshot;
  const char *shot_dir = NULL;
  dsda_arg_t *arg;
  int        success = 0;

  arg = dsda_Arg(dsda_arg_shotdir);
  if (arg->found)
    shot_dir = M_CheckWritableDir(arg->value.v_string);
  if (!shot_dir)
    shot_dir = M_CheckWritableDir(dsda_StringConfig(dsda_config_screenshot_dir));
  if (!shot_dir)
#ifdef _WIN32
    shot_dir = M_CheckWritableDir(I_DoomExeDir());
#else
    shot_dir = (M_WriteAccess(SCREENSHOT_DIR) ? SCREENSHOT_DIR : NULL);
#endif

  if (shot_dir)
  {
    startshot = shot; // CPhipps - prevent infinite loop

    do {
      int size = snprintf(NULL, 0, "%s/doom%02d" SCREENSHOT_EXT, shot_dir, shot);
      lbmname = Z_Realloc(lbmname, size+1);
      snprintf(lbmname, size+1, "%s/doom%02d" SCREENSHOT_EXT, shot_dir, shot);
      shot++;
    } while (M_FileExists(lbmname) && (shot != startshot) && (shot < 10000));

    if (!M_FileExists(lbmname))
    {
      S_StartVoidSound(gamemode==commercial ? sfx_radio : sfx_tink);
      M_DoScreenShot(lbmname); // cph
      success = 1;
    }
    Z_Free(lbmname);
    if (success) return;
  }

  doom_printf ("M_ScreenShot: Couldn't create screenshot");
  return;
}

// Safe string copy function that works like OpenBSD's strlcpy().
// Returns true if the string was not truncated.

dboolean M_StringCopy(char *dest, const char *src, size_t dest_size)
{
    size_t len;

    if (dest_size >= 1)
    {
        dest[dest_size - 1] = '\0';
        strncpy(dest, src, dest_size - 1);
    }
    else
    {
        return false;
    }

    len = strlen(dest);
    return src[len] == '\0';
}

// Safe string concat function that works like OpenBSD's strlcat().
// Returns true if string not truncated.

dboolean M_StringConcat(char *dest, const char *src, size_t dest_size)
{
    size_t offset;

    offset = strlen(dest);
    if (offset > dest_size)
    {
        offset = dest_size;
    }

    return M_StringCopy(dest + offset, src, dest_size - offset);
}

int M_StrToInt(const char *s, int *l)
{
  return (
    (sscanf(s, " 0x%x", l) == 1) ||
    (sscanf(s, " 0X%x", l) == 1) ||
    (sscanf(s, " 0%o", l) == 1) ||
    (sscanf(s, " %d", l) == 1)
  );
}

int M_StrToFloat(const char *s, float *f)
{
  return (
    (sscanf(s, " %f", f) == 1)
  );
}

int M_DoubleToInt(double x)
{
#ifdef __GNUC__
 double tmp = x;
 return (int)tmp;
#else
 return (int)x;
#endif
}

char* M_Strlwr(char* str)
{
  char* p;
  for (p=str; *p; p++) *p = tolower(*p);
  return str;
}

char* M_Strupr(char* str)
{
  char* p;
  for (p=str; *p; p++) *p = toupper(*p);
  return str;
}

char *M_StrRTrim(char *str)
{
  char *end;

  if (str)
  {
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace(*end))
    {
      end--;
    }

    // Write new null terminator
    *(end + 1) = 0;
  }

  return str;
}

void M_ArrayClear(array_t *data)
{
  data->count = 0;
}

void* M_ArrayGetNewItem(array_t *data, int itemsize)
{
  if (data->count + 1 >= data->capacity)
  {
    data->capacity = (data->capacity ? data->capacity * 2 : 128);
    data->data = Z_Realloc(data->data, data->capacity * itemsize);
  }

  data->count++;

  return (unsigned char*)data->data + (data->count - 1) * itemsize;
}
