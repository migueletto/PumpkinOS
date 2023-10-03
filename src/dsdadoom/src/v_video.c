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
 *  Gamma correction LUT stuff.
 *  Color range translation support
 *  Functions to draw patches (by post) directly to screen.
 *  Functions to blit a block to the screen.
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
//#include "SDL.h"

#include "doomdef.h"
#include "doomstat.h"
#include "r_main.h"
#include "r_draw.h"
#include "r_patch.h"
#include "m_bbox.h"
#include "w_wad.h"   /* needed for color translation lump lookup */
#include "v_video.h"
#include "i_video.h"
#include "lprintf.h"
#include "st_stuff.h"
#include "e6y.h"

#include "dsda/configuration.h"
#include "dsda/cr_table.h"
#include "dsda/global.h"
#include "dsda/palette.h"
#include "dsda/stretch.h"
#include "dsda/text_color.h"

// DWF 2012-05-10
// SetRatio sets the following global variables based on window geometry and
// user preferences. The integer ratio is hardly used anymore, so further
// simplification may be in order.
dboolean tallscreen;
unsigned int ratio_multiplier, ratio_scale;
float gl_ratio;
int psprite_offset; // Needed for "tallscreen" modes


// Each screen is [SCREENWIDTH*SCREENHEIGHT];
screeninfo_t screens[NUM_SCREENS];

/* jff 4/24/98 initialize this at runtime */
const byte *colrngs[CR_LIMIT];

int usegamma;

// [FG] translate between blood color value as per EE spec
//      and actual color translation table index

static const int bloodcolor[] = {
  0,         // 0 - Red (normal)
  CR_GRAY,   // 1 - Grey
  CR_GREEN,  // 2 - Green
  CR_BLUE,   // 3 - Blue
  CR_YELLOW, // 4 - Yellow
  CR_BLACK,  // 5 - Black
  CR_PURPLE, // 6 - Purple
  CR_WHITE,  // 7 - White
  CR_ORANGE, // 8 - Orange
};

int V_BloodColor(int blood)
{
  if (blood < 0 || blood > 8)
    blood = 0;

  return bloodcolor[blood];
}

// haleyjd: DOSDoom-style single translucency lookup-up table
// generation code. This code has a 32k (plus a bit more)
// footprint but allows a much wider range of translucency effects
// than BOOM-style translucency. This will be used for particles,
// for variable mapthing trans levels, and for screen patches.

// haleyjd: Updated 06/21/08 to use 32k lookup, mainly to fix
// additive translucency. Note this code is included in Odamex and
// so it can be considered GPL as used here, rather than BSD. But,
// I don't care either way. It is effectively dual-licensed I suppose.

unsigned int Col2RGB8[65][256];
byte RGB32k[32][32][32];

#define MAKECOLOR(a) (((a)<<3)|((a)>>2))

void V_InitFlexTranTable(void)
{
  static int flexTranInit = false;

  if (!flexTranInit)
  {
    int r, g, b, x, y, pos;
    const unsigned char *palette = V_GetPlaypal();

    // mark that we've initialized the flex tran table
    flexTranInit = true;

    // build RGB table
    for(r = 0; r < 32; r++)
    {
      for(g = 0; g < 32; g++)
      {
        for(b = 0; b < 32; b++)
        {
          RGB32k[r][g][b] = V_BestColor(palette,
            MAKECOLOR(r), MAKECOLOR(g), MAKECOLOR(b));
        }
      }
    }

    // build lookup table
    for(x = 0; x < 65; x++)
    {
      pos = 0;
      for(y = 0; y < 256; y++)
      {
        Col2RGB8[x][y] =
          (((palette[pos + 0] * x) >> 4) << 20) |
          ((palette[pos + 1] * x) >> 4) |
          (((palette[pos + 2] * x) >> 4) << 10);

        pos += 3;
      }
    }
  }
}

void V_InitColorTranslation(void)
{
  int i;
  byte* full_table;

  full_table = dsda_GenerateCRTable();

  for (i = 0; i < CR_LIMIT; ++i)
    colrngs[i] = full_table + 256 * i;

  dsda_LoadTextColor();
}

//
// V_CopyRect
//
// Copies a source rectangle in a screen buffer to a destination
// rectangle in another screen buffer. Source origin in srcx,srcy,
// destination origin in destx,desty, common size in width and height.
// Source buffer specfified by srcscrn, destination buffer by destscrn.
//
// Marks the destination rectangle on the screen dirty.
//
// No return.
//
static void FUNC_V_CopyRect(int srcscrn, int destscrn,
                int x, int y, int width, int height,
                enum patch_translation_e flags)
{
  byte *src;
  byte *dest;

  if (flags & VPT_STRETCH_MASK)
  {
    stretch_param_t *params;
    int sx = x;
    int sy = y;

    params = dsda_StretchParams(flags);

    x  = params->video->x1lookup[x];
    y  = params->video->y1lookup[y];
    width  = params->video->x2lookup[sx + width - 1] - x + 1;
    height = params->video->y2lookup[sy + height - 1] - y + 1;
    x += params->deltax1;
    y += params->deltay1;
  }

  if (x < 0)
  {
    width += x;
    x = 0;
  }

  if (x + width > SCREENWIDTH)
  {
    width = SCREENWIDTH - x;
  }

  if (y < 0)
  {
    height += y;
    y = 0;
  }

  if (y + height > SCREENHEIGHT)
  {
    height = SCREENHEIGHT - y;
  }

  if (width <= 0 || height <= 0)
  {
    return;
  }

  src = screens[srcscrn].data + screens[srcscrn].pitch * y + x;
  dest = screens[destscrn].data + screens[destscrn].pitch * y + x;

  for ( ; height>0 ; height--)
    {
      memcpy (dest, src, width);
      src += screens[srcscrn].pitch;
      dest += screens[destscrn].pitch;
    }
}

#define FILL_FLAT(dest_type, dest_pitch, pal_func)\
{\
  const byte *src, *src_p;\
  dest_type *dest, *dest_p;\
  for (sy = y ; sy < y + height; sy += 64)\
  {\
    h = (y + height - sy < 64 ? y + height - sy : 64);\
    dest = (dest_type *)screens[scrn].data + dest_pitch * sy + x;\
    src = data + 64 * ((sy - y) % 64);\
    for (sx = x; sx < x + width; sx += 64)\
    {\
      src_p = src;\
      dest_p = dest;\
      w = (x + width - sx < 64 ? x + width - sx : 64);\
      for (j = 0; j < h; j++)\
      {\
        for (i = 0; i < w; i++)\
        {\
          dest_p[i] = pal_func(src_p[i], VID_COLORWEIGHTMASK);\
        }\
        dest_p += dest_pitch;\
        src_p += 64;\
      }\
      dest += 64;\
    }\
  }\
}\

