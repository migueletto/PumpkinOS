/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2004 by
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
 *      All the clipping: columns, horizontal spans, sky columns.
 *
 *-----------------------------------------------------------------------------*/
//
// 4/25/98, 5/2/98 killough: reformatted, beautified

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>

//#include "SDL.h"

#include "doomstat.h"
#include "p_spec.h"
#include "r_main.h"
#include "r_bsp.h"
#include "r_segs.h"
#include "r_plane.h"
#include "r_things.h"
#include "r_draw.h"
#include "w_wad.h"
#include "v_video.h"
#include "lprintf.h"

#include "dsda/mapinfo.h"
#include "dsda/render_stats.h"

// OPTIMIZE: closed two sided lines as single sided

// killough 1/6/98: replaced globals with statics where appropriate

// True if any of the segs textures might be visible.
static dboolean  segtextured;
static dboolean  markfloor;      // False if the back side is the same plane.
static dboolean  markceiling;
static dboolean  maskedtexture;
static int      toptexture;
static int      bottomtexture;
static int      midtexture;

static fixed_t  toptexheight, midtexheight, bottomtexheight; // cph

angle_t         rw_normalangle; // angle to line origin
int             rw_angle1;
fixed_t         rw_distance;
const lighttable_t    **walllights;

//
// regular wall
//
static int      rw_x;
static int      rw_stopx;
static angle_t  rw_centerangle;
static fixed_t  rw_offset;
static fixed_t  rw_scale;
static fixed_t  rw_scalestep;
static fixed_t  rw_midtexturemid;
static fixed_t  rw_toptexturemid;
static fixed_t  rw_bottomtexturemid;
static int      rw_lightlevel;
static int      worldtop;
static int      worldbottom;
static int      worldhigh;
static int      worldlow;
static int64_t  pixhigh; // R_WiggleFix
static int64_t  pixlow; // R_WiggleFix
static fixed_t  pixhighstep;
static fixed_t  pixlowstep;
static int64_t  topfrac; // R_WiggleFix
static fixed_t  topstep;
static int64_t  bottomfrac; // R_WiggleFix
static fixed_t  bottomstep;
static int      *maskedtexturecol; // dropoff overflow

static int	max_rwscale = 64 * FRACUNIT;
static int	HEIGHTBITS = 12;
static int	HEIGHTUNIT = (1 << 12);
static int	invhgtbits = 4;

/* cph - allow crappy fake contrast to be disabled */
fake_contrast_mode_t fake_contrast_mode;

//
// R_FixWiggle()
// Dynamic wall/texture rescaler, AKA "WiggleHack II"
//  by Kurt "kb1" Baumgardner ("kb")
//
//  [kb] When the rendered view is positioned, such that the viewer is
//   looking almost parallel down a wall, the result of the scale
//   calculation in R_ScaleFromGlobalAngle becomes very large. And, the
//   taller the wall, the larger that value becomes. If these large
//   values were used as-is, subsequent calculations would overflow
//   and crash the program.
//
//  Therefore, vanilla Doom clamps this scale calculation, preventing it
//   from becoming larger than 0x400000 (64*FRACUNIT). This number was
//   chosen carefully, to allow reasonably-tight angles, with reasonably
//   tall sectors to be rendered, within the limits of the fixed-point
//   math system being used. When the scale gets clamped, Doom cannot
//   properly render the wall, causing an undesirable wall-bending
//   effect that I call "floor wiggle".
//
//  Modern source ports offer higher video resolutions, which worsens
//   the issue. And, Doom is simply not adjusted for the taller walls
//   found in many PWADs.
//
//  WiggleHack II attempts to correct these issues, by dynamically
//   adjusting the fixed-point math, and the maximum scale clamp,
//   on a wall-by-wall basis. This has 2 effects:
//
//  1. Floor wiggle is greatly reduced and/or eliminated.
//  2. Overflow is not longer possible, even in levels with maximum
//     height sectors.
//
//  It is not perfect across all situations. Some floor wiggle can be
//   seen, and some texture strips may be slight misaligned in extreme
//   cases. These effects cannot be corrected without increasing the
//   precision of various renderer variables, and, possibly, suffering
//   a performance penalty.
//

