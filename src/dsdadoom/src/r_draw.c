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
 *      The actual span/column drawing functions.
 *      Here find the main potential for optimization,
 *       e.g. inline assembly, different algorithms.
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_draw.h"
#include "v_video.h"
#include "st_stuff.h"
#include "g_game.h"
#include "am_map.h"
#include "lprintf.h"

#include "dsda/stretch.h"

//
// All drawing to the view buffer is accomplished in this file.
// The other refresh files only know about ccordinates,
//  not the architecture of the frame buffer.
// Conveniently, the frame buffer is a linear one,
//  and we need only the base address,
//  and the total size == width*height*depth/8.,
//

byte *viewimage;
int  viewwidth;
int  viewheight;

// Color tables for different players,
//  translate a limited part to another
//  (color ramps used for  suit colors).
//

// CPhipps - made const*'s
const byte *tranmap;          // translucency filter maps 256x256   // phares
const byte *main_tranmap;     // killough 4/11/98

//
// R_DrawColumn
// Source is the top of the column to scale.
//

// SoM: OPTIMIZE for ANYRES
typedef enum
{
   COL_NONE,
   COL_OPAQUE,
   COL_TRANS,
   COL_FLEXTRANS,
   COL_FUZZ,
   COL_FLEXADD
} columntype_e;

static int    temp_x = 0;
static int    tempyl[4], tempyh[4];

// e6y: resolution limitation is removed
static byte           *tempbuf;

static int    startx = 0;
static int    temptype = COL_NONE;
static int    commontop, commonbot;
static const byte *temptranmap = NULL;
// SoM 7-28-04: Fix the fuzz problem.
static const byte   *tempfuzzmap;

//
// Spectre/Invisibility.
//

#define FUZZTABLE 50
// proff 08/17/98: Changed for high-res
//#define FUZZOFF (SCREENWIDTH)
#define FUZZOFF 1

static const int fuzzoffset_org[FUZZTABLE] = {
  FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
  FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
  FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
  FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
  FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
  FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
  FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF
};

static int fuzzoffset[FUZZTABLE];

static int fuzzpos = 0;

// render pipelines
#define RDC_STANDARD      1
#define RDC_TRANSLUCENT   2
#define RDC_TRANSLATED    4
#define RDC_FUZZ          8
// no color mapping
#define RDC_NOCOLMAP     16

draw_vars_t drawvars = {
  NULL, // topleft
  0, // pitch
};

dboolean R_FullView(void)
{
  return viewheight == SCREENHEIGHT;
}

dboolean R_PartialView(void)
{
  return viewheight != SCREENHEIGHT;
}

dboolean R_StatusBarVisible(void)
{
  return R_PartialView() || automap_on;
}

//
// Error functions that will abort if R_FlushColumns tries to flush
// columns without a column type.
//

static void R_FlushWholeError(void)
{
   I_Error("R_FlushWholeColumns called without being initialized.\n");
}

static void R_FlushHTError(void)
{
   I_Error("R_FlushHTColumns called without being initialized.\n");
}

static void R_QuadFlushError(void)
{
   I_Error("R_FlushQuadColumn called without being initialized.\n");
}

static void (*R_FlushWholeColumns)(void) = R_FlushWholeError;
static void (*R_FlushHTColumns)(void)    = R_FlushHTError;
static void (*R_FlushQuadColumn)(void) = R_QuadFlushError;

static void R_FlushColumns(void)
{
   if(temp_x != 4 || commontop >= commonbot)
      R_FlushWholeColumns();
   else
   {
      R_FlushHTColumns();
      R_FlushQuadColumn();
   }
   temp_x = 0;
}

//
// R_ResetColumnBuffer
//
// haleyjd 09/13/04: new function to call from main rendering loop
// which gets rid of the unnecessary reset of various variables during
// column drawing.
//
void R_ResetColumnBuffer(void)
{
   // haleyjd 10/06/05: this must not be done if temp_x == 0!
   if(temp_x)
      R_FlushColumns();
   temptype = COL_NONE;
   R_FlushWholeColumns = R_FlushWholeError;
   R_FlushHTColumns    = R_FlushHTError;
   R_FlushQuadColumn   = R_QuadFlushError;
}

#define R_DRAWCOLUMN_PIPELINE RDC_STANDARD
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_TRANSLUCENT
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_FUZZ
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz
#include "r_drawflush.inl"

//
// R_DrawColumn
//

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
//

byte *translationtables;

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_STANDARD
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_STANDARD

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawColumn ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

