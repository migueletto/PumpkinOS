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
 *      Movement/collision utility functions,
 *      as used by function in p_map.c.
 *      BLOCKMAP Iterator functions,
 *      and some PIT_* functions to use for iteration.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "doomtype.h"
#include "m_bbox.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"
#include "lprintf.h"
#include "g_game.h"
#include "g_overflow.h"
#include "e6y.h"//e6y

#include "dsda/map_format.h"

//
// P_AproxDistance
// Gives an estimation of distance (not exact)
//

fixed_t CONSTFUNC P_AproxDistance(fixed_t dx, fixed_t dy)
{
  dx = D_abs(dx);
  dy = D_abs(dy);
  if (dx < dy)
    return dx+dy-(dx>>1);
  return dx+dy-(dy>>1);
}

//
// P_PointOnLineSide
// Returns 0 or 1
//
// killough 5/3/98: reformatted, cleaned up

int PUREFUNC P_CompatiblePointOnLineSide(fixed_t x, fixed_t y, const line_t *line)
{
  return
    !line->dx ? x <= line->v1->x ? line->dy > 0 : line->dy < 0 :
    !line->dy ? y <= line->v1->y ? line->dx < 0 : line->dx > 0 :
    FixedMul(y-line->v1->y, line->dx>>FRACBITS) >=
    FixedMul(line->dy>>FRACBITS, x-line->v1->x);
}

int PUREFUNC P_ZDoomPointOnLineSide(fixed_t x, fixed_t y, const line_t *line)
{
  return
    !line->dx ? x <= line->v1->x ? line->dy > 0 : line->dy < 0 :
    !line->dy ? y <= line->v1->y ? line->dx < 0 : line->dx > 0 :
    ((long long) y - line->v1->y) * line->dx >= ((long long) x - line->v1->x) * line->dy;
}

int (*P_PointOnLineSide)(fixed_t x, fixed_t y, const line_t *line);

//
// P_BoxOnLineSide
// Considers the line to be infinite
// Returns side 0 or 1, -1 if box crosses the line.
//
// killough 5/3/98: reformatted, cleaned up

int PUREFUNC P_BoxOnLineSide(const fixed_t *tmbox, const line_t *ld)
{
  switch (ld->slopetype)
    {
      int p;
    default: // shut up compiler warnings -- killough
    case ST_HORIZONTAL:
      return
      (tmbox[BOXBOTTOM] > ld->v1->y) == (p = tmbox[BOXTOP] > ld->v1->y) ?
        p ^ (ld->dx < 0) : -1;
    case ST_VERTICAL:
      return
        (tmbox[BOXLEFT] < ld->v1->x) == (p = tmbox[BOXRIGHT] < ld->v1->x) ?
        p ^ (ld->dy < 0) : -1;
    case ST_POSITIVE:
      return
        P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXBOTTOM], ld) ==
        (p = P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXTOP], ld)) ? p : -1;
    case ST_NEGATIVE:
      return
        (P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXBOTTOM], ld)) ==
        (p = P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXTOP], ld)) ? p : -1;
    }
}

//
// P_PointOnDivlineSide
// Returns 0 or 1.
//
// killough 5/3/98: reformatted, cleaned up

int PUREFUNC P_CompatiblePointOnDivlineSide(fixed_t x, fixed_t y, const divline_t *line)
{
  return
    !line->dx ? x <= line->x ? line->dy > 0 : line->dy < 0 :
    !line->dy ? y <= line->y ? line->dx < 0 : line->dx > 0 :
    (line->dy^line->dx^(x -= line->x)^(y -= line->y)) < 0 ? (line->dy^x) < 0 :
    FixedMul(y>>8, line->dx>>8) >= FixedMul(line->dy>>8, x>>8);
}

int PUREFUNC P_ZDoomPointOnDivlineSide(fixed_t x, fixed_t y, const divline_t *line)
{
  return
    !line->dx ? x <= line->x ? line->dy > 0 : line->dy < 0 :
    !line->dy ? y <= line->y ? line->dx < 0 : line->dx > 0 :
    (line->dy^line->dx^(x -= line->x)^(y -= line->y)) < 0 ? (line->dy^x) < 0 :
    (long long) y * line->dx >= (long long) x * line->dy;
}

int (*P_PointOnDivlineSide)(fixed_t x, fixed_t y, const divline_t *line);

//
// P_MakeDivline
//

void P_MakeDivline(const line_t *li, divline_t *dl)
{
  dl->x = li->v1->x;
  dl->y = li->v1->y;
  dl->dx = li->dx;
  dl->dy = li->dy;
}

//
// P_InterceptVector
// Returns the fractional intercept point
// along the first divline.
// This is only called by the addthings
// and addlines traversers.
//

/* cph - this is killough's 4/19/98 version of P_InterceptVector and
 *  P_InterceptVector2 (which were interchangeable). We still use this
 *  in compatibility mode. */
