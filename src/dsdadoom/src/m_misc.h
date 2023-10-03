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
 *  External non-system-specific stuff, like storing config settings,
 *  simple file handling, and saving screnshots.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __M_MISC__
#define __M_MISC__

#include "doomtype.h"

#include "dsda/configuration.h"
#include "dsda/input.h"

void M_ScreenShot (void);
void M_DoScreenShot (const char*); // cph

void M_LoadDefaults (void);
void M_SaveDefaults (void);

dboolean M_StringCopy(char *dest, const char *src, size_t dest_size);
dboolean M_StringConcat(char *dest, const char *src, size_t dest_size);

int M_StrToInt(const char *s, int *l);
int M_StrToFloat(const char *s, float *f);

int M_DoubleToInt(double x);

char* M_Strlwr(char* str);
char* M_Strupr(char* str);
char* M_StrRTrim(char* str);

typedef struct array_s
{
  void *data;
  int capacity;
  int count;
} array_t;
void M_ArrayClear(array_t *data);
void* M_ArrayGetNewItem(array_t *data, int itemsize);

#endif
