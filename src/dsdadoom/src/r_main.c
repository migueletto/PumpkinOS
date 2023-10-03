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
 *      Rendering main loop and setup functions,
 *       utility functions (BSP, geometry, trigonometry).
 *      See tables.c, too.
 *
 *-----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
//#include "SDL.h"

#include "doomstat.h"
#include "d_net.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_things.h"
#include "r_plane.h"
#include "r_bsp.h"
#include "r_draw.h"
#include "m_bbox.h"
#include "r_sky.h"
#include "v_video.h"
#include "lprintf.h"
#include "st_stuff.h"
#include "i_main.h"
#include "i_system.h"
#include "i_video.h"
#include "g_game.h"
#include "smooth.h"
#include "r_fps.h"
#include <math.h>
#include "e6y.h"//e6y
#include "xs_Float.h"

#include "sys.h"

#include "dsda/configuration.h"
#include "dsda/exhud.h"
#include "dsda/map_format.h"
#include "dsda/render_stats.h"
#include "dsda/settings.h"
#include "dsda/signal_context.h"
#include "dsda/stretch.h"
//#include "dsda/gl/render_scale.h"

#include "hexen/a_action.h"

// e6y
// Now they are variables. Depends from render_doom_lightmaps variable.
// Unify colour maping logic by cph is removed, because of bugs.
int LIGHTLEVELS   = 32;
int LIGHTSEGSHIFT = 3;
int LIGHTBRIGHT   = 2;

int r_frame_count;

// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW 2048

#define HEXEN_PI 3.141592657

int validcount = 1;         // increment every time a check is made
int validcount2 = 1;
const lighttable_t *fixedcolormap;
int      centerx, centery;
// e6y: wide-res
int wide_centerx;

fixed_t  focallength;
fixed_t  focallengthy;
fixed_t  globaluclip, globaldclip;
fixed_t  centerxfrac, centeryfrac;
fixed_t  yaspectmul;
fixed_t  viewheightfrac; //e6y: for correct cliping of things
fixed_t  projection;
// proff 11/06/98: Added for high-res
fixed_t  projectiony;
fixed_t  skyiscale;
fixed_t  viewx, viewy, viewz;
angle_t  viewangle;
fixed_t  viewcos, viewsin;
fixed_t  viewtancos, viewtansin;
player_t *viewplayer;
// e6y: Added for more precise flats drawing
fixed_t viewfocratio;

int r_nearclip = 5;

int FieldOfView;
int viewport[4];
float modelMatrix[16];
float projMatrix[16];

extern const lighttable_t **walllights;

//
// precalculated math tables
//

angle_t clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X.