fixed_t PUREFUNC P_InterceptVector2(const divline_t *v2, const divline_t *v1)
{
  fixed_t den;
  return (den = FixedMul(v1->dy>>8, v2->dx) - FixedMul(v1->dx>>8, v2->dy)) ?
    FixedDiv(FixedMul((v1->x - v2->x)>>8, v1->dy) +
             FixedMul((v2->y - v1->y)>>8, v1->dx), den) : 0;
}

fixed_t PUREFUNC P_InterceptVector(const divline_t *v2, const divline_t *v1)
{
  if (compatibility_level < prboom_4_compatibility)
    return P_InterceptVector2(v2, v1);
  else {
    /* cph - This was introduced at prboom_4_compatibility - no precision/overflow problems */
    int64_t den = (int64_t)v1->dy * v2->dx - (int64_t)v1->dx * v2->dy;
    den >>= 16;
    if (!den)
      return 0;
    return (fixed_t)(((int64_t)(v1->x - v2->x) * v1->dy - (int64_t)(v1->y - v2->y) * v1->dx) / den);
  }
}

//
// P_LineOpening
// Sets line_opening to the window through a two sided line.
// OPTIMIZE: keep this precalculated
//

line_opening_t line_opening;

dboolean P_GetMidTexturePosition(const line_t *line, int sideno, fixed_t *top, fixed_t *bottom)
{
  short texnum;
  const side_t *side;

  if (line->sidenum[0] == NO_INDEX || line->sidenum[1] == NO_INDEX)
  {
    return false;
  }

  side = &sides[line->sidenum[0]];

  if (!side->midtexture)
  {
    return false;
  }

  texnum = side->midtexture;
  texnum = texturetranslation[texnum];

  if (line->flags & ML_DONTPEGBOTTOM)
  {
    *bottom = side->rowoffset + side->rowoffset_mid +
                MAX(line->frontsector->floorheight, line->backsector->floorheight);
    *top = *bottom + textureheight[texnum];
  }
  else
  {
    *top = side->rowoffset + side->rowoffset_mid +
             MIN(line->frontsector->ceilingheight, line->backsector->ceilingheight);
    *bottom = *top - textureheight[texnum];
  }

  return true;
}

void P_LineOpening_3dMidtex(const line_t *line, const mobj_t *actor)
{
  fixed_t bottom3d, top3d;

  if (line->flags & ML_3DMIDTEXIMPASSIBLE && actor->flags & (MF_MISSILE | MF_BOUNCES))
  {
    return;
  }

  if (!P_GetMidTexturePosition(line, 0, &top3d, &bottom3d))
  {
    return;
  }

  if (actor->z + actor->height / 2 < (top3d + bottom3d) / 2)
  {
    line_opening.top = bottom3d;
  }
  else
  {
    if (top3d > line_opening.bottom)
    {
      line_opening.bottom = top3d;
      line_opening.abovemidtex = true;
    }
    line_opening.touchmidtex = (abs(actor->z - top3d) < 24 * FRACUNIT);
  }
}

void P_LineOpening(const line_t *linedef, const mobj_t *actor)
{
  extern int tmfloorpic;

  if (linedef->sidenum[1] == NO_INDEX)      // single sided line
  {
    line_opening.range = 0;
    return;
  }

  line_opening.frontsector = linedef->frontsector;
  line_opening.backsector = linedef->backsector;

  if (line_opening.frontsector->ceilingheight < line_opening.backsector->ceilingheight)
    line_opening.top = line_opening.frontsector->ceilingheight;
  else
    line_opening.top = line_opening.backsector->ceilingheight;

  if (line_opening.frontsector->floorheight > line_opening.backsector->floorheight)
  {
    line_opening.bottom = line_opening.frontsector->floorheight;
    line_opening.lowfloor = line_opening.backsector->floorheight;
    tmfloorpic = line_opening.frontsector->floorpic;
  }
  else
  {
    line_opening.bottom = line_opening.backsector->floorheight;
    line_opening.lowfloor = line_opening.frontsector->floorheight;
    tmfloorpic = line_opening.backsector->floorpic;
  }

  line_opening.abovemidtex = false;
  line_opening.touchmidtex = false;

  if (actor && linedef->frontsector && linedef->backsector && linedef->flags & ML_3DMIDTEX)
  {
    P_LineOpening_3dMidtex(linedef, actor);
  }

  line_opening.range = line_opening.top - line_opening.bottom;
}

//
// THING POSITION SETTING
//

//
// P_UnsetThingPosition
// Unlinks a thing from block map and sectors.
// On each position change, BLOCKMAP and other
// lookups maintaining lists ot things inside
// these structures need to be updated.
//

