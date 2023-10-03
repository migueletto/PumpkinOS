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
 *  Plats (i.e. elevator platforms) code, raising/lowering.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "m_random.h"
#include "r_main.h"
#include "p_spec.h"
#include "p_tick.h"
#include "s_sound.h"
#include "sounds.h"
#include "lprintf.h"
#include "e6y.h"//e6y

#include "dsda/id_list.h"
#include "dsda/map_format.h"

#include "hexen/p_acs.h"
#include "hexen/sn_sonix.h"

platlist_t *activeplats;       // killough 2/14/98: made global again

//
// T_PlatRaise()
//
// Action routine to move a plat up and down
//
// Passed a plat structure containing all pertinent information about the move
// No return
//
// jff 02/08/98 all cases with labels beginning with gen added to support
// generalized line type behaviors.

void T_CompatiblePlatRaise(plat_t * plat)
{
  result_e      res;

  // handle plat moving, up, down, waiting, or in stasis,
  switch(plat->status)
  {
    case up: // plat moving up
      res = T_MoveFloorPlane(plat->sector, plat->speed, plat->high, plat->crush, 1, false);

      if (heretic && !(leveltime & 31))
      {
        S_LoopSectorSound(plat->sector, g_sfx_stnmov_plats, 32);
      }

      // if a pure raise type, make the plat moving sound
      if (plat->type == raiseAndChange
          || plat->type == raiseToNearestAndChange)
      {
        if (!(leveltime&7))
          S_LoopSectorSound(plat->sector, g_sfx_stnmov_plats, 8);
      }

      // if encountered an obstacle, and not a crush type, reverse direction
      if (res == crushed && plat->crush == NO_CRUSH)
      {
        plat->count = plat->wait;
        plat->status = down;
        S_StartSectorSound(plat->sector, g_sfx_pstart);

        if (demo_compatibility &&
            (plat->type == raiseToNearestAndChange ||
             plat->type == raiseAndChange))
        {
          // For these types vanilla did not initialize plat->low in EV_DoPlat,
          // so they may descend to any depth, or not at all.
          // See https://sourceforge.net/p/prboom-plus/bugs/211/ .
          lprintf(LO_WARN, "T_PlatRaise: raise-and-change type has reversed "
                  "direction in compatibility mode - may lead to desync\n"
                  " gametic: %d sector: %d complevel: %d\n",
                  gametic, plat->sector->iSectorID, compatibility_level);
        }
      }
      else  // else handle reaching end of up stroke
      {
        if (res == pastdest) // end of stroke
        {
          // if not an instant toggle type, wait, make plat stop sound
          if (plat->type!=toggleUpDn)
          {
            plat->count = plat->wait;
            plat->status = waiting;
            S_StartSectorSound(plat->sector, g_sfx_pstop);
          }
          else // else go into stasis awaiting next toggle activation
          {
            plat->oldstatus = plat->status;//jff 3/14/98 after action wait
            plat->status = in_stasis;      //for reactivation of toggle
          }

          // lift types and pure raise types are done at end of up stroke
          // only the perpetual type waits then goes back up
          switch(plat->type)
          {
            case blazeDWUS:
            case downWaitUpStay:
            case raiseAndChange:
            case raiseToNearestAndChange:
            case genLift:
              if (heretic && plat->type == raiseToNearestAndChange) break;
              P_RemoveActivePlat(plat);     // killough
            default:
              break;
          }
        }
      }
      break;

    case down: // plat moving down
      res = T_MoveFloorPlane(plat->sector, plat->speed, plat->low, NO_CRUSH, -1, false);

      // handle reaching end of down stroke
      if (res == pastdest)
      {
        // if not an instant toggle, start waiting, make plat stop sound
        if (plat->type!=toggleUpDn) //jff 3/14/98 toggle up down
        {                           // is silent, instant, no waiting
          plat->count = plat->wait;
          plat->status = waiting;
          S_StartSectorSound(plat->sector, g_sfx_pstop);
        }
        else // instant toggles go into stasis awaiting next activation
        {
          plat->oldstatus = plat->status;//jff 3/14/98 after action wait
          plat->status = in_stasis;      //for reactivation of toggle
        }

        //jff 1/26/98 remove the plat if it bounced so it can be tried again
        //only affects plats that raise and bounce
        //killough 1/31/98: relax compatibility to demo_compatibility

        // remove the plat if its a pure raise type
        if (!comp[comp_floors])
        {
          switch(plat->type)
          {
            case raiseAndChange:
            case raiseToNearestAndChange:
              P_RemoveActivePlat(plat);
            default:
              break;
          }
        }
      }
      else if (heretic && !(leveltime & 31))
      {
        S_LoopSectorSound(plat->sector, g_sfx_stnmov_plats, 32);
      }
      break;

    case waiting: // plat is waiting
      if (!--plat->count)  // downcount and check for delay elapsed
      {
        if (plat->sector->floorheight == plat->low)
          plat->status = up;     // if at bottom, start up
        else
          plat->status = down;   // if at top, start down

        // make plat start sound
        S_StartSectorSound(plat->sector, g_sfx_pstart);
      }
      break; //jff 1/27/98 don't pickup code added later to in_stasis

    case in_stasis: // do nothing if in stasis
      break;
  }
}

