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
//      Timer functions.
//

#include <stdint.h>

#include "i_timer.h"
#include "m_fixed.h" // [crispy]
#include "doomtype.h"

#include "sys.h"
#include "host.h"

//
// I_GetTime
// returns time in 1/35th second tics
//

static uint32_t basetime = 0;
static uint64_t basecounter = 0; // [crispy]

int  I_GetTime (void)
{
    uint32_t ticks;

    ticks = DG_GetTicksMs();

    if (basetime == 0)
        basetime = ticks;

    ticks -= basetime;

    return (ticks * TICRATE) / 1000;    
}

//
// Same as I_GetTime, but returns time in milliseconds
//

int I_GetTimeMS(void)
{
    uint32_t ticks;

    ticks = DG_GetTicksMs();

    if (basetime == 0)
        basetime = ticks;

    return ticks - basetime;
}

// [crispy] Get time in microseconds

uint64_t I_GetTimeUS(void)
{
    uint64_t counter;

    counter = sys_get_clock();

    if (basecounter == 0)
        basecounter = counter;

    return counter - basecounter;
}

// Sleep for a specified number of ms

void I_Sleep(int ms)
{

    if (DG_SleepMs(ms) == -1) {
      M_Quit();
    }
}

void I_WaitVBL(int count)
{
    I_Sleep((count * 1000) / 70);
}


void I_InitTimer(void)
{
}

// [crispy]

fixed_t I_GetFracRealTime(void)
{
    return (int64_t)I_GetTimeMS() * TICRATE % 1000 * FRACUNIT / 1000;
}
