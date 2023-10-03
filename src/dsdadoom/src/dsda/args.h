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
//	DSDA (Command Line) Args
//

#ifndef __DSDA_ARGS__
#define __DSDA_ARGS__

#include "doomtype.h"

typedef enum {
  dsda_arg_help,
  dsda_arg_iwad,
  dsda_arg_file,
  dsda_arg_deh,
  dsda_arg_playdemo,
  dsda_arg_timedemo,
  dsda_arg_fastdemo,
  dsda_arg_record,
  dsda_arg_recordfromto,
  dsda_arg_from_key_frame,
  dsda_arg_warp,
  dsda_arg_skill,
  dsda_arg_episode,
  dsda_arg_complevel,
  dsda_arg_fast,
  dsda_arg_respawn,
  dsda_arg_nomonsters,
  dsda_arg_longtics,
  dsda_arg_shorttics,
  dsda_arg_heretic,
  dsda_arg_hexen,
  dsda_arg_class,
  dsda_arg_randclass,
  dsda_arg_baddemo,
  dsda_arg_dsdademo,
  dsda_arg_solo_net,
  dsda_arg_coop_spawns,
  dsda_arg_pistolstart,
  dsda_arg_chain_episodes,
  dsda_arg_stroller,
  dsda_arg_turbo,
  dsda_arg_game_speed,
  dsda_arg_tas,
  dsda_arg_build,
  dsda_arg_quit_after_brute_force,
  dsda_arg_first_input,
  dsda_arg_command,
  dsda_arg_skipsec,
  dsda_arg_skiptic,
  dsda_arg_track_pacifist,
  dsda_arg_track_100k,
  dsda_arg_time_keys,
  dsda_arg_time_use,
  dsda_arg_time_secrets,
  dsda_arg_time_all,
  dsda_arg_track_player,
  dsda_arg_track_line,
  dsda_arg_track_line_distance,
  dsda_arg_track_sector,
  dsda_arg_track_mobj,
  dsda_arg_assign,
  dsda_arg_update,
  dsda_arg_analysis,
  dsda_arg_levelstat,
  dsda_arg_export_text_file,
  dsda_arg_export_ghost,
  dsda_arg_import_ghost,
  dsda_arg_consoleplayer,
  dsda_arg_spechit,
  dsda_arg_setmem,
  dsda_arg_data,
  dsda_arg_save,
  dsda_arg_config,
  dsda_arg_hud,
  dsda_arg_shotdir,
  dsda_arg_movie,
  dsda_arg_viddump,
  dsda_arg_dehout,
  dsda_arg_verbose,
  dsda_arg_quiet,
  dsda_arg_v,
  dsda_arg_resetgamma,
  dsda_arg_forceoldbsp,
  dsda_arg_force_old_zdoom_nodes,
  dsda_arg_sigsegv,
  dsda_arg_deathmatch,
  dsda_arg_altdeath,
  dsda_arg_timer,
  dsda_arg_frags,
  dsda_arg_nosound,
  dsda_arg_nomusic,
  dsda_arg_nosfx,
  dsda_arg_nodraw,
  dsda_arg_nodeh,
  dsda_arg_nomapinfo,
  dsda_arg_noautoload,
  dsda_arg_nocheats,
  dsda_arg_nojoy,
  dsda_arg_nomouse,
  dsda_arg_no_message_box,
  dsda_arg_fullscreen,
  dsda_arg_window,
  dsda_arg_width,
  dsda_arg_height,
  dsda_arg_geometry,
  dsda_arg_vidmode,
  dsda_arg_aspect,
  dsda_arg_emulate,
  dsda_arg_doom95,
  dsda_arg_blockmap,
  dsda_arg_force_monster_avoid_hazards,
  dsda_arg_force_remove_slime_trails,
  dsda_arg_force_no_dropoff,
  dsda_arg_force_truncated_sector_specials,
  dsda_arg_force_boom_brainawake,
  dsda_arg_force_prboom_friction,
  dsda_arg_reject_pad_with_ff,
  dsda_arg_force_lxdoom_demo_compatibility,
  dsda_arg_allow_ssg_direct,
  dsda_arg_treat_no_clipping_things_as_not_blocking,
  dsda_arg_force_incorrect_processing_of_respawn_frame_entry,
  dsda_arg_force_correct_code_for_3_keys_doors_in_mbf,
  dsda_arg_uninitialize_crush_field_for_stairs,
  dsda_arg_force_boom_findnexthighestfloor,
  dsda_arg_allow_sky_transfer_in_boom,
  dsda_arg_apply_green_armor_class_to_armor_bonuses,
  dsda_arg_apply_blue_armor_class_to_megasphere,
  dsda_arg_force_incorrect_bobbing_in_boom,
  dsda_arg_boom_deh_parser,
  dsda_arg_mbf_remove_thinker_in_killmobj,
  dsda_arg_do_not_inherit_friendlyness_flag_on_spawn,
  dsda_arg_do_not_use_misc12_frame_parameters_in_a_mushroom,
  dsda_arg_apply_mbf_codepointers_to_any_complevel,
  dsda_arg_reset_monsterspawner_params_after_loading,
  dsda_arg_debug_mapinfo,
  dsda_arg_hires,
  dsda_arg_count,
} dsda_arg_identifier_t;

typedef struct {
  int count;
  dboolean found;

  union {
    int v_int;
    int* v_int_array;
    const char* v_string;
    const char** v_string_array;
  } value;
} dsda_arg_t;

void dsda_ParseCommandLineArgs(int argc, char** argv);
dsda_arg_t* dsda_Arg(dsda_arg_identifier_t id);
dboolean dsda_Flag(dsda_arg_identifier_t id);
int dsda_SimpleIntArg(dsda_arg_identifier_t id);
void dsda_UpdateIntArg(dsda_arg_identifier_t id, const char* param);
void dsda_UpdateStringArg(dsda_arg_identifier_t id, const char* param);
void dsda_AppendStringArg(dsda_arg_identifier_t id, const char* param);
void dsda_UpdateFlag(dsda_arg_identifier_t id, dboolean found);
void dsda_PrintArgHelp(void);

#endif