void T_ZDoomPlatRaise(plat_t * plat)
{
  result_e res;

  switch (plat->status)
  {
    case up:
      res = T_MoveFloorPlane(plat->sector, plat->speed, plat->high, plat->crush, 1, false);

      // if a pure raise type, make the plat moving sound
      if (plat->type == platUpByValueStay
          || plat->type == platRaiseAndStay
          || plat->type == platRaiseAndStayLockout)
      {
        if (!(leveltime & 7))
          S_LoopSectorSound(plat->sector, g_sfx_stnmov_plats, 8);
      }

      if (res == crushed && plat->crush == NO_CRUSH)
      {
        plat->count = plat->wait;
        plat->status = down;
        S_StartSectorSound(plat->sector, g_sfx_pstart);
      }
      else if (res == pastdest)
      {
        if (plat->type != platToggle)
        {
          plat->count = plat->wait;
          plat->status = waiting;
          S_StartSectorSound(plat->sector, g_sfx_pstop);

          switch (plat->type)
          {
            case platRaiseAndStayLockout:
            case platRaiseAndStay:
            case platDownByValue:
            case platDownWaitUpStay:
            case platDownWaitUpStayStone:
            case platUpByValueStay:
            case platDownToNearestFloor:
            case platDownToLowestCeiling:
              P_RemoveActivePlat(plat);
              break;
            default:
              break;
          }
        }
        else
        {
          plat->oldstatus = plat->status;
          plat->status = in_stasis;
        }
      }
      break;
    case down:
      res = T_MoveFloorPlane(plat->sector, plat->speed, plat->low, NO_CRUSH, -1, false);

      if (res == pastdest)
      {
        if (plat->type != platToggle)
        {
          plat->count = plat->wait;
          plat->status = waiting;
          S_StartSectorSound(plat->sector, g_sfx_pstop);

          switch (plat->type)
          {
            case platUpWaitDownStay:
            case platUpNearestWaitDownStay:
            case platUpByValue:
              P_RemoveActivePlat(plat);
              break;
            default:
              break;
          }
        }
        else
        {
          plat->oldstatus = plat->status;
          plat->status = in_stasis;
        }
      }
      else if (res == crushed && plat->crush == NO_CRUSH && plat->type != platToggle)
      {
        plat->count = plat->wait;
        plat->status = up;
        S_StartSectorSound(plat->sector, g_sfx_pstart);
      }

      // jff 1/26/98 remove the plat if it bounced so it can be tried again
      // only affects plats that raise and bounce
      // remove the plat if it's a pure raise type
      switch (plat->type)
      {
        case platUpByValueStay:
        case platRaiseAndStay:
        case platRaiseAndStayLockout:
          P_RemoveActivePlat(plat);
          break;
        default:
          break;
      }

      break;
    case waiting:
      if (!plat->count || !--plat->count)
      {
        if (plat->sector->floorheight == plat->low)
          plat->status = up;
        else
          plat->status = down;

        if (plat->type != platToggle)
          S_StartSectorSound(plat->sector, g_sfx_pstart);
      }
      break;
    case in_stasis:
      break;
  }
}

void T_PlatRaise(plat_t * plat)
{
  map_format.t_plat_raise(plat);
}


