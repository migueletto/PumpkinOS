/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2001 by
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
 *      Handles in-memory caching of WAD lumps
 *
 *-----------------------------------------------------------------------------
 */

// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomstat.h"
#include "doomtype.h"

#include "w_wad.h"
#include "z_zone.h"
#include "lprintf.h"

static void **lump_data;

/* W_InitCache
 *
 * cph 2001/07/07 - split from W_Init
 */
void W_InitCache(void)
{
  // set up caching
  lump_data = calloc(sizeof *lump_data, numlumps);
  if (!lump_data)
    I_Error ("W_Init: Couldn't allocate lump data");
}

void W_DoneCache(void)
{
}

/* W_LumpByNum
 * killough 4/25/98: simplified
 * CPhipps - modified for new lump locking scheme
 *           returns a const*
 */

const void *W_LumpByNum(int lump)
{
#ifdef RANGECHECK
  if ((unsigned)lump >= (unsigned)numlumps)
    I_Error ("W_LumpByNum: %i >= numlumps",lump);
#endif

  // read the lump in
  if (!lump_data[lump]) {
    lump_data[lump] = Z_Malloc(W_LumpLength(lump));
    W_ReadLump(lump, lump_data[lump]);
  }

  return lump_data[lump];
}

const void *W_LockLumpNum(int lump)
{
  return W_LumpByNum(lump);
}
