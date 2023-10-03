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
//	DSDA Stretch
//

#include "doomdef.h"
#include "doomtype.h"
#include "hu_stuff.h"
#include "r_main.h"
#include "st_stuff.h"
#include "v_video.h"

#include "dsda/configuration.h"
#include "dsda/font.h"

#include "stretch.h"

int wide_offsetx;
int wide_offset2x;
int wide_offsety;
int wide_offset2y;

int render_stretch_hud;

int patches_scalex;
int patches_scaley;

static cb_video_t video;
static cb_video_t video_stretch;
static cb_video_t video_full;
static cb_video_t video_ex_text;

static stretch_param_t* stretch_params;
static stretch_param_t stretch_params_table[patch_stretch_max][VPT_ALIGN_MAX];

static int ex_text_screenwidth;
static int ex_text_screenheight;
static int ex_text_st_scaled_height;
static double ex_text_scale_x;
static double ex_text_scale_y;


static void GenLookup(short* lookup1, short* lookup2, int size, int max, int step) {
  int i;
  fixed_t frac, lastfrac;

  memset(lookup1, 0, max * sizeof(lookup1[0]));
  memset(lookup2, 0, max * sizeof(lookup2[0]));

  lastfrac = frac = 0;

  for(i = 0; i < size; i++) {
    if(frac >> FRACBITS > lastfrac >> FRACBITS) {
      lookup1[frac >> FRACBITS] = i;
      lookup2[lastfrac >> FRACBITS] = i - 1;

      lastfrac = frac;
    }

    frac += step;
  }

  lookup2[max - 1] = size - 1;
  lookup1[max] = lookup2[max] = size;

  for(i = 1; i < max; i++) {
    if (lookup1[i] == 0 && lookup1[i - 1] != 0)
      lookup1[i] = lookup1[i - 1];

    if (lookup2[i] == 0 && lookup2[i - 1] != 0)
      lookup2[i] = lookup2[i - 1];
  }
}

static void EvaluateExTextScale(void) {
  ex_text_scale_x = dsda_IntConfig(dsda_config_ex_text_scale_x) / 100.0;
  ex_text_scale_y = dsda_IntConfig(dsda_config_ex_text_ratio_y) / 100.0;

  if (!ex_text_scale_x)
    ex_text_scale_x = (double) WIDE_SCREENWIDTH / 320;

  if (!ex_text_scale_y)
    ex_text_scale_y = 1.0;

  ex_text_scale_y *= ex_text_scale_x;

  ex_text_screenwidth = 320 * ex_text_scale_x;
  ex_text_screenheight = 200 * ex_text_scale_y;
  ex_text_st_scaled_height = g_st_height * ex_text_scale_y;
}

stretch_param_t* dsda_StretchParams(int flags) {
  if (flags & VPT_EX_TEXT)
    return &stretch_params_table[patch_stretch_ex_text][flags & VPT_ALIGN_MASK];

  return &stretch_params[flags & VPT_ALIGN_MASK];
}

static void InitExTextParam(stretch_param_t* offsets, enum patch_translation_e flags) {
  int offset2x, offset2y;

  offset2x = SCREENWIDTH - ex_text_screenwidth;
  offset2y = (SCREENHEIGHT - ex_text_screenheight) -
             R_PartialView() * (ST_SCALED_HEIGHT - ex_text_st_scaled_height);

  memset(offsets, 0, sizeof(*offsets));

  offsets->video = &video_ex_text;

  if (flags == VPT_ALIGN_LEFT || flags == VPT_ALIGN_LEFT_BOTTOM || flags == VPT_ALIGN_LEFT_TOP) {
    offsets->deltax1 = 0;
    offsets->deltax2 = 0;
  }

  if (flags == VPT_ALIGN_RIGHT || flags == VPT_ALIGN_RIGHT_BOTTOM || flags == VPT_ALIGN_RIGHT_TOP) {
    offsets->deltax1 = offset2x;
    offsets->deltax2 = offset2x;
  }

  if (flags == VPT_ALIGN_BOTTOM || flags == VPT_ALIGN_LEFT_BOTTOM || flags == VPT_ALIGN_RIGHT_BOTTOM)
    offsets->deltay1 = offset2y;
}