static void FUNC_V_FillFlat(int lump, int scrn, int x, int y, int width, int height, enum patch_translation_e flags)
{
  /* erase the entire screen to a tiled background */
  const byte *data;
  int sx, sy, w, h;
  int j, pitch;

  lump += firstflat;

  // killough 4/17/98:
  data = W_LumpByNum(lump);

  {
    const byte *src, *src_p;
    byte *dest, *dest_p;
    pitch = screens[scrn].pitch;

    for (sy = y ; sy < y + height; sy += 64)
    {
      h = (y + height - sy < 64 ? y + height - sy : 64);
      dest = screens[scrn].data + pitch * sy + x;
      src = data + 64 * ((sy - y) % 64);
      for (sx = x; sx < x + width; sx += 64)
      {
        src_p = src;
        dest_p = dest;
        w = (x + width - sx < 64 ? x + width - sx : 64);
        for (j = 0; j < h; j++)
        {
          memcpy (dest_p, src_p, w);
          dest_p += pitch;
          src_p += 64;
        }
        dest += 64;
      }
    }
  }
}

static void FUNC_V_FillPatch(int lump, int scrn, int x, int y, int width, int height, enum patch_translation_e flags)
{
  int sx, sy, w, h;

  w = R_NumPatchWidth(lump);
  h = R_NumPatchHeight(lump);

  for (sy = y; sy < y + height; sy += h)
  {
    for (sx = x; sx < x + width; sx += w)
    {
      V_DrawNumPatch(sx, sy, scrn, lump, CR_DEFAULT, flags);
    }
  }
}

/*
 * V_DrawBackground tiles a 64x64 patch over the entire screen, providing the
 * background for the Help and Setup screens, and plot text betwen levels.
 * cphipps - used to have M_DrawBackground, but that was used the framebuffer
 * directly, so this is my code from the equivalent function in f_finale.c
 */
static void FUNC_V_DrawBackground(const char* flatname, int scrn)
{
  V_FillFlatName(flatname, scrn, 0, 0, SCREENWIDTH, SCREENHEIGHT, VPT_NONE);
}

//
// V_Init
//
// Allocates the 4 full screen buffers in low DOS memory
// No return
//

void V_Init (void)
{
  int  i;

  // reset the all
  for (i = 0; i<NUM_SCREENS; i++) {
    screens[i].data = NULL;
    screens[i].not_on_heap = false;
    screens[i].width = 0;
    screens[i].height = 0;
    screens[i].pitch = 0;
  }
}