void R_FixWiggle(sector_t *sec)
{
  static int  lastheight = 0;

  static const struct
  {
    int clamp;
    int heightbits;
  } scale_values[9] = {
    {2048 * FRACUNIT, 12}, {1024 * FRACUNIT, 12}, {1024 * FRACUNIT, 11},
    { 512 * FRACUNIT, 11}, { 512 * FRACUNIT, 10}, { 256 * FRACUNIT, 10},
    { 256 * FRACUNIT, 9},  { 128 * FRACUNIT, 9},  {  64 * FRACUNIT, 9},
  };

  int height = (sec->ceilingheight - sec->floorheight) >> FRACBITS;

  // disallow negative heights, force cache initialization
  if (height < 1)
    height = 1;

  // early out?
  if (height != lastheight)
  {
    lastheight = height;

    // initialize, or handle moving sector
    if (height != sec->cachedheight)
    {
      frontsector->cachedheight = height;
      frontsector->scaleindex = 0;
      height >>= 7;
      // calculate adjustment
      while ((height >>= 1))
        frontsector->scaleindex++;
    }

    // fine-tune renderer for this wall
    max_rwscale = scale_values[frontsector->scaleindex].clamp;
    HEIGHTBITS = scale_values[frontsector->scaleindex].heightbits;
    HEIGHTUNIT = 1 << HEIGHTBITS;
    invhgtbits = 16 - HEIGHTBITS;
  }
}

//
// R_ScaleFromGlobalAngle
// Returns the texture mapping scale
//  for the current line (horizontal span)
//  at the given angle.
// rw_distance must be calculated first.
//
// killough 5/2/98: reformatted, cleaned up
// CPhipps - moved here from r_main.c

static fixed_t R_ScaleFromGlobalAngle(angle_t visangle)
{
  int anglea = ANG90 + (visangle - viewangle);
  int angleb = ANG90 + (visangle - rw_normalangle);
  int den = FixedMul(rw_distance, finesine[anglea >> ANGLETOFINESHIFT]);
  // proff 11/06/98: Changed for high-res
  fixed_t num = FixedMul(projectiony, finesine[angleb >> ANGLETOFINESHIFT]);
  fixed_t scale;

  if (den > (num >> 16))
  {
    scale = FixedDiv(num, den);

    // [kb] use R_WiggleFix clamp
    if (scale > max_rwscale)
      scale = max_rwscale;
    else if (scale < 256)
      scale = 256;
  }
  else
    scale = max_rwscale;

  return scale;
}

const int fake_contrast_value = 16;

static dboolean R_FakeContrast(seg_t *seg)
{
  return fake_contrast_mode != FAKE_CONTRAST_MODE_OFF &&
         !(map_info.flags & MI_EVEN_LIGHTING) &&
         seg && !(seg->sidedef->flags & SF_NOFAKECONTRAST) && !hexen;
}

static dboolean R_SmoothLighting(seg_t *seg)
{
  return fake_contrast_mode == FAKE_CONTRAST_MODE_SMOOTH ||
         map_info.flags & MI_SMOOTH_LIGHTING ||
         seg->sidedef->flags & SF_SMOOTHLIGHTING;
}

void R_AddContrast(seg_t *seg, int *base_lightlevel)
{
  /* cph - ...what is this for? adding contrast to rooms?
   * It looks crap in outdoor areas */
  if (R_FakeContrast(seg))
  {
    if (seg->linedef->dy == 0)
    {
      *base_lightlevel -= fake_contrast_value;
    }
    else if (seg->linedef->dx == 0)
    {
      *base_lightlevel += fake_contrast_value;
    }
    else if (R_SmoothLighting(seg))
    {
      double dx, dy;

      dx = (double) seg->linedef->dx / FRACUNIT;
      dy = (double) seg->linedef->dy / FRACUNIT;

      *base_lightlevel +=
        lround(fabs(atan(dy / dx) * 2 / M_PI) * (2 * fake_contrast_value) - fake_contrast_value);
    }
  };
}

const lighttable_t** GetLightTable(int lightlevel)
{
  int lightnum;

  R_AddContrast(curline, &lightlevel);

  lightnum = (lightlevel >> LIGHTSEGSHIFT) + (extralight * LIGHTBRIGHT);

  return scalelight[BETWEEN(0, LIGHTLEVELS - 1, lightnum)];
}

static void R_UpdateWallLights(int lightlevel)
{
  walllights = GetLightTable(lightlevel);
}

static int R_SideLightLevel(side_t *side, int base_lightlevel)
{
  return side->lightlevel +
         ((side->flags & SF_LIGHTABSOLUTE) ? 0 : base_lightlevel);
}

int R_TopLightLevel(side_t *side, int base_lightlevel)
{
  return side->lightlevel_top +
         ((side->flags & SF_LIGHTABSOLUTETOP) ? 0 : R_SideLightLevel(side, base_lightlevel));
}

int R_MidLightLevel(side_t *side, int base_lightlevel)
{
  return side->lightlevel_mid +
         ((side->flags & SF_LIGHTABSOLUTEMID) ? 0 : R_SideLightLevel(side, base_lightlevel));
}

