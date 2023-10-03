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
 *  External simple file handling.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __M_FILE__
#define __M_FILE__

#include <stdio.h>

#include "doomtype.h"
#include "host.h"

dboolean M_ReadWriteAccess(const char *name);
dboolean M_ReadAccess(const char *name);
dboolean M_WriteAccess(const char *name);
int M_MakeDir(const char *path, int require);
dboolean M_IsDir(const char *name);
dg_file_t *M_OpenFile(const char *name, const char *mode);
dg_file_t *M_OpenRB(const char *name);
dboolean M_FileExists(const char *name);
dboolean M_WriteFile (char const* name, const void* source, size_t length);
int M_ReadFile (char const* name,byte** buffer);
int M_ReadFileToString(char const *name, char **buffer);
dboolean M_RemoveFilesAtPath(const char *path);

int M_remove(const char *path);
char *M_getcwd(char *buffer, int len);
char *M_getenv(const char *name);

#ifdef _WIN32
wchar_t *ConvertUtf8ToWide(const char *str);
char *ConvertWideToUtf8(const wchar_t *wstr);
#endif
char *ConvertSysNativeMBToUtf8(const char *str);
char *ConvertUtf8ToSysNativeMB(const char *str);

#endif