int viewangletox[FINEANGLES/2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.

// e6y: resolution limitation is removed
angle_t *xtoviewangle;   // killough 2/8/98

// killough 3/20/98: Support dynamic colormaps, e.g. deep water
// killough 4/4/98: support dynamic number of them as well

int numcolormaps;
const lighttable_t *(*c_scalelight)[LIGHTLEVELS_MAX][MAXLIGHTSCALE];
const lighttable_t *(*c_zlight)[LIGHTLEVELS_MAX][MAXLIGHTZ];
const lighttable_t *(*scalelight)[MAXLIGHTSCALE];
const lighttable_t *(*zlight)[MAXLIGHTZ];
const lighttable_t *fullcolormap;
const lighttable_t **colormaps;

// killough 3/20/98, 4/4/98: end dynamic colormaps

//e6y: for Boom colormaps in OpenGL mode
dboolean use_boom_cm;
int boom_cm;         // current colormap
int frame_fixedcolormap = 0;

int extralight;                           // bumped light from gun blasts

//
// R_PointOnSide
// Traverse BSP (sub) tree,
//  check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
// killough 5/2/98: reformatted
//

// Workaround for optimization bug in clang
// fixes desync in competn/doom/fp2-3655.lmp and in dmnsns.wad dmn01m909.lmp
#if defined(__clang__)
PUREFUNC int R_CompatiblePointOnSide(volatile fixed_t x, volatile fixed_t y, const node_t *node)
#else
PUREFUNC int R_CompatiblePointOnSide(fixed_t x, fixed_t y, const node_t *node)
#endif
{
  if (!node->dx)
    return x <= node->x ? node->dy > 0 : node->dy < 0;

  if (!node->dy)
    return y <= node->y ? node->dx < 0 : node->dx > 0;

  x -= node->x;
  y -= node->y;

  // Try to quickly decide by looking at sign bits.
  if ((node->dy ^ node->dx ^ x ^ y) < 0)
    return (node->dy ^ x) < 0;  // (left is negative)
  return FixedMul(y, node->dx>>FRACBITS) >= FixedMul(node->dy>>FRACBITS, x);
}

#if defined(__clang__)
PUREFUNC int R_ZDoomPointOnSide(volatile fixed_t x, volatile fixed_t y, const node_t *node)
#else
PUREFUNC int R_ZDoomPointOnSide(fixed_t x, fixed_t y, const node_t *node)
#endif
{
  if (!node->dx)
    return x <= node->x ? node->dy > 0 : node->dy < 0;

  if (!node->dy)
    return y <= node->y ? node->dx < 0 : node->dx > 0;

  x -= node->x;
  y -= node->y;

  // Try to quickly decide by looking at sign bits.
  if ((node->dy ^ node->dx ^ x ^ y) < 0)
    return (node->dy ^ x) < 0;  // (left is negative)
  return (long long) y * node->dx >= (long long) x * node->dy;
}

int (*R_PointOnSide)(fixed_t x, fixed_t y, const node_t *node);

// killough 5/2/98: reformatted

PUREFUNC int R_CompatiblePointOnSegSide(fixed_t x, fixed_t y, const seg_t *line)
{
  fixed_t lx = line->v1->x;
  fixed_t ly = line->v1->y;
  fixed_t ldx = line->v2->x - lx;
  fixed_t ldy = line->v2->y - ly;

  if (!ldx)
    return x <= lx ? ldy > 0 : ldy < 0;

  if (!ldy)
    return y <= ly ? ldx < 0 : ldx > 0;

  x -= lx;
  y -= ly;

  // Try to quickly decide by looking at sign bits.
  if ((ldy ^ ldx ^ x ^ y) < 0)
    return (ldy ^ x) < 0;          // (left is negative)
  return FixedMul(y, ldx>>FRACBITS) >= FixedMul(ldy>>FRACBITS, x);
}

PUREFUNC int R_ZDoomPointOnSegSide(fixed_t x, fixed_t y, const seg_t *line)
{
  fixed_t lx = line->v1->x;
  fixed_t ly = line->v1->y;
  fixed_t ldx = line->v2->x - lx;
  fixed_t ldy = line->v2->y - ly;

  if (!ldx)
    return x <= lx ? ldy > 0 : ldy < 0;

  if (!ldy)
    return y <= ly ? ldx < 0 : ldx > 0;

  x -= lx;
  y -= ly;

  // Try to quickly decide by looking at sign bits.
  if ((ldy ^ ldx ^ x ^ y) < 0)
    return (ldy ^ x) < 0;          // (left is negative)
  return (long long) y * ldx >= (long long) x * ldy;
}

int (*R_PointOnSegSide)(fixed_t x, fixed_t y, const seg_t *line);

//
// R_PointToAngle
// To get a global angle from cartesian coordinates,
//  the coordinates are flipped until they are in
//  the first octant of the coordinate system, then
//  the y (<=x) is scaled and divided by x to get a
//  tangent (slope) value which is looked up in the
//  tantoangle[] table. The +1 size of tantoangle[]
//  is to handle the case when x==y without additional
//  checking.
//
// killough 5/2/98: reformatted, cleaned up

angle_t R_PointToAngleSlope(fixed_t x1, fixed_t y1, fixed_t x, fixed_t y, slope_div_fn slope_div)
{
  return (y -= y1, (x -= x1) || y) ?
    x >= 0 ?
      y >= 0 ?
        (x > y) ? tantoangle[slope_div(y,x)] :                      // octant 0
                ANG90-1-tantoangle[slope_div(x,y)] :                // octant 1
        x > (y = -y) ? 0-tantoangle[slope_div(y,x)] :               // octant 8
                       ANG270+tantoangle[slope_div(x,y)] :          // octant 7
      y >= 0 ? (x = -x) > y ? ANG180-1-tantoangle[slope_div(y,x)] : // octant 3
                            ANG90 + tantoangle[slope_div(x,y)] :    // octant 2
        (x = -x) > (y = -y) ? ANG180+tantoangle[slope_div(y,x)] :   // octant 4
                              ANG270-1-tantoangle[slope_div(x,y)] : // octant 5
    0;
}

angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x, fixed_t y)
{
  return R_PointToAngleSlope(x1, y1, x, y, SlopeDiv);
}