void P_UnsetThingPosition (mobj_t *thing)
{
  if (!(thing->flags & MF_NOSECTOR))
    {
      /* invisible things don't need to be in sector list
       * unlink from subsector
       *
       * killough 8/11/98: simpler scheme using pointers-to-pointers for prev
       * pointers, allows head node pointers to be treated like everything else
       */

      mobj_t **sprev = thing->sprev;
      mobj_t  *snext = thing->snext;
      if ((*sprev = snext))  // unlink from sector list
        snext->sprev = sprev;

        // phares 3/14/98
        //
        // Save the sector list pointed to by touching_sectorlist.
        // In P_SetThingPosition, we'll keep any nodes that represent
        // sectors the Thing still touches. We'll add new ones then, and
        // delete any nodes for sectors the Thing has vacated. Then we'll
        // put it back into touching_sectorlist. It's done this way to
        // avoid a lot of deleting/creating for nodes, when most of the
        // time you just get back what you deleted anyway.
        //
        // If this Thing is being removed entirely, then the calling
        // routine will clear out the nodes in sector_list.

      sector_list = thing->touching_sectorlist;
      thing->touching_sectorlist = NULL; //to be restored by P_SetThingPosition
    }

  if (!(thing->flags & MF_NOBLOCKMAP))
    {
      /* inert things don't need to be in blockmap
       *
       * killough 8/11/98: simpler scheme using pointers-to-pointers for prev
       * pointers, allows head node pointers to be treated like everything else
       *
       * Also more robust, since it doesn't depend on current position for
       * unlinking. Old method required computing head node based on position
       * at time of unlinking, assuming it was the same position as during
       * linking.
       */

      mobj_t *bnext, **bprev = thing->bprev;
      if (bprev && (*bprev = bnext = thing->bnext))  // unlink from block map
        bnext->bprev = bprev;
    }
}

//
// P_SetThingPosition
// Links a thing into both a block and a subsector
// based on it's x y.
// Sets thing->subsector properly
//
// killough 5/3/98: reformatted, cleaned up

void P_SetThingPosition(mobj_t *thing)
{                                                      // link into subsector
  subsector_t *ss = thing->subsector = R_PointInSubsector(thing->x, thing->y);
  if (!(thing->flags & MF_NOSECTOR))
    {
      // invisible things don't go into the sector links

      // killough 8/11/98: simpler scheme using pointer-to-pointer prev
      // pointers, allows head nodes to be treated like everything else

      mobj_t **link = &ss->sector->thinglist;
      mobj_t *snext = *link;
      if ((thing->snext = snext))
        snext->sprev = &thing->snext;
      thing->sprev = link;
      *link = thing;

      // phares 3/16/98
      //
      // If sector_list isn't NULL, it has a collection of sector
      // nodes that were just removed from this Thing.

      // Collect the sectors the object will live in by looking at
      // the existing sector_list and adding new nodes and deleting
      // obsolete ones.

      // When a node is deleted, its sector links (the links starting
      // at sector_t->touching_thinglist) are broken. When a node is
      // added, new sector links are created.

      P_CreateSecNodeList(thing,thing->x,thing->y);
      thing->touching_sectorlist = sector_list; // Attach to Thing's mobj_t
      sector_list = NULL; // clear for next time
    }

  // link into blockmap
  if (!(thing->flags & MF_NOBLOCKMAP))
    {
      // inert things don't need to be in blockmap
      int blockx = P_GetSafeBlockX(thing->x - bmaporgx);
      int blocky = P_GetSafeBlockY(thing->y - bmaporgy);
      if (blockx>=0 && blockx < bmapwidth && blocky>=0 && blocky < bmapheight)
        {
        // killough 8/11/98: simpler scheme using pointer-to-pointer prev
        // pointers, allows head nodes to be treated like everything else

        mobj_t **link = &blocklinks[blocky*bmapwidth+blockx];
        mobj_t *bnext = *link;
        if ((thing->bnext = bnext))
          bnext->bprev = &thing->bnext;
        thing->bprev = link;
        *link = thing;
      }
      else        // thing is off the map
        thing->bnext = NULL, thing->bprev = NULL;
    }
}

//
// BLOCK MAP ITERATORS
// For each line/thing in the given mapblock,
// call the passed PIT_* function.
// If the function returns false,
// exit with false without checking anything else.
//

//
// P_BlockLinesIterator
// The validcount flags are used to avoid checking lines
// that are marked in multiple mapblocks,
// so increment validcount before the first call
// to P_BlockLinesIterator, then make one or more calls
// to it.
//
// killough 5/3/98: reformatted, cleaned up