// Here is the version of R_DrawColumn that deals with translucent  // phares
// textures and sprites. It's identical to R_DrawColumn except      //    |
// for the spot where the color index is stuffed into *dest. At     //    V
// that point, the existing color index and the new color index
// are mapped through the TRANMAP lump filters to get a new color
// index whose RGB values are the average of the existing and new
// colors.
//
// Since we're concerned about performance, the 'translucent or
// opaque' decision is made outside this routine, not down where the
// actual code differences are.

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_TRANSLUCENT
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_TRANSLUCENT

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTLColumn ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// R_DrawTranslatedColumn
// Used to draw player sprites
//  with the green colorramp mapped to others.
// Could be used with different translation
//  tables, e.g. the lighter colored version
//  of the BaronOfHell, the HellKnight, uses
//  identical sprites, kinda brightened up.
//

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_TRANSLATED
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_TRANSLATED

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTranslatedColumn ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// Framebuffer postprocessing.
// Creates a fuzzy image by copying pixels
//  from adjacent ones to left and right.
// Used with an all black colormap, this
//  could create the SHADOW effect,
//  i.e. spectres and invisible players.
//

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_FUZZ
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_FUZZ

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawFuzzColumn ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

static R_DrawColumn_f drawcolumnfuncs[RDRAW_FILTER_MAXFILTERS][RDC_PIPELINE_MAXPIPELINES] = {
  {
    R_DrawColumn_PointUV,
    R_DrawTLColumn_PointUV,
    R_DrawTranslatedColumn_PointUV,
    R_DrawFuzzColumn_PointUV,
  },
  {
    R_DrawColumn_PointUV_PointZ,
    R_DrawTLColumn_PointUV_PointZ,
    R_DrawTranslatedColumn_PointUV_PointZ,
    R_DrawFuzzColumn_PointUV_PointZ,
  },
};

R_DrawColumn_f R_GetDrawColumnFunc(enum column_pipeline_e type, enum draw_filter_type_e filterz) {
  R_DrawColumn_f result = drawcolumnfuncs[filterz][type];
  if (result == NULL)
    I_Error("R_GetDrawColumnFunc: undefined function (%d, %d)", type, filterz);
  return result;
}

void R_SetDefaultDrawColumnVars(draw_column_vars_t *dcvars) {
  dcvars->x = dcvars->yl = dcvars->yh = 0;
  dcvars->iscale = dcvars->texturemid = dcvars->texheight = 0;
  dcvars->source = dcvars->prevsource = dcvars->nextsource = NULL;
  dcvars->colormap = colormaps[0];
  dcvars->translation = NULL;
  dcvars->edgeslope = dcvars->drawingmasked = 0;
  dcvars->flags = 0;

  // heretic
  dcvars->baseclip = -1;
}

//
// R_InitTranslationTables
// Creates the translation tables to map
//  the green color ramp to gray, brown, red.
// Assumes a given structure of the PLAYPAL.
// Could be read from a lump instead.
//

byte playernumtotrans[MAX_MAXPLAYERS];

// HERETIC_TODO: player colors
const byte player_colors[] = { 0x70, 0x60, 0x40, 0x20 };

void R_InitTranslationTables (void)
{
  int i, j;
#define MAXTRANS 3
  byte transtocolour[MAXTRANS];

  if (hexen)
  {
    int lumpnum = W_GetNumForName("trantbl0");
    translationtables = Z_Malloc(256 * 3 * (g_maxplayers - 1));

    for (i = 0; i < g_maxplayers; i++)
      playernumtotrans[i] = i;

    for (i = 0; i < 3 * (g_maxplayers - 1); i++)
    {
        const byte* transLump = W_LumpByNum(lumpnum + i);
        memcpy(translationtables + i * 256, transLump, 256);
    }

    return;
  }

  // killough 5/2/98:
  // Remove dependency of colormaps aligned on 256-byte boundary

  if (translationtables == NULL) // CPhipps - allow multiple calls
    translationtables = Z_Malloc(256*MAXTRANS);

  for (i=0; i<MAXTRANS; i++) transtocolour[i] = 255;

  for (i = 0; i < g_maxplayers; i++) {
    byte wantcolour = player_colors[i];
    playernumtotrans[i] = 0;
    if (wantcolour != 0x70) // Not green, would like translation
      for (j = 0; j < MAXTRANS; j++)
        if (transtocolour[j] == 255) {
          transtocolour[j] = wantcolour;
          playernumtotrans[i] = j + 1;
          break;
        }
  }

  // translate just the 16 green colors
  for (i=0; i<256; i++)
    if (i >= 0x70 && i<= 0x7f)
    {
      // CPhipps - configurable player colours
      translationtables[i] = colormaps[0][(i&0xf) + transtocolour[0]];
      translationtables[i+256] = colormaps[0][(i&0xf) + transtocolour[1]];
      translationtables[i+512] = colormaps[0][(i&0xf) + transtocolour[2]];
    }
    else  // Keep all other colors as is.
      translationtables[i]=translationtables[i+256]=translationtables[i+512]=i;
}

//
// R_DrawSpan
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//

