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
 *    Declarations etc. for logical console output
 *
 *-----------------------------------------------------------------------------*/

#ifndef __LPRINTF__
#define __LPRINTF__

#include <stdarg.h>
#include <stddef.h>
#include "doomtype.h"

typedef enum
{
  LO_INFO=1,
  LO_WARN=2,
  LO_ERROR=4,
  LO_DEBUG=8,
} OutputLevels;

#if !defined(__GNUC__) && !defined(__clang__)
#define __attribute__(x)
#endif

extern int lprintf(OutputLevels pri, const char *fmt, ...) __attribute__((format(printf,2,3)));

void I_EnableVerboseLogging(void);
void I_DisableAllLogging(void);
void I_DisableMessageBoxes(void);

/* killough 3/20/98: add const
 * killough 4/25/98: add gcc attributes
 * cphipps 01/11- moved from i_system.h */
void I_Error(const char *error, ...) __attribute__((format(printf,1,2)));
void I_Warn(const char *error, ...) __attribute__((format(printf,1,2)));

#endif