dboolean P_BlockLinesIterator(int x, int y, dboolean func(line_t*))
{
  int        offset;
  const int  *list;   // killough 3/1/98: for removal of blockmap limit

  if (x<0 || y<0 || x>=bmapwidth || y>=bmapheight)
    return true;
  offset = y*bmapwidth+x;

  if (map_format.polyobjs)
  {
    int i;
    seg_t **tempSeg;
    polyblock_t *polyLink;
    extern polyblock_t **PolyBlockMap;

    polyLink = PolyBlockMap[offset];
    while (polyLink)
    {
      if (polyLink->polyobj)
      {
        if (polyLink->polyobj->validcount != validcount)
        {
          polyLink->polyobj->validcount = validcount;
          tempSeg = polyLink->polyobj->segs;
          for (i = 0; i < polyLink->polyobj->numsegs; i++, tempSeg++)
          {
            if ((*tempSeg)->linedef->validcount == validcount)
            {
              continue;
            }
            (*tempSeg)->linedef->validcount = validcount;
            if (!func((*tempSeg)->linedef))
            {
              return false;
            }
          }
        }
      }
      polyLink = polyLink->next;
    }
  }

  offset = *(blockmap+offset);
  list = blockmaplump+offset;     // original was reading         // phares
                                  // delmiting 0 as linedef 0     // phares

  // killough 1/31/98: for compatibility we need to use the old method.
  // Most demos go out of sync, and maybe other problems happen, if we
  // don't consider linedef 0. For safety this should be qualified.

  // killough 2/22/98: demo_compatibility check
  // In mbf21, skip if all blocklists start w/ 0 (fixes btsx e2 map 20)
  if ((!demo_compatibility && !mbf21) || (mbf21 && skipblstart))
    list++;     // skip 0 starting delimiter                      // phares
  for ( ; *list != -1 ; list++)                                   // phares
    {
      line_t *ld;
#ifdef RANGECHECK
      if(*list < 0 || *list >= numlines)
        I_Error("P_BlockLinesIterator: index >= numlines");
#endif
      ld = &lines[*list];
      if (ld->validcount == validcount)
        continue;       // line has already been checked
      ld->validcount = validcount;
      if (!func(ld))
        return false;
    }
  return true;  // everything was checked
}

// MBF's P_SetThingPosition code injects an increment to validcount
// There is a bug in P_CheckPosition where the validcount is not
// incremented at the correct time. The bug is exposed in MBF.
// Under most cases, this is irrelevant, but sometimes it matters.
// Ripper projectiles in mbf21, heretic, and hexen require decoupling.
dboolean P_BlockLinesIterator2(int x, int y, dboolean func(line_t*))
{
  int        offset;
  const int  *list;   // killough 3/1/98: for removal of blockmap limit

  if (x<0 || y<0 || x>=bmapwidth || y>=bmapheight)
    return true;
  offset = y*bmapwidth+x;

  if (map_format.polyobjs)
  {
    int i;
    seg_t **tempSeg;
    polyblock_t *polyLink;
    extern polyblock_t **PolyBlockMap;

    polyLink = PolyBlockMap[offset];
    while (polyLink)
    {
      if (polyLink->polyobj)
      {
        if (polyLink->polyobj->validcount2 != validcount2)
        {
          polyLink->polyobj->validcount2 = validcount2;
          tempSeg = polyLink->polyobj->segs;
          for (i = 0; i < polyLink->polyobj->numsegs; i++, tempSeg++)
          {
            if ((*tempSeg)->linedef->validcount2 == validcount2)
            {
              continue;
            }
            (*tempSeg)->linedef->validcount2 = validcount2;
            if (!func((*tempSeg)->linedef))
            {
              return false;
            }
          }
        }
      }
      polyLink = polyLink->next;
    }
  }

  offset = *(blockmap+offset);
  list = blockmaplump+offset;     // original was reading         // phares
                                  // delmiting 0 as linedef 0     // phares

  // killough 1/31/98: for compatibility we need to use the old method.
  // Most demos go out of sync, and maybe other problems happen, if we
  // don't consider linedef 0. For safety this should be qualified.

  // killough 2/22/98: demo_compatibility check
  // In mbf21, skip if all blocklists start w/ 0 (fixes btsx e2 map 20)
  if ((!demo_compatibility && !mbf21) || (mbf21 && skipblstart))
    list++;     // skip 0 starting delimiter                      // phares
  for ( ; *list != -1 ; list++)                                   // phares
    {
      line_t *ld;
#ifdef RANGECHECK
      if(*list < 0 || *list >= numlines)
        I_Error("P_BlockLinesIterator2: index >= numlines");
#endif
      ld = &lines[*list];
      if (ld->validcount2 == validcount2)
        continue;       // line has already been checked
      ld->validcount2 = validcount2;
      if (!func(ld))
        return false;
    }
  return true;  // everything was checked
}

//
// P_BlockThingsIterator
//
// killough 5/3/98: reformatted, cleaned up

dboolean P_BlockThingsIterator(int x, int y, dboolean func(mobj_t*))
{
  mobj_t *mobj;
  if (!(x<0 || y<0 || x>=bmapwidth || y>=bmapheight))
    for (mobj = blocklinks[y*bmapwidth+x]; mobj; mobj = mobj->bnext)
      if (!func(mobj))
        return false;
  return true;
}

//
// INTERCEPT ROUTINES
//

// 1/11/98 killough: Intercept limit removed
intercept_t *intercepts, *intercept_p;

