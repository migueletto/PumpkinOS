/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 2001 by
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
 *      Transparent access to data in WADs using mmap
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#include "doomstat.h"
#include "doomtype.h"

#include "w_wad.h"
#include "z_zone.h"
#include "lprintf.h"
#include "i_system.h"
#include "m_file.h"

#include "e6y.h"//e6y

static void **lump_data;

#ifdef _WIN32
typedef struct {
  HANDLE hnd;
  OFSTRUCT fileinfo;
  HANDLE hnd_map;
  void   *data;
} mmap_info_t;

mmap_info_t *mapped_wad;

void W_DoneCache(void)
{
  size_t i;

  if (lump_data) {
    Z_Free(lump_data);
    lump_data = NULL;
  }

  if (!mapped_wad)
    return;
  for (i=0; i<numwadfiles; i++)
  {
    if (mapped_wad[i].data)
    {
      UnmapViewOfFile(mapped_wad[i].data);
      mapped_wad[i].data=NULL;
    }
    if (mapped_wad[i].hnd_map)
    {
      CloseHandle(mapped_wad[i].hnd_map);
      mapped_wad[i].hnd_map=NULL;
    }
    if (mapped_wad[i].hnd)
    {
      CloseHandle(mapped_wad[i].hnd);
      mapped_wad[i].hnd=NULL;
    }
  }
  Z_Free(mapped_wad);
  mapped_wad = NULL;
}

void W_InitCache(void)
{
  // Wipe any existing cache
  W_DoneCache();

  // set up caching
  lump_data = Z_Calloc(numlumps, sizeof *lump_data);
  if (!lump_data)
    I_Error ("W_Init: Couldn't allocate lump data");

  mapped_wad = Z_Calloc(numwadfiles,sizeof(mmap_info_t));
  memset(mapped_wad,0,sizeof(mmap_info_t)*numwadfiles);
  {
    int i;
    for (i=0; i<numlumps; i++)
    {
      int wad_index = (int)(lumpinfo[i].wadfile-wadfiles);

      if (!lumpinfo[i].wadfile)
        continue;
#ifdef RANGECHECK
      if ((wad_index<0)||((size_t)wad_index>=numwadfiles))
        I_Error("W_InitCache: wad_index out of range");
#endif
      if (!mapped_wad[wad_index].data)
      {
        wchar_t *wname = ConvertUtf8ToWide(wadfiles[wad_index].name);
        mapped_wad[wad_index].hnd = CreateFileW(wname,
          GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
          NULL, OPEN_EXISTING, 0, NULL);
        Z_Free(wname);
        if (mapped_wad[wad_index].hnd==INVALID_HANDLE_VALUE)
          I_Error("W_InitCache: CreateFile for memory mapping failed (LastError %li)",GetLastError());
        mapped_wad[wad_index].hnd_map =
          CreateFileMapping(
            mapped_wad[wad_index].hnd,
            NULL,
            PAGE_READONLY,
            0,
            0,
            NULL
          );
        if (mapped_wad[wad_index].hnd_map==NULL)
          I_Error("W_InitCache: CreateFileMapping for memory mapping failed (LastError %li)",GetLastError());
        mapped_wad[wad_index].data =
          MapViewOfFile(
            mapped_wad[wad_index].hnd_map,
            FILE_MAP_READ,
            0,
            0,
            0
          );
        if (mapped_wad[wad_index].data==NULL)
          I_Error("W_InitCache: MapViewOfFile for memory mapping failed (LastError %li)",GetLastError());
      }
    }
  }
}

const void* W_LumpByNum(int lump)
{
  int wad_index = (int)(lumpinfo[lump].wadfile-wadfiles);
#ifdef RANGECHECK
  if ((wad_index<0)||((size_t)wad_index>=numwadfiles))
    I_Error("W_LumpByNum: wad_index out of range");
  if ((unsigned)lump >= (unsigned)numlumps)
    I_Error ("W_LumpByNum: %i >= numlumps",lump);
#endif
  if (!lumpinfo[lump].wadfile)
    return NULL;
  return (void*)((unsigned char *)mapped_wad[wad_index].data+lumpinfo[lump].position);
}

#else

void ** mapped_wad;

void W_InitCache(void)
{
  //int maxfd = 0;
  // set up caching
  lump_data = Z_Calloc(numlumps, sizeof *lump_data);
  if (!lump_data)
    I_Error ("W_Init: Couldn't allocate lump data");

#if 0
  {
    int i;
    for (i=0; i<numlumps; i++)
      if (lumpinfo[i].wadfile)
        if (lumpinfo[i].wadfile->handle > maxfd) maxfd = lumpinfo[i].wadfile->handle;
  }
  mapped_wad = Z_Calloc(maxfd+1,sizeof *mapped_wad);
#endif
  mapped_wad = Z_Calloc(numlumps+1,sizeof *mapped_wad);
  {
    int i;
    for (i=0; i<numlumps; i++) {
      if (lumpinfo[i].wadfile) {
        int fd = lumpinfo[i].wadfile->handle;
        if (!mapped_wad[fd])
          if ((mapped_wad[fd] = mmap(NULL,I_Filelength(fd),PROT_READ,MAP_SHARED,fd,0)) == MAP_FAILED)
            I_Error("W_InitCache: failed to mmap");
      }
    }
  }
}

void W_DoneCache(void)
{
  {
    int i;
    for (i=0; i<numlumps; i++)
      if (lumpinfo[i].wadfile) {
        int fd = lumpinfo[i].wadfile->handle;
        if (fd > 0 && mapped_wad[fd]) {
          if (munmap(mapped_wad[fd],I_Filelength(fd)))
            I_Error("W_DoneCache: failed to munmap");
          mapped_wad[fd] = NULL;
        }
      }
  }
  Z_Free(mapped_wad);
  mapped_wad = NULL;
}

const void* W_LumpByNum(int lump)
{
#ifdef RANGECHECK
  if ((unsigned)lump >= (unsigned)numlumps)
    I_Error ("W_LumpByNum: %i >= numlumps",lump);
#endif
  if (!lumpinfo[lump].wadfile)
    return NULL;

  return
    (const void *) (
      ((const byte *) (mapped_wad[lumpinfo[lump].wadfile->handle]))
      + lumpinfo[lump].position
    );
}
#endif

/*
 * W_LockLumpNum
 *
 * This copies the lump into a malloced memory region and returns its address
 * instead of returning a pointer into the memory mapped area
 *
 */
const void* W_LockLumpNum(int lump)
{
  size_t len = W_LumpLength(lump);
  const void *data = W_LumpByNum(lump);

  // read the lump in
  if (!lump_data[lump]) {
    lump_data[lump] = Z_Malloc(len);
    memcpy(lump_data[lump], data, len);
  }

  return lump_data[lump];
}
