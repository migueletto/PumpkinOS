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
 *   -Loads and initializes texture and flat animation sequences
 *   -Implements utility functions for all linedef/sector special handlers
 *   -Dispatches walkover and gun line triggers
 *   -Initializes and implements special sector types
 *   -Implements donut linedef triggers
 *   -Initializes and implements BOOM linedef triggers for
 *     Scrollers/Conveyors
 *     Friction
 *     Wind/Current
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_setup.h"
#include "m_random.h"
#include "d_englsh.h"
#include "w_wad.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_user.h"
#include "g_game.h"
#include "p_inter.h"
#include "p_enemy.h"
#include "s_sound.h"
#include "sounds.h"
#include "i_sound.h"
#include "m_bbox.h"                                         // phares 3/20/98
#include "d_deh.h"
#include "r_plane.h"
#include "hu_stuff.h"
#include "lprintf.h"
#include "e6y.h"//e6y

#include "dsda.h"
#include "dsda/args.h"
#include "dsda/configuration.h"
#include "dsda/global.h"
#include "dsda/id_list.h"
#include "dsda/line_special.h"
#include "dsda/map_format.h"
#include "dsda/mapinfo.h"
#include "dsda/messenger.h"
#include "dsda/thing_id.h"
#include "dsda/utility.h"

//
//      source animation definition
//
//
#ifdef _MSC_VER // proff: This is the same as __attribute__ ((packed)) in GNUC
#pragma pack(push)
#pragma pack(1)
#endif //_MSC_VER

#if defined(__MWERKS__)
#pragma options align=packed
#endif

typedef struct
{
  signed char istexture; //jff 3/23/98 make char for comparison // cph - make signed
  char        endname[9];           //  if false, it is a flat
  char        startname[9];
  int         speed;
} PACKEDATTR animdef_t; //jff 3/23/98 pack to read from memory

#if defined(__MWERKS__)
#pragma options align=reset
#endif

#ifdef _MSC_VER
#pragma pack(pop)
#endif //_MSC_VER

#define MAXANIMS 32                   // no longer a strict limit -- killough

static anim_t*  lastanim;
static anim_t*  anims;                // new structure w/o limits -- killough
static size_t maxanims;

// killough 3/7/98: Initialize generalized scrolling
static void P_SpawnScrollers(void);

static void P_SpawnFriction(void);    // phares 3/16/98
static void P_SpawnPushers(void);     // phares 3/20/98

static const animdef_t heretic_animdefs[] = {
    // false = flat
    // true = texture
    { false, "FLTWAWA3", "FLTWAWA1", 8 }, // Water
    { false, "FLTSLUD3", "FLTSLUD1", 8 }, // Sludge
    { false, "FLTTELE4", "FLTTELE1", 6 }, // Teleport
    { false, "FLTFLWW3", "FLTFLWW1", 9 }, // River - West
    { false, "FLTLAVA4", "FLTLAVA1", 8 }, // Lava
    { false, "FLATHUH4", "FLATHUH1", 8 }, // Super Lava
    { true,  "LAVAFL3",  "LAVAFL1",  6 },    // Texture: Lavaflow
    { true,  "WATRWAL3", "WATRWAL1", 4 },  // Texture: Waterfall
    { -1 }
};

// heretic
#define MAXLINEANIMS 64*256
short numlinespecials;
line_t *linespeciallist[MAXLINEANIMS];

//e6y
void MarkAnimatedTextures(void)
{
#if 0
  anim_t* anim;

  anim_textures = Z_Calloc(numtextures, sizeof(TAnimItemParam));
  anim_flats = Z_Calloc(numflats, sizeof(TAnimItemParam));

  for (anim = anims ; anim < lastanim ; anim++)
  {
    int i;
    for (i = 0; i < anim->numpics ; i++)
    {
      if (anim->istexture)
      {
        anim_textures[anim->basepic + i].anim = anim;
        anim_textures[anim->basepic + i].index = i + 1;
      }
      else
      {
        anim_flats[anim->basepic + i].anim = anim;
        anim_flats[anim->basepic + i].index = i + 1;
      }
    }
  }
#endif
}

//
// P_InitPicAnims
//
// Load the table of animation definitions, checking for existence of
// the start and end of each frame. If the start doesn't exist the sequence
// is skipped, if the last doesn't exist, BOOM exits.
//
// Wall/Flat animation sequences, defined by name of first and last frame,
// The full animation sequence is given using all lumps between the start
// and end entry, in the order found in the WAD file.
//
// This routine modified to read its data from a predefined lump or
// PWAD lump called ANIMATED rather than a static table in this module to
// allow wad designers to insert or modify animation sequences.
//
// Lump format is an array of byte packed animdef_t structures, terminated
// by a structure with istexture == -1. The lump can be generated from a
// text source file using SWANTBLS.EXE, distributed with the BOOM utils.
// The standard list of switches and animations is contained in the example
// source text file DEFSWANI.DAT also in the BOOM util distribution.
//
//
void P_InitPicAnims (void)
{
  int         i;
  const animdef_t *animdefs; //jff 3/23/98 pointer to animation lump
  int         lump = -1;
  //  Init animation

  if (map_format.animdefs)
  {
    MarkAnimatedTextures();//e6y
    return;
  }

  if (heretic)
  {
    animdefs = heretic_animdefs;
  }
  else
  {
    lump = W_GetNumForName("ANIMATED"); // cph - new wad lump handling
    //jff 3/23/98 read from predefined or wad lump instead of table
    animdefs = (const animdef_t *)W_LumpByNum(lump);
  }

  lastanim = anims;
  for (i=0 ; animdefs[i].istexture != -1 ; i++)
  {
    // 1/11/98 killough -- removed limit by array-doubling
    if (lastanim >= anims + maxanims)
    {
      size_t newmax = maxanims ? maxanims*2 : MAXANIMS;
      anims = Z_Realloc(anims, newmax*sizeof(*anims));   // killough
      lastanim = anims + maxanims;
      maxanims = newmax;
    }

    if (animdefs[i].istexture)
    {
      // different episode ?
      if (R_CheckTextureNumForName(animdefs[i].startname) == -1)
          continue;

      lastanim->picnum = R_TextureNumForName (animdefs[i].endname);
      lastanim->basepic = R_TextureNumForName (animdefs[i].startname);
    }
    else
    {
      if (!W_LumpNameExists2(animdefs[i].startname, ns_flats))  // killough 4/17/98
          continue;

      lastanim->picnum = R_FlatNumForName (animdefs[i].endname);
      lastanim->basepic = R_FlatNumForName (animdefs[i].startname);
    }

    lastanim->istexture = animdefs[i].istexture;
    lastanim->numpics = lastanim->picnum - lastanim->basepic + 1;

    if (lastanim->numpics < 2)
        I_Error ("P_InitPicAnims: bad cycle from %s to %s",
                  animdefs[i].startname,
                  animdefs[i].endname);

    lastanim->speed = LittleLong(animdefs[i].speed); // killough 5/5/98: add LONG()
    lastanim++;
  }

  MarkAnimatedTextures();//e6y
}

///////////////////////////////////////////////////////////////
//
// Linedef and Sector Special Implementation Utility Functions
//
///////////////////////////////////////////////////////////////

//
// getSide()
//
// Will return a side_t*
//  given the number of the current sector,
//  the line number, and the side (0/1) that you want.
//
// Note: if side=1 is specified, it must exist or results undefined
//
side_t* getSide
( int           currentSector,
  int           line,
  int           side )
{
  return &sides[ (sectors[currentSector].lines[line])->sidenum[side] ];
}


//
// getSector()
//
// Will return a sector_t*
//  given the number of the current sector,
//  the line number and the side (0/1) that you want.
//
// Note: if side=1 is specified, it must exist or results undefined
//
sector_t* getSector
( int           currentSector,
  int           line,
  int           side )
{
  return sides[ (sectors[currentSector].lines[line])->sidenum[side] ].sector;
}


//
// twoSided()
//
// Given the sector number and the line number,
//  it will tell you whether the line is two-sided or not.
//
// modified to return actual two-sidedness rather than presence
// of 2S flag unless compatibility optioned
//
int twoSided
( int   sector,
  int   line )
{
  //jff 1/26/98 return what is actually needed, whether the line
  //has two sidedefs, rather than whether the 2S flag is set

  return (comp[comp_model]) ?
    (sectors[sector].lines[line])->flags & ML_TWOSIDED
    :
    (sectors[sector].lines[line])->sidenum[1] != NO_INDEX;
}


//
// getNextSector()
//
// Return sector_t * of sector next to current across line.
//
// Note: returns NULL if not two-sided line, or both sides refer to sector
//
sector_t* getNextSector
( line_t*       line,
  sector_t*     sec )
{
  //jff 1/26/98 check unneeded since line->backsector already
  //returns NULL if the line is not two sided, and does so from
  //the actual two-sidedness of the line, rather than its 2S flag

  if (comp[comp_model])
  {
    if (!(line->flags & ML_TWOSIDED))
      return NULL;
  }

  if (line->frontsector == sec) {
    if (comp[comp_model] || line->backsector!=sec)
      return line->backsector; //jff 5/3/98 don't retn sec unless compatibility
    else                       // fixes an intra-sector line breaking functions
      return NULL;             // like floor->highest floor
  }
  return line->frontsector;
}


//
// P_FindLowestFloorSurrounding()
//
// Returns the fixed point value of the lowest floor height
// in the sector passed or its surrounding sectors.
//
fixed_t P_FindLowestFloorSurrounding(sector_t* sec)
{
  int                 i;
  line_t*             check;
  sector_t*           other;
  fixed_t             floor = sec->floorheight;

  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);

    if (!other)
      continue;

    if (other->floorheight < floor)
      floor = other->floorheight;
  }
  return floor;
}


//
// P_FindHighestFloorSurrounding()
//
// Passed a sector, returns the fixed point value of the largest
// floor height in the surrounding sectors, not including that passed
//
// NOTE: if no surrounding sector exists -32000*FRACUINT is returned
//       if compatibility then -500*FRACUNIT is the smallest return possible
//
fixed_t P_FindHighestFloorSurrounding(sector_t *sec)
{
  int i;
  line_t* check;
  sector_t* other;
  fixed_t floor = -500*FRACUNIT;

  //jff 1/26/98 Fix initial value for floor to not act differently
  //in sections of wad that are below -500 units
  if (!comp[comp_model])       /* jff 3/12/98 avoid ovf */
    floor = -32000*FRACUNIT;   // in height calculations

  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);

    if (!other)
      continue;

    if (other->floorheight > floor)
      floor = other->floorheight;
  }
  return floor;
}


//
// P_FindNextHighestFloor()
//
// Passed a sector and a floor height, returns the fixed point value
// of the smallest floor height in a surrounding sector larger than
// the floor height passed. If no such height exists the floorheight
// passed is returned.
//
// Rewritten by Lee Killough to avoid fixed array and to be faster
//
fixed_t P_FindNextHighestFloor(sector_t *sec, int currentheight)
{
  sector_t *other;
  int i;

  // e6y
  // Original P_FindNextHighestFloor() is restored for demo_compatibility
  // Adapted for prboom's complevels
  if (demo_compatibility && !prboom_comp[PC_FORCE_BOOM_FINDNEXTHIGHESTFLOOR].state)
  {
    int h;
    int min;
    static int MAX_ADJOINING_SECTORS = 0;
    static fixed_t *heightlist = NULL;
    static int heightlist_size = 0;
    line_t* check;
    fixed_t height = currentheight;
    static fixed_t last_height_0 = 0;

    // 20 adjoining sectors max!
    if (!MAX_ADJOINING_SECTORS)
      MAX_ADJOINING_SECTORS = dsda_Flag(dsda_arg_doom95) ? 500 : 20;

    if (sec->linecount > heightlist_size)
    {
      do
      {
        heightlist_size = heightlist_size ? heightlist_size * 2 : 128;
      } while (sec->linecount > heightlist_size);
      heightlist = Z_Realloc(heightlist, heightlist_size * sizeof(heightlist[0]));
    }

    for (i=0, h=0 ;i < sec->linecount ; i++)
    {
      check = sec->lines[i];
      other = getNextSector(check,sec);

      if (!other)
        continue;

      if (other->floorheight > height)
      {
        // e6y
        // Emulation of stack overflow.
        // 20: overflow affects nothing - just a luck;
        // 21: can be emulated;
        // 22..26: overflow affects saved registers - unpredictable behaviour, can crash;
        // 27: overflow affects return address - crash with high probability;
        if (compatibility_level < dosdoom_compatibility && h >= MAX_ADJOINING_SECTORS)
        {
          if (h == MAX_ADJOINING_SECTORS + 1)
            height = other->floorheight;

          // 20 & 21 are common and not "warning" worthy
          if (h > MAX_ADJOINING_SECTORS + 1)
          {
            lprintf(LO_WARN, "P_FindNextHighestFloor: Overflow of heightlist[%d] array is detected.\n", MAX_ADJOINING_SECTORS);
            lprintf(LO_WARN, " Sector %d, line %d, heightlist index %d: ", sec->iSectorID, sec->lines[i]->iLineID, h);

            if (h <= MAX_ADJOINING_SECTORS + 6)
              lprintf(LO_WARN, "cannot be emulated - unpredictable behaviour.\n");
            else
              lprintf(LO_WARN, "cannot be emulated - crash with high probability.\n");
          }
        }
        heightlist[h++] = other->floorheight;
      }

      // Check for overflow. Warning.
      if (compatibility_level >= dosdoom_compatibility && h >= MAX_ADJOINING_SECTORS)
      {
        lprintf( LO_WARN, "Sector with more than 20 adjoining sectors\n" );
        break;
      }
    }

    // Find lowest height in list
    if (!h)
    {
      // cph - my guess at doom v1.2 - 1.4beta compatibility here.
      // If there are no higher neighbouring sectors, Heretic just returned
      // heightlist[0] (local variable), i.e. noise off the stack. 0 is right for
      // RETURN01 E1M2, so let's take that.
      //
      // SmileTheory's response:
      // It's not *quite* random stack noise. If this function is called
      // as part of a loop, heightlist will be at the same location as in
      // the previous call. Doing it this way fixes 1_ON_1.WAD.
      return (compatibility_level < doom_1666_compatibility ? last_height_0 : currentheight);
    }

    last_height_0 = heightlist[0];
    min = heightlist[0];

    // Range checking?
    for (i = 1;i < h;i++)
    {
      if (heightlist[i] < min)
        min = heightlist[i];
    }

    return min;
  }


  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
         other->floorheight > currentheight)
    {
      int height = other->floorheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->floorheight < height &&
            other->floorheight > currentheight)
          height = other->floorheight;
      return height;
    }
  /* cph - my guess at doom v1.2 - 1.4beta compatibility here.
   * If there are no higher neighbouring sectors, Heretic just returned
   * heightlist[0] (local variable), i.e. noise off the stack. 0 is right for
   * RETURN01 E1M2, so let's take that. */
  return (compatibility_level < doom_1666_compatibility ? 0 : currentheight);
}


//
// P_FindNextLowestFloor()
//
// Passed a sector and a floor height, returns the fixed point value
// of the largest floor height in a surrounding sector smaller than
// the floor height passed. If no such height exists the floorheight
// passed is returned.
//
// jff 02/03/98 Twiddled Lee's P_FindNextHighestFloor to make this
//
fixed_t P_FindNextLowestFloor(sector_t *sec, int currentheight)
{
  sector_t *other;
  int i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
         other->floorheight < currentheight)
    {
      int height = other->floorheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->floorheight > height &&
            other->floorheight < currentheight)
          height = other->floorheight;
      return height;
    }
  return currentheight;
}


//
// P_FindNextLowestCeiling()
//
// Passed a sector and a ceiling height, returns the fixed point value
// of the largest ceiling height in a surrounding sector smaller than
// the ceiling height passed. If no such height exists the ceiling height
// passed is returned.
//
// jff 02/03/98 Twiddled Lee's P_FindNextHighestFloor to make this
//
fixed_t P_FindNextLowestCeiling(sector_t *sec, int currentheight)
{
  sector_t *other;
  int i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
        other->ceilingheight < currentheight)
    {
      int height = other->ceilingheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->ceilingheight > height &&
            other->ceilingheight < currentheight)
          height = other->ceilingheight;
      return height;
    }
  return currentheight;
}


//
// P_FindNextHighestCeiling()
//
// Passed a sector and a ceiling height, returns the fixed point value
// of the smallest ceiling height in a surrounding sector larger than
// the ceiling height passed. If no such height exists the ceiling height
// passed is returned.
//
// jff 02/03/98 Twiddled Lee's P_FindNextHighestFloor to make this
//
fixed_t P_FindNextHighestCeiling(sector_t *sec, int currentheight)
{
  sector_t *other;
  int i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
         other->ceilingheight > currentheight)
    {
      int height = other->ceilingheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->ceilingheight < height &&
            other->ceilingheight > currentheight)
          height = other->ceilingheight;
      return height;
    }
  return currentheight;
}


//
// P_FindLowestCeilingSurrounding()
//
// Passed a sector, returns the fixed point value of the smallest
// ceiling height in the surrounding sectors, not including that passed
//
// NOTE: if no surrounding sector exists 32000*FRACUINT is returned
//       but if compatibility then INT_MAX is the return
//
fixed_t P_FindLowestCeilingSurrounding(sector_t* sec)
{
  int                 i;
  line_t*             check;
  sector_t*           other;
  fixed_t             height = INT_MAX;

  /* jff 3/12/98 avoid ovf in height calculations */
  if (!comp[comp_model]) height = 32000*FRACUNIT;

  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);

    if (!other)
      continue;

    if (other->ceilingheight < height)
      height = other->ceilingheight;
  }
  return height;
}


//
// P_FindHighestCeilingSurrounding()
//
// Passed a sector, returns the fixed point value of the largest
// ceiling height in the surrounding sectors, not including that passed
//
// NOTE: if no surrounding sector exists -32000*FRACUINT is returned
//       but if compatibility then 0 is the smallest return possible
//
fixed_t P_FindHighestCeilingSurrounding(sector_t* sec)
{
  int             i;
  line_t* check;
  sector_t*       other;
  fixed_t height = 0;

  /* jff 1/26/98 Fix initial value for floor to not act differently
   * in sections of wad that are below 0 units
   * jff 3/12/98 avoid ovf in height calculations */
  if (!comp[comp_model]) height = -32000*FRACUNIT;

  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);

    if (!other)
      continue;

    if (other->ceilingheight > height)
      height = other->ceilingheight;
  }
  return height;
}


//
// P_FindShortestTextureAround()
//
// Passed a sector number, returns the shortest lower texture on a
// linedef bounding the sector.
//
// Note: If no lower texture exists 32000*FRACUNIT is returned.
//       but if compatibility then INT_MAX is returned
//
// jff 02/03/98 Add routine to find shortest lower texture
//
fixed_t P_FindShortestTextureAround(int secnum)
{
  int minsize = INT_MAX;
  side_t*     side;
  int i;
  sector_t *sec = &sectors[secnum];

  if (!comp[comp_model])
    minsize = 32000<<FRACBITS; //jff 3/13/98 prevent overflow in height calcs

  for (i = 0; i < sec->linecount; i++)
  {
    if (twoSided(secnum, i))
    {
      side = getSide(secnum,i,0);
      if (side->bottomtexture > 0)  //jff 8/14/98 texture 0 is a placeholder
        if (textureheight[side->bottomtexture] < minsize)
          minsize = textureheight[side->bottomtexture];
      side = getSide(secnum,i,1);
      if (side->bottomtexture > 0)  //jff 8/14/98 texture 0 is a placeholder
        if (textureheight[side->bottomtexture] < minsize)
          minsize = textureheight[side->bottomtexture];
    }
  }
  return minsize;
}


//
// P_FindShortestUpperAround()
//
// Passed a sector number, returns the shortest upper texture on a
// linedef bounding the sector.
//
// Note: If no upper texture exists 32000*FRACUNIT is returned.
//       but if compatibility then INT_MAX is returned
//
// jff 03/20/98 Add routine to find shortest upper texture
//
fixed_t P_FindShortestUpperAround(int secnum)
{
  int minsize = INT_MAX;
  side_t*     side;
  int i;
  sector_t *sec = &sectors[secnum];

  if (!comp[comp_model])
    minsize = 32000<<FRACBITS; //jff 3/13/98 prevent overflow
                               // in height calcs
  for (i = 0; i < sec->linecount; i++)
  {
    if (twoSided(secnum, i))
    {
      side = getSide(secnum,i,0);
      if (side->toptexture > 0)     //jff 8/14/98 texture 0 is a placeholder
        if (textureheight[side->toptexture] < minsize)
          minsize = textureheight[side->toptexture];
      side = getSide(secnum,i,1);
      if (side->toptexture > 0)     //jff 8/14/98 texture 0 is a placeholder
        if (textureheight[side->toptexture] < minsize)
          minsize = textureheight[side->toptexture];
    }
  }
  return minsize;
}


//
// P_FindModelFloorSector()
//
// Passed a floor height and a sector number, return a pointer to a
// a sector with that floor height across the lowest numbered two sided
// line surrounding the sector.
//
// Note: If no sector at that height bounds the sector passed, return NULL
//
// jff 02/03/98 Add routine to find numeric model floor
//  around a sector specified by sector number
// jff 3/14/98 change first parameter to plain height to allow call
//  from routine not using floormove_t
//
sector_t *P_FindModelFloorSector(fixed_t floordestheight,int secnum)
{
  int i;
  sector_t *sec=NULL;
  int linecount;

  sec = &sectors[secnum]; //jff 3/2/98 woops! better do this
  //jff 5/23/98 don't disturb sec->linecount while searching
  // but allow early exit in old demos
  linecount = sec->linecount;
  for (i = 0; i < (demo_compatibility && sec->linecount<linecount?
                   sec->linecount : linecount); i++)
  {
    if ( twoSided(secnum, i) )
    {
      if (getSide(secnum,i,0)->sector->iSectorID == secnum)
          sec = getSector(secnum,i,1);
      else
          sec = getSector(secnum,i,0);

      if (heretic || sec->floorheight == floordestheight)
        return sec;
    }
  }
  return NULL;
}


//
// P_FindModelCeilingSector()
//
// Passed a ceiling height and a sector number, return a pointer to a
// a sector with that ceiling height across the lowest numbered two sided
// line surrounding the sector.
//
// Note: If no sector at that height bounds the sector passed, return NULL
//
// jff 02/03/98 Add routine to find numeric model ceiling
//  around a sector specified by sector number
//  used only from generalized ceiling types
// jff 3/14/98 change first parameter to plain height to allow call
//  from routine not using ceiling_t
//
sector_t *P_FindModelCeilingSector(fixed_t ceildestheight,int secnum)
{
  int i;
  sector_t *sec=NULL;
  int linecount;

  sec = &sectors[secnum]; //jff 3/2/98 woops! better do this
  //jff 5/23/98 don't disturb sec->linecount while searching
  // but allow early exit in old demos
  linecount = sec->linecount;
  for (i = 0; i < (demo_compatibility && sec->linecount<linecount?
                   sec->linecount : linecount); i++)
  {
    if ( twoSided(secnum, i) )
    {
      if (getSide(secnum,i,0)->sector->iSectorID == secnum)
          sec = getSector(secnum,i,1);
      else
          sec = getSector(secnum,i,0);

      if (sec->ceilingheight == ceildestheight)
        return sec;
    }
  }
  return NULL;
}

// Converts Hexen's 0 (meaning no crush) to the internal value
int P_ConvertHexenCrush(int crush)
{
  return (crush ? crush : NO_CRUSH);
}

//
// P_FindMinSurroundingLight()
//
// Passed a sector and a light level, returns the smallest light level
// in a surrounding sector less than that passed. If no smaller light
// level exists, the light level passed is returned.
//
int P_FindMinSurroundingLight
( sector_t*     sector,
  int           max )
{
  int         i;
  int         min;
  line_t*     line;
  sector_t*   check;

  min = max;
  for (i=0 ; i < sector->linecount ; i++)
  {
    line = sector->lines[i];
    check = getNextSector(line,sector);

    if (!check)
      continue;

    if (check->lightlevel < min)
      min = check->lightlevel;
  }
  return min;
}


