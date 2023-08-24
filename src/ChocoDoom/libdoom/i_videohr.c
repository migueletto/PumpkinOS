//
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
//     SDL emulation of VGA 640x480x4 planar video mode,
//     for Hexen startup loading screen.
//

#include "string.h"

#include "doomtype.h"
#include "i_timer.h"
#include "i_video.h"

// Palette fade-in takes two seconds

#define FADE_TIME 2000

#define HR_SCREENWIDTH 640
#define HR_SCREENHEIGHT 480

static const char *window_title = "";

boolean I_SetVideoModeHR(void)
{
    return true;
}

void I_SetWindowTitleHR(const char *title)
{
    window_title = title;
}

void I_UnsetVideoModeHR(void)
{
}

void I_ClearScreenHR(void)
{
}

void I_SlamBlockHR(int x, int y, int w, int h, const byte *src)
{
}

void I_SlamHR(const byte *buffer)
{
    I_SlamBlockHR(0, 0, HR_SCREENWIDTH, HR_SCREENHEIGHT, buffer);
}

void I_InitPaletteHR(void)
{
    // ...
}

void I_SetPaletteHR(const byte *palette)
{
}

void I_FadeToPaletteHR(const byte *palette)
{
}

void I_BlackPaletteHR(void)
{
    byte blackpal[16 * 3];

    memset(blackpal, 0, sizeof(blackpal));

    I_SetPaletteHR(blackpal);
}

// Check if the user has hit the escape key to abort startup.
boolean I_CheckAbortHR(void)
{
    return 0;
}

