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
//	DSDA HUD Component Base
//

#ifndef __DSDA_HUD_COMPONENT_BASE__
#define __DSDA_HUD_COMPONENT_BASE__

#include <stdio.h>
#include <math.h>

#include "am_map.h"
#include "doomdef.h"
#include "doomstat.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "m_menu.h"
#include "p_mobj.h"
#include "p_pspr.h"
#include "p_spec.h"
#include "p_tick.h"
#include "r_data.h"
#include "r_main.h"
#include "r_state.h"
#include "v_video.h"
#include "w_wad.h"

#include "dsda.h"
#include "dsda/demo.h"
#include "dsda/exhud.h"
#include "dsda/font.h"
#include "dsda/global.h"
#include "dsda/settings.h"
#include "dsda/text_color.h"
#include "dsda/utility.h"

#define DSDA_TEXT_SIZE 200
#define DSDA_CHAR_HEIGHT 8
#define DSDA_CHAR_WIDTH 6

typedef struct {
  hu_textline_t text;
  char msg[DSDA_TEXT_SIZE];
} dsda_text_t;

typedef struct {
  int x;
  int y;
  int vpt;
} dsda_patch_component_t;

int dsda_HudComponentY(int y_offset, int vpt, double ratio);
void dsda_InitTextHC(dsda_text_t* component, int x_offset, int y_offset, int vpt);
void dsda_InitBlockyHC(dsda_text_t* component, int x_offset, int y_offset, int vpt);
void dsda_InitPatchHC(dsda_patch_component_t* component, int x_offset, int y_offset, int vpt);
fixed_t dsda_HexenArmor(player_t* player);
int dsda_AmmoColor(player_t* player);
void dsda_DrawBigNumber(int x, int y, int delta_x, int delta_y, int cm, int vpt, int count, int n);
void dsda_DrawBasicText(dsda_text_t* component);
void dsda_RefreshHudText(dsda_text_t* component);

#endif