int R_BottomLightLevel(side_t *side, int base_lightlevel)
{
  return side->lightlevel_bottom +
         ((side->flags & SF_LIGHTABSOLUTEBOTTOM) ? 0 : R_SideLightLevel(side, base_lightlevel));
}

static void R_ApplyTopLight(side_t *side)
{
  int lightlevel;

  lightlevel = R_TopLightLevel(side, rw_lightlevel);

  R_UpdateWallLights(lightlevel);
}

static void R_ApplyMidLight(side_t *side)
{
  int lightlevel;

  lightlevel = R_MidLightLevel(side, rw_lightlevel);

  R_UpdateWallLights(lightlevel);
}

static void R_ApplyBottomLight(side_t *side)
{
  int lightlevel;

  lightlevel = R_BottomLightLevel(side, rw_lightlevel);

  R_UpdateWallLights(lightlevel);
}

static void R_ApplyLightColormap(draw_column_vars_t *dcvars, fixed_t scale)
{
  if (!fixedcolormap)
  {
    int index = (int)(((int64_t) scale * 160 / wide_centerx) >> LIGHTSCALESHIFT);
    if (index >= MAXLIGHTSCALE)
        index = MAXLIGHTSCALE - 1;

    dcvars->colormap = walllights[index];
  }
  else
  {
    dcvars->colormap = fixedcolormap;
  }
}

//
// R_RenderMaskedSegRange
//

void R_RenderMaskedSegRange(drawseg_t *ds, int x1, int x2)
{
  int      texnum;
  sector_t tempsec;      // killough 4/13/98
  const rpatch_t *patch;
  R_DrawColumn_f colfunc;
  draw_column_vars_t dcvars;

  R_SetDefaultDrawColumnVars(&dcvars);

  // Calculate light table.
  // Use different light tables
  //   for horizontal / vertical / diagonal. Diagonal?

  curline = ds->curline;  // OPTIMIZE: get rid of LIGHTSEGSHIFT globally

  // killough 4/11/98: draw translucent 2s normal textures

  colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_STANDARD, RDRAW_FILTER_POINT);
  if (curline->linedef->tranmap)
  {
    colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_TRANSLUCENT, RDRAW_FILTER_POINT);
    tranmap = curline->linedef->tranmap;
  }
  // killough 4/11/98: end translucent 2s normal code

  frontsector = curline->frontsector;
  backsector = curline->backsector;

  // cph 2001/11/25 - middle textures did not animate in v1.2
  texnum = curline->sidedef->midtexture;
  if (raven || !comp[comp_maskedanim])
    texnum = texturetranslation[texnum];

  // killough 4/13/98: get correct lightlevel for 2s normal textures
  rw_lightlevel = R_FakeFlat(frontsector, &tempsec, NULL, NULL, false) ->lightlevel;
  R_ApplyMidLight(curline->sidedef);

  maskedtexturecol = ds->maskedtexturecol;

  rw_scalestep = ds->scalestep;
  spryscale = ds->scale1 + (x1 - ds->x1)*rw_scalestep;
  mfloorclip = ds->sprbottomclip;
  mceilingclip = ds->sprtopclip;

  // find positioning
  if (curline->linedef->flags & ML_DONTPEGBOTTOM)
    {
      dcvars.texturemid = frontsector->floorheight > backsector->floorheight
        ? frontsector->floorheight : backsector->floorheight;
      dcvars.texturemid = dcvars.texturemid + textureheight[texnum] - viewz;
    }
  else
    {
      dcvars.texturemid =frontsector->ceilingheight<backsector->ceilingheight
        ? frontsector->ceilingheight : backsector->ceilingheight;
      dcvars.texturemid = dcvars.texturemid - viewz;
    }

  dcvars.texturemid += curline->sidedef->rowoffset + curline->sidedef->rowoffset_mid;

  patch = R_TextureCompositePatchByNum(texnum);

  // draw the columns
  for (dcvars.x = x1 ; dcvars.x <= x2 ; dcvars.x++, spryscale += rw_scalestep)
    if (maskedtexturecol[dcvars.x] != INT_MAX) // dropoff overflow
      {
        fixed_t texturecolumn;

        R_ApplyLightColormap(&dcvars, spryscale);

        // killough 3/2/98:
        //
        // This calculation used to overflow and cause crashes in Doom:
        //
        // sprtopscreen = centeryfrac - FixedMul(dcvars.texturemid, spryscale);
        //
        // This code fixes it, by using double-precision intermediate
        // arithmetic and by skipping the drawing of 2s normals whose
        // mapping to screen coordinates is totally out of range:

        {
          int64_t t = ((int64_t) centeryfrac << FRACBITS) -
            (int64_t) dcvars.texturemid * spryscale;
          if (t + (int64_t) textureheight[texnum] * spryscale < 0 ||
              t > (int64_t) SCREENHEIGHT << FRACBITS*2)
            continue;        // skip if the texture is out of screen's range
          sprtopscreen = (int64_t)(t >> FRACBITS); // R_WiggleFix
        }

        dcvars.iscale = 0xffffffffu / (unsigned) spryscale;

        texturecolumn = maskedtexturecol[dcvars.x] +
                        (curline->sidedef->textureoffset_mid >> FRACBITS);

        // killough 1/25/98: here's where Medusa came in, because
        // it implicitly assumed that the column was all one patch.
        // Originally, Doom did not construct complete columns for
        // multipatched textures, so there were no header or trailer
        // bytes in the column referred to below, which explains
        // the Medusa effect. The fix is to construct true columns
        // when forming multipatched textures (see r_data.c).

        // draw the texture
        R_DrawMaskedColumn(
          patch,
          colfunc,
          &dcvars,
          R_GetPatchColumnWrapped(patch, texturecolumn),
          R_GetPatchColumnWrapped(patch, texturecolumn-1),
          R_GetPatchColumnWrapped(patch, texturecolumn+1)
        );

        maskedtexturecol[dcvars.x] = INT_MAX; // dropoff overflow
      }

  curline = NULL; /* cph 2001/11/18 - must clear curline now we're done with it, so R_ColourMap doesn't try using it for other things */
}

