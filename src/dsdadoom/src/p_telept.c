/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2002 by
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
 *      Teleportation.
 *
 *-----------------------------------------------------------------------------*/

#include "doomdef.h"
#include "doomstat.h"
#include "p_spec.h"
#include "p_maputl.h"
#include "p_map.h"
#include "r_main.h"
#include "p_tick.h"
#include "s_sound.h"
#include "sounds.h"
#include "p_user.h"
#include "smooth.h"
#include "m_random.h"

#include "dsda/id_list.h"
#include "dsda/thing_id.h"

// More will be added
static dboolean P_IsTeleportDestination(mobj_t *mo)
{
  return mo->type == MT_TELEPORTMAN ||
         mo->type == ZMT_TELEPORTDEST2 || mo->type == ZMT_TELEPORTDEST3;
}

static dboolean P_UseTeleportDestinationHeight(mobj_t *mo)
{
  return mo->type == ZMT_TELEPORTDEST2 || mo->type == ZMT_TELEPORTDEST3;
}

static dboolean P_IsMapSpot(mobj_t *mo)
{
  return mo->type == ZMT_MAPSPOT || mo->type == ZMT_MAPSPOT_GRAVITY;
}

static mobj_t* P_TeleportDestination(short thing_id, int tag)
{
  const int *id_p;

  if (thing_id)
  {
    int count = 0;
    mobj_t *target;
    thing_id_search_t search;

    dsda_ResetThingIDSearch(&search);
    while ((target = dsda_FindMobjFromThingID(thing_id, &search)))
    {
      if (P_IsTeleportDestination(target))
      {
        if (!tag || target->subsector->sector->tag == tag)
        {
          ++count;
        }
      }
    }

    if (!count)
    {
      if (!tag)
      {
        // Fall back on map spots
        dsda_ResetThingIDSearch(&search);
        while ((target = dsda_FindMobjFromThingID(thing_id, &search)))
        {
          if (P_IsMapSpot(target))
          {
            break;
          }
        }

        // Fall back on any nonblocking thing
        if (!target)
        {
          dsda_ResetThingIDSearch(&search);
          while ((target = dsda_FindMobjFromThingID(thing_id, &search)))
          {
            if (!(target->flags & MF_SOLID))
            {
              break;
            }
          }
        }

        return target;
      }
    }
    else
    {
      if (count > 1)
      {
        count = 1 + (P_Random(pr_hexen) % count);
      }

      dsda_ResetThingIDSearch(&search);
      while ((target = dsda_FindMobjFromThingID(thing_id, &search)))
      {
        if (P_IsTeleportDestination(target))
        {
          if (!tag || target->subsector->sector->tag == tag)
          {
            if (!--count)
            {
              return target;
            }
          }
        }
      }
    }

    return NULL;
  }

  FIND_SECTORS(id_p, tag)
  {
    register thinker_t* th = NULL;
    while ((th = P_NextThinker(th,th_misc)) != NULL)
      if (th->function == P_MobjThinker) {
        register mobj_t* m = (mobj_t*)th;
        if (m->type == MT_TELEPORTMAN  &&
            m->subsector->sector->iSectorID == *id_p)
            return m;
      }
  }
  return NULL;
}
//
// TELEPORTATION
//
// killough 5/3/98: reformatted, cleaned up

