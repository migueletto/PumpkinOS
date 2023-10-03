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
//	DSDA Coordinate Display HUD Component
//

#include "base.h"
#include "sys.h"

#include "coordinate_display.h"

#define THRESHOLD_1V 15.11
#define THRESHOLD_2V 19.35
#define THRESHOLD_3V 21.37

#define THRESHOLD_1D 16.67
#define THRESHOLD_2D 21.35
#define THRESHOLD_3D 23.58

typedef struct {
  dsda_text_t dsda_x_display;
  dsda_text_t dsda_y_display;
  dsda_text_t dsda_z_display;
  dsda_text_t dsda_a_display;
  dsda_text_t dsda_v_display;
  dsda_text_t dsda_vx_display;
  dsda_text_t dsda_vy_display;
  dsda_text_t dsda_d_display;
  dsda_text_t dsda_dx_display;
  dsda_text_t dsda_dy_display;
} local_component_t;

static local_component_t* local;

static const char* dsda_coordinate_color;
static const char* dsda_velocity_color;
static const char* dsda_distance_color;

static double dsda_CalculateVelocity(void) {
  double vx, vy;

  vx = (double) players[displayplayer].mo->momx / FRACUNIT;
  vy = (double) players[displayplayer].mo->momy / FRACUNIT;

  return sys_sqrt(vx * vx + vy * vy);
}

static double dsda_CalculateDistance(void) {
  double vx, vy;
  mobj_t* mo;

  mo = players[displayplayer].mo;

  vx = (double) (mo->x - mo->PrevX) / FRACUNIT;
  vy = (double) (mo->y - mo->PrevY) / FRACUNIT;

  return sys_sqrt(vx * vx + vy * vy);
}

static void dsda_WriteCoordinate(dsda_text_t* text, fixed_t x, const char* ch) {
  char str[FIXED_STRING_LENGTH];

  dsda_FixedToString(str, x);

  snprintf(text->msg, sizeof(text->msg), "%s%s: %s", dsda_coordinate_color, ch, str);

  dsda_RefreshHudText(text);
}

static void dsda_WriteAngle(dsda_text_t* text, angle_t x, const char* ch) {
  dsda_angle_t value;

  value = dsda_SplitAngle(x);

  if (value.frac)
    snprintf(text->msg, sizeof(text->msg), "%s%s: %i.%03i",
             dsda_coordinate_color, ch, value.base, value.frac);
  else
    snprintf(text->msg, sizeof(text->msg), "%s%s: %i",
             dsda_coordinate_color, ch, value.base);

  dsda_RefreshHudText(text);
}

static void dsda_WriteCoordinateSimple(dsda_text_t* text, fixed_t x, const char* ch, const char* color) {
  dsda_fixed_t value;

  value = dsda_SplitFixed(x);

  if (value.frac) {
    if (value.negative && !value.base)
      snprintf(text->msg, sizeof(text->msg), "%s%s: -%i.%03i",
               color, ch, value.base, 1000 * value.frac / 0xffff);
    else
      snprintf(text->msg, sizeof(text->msg), "%s%s: %i.%03i",
               color, ch, value.base, 1000 * value.frac / 0xffff);
  }
  else
    snprintf(text->msg, sizeof(text->msg), "%s%s: %i", color, ch, value.base);

  dsda_RefreshHudText(text);
}

static void dsda_WriteVelocity(dsda_text_t* text) {
  double v;

  v = dsda_CalculateVelocity();

  dsda_velocity_color =
    v >= THRESHOLD_3V ?
      dsda_TextColor(dsda_tc_exhud_coords_fast)   :
    v >= THRESHOLD_2V ?
      dsda_TextColor(dsda_tc_exhud_coords_sr50)  :
    v >= THRESHOLD_1V ?
      dsda_TextColor(dsda_tc_exhud_coords_sr40) :
    dsda_TextColor(dsda_tc_exhud_coords_mf50);

  if (v)
    snprintf(text->msg, sizeof(text->msg), "%sV: %.3f", dsda_velocity_color, v);
  else
    snprintf(text->msg, sizeof(text->msg), "%sV: 0", dsda_velocity_color);

  dsda_RefreshHudText(text);
}

