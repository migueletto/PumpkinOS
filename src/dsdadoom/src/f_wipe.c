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
 *      Mission begin melt/wipe screen special effect.
 *
 *-----------------------------------------------------------------------------
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "z_zone.h"
#include "doomdef.h"
#include "i_video.h"
#include "v_video.h"
#include "m_random.h"
#include "f_wipe.h"
//#include "gl_struct.h"
#include "e6y.h"//e6y

#include "dsda/settings.h"

//
// SCREEN WIPE PACKAGE
//

// Parts re-written to support true-color video modes. Column-major
// formatting removed. - POPE

// CPhipps - macros for the source and destination screens
#define SRC_SCR 2
#define DEST_SCR 3

static screeninfo_t wipe_scr_start;
static screeninfo_t wipe_scr_end;
static screeninfo_t wipe_scr;

// e6y: resolution limitation is removed
static int *y_lookup = NULL;

// e6y: resolution limitation is removed
void R_InitMeltRes(void)
{
  if (y_lookup) Z_Free(y_lookup);

  y_lookup = Z_Calloc(1, SCREENWIDTH * sizeof(*y_lookup));
}

static int wipe_initMelt(int ticks)
{
  int i;

  if (V_IsSoftwareMode())
  {
    // copy start screen to main screen
    for(i=0;i<SCREENHEIGHT;i++)
    memcpy(wipe_scr.data+i*wipe_scr.pitch,
           wipe_scr_start.data+i*wipe_scr_start.pitch,
           SCREENWIDTH);
  }

  // setup initial column positions (y<0 => not ready to scroll yet)
  y_lookup[0] = -(M_Random()%16);
  for (i=1;i<SCREENWIDTH;i++)
    {
      int r = (M_Random()%3) - 1;
      y_lookup[i] = y_lookup[i-1] + r;
      if (y_lookup[i] > 0)
        y_lookup[i] = 0;
      else
        if (y_lookup[i] == -16)
          y_lookup[i] = -15;
    }
  return 0;
}

static int wipe_doMelt(int ticks)
{
  dboolean done = true;
  int i;

  while (ticks--) {
    for (i=0;i<(SCREENWIDTH);i++) {
      if (y_lookup[i]<0) {
        y_lookup[i]++;
        done = false;
        continue;
      }
      if (y_lookup[i] < SCREENHEIGHT) {
        byte *s, *d;
        int j, dy;

        /* cph 2001/07/29 -
          *  The original melt rate was 8 pixels/sec, i.e. 25 frames to melt
          *  the whole screen, so make the melt rate depend on SCREENHEIGHT
          *  so it takes no longer in high res
          */
        dy = (y_lookup[i] < 16) ? y_lookup[i]+1 : SCREENHEIGHT/25;
        if (y_lookup[i]+dy >= SCREENHEIGHT)
          dy = SCREENHEIGHT - y_lookup[i];

       if (V_IsSoftwareMode()) {
        s = wipe_scr_end.data    + (y_lookup[i]*wipe_scr_end.pitch+i);
        d = wipe_scr.data        + (y_lookup[i]*wipe_scr.pitch+i);
        for (j=dy;j;j--) {
          d[0] = s[0];
          d += wipe_scr.pitch;
          s += wipe_scr_end.pitch;
        }
       }
        y_lookup[i] += dy;
       if (V_IsSoftwareMode()) {
        s = wipe_scr_start.data  + i;
        d = wipe_scr.data        + (y_lookup[i]*wipe_scr.pitch+i);
        for (j=SCREENHEIGHT-y_lookup[i];j;j--) {
          d[0] = s[0];
          d += wipe_scr.pitch;
          s += wipe_scr_end.pitch;
        }
       }
        done = false;
      }
    }
  }
#if 0
  if (V_IsOpenGLMode())
  {
    gld_wipe_doMelt(ticks, y_lookup);
  }
#endif
  return done;
}

// CPhipps - modified to allocate and deallocate screens[2 to 3] as needed, saving memory

static int wipe_exitMelt(int ticks)
{
#if 0
  if (V_IsOpenGLMode())
  {
    gld_wipe_exitMelt(ticks);
    return 0;
  }
#endif

  V_FreeScreen(&wipe_scr_start);
  wipe_scr_start.width = 0;
  wipe_scr_start.height = 0;
  V_FreeScreen(&wipe_scr_end);
  wipe_scr_end.width = 0;
  wipe_scr_end.height = 0;
  // Paranoia
  screens[SRC_SCR] = wipe_scr_start;
  screens[DEST_SCR] = wipe_scr_end;
  return 0;
}

int wipe_StartScreen(void)
{
  if(dsda_PendingSkipWipe() || wasWiped) return 0;//e6y
  wasWiped = true;//e6y

#if 0
  if (V_IsOpenGLMode())
  {
    gld_wipe_StartScreen();
    return 0;
  }
#endif

  wipe_scr_start.width = SCREENWIDTH;
  wipe_scr_start.height = SCREENHEIGHT;
  wipe_scr_start.pitch = screens[0].pitch;

  //e6y: fixed slowdown at 1024x768 on some systems
  if (!(wipe_scr_start.pitch % 1024))
    wipe_scr_start.pitch += 32;

  wipe_scr_start.not_on_heap = false;
  V_AllocScreen(&wipe_scr_start);
  screens[SRC_SCR] = wipe_scr_start;
  V_CopyScreen(0, SRC_SCR); // Copy start screen to buffer
  return 0;
}

int wipe_EndScreen(void)
{
  if(dsda_PendingSkipWipe() || !wasWiped) return 0;//e6y
  wasWiped = false;//e6y

#if 0
  if (V_IsOpenGLMode())
  {
    gld_wipe_EndScreen();
    return 0;
  }
#endif

  wipe_scr_end.width = SCREENWIDTH;
  wipe_scr_end.height = SCREENHEIGHT;
  wipe_scr_end.pitch = screens[0].pitch;

  //e6y: fixed slowdown at 1024x768 on some systems
  if (!(wipe_scr_end.pitch % 1024))
    wipe_scr_end.pitch += 32;

  wipe_scr_end.not_on_heap = false;
  V_AllocScreen(&wipe_scr_end);
  screens[DEST_SCR] = wipe_scr_end;
  V_CopyScreen(0, DEST_SCR); // Copy end screen to buffer
  V_CopyScreen(SRC_SCR, 0); // restore start screen
  return 0;
}

// killough 3/5/98: reformatted and cleaned up
int wipe_ScreenWipe(int ticks)
{
  static dboolean go;                               // when zero, stop the wipe

  if (!dsda_RenderWipeScreen())
    return 0;//e6y

  if (!go)                                         // initial stuff
  {
    go = 1;
    wipe_scr = screens[0];
    wipe_initMelt(ticks);
  }

  // do a piece of wipe-in
  if (wipe_doMelt(ticks))     // final stuff
  {
    wipe_exitMelt(ticks);
    go = 0;
  }
  return !go;
}
