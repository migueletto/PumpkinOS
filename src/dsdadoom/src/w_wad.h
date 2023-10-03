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
 *      WAD I/O functions.
 *
 *-----------------------------------------------------------------------------*/


#ifndef __W_WAD__
#define __W_WAD__

#include <stddef.h>

#include "host.h"

//
// TYPES
//

typedef struct
{
  char identification[4];                  // Should be "IWAD" or "PWAD".
  int  numlumps;
  int  infotableofs;
} wadinfo_t;

typedef struct
{
  int  filepos;
  int  size;
  char name[8];
} filelump_t;

#define LUMP_NOT_FOUND -1

//
// WADFILE I/O related stuff.
//

// CPhipps - defined enum in wider scope
// Ty 08/29/98 - add source field to identify where this lump came from
typedef enum {
  source_skip = -1,
  source_iwad=0,    // iwad file load
  source_pre,       // predefined lump
  source_auto_load, // lump auto-loaded by config file
  source_pwad,      // pwad file load
  source_lmp,       // lmp file load
  source_net        // CPhipps

  //e6y
//  ,source_deh_auto_load
  ,source_deh
  ,source_err

} wad_source_t;

// CPhipps - changed wad init
// We _must_ have the wadfiles[] the same as those actually loaded, so there
// is no point having these separate entities. This belongs here.
typedef struct {
  char* name;
  wad_source_t src;
  dg_file_t *handle;
} wadfile_info_t;

extern wadfile_info_t *wadfiles;

extern size_t numwadfiles; // CPhipps - size of the wadfiles array

void W_Init(void); // CPhipps - uses the above array
void W_InitCache(void);
void W_DoneCache(void);
void W_Shutdown(void);

typedef enum
{
  ns_global=0,
  ns_sprites,
  ns_flats,
  ns_colormaps,
  ns_prboom,
  ns_demos,
  ns_hires,
} li_namespace_e; // haleyjd 05/21/02: renamed from "namespace"

typedef struct
{
  // WARNING: order of some fields important (see info.c).

  char  name[9];
  int   size;

  // killough 1/31/98: hash table fields, used for ultra-fast hash table lookup
  int index, next;

  // killough 4/17/98: namespace tags, to prevent conflicts between resources
  li_namespace_e li_namespace; // haleyjd 05/21/02: renamed from "namespace"

  wadfile_info_t *wadfile;
  int position;
  wad_source_t source;
  int flags; //e6y
} lumpinfo_t;

// e6y: lump flags
#define LUMP_STATIC 0x00000001 /* assigned gltexture should be static */
#define LUMP_PRBOOM 0x00000002 /* from internal resource */

extern lumpinfo_t *lumpinfo;
extern int        numlumps;

int     W_FindNumFromName2(const char *name, int ns, int lump);

static inline
int     W_FindNumFromName(const char *name, int lump)
        { return W_FindNumFromName2(name, ns_global, lump); }

static inline
int     W_CheckNumForName2(const char *name, int ns)
        { return W_FindNumFromName2(name, ns, LUMP_NOT_FOUND); }

static inline
int     W_CheckNumForName(const char *name)
        { return W_CheckNumForName2(name, ns_global); }

int     W_CheckNumForNameInternal(const char *name);
int     W_ListNumFromName(const char *name, int lump);
int     W_GetNumForName (const char* name);
const lumpinfo_t* W_GetLumpInfoByNum(int lump);
int     W_LumpLength (int lump);
int     W_SafeLumpLength (int lump);
const char *W_LumpName(int lump);
void    W_ReadLump (int lump, void *dest);
char*   W_ReadLumpToString (int lump);
// CPhipps - modified for 'new' lump locking
const void* W_SafeLumpByNum (int lump);
const void* W_LumpByNum (int lump);
const void* W_LockLumpNum(int lump);

int W_LumpNumExists(int lump);
int W_LumpNameExists(const char *name);
int W_LumpNameExists2(const char *name, int ns);

// CPhipps - convenience macros
//#define W_LumpByNum(num) (W_LumpByNum)((num),1)
#define W_LumpByName(name) W_LumpByNum (W_GetNumForName(name))

char *AddDefaultExtension(char *, const char *);  // killough 1/18/98
void ExtractFileBase(const char *, char *);       // killough
unsigned W_LumpNameHash(const char *s);           // killough 1/31/98
void W_HashLumps(void);                           // cph 2001/07/07 - made public
int W_LumpNumInPortWad(int lump);

#endif