//
// V_DrawMemPatch
//
// CPhipps - unifying patch drawing routine, handles all cases and combinations
//  of stretching, flipping and translating
//
// This function is big, hopefully not too big that gcc can't optimise it well.
// In fact it packs pretty well, there is no big performance lose for all this merging;
// the inner loops themselves are just the same as they always were
// (indeed, laziness of the people who wrote the 'clones' of the original V_DrawPatch
//  means that their inner loops weren't so well optimised, so merging code may even speed them).
//
static void V_DrawMemPatch(int x, int y, int scrn, const rpatch_t *patch,
        int cm, enum patch_translation_e flags)
{
  const byte *trans;

  stretch_param_t *params;

  if (cm == CR_DEFAULT)
    trans = NULL;
  else if (cm < CR_LIMIT)
    trans = colrngs[cm];
  else
    trans = translationtables + 256*((cm - CR_LIMIT) - 1);

  if (!(flags & VPT_NOOFFSET))
  {
    y -= patch->topoffset;
    x -= patch->leftoffset;
  }

  // CPhipps - auto-no-stretch if not high-res
  if ((flags & VPT_STRETCH_MASK) && SCREEN_320x200)
    flags &= ~VPT_STRETCH_MASK;

  params = dsda_StretchParams(flags);

  // CPhipps - null translation pointer => no translation
  if (!trans)
    flags &= ~VPT_TRANS;

  // [FG] automatically center wide patches without horizontal offset
  if (patch->width > 320 && patch->leftoffset == 0)
    x -= (patch->width - 320) / 2;

  if (!(flags & VPT_STRETCH_MASK)) {
    int             col;
    byte           *desttop = screens[scrn].data+y*screens[scrn].pitch+x;
    int    w = patch->width;

    if (y<0 || y+patch->height > ((flags & VPT_STRETCH) ? 200 :  SCREENHEIGHT)) {
      // killough 1/19/98: improved error message:
      lprintf(LO_WARN, "V_DrawMemPatch8: Patch (%d,%d)-(%d,%d) exceeds LFB in vertical direction (horizontal is clipped)\n"
              "Bad V_DrawMemPatch8 (flags=%u)", x, y, x+patch->width, y+patch->height, flags);
      return;
    }

    w--; // CPhipps - note: w = width-1 now, speeds up flipping

    for (col=0 ; col<=w ; desttop++, col++, x++) {
      int i;
      const int colindex = (flags & VPT_FLIP) ? (w - col) : (col);
      const rcolumn_t *column = R_GetPatchColumn(patch, colindex);

      if (x < 0)
        continue;
      if (x >= SCREENWIDTH)
        break;

      // step through the posts in a column
      for (i=0; i<column->numPosts; i++) {
        const rpost_t *post = &column->posts[i];
        // killough 2/21/98: Unrolled and performance-tuned

        const byte *source = column->pixels + post->topdelta;
        byte *dest = desttop + post->topdelta*screens[scrn].pitch;
        int count = post->length;

        if (!(flags & VPT_TRANS)) {
          if ((count-=4)>=0)
            do {
              register byte s0,s1;
              s0 = source[0];
              s1 = source[1];
              dest[0] = s0;
              dest[screens[scrn].pitch] = s1;
              dest += screens[scrn].pitch*2;
              s0 = source[2];
              s1 = source[3];
              source += 4;
              dest[0] = s0;
              dest[screens[scrn].pitch] = s1;
              dest += screens[scrn].pitch*2;
            } while ((count-=4)>=0);
          if (count+=4)
            do {
              *dest = *source++;
              dest += screens[scrn].pitch;
            } while (--count);
        } else {
          // CPhipps - merged translation code here
          if ((count-=4)>=0)
            do {
              register byte s0,s1;
              s0 = source[0];
              s1 = source[1];
              s0 = trans[s0];
              s1 = trans[s1];
              dest[0] = s0;
              dest[screens[scrn].pitch] = s1;
              dest += screens[scrn].pitch*2;
              s0 = source[2];
              s1 = source[3];
              s0 = trans[s0];
              s1 = trans[s1];
              source += 4;
              dest[0] = s0;
              dest[screens[scrn].pitch] = s1;
              dest += screens[scrn].pitch*2;
            } while ((count-=4)>=0);
          if (count+=4)
            do {
              *dest = trans[*source++];
              dest += screens[scrn].pitch;
            } while (--count);
        }
      }
    }
  }
  else {
    // CPhipps - move stretched patch drawing code here
    //         - reformat initialisers, move variables into inner blocks

    int   col;
    int   w = (patch->width << 16) - 1; // CPhipps - -1 for faster flipping
    int   left, right, top, bottom;
    int   DXI, DYI;
    int   deltay1;
    R_DrawColumn_f colfunc;
    draw_column_vars_t dcvars;
    draw_vars_t olddrawvars = drawvars;

    R_SetDefaultDrawColumnVars(&dcvars);

    drawvars.topleft = screens[scrn].data;
    drawvars.pitch = screens[scrn].pitch;

    if (flags & VPT_TRANS) {
      colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_TRANSLATED, RDRAW_FILTER_NONE);
      dcvars.translation = trans;
    } else {
      colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_STANDARD, RDRAW_FILTER_NONE);
    }

    DXI = params->video->xstep;
    DYI = params->video->ystep;

    left = (x < 0 || x > 320 ? (x * params->video->width) / 320 : params->video->x1lookup[x]);
    top =  (y < 0 || y > 200 ? (y * params->video->height) / 200 : params->video->y1lookup[y]);

    if (x + patch->width < 0 || x + patch->width > 320)
      right = ( ((x + patch->width) * params->video->width - 1) / 320 );
    else
      right = params->video->x2lookup[x + patch->width - 1];

    if (y + patch->height < 0 || y + patch->height > 200)
      bottom = ( ((y + patch->height - 0) * params->video->height) / 200 );
    else
      bottom = params->video->y2lookup[y + patch->height - 1];

    deltay1 = params->deltay1;

    if (TOP_ALIGNMENT(flags & VPT_STRETCH_MASK))
      deltay1 += global_patch_top_offset;

    left   += params->deltax1;
    right  += params->deltax2;
    top    += deltay1;
    bottom += deltay1;

    dcvars.texheight = patch->height;
    dcvars.iscale = DYI;
    dcvars.drawingmasked = MAX(patch->width, patch->height) > 8;

    col = 0;

    for (dcvars.x=left; dcvars.x<=right; dcvars.x++, col+=DXI) {
      int i;
      const int colindex = (flags & VPT_FLIP) ? ((w - col)>>16): (col>>16);
      const rcolumn_t *column = R_GetPatchColumn(patch, colindex);
      const rcolumn_t *prevcolumn = R_GetPatchColumn(patch, colindex-1);
      const rcolumn_t *nextcolumn = R_GetPatchColumn(patch, colindex+1);

      // ignore this column if it's to the left of our clampRect
      if (dcvars.x < 0)
        continue;
      if (dcvars.x >= SCREENWIDTH)
        break;

      // step through the posts in a column
      for (i=0; i<column->numPosts; i++) {
        const rpost_t *post = &column->posts[i];
        int yoffset = 0;

        //e6y
        if (!(flags & VPT_STRETCH_MASK))
        {
          dcvars.yl = y + post->topdelta;
          dcvars.yh = ((((y + post->topdelta + post->length) << 16) - (FRACUNIT>>1))>>FRACBITS);
        }
        else
        {
          // e6y
          // More accurate patch drawing from Eternity.
          // Predefined arrays are used instead of dynamic calculation
          // of the top and bottom screen coordinates of a column.

          int tmpy;

          tmpy = y + post->topdelta;
          if (tmpy < 0 || tmpy > 200)
            dcvars.yl = (tmpy * params->video->height) / 200 + deltay1;
          else
            dcvars.yl = params->video->y1lookup[tmpy] + deltay1;

          tmpy = y + post->topdelta + post->length - 1;
          if (tmpy < 0 || tmpy > 200)
            dcvars.yh = (tmpy * params->video->height) / 200 + deltay1;
          else
            dcvars.yh = params->video->y2lookup[tmpy] + deltay1;
        }
        dcvars.edgeslope = post->slope;

        if ((dcvars.yh < 0) || (dcvars.yh < top))
          continue;
        if ((dcvars.yl >= SCREENHEIGHT) || (dcvars.yl >= bottom))
          continue;

        if (dcvars.yh >= bottom) {
          //dcvars.yh = bottom-1;
          dcvars.edgeslope &= ~RDRAW_EDGESLOPE_BOT_MASK;
        }
        if (dcvars.yh >= SCREENHEIGHT) {
          dcvars.yh = SCREENHEIGHT-1;
          dcvars.edgeslope &= ~RDRAW_EDGESLOPE_BOT_MASK;
        }

        if (dcvars.yl < 0) {
          yoffset = 0-dcvars.yl;
          dcvars.yl = 0;
          dcvars.edgeslope &= ~RDRAW_EDGESLOPE_TOP_MASK;
        }
        if (dcvars.yl < top) {
          yoffset = top-dcvars.yl;
          dcvars.yl = top;
          dcvars.edgeslope &= ~RDRAW_EDGESLOPE_TOP_MASK;
        }

        dcvars.source = column->pixels + post->topdelta + yoffset;
        dcvars.prevsource = prevcolumn ? prevcolumn->pixels + post->topdelta + yoffset: dcvars.source;
        dcvars.nextsource = nextcolumn ? nextcolumn->pixels + post->topdelta + yoffset: dcvars.source;

        dcvars.texturemid = -((dcvars.yl-centery)*dcvars.iscale);

        //e6y
        dcvars.dy = deltay1;
        dcvars.flags |= DRAW_COLUMN_ISPATCH;

        colfunc(&dcvars);
      }
    }

    R_ResetColumnBuffer();
    drawvars = olddrawvars;
  }
}

// CPhipps - some simple, useful wrappers for that function, for drawing patches from wads

// CPhipps - GNU C only suppresses generating a copy of a function if it is
// static inline; other compilers have different behaviour.
// This inline is _only_ for the function below

static void FUNC_V_DrawNumPatch(int x, int y, int scrn, int lump,
         int cm, enum patch_translation_e flags)
{
  V_DrawMemPatch(x, y, scrn, R_PatchByNum(lump), cm, flags);
}