//
// R_RenderSegLoop
// Draws zero, one, or two textures (and possibly a masked texture) for walls.
// Can draw or mark the starting pixel of floor and ceiling textures.
// CALLED: CORE LOOPING ROUTINE.
//

static int didsolidcol; /* True if at least one column was marked solid */

static void R_RenderSegLoop (void)
{
  const rpatch_t *tex_patch;
  R_DrawColumn_f colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_STANDARD, RDRAW_FILTER_POINT);
  draw_column_vars_t dcvars;
  fixed_t texturecolumn = 0;
  fixed_t specific_texturecolumn = 0;

  R_SetDefaultDrawColumnVars(&dcvars);

  dsda_RecordDrawSeg();

  for ( ; rw_x < rw_stopx ; rw_x++)
  {
    // mark floor / ceiling areas
    int yh = (int)(bottomfrac>>HEIGHTBITS);
    int yl = (int)((topfrac+HEIGHTUNIT-1)>>HEIGHTBITS);

    // no space above wall?
    int bottom,top = ceilingclip[rw_x]+1;

    if (yl < top)
      yl = top;

    if (markceiling)
    {
      bottom = yl-1;

      if (bottom >= floorclip[rw_x])
        bottom = floorclip[rw_x]-1;

      if (top <= bottom)
      {
        ceilingplane->top[rw_x] = top;
        ceilingplane->bottom[rw_x] = bottom;
      }
      // SoM: this should be set here
      ceilingclip[rw_x] = bottom;
    }

    bottom = floorclip[rw_x]-1;
    if (yh > bottom)
      yh = bottom;

    if (markfloor)
    {

      top  = yh < ceilingclip[rw_x] ? ceilingclip[rw_x] : yh;

      if (++top <= bottom)
      {
        floorplane->top[rw_x] = top;
        floorplane->bottom[rw_x] = bottom;
      }
      // SoM: This should be set here to prevent overdraw
      floorclip[rw_x] = top;
    }

    // texturecolumn and lighting are independent of wall tiers
    if (segtextured)
    {
      // calculate texture offset
      angle_t angle =(rw_centerangle+xtoviewangle[rw_x])>>ANGLETOFINESHIFT;

      texturecolumn = rw_offset-FixedMul(finetangent[angle],rw_distance);
      texturecolumn >>= FRACBITS;

      dcvars.x = rw_x;
      dcvars.iscale = 0xffffffffu / (unsigned)rw_scale;
    }

    // draw the wall tiers
    if (midtexture)
    {
      specific_texturecolumn = texturecolumn +
                               (curline->sidedef->textureoffset_mid >> FRACBITS);

      dcvars.yl = yl;     // single sided line
      dcvars.yh = yh;
      dcvars.texturemid = rw_midtexturemid;
      tex_patch = R_TextureCompositePatchByNum(midtexture);
      dcvars.source = R_GetTextureColumn(tex_patch, specific_texturecolumn);
      dcvars.prevsource = R_GetTextureColumn(tex_patch, specific_texturecolumn-1);
      dcvars.nextsource = R_GetTextureColumn(tex_patch, specific_texturecolumn+1);
      dcvars.texheight = midtexheight;
      if (!fixedcolormap)
        R_ApplyMidLight(curline->sidedef);
      R_ApplyLightColormap(&dcvars, rw_scale);
      colfunc(&dcvars);
      tex_patch = NULL;
      ceilingclip[rw_x] = viewheight;
      floorclip[rw_x] = -1;
    }
    else
    {
      // two sided line
      if (toptexture)
      {
        // top wall
        int mid = (int)(pixhigh>>HEIGHTBITS);
        pixhigh += pixhighstep;

        if (mid >= floorclip[rw_x])
          mid = floorclip[rw_x]-1;

        if (mid >= yl)
        {
          specific_texturecolumn = texturecolumn +
                                   (curline->sidedef->textureoffset_top >> FRACBITS);

          dcvars.yl = yl;
          dcvars.yh = mid;
          dcvars.texturemid = rw_toptexturemid;
          tex_patch = R_TextureCompositePatchByNum(toptexture);
          dcvars.source = R_GetTextureColumn(tex_patch,specific_texturecolumn);
          dcvars.prevsource = R_GetTextureColumn(tex_patch,specific_texturecolumn-1);
          dcvars.nextsource = R_GetTextureColumn(tex_patch,specific_texturecolumn+1);
          dcvars.texheight = toptexheight;
          if (!fixedcolormap)
            R_ApplyTopLight(curline->sidedef);
          R_ApplyLightColormap(&dcvars, rw_scale);
          colfunc(&dcvars);
          tex_patch = NULL;
          ceilingclip[rw_x] = mid;
        }
        else
          ceilingclip[rw_x] = yl-1;
      }
      else  // no top wall
      {
        if (markceiling)
          ceilingclip[rw_x] = yl-1;
      }

      if (bottomtexture)          // bottom wall
      {
        int mid = (int)((pixlow+HEIGHTUNIT-1)>>HEIGHTBITS);
        pixlow += pixlowstep;

        // no space above wall?
        if (mid <= ceilingclip[rw_x])
          mid = ceilingclip[rw_x]+1;

        if (mid <= yh)
        {
          specific_texturecolumn = texturecolumn +
                                   (curline->sidedef->textureoffset_bottom >> FRACBITS);

          dcvars.yl = mid;
          dcvars.yh = yh;
          dcvars.texturemid = rw_bottomtexturemid;
          tex_patch = R_TextureCompositePatchByNum(bottomtexture);
          dcvars.source = R_GetTextureColumn(tex_patch, specific_texturecolumn);
          dcvars.prevsource = R_GetTextureColumn(tex_patch, specific_texturecolumn-1);
          dcvars.nextsource = R_GetTextureColumn(tex_patch, specific_texturecolumn+1);
          dcvars.texheight = bottomtexheight;
          if (!fixedcolormap)
            R_ApplyBottomLight(curline->sidedef);
          R_ApplyLightColormap(&dcvars, rw_scale);
          colfunc(&dcvars);
          tex_patch = NULL;
          floorclip[rw_x] = mid;
        }
        else
          floorclip[rw_x] = yh+1;
      }
      else        // no bottom wall
      {
        if (markfloor)
          floorclip[rw_x] = yh+1;
      }

      // cph - if we completely blocked further sight through this column,
      // add this info to the solid columns array for r_bsp.c
      if ((markceiling || markfloor) && (floorclip[rw_x] <= ceilingclip[rw_x] + 1))
      {
        solidcol[rw_x] = 1; didsolidcol = 1;
      }

      // save texturecol for backdrawing of masked mid texture
      if (maskedtexture)
        maskedtexturecol[rw_x] = texturecolumn;
    }

    rw_scale += rw_scalestep;
    topfrac += topstep;
    bottomfrac += bottomstep;
  }
}

