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
 *      System specific interface stuff.
 *
 *-----------------------------------------------------------------------------*/

#include "i_video.h"
#include "i_system.h"
#include "d_main.h"
#include "r_things.h"
#include "r_draw.h"
#include "r_plane.h"
#include "r_main.h"
#include "f_wipe.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "am_map.h"
#include "doomstat.h"

#include "host.h"
#include "debug.h"

struct color {
  uint8_t b;
  uint8_t g;
  uint8_t r;
  uint16_t c16;
};

static struct color colors[256];
static byte *I_VideoBuffer = NULL;
static dboolean initialized = false;

void I_PreInitGraphics(void) {
}

static void I_InitBuffersRes(void) {
  R_InitMeltRes();
  R_InitSpritesRes();
  R_InitBuffersRes();
  R_InitPlanesRes();
  R_InitVisplanesRes();
}

void I_InitScreenResolution(void) {
  int i;

  if (hires) {
    SCREENWIDTH = 640;
    SCREENHEIGHT = 400;
    SCREENPITCH = SCREENWIDTH;
  } else {
    SCREENWIDTH = 320;
    SCREENHEIGHT = 200;
    SCREENPITCH = SCREENWIDTH;
  }

  V_InitMode(VID_MODESW);
  V_FreeScreens();

  for (i = 0; i < 3; i++) {
    screens[i].width = SCREENWIDTH;
    screens[i].height = SCREENHEIGHT;
    screens[i].pitch = SCREENPITCH;
  }

  screens[4].width = SCREENWIDTH;
  screens[4].height = SCREENHEIGHT;
  screens[4].pitch = SCREENPITCH;

  I_InitBuffersRes();

  DG_debug(DEBUG_INFO, "I_InitScreenResolution: %dx%d (hires %d)", SCREENWIDTH, SCREENHEIGHT, hires);
}

void I_InitGraphics(void) {
  if (!initialized) {
    I_AtExit(I_ShutdownGraphics, true, "I_ShutdownGraphics", exit_priority_normal);
    DG_debug(DEBUG_INFO, "I_InitGraphics: %dx%d", SCREENWIDTH, SCREENHEIGHT);
    I_UpdateVideoMode();
    initialized = true;
  }
}

static void I_UploadNewPalette(void) {
  byte *palette;
  uint32_t r, g, b;
  int num, i;

  num = W_GetNumForName("PLAYPAL");
  palette = (byte *)W_LumpByNum(num);

  for (i = 0; i < 256; i++) {
    r = colors[i].r = *palette++;
    g = colors[i].g = *palette++;
    b = colors[i].b = *palette++;
    colors[i].c16 = DG_color16(r, g, b);
  }
}

void I_UpdateVideoMode(void) {
  I_VideoBuffer = malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(*I_VideoBuffer));
  memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT * sizeof(*I_VideoBuffer));

  screens[0].not_on_heap = true;
  screens[0].data = (unsigned char *)I_VideoBuffer;
  screens[0].pitch = SCREENWIDTH;

  V_AllocScreens();
  R_InitBuffer(SCREENWIDTH, SCREENHEIGHT);

  R_ExecuteSetViewSize();

  V_SetPalette(0);
  I_UploadNewPalette();

  ST_SetResolution();
  AM_SetResolution();

  DG_debug(DEBUG_INFO, "I_UpdateVideoMode: %dx%d", SCREENWIDTH, SCREENHEIGHT);
}

void I_ShutdownGraphics(void) {
  if (I_VideoBuffer) {
    free(I_VideoBuffer);
    I_VideoBuffer = NULL;
  }
}

void I_SetPalette(int pal) {
  if (pal != 0) {
    DG_debug(DEBUG_INFO, "I_SetPalette: non-zero palette %d", pal);
  }
}

void I_FinishUpdate(void) {
  uint16_t *line_out;
  uint8_t *line_in;
  uint32_t i, n;

  if (!initialized || !I_VideoBuffer) return;

  line_in  = I_VideoBuffer;
  line_out = DG_GetScreenBuffer();

  n = SCREENWIDTH * SCREENHEIGHT;
  for (i = 0; i < n; i++) {
    *line_out = colors[*line_in].c16;
    line_in++;
    line_out++;
  }

  DG_DrawFrame();
}

int I_ScreenShot(const char *fname) {
  return 0;
}

static void I_GetEvent(void) {
  event_t event;
  int pressed, r;
  unsigned char key;

  for (;;) {
    r = DG_GetKey(&pressed, &key);
    if (r == 0) break;
    if (r == -1) {
      M_Quit();
      break;
    }

    if (pressed) {
      event.type = ev_keydown;
      event.data1.i = key;

      if (event.data1.i != 0) {
        D_PostEvent(&event);
      }
    } else {
      event.type = ev_keyup;
      event.data1.i = key;

      if (event.data1.i != 0) {
        D_PostEvent(&event);
      }
      break;
    }
  }
}

/* I_StartTic
 * Called by D_DoomLoop,
 * called before processing each tic in a frame.
 * Quick syncronous operations are performed here.
 * Can call D_PostEvent.
 */
void I_StartTic(void) {
  I_GetEvent();
}

/* I_StartFrame
 * Called by D_DoomLoop,
 * called before processing any tics in a frame
 * (just after displaying a frame).
 * Time consuming syncronous operations
 * are performed here (joystick reading).
 * Can call D_PostEvent.
 */

void I_StartFrame(void) {
}