angle_t R_PointToAngleEx(fixed_t x, fixed_t y)
{
  return R_PointToAngleEx2(viewx, viewy, x, y);
}

angle_t R_PointToAngleEx2(fixed_t x1, fixed_t y1, fixed_t x, fixed_t y)
{
  // [crispy] fix overflows for very long distances
  int64_t y_viewy = (int64_t)y - y1;
  int64_t x_viewx = (int64_t)x - x1;

  // [crispy] the worst that could happen is e.g. INT_MIN-INT_MAX = 2*INT_MIN
  if (x_viewx < INT_MIN || x_viewx > INT_MAX ||y_viewy < INT_MIN || y_viewy > INT_MAX)
  {
    // [crispy] preserving the angle by halfing the distance in both directions
    x = (int)(x_viewx / 2 + x1);
    y = (int)(y_viewy / 2 + y1);
  }

  return R_PointToAngleSlope(x1, y1, x, y, SlopeDivEx);
}


//-----------------------------------------------------------------------------
//
// ! Returns the pseudoangle between the line p1 to (infinity, p1.y) and the
// line from p1 to p2. The pseudoangle has the property that the ordering of
// points by true angle anround p1 and ordering of points by pseudoangle are the
// same.
//
// For clipping exact angles are not needed. Only the ordering matters.
// This is about as fast as the fixed point R_PointToAngle2 but without
// the precision issues associated with that function.
//
//-----------------------------------------------------------------------------

angle_t R_PointToPseudoAngle (fixed_t x, fixed_t y)
{
  // Note: float won't work here as it's less precise than the BAM values being passed as parameters
  double vecx = (double)x - viewx;
  double vecy = (double)y - viewy;

  if (vecx == 0 && vecy == 0)
  {
    return 0;
  }
  else
  {
    double result = vecy / (fabs(vecx) + fabs(vecy));
    if (vecx < 0)
    {
      result = 2.0 - result;
    }
    return (angle_t)xs_CRoundToInt(result * (1 << 30));
  }
}

//
// R_InitTextureMapping
//
// killough 5/2/98: reformatted

static void R_InitTextureMapping (void)
{
  int i,x;
  FieldOfView = FIELDOFVIEW;

  // For widescreen displays, increase the FOV so that the middle part of the
  // screen that would be visible on a 4:3 display has the requested FOV.
  if (wide_centerx != centerx)
  { // wide_centerx is what centerx would be if the display was not widescreen
    FieldOfView = (int)(atan((double)centerx * tan((double)FieldOfView * M_PI / FINEANGLES) / (double)wide_centerx) * FINEANGLES / M_PI);
    if (FieldOfView > 160 * FINEANGLES / 360)
      FieldOfView = 160 * FINEANGLES / 360;
  }

  // Use tangent table to generate viewangletox:
  //  viewangletox will give the next greatest x
  //  after the view angle.
  //
  // Calc focallength
  //  so FIELDOFVIEW angles covers SCREENWIDTH.

  focallength = FixedDiv(centerxfrac, finetangent[FINEANGLES/4 + FieldOfView/2]);
  focallengthy = Scale(centerxfrac, yaspectmul, finetangent[FINEANGLES/4 + FieldOfView/2]);

  for (i=0 ; i<FINEANGLES/2 ; i++)
    {
      int t;
      int limit = finetangent[FINEANGLES/4 + FieldOfView/2];
      if (finetangent[i] > limit)
        t = -1;
      else
        if (finetangent[i] < -limit)
          t = viewwidth+1;
      else
        {
          t = FixedMul(finetangent[i], focallength);
          t = (centerxfrac - t + FRACUNIT-1) >> FRACBITS;
          if (t < -1)
            t = -1;
          else
            if (t > viewwidth+1)
              t = viewwidth+1;
        }
      viewangletox[i] = t;
    }

  // Scan viewangletox[] to generate xtoviewangle[]:
  //  xtoviewangle will give the smallest view angle
  //  that maps to x.

  for (x=0; x<=viewwidth; x++)
    {
      for (i=0; viewangletox[i] > x; i++)
        ;
      xtoviewangle[x] = (i<<ANGLETOFINESHIFT)-ANG90;
    }

  // Take out the fencepost cases from viewangletox.
  for (i=0; i<FINEANGLES/2; i++)
    if (viewangletox[i] == -1)
      viewangletox[i] = 0;
    else
      if (viewangletox[i] == viewwidth+1)
        viewangletox[i] = viewwidth;

  clipangle = xtoviewangle[0];
}