void R_DrawSpan(draw_span_vars_t *dsvars) {
  unsigned count = dsvars->x2 - dsvars->x1 + 1;
  fixed_t xfrac = dsvars->xfrac;
  fixed_t yfrac = dsvars->yfrac;
  const fixed_t xstep = dsvars->xstep;
  const fixed_t ystep = dsvars->ystep;
  const byte *source = dsvars->source;
  const byte *colormap = dsvars->colormap;
  byte *dest = drawvars.topleft + dsvars->y*drawvars.pitch + dsvars->x1;

  while (count) {
    const fixed_t xtemp = (xfrac >> 16) & 63;
    const fixed_t ytemp = (yfrac >> 10) & 4032;
    const fixed_t spot = xtemp | ytemp;
    xfrac += xstep;
    yfrac += ystep;
    *dest++ = colormap[source[spot]];
    count--;
  }
}

void R_InitBuffersRes(void)
{
  extern byte *solidcol;

  if (solidcol) Z_Free(solidcol);
  if (tempbuf) Z_Free(tempbuf);

  solidcol = Z_Calloc(1, SCREENWIDTH * sizeof(*solidcol));
  tempbuf = Z_Calloc(1, (SCREENHEIGHT * 4) * sizeof(*tempbuf));

  temp_x = 0;
}

//
// R_InitBuffer
// Creats lookup tables that avoid
//  multiplies and other hazzles
//  for getting the framebuffer address
//  of a pixel to draw.
//

void R_InitBuffer(int width, int height)
{
  int i;

  drawvars.topleft = screens[0].data;
  drawvars.pitch = screens[0].pitch;

  for (i=0; i<FUZZTABLE; i++)
    fuzzoffset[i] = fuzzoffset_org[i]*screens[0].pitch;
}

//
// R_FillBackScreen
// Fills the back screen with a pattern
//  for variable screen sizes
// Also draws a beveled edge.
//
// CPhipps - patch drawing updated

void R_FillBackScreen (void)
{
  int automap = automap_on;

  if (grnrock.lumpnum == 0)
    return;

  V_BeginUIDraw();

  // e6y: wide-res
  if (ratio_multiplier != ratio_scale || wide_offsety)
  {
    int only_stbar;

    only_stbar = V_IsSoftwareMode() || automap || R_PartialView();

    if (only_stbar && ST_SCALED_OFFSETX > 0)
    {
      int stbar_top = SCREENHEIGHT - ST_SCALED_HEIGHT;

      if (V_IsOpenGLMode())
        V_FillFlat(grnrock.lumpnum, 1, 0, stbar_top, SCREENWIDTH, ST_SCALED_HEIGHT, VPT_NONE);
      else
      {
        V_FillFlat(grnrock.lumpnum, 1,
          0, stbar_top, ST_SCALED_OFFSETX, ST_SCALED_HEIGHT, VPT_NONE);
        V_FillFlat(grnrock.lumpnum, 1,
          SCREENWIDTH - ST_SCALED_OFFSETX, stbar_top, ST_SCALED_OFFSETX, ST_SCALED_HEIGHT, VPT_NONE);

        // For custom huds, need to put the backfill inside the bar area (in the copy buffer)
        V_FillFlat(grnrock.lumpnum, 0,
          ST_SCALED_OFFSETX, stbar_top, SCREENWIDTH - 2 * ST_SCALED_OFFSETX, ST_SCALED_HEIGHT, VPT_NONE);
      }

      // heretic_note: I think this looks bad, so I'm skipping it...
      if (!heretic)
      {
        // line between view and status bar
        V_FillPatch(brdr_b.lumpnum, 1, 0, stbar_top, ST_SCALED_OFFSETX, brdr_b.height, VPT_NONE);
        V_FillPatch(brdr_b.lumpnum, 1, SCREENWIDTH - ST_SCALED_OFFSETX, stbar_top, ST_SCALED_OFFSETX, brdr_b.height, VPT_NONE);
      }
    }
  }

  V_EndUIDraw();
}

//
// Copy a screen buffer.
//

static void R_CopyScreenBufferSection(int x, int y, int count)
{
  if (V_IsSoftwareMode())
    memcpy(screens[0].data+y*screens[0].pitch+x,
           screens[1].data+y*screens[1].pitch+x,
           count);   // LFB copy.
}

//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//

void R_DrawViewBorder(void)
{
  int i;

  if (V_IsOpenGLMode()) {
    // proff 11/99: we don't have a backscreen in OpenGL from where we can copy this
    R_FillBackScreen();
    return;
  }

  // e6y: wide-res
  if ((ratio_multiplier != ratio_scale || wide_offsety) && R_StatusBarVisible())
  {
    for (i = SCREENHEIGHT - ST_SCALED_HEIGHT; i < SCREENHEIGHT; i++)
    {
      R_CopyScreenBufferSection(0, i, ST_SCALED_OFFSETX);
      R_CopyScreenBufferSection(SCREENWIDTH - ST_SCALED_OFFSETX, i, ST_SCALED_OFFSETX);
    }
  }
}

void R_SetFuzzPos(int fp)
{
  fuzzpos = fp;
}

int R_GetFuzzPos()
{
  return fuzzpos;
}
