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
//	DSDA Features
//

#include "doomtype.h"

typedef enum {
  uf_menu,
  uf_exhud,
  uf_advhud,
  uf_crosshair,
  uf_quickstartcache,
  uf_100k,
  uf_console,
  // 7
  // 8
  // 9
  // 10
  // 11
  // 12
  // 13
  // 14
  // 15
  // 16
  // 17
  // 18
  // 19
  // 20
  // 21
  // 22
  // 23
  // 24
  // 25
  // 26
  // 27
  // 28
  // 29

  uf_unknown = 30,
  uf_invalid = 31,

  uf_iddt = 32,
  uf_automap,
  uf_liteamp,
  uf_build,
  uf_buildzero,
  uf_bruteforce,
  uf_tracker,
  uf_keyframe,
  uf_skip,
  uf_wipescreen,
  uf_speedup,
  uf_slowdown,
  uf_coordinates,
  uf_mouselook,
  uf_weaponalignment,
  uf_commanddisplay,
  uf_crosshaircolor,
  uf_crosshairlock,
  uf_shadows,
  uf_painpalette,
  uf_bonuspalette,
  uf_powerpalette,
  uf_healthbar,
  uf_alwayssr50,
  uf_maxplayercorpse,
  uf_hideweapon,
  uf_showalive,
  uf_join,
  uf_mouse_and_controller,
  uf_ghost,
  uf_advanced_map,
  // 63
} dsda_feature_flag_t;

#define FEATURE_SIZE 8

void dsda_TrackFeature(int feature);
void dsda_ResetFeatures(void);
uint64_t dsda_UsedFeatures(void);
void dsda_MergeFeatures(uint64_t source);
void dsda_CopyFeatures(byte* result);
void dsda_CopyFeatures2(byte* result, uint64_t source);
char* dsda_DescribeFeatures(void);