// Check for limit and double size if necessary -- killough
void check_intercept(void)
{
  static size_t num_intercepts;
  size_t offset = intercept_p - intercepts;
  if (offset >= num_intercepts)
    {
      num_intercepts = num_intercepts ? num_intercepts*2 : 128;
      intercepts = Z_Realloc(intercepts, sizeof(*intercepts)*num_intercepts);
      intercept_p = intercepts + offset;
    }
}

divline_t trace;

// PIT_AddLineIntercepts.
// Looks for lines in the given block
// that intercept the given trace
// to add to the intercepts list.
//
// A line is crossed if its endpoints
// are on opposite sides of the trace.
//
// killough 5/3/98: reformatted, cleaned up

dboolean PIT_AddLineIntercepts(line_t *ld)
{
  int       s1;
  int       s2;
  fixed_t   frac;
  divline_t dl;

  // avoid precision problems with two routines
  if (trace.dx >  FRACUNIT*16 || trace.dy >  FRACUNIT*16 ||
      trace.dx < -FRACUNIT*16 || trace.dy < -FRACUNIT*16)
    {
      s1 = P_PointOnDivlineSide (ld->v1->x, ld->v1->y, &trace);
      s2 = P_PointOnDivlineSide (ld->v2->x, ld->v2->y, &trace);
    }
  else
    {
      s1 = P_PointOnLineSide (trace.x, trace.y, ld);
      s2 = P_PointOnLineSide (trace.x+trace.dx, trace.y+trace.dy, ld);
    }

  if (s1 == s2)
    return true;        // line isn't crossed

  // hit the line
  P_MakeDivline(ld, &dl);
  frac = P_InterceptVector(&trace, &dl);

  if (frac < 0)
    return true;        // behind source

  check_intercept();    // killough

  intercept_p->frac = frac;
  intercept_p->isaline = true;
  intercept_p->d.line = ld;
  InterceptsOverrun(intercept_p - intercepts, intercept_p);//e6y
  intercept_p++;

  return true;  // continue
}

//
// PIT_AddThingIntercepts
//
// killough 5/3/98: reformatted, cleaned up

dboolean PIT_AddThingIntercepts(mobj_t *thing)
{
  fixed_t   x1, y1;
  fixed_t   x2, y2;
  int       s1, s2;
  divline_t dl;
  fixed_t   frac;

  // check a corner to corner crossection for hit
  if ((trace.dx ^ trace.dy) > 0)
    {
      x1 = thing->x - thing->radius;
      y1 = thing->y + thing->radius;
      x2 = thing->x + thing->radius;
      y2 = thing->y - thing->radius;
    }
  else
    {
      x1 = thing->x - thing->radius;
      y1 = thing->y - thing->radius;
      x2 = thing->x + thing->radius;
      y2 = thing->y + thing->radius;
    }

  s1 = P_PointOnDivlineSide (x1, y1, &trace);
  s2 = P_PointOnDivlineSide (x2, y2, &trace);

  if (s1 == s2)
    return true;                // line isn't crossed

  dl.x = x1;
  dl.y = y1;
  dl.dx = x2-x1;
  dl.dy = y2-y1;

  frac = P_InterceptVector (&trace, &dl);

  if (frac < 0)
    return true;                // behind source

  check_intercept();            // killough

  intercept_p->frac = frac;
  intercept_p->isaline = false;
  intercept_p->d.thing = thing;
  InterceptsOverrun(intercept_p - intercepts, intercept_p);//e6y
  intercept_p++;

  return true;          // keep going
}

//
// P_TraverseIntercepts
// Returns true if the traverser function returns true
// for all lines.
//
// killough 5/3/98: reformatted, cleaned up

dboolean P_TraverseIntercepts(traverser_t func, fixed_t maxfrac)
{
  intercept_t *in = NULL;
  int count = intercept_p - intercepts;
  while (count--)
    {
      fixed_t dist = INT_MAX;
      intercept_t *scan;
      for (scan = intercepts; scan < intercept_p; scan++)
        if (scan->frac < dist)
          dist = (in=scan)->frac;
      if (dist > maxfrac)
        return true;    // checked everything in range
      if (!func(in))
        return false;           // don't bother going farther
      in->frac = INT_MAX;
    }
  return true;                  // everything was traversed
}

//
// P_PathTraverse
// Traces a line from x1,y1 to x2,y2,
// calling the traverser function for each.
// Returns true if the traverser function returns true
// for all lines.
//
// killough 5/3/98: reformatted, cleaned up

dboolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2,
                       int flags, dboolean trav(intercept_t *))
{
  fixed_t xt1, yt1;
  fixed_t xt2, yt2;
  fixed_t xstep, ystep;
  fixed_t partial;
  fixed_t xintercept, yintercept;
  int     mapx, mapy;
  int     mapx1, mapy1;
  int     mapxstep, mapystep;
  int     count;

  validcount++;
  intercept_p = intercepts;

  if (!((x1-bmaporgx)&(MAPBLOCKSIZE-1)))
    x1 += FRACUNIT;     // don't side exactly on a line

  if (!((y1-bmaporgy)&(MAPBLOCKSIZE-1)))
    y1 += FRACUNIT;     // don't side exactly on a line

  trace.x = x1;
  trace.y = y1;
  trace.dx = x2 - x1;
  trace.dy = y2 - y1;

  if (comperr(comperr_blockmap))
  {
    int64_t _x1, _x2, _y1, _y2;

    _x1 = (int64_t)x1 - bmaporgx;
    _y1 = (int64_t)y1 - bmaporgy;
    xt1 = (int)(_x1>>MAPBLOCKSHIFT);
    yt1 = (int)(_y1>>MAPBLOCKSHIFT);

    mapx1 = (int)(_x1>>MAPBTOFRAC);
    mapy1 = (int)(_y1>>MAPBTOFRAC);

    _x2 = (int64_t)x2 - bmaporgx;
    _y2 = (int64_t)y2 - bmaporgy;
    xt2 = (int)(_x2>>MAPBLOCKSHIFT);
    yt2 = (int)(_y2>>MAPBLOCKSHIFT);

    x1 -= bmaporgx;
    y1 -= bmaporgy;
    x2 -= bmaporgx;
    y2 -= bmaporgy;
  }
  else
  {
    x1 -= bmaporgx;
    y1 -= bmaporgy;
    xt1 = x1>>MAPBLOCKSHIFT;
    yt1 = y1>>MAPBLOCKSHIFT;

    mapx1 = x1>>MAPBTOFRAC;
    mapy1 = y1>>MAPBTOFRAC;

    x2 -= bmaporgx;
    y2 -= bmaporgy;
    xt2 = x2>>MAPBLOCKSHIFT;
    yt2 = y2>>MAPBLOCKSHIFT;
  }

  if (xt2 > xt1)
    {
      mapxstep = 1;
      partial = FRACUNIT - (mapx1&(FRACUNIT-1));
      ystep = FixedDiv (y2-y1,D_abs(x2-x1));
    }
  else
    if (xt2 < xt1)
      {
        mapxstep = -1;
        partial = mapx1&(FRACUNIT-1);
        ystep = FixedDiv (y2-y1,D_abs(x2-x1));
      }
    else
      {
        mapxstep = 0;
        partial = FRACUNIT;
        ystep = 256*FRACUNIT;
      }

  yintercept = mapy1 + FixedMul(partial, ystep);

  if (yt2 > yt1)
    {
      mapystep = 1;
      partial = FRACUNIT - (mapy1&(FRACUNIT-1));
      xstep = FixedDiv (x2-x1,D_abs(y2-y1));
    }
  else
    if (yt2 < yt1)
      {
        mapystep = -1;
        partial = mapy1&(FRACUNIT-1);
        xstep = FixedDiv (x2-x1,D_abs(y2-y1));
      }
    else
      {
        mapystep = 0;
        partial = FRACUNIT;
        xstep = 256*FRACUNIT;
      }

  xintercept = mapx1 + FixedMul(partial, xstep);

  // Step through map blocks.
  // Count is present to prevent a round off error
  // from skipping the break.

  mapx = xt1;
  mapy = yt1;

  for (count = 0; count < 64; count++)
    {
      if (flags & PT_ADDLINES)
        if (!P_BlockLinesIterator(mapx, mapy,PIT_AddLineIntercepts))
          return false; // early out

      if (flags & PT_ADDTHINGS)
        if (!P_BlockThingsIterator(mapx, mapy,PIT_AddThingIntercepts))
          return false; // early out

      if (mapx == xt2 && mapy == yt2)
        break;

      if ((yintercept >> FRACBITS) == mapy)
        {
          yintercept += ystep;
          mapx += mapxstep;
        }
      else
        if ((xintercept >> FRACBITS) == mapx)
          {
            xintercept += xstep;
            mapy += mapystep;
          }
    }

  // go through the sorted list
  return P_TraverseIntercepts(trav, FRACUNIT);
}

//
// RoughBlockCheck
// [XA] adapted from Hexen -- used by P_RoughTargetSearch
//

static mobj_t *Hexen_RoughBlockCheck(mobj_t * mo, int index);

static mobj_t *RoughBlockCheck(mobj_t *mo, int index, angle_t fov)
{
  mobj_t *link;

  if (hexen) return Hexen_RoughBlockCheck(mo, index);

  link = blocklinks[index];
  while (link)
  {
    // skip non-shootable actors
    if (!(link->flags & MF_SHOOTABLE))
    {
      link = link->bnext;
      continue;
    }

    // skip dormant actors
    if (link->flags2 & MF2_DORMANT)
    {
        link = link->bnext;
        continue;
    }

    // skip the projectile's owner
    if (link == mo->target)
    {
      link = link->bnext;
      continue;
    }

    // skip actors on the same "team", unless infighting or deathmatching
    if (mo->target &&
      !((link->flags ^ mo->target->flags) & MF_FRIEND) &&
      mo->target->target != link &&
      !(deathmatch && link->player && mo->target->player))
    {
      link = link->bnext;
      continue;
    }

    // skip actors outside of specified FOV
    if (fov > 0 && !P_CheckFov(mo, link, fov))
    {
      link = link->bnext;
      continue;
    }

    // skip actors not in line of sight
    if (!P_CheckSight(mo, link))
    {
      link = link->bnext;
      continue;
    }

    // all good! return it.
    return link;
  }

  // couldn't find a valid target
  return NULL;
}

