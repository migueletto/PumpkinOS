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
 * DESCRIPTION:  none
 *
 *-----------------------------------------------------------------------------*/

#ifndef __HULIB__
#define __HULIB__

#include "v_video.h"  //jff 2/16/52 include color range defs

#include "dsda/font.h"

/* background and foreground screen numbers
 * different from other modules. */
//e6y #define BG      1
#define FG      0

#define HU_MAXLINELENGTH  80

// Text Line widget
typedef struct
{
  // left-justified position of scrolling text window
  int   x;
  int   y;

  const patchnum_t* f;                    // font
  int   sc;                             // start character
  //const char *cr;                       //jff 2/16/52 output color range
  // Proff - Made this an int again. Needed for OpenGL
  int   cm;                         //jff 2/16/52 output color range

  // killough 1/23/98: Support multiple lines:
  #define MAXLINES 25

  int   linelen;
  char  l[HU_MAXLINELENGTH*MAXLINES+1]; // line of text
  int   len;                            // current line length

  // e6y: wide-res
  enum patch_translation_e flags;

  int line_height;
  int space_width;
} hu_textline_t;

//
// textline code
//

// clear a line of text
void HUlib_clearTextLine(hu_textline_t *t);

void HUlib_initTextLine
(
  hu_textline_t *t,
  int x,
  int y,
  const dsda_font_t *f,
  int cm,    //jff 2/16/98 add color range parameter
  enum patch_translation_e flags
);

// returns success
dboolean HUlib_addCharToTextLine(hu_textline_t *t, char ch);

// draws tline
void HUlib_drawTextLine(hu_textline_t *l, dboolean drawcursor);
void HUlib_drawOffsetTextLine(hu_textline_t* l, int offset);

//e6y
void HUlib_setTextXCenter(hu_textline_t* t);

char HUlib_Color(int cm);

#endif
