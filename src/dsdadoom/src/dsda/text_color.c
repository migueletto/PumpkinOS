//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA Text Color
//

#include "doomdef.h"
#include "hu_lib.h"
#include "lprintf.h"
#include "w_wad.h"
#include "v_video.h"
#include "stricmp.h"

#include "dsda/utility.h"

#include "text_color.h"

typedef struct {
  const char* key;
  int color_range;
  char color_str[3];
} dsda_text_color_t;

dsda_text_color_t dsda_text_colors[] = {
  [dsda_tc_exhud_time_label] = { "exhud_time_label", CR_GRAY },
  [dsda_tc_exhud_level_time] = { "exhud_level_time", CR_GREEN },
  [dsda_tc_exhud_total_time] = { "exhud_total_time", CR_GOLD },
  [dsda_tc_exhud_demo_length] = { "exhud_demo_length", CR_BROWN },
  [dsda_tc_exhud_armor_zero] = { "exhud_armor_zero", CR_GRAY },
  [dsda_tc_exhud_armor_one] = { "exhud_armor_one", CR_GREEN },
  [dsda_tc_exhud_armor_two] = { "exhud_armor_two", CR_LIGHTBLUE },
  [dsda_tc_exhud_command_entry] = { "exhud_command_entry", CR_GRAY },
  [dsda_tc_exhud_command_queue] = { "exhud_command_queue", CR_GOLD },
  [dsda_tc_exhud_coords_base] = { "exhud_coords_base", CR_GREEN },
  [dsda_tc_exhud_coords_mf50] = { "exhud_coords_mf50", CR_GRAY },
  [dsda_tc_exhud_coords_sr40] = { "exhud_coords_sr40", CR_GREEN },
  [dsda_tc_exhud_coords_sr50] = { "exhud_coords_sr50", CR_LIGHTBLUE },
  [dsda_tc_exhud_coords_fast] = { "exhud_coords_fast", CR_RED },
  [dsda_tc_exhud_fps_bad] = { "exhud_fps_bad", CR_RED },
  [dsda_tc_exhud_fps_fine] = { "exhud_fps_fine", CR_GRAY },
  [dsda_tc_exhud_health_bad] = { "exhud_health_bad", CR_RED },
  [dsda_tc_exhud_health_warning] = { "exhud_health_warning", CR_GOLD },
  [dsda_tc_exhud_health_ok] = { "exhud_health_ok", CR_GREEN },
  [dsda_tc_exhud_health_super] = { "exhud_health_super", CR_LIGHTBLUE },
  [dsda_tc_exhud_line_close] = { "exhud_line_close", CR_GREEN },
  [dsda_tc_exhud_line_far] = { "exhud_line_far", CR_GRAY },
  [dsda_tc_exhud_line_special] = { "exhud_line_special", CR_GREEN },
  [dsda_tc_exhud_line_normal] = { "exhud_line_normal", CR_GRAY },
  [dsda_tc_exhud_mobj_alive] = { "exhud_mobj_alive", CR_GREEN },
  [dsda_tc_exhud_mobj_dead] = { "exhud_mobj_dead", CR_GRAY },
  [dsda_tc_exhud_player_damage] = { "exhud_player_damage", CR_GREEN },
  [dsda_tc_exhud_player_neutral] = { "exhud_player_neutral", CR_GRAY },
  [dsda_tc_exhud_ammo_label] = { "exhud_ammo_label", CR_GRAY },
  [dsda_tc_exhud_ammo_mana1] = { "exhud_ammo_mana1", CR_LIGHTBLUE },
  [dsda_tc_exhud_ammo_mana2] = { "exhud_ammo_mana2", CR_GREEN },
  [dsda_tc_exhud_ammo_value] = { "exhud_ammo_value", CR_GRAY },
  [dsda_tc_exhud_ammo_bad] = { "exhud_ammo_bad", CR_RED },
  [dsda_tc_exhud_ammo_warning] = { "exhud_ammo_warning", CR_GOLD },
  [dsda_tc_exhud_ammo_ok] = { "exhud_ammo_ok", CR_GREEN },
  [dsda_tc_exhud_ammo_full] = { "exhud_ammo_full", CR_LIGHTBLUE },
  [dsda_tc_exhud_render_label] = { "exhud_render_label", CR_GRAY },
  [dsda_tc_exhud_render_good] = { "exhud_render_good", CR_GOLD },
  [dsda_tc_exhud_render_bad] = { "exhud_render_bad", CR_RED },
  [dsda_tc_exhud_sector_active] = { "exhud_sector_active", CR_RED },
  [dsda_tc_exhud_sector_special] = { "exhud_sector_special", CR_GREEN },
  [dsda_tc_exhud_sector_normal] = { "exhud_sector_normal", CR_GRAY },
  [dsda_tc_exhud_speed_label] = { "exhud_speed_label", CR_GRAY },
  [dsda_tc_exhud_speed_slow] = { "exhud_speed_slow", CR_GOLD },
  [dsda_tc_exhud_speed_normal] = { "exhud_speed_normal", CR_GREEN },
  [dsda_tc_exhud_speed_fast] = { "exhud_speed_fast", CR_LIGHTBLUE },
  [dsda_tc_exhud_totals_label] = { "exhud_totals_label", CR_RED },
  [dsda_tc_exhud_totals_value] = { "exhud_totals_value", CR_GOLD },
  [dsda_tc_exhud_totals_max] = { "exhud_totals_max", CR_LIGHTBLUE },
  [dsda_tc_exhud_weapon_label] = { "exhud_weapon_label", CR_GRAY },
  [dsda_tc_exhud_weapon_owned] = { "exhud_weapon_owned", CR_GREEN },
  [dsda_tc_exhud_weapon_berserk] = { "exhud_weapon_berserk", CR_LIGHTBLUE },
  [dsda_tc_exhud_attempts] = { "exhud_attempts", CR_GRAY },
  [dsda_tc_exhud_event_split] = { "exhud_event_split", CR_GRAY },
  [dsda_tc_exhud_line_activation] = { "exhud_line_activation", CR_GRAY },
  [dsda_tc_exhud_local_time] = { "exhud_local_time", CR_GRAY },
  [dsda_tc_exhud_free_text] = { "exhud_free_text", CR_GRAY },
  [dsda_tc_hud_message] = { "hud_message", CR_DEFAULT },
  [dsda_tc_hud_secret_message] = { "hud_secret_message", CR_GOLD },
  [dsda_tc_map_coords] = { "map_coords", CR_GREEN },
  [dsda_tc_map_time_level] = { "map_time_level", CR_GRAY },
  [dsda_tc_map_time_total] = { "map_time_total", CR_GRAY },
  [dsda_tc_map_title] = { "map_title", CR_GOLD },
  [dsda_tc_map_totals_label] = { "map_totals_label", CR_RED },
  [dsda_tc_map_totals_value] = { "map_totals_value", CR_GRAY },
  [dsda_tc_map_totals_max] = { "map_totals_max", CR_LIGHTBLUE },
  [dsda_tc_inter_split_normal] = { "inter_split_normal", CR_GRAY },
  [dsda_tc_inter_split_good] = { "inter_split_good", CR_GREEN },
  [dsda_tc_menu_title] = { "menu_title", CR_GOLD },
  [dsda_tc_menu_label] = { "menu_label", CR_RED },
  [dsda_tc_menu_label_highlight] = { "menu_label_highlight", CR_BRICK },
  [dsda_tc_menu_label_edit] = { "menu_label_edit", CR_GRAY },
  [dsda_tc_menu_value] = { "menu_value", CR_GREEN },
  [dsda_tc_menu_value_highlight] = { "menu_value_highlight", CR_BRICK },
  [dsda_tc_menu_value_edit] = { "menu_value_edit", CR_GRAY },
  [dsda_tc_menu_info_highlight] = { "menu_info_highlight", CR_BRICK },
  [dsda_tc_menu_info_edit] = { "menu_info_edit", CR_GRAY },
  [dsda_tc_menu_warning] = { "menu_warning", CR_RED },
  [dsda_tc_stbar_health_bad] = { "stbar_health_bad", CR_RED },
  [dsda_tc_stbar_health_warning] = { "stbar_health_warning", CR_GOLD },
  [dsda_tc_stbar_health_ok] = { "stbar_health_ok", CR_GREEN },
  [dsda_tc_stbar_health_super] = { "stbar_health_super", CR_BLUE },
  [dsda_tc_stbar_armor_zero] = { "stbar_armor_zero", CR_GRAY },
  [dsda_tc_stbar_armor_one] = { "stbar_armor_one", CR_GREEN },
  [dsda_tc_stbar_armor_two] = { "stbar_armor_two", CR_BLUE },
  [dsda_tc_stbar_ammo_bad] = { "stbar_ammo_bad", CR_RED },
  [dsda_tc_stbar_ammo_warning] = { "stbar_ammo_warning", CR_GOLD },
  [dsda_tc_stbar_ammo_ok] = { "stbar_ammo_ok", CR_GREEN },
  [dsda_tc_stbar_ammo_full] = { "stbar_ammo_full", CR_BLUE },
  { NULL },
};