static void dsda_WriteDistance(dsda_text_t* text) {
  double v;

  v = dsda_CalculateDistance();

  dsda_distance_color =
    v >= THRESHOLD_3D ?
      dsda_TextColor(dsda_tc_exhud_coords_fast)   :
    v >= THRESHOLD_2D ?
      dsda_TextColor(dsda_tc_exhud_coords_sr50)  :
    v >= THRESHOLD_1D ?
      dsda_TextColor(dsda_tc_exhud_coords_sr40) :
    dsda_TextColor(dsda_tc_exhud_coords_mf50);

  if (v)
    snprintf(text->msg, sizeof(text->msg), "%sD: %.3f", dsda_distance_color, v);
  else
    snprintf(text->msg, sizeof(text->msg), "%sD: 0", dsda_distance_color);

  dsda_RefreshHudText(text);
}

void dsda_InitCoordinateDisplayHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  dsda_coordinate_color = dsda_TextColor(dsda_tc_exhud_coords_base);

  dsda_InitTextHC(&local->dsda_x_display, x_offset, y_offset, vpt);
  dsda_InitTextHC(&local->dsda_y_display, x_offset, y_offset + 8, vpt);
  dsda_InitTextHC(&local->dsda_z_display, x_offset, y_offset + 16, vpt);
  dsda_InitTextHC(&local->dsda_a_display, x_offset, y_offset + 24, vpt);
  dsda_InitTextHC(&local->dsda_v_display, x_offset, y_offset + 40, vpt);
  dsda_InitTextHC(&local->dsda_vx_display, x_offset, y_offset + 48, vpt);
  dsda_InitTextHC(&local->dsda_vy_display, x_offset, y_offset + 56, vpt);
  dsda_InitTextHC(&local->dsda_d_display, x_offset, y_offset + 72, vpt);
  dsda_InitTextHC(&local->dsda_dx_display, x_offset, y_offset + 80, vpt);
  dsda_InitTextHC(&local->dsda_dy_display, x_offset, y_offset + 88, vpt);
}

void dsda_UpdateCoordinateDisplayHC(void* data) {
  mobj_t* mo;

  local = data;

  mo = players[displayplayer].mo;

  dsda_WriteCoordinate(&local->dsda_x_display, mo->x, "X");
  dsda_WriteCoordinate(&local->dsda_y_display, mo->y, "Y");
  dsda_WriteCoordinate(&local->dsda_z_display, mo->z, "Z");
  dsda_WriteAngle(&local->dsda_a_display, mo->angle, "A");
  dsda_WriteVelocity(&local->dsda_v_display);
  dsda_WriteCoordinateSimple(&local->dsda_vx_display, mo->momx, "X", dsda_velocity_color);
  dsda_WriteCoordinateSimple(&local->dsda_vy_display, mo->momy, "Y", dsda_velocity_color);
  dsda_WriteDistance(&local->dsda_d_display);
  dsda_WriteCoordinateSimple(&local->dsda_dx_display, mo->x - mo->PrevX, "X", dsda_distance_color);
  dsda_WriteCoordinateSimple(&local->dsda_dy_display, mo->y - mo->PrevY, "Y", dsda_distance_color);

  dsda_RefreshHudText(&local->dsda_x_display);
  dsda_RefreshHudText(&local->dsda_y_display);
  dsda_RefreshHudText(&local->dsda_z_display);
  dsda_RefreshHudText(&local->dsda_a_display);
  dsda_RefreshHudText(&local->dsda_v_display);
  dsda_RefreshHudText(&local->dsda_vx_display);
  dsda_RefreshHudText(&local->dsda_vy_display);
  dsda_RefreshHudText(&local->dsda_d_display);
  dsda_RefreshHudText(&local->dsda_dx_display);
  dsda_RefreshHudText(&local->dsda_dy_display);
}

void dsda_DrawCoordinateDisplayHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->dsda_x_display);
  dsda_DrawBasicText(&local->dsda_y_display);
  dsda_DrawBasicText(&local->dsda_z_display);
  dsda_DrawBasicText(&local->dsda_a_display);
  dsda_DrawBasicText(&local->dsda_v_display);
  dsda_DrawBasicText(&local->dsda_vx_display);
  dsda_DrawBasicText(&local->dsda_vy_display);
  dsda_DrawBasicText(&local->dsda_d_display);
  dsda_DrawBasicText(&local->dsda_dx_display);
  dsda_DrawBasicText(&local->dsda_dy_display);
}