//
// R_InitLightTables
// Only inits the zlight table,
//  because the scalelight table changes with view size.
//

#define DISTMAP 2

static void R_InitLightTables (void)
{
  int i;
  int render_doom_lightmaps;

  // killough 4/4/98: dynamic colormaps
  c_zlight = Z_Malloc(sizeof(*c_zlight) * numcolormaps);
  c_scalelight = Z_Malloc(sizeof(*c_scalelight) * numcolormaps);

  // hexen_note: does hexen require render_doom_lightmaps?
  render_doom_lightmaps = dsda_IntConfig(dsda_config_render_doom_lightmaps);

  LIGHTLEVELS   = (render_doom_lightmaps ? 16 : 32);
  LIGHTSEGSHIFT = (render_doom_lightmaps ? 4 : 3);
  LIGHTBRIGHT   = (render_doom_lightmaps ? 1 : 2);

  // Calculate the light levels to use
  //  for each level / distance combination.
  for (i=0; i< LIGHTLEVELS; i++)
    {
      // SoM: the LIGHTBRIGHT constant must be used to scale the start offset of
      // the colormaps, otherwise the levels are staggered and become slightly
      // darker.
      int j, startmap = ((LIGHTLEVELS-LIGHTBRIGHT-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;
      for (j=0; j<MAXLIGHTZ; j++)
        {
    // CPhipps - use 320 here instead of SCREENWIDTH, otherwise hires is
    //           brighter than normal res
          int scale = FixedDiv ((320/2*FRACUNIT), (j+1)<<LIGHTZSHIFT);
          int t, level = startmap - (scale >>= LIGHTSCALESHIFT)/DISTMAP;

          if (level < 0)
            level = 0;
          else
            if (level >= NUMCOLORMAPS)
              level = NUMCOLORMAPS-1;

          // killough 3/20/98: Initialize multiple colormaps
          level *= 256;
          for (t=0; t<numcolormaps; t++)         // killough 4/4/98
            c_zlight[t][i][j] = colormaps[t] + level;
        }
    }
}

//
// R_SetViewSize
// Do not really change anything here,
//  because it might be in the middle of a refresh.
// The change will take effect next refresh.
//

dboolean setsizeneeded;
static int setblocks;

void R_SetViewSize(void)
{
  setsizeneeded = true;
  setblocks = dsda_IntConfig(dsda_config_screenblocks);
}

void R_MultMatrixVecd(const float matrix[16], const float in[4], float out[4])
{
  int i;

  for (i = 0; i < 4; i++)
  {
    out[i] =
      in[0] * matrix[0*4+i] +
      in[1] * matrix[1*4+i] +
      in[2] * matrix[2*4+i] +
      in[3] * matrix[3*4+i];
  }
}

int R_Project(float objx, float objy, float objz, float *winx, float *winy, float *winz)
{
  float in[4];
  float out[4];

  in[0] = objx;
  in[1] = objy;
  in[2] = objz;
  in[3] = 1.0f;

  R_MultMatrixVecd(modelMatrix, in, out);
  R_MultMatrixVecd(projMatrix, out, in);

  if (in[3] == 0.0f)
    return false;

  in[0] /= in[3];
  in[1] /= in[3];
  in[2] /= in[3];

  /* Map x, y and z to range 0-1 */
  in[0] = in[0] * 0.5f + 0.5f;
  in[1] = in[1] * 0.5f + 0.5f;
  in[2] = in[2] * 0.5f + 0.5f;

  /* Map x,y to viewport */
  in[0] = in[0] * viewport[2] + viewport[0];
  in[1] = in[1] * viewport[3] + viewport[1];

  *winx = in[0];
  *winy = in[1];
  *winz = in[2];

  return true;
}

void R_SetupViewport(void)
{
  viewport[0] = 0;
  viewport[1] = (SCREENHEIGHT - viewheight) / 2;
  viewport[2] = viewwidth;
  viewport[3] = SCREENHEIGHT;
}

void R_SetupPerspective(float fovy, float aspect, float znear)
{
  int i;
  float focallength = 1.0f / (float)tan(fovy * (float)M_PI / 360.0f);
  float *m = projMatrix;

  for (i = 0; i < 16; i++)
    m[i] = 0.0f;

  m[0] = focallength / aspect;
  m[5] = focallength;
  m[10] = -1;
  m[11] = -1;
  m[14] = -2 * znear;
}

void R_BuildModelViewMatrix(void)
{
  float x, y, z;
  float yaw, pitch;
  float A, B, C, D;
  float *m = modelMatrix;

  yaw = 270.0f - (float)(viewangle>>ANGLETOFINESHIFT) * 360.0f / FINEANGLES;
  yaw *= (float)M_PI / 180.0f;
  pitch = 0;
#if 0
  if (V_IsOpenGLMode())
  {
    pitch = (float)(viewpitch>>ANGLETOFINESHIFT) * 360.0f / FINEANGLES;
    pitch *= (float)M_PI / 180.0f;
  }
#endif

  x =  (float)viewx / MAP_SCALE;
  z = -(float)viewy / MAP_SCALE;
  y = -(float)viewz / MAP_SCALE;

/*
  R_LoadIdentity(modelMatrix);
  R_Rotate(modelMatrix, pitch, 0);
  R_Rotate(modelMatrix, yaw, 1);
  R_Translate(modelMatrix, x, y, z);
*/

  A = (float)sys_cos(pitch);
  B = (float)sys_sin(pitch);
  C = (float)sys_cos(yaw);
  D = (float)sys_sin(yaw);

  m[0] = C;
  m[1] = D*B;
  m[2] = -D*A;
  m[3] = 0;

  m[4] = 0;
  m[5] = A;
  m[6] = B;
  m[7] = 0;

  m[8] = D;
  m[9] = -C*B;
  m[10] = A*C;
  m[11] = 0;

  m[12] = 0;
  m[13] = 0;
  m[14] = 0;
  m[15] = 1;

  m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];
  m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];
  m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
  m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];
}