static void FUNC_V_DrawNumPatchPrecise(float x, float y, int scrn, int lump,
         int cm, enum patch_translation_e flags)
{
  V_DrawMemPatch((int)x, (int)y, scrn, R_PatchByNum(lump), cm, flags);
}

static int currentPaletteIndex = 0;

//
// V_SetPalette
//
// CPhipps - New function to set the palette to palette number pal.
// Handles loading of PLAYPAL and calls I_SetPalette

void V_SetPalette(int pal)
{
  currentPaletteIndex = pal;

  //if (V_IsOpenGLMode()) {
    //gld_SetPalette(pal);
  //} else {
    I_SetPalette(pal);
  //}
}

void V_SetPlayPal(int playpal_index)
{
  dsda_SetPlayPal(playpal_index);
  R_UpdatePlayPal();
  V_SetPalette(currentPaletteIndex);

#if 0
  if (V_IsOpenGLMode())
  {
    gld_FlushTextures();
  }
#endif
}

//
// V_FillRect
//
// CPhipps - New function to fill a rectangle with a given colour
static void V_FillRect8(int scrn, int x, int y, int width, int height, byte colour)
{
  byte* dest = screens[scrn].data + x + y*screens[scrn].pitch;
  while (height--) {
    memset(dest, colour, width);
    dest += screens[scrn].pitch;
  }
}

static void WRAP_V_DrawLine(fline_t* fl, int color);
static void V_PlotPixel8(int scrn, int x, int y, byte color);

static void WRAP_V_DrawLineWu(fline_t* fl, int color);
static void V_PlotPixelWu8(int scrn, int x, int y, byte color, int weight);

#if 0
static void WRAP_gld_BeginUIDraw(void)
{
  gld_BeginUIDraw();
}
static void WRAP_gld_EndUIDraw(void)
{
  gld_EndUIDraw();
}
static void WRAP_gld_BeginAutomapDraw(void)
{
  gld_BeginAutomapDraw();
}
static void WRAP_gld_EndAutomapDraw(void)
{
  gld_EndAutomapDraw();
}
static void WRAP_gld_FillRect(int scrn, int x, int y, int width, int height, byte colour)
{
  gld_FillBlock(x,y,width,height,colour);
}
static void WRAP_gld_CopyRect(int srcscrn, int destscrn, int x, int y, int width, int height, enum patch_translation_e flags)
{
}
static void WRAP_gld_DrawBackground(const char *flatname, int n)
{
  gld_FillFlatName(flatname, 0, 0, SCREENWIDTH, SCREENHEIGHT, VPT_NONE);
}
static void WRAP_gld_FillFlat(int lump, int n, int x, int y, int width, int height, enum patch_translation_e flags)
{
  gld_FillFlat(lump, x, y, width, height, flags);
}
static void WRAP_gld_FillPatch(int lump, int n, int x, int y, int width, int height, enum patch_translation_e flags)
{
  gld_FillPatch(lump, x, y, width, height, flags);
}
static void WRAP_gld_DrawNumPatch(int x, int y, int scrn, int lump, int cm, enum patch_translation_e flags)
{
  gld_DrawNumPatch(x,y,lump,cm,flags);
}
static void WRAP_gld_DrawNumPatchPrecise(float x, float y, int scrn, int lump, int cm, enum patch_translation_e flags)
{
  gld_DrawNumPatch_f(x,y,lump,cm,flags);
}
static void V_PlotPixelGL(int scrn, int x, int y, byte color) {
  gld_DrawLine(x-1, y, x+1, y, color);
  gld_DrawLine(x, y-1, x, y+1, color);
}
static void V_PlotPixelWuGL(int scrn, int x, int y, byte color, int weight) {
  V_PlotPixelGL(scrn, x, y, color);
}
static void WRAP_gld_DrawLine(fline_t* fl, int color)
{
  gld_DrawLine_f(fl->a.fx, fl->a.fy, fl->b.fx, fl->b.fy, color);
}
#endif

static void NULL_BeginUIDraw(void) {}
static void NULL_EndUIDraw(void) {}
static void NULL_BeginAutomapDraw(void) {}
static void NULL_EndAutomapDraw(void) {}
static void NULL_FillRect(int scrn, int x, int y, int width, int height, byte colour) {}
static void NULL_CopyRect(int srcscrn, int destscrn, int x, int y, int width, int height, enum patch_translation_e flags) {}
static void NULL_FillFlat(int lump, int n, int x, int y, int width, int height, enum patch_translation_e flags) {}
static void NULL_FillPatch(int lump, int n, int x, int y, int width, int height, enum patch_translation_e flags) {}
static void NULL_DrawBackground(const char *flatname, int n) {}
static void NULL_DrawNumPatch(int x, int y, int scrn, int lump, int cm, enum patch_translation_e flags) {}
static void NULL_DrawNumPatchPrecise(float x, float y, int scrn, int lump, int cm, enum patch_translation_e flags) {}
static void NULL_PlotPixel(int scrn, int x, int y, byte color) {}
static void NULL_PlotPixelWu(int scrn, int x, int y, byte color, int weight) {}
static void NULL_DrawLine(fline_t* fl, int color) {}
static void NULL_DrawLineWu(fline_t* fl, int color) {}

static video_mode_t current_videomode = VID_MODESW;

V_BeginUIDraw_f V_BeginUIDraw = NULL_BeginUIDraw;
V_EndUIDraw_f V_EndUIDraw = NULL_EndUIDraw;
V_BeginUIDraw_f V_BeginAutomapDraw = NULL_BeginAutomapDraw;
V_EndUIDraw_f V_EndAutomapDraw = NULL_EndAutomapDraw;
V_CopyRect_f V_CopyRect = NULL_CopyRect;
V_FillRect_f V_FillRect = NULL_FillRect;
V_DrawNumPatch_f V_DrawNumPatch = NULL_DrawNumPatch;
V_DrawNumPatchPrecise_f V_DrawNumPatchPrecise = NULL_DrawNumPatchPrecise;
V_FillFlat_f V_FillFlat = NULL_FillFlat;
V_FillPatch_f V_FillPatch = NULL_FillPatch;
V_DrawBackground_f V_DrawBackground = NULL_DrawBackground;
V_PlotPixel_f V_PlotPixel = NULL_PlotPixel;
V_PlotPixelWu_f V_PlotPixelWu = NULL_PlotPixelWu;
V_DrawLine_f V_DrawLine = NULL_DrawLine;
V_DrawLineWu_f V_DrawLineWu = NULL_DrawLineWu;