//
// P_CanUnlockGenDoor()
//
// Passed a generalized locked door linedef and a player, returns whether
// the player has the keys necessary to unlock that door.
//
// Note: The linedef passed MUST be a generalized locked door type
//       or results are undefined.
//
// jff 02/05/98 routine added to test for unlockability of
//  generalized locked doors
//
dboolean P_CanUnlockGenDoor
( line_t* line,
  player_t* player)
{
  // does this line special distinguish between skulls and keys?
  int skulliscard = (line->special & LockedNKeys)>>LockedNKeysShift;

  // determine for each case of lock type if player's keys are adequate
  switch((line->special & LockedKey)>>LockedKeyShift)
  {
    case AnyKey:
      if
      (
        !player->cards[it_redcard] &&
        !player->cards[it_redskull] &&
        !player->cards[it_bluecard] &&
        !player->cards[it_blueskull] &&
        !player->cards[it_yellowcard] &&
        !player->cards[it_yellowskull]
      )
      {
        dsda_AddPlayerMessage(s_PD_ANY, player);
        S_StartMobjSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case RCard:
      if
      (
        !player->cards[it_redcard] &&
        (!skulliscard || !player->cards[it_redskull])
      )
      {
        dsda_AddPlayerMessage(skulliscard ? s_PD_REDK : s_PD_REDC, player);
        S_StartMobjSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case BCard:
      if
      (
        !player->cards[it_bluecard] &&
        (!skulliscard || !player->cards[it_blueskull])
      )
      {
        dsda_AddPlayerMessage(skulliscard ? s_PD_BLUEK : s_PD_BLUEC, player);
        S_StartMobjSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case YCard:
      if
      (
        !player->cards[it_yellowcard] &&
        (!skulliscard || !player->cards[it_yellowskull])
      )
      {
        dsda_AddPlayerMessage(skulliscard ? s_PD_YELLOWK : s_PD_YELLOWC, player);
        S_StartMobjSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case RSkull:
      if
      (
        !player->cards[it_redskull] &&
        (!skulliscard || !player->cards[it_redcard])
      )
      {
        dsda_AddPlayerMessage(skulliscard ? s_PD_REDK : s_PD_REDS, player);
        S_StartMobjSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case BSkull:
      if
      (
        !player->cards[it_blueskull] &&
        (!skulliscard || !player->cards[it_bluecard])
      )
      {
        dsda_AddPlayerMessage(skulliscard ? s_PD_BLUEK : s_PD_BLUES, player);
        S_StartMobjSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case YSkull:
      if
      (
        !player->cards[it_yellowskull] &&
        (!skulliscard || !player->cards[it_yellowcard])
      )
      {
        dsda_AddPlayerMessage(skulliscard ? s_PD_YELLOWK : s_PD_YELLOWS, player);
        S_StartMobjSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case AllKeys:
      if
      (
        !skulliscard &&
        (
          !player->cards[it_redcard] ||
          !player->cards[it_redskull] ||
          !player->cards[it_bluecard] ||
          !player->cards[it_blueskull] ||
          !player->cards[it_yellowcard] ||
          !player->cards[it_yellowskull]
        )
      )
      {
        dsda_AddPlayerMessage(s_PD_ALL6, player);
        S_StartMobjSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      if
      (
        skulliscard &&
        (
          (!player->cards[it_redcard] &&
            !player->cards[it_redskull]) ||
          (!player->cards[it_bluecard] &&
            !player->cards[it_blueskull]) ||
          // e6y
          // Compatibility with buggy MBF behavior when 3-key door works with only 2 keys
          // There is no more desync on 10sector.wad\ts27-137.lmp
          // http://www.doomworld.com/tas/ts27-137.zip
          (!player->cards[it_yellowcard] &&
            (compatibility_level == mbf_compatibility &&
             !prboom_comp[PC_FORCE_CORRECT_CODE_FOR_3_KEYS_DOORS_IN_MBF].state ?
             player->cards[it_yellowskull] :
             !player->cards[it_yellowskull]))
        )
      )
      {
        dsda_AddPlayerMessage(s_PD_ALL3, player);
        S_StartMobjSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
  }
  return true;
}

dboolean P_CheckKeys(mobj_t *mo, zdoom_lock_t lock, dboolean legacy)
{
  player_t *player;
  const char *message = NULL;
  int sfx = sfx_None;
  dboolean successful = true;

  if (!mo || !mo->player)
    return false;

  player = mo->player;

  switch (lock)
  {
    case zk_none:
      break;
    case zk_red_card:
      if (!player->cards[it_redcard])
      {
        message = legacy ? s_PD_REDC : NULL;
        sfx = legacy ? sfx_oof : sfx_None;
        successful = false;
      }
      break;
    case zk_blue_card:
      if (!player->cards[it_bluecard])
      {
        message = legacy ? s_PD_BLUEC : NULL;
        sfx = legacy ? sfx_oof : sfx_None;
        successful = false;
      }
      break;
    case zk_yellow_card:
      if (!player->cards[it_yellowcard])
      {
        message = legacy ? s_PD_YELLOWC : NULL;
        sfx = legacy ? sfx_oof : sfx_None;
        successful = false;
      }
      break;
    case zk_red_skull:
      if (!player->cards[it_redskull])
      {
        message = legacy ? s_PD_REDS : NULL;
        sfx = legacy ? sfx_oof : sfx_None;
        successful = false;
      }
      break;
    case zk_blue_skull:
      if (!player->cards[it_blueskull])
      {
        message = legacy ? s_PD_BLUES : NULL;
        sfx = legacy ? sfx_oof : sfx_None;
        successful = false;
      }
      break;
    case zk_yellow_skull:
      if (!player->cards[it_yellowskull])
      {
        message = legacy ? s_PD_YELLOWS : NULL;
        sfx = legacy ? sfx_oof : sfx_None;
        successful = false;
      }
      break;
    case zk_any:
      if (
        !player->cards[it_redcard] &&
        !player->cards[it_redskull] &&
        !player->cards[it_bluecard] &&
        !player->cards[it_blueskull] &&
        !player->cards[it_yellowcard] &&
        !player->cards[it_yellowskull]
      )
      {
        message = legacy ? s_PD_ANY : NULL;
        sfx = legacy ? sfx_oof : sfx_None;
        successful = false;
      }
      break;
    case zk_all:
      if (
        !player->cards[it_redcard] ||
        !player->cards[it_redskull] ||
        !player->cards[it_bluecard] ||
        !player->cards[it_blueskull] ||
        !player->cards[it_yellowcard] ||
        !player->cards[it_yellowskull]
      )
      {
        message = legacy ? s_PD_ALL6 : NULL;
        sfx = legacy ? sfx_oof : sfx_None;
        successful = false;
      }
      break;
    case zk_red:
    case zk_redx:
      if (!player->cards[it_redcard] && !player->cards[it_redskull])
      {
        message = legacy ? s_PD_REDK : NULL;
        sfx = legacy ? sfx_oof : sfx_None;
        successful = false;
      }
      break;
    case zk_blue:
    case zk_bluex:
      if (!player->cards[it_bluecard] && !player->cards[it_blueskull])
      {
        message = legacy ? s_PD_BLUEK : NULL;
        sfx = legacy ? sfx_oof : sfx_None;
        successful = false;
      }
      break;
    case zk_yellow:
    case zk_yellowx:
      if (!player->cards[it_yellowcard] && !player->cards[it_yellowskull])
      {
        message = legacy ? s_PD_YELLOWK : NULL;
        sfx = legacy ? sfx_oof : sfx_None;
        successful = false;
      }
      break;
    case zk_each_color:
      if (
        (!player->cards[it_redcard] && !player->cards[it_redskull]) ||
        (!player->cards[it_bluecard] && !player->cards[it_blueskull]) ||
        (!player->cards[it_yellowcard] && !player->cards[it_yellowskull])
      )
      {
        message = legacy ? s_PD_ALL3 : NULL;
        sfx = legacy ? sfx_oof : sfx_None;
        successful = false;
      }
    default:
      break;
  }

  if (message)
  {
    dsda_AddPlayerMessage(message, player);
  }

  if (sfx != sfx_None)
  {
    S_StartMobjSound(mo, sfx);
  }

  return successful;
}


//
// P_SectorActive()
//
// In old compatibility levels, floor and ceiling data couldn't coexist.
// Lighting data is only relevant in zdoom levels.
//

dboolean PUREFUNC P_PlaneActive(const sector_t *sec)
{
  return sec->ceilingdata != NULL || sec->floordata != NULL;
}

dboolean PUREFUNC P_CeilingActive(const sector_t *sec)
{
  return sec->ceilingdata != NULL || (demo_compatibility && sec->floordata != NULL);
}

dboolean PUREFUNC P_FloorActive(const sector_t *sec)
{
  return sec->floordata != NULL || (demo_compatibility && sec->ceilingdata != NULL);
}

dboolean PUREFUNC P_LightingActive(const sector_t *sec)
{
  return sec->lightingdata != NULL;
}

short P_FloorLightLevel(const sector_t *sec)
{
  return sec->lightlevel_floor + (
    (sec->flags & SECF_LIGHTFLOORABSOLUTE) ? 0 : (
      sec->floorlightsec == -1 ? sec->lightlevel
                               : sectors[sec->floorlightsec].lightlevel
    )
  );
}

short P_CeilingLightLevel(const sector_t *sec)
{
  return sec->lightlevel_ceiling + (
    (sec->flags & SECF_LIGHTCEILINGABSOLUTE) ? 0 : (
      sec->ceilinglightsec == -1 ? sec->lightlevel
                               : sectors[sec->ceilinglightsec].lightlevel
    )
  );
}

dboolean P_FloorPlanesDiffer(const sector_t *sec, const sector_t *other)
{
  return sec->floorpic != other->floorpic ||
         sec->floor_xoffs != other->floor_xoffs ||
         sec->floor_yoffs != other->floor_yoffs ||
         sec->floor_rotation != other->floor_rotation ||
         sec->floor_xscale != other->floor_xscale ||
         sec->floor_yscale != other->floor_yscale ||
         sec->special != other->special ||
         sec->floorlightsec != other->floorlightsec ||
         P_FloorLightLevel(sec) != P_FloorLightLevel(other);
}

dboolean P_CeilingPlanesDiffer(const sector_t *sec, const sector_t *other)
{
  return sec->ceilingpic != other->ceilingpic ||
         sec->ceiling_xoffs != other->ceiling_xoffs ||
         sec->ceiling_yoffs != other->ceiling_yoffs ||
         sec->ceiling_rotation != other->ceiling_rotation ||
         sec->ceiling_xscale != other->ceiling_xscale ||
         sec->ceiling_yscale != other->ceiling_yscale ||
         sec->ceilinglightsec != other->ceilinglightsec ||
         P_CeilingLightLevel(sec) != P_CeilingLightLevel(other);
}

//
// P_CheckTag()
//
// Passed a line, returns true if the tag is non-zero or the line special
// allows no tag without harm. If compatibility, all linedef specials are
// allowed to have zero tag.
//
// Note: Only line specials activated by walkover, pushing, or shooting are
//       checked by this routine.
//
// jff 2/27/98 Added to check for zero tag allowed for regular special types
//
int P_CheckTag(line_t *line)
{
  /* tag not zero, allowed, or
   * killough 11/98: compatibility option */
  if (comp[comp_zerotags] || line->tag)//e6y
    return 1;

  switch(line->special)
  {
    case 1:                 // Manual door specials
    case 26:
    case 27:
    case 28:
    case 31:
    case 32:
    case 33:
    case 34:
    case 117:
    case 118:

    case 139:               // Lighting specials
    case 170:
    case 79:
    case 35:
    case 138:
    case 171:
    case 81:
    case 13:
    case 192:
    case 169:
    case 80:
    case 12:
    case 194:
    case 173:
    case 157:
    case 104:
    case 193:
    case 172:
    case 156:
    case 17:

    case 195:               // Thing teleporters
    case 174:
    case 97:
    case 39:
    case 126:
    case 125:
    case 210:
    case 209:
    case 208:
    case 207:

    case 11:                // Exits
    case 52:
    case 197:
    case 51:
    case 124:
    case 198:

    case 48:                // Scrolling walls
    case 85:
      return 1;   // zero tag allowed

    default:
      break;
  }
  return 0;       // zero tag not allowed
}

static const damage_t no_damage = { 0 };

static void P_TransferSectorFlags(unsigned int *dest, unsigned int source)
{
  *dest &= ~SECF_TRANSFERMASK;
  *dest |= source & SECF_TRANSFERMASK;
}

static void P_ResetSectorTransferFlags(unsigned int *flags)
{
  *flags &= ~SECF_TRANSFERMASK;
}

void P_CopySectorSpecial(sector_t *dest, sector_t *source)
{
  dest->special = source->special;
  dest->damage = source->damage;
  P_TransferSectorFlags(&dest->flags, source->flags);
}

void P_TransferSpecial(sector_t *sector, newspecial_t *newspecial)
{
  sector->special = newspecial->special;
  sector->damage = newspecial->damage;
  P_TransferSectorFlags(&sector->flags, newspecial->flags);
}

void P_CopyTransferSpecial(newspecial_t *newspecial, sector_t *sector)
{
  newspecial->special = sector->special;
  newspecial->damage = sector->damage;
  P_TransferSectorFlags(&newspecial->flags, sector->flags);
}

void P_ResetTransferSpecial(newspecial_t *newspecial)
{
  newspecial->special = 0;
  newspecial->damage = no_damage;
  P_ResetSectorTransferFlags(&newspecial->flags);
}

void P_ResetSectorSpecial(sector_t *sector)
{
  sector->special = 0;
  sector->damage = no_damage;
  P_ResetSectorTransferFlags(&sector->flags);
}

void P_ClearNonGeneralizedSectorSpecial(sector_t *sector)
{
  // jff 3/14/98 clear non-generalized sector type
  sector->special &= map_format.generalized_mask;
}

dboolean P_IsSpecialSector(sector_t *sector)
{
  return sector->special || sector->flags & SECF_SECRET || sector->damage.amount;
}

static void P_AddSectorSecret(sector_t *sector)
{
  totalsecret++;
  sector->flags |= SECF_SECRET | SECF_WASSECRET;
}

void P_AddMobjSecret(mobj_t *mobj)
{
  totalsecret++;
  mobj->flags2 |= MF2_COUNTSECRET;
}

void P_PlayerCollectSecret(player_t *player)
{
  player->secretcount++;

  if (dsda_IntConfig(dsda_config_hudadd_secretarea))
  {
    int sfx_id = raven ? g_sfx_secret :
                 I_GetSfxLumpNum(&S_sfx[g_sfx_secret]) < 0 ? sfx_itmbk : g_sfx_secret;
    SetCustomMessage(player - players, "A secret is revealed!", 2 * TICRATE, sfx_id);
  }
}

static void P_CollectSecretCommon(sector_t *sector, player_t *player)
{
  sector->flags &= ~SECF_SECRET;

  P_PlayerCollectSecret(player);

  dsda_WatchSecret();
}

static void P_CollectSecretVanilla(sector_t *sector, player_t *player)
{
  sector->special = 0;
  P_CollectSecretCommon(sector, player);
}

static void P_CollectSecretBoom(sector_t *sector, player_t *player)
{
  sector->special &= ~SECRET_MASK;

  if (sector->special < 32) // if all extended bits clear,
    sector->special = 0;    // sector is not special anymore

  P_CollectSecretCommon(sector, player);
}

static void P_CollectSecretZDoom(sector_t *sector, player_t *player)
{
  P_CollectSecretCommon(sector, player);
}

//
// P_IsSecret()
//
// Passed a sector, returns if the sector secret type is still active, i.e.
// secret type is set and the secret has not yet been obtained.
//
// jff 3/14/98 added to simplify checks for whether sector is secret
//  in automap and other places
//
dboolean PUREFUNC P_IsSecret(const sector_t *sec)
{
  return (sec->flags & SECF_SECRET) != 0;
}


//
// P_WasSecret()
//
// Passed a sector, returns if the sector secret type is was active, i.e.
// secret type was set and the secret has been obtained already.
//
// jff 3/14/98 added to simplify checks for whether sector is secret
//  in automap and other places
//
dboolean PUREFUNC P_WasSecret(const sector_t *sec)
{
  return (sec->flags & SECF_WASSECRET) != 0;
}

dboolean PUREFUNC P_RevealedSecret(const sector_t *sec)
{
  return P_WasSecret(sec) && !P_IsSecret(sec);
}

// TODO: account for map / cluster info (zdoom: CheckIfExitIsGood)
dboolean P_CanExit(mobj_t *mo)
{
  return !(mo && mo->player && mo->player->playerstate == PST_DEAD);
}

void P_CrossHexenSpecialLine(line_t *line, int side, mobj_t *thing, dboolean bossaction)
{
  if (thing->player)
  {
    P_ActivateLine(line, thing, side, SPAC_CROSS);
  }
  else if (thing->flags2 & MF2_MCROSS)
  {
    P_ActivateLine(line, thing, side, SPAC_MCROSS);
  }
  else if (thing->flags2 & MF2_PCROSS)
  {
    P_ActivateLine(line, thing, side, SPAC_PCROSS);
  }
}

//////////////////////////////////////////////////////////////////////////
//
// Events
//
// Events are operations triggered by using, crossing,
// or shooting special lines, or by timed thinkers.
//
/////////////////////////////////////////////////////////////////////////

//
// P_CrossSpecialLine - Walkover Trigger Dispatcher
//
// Called every time a thing origin is about
//  to cross a line with a non 0 special, whether a walkover type or not.
//
// jff 02/12/98 all W1 lines were fixed to check the result from the EV_
//  function before clearing the special. This avoids losing the function
//  of the line, should the sector already be active when the line is
//  crossed. Change is qualified by demo_compatibility.
//
// CPhipps - take a line_t pointer instead of a line number, as in MBF
void P_CrossCompatibleSpecialLine(line_t *line, int side, mobj_t *thing, dboolean bossaction)
{
  int ok;

  dsda_WatchLineActivation(line, thing);

  //  Things that should never trigger lines
  //
  // e6y: Improved support for Doom v1.2
  if (compatibility_level == doom_12_compatibility)
  {
    if (line->special > 98 && line->special != 104)
    {
      return;
    }
  }
  else
  {
    if (!thing->player && !bossaction)
    {
      // Things that should NOT trigger specials...
      switch(thing->type)
      {
      case MT_ROCKET:
      case MT_PLASMA:
      case MT_BFG:
      case MT_TROOPSHOT:
      case MT_HEADSHOT:
      case MT_BRUISERSHOT:
        return;
        break;

      default: break;
      }
    }
  }

  //jff 02/04/98 add check here for generalized lindef types
  if (!demo_compatibility) // generalized types not recognized if old demo
  {
    // pointer to line function is NULL by default, set non-null if
    // line special is walkover generalized linedef type
    int (*linefunc)(line_t *line)=NULL;

    // check each range of generalized linedefs
    if ((unsigned)line->special >= GenEnd)
    {
      // Out of range for GenFloors
    }
    else if ((unsigned)line->special >= GenFloorBase)
    {
      if (!thing->player && !bossaction)
        if ((line->special & FloorChange) || !(line->special & FloorModel))
          return;     // FloorModel is "Allow Monsters" if FloorChange is 0
      if (!line->tag) //jff 2/27/98 all walk generalized types require tag
        return;
      linefunc = EV_DoGenFloor;
    }
    else if ((unsigned)line->special >= GenCeilingBase)
    {
      if (!thing->player && !bossaction)
        if ((line->special & CeilingChange) || !(line->special & CeilingModel))
          return;     // CeilingModel is "Allow Monsters" if CeilingChange is 0
      if (!line->tag) //jff 2/27/98 all walk generalized types require tag
        return;
      linefunc = EV_DoGenCeiling;
    }
    else if ((unsigned)line->special >= GenDoorBase)
    {
      if (!thing->player && !bossaction)
      {
        if (!(line->special & DoorMonster))
          return;                    // monsters disallowed from this door
        if (line->flags & ML_SECRET) // they can't open secret doors either
          return;
      }
      if (!line->tag) //3/2/98 move outside the monster check
        return;
      linefunc = EV_DoGenDoor;
    }
    else if ((unsigned)line->special >= GenLockedBase)
    {
      if (!thing->player || bossaction) // boss actions can't handle locked doors
        return;                     // monsters disallowed from unlocking doors
      if (((line->special&TriggerType)==WalkOnce) || ((line->special&TriggerType)==WalkMany))
      { //jff 4/1/98 check for being a walk type before reporting door type
        if (!P_CanUnlockGenDoor(line,thing->player))
          return;
      }
      else
        return;
      linefunc = EV_DoGenLockedDoor;
    }
    else if ((unsigned)line->special >= GenLiftBase)
    {
      if (!thing->player && !bossaction)
        if (!(line->special & LiftMonster))
          return; // monsters disallowed
      if (!line->tag) //jff 2/27/98 all walk generalized types require tag
        return;
      linefunc = EV_DoGenLift;
    }
    else if ((unsigned)line->special >= GenStairsBase)
    {
      if (!thing->player && !bossaction)
        if (!(line->special & StairMonster))
          return; // monsters disallowed
      if (!line->tag) //jff 2/27/98 all walk generalized types require tag
        return;
      linefunc = EV_DoGenStairs;
    }
    else if (mbf21 && (unsigned)line->special >= GenCrusherBase)
    {
      // haleyjd 06/09/09: This was completely forgotten in BOOM, disabling
      // all generalized walk-over crusher types!
      if (!thing->player && !bossaction)
        if (!(line->special & StairMonster))
          return; // monsters disallowed
      if (!line->tag) //jff 2/27/98 all walk generalized types require tag
        return;
      linefunc = EV_DoGenCrusher;
    }

    if (linefunc) // if it was a valid generalized type
      switch((line->special & TriggerType) >> TriggerTypeShift)
      {
        case WalkOnce:
          if (linefunc(line))
            line->special = 0;    // clear special if a walk once type
          return;
        case WalkMany:
          linefunc(line);
          return;
        default:                  // if not a walk type, do nothing here
          return;
      }
  }

  if (!thing->player || bossaction)
  {
    ok = 0;
    switch(line->special)
    {
      // teleporters are blocked for boss actions.
      case 39:      // teleport trigger
      case 97:      // teleport retrigger
      case 125:     // teleport monsteronly trigger
      case 126:     // teleport monsteronly retrigger
        //jff 3/5/98 add ability of monsters etc. to use teleporters
      case 208:     //silent thing teleporters
      case 207:
      case 243:     //silent line-line teleporter
      case 244:     //jff 3/6/98 make fit within DCK's 256 linedef types
      case 262:     //jff 4/14/98 add monster only
      case 263:     //jff 4/14/98 silent thing,line,line rev types
      case 264:     //jff 4/14/98 plus player/monster silent line
      case 265:     //            reversed types
      case 266:
      case 267:
      case 268:
      case 269:
        if (bossaction) return;

      case 4:       // raise door
      case 10:      // plat down-wait-up-stay trigger
      case 88:      // plat down-wait-up-stay retrigger
        ok = 1;
        break;
    }
    if (!ok)
      return;
  }

  if (!P_CheckTag(line))  //jff 2/27/98 disallow zero tag on some types
    return;

  // Dispatch on the line special value to the line's action routine
  // If a once only function, and successful, clear the line special

  switch (line->special)
  {
      // Regular walk once triggers

    case 2:
      // Open Door
      if (EV_DoDoor(line,openDoor) || demo_compatibility)
        line->special = 0;
      break;

    case 3:
      // Close Door
      if (EV_DoDoor(line,closeDoor) || demo_compatibility)
        line->special = 0;
      break;

    case 4:
      // Raise Door
      if (EV_DoDoor(line,normal) || demo_compatibility)
        line->special = 0;
      break;

    case 5:
      // Raise Floor
      if (EV_DoFloor(line,raiseFloor) || demo_compatibility)
        line->special = 0;
      break;

    case 6:
      // Fast Ceiling Crush & Raise
      if (EV_DoCeiling(line,fastCrushAndRaise) || demo_compatibility)
        line->special = 0;
      break;

    case 8:
      // Build Stairs
      if (EV_BuildStairs(line,build8) || demo_compatibility)
        line->special = 0;
      break;

    case 10:
      // PlatDownWaitUp
      if (EV_DoPlat(line,downWaitUpStay,0) || demo_compatibility)
        line->special = 0;
      break;

    case 12:
      // Light Turn On - brightest near
      if (EV_LightTurnOn(line,0) || demo_compatibility)
        line->special = 0;
      break;

    case 13:
      // Light Turn On 255
      if (EV_LightTurnOn(line,255) || demo_compatibility)
        line->special = 0;
      break;

    case 16:
      // Close Door 30
      if (EV_DoDoor(line,close30ThenOpen) || demo_compatibility)
        line->special = 0;
      break;

    case 17:
      // Start Light Strobing
      if (EV_StartLightStrobing(line) || demo_compatibility)
        line->special = 0;
      break;

    case 19:
      // Lower Floor
      if (EV_DoFloor(line,lowerFloor) || demo_compatibility)
        line->special = 0;
      break;

    case 22:
      // Raise floor to nearest height and change texture
      if (EV_DoPlat(line,raiseToNearestAndChange,0) || demo_compatibility)
        line->special = 0;
      break;

    case 25:
      // Ceiling Crush and Raise
      if (EV_DoCeiling(line,crushAndRaise) || demo_compatibility)
        line->special = 0;
      break;

    case 30:
      // Raise floor to shortest texture height
      //  on either side of lines.
      if (EV_DoFloor(line,raiseToTexture) || demo_compatibility)
        line->special = 0;
      break;

    case 35:
      // Lights Very Dark
      if (EV_LightTurnOn(line,35) || demo_compatibility)
        line->special = 0;
      break;

    case 36:
      // Lower Floor (TURBO)
      if (EV_DoFloor(line,turboLower) || demo_compatibility)
        line->special = 0;
      break;

    case 37:
      // LowerAndChange
      if (EV_DoFloor(line,lowerAndChange) || demo_compatibility)
        line->special = 0;
      break;

    case 38:
      // Lower Floor To Lowest
      if (EV_DoFloor(line, lowerFloorToLowest) || demo_compatibility)
        line->special = 0;
      break;

    case 39:
      // TELEPORT! //jff 02/09/98 fix using up with wrong side crossing
      if (map_format.ev_teleport(0, line->tag, line, side, thing, TELF_VANILLA) || demo_compatibility)
        line->special = 0;
      break;

    case 40:
      // RaiseCeilingLowerFloor
      if (demo_compatibility)
      {
        EV_DoCeiling( line, raiseToHighest );
        EV_DoFloor( line, lowerFloorToLowest ); //jff 02/12/98 doesn't work
        line->special = 0;
      }
      else
        if (EV_DoCeiling(line, raiseToHighest))
          line->special = 0;
      break;

    case 44:
      // Ceiling Crush
      if (EV_DoCeiling(line, lowerAndCrush) || demo_compatibility)
        line->special = 0;
      break;

    case 52:
      // EXIT!
      // killough 10/98: prevent zombies from exiting levels
      if (bossaction || (!(thing->player && thing->player->health <= 0 && !comp[comp_zombie])))
        G_ExitLevel(0);
      break;

    case 53:
      // Perpetual Platform Raise
      if (EV_DoPlat(line,perpetualRaise,0) || demo_compatibility)
        line->special = 0;
      break;

    case 54:
      // Platform Stop
      if (EV_StopPlat(line) || demo_compatibility)
        line->special = 0;
      break;

    case 56:
      // Raise Floor Crush
      if (EV_DoFloor(line,raiseFloorCrush) || demo_compatibility)
        line->special = 0;
      break;

    case 57:
      // Ceiling Crush Stop
      if (EV_CeilingCrushStop(line) || demo_compatibility)
        line->special = 0;
      break;

    case 58:
      // Raise Floor 24
      if (EV_DoFloor(line,raiseFloor24) || demo_compatibility)
        line->special = 0;
      break;

    case 59:
      // Raise Floor 24 And Change
      if (EV_DoFloor(line,raiseFloor24AndChange) || demo_compatibility)
        line->special = 0;
      break;

    case 100:
      // Build Stairs Turbo 16
      if (EV_BuildStairs(line,turbo16) || demo_compatibility)
        line->special = 0;
      break;

    case 104:
      // Turn lights off in sector(tag)
      if (EV_TurnTagLightsOff(line) || demo_compatibility)
        line->special = 0;
      break;

    case 108:
      // Blazing Door Raise (faster than TURBO!)
      if (EV_DoDoor(line,blazeRaise) || demo_compatibility)
        line->special = 0;
      break;

    case 109:
      // Blazing Door Open (faster than TURBO!)
      if (EV_DoDoor (line,blazeOpen) || demo_compatibility)
        line->special = 0;
      break;

    case 110:
      // Blazing Door Close (faster than TURBO!)
      if (EV_DoDoor (line,blazeClose) || demo_compatibility)
        line->special = 0;
      break;

    case 119:
      // Raise floor to nearest surr. floor
      if (EV_DoFloor(line,raiseFloorToNearest) || demo_compatibility)
        line->special = 0;
      break;

    case 121:
      // Blazing PlatDownWaitUpStay
      if (EV_DoPlat(line,blazeDWUS,0) || demo_compatibility)
        line->special = 0;
      break;

    case 124:
      // Secret EXIT
      // killough 10/98: prevent zombies from exiting levels
      // CPhipps - change for lxdoom's compatibility handling
      if (bossaction || (!(thing->player && thing->player->health <= 0 && !comp[comp_zombie])))
        G_SecretExitLevel(0);
      break;

    case 125:
      // TELEPORT MonsterONLY
      if (!thing->player &&
          (map_format.ev_teleport(0, line->tag, line, side, thing, TELF_VANILLA) || demo_compatibility))
        line->special = 0;
      break;

    case 130:
      // Raise Floor Turbo
      if (EV_DoFloor(line,raiseFloorTurbo) || demo_compatibility)
        line->special = 0;
      break;

    case 141:
      // Silent Ceiling Crush & Raise
      if (EV_DoCeiling(line,silentCrushAndRaise) || demo_compatibility)
        line->special = 0;
      break;

      // Regular walk many retriggerable

    case 72:
      // Ceiling Crush
      EV_DoCeiling( line, lowerAndCrush );
      break;

    case 73:
      // Ceiling Crush and Raise
      EV_DoCeiling(line,crushAndRaise);
      break;

    case 74:
      // Ceiling Crush Stop
      EV_CeilingCrushStop(line);
      break;

    case 75:
      // Close Door
      EV_DoDoor(line,closeDoor);
      break;

    case 76:
      // Close Door 30
      EV_DoDoor(line,close30ThenOpen);
      break;

    case 77:
      // Fast Ceiling Crush & Raise
      EV_DoCeiling(line,fastCrushAndRaise);
      break;

    case 79:
      // Lights Very Dark
      EV_LightTurnOn(line,35);
      break;

    case 80:
      // Light Turn On - brightest near
      EV_LightTurnOn(line,0);
      break;

    case 81:
      // Light Turn On 255
      EV_LightTurnOn(line,255);
      break;

    case 82:
      // Lower Floor To Lowest
      EV_DoFloor( line, lowerFloorToLowest );
      break;

    case 83:
      // Lower Floor
      EV_DoFloor(line,lowerFloor);
      break;

    case 84:
      // LowerAndChange
      EV_DoFloor(line,lowerAndChange);
      break;

    case 86:
      // Open Door
      EV_DoDoor(line,openDoor);
      break;

    case 87:
      // Perpetual Platform Raise
      EV_DoPlat(line,perpetualRaise,0);
      break;

    case 88:
      // PlatDownWaitUp
      EV_DoPlat(line,downWaitUpStay,0);
      break;

    case 89:
      // Platform Stop
      EV_StopPlat(line);
      break;

    case 90:
      // Raise Door
      EV_DoDoor(line,normal);
      break;

    case 91:
      // Raise Floor
      EV_DoFloor(line,raiseFloor);
      break;

    case 92:
      // Raise Floor 24
      EV_DoFloor(line,raiseFloor24);
      break;

    case 93:
      // Raise Floor 24 And Change
      EV_DoFloor(line,raiseFloor24AndChange);
      break;

    case 94:
      // Raise Floor Crush
      EV_DoFloor(line,raiseFloorCrush);
      break;

    case 95:
      // Raise floor to nearest height
      // and change texture.
      EV_DoPlat(line,raiseToNearestAndChange,0);
      break;

    case 96:
      // Raise floor to shortest texture height
      // on either side of lines.
      EV_DoFloor(line,raiseToTexture);
      break;

    case 97:
      // TELEPORT!
      map_format.ev_teleport( 0, line->tag, line, side, thing, TELF_VANILLA );
      break;

    case 98:
      // Lower Floor (TURBO)
      EV_DoFloor(line,turboLower);
      break;

    case 105:
      // Blazing Door Raise (faster than TURBO!)
      EV_DoDoor (line,blazeRaise);
      break;

    case 106:
      // Blazing Door Open (faster than TURBO!)
      EV_DoDoor (line,blazeOpen);
      break;

    case 107:
      // Blazing Door Close (faster than TURBO!)
      EV_DoDoor (line,blazeClose);
      break;

    case 120:
      // Blazing PlatDownWaitUpStay.
      EV_DoPlat(line,blazeDWUS,0);
      break;

    case 126:
      // TELEPORT MonsterONLY.
      if (!thing->player)
        map_format.ev_teleport( 0, line->tag, line, side, thing, TELF_VANILLA );
      break;

    case 128:
      // Raise To Nearest Floor
      EV_DoFloor(line,raiseFloorToNearest);
      break;

    case 129:
      // Raise Floor Turbo
      EV_DoFloor(line,raiseFloorTurbo);
      break;

      // Extended walk triggers

      // jff 1/29/98 added new linedef types to fill all functions out so that
      // all have varieties SR, S1, WR, W1

      // killough 1/31/98: "factor out" compatibility test, by
      // adding inner switch qualified by compatibility flag.
      // relax test to demo_compatibility

      // killough 2/16/98: Fix problems with W1 types being cleared too early

    default:
      if (!demo_compatibility)
        switch (line->special)
        {
          // Extended walk once triggers

          case 142:
            // Raise Floor 512
            // 142 W1  EV_DoFloor(raiseFloor512)
            if (EV_DoFloor(line,raiseFloor512))
              line->special = 0;
            break;

          case 143:
            // Raise Floor 24 and change
            // 143 W1  EV_DoPlat(raiseAndChange,24)
            if (EV_DoPlat(line,raiseAndChange,24))
              line->special = 0;
            break;

          case 144:
            // Raise Floor 32 and change
            // 144 W1  EV_DoPlat(raiseAndChange,32)
            if (EV_DoPlat(line,raiseAndChange,32))
              line->special = 0;
            break;

          case 145:
            // Lower Ceiling to Floor
            // 145 W1  EV_DoCeiling(lowerToFloor)
            if (EV_DoCeiling( line, lowerToFloor ))
              line->special = 0;
            break;

          case 146:
            // Lower Pillar, Raise Donut
            // 146 W1  EV_DoDonut()
            if (EV_DoDonut(line))
              line->special = 0;
            break;

          case 199:
            // Lower ceiling to lowest surrounding ceiling
            // 199 W1 EV_DoCeiling(lowerToLowest)
            if (EV_DoCeiling(line,lowerToLowest))
              line->special = 0;
            break;

          case 200:
            // Lower ceiling to highest surrounding floor
            // 200 W1 EV_DoCeiling(lowerToMaxFloor)
            if (EV_DoCeiling(line,lowerToMaxFloor))
              line->special = 0;
            break;

          case 207:
            // killough 2/16/98: W1 silent teleporter (normal kind)
            if (map_format.ev_teleport(0, line->tag, line, side, thing, TELF_SILENT))
              line->special = 0;
            break;

            //jff 3/16/98 renumber 215->153
          case 153: //jff 3/15/98 create texture change no motion type
            // Texture/Type Change Only (Trig)
            // 153 W1 Change Texture/Type Only
            if (EV_DoChange(line,trigChangeOnly,line->tag))
              line->special = 0;
            break;

          case 239: //jff 3/15/98 create texture change no motion type
            // Texture/Type Change Only (Numeric)
            // 239 W1 Change Texture/Type Only
            if (EV_DoChange(line,numChangeOnly,line->tag))
              line->special = 0;
            break;

          case 219:
            // Lower floor to next lower neighbor
            // 219 W1 Lower Floor Next Lower Neighbor
            if (EV_DoFloor(line,lowerFloorToNearest))
              line->special = 0;
            break;

          case 227:
            // Raise elevator next floor
            // 227 W1 Raise Elevator next floor
            if (EV_DoElevator(line,elevateUp))
              line->special = 0;
            break;

          case 231:
            // Lower elevator next floor
            // 231 W1 Lower Elevator next floor
            if (EV_DoElevator(line,elevateDown))
              line->special = 0;
            break;

          case 235:
            // Elevator to current floor
            // 235 W1 Elevator to current floor
            if (EV_DoElevator(line,elevateCurrent))
              line->special = 0;
            break;

          case 243: //jff 3/6/98 make fit within DCK's 256 linedef types
            // killough 2/16/98: W1 silent teleporter (linedef-linedef kind)
            if (EV_SilentLineTeleport(line, side, thing, line->tag, false))
              line->special = 0;
            break;

          case 262: //jff 4/14/98 add silent line-line reversed
            if (EV_SilentLineTeleport(line, side, thing, line->tag, true))
              line->special = 0;
            break;

          case 264: //jff 4/14/98 add monster-only silent line-line reversed
            if (!thing->player &&
                EV_SilentLineTeleport(line, side, thing, line->tag, true))
              line->special = 0;
            break;

          case 266: //jff 4/14/98 add monster-only silent line-line
            if (!thing->player &&
                EV_SilentLineTeleport(line, side, thing, line->tag, false))
              line->special = 0;
            break;

          case 268: //jff 4/14/98 add monster-only silent
            if (!thing->player && map_format.ev_teleport(0, line->tag, line, side, thing, TELF_SILENT))
              line->special = 0;
            break;

          //jff 1/29/98 end of added W1 linedef types

          // Extended walk many retriggerable

          //jff 1/29/98 added new linedef types to fill all functions
          //out so that all have varieties SR, S1, WR, W1

          case 147:
            // Raise Floor 512
            // 147 WR  EV_DoFloor(raiseFloor512)
            EV_DoFloor(line,raiseFloor512);
            break;

          case 148:
            // Raise Floor 24 and Change
            // 148 WR  EV_DoPlat(raiseAndChange,24)
            EV_DoPlat(line,raiseAndChange,24);
            break;

          case 149:
            // Raise Floor 32 and Change
            // 149 WR  EV_DoPlat(raiseAndChange,32)
            EV_DoPlat(line,raiseAndChange,32);
            break;

          case 150:
            // Start slow silent crusher
            // 150 WR  EV_DoCeiling(silentCrushAndRaise)
            EV_DoCeiling(line,silentCrushAndRaise);
            break;

          case 151:
            // RaiseCeilingLowerFloor
            // 151 WR  EV_DoCeiling(raiseToHighest),
            //         EV_DoFloor(lowerFloortoLowest)
            EV_DoCeiling( line, raiseToHighest );
            EV_DoFloor( line, lowerFloorToLowest );
            break;

          case 152:
            // Lower Ceiling to Floor
            // 152 WR  EV_DoCeiling(lowerToFloor)
            EV_DoCeiling( line, lowerToFloor );
            break;

            //jff 3/16/98 renumber 153->256
          case 256:
            // Build stairs, step 8
            // 256 WR EV_BuildStairs(build8)
            EV_BuildStairs(line,build8);
            break;

            //jff 3/16/98 renumber 154->257
          case 257:
            // Build stairs, step 16
            // 257 WR EV_BuildStairs(turbo16)
            EV_BuildStairs(line,turbo16);
            break;

          case 155:
            // Lower Pillar, Raise Donut
            // 155 WR  EV_DoDonut()
            EV_DoDonut(line);
            break;

          case 156:
            // Start lights strobing
            // 156 WR Lights EV_StartLightStrobing()
            EV_StartLightStrobing(line);
            break;

          case 157:
            // Lights to dimmest near
            // 157 WR Lights EV_TurnTagLightsOff()
            EV_TurnTagLightsOff(line);
            break;

          case 201:
            // Lower ceiling to lowest surrounding ceiling
            // 201 WR EV_DoCeiling(lowerToLowest)
            EV_DoCeiling(line,lowerToLowest);
            break;

          case 202:
            // Lower ceiling to highest surrounding floor
            // 202 WR EV_DoCeiling(lowerToMaxFloor)
            EV_DoCeiling(line,lowerToMaxFloor);
            break;

          case 208:
            // killough 2/16/98: WR silent teleporter (normal kind)
            map_format.ev_teleport(0, line->tag, line, side, thing, TELF_SILENT);
            break;

          case 212: //jff 3/14/98 create instant toggle floor type
            // Toggle floor between C and F instantly
            // 212 WR Instant Toggle Floor
            EV_DoPlat(line,toggleUpDn,0);
            break;

          //jff 3/16/98 renumber 216->154
          case 154: //jff 3/15/98 create texture change no motion type
            // Texture/Type Change Only (Trigger)
            // 154 WR Change Texture/Type Only
            EV_DoChange(line,trigChangeOnly,line->tag);
            break;

          case 240: //jff 3/15/98 create texture change no motion type
            // Texture/Type Change Only (Numeric)
            // 240 WR Change Texture/Type Only
            EV_DoChange(line,numChangeOnly,line->tag);
            break;

          case 220:
            // Lower floor to next lower neighbor
            // 220 WR Lower Floor Next Lower Neighbor
            EV_DoFloor(line,lowerFloorToNearest);
            break;

          case 228:
            // Raise elevator next floor
            // 228 WR Raise Elevator next floor
            EV_DoElevator(line,elevateUp);
            break;

          case 232:
            // Lower elevator next floor
            // 232 WR Lower Elevator next floor
            EV_DoElevator(line,elevateDown);
            break;

          case 236:
            // Elevator to current floor
            // 236 WR Elevator to current floor
            EV_DoElevator(line,elevateCurrent);
            break;

          case 244: //jff 3/6/98 make fit within DCK's 256 linedef types
            // killough 2/16/98: WR silent teleporter (linedef-linedef kind)
            EV_SilentLineTeleport(line, side, thing, line->tag, false);
            break;

          case 263: //jff 4/14/98 add silent line-line reversed
            EV_SilentLineTeleport(line, side, thing, line->tag, true);
            break;

          case 265: //jff 4/14/98 add monster-only silent line-line reversed
            if (!thing->player)
              EV_SilentLineTeleport(line, side, thing, line->tag, true);
            break;

          case 267: //jff 4/14/98 add monster-only silent line-line
            if (!thing->player)
              EV_SilentLineTeleport(line, side, thing, line->tag, false);
            break;

          case 269: //jff 4/14/98 add monster-only silent
            if (!thing->player)
              map_format.ev_teleport(0, line->tag, line, side, thing, TELF_SILENT);
            break;

            //jff 1/29/98 end of added WR linedef types
        }
      break;
  }
}

void P_CrossZDoomSpecialLine(line_t *line, int side, mobj_t *thing, dboolean bossaction)
{
  if (thing->player)
  {
    P_ActivateLine(line, thing, side, SPAC_CROSS);
  }
  else if (thing->flags2 & MF2_MCROSS)
  {
    P_ActivateLine(line, thing, side, SPAC_MCROSS);
  }
  else if (thing->flags2 & MF2_PCROSS)
  {
    P_ActivateLine(line, thing, side, SPAC_PCROSS);
  }
  else if (line->special == zl_teleport ||
           line->special == zl_teleport_no_fog ||
           line->special == zl_teleport_line)
  { // [RH] Just a little hack for BOOM compatibility
    P_ActivateLine(line, thing, side, SPAC_MCROSS);
  }
  else
  {
    P_ActivateLine(line, thing, side, SPAC_ANYCROSS);
  }
}

//
// P_ShootSpecialLine - Gun trigger special dispatcher
//
// Called when a thing shoots a special line with bullet, shell, saw, or fist.
//
// jff 02/12/98 all G1 lines were fixed to check the result from the EV_
// function before clearing the special. This avoids losing the function
// of the line, should the sector already be in motion when the line is
// impacted. Change is qualified by demo_compatibility.
//

void P_ShootCompatibleSpecialLine(mobj_t *thing, line_t *line)
{
  //jff 02/04/98 add check here for generalized linedef
  if (!demo_compatibility)
  {
    // pointer to line function is NULL by default, set non-null if
    // line special is gun triggered generalized linedef type
    int (*linefunc)(line_t *line)=NULL;

    // check each range of generalized linedefs
    if ((unsigned)line->special >= GenEnd)
    {
      // Out of range for GenFloors
    }
    else if ((unsigned)line->special >= GenFloorBase)
    {
      if (!thing->player)
        if ((line->special & FloorChange) || !(line->special & FloorModel))
          return;   // FloorModel is "Allow Monsters" if FloorChange is 0
      if (!line->tag) //jff 2/27/98 all gun generalized types require tag
        return;

      linefunc = EV_DoGenFloor;
    }
    else if ((unsigned)line->special >= GenCeilingBase)
    {
      if (!thing->player)
        if ((line->special & CeilingChange) || !(line->special & CeilingModel))
          return;   // CeilingModel is "Allow Monsters" if CeilingChange is 0
      if (!line->tag) //jff 2/27/98 all gun generalized types require tag
        return;
      linefunc = EV_DoGenCeiling;
    }
    else if ((unsigned)line->special >= GenDoorBase)
    {
      if (!thing->player)
      {
        if (!(line->special & DoorMonster))
          return;   // monsters disallowed from this door
        if (line->flags & ML_SECRET) // they can't open secret doors either
          return;
      }
      if (!line->tag) //jff 3/2/98 all gun generalized types require tag
        return;
      linefunc = EV_DoGenDoor;
    }
    else if ((unsigned)line->special >= GenLockedBase)
    {
      if (!thing->player)
        return;   // monsters disallowed from unlocking doors
      if (((line->special&TriggerType)==GunOnce) || ((line->special&TriggerType)==GunMany))
      { //jff 4/1/98 check for being a gun type before reporting door type
        if (!P_CanUnlockGenDoor(line,thing->player))
          return;
      }
      else
        return;
      if (!line->tag) //jff 2/27/98 all gun generalized types require tag
        return;

      linefunc = EV_DoGenLockedDoor;
    }
    else if ((unsigned)line->special >= GenLiftBase)
    {
      if (!thing->player)
        if (!(line->special & LiftMonster))
          return; // monsters disallowed
      linefunc = EV_DoGenLift;
    }
    else if ((unsigned)line->special >= GenStairsBase)
    {
      if (!thing->player)
        if (!(line->special & StairMonster))
          return; // monsters disallowed
      if (!line->tag) //jff 2/27/98 all gun generalized types require tag
        return;
      linefunc = EV_DoGenStairs;
    }
    else if ((unsigned)line->special >= GenCrusherBase)
    {
      if (!thing->player)
        if (!(line->special & StairMonster))
          return; // monsters disallowed
      if (!line->tag) //jff 2/27/98 all gun generalized types require tag
        return;
      linefunc = EV_DoGenCrusher;
    }

    if (linefunc)
      switch((line->special & TriggerType) >> TriggerTypeShift)
      {
        case GunOnce:
          if (linefunc(line))
            P_ChangeSwitchTexture(line,0);
          return;
        case GunMany:
          if (linefunc(line))
            P_ChangeSwitchTexture(line,1);
          return;
        default:  // if not a gun type, do nothing here
          return;
      }
  }

  // Impacts that other things can activate.
  if (!thing->player)
  {
    int ok = 0;
    switch(line->special)
    {
      case 46:
        // 46 GR Open door on impact weapon is monster activatable
        ok = 1;
        break;
    }
    if (!ok)
      return;
  }

  if (!P_CheckTag(line))  //jff 2/27/98 disallow zero tag on some types
    return;

  switch(line->special)
  {
    case 24:
      // 24 G1 raise floor to highest adjacent
      if (EV_DoFloor(line,raiseFloor) || demo_compatibility)
        P_ChangeSwitchTexture(line,0);
      break;

    case 46:
      // 46 GR open door, stay open
      EV_DoDoor(line,g_door_open);
      P_ChangeSwitchTexture(line,1);
      break;

    case 47:
      // 47 G1 raise floor to nearest and change texture and type
      if (EV_DoPlat(line,raiseToNearestAndChange,0) || demo_compatibility)
        P_ChangeSwitchTexture(line,0);
      break;

    //jff 1/30/98 added new gun linedefs here
    // killough 1/31/98: added demo_compatibility check, added inner switch

    default:
      if (!demo_compatibility)
        switch (line->special)
        {
          case 197:
            // Exit to next level
            // killough 10/98: prevent zombies from exiting levels
            if(thing->player && thing->player->health<=0 && !comp[comp_zombie])
              break;
            P_ChangeSwitchTexture(line,0);
            G_ExitLevel(0);
            break;

          case 198:
            // Exit to secret level
            // killough 10/98: prevent zombies from exiting levels
            if(thing->player && thing->player->health<=0 && !comp[comp_zombie])
              break;
            P_ChangeSwitchTexture(line,0);
            G_SecretExitLevel(0);
            break;
            //jff end addition of new gun linedefs
        }
      break;
  }
}

void P_ShootHexenSpecialLine(mobj_t *thing, line_t *line)
{
  P_ActivateLine(line, thing, 0, SPAC_IMPACT);
}

static void P_ApplySectorDamage(player_t *player, int damage, int leak)
{
  if (!player->powers[pw_ironfeet] || (leak && P_Random(pr_slimehurt) < leak))
    if (!(leveltime & 0x1f))
      P_DamageMobj(player->mo, NULL, NULL, damage);
}

static void P_ApplySectorDamageEndLevel(player_t *player)
{
  if (comp[comp_god])
    player->cheats &= ~CF_GODMODE;

  if (!(leveltime & 0x1f))
    P_DamageMobj(player->mo, NULL, NULL, 20);

  if (player->health <= 10)
    G_ExitLevel(0);
}

static void P_ApplyGeneralizedSectorDamage(player_t *player, int bits)
{
  switch (bits & 3)
  {
    case 0:
      break;
    case 1:
      P_ApplySectorDamage(player, 5, 0);
      break;
    case 2:
      P_ApplySectorDamage(player, 10, 0);
      break;
    case 3:
      P_ApplySectorDamage(player, 20, 5);
      break;
  }
}

void P_PlayerInCompatibleSector(player_t *player, sector_t *sector)
{
  //jff add if to handle old vs generalized types
  if (sector->special < 32) // regular sector specials
  {
    switch (sector->special)
    {
      case 5:
        P_ApplySectorDamage(player, 10, 0);
        break;
      case 7:
        P_ApplySectorDamage(player, 5, 0);
        break;
      case 16:
      case 4:
        P_ApplySectorDamage(player, 20, 5);
        break;
      case 9:
        P_CollectSecretVanilla(sector, player);
        break;
      case 11:
        P_ApplySectorDamageEndLevel(player);
        break;
      default:
        break;
    }
  }
  else //jff 3/14/98 handle extended sector damage
  {
    if (mbf21 && sector->special & DEATH_MASK)
    {
      int i;

      switch ((sector->special & DAMAGE_MASK) >> DAMAGE_SHIFT)
      {
        case 0:
          if (!player->powers[pw_invulnerability] && !player->powers[pw_ironfeet])
            P_DamageMobj(player->mo, NULL, NULL, 10000);
          break;
        case 1:
          P_DamageMobj(player->mo, NULL, NULL, 10000);
          break;
        case 2:
          for (i = 0; i < g_maxplayers; i++)
            if (playeringame[i])
              P_DamageMobj(players[i].mo, NULL, NULL, 10000);
          G_ExitLevel(0);
          break;
        case 3:
          for (i = 0; i < g_maxplayers; i++)
            if (playeringame[i])
              P_DamageMobj(players[i].mo, NULL, NULL, 10000);
          G_SecretExitLevel(0);
          break;
      }
    }
    else
    {
      P_ApplyGeneralizedSectorDamage(player, (sector->special & DAMAGE_MASK) >> DAMAGE_SHIFT);
    }
  }

  if (sector->flags & SECF_SECRET)
  {
    P_CollectSecretBoom(sector, player);
  }
}

void P_PlayerInZDoomSector(player_t *player, sector_t *sector)
{
  static const int heretic_carry[5] = {
    2048 * 5,
    2048 * 10,
    2048 * 25,
    2048 * 30,
    2048 * 35
  };

  static const int hexen_carry[3] = {
    2048 * 5,
    2048 * 10,
    2048 * 25
  };

  if (sector->damage.amount > 0)
  {
    if (sector->flags & SECF_ENDGODMODE)
    {
      player->cheats &= ~CF_GODMODE;
    }

    if (
      sector->flags & SECF_DMGUNBLOCKABLE ||
      !player->powers[pw_ironfeet] ||
      (sector->damage.leakrate && P_Random(pr_slimehurt) < sector->damage.leakrate)
    )
    {
      if (sector->flags & SECF_HAZARD)
      {
        player->hazardcount += sector->damage.amount;
        player->hazardinterval = sector->damage.interval;
      }
      else
      {
        if (leveltime % sector->damage.interval == 0)
        {
          P_DamageMobj(player->mo, NULL, NULL, sector->damage.amount);

          if (sector->flags & SECF_ENDLEVEL && player->health <= 10)
          {
            G_ExitLevel(0);
          }

          if (sector->flags & SECF_DMGTERRAINFX)
          {
            // MAP_FORMAT_TODO: damage special effects
          }
        }
      }
    }
  }
  else if (sector->damage.amount < 0)
  {
    if (leveltime % sector->damage.interval == 0)
    {
      P_GiveBody(player, -sector->damage.amount);
    }
  }

  switch (sector->special)
  {
    case zs_d_scroll_east_lava_damage:
      P_Thrust(player, 0, 2048 * 28);
      break;
    case zs_scroll_strife_current:
      {
        int anglespeed;
        fixed_t carryspeed;
        angle_t angle;

        anglespeed = sector->tag - 100;
        carryspeed = (anglespeed % 10) * 4096;
        angle = (anglespeed / 10) * ANG45;
        P_Thrust(player, angle, carryspeed);
      }
      break;
    case zs_carry_east5:
    case zs_carry_east10:
    case zs_carry_east25:
    case zs_carry_east30:
    case zs_carry_east35:
      P_Thrust(player, 0, heretic_carry[sector->special - zs_carry_east5]);
      break;
    case zs_carry_north5:
    case zs_carry_north10:
    case zs_carry_north25:
    case zs_carry_north30:
    case zs_carry_north35:
      P_Thrust(player, ANG90, heretic_carry[sector->special - zs_carry_north5]);
      break;
    case zs_carry_south5:
    case zs_carry_south10:
    case zs_carry_south25:
    case zs_carry_south30:
    case zs_carry_south35:
      P_Thrust(player, ANG270, heretic_carry[sector->special - zs_carry_south5]);
      break;
    case zs_carry_west5:
    case zs_carry_west10:
    case zs_carry_west25:
    case zs_carry_west30:
    case zs_carry_west35:
      P_Thrust(player, ANG180, heretic_carry[sector->special - zs_carry_west5]);
      break;
    case zs_scroll_north_slow:
    case zs_scroll_north_medium:
    case zs_scroll_north_fast:
      P_Thrust(player, ANG90, hexen_carry[sector->special - zs_scroll_north_slow]);
      break;
    case zs_scroll_east_slow:
    case zs_scroll_east_medium:
    case zs_scroll_east_fast:
      P_Thrust(player, 0, hexen_carry[sector->special - zs_scroll_east_slow]);
      break;
    case zs_scroll_south_slow:
    case zs_scroll_south_medium:
    case zs_scroll_south_fast:
      P_Thrust(player, ANG270, hexen_carry[sector->special - zs_scroll_south_slow]);
      break;
    case zs_scroll_west_slow:
    case zs_scroll_west_medium:
    case zs_scroll_west_fast:
      P_Thrust(player, ANG180, hexen_carry[sector->special - zs_scroll_west_slow]);
      break;
    case zs_scroll_northwest_slow:
    case zs_scroll_northwest_medium:
    case zs_scroll_northwest_fast:
      P_Thrust(player, ANG135, hexen_carry[sector->special - zs_scroll_northwest_slow]);
      break;
    case zs_scroll_northeast_slow:
    case zs_scroll_northeast_medium:
    case zs_scroll_northeast_fast:
      P_Thrust(player, ANG45, hexen_carry[sector->special - zs_scroll_northeast_slow]);
      break;
    case zs_scroll_southeast_slow:
    case zs_scroll_southeast_medium:
    case zs_scroll_southeast_fast:
      P_Thrust(player, ANG315, hexen_carry[sector->special - zs_scroll_southeast_slow]);
      break;
    case zs_scroll_southwest_slow:
    case zs_scroll_southwest_medium:
    case zs_scroll_southwest_fast:
      P_Thrust(player, ANG225, hexen_carry[sector->special - zs_scroll_southwest_slow]);
      break;
    default:
      break;
  }

  if (sector->flags & SECF_SECRET)
  {
    P_CollectSecretZDoom(sector, player);
  }
}

//
// P_PlayerInSpecialSector()
//
// Called every tick frame
//  that the player origin is in a special sector
//
// Changed to ignore sector types the engine does not recognize
//

void P_PlayerInSpecialSector (player_t* player)
{
  sector_t*   sector;

  sector = player->mo->subsector->sector;

  // Falling, not all the way down yet?
  // Sector specials don't apply in mid-air
  if (player->mo->z != sector->floorheight)
    return;

  map_format.player_in_special_sector(player, sector);
}

dboolean P_MobjInCompatibleSector(mobj_t *mobj)
{
  if (mbf21)
  {
    sector_t* sector = mobj->subsector->sector;

    if (
      sector->special & KILL_MONSTERS_MASK &&
      mobj->z == mobj->floorz &&
      mobj->player == NULL &&
      mobj->flags & MF_SHOOTABLE &&
      !(mobj->flags & MF_FLOAT)
    )
    {
      P_DamageMobj(mobj, NULL, NULL, 10000);

      // must have been removed
      if (mobj->thinker.function != P_MobjThinker)
        return true;
    }
  }

  return false;
}

dboolean P_MobjInHereticSector(mobj_t *mobj)
{
  return false;
}

dboolean P_MobjInHexenSector(mobj_t *mobj)
{
  return false;
}

dboolean P_MobjInZDoomSector(mobj_t *mobj)
{
  return false;
}

//
// P_UpdateSpecials()
//
// Check level timer, frag counter,
// animate flats, scroll walls,
// change button textures
//
// Reads and modifies globals:
//  levelTimer, levelTimeCount,
//  levelFragLimit, levelFragLimitCount
//

static dboolean  levelTimer;
static int      levelTimeCount;
dboolean         levelFragLimit;      // Ty 03/18/98 Added -frags support
int             levelFragLimitCount; // Ty 03/18/98 Added -frags support

void P_UpdateSpecials (void)
{
  anim_t*     anim;
  int         pic;
  int         i;

  // hexen_note: possibly not needed?
  if (!hexen)
  {
    // Downcount level timer, exit level if elapsed
    if (levelTimer == true)
    {
      levelTimeCount--;
      if (!levelTimeCount)
        G_ExitLevel(0);
    }

    // Check frag counters, if frag limit reached, exit level // Ty 03/18/98
    //  Seems like the total frags should be kept in a simple
    //  array somewhere, but until they are...
    if (levelFragLimit == true)  // we used -frags so compare count
    {
      int k,m,fragcount,exitflag=false;
      for (k = 0; k < g_maxplayers; k++)
      {
        if (!playeringame[k]) continue;
        fragcount = 0;
        for (m = 0; m < g_maxplayers; m++)
        {
          if (!playeringame[m]) continue;
            fragcount += (m!=k)?  players[k].frags[m] : -players[k].frags[m];
        }
        if (fragcount >= levelFragLimitCount) exitflag = true;
        if (exitflag == true) break; // skip out of the loop--we're done
      }
      if (exitflag == true)
        G_ExitLevel(0);
    }
  }

  // MAP_FORMAT_TODO: needs investigation
  if (!map_format.animdefs)
  {
    // Animate flats and textures globally
    for (anim = anims ; anim < lastanim ; anim++)
    {
      for (i = 0; i < anim->numpics; ++i)
      {
        pic = anim->basepic + ((leveltime / anim->speed + i) % anim->numpics);
        if (anim->istexture)
          texturetranslation[anim->basepic + i] = pic;
        else
          flattranslation[anim->basepic + i] = pic;
      }
    }
  }

  // Check buttons (retriggerable switches) and change texture on timeout
  for (i = 0; i < MAXBUTTONS; i++)
    if (buttonlist[i].btimer)
    {
      buttonlist[i].btimer--;
      if (!buttonlist[i].btimer)
      {
        switch(buttonlist[i].where)
        {
          case top:
            sides[buttonlist[i].line->sidenum[0]].toptexture =
              buttonlist[i].btexture;
            break;

          case middle:
            sides[buttonlist[i].line->sidenum[0]].midtexture =
              buttonlist[i].btexture;
            break;

          case bottom:
            sides[buttonlist[i].line->sidenum[0]].bottomtexture =
              buttonlist[i].btexture;
            break;
        }
        if (!hexen)
        {
          /* don't take the address of the switch's sound origin,
           * unless in a compatibility mode. */
          degenmobj_t *so = buttonlist[i].soundorg;
          if (comp[comp_sound] || compatibility_level < prboom_6_compatibility)
            /* since the buttonlist array is usually zeroed out,
             * button popouts generally appear to come from (0,0) */
            so = (degenmobj_t *) &buttonlist[i].soundorg;
          S_StartLineSound(buttonlist[i].line, so, g_sfx_swtchn);
        }
        memset(&buttonlist[i],0,sizeof(button_t));
      }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Sector and Line special thinker spawning at level startup
//
//////////////////////////////////////////////////////////////////////

//
// Add_Scroller()
//
// Add a generalized scroller to the thinker list.
//
// type: the enumerated type of scrolling: floor, ceiling, floor carrier,
//   wall, floor carrier & scroller
//
// (dx,dy): the direction and speed of the scrolling or its acceleration
//
// control: the sector whose heights control this scroller's effect
//   remotely, or -1 if no control sector
//
// affectee: the index of the affected object (sector or sidedef)
//
// accel: non-zero if this is an accelerative effect
//

static void Add_Scroller(int type, fixed_t dx, fixed_t dy,
                         int control, int affectee, int accel, int flags)
{
  scroll_t *s = Z_MallocLevel(sizeof *s);
  s->thinker.function = T_Scroll;
  s->type = type;
  s->dx = dx;
  s->dy = dy;
  s->accel = accel;
  s->flags = flags;
  s->vdx = s->vdy = 0;
  if ((s->control = control) != -1)
    s->last_height =
      sectors[control].floorheight + sectors[control].ceilingheight;
  s->affectee = affectee;
  P_AddThinker(&s->thinker);
}

//
// P_SpawnSpecials
// After the map has been loaded,
//  scan for specials that spawn thinkers
//

void P_SpawnCompatibleSectorSpecial(sector_t *sector, int i)
{
  if (sector->special & SECRET_MASK) //jff 3/15/98 count extended
    P_AddSectorSecret(sector);

  if (sector->special & FRICTION_MASK)
    sector->flags |= SECF_FRICTION;

  if (sector->special & PUSH_MASK)
    sector->flags |= SECF_PUSH;

  switch ((demo_compatibility && !prboom_comp[PC_TRUNCATED_SECTOR_SPECIALS].state) ?
          sector->special : sector->special & 31)
  {
    case 1:
      // random off
      P_SpawnLightFlash(sector);
      break;

    case 2:
      // strobe fast
      P_SpawnStrobeFlash(sector, FASTDARK, 0);
      break;

    case 3:
      // strobe slow
      P_SpawnStrobeFlash(sector, SLOWDARK, 0);
      break;

    case 4:
      // strobe fast/death slime
      P_SpawnStrobeFlash(sector, FASTDARK, 0);
      if (heretic)
        sector->special = 4;
      else
        sector->special |= 3 << DAMAGE_SHIFT; //jff 3/14/98 put damage bits in
      break;

    case 8:
      // glowing light
      P_SpawnGlowingLight(sector);
      break;
    case 9:
      // secret sector
      if (sector->special < 32) //jff 3/14/98 bits don't count unless not
        P_AddSectorSecret(sector);
      break;

    case 10:
      // door close in 30 seconds
      P_SpawnDoorCloseIn30(sector);
      break;

    case 12:
      // sync strobe slow
      P_SpawnStrobeFlash(sector, SLOWDARK, 1);
      break;

    case 13:
      // sync strobe fast
      P_SpawnStrobeFlash(sector, FASTDARK, 1);
      break;

    case 14:
      // door raise in 5 minutes
      P_SpawnDoorRaiseIn5Mins(sector, i);
      break;

    case 17:
      // fire flickering
      P_SpawnFireFlicker(sector);
      break;
  }
}

void P_SpawnZDoomLights(sector_t *sector)
{
  switch (sector->special)
  {
    case zs_light_phased:
      P_SpawnPhasedLight(sector, 80, -1);
      break;
    case zs_light_sequence_start:
      P_SpawnLightSequence(sector, 1);
      break;
    case zs_d_light_flicker:
      P_SpawnLightFlash(sector);
      break;
    case zs_d_light_strobe_fast:
      P_SpawnStrobeFlash(sector, FASTDARK, 0);
      break;
    case zs_d_light_strobe_slow:
      P_SpawnStrobeFlash(sector, SLOWDARK, 0);
      break;
    case zs_d_light_strobe_hurt:
      P_SpawnStrobeFlash(sector, FASTDARK, 0);
      sector->special |= zs_d_light_strobe_hurt;
      break;
    case zs_d_light_glow:
      P_SpawnGlowingLight(sector);
      break;
    case zs_d_light_strobe_slow_sync:
      P_SpawnStrobeFlash(sector, SLOWDARK, 1);
      break;
    case zs_d_light_strobe_fast_sync:
      P_SpawnStrobeFlash(sector, FASTDARK, 1);
      break;
    case zs_d_light_fire_flicker:
      P_SpawnFireFlicker(sector);
      break;
    case zs_d_scroll_east_lava_damage:
      P_SpawnStrobeFlash(sector, FASTDARK, 0);
      sector->special |= zs_d_scroll_east_lava_damage;
      break;
    case zs_s_light_strobe_hurt:
      P_SpawnStrobeFlash(sector, FASTDARK, 0);
      sector->special |= zs_s_light_strobe_hurt;
      break;
    default:
      break;
  }
}

void P_SetupSectorDamage(sector_t *sector, short amount,
                         byte interval, byte leakrate, unsigned int flags)
{
  // Only set if damage is not yet initialized.
  if (sector->damage.amount)
    return;

  sector->damage.amount = amount;
  sector->damage.interval = interval;
  sector->damage.leakrate = leakrate;
  sector->flags = (sector->flags & ~SECF_DAMAGEFLAGS) | (flags & SECF_DAMAGEFLAGS);
}

static void P_SpawnZDoomGeneralizedSpecials(sector_t *sector)
{
  int damage_bits = (sector->special & ZDOOM_DAMAGE_MASK) >> 8;

  switch (damage_bits & 3)
  {
    case 0:
      break;
    case 1:
      P_SetupSectorDamage(sector, 5, 32, 0, 0);
      break;
    case 2:
      P_SetupSectorDamage(sector, 10, 32, 0, 0);
      break;
    case 3:
      P_SetupSectorDamage(sector, 20, 32, 5, 0);
      break;
  }

  if (sector->special & ZDOOM_SECRET_MASK)
    P_AddSectorSecret(sector);

  if (sector->special & ZDOOM_FRICTION_MASK)
    sector->flags |= SECF_FRICTION;

  if (sector->special & ZDOOM_PUSH_MASK)
    sector->flags |= SECF_PUSH;
}

void P_SpawnZDoomSectorSpecial(sector_t *sector, int i)
{
  P_SpawnZDoomGeneralizedSpecials(sector);

  sector->special &= 0xff;

  P_SpawnZDoomLights(sector);

  switch (sector->special)
  {
    case zs_d_scroll_east_lava_damage:
      Add_Scroller(sc_floor, -4, 0, -1, sector - sectors, 0, 0);
      P_SetupSectorDamage(sector, 5, 32, 0, SECF_DMGTERRAINFX | SECF_DMGUNBLOCKABLE);
      break;
    case zs_s_light_strobe_hurt:
    case zs_d_damage_nukage:
      P_SetupSectorDamage(sector, 5, 32, 0, 0);
      sector->special = 0;
      break;
    case zs_d_damage_hellslime:
      P_SetupSectorDamage(sector, 10, 32, 0, 0);
      sector->special = 0;
      break;
    case zs_d_light_strobe_hurt:
    case zs_d_damage_super_hellslime:
      P_SetupSectorDamage(sector, 20, 32, 5, 0);
      sector->special = 0;
      break;
    case zs_d_damage_end:
      P_SetupSectorDamage(sector, 20, 32, 0, SECF_ENDGODMODE | SECF_ENDLEVEL | SECF_DMGUNBLOCKABLE);
      sector->special = 0;
      break;
    case zs_damage_instant_death:
      P_SetupSectorDamage(sector, 10000, 1, 0, SECF_DMGUNBLOCKABLE);
      sector->special = 0;
      break;
    case zs_h_damage_sludge:
      P_SetupSectorDamage(sector, 4, 32, 0, 0);
      sector->special = 0;
      break;
    case zs_d_damage_lava_wimpy:
      P_SetupSectorDamage(sector, 5, 32, 0, SECF_DMGTERRAINFX | SECF_DMGUNBLOCKABLE);
      sector->special = 0;
      break;
    case zs_d_damage_lava_hefty:
      P_SetupSectorDamage(sector, 8, 32, 0, SECF_DMGTERRAINFX | SECF_DMGUNBLOCKABLE);
      sector->special = 0;
      break;
    case zs_s_damage_hellslime:
      P_SetupSectorDamage(sector, 2, 32, 0, SECF_HAZARD);
      sector->special = 0;
      break;
    case zs_s_damage_super_hellslime:
      P_SetupSectorDamage(sector, 4, 32, 0, SECF_HAZARD);
      sector->special = 0;
      break;
    case zs_sector_heal:
      P_SetupSectorDamage(sector, -1, 32, 0, 0);
      sector->special = 0;
      break;
    case zs_d_sector_door_close_in_30:
      P_SpawnDoorCloseIn30(sector);
      sector->special = 0;
      break;
    case zs_d_sector_door_raise_in_5_mins:
      P_SpawnDoorRaiseIn5Mins(sector, i);
      sector->special = 0;
      break;
    case zs_d_friction_low:
      sector->friction = FRICTION_LOW;
      sector->movefactor = 0x269;
      sector->flags |= SECF_FRICTION;
      break;
    case zs_sector_hidden:
      sector->flags |= SECF_HIDDEN;
      sector->special = 0;
      break;
    case zs_sky2:
      // sector->sky = PL_SKYFLAT;
      sector->special = 0;
      break;
    default:
      if (sector->special >= zs_scroll_north_slow &&
          sector->special <= zs_scroll_southwest_fast)
      { // Hexen scroll special
        static const fixed_t hexenScrollies[24][2] =
        {
          {  0,  1 }, {  0,  2 }, {  0,  4 },
          { -1,  0 }, { -2,  0 }, { -4,  0 },
          {  0, -1 }, {  0, -2 }, {  0, -4 },
          {  1,  0 }, {  2,  0 }, {  4,  0 },
          {  1,  1 }, {  2,  2 }, {  4,  4 },
          { -1,  1 }, { -2,  2 }, { -4,  4 },
          { -1, -1 }, { -2, -2 }, { -4, -4 },
          {  1, -1 }, {  2, -2 }, {  4, -4 }
        };

        int i;
        fixed_t dx, dy;

        i = sector->special - zs_scroll_north_slow;
        dx = FixedDiv(hexenScrollies[i][0] << FRACBITS, 2);
        dy = FixedDiv(hexenScrollies[i][1] << FRACBITS, 2);
        Add_Scroller(sc_floor, dx, dy, -1, sector - sectors, 0, 0);
      }
      else if (sector->special >= zs_carry_east5 &&
            sector->special <= zs_carry_east35)
      { // Heretic scroll special
        // Only east scrollers also scroll the texture
        fixed_t dx = FixedDiv((1 << (sector->special - zs_carry_east5)) << FRACBITS, 2);
        Add_Scroller(sc_floor, dx, 0, -1, sector - sectors, 0, 0);
      }
      break;
  }
}

static void P_SpawnVanillaExtras(void)
{
  int i;

  // allow MBF sky transfers in all complevels
  if (!heretic)
    for (i = 0; i < numlines; ++i)
      switch (lines[i].special)
      {
        const int *id_p;

        case 271:   // Regular sky
        case 272:   // Same, only flipped
          FIND_SECTORS(id_p, lines[i].tag)
            sectors[*id_p].sky = i | PL_SKYFLAT;
        break;
      }
}

void P_SpawnCompatibleExtra(line_t *l, int i)
{
  const int *id_p;
  int sec;

  switch (l->special)
  {
    // killough 3/7/98:
    // support for drawn heights coming from different sector
    case 242:
      sec = sides[*l->sidenum].sector->iSectorID;
      FIND_SECTORS(id_p, lines[i].tag)
        sectors[*id_p].heightsec = sec;
      break;

    // killough 3/16/98: Add support for setting
    // floor lighting independently (e.g. lava)
    case 213:
      sec = sides[*l->sidenum].sector->iSectorID;
      FIND_SECTORS(id_p, lines[i].tag)
        sectors[*id_p].floorlightsec = sec;
      break;

    // killough 4/11/98: Add support for setting
    // ceiling lighting independently
    case 261:
      sec = sides[*l->sidenum].sector->iSectorID;
      FIND_SECTORS(id_p, lines[i].tag)
        sectors[*id_p].ceilinglightsec = sec;
      break;

      // killough 10/98:
      //
      // Support for sky textures being transferred from sidedefs.
      // Allows scrolling and other effects (but if scrolling is
      // used, then the same sector tag needs to be used for the
      // sky sector, the sky-transfer linedef, and the scroll-effect
      // linedef). Still requires user to use F_SKY1 for the floor
      // or ceiling texture, to distinguish floor and ceiling sky.

    case 271:   // Regular sky
    case 272:   // Same, only flipped
      FIND_SECTORS(id_p, lines[i].tag)
        sectors[*id_p].sky = i | PL_SKYFLAT;
      break;
  }
}

void P_SpawnZDoomExtra(line_t *l, int i)
{
  const int *id_p;
  int sec;

  switch (l->special)
  {
    // killough 3/7/98:
    // support for drawn heights coming from different sector
    case zl_transfer_heights:
      sec = sides[*l->sidenum].sector->iSectorID;
      FIND_SECTORS(id_p, l->special_args[0])
        sectors[*id_p].heightsec = sec;
      break;

    // killough 3/16/98: Add support for setting
    // floor lighting independently (e.g. lava)
    case zl_transfer_floor_light:
      sec = sides[*l->sidenum].sector->iSectorID;
      FIND_SECTORS(id_p, l->special_args[0])
        sectors[*id_p].floorlightsec = sec;
      break;

    // killough 4/11/98: Add support for setting
    // ceiling lighting independently
    case zl_transfer_ceiling_light:
      sec = sides[*l->sidenum].sector->iSectorID;
      FIND_SECTORS(id_p, l->special_args[0])
        sectors[*id_p].ceilinglightsec = sec;
      break;

    // [RH] ZDoom Static_Init settings
    case zl_static_init:
      switch (l->special_args[1])
      {
        case zi_init_gravity:
        {
          fixed_t grav = FixedDiv(P_AproxDistance(l->dx, l->dy), 100 * FRACUNIT);
          sec = sides[*l->sidenum].sector->iSectorID;
          FIND_SECTORS(id_p, l->special_args[0])
            sectors[*id_p].gravity = grav;
        }
        break;

        case zi_init_damage:
        {
          damage_t damage;
          unsigned int flags = 0;

          damage.amount = P_AproxDistance(l->dx, l->dy) >> FRACBITS;
          if (damage.amount < 20)
          {
            damage.leakrate = 0;
            damage.interval = 32;
          }
          else if (damage.amount < 50)
          {
            damage.leakrate = 5;
            damage.interval = 32;
          }
          else
          {
            flags |= SECF_DMGUNBLOCKABLE;
            damage.leakrate = 0;
            damage.interval = 1;
          }

          sec = sides[*l->sidenum].sector->iSectorID;
          FIND_SECTORS(id_p, l->special_args[0])
          {
            sectors[*id_p].damage = damage;
            sectors[*id_p].flags |= flags;
          }
        }
        break;

        // killough 10/98:
        //
        // Support for sky textures being transferred from sidedefs.
        // Allows scrolling and other effects (but if scrolling is
        // used, then the same sector tag needs to be used for the
        // sky sector, the sky-transfer linedef, and the scroll-effect
        // linedef). Still requires user to use F_SKY1 for the floor
        // or ceiling texture, to distinguish floor and ceiling sky.
        case zi_init_transfer_sky:
          FIND_SECTORS(id_p, l->special_args[0])
            sectors[*id_p].sky = i | PL_SKYFLAT;
          break;
      }
      break;
  }
}

static void P_SpawnExtras(void)
{
  int i;
  line_t *l;

  for (i = 0, l = lines; i < numlines; i++, l++)
    map_format.spawn_extra(l, i);
}

static void P_EvaluateDeathmatchParams(void)
{
  dsda_arg_t *arg;

  levelTimer = false;

  arg = dsda_Arg(dsda_arg_timer);
  if (arg->found && deathmatch)
  {
    levelTimer = true;
    levelTimeCount = arg->value.v_int * 60 * TICRATE;
  }

  levelFragLimit = false;

  arg = dsda_Arg(dsda_arg_frags);
  if (arg->found && deathmatch)
  {
    levelFragLimit = true;
    levelFragLimitCount = arg->value.v_int;
  }
}

static void P_InitSectorSpecials(void)
{
  int i;
  sector_t* sector;

  sector = sectors;
  for (i = 0; i < numsectors; i++, sector++)
    if (sector->special)
      map_format.init_sector_special(sector, i);
}

static void P_InitButtons(void)
{
  int i;

  for (i = 0;i < MAXBUTTONS;i++)
    memset(&buttonlist[i],0,sizeof(button_t));
}

static void Hexen_P_SpawnSpecials(void);

// Parses command line parameters.
void P_SpawnSpecials (void)
{
  if (hexen) return Hexen_P_SpawnSpecials();

  P_EvaluateDeathmatchParams();

  P_InitSectorSpecials();

  if (heretic) P_SpawnLineSpecials();

  P_RemoveAllActiveCeilings();  // jff 2/22/98 use killough's scheme
  P_RemoveAllActivePlats();     // killough

  P_InitButtons();

  P_SpawnScrollers(); // killough 3/7/98: Add generalized scrollers

  if (demo_compatibility) return P_SpawnVanillaExtras();

  P_SpawnFriction();  // phares 3/12/98: New friction model using linedefs
  P_SpawnPushers();   // phares 3/20/98: New pusher model using linedefs
  P_SpawnExtras();

  // MAP_FORMAT_TODO: Start "Open" scripts
}

// killough 2/28/98:
//
// This function, with the help of r_plane.c and r_bsp.c, supports generalized
// scrolling floors and walls, with optional mobj-carrying properties, e.g.
// conveyor belts, rivers, etc. A linedef with a special type affects all
// tagged sectors the same way, by creating scrolling and/or object-carrying
// properties. Multiple linedefs may be used on the same sector and are
// cumulative, although the special case of scrolling a floor and carrying
// things on it, requires only one linedef. The linedef's direction determines
// the scrolling direction, and the linedef's length determines the scrolling
// speed. This was designed so that an edge around the sector could be used to
// control the direction of the sector's scrolling, which is usually what is
// desired.
//
// Process the active scrollers.
//
// This is the main scrolling code
// killough 3/7/98

void T_Scroll(scroll_t *s)
{
  fixed_t dx = s->dx, dy = s->dy;

  if (s->control != -1)
    {   // compute scroll amounts based on a sector's height changes
      fixed_t height = sectors[s->control].floorheight +
        sectors[s->control].ceilingheight;
      fixed_t delta = height - s->last_height;
      s->last_height = height;
      dx = FixedMul(dx, delta);
      dy = FixedMul(dy, delta);
    }

  // killough 3/14/98: Add acceleration
  if (s->accel)
    {
      s->vdx = dx += s->vdx;
      s->vdy = dy += s->vdy;
    }

  if (!(dx | dy))                   // no-op if both (x,y) offsets 0
    return;

  switch (s->type)
    {
      side_t *side;
      sector_t *sec;
      fixed_t height, waterheight;  // killough 4/4/98: add waterheight
      msecnode_t *node;
      mobj_t *thing;

    case sc_side:                   // killough 3/7/98: Scroll wall texture
        side = sides + s->affectee;
        if (!s->flags)
        {
          side->textureoffset += dx;
          side->rowoffset += dy;
        }
        else
        {
          if (s->flags & SCROLL_TOP)
          {
            side->textureoffset_top += dx;
            side->rowoffset_top += dy;
          }

          if (s->flags & SCROLL_MID)
          {
            side->textureoffset_mid += dx;
            side->rowoffset_mid += dy;
          }

          if (s->flags & SCROLL_BOTTOM)
          {
            side->textureoffset_bottom += dx;
            side->rowoffset_bottom += dy;
          }
        }
        break;

    case sc_floor:                  // killough 3/7/98: Scroll floor texture
        sec = sectors + s->affectee;
        sec->floor_xoffs += dx;
        sec->floor_yoffs += dy;
        break;

    case sc_ceiling:               // killough 3/7/98: Scroll ceiling texture
        sec = sectors + s->affectee;
        sec->ceiling_xoffs += dx;
        sec->ceiling_yoffs += dy;
        break;

    case sc_carry:

      // killough 3/7/98: Carry things on floor
      // killough 3/20/98: use new sector list which reflects true members
      // killough 3/27/98: fix carrier bug
      // killough 4/4/98: Underwater, carry things even w/o gravity

      sec = sectors + s->affectee;
      height = sec->floorheight;
      waterheight = sec->heightsec != -1 &&
        sectors[sec->heightsec].floorheight > height ?
        sectors[sec->heightsec].floorheight : INT_MIN;

      for (node = sec->touching_thinglist; node; node = node->m_snext)
        if (!((thing = node->m_thing)->flags & MF_NOCLIP) &&
            (!(thing->flags & MF_NOGRAVITY || thing->z > height) ||
             thing->z < waterheight))
          {
            // Move objects only if on floor or underwater,
            // non-floating, and clipped.
            thing->momx += dx;
            thing->momy += dy;
            thing->intflags |= MIF_SCROLLING;
          }
      break;

    case sc_carry_ceiling:       // to be added later
      break;
    }
}

// Adds wall scroller. Scroll amount is rotated with respect to wall's
// linedef first, so that scrolling towards the wall in a perpendicular
// direction is translated into vertical motion, while scrolling along
// the wall in a parallel direction is translated into horizontal motion.
//
// killough 5/25/98: cleaned up arithmetic to avoid drift due to roundoff
//
// killough 10/98:
// fix scrolling aliasing problems, caused by long linedefs causing overflowing

static void Add_WallScroller(fixed_t dx, fixed_t dy, const line_t *l,
                             int control, int accel)
{
  fixed_t x = D_abs(l->dx), y = D_abs(l->dy), d;
  if (y > x)
    d = x, x = y, y = d;
  d = FixedDiv(x, finesine[(tantoangle[FixedDiv(y,x) >> DBITS] + ANG90)
                          >> ANGLETOFINESHIFT]);

  // CPhipps - Import scroller calc overflow fix, compatibility optioned
  if (compatibility_level >= lxdoom_1_compatibility) {
    x = (fixed_t)(((int64_t)dy * -(int64_t)l->dy - (int64_t)dx * (int64_t)l->dx) / (int64_t)d);  // killough 10/98:
    y = (fixed_t)(((int64_t)dy * (int64_t)l->dx - (int64_t)dx * (int64_t)l->dy) / (int64_t)d);   // Use long long arithmetic
  } else {
    x = -FixedDiv(FixedMul(dy, l->dy) + FixedMul(dx, l->dx), d);
    y = -FixedDiv(FixedMul(dx, l->dy) - FixedMul(dy, l->dx), d);
  }
  Add_Scroller(sc_side, x, y, control, *l->sidenum, accel, 0);
}

// Amount (dx,dy) vector linedef is shifted right to get scroll amount
#define SCROLL_SHIFT 5

// Factor to scale scrolling effect into mobj-carrying properties = 3/32.
// (This is so scrolling floors and objects on them can move at same speed.)
#define CARRYFACTOR ((fixed_t)(FRACUNIT*.09375))

void P_SpawnCompatibleScroller(line_t *l, int i)
{
  fixed_t dx = l->dx >> SCROLL_SHIFT;  // direction and speed of scrolling
  fixed_t dy = l->dy >> SCROLL_SHIFT;
  int control = -1, accel = 0;         // no control sector or acceleration
  int special = l->special;

  if (demo_compatibility && special != 48) return; //e6y

  // killough 3/7/98: Types 245-249 are same as 250-254 except that the
  // first side's sector's heights cause scrolling when they change, and
  // this linedef controls the direction and speed of the scrolling. The
  // most complicated linedef since donuts, but powerful :)
  //
  // killough 3/15/98: Add acceleration. Types 214-218 are the same but
  // are accelerative.

  if (special >= 245 && special <= 249)         // displacement scrollers
  {
    special += 250 - 245;
    control = sides[*l->sidenum].sector->iSectorID;
  }
  else if (special >= 214 && special <= 218)    // accelerative scrollers
  {
    accel = 1;
    special += 250 - 214;
    control = sides[*l->sidenum].sector->iSectorID;
  }

  switch (special)
  {
    int side;
    const int *id_p;

    case 250:   // scroll effect ceiling
      FIND_SECTORS(id_p, l->tag)
        Add_Scroller(sc_ceiling, -dx, dy, control, *id_p, accel, 0);
      break;

    case 251:   // scroll effect floor
    case 253:   // scroll and carry objects on floor
      FIND_SECTORS(id_p, l->tag)
        Add_Scroller(sc_floor, -dx, dy, control, *id_p, accel, 0);
      if (special != 253)
        break;
      // fallthrough

    case 252: // carry objects on floor
      dx = FixedMul(dx, CARRYFACTOR);
      dy = FixedMul(dy, CARRYFACTOR);
      FIND_SECTORS(id_p, l->tag)
        Add_Scroller(sc_carry, dx, dy, control, *id_p, accel, 0);
      break;

      // killough 3/1/98: scroll wall according to linedef
      // (same direction and speed as scrolling floors)
    case 254:
      for (id_p = dsda_FindLinesFromID(l->tag); *id_p >= 0; id_p++)
        if (*id_p != i)
          Add_WallScroller(dx, dy, lines + *id_p, control, accel);
      break;

    case 255:    // killough 3/2/98: scroll according to sidedef offsets
      side = lines[i].sidenum[0];
      Add_Scroller(sc_side, -sides[side].textureoffset,
                   sides[side].rowoffset, -1, side, accel, 0);
      break;

    case 1024: // special 255 with tag control
    case 1025:
    case 1026:
      if (l->tag == 0)
        I_Error("Line %d is missing a tag!", i);

      if (special > 1024)
        control = sides[*l->sidenum].sector->iSectorID;

      if (special == 1026)
        accel = 1;

      side = lines[i].sidenum[0];
      dx = -sides[side].textureoffset / 8;
      dy = sides[side].rowoffset / 8;
      for (id_p = dsda_FindLinesFromID(l->tag); *id_p >= 0; id_p++)
        if (*id_p != i)
          Add_Scroller(sc_side, dx, dy, control, lines[*id_p].sidenum[0], accel, 0);

      break;

    case 48:                  // scroll first side
      Add_Scroller(sc_side,  FRACUNIT, 0, -1, lines[i].sidenum[0], accel, 0);
      break;

    case 85:                  // jff 1/30/98 2-way scroll
      Add_Scroller(sc_side, -FRACUNIT, 0, -1, lines[i].sidenum[0], accel, 0);
      break;
  }
}

static int copyscroller_count = 0;
static int copyscroller_max = 0;
static line_t **copyscrollers;

static void P_AddCopyScroller(line_t *l)
{
  while (copyscroller_count >= copyscroller_max)
  {
    copyscroller_max = copyscroller_max ? copyscroller_max * 2 : 8;
    copyscrollers = Z_Realloc(copyscrollers, copyscroller_max * sizeof(*copyscrollers));
  }

  copyscrollers[copyscroller_count++] = l;
}

static void P_InitCopyScrollers(void)
{
  int i;
  line_t *l;

  if (!map_format.zdoom) return;

  for (i = 0, l = lines; i < numlines; i++, l++)
    if (l->special == zl_sector_copy_scroller)
    {
      // don't allow copying the scroller if the sector has the same tag
      //   as it would just duplicate it.
      if (l->frontsector->tag == l->special_args[0])
        P_AddCopyScroller(l);

      l->special = 0;
    }
}

static void P_FreeCopyScrollers(void)
{
  if (copyscrollers)
  {
    copyscroller_count = 0;
    copyscroller_max = 0;
    Z_Free(copyscrollers);
  }
}

void P_SpawnZDoomScroller(line_t *l, int i)
{
  fixed_t dx = 0;               // direction and speed of scrolling
  fixed_t dy = 0;
  int control = -1, accel = 0;  // no control sector or acceleration
  int special = l->special;

  if (special == zl_scroll_ceiling ||
      special == zl_scroll_floor   ||
      special == zl_scroll_texture_model)
  {
    if (l->special_args[1] & 3)
    {
      // if 1, then displacement
      // if 2, then accelerative (also if 3)
      control = sides[*l->sidenum].sector->iSectorID;
      if (l->special_args[1] & 2)
        accel = 1;
    }

    if (special == zl_scroll_texture_model || l->special_args[1] & 4)
    {
      // The line housing the special controls the
      // direction and speed of scrolling.
      dx = l->dx >> SCROLL_SHIFT;
      dy = l->dy >> SCROLL_SHIFT;
    }
    else
    {
      // The speed and direction are parameters to the special.
      dx = (fixed_t) (l->special_args[3] - 128) * FRACUNIT / 32;
      dy = (fixed_t) (l->special_args[4] - 128) * FRACUNIT / 32;
    }
  }

  switch (special)
  {
    int j;
    const int *id_p;

    case zl_scroll_ceiling:
      FIND_SECTORS(id_p, l->special_args[0])
        Add_Scroller(sc_ceiling, -dx, dy, control, *id_p, accel, 0);

      for (j = 0; j < copyscroller_count; ++j)
      {
        line_t *cs = copyscrollers[j];

        if (cs->special_args[0] == l->special_args[0] && cs->special_args[1] & 1)
          Add_Scroller(sc_ceiling, -dx, dy, control, cs->frontsector->iSectorID, accel, 0);
      }

      l->special = 0;
      break;
    case zl_scroll_floor:
      if (l->special_args[2] != 1)
      { // scroll the floor texture
        FIND_SECTORS(id_p, l->special_args[0])
          Add_Scroller(sc_floor, -dx, dy, control, *id_p, accel, 0);

        for (j = 0; j < copyscroller_count; ++j)
        {
          line_t *cs = copyscrollers[j];

          if (cs->special_args[0] == l->special_args[0] && cs->special_args[1] & 2)
            Add_Scroller(sc_floor, -dx, dy, control, cs->frontsector->iSectorID, accel, 0);
        }
      }

      if (l->special_args[2] > 0)
      { // carry objects on the floor
        dx = FixedMul(dx, CARRYFACTOR);
        dy = FixedMul(dy, CARRYFACTOR);
        FIND_SECTORS(id_p, l->special_args[0])
          Add_Scroller(sc_carry, dx, dy, control, *id_p, accel, 0);

        for (j = 0; j < copyscroller_count; ++j)
        {
          line_t *cs = copyscrollers[j];

          if (cs->special_args[0] == l->special_args[0] && cs->special_args[1] & 4)
            Add_Scroller(sc_carry, dx, dy, control, cs->frontsector->iSectorID, accel, 0);
        }
      }

      l->special = 0;
      break;
    case zl_scroll_texture_model:
      // killough 3/1/98: scroll wall according to linedef
      // (same direction and speed as scrolling floors)
      for (id_p = dsda_FindLinesFromID(l->special_args[0]); *id_p >= 0; id_p++)
        if (*id_p != i)
          Add_WallScroller(dx, dy, lines + *id_p, control, accel);

      l->special = 0;
      break;
    case zl_scroll_texture_offsets:
      // killough 3/2/98: scroll according to sidedef offsets
      j = lines[i].sidenum[0];
      Add_Scroller(sc_side, -sides[j].textureoffset, sides[j].rowoffset, -1, j, accel, l->special_args[0]);
      l->special = 0;
      break;
    case zl_scroll_texture_left:
      j = lines[i].sidenum[0];
      Add_Scroller(sc_side, FRACUNIT * l->special_args[0] / 64, 0, -1, j, accel, l->special_args[1]);
      break;
    case zl_scroll_texture_right:
      j = lines[i].sidenum[0];
      Add_Scroller(sc_side, -FRACUNIT * l->special_args[0] / 64, 0, -1, j, accel, l->special_args[1]);
      break;
    case zl_scroll_texture_up:
      j = lines[i].sidenum[0];
      Add_Scroller(sc_side, 0, FRACUNIT * l->special_args[0] / 64, -1, j, accel, l->special_args[1]);
      break;
    case zl_scroll_texture_down:
      j = lines[i].sidenum[0];
      Add_Scroller(sc_side, 0, -FRACUNIT * l->special_args[0] / 64, -1, j, accel, l->special_args[1]);
      break;
    case zl_scroll_texture_both:
      j = lines[i].sidenum[0];

      if (l->special_args[0] == 0) {
        dx = FRACUNIT * (l->special_args[1] - l->special_args[2]) / 64;
        dy = FRACUNIT * (l->special_args[4] - l->special_args[3]) / 64;
        Add_Scroller(sc_side, dx, dy, -1, j, accel, 0);
      }

      l->special = 0;
      break;
    default:
      break;
  }
}

// Initialize the scrollers
static void P_SpawnScrollers(void)
{
  int i;
  line_t *l;

  P_InitCopyScrollers();

  for (i = 0, l = lines; i < numlines; i++, l++)
    map_format.spawn_scroller(l, i);

  P_FreeCopyScrollers();
}

// e6y
// restored boom's friction code

/////////////////////////////
//
// Add a friction thinker to the thinker list
//
// Add_Friction adds a new friction thinker to the list of active thinkers.
//

static void Add_Friction(int friction, int movefactor, int affectee)
{
    friction_t *f = Z_MallocLevel(sizeof *f);

    f->thinker.function/*.acp1*/ = /*(actionf_p1) */T_Friction;
    f->friction = friction;
    f->movefactor = movefactor;
    f->affectee = affectee;
    P_AddThinker(&f->thinker);
}

/////////////////////////////
//
// This is where abnormal friction is applied to objects in the sectors.
// A friction thinker has been spawned for each sector where less or
// more friction should be applied. The amount applied is proportional to
// the length of the controlling linedef.

void T_Friction(friction_t *f)
{
    sector_t *sec;
    mobj_t   *thing;
    msecnode_t* node;

    if (compatibility || !variable_friction)
        return;

    sec = sectors + f->affectee;

    // Be sure the special sector type is still turned on. If so, proceed.
    // Else, bail out; the sector type has been changed on us.

    if (!(sec->flags & SECF_FRICTION))
        return;

    // Assign the friction value to players on the floor, non-floating,
    // and clipped. Normally the object's friction value is kept at
    // ORIG_FRICTION and this thinker changes it for icy or muddy floors.

    // In Phase II, you can apply friction to Things other than players.

    // When the object is straddling sectors with the same
    // floorheight that have different frictions, use the lowest
    // friction value (muddy has precedence over icy).

    node = sec->touching_thinglist; // things touching this sector
    while (node)
        {
        thing = node->m_thing;
        if (thing->player &&
            !(thing->flags & (MF_NOGRAVITY | MF_NOCLIP)) &&
            thing->z <= sec->floorheight)
            {
            if ((thing->friction == ORIG_FRICTION) ||     // normal friction?
              (f->friction < thing->friction))
                {
                thing->friction   = f->friction;
                thing->movefactor = f->movefactor;
                }
            }
        node = node->m_snext;
        }
}


// killough 3/7/98 -- end generalized scroll effects

////////////////////////////////////////////////////////////////////////////
//
// FRICTION EFFECTS
//
// phares 3/12/98: Start of friction effects
//
// As the player moves, friction is applied by decreasing the x and y
// momentum values on each tic. By varying the percentage of decrease,
// we can simulate muddy or icy conditions. In mud, the player slows
// down faster. In ice, the player slows down more slowly.
//
// The amount of friction change is controlled by the length of a linedef
// with type 223. A length < 100 gives you mud. A length > 100 gives you ice.
//
// Also, each sector where these effects are to take place is given a
// new special type _______. Changing the type value at runtime allows
// these effects to be turned on or off.
//
// Sector boundaries present problems. The player should experience these
// friction changes only when his feet are touching the sector floor. At
// sector boundaries where floor height changes, the player can find
// himself still 'in' one sector, but with his feet at the floor level
// of the next sector (steps up or down). To handle this, Thinkers are used
// in icy/muddy sectors. These thinkers examine each object that is touching
// their sectors, looking for players whose feet are at the same level as
// their floors. Players satisfying this condition are given new friction
// values that are applied by the player movement code later.
//
// killough 8/28/98:
//
// Completely redid code, which did not need thinkers, and which put a heavy
// drag on CPU. Friction is now a property of sectors, NOT objects inside
// them. All objects, not just players, are affected by it, if they touch
// the sector's floor. Code simpler and faster, only calling on friction
// calculations when an object needs friction considered, instead of doing
// friction calculations on every sector during every tic.
//
// Although this -might- ruin Boom demo sync involving friction, it's the only
// way, short of code explosion, to fix the original design bug. Fixing the
// design bug in Boom's original friction code, while maintaining demo sync
// under every conceivable circumstance, would double or triple code size, and
// would require maintenance of buggy legacy code which is only useful for old
// demos. Doom demos, which are more important IMO, are not affected by this
// change.
//
/////////////////////////////
//
// Initialize the sectors where friction is increased or decreased

static void P_ApplySectorFriction(int tag, int value, int use_thinker)
{
  const int *id_p;
  int friction, movefactor;

  friction = (0x1EB8 * value) / 0x80 + 0xD000;

  // The following check might seem odd. At the time of movement,
  // the move distance is multiplied by 'friction/0x10000', so a
  // higher friction value actually means 'less friction'.

  if (friction > ORIG_FRICTION)       // ice
    movefactor = ((0x10092 - friction) * (0x70)) / 0x158;
  else
    movefactor = ((friction - 0xDB34) * (0xA)) / 0x80;

  if (mbf_features)
  { // killough 8/28/98: prevent odd situations
    if (friction > FRACUNIT)
      friction = FRACUNIT;
    if (friction < 0)
      friction = 0;
    if (movefactor < 32)
      movefactor = 32;
  }

  FIND_SECTORS(id_p, tag)
  {
    // killough 8/28/98:
    //
    // Instead of spawning thinkers, which are slow and expensive,
    // modify the sector's own friction values. Friction should be
    // a property of sectors, not objects which reside inside them.
    // Original code scanned every object in every friction sector
    // on every tic, adjusting its friction, putting unnecessary
    // drag on CPU. New code adjusts friction of sector only once
    // at level startup, and then uses this friction value.

    //e6y: boom's friction code for boom compatibility
    if (use_thinker)
      Add_Friction(friction, movefactor, *id_p);

    sectors[*id_p].friction = friction;
    sectors[*id_p].movefactor = movefactor;
  }
}

void P_SpawnCompatibleFriction(line_t *l)
{
  if (l->special == 223)
  {
    int value, use_thinker;

    value = P_AproxDistance(l->dx, l->dy) >> FRACBITS;
    use_thinker = !demo_compatibility && !mbf_features && !prboom_comp[PC_PRBOOM_FRICTION].state;

    P_ApplySectorFriction(l->tag, value, use_thinker);
  }
}

void P_SpawnZDoomFriction(line_t *l)
{
  if (l->special == zl_sector_set_friction)
  {
    int value;

    if (l->special_args[1])
      value = l->special_args[1] <= 200 ? l->special_args[1] : 200;
    else
      value = P_AproxDistance(l->dx, l->dy) >> FRACBITS;

    P_ApplySectorFriction(l->special_args[0], value, false);

    l->special = 0;
  }
}

static void P_SpawnFriction(void)
{
  int i;
  line_t *l = lines;

  for (i = 0; i < numlines; i++, l++)
    map_format.spawn_friction(l);
}

//
// phares 3/12/98: End of friction effects
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
// PUSH/PULL EFFECT
//
// phares 3/20/98: Start of push/pull effects
//
// This is where push/pull effects are applied to objects in the sectors.
//
// There are four kinds of push effects
//
// 1) Pushing Away
//
//    Pushes you away from a point source defined by the location of an
//    MT_PUSH Thing. The force decreases linearly with distance from the
//    source. This force crosses sector boundaries and is felt w/in a circle
//    whose center is at the MT_PUSH. The force is felt only if the point
//    MT_PUSH can see the target object.
//
// 2) Pulling toward
//
//    Same as Pushing Away except you're pulled toward an MT_PULL point
//    source. This force crosses sector boundaries and is felt w/in a circle
//    whose center is at the MT_PULL. The force is felt only if the point
//    MT_PULL can see the target object.
//
// 3) Wind
//
//    Pushes you in a constant direction. Full force above ground, half
//    force on the ground, nothing if you're below it (water).
//
// 4) Current
//
//    Pushes you in a constant direction. No force above ground, full
//    force if on the ground or below it (water).
//
// The magnitude of the force is controlled by the length of a controlling
// linedef. The force vector for types 3 & 4 is determined by the angle
// of the linedef, and is constant.
//
// For each sector where these effects occur, the sector special type has
// to have the PUSH_MASK bit set. If this bit is turned off by a switch
// at run-time, the effect will not occur. The controlling sector for
// types 1 & 2 is the sector containing the MT_PUSH/MT_PULL Thing.


#define PUSH_FACTOR 7

/////////////////////////////
//
// Add a push thinker to the thinker list

static void Add_Pusher(int type, int x_mag, int y_mag, mobj_t* source, int affectee)
{
    pusher_t *p = Z_MallocLevel(sizeof *p);

    p->thinker.function = T_Pusher;
    p->source = source;
    p->type = type;
    p->x_mag = x_mag>>FRACBITS;
    p->y_mag = y_mag>>FRACBITS;
    p->magnitude = P_AproxDistance(p->x_mag,p->y_mag);
    if (source) // point source exist?
        {
        p->radius = (p->magnitude)<<(FRACBITS+1); // where force goes to zero
        p->x = p->source->x;
        p->y = p->source->y;
        }
    p->affectee = affectee;
    P_AddThinker(&p->thinker);
}

/////////////////////////////
//
// PIT_PushThing determines the angle and magnitude of the effect.
// The object's x and y momentum values are changed.
//
// tmpusher belongs to the point source (MT_PUSH/MT_PULL).
//
// killough 10/98: allow to affect things besides players

pusher_t* tmpusher; // pusher structure for blockmap searches

static dboolean PIT_PushThing(mobj_t* thing)
{
  /* killough 10/98: made more general */
  if (!mbf_features ?
      thing->player && !(thing->flags & (MF_NOCLIP | MF_NOGRAVITY)) :
      (sentient(thing) || thing->flags & MF_SHOOTABLE) &&
      !(thing->flags & MF_NOCLIP))
    {
      angle_t pushangle;
      fixed_t speed;
      fixed_t sx = tmpusher->x;
      fixed_t sy = tmpusher->y;

      speed = (tmpusher->magnitude -
               ((P_AproxDistance(thing->x - sx,thing->y - sy)
                 >>FRACBITS)>>1))<<(FRACBITS-PUSH_FACTOR-1);

      // killough 10/98: make magnitude decrease with square
      // of distance, making it more in line with real nature,
      // so long as it's still in range with original formula.
      //
      // Removes angular distortion, and makes effort required
      // to stay close to source, grow increasingly hard as you
      // get closer, as expected. Still, it doesn't consider z :(

      if (speed > 0 && mbf_features)
        {
          int x = (thing->x-sx) >> FRACBITS;
          int y = (thing->y-sy) >> FRACBITS;
          speed = (int)(((uint64_t) tmpusher->magnitude << 23) / (x*x+y*y+1));
        }

      // If speed <= 0, you're outside the effective radius. You also have
      // to be able to see the push/pull source point.

      if (speed > 0 && P_CheckSight(thing,tmpusher->source))
        {
          pushangle = R_PointToAngle2(thing->x,thing->y,sx,sy);
          if (tmpusher->source->type == MT_PUSH)
            pushangle += ANG180;    // away
          pushangle >>= ANGLETOFINESHIFT;
          thing->momx += FixedMul(speed,finecosine[pushangle]);
          thing->momy += FixedMul(speed,finesine[pushangle]);
          thing->intflags |= MIF_SCROLLING;
        }
    }
  return true;
}

/////////////////////////////
//
// T_Pusher looks for all objects that are inside the radius of
// the effect.
//

void T_Pusher(pusher_t *p)
{
    sector_t *sec;
    mobj_t   *thing;
    msecnode_t* node;
    int xspeed,yspeed;
    int xl,xh,yl,yh,bx,by;
    int radius;
    int ht = 0;

    if (!allow_pushers)
        return;

    sec = sectors + p->affectee;

    // Be sure the special sector type is still turned on. If so, proceed.
    // Else, bail out; the sector type has been changed on us.

    if (!(sec->flags & SECF_PUSH))
        return;

    // For constant pushers (wind/current) there are 3 situations:
    //
    // 1) Affected Thing is above the floor.
    //
    //    Apply the full force if wind, no force if current.
    //
    // 2) Affected Thing is on the ground.
    //
    //    Apply half force if wind, full force if current.
    //
    // 3) Affected Thing is below the ground (underwater effect).
    //
    //    Apply no force if wind, full force if current.

    if (p->type == p_push)
        {

        // Seek out all pushable things within the force radius of this
        // point pusher. Crosses sectors, so use blockmap.

        tmpusher = p; // MT_PUSH/MT_PULL point source
        radius = p->radius; // where force goes to zero
        tmbbox[BOXTOP]    = p->y + radius;
        tmbbox[BOXBOTTOM] = p->y - radius;
        tmbbox[BOXRIGHT]  = p->x + radius;
        tmbbox[BOXLEFT]   = p->x - radius;

        xl = P_GetSafeBlockX(tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS);
        xh = P_GetSafeBlockX(tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS);
        yl = P_GetSafeBlockY(tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS);
        yh = P_GetSafeBlockY(tmbbox[BOXTOP] - bmaporgy + MAXRADIUS);
        for (bx=xl ; bx<=xh ; bx++)
            for (by=yl ; by<=yh ; by++)
                P_BlockThingsIterator(bx,by,PIT_PushThing);
        return;
        }

    // constant pushers p_wind and p_current

    if (sec->heightsec != -1) // special water sector?
        ht = sectors[sec->heightsec].floorheight;
    node = sec->touching_thinglist; // things touching this sector
    for ( ; node ; node = node->m_snext)
        {
        thing = node->m_thing;
        if (!thing->player || (thing->flags & (MF_NOGRAVITY | MF_NOCLIP)))
            continue;
        if (p->type == p_wind)
            {
            if (sec->heightsec == -1) // NOT special water sector
                if (thing->z > thing->floorz) // above ground
                    {
                    xspeed = p->x_mag; // full force
                    yspeed = p->y_mag;
                    }
                else // on ground
                    {
                    xspeed = (p->x_mag)>>1; // half force
                    yspeed = (p->y_mag)>>1;
                    }
            else // special water sector
                {
                if (thing->z > ht) // above ground
                    {
                    xspeed = p->x_mag; // full force
                    yspeed = p->y_mag;
                    }
                else if (thing->player->viewz < ht) // underwater
                    xspeed = yspeed = 0; // no force
                else // wading in water
                    {
                    xspeed = (p->x_mag)>>1; // half force
                    yspeed = (p->y_mag)>>1;
                    }
                }
            }
        else // p_current
            {
            if (sec->heightsec == -1) // NOT special water sector
                if (thing->z > sec->floorheight) // above ground
                    xspeed = yspeed = 0; // no force
                else // on ground
                    {
                    xspeed = p->x_mag; // full force
                    yspeed = p->y_mag;
                    }
            else // special water sector
                if (thing->z > ht) // above ground
                    xspeed = yspeed = 0; // no force
                else // underwater
                    {
                    xspeed = p->x_mag; // full force
                    yspeed = p->y_mag;
                    }
            }
        thing->momx += xspeed<<(FRACBITS-PUSH_FACTOR);
        thing->momy += yspeed<<(FRACBITS-PUSH_FACTOR);
        thing->intflags |= MIF_SCROLLING;
        }
}

/////////////////////////////
//
// P_GetPushThing() returns a pointer to an MT_PUSH or MT_PULL thing,
// NULL otherwise.

mobj_t* P_GetPushThing(int s)
{
    mobj_t* thing;
    sector_t* sec;

    sec = sectors + s;
    thing = sec->thinglist;
    while (thing)
        {
        switch(thing->type)
            {
          case MT_PUSH:
          case MT_PULL:
            return thing;
          default:
            break;
            }
        thing = thing->snext;
        }
    return NULL;
}

/////////////////////////////
//
// Initialize the sectors where pushers are present
//

void P_SpawnCompatiblePusher(line_t *l)
{
  const int *id_p;
  mobj_t* thing;

  switch(l->special)
  {
    case 224: // wind
      FIND_SECTORS(id_p, l->tag)
        Add_Pusher(p_wind, l->dx, l->dy, NULL, *id_p);
      break;
    case 225: // current
      FIND_SECTORS(id_p, l->tag)
        Add_Pusher(p_current, l->dx, l->dy, NULL, *id_p);
      break;
    case 226: // push/pull
      FIND_SECTORS(id_p, l->tag)
      {
        thing = P_GetPushThing(*id_p);
        if (thing) // No MT_P* means no effect
          Add_Pusher(p_push, l->dx, l->dy, thing, *id_p);
      }
      break;
  }
}

static void CalculatePushVector(line_t *l, int magnitude, int angle, fixed_t *dx, fixed_t *dy)
{
  if (l->special_args[3])
  {
    *dx = l->dx;
    *dy = l->dy;
    return;
  }

  angle = angle * (ANG180 >> 7); // 256 is 360
  angle >>= ANGLETOFINESHIFT;
  magnitude <<= FRACBITS;

  *dx = FixedMul(magnitude, finecosine[angle]);
  *dy = FixedMul(magnitude, finesine[angle]);
}

void P_SpawnZDoomPusher(line_t *l)
{
  const int *id_p;
  mobj_t* thing;
  fixed_t dx, dy;

  switch (l->special)
  {
    case zl_sector_set_wind: // wind
      CalculatePushVector(l, l->special_args[1], l->special_args[2], &dx, &dy);
      FIND_SECTORS(id_p, l->special_args[0])
        Add_Pusher(p_wind, dx, dy, NULL, *id_p);
      l->special = 0;
      break;
    case zl_sector_set_current: // current
      CalculatePushVector(l, l->special_args[1], l->special_args[2], &dx, &dy);
      FIND_SECTORS(id_p, l->special_args[0])
        Add_Pusher(p_current, dx, dy, NULL, *id_p);
      l->special = 0;
      break;
    case zl_point_push_set_force: // push/pull
      CalculatePushVector(l, l->special_args[2], 0, &dx, &dy);
      if (l->special_args[0])
      {  // [RH] Find thing by sector
        FIND_SECTORS(id_p, l->special_args[0])
        {
          thing = P_GetPushThing(*id_p);
          if (thing) // No MT_P* means no effect
          {
            // [RH] Allow narrowing it down by tid
            if (!l->special_args[1] || l->special_args[1] == thing->tid)
              Add_Pusher(p_push, dx, dy, thing, *id_p);
          }
        }
      }
      else
      {  // [RH] Find thing by tid
        int s;

        for (s = -1; (thing = P_FindMobjFromTID(l->special_args[1], &s)) != NULL;)
          if (thing->type == map_format.mt_push || thing->type == map_format.mt_pull)
            Add_Pusher(p_push, dx, dy, thing, thing->subsector->sector->iSectorID);
      }
      l->special = 0;
      break;
  }
}

static void P_SpawnPushers(void)
{
    int i;
    line_t *l = lines;

    for (i = 0; i < numlines; i++, l++)
      map_format.spawn_pusher(l);
}

//
// phares 3/20/98: End of Pusher effects
//
////////////////////////////////////////////////////////////////////////////

// heretic

#include "heretic/def.h"

typedef enum
{
    afxcmd_play,                // (sound)
    afxcmd_playabsvol,          // (sound, volume)
    afxcmd_playrelvol,          // (sound, volume)
    afxcmd_delay,               // (ticks)
    afxcmd_delayrand,           // (andbits)
    afxcmd_end                  // ()
} afxcmd_t;

int *LevelAmbientSfx[MAX_AMBIENT_SFX];
int *AmbSfxPtr;
int AmbSfxPtrIndex;
int AmbSfxCount;
int AmbSfxTics;
int AmbSfxVolume;

int AmbSndSeqInit[] = {         // Startup
    afxcmd_end
};
int AmbSndSeq1[] = {            // Scream
    afxcmd_play, heretic_sfx_amb1,
    afxcmd_end
};
int AmbSndSeq2[] = {            // Squish
    afxcmd_play, heretic_sfx_amb2,
    afxcmd_end
};
int AmbSndSeq3[] = {            // Drops
    afxcmd_play, heretic_sfx_amb3,
    afxcmd_delay, 16,
    afxcmd_delayrand, 31,
    afxcmd_play, heretic_sfx_amb7,
    afxcmd_delay, 16,
    afxcmd_delayrand, 31,
    afxcmd_play, heretic_sfx_amb3,
    afxcmd_delay, 16,
    afxcmd_delayrand, 31,
    afxcmd_play, heretic_sfx_amb7,
    afxcmd_delay, 16,
    afxcmd_delayrand, 31,
    afxcmd_play, heretic_sfx_amb3,
    afxcmd_delay, 16,
    afxcmd_delayrand, 31,
    afxcmd_play, heretic_sfx_amb7,
    afxcmd_delay, 16,
    afxcmd_delayrand, 31,
    afxcmd_end
};
int AmbSndSeq4[] = {            // SlowFootSteps
    afxcmd_play, heretic_sfx_amb4,
    afxcmd_delay, 15,
    afxcmd_playrelvol, heretic_sfx_amb11, -3,
    afxcmd_delay, 15,
    afxcmd_playrelvol, heretic_sfx_amb4, -3,
    afxcmd_delay, 15,
    afxcmd_playrelvol, heretic_sfx_amb11, -3,
    afxcmd_delay, 15,
    afxcmd_playrelvol, heretic_sfx_amb4, -3,
    afxcmd_delay, 15,
    afxcmd_playrelvol, heretic_sfx_amb11, -3,
    afxcmd_delay, 15,
    afxcmd_playrelvol, heretic_sfx_amb4, -3,
    afxcmd_delay, 15,
    afxcmd_playrelvol, heretic_sfx_amb11, -3,
    afxcmd_end
};
int AmbSndSeq5[] = {            // Heartbeat
    afxcmd_play, heretic_sfx_amb5,
    afxcmd_delay, 35,
    afxcmd_play, heretic_sfx_amb5,
    afxcmd_delay, 35,
    afxcmd_play, heretic_sfx_amb5,
    afxcmd_delay, 35,
    afxcmd_play, heretic_sfx_amb5,
    afxcmd_end
};
int AmbSndSeq6[] = {            // Bells
    afxcmd_play, heretic_sfx_amb6,
    afxcmd_delay, 17,
    afxcmd_playrelvol, heretic_sfx_amb6, -8,
    afxcmd_delay, 17,
    afxcmd_playrelvol, heretic_sfx_amb6, -8,
    afxcmd_delay, 17,
    afxcmd_playrelvol, heretic_sfx_amb6, -8,
    afxcmd_end
};
int AmbSndSeq7[] = {            // Growl
    afxcmd_play, heretic_sfx_bstsit,
    afxcmd_end
};
int AmbSndSeq8[] = {            // Magic
    afxcmd_play, heretic_sfx_amb8,
    afxcmd_end
};
int AmbSndSeq9[] = {            // Laughter
    afxcmd_play, heretic_sfx_amb9,
    afxcmd_delay, 16,
    afxcmd_playrelvol, heretic_sfx_amb9, -4,
    afxcmd_delay, 16,
    afxcmd_playrelvol, heretic_sfx_amb9, -4,
    afxcmd_delay, 16,
    afxcmd_playrelvol, heretic_sfx_amb10, -4,
    afxcmd_delay, 16,
    afxcmd_playrelvol, heretic_sfx_amb10, -4,
    afxcmd_delay, 16,
    afxcmd_playrelvol, heretic_sfx_amb10, -4,
    afxcmd_end
};
int AmbSndSeq10[] = {           // FastFootsteps
    afxcmd_play, heretic_sfx_amb4,
    afxcmd_delay, 8,
    afxcmd_playrelvol, heretic_sfx_amb11, -3,
    afxcmd_delay, 8,
    afxcmd_playrelvol, heretic_sfx_amb4, -3,
    afxcmd_delay, 8,
    afxcmd_playrelvol, heretic_sfx_amb11, -3,
    afxcmd_delay, 8,
    afxcmd_playrelvol, heretic_sfx_amb4, -3,
    afxcmd_delay, 8,
    afxcmd_playrelvol, heretic_sfx_amb11, -3,
    afxcmd_delay, 8,
    afxcmd_playrelvol, heretic_sfx_amb4, -3,
    afxcmd_delay, 8,
    afxcmd_playrelvol, heretic_sfx_amb11, -3,
    afxcmd_end
};

int *AmbientSfx[] = {
    AmbSndSeq1,                 // Scream
    AmbSndSeq2,                 // Squish
    AmbSndSeq3,                 // Drops
    AmbSndSeq4,                 // SlowFootsteps
    AmbSndSeq5,                 // Heartbeat
    AmbSndSeq6,                 // Bells
    AmbSndSeq7,                 // Growl
    AmbSndSeq8,                 // Magic
    AmbSndSeq9,                 // Laughter
    AmbSndSeq10                 // FastFootsteps
};

int *TerrainTypes;
struct
{
    const char *name;
    int type;
} TerrainTypeDefs[2][6] =
{
  {
    { "FLTWAWA1", FLOOR_WATER },
    { "FLTFLWW1", FLOOR_WATER },
    { "FLTLAVA1", FLOOR_LAVA },
    { "FLATHUH1", FLOOR_LAVA },
    { "FLTSLUD1", FLOOR_SLUDGE },
    { "END", -1 }
  },
  {
    { "X_005", FLOOR_WATER },
    { "X_001", FLOOR_LAVA },
    { "X_009", FLOOR_SLUDGE },
    { "F_033", FLOOR_ICE },
    { "END", -1 }
  }
};

mobj_t LavaInflictor;

void P_AddAmbientSfx(int sequence)
{
    if (AmbSfxCount == MAX_AMBIENT_SFX)
    {
        I_Error("Too many ambient sound sequences");
    }
    LevelAmbientSfx[AmbSfxCount++] = AmbientSfx[sequence];
}

void P_InitAmbientSound(void)
{
    AmbSfxCount = 0;
    AmbSfxVolume = 0;
    AmbSfxTics = 10 * TICRATE;
    AmbSfxPtrIndex = -1;
    AmbSfxPtr = AmbSndSeqInit;
}

void P_AmbientSound(void)
{
    afxcmd_t cmd;
    int sound;
    dboolean done;

    if (!AmbSfxCount)
    {                           // No ambient sound sequences on current level
        return;
    }
    if (--AmbSfxTics)
    {
        return;
    }
    done = false;
    do
    {
        cmd = *AmbSfxPtr++;
        switch (cmd)
        {
            case afxcmd_play:
                AmbSfxVolume = P_Random(pr_heretic) >> 2;
                S_StartAmbientSound(NULL, *AmbSfxPtr++, AmbSfxVolume);
                break;
            case afxcmd_playabsvol:
                sound = *AmbSfxPtr++;
                AmbSfxVolume = *AmbSfxPtr++;
                S_StartAmbientSound(NULL, sound, AmbSfxVolume);
                break;
            case afxcmd_playrelvol:
                sound = *AmbSfxPtr++;
                AmbSfxVolume += *AmbSfxPtr++;
                if (AmbSfxVolume < 0)
                {
                    AmbSfxVolume = 0;
                }
                else if (AmbSfxVolume > 127)
                {
                    AmbSfxVolume = 127;
                }
                S_StartAmbientSound(NULL, sound, AmbSfxVolume);
                break;
            case afxcmd_delay:
                AmbSfxTics = *AmbSfxPtr++;
                done = true;
                break;
            case afxcmd_delayrand:
                AmbSfxTics = P_Random(pr_heretic) & (*AmbSfxPtr++);
                done = true;
                break;
            case afxcmd_end:
                AmbSfxTics = 6 * TICRATE + P_Random(pr_heretic);
                AmbSfxPtrIndex = P_Random(pr_heretic) % AmbSfxCount;
                AmbSfxPtr = LevelAmbientSfx[AmbSfxPtrIndex];
                done = true;
                break;
            default:
                I_Error("P_AmbientSound: Unknown afxcmd %d", cmd);
                break;
        }
    }
    while (done == false);
}

void P_InitLava(void)
{
    if (!raven) return;

    memset(&LavaInflictor, 0, sizeof(mobj_t));
    LavaInflictor.type = g_lava_type;
    LavaInflictor.flags2 = MF2_FIREDAMAGE | MF2_NODMGTHRUST;
}

void P_InitTerrainTypes(void)
{
    int i;
    int lump;
    int size;

    if (!raven) return;

    size = (numflats + 1) * sizeof(int);
    TerrainTypes = Z_Malloc(size);
    memset(TerrainTypes, 0, size);
    for (i = 0; TerrainTypeDefs[hexen][i].type != -1; i++)
    {
        lump = W_CheckNumForName2(TerrainTypeDefs[hexen][i].name, ns_flats);
        if (lump != LUMP_NOT_FOUND)
        {
            TerrainTypes[lump - firstflat] = TerrainTypeDefs[hexen][i].type;
        }
    }
}

void P_CrossHereticSpecialLine(line_t * line, int side, mobj_t * thing, dboolean bossaction)
{
    if (!thing->player)
    {                           // Check if trigger allowed by non-player mobj
        switch (line->special)
        {
            case 39:           // Trigger_TELEPORT
            case 97:           // Retrigger_TELEPORT
            case 4:            // Trigger_Raise_Door
                //case 10:      // PLAT DOWN-WAIT-UP-STAY TRIGGER
                //case 88:      // PLAT DOWN-WAIT-UP-STAY RETRIGGER
                break;
            default:
                return;
                break;
        }
    }
    switch (line->special)
    {
            //====================================================
            // TRIGGERS
            //====================================================
        case 2:                // Open Door
            EV_DoDoor(line, vld_open);
            line->special = 0;
            break;
        case 3:                // Close Door
            EV_DoDoor(line, vld_close);
            line->special = 0;
            break;
        case 4:                // Raise Door
            EV_DoDoor(line, vld_normal);
            line->special = 0;
            break;
        case 5:                // Raise Floor
            EV_DoFloor(line, raiseFloor);
            line->special = 0;
            break;
        case 6:                // Fast Ceiling Crush & Raise
            EV_DoCeiling(line, fastCrushAndRaise);
            line->special = 0;
            break;
        case 8:                // Trigger_Build_Stairs (8 pixel steps)
            EV_BuildStairs(line, heretic_build8);
            line->special = 0;
            break;
        case 106:              // Trigger_Build_Stairs_16 (16 pixel steps)
            EV_BuildStairs(line, heretic_turbo16);
            line->special = 0;
            break;
        case 10:               // PlatDownWaitUp
            EV_DoPlat(line, downWaitUpStay, 0);
            line->special = 0;
            break;
        case 12:               // Light Turn On - brightest near
            EV_LightTurnOn(line, 0);
            line->special = 0;
            break;
        case 13:               // Light Turn On 255
            EV_LightTurnOn(line, 255);
            line->special = 0;
            break;
        case 16:               // Close Door 30
            EV_DoDoor(line, vld_close30ThenOpen);
            line->special = 0;
            break;
        case 17:               // Start Light Strobing
            EV_StartLightStrobing(line);
            line->special = 0;
            break;
        case 19:               // Lower Floor
            EV_DoFloor(line, lowerFloor);
            line->special = 0;
            break;
        case 22:               // Raise floor to nearest height and change texture
            EV_DoPlat(line, raiseToNearestAndChange, 0);
            line->special = 0;
            break;
        case 25:               // Ceiling Crush and Raise
            EV_DoCeiling(line, crushAndRaise);
            line->special = 0;
            break;
        case 30:               // Raise floor to shortest texture height
            // on either side of lines
            EV_DoFloor(line, raiseToTexture);
            line->special = 0;
            break;
        case 35:               // Lights Very Dark
            EV_LightTurnOn(line, 35);
            line->special = 0;
            break;
        case 36:               // Lower Floor (TURBO)
            EV_DoFloor(line, turboLower);
            line->special = 0;
            break;
        case 37:               // LowerAndChange
            EV_DoFloor(line, lowerAndChange);
            line->special = 0;
            break;
        case 38:               // Lower Floor To Lowest
            EV_DoFloor(line, lowerFloorToLowest);
            line->special = 0;
            break;
        case 39:               // TELEPORT!
            map_format.ev_teleport(0, line->tag, line, side, thing, TELF_VANILLA);
            line->special = 0;
            break;
        case 40:               // RaiseCeilingLowerFloor
            EV_DoCeiling(line, raiseToHighest);
            EV_DoFloor(line, lowerFloorToLowest);
            line->special = 0;
            break;
        case 44:               // Ceiling Crush
            EV_DoCeiling(line, lowerAndCrush);
            line->special = 0;
            break;
        case 52:               // EXIT!
            G_ExitLevel(0);
            line->special = 0;
            break;
        case 53:               // Perpetual Platform Raise
            EV_DoPlat(line, perpetualRaise, 0);
            line->special = 0;
            break;
        case 54:               // Platform Stop
            EV_StopPlat(line);
            line->special = 0;
            break;
        case 56:               // Raise Floor Crush
            EV_DoFloor(line, raiseFloorCrush);
            line->special = 0;
            break;
        case 57:               // Ceiling Crush Stop
            EV_CeilingCrushStop(line);
            line->special = 0;
            break;
        case 58:               // Raise Floor 24
            EV_DoFloor(line, raiseFloor24);
            line->special = 0;
            break;
        case 59:               // Raise Floor 24 And Change
            EV_DoFloor(line, raiseFloor24AndChange);
            line->special = 0;
            break;
        case 104:              // Turn lights off in sector(tag)
            EV_TurnTagLightsOff(line);
            line->special = 0;
            break;
        case 105:              // Trigger_SecretExit
            G_SecretExitLevel(0);
            line->special = 0;
            break;

            //====================================================
            // RE-DOABLE TRIGGERS
            //====================================================

        case 72:               // Ceiling Crush
            EV_DoCeiling(line, lowerAndCrush);
            break;
        case 73:               // Ceiling Crush and Raise
            EV_DoCeiling(line, crushAndRaise);
            break;
        case 74:               // Ceiling Crush Stop
            EV_CeilingCrushStop(line);
            break;
        case 75:               // Close Door
            EV_DoDoor(line, vld_close);
            break;
        case 76:               // Close Door 30
            EV_DoDoor(line, vld_close30ThenOpen);
            break;
        case 77:               // Fast Ceiling Crush & Raise
            EV_DoCeiling(line, fastCrushAndRaise);
            break;
        case 79:               // Lights Very Dark
            EV_LightTurnOn(line, 35);
            break;
        case 80:               // Light Turn On - brightest near
            EV_LightTurnOn(line, 0);
            break;
        case 81:               // Light Turn On 255
            EV_LightTurnOn(line, 255);
            break;
        case 82:               // Lower Floor To Lowest
            EV_DoFloor(line, lowerFloorToLowest);
            break;
        case 83:               // Lower Floor
            EV_DoFloor(line, lowerFloor);
            break;
        case 84:               // LowerAndChange
            EV_DoFloor(line, lowerAndChange);
            break;
        case 86:               // Open Door
            EV_DoDoor(line, vld_open);
            break;
        case 87:               // Perpetual Platform Raise
            EV_DoPlat(line, perpetualRaise, 0);
            break;
        case 88:               // PlatDownWaitUp
            EV_DoPlat(line, downWaitUpStay, 0);
            break;
        case 89:               // Platform Stop
            EV_StopPlat(line);
            break;
        case 90:               // Raise Door
            EV_DoDoor(line, vld_normal);
            break;
        case 100:              // Retrigger_Raise_Door_Turbo
            EV_DoDoor(line, vld_normal_turbo);
            break;
        case 91:               // Raise Floor
            EV_DoFloor(line, raiseFloor);
            break;
        case 92:               // Raise Floor 24
            EV_DoFloor(line, raiseFloor24);
            break;
        case 93:               // Raise Floor 24 And Change
            EV_DoFloor(line, raiseFloor24AndChange);
            break;
        case 94:               // Raise Floor Crush
            EV_DoFloor(line, raiseFloorCrush);
            break;
        case 95:               // Raise floor to nearest height and change texture
            EV_DoPlat(line, raiseToNearestAndChange, 0);
            break;
        case 96:               // Raise floor to shortest texture height
            // on either side of lines
            EV_DoFloor(line, raiseToTexture);
            break;
        case 97:               // TELEPORT!
            map_format.ev_teleport(0, line->tag, line, side, thing, TELF_VANILLA);
            break;
        case 98:               // Lower Floor (TURBO)
            EV_DoFloor(line, turboLower);
            break;
    }
}

void P_PlayerInHereticSector(player_t * player, sector_t * sector)
{
    static int pushTab[5] = {
        2048 * 5,
        2048 * 10,
        2048 * 25,
        2048 * 30,
        2048 * 35
    };

    switch (sector->special)
    {
        case 7:                // Damage_Sludge
            if (!(leveltime & 31))
            {
                P_DamageMobj(player->mo, NULL, NULL, 4);
            }
            break;
        case 5:                // Damage_LavaWimpy
            if (!(leveltime & 15))
            {
                P_DamageMobj(player->mo, &LavaInflictor, NULL, 5);
                P_HitFloor(player->mo);
            }
            break;
        case 16:               // Damage_LavaHefty
            if (!(leveltime & 15))
            {
                P_DamageMobj(player->mo, &LavaInflictor, NULL, 8);
                P_HitFloor(player->mo);
            }
            break;
        case 4:                // Scroll_EastLavaDamage
            P_Thrust(player, 0, 2048 * 28);
            if (!(leveltime & 15))
            {
                P_DamageMobj(player->mo, &LavaInflictor, NULL, 5);
                P_HitFloor(player->mo);
            }
            break;
        case 9:                // SecretArea
            P_CollectSecretVanilla(sector, player);

            break;
        case 11:               // Exit_SuperDamage (DOOM E1M8 finale)
            break;

        case 25:
        case 26:
        case 27:
        case 28:
        case 29:               // Scroll_North
            P_Thrust(player, ANG90, pushTab[sector->special - 25]);
            break;
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:               // Scroll_East
            P_Thrust(player, 0, pushTab[sector->special - 20]);
            break;
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:               // Scroll_South
            P_Thrust(player, ANG270, pushTab[sector->special - 30]);
            break;
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:               // Scroll_West
            P_Thrust(player, ANG180, pushTab[sector->special - 35]);
            break;

        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
            // Wind specials are handled in (P_mobj):P_XYMovement
            break;

        case 15:               // Friction_Low
            // Only used in (P_mobj):P_XYMovement and (P_user):P_Thrust
            break;

        default:
            I_Error("P_PlayerInSpecialSector: "
                    "unknown special %i", sector->special);
    }
}

void P_SpawnLineSpecials(void)
{
    int i;

    if (!heretic) return;

    //
    //      Init line EFFECTs
    //

    numlinespecials = 0;
    for (i = 0; i < numlines; i++)
        switch (lines[i].special)
        {
            case 48:           // Effect_Scroll_Left
            case 99:           // Effect_Scroll_Right
                linespeciallist[numlinespecials] = &lines[i];
                numlinespecials++;
                break;
        }
}

// hexen

#define MAX_TAGGED_LINES 64

static struct
{
    line_t *line;
    int lineTag;
} TaggedLines[MAX_TAGGED_LINES];
static int TaggedLineCount;

void P_PlayerOnSpecialFlat(player_t * player, int floorType)
{
    if (player->mo->z != player->mo->floorz)
    {                           // Player is not touching the floor
        return;
    }
    switch (floorType)
    {
        case FLOOR_LAVA:
            if (!(leveltime & 31))
            {
                P_DamageMobj(player->mo, &LavaInflictor, NULL, 10);
                S_StartMobjSound(player->mo, hexen_sfx_lava_sizzle);
            }
            break;
        default:
            break;
    }
}

line_t *P_FindLine(int lineTag, int *searchPosition)
{
    int i;

    for (i = *searchPosition + 1; i < TaggedLineCount; i++)
    {
        if (TaggedLines[i].lineTag == lineTag)
        {
            *searchPosition = i;
            return TaggedLines[i].line;
        }
    }
    *searchPosition = -1;
    return NULL;
}

dboolean EV_SectorSoundChange(byte * args)
{
    const int *id_p;
    dboolean rtn;

    if (!args[0])
    {
        return false;
    }
    rtn = false;
    FIND_SECTORS(id_p, args[0])
    {
        sectors[*id_p].seqType = args[1];
        rtn = true;
    }
    return rtn;
}

char LockedBuffer[80];

static dboolean CheckedLockedDoor(mobj_t * mo, byte lock)
{
    extern char *TextKeyMessages[11];

    if (!mo->player)
    {
        return false;
    }
    if (!lock)
    {
        return true;
    }
    if (!mo->player->cards[lock - 1])
    {
        snprintf(LockedBuffer, sizeof(LockedBuffer),
                 "YOU NEED THE %s\n", TextKeyMessages[lock - 1]);
        P_SetMessage(mo->player, LockedBuffer, true);
        S_StartMobjSound(mo, hexen_sfx_door_locked);
        return false;
    }
    return true;
}

dboolean EV_LineSearchForPuzzleItem(line_t * line, byte * args, mobj_t * mo)
{
    player_t *player;
    int i;
    int type;
    artitype_t arti;

    if (!mo)
        return false;
    player = mo->player;
    if (!player)
        return false;

    // Search player's inventory for puzzle items
    for (i = 0; i < player->artifactCount; i++)
    {
        arti = player->inventory[i].type;
        type = arti - hexen_arti_firstpuzzitem;
        if (type < 0)
            continue;
        if (type == line->special_args[0])
        {
            // A puzzle item was found for the line
            if (P_UseArtifact(player, arti))
            {
                // A puzzle item was found for the line
                P_PlayerRemoveArtifact(player, i);
                if (player == &players[consoleplayer])
                {
                    if (arti < hexen_arti_firstpuzzitem)
                    {
                        S_StartVoidSound(hexen_sfx_artifact_use);
                    }
                    else
                    {
                        S_StartVoidSound(hexen_sfx_puzzle_success);
                    }
                    ArtifactFlash = 4;
                }
                return true;
            }
        }
    }
    return false;
}

dboolean P_TestActivateZDoomLine(line_t *line, mobj_t *mo, int side, line_activation_t activationType)
{
  line_activation_t lineActivation;

  lineActivation = line->activation;

  if (line->flags & ML_FIRSTSIDEONLY && side)
  {
    return false;
  }

  if (
    line->special == zl_teleport &&
    lineActivation & SPAC_CROSS &&
    activationType == SPAC_PCROSS &&
    mo && mo->flags & MF_MISSILE
  )
  { // Let missiles use regular player teleports
    lineActivation |= SPAC_PCROSS;
  }

  if (activationType == SPAC_USE || activationType == SPAC_USEBACK)
  {
    if (
      (line->flags & ML_CHECKSWITCHRANGE || map_info.flags & MI_CHECK_SWITCH_RANGE) &&
      !P_CheckSwitchRange(line, mo, side)
    )
    {
      return false;
    }
  }

  if (activationType == SPAC_USE &&
      lineActivation & SPAC_MUSE &&
      mo && !mo->player && mo->flags2 & MF2_CANUSEWALLS)
  {
    return true;
  }

  if (activationType == SPAC_PUSH &&
      lineActivation & SPAC_MPUSH &&
      mo && !mo->player && mo->flags2 & MF2_PUSHWALL)
  {
    return true;
  }

  if (!(lineActivation & activationType))
  {
    if (activationType != SPAC_MCROSS || lineActivation != SPAC_CROSS)
    {
      return false;
    }
  }

  if (activationType == SPAC_ANYCROSS)
  {
    return true;
  }

  if (
    mo && !mo->player &&
    !(mo->flags & MF_MISSILE) &&
    !(line->flags & ML_MONSTERSCANACTIVATE) &&
    (activationType != SPAC_MCROSS || !(lineActivation & SPAC_MCROSS))
  )
  {
    dboolean noway = true;

    // [RH] monsters' ability to activate this line depends on its type
    // In Hexen, only MCROSS lines could be activated by monsters. With
    // lax activation checks, monsters can also activate certain lines
    // even without them being marked as monster activate-able. This is
    // the default for non-Hexen maps in Hexen format.
    if (!(map_info.flags & MI_LAX_MONSTER_ACTIVATION))
    {
      return false;
    }

    if ((activationType == SPAC_USE || activationType == SPAC_PUSH) && line->flags & ML_SECRET)
      return false;    // never open secret doors

    switch (activationType)
    {
      case SPAC_USE:
      case SPAC_PUSH:
        switch (line->special)
        {
          case zl_door_raise:
            if (line->special_args[0] == 0 && line->special_args[1] < 64)
              noway = false;
            break;
          case zl_teleport:
          case zl_teleport_no_fog:
            noway = false;
        }
        break;

      case SPAC_MCROSS:
        if (!(lineActivation & SPAC_MCROSS))
        {
          switch (line->special)
          {
            case zl_door_raise:
              if (line->special_args[1] >= 64)
                break;
            case zl_teleport:
            case zl_teleport_no_fog:
            case zl_teleport_line:
            case zl_plat_down_wait_up_stay_lip:
            case zl_plat_down_wait_up_stay:
              noway = false;
          }
        }
        else
          noway = false;
        break;

      default:
        noway = false;
    }
    return !noway;
  }

  if (
    activationType == SPAC_MCROSS &&
    !(lineActivation & SPAC_MCROSS) &&
    !(line->flags & ML_MONSTERSCANACTIVATE)
  )
  {
    return false;
  }

  return true;
}

dboolean P_TestActivateHexenLine(line_t *line, mobj_t *mo, int side, line_activation_t activationType)
{
  line_activation_t lineActivation;

  lineActivation = line->activation;

  if (lineActivation != activationType)
  {
      return false;
  }

  if (!mo->player && !(mo->flags & MF_MISSILE))
  {
      if (lineActivation != SPAC_MCROSS)
      {                       // currently, monsters can only activate the MCROSS activation type
          return false;
      }
      if (line->flags & ML_SECRET)
          return false;       // never open secret doors
  }

  return true;
}

dboolean P_ActivateLine(line_t * line, mobj_t * mo, int side, line_activation_t activationType)
{
  dboolean repeat;
  dboolean buttonSuccess;

  if (!map_format.test_activate_line(line, mo, side, activationType))
  {
    return false;
  }

  if (line->locknumber)
  {
    dboolean legacy;

    switch (line->special)
    {
      case zl_door_close:
      case zl_door_open:
      case zl_door_raise:
      case zl_door_locked_raise:
      case zl_door_close_wait_open:
      case zl_door_wait_raise:
      case zl_door_wait_close:
      case zl_generic_door:
        legacy = true;
        break;
      default:
        legacy = false;
    }

    if (!P_CheckKeys(mo, line->locknumber, legacy))
    {
      return false;
    }
  }

  repeat = (line->flags & ML_REPEATSPECIAL) != 0;

  buttonSuccess =
    map_format.execute_line_special(line->special, line->special_args, line, side, mo);

  if (!repeat && buttonSuccess)
  {                           // clear the special on non-retriggerable lines
    line->special = 0;
  }

  if (buttonSuccess && line->activation & map_format.switch_activation)
  {
    P_ChangeSwitchTexture(line, repeat);
  }

  return true;
}

void P_PlayerInHexenSector(player_t * player, sector_t * sector)
{
    static int pushTab[3] = {
        2048 * 5,
        2048 * 10,
        2048 * 25
    };

    switch (sector->special)
    {
        case 9:                // SecretArea
            P_CollectSecretVanilla(sector, player);
            break;

        case 201:
        case 202:
        case 203:              // Scroll_North_xxx
            P_Thrust(player, ANG90, pushTab[sector->special - 201]);
            break;
        case 204:
        case 205:
        case 206:              // Scroll_East_xxx
            P_Thrust(player, 0, pushTab[sector->special - 204]);
            break;
        case 207:
        case 208:
        case 209:              // Scroll_South_xxx
            P_Thrust(player, ANG270, pushTab[sector->special - 207]);
            break;
        case 210:
        case 211:
        case 212:              // Scroll_West_xxx
            P_Thrust(player, ANG180, pushTab[sector->special - 210]);
            break;
        case 213:
        case 214:
        case 215:              // Scroll_NorthWest_xxx
            P_Thrust(player, ANG90 + ANG45, pushTab[sector->special - 213]);
            break;
        case 216:
        case 217:
        case 218:              // Scroll_NorthEast_xxx
            P_Thrust(player, ANG45, pushTab[sector->special - 216]);
            break;
        case 219:
        case 220:
        case 221:              // Scroll_SouthEast_xxx
            P_Thrust(player, ANG270 + ANG45, pushTab[sector->special - 219]);
            break;
        case 222:
        case 223:
        case 224:              // Scroll_SouthWest_xxx
            P_Thrust(player, ANG180 + ANG45, pushTab[sector->special - 222]);
            break;

        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
            // Wind specials are handled in (P_mobj):P_XYMovement
            break;

        case 26:               // Stairs_Special1
        case 27:               // Stairs_Special2
            // Used in (P_floor):ProcessStairSector
            break;

        case 198:              // Lightning Special
        case 199:              // Lightning Flash special
        case 200:              // Sky2
            // Used in (R_plane):R_Drawplanes
            break;
        default:
            I_Error("P_PlayerInSpecialSector: "
                    "unknown special %i", sector->special);
    }
}

#include "hexen/a_action.h"
#include "hexen/p_anim.h"
#include "hexen/p_things.h"
#include "hexen/p_acs.h"
#include "hexen/po_man.h"

static dboolean P_ArgToCrushType(int arg)
{
  return arg == 1 ? false : arg == 2 ? true : hexen;
}

static crushmode_e P_ArgToCrushMode(int arg, dboolean slowdown)
{
  static const crushmode_e map[] = { crushDoom, crushHexen, crushSlowdown };

  if (arg >= 1 && arg <= 3) return map[arg - 1];

  return hexen ? crushHexen : slowdown ? crushSlowdown : crushDoom;
}

static int P_ArgToCrush(int arg)
{
  return (arg > 0) ? arg : NO_CRUSH;
}

static byte P_ArgToChange(int arg)
{
  static const byte ChangeMap[8] = { 0, 1, 5, 3, 7, 2, 6, 0 };

  return (arg >= 0 && arg < 8) ? ChangeMap[arg] : 0;
}

static fixed_t P_ArgToSpeed(fixed_t arg)
{
  return arg * FRACUNIT / 8;
}

static fixed_t P_ArgsToFixed(fixed_t arg_i, fixed_t arg_f)
{
  return (arg_i << FRACBITS) + (arg_f << FRACBITS) / 100;
}

static angle_t P_ArgToAngle(angle_t arg)
{
  return arg * (ANG180 / 128);
}

dboolean P_ExecuteZDoomLineSpecial(int special, int * args, line_t * line, int side, mobj_t * mo)
{
  dboolean buttonSuccess = false;

  switch (special)
  {
    case zl_door_close:
      buttonSuccess = EV_DoZDoomDoor(closeDoor, line, mo, args[0],
                                     args[1], 0, 0, args[2], false, 0);
      break;
    case zl_door_open:
      buttonSuccess = EV_DoZDoomDoor(openDoor, line, mo, args[0],
                                    args[1], 0, 0, args[2], false, 0);
      break;
    case zl_door_raise:
      buttonSuccess = EV_DoZDoomDoor(normal, line, mo, args[0],
                                    args[1], args[2], 0, args[3], false, 0);
      break;
    case zl_door_locked_raise:
      buttonSuccess = EV_DoZDoomDoor(args[2] ? normal : openDoor, line, mo, args[0],
                                     args[1], args[2], args[3], args[4], false, 0);
      break;
    case zl_door_close_wait_open:
      buttonSuccess = EV_DoZDoomDoor(genCdO, line, mo, args[0],
                                     args[1], args[2] * 35 / 8, 0, args[3], false, 0);
      break;
    case zl_door_wait_raise:
      buttonSuccess = EV_DoZDoomDoor(waitRaiseDoor, line, mo, args[0],
                                     args[1], args[2], 0, args[4], false, args[3]);
      break;
    case zl_door_wait_close:
      buttonSuccess = EV_DoZDoomDoor(waitCloseDoor, line, mo, args[0],
                                     args[1], 0, 0, args[3], false, args[2]);
      break;
    case zl_generic_door:
      {
        int tag, lightTag;
        vldoor_e type;
        dboolean boomgen = false;

        switch (args[2] & 63)
        {
          case 0:
            type = normal;
            break;
          case 1:
            type = openDoor;
            break;
          case 2:
            type = genCdO;
            break;
          case 3:
            type = closeDoor;
            break;
          default:
            return 0;
        }

        // Boom doesn't allow manual generalized doors to be activated while they move
        if (args[2] & 64)
          boomgen = true;

        if (args[2] & 128)
        {
          tag = 0;
          lightTag = args[0];
        }
        else
        {
          tag = args[0];
          lightTag = 0;
        }

        buttonSuccess = EV_DoZDoomDoor(type, line, mo, tag, args[1],
                                       args[3] * 35 / 8, args[4], lightTag, boomgen, 0);
      }
      break;
    case zl_pillar_build:
      buttonSuccess = EV_DoZDoomPillar(pillarBuild, line, args[0], P_ArgToSpeed(args[1]),
                                       args[2], 0, NO_CRUSH, false);
      break;
    case zl_pillar_build_and_crush:
      buttonSuccess = EV_DoZDoomPillar(pillarBuild, line, args[0], P_ArgToSpeed(args[1]),
                                       args[2], 0, args[3], P_ArgToCrushType(args[4]));
      break;
    case zl_pillar_open:
      buttonSuccess = EV_DoZDoomPillar(pillarOpen, line, args[0], P_ArgToSpeed(args[1]),
                                       args[2], args[3], NO_CRUSH, false);
      break;
    case zl_elevator_move_to_floor:
      buttonSuccess = EV_DoZDoomElevator(line, elevateCurrent, P_ArgToSpeed(args[1]),
                                         0, args[0]);
      break;
    case zl_elevator_raise_to_nearest:
      buttonSuccess = EV_DoZDoomElevator(line, elevateUp, P_ArgToSpeed(args[1]),
                                         0, args[0]);
      break;
    case zl_elevator_lower_to_nearest:
      buttonSuccess = EV_DoZDoomElevator(line, elevateDown, P_ArgToSpeed(args[1]),
                                         0, args[0]);
      break;
    case zl_floor_and_ceiling_lower_by_value:
      buttonSuccess = EV_DoZDoomElevator(line, elevateLower, P_ArgToSpeed(args[1]),
                                         args[2], args[0]);
      break;
    case zl_floor_and_ceiling_raise_by_value:
      buttonSuccess = EV_DoZDoomElevator(line, elevateRaise, P_ArgToSpeed(args[1]),
                                         args[2], args[0]);
      break;
    case zl_floor_lower_by_value:
      buttonSuccess = EV_DoZDoomFloor(floorLowerByValue, line, args[0], args[1], args[2],
                                      NO_CRUSH, P_ArgToChange(args[3]), false, false);
      break;
    case zl_floor_lower_to_lowest:
      buttonSuccess = EV_DoZDoomFloor(floorLowerToLowest, line, args[0], args[1], 0,
                                      NO_CRUSH, P_ArgToChange(args[2]), false, false);
      break;
    case zl_floor_lower_to_highest:
      buttonSuccess = EV_DoZDoomFloor(floorLowerToHighest, line, args[0], args[1],
                                      args[2] - 128, NO_CRUSH, 0, false, args[3] == 1);
      break;
    case zl_floor_lower_to_highest_ee:
      buttonSuccess = EV_DoZDoomFloor(floorLowerToHighest, line, args[0], args[1], 0,
                                      NO_CRUSH, P_ArgToChange(args[2]), false, false);
      break;
    case zl_floor_lower_to_nearest:
      buttonSuccess = EV_DoZDoomFloor(floorLowerToNearest, line, args[0], args[1], 0,
                                      NO_CRUSH, P_ArgToChange(args[2]), false, false);
      break;
    case zl_floor_raise_by_value:
      buttonSuccess = EV_DoZDoomFloor(floorRaiseByValue, line, args[0], args[1], args[2],
                                      P_ArgToCrush(args[4]), P_ArgToChange(args[3]), true, false);
      break;
    case zl_floor_raise_to_highest:
      buttonSuccess = EV_DoZDoomFloor(floorRaiseToHighest, line, args[0], args[1], 0,
                                      P_ArgToCrush(args[3]), P_ArgToChange(args[2]), true, false);
      break;
    case zl_floor_raise_to_nearest:
      buttonSuccess = EV_DoZDoomFloor(floorRaiseToNearest, line, args[0], args[1], 0,
                                      P_ArgToCrush(args[3]), P_ArgToChange(args[2]), true, false);
      break;
    case zl_floor_raise_to_lowest:
      buttonSuccess = EV_DoZDoomFloor(floorRaiseToLowest, line, args[0], 2, 0,
                                      P_ArgToCrush(args[3]), P_ArgToChange(args[2]), true, false);
      break;
    case zl_floor_raise_and_crush:
      buttonSuccess = EV_DoZDoomFloor(floorRaiseAndCrush, line, args[0], args[1], 0,
                                      args[2], 0, P_ArgToCrushType(args[3]), false);
      break;
    case zl_floor_raise_and_crushdoom:
      buttonSuccess = EV_DoZDoomFloor(floorRaiseAndCrushDoom, line, args[0], args[1], 0,
                                      args[2], 0, P_ArgToCrushType(args[3]), false);
      break;
    case zl_floor_raise_by_value_times_8:
      buttonSuccess = EV_DoZDoomFloor(floorRaiseByValue, line, args[0], args[1], args[2] * 8,
                                      P_ArgToCrush(args[4]), P_ArgToChange(args[3]), true, false);
      break;
    case zl_floor_lower_by_value_times_8:
      buttonSuccess = EV_DoZDoomFloor(floorLowerByValue, line, args[0], args[1], args[2] * 8,
                                      NO_CRUSH, P_ArgToChange(args[3]), false, false);
      break;
    case zl_floor_lower_instant:
      buttonSuccess = EV_DoZDoomFloor(floorLowerInstant, line, args[0], 0, args[2] * 8,
                                      NO_CRUSH, P_ArgToChange(args[3]), false, false);
      break;
    case zl_floor_raise_instant:
      buttonSuccess = EV_DoZDoomFloor(floorRaiseInstant, line, args[0], 0, args[2] * 8,
                                      P_ArgToCrush(args[4]), P_ArgToChange(args[3]), true, false);
      break;
    case zl_floor_to_ceiling_instant:
      buttonSuccess = EV_DoZDoomFloor(floorLowerToCeiling, line, args[0], 0, args[3],
                                      P_ArgToCrush(args[2]), P_ArgToChange(args[1]), true, false);
      break;
    case zl_floor_move_to_value:
      buttonSuccess = EV_DoZDoomFloor(floorMoveToValue, line, args[0], args[1],
                                      args[2] * (args[3] ? -1 : 1),
                                      NO_CRUSH, P_ArgToChange(args[4]), false, false);
      break;
    case zl_floor_move_to_value_times_8:
      buttonSuccess = EV_DoZDoomFloor(floorMoveToValue, line, args[0], args[1],
                                      args[2] * 8 * (args[3] ? -1 : 1),
                                      NO_CRUSH, P_ArgToChange(args[4]), false, false);
      break;
    case zl_floor_move_to_value_and_crush:
      buttonSuccess = EV_DoZDoomFloor(floorMoveToValue, line, args[0], args[1],
                                      args[2], P_ArgToCrush(args[3]),
                                      0, P_ArgToCrushType(args[4]), false);
      break;
    case zl_floor_raise_to_lowest_ceiling:
      buttonSuccess = EV_DoZDoomFloor(floorRaiseToLowestCeiling, line, args[0], args[1], 0,
                                      P_ArgToCrush(args[3]), P_ArgToChange(args[2]), true, false);
      break;
    case zl_floor_lower_to_lowest_ceiling:
      buttonSuccess = EV_DoZDoomFloor(floorLowerToLowestCeiling, line, args[0], args[1], args[4],
                                      NO_CRUSH, P_ArgToChange(args[2]), true, false);
      break;
    case zl_floor_raise_by_texture:
      buttonSuccess = EV_DoZDoomFloor(floorRaiseByTexture, line, args[0], args[1], 0,
                                      P_ArgToCrush(args[3]), P_ArgToChange(args[2]), true, false);
      break;
    case zl_floor_lower_by_texture:
      buttonSuccess = EV_DoZDoomFloor(floorLowerByTexture, line, args[0], args[1], 0,
                                      NO_CRUSH, P_ArgToChange(args[2]), true, false);
      break;
    case zl_floor_raise_to_ceiling:
      buttonSuccess = EV_DoZDoomFloor(floorRaiseToCeiling, line, args[0], args[1], args[4],
                                      P_ArgToCrush(args[3]), P_ArgToChange(args[2]), true, false);
      break;
    case zl_floor_raise_by_value_tx_ty:
      buttonSuccess = EV_DoZDoomFloor(floorRaiseAndChange, line, args[0], args[1], args[2],
                                      NO_CRUSH, 0, false, false);
      break;
    case zl_floor_lower_to_lowest_tx_ty:
      buttonSuccess = EV_DoZDoomFloor(floorLowerAndChange, line, args[0], args[1], args[2],
                                      NO_CRUSH, 0, false, false);
      break;
    case zl_generic_floor:
      {
        floor_e type;
        dboolean raise_or_lower;
        byte index;

        static floor_e floor_type[2][7] = {
          {
            floorLowerByValue,
            floorLowerToHighest,
            floorLowerToLowest,
            floorLowerToNearest,
            floorLowerToLowestCeiling,
            floorLowerToCeiling,
            floorLowerByTexture,
          },
          {
            floorRaiseByValue,
            floorRaiseToHighest,
            floorRaiseToLowest,
            floorRaiseToNearest,
            floorRaiseToLowestCeiling,
            floorRaiseToCeiling,
            floorRaiseByTexture,
          }
        };

        raise_or_lower = (args[4] & 8) >> 3;
        index = (args[3] < 7) ? args[3] : 0;
        type = floor_type[raise_or_lower][index];

        buttonSuccess = EV_DoZDoomFloor(type, line, args[0], args[1], args[2],
                                        (args[4] & 16) ? 20 : NO_CRUSH, args[4] & 7, false, false);
      }
      break;
    case zl_floor_crush_stop:
      buttonSuccess = EV_ZDoomFloorCrushStop(args[0]);
      break;
    case zl_floor_stop:
      buttonSuccess = EV_ZDoomFloorStop(args[0], line);
      break;
    case zl_floor_donut:
      buttonSuccess = EV_DoZDoomDonut(args[0], line, P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]));
      break;
    case zl_ceiling_lower_by_value:
      buttonSuccess = EV_DoZDoomCeiling(ceilLowerByValue, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        args[2], P_ArgToCrush(args[4]), 0,
                                        P_ArgToChange(args[3]), false);
      break;
    case zl_ceiling_raise_by_value:
      buttonSuccess = EV_DoZDoomCeiling(ceilRaiseByValue, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        args[2], P_ArgToCrush(args[4]), 0,
                                        P_ArgToChange(args[3]), false);
      break;
    case zl_ceiling_lower_by_value_times_8:
      buttonSuccess = EV_DoZDoomCeiling(ceilLowerByValue, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        args[2] * 8, NO_CRUSH, 0,
                                        P_ArgToChange(args[3]), false);
      break;
    case zl_ceiling_raise_by_value_times_8:
      buttonSuccess = EV_DoZDoomCeiling(ceilRaiseByValue, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        args[2] * 8, NO_CRUSH, 0,
                                        P_ArgToChange(args[3]), false);
      break;
    case zl_ceiling_crush_and_raise:
      buttonSuccess = EV_DoZDoomCeiling(ceilCrushAndRaise, line, args[0],
                                        P_ArgToSpeed(args[1]), P_ArgToSpeed(args[1]) / 2,
                                        8, args[2], 0,
                                        0, P_ArgToCrushMode(args[3], false));
      break;
    case zl_ceiling_lower_and_crush:
      buttonSuccess = EV_DoZDoomCeiling(ceilLowerAndCrush, line, args[0],
                                        P_ArgToSpeed(args[1]), P_ArgToSpeed(args[1]),
                                        8, args[2], 0,
                                        0, P_ArgToCrushMode(args[3], args[1] == 8));
      break;
    case zl_ceiling_lower_and_crush_dist:
      buttonSuccess = EV_DoZDoomCeiling(ceilLowerAndCrush, line, args[0],
                                        P_ArgToSpeed(args[1]), P_ArgToSpeed(args[1]),
                                        args[3], args[2], 0,
                                        0, P_ArgToCrushMode(args[4], args[1] == 8));
      break;
    case zl_ceiling_crush_raise_and_stay:
      buttonSuccess = EV_DoZDoomCeiling(ceilCrushRaiseAndStay, line, args[0],
                                        P_ArgToSpeed(args[1]), P_ArgToSpeed(args[1]) / 2,
                                        8, args[2], 0,
                                        0, P_ArgToCrushMode(args[3], false));
      break;
    case zl_ceiling_move_to_value_times_8:
      buttonSuccess = EV_DoZDoomCeiling(ceilMoveToValue, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        args[2] * 8 * (args[3] ? -1 : 1), NO_CRUSH, 0,
                                        P_ArgToChange(args[4]), false);
      break;
    case zl_ceiling_move_to_value:
      buttonSuccess = EV_DoZDoomCeiling(ceilMoveToValue, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        args[2] * (args[3] ? -1 : 1), NO_CRUSH, 0,
                                        P_ArgToChange(args[4]), false);
      break;
    case zl_ceiling_move_to_value_and_crush:
      buttonSuccess = EV_DoZDoomCeiling(ceilMoveToValue, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        args[2], P_ArgToCrush(args[3]), 0,
                                        0, P_ArgToCrushMode(args[4], args[1] == 8));
      break;
    case zl_ceiling_lower_to_highest_floor:
      buttonSuccess = EV_DoZDoomCeiling(ceilLowerToHighestFloor, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        args[4], P_ArgToCrush(args[3]), 0,
                                        P_ArgToChange(args[2]), false);
      break;
    case zl_ceiling_lower_instant:
      buttonSuccess = EV_DoZDoomCeiling(ceilLowerInstant, line, args[0],
                                        0, 0,
                                        args[2] * 8, P_ArgToCrush(args[4]), 0,
                                        P_ArgToChange(args[3]), false);
      break;
    case zl_ceiling_raise_instant:
      buttonSuccess = EV_DoZDoomCeiling(ceilRaiseInstant, line, args[0],
                                        0, 0,
                                        args[2] * 8, NO_CRUSH, 0,
                                        P_ArgToChange(args[3]), false);
      break;
    case zl_ceiling_crush_raise_and_stay_a:
      buttonSuccess = EV_DoZDoomCeiling(ceilCrushRaiseAndStay, line, args[0],
                                        P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]),
                                        0, args[3], 0,
                                        0, P_ArgToCrushMode(args[4], false));
      break;
    case zl_ceiling_crush_raise_and_stay_sil_a:
      buttonSuccess = EV_DoZDoomCeiling(ceilCrushRaiseAndStay, line, args[0],
                                        P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]),
                                        0, args[3], 1,
                                        0, P_ArgToCrushMode(args[4], false));
      break;
    case zl_ceiling_crush_and_raise_a:
      buttonSuccess = EV_DoZDoomCeiling(ceilCrushAndRaise, line, args[0],
                                        P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]),
                                        0, args[3], 0,
                                        0, P_ArgToCrushMode(args[4], args[1] == 8 && args[2] == 8));
      break;
    case zl_ceiling_crush_and_raise_dist:
      buttonSuccess = EV_DoZDoomCeiling(ceilCrushAndRaise, line, args[0],
                                        P_ArgToSpeed(args[2]), P_ArgToSpeed(args[2]),
                                        args[1], args[3], 0,
                                        0, P_ArgToCrushMode(args[4], args[2] == 8));
      break;
    case zl_ceiling_crush_and_raise_silent_a:
      buttonSuccess = EV_DoZDoomCeiling(ceilCrushAndRaise, line, args[0],
                                        P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]),
                                        0, args[3], 1,
                                        0, P_ArgToCrushMode(args[4], args[1] == 8 && args[2] == 8));
      break;
    case zl_ceiling_crush_and_raise_silent_dist:
      buttonSuccess = EV_DoZDoomCeiling(ceilCrushAndRaise, line, args[0],
                                        P_ArgToSpeed(args[2]), P_ArgToSpeed(args[2]),
                                        args[1], args[3], 1,
                                        0, P_ArgToCrushMode(args[4], args[2] == 8));
      break;
    case zl_ceiling_raise_to_nearest:
      buttonSuccess = EV_DoZDoomCeiling(ceilRaiseToNearest, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        0, NO_CRUSH, P_ArgToChange(args[2]),
                                        0, false);
      break;
    case zl_ceiling_raise_to_highest:
      buttonSuccess = EV_DoZDoomCeiling(ceilRaiseToHighest, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        0, NO_CRUSH, P_ArgToChange(args[2]),
                                        0, false);
      break;
    case zl_ceiling_raise_to_lowest:
      buttonSuccess = EV_DoZDoomCeiling(ceilRaiseToLowest, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        0, NO_CRUSH, P_ArgToChange(args[2]),
                                        0, false);
      break;
    case zl_ceiling_raise_to_highest_floor:
      buttonSuccess = EV_DoZDoomCeiling(ceilRaiseToHighestFloor, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        0, NO_CRUSH, P_ArgToChange(args[2]),
                                        0, false);
      break;
    case zl_ceiling_raise_by_texture:
      buttonSuccess = EV_DoZDoomCeiling(ceilRaiseByTexture, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        0, NO_CRUSH, P_ArgToChange(args[2]),
                                        0, false);
      break;
    case zl_ceiling_lower_to_lowest:
      buttonSuccess = EV_DoZDoomCeiling(ceilLowerToLowest, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        0, P_ArgToCrush(args[3]), 0,
                                        P_ArgToChange(args[2]), false);
      break;
    case zl_ceiling_lower_to_nearest:
      buttonSuccess = EV_DoZDoomCeiling(ceilLowerToNearest, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        0, P_ArgToCrush(args[3]), 0,
                                        P_ArgToChange(args[2]), false);
      break;
    case zl_ceiling_to_highest_instant:
      buttonSuccess = EV_DoZDoomCeiling(ceilLowerToHighest, line, args[0],
                                        2 * FRACUNIT, 0,
                                        0, P_ArgToCrush(args[2]), 0,
                                        P_ArgToChange(args[1]), false);
      break;
    case zl_ceiling_to_floor_instant:
      buttonSuccess = EV_DoZDoomCeiling(ceilRaiseToFloor, line, args[0],
                                        2 * FRACUNIT, 0,
                                        args[3], P_ArgToCrush(args[2]), 0,
                                        P_ArgToChange(args[1]), false);
      break;
    case zl_ceiling_lower_to_floor:
      buttonSuccess = EV_DoZDoomCeiling(ceilLowerToFloor, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        args[4], P_ArgToCrush(args[3]), 0,
                                        P_ArgToChange(args[4]), false);
      break;
    case zl_ceiling_lower_by_texture:
      buttonSuccess = EV_DoZDoomCeiling(ceilLowerByTexture, line, args[0],
                                        P_ArgToSpeed(args[1]), 0,
                                        0, P_ArgToCrush(args[3]), 0,
                                        P_ArgToChange(args[4]), false);
      break;
    case zl_generic_ceiling:
      {
        ceiling_e type;
        dboolean raise_or_lower;
        byte index;

        static ceiling_e ceiling_type[2][7] = {
          {
            ceilLowerByValue,
            ceilLowerToHighest,
            ceilLowerToLowest,
            ceilLowerToNearest,
            ceilLowerToHighestFloor,
            ceilLowerToFloor,
            ceilLowerByTexture,
          },
          {
            ceilRaiseByValue,
            ceilRaiseToHighest,
            ceilRaiseToLowest,
            ceilRaiseToNearest,
            ceilRaiseToHighestFloor,
            ceilRaiseToFloor,
            ceilRaiseByTexture,
          }
        };

        raise_or_lower = (args[4] & 8) >> 3;
        index = (args[3] < 7) ? args[3] : 0;
        type = ceiling_type[raise_or_lower][index];

        buttonSuccess = EV_DoZDoomCeiling(type, line, args[0],
                                          P_ArgToSpeed(args[1]), P_ArgToSpeed(args[1]),
                                          args[2], (args[4] & 16) ? 20 : NO_CRUSH, 0,
                                          args[4] & 7, false);
      }
      break;
    case zl_ceiling_crush_stop:
      {
        dboolean remove;

        switch (args[3])
        {
          case 1:
            remove = false;
            break;
          case 2:
            remove = true;
            break;
          default:
            remove = hexen;
            break;
        }

        buttonSuccess = EV_ZDoomCeilingCrushStop(args[0], remove);
      }
      break;
    case zl_ceiling_stop:
      buttonSuccess = EV_ZDoomCeilingStop(args[0], line);
      break;
    case zl_generic_crusher:
      buttonSuccess = EV_DoZDoomCeiling(ceilCrushAndRaise, line, args[0],
                                        P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]),
                                        0, args[4], args[3] ? 2 : 0,
                                        0, (args[1] <= 24 && args[2] <= 24) ? crushSlowdown : crushDoom);
      break;
    case zl_generic_crusher2:
      buttonSuccess = EV_DoZDoomCeiling(ceilCrushAndRaise, line, args[0],
                                        P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]),
                                        0, args[4], args[3] ? 2 : 0,
                                        0, crushHexen);
      break;
    case zl_floor_waggle:
      buttonSuccess = EV_StartPlaneWaggle(args[0], line, args[1], args[2], args[3], args[4], false);
      break;
    case zl_ceiling_waggle:
      buttonSuccess = EV_StartPlaneWaggle(args[0], line, args[1], args[2], args[3], args[4], true);
      break;
    case zl_floor_and_ceiling_lower_raise:
      buttonSuccess = EV_DoZDoomCeiling(ceilRaiseToHighest, line, args[0],
                                        P_ArgToSpeed(args[2]), 0, 0, 0, 0, 0, false);
      buttonSuccess |= EV_DoZDoomFloor(floorLowerToLowest, line, args[0],
                                       args[1], 0, NO_CRUSH, 0, false, false);
      break;
    case zl_stairs_build_down:
      buttonSuccess = EV_BuildZDoomStairs(args[0], stairBuildDown, line,
                                          args[2], P_ArgToSpeed(args[1]), args[3],
                                          args[4], 0, STAIR_USE_SPECIALS);
      break;
    case zl_stairs_build_up:
      buttonSuccess = EV_BuildZDoomStairs(args[0], stairBuildUp, line,
                                          args[2], P_ArgToSpeed(args[1]), args[3],
                                          args[4], 0, STAIR_USE_SPECIALS);
      break;
    case zl_stairs_build_down_sync:
      buttonSuccess = EV_BuildZDoomStairs(args[0], stairBuildDown, line,
                                          args[2], P_ArgToSpeed(args[1]), 0,
                                          args[3], 0, STAIR_USE_SPECIALS | STAIR_SYNC);
      break;
    case zl_stairs_build_up_sync:
      buttonSuccess = EV_BuildZDoomStairs(args[0], stairBuildUp, line,
                                          args[2], P_ArgToSpeed(args[1]), 0,
                                          args[3], 0, STAIR_USE_SPECIALS | STAIR_SYNC);
      break;
    case zl_stairs_build_down_doom:
      buttonSuccess = EV_BuildZDoomStairs(args[0], stairBuildDown, line,
                                          args[2], P_ArgToSpeed(args[1]), args[3],
                                          args[4], 0, 0);
      break;
    case zl_stairs_build_up_doom:
      buttonSuccess = EV_BuildZDoomStairs(args[0], stairBuildUp, line,
                                          args[2], P_ArgToSpeed(args[1]), args[3],
                                          args[4], 0, 0);
      break;
    case zl_stairs_build_down_doom_sync:
      buttonSuccess = EV_BuildZDoomStairs(args[0], stairBuildDown, line,
                                          args[2], P_ArgToSpeed(args[1]), 0,
                                          args[3], 0, STAIR_SYNC);
      break;
    case zl_stairs_build_up_doom_sync:
      buttonSuccess = EV_BuildZDoomStairs(args[0], stairBuildUp, line,
                                          args[2], P_ArgToSpeed(args[1]), 0,
                                          args[3], 0, STAIR_SYNC);
      break;
    case zl_stairs_build_up_doom_crush:
      buttonSuccess = EV_BuildZDoomStairs(args[0], stairBuildUp, line,
                                          args[2], P_ArgToSpeed(args[1]), args[3],
                                          args[4], 0, STAIR_CRUSH);
      break;
    case zl_generic_stairs:
      {
        stair_e type;

        type = (args[3] & 1) ? stairBuildUp : stairBuildDown;
        buttonSuccess = EV_BuildZDoomStairs(args[0], type, line,
                                            args[2], P_ArgToSpeed(args[1]), 0,
                                            args[4], args[3] & 2, 0);

        // Toggle direction of next activation of repeatable stairs
        if (buttonSuccess && line &&
            line->flags & ML_REPEATSPECIAL &&
            line->special == zl_generic_stairs)
        {
          line->special_args[3] ^= 1;
        }
      }
      break;
    case zl_plat_stop:
      {
        dboolean remove;

        switch (args[3])
        {
          case 1:
            remove = false;
            break;
          case 2:
            remove = true;
            break;
          default:
            remove = hexen;
            break;
        }

        EV_StopZDoomPlat(args[0], remove);
        buttonSuccess = 1;
      }
      break;
    case zl_plat_perpetual_raise:
      buttonSuccess = EV_DoZDoomPlat(args[0], line, platPerpetualRaise, 0,
                                     P_ArgToSpeed(args[1]), args[2], 8, 0);
      break;
    case zl_plat_perpetual_raise_lip:
      buttonSuccess = EV_DoZDoomPlat(args[0], line, platPerpetualRaise, 0,
                                     P_ArgToSpeed(args[1]), args[2], args[3], 0);
      break;
    case zl_plat_down_wait_up_stay:
      buttonSuccess = EV_DoZDoomPlat(args[0], line, platDownWaitUpStay, 0,
                                     P_ArgToSpeed(args[1]), args[2], 8, 0);
      break;
    case zl_plat_down_wait_up_stay_lip:
      buttonSuccess = EV_DoZDoomPlat(args[0], line,
                                     args[4] ? platDownWaitUpStayStone : platDownWaitUpStay, 0,
                                     P_ArgToSpeed(args[1]), args[2], args[3], 0);
      break;
    case zl_plat_down_by_value:
      buttonSuccess = EV_DoZDoomPlat(args[0], line, platDownByValue, args[3] * 8,
                                     P_ArgToSpeed(args[1]), args[2], 0, 0);
      break;
    case zl_plat_up_by_value:
      buttonSuccess = EV_DoZDoomPlat(args[0], line, platUpByValue, args[3] * 8,
                                     P_ArgToSpeed(args[1]), args[2], 0, 0);
      break;
    case zl_plat_up_wait_down_stay:
      buttonSuccess = EV_DoZDoomPlat(args[0], line, platUpWaitDownStay, 0,
                                     P_ArgToSpeed(args[1]), args[2], 0, 0);
      break;
    case zl_plat_up_nearest_wait_down_stay:
      buttonSuccess = EV_DoZDoomPlat(args[0], line, platUpNearestWaitDownStay, 0,
                                     P_ArgToSpeed(args[1]), args[2], 0, 0);
      break;
    case zl_plat_raise_and_stay_tx0:
      {
        plattype_e type;

        switch (args[3])
        {
          case 1:
            type = platRaiseAndStay;
            break;
          case 2:
            type = platRaiseAndStayLockout;
            break;
          default:
            type = (heretic ? platRaiseAndStayLockout : platRaiseAndStay);
            break;
        }

        buttonSuccess = EV_DoZDoomPlat(args[0], line, type, 0, P_ArgToSpeed(args[1]), 0, 0, 1);
      }
      break;
    case zl_plat_up_by_value_stay_tx:
      buttonSuccess = EV_DoZDoomPlat(args[0], line, platUpByValueStay, args[2] * 8,
                                     P_ArgToSpeed(args[1]), 0, 0, 2);
      break;
    case zl_plat_toggle_ceiling:
      buttonSuccess = EV_DoZDoomPlat(args[0], line, platToggle, 0, 0, 0, 0, 0);
      break;
    case zl_generic_lift:
      {
        plattype_e type;

        switch (args[3])
        {
          case 1:
            type = platDownWaitUpStay;
            break;
          case 2:
            type = platDownToNearestFloor;
            break;
          case 3:
            type = platDownToLowestCeiling;
            break;
          case 4:
            type = platPerpetualRaise;
            break;
          default:
            type = platUpByValue;
            break;
        }

        buttonSuccess = EV_DoZDoomPlat(args[0], line, type, args[4] * 8,
                                       P_ArgToSpeed(args[1]), args[2] * 35 / 8, 0, 0);
      }
      break;
    case zl_line_set_blocking:
      if (args[0])
      {
        int i;
        const int *id_p;
        static const int flags[] =
        {
          ML_BLOCKING,
          ML_BLOCKMONSTERS,
          ML_BLOCKPLAYERS,
          ML_BLOCKFLOATERS,
          ML_BLOCKPROJECTILES,
          ML_BLOCKEVERYTHING,
          ML_JUMPOVER,
          ML_BLOCKUSE,
          ML_BLOCKSIGHT,
          ML_BLOCKHITSCAN,
          ML_SOUNDBLOCK,
          -1
        };

        int setflags = 0;
        int clearflags = 0;

        for (i = 0; flags[i] != -1; i++, args[1] >>= 1, args[2] >>= 1)
        {
          if (args[1] & 1) setflags |= flags[i];
          if (args[2] & 1) clearflags |= flags[i];
        }

        for (id_p = dsda_FindLinesFromID(args[0]); *id_p >= 0; id_p++)
        {
          lines[*id_p].flags = (lines[*id_p].flags & ~clearflags) | setflags;
        }

        buttonSuccess = 1;
      }
      break;
    case zl_sector_change_flags:
      if (args[0])
      {
        int i;
        const int *id_p;
        static const int flags[] =
        {
          SECF_SILENT,
          0,
          0,
          0,
          SECF_FRICTION,
          SECF_PUSH,
          0,
          0,
          SECF_ENDGODMODE,
          SECF_ENDLEVEL,
          SECF_HAZARD,
          SECF_NOATTACK,
          -1
        };

        int setflags = 0;
        int clearflags = 0;

        for (i = 0; flags[i] != -1; i++, args[1] >>= 1, args[2] >>= 1)
        {
          if (args[1] & 1) setflags |= flags[i];
          if (args[2] & 1) clearflags |= flags[i];
        }

        FIND_SECTORS(id_p, args[0])
          sectors[*id_p].flags = (sectors[*id_p].flags & ~clearflags) | setflags;

        buttonSuccess = 1;
      }
      break;
    case zl_line_set_automap_flags:
      if (args[0])
      {
        int i;
        const int *id_p;
        static const int flags[] =
        {
          ML_SECRET,
          ML_DONTDRAW,
          ML_MAPPED,
          ML_REVEALED,
          -1
        };

        int setflags = 0;
        int clearflags = 0;

        for (i = 0; flags[i] != -1; i++, args[1] >>= 1, args[2] >>= 1)
        {
          if (args[1] & 1) setflags |= flags[i];
          if (args[2] & 1) clearflags |= flags[i];
        }

        for (id_p = dsda_FindLinesFromID(args[0]); *id_p >= 0; id_p++)
        {
          lines[*id_p].flags = (lines[*id_p].flags & ~clearflags) | setflags;
        }

        buttonSuccess = 1;
      }
      break;
    case zl_line_set_automap_style:
      if (args[1] >= 0 && args[1] <= ams_portal)
      {
        const int *id_p;

        for (id_p = dsda_FindLinesFromID(args[0]); *id_p >= 0; id_p++)
        {
          lines[*id_p].automap_style = args[1];
        }

        buttonSuccess = 1;
      }
      break;
    case zl_scroll_wall:
      if (args[0])
      {
        const int *id_p;
        int side = !!args[3];

        for (id_p = dsda_FindLinesFromID(args[0]); *id_p >= 0; id_p++)
        {
          Add_Scroller(sc_side, args[1], args[2], -1, lines[*id_p].sidenum[side], 0, args[4]);
        }

        buttonSuccess = 1;
      }
      break;
    case zl_line_set_texture_offset:
      if (args[0])
      {
        const int *id_p;
        const int NO_CHANGE = 32767 << FRACBITS;
        int sidenum = !!args[3];

        for (id_p = dsda_FindLinesFromID(args[0]); *id_p >= 0; id_p++)
        {
          side_t *side = &sides[lines[*id_p].sidenum[sidenum]];

          if (args[4] & 8)
          {
            if (args[1] != NO_CHANGE)
            {
              if (args[4] & 1)
                side->textureoffset_top += args[1];

              if (args[4] & 2)
                side->textureoffset_mid += args[1];

              if (args[4] & 4)
                side->textureoffset_bottom += args[1];
            }

            if (args[2] != NO_CHANGE)
            {
              if (args[4] & 1)
                side->rowoffset_top += args[2];

              if (args[4] & 2)
                side->rowoffset_mid += args[2];

              if (args[4] & 4)
                side->rowoffset_bottom += args[2];
            }
          }
          else
          {
            if (args[1] != NO_CHANGE)
            {
              if (args[4] & 1)
                side->textureoffset_top = args[1];

              if (args[4] & 2)
                side->textureoffset_mid = args[1];

              if (args[4] & 4)
                side->textureoffset_bottom = args[1];
            }

            if (args[2] != NO_CHANGE)
            {
              if (args[4] & 1)
                side->rowoffset_top = args[2];

              if (args[4] & 2)
                side->rowoffset_mid = args[2];

              if (args[4] & 4)
                side->rowoffset_bottom = args[2];
            }
          }
        }

        buttonSuccess = 1;
      }
      break;
    case zl_line_set_texturescale:
      if (args[0])
      {
        const int *id_p;
        const int NO_CHANGE = 32767 << FRACBITS;
        int sidenum = !!args[3];

        if (!args[1])
          args[1] = FRACUNIT;

        if (!args[2])
          args[2] = FRACUNIT;

        for (id_p = dsda_FindLinesFromID(args[0]); *id_p >= 0; id_p++)
        {
          side_t *side = &sides[lines[*id_p].sidenum[sidenum]];

          if (args[4] & 8)
          {
            if (args[1] != NO_CHANGE)
            {
              if (args[4] & 1)
                side->scalex_top = FixedMul(side->scalex_top, args[1]);

              if (args[4] & 2)
                side->scalex_mid = FixedMul(side->scalex_mid, args[1]);

              if (args[4] & 4)
                side->scalex_bottom = FixedMul(side->scalex_bottom, args[1]);
            }

            if (args[2] != NO_CHANGE)
            {
              if (args[4] & 1)
                side->scaley_top = FixedMul(side->scaley_top, args[2]);

              if (args[4] & 2)
                side->scaley_mid = FixedMul(side->scaley_mid, args[2]);

              if (args[4] & 4)
                side->scaley_bottom = FixedMul(side->scaley_bottom, args[2]);
            }
          }
          else
          {
            if (args[1] != NO_CHANGE)
            {
              if (args[4] & 1)
                side->scalex_top = args[1];

              if (args[4] & 2)
                side->scalex_mid = args[1];

              if (args[4] & 4)
                side->scalex_bottom = args[1];
            }

            if (args[2] != NO_CHANGE)
            {
              if (args[4] & 1)
                side->scaley_top = args[2];

              if (args[4] & 2)
                side->scaley_mid = args[2];

              if (args[4] & 4)
                side->scaley_bottom = args[2];
            }
          }
        }

        buttonSuccess = 1;
      }
      break;
    case zl_noise_alert:
      {
        extern void P_NoiseAlert(mobj_t *target, mobj_t *emitter);

        mobj_t *target, *emitter;

        if (!args[0])
        {
          target = mo;
        }
        else
        {
          // not supported yet
          target = NULL;
        }

        if (!args[1])
        {
          emitter = mo;
        }
        else
        {
          // not supported yet
          emitter = NULL;
        }

        if (emitter)
        {
          P_NoiseAlert(target, emitter);
        }

        buttonSuccess = 1;
      }
      break;
    case zl_sector_set_gravity:
      {
        fixed_t gravity;
        const int *id_p;

        if (args[2] > 99)
          args[2] = 99;

        gravity = P_ArgsToFixed(args[1], args[2]);

        FIND_SECTORS(id_p, args[0])
          sectors[*id_p].gravity = gravity;
      }
      buttonSuccess = 1;
      break;
    case zl_sector_set_damage:
      {
        const int *id_p;
        dboolean unblockable = false;

        if (args[3] == 0)
        {
          if (args[1] < 20)
          {
            args[4] = 0;
            args[3] = 32;
          }
          else if (args[1] < 50)
          {
            args[4] = 5;
            args[3] = 32;
          }
          else
          {
            unblockable = true;
            args[4] = 0;
            args[3] = 1;
          }
        }

        FIND_SECTORS(id_p, args[0])
        {
          sectors[*id_p].damage.amount = args[1];
          sectors[*id_p].damage.interval = args[3];
          sectors[*id_p].damage.leakrate = args[4];
          if (unblockable)
            sectors[*id_p].flags |= SECF_DMGUNBLOCKABLE;
          else
            sectors[*id_p].flags &= ~SECF_DMGUNBLOCKABLE;
        }
      }
      buttonSuccess = 1;
      break;
    case zl_floor_transfer_numeric:
      buttonSuccess = EV_DoChange(line, numChangeOnly, args[0]);
      break;
    case zl_floor_transfer_trigger:
      buttonSuccess = EV_DoChange(line, trigChangeOnly, args[0]);
      break;
    case zl_sector_set_floor_panning:
      {
        const int *id_p;
        fixed_t xoffs, yoffs;

        xoffs = P_ArgsToFixed(args[1], args[2]);
        yoffs = P_ArgsToFixed(args[3], args[4]);

        FIND_SECTORS(id_p, args[0])
        {
          sectors[*id_p].floor_xoffs = xoffs;
          sectors[*id_p].floor_yoffs = yoffs;
        }
      }
      buttonSuccess = 1;
      break;
    case zl_sector_set_ceiling_panning:
      {
        const int *id_p;
        fixed_t xoffs, yoffs;

        xoffs = P_ArgsToFixed(args[1], args[2]);
        yoffs = P_ArgsToFixed(args[3], args[4]);

        FIND_SECTORS(id_p, args[0])
        {
          sectors[*id_p].ceiling_xoffs = xoffs;
          sectors[*id_p].ceiling_yoffs = yoffs;
        }
      }
      buttonSuccess = 1;
      break;
    case zl_sector_set_rotation:
      {
        const int *id_p;
        angle_t floor, ceiling;

        floor = dsda_DegreesToAngle(args[1]);
        ceiling = dsda_DegreesToAngle(args[2]);

        FIND_SECTORS(id_p, args[0])
        {
          sectors[*id_p].floor_rotation = floor;
          sectors[*id_p].ceiling_rotation = ceiling;
        }
      }
      buttonSuccess = 1;
      break;
    case zl_sector_set_floor_scale:
      {
        const int *id_p;
        fixed_t xscale, yscale;

        xscale = (args[1] << FRACBITS) + (args[2] << FRACBITS) / 100;
        yscale = (args[3] << FRACBITS) + (args[4] << FRACBITS) / 100;

        if (xscale)
          xscale = FixedDiv(FRACUNIT, xscale);

        if (yscale)
          yscale = FixedDiv(FRACUNIT, yscale);

        FIND_SECTORS(id_p, args[0])
        {
          sectors[*id_p].floor_xscale = xscale;
          sectors[*id_p].floor_yscale = yscale;
        }
      }
      buttonSuccess = 1;
      break;
    case zl_sector_set_floor_scale2:
      {
        const int *id_p;
        fixed_t xscale, yscale;

        xscale = args[1];
        yscale = args[2];

        if (xscale)
          xscale = FixedDiv(FRACUNIT, xscale);

        if (yscale)
          yscale = FixedDiv(FRACUNIT, yscale);

        FIND_SECTORS(id_p, args[0])
        {
          sectors[*id_p].floor_xscale = xscale;
          sectors[*id_p].floor_yscale = yscale;
        }
      }
      buttonSuccess = 1;
      break;
    case zl_sector_set_ceiling_scale:
      {
        const int *id_p;
        fixed_t xscale, yscale;

        xscale = (args[1] << FRACBITS) + (args[2] << FRACBITS) / 100;
        yscale = (args[3] << FRACBITS) + (args[4] << FRACBITS) / 100;

        if (xscale)
          xscale = FixedDiv(FRACUNIT, xscale);

        if (yscale)
          yscale = FixedDiv(FRACUNIT, yscale);

        FIND_SECTORS(id_p, args[0])
        {
          sectors[*id_p].ceiling_xscale = xscale;
          sectors[*id_p].ceiling_yscale = yscale;
        }
      }
      buttonSuccess = 1;
      break;
    case zl_sector_set_ceiling_scale2:
      {
        const int *id_p;
        fixed_t xscale, yscale;

        xscale = args[1];
        yscale = args[2];

        if (xscale)
          xscale = FixedDiv(FRACUNIT, xscale);

        if (yscale)
          yscale = FixedDiv(FRACUNIT, yscale);

        FIND_SECTORS(id_p, args[0])
        {
          sectors[*id_p].ceiling_xscale = xscale;
          sectors[*id_p].ceiling_yscale = yscale;
        }
      }
      buttonSuccess = 1;
      break;
    case zl_heal_thing:
      if (mo)
      {
        int max = args[1];

        buttonSuccess = 1;

        if (!max || !mo->player)
        {
          P_HealMobj(mo, args[0]);
          break;
        }
        else if (max == 1)
        {
          max = max_soul;
        }

        if (mo->health < max)
        {
          mo->health += P_PlayerHealthIncrease(args[0]);
          if (mo->health > max && max > 0)
          {
            mo->health = max;
          }
          mo->player->health = mo->health;
        }
      }
      break;
    case zl_force_field:
      if (mo)
      {
        P_DamageMobj(mo, NULL, NULL, 16);
        P_ThrustMobj(mo, ANG180 + mo->angle, 2048 * 250);
      }
      buttonSuccess = 1;
      break;
    case zl_clear_force_field:
      {
        const int *id_p;

        FIND_SECTORS(id_p, args[0])
        {
          int i;

          buttonSuccess = 1;

          for (i = 0; i < sectors[*id_p].linecount; i++)
          {
            line_t *line = sectors[*id_p].lines[i];

            if (line->backsector && line->special == zl_force_field)
            {
              line->flags &= ~(ML_BLOCKING | ML_BLOCKEVERYTHING);
              line->special = 0;
              sides[line->sidenum[0]].midtexture = NO_TEXTURE;
              sides[line->sidenum[1]].midtexture = NO_TEXTURE;
            }
          }
        }
      }
      break;
    case zl_exit_normal:
      G_ExitLevel(args[0]);
      buttonSuccess = 1;
      break;
    case zl_exit_secret:
      G_SecretExitLevel(args[0]);
      buttonSuccess = 1;
      break;
    case zl_teleport_new_map:
      if (!side)
      {
        if (P_CanExit(mo))
        {
          int flags;

          flags = args[2] ? LF_SET_ANGLE : 0;

          // TODO: this crashes if the map doesn't exist (gzdoom does a no-op)
          G_Completed(args[0], args[1], flags, mo->angle);
          buttonSuccess = 1;
        }
      }
      break;
    case zl_teleport_end_game:
      if (!side)
      {
        if (P_CanExit(mo))
        {
          G_Completed(LEAVE_VICTORY, LEAVE_VICTORY, 0, 0);
          buttonSuccess = 1;
        }
      }
      break;
    case zl_polyobj_rotate_left:
      buttonSuccess = EV_RotateZDoomPoly(line, args[0], args[1], args[2], 1, false);
      break;
    case zl_polyobj_or_rotate_left:
      buttonSuccess = EV_RotateZDoomPoly(line, args[0], args[1], args[2], 1, true);
      break;
    case zl_polyobj_rotate_right:
      buttonSuccess = EV_RotateZDoomPoly(line, args[0], args[1], args[2], -1, false);
      break;
    case zl_polyobj_or_rotate_right:
      buttonSuccess = EV_RotateZDoomPoly(line, args[0], args[1], args[2], -1, true);
      break;
    case zl_polyobj_move:
      buttonSuccess = EV_MoveZDoomPoly(line, args[0], args[1],
                                       args[2], args[3], false, false);
      break;
    case zl_polyobj_or_move:
      buttonSuccess = EV_MoveZDoomPoly(line, args[0], args[1],
                                       args[2], args[3], false, true);
      break;
    case zl_polyobj_move_times_8:
      buttonSuccess = EV_MoveZDoomPoly(line, args[0], args[1],
                                       args[2], args[3], true, false);
      break;
    case zl_polyobj_or_move_times_8:
      buttonSuccess = EV_MoveZDoomPoly(line, args[0], args[1],
                                       args[2], args[3], true, true);
      break;
    case zl_polyobj_door_swing:
      buttonSuccess = EV_OpenZDoomPolyDoor(line, args[0], args[1],
                                           args[2], args[3], args[4], PODOOR_SWING);
      break;
    case zl_polyobj_door_slide:
      buttonSuccess = EV_OpenZDoomPolyDoor(line, args[0], args[1],
                                           args[2], args[3], args[4], PODOOR_SLIDE);
      break;
    case zl_polyobj_move_to:
      buttonSuccess = EV_MovePolyTo(line, args[0], P_ArgToSpeed(args[1]),
                                    args[2] << FRACBITS, args[3] << FRACBITS, false);
      break;
    case zl_polyobj_or_move_to:
      buttonSuccess = EV_MovePolyTo(line, args[0], P_ArgToSpeed(args[1]),
                                    args[2] << FRACBITS, args[3] << FRACBITS, true);
      break;
    case zl_polyobj_move_to_spot:
      {
        mobj_t *dest;
        thing_id_search_t search;

        dsda_ResetThingIDSearch(&search);
        dest = dsda_FindMobjFromThingID(args[2], &search);

        if (!dest)
        {
          break;
        }

        buttonSuccess = EV_MovePolyTo(line, args[0], P_ArgToSpeed(args[1]),
                                      dest->x, dest->y, false);
      }
      break;
    case zl_polyobj_or_move_to_spot:
      {
        mobj_t *dest;
        thing_id_search_t search;

        dsda_ResetThingIDSearch(&search);
        dest = dsda_FindMobjFromThingID(args[2], &search);

        if (!dest)
        {
          break;
        }

        buttonSuccess = EV_MovePolyTo(line, args[0], P_ArgToSpeed(args[1]),
                                      dest->x, dest->y, true);
      }
      break;
    case zl_polyobj_stop:
      buttonSuccess = EV_StopPoly(args[0]);
      break;
    case zl_radius_quake:
      {
        mobj_t *spawn_location;
        thing_id_search_t search;

        args[0] = BETWEEN(1, 9, args[0]);

        dsda_ResetThingIDSearch(&search);
        while ((spawn_location = dsda_FindMobjFromThingIDOrMobj(args[4], mo, &search)))
        {
          dsda_SpawnQuake(spawn_location, args[0], args[1], args[2], args[3]);
          buttonSuccess = 1;
        }
      }
      break;
    case zl_thing_move:
      {
        mobj_t *target;
        mobj_t *dest;
        thing_id_search_t search;

        dsda_ResetThingIDSearch(&search);
        target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search);
        dsda_ResetThingIDSearch(&search);
        dest = dsda_FindMobjFromThingID(args[1], &search);

        if (target && dest)
        {
          buttonSuccess = P_MoveThing(target, dest->x, dest->y, dest->z, args[2] ? false : true);
        }
      }
      break;
    case zl_teleport_other:
      if (args[0] && args[1])
      {
        mobj_t *target;
        thing_id_search_t search;

        dsda_ResetThingIDSearch(&search);
        while ((target = dsda_FindMobjFromThingID(args[0], &search)))
        {
          buttonSuccess |= map_format.ev_teleport(args[1], 0, NULL, 0, target,
                                                  args[2] ? (TELF_DESTFOG | TELF_SOURCEFOG) :
                                                            TELF_KEEPORIENTATION);
        }
      }
      break;
    case zl_teleport_group:
      buttonSuccess = EV_TeleportGroup(args[0], mo, args[1], args[2], args[3], args[4]);
      break;
    case zl_teleport_in_sector:
      buttonSuccess = EV_TeleportInSector(args[0], args[1], args[2], args[3], args[4]);
      break;
    case zl_teleport:
      {
        int flags = TELF_DESTFOG;

        if (!args[2])
          flags |= TELF_SOURCEFOG;

        buttonSuccess = map_format.ev_teleport(args[0], args[1], line, side, mo, flags);
      }
      break;
    case zl_teleport_no_fog:
      {
        int flags = 0;

        switch (args[1])
        {
          case 0:
            flags |= TELF_KEEPORIENTATION;
            break;

          case 2:
            if (line)
              flags |= TELF_KEEPORIENTATION | TELF_ROTATEBOOM;
            break;

          case 3:
            if (line)
              flags |= TELF_KEEPORIENTATION | TELF_ROTATEBOOMINVERSE;
            break;

          default:
            break;
        }

        if (args[3])
          flags |= TELF_KEEPHEIGHT;

        buttonSuccess = map_format.ev_teleport(args[0], args[2], line, side, mo, flags);
      }
      break;
    case zl_teleport_no_stop:
      {
        int flags = TELF_DESTFOG | TELF_KEEPVELOCITY;

        if (!args[2])
          flags |= TELF_SOURCEFOG;

        buttonSuccess = map_format.ev_teleport(args[0], args[1], line, side, mo, flags);
      }
      break;
    case zl_teleport_zombie_changer:
      if (mo)
      {
        map_format.ev_teleport(args[0], args[1], line, side, mo, 0);
        if (mo->health >= 0 && mo->info->painstate)
        {
          P_SetMobjState(mo, mo->info->painstate);
        }
        buttonSuccess = 1;
      }
      break;
    case zl_teleport_line:
      buttonSuccess = EV_SilentLineTeleport(line, side, mo, args[1], args[2]);
      break;
    case zl_light_raise_by_value:
      EV_LightChange(args[0], args[1]);
      buttonSuccess = 1;
      break;
    case zl_light_lower_by_value:
      EV_LightChange(args[0], - (short) args[1]);
      buttonSuccess = 1;
      break;
    case zl_light_change_to_value:
      EV_LightSet(args[0], args[1]);
      buttonSuccess = 1;
      break;
    case zl_light_min_neighbor:
      EV_LightSetMinNeighbor(args[0]);
      buttonSuccess = 1;
      break;
    case zl_light_max_neighbor:
      EV_LightSetMaxNeighbor(args[0]);
      buttonSuccess = 1;
      break;
    case zl_light_fade:
      EV_StartLightFading(args[0], args[1], args[2]);
      buttonSuccess = 1;
      break;
    case zl_light_glow:
      EV_StartLightGlowing(args[0], args[1], args[2], args[3]);
      buttonSuccess = 1;
      break;
    case zl_light_flicker:
      EV_StartLightFlickering(args[0], args[1], args[2]);
      buttonSuccess = 1;
      break;
    case zl_light_strobe:
      EV_StartZDoomLightStrobing(args[0], args[1], args[2], args[3], args[4]);
      buttonSuccess = 1;
      break;
    case zl_light_strobe_doom:
      EV_StartZDoomLightStrobingDoom(args[0], args[1], args[2]);
      buttonSuccess = 1;
      break;
    case zl_light_stop:
      EV_StopLightEffect(args[0]);
      buttonSuccess = 1;
      break;
    case zl_thing_set_special:
      {
        mobj_t *target;
        thing_id_search_t search;

        dsda_ResetThingIDSearch(&search);
        while ((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
        {
          target->special = args[1];
          target->special_args[0] = args[2];
          target->special_args[1] = args[3];
          target->special_args[2] = args[4];
        }
      }
      buttonSuccess = 1;
      break;
    case zl_thing_spawn:
      buttonSuccess =
        P_SpawnThing(args[0], mo, args[1], P_ArgToAngle(args[2]), true, args[3]);
      break;
    case zl_thing_spawn_no_fog:
      buttonSuccess =
        P_SpawnThing(args[0], mo, args[1], P_ArgToAngle(args[2]), false, args[3]);
      break;
    case zl_thing_spawn_facing:
      buttonSuccess =
        P_SpawnThing(args[0], mo, args[1], ANGLE_MAX, args[2] ? false : true, args[3]);
      break;
    case zl_thing_projectile:
      buttonSuccess = P_SpawnProjectile(args[0], mo, args[1], P_ArgToAngle(args[2]),
                                        P_ArgToSpeed(args[3]), P_ArgToSpeed(args[4]),
                                        0, NULL, 0, 0);
      break;
    case zl_thing_projectile_gravity:
      buttonSuccess = P_SpawnProjectile(args[0], mo, args[1], P_ArgToAngle(args[2]),
                                        P_ArgToSpeed(args[3]), P_ArgToSpeed(args[4]),
                                        0, NULL, 1, 0);
      break;
    case zl_thing_projectile_aimed:
      buttonSuccess = P_SpawnProjectile(args[0], mo, args[1], 0,
                                        P_ArgToSpeed(args[2]), 0,
                                        args[3], mo, 0, args[4]);
      break;
    case zl_thing_projectile_intercept:
      // ZDoom's implementation relies on a bunch of trigonometry
      // I tried converting this to fixed points,
      //   but the calculations easily go out of bounds (dot products).
      // Needs a different implementation, or 64 bit fixed point conversions
      // Falling back on the default aimed behaviour for now
      buttonSuccess = P_SpawnProjectile(args[0], mo, args[1], 0,
                                        P_ArgToSpeed(args[2]), 0,
                                        args[3], mo, 0, args[4]);
      break;
    case zl_thing_stop:
      {
        mobj_t *target;
        thing_id_search_t search;

        dsda_ResetThingIDSearch(&search);
        while ((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
        {
          buttonSuccess = 1;

          target->momx = 0;
          target->momy = 0;
          target->momz = 0;

          if (target->player)
          {
            target->player->momx = 0;
            target->player->momy = 0;
          }
        }
      }
      break;
    case zl_thing_change_tid:
      {
        mobj_t *target;
        thing_id_search_t search;

        dsda_ResetThingIDSearch(&search);
        while ((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
        {
          dsda_RemoveMobjThingID(target);
          target->tid = args[1];
          if (target->tid)
            dsda_AddMobjThingID(target, args[1]);
        }
      }
      buttonSuccess = 1;
      break;
    case zl_thing_hate:
      {
        mobj_t *hater;
        mobj_t *target;
        thing_id_search_t search;
        thing_id_search_t target_search;

        // Currently no support for this arg
        if (args[2])
        {
          break;
        }

        if (!args[0] && mo && mo->player)
        {
          break;
        }

        buttonSuccess = 1;

        dsda_ResetThingIDSearch(&target_search);
        while ((target = dsda_FindMobjFromThingIDOrMobj(args[1], mo, &target_search)))
        {
          if (
            target->flags & MF_SHOOTABLE &&
            target->health > 0 &&
            !(target->flags2 & MF2_DORMANT)
          )
          {
            break;
          }
        }

        if (!target)
        {
          break;
        }

        dsda_ResetThingIDSearch(&search);
        while ((hater = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
        {
          if (
            hater->health > 0 &&
            hater->flags & MF_SHOOTABLE &&
            hater->info->seestate
          )
          {
            while ((target = dsda_FindMobjFromThingIDOrMobj(args[1], mo, &target_search)))
            {
              if (
                target->flags & MF_SHOOTABLE &&
                target->health > 0 &&
                !(target->flags2 & MF2_DORMANT) &&
                target != hater
              )
              {
                break;
              }
            }

            // Restart from beginning of list
            if (!target)
            {
              dsda_ResetThingIDSearch(&target_search);
              while ((target = dsda_FindMobjFromThingIDOrMobj(args[1], mo, &target_search)))
              {
                if (
                  target->flags & MF_SHOOTABLE &&
                  target->health > 0 &&
                  !(target->flags2 & MF2_DORMANT) &&
                  target != hater
                )
                {
                  break;
                }
              }
            }

            // We might have no target if the hater is the only possible target
            if (target)
            {
              P_SetTarget(&hater->lastenemy, hater->target);
              P_SetTarget(&hater->target, target);

              if (!(hater->flags2 & MF2_DORMANT))
              {
                P_SetMobjState(hater, hater->info->seestate);
              }
            }
          }
        }
      }
      break;
    case zl_thing_remove:
      {
        mobj_t *target;
        thing_id_search_t search;

        dsda_ResetThingIDSearch(&search);
        while ((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
        {
          if (!target->player)
          {
            if (target->flags & MF_COUNTKILL)
              dsda_WatchKill(&players[consoleplayer], target);

            P_RemoveMobj(target);
          }
        }
      }
      buttonSuccess = 1;
      break;
    case zl_thing_activate:
      {
        mobj_t *target;
        thing_id_search_t search;

        dsda_ResetThingIDSearch(&search);
        while ((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
        {
          if (target->flags2 & MF2_DORMANT)
          {
            target->flags2 &= ~MF2_DORMANT;
            target->tics = 1;
          }

          buttonSuccess = 1;
        }
      }
      break;
    case zl_thing_deactivate:
      {
        mobj_t *target;
        thing_id_search_t search;

        dsda_ResetThingIDSearch(&search);
        while ((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
        {
          if (!(target->flags2 & MF2_DORMANT))
          {
            target->flags2 |= MF2_DORMANT;
            target->tics = -1;
          }

          buttonSuccess = 1;
        }
      }
      break;
    case zl_thrust_thing:
      {
        fixed_t thrust;
        mobj_t *target;
        thing_id_search_t search;

        thrust = args[1] * FRACUNIT;

        dsda_ResetThingIDSearch(&search);
        while ((target = dsda_FindMobjFromThingIDOrMobj(args[3], mo, &search)))
        {
          P_ThrustMobj(target, P_ArgToAngle(args[0]), thrust);
        }

        buttonSuccess = (args[3] != 0 || mo);
      }
      break;
    case zl_thrust_thing_z:
      {
        fixed_t thrust;
        mobj_t *target;
        thing_id_search_t search;

        thrust = args[1] * FRACUNIT / 4;

        if (args[2])
          thrust = -thrust;

        dsda_ResetThingIDSearch(&search);
        while ((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
        {
          if (!args[3])
            target->momz = thrust;
          else
            target->momz += thrust;

          buttonSuccess = 1;
        }
      }
      break;
    case zl_thing_raise:
      {
        mobj_t *target;
        thing_id_search_t search;

        dsda_ResetThingIDSearch(&search);
        while ((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
        {
          buttonSuccess |= P_RaiseThing(target, NULL);
        }
      }
    	break;
    case zl_damage_thing:
      if (mo)
      {
        if (args[0] < 0)
        {
          P_HealMobj(mo, -args[0]);
        }
        else
        {
          P_DamageMobj(mo, NULL, NULL, args[0] ? args[0] : 10000);
        }
        buttonSuccess = 1;
      }
      break;
    case zl_thing_damage:
      {
        mobj_t *target;
        thing_id_search_t search;

        dsda_ResetThingIDSearch(&search);
        while ((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
        {
          if (target->flags & MF_SHOOTABLE)
          {
            if (args[1] > 0)
            {
              P_DamageMobj(target, NULL, mo, args[1]);
            }
            else
            {
              P_HealMobj(mo, -args[1]);
            }
          }
        }
      }
      buttonSuccess = 1;
      break;
    case zl_thing_destroy:
      if (!args[0] && !args[2])
      {
        P_Massacre();
      }
      else if (!args[0])
      {
        const int *id_p;

        FIND_SECTORS(id_p, args[2])
        {
          msecnode_t *n;
          sector_t *sec;

          sec = &sectors[*id_p];
          for (n = sec->touching_thinglist; n;)
          {
            mobj_t *target = n->m_thing;

            // Not sure if n might be freed when an enemy dies,
            //   so let's get the next node before applying the damage
            n = n->m_snext;

            if (target->flags & MF_SHOOTABLE)
              P_DamageMobj(target, NULL, mo, args[1] ? 10000 : target->health);
          }
        }
      }
      else
      {
        mobj_t *target;
        thing_id_search_t search;

        dsda_ResetThingIDSearch(&search);
        while ((target = dsda_FindMobjFromThingID(args[0], &search)))
        {
          if (
            target->flags & MF_SHOOTABLE &&
            (!args[2] || target->subsector->sector->tag == args[2])
          )
            P_DamageMobj(target, NULL, mo, args[1] ? 10000 : target->health);
        }
      }
      buttonSuccess = 1;
      break;
    default:
      break;
  }

  return buttonSuccess;
}

dboolean P_ExecuteHexenLineSpecial(int special, int * special_args, line_t * line, int side, mobj_t * mo)
{
    byte args[5];
    dboolean buttonSuccess = false;

    COLLAPSE_SPECIAL_ARGS(args, special_args);

    switch (special)
    {
        case 1:                // Poly Start Line
            break;
        case 2:                // Poly Rotate Left
            buttonSuccess = EV_RotatePoly(line, args, 1, false);
            break;
        case 3:                // Poly Rotate Right
            buttonSuccess = EV_RotatePoly(line, args, -1, false);
            break;
        case 4:                // Poly Move
            buttonSuccess = EV_MovePoly(line, args, false, false);
            break;
        case 6:                // Poly Move Times 8
            buttonSuccess = EV_MovePoly(line, args, true, false);
            break;
        case 7:                // Poly Door Swing
            buttonSuccess = EV_OpenPolyDoor(line, args, PODOOR_SWING);
            break;
        case 8:                // Poly Door Slide
            buttonSuccess = EV_OpenPolyDoor(line, args, PODOOR_SLIDE);
            break;
        case 10:               // Door Close
            buttonSuccess = Hexen_EV_DoDoor(line, args, DREV_CLOSE);
            break;
        case 11:               // Door Open
            if (!args[0])
            {
                buttonSuccess = Hexen_EV_VerticalDoor(line, mo);
            }
            else
            {
                buttonSuccess = Hexen_EV_DoDoor(line, args, DREV_OPEN);
            }
            break;
        case 12:               // Door Raise
            if (!args[0])
            {
                buttonSuccess = Hexen_EV_VerticalDoor(line, mo);
            }
            else
            {
                buttonSuccess = Hexen_EV_DoDoor(line, args, DREV_NORMAL);
            }
            break;
        case 13:               // Door Locked_Raise
            if (CheckedLockedDoor(mo, args[3]))
            {
                if (!args[0])
                {
                    buttonSuccess = Hexen_EV_VerticalDoor(line, mo);
                }
                else
                {
                    buttonSuccess = Hexen_EV_DoDoor(line, args, DREV_NORMAL);
                }
            }
            break;
        case 20:               // Floor Lower by Value
            buttonSuccess = Hexen_EV_DoFloor(line, args, FLEV_LOWERFLOORBYVALUE);
            break;
        case 21:               // Floor Lower to Lowest
            buttonSuccess = Hexen_EV_DoFloor(line, args, FLEV_LOWERFLOORTOLOWEST);
            break;
        case 22:               // Floor Lower to Nearest
            buttonSuccess = Hexen_EV_DoFloor(line, args, FLEV_LOWERFLOOR);
            break;
        case 23:               // Floor Raise by Value
            buttonSuccess = Hexen_EV_DoFloor(line, args, FLEV_RAISEFLOORBYVALUE);
            break;
        case 24:               // Floor Raise to Highest
            buttonSuccess = Hexen_EV_DoFloor(line, args, FLEV_RAISEFLOOR);
            break;
        case 25:               // Floor Raise to Nearest
            buttonSuccess = Hexen_EV_DoFloor(line, args, FLEV_RAISEFLOORTONEAREST);
            break;
        case 26:               // Stairs Build Down Normal
            buttonSuccess = Hexen_EV_BuildStairs(line, args, -1, STAIRS_NORMAL);
            break;
        case 27:               // Build Stairs Up Normal
            buttonSuccess = Hexen_EV_BuildStairs(line, args, 1, STAIRS_NORMAL);
            break;
        case 28:               // Floor Raise and Crush
            buttonSuccess = Hexen_EV_DoFloor(line, args, FLEV_RAISEFLOORCRUSH);
            break;
        case 29:               // Build Pillar (no crushing)
            buttonSuccess = EV_BuildPillar(line, args, false);
            break;
        case 30:               // Open Pillar
            buttonSuccess = EV_OpenPillar(line, args);
            break;
        case 31:               // Stairs Build Down Sync
            buttonSuccess = Hexen_EV_BuildStairs(line, args, -1, STAIRS_SYNC);
            break;
        case 32:               // Build Stairs Up Sync
            buttonSuccess = Hexen_EV_BuildStairs(line, args, 1, STAIRS_SYNC);
            break;
        case 35:               // Raise Floor by Value Times 8
            buttonSuccess = Hexen_EV_DoFloor(line, args, FLEV_RAISEBYVALUETIMES8);
            break;
        case 36:               // Lower Floor by Value Times 8
            buttonSuccess = Hexen_EV_DoFloor(line, args, FLEV_LOWERBYVALUETIMES8);
            break;
        case 40:               // Ceiling Lower by Value
            buttonSuccess = Hexen_EV_DoCeiling(line, args, CLEV_LOWERBYVALUE);
            break;
        case 41:               // Ceiling Raise by Value
            buttonSuccess = Hexen_EV_DoCeiling(line, args, CLEV_RAISEBYVALUE);
            break;
        case 42:               // Ceiling Crush and Raise
            buttonSuccess = Hexen_EV_DoCeiling(line, args, CLEV_CRUSHANDRAISE);
            break;
        case 43:               // Ceiling Lower and Crush
            buttonSuccess = Hexen_EV_DoCeiling(line, args, CLEV_LOWERANDCRUSH);
            break;
        case 44:               // Ceiling Crush Stop
            buttonSuccess = Hexen_EV_CeilingCrushStop(line, args);
            break;
        case 45:               // Ceiling Crush Raise and Stay
            buttonSuccess = Hexen_EV_DoCeiling(line, args, CLEV_CRUSHRAISEANDSTAY);
            break;
        case 46:               // Floor Crush Stop
            buttonSuccess = EV_FloorCrushStop(line, args);
            break;
        case 60:               // Plat Perpetual Raise
            buttonSuccess = EV_DoHexenPlat(line, args, PLAT_PERPETUALRAISE, 0);
            break;
        case 61:               // Plat Stop
            Hexen_EV_StopPlat(line, args);
            break;
        case 62:               // Plat Down-Wait-Up-Stay
            buttonSuccess = EV_DoHexenPlat(line, args, PLAT_DOWNWAITUPSTAY, 0);
            break;
        case 63:               // Plat Down-by-Value*8-Wait-Up-Stay
            buttonSuccess = EV_DoHexenPlat(line, args, PLAT_DOWNBYVALUEWAITUPSTAY,
                                      0);
            break;
        case 64:               // Plat Up-Wait-Down-Stay
            buttonSuccess = EV_DoHexenPlat(line, args, PLAT_UPWAITDOWNSTAY, 0);
            break;
        case 65:               // Plat Up-by-Value*8-Wait-Down-Stay
            buttonSuccess = EV_DoHexenPlat(line, args, PLAT_UPBYVALUEWAITDOWNSTAY,
                                      0);
            break;
        case 66:               // Floor Lower Instant * 8
            buttonSuccess = Hexen_EV_DoFloor(line, args, FLEV_LOWERTIMES8INSTANT);
            break;
        case 67:               // Floor Raise Instant * 8
            buttonSuccess = Hexen_EV_DoFloor(line, args, FLEV_RAISETIMES8INSTANT);
            break;
        case 68:               // Floor Move to Value * 8
            buttonSuccess = Hexen_EV_DoFloor(line, args, FLEV_MOVETOVALUETIMES8);
            break;
        case 69:               // Ceiling Move to Value * 8
            buttonSuccess = Hexen_EV_DoCeiling(line, args, CLEV_MOVETOVALUETIMES8);
            break;
        case 70:               // Teleport
            if (side == 0)
            {                   // Only teleport when crossing the front side of a line
                buttonSuccess = EV_HexenTeleport(args[0], mo, true);
            }
            break;
        case 71:               // Teleport, no fog
            if (side == 0)
            {                   // Only teleport when crossing the front side of a line
                buttonSuccess = EV_HexenTeleport(args[0], mo, false);
            }
            break;
        case 72:               // Thrust Mobj
            if (!side)          // Only thrust on side 0
            {
                P_ThrustMobj(mo, args[0] * (ANG90 / 64),
                             args[1] << FRACBITS);
                buttonSuccess = 1;
            }
            break;
        case 73:               // Damage Mobj
            if (args[0])
            {
                P_DamageMobj(mo, NULL, NULL, args[0]);
            }
            else
            {                   // If arg1 is zero, then guarantee a kill
                P_DamageMobj(mo, NULL, NULL, 10000);
            }
            buttonSuccess = 1;
            break;
        case 74:               // Teleport_NewMap
            if (side == 0)
            {                   // Only teleport when crossing the front side of a line
                if (!(mo && mo->player && mo->player->playerstate == PST_DEAD)) // Players must be alive to teleport
                {
                    G_Completed(args[0], args[1], 0, 0);
                    buttonSuccess = true;
                }
            }
            break;
        case 75:               // Teleport_EndGame
            if (side == 0)
            {                   // Only teleport when crossing the front side of a line
                if (!(mo && mo->player && mo->player->playerstate == PST_DEAD)) // Players must be alive to teleport
                {
                    buttonSuccess = true;
                    if (deathmatch)
                    {           // Winning in deathmatch just goes back to map 1
                        G_Completed(1, 0, 0, 0);
                    }
                    else
                    {           // Starts the Finale
                        G_Completed(LEAVE_VICTORY, LEAVE_VICTORY, 0, 0);
                    }
                }
            }
            break;
        case 80:               // ACS_Execute
            buttonSuccess =
                P_StartACS(args[0], args[1], &args[2], mo, line, side);
            break;
        case 81:               // ACS_Suspend
            buttonSuccess = P_SuspendACS(args[0], args[1]);
            break;
        case 82:               // ACS_Terminate
            buttonSuccess = P_TerminateACS(args[0], args[1]);
            break;
        case 83:               // ACS_LockedExecute
            buttonSuccess = P_StartLockedACS(line, args, mo, side);
            break;
        case 90:               // Poly Rotate Left Override
            buttonSuccess = EV_RotatePoly(line, args, 1, true);
            break;
        case 91:               // Poly Rotate Right Override
            buttonSuccess = EV_RotatePoly(line, args, -1, true);
            break;
        case 92:               // Poly Move Override
            buttonSuccess = EV_MovePoly(line, args, false, true);
            break;
        case 93:               // Poly Move Times 8 Override
            buttonSuccess = EV_MovePoly(line, args, true, true);
            break;
        case 94:               // Build Pillar Crush
            buttonSuccess = EV_BuildPillar(line, args, true);
            break;
        case 95:               // Lower Floor and Ceiling
            buttonSuccess = EV_DoFloorAndCeiling(line, args, false);
            break;
        case 96:               // Raise Floor and Ceiling
            buttonSuccess = EV_DoFloorAndCeiling(line, args, true);
            break;
        case 109:              // Force Lightning
            buttonSuccess = true;
            P_ForceLightning();
            break;
        case 110:              // Light Raise by Value
            buttonSuccess = EV_SpawnLight(line, args, LITE_RAISEBYVALUE);
            break;
        case 111:              // Light Lower by Value
            buttonSuccess = EV_SpawnLight(line, args, LITE_LOWERBYVALUE);
            break;
        case 112:              // Light Change to Value
            buttonSuccess = EV_SpawnLight(line, args, LITE_CHANGETOVALUE);
            break;
        case 113:              // Light Fade
            buttonSuccess = EV_SpawnLight(line, args, LITE_FADE);
            break;
        case 114:              // Light Glow
            buttonSuccess = EV_SpawnLight(line, args, LITE_GLOW);
            break;
        case 115:              // Light Flicker
            buttonSuccess = EV_SpawnLight(line, args, LITE_FLICKER);
            break;
        case 116:              // Light Strobe
            buttonSuccess = EV_SpawnLight(line, args, LITE_STROBE);
            break;
        case 120:              // Quake Tremor
            buttonSuccess = A_LocalQuake(args, mo);
            break;
        case 129:              // UsePuzzleItem
            buttonSuccess = EV_LineSearchForPuzzleItem(line, args, mo);
            break;
        case 130:              // Thing_Activate
            buttonSuccess = EV_ThingActivate(args[0]);
            break;
        case 131:              // Thing_Deactivate
            buttonSuccess = EV_ThingDeactivate(args[0]);
            break;
        case 132:              // Thing_Remove
            buttonSuccess = EV_ThingRemove(args[0]);
            break;
        case 133:              // Thing_Destroy
            buttonSuccess = EV_ThingDestroy(args[0]);
            break;
        case 134:              // Thing_Projectile
            buttonSuccess = EV_ThingProjectile(args, 0);
            break;
        case 135:              // Thing_Spawn
            buttonSuccess = EV_ThingSpawn(args, 1);
            break;
        case 136:              // Thing_ProjectileGravity
            buttonSuccess = EV_ThingProjectile(args, 1);
            break;
        case 137:              // Thing_SpawnNoFog
            buttonSuccess = EV_ThingSpawn(args, 0);
            break;
        case 138:              // Floor_Waggle
            buttonSuccess = EV_StartFloorWaggle(args[0], args[1],
                                                args[2], args[3], args[4]);
            break;
        case 140:              // Sector_SoundChange
            buttonSuccess = EV_SectorSoundChange(args);
            break;
        default:
            break;
    }
    return buttonSuccess;
}

static void Hexen_P_SpawnSpecials(void)
{
    sector_t *sector;
    int i;

    //
    //      Init special SECTORs
    //
    sector = sectors;
    for (i = 0; i < numsectors; i++, sector++)
    {
        if (!sector->special)
            continue;
        switch (sector->special)
        {
            case 1:            // Phased light
                // Hardcoded base, use sector->lightlevel as the index
                P_SpawnPhasedLight(sector, 80, -1);
                break;
            case 2:            // Phased light sequence start
                P_SpawnLightSequence(sector, 1);
                break;
                // Specials 3 & 4 are used by the phased light sequences
        }
    }


    //
    //      Init line EFFECTs
    //
    numlinespecials = 0;
    TaggedLineCount = 0;
    for (i = 0; i < numlines; i++)
    {
        switch (lines[i].special)
        {
            case 100:          // Scroll_Texture_Left
            case 101:          // Scroll_Texture_Right
            case 102:          // Scroll_Texture_Up
            case 103:          // Scroll_Texture_Down
                linespeciallist[numlinespecials] = &lines[i];
                numlinespecials++;
                break;
            case 121:          // Line_SetIdentification
                if (lines[i].special_args[0])
                {
                    if (TaggedLineCount == MAX_TAGGED_LINES)
                    {
                        I_Error("P_SpawnSpecials: MAX_TAGGED_LINES "
                                "(%d) exceeded.", MAX_TAGGED_LINES);
                    }
                    TaggedLines[TaggedLineCount].line = &lines[i];
                    TaggedLines[TaggedLineCount++].lineTag = lines[i].special_args[0];
                }
                lines[i].special = 0;
                break;
        }
    }

    //
    //      Init other misc stuff
    //
    P_RemoveAllActiveCeilings();
    P_RemoveAllActivePlats();
    for (i = 0; i < MAXBUTTONS; i++)
        memset(&buttonlist[i], 0, sizeof(button_t));

    // Initialize flat and texture animations
    P_InitFTAnims();
}
