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
#include <stdint.h>
#include <string.h>

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
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "host.h"
#include "debug.h"

// These are (1) the 320x200x8 paletted buffer that we draw to (i.e. the one
// that holds I_VideoBuffer), (2) the 320x200x32 RGBA intermediate buffer that
// we blit the former buffer to, (3) the intermediate 320x200 texture that we
// load the RGBA buffer to and that we render into another texture (4) which
// is upscaled by an integer factor UPSCALE using "nearest" scaling and which
// in turn is finally rendered to screen using "linear" scaling.

struct xcolor {
  uint32_t b:8;
  uint32_t g:8;
  uint32_t r:8;
  uint32_t a:8;
};

struct color {
  uint8_t b;
  uint8_t g;
  uint8_t r;
  uint16_t c16;
};

static struct color colors[256];

// display has been set up?

static int initialized = 0;

// disable mouse?

int usemouse = 0;

// Save screenshots in PNG format.

int png_screenshots = 0;

// SDL video driver name

char *video_driver = "";

// Window position:

char *window_position = "center";

// SDL display number on which to run.

int video_display = 0;

// Screen width and height, from configuration file.

int window_width = 800;
int window_height = 600;

// Fullscreen mode, 0x0 for SDL_WINDOW_FULLSCREEN_DESKTOP.

int fullscreen_width = 0, fullscreen_height = 0;

// Maximum number of pixels to use for intermediate scale buffer.

static int max_scaling_buffer_pixels = 16000000;

// Run in full screen mode?  (int type for config code)

int fullscreen = 1;

// Aspect ratio correction mode

int aspect_ratio_correct = 1;

// Force integer scales for resolution-independent rendering

int integer_scaling = 0;

// VGA Porch palette change emulation

int vga_porch_flash = 0;

// Force software rendering, for systems which lack effective hardware
// acceleration

int force_software_renderer = 0;

// Time to wait for the screen to settle on startup before starting the
// game (ms)

static int startup_delay = 1000;

// Grab the mouse? (int type for config code). nograbmouse_override allows
// this to be temporarily disabled via the command line.

static int grabmouse = 1;
static int nograbmouse_override = 0;

// The screen buffer; this is modified to draw things to the screen

pixel_t *I_VideoBuffer = NULL;

// If true, game is running as a screensaver

int screensaver_mode = 0;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

int screenvisible = 1;

// If true, we display dots at the bottom of the screen to 
// indicate FPS.

static int display_fps_dots;

// If this is true, the screen is rendered but not blitted to the
// video buffer.

static int noblit;

// Callback function to invoke to determine whether to grab the 
// mouse pointer.

static grabmouse_callback_t grabmouse_callback = NULL;

// Gamma correction level to use

int usegamma = 0;

// Joystick/gamepad hysteresis
unsigned int joywait = 0;

void I_SetGrabMouseCallback(grabmouse_callback_t func)
{
    grabmouse_callback = func;
}

// Set the variable controlling FPS dots.

void I_DisplayFPSDots(int dots_on)
{
    display_fps_dots = dots_on;
}

void I_ShutdownGraphics(void)
{
    if (initialized)
    {
        initialized = 0;
    }
    if (I_VideoBuffer) free(I_VideoBuffer);
}

//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

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
  DG_SetWindowTitle(title);
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
    noblit = 0;
    nograbmouse_override = 0;
    fullscreen = 1;
}

// Check if we have been invoked as a screensaver by xscreensaver.

void I_CheckIsScreensaver(void)
{
}

void I_GetWindowPosition(int *x, int *y, int w, int h)
{
}

void I_InitGraphics(void)
{
  I_VideoBuffer = malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(*I_VideoBuffer));
  memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT * sizeof(*I_VideoBuffer));
  initialized = true;
}

// Bind all variables controlling video options into the configuration
// file system.
void I_BindVideoVariables(void)
{
    //M_BindIntVariable("use_mouse",                 &usemouse);
    M_BindIntVariable("fullscreen",                &fullscreen);
    M_BindIntVariable("video_display",             &video_display);
    M_BindIntVariable("aspect_ratio_correct",      &aspect_ratio_correct);
    M_BindIntVariable("integer_scaling",           &integer_scaling);
    M_BindIntVariable("vga_porch_flash",           &vga_porch_flash);
    M_BindIntVariable("startup_delay",             &startup_delay);
    M_BindIntVariable("fullscreen_width",          &fullscreen_width);
    M_BindIntVariable("fullscreen_height",         &fullscreen_height);
    M_BindIntVariable("force_software_renderer",   &force_software_renderer);
    M_BindIntVariable("max_scaling_buffer_pixels", &max_scaling_buffer_pixels);
    M_BindIntVariable("window_width",              &window_width);
    M_BindIntVariable("window_height",             &window_height);
    M_BindIntVariable("grabmouse",                 &grabmouse);
    M_BindStringVariable("video_driver",           &video_driver);
    M_BindStringVariable("window_position",        &window_position);
    M_BindIntVariable("usegamma",                  &usegamma);
    M_BindIntVariable("png_screenshots",           &png_screenshots);
}