//
// V_InitMode
//
void V_InitMode(video_mode_t mode) {
#if 0
  switch (mode) {
    case VID_MODESW:
#endif
      lprintf(LO_DEBUG, "V_InitMode: using software video mode\n");
      V_BeginUIDraw = NULL_BeginUIDraw; // [XA] no-op in software
      V_EndUIDraw = NULL_EndUIDraw; // [XA] ditto for the other begin/ends
      V_BeginAutomapDraw = NULL_BeginAutomapDraw;
      V_EndAutomapDraw = NULL_EndAutomapDraw;
      V_CopyRect = FUNC_V_CopyRect;
      V_FillRect = V_FillRect8;
      V_DrawNumPatch = FUNC_V_DrawNumPatch;
      V_DrawNumPatchPrecise = FUNC_V_DrawNumPatchPrecise;
      V_FillFlat = FUNC_V_FillFlat;
      V_FillPatch = FUNC_V_FillPatch;
      V_DrawBackground = FUNC_V_DrawBackground;
      V_PlotPixel = V_PlotPixel8;
      V_PlotPixelWu = V_PlotPixelWu8;
      V_DrawLine = WRAP_V_DrawLine;
      V_DrawLineWu = WRAP_V_DrawLineWu;
      current_videomode = VID_MODESW;
#if 0
      break;
    case VID_MODEGL:
      lprintf(LO_DEBUG, "V_InitMode: using OpenGL video mode\n");
      V_BeginUIDraw = WRAP_gld_BeginUIDraw;
      V_EndUIDraw = WRAP_gld_EndUIDraw;
      V_BeginAutomapDraw = WRAP_gld_BeginAutomapDraw;
      V_EndAutomapDraw = WRAP_gld_EndAutomapDraw;
      V_CopyRect = WRAP_gld_CopyRect;
      V_FillRect = WRAP_gld_FillRect;
      V_DrawNumPatch = WRAP_gld_DrawNumPatch;
      V_DrawNumPatchPrecise = WRAP_gld_DrawNumPatchPrecise;
      V_FillFlat = WRAP_gld_FillFlat;
      V_FillPatch = WRAP_gld_FillPatch;
      V_DrawBackground = WRAP_gld_DrawBackground;
      V_PlotPixel = V_PlotPixelGL;
      V_PlotPixelWu = V_PlotPixelWuGL;
      V_DrawLine = WRAP_gld_DrawLine;
      V_DrawLineWu = WRAP_gld_DrawLine;
      current_videomode = VID_MODEGL;
      break;
  }
#endif
}

dboolean V_IsSoftwareMode(void) {
  //return current_videomode == VID_MODESW;
  return 1;
}

dboolean V_IsOpenGLMode(void) {
  //return current_videomode == VID_MODEGL;
  return 0;
}

dboolean V_IsUILightmodeIndexed(void) {
  //return gl_ui_lightmode_indexed;
  return 0;
}

dboolean V_IsAutomapLightmodeIndexed(void) {
  //return gl_automap_lightmode_indexed;
  return 0;
}

void V_CopyScreen(int srcscrn, int destscrn)
{
  V_CopyRect(srcscrn, destscrn, 0, 0, SCREENWIDTH, SCREENHEIGHT, VPT_NONE);
}

//
// V_AllocScreen
//
void V_AllocScreen(screeninfo_t *scrn) {
  if (!scrn->not_on_heap)
    if ((scrn->pitch * scrn->height) > 0)
      //e6y: Clear the screen to black.
      scrn->data = Z_Calloc(scrn->pitch*scrn->height, 1);
}

//
// V_AllocScreens
//
void V_AllocScreens(void) {
  int i;

  for (i=0; i<NUM_SCREENS; i++)
    V_AllocScreen(&screens[i]);
}

//
// V_FreeScreen
//
void V_FreeScreen(screeninfo_t *scrn) {
  if (!scrn->not_on_heap) {
    Z_Free(scrn->data);
    scrn->data = NULL;
  }
}

//
// V_FreeScreens
//
void V_FreeScreens(void) {
  int i;

  for (i=0; i<NUM_SCREENS; i++)
    V_FreeScreen(&screens[i]);
}

static void V_PlotPixel8(int scrn, int x, int y, byte color) {
  screens[scrn].data[x+screens[scrn].pitch*y] = color;
}

#define PUTDOT(xx,yy,cc) V_PlotPixel(0,xx,yy,(byte)cc)

//
// WRAP_V_DrawLine()
//
// Draw a line in the frame buffer.
// Classic Bresenham w/ whatever optimizations needed for speed
//
// Passed the frame coordinates of line, and the color to be drawn
// Returns nothing
//
static void WRAP_V_DrawLine(fline_t* fl, int color)
{
  register int x;
  register int y;
  register int dx;
  register int dy;
  register int sx;
  register int sy;
  register int ax;
  register int ay;
  register int d;

#ifdef RANGECHECK         // killough 2/22/98
  static int fuck = 0;

  // For debugging only
  if
  (
       fl->a.x < 0 || fl->a.x >= SCREENWIDTH
    || fl->a.y < 0 || fl->a.y >= SCREENHEIGHT
    || fl->b.x < 0 || fl->b.x >= SCREENWIDTH
    || fl->b.y < 0 || fl->b.y >= SCREENHEIGHT
  )
  {
    //jff 8/3/98 use logical output routine
    lprintf(LO_DEBUG, "fuck %d \r", fuck++);
    return;
  }
#endif

  dx = fl->b.x - fl->a.x;
  ax = 2 * (dx<0 ? -dx : dx);
  sx = dx<0 ? -1 : 1;

  dy = fl->b.y - fl->a.y;
  ay = 2 * (dy<0 ? -dy : dy);
  sy = dy<0 ? -1 : 1;

  x = fl->a.x;
  y = fl->a.y;

  if (ax > ay)
  {
    d = ay - ax/2;
    while (1)
    {
      PUTDOT(x,y,color);
      if (x == fl->b.x) return;
      if (d>=0)
      {
        y += sy;
        d -= ax;
      }
      x += sx;
      d += ay;
    }
  }
  else
  {
    d = ax - ay/2;
    while (1)
    {
      PUTDOT(x, y, color);
      if (y == fl->b.y) return;
      if (d >= 0)
      {
        x += sx;
        d -= ay;
      }
      y += sy;
      d += ax;
    }
  }
}

//extern SDL_Surface *screen;
#define RGB2COLOR(r, g, b)\
  ((r >> screen->format->Rloss) << screen->format->Rshift) |\
  ((g >> screen->format->Gloss) << screen->format->Gshift) |\
  ((b >> screen->format->Bloss) << screen->format->Bshift)\