static int P_TeleportToDestination(mobj_t *destination, line_t *line, mobj_t *thing, int flags)
{
  fixed_t oldx = thing->x;
  fixed_t oldy = thing->y;
  fixed_t oldz = thing->z;
  fixed_t momx = thing->momx;
  fixed_t momy = thing->momy;
  fixed_t z = thing->z - thing->floorz;
  player_t *player = thing->player;
  angle_t angle = 0;
  fixed_t c = 0, s = 0;

  // Get the angle between the exit thing and source linedef.
  // Rotate 90 degrees, so that walking perpendicularly across
  // teleporter linedef causes thing to exit in the direction
  // indicated by the exit thing.
  if (flags & (TELF_ROTATEBOOM | TELF_ROTATEBOOMINVERSE) && line)
  {
    angle = R_PointToAngle2(0, 0, line->dx, line->dy) - destination->angle + ANG90;

    if (flags & TELF_ROTATEBOOMINVERSE)
      angle = angle + ANG180;

    s = finesine[angle >> ANGLETOFINESHIFT];
    c = finecosine[angle >> ANGLETOFINESHIFT];
  }

  // killough 5/12/98: exclude voodoo dolls:
  if (player && player->mo != thing)
    player = NULL;

  if (!P_TeleportMove(thing, destination->x, destination->y, false)) /* killough 8/9/98 */
    return 0;

  if (flags & (TELF_ROTATEBOOM | TELF_ROTATEBOOMINVERSE))
  {
    if (line)
    {
      // Rotate thing according to difference in angles
      thing->angle += angle;

      // Rotate thing's momentum to come out of exit just like it entered
      thing->momx = FixedMul(momx, c) - FixedMul(momy, s);
      thing->momy = FixedMul(momy, c) + FixedMul(momx, s);
    }
  }
  else if (!(flags & TELF_KEEPORIENTATION))
  {
    thing->angle = destination->angle;
  }

  if (P_UseTeleportDestinationHeight(destination))
    thing->z = destination->z;
  else if (flags & TELF_KEEPHEIGHT)
    thing->z = thing->floorz + z;
  else if (compatibility_level != finaldoom_compatibility)
    thing->z = thing->floorz;
  thing->PrevZ = thing->z;

  if (flags & TELF_SOURCEFOG)
  {
    // spawn teleport fog and emit sound at source
    S_StartMobjSound(P_SpawnMobj(oldx, oldy, oldz, MT_TFOG), sfx_telept);
  }

  if (flags & TELF_DESTFOG)
  {
    // spawn teleport fog and emit sound at destination
    S_StartMobjSound(
      P_SpawnMobj(
        destination->x + 20 * finecosine[destination->angle >> ANGLETOFINESHIFT],
        destination->y + 20 * finesine[destination->angle >> ANGLETOFINESHIFT],
        thing->z, MT_TFOG
      ),
      sfx_telept
    );
  }

  /* don't move for a bit
    * cph - DEMOSYNC - BOOM had (player) here? */
  if (
    thing->player &&
    ((flags & TELF_DESTFOG) || !(flags & TELF_KEEPORIENTATION)) &&
    !(flags & TELF_KEEPVELOCITY)
  )
    thing->reactiontime = 18;

  if (!(flags & TELF_KEEPORIENTATION) && !(flags & TELF_KEEPVELOCITY))
  {
    thing->momx = thing->momy = thing->momz = 0;

    /* killough 10/98: kill all bobbing momentum too */
    if (player)
      player->momx = player->momy = 0;
  }

  if (player)
  {
    // This code was different between silent and non-silent functions.
    if (flags & TELF_KEEPORIENTATION)
    {
      // Adjust player's view, in case there has been a height change
      if (player)
      {
        // Save the current deltaviewheight, used in stepping
        fixed_t deltaviewheight = player->deltaviewheight;

        // Clear deltaviewheight, since we don't want any changes
        player->deltaviewheight = 0;

        // Set player's view according to the newly set parameters
        P_CalcHeight(player);

        // Reset the delta to have the same dynamics as before
        player->deltaviewheight = deltaviewheight;
      }
    }
    else
    {
      player->viewz = thing->z + player->viewheight;
    }

    // e6y
    R_ResetAfterTeleport(player);
  }

  return 1;
}

int EV_TeleportGroup(short group_tid, mobj_t *thing, short source_tid, short dest_tid,
                     dboolean move_source, dboolean fog)
{
  int result = 0;
  mobj_t *source;
  mobj_t *dest;
  mobj_t *target;
  thing_id_search_t search;

  dsda_ResetThingIDSearch(&search);
  source = dsda_FindMobjFromThingID(source_tid, &search);

  dest = P_TeleportDestination(dest_tid, 0);

  if (source && dest)
  {
    int flags;
    angle_t an;
    fixed_t dcos, dsin;

    flags = fog ? (TELF_DESTFOG | TELF_SOURCEFOG) : TELF_KEEPORIENTATION;

    an = dest->angle - source->angle;
    dcos = finecosine[an >> ANGLETOFINESHIFT];
    dsin = finesine[an >> ANGLETOFINESHIFT];

    dsda_ResetThingIDSearch(&search);
    while ((target = dsda_FindMobjFromThingIDOrMobj(group_tid, thing, &search)))
    {
      fixed_t dx, dy;
      mobj_t target_dest;

      memset(&target_dest, 0, sizeof(target_dest));

      dx = target->x - source->x;
      dy = target->y - source->y;
      target_dest.x = dest->x + FixedMul(dx, dcos) - FixedMul(dy, dsin);
      target_dest.y = dest->y + FixedMul(dx, dsin) + FixedMul(dy, dcos);
      target_dest.angle = target->angle;
      target_dest.type = dest->type;

      result |= P_TeleportToDestination(&target_dest, NULL, target, flags);
    }

    if (result && move_source)
    {
      P_TeleportToDestination(dest, NULL, source, TELF_KEEPORIENTATION);
      source->angle = dest->angle;
    }
  }

  return result;
}

