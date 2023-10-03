//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
//	DOOM graphics stuff for SDL.
//


#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "crispy.h"

#include "config.h"
#include "d_loop.h"
#include "deh_str.h"
#include "doomtype.h"
#include "i_input.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "tables.h"
#include "v_diskicon.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "host.h"
#include "debug.h"

int SCREENWIDTH, SCREENHEIGHT, SCREENHEIGHT_4_3;
int NONWIDEWIDTH; // [crispy] non-widescreen SCREENWIDTH
int WIDESCREENDELTA; // [crispy] horizontal widescreen offset

// These are (1) the window (or the full screen) that our game is rendered to
// and (2) the renderer that scales the texture (see below) into this window.

// Window title

static const char *window_title = "";

// These are (1) the 320x200x8 paletted buffer that we draw to (i.e. the one
// that holds I_VideoBuffer), (2) the 320x200x32 RGBA intermediate buffer that
// we blit the former buffer to, (3) the intermediate 320x200 texture that we
// load the RGBA buffer to and that we render into another texture (4) which
// is upscaled by an integer factor UPSCALE using "nearest" scaling and which
// in turn is finally rendered to screen using "linear" scaling.

struct color {
  uint8_t b;
  uint8_t g;
  uint8_t r;
  uint16_t c16;
};

static struct color colors[256];

// display has been set up?

static boolean initialized = false;

// disable mouse?

int usemouse = 1;

// Aspect ratio correction mode

int aspect_ratio_correct = false;

// Force software rendering, for systems which lack effective hardware
// acceleration

int force_software_renderer = false;

// The screen buffer; this is modified to draw things to the screen

pixel_t *I_VideoBuffer = NULL;

// If true, game is running as a screensaver

boolean screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible = true;

// If true, we display dots at the bottom of the screen to 
// indicate FPS.

static boolean display_fps_dots;

// Does the window currently have focus?

//static boolean window_focused = true;

// Gamma correction level to use

int usegamma = 0;

// Joystick/gamepad hysteresis
unsigned int joywait = 0;

// Set the variable controlling FPS dots.

void I_DisplayFPSDots(boolean dots_on)
{
    display_fps_dots = dots_on;
}

void I_ShutdownGraphics(void)
{
    if (initialized)
    {
        initialized = false;
    }
}



//
// I_StartFrame
//
void I_StartFrame (void)
{
  DG_StartFrame();
}

static unsigned char TranslateKey(unsigned char key)
{
  return key;
}

static unsigned char GetTypedChar(unsigned char key)
{
  return key;
}

void I_GetEvent(void)
{
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
      event.data1 = TranslateKey(key);
      event.data2 = event.data1; // XXX GetLocalizedKey(key)
      event.data3 = GetTypedChar(key);

      if (event.data1 != 0) {
        D_PostEvent(&event);
      }
    } else {
      event.type = ev_keyup;
      event.data1 = TranslateKey(key);
      event.data2 = 0;
      event.data3 = 0;

      if (event.data1 != 0) {
        D_PostEvent(&event);
      }
      break;
    }
  }
}