//
// P_RoughTargetSearch
// Searches though the surrounding mapblocks for monsters/players
// based on Hexen's P_RoughMonsterSearch
//
// distance is in MAPBLOCKUNITS

mobj_t *P_RoughTargetSearch(mobj_t *mo, angle_t fov, int distance)
{
  int blockX;
  int blockY;
  int startX, startY;
  int blockIndex;
  int firstStop;
  int secondStop;
  int thirdStop;
  int finalStop;
  int count;
  mobj_t *target;

  startX = (mo->x - bmaporgx) >> MAPBLOCKSHIFT;
  startY = (mo->y - bmaporgy) >> MAPBLOCKSHIFT;

  if (startX >= 0 && startX < bmapwidth && startY >= 0 && startY < bmapheight)
  {
    if ((target = RoughBlockCheck(mo, startY*bmapwidth + startX, fov)))
    { // found a target right away
      return target;
    }
  }
  for (count = 1; count <= distance; count++)
  {
    blockX = startX - count;
    blockY = startY - count;

    if (blockY < 0)
    {
      blockY = 0;
    }
    else if (blockY >= bmapheight)
    {
      blockY = bmapheight - 1;
    }
    if (blockX < 0)
    {
      blockX = 0;
    }
    else if (blockX >= bmapwidth)
    {
      blockX = bmapwidth - 1;
    }
    blockIndex = blockY * bmapwidth + blockX;
    firstStop = startX + count;
    if (firstStop < 0)
    {
      continue;
    }
    if (firstStop >= bmapwidth)
    {
      firstStop = bmapwidth - 1;
    }
    secondStop = startY + count;
    if (secondStop < 0)
    {
      continue;
    }
    if (secondStop >= bmapheight)
    {
      secondStop = bmapheight - 1;
    }
    thirdStop = secondStop * bmapwidth + blockX;
    secondStop = secondStop * bmapwidth + firstStop;
    firstStop += blockY * bmapwidth;
    finalStop = blockIndex;

    // Trace the first block section (along the top)
    for (; blockIndex <= firstStop; blockIndex++)
    {
      if ((target = RoughBlockCheck(mo, blockIndex, fov)))
      {
        return target;
      }
    }
    // Trace the second block section (right edge)
    for (blockIndex--; blockIndex <= secondStop; blockIndex += bmapwidth)
    {
      if ((target = RoughBlockCheck(mo, blockIndex, fov)))
      {
        return target;
      }
    }
    // Trace the third block section (bottom edge)
    for (blockIndex -= bmapwidth; blockIndex >= thirdStop; blockIndex--)
    {
      if ((target = RoughBlockCheck(mo, blockIndex, fov)))
      {
        return target;
      }
    }
    // Trace the final block section (left edge)
    for (blockIndex++; blockIndex > finalStop; blockIndex -= bmapwidth)
    {
      if ((target = RoughBlockCheck(mo, blockIndex, fov)))
      {
        return target;
      }
    }
  }
  return NULL;
}

// MAES: support 512x512 blockmaps.
int P_GetSafeBlockX(int coord)
{
  coord >>= MAPBLOCKSHIFT;

  // If x is LE than those special values, interpret as positive.
  // Otherwise, leave it as it is.
  if (comperr(comperr_blockmap) && coord <= blockmapxneg)
    return coord & 0x1FF; // Broke width boundary

  return coord;
}

// MAES: support 512x512 blockmaps.
int P_GetSafeBlockY(int coord)
{
  coord >>= MAPBLOCKSHIFT;

  // If y is LE than those special values, interpret as positive.
  // Otherwise, leave it as it is.
  if (comperr(comperr_blockmap) && coord <= blockmapyneg)
    return coord & 0x1FF; // Broke width boundary

  return coord;
}

// e6y
//
// Intercepts memory table.  This is where various variables are located
// in memory in Vanilla Doom.  When the intercepts table overflows, we
// need to write to them.
//
// Almost all of the values to overwrite are 32-bit integers, except for
// playerstarts, which is effectively an array of 16-bit integers and
// must be treated differently.

extern fixed_t bulletslope;