int EV_TeleportInSector(int tag, short source_tid, short dest_tid,
                        dboolean fog, short group_tid)
{
  int result = 0;
  mobj_t *source;
  mobj_t *dest;
  mobj_t *target;
  thing_id_search_t search;

  dsda_ResetThingIDSearch(&search);
  source = dsda_FindMobjFromThingID(source_tid, &search);

  dest = P_TeleportDestination(dest_tid, 0);

  if (source && dest)
  {
    const int *id_p;
    int flags;
    angle_t an;
    fixed_t dcos, dsin;

    flags = fog ? (TELF_DESTFOG | TELF_SOURCEFOG) : TELF_KEEPORIENTATION;

    an = dest->angle - source->angle;
    dcos = finecosine[an >> ANGLETOFINESHIFT];
    dsin = finesine[an >> ANGLETOFINESHIFT];

    FIND_SECTORS(id_p, tag)
    {
      mobj_in_sector_t mis;

      P_InitSectorSearch(&mis, &sectors[*id_p]);
      while ((target = P_FindMobjInSector(&mis)))
      {
        fixed_t dx, dy;
        mobj_t target_dest;

        if (group_tid && target->tid != group_tid)
          continue;

        memset(&target_dest, 0, sizeof(target_dest));

        dx = target->x - source->x;
        dy = target->y - source->y;
        target_dest.x = dest->x + FixedMul(dx, dcos) - FixedMul(dy, dsin);
        target_dest.y = dest->y + FixedMul(dx, dsin) + FixedMul(dy, dcos);
        target_dest.angle = target->angle;
        target_dest.type = dest->type;

        result |= P_TeleportToDestination(&target_dest, NULL, target, flags);
      }
    }
  }

  return result;
}

int EV_CompatibleTeleport(short thing_id, int tag, line_t *line, int side, mobj_t *thing, int flags)
{
  mobj_t *m;

  // don't teleport missiles
  // Don't teleport if hit back of line,
  //  so you can get out of teleporter.
  if (side || thing->flags & MF_MISSILE)
    return 0;

  if ((m = P_TeleportDestination(thing_id, tag)) != NULL)
  {
    return P_TeleportToDestination(m, line, thing, flags);
  }

  return 0;
}

//
// Silent linedef-based TELEPORTATION, by Lee Killough
// Primarily for rooms-over-rooms etc.
// This is the complete player-preserving kind of teleporter.
// It has advantages over the teleporter with thing exits.
//

// maximum fixed_t units to move object to avoid hiccups
#define FUDGEFACTOR 10