//
// EV_DoPlat
//
// Handle Plat linedef types
//
// Passed the linedef that activated the plat, the type of plat action,
// and for some plat types, an amount to raise
// Returns true if a thinker is started, or restarted from stasis
//

int EV_DoZDoomPlat(int tag, line_t *line, plattype_e type, fixed_t height,
                   fixed_t speed, int delay, fixed_t lip, int change)
{
  plat_t *plat;
  const int *id_p;
  int rtn = 0;
  sector_t *sec;

  height *= FRACUNIT;
  lip *= FRACUNIT;

  if (tag)
  {
    // Activate all <type> plats that are in_stasis
    switch (type)
    {
      case platToggle:
        rtn = 1;
      case platPerpetualRaise:
        P_ActivateInStasis(tag);
        break;
      default:
        break;
    }
  }

  FIND_SECTORS2(id_p, tag, line)
  {
    sec = &sectors[*id_p];

    if (P_FloorActive(sec))
      continue;

    rtn = 1;

    plat = Z_MallocLevel(sizeof(*plat));
    memset(plat, 0, sizeof(*plat));
    P_AddThinker(&plat->thinker);

    plat->sector = sec;
    sec->floordata = plat;
    plat->thinker.function = T_PlatRaise;
    plat->type = type;
    plat->crush = NO_CRUSH;
    plat->tag = tag;
    plat->speed = speed;
    plat->wait = delay;
    plat->low = sec->floorheight;
    plat->high = sec->floorheight;

    if (change)
    {
      if (line)
        sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
      if (change == 1)
        P_ResetSectorSpecial(sec);
    }

    switch (type)
    {
      case platRaiseAndStay:
      case platRaiseAndStayLockout:
        plat->high = P_FindNextHighestFloor(sec, sec->floorheight);
        plat->status = up;
        P_ResetSectorSpecial(sec);
        S_StartSectorSound(sec, g_sfx_stnmov_plats);
        break;
      case platUpByValue:
      case platUpByValueStay:
        plat->high = sec->floorheight + height;
        plat->status = up;
        S_StartSectorSound(sec, g_sfx_stnmov_plats);
        break;
      case platDownByValue:
        plat->low = sec->floorheight - height;
        plat->status = down;
        S_StartSectorSound(sec, g_sfx_stnmov_plats);
        break;
      case platDownWaitUpStay:
      case platDownWaitUpStayStone:
        plat->low = P_FindLowestFloorSurrounding(sec) + lip;
        if (plat->low > sec->floorheight)
          plat->low = sec->floorheight;
        plat->status = down;
        S_StartSectorSound(sec, type == platDownWaitUpStay ? g_sfx_pstart : g_sfx_stnmov_plats);
        break;
      case platUpNearestWaitDownStay:
        plat->high = P_FindNextHighestFloor(sec, sec->floorheight);
        plat->status = up;
        S_StartSectorSound(sec, g_sfx_pstart);
        break;
      case platUpWaitDownStay:
        plat->high = P_FindHighestFloorSurrounding(sec);
        if (plat->high < sec->floorheight)
          plat->high = sec->floorheight;
        plat->status = up;
        S_StartSectorSound(sec, g_sfx_pstart);
        break;
      case platPerpetualRaise:
        plat->low = P_FindLowestFloorSurrounding(sec) + lip;
        if (plat->low > sec->floorheight)
          plat->low = sec->floorheight;
        plat->high = P_FindHighestFloorSurrounding(sec);
        if (plat->high < sec->floorheight)
          plat->high = sec->floorheight;
        plat->status = (P_Random(pr_plats) & 1) ? up : down;
        S_StartSectorSound(sec, g_sfx_pstart);
        break;
      case platToggle:
        plat->crush = DOOM_CRUSH;
        plat->low = sec->ceilingheight;
        plat->status = down;
        break;
      case platDownToNearestFloor:
        plat->low = P_FindNextLowestFloor(sec, sec->floorheight) + lip;
        plat->status = down;
        S_StartSectorSound(sec, g_sfx_pstart);
        break;
      case platDownToLowestCeiling:
        plat->low = P_FindLowestCeilingSurrounding(sec);
        if (plat->low > sec->floorheight)
          plat->low = sec->floorheight;
        plat->status = down;
        S_StartSectorSound(sec, g_sfx_pstart);
        break;
      default:
        break;
    }

    P_AddActivePlat(plat);
  }

  return rtn;
}