// Given 65536, we need 2048; 65536 / 2048 == 32 == 2^5
// Why 2048? ANG90 == 0x40000000 which >> 19 == 0x800 == 2048.
// The trigonometric correction is based on an angle from 0 to 90.
#define wu_fineshift 5

#define wu_weightbits 6

// Given 64 levels in the Col2RGB8 table, 65536 / 64 == 1024 == 2^10
#define wu_fixedshift (16-wu_weightbits)

//
// V_PlotPixelWu
//
// haleyjd 06/13/09: Pixel plotter for Wu line drawing.
//
static void V_PlotPixelWu8(int scrn, int x, int y, byte color, int weight)
{
  unsigned int bg_color = screens[scrn].data[x+screens[scrn].pitch*y];
  unsigned int *fg2rgb = Col2RGB8[weight];
  unsigned int *bg2rgb = Col2RGB8[64 - weight];
  unsigned int fg = fg2rgb[color];
  unsigned int bg = bg2rgb[bg_color];

  fg = (fg + bg) | 0x1f07c1f;
  V_PlotPixel(scrn, x, y, RGB32k[0][0][fg & (fg >> 15)]);
}

//
// WRAP_V_DrawLineWu
//
// haleyjd 06/12/09: Wu line drawing for the automap, with trigonometric
// brightness correction by SoM. I call this the Wu-McGranahan line drawing
// algorithm.
//
void WRAP_V_DrawLineWu(fline_t *fl, int color)
{
  unsigned short erroracc, erroradj, erroracctmp;
  int dx, dy, xdir = 1;
  int x, y;

  // swap end points if necessary
  if(fl->a.y > fl->b.y)
  {
    fpoint_t tmp = fl->a;

    fl->a = fl->b;
    fl->b = tmp;
  }

  // determine change in x, y and direction of travel
  dx = fl->b.x - fl->a.x;
  dy = fl->b.y - fl->a.y;

  if(dx < 0)
  {
    dx   = -dx;
    xdir = -xdir;
  }

  // detect special cases -- horizontal, vertical, and 45 degrees;
  // revert to Bresenham
  if(dx == 0 || dy == 0 || dx == dy)
  {
    V_DrawLine(fl, color);
    return;
  }

  // draw first pixel
  PUTDOT(fl->a.x, fl->a.y, color);

  x = fl->a.x;
  y = fl->a.y;

  if(dy > dx)
  {
    // line is y-axis major.
    erroracc = 0;
    erroradj = (unsigned short)(((unsigned int)dx << 16) / (unsigned int)dy);

    while(--dy)
    {
      erroracctmp = erroracc;

      erroracc += erroradj;

      // if error has overflown, advance x coordinate
      if(erroracc <= erroracctmp)
        x += xdir;

      y += 1; // advance y

      // the trick is in the trig!
      V_PlotPixelWu(0, x, y, (byte)color,
        finecosine[erroracc >> wu_fineshift] >> wu_fixedshift);
      V_PlotPixelWu(0, x + xdir, y, (byte)color,
        finesine[erroracc >> wu_fineshift] >> wu_fixedshift);
    }
  }
  else
  {
    // line is x-axis major.
    erroracc = 0;
    erroradj = (unsigned short)(((unsigned int)dy << 16) / (unsigned int)dx);

    while(--dx)
    {
      erroracctmp = erroracc;

      erroracc += erroradj;

      // if error has overflown, advance y coordinate
      if(erroracc <= erroracctmp)
        y += 1;

      x += xdir; // advance x

      // the trick is in the trig!
      V_PlotPixelWu(0, x, y, (byte)color,
        finecosine[erroracc >> wu_fineshift] >> wu_fixedshift);
      V_PlotPixelWu(0, x, y + 1, (byte)color,
        finesine[erroracc >> wu_fineshift] >> wu_fixedshift);
    }
  }

  // draw last pixel
  PUTDOT(fl->b.x, fl->b.y, color);
}


const unsigned char* V_GetPlaypal(void)
{
  dsda_playpal_t* playpal_data;

  playpal_data = dsda_PlayPalData();

  if (!playpal_data->lump)
  {
    int lump = W_GetNumForName(playpal_data->lump_name);
    const byte *data = W_LumpByNum(lump);
    playpal_data->length = W_LumpLength(lump);
    playpal_data->lump = Z_Malloc(playpal_data->length);
    memcpy(playpal_data->lump, data, playpal_data->length);
  }

  return playpal_data->lump;
}

void V_FreePlaypal(void)
{
  dsda_FreePlayPal();
}

int V_GetPlaypalCount(void)
{
  V_GetPlaypal(); // ensure playpal data is initialized
  return (dsda_PlayPalData()->length / PALETTE_SIZE);
}

void V_ClearBorder(void)
{
  int bordtop, bordbottom, bordleft, bordright;

  if (render_stretch_hud == patch_stretch_fit_to_width)
    return;

  bordleft = wide_offsetx;
  bordright = wide_offset2x - wide_offsetx;
  bordtop = wide_offsety;
  bordbottom = wide_offset2y - wide_offsety;

  if (bordtop > 0)
  {
    // Top
    V_FillRect(0, 0, 0, SCREENWIDTH, bordtop, 0);
    // Bottom
    V_FillRect(0, 0, SCREENHEIGHT - bordbottom, SCREENWIDTH, bordbottom, 0);
  }

  if (bordleft > 0)
  {
    // Left
    V_FillRect(0, 0, bordtop, bordleft, SCREENHEIGHT - bordbottom - bordtop, 0);
    // Right
    V_FillRect(0, SCREENWIDTH - bordright, bordtop, bordright, SCREENHEIGHT - bordbottom - bordtop, 0);
  }
}

// DWF 2012-05-10
// Euclid's algorithm to find the greatest common divisor.
static unsigned int gcd (unsigned int n, unsigned int d) { return (d ? gcd(d, n%d) : n); }

// DWF 2012-05-10
// Reduce aspect ratio fractions to make the log messages nicer.  Even if
// integer math were still being used for FPS scaling, this would not
// necessarily speed it up, but it does no harm.
// Order of parameters (numerator, denominator) doesn't matter.
static void ReduceFraction (unsigned *num1, unsigned *num2)
{
  unsigned int g;
  assert(*num1 || *num2);
  g = gcd (*num1, *num2);
  *num1 /= g;
  *num2 /= g;
}

// DWF 2012-05-01
// C substitute for C++ std::swap.
static void swap(unsigned int *num1, unsigned int *num2)
{
  unsigned int temp = *num1;
  *num1 = *num2;
  *num2 = temp;
}