int EV_SilentLineTeleport(line_t *line, int side, mobj_t *thing,
                          int tag, dboolean reverse)
{
  const int *i;
  line_t *l;

  if (side || thing->flags & MF_MISSILE)
    return 0;

  for (i = dsda_FindLinesFromID(tag); *i >= 0; i++)
    if ((l = lines + *i) != line && l->backsector)
      {
        // Get the thing's position along the source linedef
        fixed_t pos = D_abs(line->dx) > D_abs(line->dy) ?
          FixedDiv(thing->x - line->v1->x, line->dx) :
          FixedDiv(thing->y - line->v1->y, line->dy) ;

        // Get the angle between the two linedefs, for rotating
        // orientation and momentum. Rotate 180 degrees, and flip
        // the position across the exit linedef, if reversed.
        angle_t angle = (reverse ? pos = FRACUNIT-pos, 0 : ANG180) +
          R_PointToAngle2(0, 0, l->dx, l->dy) -
          R_PointToAngle2(0, 0, line->dx, line->dy);

        // Interpolate position across the exit linedef
        fixed_t x = l->v2->x - FixedMul(pos, l->dx);
        fixed_t y = l->v2->y - FixedMul(pos, l->dy);

        // Sine, cosine of angle adjustment
        fixed_t s = finesine[angle>>ANGLETOFINESHIFT];
        fixed_t c = finecosine[angle>>ANGLETOFINESHIFT];

        // Maximum distance thing can be moved away from interpolated
        // exit, to ensure that it is on the correct side of exit linedef
        int fudge = FUDGEFACTOR;

        // Whether this is a player, and if so, a pointer to its player_t.
        // Voodoo dolls are excluded by making sure thing->player->mo==thing.
        player_t *player = thing->player && thing->player->mo == thing ?
          thing->player : NULL;

        // Whether walking towards first side of exit linedef steps down
        int stepdown =
          l->frontsector->floorheight < l->backsector->floorheight;

        // Height of thing above ground
        fixed_t z = thing->z - thing->floorz;

        // Side to exit the linedef on positionally.
        //
        // Notes:
        //
        // This flag concerns exit position, not momentum. Due to
        // roundoff error, the thing can land on either the left or
        // the right side of the exit linedef, and steps must be
        // taken to make sure it does not end up on the wrong side.
        //
        // Exit momentum is always towards side 1 in a reversed
        // teleporter, and always towards side 0 otherwise.
        //
        // Exiting positionally on side 1 is always safe, as far
        // as avoiding oscillations and stuck-in-wall problems,
        // but may not be optimum for non-reversed teleporters.
        //
        // Exiting on side 0 can cause oscillations if momentum
        // is towards side 1, as it is with reversed teleporters.
        //
        // Exiting on side 1 slightly improves player viewing
        // when going down a step on a non-reversed teleporter.

        int side = reverse || (player && stepdown);

        // Make sure we are on correct side of exit linedef.
        while (P_PointOnLineSide(x, y, l) != side && --fudge>=0)
          if (D_abs(l->dx) > D_abs(l->dy))
            y -= (l->dx < 0) != side ? -1 : 1;
          else
            x += (l->dy < 0) != side ? -1 : 1;

        // Attempt to teleport, aborting if blocked
        if (!P_TeleportMove(thing, x, y, false)) /* killough 8/9/98 */
          return 0;

        // e6y
        if (player && player->mo == thing)
          R_ResetAfterTeleport(player);

        // Adjust z position to be same height above ground as before.
        // Ground level at the exit is measured as the higher of the
        // two floor heights at the exit linedef.
        thing->z = z + sides[l->sidenum[stepdown]].sector->floorheight;
        thing->PrevZ = thing->z;

        // Rotate thing's orientation according to difference in linedef angles
        thing->angle += angle;

        // Momentum of thing crossing teleporter linedef
        x = thing->momx;
        y = thing->momy;

        // Rotate thing's momentum to come out of exit just like it entered
        thing->momx = FixedMul(x, c) - FixedMul(y, s);
        thing->momy = FixedMul(y, c) + FixedMul(x, s);

        // Adjust a player's view, in case there has been a height change
        if (player)
          {
            // Save the current deltaviewheight, used in stepping
            fixed_t deltaviewheight = player->deltaviewheight;

            // Clear deltaviewheight, since we don't want any changes now
            player->deltaviewheight = 0;

            // Set player's view according to the newly set parameters
            P_CalcHeight(player);

            // Reset the delta to have the same dynamics as before
            player->deltaviewheight = deltaviewheight;
          }

        // e6y
        if (player && player->mo == thing)
          R_ResetAfterTeleport(player);

        return 1;
      }
  return 0;
}

// heretic

#include "heretic/def.h"