int EV_DoPlat
( line_t*       line,
  plattype_e    type,
  int           amount )
{
  plat_t* plat;
  const int *id_p;
  int             rtn;
  sector_t*       sec;

  rtn = 0;

  // Activate all <type> plats that are in_stasis
  switch(type)
  {
    case perpetualRaise:
      P_ActivateInStasis(line->tag);
      break;

    case toggleUpDn:
      P_ActivateInStasis(line->tag);
      rtn=1;
      break;

    default:
      break;
  }

  // act on all sectors tagged the same as the activating linedef
  FIND_SECTORS(id_p, line->tag)
  {
    sec = &sectors[*id_p];

    // don't start a second floor function if already moving
    if (P_FloorActive(sec)) //jff 2/23/98 multiple thinkers
      continue;

    // Create a thinker
    rtn = 1;
    plat = Z_MallocLevel( sizeof(*plat));
    memset(plat, 0, sizeof(*plat));
    P_AddThinker(&plat->thinker);

    plat->type = type;
    plat->sector = sec;
    plat->sector->floordata = plat; //jff 2/23/98 multiple thinkers
    plat->thinker.function = T_PlatRaise;
    plat->crush = NO_CRUSH;
    plat->tag = line->tag;

    //jff 1/26/98 Avoid raise plat bouncing a head off a ceiling and then
    //going down forever -- default low to plat height when triggered
    plat->low = sec->floorheight; // heretic_note: not in heretic

    // set up plat according to type
    switch(type)
    {
      case raiseToNearestAndChange:
        plat->speed = PLATSPEED/2;
        sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
        plat->high = P_FindNextHighestFloor(sec,sec->floorheight);
        plat->wait = 0;
        plat->status = up;
        P_ResetSectorSpecial(sec);

        S_StartSectorSound(sec, g_sfx_stnmov_plats);
        break;

      case raiseAndChange:
        plat->speed = PLATSPEED/2;
        sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
        plat->high = sec->floorheight + amount*FRACUNIT;
        plat->wait = 0;
        plat->status = up;

        S_StartSectorSound(sec, g_sfx_stnmov_plats);
        break;

      case downWaitUpStay:
        plat->speed = PLATSPEED * 4;
        plat->low = P_FindLowestFloorSurrounding(sec);

        if (plat->low > sec->floorheight)
          plat->low = sec->floorheight;

        plat->high = sec->floorheight;
        plat->wait = 35*PLATWAIT;
        plat->status = down;
        S_StartSectorSound(sec, g_sfx_pstart);
        break;

      case blazeDWUS:
        plat->speed = PLATSPEED * 8;
        plat->low = P_FindLowestFloorSurrounding(sec);

        if (plat->low > sec->floorheight)
          plat->low = sec->floorheight;

        plat->high = sec->floorheight;
        plat->wait = 35*PLATWAIT;
        plat->status = down;
        S_StartSectorSound(sec, sfx_pstart);
        break;

      case perpetualRaise:
        plat->speed = PLATSPEED;
        plat->low = P_FindLowestFloorSurrounding(sec);

        if (plat->low > sec->floorheight)
          plat->low = sec->floorheight;

        plat->high = P_FindHighestFloorSurrounding(sec);

        if (plat->high < sec->floorheight)
          plat->high = sec->floorheight;

        plat->wait = 35*PLATWAIT;
        plat->status = P_Random(pr_plats)&1;

        S_StartSectorSound(sec, g_sfx_pstart);
        break;

      case toggleUpDn: //jff 3/14/98 add new type to support instant toggle
        plat->speed = PLATSPEED;  //not used
        plat->wait = 35*PLATWAIT; //not used
        plat->crush = DOOM_CRUSH; //jff 3/14/98 crush anything in the way

        // set up toggling between ceiling, floor inclusive
        plat->low = sec->ceilingheight;
        plat->high = sec->floorheight;
        plat->status =  down;
        break;

      default:
        break;
    }
    P_AddActivePlat(plat);  // add plat to list of active plats
  }
  return rtn;
}

// The following were all rewritten by Lee Killough
// to use the new structure which places no limits
// on active plats. It also avoids spending as much
// time searching for active plats. Previously a
// fixed-size array was used, with NULL indicating
// empty entries, while now a doubly-linked list
// is used.