// DWF 2012-05-01
// Set global variables for video scaling.
void SetRatio(int width, int height)
{
  lprintf(LO_DEBUG, "SetRatio: width/height parameters %dx%d\n", width, height);

  ratio_multiplier = width;
  ratio_scale = height;
  ReduceFraction(&ratio_multiplier, &ratio_scale);

  // The terms storage aspect ratio, pixel aspect ratio, and display aspect
  // ratio came from Wikipedia.  SAR x PAR = DAR
  lprintf(LO_DEBUG, "SetRatio: storage aspect ratio %u:%u\n", ratio_multiplier, ratio_scale);
  if (height == 200 || height == 400)
  {
    lprintf(LO_DEBUG, "SetRatio: recognized VGA mode with pixel aspect ratio 5:6\n");
    ratio_multiplier = width * 5;
    ratio_scale = height * 6;
    ReduceFraction(&ratio_multiplier, &ratio_scale);
  }
  else
  {
    lprintf(LO_DEBUG, "SetRatio: assuming square pixels\n");
  }
  lprintf(LO_DEBUG, "SetRatio: display aspect ratio %u:%u\n", ratio_multiplier, ratio_scale);

  // If user wants to force aspect ratio, let them.
  {
    unsigned int new_multiplier = ratio_multiplier;
    unsigned int new_scale = ratio_scale;
    int render_aspect = dsda_IntConfig(dsda_config_render_aspect);

    // Hardcoded to match render_aspects_list
    switch (render_aspect)
    {
      case 0:
        break;
      case 1:
        new_multiplier = 16;
        new_scale = 9;
        break;
      case 2:
        new_multiplier = 16;
        new_scale = 10;
        break;
      case 3:
        new_multiplier = 4;
        new_scale = 3;
        break;
      case 4:
        new_multiplier = 5;
        new_scale = 4;
        break;
      default:
        lprintf(LO_ERROR, "SetRatio: render_aspect has invalid value %d\n", render_aspect);
    }

    if (ratio_multiplier != new_multiplier || ratio_scale != new_scale)
    {
      lprintf(LO_DEBUG, "SetRatio: overruled by user configuration setting\n");
      ratio_multiplier = new_multiplier;
      ratio_scale = new_scale;
      lprintf(LO_DEBUG, "SetRatio: revised display aspect ratio %u:%u\n", ratio_multiplier, ratio_scale);
    }
  }

  gl_ratio = RMUL * ratio_multiplier / ratio_scale;
  lprintf(LO_DEBUG, "SetRatio: gl_ratio %f\n", gl_ratio);

  // Calculate modified multiplier following the pattern of the old
  // BaseRatioSizes table in PrBoom-Plus 2.5.1.3.
  swap(&ratio_multiplier, &ratio_scale);
  ratio_multiplier *= 4;
  ratio_scale *= 3;
  ReduceFraction(&ratio_multiplier, &ratio_scale);

  tallscreen = (ratio_scale < ratio_multiplier);
  if (tallscreen)
  {
    lprintf(LO_DEBUG, "SetRatio: tallscreen aspect recognized; flipping multiplier\n");
    swap(&ratio_multiplier, &ratio_scale);
    psprite_offset = (int)(6.5*FRACUNIT);
  }
  else
  {
    psprite_offset = 0;
  }
  lprintf(LO_DEBUG, "SetRatio: multiplier %u/%u\n", ratio_multiplier, ratio_scale);

  // The rest is carried over from CheckRatio in PrBoom-Plus 2.5.1.3.
  if (tallscreen)
  {
    WIDE_SCREENWIDTH = SCREENWIDTH;
    WIDE_SCREENHEIGHT = SCREENHEIGHT * ratio_multiplier / ratio_scale;
  }
  else
  {
    WIDE_SCREENWIDTH = SCREENWIDTH * ratio_multiplier / ratio_scale;
    WIDE_SCREENHEIGHT = SCREENHEIGHT;
  }

  WIDE_SCREENWIDTH = MAX(1, WIDE_SCREENWIDTH);
  WIDE_SCREENHEIGHT = MAX(1, WIDE_SCREENHEIGHT);

  yaspectmul = Scale((320<<FRACBITS), WIDE_SCREENHEIGHT, 200 * WIDE_SCREENWIDTH);

  dsda_EvaluatePatchScale();

  SCREEN_320x200 =
    (SCREENWIDTH == 320) && (SCREENHEIGHT == 200) &&
    (WIDE_SCREENWIDTH == 320) && (WIDE_SCREENHEIGHT == 200);

  // [FG] support widescreen status bar backgrounds
  ST_SetScaledWidth();
}

void V_GetWideRect(int *x, int *y, int *w, int *h, enum patch_translation_e flags)
{
  stretch_param_t *params = dsda_StretchParams(flags);
  int sx = *x;
  int sy = *y;

  *x = params->video->x1lookup[*x];
  *y = params->video->y1lookup[*y];
  *w = params->video->x2lookup[sx + *w - 1] - *x + 1;
  *h = params->video->y2lookup[sy + *h - 1] - *y + 1;
  *x += params->deltax1;
  *y += params->deltay1;
}

//
// V_BestColor
//
// Adapted from zdoom -- thanks to Randy Heit.
//
// This always assumes a 256-color palette;
// it's intended for use in startup functions to match hard-coded
// color values to the best fit in the game's palette (allows
// cross-game usage among other things).
//
int V_BestColor(const unsigned char *palette, int r, int g, int b)
{
  int color;

  // use color 0 as a worst-case match for any color
  int bestcolor = 0;
  int bestdist = 257 * 257 + 257 * 257 + 257 * 257;

  for (color = 0; color < 256; color++)
  {
    int dr, dg, db, dist;

    dr = r - *palette++;
    dg = g - *palette++;
    db = b - *palette++;

    dist = dr * dr + dg * dg + db * db;

    if (dist < bestdist)
    {
      // exact match
      if (dist == 0)
        return color;

      bestdist = dist;
      bestcolor = color;
    }
  }

  return bestcolor;
}

// Alt-Enter: fullscreen <-> windowed
void V_ToggleFullscreen(void)
{
  //dsda_UpdateIntConfig(dsda_config_use_fullscreen, !desired_fullscreen, true);
}

void V_ChangeScreenResolution(void)
{
  I_UpdateVideoMode();

#if 0
  if (V_IsOpenGLMode())
  {
    gld_PreprocessLevel();
  }
#endif
}

void V_FillRectVPT(int scrn, int x, int y, int width, int height, byte color, enum patch_translation_e flags)
{
  V_GetWideRect(&x, &y, &width, &height, flags);
  V_FillRect(scrn, x, y, width, height, color);
}

