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

#ifndef __DSDA_STRETCH__
#define __DSDA_STRETCH__

typedef struct
{
   fixed_t xstep;
   fixed_t ystep;
   int width, height;
   short x1lookup[321];
   short y1lookup[201];
   short x2lookup[321];
   short y2lookup[201];
} cb_video_t;

typedef struct stretch_param_s
{
  cb_video_t *video;
  int deltax1;
  int deltay1;
  int deltax2;
  int deltay2;
} stretch_param_t;

typedef enum
{
  patch_stretch_not_adjusted,
  patch_stretch_doom_format,
  patch_stretch_fit_to_width,

  patch_stretch_max_config,

  patch_stretch_ex_text = patch_stretch_max_config,

  patch_stretch_max
} patch_stretch_t;

extern int wide_offsetx;
extern int wide_offset2x;
extern int wide_offsety;
extern int wide_offset2y;

extern int render_stretch_hud;

extern int patches_scalex;
extern int patches_scaley;

stretch_param_t* dsda_StretchParams(int flags);
void dsda_SetupStretchParams(void);
void dsda_EvaluatePatchScale(void);

#endif