//
// P_ActivateInStasis()
//
// Activate a plat that has been put in stasis
// (stopped perpetual floor, instant floor/ceil toggle)
//
// Passed the tag of the plat that should be reactivated
// Returns nothing
//
void P_ActivateInStasis(int tag)
{
  platlist_t *pl;
  for (pl=activeplats; pl; pl=pl->next)   // search the active plats
  {
    plat_t *plat = pl->plat;              // for one in stasis with right tag
    if (plat->tag == tag && plat->status == in_stasis)
    {
      if (plat->type==toggleUpDn) //jff 3/14/98 reactivate toggle type
        plat->status = plat->oldstatus==up? down : up;
      else
        plat->status = plat->oldstatus;
      plat->thinker.function = T_PlatRaise;
    }
  }
}

//
// EV_StopPlat()
//
// Handler for "stop perpetual floor" linedef type
//
// Passed the linedef that stopped the plat
// Returns true if a plat was put in stasis
//
// jff 2/12/98 added int return value, fixed return
//

void EV_StopZDoomPlat(int tag, dboolean remove)
{
  platlist_t *pl;

  for (pl = activeplats; pl; pl = pl->next)
  {
    plat_t *plat = pl->plat;
    if (plat->status != in_stasis && plat->tag == tag)
    {
      if (!remove)
      {
        plat->oldstatus = plat->status;
        plat->status = in_stasis;
      }
      else
      {
        P_RemoveActivePlat(plat);
      }
    }
  }
}

int EV_StopPlat(line_t* line)
{
  platlist_t *pl;
  for (pl=activeplats; pl; pl=pl->next)  // search the active plats
  {
    plat_t *plat = pl->plat;             // for one with the tag not in stasis
    if (plat->status != in_stasis && plat->tag == line->tag)
    {
      plat->oldstatus = plat->status;    // put it in stasis
      plat->status = in_stasis;
      plat->thinker.function = NULL;
    }
  }
  return 1;
}

//
// P_AddActivePlat()
//
// Add a plat to the head of the active plat list
//
// Passed a pointer to the plat to add
// Returns nothing
//
void P_AddActivePlat(plat_t* plat)
{
  platlist_t *list = Z_Malloc(sizeof *list);
  list->plat = plat;
  plat->list = list;
  if ((list->next = activeplats))
    list->next->prev = &list->next;
  list->prev = &activeplats;
  activeplats = list;
}

//
// P_RemoveActivePlat()
//
// Remove a plat from the active plat list
//
// Passed a pointer to the plat to remove
// Returns nothing
//
void P_RemoveActivePlat(plat_t* plat)
{
  platlist_t *list = plat->list;
  plat->sector->floordata = NULL; //jff 2/23/98 multiple thinkers
  P_TagFinished(plat->sector->tag);
  P_RemoveThinker(&plat->thinker);
  if ((*list->prev = list->next))
    list->next->prev = list->prev;
  Z_Free(list);
}

//
// P_RemoveAllActivePlats()
//
// Remove all plats from the active plat list
//
// Passed nothing, returns nothing
//
void P_RemoveAllActivePlats(void)
{
  while (activeplats)
  {
    platlist_t *next = activeplats->next;
    Z_Free(activeplats);
    activeplats = next;
  }
}

// hexen