int V_FillHeightVPT(int scrn, int y, int height, byte color, enum patch_translation_e flags)
{
  stretch_param_t *params = dsda_StretchParams(flags);
  int sy = y;

  y = params->video->y1lookup[y];
  height = params->video->y2lookup[sy + height - 1] - y + 1;
  y += params->deltay1;
  V_FillRect(scrn, 0, y, SCREENWIDTH, height, color);

  return height;
}

// heretic

#define HERETIC_RAW_SCREEN_SIZE 64000

// heretic_note: is something already implemented to handle this?
void V_DrawRawScreen(const char *lump_name)
{
  V_DrawRawScreenSection(lump_name, 0, 0, 200);
}

void V_DrawRawScreenSection(const char *lump_name, int source_offset, int dest_y_offset, int dest_y_limit)
{
  int i, j;
  float x_factor, y_factor;
  int x_offset /*, y_offset*/;
  const byte* raw;

  // e6y: wide-res
  // NOTE: the size isn't quite right on all resolutions,
  // which causes the black bars on the edge of heretic E3's
  // bottom endscreen to overlap the top screen during scrolling.
  // this happens in both software and GL at the time of writing.
  V_ClearBorder();

  // custom widescreen assets are a different format
  {
    int lump;

    lump = W_CheckNumForName(lump_name);
    if (W_LumpLength(lump) != HERETIC_RAW_SCREEN_SIZE)
    {
      V_DrawNamePatch(0, 0, 0, lump_name, CR_DEFAULT, VPT_STRETCH);
      return;
    }
  }

  x_factor = (float)SCREENWIDTH / 320;
  y_factor = (float)SCREENHEIGHT / 200;

  if (y_factor < x_factor)
    x_factor = y_factor;

  x_offset = (int)((SCREENWIDTH - (x_factor * 320)) / 2);
  //y_offset = (int)((dest_y_offset * y_factor) - (source_offset * y_factor / 320));

  // TODO: create a V_FillRaw alias and call that instead of the gld_ func directly,
  // though that means there needs to be a software version too (that's ideally a
  // bit more efficient than the current code's thousands-of-little-boxes approach)
#if 0
  if (V_IsOpenGLMode()) {
    gld_FillRawName(lump_name, x_offset, y_offset, 320, 200, 320 * x_factor, 200 * y_factor, VPT_STRETCH_REAL);
    return;
  }
#endif

  raw = (const byte *)W_LumpByName(lump_name) + source_offset;

  for (j = dest_y_offset; j < dest_y_offset + dest_y_limit; ++j)
    for (i = 0; i < 320; ++i, ++raw)
    {
      int x, y, width, height;

      x = (int)(i * x_factor);
      y = (int)(j * y_factor);
      width = (int)((i + 1) * x_factor) - x;
      height = (int)((j + 1) * y_factor) - y;

      V_FillRect(0, x_offset + x, y, width, height, *raw);
    }
}

void V_DrawShadowedNumPatch(int x, int y, int lump)
{
  V_DrawNumPatch(x, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
}

void V_DrawShadowedNamePatch(int x, int y, const char* name)
{
  V_DrawNamePatch(x, y, 0, name, CR_DEFAULT, VPT_STRETCH);
}

void V_DrawTLNumPatch(int x, int y, int lump)
{
  V_DrawNumPatch(x, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
}

void V_DrawTLNamePatch(int x, int y, const char* name)
{
  V_DrawNamePatch(x, y, 0, name, CR_DEFAULT, VPT_STRETCH);
}

void V_DrawAltTLNumPatch(int x, int y, int lump)
{
  V_DrawNumPatch(x, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
}

// void V_DrawShadowedPatch(int x, int y, patch_t *patch)
// {
//     int count, col;
//     column_t *column;
//     pixel_t *desttop, *dest;
//     byte *source;
//     pixel_t *desttop2, *dest2;
//     int w;
//
//     y -= SHORT(patch->topoffset);
//     x -= SHORT(patch->leftoffset);
//
//     if (x < 0
//      || x + SHORT(patch->width) > ORIGWIDTH
//      || y < 0
//      || y + SHORT(patch->height) > ORIGHEIGHT)
//     {
//         I_Error("Bad V_DrawShadowedPatch");
//     }
//
//     col = 0;
//     desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);
//     desttop2 = dest_screen + (((y + 2) * dy) >> FRACBITS) * SCREENWIDTH + (((x + 2) * dx) >> FRACBITS);
//
//     w = SHORT(patch->width);
//     for (; col < w << FRACBITS; x++, col+=dxi, desttop++, desttop2++)
//     {
//         column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col >> FRACBITS]));
//
//         // step through the posts in a column
//
//         while (column->topdelta != 0xff)
//         {
//             int srccol = 0;
//             source = (byte *) column + 3;
//             dest = desttop + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
//             dest2 = desttop2 + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
//             count = (column->length * dy) >> FRACBITS;
//
//             while (count--)
//             {
//                 *dest2 = tinttable[((*dest2) << 8)];
//                 dest2 += SCREENWIDTH;
//                 *dest = source[srccol >> FRACBITS];
//                 srccol += dyi;
//                 dest += SCREENWIDTH;
//
//             }
//             column = (column_t *) ((byte *) column + column->length + 4);
//         }
//     }
// }

// void V_DrawTLPatch(int x, int y, patch_t * patch)
// {
//     int count, col;
//     column_t *column;
//     pixel_t *desttop, *dest;
//     byte *source;
//     int w;
//
//     y -= SHORT(patch->topoffset);
//     x -= SHORT(patch->leftoffset);
//
//     if (x < 0
//      || x + SHORT(patch->width) > ORIGWIDTH
//      || y < 0
//      || y + SHORT(patch->height) > ORIGHEIGHT)
//     {
//         I_Error("Bad V_DrawTLPatch");
//     }
//
//     col = 0;
//     desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);
//
//     w = SHORT(patch->width);
//     for (; col < w << FRACBITS; x++, col+=dxi, desttop++)
//     {
//         column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col >> FRACBITS]));
//
//         // step through the posts in a column
//
//         while (column->topdelta != 0xff)
//         {
//             int srccol = 0;
//             source = (byte *) column + 3;
//             dest = desttop + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
//             count = (column->length * dy) >> FRACBITS;
//
//             while (count--)
//             {
//                 *dest = tinttable[((*dest) << 8) + source[srccol >> FRACBITS]];
//                 srccol += dyi;
//                 dest += SCREENWIDTH;
//             }
//             column = (column_t *) ((byte *) column + column->length + 4);
//         }
//     }
// }