intercepts_overrun_t intercepts_overrun[] =
{
  {4,   NULL,                          NULL},
  {4,   NULL, /* &earlyout, */         NULL},
  {4,   NULL, /* &intercept_p, */      NULL},
  {4,   &line_opening.lowfloor,        NULL},
  {4,   &line_opening.bottom,          NULL},
  {4,   &line_opening.top,             NULL},
  {4,   &line_opening.range,           NULL},
  {4,   NULL,                          NULL},
  {120, NULL, /* &activeplats, */      NULL},
  {8,   NULL,                          NULL},
  {4,   &bulletslope,                  NULL},
  {4,   NULL, /* &swingx, */           NULL},
  {4,   NULL, /* &swingy, */           NULL},
  {4,   NULL,                          NULL},
  {4,  &playerstarts[0][0].x,       &playerstarts[0][0].y},
  {4,  &playerstarts[0][0].angle,   &playerstarts[0][0].type},
  {4,  &playerstarts[0][0].options, &playerstarts[0][1].x},
  {4,  &playerstarts[0][1].y,       &playerstarts[0][1].angle},
  {4,  &playerstarts[0][1].type,    &playerstarts[0][1].options},
  {4,  &playerstarts[0][2].x,       &playerstarts[0][2].y},
  {4,  &playerstarts[0][2].angle,   &playerstarts[0][2].type},
  {4,  &playerstarts[0][2].options, &playerstarts[0][3].x},
  {4,  &playerstarts[0][3].y,       &playerstarts[0][3].angle},
  {4,  &playerstarts[0][3].type,    &playerstarts[0][3].options},
  {4,   NULL, /* &blocklinks, */       NULL},
  {4,   &bmapwidth,                    NULL},
  {4,   NULL, /* &blockmap, */         NULL},
  {4,   &bmaporgx,                     NULL},
  {4,   &bmaporgy,                     NULL},
  {4,   NULL, /* &blockmaplump, */     NULL},
  {4,   &bmapheight,                   NULL},
  {0,   NULL,                          NULL},
};

// hexen

static mobj_t *Hexen_RoughBlockCheck(mobj_t * mo, int index)
{
    mobj_t *link;
    mobj_t *master;
    angle_t angle;

    link = blocklinks[index];
    while (link)
    {
        if (mo->player)         // Minotaur looking around player
        {
            if ((link->flags & MF_COUNTKILL) ||
                (link->player && (link != mo)))
            {
                if (!(link->flags & MF_SHOOTABLE))
                {
                    link = link->bnext;
                    continue;
                }
                if (link->flags2 & MF2_DORMANT)
                {
                    link = link->bnext;
                    continue;
                }
                if ((link->type == HEXEN_MT_MINOTAUR) &&
                    (link->special1.m == mo))
                {
                    link = link->bnext;
                    continue;
                }
                if (netgame && !deathmatch && link->player)
                {
                    link = link->bnext;
                    continue;
                }
                if (P_CheckSight(mo, link))
                {
                    return link;
                }
            }
            link = link->bnext;
        }
        else if (mo->type == HEXEN_MT_MINOTAUR)       // looking around minotaur
        {
            master = mo->special1.m;
            if ((link->flags & MF_COUNTKILL) ||
                (link->player && (link != master)))
            {
                if (!(link->flags & MF_SHOOTABLE))
                {
                    link = link->bnext;
                    continue;
                }
                if (link->flags2 & MF2_DORMANT)
                {
                    link = link->bnext;
                    continue;
                }
                if ((link->type == HEXEN_MT_MINOTAUR) &&
                    (link->special1.m == mo->special1.m))
                {
                    link = link->bnext;
                    continue;
                }
                if (netgame && !deathmatch && link->player)
                {
                    link = link->bnext;
                    continue;
                }
                if (P_CheckSight(mo, link))
                {
                    return link;
                }
            }
            link = link->bnext;
        }
        else if (mo->type == HEXEN_MT_MSTAFF_FX2)     // bloodscourge
        {
            if ((link->flags & MF_COUNTKILL ||
                 (link->player && link != mo->target))
                && !(link->flags2 & MF2_DORMANT))
            {
                if (!(link->flags & MF_SHOOTABLE))
                {
                    link = link->bnext;
                    continue;
                }
                if (netgame && !deathmatch && link->player)
                {
                    link = link->bnext;
                    continue;
                }
                else if (P_CheckSight(mo, link))
                {
                    master = mo->target;
                    angle = R_PointToAngle2(master->x, master->y,
                                            link->x, link->y) - master->angle;
                    angle >>= 24;
                    if (angle > 226 || angle < 30)
                    {
                        return link;
                    }
                }
            }
            link = link->bnext;
        }
        else                    // spirits
        {
            if ((link->flags & MF_COUNTKILL ||
                 (link->player && link != mo->target))
                && !(link->flags2 & MF2_DORMANT))
            {
                if (!(link->flags & MF_SHOOTABLE))
                {
                    link = link->bnext;
                    continue;
                }
                if (netgame && !deathmatch && link->player)
                {
                    link = link->bnext;
                    continue;
                }
                if (link == mo->target)
                {
                    link = link->bnext;
                    continue;
                }
                else if (P_CheckSight(mo, link))
                {
                    return link;
                }
            }
            link = link->bnext;
        }
    }
    return NULL;
}