void T_HexenPlatRaise(plat_t * plat)
{
    result_e res;

    switch (plat->status)
    {
        case up:
            res = T_MoveFloorPlane(plat->sector, plat->speed, plat->high, plat->crush, 1, true);
            if (res == crushed && plat->crush == NO_CRUSH)
            {
                plat->count = plat->wait;
                plat->status = down;
                SN_StartSequence((mobj_t *) & plat->sector->soundorg,
                                 SEQ_PLATFORM + plat->sector->seqType);
            }
            else if (res == pastdest)
            {
                plat->count = plat->wait;
                plat->status = waiting;
                SN_StopSequence((mobj_t *) & plat->sector->soundorg);
                switch (plat->type)
                {
                    case PLAT_DOWNWAITUPSTAY:
                    case PLAT_DOWNBYVALUEWAITUPSTAY:
                        P_RemoveActivePlat(plat);
                        break;
                    default:
                        break;
                }
            }
            break;
        case down:
            res = T_MoveFloorPlane(plat->sector, plat->speed, plat->low, NO_CRUSH, -1, true);
            if (res == pastdest)
            {
                plat->count = plat->wait;
                plat->status = waiting;
                switch (plat->type)
                {
                    case PLAT_UPWAITDOWNSTAY:
                    case PLAT_UPBYVALUEWAITDOWNSTAY:
                        P_RemoveActivePlat(plat);
                        break;
                    default:
                        break;
                }
                SN_StopSequence((mobj_t *) & plat->sector->soundorg);
            }
            break;
        case waiting:
            if (!--plat->count)
            {
                if (plat->sector->floorheight == plat->low)
                    plat->status = up;
                else
                    plat->status = down;
                SN_StartSequence((mobj_t *) & plat->sector->soundorg,
                                 SEQ_PLATFORM + plat->sector->seqType);
            }
          break;

        default:
          break;
    }
}

int EV_DoHexenPlat(line_t * line, byte * args, plattype_e type, int amount)
{
    plat_t *plat;
    const int *id_p;
    int rtn;
    sector_t *sec;

    rtn = 0;

    FIND_SECTORS(id_p, args[0])
    {
        sec = &sectors[*id_p];
        if (sec->floordata || sec->ceilingdata)
            continue;

        //
        // Find lowest & highest floors around sector
        //
        rtn = 1;
        plat = Z_MallocLevel(sizeof(*plat));
        memset(plat, 0, sizeof(*plat));
        P_AddThinker(&plat->thinker);

        plat->type = type;
        plat->sector = sec;
        plat->sector->floordata = plat;
        plat->thinker.function = T_PlatRaise;
        plat->crush = NO_CRUSH;
        plat->tag = args[0];
        plat->speed = args[1] * (FRACUNIT / 8);
        switch (type)
        {
            case PLAT_DOWNWAITUPSTAY:
                plat->low = P_FindLowestFloorSurrounding(sec) + 8 * FRACUNIT;
                if (plat->low > sec->floorheight)
                    plat->low = sec->floorheight;
                plat->high = sec->floorheight;
                plat->wait = args[2];
                plat->status = down;
                break;
            case PLAT_DOWNBYVALUEWAITUPSTAY:
                plat->low = sec->floorheight - args[3] * 8 * FRACUNIT;
                if (plat->low > sec->floorheight)
                    plat->low = sec->floorheight;
                plat->high = sec->floorheight;
                plat->wait = args[2];
                plat->status = down;
                break;
            case PLAT_UPWAITDOWNSTAY:
                plat->high = P_FindHighestFloorSurrounding(sec);
                if (plat->high < sec->floorheight)
                    plat->high = sec->floorheight;
                plat->low = sec->floorheight;
                plat->wait = args[2];
                plat->status = up;
                break;
            case PLAT_UPBYVALUEWAITDOWNSTAY:
                plat->high = sec->floorheight + args[3] * 8 * FRACUNIT;
                if (plat->high < sec->floorheight)
                    plat->high = sec->floorheight;
                plat->low = sec->floorheight;
                plat->wait = args[2];
                plat->status = up;
                break;
            case PLAT_PERPETUALRAISE:
                plat->low = P_FindLowestFloorSurrounding(sec) + 8 * FRACUNIT;
                if (plat->low > sec->floorheight)
                    plat->low = sec->floorheight;
                plat->high = P_FindHighestFloorSurrounding(sec);
                if (plat->high < sec->floorheight)
                    plat->high = sec->floorheight;
                plat->wait = args[2];
                plat->status = P_Random(pr_hexen) & 1;
                break;
            default:
              break;
        }
        P_AddActivePlat(plat);
        SN_StartSequence((mobj_t *) & sec->soundorg,
                         SEQ_PLATFORM + sec->seqType);
    }
    return rtn;
}

// hexen_note: why set the tags? not sure if this is correct
void Hexen_EV_StopPlat(line_t * line, byte * args)
{
    platlist_t *pl;
    for (pl = activeplats; pl; pl = pl->next)
    {
        pl->plat->tag = args[0];

        if (pl->plat->tag != 0)
        {
            P_RemoveActivePlat(pl->plat);
            return;
        }
    }
}
