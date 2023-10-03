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
 *      Zone Memory Allocation. Neat.
 *
 * Neat enough to be rewritten by Lee Killough...
 *
 * Must not have been real neat :)
 *
 * Made faster and more general, and added wrappers for all of Doom's
 * memory allocation functions, including malloc() and similar functions.
 * Added line and file numbers, in case of error. Added performance
 * statistics and tunables.
 *-----------------------------------------------------------------------------
 */


// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "z_zone.h"
#include "doomstat.h"
#include "v_video.h"
#include "g_game.h"
#include "lprintf.h"

#ifdef DJGPP
#include <dpmi.h>
#endif

#define ZONE_SIGNATURE 0x931d4a11

enum {
  ZONE_STATIC,
  ZONE_LEVEL,
  ZONE_MAX
};

typedef struct memblock {
  unsigned signature;
  struct memblock *next,*prev;
  size_t size;
  unsigned char tag;
} memblock_t;

static const size_t HEADER_SIZE = sizeof(memblock_t);

static memblock_t *blockbytag[ZONE_MAX];

/* Z_Malloc
 * cph - the algorithm here was a very simple first-fit round-robin
 *  one - just keep looping around, freeing everything we can until
 *  we get a large enough space
 *
 * This has been changed now; we still do the round-robin first-fit,
 * but we only free the blocks we actually end up using; we don't
 * free all the stuff we just pass on the way.
 */

static void *Z_MallocTag(size_t size, int tag)
{
  memblock_t *block = NULL;

  if (!size)
    return NULL; // malloc(0) returns NULL

  if (!(block = malloc(size + HEADER_SIZE)))
  {
    I_Error ("Z_Malloc: Failure trying to allocate %lu bytes", (unsigned long) size);
  }

  if (!blockbytag[tag])
  {
    blockbytag[tag] = block;
    block->next = block->prev = block;
  }
  else
  {
    blockbytag[tag]->prev->next = block;
    block->prev = blockbytag[tag]->prev;
    block->next = blockbytag[tag];
    blockbytag[tag]->prev = block;
  }

  block->size = size;
  block->signature = ZONE_SIGNATURE;
  block->tag = tag;           // tag
  block = (memblock_t *)((char *) block + HEADER_SIZE);

  return block;
}

void Z_Free(void *p)
{
  memblock_t *block = (memblock_t *)((char *) p - HEADER_SIZE);

  if (!p)
    return;

  if (block->signature != ZONE_SIGNATURE)
    I_Error("Z_Free: freed a non-zone pointer");
  block->signature = 0;       // Nullify signature so another free fails

  if (block == block->next)
    blockbytag[block->tag] = NULL;
  else
    if (blockbytag[block->tag] == block)
      blockbytag[block->tag] = block->next;
  block->prev->next = block->next;
  block->next->prev = block->prev;

  free(block);
}

static void Z_FreeTag(int tag)
{
  memblock_t *block, *end_block;

  if (tag < 0 || tag >= ZONE_MAX)
    I_Error("Z_FreeTag: Tag %i does not exist", tag);

  block = blockbytag[tag];
  if (!block)
    return;
  end_block = block->prev;
  while (1)
  {
    memblock_t *next = block->next;
    Z_Free((char *) block + HEADER_SIZE);
    if (block == end_block)
      break;
    block = next;               // Advance to next block
  }
}

static void *Z_ReallocTag(void *ptr, size_t n, int tag)
{
  void *p = Z_MallocTag(n, tag);
  if (ptr)
    {
      memblock_t *block = (memblock_t *)((char *) ptr - HEADER_SIZE);
      memcpy(p, ptr, n <= block->size ? n : block->size);
      Z_Free(ptr);
    }
  return p;
}

static void *Z_CallocTag(size_t n1, size_t n2, int tag)
{
  return
    (n1*=n2) ? memset(Z_MallocTag(n1, tag), 0, n1) : NULL;
}

static char *Z_StrdupTag(const char *s, int tag)
{
  return strcpy(Z_MallocTag(strlen(s)+1, tag), s);
}

void *Z_Malloc(size_t size)
{
  return Z_MallocTag(size, ZONE_STATIC);
}

void *Z_Calloc(size_t n, size_t n2)
{
  return Z_CallocTag(n, n2, ZONE_STATIC);
}

void *Z_Realloc(void *p, size_t n)
{
  return Z_ReallocTag(p, n, ZONE_STATIC);
}

char *Z_Strdup(const char *s)
{
  return Z_StrdupTag(s, ZONE_STATIC);
}

void Z_FreeLevel(void)
{
  return Z_FreeTag(ZONE_LEVEL);
}

void *Z_MallocLevel(size_t size)
{
  return Z_MallocTag(size, ZONE_LEVEL);
}

void *Z_CallocLevel(size_t n, size_t n2)
{
  return Z_CallocTag(n, n2, ZONE_LEVEL);
}

void *Z_ReallocLevel(void *p, size_t n)
{
  return Z_ReallocTag(p, n, ZONE_LEVEL);
}

char *Z_StrdupLevel(const char *s)
{
  return Z_StrdupTag(s, ZONE_LEVEL);
}