dboolean P_Teleport(mobj_t * thing, fixed_t x, fixed_t y, angle_t angle, dboolean useFog)
{
    fixed_t oldx;
    fixed_t oldy;
    fixed_t oldz;
    fixed_t aboveFloor;
    fixed_t fogDelta;
    player_t *player;
    unsigned an;
    mobj_t *fog;

    oldx = thing->x;
    oldy = thing->y;
    oldz = thing->z;
    aboveFloor = thing->z - thing->floorz;
    if (!P_TeleportMove(thing, x, y, false))
    {
        return (false);
    }
    if (thing->player)
    {
        player = thing->player;
        if (player->powers[pw_flight] && aboveFloor)
        {
            thing->z = thing->floorz + aboveFloor;
            if (thing->z + thing->height > thing->ceilingz)
            {
                thing->z = thing->ceilingz - thing->height;
            }
            player->viewz = thing->z + player->viewheight;
        }
        else
        {
            thing->z = thing->floorz;
            player->viewz = thing->z + player->viewheight;
            if (useFog)
            {
                player->lookdir = 0;
            }
        }
    }
    else if (thing->flags & MF_MISSILE)
    {
        thing->z = thing->floorz + aboveFloor;
        if (thing->z + thing->height > thing->ceilingz)
        {
            thing->z = thing->ceilingz - thing->height;
        }
    }
    else
    {
        thing->z = thing->floorz;
    }
    // Spawn teleport fog at source and destination
    if (useFog)
    {
        fogDelta = thing->flags & MF_MISSILE ? 0 : TELEFOGHEIGHT;
        fog = P_SpawnMobj(oldx, oldy, oldz + fogDelta, g_mt_tfog);
        S_StartMobjSound(fog, g_sfx_telept);
        an = angle >> ANGLETOFINESHIFT;
        fog = P_SpawnMobj(x + 20 * finecosine[an],
                          y + 20 * finesine[an], thing->z + fogDelta, g_mt_tfog);
        S_StartMobjSound(fog, g_sfx_telept);
        if (thing->player &&
            !thing->player->powers[pw_weaponlevel2] &&
            !thing->player->powers[pw_speed])
        {                           // Freeze player for about .5 sec
            thing->reactiontime = 18;
        }
        thing->angle = angle;
    }

    if (hexen)
    {
        if (thing->flags2 & MF2_FOOTCLIP)
        {
            if (thing->z == thing->subsector->sector->floorheight
                && P_GetThingFloorType(thing) > FLOOR_SOLID)
            {
                thing->floorclip = 10 * FRACUNIT;
            }
            else
            {
                thing->floorclip = 0;
            }
        }
    }
    else
    {
        if (thing->flags2 & MF2_FOOTCLIP
            && P_GetThingFloorType(thing) != FLOOR_SOLID)
        {
            thing->flags2 |= MF2_FEETARECLIPPED;
        }
        else if (thing->flags2 & MF2_FEETARECLIPPED)
        {
            thing->flags2 &= ~MF2_FEETARECLIPPED;
        }
    }

    if (thing->flags & MF_MISSILE)
    {
        angle >>= ANGLETOFINESHIFT;
        thing->momx = FixedMul(thing->info->speed, finecosine[angle]);
        thing->momy = FixedMul(thing->info->speed, finesine[angle]);
    }
    else if (useFog)
    {
        thing->momx = thing->momy = thing->momz = 0;
    }

    if (thing->player)
    {
        R_ResetAfterTeleport(thing->player);
    }

    return (true);
}

int EV_HereticTeleport(short thing_id, int tag, line_t * line, int side, mobj_t * thing, int flags)
{
    int i;
    mobj_t *m;
    thinker_t *thinker;
    sector_t *sector;

    if (thing->flags2 & MF2_NOTELEPORT)
    {
        return (false);
    }
    if (side == 1)
    {                           // Don't teleport when crossing back side
        return (false);
    }
    for (i = 0; i < numsectors; i++)
    {
        if (sectors[i].tag == tag)
        {
            for (thinker = thinkercap.next; thinker != &thinkercap;
                 thinker = thinker->next)
            {
                if (thinker->function != P_MobjThinker)
                {               // Not a mobj
                    continue;
                }
                m = (mobj_t *) thinker;
                if (m->type != HERETIC_MT_TELEPORTMAN)
                {               // Not a teleportman
                    continue;
                }
                sector = m->subsector->sector;
                if (sector - sectors != i)
                {               // Wrong sector
                    continue;
                }
                return (P_Teleport(thing, m->x, m->y, m->angle, true));
            }
        }
    }
    return (false);
}

// hexen

#include "m_random.h"
#include "lprintf.h"

dboolean EV_HexenTeleport(int tid, mobj_t * thing, dboolean fog)
{
    int i;
    int count;
    mobj_t *mo;
    int searcher;

    if (!thing)
    {                           // Teleport function called with an invalid mobj
        return false;
    }
    if (thing->flags2 & MF2_NOTELEPORT)
    {
        return false;
    }
    count = 0;
    searcher = -1;
    while (P_FindMobjFromTID(tid, &searcher) != NULL)
    {
        count++;
    }
    if (count == 0)
    {
        return false;
    }
    count = 1 + (P_Random(pr_hexen) % count);
    searcher = -1;
    mo = NULL;

    for (i = 0; i < count; i++)
    {
        mo = P_FindMobjFromTID(tid, &searcher);
    }
    if (mo == NULL)
    {
        I_Error("Can't find teleport mapspot\n");
    }
    return P_Teleport(thing, mo->x, mo->y, mo->angle, fog);
}