void dsda_UpdateExTextOffset(enum patch_translation_e flags, int offset) {
  stretch_params_table[patch_stretch_ex_text][flags].deltay1 +=
    (ST_SCALED_HEIGHT - ex_text_st_scaled_height) * offset * hud_font.line_height / g_st_height +
    (hud_font.line_height - exhud_font.line_height) * ex_text_scale_y * (offset > 0 ? 1 : -1);
}

void dsda_ResetExTextOffsets(void) {
  int k;

  for (k = 0; k < VPT_ALIGN_MAX; k++)
    InitExTextParam(&stretch_params_table[patch_stretch_ex_text][k], k);
}

static void InitStretchParam(stretch_param_t* offsets, int stretch, enum patch_translation_e flags) {
  memset(offsets, 0, sizeof(*offsets));

  switch (stretch) {
    case patch_stretch_not_adjusted:
      if (flags == VPT_ALIGN_WIDE) {
        offsets->video = &video_stretch;
        offsets->deltax1 = (SCREENWIDTH - WIDE_SCREENWIDTH) / 2;
        offsets->deltax2 = (SCREENWIDTH - WIDE_SCREENWIDTH) / 2;
      }
      else {
        offsets->video = &video;
        offsets->deltax1 = wide_offsetx;
        offsets->deltax2 = wide_offsetx;
      }
      break;
    case patch_stretch_doom_format:
      offsets->video = &video_stretch;
      offsets->deltax1 = (SCREENWIDTH - WIDE_SCREENWIDTH) / 2;
      offsets->deltax2 = (SCREENWIDTH - WIDE_SCREENWIDTH) / 2;
      break;
    case patch_stretch_fit_to_width:
      offsets->video = &video_full;
      offsets->deltax1 = 0;
      offsets->deltax2 = 0;
      break;
  }

  if (flags == VPT_ALIGN_LEFT || flags == VPT_ALIGN_LEFT_BOTTOM || flags == VPT_ALIGN_LEFT_TOP) {
    offsets->deltax1 = 0;
    offsets->deltax2 = 0;
  }

  if (flags == VPT_ALIGN_RIGHT || flags == VPT_ALIGN_RIGHT_BOTTOM || flags == VPT_ALIGN_RIGHT_TOP) {
    offsets->deltax1 *= 2;
    offsets->deltax2 *= 2;
  }

  offsets->deltay1 = wide_offsety;

  if (flags == VPT_ALIGN_BOTTOM || flags == VPT_ALIGN_LEFT_BOTTOM || flags == VPT_ALIGN_RIGHT_BOTTOM)
    offsets->deltay1 = wide_offset2y;

  if (flags == VPT_ALIGN_TOP || flags == VPT_ALIGN_LEFT_TOP || flags == VPT_ALIGN_RIGHT_TOP)
    offsets->deltay1 = 0;

  if (flags == VPT_ALIGN_WIDE && !tallscreen)
    offsets->deltay1 = 0;
}