const char* dsda_TextColor(dsda_text_color_index_t i) {
  return dsda_text_colors[i].color_str;
}

int dsda_TextCR(dsda_text_color_index_t i) {
  return dsda_text_colors[i].color_range;
}

void dsda_LoadTextColor(void) {
  char* lump;
  char** lines;
  const char* line;
  int line_i;
  int color_range;
  char key[33] = { 0 };
  dsda_text_color_t* p;

  lump = W_ReadLumpToString(W_GetNumForName("DSDATC"));

  lines = dsda_SplitString(lump, "\n\r");

  for (line_i = 0; lines[line_i]; ++line_i) {
    line = lines[line_i];

    if (!line[0] || line[0] == '/')
      continue;

    if (sscanf(line, "%32s %d", key, &color_range) != 2)
      I_Error("DSDATC lump has unknown format! (%s)", line);

    for (p = dsda_text_colors; p->key; p++)
      if (!strcasecmp(key, p->key)) {
        p->color_range = color_range;
        break;
      }

    if (!p->key)
      I_Error("DSDATC lump has unknown key %s!", key);
  }

  for (p = dsda_text_colors; p->key; p++) {
    p->color_str[0] = '\x1b';
    p->color_str[1] = HUlib_Color(p->color_range);
  }

  Z_Free(lines);
  Z_Free(lump);
}

static const char* color_name_to_index[CR_LIMIT] = {
  "",
  "brick",
  "tan",
  "gray",
  "green",
  "brown",
  "gold",
  "red",
  "blue",
  "orange",
  "yellow",
  "light blue",
  "black",
  "purple",
  "white",
};

int dsda_ColorNameToIndex(const char* name) {
  int i;

  if (!name)
    return CR_DEFAULT;

  for (i = CR_DEFAULT + 1; i < CR_LIMIT; ++i)
    if (!stricmp(color_name_to_index[i], name))
      return i;

  return CR_DEFAULT;
}