//
// R_StoreWallRange
// A wall segment will be drawn
//  between start and stop pixels (inclusive).
//
void R_StoreWallRange(const int start, const int stop)
{
  const int shift_bits = 1;
  int64_t dx, dy, dx1, dy1, len, dist;

  if (ds_p == drawsegs+maxdrawsegs)   // killough 1/98 -- fix 2s line HOM
  {
    unsigned pos = ds_p - drawsegs; // jff 8/9/98 fix from ZDOOM1.14a
    unsigned newmax = maxdrawsegs ? maxdrawsegs*2 : 128; // killough
    drawsegs = Z_Realloc(drawsegs,newmax*sizeof(*drawsegs));
    ds_p = drawsegs + pos;          // jff 8/9/98 fix from ZDOOM1.14a
    maxdrawsegs = newmax;
  }

  if(curline->linedef)
    curline->linedef->flags |= ML_MAPPED;

#if 0
  if (V_IsOpenGLMode())
  {
    // proff 11/99: the rest of the calculations is not needed for OpenGL
    ds_p++->curline = curline;
    gld_AddWall(curline);

    return;
  }
#endif

#ifdef RANGECHECK
  if (start >=viewwidth || start > stop)
    I_Error ("Bad R_RenderWallRange: %i to %i", start , stop);
#endif

  sidedef = curline->sidedef;
  linedef = curline->linedef;

  // mark the segment as visible for auto map
  linedef->flags |= ML_MAPPED;

  // calculate rw_distance for scale calculation
  rw_normalangle = curline->pangle + ANG90; // [crispy] use re-calculated angle

  // [Linguica] Fix long wall error
  // shift right to avoid possibility of int64 overflow in rw_distance calculation
  dx = ((int64_t)curline->v2->px - curline->v1->px) >> shift_bits;
  dy = ((int64_t)curline->v2->py - curline->v1->py) >> shift_bits;
  dx1 = ((int64_t)viewx - curline->v1->px) >> shift_bits;
  dy1 = ((int64_t)viewy - curline->v1->py) >> shift_bits;
  len = curline->halflength; // No need to shift

  dist = (((dy * dx1 - dx * dy1) / len) << shift_bits);
  rw_distance = (fixed_t)BETWEEN(INT_MIN, INT_MAX, dist);

  ds_p->x1 = rw_x = start;
  ds_p->x2 = stop;
  ds_p->curline = curline;
  rw_stopx = stop+1;

  {     // killough 1/6/98, 2/1/98: remove limit on openings
    extern int *openings; // dropoff overflow
    extern size_t maxopenings;
    size_t pos = lastopening - openings;
    size_t need = (rw_stopx - start)*sizeof(*lastopening) + pos;
    if (need > maxopenings)
    {
      drawseg_t *ds;                //jff 8/9/98 needed for fix from ZDoom
      int *oldopenings = openings; // dropoff overflow
      int *oldlast = lastopening; // dropoff overflow

      do
        maxopenings = maxopenings ? maxopenings*2 : 16384;
      while (need > maxopenings);
      openings = Z_Realloc(openings, maxopenings * sizeof(*openings));
      lastopening = openings + pos;

      // jff 8/9/98 borrowed fix for openings from ZDOOM1.14
      // [RH] We also need to adjust the openings pointers that
      //    were already stored in drawsegs.
      for (ds = drawsegs; ds < ds_p; ds++)
      {
#define ADJUST(p) if (ds->p + ds->x1 >= oldopenings && ds->p + ds->x1 <= oldlast)\
                    ds->p = ds->p - oldopenings + openings;
        ADJUST (maskedtexturecol);
        ADJUST (sprtopclip);
        ADJUST (sprbottomclip);
      }
#undef ADJUST
    }
  }  // killough: end of code to remove limits on openings

  worldtop = frontsector->ceilingheight - viewz;
  worldbottom = frontsector->floorheight - viewz;

  R_FixWiggle(frontsector);

  // calculate scale at both ends and step

  ds_p->scale1 = rw_scale =
    R_ScaleFromGlobalAngle (viewangle + xtoviewangle[start]);

  if (stop > start)
  {
    ds_p->scale2 = R_ScaleFromGlobalAngle (viewangle + xtoviewangle[stop]);
    ds_p->scalestep = rw_scalestep = (ds_p->scale2-rw_scale) / (stop-start);
  }
  else
    ds_p->scale2 = ds_p->scale1;

  // calculate texture boundaries
  //  and decide if floor / ceiling marks are needed

  midtexture = toptexture = bottomtexture = maskedtexture = 0;
  ds_p->maskedtexturecol = NULL;

  if (!backsector)
  {
    // single sided line
    midtexture = texturetranslation[sidedef->midtexture];
    midtexheight = (linedef->r_flags & RF_MID_TILE) ? 0 : textureheight[midtexture] >> FRACBITS;

    // a single sided line is terminal, so it must mark ends
    markfloor = markceiling = true;

    if (linedef->flags & ML_DONTPEGBOTTOM)
    {         // bottom of texture at bottom
      fixed_t vtop = frontsector->floorheight +
        textureheight[sidedef->midtexture];
      rw_midtexturemid = vtop - viewz;
    }
    else        // top of texture at top
      rw_midtexturemid = worldtop;

    rw_midtexturemid += FixedMod(sidedef->rowoffset + sidedef->rowoffset_mid,
                                 textureheight[midtexture]);

    ds_p->silhouette = SIL_BOTH;
    ds_p->sprtopclip = screenheightarray;
    ds_p->sprbottomclip = negonearray;
    ds_p->bsilheight = INT_MAX;
    ds_p->tsilheight = INT_MIN;
  }
  else      // two sided line
  {
    ds_p->sprtopclip = ds_p->sprbottomclip = NULL;
    ds_p->silhouette = 0;

    if (linedef->r_flags & RF_CLOSED)
    { /* cph - closed 2S line e.g. door */
      // cph - killough's (outdated) comment follows - this deals with both
      // "automap fixes", his and mine
      // killough 1/17/98: this test is required if the fix
      // for the automap bug (r_bsp.c) is used, or else some
      // sprites will be displayed behind closed doors. That
      // fix prevents lines behind closed doors with dropoffs
      // from being displayed on the automap.

      ds_p->silhouette = SIL_BOTH;
      ds_p->sprbottomclip = negonearray;
      ds_p->bsilheight = INT_MAX;
      ds_p->sprtopclip = screenheightarray;
      ds_p->tsilheight = INT_MIN;
    }
    else
    { /* not solid - old code */
      if (frontsector->floorheight > backsector->floorheight)
      {
        ds_p->silhouette = SIL_BOTTOM;
        ds_p->bsilheight = frontsector->floorheight;
      }
      else if (backsector->floorheight > viewz)
      {
        ds_p->silhouette = SIL_BOTTOM;
        ds_p->bsilheight = INT_MAX;
      }

      if (frontsector->ceilingheight < backsector->ceilingheight)
      {
        ds_p->silhouette |= SIL_TOP;
        ds_p->tsilheight = frontsector->ceilingheight;
      }
      else if (backsector->ceilingheight < viewz)
      {
        ds_p->silhouette |= SIL_TOP;
        ds_p->tsilheight = INT_MIN;
      }
    }

    worldhigh = backsector->ceilingheight - viewz;
    worldlow = backsector->floorheight - viewz;

    // hack to allow height changes in outdoor areas
    if (frontsector->ceilingpic == skyflatnum
        && backsector->ceilingpic == skyflatnum)
      worldtop = worldhigh;

    markfloor = worldlow != worldbottom
      || P_FloorPlanesDiffer(frontsector, backsector)

      // killough 4/15/98: prevent 2s normals
      // from bleeding through deep water
      || frontsector->heightsec != -1

      || (sidedef->midtexture && (sidedef->flags & SF_CLIPMIDTEX || linedef->flags & ML_CLIPMIDTEX))
      ;

    markceiling = worldhigh != worldtop
      || P_CeilingPlanesDiffer(frontsector, backsector)

      // killough 4/15/98: prevent 2s normals
      // from bleeding through fake ceilings
      || (frontsector->heightsec != -1 &&
          frontsector->ceilingpic!=skyflatnum)

      || (sidedef->midtexture && (sidedef->flags & SF_CLIPMIDTEX || linedef->flags & ML_CLIPMIDTEX))
      ;

    if (backsector->ceilingheight <= frontsector->floorheight
        || backsector->floorheight >= frontsector->ceilingheight)
      markceiling = markfloor = true;   // closed door

    if (worldhigh < worldtop)   // top texture
    {
      toptexture = texturetranslation[sidedef->toptexture];
      toptexheight = (linedef->r_flags & RF_TOP_TILE) ? 0 : textureheight[toptexture] >> FRACBITS;
      rw_toptexturemid = linedef->flags & ML_DONTPEGTOP ? worldtop :
        backsector->ceilingheight+textureheight[sidedef->toptexture]-viewz;
      rw_toptexturemid += FixedMod(sidedef->rowoffset + sidedef->rowoffset_top,
                                   textureheight[toptexture]);
    }

    if (worldlow > worldbottom) // bottom texture
    {
      bottomtexture = texturetranslation[sidedef->bottomtexture];
      bottomtexheight = (linedef->r_flags & RF_BOT_TILE) ? 0 : textureheight[bottomtexture] >> FRACBITS;
      rw_bottomtexturemid = linedef->flags & ML_DONTPEGBOTTOM ? worldtop :
        worldlow;
      rw_bottomtexturemid += FixedMod(sidedef->rowoffset + sidedef->rowoffset_bottom,
                                      textureheight[bottomtexture]);
    }

    // allocate space for masked texture tables
    if (sidedef->midtexture)    // masked midtexture
    {
      maskedtexture = true;
      ds_p->maskedtexturecol = maskedtexturecol = lastopening - rw_x;
      lastopening += rw_stopx - rw_x;
    }
  }

  // calculate rw_offset (only needed for textured lines)
  segtextured = midtexture | toptexture | bottomtexture | maskedtexture;

  if (segtextured)
  {
    rw_offset = (fixed_t)(((dx * dx1 + dy * dy1) / len) << shift_bits);

    rw_offset += sidedef->textureoffset + curline->offset;

    rw_centerangle = ANG90 + viewangle - rw_normalangle;

    rw_lightlevel = frontsector->lightlevel;
  }

  // if a floor / ceiling plane is on the wrong side of the view
  // plane, it is definitely invisible and doesn't need to be marked.

  // killough 3/7/98: add deep water check
  if (frontsector->heightsec == -1)
  {
    if (frontsector->floorheight >= viewz)       // above view plane
      markfloor = false;
    if (frontsector->ceilingheight <= viewz &&
        frontsector->ceilingpic != skyflatnum)   // below view plane
      markceiling = false;
  }

  // The original code compared values after shifting them right.
  // The code above here makes judgements based on the comparison before shifting.
  // In some cases (see: hexen floor waggles when the texture visibility is ~0),
  //   the discrepancy would cause textures to be drawn with the wrong height.
  {
    dboolean low_greater_than_bottom = (worldlow > worldbottom);
    dboolean high_less_than_top = (worldhigh < worldtop);

    // calculate incremental stepping values for texture edges
    worldtop >>= invhgtbits;
    worldbottom >>= invhgtbits;

    topstep = -FixedMul (rw_scalestep, worldtop);
    topfrac = ((int64_t)centeryfrac>>invhgtbits) - (((int64_t)worldtop*rw_scale)>>FRACBITS); // R_WiggleFix

    bottomstep = -FixedMul (rw_scalestep,worldbottom);
    bottomfrac = ((int64_t)centeryfrac>>invhgtbits) - (((int64_t)worldbottom*rw_scale)>>FRACBITS); // R_WiggleFix

    if (backsector)
    {
      worldhigh >>= invhgtbits;
      worldlow >>= invhgtbits;

      if (high_less_than_top)
      {
        pixhigh = ((int64_t)centeryfrac>>invhgtbits) - (((int64_t)worldhigh*rw_scale)>>FRACBITS); // R_WiggleFix
        pixhighstep = -FixedMul (rw_scalestep,worldhigh);
      }
      if (low_greater_than_bottom)
      {
        pixlow = ((int64_t)centeryfrac>>invhgtbits) - (((int64_t)worldlow*rw_scale)>>FRACBITS); // R_WiggleFix
        pixlowstep = -FixedMul (rw_scalestep,worldlow);
      }
    }
  }

  // render it
  if (markceiling)
  {
    if (ceilingplane)   // killough 4/11/98: add NULL ptr checks
      ceilingplane = R_CheckPlane (ceilingplane, rw_x, rw_stopx-1);
    else
      markceiling = 0;
  }

  if (markfloor)
  {
    if (floorplane)     // killough 4/11/98: add NULL ptr checks
    {
      /* cph 2003/04/18  - ceilingplane and floorplane might be the same
       * visplane (e.g. if both skies); R_CheckPlane doesn't know about
       * modifications to the plane that might happen in parallel with the check
       * being made, so we have to override it and split them anyway if that is
       * a possibility, otherwise the floor marking would overwrite the ceiling
       * marking, resulting in HOM. */
      if (markceiling && ceilingplane == floorplane)
	      floorplane = R_DupPlane (floorplane, rw_x, rw_stopx-1);
      else
	      floorplane = R_CheckPlane (floorplane, rw_x, rw_stopx-1);
    }
    else
    {
      markfloor = 0;
    }
  }

  didsolidcol = 0;
  R_RenderSegLoop();

  /* cph - if a column was made solid by this wall, we _must_ save full clipping info */
  if (backsector && didsolidcol)
  {
    if (!(ds_p->silhouette & SIL_BOTTOM))
    {
      ds_p->silhouette |= SIL_BOTTOM;
      ds_p->bsilheight = backsector->floorheight;
    }
    if (!(ds_p->silhouette & SIL_TOP))
    {
      ds_p->silhouette |= SIL_TOP;
      ds_p->tsilheight = backsector->ceilingheight;
    }
  }

  // save sprite clipping info
  if ((ds_p->silhouette & SIL_TOP || maskedtexture) && !ds_p->sprtopclip)
  {
    memcpy (lastopening, ceilingclip+start, sizeof(*lastopening)*(rw_stopx-start)); // dropoff overflow
    ds_p->sprtopclip = lastopening - start;
    lastopening += rw_stopx - start;
  }
  if ((ds_p->silhouette & SIL_BOTTOM || maskedtexture) && !ds_p->sprbottomclip)
  {
    memcpy (lastopening, floorclip+start, sizeof(*lastopening)*(rw_stopx-start)); // dropoff overflow
    ds_p->sprbottomclip = lastopening - start;
    lastopening += rw_stopx - start;
  }
  if (maskedtexture && !(ds_p->silhouette & SIL_TOP))
  {
    ds_p->silhouette |= SIL_TOP;
    ds_p->tsilheight = INT_MIN;
  }
  if (maskedtexture && !(ds_p->silhouette & SIL_BOTTOM))
  {
    ds_p->silhouette |= SIL_BOTTOM;
    ds_p->bsilheight = INT_MAX;
  }
  ds_p++;
}