void dsda_SetupStretchParams(void) {
  int i, k;

  EvaluateExTextScale();

  for (k = 0; k < VPT_ALIGN_MAX; k++)
    for (i = 0; i < patch_stretch_max_config; i++)
      InitStretchParam(&stretch_params_table[i][k], i, k);

  dsda_ResetExTextOffsets();

  stretch_params = stretch_params_table[render_stretch_hud];

  video.xstep = ((320 << FRACBITS) / 320 / patches_scalex) + 1;
  video.ystep = ((200 << FRACBITS) / 200 / patches_scaley) + 1;
  video_stretch.xstep = ((320 << FRACBITS) / WIDE_SCREENWIDTH) + 1;
  video_stretch.ystep = ((200 << FRACBITS) / WIDE_SCREENHEIGHT) + 1;
  video_full.xstep = ((320 << FRACBITS) / SCREENWIDTH) + 1;
  video_full.ystep = ((200 << FRACBITS) / SCREENHEIGHT) + 1;
  video_ex_text.xstep = ((320 << FRACBITS) / ex_text_screenwidth) + 1;
  video_ex_text.ystep = ((200 << FRACBITS) / ex_text_screenheight) + 1;

  video.width = 320 * patches_scalex;
  video.height = 200 * patches_scaley;
  GenLookup(video.x1lookup, video.x2lookup, video.width, 320, video.xstep);
  GenLookup(video.y1lookup, video.y2lookup, video.height, 200, video.ystep);

  video_stretch.width = WIDE_SCREENWIDTH;
  video_stretch.height = WIDE_SCREENHEIGHT;
  GenLookup(video_stretch.x1lookup, video_stretch.x2lookup, video_stretch.width, 320, video_stretch.xstep);
  GenLookup(video_stretch.y1lookup, video_stretch.y2lookup, video_stretch.height, 200, video_stretch.ystep);

  video_full.width = SCREENWIDTH;
  video_full.height = SCREENHEIGHT;
  GenLookup(video_full.x1lookup, video_full.x2lookup, video_full.width, 320, video_full.xstep);
  GenLookup(video_full.y1lookup, video_full.y2lookup, video_full.height, 200, video_full.ystep);

  video_ex_text.width = ex_text_screenwidth;
  video_ex_text.height = ex_text_screenheight;
  GenLookup(video_ex_text.x1lookup, video_ex_text.x2lookup, video_ex_text.width, 320, video_ex_text.xstep);
  GenLookup(video_ex_text.y1lookup, video_ex_text.y2lookup, video_ex_text.height, 200, video_ex_text.ystep);
}

void dsda_EvaluatePatchScale(void) {
  int render_patches_scalex;
  int render_patches_scaley;

  render_patches_scalex = dsda_IntConfig(dsda_config_render_patches_scalex);
  render_patches_scaley = dsda_IntConfig(dsda_config_render_patches_scaley);

  patches_scalex = MIN(SCREENWIDTH / 320, SCREENHEIGHT / 200);
  patches_scalex = MAX(1, patches_scalex);
  patches_scaley = patches_scalex;

  if (render_patches_scalex > 0)
    patches_scalex = MIN(render_patches_scalex, patches_scalex);

  if (render_patches_scaley > 0)
    patches_scaley = MIN(render_patches_scaley, patches_scaley);

  ST_SCALED_HEIGHT = g_st_height * patches_scaley;

  if (SCREENWIDTH < 320 || WIDE_SCREENWIDTH < 320 ||
      SCREENHEIGHT < 200 || WIDE_SCREENHEIGHT < 200)
    render_stretch_hud = patch_stretch_fit_to_width;

  if (raven && render_stretch_hud == 0)
    render_stretch_hud++;

  switch (render_stretch_hud) {
    case patch_stretch_not_adjusted:
      wide_offset2x = SCREENWIDTH - patches_scalex * 320;
      wide_offset2y = SCREENHEIGHT - patches_scaley * 200;
      break;
    case patch_stretch_doom_format:
      ST_SCALED_HEIGHT = g_st_height * WIDE_SCREENHEIGHT / 200;

      wide_offset2x = SCREENWIDTH - WIDE_SCREENWIDTH;
      wide_offset2y = SCREENHEIGHT - WIDE_SCREENHEIGHT;
      break;
    case patch_stretch_fit_to_width:
      ST_SCALED_HEIGHT = g_st_height * SCREENHEIGHT / 200;

      wide_offset2x = 0;
      wide_offset2y = 0;
      break;
  }

  wide_offsetx = wide_offset2x / 2;
  wide_offsety = wide_offset2y / 2;
}
