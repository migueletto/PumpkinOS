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
 * DESCRIPTION:  heads-up text and input code
 *
 *-----------------------------------------------------------------------------
 */

#include "doomdef.h"
#include "doomstat.h"
#include "v_video.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "r_main.h"
#include "r_draw.h"

#include "dsda/stretch.h"

#define HU_COLOR 0x30

char HUlib_Color(int cm)
{
  return HU_COLOR + cm;
}

////////////////////////////////////////////////////////
//
// Basic text line widget
//
////////////////////////////////////////////////////////

//
// HUlib_clearTextLine()
//
// Blank the internal text line in a hu_textline_t widget
//
// Passed a hu_textline_t, returns nothing
//
void HUlib_clearTextLine(hu_textline_t* t)
{
  t->linelen =         // killough 1/23 98: support multiple lines
    t->len = 0;
  t->l[0] = 0;
}

//
// HUlib_initTextLine()
//
// Initialize a hu_textline_t widget. Set the position, font, start char
// of the font, and color range to be used.
//
// Passed a hu_textline_t, and the values used to initialize
// Returns nothing
//
void HUlib_initTextLine(hu_textline_t* t, int x, int y,
      const dsda_font_t* f, int cm, enum patch_translation_e flags )
  //jff 2/16/98 add color range parameter
{
  t->x = x;
  t->y = y;
  t->f = f->font;
  t->sc = f->start;
  t->cm = cm;
  t->flags = flags;
  t->line_height = f->line_height;
  t->space_width = f->space_width;
  HUlib_clearTextLine(t);
}

//
// HUlib_addCharToTextLine()
//
// Adds a character at the end of the text line in a hu_textline_t widget
//
// Passed the hu_textline_t and the char to add
// Returns false if already at length limit, true if the character added
//
dboolean HUlib_addCharToTextLine
( hu_textline_t*  t,
  char      ch )
{
  // killough 1/23/98 -- support multiple lines
  if (t->linelen == HU_MAXLINELENGTH)
    return false;
  else
  {
    t->linelen++;
    if (ch == '\n')
      t->linelen=0;

    t->l[t->len++] = ch;
    t->l[t->len] = 0;
    return true;
  }

}

//
// HUlib_drawTextLine()
//
// Draws a hu_textline_t widget
//
// Passed the hu_textline_t and flag whether to draw a cursor
// Returns nothing
//
void HUlib_drawTextLine
( hu_textline_t* l,
  dboolean drawcursor )
{

  int     i;
  int     w;
  int     x;
  unsigned char c;
  int oc = l->cm; //jff 2/17/98 remember default color
  int y;          // killough 1/18/98 -- support multiple lines

  // draw the new stuff

  x = l->x;
  y = l->y;
  for (i=0;i<l->len;i++)
  {
    c = toupper(l->l[i]); //jff insure were not getting a cheap toupper conv.

    if (c=='\n')         // killough 1/18/98 -- support multiple lines
    {
      x = l->x;
      y += l->line_height;
    }
    else if (c=='\t')    // killough 1/23/98 -- support tab stops
      x=x-x%80+80;
    else if (c=='\x1b')  //jff 2/17/98 escape code for color change
    {                    //jff 3/26/98 changed to actual escape char
      if (++i < l->len)
      {
        if (l->l[i] >= HU_COLOR && l->l[i] < HU_COLOR + CR_LIMIT)
          l->cm = l->l[i] - HU_COLOR;
        else if (l->l[i] < HU_COLOR)
          x += l->l[i];
      }
    }
    else  if (c != ' ' && c >= l->sc && c <= 127)
    {
      w = l->f[c - l->sc].width;
      if (x+w-l->f[c - l->sc].leftoffset > BASE_WIDTH)
        break;
      // killough 1/18/98 -- support multiple lines:
      // CPhipps - patch drawing updated
      V_DrawNumPatch(x, y, FG, l->f[c - l->sc].lumpnum, l->cm, VPT_TRANS | l->flags);
      x += w;
    }
    else
    {
      x += l->space_width;
      if (x >= BASE_WIDTH)
      break;
    }
  }
  l->cm = oc; //jff 2/17/98 restore original color

  // draw the cursor if requested
  if (drawcursor && x + l->f['_' - l->sc].width <= BASE_WIDTH)
  {
    // killough 1/18/98 -- support multiple lines
    // CPhipps - patch drawing updated
    V_DrawNumPatch(x, y, FG, l->f['_' - l->sc].lumpnum, CR_DEFAULT, VPT_NONE | l->flags);
  }
}

void HUlib_drawOffsetTextLine(hu_textline_t* l, int offset)
{
  int old_y;

  old_y = l->y;
  l->y += offset;
  HUlib_drawTextLine(l, false);
  l->y = old_y;
}

//
// HUlib_setTextXCenter()
//
// Centering a hu_textline_t
//
// Passed the hu_textline_t
// Returns nothing
//
void HUlib_setTextXCenter(hu_textline_t* t)
{
  char *s = t->l;
  t->x = 320;
  while (*s)
  {
    int c = toupper(*(s++)) - HU_FONTSTART;
    t->x -= (c < 0 || c > HU_FONTSIZE ? t->space_width : t->f[c].width);
  }
  if (t->x < 0)
    t->x = 0;

  t->x >>= 1;
}