//
// R_ExecuteSetViewSize
//

void R_ExecuteSetViewSize (void)
{
  int i;
  int cheight;

  setsizeneeded = false;

  SetRatio(SCREENWIDTH, SCREENHEIGHT);

  if (setblocks == 11)
  {
    viewheight = SCREENHEIGHT;
    freelookviewheight = viewheight;
  }
  // proff 09/24/98: Added for high-res
  else
  {
    viewheight = SCREENHEIGHT - ST_SCALED_HEIGHT;
    freelookviewheight = SCREENHEIGHT;
  }

  viewwidth = SCREENWIDTH;

  viewheightfrac = viewheight<<FRACBITS;//e6y

  centery = viewheight/2;
  centerx = viewwidth/2;
  centerxfrac = centerx<<FRACBITS;
  centeryfrac = centery<<FRACBITS;

  if (tallscreen)
  {
    wide_centerx = centerx;
    cheight = SCREENHEIGHT * ratio_multiplier / ratio_scale;
  }
  else
  {
    wide_centerx = centerx * ratio_multiplier / ratio_scale;
    cheight = SCREENHEIGHT;
  }

  // e6y: wide-res
  projection = wide_centerx<<FRACBITS;

// proff 11/06/98: Added for high-res
  // calculate projectiony using int64_t math to avoid overflow when SCREENWIDTH>4228
  projectiony = (fixed_t)((((int64_t)cheight * centerx * 320) / 200) / SCREENWIDTH * FRACUNIT);
  // e6y: this is a precalculated value for more precise flats drawing (see R_MapPlane)
  viewfocratio = projectiony / wide_centerx;

  dsda_SetupStretchParams();

  R_InitBuffer (SCREENWIDTH, viewheight);

  R_InitTextureMapping();

  // psprite scales
  // proff 08/17/98: Changed for high-res
  // proff 11/06/98: Added for high-res
  // e6y: wide-res
  pspritexscale = (wide_centerx << FRACBITS) / 160;
  pspriteyscale = (cheight << FRACBITS) / 200;
  pspriteiscale = FixedDiv (FRACUNIT, pspritexscale);
  // [FG] make sure that the product of the weapon sprite scale factor
  //      and its reciprocal is always at least FRACUNIT to
  //      fix garbage lines at the top of weapon sprites
  pspriteiyscale = FixedDiv (FRACUNIT, pspriteyscale);
  while (FixedMul(pspriteiyscale, pspriteyscale) < FRACUNIT)
    pspriteiyscale++;

  //e6y: added for GL
  pspritexscale_f = (float)wide_centerx/160.0f;
  pspriteyscale_f = (float)cheight / 200.0f;

  skyiscale = (200 << FRACBITS) / SCREENHEIGHT;

	// [RH] Sky height fix for screens not 200 (or 240) pixels tall
	R_InitSkyMap();

  // thing clipping
  for (i=0 ; i<viewwidth ; i++)
    screenheightarray[i] = viewheight;

  for (i=0 ; i<viewwidth ; i++)
    {
      fixed_t cosadj = D_abs(finecosine[xtoviewangle[i]>>ANGLETOFINESHIFT]);
      distscale[i] = FixedDiv(FRACUNIT,cosadj);
    }

  // e6y
  // Calculate the light levels to use
  //  for each level / scale combination.
  // Calculate the light levels to use
  //  for each level / scale combination.
  for (i=0; i<LIGHTLEVELS; i++)
  {
    int j, startmap = ((LIGHTLEVELS-LIGHTBRIGHT-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;
    for (j=0 ; j<MAXLIGHTSCALE ; j++)
    {
      int t, level = startmap - j/DISTMAP;

      if (level < 0)
        level = 0;

      if (level >= NUMCOLORMAPS)
        level = NUMCOLORMAPS-1;

      // killough 3/20/98: initialize multiple colormaps
      level *= 256;

      for (t=0; t<numcolormaps; t++)     // killough 4/4/98
        c_scalelight[t][i][j] = colormaps[t] + level;
    }
  }

#if 0
  if (V_IsOpenGLMode())
    dsda_GLSetRenderViewportParams();
#endif

  dsda_InitExHud();
  dsda_BeginRenderStats();
}

//
// R_Init
//

void R_Init (void)
{
  // CPhipps - R_DrawColumn isn't constant anymore, so must
  //  initialise in code
  // current column draw function
  lprintf(LO_DEBUG, "\nR_LoadTrigTables: ");
  R_LoadTrigTables();
  lprintf(LO_DEBUG, "\nR_InitData: ");
  R_InitData();
  R_SetViewSize();
  lprintf(LO_DEBUG, "\nR_Init: R_InitPlanes ");
  R_InitPlanes();
  lprintf(LO_DEBUG, "R_InitLightTables ");
  R_InitLightTables();
  lprintf(LO_DEBUG, "R_InitSkyMap ");
  R_InitSkyMap();
  lprintf(LO_DEBUG, "R_InitTranslationsTables ");
  R_InitTranslationTables();
  lprintf(LO_DEBUG, "R_InitPatches ");
  R_InitPatches();
}

//
// R_PointInSubsector
//
// killough 5/2/98: reformatted, cleaned up

subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{
  int nodenum = numnodes-1;

  // special case for trivial maps (single subsector, no nodes)
  if (numnodes == 0)
    return subsectors;

  while (!(nodenum & NF_SUBSECTOR))
    nodenum = nodes[nodenum].children[R_PointOnSide(x, y, nodes+nodenum)];
  return &subsectors[nodenum & ~NF_SUBSECTOR];
}

//
// R_SetupFreelook
//

void R_SetupFreelook(void)
{
  if (V_IsSoftwareMode())
  {
    fixed_t InvZtoScale;
    fixed_t dy;
    int i;

    centery = viewheight / 2;
    if (raven || dsda_MouseLook())
    {
      dy = FixedMul(focallengthy, finetangent[(ANG90-viewpitch)>>ANGLETOFINESHIFT]);
      centery += dy >> FRACBITS;
    }
    centeryfrac = centery<<FRACBITS;

    InvZtoScale = yaspectmul * centerx;
    globaluclip = FixedDiv (-centeryfrac, InvZtoScale);
    globaldclip = FixedDiv ((viewheight<<FRACBITS)-centeryfrac, InvZtoScale);

    for (i=0; i<viewheight; i++)
    {
      dy = D_abs(((i-centery)<<FRACBITS)+FRACUNIT/2);
      yslope[i] = FixedDiv(projectiony, dy);
    }
  }
}

//
// R_SetupMatrix
//

void R_SetupMatrix(void)
{
  float fovy, aspect, znear;
  int r_nearclip = 5;

  R_SetupViewport();

  fovy = gl_render_fovy;
  aspect = gl_render_ratio;
  znear = (float)r_nearclip / 100.0f;

  R_SetupPerspective(fovy, aspect, znear);
  R_BuildModelViewMatrix();
}

void R_ResetColorMap(void)
{
  fullcolormap = colormaps[0];
  fixedcolormap = 0;
}

//
// R_SetupFrame
//

static void R_SetupFrame (player_t *player)
{
  dboolean HU_CrosshairEnabled(void);

  int i, cm;

  int FocalTangent = finetangent[FINEANGLES/4 + FieldOfView/2];

  viewplayer = player;

  extralight = player->extralight;

  viewsin = finesine[viewangle>>ANGLETOFINESHIFT];
  viewcos = finecosine[viewangle>>ANGLETOFINESHIFT];

  viewtansin = FixedMul(FocalTangent, viewsin);
  viewtancos = FixedMul(FocalTangent, viewcos);

  R_SetupFreelook();

  // killough 3/20/98, 4/4/98: select colormap based on player status

  if (player->mo->subsector->sector->heightsec != -1)
    {
      const sector_t *s = player->mo->subsector->sector->heightsec + sectors;
      cm = viewz < s->floorheight ? s->bottommap : viewz > s->ceilingheight ?
        s->topmap : s->midmap;
      if (cm < 0 || cm > numcolormaps)
        cm = 0;
    }
  else
    cm = 0;

  //e6y: save previous and current colormap
  boom_cm = cm;

  fullcolormap = colormaps[cm];
  zlight = c_zlight[cm];
  scalelight = c_scalelight[cm];

  //e6y
  frame_fixedcolormap = player->fixedcolormap;
  if (frame_fixedcolormap < 0 || frame_fixedcolormap > NUMCOLORMAPS)
  {
    I_Error("<fixedcolormap> value out of range: %d\n", player->fixedcolormap);
  }

  if (player->fixedcolormap)
    {
      // killough 3/20/98: localize scalelightfixed (readability/optimization)
      static const lighttable_t *scalelightfixed[MAXLIGHTSCALE];

      fixedcolormap = fullcolormap   // killough 3/20/98: use fullcolormap
        + player->fixedcolormap*256*sizeof(lighttable_t);

      walllights = scalelightfixed;

      for (i=0 ; i<MAXLIGHTSCALE ; i++)
        scalelightfixed[i] = fixedcolormap;
    }
  else
    fixedcolormap = 0;

  R_SetClipPlanes();

  if (/*V_IsOpenGLMode() ||*/ HU_CrosshairEnabled())
    R_SetupMatrix();

  validcount++;
}

static void R_InitDrawScene(void)
{
  // Framerate-independent fuzz progression
  static int fuzzgametic = 0;
  static int savedfuzzpos = 0;

#if 0
  if (V_IsOpenGLMode())
  {
    // proff 11/99: clear buffers
    gld_InitDrawScene();

    if (!automap_on)
    {
      // proff 11/99: switch to perspective mode
      gld_StartDrawScene();
    }
  } else {
#endif
    if (dsda_IntConfig(dsda_config_flashing_hom))
    { // killough 2/10/98: add flashing red HOM indicators
      unsigned char color=(gametic % 20) < 9 ? 0xb0 : 0;
      V_FillRect(0, 0, 0, viewwidth, viewheight, color);
      R_DrawViewBorder();
    }

    // Only progress software fuzz offset if the gametic has progressed
    if (fuzzgametic != gametic)
    {
      fuzzgametic = gametic;
      savedfuzzpos = R_GetFuzzPos();
    }
    else
    {
      R_SetFuzzPos(savedfuzzpos);
    }
  //}
}

static void R_RenderBSPNodes(void)
{
  // Make displayed player invisible locally
  if (localQuakeHappening[displayplayer] && gamestate == GS_LEVEL)
  {
    players[displayplayer].mo->flags2 |= MF2_DONTDRAW;
    R_RenderBSPNode(numnodes - 1);  // head node is the last node output
    players[displayplayer].mo->flags2 &= ~MF2_DONTDRAW;
  }
  else
  {
    // The head node is the last node output.
    R_RenderBSPNode(numnodes - 1);
  }

  if (map_format.zdoom /*&& V_IsOpenGLMode()*/)
  {
    R_ForceRenderPolyObjs();
  }
}

//
// R_RenderView
//

void R_RenderPlayerView (player_t* player)
{
  r_frame_count++;

  DSDA_ADD_CONTEXT(sf_setup_frame);
  R_SetupFrame (player);
  DSDA_REMOVE_CONTEXT(sf_setup_frame);

  DSDA_ADD_CONTEXT(sf_clear);
  R_ClearClipSegs ();
  R_ClearDrawSegs ();
  R_ClearPlanes ();
  R_ClearSprites ();
  DSDA_REMOVE_CONTEXT(sf_clear);

  DSDA_ADD_CONTEXT(sf_init_scene);
  R_InitDrawScene();
  DSDA_REMOVE_CONTEXT(sf_init_scene);

  FakeNetUpdate();

#if 0
  if (V_IsOpenGLMode()) {
    DSDA_ADD_CONTEXT(sf_gl_frustum);
    gld_FrustumSetup();
    DSDA_REMOVE_CONTEXT(sf_gl_frustum);
  }
#endif

  DSDA_ADD_CONTEXT(sf_bsp_nodes);
  R_RenderBSPNodes();
  DSDA_REMOVE_CONTEXT(sf_bsp_nodes);

  FakeNetUpdate();

  if (V_IsSoftwareMode())
  {
    DSDA_ADD_CONTEXT(sf_draw_planes);
    R_DrawPlanes();
    DSDA_REMOVE_CONTEXT(sf_draw_planes);
  }

  DSDA_ADD_CONTEXT(sf_reset_column_buffer);
  R_ResetColumnBuffer();
  DSDA_REMOVE_CONTEXT(sf_reset_column_buffer);

  FakeNetUpdate();

  if (V_IsSoftwareMode()) {
    DSDA_ADD_CONTEXT(sf_draw_masked);
    R_DrawMasked ();
    R_ResetColumnBuffer();
    DSDA_REMOVE_CONTEXT(sf_draw_masked);
  }

  FakeNetUpdate();

  if (/*V_IsOpenGLMode() &&*/ !automap_on) {
    DSDA_ADD_CONTEXT(sf_draw_scene);
    //gld_DrawScene(player);
    //gld_EndDrawScene();
    DSDA_REMOVE_CONTEXT(sf_draw_scene);
  }
}