//
// I_StartTic
//
void I_StartTic (void)
{
    if (!initialized)
    {
        return;
    }

    I_GetEvent();
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

// [AM] Fractional part of the current tic, in the half-open
//      range of [0.0, 1.0).  Used for interpolation.
fixed_t fractionaltic;

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{
  uint16_t *line_out;
  uint8_t *line_in;
  uint32_t i, n;

  if (!initialized) return;

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

//
// I_ReadScreen
//
void I_ReadScreen (pixel_t* scr)
{
    memcpy(scr, I_VideoBuffer, SCREENWIDTH*SCREENHEIGHT*sizeof(*scr));
}


//
// I_SetPalette
//
// [crispy] intermediate gamma levels
byte **gamma2table = NULL;
void I_SetGammaTable (void)
{
	int i;

	gamma2table = malloc(9 * sizeof(*gamma2table));

	// [crispy] 5 original gamma levels
	for (i = 0; i < 5; i++)
	{
		gamma2table[2*i] = (byte *)gammatable[i];
	}

	// [crispy] 4 intermediate gamma levels
	for (i = 0; i < 4; i++)
	{
		int j;

		gamma2table[2*i+1] = malloc(256 * sizeof(**gamma2table));

		for (j = 0; j < 256; j++)
		{
			gamma2table[2*i+1][j] = (gamma2table[2*i][j] + gamma2table[2*i+2][j]) / 2;
		}
	}
}

void I_SetPalette (byte *doompalette)
{
    uint32_t r, g, b;
    int i;

    for (i=0; i<256; ++i ) {
        r = colors[i].r = gammatable[usegamma][*doompalette++];
        g = colors[i].g = gammatable[usegamma][*doompalette++];
        b = colors[i].b = gammatable[usegamma][*doompalette++];
        colors[i].c16 = DG_color16(r, g, b);
    }
}

// Given an RGB value, find the closest matching palette index.

int I_GetPaletteIndex(int r, int g, int b)
{
    int best, best_diff, diff;
    int i;

    best = 0; best_diff = INT_MAX;

    for (i = 0; i < 256; ++i)
    {
        diff = (r - colors[i].r) * (r - colors[i].r)
             + (g - colors[i].g) * (g - colors[i].g)
             + (b - colors[i].b) * (b - colors[i].b);

        if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }

        if (diff == 0)
        {
            break;
        }
    }

    return best;
}

// 
// Set the window title
//

void I_SetWindowTitle(const char *title)
{
    window_title = title;
}

//
// Call the SDL function to set the window title, based on 
// the title set with I_SetWindowTitle.
//

void I_InitWindowTitle(void)
{
}

void I_RegisterWindowIcon(const unsigned int *icon, int width, int height)
{
}

// Set the application icon

void I_InitWindowIcon(void)
{
}

void I_GraphicsCheckCommandLine(void)
{
}

// Check if we have been invoked as a screensaver by xscreensaver.

void I_CheckIsScreensaver(void)
{
    char *env;

    env = getenv("XSCREENSAVER_WINDOW");

    if (env != NULL)
    {
        screensaver_mode = true;
    }
}

void I_GetWindowPosition(int *x, int *y, int w, int h)
{
}

// [crispy] re-calculate SCREENWIDTH, SCREENHEIGHT, NONWIDEWIDTH and WIDESCREENDELTA
void I_GetScreenDimensions (void)
{
	int w = 16, h = 10;
	int ah;

	SCREENWIDTH = ORIGWIDTH << crispy->hires;
	SCREENHEIGHT = ORIGHEIGHT << crispy->hires;

	NONWIDEWIDTH = SCREENWIDTH;

	ah = (aspect_ratio_correct == 1) ? (6 * SCREENHEIGHT / 5) : SCREENHEIGHT;

	// [crispy] widescreen rendering makes no sense without aspect ratio correction
	if (crispy->widescreen && aspect_ratio_correct == 1)
	{
		switch(crispy->widescreen)
		{
			case RATIO_16_10:
				w = 16;
				h = 10;
				break;
			case RATIO_16_9:
				w = 16;
				h = 9;
				break;
			case RATIO_21_9:
				w = 21;
				h = 9;
				break;
			default:
				break;
		}

		SCREENWIDTH = w * ah / h;
		// [crispy] make sure SCREENWIDTH is an integer multiple of 4 ...
		SCREENWIDTH = (SCREENWIDTH + (crispy->hires ? 0 : 3)) & (int)~3;
		// [crispy] ... but never exceeds MAXWIDTH (array size!)
		SCREENWIDTH = MIN(SCREENWIDTH, MAXWIDTH);
	}

	WIDESCREENDELTA = ((SCREENWIDTH - NONWIDEWIDTH) >> crispy->hires) / 2;
}

// [crispy] calls native SDL vsync toggle
void I_ToggleVsync (void)
{
}

void I_InitGraphics(void)
{
    byte *doompal;

    I_GetScreenDimensions();
    V_Init();
    doompal = W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE);
    I_SetPalette(doompal);

    I_VideoBuffer = malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(*I_VideoBuffer));
    memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT * sizeof(*I_VideoBuffer));

    initialized = true;
    I_AtExit(I_ShutdownGraphics, true);
}

// [crispy] re-initialize only the parts of the rendering stack that are really necessary

void I_ReInitGraphics (int reinit)
{
}

// [crispy] take screenshot of the rendered image

void I_RenderReadPixels(byte **data, int *w, int *h, int *p)
{
}

// Bind all variables controlling video options into the configuration
// file system.
void I_BindVideoVariables(void)
{
    M_BindIntVariable("usegamma",                  &usegamma);
}

#ifdef CRISPY_TRUECOLOR
const pixel_t I_BlendAdd (const pixel_t bg, const pixel_t fg)
{
	uint32_t r, g, b;

	if ((r = (fg & rmask) + (bg & rmask)) > rmask) r = rmask;
	if ((g = (fg & gmask) + (bg & gmask)) > gmask) g = gmask;
	if ((b = (fg & bmask) + (bg & bmask)) > bmask) b = bmask;

	return amask | r | g | b;
}

// [crispy] http://stereopsis.com/doubleblend.html
const pixel_t I_BlendDark (const pixel_t bg, const int d)
{
	const uint32_t ag = (bg & 0xff00ff00) >> 8;
	const uint32_t rb =  bg & 0x00ff00ff;

	uint32_t sag = d * ag;
	uint32_t srb = d * rb;

	sag = sag & 0xff00ff00;
	srb = (srb >> 8) & 0x00ff00ff;

	return amask | sag | srb;
}

const pixel_t I_BlendOver (const pixel_t bg, const pixel_t fg)
{
	const uint32_t r = ((blend_alpha * (fg & rmask) + (0xff - blend_alpha) * (bg & rmask)) >> 8) & rmask;
	const uint32_t g = ((blend_alpha * (fg & gmask) + (0xff - blend_alpha) * (bg & gmask)) >> 8) & gmask;
	const uint32_t b = ((blend_alpha * (fg & bmask) + (0xff - blend_alpha) * (bg & bmask)) >> 8) & bmask;

	return amask | r | g | b;
}

const pixel_t (*blendfunc) (const pixel_t fg, const pixel_t bg) = I_BlendOver;

const pixel_t I_MapRGB (const uint8_t r, const uint8_t g, const uint8_t b)
{
/*
	return amask |
	        (((r * rmask) >> 8) & rmask) |
	        (((g * gmask) >> 8) & gmask) |
	        (((b * bmask) >> 8) & bmask);
*/
	return SDL_MapRGB(argbbuffer->format, r, g, b);
}
#endif
