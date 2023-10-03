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

#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

#include "m_fixed.h"
#include "host.h"

#ifdef _MSC_VER
#define    F_OK    0    /* Check for file existence */
#define    W_OK    2    /* Check for write permission */
#define    R_OK    4    /* Check for read permission */
#endif

extern int interpolation_method;
extern int ms_to_next_tick;
dboolean I_StartDisplay(void);
void I_EndDisplay(void);
fixed_t I_GetTimeFrac (void);

unsigned long I_GetRandomTimeSeed(void); /* cphipps */

void I_uSleep(unsigned long usecs);

/* cphipps - I_GetVersionString
 * Returns a version string in the given buffer
 */
const char* I_GetVersionString(char* buf, size_t sz);

/* cphipps - I_SigString
 * Returns a string describing a signal number
 */
const char* I_SigString(char* buf, size_t sz, int signum);

#ifdef _WIN32
void I_SwitchToWindow(HWND hwnd);
#endif

// e6y
const char* I_GetTempDir(void);

const char *I_DoomExeDir(void); // killough 2/16/98: path to executable's dir

dboolean HasTrailingSlash(const char* dn);
char* I_RequireFile(const char* wfname, const char* ext);
char* I_FindFile(const char* wfname, const char* ext);
const char* I_FindFile2(const char* wfname, const char* ext);
char* I_RequireAnyFile(const char* wfname, const char** ext);

char* I_RequireWad(const char* wfname);
char* I_FindWad(const char* wfname);

char* I_RequireDeh(const char* wfname);
char* I_FindDeh(const char* wfname);

char* I_RequireZip(const char* wfname);
char* I_FindZip(const char* wfname);

/* cph 2001/11/18 - wrapper for read(2) which deals with partial reads */
void I_Read(dg_file_t *fd, void* buf, size_t sz);

/* cph 2001/11/18 - Move W_Filelength to i_system.c */
int I_Filelength(dg_file_t *handle);

// Schedule a function to be called when the program exits.
// If run_if_error is true, the function is called if the exit
// is due to an error (I_Error)

typedef enum
{
  exit_priority_first,
  exit_priority_normal,
  exit_priority_last,
  exit_priority_max,
} exit_priority_t;

typedef void (*atexit_func_t)(void);
void I_AtExit(atexit_func_t func, dboolean run_if_error,
              const char* name, exit_priority_t priority);

#endif
