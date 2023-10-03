/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000,2002 by
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
 *      Enemy thinking, AI.
 *      Action Pointer Functions
 *      that are associated with states/frames.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "m_random.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"
#include "p_spec.h"
#include "s_sound.h"
#include "sounds.h"
#include "p_inter.h"
#include "g_game.h"
#include "p_enemy.h"
#include "p_tick.h"
#include "i_sound.h"
#include "m_bbox.h"
#include "hu_stuff.h"
#include "lprintf.h"
#include "e6y.h"//e6y

#include "dsda.h"
#include "dsda/configuration.h"
#include "dsda/id_list.h"
#include "dsda/map_format.h"
#include "dsda/mapinfo.h"
#include "dsda/skill_info.h"

static mobj_t *current_actor;

typedef enum {
  DI_EAST,
  DI_NORTHEAST,
  DI_NORTH,
  DI_NORTHWEST,
  DI_WEST,
  DI_SOUTHWEST,
  DI_SOUTH,
  DI_SOUTHEAST,
  DI_NODIR,
  NUMDIRS
} dirtype_e;

typedef int dirtype_t;

static void P_NewChaseDir(mobj_t *actor);
void P_ZBumpCheck(mobj_t *);                                        // phares

//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//

//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//
// killough 5/5/98: reformatted, cleaned up

static void P_RecursiveSound(sector_t *sec, int soundblocks, mobj_t *soundtarget)
{
  int i;

  // wake up all monsters in this sector
  if (sec->validcount == validcount && sec->soundtraversed <= soundblocks+1)
    return;             // already flooded

  sec->validcount = validcount;
  sec->soundtraversed = soundblocks+1;
  P_SetTarget(&sec->soundtarget, soundtarget);

  for (i=0; i<sec->linecount; i++)
  {
    sector_t *other;
    line_t *check = sec->lines[i];

    if (!(check->flags & ML_TWOSIDED))
      continue;

    P_LineOpening(check, NULL);

    if (line_opening.range <= 0)
      continue;       // closed door

    other=sides[check->sidenum[sides[check->sidenum[0]].sector==sec]].sector;

    if (!(check->flags & ML_SOUNDBLOCK))
      P_RecursiveSound(other, soundblocks, soundtarget);
    else
      if (!soundblocks)
        P_RecursiveSound(other, 1, soundtarget);
  }
}

//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
void P_NoiseAlert(mobj_t *target, mobj_t *emitter)
{
  if (target != NULL && target->player && (target->player->cheats & CF_NOTARGET))
    return;

  validcount++;
  P_RecursiveSound(emitter->subsector->sector, 0, target);
}

//
// P_CheckRange
//

static dboolean P_CheckRange(mobj_t *actor, fixed_t range)
{
  mobj_t *pl = actor->target;

  return  // killough 7/18/98: friendly monsters don't attack other friends
    pl &&
    !(actor->flags & pl->flags & MF_FRIEND) &&
    P_AproxDistance(pl->x-actor->x, pl->y-actor->y) < range &&
    P_CheckSight(actor, actor->target) &&
    ( // finite height!
      !raven ||
      (
        pl->z <= actor->z + actor->height &&
        actor->z <= pl->z + pl->height
      )
    );
}

//
// P_CheckMeleeRange
//

static dboolean P_CheckMeleeRange(mobj_t *actor)
{
  int range;

  if (actor->subsector->sector->flags & SECF_NOATTACK)
    return false;

  range = actor->info->meleerange;

  if (compatibility_level != doom_12_compatibility)
    range += actor->target->info->radius - 20 * FRACUNIT;

  return P_CheckRange(actor, range);
}

//
// P_HitFriend()
//
// killough 12/98
// This function tries to prevent shooting at friends

static dboolean P_HitFriend(mobj_t *actor)
{
  return actor->flags & MF_FRIEND && actor->target &&
    (P_AimLineAttack(actor,
         R_PointToAngle2(actor->x, actor->y,
             actor->target->x, actor->target->y),
         P_AproxDistance(actor->x-actor->target->x,
             actor->y-actor->target->y), 0),
     linetarget) && linetarget != actor->target &&
    !((linetarget->flags ^ actor->flags) & MF_FRIEND);
}

//
// P_CheckMissileRange
//
static dboolean P_CheckMissileRange(mobj_t *actor)
{
  fixed_t dist;

  if (actor->subsector->sector->flags & SECF_NOATTACK)
    return false;

  if (!P_CheckSight(actor, actor->target))
    return false;

  if (actor->flags & MF_JUSTHIT)
  {      // the target just hit the enemy, so fight back!
    actor->flags &= ~MF_JUSTHIT;

    /* killough 7/18/98: no friendly fire at corpses
     * killough 11/98: prevent too much infighting among friends
     * cph - yikes, talk about fitting everything on one line... */

    return
      !(actor->flags & MF_FRIEND) ||
      (actor->target->health > 0 &&
       (!(actor->target->flags & MF_FRIEND) ||
        (actor->target->player ?
         monster_infighting || P_Random(pr_defect) >128 :
         !(actor->target->flags & MF_JUSTHIT) && P_Random(pr_defect) >128)));
  }

  /* killough 7/18/98: friendly monsters don't attack other friendly
   * monsters or players (except when attacked, and then only once)
   */
  if (actor->flags & actor->target->flags & MF_FRIEND)
    return false;

  if (actor->reactiontime)
    return false;       // do not attack yet

  // OPTIMIZE: get this from a global checksight
  dist = P_AproxDistance ( actor->x-actor->target->x,
                           actor->y-actor->target->y) - 64*FRACUNIT;

  if (!actor->info->meleestate)
    dist -= 128*FRACUNIT;       // no melee attack, so fire more

  dist >>= FRACBITS;

  if (actor->flags2 & MF2_SHORTMRANGE)
    if (dist > 14*64)
      return false;     // too far away

  if (actor->flags2 & MF2_LONGMELEE)
  {
    if (dist < 196)
      return false;   // close for fist attack
  }

  if (actor->flags2 & MF2_RANGEHALF)
    dist >>= 1;

  if (dist > 200)
    dist = 200;

  if (actor->flags2 & MF2_HIGHERMPROB && dist > 160)
    dist = 160;

  if (P_Random(pr_missrange) < dist)
    return false;

  if (P_HitFriend(actor))
    return false;

  return true;
}

/*
 * P_IsOnLift
 *
 * killough 9/9/98:
 *
 * Returns true if the object is on a lift. Used for AI,
 * since it may indicate the need for crowded conditions,
 * or that a monster should stay on the lift for a while
 * while it goes up or down.
 */

static dboolean P_IsOnLift(const mobj_t *actor)
{
  const sector_t *sec = actor->subsector->sector;
  const int *l;

  // Short-circuit: it's on a lift which is active.
  if (sec->floordata && ((thinker_t *) sec->floordata)->function==T_PlatRaise)
    return true;

  // Check to see if it's in a sector which can be activated as a lift.
  if (sec->tag)
    for (l = dsda_FindLinesFromID(sec->tag); *l >= 0; l++)
      switch (lines[*l].special)
  {
  case  10: case  14: case  15: case  20: case  21: case  22:
  case  47: case  53: case  62: case  66: case  67: case  68:
  case  87: case  88: case  95: case 120: case 121: case 122:
  case 123: case 143: case 162: case 163: case 181: case 182:
  case 144: case 148: case 149: case 211: case 227: case 228:
  case 231: case 232: case 235: case 236:
    return true;
  }

  return false;
}

/*
 * P_IsUnderDamage
 *
 * killough 9/9/98:
 *
 * Returns nonzero if the object is under damage based on
 * their current position. Returns 1 if the damage is moderate,
 * -1 if it is serious. Used for AI.
 */

static int P_IsUnderDamage(mobj_t *actor)
{
  const struct msecnode_s *seclist;
  const ceiling_t *cl;             // Crushing ceiling
  int dir = 0;
  for (seclist=actor->touching_sectorlist; seclist; seclist=seclist->m_tnext)
    if ((cl = seclist->m_sector->ceilingdata) &&
  cl->thinker.function == T_MoveCeiling)
      dir |= cl->direction;
  return dir;
}

//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//

static fixed_t xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
static fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

// 1/11/98 killough: Limit removed on special lines crossed
extern  line_t **spechit;          // New code -- killough
extern  int    numspechit;

static dboolean P_Move(mobj_t *actor, dboolean dropoff) /* killough 9/12/98 */
{
  fixed_t tryx, tryy, deltax, deltay, origx, origy;
  dboolean try_ok;
  int movefactor = ORIG_FRICTION_FACTOR;    // killough 10/98
  int friction = ORIG_FRICTION;
  int speed;

  if (actor->flags2 & MF2_BLASTED)
    return true;
  if (actor->movedir == DI_NODIR)
    return false;

#ifdef RANGECHECK
  if ((unsigned)actor->movedir >= 8)
    I_Error ("P_Move: Weird actor->movedir!");
#endif

  // killough 10/98: make monsters get affected by ice and sludge too:

  if (monster_friction)
    movefactor = P_GetMoveFactor(actor, &friction);

  speed = actor->info->speed;

  if (friction < ORIG_FRICTION &&     // sludge
      !(speed = ((ORIG_FRICTION_FACTOR - (ORIG_FRICTION_FACTOR-movefactor)/2)
     * speed) / ORIG_FRICTION_FACTOR))
    speed = 1;      // always give the monster a little bit of speed

  tryx = (origx = actor->x) + (deltax = speed * xspeed[actor->movedir]);
  tryy = (origy = actor->y) + (deltay = speed * yspeed[actor->movedir]);

  try_ok = P_TryMove(actor, tryx, tryy, dropoff);

  // killough 10/98:
  // Let normal momentum carry them, instead of steptoeing them across ice.

  if (try_ok && friction > ORIG_FRICTION)
  {
    actor->x = origx;
    actor->y = origy;
    movefactor *= FRACUNIT / ORIG_FRICTION_FACTOR / 4;
    actor->momx += FixedMul(deltax, movefactor);
    actor->momy += FixedMul(deltay, movefactor);
  }

  // [RH] If a walking monster is no longer on the floor, move it down
  // to the floor if it is within MaxStepHeight, presuming that it is
  // actually walking down a step.
  if (
      try_ok &&
      map_format.zdoom &&
      actor->z > actor->floorz &&
      actor->z <= actor->floorz + (24 << FRACBITS) &&
      !(actor->flags & MF_NOGRAVITY) &&
      !(actor->flags2 & MF2_ONMOBJ))
  {
    fixed_t saved_z = actor->z;

    actor->z = actor->floorz;

    // Make sure that there isn't some other actor between us and
    // the floor we could get stuck in. The old code did not do this.
    if (!P_CheckPosition(actor, actor->x, actor->y))
    {
      actor->z = saved_z;
    }
  }

  if (!try_ok)
  {      // open any specials
    int good;

    if (actor->flags & MF_FLOAT && floatok)
    {
      if (actor->z < tmfloorz)          // must adjust height
        actor->z += FLOATSPEED;
      else
        actor->z -= FLOATSPEED;

      actor->flags |= MF_INFLOAT;

      return true;
    }

    if (!numspechit)
      return false;

    actor->movedir = DI_NODIR;

    /* if the special is not a door that can be opened, return false
     *
     * killough 8/9/98: this is what caused monsters to get stuck in
     * doortracks, because it thought that the monster freed itself
     * by opening a door, even if it was moving towards the doortrack,
     * and not the door itself.
     *
     * killough 9/9/98: If a line blocking the monster is activated,
     * return true 90% of the time. If a line blocking the monster is
     * not activated, but some other line is, return false 90% of the
     * time. A bit of randomness is needed to ensure it's free from
     * lockups, but for most cases, it returns the correct result.
     *
     * Do NOT simply return false 1/4th of the time (causes monsters to
     * back out when they shouldn't, and creates secondary stickiness).
     */

    for (good = false; numspechit--; )
      if (P_UseSpecialLine(actor, spechit[numspechit], 0, false))
        good |= spechit[numspechit] == blockline ? 1 : 2;

    if (raven) return good > 0;

    /* cph - compatibility maze here
     * Boom v2.01 and orig. Doom return "good"
     * Boom v2.02 and LxDoom return good && (P_Random(pr_trywalk)&3)
     * MBF plays even more games
     */
    if (!good || comp[comp_doorstuck]) return good;
    if (!mbf_features)
      return (P_Random(pr_trywalk)&3); /* jff 8/13/98 */
    else /* finally, MBF code */
      return ((P_Random(pr_opendoor) >= 230) ^ (good & 1));
  }
  else
    actor->flags &= ~MF_INFLOAT;

  /* killough 11/98: fall more slowly, under gravity, if felldown==true */
  if (!map_format.zdoom && !(actor->flags & MF_FLOAT) && (!felldown || !mbf_features)) {
    if (raven && actor->z > actor->floorz)
    {
      P_HitFloor(actor);
    }
    actor->z = actor->floorz;
  }

  return true;
}

/*
 * P_SmartMove
 *
 * killough 9/12/98: Same as P_Move, except smarter
 */

static dboolean P_SmartMove(mobj_t *actor)
{
  mobj_t *target = actor->target;
  int on_lift, dropoff = false, under_damage;
  int tmp_monster_avoid_hazards = (prboom_comp[PC_MONSTER_AVOID_HAZARDS].state ?
    true : (demo_compatibility ? false : monster_avoid_hazards));//e6y

  /* killough 9/12/98: Stay on a lift if target is on one */
  on_lift = !comp[comp_staylift]
    && target && target->health > 0
    && target->subsector->sector->tag==actor->subsector->sector->tag &&
    P_IsOnLift(actor);

  under_damage = tmp_monster_avoid_hazards && P_IsUnderDamage(actor);//e6y

  // killough 10/98: allow dogs to drop off of taller ledges sometimes.
  // dropoff==1 means always allow it, dropoff==2 means only up to 128 high,
  // and only if the target is immediately on the other side of the line.

  // haleyjd: allow all friends of HelperType to also jump down

  if ((actor->type == MT_DOGS || (actor->type == (HelperThing-1) && actor->flags&MF_FRIEND))
      && target && dog_jumping &&
      !((target->flags ^ actor->flags) & MF_FRIEND) &&
      P_AproxDistance(actor->x - target->x,
          actor->y - target->y) < FRACUNIT*144 &&
      P_Random(pr_dropoff) < 235)
    dropoff = 2;

  if (!P_Move(actor, dropoff))
    return false;

  // killough 9/9/98: avoid crushing ceilings or other damaging areas
  if (
      (on_lift && P_Random(pr_stayonlift) < 230 &&      // Stay on lift
       !P_IsOnLift(actor))
      ||
      (tmp_monster_avoid_hazards && !under_damage &&//e6y  // Get away from damage
       (under_damage = P_IsUnderDamage(actor)) &&
       (under_damage < 0 || P_Random(pr_avoidcrush) < 200))
      )
    actor->movedir = DI_NODIR;    // avoid the area (most of the time anyway)

  return true;
}

//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//

// HERETIC_NOTE: Quite sure P_SmartMove == P_Move for heretic
static dboolean P_TryWalk(mobj_t *actor)
{
  if (!P_SmartMove(actor))
    return false;
  actor->movecount = P_Random(pr_trywalk)&15;
  return true;
}

//
// P_DoNewChaseDir
//
// killough 9/8/98:
//
// Most of P_NewChaseDir(), except for what
// determines the new direction to take
//

static void P_DoNewChaseDir(mobj_t *actor, fixed_t deltax, fixed_t deltay)
{
  dirtype_t xdir, ydir, tdir;
  dirtype_t olddir = actor->movedir;
  dirtype_t turnaround = olddir;

  if (turnaround != DI_NODIR)         // find reverse direction
    turnaround ^= 4;

  xdir =
    deltax >  10*FRACUNIT ? DI_EAST :
    deltax < -10*FRACUNIT ? DI_WEST : DI_NODIR;

  ydir =
    deltay < -10*FRACUNIT ? DI_SOUTH :
    deltay >  10*FRACUNIT ? DI_NORTH : DI_NODIR;

  // try direct route
  if (xdir != DI_NODIR && ydir != DI_NODIR && turnaround !=
      (actor->movedir = deltay < 0 ? deltax > 0 ? DI_SOUTHEAST : DI_SOUTHWEST :
       deltax > 0 ? DI_NORTHEAST : DI_NORTHWEST) && P_TryWalk(actor))
    return;

  // try other directions
  if (P_Random(pr_newchase) > 200 || D_abs(deltay)>D_abs(deltax))
    tdir = xdir, xdir = ydir, ydir = tdir;

  if ((xdir == turnaround ? xdir = DI_NODIR : xdir) != DI_NODIR &&
      (actor->movedir = xdir, P_TryWalk(actor)))
    return;         // either moved forward or attacked

  if ((ydir == turnaround ? ydir = DI_NODIR : ydir) != DI_NODIR &&
      (actor->movedir = ydir, P_TryWalk(actor)))
    return;

  // there is no direct path to the player, so pick another direction.
  if (olddir != DI_NODIR && (actor->movedir = olddir, P_TryWalk(actor)))
    return;

  // randomly determine direction of search
  if (P_Random(pr_newchasedir) & 1)
  {
    for (tdir = DI_EAST; tdir <= DI_SOUTHEAST; tdir++)
      if (tdir != turnaround && (actor->movedir = tdir, P_TryWalk(actor)))
        return;
  }
  else
    for (tdir = DI_SOUTHEAST; tdir != DI_EAST-1; tdir--)
      if (tdir != turnaround && (actor->movedir = tdir, P_TryWalk(actor)))
        return;

  if ((actor->movedir = turnaround) != DI_NODIR && !P_TryWalk(actor))
    actor->movedir = DI_NODIR;
}

//
// killough 11/98:
//
// Monsters try to move away from tall dropoffs.
//
// In Doom, they were never allowed to hang over dropoffs,
// and would remain stuck if involuntarily forced over one.
// This logic, combined with p_map.c (P_TryMove), allows
// monsters to free themselves without making them tend to
// hang over dropoffs.

static fixed_t dropoff_deltax, dropoff_deltay, floorz;

static dboolean PIT_AvoidDropoff(line_t *line)
{
  if (line->backsector                          && // Ignore one-sided linedefs
      tmbbox[BOXRIGHT]  > line->bbox[BOXLEFT]   &&
      tmbbox[BOXLEFT]   < line->bbox[BOXRIGHT]  &&
      tmbbox[BOXTOP]    > line->bbox[BOXBOTTOM] && // Linedef must be contacted
      tmbbox[BOXBOTTOM] < line->bbox[BOXTOP]    &&
      P_BoxOnLineSide(tmbbox, line) == -1)
    {
      fixed_t front = line->frontsector->floorheight;
      fixed_t back  = line->backsector->floorheight;
      angle_t angle;

      // The monster must contact one of the two floors,
      // and the other must be a tall dropoff (more than 24).

      if (back == floorz && front < floorz - FRACUNIT*24)
  angle = R_PointToAngle2(0,0,line->dx,line->dy);   // front side dropoff
      else
  if (front == floorz && back < floorz - FRACUNIT*24)
    angle = R_PointToAngle2(line->dx,line->dy,0,0); // back side dropoff
  else
    return true;

      // Move away from dropoff at a standard speed.
      // Multiple contacted linedefs are cumulative (e.g. hanging over corner)
      dropoff_deltax -= finesine[angle >> ANGLETOFINESHIFT]*32;
      dropoff_deltay += finecosine[angle >> ANGLETOFINESHIFT]*32;
    }
  return true;
}

//
// Driver for above
//

static fixed_t P_AvoidDropoff(mobj_t *actor)
{
  int yh=P_GetSafeBlockY((tmbbox[BOXTOP]   = actor->y+actor->radius)-bmaporgy);
  int yl=P_GetSafeBlockY((tmbbox[BOXBOTTOM]= actor->y-actor->radius)-bmaporgy);
  int xh=P_GetSafeBlockX((tmbbox[BOXRIGHT] = actor->x+actor->radius)-bmaporgx);
  int xl=P_GetSafeBlockX((tmbbox[BOXLEFT]  = actor->x-actor->radius)-bmaporgx);
  int bx, by;

  floorz = actor->z;            // remember floor height

  dropoff_deltax = dropoff_deltay = 0;

  // check lines

  validcount++;
  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      P_BlockLinesIterator(bx, by, PIT_AvoidDropoff);  // all contacted lines

  return dropoff_deltax | dropoff_deltay;   // Non-zero if movement prescribed
}

//
// P_NewChaseDir
//
// killough 9/8/98: Split into two functions
//

static void P_NewChaseDir(mobj_t *actor)
{
  mobj_t *target = actor->target;
  fixed_t deltax = target->x - actor->x;
  fixed_t deltay = target->y - actor->y;

  // killough 8/8/98: sometimes move away from target, keeping distance
  //
  // 1) Stay a certain distance away from a friend, to avoid being in their way
  // 2) Take advantage over an enemy without missiles, by keeping distance

  actor->strafecount = 0;

  if (mbf_features) {
    if (
      actor->floorz - actor->dropoffz > FRACUNIT*24 &&
      actor->z <= actor->floorz &&
      !(actor->flags & (MF_DROPOFF|MF_FLOAT)) &&
      !comp[comp_dropoff] &&
      P_AvoidDropoff(actor)
    ) /* Move away from dropoff */
    {
      P_DoNewChaseDir(actor, dropoff_deltax, dropoff_deltay);

      // If moving away from dropoff, set movecount to 1 so that
      // small steps are taken to get monster away from dropoff.

      actor->movecount = 1;
      return;
    }
    else
    {
      fixed_t dist = P_AproxDistance(deltax, deltay);

      // Move away from friends when too close, except
      // in certain situations (e.g. a crowded lift)

      if (actor->flags & target->flags & MF_FRIEND &&
          distfriend << FRACBITS > dist &&
          !P_IsOnLift(target) && !P_IsUnderDamage(actor))
      {
        deltax = -deltax, deltay = -deltay;
      }
      else if (target->health > 0 && (actor->flags ^ target->flags) & MF_FRIEND)
      {   // Live enemy target
        if (
          monster_backing &&
          actor->info->missilestate &&
          actor->type != MT_SKULL &&
          (
            (!target->info->missilestate && dist < target->info->meleerange*2) ||
            (
              target->player && dist < target->player->mo->info->meleerange*3 &&
              weaponinfo[target->player->readyweapon].flags & WPF_FLEEMELEE
            )
          )
        )
        {       // Back away from melee attacker
          actor->strafecount = P_Random(pr_enemystrafe) & 15;
          deltax = -deltax, deltay = -deltay;
        }
      }
    }
  }

  P_DoNewChaseDir(actor, deltax, deltay);

  // If strafing, set movecount to strafecount so that old Doom
  // logic still works the same, except in the strafing part

  if (actor->strafecount)
    actor->movecount = actor->strafecount;
}

//
// P_IsVisible
//
// killough 9/9/98: whether a target is visible to a monster
//

static dboolean P_IsVisible(mobj_t *actor, mobj_t *mo, dboolean allaround)
{
  if (!allaround)
    {
      angle_t an = R_PointToAngle2(actor->x, actor->y,
           mo->x, mo->y) - actor->angle;
      if (an > ANG90 && an < ANG270 &&
    P_AproxDistance(mo->x-actor->x, mo->y-actor->y) > WAKEUPRANGE)
  return false;
    }
  return P_CheckSight(actor, mo);
}

//
// PIT_FindTarget
//
// killough 9/5/98
//
// Finds monster targets for other monsters
//

static int current_allaround;

static dboolean PIT_FindTarget(mobj_t *mo)
{
  mobj_t *actor = current_actor;

  if (!((mo->flags ^ actor->flags) & MF_FRIEND &&        // Invalid target
  mo->health > 0 && (mo->flags & MF_COUNTKILL || mo->type == MT_SKULL)))
    return true;

  // If the monster is already engaged in a one-on-one attack
  // with a healthy friend, don't attack around 60% the time
  {
    const mobj_t *targ = mo->target;
    if (targ && targ->target == mo &&
  P_Random(pr_skiptarget) > 100 &&
  (targ->flags ^ mo->flags) & MF_FRIEND &&
  targ->health*2 >= P_MobjSpawnHealth(targ))
      return true;
  }

  if (!P_IsVisible(actor, mo, current_allaround))
    return true;

  P_SetTarget(&actor->lastenemy, actor->target);  // Remember previous target
  P_SetTarget(&actor->target, mo);                // Found target

  // Move the selected monster to the end of its associated
  // list, so that it gets searched last next time.

  {
    thinker_t *cap = &thinkerclasscap[mo->flags & MF_FRIEND ?
             th_friends : th_enemies];
    (mo->thinker.cprev->cnext = mo->thinker.cnext)->cprev = mo->thinker.cprev;
    (mo->thinker.cprev = cap->cprev)->cnext = &mo->thinker;
    (mo->thinker.cnext = cap)->cprev = &mo->thinker;
  }

  return false;
}

//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//

static dboolean P_LookForPlayers(mobj_t *actor, dboolean allaround)
{
  player_t *player;
  int stop, stopc, c;

  if (raven) return Raven_P_LookForPlayers(actor, allaround);

  if (actor->flags & MF_FRIEND)
  {  // killough 9/9/98: friendly monsters go about players differently
    int anyone;

    // Go back to a player, no matter whether it's visible or not
    for (anyone=0; anyone<=1; anyone++)
      for (c=0; c<g_maxplayers; c++)
        if (playeringame[c] && players[c].playerstate==PST_LIVE &&
            (anyone || P_IsVisible(actor, players[c].mo, allaround)))
        {
          P_SetTarget(&actor->target, players[c].mo);

          // killough 12/98:
          // get out of refiring loop, to avoid hitting player accidentally

          if (actor->info->missilestate)
          {
            P_SetMobjState(actor, actor->info->seestate);
            actor->flags &= ~MF_JUSTHIT;
          }

          return true;
        }

    return false;
  }

  // Change mask of 3 to (g_maxplayers-1) -- killough 2/15/98:
  stop = (actor->lastlook-1)&(g_maxplayers-1);

  c = 0;

  stopc = !mbf_features &&
    !demo_compatibility && monsters_remember ?
    g_maxplayers : 2;       // killough 9/9/98

  for (;; actor->lastlook = (actor->lastlook+1)&(g_maxplayers-1))
    {
      if (!playeringame[actor->lastlook])
  continue;

      // killough 2/15/98, 9/9/98:
      if (c++ == stopc || actor->lastlook == stop)  // done looking
      {
        // e6y
        // Fixed Boom incompatibilities. The following code was missed.
        // There are no more desyncs on Donce's demos on horror.wad

        // Use last known enemy if no players sighted -- killough 2/15/98:
        if (!mbf_features && !demo_compatibility && monsters_remember)
        {
          if (actor->lastenemy && actor->lastenemy->health > 0)
          {
            actor->target = actor->lastenemy;
            actor->lastenemy = NULL;
            return true;
          }
        }

        return false;
      }

      player = &players[actor->lastlook];

      if (player->cheats & CF_NOTARGET)
        continue; // no target

      if (player->health <= 0)
  continue;               // dead

      if (!P_IsVisible(actor, player->mo, allaround))
  continue;

      P_SetTarget(&actor->target, player->mo);

      /* killough 9/9/98: give monsters a threshold towards getting players
       * (we don't want it to be too easy for a player with dogs :)
       */
      if (!comp[comp_pursuit])
  actor->threshold = 60;

      return true;
    }
}

//
// Friendly monsters, by Lee Killough 7/18/98
//
// Friendly monsters go after other monsters first, but
// also return to owner if they cannot find any targets.
// A marine's best friend :)  killough 7/18/98, 9/98
//

static dboolean P_LookForMonsters(mobj_t *actor, dboolean allaround)
{
  thinker_t *cap, *th;

  if (demo_compatibility)
    return false;

  if (actor->lastenemy && actor->lastenemy->health > 0 && monsters_remember &&
      !(actor->lastenemy->flags & actor->flags & MF_FRIEND)) // not friends
    {
      P_SetTarget(&actor->target, actor->lastenemy);
      P_SetTarget(&actor->lastenemy, NULL);
      return true;
    }

  /* Old demos do not support monster-seeking bots */
  if (!mbf_features)
    return false;

  // Search the threaded list corresponding to this object's potential targets
  cap = &thinkerclasscap[actor->flags & MF_FRIEND ? th_enemies : th_friends];

  // Search for new enemy

  if (cap->cnext != cap)        // Empty list? bail out early
  {
    int x = P_GetSafeBlockX(actor->x - bmaporgx);
    int y = P_GetSafeBlockY(actor->y - bmaporgy);
    int d;

    current_actor = actor;
    current_allaround = allaround;

    // Search first in the immediate vicinity.

    if (!P_BlockThingsIterator(x, y, PIT_FindTarget))
      return true;

    for (d = 1; d < 5; d++)
    {
      int i = 1 - d;
      do
        if (!P_BlockThingsIterator(x + i, y - d, PIT_FindTarget) ||
            !P_BlockThingsIterator(x + i, y + d, PIT_FindTarget))
          return true;
      while (++i < d);

      do
        if (!P_BlockThingsIterator(x - d, y + i, PIT_FindTarget) ||
            !P_BlockThingsIterator(x + d, y + i, PIT_FindTarget))
          return true;
      while (--i + d >= 0);
    }

    {   // Random number of monsters, to prevent patterns from forming
      int n = (P_Random(pr_friends) & 31) + 15;

      for (th = cap->cnext; th != cap; th = th->cnext)
        if (--n < 0)
        {
          // Only a subset of the monsters were searched. Move all of
          // the ones which were searched so far, to the end of the list.

          (cap->cnext->cprev = cap->cprev)->cnext = cap->cnext;
          (cap->cprev = th->cprev)->cnext = cap;
          (th->cprev = cap)->cnext = th;
          break;
       }
        else
          if (!PIT_FindTarget((mobj_t *) th))   // If target sighted
            return true;
    }
  }

  return false;  // No monster found
}

//
// P_LookForTargets
//
// killough 9/5/98: look for targets to go after, depending on kind of monster
//

static dboolean P_LookForTargets(mobj_t *actor, int allaround)
{
  return actor->flags & MF_FRIEND ?
    P_LookForMonsters(actor, allaround) || P_LookForPlayers (actor, allaround):
    P_LookForPlayers (actor, allaround) || P_LookForMonsters(actor, allaround);
}

//
// P_HelpFriend
//
// killough 9/8/98: Help friends in danger of dying
//

static dboolean P_HelpFriend(mobj_t *actor)
{
  thinker_t *cap, *th;

  // If less than 33% health, self-preservation rules
  if (actor->health*3 < P_MobjSpawnHealth(actor))
    return false;

  current_actor = actor;
  current_allaround = true;

  // Possibly help a friend under 50% health
  cap = &thinkerclasscap[actor->flags & MF_FRIEND ? th_friends : th_enemies];

  for (th = cap->cnext; th != cap; th = th->cnext)
    if (((mobj_t *) th)->health*2 >= P_MobjSpawnHealth((mobj_t *) th))
      {
  if (P_Random(pr_helpfriend) < 180)
    break;
      }
    else
      if (((mobj_t *) th)->flags & MF_JUSTHIT &&
    ((mobj_t *) th)->target &&
    ((mobj_t *) th)->target != actor->target &&
    !PIT_FindTarget(((mobj_t *) th)->target))
  {
    // Ignore any attacking monsters, while searching for friend
    actor->threshold = BASETHRESHOLD;
    return true;
  }

  return false;
}

//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
void A_KeenDie(mobj_t* mo)
{
  thinker_t *th;
  line_t   junk;

  A_Fall(mo);

  // scan the remaining thinkers to see if all Keens are dead

  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (th->function == P_MobjThinker)
      {
        mobj_t *mo2 = (mobj_t *) th;
        if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
          return;                           // other Keen not dead
      }

  junk.tag = 666;
  EV_DoDoor(&junk,openDoor);
}


//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//

void A_Look(mobj_t *actor)
{
  mobj_t *targ = actor->subsector->sector->soundtarget;
  actor->threshold = 0; // any shot will wake up

  if (targ && targ->player && (targ->player->cheats & CF_NOTARGET))
    return;

  /* killough 7/18/98:
   * Friendly monsters go after other monsters first, but
   * also return to player, without attacking them, if they
   * cannot find any targets. A marine's best friend :)
   */
  actor->pursuecount = 0;

  if (
    !(actor->flags & MF_FRIEND && P_LookForTargets(actor, false)) &&
    !(
      (targ = actor->subsector->sector->soundtarget) &&
      targ->flags & MF_SHOOTABLE &&
      (
        P_SetTarget(&actor->target, targ),
        !(actor->flags & MF_AMBUSH) ||
        P_CheckSight(actor, targ)
      )
    ) &&
    (
      actor->flags & MF_FRIEND || !P_LookForTargets(actor, false)
    )
  )
    return;

  // go into chase state

  if (actor->info->seesound)
  {
    int sound;
    sound = actor->info->seesound;

    if (!raven)
      switch (sound)
      {
        case sfx_posit1:
        case sfx_posit2:
        case sfx_posit3:
          sound = sfx_posit1 + P_Random(pr_see) % 3;
          break;

        case sfx_bgsit1:
        case sfx_bgsit2:
          sound = sfx_bgsit1 + P_Random(pr_see) % 2;
          break;

        default:
          break;
      }

    if (actor->flags2 & (MF2_BOSS | MF2_FULLVOLSOUNDS))
      S_StartVoidSound(sound);          // full volume
    else
    {
      S_StartMobjSound(actor, sound);

      // [FG] make seesounds uninterruptible
      if (full_sounds)
        S_UnlinkSound(actor);
    }
  }
  P_SetMobjState(actor, actor->info->seestate);
}

#if 0
//
// A_KeepChasing
//
// killough 10/98:
// Allows monsters to continue movement while attacking
//

static void A_KeepChasing(mobj_t *actor)
{
  if (actor->movecount)
    {
      actor->movecount--;
      if (actor->strafecount)
        actor->strafecount--;
      P_SmartMove(actor);
    }
}
#endif

//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//

void A_Chase(mobj_t *actor)
{
  if (actor->reactiontime)
    actor->reactiontime--;

  if (actor->threshold) { /* modify target threshold */
    if (compatibility_level == doom_12_compatibility)
    {
      actor->threshold--;
    }
    else
    {
      if (!actor->target || actor->target->health <= 0)
        actor->threshold = 0;
      else
        actor->threshold--;
    }
  }

  if (raven && skill_info.flags & SI_FAST_MONSTERS)
  {                           // Monsters move faster in nightmare mode
    actor->tics -= actor->tics / 2;
    if (actor->tics < 3)
    {
      actor->tics = 3;
    }
  }

  /* turn towards movement direction if not there yet
   * killough 9/7/98: keep facing towards target if strafing or backing out
   */

  if (actor->strafecount)
    A_FaceTarget(actor);
  else if (actor->movedir < 8)
  {
    int delta = (actor->angle &= (7<<29)) - (actor->movedir << 29);
    if (delta > 0)
      actor->angle -= ANG90/2;
    else
      if (delta < 0)
        actor->angle += ANG90/2;
  }

  if (!actor->target || !(actor->target->flags&MF_SHOOTABLE))
  {
    if (!P_LookForTargets(actor,true)) // look for a new target
      P_SetMobjState(actor, actor->info->spawnstate); // no new target
    return;
  }

  // do not attack twice in a row
  if (actor->flags & MF_JUSTATTACKED)
  {
    actor->flags &= ~MF_JUSTATTACKED;
    if (!(skill_info.flags & SI_FAST_MONSTERS))
      P_NewChaseDir(actor);
    return;
  }

  // check for melee attack
  if (actor->info->meleestate && P_CheckMeleeRange(actor))
  {
    if (actor->info->attacksound)
      S_StartMobjSound(actor, actor->info->attacksound);
    P_SetMobjState(actor, actor->info->meleestate);
    /* killough 8/98: remember an attack
    * cph - DEMOSYNC? */
    if (!actor->info->missilestate && !raven)
      actor->flags |= MF_JUSTHIT;
    return;
  }

  // check for missile attack
  if (actor->info->missilestate)
    if (!(!(skill_info.flags & SI_FAST_MONSTERS) && actor->movecount))
      if (P_CheckMissileRange(actor))
      {
        P_SetMobjState(actor, actor->info->missilestate);
        actor->flags |= MF_JUSTATTACKED;
        return;
      }

  if (!actor->threshold) {
    if (!mbf_features)
    {   /* killough 9/9/98: for backward demo compatibility */
      if (netgame && !P_CheckSight(actor, actor->target) &&
          P_LookForPlayers(actor, true))
        return;
    }
    /* killough 7/18/98, 9/9/98: new monster AI */
    else if (help_friends && P_HelpFriend(actor))
      return;      /* killough 9/8/98: Help friends in need */
    /* Look for new targets if current one is bad or is out of view */
    else if (actor->pursuecount)
      actor->pursuecount--;
    else {
      /* Our pursuit time has expired. We're going to think about
       * changing targets */
      actor->pursuecount = BASETHRESHOLD;

      // look for new target, unless conditions are met
      if (
        !(
          actor->target &&                       // have a target
          actor->target->health > 0 &&           // and the target is alive
          (
            (comp[comp_pursuit] && !netgame) ||  // and using old pursuit behaviour
            (
              (                                  // or the target is not friendly
                (actor->target->flags ^ actor->flags) & MF_FRIEND ||
                (!(actor->flags & MF_FRIEND) && monster_infighting)
              ) &&
              P_CheckSight(actor, actor->target) // and we can see it
            )
          )
        ) &&
        P_LookForTargets(actor, true)
      )
        return;

      /* (Current target was good, or no new target was found.)
       *
       * If monster is a missile-less friend, give up pursuit and
       * return to player, if no attacks have occurred recently.
       */

      if (!actor->info->missilestate && actor->flags & MF_FRIEND) {
        if (actor->flags & MF_JUSTHIT)          /* if recent action, */
          actor->flags &= ~MF_JUSTHIT;          /* keep fighting */
        else if (P_LookForPlayers(actor, true)) /* else return to player */
          return;
      }
    }
  }

  if (actor->strafecount)
    actor->strafecount--;

  // chase towards player
  if (--actor->movecount < 0 || !P_SmartMove(actor))
    P_NewChaseDir(actor);

  // make active sound
  if (actor->info->activesound && P_Random(pr_see) < 3)
  {
    if (heretic && actor->type == HERETIC_MT_WIZARD && P_Random(pr_heretic) < 128)
    {
      S_StartMobjSound(actor, actor->info->seesound);
    }
    else if (heretic && actor->type == HERETIC_MT_SORCERER2)
    {
      S_StartVoidSound(actor->info->activesound);
    }
    else if (hexen && actor->type == HEXEN_MT_BISHOP && P_Random(pr_hexen) < 128)
    {
      S_StartMobjSound(actor, actor->info->seesound);
    }
    else if (hexen && actor->type == HEXEN_MT_PIG)
    {
      S_StartMobjSound(actor, hexen_sfx_pig_active1 + (P_Random(pr_hexen) & 1));
    }
    else if (hexen && actor->flags2 & MF2_BOSS)
    {
      S_StartVoidSound(actor->info->activesound);
    }
    else
    {
      S_StartMobjSound(actor, actor->info->activesound);
    }
  }
}

//
// A_FaceTarget
//
void A_FaceTarget(mobj_t *actor)
{
  if (!actor->target)
    return;
  actor->flags &= ~MF_AMBUSH;
  actor->angle = R_PointToAngle2(actor->x, actor->y,
                                 actor->target->x, actor->target->y);
  if (actor->target->flags & MF_SHADOW)
    { // killough 5/5/98: remove dependence on order of evaluation:
      int t = P_Random(pr_facetarget);
      actor->angle += (t-P_Random(pr_facetarget))<<21;
    }
}

//
// A_PosAttack
//

void A_PosAttack(mobj_t *actor)
{
  int angle, damage, slope, t;

  if (!actor->target)
    return;
  A_FaceTarget(actor);
  angle = actor->angle;
  slope = P_AimLineAttack(actor, angle, MISSILERANGE, 0); /* killough 8/2/98 */
  S_StartMobjSound(actor, sfx_pistol);

  // killough 5/5/98: remove dependence on order of evaluation:
  t = P_Random(pr_posattack);
  angle += (t - P_Random(pr_posattack))<<20;
  damage = (P_Random(pr_posattack)%5 + 1)*3;
  P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

void A_SPosAttack(mobj_t* actor)
{
  int i, bangle, slope;

  if (!actor->target)
    return;
  S_StartMobjSound(actor, sfx_shotgn);
  A_FaceTarget(actor);
  bangle = actor->angle;
  slope = P_AimLineAttack(actor, bangle, MISSILERANGE, 0); /* killough 8/2/98 */
  for (i=0; i<3; i++)
    {  // killough 5/5/98: remove dependence on order of evaluation:
      int t = P_Random(pr_sposattack);
      int angle = bangle + ((t - P_Random(pr_sposattack))<<20);
      int damage = ((P_Random(pr_sposattack)%5)+1)*3;
      P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
    }
}

void A_CPosAttack(mobj_t *actor)
{
  int angle, bangle, damage, slope, t;

  if (!actor->target)
    return;
  S_StartMobjSound(actor, sfx_shotgn);
  A_FaceTarget(actor);
  bangle = actor->angle;
  slope = P_AimLineAttack(actor, bangle, MISSILERANGE, 0); /* killough 8/2/98 */

  // killough 5/5/98: remove dependence on order of evaluation:
  t = P_Random(pr_cposattack);
  angle = bangle + ((t - P_Random(pr_cposattack))<<20);
  damage = ((P_Random(pr_cposattack)%5)+1)*3;
  P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

void A_CPosRefire(mobj_t *actor)
{
  // keep firing unless target got out of sight
  A_FaceTarget(actor);

  /* killough 12/98: Stop firing if a friend has gotten in the way */
  if (P_HitFriend(actor))
    goto stop;

  /* killough 11/98: prevent refiring on friends continuously */
  if (P_Random(pr_cposrefire) < 40) {
    if (actor->target && actor->flags & actor->target->flags & MF_FRIEND)
      goto stop;
    else
      return;
  }

  if (!actor->target || actor->target->health <= 0
      || !P_CheckSight(actor, actor->target))
stop:  P_SetMobjState(actor, actor->info->seestate);
}

void A_SpidRefire(mobj_t* actor)
{
  // keep firing unless target got out of sight
  A_FaceTarget(actor);

  /* killough 12/98: Stop firing if a friend has gotten in the way */
  if (P_HitFriend(actor))
    goto stop;

  if (P_Random(pr_spidrefire) < 10)
    return;

  // killough 11/98: prevent refiring on friends continuously
  if (!actor->target || actor->target->health <= 0
      || actor->flags & actor->target->flags & MF_FRIEND
      || !P_CheckSight(actor, actor->target))
    stop: P_SetMobjState(actor, actor->info->seestate);
}

void A_BspiAttack(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  P_SpawnMissile(actor, actor->target, MT_ARACHPLAZ);  // launch a missile
}

//
// A_TroopAttack
//

void A_TroopAttack(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  if (P_CheckMeleeRange(actor))
    {
      int damage;
      S_StartMobjSound(actor, sfx_claw);
      damage = (P_Random(pr_troopattack)%8+1)*3;
      P_DamageMobj(actor->target, actor, actor, damage);
      return;
    }
  P_SpawnMissile(actor, actor->target, MT_TROOPSHOT);  // launch a missile
}

void A_SargAttack(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  if (compatibility_level == doom_12_compatibility)
  {
    int damage = ((P_Random(pr_sargattack)%10)+1)*4;
    P_LineAttack(actor, actor->angle, MELEERANGE, 0, damage);
  }
  else
  {
  if (P_CheckMeleeRange(actor))
    {
      int damage = ((P_Random(pr_sargattack)%10)+1)*4;
      P_DamageMobj(actor->target, actor, actor, damage);
    }
  }
}

void A_HeadAttack(mobj_t * actor)
{
  int i;
  mobj_t *fire;
  mobj_t *baseFire;
  mobj_t *mo;
  mobj_t *target;
  int randAttack;
  static int atkResolve1[] = { 50, 150 };
  static int atkResolve2[] = { 150, 200 };
  int dist;

  // Ice ball     (close 20% : far 60%)
  // Fire column  (close 40% : far 20%)
  // Whirlwind    (close 40% : far 20%)
  // Distance threshold = 8 cells

  target = actor->target;
  if (target == NULL) return;

  A_FaceTarget(actor);

  if (P_CheckMeleeRange(actor))
  {
    int damage = heretic ? HITDICE(6) : (P_Random(pr_headattack) % 6 + 1) * 10;
    P_DamageMobj(target, actor, actor, damage);
    return;
  }

  if (!heretic) {
    P_SpawnMissile(actor, target, MT_HEADSHOT);
    return;
  }

  dist = P_AproxDistance(actor->x - target->x, actor->y - target->y)
         > 8 * 64 * FRACUNIT;
  randAttack = P_Random(pr_heretic);
  if (randAttack < atkResolve1[dist])
  {                           // Ice ball
    P_SpawnMissile(actor, target, HERETIC_MT_HEADFX1);
    S_StartMobjSound(actor, heretic_sfx_hedat2);
  }
  else if (randAttack < atkResolve2[dist])
  {                           // Fire column
    baseFire = P_SpawnMissile(actor, target, HERETIC_MT_HEADFX3);
    if (baseFire != NULL)
    {
      P_SetMobjState(baseFire, HERETIC_S_HEADFX3_4);      // Don't grow
      for (i = 0; i < 5; i++)
      {
        fire = P_SpawnMobj(baseFire->x, baseFire->y,
                           baseFire->z, HERETIC_MT_HEADFX3);
        if (i == 0)
        {
          S_StartMobjSound(actor, heretic_sfx_hedat1);
        }
        P_SetTarget(&fire->target, baseFire->target);
        fire->angle = baseFire->angle;
        fire->momx = baseFire->momx;
        fire->momy = baseFire->momy;
        fire->momz = baseFire->momz;
        fire->damage = 0;
        fire->health = (i + 1) * 2;
        P_CheckMissileSpawn(fire);
      }
    }
  }
  else
  {                           // Whirlwind
    mo = P_SpawnMissile(actor, target, HERETIC_MT_WHIRLWIND);
    if (mo != NULL)
    {
      mo->z -= 32 * FRACUNIT;
      P_SetTarget(&mo->special1.m, target);
      mo->special2.i = 50;  // Timer for active sound
      mo->health = 20 * TICRATE;       // Duration
      S_StartMobjSound(actor, heretic_sfx_hedat3);
    }
  }
}

void A_CyberAttack(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  P_SpawnMissile(actor, actor->target, MT_ROCKET);
}

void A_BruisAttack(mobj_t *actor)
{
  if (!actor->target)
    return;
  if (P_CheckMeleeRange(actor))
    {
      int damage;
      S_StartMobjSound(actor, sfx_claw);
      damage = (P_Random(pr_bruisattack)%8+1)*10;
      P_DamageMobj(actor->target, actor, actor, damage);
      return;
    }
  P_SpawnMissile(actor, actor->target, MT_BRUISERSHOT);  // launch a missile
}

//
// A_SkelMissile
//

void A_SkelMissile(mobj_t *actor)
{
  mobj_t *mo;

  if (!actor->target)
    return;

  A_FaceTarget (actor);
  actor->z += 16*FRACUNIT;      // so missile spawns higher
  mo = P_SpawnMissile (actor, actor->target, MT_TRACER);
  actor->z -= 16*FRACUNIT;      // back to normal

  mo->x += mo->momx;
  mo->y += mo->momy;
  P_SetTarget(&mo->tracer, actor->target);
}

int     TRACEANGLE = 0xc000000;

void A_Tracer(mobj_t *actor)
{
  angle_t       exact;
  fixed_t       dist;
  fixed_t       slope;
  mobj_t        *dest;
  mobj_t        *th;

  /* killough 1/18/98: this is why some missiles do not have smoke
   * and some do. Also, internal demos start at random gametics, thus
   * the bug in which revenants cause internal demos to go out of sync.
   *
   * killough 3/6/98: fix revenant internal demo bug by subtracting
   * levelstarttic from gametic.
   *
   * killough 9/29/98: use new "basetic" so that demos stay in sync
   * during pauses and menu activations, while retaining old demo sync.
   *
   * leveltime would have been better to use to start with in Doom, but
   * since old demos were recorded using gametic, we must stick with it,
   * and improvise around it (using leveltime causes desync across levels).
   */

  if (boom_logictic & 3)
    return;

  // spawn a puff of smoke behind the rocket
  P_SpawnPuff(actor->x, actor->y, actor->z);

  th = P_SpawnMobj (actor->x-actor->momx,
                    actor->y-actor->momy,
                    actor->z, MT_SMOKE);

  th->momz = FRACUNIT;
  th->tics -= P_Random(pr_tracer) & 3;
  if (th->tics < 1)
    th->tics = 1;

  // adjust direction
  dest = actor->tracer;

  if (!dest || dest->health <= 0)
    return;

  // change angle
  exact = R_PointToAngle2(actor->x, actor->y, dest->x, dest->y);

  if (exact != actor->angle) {
    if (exact - actor->angle > 0x80000000)
      {
        actor->angle -= TRACEANGLE;
        if (exact - actor->angle < 0x80000000)
          actor->angle = exact;
      }
    else
      {
        actor->angle += TRACEANGLE;
        if (exact - actor->angle > 0x80000000)
          actor->angle = exact;
      }
  }

  exact = actor->angle>>ANGLETOFINESHIFT;
  actor->momx = FixedMul(actor->info->speed, finecosine[exact]);
  actor->momy = FixedMul(actor->info->speed, finesine[exact]);

  // change slope
  dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);

  dist = dist / actor->info->speed;

  if (dist < 1)
    dist = 1;

  slope = (dest->z+40*FRACUNIT - actor->z) / dist;

  if (slope < actor->momz)
    actor->momz -= FRACUNIT/8;
  else
    actor->momz += FRACUNIT/8;
}

void A_SkelWhoosh(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  S_StartMobjSound(actor,sfx_skeswg);
}

void A_SkelFist(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  if (P_CheckMeleeRange(actor))
    {
      int damage = ((P_Random(pr_skelfist)%10)+1)*6;
      S_StartMobjSound(actor, sfx_skepch);
      P_DamageMobj(actor->target, actor, actor, damage);
    }
}

//
// PIT_VileCheck
// Detect a corpse that could be raised.
//

mobj_t* corpsehit;
mobj_t* vileobj;
fixed_t viletryx;
fixed_t viletryy;
int viletryradius;

static dboolean PIT_VileCheck(mobj_t *thing)
{
  int     maxdist;
  dboolean check;

  if (!(thing->flags & MF_CORPSE) )
    return true;        // not a monster

  if (thing->tics != -1)
    return true;        // not lying still yet

  if (thing->info->raisestate == g_s_null)
    return true;        // monster doesn't have a raise state

  maxdist = thing->info->radius + viletryradius;

  if (D_abs(thing->x-viletryx) > maxdist || D_abs(thing->y-viletryy) > maxdist)
    return true;                // not actually touching

// Check to see if the radius and height are zero. If they are      // phares
// then this is a crushed monster that has been turned into a       //   |
// gib. One of the options may be to ignore this guy.               //   V

// Option 1: the original, buggy method, -> ghost (compatibility)
// Option 2: ressurect the monster, but not as a ghost
// Option 3: ignore the gib

//    if (Option3)                                                  //   ^
//        if ((thing->height == 0) && (thing->radius == 0))         //   |
//            return true;                                          // phares

    corpsehit = thing;
    corpsehit->momx = corpsehit->momy = 0;
    if (comp[comp_vile])                                            // phares
      {                                                             //   |
        corpsehit->height <<= 2;                                    //   V
        check = P_CheckPosition(corpsehit,corpsehit->x,corpsehit->y);
        corpsehit->height >>= 2;
      }
    else
      {
        int height,radius;

        height = corpsehit->height; // save temporarily
        radius = corpsehit->radius; // save temporarily
        corpsehit->height = corpsehit->info->height;
        corpsehit->radius = corpsehit->info->radius;
        corpsehit->flags |= MF_SOLID;
        check = P_CheckPosition(corpsehit,corpsehit->x,corpsehit->y);
        corpsehit->height = height; // restore
        corpsehit->radius = radius; // restore                      //   ^
        corpsehit->flags &= ~MF_SOLID;
      }                                                             //   |
                                                                    // phares
    if (!check)
      return true;              // doesn't fit here
    return false;               // got one, so stop checking
}

dboolean P_RaiseThing(mobj_t *corpse, mobj_t *raiser)
{
  uint64_t oldflags;
  fixed_t oldheight, oldradius;
  mobjinfo_t *info;

  if (!(corpse->flags & MF_CORPSE))
    return false;

  info = corpse->info;

  if (info->raisestate == g_s_null)
    return false;

  corpse->momx = 0;
  corpse->momy = 0;

  oldheight = corpse->height;
  oldradius = corpse->radius;
  oldflags = corpse->flags;

  corpse->height = info->height;
  corpse->radius = info->radius;
  corpse->flags |= MF_SOLID;

  if (!P_CheckPosition(corpse, corpse->x, corpse->y))
  {
    corpse->height = oldheight;
    corpse->radius = oldradius;
    corpse->flags = oldflags;
    return false;
  }

  S_StartMobjSound(corpse, sfx_slop);

  P_SetMobjState(corpse, info->raisestate);

  corpse->flags = info->flags;
  corpse->flags |= MF_RESSURECTED;
  corpse->flags &= ~MF_JUSTHIT;

  if (raiser)
  {
    corpse->flags = (corpse->flags & ~MF_FRIEND) | (raiser->flags & MF_FRIEND);
  }

  dsda_WatchResurrection(corpse, raiser);

  if (!((corpse->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    totallive++;

  corpse->health = P_MobjSpawnHealth(corpse);
  corpse->color = 0;
  P_SetTarget(&corpse->target, NULL);
  P_SetTarget(&corpse->lastenemy, NULL);

  P_UpdateThinker(&corpse->thinker);

  return true;
}

//
// P_HealCorpse
// Check for ressurecting a body
//

static dboolean P_HealCorpse(mobj_t* actor, int radius, statenum_t healstate, sfxenum_t healsound)
{
  int xl, xh;
  int yl, yh;
  int bx, by;

  if (actor->movedir != DI_NODIR)
  {
    // check for corpses to raise
    viletryx =
      actor->x + actor->info->speed*xspeed[actor->movedir];
    viletryy =
      actor->y + actor->info->speed*yspeed[actor->movedir];

    xl = P_GetSafeBlockX(viletryx - bmaporgx - MAXRADIUS*2);
    xh = P_GetSafeBlockX(viletryx - bmaporgx + MAXRADIUS*2);
    yl = P_GetSafeBlockY(viletryy - bmaporgy - MAXRADIUS*2);
    yh = P_GetSafeBlockY(viletryy - bmaporgy + MAXRADIUS*2);

    vileobj = actor;
    viletryradius = radius;
    for (bx=xl ; bx<=xh ; bx++)
    {
      for (by=yl ; by<=yh ; by++)
      {
        // Call PIT_VileCheck to check
        // whether object is a corpse
        // that canbe raised.
        if (!P_BlockThingsIterator(bx,by,PIT_VileCheck))
        {
          mobjinfo_t *info;

          // got one!
          mobj_t* temp = actor->target;
          actor->target = corpsehit;
          A_FaceTarget(actor);
          actor->target = temp;

          P_SetMobjState(actor, healstate);
          S_StartMobjSound(corpsehit, healsound);
          info = corpsehit->info;

          P_SetMobjState(corpsehit,info->raisestate);

          if (comp[comp_vile])                              // phares
            corpsehit->height <<= 2;                        //   |
          else                                              //   V
          {
            corpsehit->height = info->height; // fix Ghost bug
            corpsehit->radius = info->radius; // fix Ghost bug
          }                                                 // phares

          /* killough 7/18/98:
          * friendliness is transferred from AV to raised corpse
          */
          corpsehit->flags =
            (info->flags & ~MF_FRIEND) | (actor->flags & MF_FRIEND);
          corpsehit->flags = corpsehit->flags | MF_RESSURECTED;//e6y

          dsda_WatchResurrection(corpsehit, actor);

          if (!((corpsehit->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
            totallive++;

          corpsehit->health = P_MobjSpawnHealth(corpsehit);
          corpsehit->color = 0;
          P_SetTarget(&corpsehit->target, NULL);  // killough 11/98

          if (mbf_features)
          {         /* kilough 9/9/98 */
            P_SetTarget(&corpsehit->lastenemy, NULL);
            corpsehit->flags &= ~MF_JUSTHIT;
          }

          /* killough 8/29/98: add to appropriate thread */
          P_UpdateThinker(&corpsehit->thinker);

          return true;
        }
      }
    }
  }
  return false;
}

//
// A_VileChase
//

void A_VileChase(mobj_t* actor)
{
  if (!P_HealCorpse(actor, mobjinfo[MT_VILE].radius, S_VILE_HEAL1, sfx_slop))
    A_Chase(actor);  // Return to normal attack.
}

//
// A_VileStart
//

void A_VileStart(mobj_t *actor)
{
  S_StartMobjSound(actor, sfx_vilatk);
}

//
// A_Fire
// Keep fire in front of player unless out of sight
//

void A_StartFire(mobj_t *actor)
{
  S_StartMobjSound(actor,sfx_flamst);
  A_Fire(actor);
}

void A_FireCrackle(mobj_t* actor)
{
  S_StartMobjSound(actor,sfx_flame);
  A_Fire(actor);
}

void A_Fire(mobj_t *actor)
{
  mobj_t* target;
  unsigned an;
  mobj_t *dest = actor->tracer;

  if (!dest)
    return;

  target = P_SubstNullMobj(actor->target);

  // don't move it if the vile lost sight
  if (!P_CheckSight(target, dest) )
    return;

  an = dest->angle >> ANGLETOFINESHIFT;

  P_UnsetThingPosition(actor);
  actor->x = dest->x + FixedMul(24*FRACUNIT, finecosine[an]);
  actor->y = dest->y + FixedMul(24*FRACUNIT, finesine[an]);
  actor->z = dest->z;
  P_SetThingPosition(actor);
}

//
// A_VileTarget
// Spawn the hellfire
//

void A_VileTarget(mobj_t *actor)
{
  mobj_t *fog;

  if (!actor->target)
    return;

  A_FaceTarget(actor);

  // killough 12/98: fix Vile fog coordinates // CPhipps - compatibility optioned
  fog = P_SpawnMobj(actor->target->x,
    (compatibility_level < lxdoom_1_compatibility) ? actor->target->x : actor->target->y,
                    actor->target->z,MT_FIRE);

  P_SetTarget(&actor->tracer, fog);
  P_SetTarget(&fog->target, actor);
  P_SetTarget(&fog->tracer, actor->target);
  A_Fire(fog);
}

//
// A_VileAttack
//

void A_VileAttack(mobj_t *actor)
{
  mobj_t *fire;
  int    an;

  if (!actor->target)
    return;

  A_FaceTarget(actor);

  if (!P_CheckSight(actor, actor->target))
    return;

  S_StartMobjSound(actor, sfx_barexp);
  P_DamageMobj(actor->target, actor, actor, 20);
  actor->target->momz = 1000*FRACUNIT/actor->target->info->mass;

  an = actor->angle >> ANGLETOFINESHIFT;

  fire = actor->tracer;

  if (!fire)
    return;

  // move the fire between the vile and the player
  fire->x = actor->target->x - FixedMul (24*FRACUNIT, finecosine[an]);
  fire->y = actor->target->y - FixedMul (24*FRACUNIT, finesine[an]);
  P_RadiusAttack(fire, actor, 70, 70, true);
}

//
// Mancubus attack,
// firing three missiles (bruisers)
// in three different directions?
// Doesn't look like it.
//

#define FATSPREAD       (ANG90/8)

void A_FatRaise(mobj_t *actor)
{
  A_FaceTarget(actor);
  S_StartMobjSound(actor, sfx_manatk);
}

void A_FatAttack1(mobj_t *actor)
{
  mobj_t *mo;
  mobj_t* target;
  int    an;

  if (!actor->target)
    return;

  A_FaceTarget(actor);

  // Change direction  to ...
  actor->angle += FATSPREAD;
  target = P_SubstNullMobj(actor->target);
  P_SpawnMissile(actor, target, MT_FATSHOT);

  mo = P_SpawnMissile (actor, target, MT_FATSHOT);
  mo->angle += FATSPREAD;
  an = mo->angle >> ANGLETOFINESHIFT;
  mo->momx = FixedMul(mo->info->speed, finecosine[an]);
  mo->momy = FixedMul(mo->info->speed, finesine[an]);
}

void A_FatAttack2(mobj_t *actor)
{
  mobj_t *mo;
  mobj_t* target;
  int    an;

  if (!actor->target)
    return;

  A_FaceTarget(actor);
  // Now here choose opposite deviation.
  actor->angle -= FATSPREAD;
  target = P_SubstNullMobj(actor->target);
  P_SpawnMissile(actor, target, MT_FATSHOT);

  mo = P_SpawnMissile(actor, target, MT_FATSHOT);
  mo->angle -= FATSPREAD*2;
  an = mo->angle >> ANGLETOFINESHIFT;
  mo->momx = FixedMul(mo->info->speed, finecosine[an]);
  mo->momy = FixedMul(mo->info->speed, finesine[an]);
}

void A_FatAttack3(mobj_t *actor)
{
  mobj_t *mo;
  mobj_t* target;
  int    an;

  if (!actor->target)
    return;

  A_FaceTarget(actor);

  target = P_SubstNullMobj(actor->target);

  mo = P_SpawnMissile(actor, target, MT_FATSHOT);
  mo->angle -= FATSPREAD/2;
  an = mo->angle >> ANGLETOFINESHIFT;
  mo->momx = FixedMul(mo->info->speed, finecosine[an]);
  mo->momy = FixedMul(mo->info->speed, finesine[an]);

  mo = P_SpawnMissile(actor, target, MT_FATSHOT);
  mo->angle += FATSPREAD/2;
  an = mo->angle >> ANGLETOFINESHIFT;
  mo->momx = FixedMul(mo->info->speed, finecosine[an]);
  mo->momy = FixedMul(mo->info->speed, finesine[an]);
}


//
// SkullAttack
// Fly at the player like a missile.
//
#define SKULLSPEED              (20*FRACUNIT)

void A_SkullAttack(mobj_t *actor)
{
  mobj_t  *dest;
  angle_t an;
  int     dist;

  if (!actor->target)
    return;

  dest = actor->target;
  actor->flags |= MF_SKULLFLY;

  S_StartMobjSound(actor, actor->info->attacksound);
  A_FaceTarget(actor);
  an = actor->angle >> ANGLETOFINESHIFT;
  actor->momx = FixedMul(SKULLSPEED, finecosine[an]);
  actor->momy = FixedMul(SKULLSPEED, finesine[an]);
  dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
  dist = dist / SKULLSPEED;

  if (dist < 1)
    dist = 1;
  actor->momz = (dest->z+(dest->height>>1) - actor->z) / dist;
}

void A_BetaSkullAttack(mobj_t *actor)
{
  int damage;

  if (compatibility_level < mbf_compatibility)
    return;

  if (!actor->target || actor->target->type == MT_SKULL)
    return;

  S_StartMobjSound(actor, actor->info->attacksound);
  A_FaceTarget(actor);
  damage = (P_Random(pr_skullfly)%8+1)*actor->info->damage;
  P_DamageMobj(actor->target, actor, actor, damage);
}

void A_Stop(mobj_t *actor)
{
  if (compatibility_level < mbf_compatibility)
    return;

  actor->momx = actor->momy = actor->momz = 0;
}

//
// A_PainShootSkull
// Spawn a lost soul and launch it at the target
//

static void A_PainShootSkull(mobj_t *actor, angle_t angle)
{
  fixed_t       x,y,z;
  mobj_t        *newmobj;
  angle_t       an;
  int           prestep;

// The original code checked for 20 skulls on the level,            // phares
// and wouldn't spit another one if there were. If not in           // phares
// compatibility mode, we remove the limit.                         // phares
                                                                    // phares
  if (comp[comp_pain]) /* killough 10/98: compatibility-optioned */
    {
      // count total number of skulls currently on the level
      int count = 0;
      thinker_t *currentthinker = NULL;
      while ((currentthinker = P_NextThinker(currentthinker,th_all)) != NULL)
        if ((currentthinker->function == P_MobjThinker)
            && ((mobj_t *)currentthinker)->type == MT_SKULL)
          count++;
      if (count > 20)                                               // phares
        return;                                                     // phares
    }

  // okay, there's room for another one

  an = angle >> ANGLETOFINESHIFT;

  prestep = 4*FRACUNIT + 3*(actor->info->radius + mobjinfo[MT_SKULL].radius)/2;

  x = actor->x + FixedMul(prestep, finecosine[an]);
  y = actor->y + FixedMul(prestep, finesine[an]);
  z = actor->z + 8*FRACUNIT;

  if (comp[comp_skull])   /* killough 10/98: compatibility-optioned */
    newmobj = P_SpawnMobj(x, y, z, MT_SKULL);                     // phares
  else                                                            //   V
    {
      // Check whether the Lost Soul is being fired through a 1-sided
      // wall or an impassible line, or a "monsters can't cross" line.
      // If it is, then we don't allow the spawn. This is a bug fix, but
      // it should be considered an enhancement, since it may disturb
      // existing demos, so don't do it in compatibility mode.

      if (Check_Sides(actor,x,y))
        return;

      newmobj = P_SpawnMobj(x, y, z, MT_SKULL);

      // Check to see if the new Lost Soul's z value is above the
      // ceiling of its new sector, or below the floor. If so, kill it.

      if ((newmobj->z >
           (newmobj->subsector->sector->ceilingheight - newmobj->height)) ||
          (newmobj->z < newmobj->subsector->sector->floorheight))
        {
          // kill it immediately
          P_DamageMobj(newmobj,actor,actor,10000);
          return;                                                 //   ^
        }                                                         //   |
     }                                                            // phares

  /* killough 7/20/98: PEs shoot lost souls with the same friendliness */
  newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (actor->flags & MF_FRIEND);

  /* killough 8/29/98: add to appropriate thread */
  P_UpdateThinker(&newmobj->thinker);

  // Check for movements.
  // killough 3/15/98: don't jump over dropoffs:

  if (!P_TryMove(newmobj, newmobj->x, newmobj->y, false))
    {
      // kill it immediately
      P_DamageMobj(newmobj, actor, actor, 10000);
      return;
    }

  P_SetTarget(&newmobj->target, actor->target);
  A_SkullAttack(newmobj);
}

//
// A_PainAttack
// Spawn a lost soul and launch it at the target
//

void A_PainAttack(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  A_PainShootSkull(actor, actor->angle);
}

void A_PainDie(mobj_t *actor)
{
  A_Fall(actor);
  A_PainShootSkull(actor, actor->angle+ANG90);
  A_PainShootSkull(actor, actor->angle+ANG180);
  A_PainShootSkull(actor, actor->angle+ANG270);
}

void Heretic_A_Scream(mobj_t *actor);
void Hexen_A_Scream(mobj_t *actor);

void A_Scream(mobj_t *actor)
{
  int sound;

  if (heretic) return Heretic_A_Scream(actor);
  if (hexen) return Hexen_A_Scream(actor);

  switch (actor->info->deathsound)
    {
    case 0:
      return;

    case sfx_podth1:
    case sfx_podth2:
    case sfx_podth3:
      sound = sfx_podth1 + P_Random(pr_scream)%3;
      break;

    case sfx_bgdth1:
    case sfx_bgdth2:
      sound = sfx_bgdth1 + P_Random(pr_scream)%2;
      break;

    default:
      sound = actor->info->deathsound;
      break;
    }

  // Check for bosses.
  if (actor->flags2 & (MF2_BOSS | MF2_FULLVOLSOUNDS))
    S_StartVoidSound(sound); // full volume
  else
    S_StartMobjSound(actor, sound);
}

void A_XScream(mobj_t *actor)
{
  S_StartMobjSound(actor, sfx_slop);
}

void A_SkullPop(mobj_t *actor)
{
  mobj_t *mo;
  player_t *player;
  int sfx_id;

  if (!raven && (demorecording || demoplayback))
    return;

  if (!raven) {
    sfx_id = (I_GetSfxLumpNum(&S_sfx[sfx_gibdth]) < 0 ? sfx_pldeth : sfx_gibdth);
    S_StartMobjSound(actor, sfx_id);
  }

  if (hexen && !actor->player)
  {
    return;
  }

  actor->flags &= ~MF_SOLID;
  mo = P_SpawnMobj(actor->x, actor->y, actor->z + 48 * FRACUNIT, g_skullpop_mt);
  //mo->target = actor;
  mo->momx = P_SubRandom() << 9;
  mo->momy = P_SubRandom() << 9;
  mo->momz = FRACUNIT * 2 + (P_Random(pr_heretic) << 6);
  // Attach player mobj to bloody skull
  player = actor->player;
  actor->player = NULL;
  if (hexen)
    actor->special1.i = player->pclass;
  mo->player = player;
  mo->health = actor->health;
  mo->angle = actor->angle;
  mo->pitch = 0;
  if (player)
  {
    player->mo = mo;
    player->lookdir = 0;
    player->damagecount = 32;
  }
}

void A_Pain(mobj_t *actor)
{
  if (actor->info->painsound)
    S_StartMobjSound(actor, actor->info->painsound);
}

void A_Fall(mobj_t *actor)
{
  // actor is on ground, it can be walked over
  actor->flags &= ~MF_SOLID;
}

//
// A_Explode
//
void A_Explode(mobj_t *thingy)
{
  int damage;
  int distance;
  dboolean damageSelf;

  damage = 128;
  distance = 128;
  damageSelf = true;

  if (raven)
  {
    switch (thingy->type)
    {
      case HERETIC_MT_FIREBOMB:      // Time Bombs
      case HEXEN_MT_FIREBOMB:        // Time Bombs
        thingy->z += 32 * FRACUNIT;
        thingy->flags &= ~MF_SHADOW;
        break;
      case HERETIC_MT_MNTRFX2:       // Minotaur floor fire
        damage = 24;
        distance = damage;
        break;
      case HERETIC_MT_SOR2FX1:       // D'Sparil missile
        damage = 80 + (P_Random(pr_heretic) & 31);
        distance = damage;
        break;
      case HEXEN_MT_MNTRFX2:       // Minotaur floor fire
        damage = 24;
        break;
      case HEXEN_MT_BISHOP:        // Bishop radius death
        damage = 25 + (P_Random(pr_hexen) & 15);
        break;
      case HEXEN_MT_HAMMER_MISSILE:        // Fighter Hammer
        damage = 128;
        damageSelf = false;
        break;
      case HEXEN_MT_FSWORD_MISSILE:        // Fighter Runesword
        damage = 64;
        damageSelf = false;
        break;
      case HEXEN_MT_CIRCLEFLAME:   // Cleric Flame secondary flames
        damage = 20;
        damageSelf = false;
        break;
      case HEXEN_MT_SORCBALL1:     // Sorcerer balls
      case HEXEN_MT_SORCBALL2:
      case HEXEN_MT_SORCBALL3:
        distance = 255;
        damage = 255;
        thingy->special_args[0] = 1; // don't play bounce
        break;
      case HEXEN_MT_SORCFX1:       // Sorcerer spell 1
        damage = 30;
        break;
      case HEXEN_MT_SORCFX4:       // Sorcerer spell 4
        damage = 20;
        break;
      case HEXEN_MT_TREEDESTRUCTIBLE:
        damage = 10;
        break;
      case HEXEN_MT_DRAGON_FX2:
        damage = 80;
        damageSelf = false;
        break;
      case HEXEN_MT_MSTAFF_FX:
        damage = 64;
        distance = 192;
        damageSelf = false;
        break;
      case HEXEN_MT_MSTAFF_FX2:
        damage = 80;
        distance = 192;
        damageSelf = false;
        break;
      case HEXEN_MT_POISONCLOUD:
        damage = 4;
        distance = 40;
        break;
      case HEXEN_MT_ZXMAS_TREE:
      case HEXEN_MT_ZSHRUB2:
        damage = 30;
        distance = 64;
        break;
      default:
        break;
    }
  }

  P_RadiusAttack(thingy, thingy->target, damage, distance, damageSelf);
  if (
    heretic ||
    (
      hexen &&
      thingy->z <= thingy->floorz + (distance << FRACBITS) &&
      thingy->type != HEXEN_MT_POISONCLOUD
    )
  ) P_HitFloor(thingy);
}

//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//

dboolean P_CheckBossDeath(mobj_t *mo)
{
  int i;
  thinker_t* th;

  // make sure there is a player alive for victory
  for (i = 0; i < g_maxplayers; i++)
    if (playeringame[i] && players[i].health > 0)
      break;

  if (i == g_maxplayers)
    return false; // no one left alive, so do not end game

  // scan the remaining thinkers to see
  // if all bosses are dead
  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (th->function == P_MobjThinker) {
      mobj_t* mo2 = (mobj_t*) th;

      if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
        return false; // other boss not dead
    }

  return true;
}

void A_BossDeath(mobj_t *mo)
{
  line_t junk;

  // heretic_note: probably we can adopt the clean heretic style and merge
  if (heretic) return Heretic_A_BossDeath(mo);

  if (dsda_BossAction(mo))
  {
    return;
  }

  if (gamemode == commercial)
  {
    if (gamemap != 7)
      return;

    if (!(mo->flags2 & (MF2_MAP07BOSS1 | MF2_MAP07BOSS2)))
      return;
  }
  else
  {
    // e6y
    // Additional check of gameepisode is necessary, because
    // there is no right or wrong solution for E4M6 in original EXEs,
    // there's nothing to emulate.
    if (comp[comp_666] && gameepisode < 4)
    {
      // e6y
      // Only following checks are present in doom2.exe ver. 1.666 and 1.9
      // instead of separate checks for each episode in doomult.exe, plutonia.exe and tnt.exe
      // There is no more desync on doom.wad\episode3.lmp
      // http://www.doomworld.com/idgames/index.php?id=6909
      if (gamemap != 8)
        return;
      if (mo->flags2 & MF2_E1M8BOSS && gameepisode != 1)
        return;
    }
    else
    {
      switch(gameepisode)
      {
      case 1:
        if (gamemap != 8)
          return;

        if (!(mo->flags2 & MF2_E1M8BOSS))
          return;
        break;

      case 2:
        if (gamemap != 8)
          return;

        if (!(mo->flags2 & MF2_E2M8BOSS))
          return;
        break;

      case 3:
        if (gamemap != 8)
          return;

        if (!(mo->flags2 & MF2_E3M8BOSS))
          return;

        break;

      case 4:
        switch(gamemap)
        {
          case 6:
            if (!(mo->flags2 & MF2_E4M6BOSS))
              return;
            break;

          case 8:
            if (!(mo->flags2 & MF2_E4M8BOSS))
              return;
            break;

          default:
            return;
            break;
        }
        break;

      default:
        if (gamemap != 8)
          return;
        break;
      }
    }
  }

  if (!P_CheckBossDeath(mo))
  {
    return;
  }

  // victory!
  if ( gamemode == commercial)
  {
    if (gamemap == 7)
    {
      if (mo->flags2 & MF2_MAP07BOSS1)
      {
        junk.tag = 666;
        EV_DoFloor(&junk,lowerFloorToLowest);
        return;
      }

      if (mo->flags2 & MF2_MAP07BOSS2)
      {
        junk.tag = 667;
        EV_DoFloor(&junk,raiseToTexture);
        return;
      }
    }
  }
  else
  {
    switch(gameepisode)
    {
      case 1:
        junk.tag = 666;
        EV_DoFloor(&junk, lowerFloorToLowest);
        return;
        break;

      case 4:
        switch(gamemap)
        {
          case 6:
            junk.tag = 666;
            EV_DoDoor(&junk, blazeOpen);
            return;
            break;

          case 8:
            junk.tag = 666;
            EV_DoFloor(&junk, lowerFloorToLowest);
            return;
            break;
        }
    }
  }
  G_ExitLevel(0);
}


void A_Hoof (mobj_t* mo)
{
    S_StartMobjSound(mo, sfx_hoof);
    A_Chase(mo);
}

void A_Metal(mobj_t *mo)
{
  S_StartMobjSound(mo, sfx_metal);
  A_Chase(mo);
}

void A_BabyMetal(mobj_t *mo)
{
  S_StartMobjSound(mo, sfx_bspwlk);
  A_Chase(mo);
}

void A_OpenShotgun2(player_t *player, pspdef_t *psp)
{
  S_StartMobjSound(player->mo, sfx_dbopn);
}

void A_LoadShotgun2(player_t *player, pspdef_t *psp)
{
  S_StartMobjSound(player->mo, sfx_dbload);
}

void A_CloseShotgun2(player_t *player, pspdef_t *psp)
{
  S_StartMobjSound(player->mo, sfx_dbcls);
  A_ReFire(player,psp);
}

// killough 2/7/98: Remove limit on icon landings:
mobj_t **braintargets;
int    numbraintargets_alloc;
int    numbraintargets;

struct brain_s brain;   // killough 3/26/98: global state of boss brain

// killough 3/26/98: initialize icon landings at level startup,
// rather than at boss wakeup, to prevent savegame-related crashes

void P_SpawnBrainTargets(void)  // killough 3/26/98: renamed old function
{
  thinker_t *thinker;

  // find all the target spots
  numbraintargets = 0;
  brain.targeton = 0;
  brain.easy = 0;           // killough 3/26/98: always init easy to 0

  for (thinker = thinkercap.next ;
       thinker != &thinkercap ;
       thinker = thinker->next)
    if (thinker->function == P_MobjThinker)
      {
        mobj_t *m = (mobj_t *) thinker;

        if (m->type == MT_BOSSTARGET )
          {   // killough 2/7/98: remove limit on icon landings:
            if (numbraintargets >= numbraintargets_alloc)
              braintargets = Z_Realloc(braintargets,
                      (numbraintargets_alloc = numbraintargets_alloc ?
                       numbraintargets_alloc*2 : 32) *sizeof *braintargets);
            braintargets[numbraintargets++] = m;
          }
      }
}

void A_BrainAwake(mobj_t *mo)
{
  //e6y
  if (demo_compatibility && !prboom_comp[PC_BOOM_BRAINAWAKE].state)
  {
    brain.targeton = 0;
    brain.easy = 0;
  }

  S_StartVoidSound(sfx_bossit); // killough 3/26/98: only generates sound now
}

void A_BrainPain(mobj_t *mo)
{
  S_StartVoidSound(sfx_bospn);
}

void A_BrainScream(mobj_t *mo)
{
  int x;
  for (x=mo->x - 196*FRACUNIT ; x< mo->x + 320*FRACUNIT ; x+= FRACUNIT*8)
    {
      int y = mo->y - 320*FRACUNIT;
      int z = 128 + P_Random(pr_brainscream)*2*FRACUNIT;
      mobj_t *th = P_SpawnMobj (x,y,z, MT_ROCKET);
      th->momz = P_Random(pr_brainscream)*512;
      P_SetMobjState(th, S_BRAINEXPLODE1);
      th->tics -= P_Random(pr_brainscream)&7;
      if (th->tics < 1)
        th->tics = 1;
    }
  S_StartVoidSound(sfx_bosdth);
}

void A_BrainExplode(mobj_t *mo)
{  // killough 5/5/98: remove dependence on order of evaluation:
  int t = P_Random(pr_brainexp);
  int x = mo->x + (t - P_Random(pr_brainexp))*2048;
  int y = mo->y;
  int z = 128 + P_Random(pr_brainexp)*2*FRACUNIT;
  mobj_t *th = P_SpawnMobj(x,y,z, MT_ROCKET);
  th->momz = P_Random(pr_brainexp)*512;
  P_SetMobjState(th, S_BRAINEXPLODE1);
  th->tics -= P_Random(pr_brainexp)&7;
  if (th->tics < 1)
    th->tics = 1;
}

void A_BrainDie(mobj_t *mo)
{
  G_ExitLevel(0);
}

void A_BrainSpit(mobj_t *mo)
{
  mobj_t *targ, *newmobj;

  if (!numbraintargets)     // killough 4/1/98: ignore if no targets
    return;

  brain.easy ^= 1;          // killough 3/26/98: use brain struct
  if (skill_info.flags & SI_EASY_BOSS_BRAIN && !brain.easy)
    return;

  // shoot a cube at current target
  targ = braintargets[brain.targeton++]; // killough 3/26/98:
  brain.targeton %= numbraintargets;     // Use brain struct for targets

  // spawn brain missile
  newmobj = P_SpawnMissile(mo, targ, MT_SPAWNSHOT);

  // e6y: do not crash with 'incorrect' DEHs
  if (!newmobj || !newmobj->state || newmobj->momy == 0 || newmobj->state->tics == 0)
    I_Error("A_BrainSpit: can't spawn brain missile (incorrect DEH)");

  P_SetTarget(&newmobj->target, targ);
  newmobj->reactiontime = (short)(((targ->y-mo->y)/newmobj->momy)/newmobj->state->tics);

  // killough 7/18/98: brain friendliness is transferred
  newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (mo->flags & MF_FRIEND);

  // killough 8/29/98: add to appropriate thread
  P_UpdateThinker(&newmobj->thinker);

  S_StartVoidSound(sfx_bospit);
}

// travelling cube sound
void A_SpawnSound(mobj_t *mo)
{
  S_StartMobjSound(mo,sfx_boscub);
  A_SpawnFly(mo);
}

void A_SpawnFly(mobj_t *mo)
{
  mobj_t *newmobj;
  mobj_t *fog;
  mobj_t *targ;
  int    r;
  mobjtype_t type;

  if (--mo->reactiontime)
    return;     // still flying

  targ = P_SubstNullMobj(mo->target);

  // First spawn teleport fog.
  fog = P_SpawnMobj(targ->x, targ->y, targ->z, MT_SPAWNFIRE);
  S_StartMobjSound(fog, sfx_telept);

  // Randomly select monster to spawn.
  r = P_Random(pr_spawnfly);

  // Probability distribution (kind of :), decreasing likelihood.
  if ( r<50 )
    type = MT_TROOP;
  else if (r<90)
    type = MT_SERGEANT;
  else if (r<120)
    type = MT_SHADOWS;
  else if (r<130)
    type = MT_PAIN;
  else if (r<160)
    type = MT_HEAD;
  else if (r<162)
    type = MT_VILE;
  else if (r<172)
    type = MT_UNDEAD;
  else if (r<192)
    type = MT_BABY;
  else if (r<222)
    type = MT_FATSO;
  else if (r<246)
    type = MT_KNIGHT;
  else
    type = MT_BRUISER;

  newmobj = P_SpawnMobj(targ->x, targ->y, targ->z, type);

  /* killough 7/18/98: brain friendliness is transferred */
  newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (mo->flags & MF_FRIEND);

  //e6y: monsters spawned by Icon of Sin should not be countable for total killed.
  newmobj->flags |= MF_RESSURECTED;

  dsda_WatchIconSpawn(newmobj);

  /* killough 8/29/98: add to appropriate thread */
  P_UpdateThinker(&newmobj->thinker);

  if (P_LookForTargets(newmobj,true))      /* killough 9/4/98 */
    P_SetMobjState(newmobj, newmobj->info->seestate);

    // telefrag anything in this spot
  P_TeleportMove(newmobj, newmobj->x, newmobj->y, true); /* killough 8/9/98 */

  // remove self (i.e., cube).
  P_RemoveMobj(mo);
}

void A_PlayerScream(mobj_t *mo)
{
  int sound = sfx_pldeth;  // Default death sound.
  if (gamemode != shareware && mo->health < -50)
    sound = sfx_pdiehi;   // IF THE PLAYER DIES LESS THAN -50% WITHOUT GIBBING
  S_StartMobjSound(mo, sound);
}

/* cph - MBF-added codepointer functions */

// killough 11/98: kill an object
void A_Die(mobj_t *actor)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  P_DamageMobj(actor, NULL, NULL, actor->health);
}

//
// A_Detonate
// killough 8/9/98: same as A_Explode, except that the damage is variable
//

void A_Detonate(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  P_RadiusAttack(mo, mo->target, mo->info->damage, mo->info->damage, true);
}

//
// killough 9/98: a mushroom explosion effect, sorta :)
// Original idea: Linguica
//

void A_Mushroom(mobj_t *actor)
{
  int i, j, n;

  // Mushroom parameters are part of code pointer's state
  dboolean use_misc;
  fixed_t misc1, misc2;

  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  use_misc = mbf21 || (
    compatibility_level == mbf_compatibility &&
    !prboom_comp[PC_DO_NOT_USE_MISC12_FRAME_PARAMETERS_IN_A_MUSHROOM].state
  );
  misc1 = ((use_misc && actor->state->misc1) ? actor->state->misc1 : FRACUNIT * 4);
  misc2 = ((use_misc && actor->state->misc2) ? actor->state->misc2 : FRACUNIT / 2);
  n = actor->info->damage;

  A_Explode(actor);  // First make normal explosion

  // Now launch mushroom cloud
  for (i = -n; i <= n; i += 8)
    for (j = -n; j <= n; j += 8)
    {
      mobj_t target = *actor, *mo;
      target.x += i << FRACBITS;    // Aim in many directions from source
      target.y += j << FRACBITS;
      target.z += P_AproxDistance(i,j) * misc1;         // Aim up fairly high
      mo = P_SpawnMissile(actor, &target, MT_FATSHOT);  // Launch fireball
      mo->momx = FixedMul(mo->momx, misc2);
      mo->momy = FixedMul(mo->momy, misc2);             // Slow down a bit
      mo->momz = FixedMul(mo->momz, misc2);
      mo->flags &= ~MF_NOGRAVITY;   // Make debris fall under gravity
    }
}

//
// killough 11/98
//
// The following were inspired by Len Pitre
//
// A small set of highly-sought-after code pointers
//

void A_Spawn(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  if (mo->state->misc1)
  {
    mobj_t *newmobj =
      P_SpawnMobj(mo->x, mo->y, (mo->state->misc2 << FRACBITS) + mo->z, mo->state->misc1 - 1);

    if (
      mbf_features &&
      comp[comp_friendlyspawn] &&
      !prboom_comp[PC_DO_NOT_INHERIT_FRIENDLYNESS_FLAG_ON_SPAWN].state
    )
      newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (mo->flags & MF_FRIEND);
  }
}

void A_Turn(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  mo->angle += (unsigned int)(((uint64_t) mo->state->misc1 << 32) / 360);
}

void A_Face(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  mo->angle = (unsigned int)(((uint64_t) mo->state->misc1 << 32) / 360);
}

void A_Scratch(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  mo->target && (A_FaceTarget(mo), P_CheckMeleeRange(mo)) ?
    mo->state->misc2 ? S_StartMobjSound(mo, mo->state->misc2) : (void) 0,
    P_DamageMobj(mo->target, mo, mo, mo->state->misc1) : (void) 0;
}

void A_PlaySound(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  S_StartMobjSound(mo->state->misc2 ? NULL : mo, mo->state->misc1);
}

void A_RandomJump(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  if (P_Random(pr_randomjump) < mo->state->misc2)
    P_SetMobjState(mo, mo->state->misc1);
}

//
// This allows linedef effects to be activated inside deh frames.
//

void A_LineEffect(mobj_t *mo)
{
  static line_t junk;
  player_t player;
  player_t *oldplayer;

  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  junk = *lines;
  oldplayer = mo->player;
  mo->player = &player;
  player.health = 100;
  junk.special = (short)mo->state->misc1;
  if (!junk.special)
    return;
  junk.tag = (short)mo->state->misc2;
  if (!P_UseSpecialLine(mo, &junk, 0, false))
    map_format.cross_special_line(&junk, 0, mo, false);
  mo->state->misc1 = junk.special;
  mo->player = oldplayer;
}

//
// [XA] New mbf21 codepointers
//

//
// A_SpawnObject
// Basically just A_Spawn with better behavior and more args.
//   args[0]: Type of actor to spawn
//   args[1]: Angle (degrees, in fixed point), relative to calling actor's angle
//   args[2]: X spawn offset (fixed point), relative to calling actor
//   args[3]: Y spawn offset (fixed point), relative to calling actor
//   args[4]: Z spawn offset (fixed point), relative to calling actor
//   args[5]: X velocity (fixed point)
//   args[6]: Y velocity (fixed point)
//   args[7]: Z velocity (fixed point)
//
void A_SpawnObject(mobj_t *actor)
{
  int type, angle, ofs_x, ofs_y, ofs_z, vel_x, vel_y, vel_z;
  angle_t an;
  int fan, dx, dy;
  mobj_t *mo;

  if (!mbf21 || !actor->state->args[0])
    return;

  type  = actor->state->args[0] - 1;
  angle = actor->state->args[1];
  ofs_x = actor->state->args[2];
  ofs_y = actor->state->args[3];
  ofs_z = actor->state->args[4];
  vel_x = actor->state->args[5];
  vel_y = actor->state->args[6];
  vel_z = actor->state->args[7];

  // calculate position offsets
  an = actor->angle + (unsigned int)(((int64_t)angle << 16) / 360);
  fan = an >> ANGLETOFINESHIFT;
  dx = FixedMul(ofs_x, finecosine[fan]) - FixedMul(ofs_y, finesine[fan]  );
  dy = FixedMul(ofs_x, finesine[fan]  ) + FixedMul(ofs_y, finecosine[fan]);

  // spawn it, yo
  mo = P_SpawnMobj(actor->x + dx, actor->y + dy, actor->z + ofs_z, type);
  if (!mo)
    return;

  // angle dangle
  mo->angle = an;

  // set velocity
  mo->momx = FixedMul(vel_x, finecosine[fan]) - FixedMul(vel_y, finesine[fan]  );
  mo->momy = FixedMul(vel_x, finesine[fan]  ) + FixedMul(vel_y, finecosine[fan]);
  mo->momz = vel_z;

  // if spawned object is a missile, set target+tracer
  if (mo->info->flags & (MF_MISSILE | MF_BOUNCES))
  {
    // if spawner is also a missile, copy 'em
    if (actor->info->flags & (MF_MISSILE | MF_BOUNCES))
    {
      P_SetTarget(&mo->target, actor->target);
      P_SetTarget(&mo->tracer, actor->tracer);
    }
    // otherwise, set 'em as if a monster fired 'em
    else
    {
      P_SetTarget(&mo->target, actor);
      P_SetTarget(&mo->tracer, actor->target);
    }
  }

  // [XA] don't bother with the dont-inherit-friendliness hack
  // that exists in A_Spawn, 'cause WTF is that about anyway?
}

//
// A_MonsterProjectile
// A parameterized monster projectile attack.
//   args[0]: Type of actor to spawn
//   args[1]: Angle (degrees, in fixed point), relative to calling actor's angle
//   args[2]: Pitch (degrees, in fixed point), relative to calling actor's pitch; approximated
//   args[3]: X/Y spawn offset, relative to calling actor's angle
//   args[4]: Z spawn offset, relative to actor's default projectile fire height
//
void A_MonsterProjectile(mobj_t *actor)
{
  int type, angle, pitch, spawnofs_xy, spawnofs_z;
  mobj_t *mo;
  int an;

  if (!mbf21 || !actor->target || !actor->state->args[0])
    return;

  type        = actor->state->args[0] - 1;
  angle       = actor->state->args[1];
  pitch       = actor->state->args[2];
  spawnofs_xy = actor->state->args[3];
  spawnofs_z  = actor->state->args[4];

  A_FaceTarget(actor);
  mo = P_SpawnMissile(actor, actor->target, type);
  if (!mo)
    return;

  // adjust angle
  mo->angle += (unsigned int)(((int64_t)angle << 16) / 360);
  an = mo->angle >> ANGLETOFINESHIFT;
  mo->momx = FixedMul(mo->info->speed, finecosine[an]);
  mo->momy = FixedMul(mo->info->speed, finesine[an]);

  // adjust pitch (approximated, using Doom's ye olde
  // finetangent table; same method as monster aim)
  mo->momz += FixedMul(mo->info->speed, DegToSlope(pitch));

  // adjust position
  an = (actor->angle - ANG90) >> ANGLETOFINESHIFT;
  mo->x += FixedMul(spawnofs_xy, finecosine[an]);
  mo->y += FixedMul(spawnofs_xy, finesine[an]);
  mo->z += spawnofs_z;

  // always set the 'tracer' field, so this pointer
  // can be used to fire seeker missiles at will.
  P_SetTarget(&mo->tracer, actor->target);
}

//
// A_MonsterBulletAttack
// A parameterized monster bullet attack.
//   args[0]: Horizontal spread (degrees, in fixed point)
//   args[1]: Vertical spread (degrees, in fixed point)
//   args[2]: Number of bullets to fire; if not set, defaults to 1
//   args[3]: Base damage of attack (e.g. for 3d5, customize the 3); if not set, defaults to 3
//   args[4]: Attack damage modulus (e.g. for 3d5, customize the 5); if not set, defaults to 5
//
void A_MonsterBulletAttack(mobj_t *actor)
{
  int hspread, vspread, numbullets, damagebase, damagemod;
  int aimslope, i, damage, angle, slope;

  if (!mbf21 || !actor->target)
    return;

  hspread    = actor->state->args[0];
  vspread    = actor->state->args[1];
  numbullets = actor->state->args[2];
  damagebase = actor->state->args[3];
  damagemod  = actor->state->args[4];

  A_FaceTarget(actor);
  S_StartMobjSound(actor, actor->info->attacksound);

  aimslope = P_AimLineAttack(actor, actor->angle, MISSILERANGE, 0);

  for (i = 0; i < numbullets; i++)
  {
    damage = (P_Random(pr_mbf21) % damagemod + 1) * damagebase;
    angle = (int)actor->angle + P_RandomHitscanAngle(pr_mbf21, hspread);
    slope = aimslope + P_RandomHitscanSlope(pr_mbf21, vspread);

    P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
  }
}

//
// A_MonsterMeleeAttack
// A parameterized monster melee attack.
//   args[0]: Base damage of attack (e.g. for 3d8, customize the 3); if not set, defaults to 3
//   args[1]: Attack damage modulus (e.g. for 3d8, customize the 8); if not set, defaults to 8
//   args[2]: Sound to play if attack hits
//   args[3]: Range (fixed point); if not set, defaults to monster's melee range
//
void A_MonsterMeleeAttack(mobj_t *actor)
{
  int damagebase, damagemod, hitsound, range;
  int damage;

  if (!mbf21 || !actor->target)
    return;

  damagebase = actor->state->args[0];
  damagemod  = actor->state->args[1];
  hitsound   = actor->state->args[2];
  range      = actor->state->args[3];

  if (range == 0)
    range = actor->info->meleerange;

  range += actor->target->info->radius - 20 * FRACUNIT;

  A_FaceTarget(actor);
  if (!P_CheckRange(actor, range))
    return;

  S_StartMobjSound(actor, hitsound);

  damage = (P_Random(pr_mbf21) % damagemod + 1) * damagebase;
  P_DamageMobj(actor->target, actor, actor, damage);
}

//
// A_RadiusDamage
// A parameterized version of A_Explode. Friggin' finally. :P
//   args[0]: Damage (int)
//   args[1]: Radius (also int; no real need for fractional precision here)
//
void A_RadiusDamage(mobj_t *actor)
{
  if (!mbf21 || !actor->state)
    return;

  P_RadiusAttack(actor, actor->target, actor->state->args[0], actor->state->args[1], true);
}

//
// A_NoiseAlert
// Alerts nearby monsters (via sound) to the calling actor's target's presence.
//
void A_NoiseAlert(mobj_t *actor)
{
  if (!mbf21 || !actor->target)
    return;

  P_NoiseAlert(actor->target, actor);
}

//
// A_HealChase
// A parameterized version of A_VileChase.
//   args[0]: State to jump to on the calling actor when resurrecting a corpse
//   args[1]: Sound to play when resurrecting a corpse
//
void A_HealChase(mobj_t* actor)
{
  int state, sound;

  if (!mbf21 || !actor)
    return;

  state = actor->state->args[0];
  sound = actor->state->args[1];

  if (!P_HealCorpse(actor, actor->info->radius, state, sound))
    A_Chase(actor);
}

//
// A_SeekTracer
// A parameterized seeker missile function.
//   args[0]: direct-homing threshold angle (degrees, in fixed point)
//   args[1]: maximum turn angle (degrees, in fixed point)
//
void A_SeekTracer(mobj_t *actor)
{
  angle_t threshold, maxturnangle;

  if (!mbf21 || !actor)
    return;

  threshold    = FixedToAngle(actor->state->args[0]);
  maxturnangle = FixedToAngle(actor->state->args[1]);

  P_SeekerMissile(actor, &actor->tracer, threshold, maxturnangle, true);
}

//
// A_FindTracer
// Search for a valid tracer (seek target), if the calling actor doesn't already have one.
//   args[0]: field-of-view to search in (degrees, in fixed point); if zero, will search in all directions
//   args[1]: distance to search (map blocks, i.e. 128 units)
//
void A_FindTracer(mobj_t *actor)
{
  angle_t fov;
  int dist;

  if (!mbf21 || !actor || actor->tracer)
    return;

  fov  = FixedToAngle(actor->state->args[0]);
  dist =             (actor->state->args[1]);

  P_SetTarget(&actor->tracer, P_RoughTargetSearch(actor, fov, dist));
}

//
// A_ClearTracer
// Clear current tracer (seek target).
//
void A_ClearTracer(mobj_t *actor)
{
  if (!mbf21 || !actor)
    return;

  P_SetTarget(&actor->tracer, NULL);
}

//
// A_JumpIfHealthBelow
// Jumps to a state if caller's health is below the specified threshold.
//   args[0]: State to jump to
//   args[1]: Health threshold
//
void A_JumpIfHealthBelow(mobj_t* actor)
{
  int state, health;

  if (!mbf21 || !actor)
    return;

  state  = actor->state->args[0];
  health = actor->state->args[1];

  if (actor->health < health)
    P_SetMobjState(actor, state);
}

//
// A_JumpIfTargetInSight
// Jumps to a state if caller's target is in line-of-sight.
//   args[0]: State to jump to
//   args[1]: Field-of-view to check (degrees, in fixed point); if zero, will check in all directions
//
void A_JumpIfTargetInSight(mobj_t* actor)
{
  int state;
  angle_t fov;

  if (!mbf21 || !actor || !actor->target)
    return;

  state =             (actor->state->args[0]);
  fov   = FixedToAngle(actor->state->args[1]);

  // Check FOV first since it's faster
  if (fov > 0 && !P_CheckFov(actor, actor->target, fov))
    return;

  if (P_CheckSight(actor, actor->target))
    P_SetMobjState(actor, state);
}

//
// A_JumpIfTargetCloser
// Jumps to a state if caller's target is closer than the specified distance.
//   args[0]: State to jump to
//   args[1]: Distance threshold
//
void A_JumpIfTargetCloser(mobj_t* actor)
{
  int state, distance;

  if (!mbf21 || !actor || !actor->target)
    return;

  state    = actor->state->args[0];
  distance = actor->state->args[1];

  if (distance > P_AproxDistance(actor->x - actor->target->x,
                                 actor->y - actor->target->y))
    P_SetMobjState(actor, state);
}

//
// A_JumpIfTracerInSight
// Jumps to a state if caller's tracer (seek target) is in line-of-sight.
//   args[0]: State to jump to
//   args[1]: Field-of-view to check (degrees, in fixed point); if zero, will check in all directions
//
void A_JumpIfTracerInSight(mobj_t* actor)
{
  angle_t fov;
  int state;

  if (!mbf21 || !actor || !actor->tracer)
    return;

  state =             (actor->state->args[0]);
  fov   = FixedToAngle(actor->state->args[1]);

  // Check FOV first since it's faster
  if (fov > 0 && !P_CheckFov(actor, actor->tracer, fov))
    return;

  if (P_CheckSight(actor, actor->tracer))
    P_SetMobjState(actor, state);
}

//
// A_JumpIfTracerCloser
// Jumps to a state if caller's tracer (seek target) is closer than the specified distance.
//   args[0]: State to jump to
//   args[1]: Distance threshold (fixed point)
//
void A_JumpIfTracerCloser(mobj_t* actor)
{
  int state, distance;

  if (!mbf21 || !actor || !actor->tracer)
    return;

  state    = actor->state->args[0];
  distance = actor->state->args[1];

  if (distance > P_AproxDistance(actor->x - actor->tracer->x,
                                 actor->y - actor->tracer->y))
    P_SetMobjState(actor, state);
}

//
// A_JumpIfFlagsSet
// Jumps to a state if caller has the specified thing flags set.
//   args[0]: State to jump to
//   args[1]: Standard Flag(s) to check
//   args[2]: MBF21 Flag(s) to check
//
void A_JumpIfFlagsSet(mobj_t* actor)
{
  int state;
  uint64_t flags, flags2;

  if (!mbf21 || !actor)
    return;

  state  = actor->state->args[0];
  flags  = actor->state->args[1];
  flags2 = actor->state->args[2];

  if ((actor->flags & flags) == flags &&
      (actor->flags2 & flags2) == flags2)
    P_SetMobjState(actor, state);
}

//
// A_AddFlags
// Adds the specified thing flags to the caller.
//   args[0]: Standard Flag(s) to add
//   args[1]: MBF21 Flag(s) to add
//
void A_AddFlags(mobj_t* actor)
{
  uint64_t flags, flags2;

  if (!mbf21 || !actor)
    return;

  flags  = actor->state->args[0];
  flags2 = actor->state->args[1];

  actor->flags  |= flags;
  actor->flags2 |= flags2;
}

//
// A_RemoveFlags
// Removes the specified thing flags from the caller.
//   args[0]: Flag(s) to remove
//   args[1]: MBF21 Flag(s) to remove
//
void A_RemoveFlags(mobj_t* actor)
{
  uint64_t flags, flags2;

  if (!mbf21 || !actor)
    return;

  flags  = actor->state->args[0];
  flags2 = actor->state->args[1];

  actor->flags  &= ~flags;
  actor->flags2 &= ~flags2;
}



// heretic

#include "heretic/def.h"

#define MAX_BOSS_SPOTS 8

typedef struct
{
    fixed_t x;
    fixed_t y;
    angle_t angle;
} BossSpot_t;

static int BossSpotCount;
static BossSpot_t BossSpots[MAX_BOSS_SPOTS];

void P_InitMonsters(void)
{
    BossSpotCount = 0;
}

void P_AddBossSpot(fixed_t x, fixed_t y, angle_t angle)
{
    if (BossSpotCount == MAX_BOSS_SPOTS)
    {
        I_Error("Too many boss spots.");
    }
    BossSpots[BossSpotCount].x = x;
    BossSpots[BossSpotCount].y = y;
    BossSpots[BossSpotCount].angle = angle;
    BossSpotCount++;
}

void A_DripBlood(mobj_t * actor)
{
    mobj_t *mo;
    int r1,r2;

    r1 = P_SubRandom();
    r2 = P_SubRandom();

    mo = P_SpawnMobj(actor->x + (r2 << 11),
                     actor->y + (r1 << 11), actor->z,
                     HERETIC_MT_BLOOD);
    mo->momx = P_SubRandom() << 10;
    mo->momy = P_SubRandom() << 10;
    mo->flags2 |= MF2_LOGRAV;
}

void A_KnightAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(3));
        S_StartMobjSound(actor, heretic_sfx_kgtat2);
        return;
    }
    // Throw axe
    S_StartMobjSound(actor, actor->info->attacksound);
    if (actor->type == HERETIC_MT_KNIGHTGHOST || P_Random(pr_heretic) < 40)
    {                           // Red axe
        P_SpawnMissile(actor, actor->target, HERETIC_MT_REDAXE);
        return;
    }
    // Green axe
    P_SpawnMissile(actor, actor->target, HERETIC_MT_KNIGHTAXE);
}

void A_ImpExplode(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_IMPCHUNK1);
    mo->momx = P_SubRandom() << 10;
    mo->momy = P_SubRandom() << 10;
    mo->momz = 9 * FRACUNIT;
    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_IMPCHUNK2);
    mo->momx = P_SubRandom() << 10;
    mo->momy = P_SubRandom() << 10;
    mo->momz = 9 * FRACUNIT;
    if (actor->special1.i == 666)
    {                           // Extreme death crash
        P_SetMobjState(actor, HERETIC_S_IMP_XCRASH1);
    }
}

void A_BeastPuff(mobj_t * actor)
{
    if (P_Random(pr_heretic) > 64)
    {
        int r1,r2,r3;
        r1 = P_SubRandom();
        r2 = P_SubRandom();
        r3 = P_SubRandom();
        P_SpawnMobj(actor->x + (r3 << 10),
                    actor->y + (r2 << 10),
                    actor->z + (r1 << 10), HERETIC_MT_PUFFY);
    }
}

void A_ImpMeAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    S_StartMobjSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, 5 + (P_Random(pr_heretic) & 7));
    }
}

void A_ImpMsAttack(mobj_t * actor)
{
    mobj_t *dest;
    angle_t an;
    int dist;

    if (!actor->target || P_Random(pr_heretic) > 64)
    {
        P_SetMobjState(actor, actor->info->seestate);
        return;
    }
    dest = actor->target;
    actor->flags |= MF_SKULLFLY;
    S_StartMobjSound(actor, actor->info->attacksound);
    A_FaceTarget(actor);
    an = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul(12 * FRACUNIT, finecosine[an]);
    actor->momy = FixedMul(12 * FRACUNIT, finesine[an]);
    dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
    dist = dist / (12 * FRACUNIT);
    if (dist < 1)
    {
        dist = 1;
    }
    actor->momz = (dest->z + (dest->height >> 1) - actor->z) / dist;
}

void A_ImpMsAttack2(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    S_StartMobjSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, 5 + (P_Random(pr_heretic) & 7));
        return;
    }
    P_SpawnMissile(actor, actor->target, HERETIC_MT_IMPBALL);
}

void A_ImpDeath(mobj_t * actor)
{
    actor->flags &= ~MF_SOLID;
    actor->flags2 |= MF2_FOOTCLIP;
    if (actor->z <= actor->floorz)
    {
        P_SetMobjState(actor, HERETIC_S_IMP_CRASH1);
    }
}

void A_ImpXDeath1(mobj_t * actor)
{
    actor->flags &= ~MF_SOLID;
    actor->flags |= MF_NOGRAVITY;
    actor->flags2 |= MF2_FOOTCLIP;
    actor->special1.i = 666;      // Flag the crash routine
}

void A_ImpXDeath2(mobj_t * actor)
{
    actor->flags &= ~MF_NOGRAVITY;
    if (actor->z <= actor->floorz)
    {
        P_SetMobjState(actor, HERETIC_S_IMP_CRASH1);
    }
}

dboolean P_UpdateChicken(mobj_t * actor, int tics)
{
    mobj_t *fog;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    mobjtype_t moType;
    mobj_t *mo;
    mobj_t oldChicken;

    actor->special1.i -= tics;
    if (actor->special1.i > 0)
    {
        return (false);
    }
    moType = actor->special2.i;
    x = actor->x;
    y = actor->y;
    z = actor->z;
    oldChicken = *actor;
    P_SetMobjState(actor, HERETIC_S_FREETARGMOBJ);
    mo = P_SpawnMobj(x, y, z, moType);
    dsda_WatchUnMorph(mo);
    if (P_TestMobjLocation(mo) == false)
    {                           // Didn't fit
        P_RemoveMobj(mo);
        mo = P_SpawnMobj(x, y, z, HERETIC_MT_CHICKEN);
        mo->angle = oldChicken.angle;
        mo->flags = oldChicken.flags;
        mo->health = oldChicken.health;
        P_SetTarget(&mo->target, oldChicken.target);
        mo->special1.i = 5 * 35;  // Next try in 5 seconds
        mo->special2.i = moType;
        dsda_WatchMorph(mo);
        return (false);
    }
    mo->angle = oldChicken.angle;
    P_SetTarget(&mo->target, oldChicken.target);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, HERETIC_MT_TFOG);
    S_StartMobjSound(fog, heretic_sfx_telept);
    return (true);
}

void A_ChicAttack(mobj_t * actor)
{
    if (P_UpdateChicken(actor, 18))
    {
        return;
    }
    if (!actor->target)
    {
        return;
    }
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, 1 + (P_Random(pr_heretic) & 1));
    }
}

void A_ChicLook(mobj_t * actor)
{
    if (P_UpdateChicken(actor, 10))
    {
        return;
    }
    A_Look(actor);
}

void A_ChicChase(mobj_t * actor)
{
    if (P_UpdateChicken(actor, 3))
    {
        return;
    }
    A_Chase(actor);
}

void A_ChicPain(mobj_t * actor)
{
    if (P_UpdateChicken(actor, 10))
    {
        return;
    }
    S_StartMobjSound(actor, actor->info->painsound);
}

void A_Feathers(mobj_t * actor)
{
    int i;
    int count;
    mobj_t *mo;

    if (actor->health > 0)
    {                           // Pain
        count = P_Random(pr_heretic) < 32 ? 2 : 1;
    }
    else
    {                           // Death
        count = 5 + (P_Random(pr_heretic) & 3);
    }
    for (i = 0; i < count; i++)
    {
        mo = P_SpawnMobj(actor->x, actor->y, actor->z + 20 * FRACUNIT,
                         HERETIC_MT_FEATHER);
        P_SetTarget(&mo->target, actor);
        mo->momx = P_SubRandom() << 8;
        mo->momy = P_SubRandom() << 8;
        mo->momz = FRACUNIT + (P_Random(pr_heretic) << 9);
        P_SetMobjState(mo, HERETIC_S_FEATHER1 + (P_Random(pr_heretic) & 7));
    }
}

void A_MummyAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    S_StartMobjSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(2));
        S_StartMobjSound(actor, heretic_sfx_mumat2);
        return;
    }
    S_StartMobjSound(actor, heretic_sfx_mumat1);
}

void A_MummyAttack2(mobj_t * actor)
{
    mobj_t *mo;

    if (!actor->target)
    {
        return;
    }

    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(2));
        return;
    }
    mo = P_SpawnMissile(actor, actor->target, HERETIC_MT_MUMMYFX1);

    if (mo != NULL)
    {
        P_SetTarget(&mo->special1.m, actor->target);
    }
}

void A_MummyFX1Seek(mobj_t * actor)
{
    P_SeekerMissile(actor, &actor->special1.m, ANG1_X * 10, ANG1_X * 20, false);
}

void A_MummySoul(mobj_t * mummy)
{
    mobj_t *mo;

    mo = P_SpawnMobj(mummy->x, mummy->y, mummy->z + 10 * FRACUNIT,
                     HERETIC_MT_MUMMYSOUL);
    mo->momz = FRACUNIT;
}

void A_Sor1Pain(mobj_t * actor)
{
    actor->special1.i = 20;       // Number of steps to walk fast
    A_Pain(actor);
}

void A_Sor1Chase(mobj_t * actor)
{
    if (actor->special1.i)
    {
        actor->special1.i--;
        actor->tics -= 3;
    }
    A_Chase(actor);
}

void A_Srcr1Attack(mobj_t * actor)
{
    mobj_t *mo;
    fixed_t momz;
    angle_t angle;

    if (!actor->target)
    {
        return;
    }
    S_StartMobjSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(8));
        return;
    }
    if (actor->health > (P_MobjSpawnHealth(actor) / 3) * 2)
    {                           // Spit one fireball
        P_SpawnMissile(actor, actor->target, HERETIC_MT_SRCRFX1);
    }
    else
    {                           // Spit three fireballs
        mo = P_SpawnMissile(actor, actor->target, HERETIC_MT_SRCRFX1);
        if (mo)
        {
            momz = mo->momz;
            angle = mo->angle;
            P_SpawnMissileAngle(actor, HERETIC_MT_SRCRFX1, angle - ANG1_X * 3, momz);
            P_SpawnMissileAngle(actor, HERETIC_MT_SRCRFX1, angle + ANG1_X * 3, momz);
        }
        if (actor->health < P_MobjSpawnHealth(actor) / 3)
        {                       // Maybe attack again
            if (actor->special1.i)
            {                   // Just attacked, so don't attack again
                actor->special1.i = 0;
            }
            else
            {                   // Set state to attack again
                actor->special1.i = 1;
                P_SetMobjState(actor, HERETIC_S_SRCR1_ATK4);
            }
        }
    }
}

void A_SorcererRise(mobj_t * actor)
{
    mobj_t *mo;

    actor->flags &= ~MF_SOLID;
    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_SORCERER2);
    P_SetMobjState(mo, HERETIC_S_SOR2_RISE1);
    mo->angle = actor->angle;
    P_SetTarget(&mo->target, actor->target);
}

void P_DSparilTeleport(mobj_t * actor)
{
    int i;
    fixed_t x;
    fixed_t y;
    fixed_t prevX;
    fixed_t prevY;
    fixed_t prevZ;
    mobj_t *mo;

    if (!BossSpotCount)
    {                           // No spots
        return;
    }
    i = P_Random(pr_heretic);
    do
    {
        i++;
        x = BossSpots[i % BossSpotCount].x;
        y = BossSpots[i % BossSpotCount].y;
    }
    while (P_AproxDistance(actor->x - x, actor->y - y) < 128 * FRACUNIT);
    prevX = actor->x;
    prevY = actor->y;
    prevZ = actor->z;
    if (P_TeleportMove(actor, x, y, false))
    {
        mo = P_SpawnMobj(prevX, prevY, prevZ, HERETIC_MT_SOR2TELEFADE);
        S_StartMobjSound(mo, heretic_sfx_telept);
        P_SetMobjState(actor, HERETIC_S_SOR2_TELE1);
        S_StartMobjSound(actor, heretic_sfx_telept);
        actor->z = actor->floorz;
        actor->angle = BossSpots[i % BossSpotCount].angle;
        actor->momx = actor->momy = actor->momz = 0;
    }
}


void A_Srcr2Decide(mobj_t * actor)
{
    static int chance[] = {
        192, 120, 120, 120, 64, 64, 32, 16, 0
    };

    if (!BossSpotCount)
    {                           // No spots
        return;
    }
    if (P_Random(pr_heretic) < chance[actor->health / (P_MobjSpawnHealth(actor) / 8)])
    {
        P_DSparilTeleport(actor);
    }
}

void A_Srcr2Attack(mobj_t * actor)
{
    int chance;

    if (!actor->target)
    {
        return;
    }
    S_StartVoidSound(actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(20));
        return;
    }
    chance = actor->health < P_MobjSpawnHealth(actor) / 2 ? 96 : 48;
    if (P_Random(pr_heretic) < chance)
    {                           // Wizard spawners
        P_SpawnMissileAngle(actor, HERETIC_MT_SOR2FX2,
                            actor->angle - ANG45, FRACUNIT / 2);
        P_SpawnMissileAngle(actor, HERETIC_MT_SOR2FX2,
                            actor->angle + ANG45, FRACUNIT / 2);
    }
    else
    {                           // Blue bolt
        P_SpawnMissile(actor, actor->target, HERETIC_MT_SOR2FX1);
    }
}

void A_BlueSpark(mobj_t * actor)
{
    int i;
    mobj_t *mo;

    for (i = 0; i < 2; i++)
    {
        mo = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_SOR2FXSPARK);
        mo->momx = P_SubRandom() << 9;
        mo->momy = P_SubRandom() << 9;
        mo->momz = FRACUNIT + (P_Random(pr_heretic) << 8);
    }
}

void A_GenWizard(mobj_t * actor)
{
    mobj_t *mo;
    mobj_t *fog;

    mo = P_SpawnMobj(actor->x, actor->y,
                     actor->z - mobjinfo[HERETIC_MT_WIZARD].height / 2, HERETIC_MT_WIZARD);
    if (P_TestMobjLocation(mo) == false)
    {                           // Didn't fit
        P_RemoveMobj(mo);
        return;
    }
    actor->momx = actor->momy = actor->momz = 0;
    P_SetMobjState(actor, mobjinfo[actor->type].deathstate);
    actor->flags &= ~MF_MISSILE;
    fog = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_TFOG);
    S_StartMobjSound(fog, heretic_sfx_telept);
}

void P_Massacre(void)
{
    mobj_t *mo;
    thinker_t *think;

    for (think = thinkercap.next; think != &thinkercap; think = think->next)
    {
        if (think->function != P_MobjThinker)
        {                       // Not a mobj thinker
            continue;
        }
        mo = (mobj_t *) think;
        if ((mo->flags & MF_COUNTKILL) && (mo->health > 0))
        {
            if (hexen)
            {
              mo->flags2 &= ~(MF2_NONSHOOTABLE + MF2_INVULNERABLE);
              mo->flags |= MF_SHOOTABLE;
            }
            P_DamageMobj(mo, NULL, NULL, 10000);
        }
    }
}

void A_Sor2DthInit(mobj_t * actor)
{
    actor->special1.i = 7;        // Animation loop counter
    P_Massacre();               // Kill monsters early
}

void A_Sor2DthLoop(mobj_t * actor)
{
    if (--actor->special1.i)
    {                           // Need to loop
        P_SetMobjState(actor, HERETIC_S_SOR2_DIE4);
    }
}

void A_SorZap(mobj_t * actor)
{
    S_StartVoidSound(heretic_sfx_sorzap);
}

void A_SorRise(mobj_t * actor)
{
    S_StartVoidSound(heretic_sfx_sorrise);
}

void A_SorDSph(mobj_t * actor)
{
    S_StartVoidSound(heretic_sfx_sordsph);
}

void A_SorDExp(mobj_t * actor)
{
    S_StartVoidSound(heretic_sfx_sordexp);
}

void A_SorDBon(mobj_t * actor)
{
    S_StartVoidSound(heretic_sfx_sordbon);
}

void A_SorSightSnd(mobj_t * actor)
{
    S_StartVoidSound(heretic_sfx_sorsit);
}

void A_MinotaurAtk1(mobj_t * actor)
{
    player_t *player;

    if (!actor->target)
    {
        return;
    }
    S_StartMobjSound(actor, g_mntr_atk1_sfx);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(4));
        if (heretic && (player = actor->target->player) != NULL)
        {                       // Squish the player
            player->deltaviewheight = -16 * FRACUNIT;
        }
    }
}

void A_MinotaurDecide(mobj_t * actor)
{
    angle_t angle;
    mobj_t *target;
    int dist;

    target = actor->target;
    if (!target)
    {
        return;
    }
    if (heretic)
      S_StartMobjSound(actor, heretic_sfx_minsit);
    dist = P_AproxDistance(actor->x - target->x, actor->y - target->y);
    if (target->z + target->height > actor->z
        && target->z + target->height < actor->z + actor->height
        && dist < g_mntr_decide_range * 64 * FRACUNIT
        && dist > 1 * 64 * FRACUNIT && P_Random(pr_heretic) < g_mntr_charge_rng)
    {                           // Charge attack
        // Don't call the state function right away
        P_SetMobjStateNF(actor, g_mntr_charge_state);
        actor->flags |= MF_SKULLFLY;
        A_FaceTarget(actor);
        angle = actor->angle >> ANGLETOFINESHIFT;
        actor->momx = FixedMul(g_mntr_charge_speed, finecosine[angle]);
        actor->momy = FixedMul(g_mntr_charge_speed, finesine[angle]);
        // Charge duration
        if (hexen)
          actor->special_args[4] = 35 / 2;
        else
          actor->special1.i = 35 / 2;
    }
    else if (target->z == target->floorz
             && dist < 9 * 64 * FRACUNIT && P_Random(pr_heretic) < g_mntr_fire_rng)
    {                           // Floor fire attack
        P_SetMobjState(actor, g_mntr_fire_state);
        actor->special2.i = 0;
    }
    else
    {                           // Swing attack
        A_FaceTarget(actor);
        // Don't need to call P_SetMobjState because the current state
        // falls through to the swing attack
    }
}

void A_MinotaurCharge(mobj_t * actor)
{
    mobj_t *puff;

    if (hexen && !actor->target)
      return;

    if (hexen ? actor->special_args[4] > 0 : actor->special1.i)
    {
        puff = P_SpawnMobj(actor->x, actor->y, actor->z, g_mntr_charge_puff);
        puff->momz = 2 * FRACUNIT;
        if (hexen)
          actor->special_args[4]--;
        else
          actor->special1.i--;
    }
    else
    {
        actor->flags &= ~MF_SKULLFLY;
        P_SetMobjState(actor, actor->info->seestate);
    }
}

void A_MinotaurAtk2(mobj_t * actor)
{
    mobj_t *mo;
    angle_t angle;
    fixed_t momz;

    if (!actor->target)
    {
        return;
    }
    S_StartMobjSound(actor, g_mntr_atk2_sfx);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(g_mntr_atk2_dice));
        return;
    }
    mo = P_SpawnMissile(actor, actor->target, g_mntr_atk2_missile);
    if (mo)
    {
        if (heretic)
          S_StartMobjSound(mo, g_mntr_atk2_sfx);
        momz = mo->momz;
        angle = mo->angle;
        P_SpawnMissileAngle(actor, g_mntr_atk2_missile, angle - (ANG45 / 8), momz);
        P_SpawnMissileAngle(actor, g_mntr_atk2_missile, angle + (ANG45 / 8), momz);
        P_SpawnMissileAngle(actor, g_mntr_atk2_missile, angle - (ANG45 / 16), momz);
        P_SpawnMissileAngle(actor, g_mntr_atk2_missile, angle + (ANG45 / 16), momz);
    }
}

void A_MinotaurAtk3(mobj_t * actor)
{
    mobj_t *mo;
    player_t *player;

    if (!actor->target)
    {
        return;
    }
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(g_mntr_atk3_dice));
        if ((player = actor->target->player) != NULL)
        {                       // Squish the player
            player->deltaviewheight = -16 * FRACUNIT;
        }
    }
    else
    {
        mo = P_SpawnMissile(actor, actor->target, g_mntr_atk3_missile);
        if (mo != NULL)
        {
            S_StartMobjSound(mo, g_mntr_atk3_sfx);
        }
    }
    if (P_Random(pr_heretic) < 192 && actor->special2.i == 0)
    {
        P_SetMobjState(actor, g_mntr_atk3_state);
        actor->special2.i = 1;
    }
}

void A_MntrFloorFire(mobj_t * actor)
{
    mobj_t *mo;
    int r1, r2;

    r1 = P_SubRandom();
    r2 = P_SubRandom();

    actor->z = actor->floorz;
    mo = P_SpawnMobj(actor->x + (r2 << 10),
                     actor->y + (r1 << 10), ONFLOORZ,
                     g_mntr_fire);
    P_SetTarget(&mo->target, actor->target);
    mo->momx = 1;               // Force block checking
    P_CheckMissileSpawn(mo);
}

void A_BeastAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    S_StartMobjSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(3));
        return;
    }
    P_SpawnMissile(actor, actor->target, HERETIC_MT_BEASTBALL);
}

void A_WhirlwindSeek(mobj_t * actor)
{
    actor->health -= 3;
    if (actor->health < 0)
    {
        actor->momx = actor->momy = actor->momz = 0;
        P_SetMobjState(actor, mobjinfo[actor->type].deathstate);
        actor->flags &= ~MF_MISSILE;
        return;
    }
    if ((actor->special2.i -= 3) < 0)
    {
        actor->special2.i = 58 + (P_Random(pr_heretic) & 31);
        S_StartMobjSound(actor, heretic_sfx_hedat3);
    }
    if (actor->special1.m
        && (((mobj_t *) (actor->special1.m))->flags & MF_SHADOW))
    {
        return;
    }
    P_SeekerMissile(actor, &actor->special1.m, ANG1_X * 10, ANG1_X * 30, false);
}

void A_HeadIceImpact(mobj_t * ice)
{
    unsigned int i;
    angle_t angle;
    mobj_t *shard;

    for (i = 0; i < 8; i++)
    {
        shard = P_SpawnMobj(ice->x, ice->y, ice->z, HERETIC_MT_HEADFX2);
        angle = i * ANG45;
        P_SetTarget(&shard->target, ice->target);
        shard->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        shard->momx = FixedMul(shard->info->speed, finecosine[angle]);
        shard->momy = FixedMul(shard->info->speed, finesine[angle]);
        shard->momz = (fixed_t)(-.6 * FRACUNIT);
        P_CheckMissileSpawn(shard);
    }
}

void A_HeadFireGrow(mobj_t * fire)
{
    fire->health--;
    fire->z += 9 * FRACUNIT;
    if (fire->health == 0)
    {
        fire->damage = fire->info->damage;
        P_SetMobjState(fire, HERETIC_S_HEADFX3_4);
    }
}

void A_SnakeAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        P_SetMobjState(actor, HERETIC_S_SNAKE_WALK1);
        return;
    }
    S_StartMobjSound(actor, actor->info->attacksound);
    A_FaceTarget(actor);
    P_SpawnMissile(actor, actor->target, HERETIC_MT_SNAKEPRO_A);
}

void A_SnakeAttack2(mobj_t * actor)
{
    if (!actor->target)
    {
        P_SetMobjState(actor, HERETIC_S_SNAKE_WALK1);
        return;
    }
    S_StartMobjSound(actor, actor->info->attacksound);
    A_FaceTarget(actor);
    P_SpawnMissile(actor, actor->target, HERETIC_MT_SNAKEPRO_B);
}

void A_ClinkAttack(mobj_t * actor)
{
    int damage;

    if (!actor->target)
    {
        return;
    }
    S_StartMobjSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        damage = ((P_Random(pr_heretic) % 7) + 3);
        P_DamageMobj(actor->target, actor, actor, damage);
    }
}

void A_GhostOff(mobj_t * actor)
{
    actor->flags &= ~MF_SHADOW;
}

void A_WizAtk1(mobj_t * actor)
{
    A_FaceTarget(actor);
    actor->flags &= ~MF_SHADOW;
}

void A_WizAtk2(mobj_t * actor)
{
    A_FaceTarget(actor);
    actor->flags |= MF_SHADOW;
}

void A_WizAtk3(mobj_t * actor)
{
    mobj_t *mo;
    angle_t angle;
    fixed_t momz;

    actor->flags &= ~MF_SHADOW;
    if (!actor->target)
    {
        return;
    }
    S_StartMobjSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(4));
        return;
    }
    mo = P_SpawnMissile(actor, actor->target, HERETIC_MT_WIZFX1);
    if (mo)
    {
        momz = mo->momz;
        angle = mo->angle;
        P_SpawnMissileAngle(actor, HERETIC_MT_WIZFX1, angle - (ANG45 / 8), momz);
        P_SpawnMissileAngle(actor, HERETIC_MT_WIZFX1, angle + (ANG45 / 8), momz);
    }
}

void P_DropItem(mobj_t * source, mobjtype_t type, int special, int chance)
{
    mobj_t *mo;

    if (P_Random(pr_heretic) > chance)
    {
        return;
    }
    mo = P_SpawnMobj(source->x, source->y,
                     source->z + (source->height >> 1), type);
    mo->momx = P_SubRandom() << 8;
    mo->momy = P_SubRandom() << 8;
    mo->momz = FRACUNIT * 5 + (P_Random(pr_heretic) << 10);
    mo->flags |= MF_DROPPED;
    mo->health = special;
}

void A_NoBlocking(mobj_t * actor)
{
    actor->flags &= ~MF_SOLID;

    if (hexen)
      return;

    // Check for monsters dropping things
    switch (actor->type)
    {
        case HERETIC_MT_MUMMY:
        case HERETIC_MT_MUMMYLEADER:
        case HERETIC_MT_MUMMYGHOST:
        case HERETIC_MT_MUMMYLEADERGHOST:
            P_DropItem(actor, HERETIC_MT_AMGWNDWIMPY, 3, 84);
            break;
        case HERETIC_MT_KNIGHT:
        case HERETIC_MT_KNIGHTGHOST:
            P_DropItem(actor, HERETIC_MT_AMCBOWWIMPY, 5, 84);
            break;
        case HERETIC_MT_WIZARD:
            P_DropItem(actor, HERETIC_MT_AMBLSRWIMPY, 10, 84);
            P_DropItem(actor, HERETIC_MT_ARTITOMEOFPOWER, 0, 4);
            break;
        case HERETIC_MT_HEAD:
            P_DropItem(actor, HERETIC_MT_AMBLSRWIMPY, 10, 84);
            P_DropItem(actor, HERETIC_MT_ARTIEGG, 0, 51);
            break;
        case HERETIC_MT_BEAST:
            P_DropItem(actor, HERETIC_MT_AMCBOWWIMPY, 10, 84);
            break;
        case HERETIC_MT_CLINK:
            P_DropItem(actor, HERETIC_MT_AMSKRDWIMPY, 20, 84);
            break;
        case HERETIC_MT_SNAKE:
            P_DropItem(actor, HERETIC_MT_AMPHRDWIMPY, 5, 84);
            break;
        case HERETIC_MT_MINOTAUR:
            P_DropItem(actor, HERETIC_MT_ARTISUPERHEAL, 0, 51);
            P_DropItem(actor, HERETIC_MT_AMPHRDWIMPY, 10, 84);
            break;
        default:
            break;
    }
}

void A_PodPain(mobj_t * actor)
{
    int i;
    int count;
    int chance;
    mobj_t *goo;

    chance = P_Random(pr_heretic);
    if (chance < 128)
    {
        return;
    }
    count = chance > 240 ? 2 : 1;
    for (i = 0; i < count; i++)
    {
        goo = P_SpawnMobj(actor->x, actor->y,
                          actor->z + 48 * FRACUNIT, HERETIC_MT_PODGOO);
        P_SetTarget(&goo->target, actor);
        goo->momx = P_SubRandom() << 9;
        goo->momy = P_SubRandom() << 9;
        goo->momz = FRACUNIT / 2 + (P_Random(pr_heretic) << 9);
    }
}

void A_RemovePod(mobj_t * actor)
{
    mobj_t *mo;

    if (actor->special2.m)
    {
        mo = (mobj_t *) actor->special2.m;
        if (mo->special1.i > 0)
        {
            mo->special1.i--;
        }
    }
}

#define MAX_GEN_PODS 16

void A_MakePod(mobj_t * actor)
{
    mobj_t *mo;
    fixed_t x;
    fixed_t y;

    if (actor->special1.i == MAX_GEN_PODS)
    {                           // Too many generated pods
        return;
    }
    x = actor->x;
    y = actor->y;
    mo = P_SpawnMobj(x, y, ONFLOORZ, HERETIC_MT_POD);
    if (P_CheckPosition(mo, x, y) == false)
    {                           // Didn't fit
        P_RemoveMobj(mo);
        return;
    }
    P_SetMobjState(mo, HERETIC_S_POD_GROW1);
    P_ThrustMobj(mo, P_Random(pr_heretic) << 24, (fixed_t) (4.5 * FRACUNIT));
    S_StartMobjSound(mo, heretic_sfx_newpod);
    actor->special1.i++;          // Increment generated pod count
    P_SetTarget(&mo->special2.m, actor);       // Link the generator to the pod
    return;
}

void A_ESound(mobj_t * mo)
{
    int sound = heretic_sfx_None;

    switch (mo->type)
    {
        case HERETIC_MT_SOUNDWATERFALL:
            sound = heretic_sfx_waterfl;
            break;
        case HERETIC_MT_SOUNDWIND:
            sound = heretic_sfx_wind;
            break;
        case HEXEN_MT_SOUNDWIND:
            sound = hexen_sfx_wind;
            break;
        default:
            break;
    }
    S_StartMobjSound(mo, sound);
}

void A_SpawnTeleGlitter(mobj_t * actor)
{
    mobj_t *mo;
    int r1, r2;

    r1 = P_Random(pr_heretic);
    r2 = P_Random(pr_heretic);
    mo = P_SpawnMobj(actor->x + ((r2 & 31) - 16) * FRACUNIT,
                     actor->y + ((r1 & 31) - 16) * FRACUNIT,
                     actor->subsector->sector->floorheight, HERETIC_MT_TELEGLITTER);
    mo->momz = FRACUNIT / 4;
}

void A_SpawnTeleGlitter2(mobj_t * actor)
{
    mobj_t *mo;
    int r1, r2;

    r1 = P_Random(pr_heretic);
    r2 = P_Random(pr_heretic);
    mo = P_SpawnMobj(actor->x + ((r2 & 31) - 16) * FRACUNIT,
                     actor->y + ((r1 & 31) - 16) * FRACUNIT,
                     actor->subsector->sector->floorheight, HERETIC_MT_TELEGLITTER2);
    mo->momz = FRACUNIT / 4;
}

void A_AccTeleGlitter(mobj_t * actor)
{
    if (++actor->health > 35)
    {
        actor->momz += actor->momz / 2;
    }
}

void A_InitKeyGizmo(mobj_t * gizmo)
{
    mobj_t *mo;
    statenum_t state = g_s_null;

    switch (gizmo->type)
    {
        case HERETIC_MT_KEYGIZMOBLUE:
            state = HERETIC_S_KGZ_BLUEFLOAT1;
            break;
        case HERETIC_MT_KEYGIZMOGREEN:
            state = HERETIC_S_KGZ_GREENFLOAT1;
            break;
        case HERETIC_MT_KEYGIZMOYELLOW:
            state = HERETIC_S_KGZ_YELLOWFLOAT1;
            break;
        default:
            break;
    }
    mo = P_SpawnMobj(gizmo->x, gizmo->y, gizmo->z + 60 * FRACUNIT,
                     HERETIC_MT_KEYGIZMOFLOAT);
    P_SetMobjState(mo, state);
}

void A_VolcanoSet(mobj_t * volcano)
{
    volcano->tics = 105 + (P_Random(pr_heretic) & 127);
}

void A_VolcanoBlast(mobj_t * volcano)
{
    int i;
    int count;
    mobj_t *blast;
    angle_t angle;

    count = 1 + (P_Random(pr_heretic) % 3);
    for (i = 0; i < count; i++)
    {
        blast = P_SpawnMobj(volcano->x, volcano->y, volcano->z + 44 * FRACUNIT, HERETIC_MT_VOLCANOBLAST);
        P_SetTarget(&blast->target, volcano);
        angle = P_Random(pr_heretic) << 24;
        blast->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        blast->momx = FixedMul(1 * FRACUNIT, finecosine[angle]);
        blast->momy = FixedMul(1 * FRACUNIT, finesine[angle]);
        blast->momz = (fixed_t)(2.5 * FRACUNIT) + (P_Random(pr_heretic) << 10);
        S_StartMobjSound(blast, heretic_sfx_volsht);
        P_CheckMissileSpawn(blast);
    }
}

void A_VolcBallImpact(mobj_t * ball)
{
    unsigned int i;
    mobj_t *tiny;
    angle_t angle;

    if (ball->z <= ball->floorz)
    {
        ball->flags |= MF_NOGRAVITY;
        ball->flags2 &= ~MF2_LOGRAV;
        ball->z += 28 * FRACUNIT;
        //ball->momz = 3*FRACUNIT;
    }
    P_RadiusAttack(ball, ball->target, 25, 25, true);
    for (i = 0; i < 4; i++)
    {
        tiny = P_SpawnMobj(ball->x, ball->y, ball->z, HERETIC_MT_VOLCANOTBLAST);
        P_SetTarget(&tiny->target, ball);
        angle = i * ANG90;
        tiny->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        tiny->momx = FixedMul((fixed_t)(FRACUNIT * .7), finecosine[angle]);
        tiny->momy = FixedMul((fixed_t)(FRACUNIT * .7), finesine[angle]);
        tiny->momz = FRACUNIT + (P_Random(pr_heretic) << 9);
        P_CheckMissileSpawn(tiny);
    }
}

void A_CheckSkullFloor(mobj_t * actor)
{
    if (actor->z <= actor->floorz)
    {
        P_SetMobjState(actor, g_s_bloodyskullx1);
        if (hexen)
          S_StartMobjSound(actor, hexen_sfx_drip);
    }
}

void A_CheckSkullDone(mobj_t * actor)
{
    if (actor->special2.i == 666)
    {
        P_SetMobjState(actor, g_s_bloodyskullx2);
    }
}

void A_CheckBurnGone(mobj_t * actor)
{
    if (actor->special2.i == 666)
    {
        P_SetMobjState(actor, g_s_play_fdth20);
    }
}

void A_FreeTargMobj(mobj_t * mo)
{
    mo->momx = mo->momy = mo->momz = 0;
    mo->z = mo->ceilingz + 4 * FRACUNIT;
    mo->flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY | MF_SOLID);
    mo->flags |= MF_CORPSE | MF_DROPOFF | MF_NOGRAVITY;
    mo->flags2 &= ~(MF2_PASSMOBJ | MF2_LOGRAV);
    mo->player = NULL;

    // hexen_note: can we do this in heretic too?
    if (hexen)
    {
      mo->flags &= ~(MF_COUNTKILL);
      mo->flags2 |= MF2_DONTDRAW;
      mo->health = -1000;         // Don't resurrect
    }
}

static int bodyqueslot, bodyquesize;
static mobj_t** bodyque;

void A_ResetPlayerCorpseQueue(void)
{
  bodyqueslot = 0;
  bodyquesize = dsda_IntConfig(dsda_config_max_player_corpse);
}

void A_AddPlayerCorpse(mobj_t * actor)
{
  if (bodyquesize > 0)
  {
    static int queuesize;

    if (queuesize < bodyquesize)
  	{
  	  bodyque = Z_Realloc(bodyque, bodyquesize * sizeof(*bodyque));
  	  memset(bodyque + queuesize, 0, (bodyquesize - queuesize) * sizeof(*bodyque));
  	  queuesize = bodyquesize;
  	}

    if (bodyqueslot >= bodyquesize)
  	  P_RemoveMobj(bodyque[bodyqueslot % bodyquesize]);

    bodyque[bodyqueslot++ % bodyquesize] = actor;
  }
  else if (!bodyquesize)
    P_RemoveMobj(actor);
}

void A_FlameSnd(mobj_t * actor)
{
    S_StartMobjSound(actor, heretic_sfx_hedat1);    // Burn sound
}

void A_HideThing(mobj_t * actor)
{
    //P_UnsetThingPosition(actor);
    actor->flags2 |= MF2_DONTDRAW;
}

void A_UnHideThing(mobj_t * actor)
{
    //P_SetThingPosition(actor);
    actor->flags2 &= ~MF2_DONTDRAW;
}

void Heretic_A_Scream(mobj_t * actor)
{
    switch (actor->type)
    {
        case HERETIC_MT_CHICPLAYER:
        case HERETIC_MT_SORCERER1:
        case HERETIC_MT_MINOTAUR:
            // Make boss death sounds full volume
            S_StartVoidSound(actor->info->deathsound);
            break;
        case HERETIC_MT_PLAYER:
            // Handle the different player death screams
            if (actor->special1.i < 10)
            {                   // Wimpy death sound
                S_StartMobjSound(actor, heretic_sfx_plrwdth);
            }
            else if (actor->health > -50)
            {                   // Normal death sound
                S_StartMobjSound(actor, actor->info->deathsound);
            }
            else if (actor->health > -100)
            {                   // Crazy death sound
                S_StartMobjSound(actor, heretic_sfx_plrcdth);
            }
            else
            {                   // Extreme death sound
                S_StartMobjSound(actor, heretic_sfx_gibdth);
            }
            break;
        default:
            S_StartMobjSound(actor, actor->info->deathsound);
            break;
    }
}

void Heretic_A_BossDeath(mobj_t * actor)
{
    mobj_t *mo;
    thinker_t *think;
    line_t dummyLine;
    static mobjtype_t bossType[6] = {
        HERETIC_MT_HEAD,
        HERETIC_MT_MINOTAUR,
        HERETIC_MT_SORCERER2,
        HERETIC_MT_HEAD,
        HERETIC_MT_MINOTAUR,
        -1
    };

    if (gamemap != 8)
    {                           // Not a boss level
        return;
    }
    if (actor->type != bossType[gameepisode - 1])
    {                           // Not considered a boss in this episode
        return;
    }
    // Make sure all bosses are dead
    for (think = thinkercap.next; think != &thinkercap; think = think->next)
    {
        if (think->function != P_MobjThinker)
        {                       // Not a mobj thinker
            continue;
        }
        mo = (mobj_t *) think;
        if ((mo != actor) && (mo->type == actor->type) && (mo->health > 0))
        {                       // Found a living boss
            return;
        }
    }
    if (gameepisode > 1)
    {                           // Kill any remaining monsters
        P_Massacre();
    }
    dummyLine.tag = 666;
    EV_DoFloor(&dummyLine, lowerFloor);
}

#define MONS_LOOK_LIMIT 64

// Not proxied by P_LookForMonsters - this is post-death brawling
dboolean Heretic_P_LookForMonsters(mobj_t * actor)
{
    int count;
    mobj_t *mo;
    thinker_t *think;

    if (!P_CheckSight(players[0].mo, actor))
    {                           // Player can't see monster
        return (false);
    }
    count = 0;
    for (think = thinkercap.next; think != &thinkercap; think = think->next)
    {
        if (think->function != P_MobjThinker)
        {                       // Not a mobj thinker
            continue;
        }
        mo = (mobj_t *) think;
        if (!(mo->flags & MF_COUNTKILL) || (mo == actor) || (mo->health <= 0))
        {                       // Not a valid monster
            continue;
        }
        if (P_AproxDistance(actor->x - mo->x, actor->y - mo->y) > g_mons_look_range)
        {                       // Out of range
            continue;
        }
        if (P_Random(pr_heretic) < 16)
        {                       // Skip
            continue;
        }
        if (count++ > MONS_LOOK_LIMIT)
        {                       // Stop searching
            return (false);
        }
        if (!P_CheckSight(actor, mo))
        {                       // Out of sight
            continue;
        }
        if (actor->type == HEXEN_MT_MINOTAUR)
        {
            // hexen_note: this is supposed to be mo->target != actor->special1.p->mo
            // - hexen never actually set p (player_t *), so it's m (mobj_t *) (union)
            // - mo is the first entry in player_t
            // - thinker_t is the first entry in mobj_t, whose first entry is the previous thinker
            // - this resolves to actor->special1.m->thinker.prev
            if ((mo->type == HEXEN_MT_MINOTAUR) &&
                (mo->target != (mobj_t *) actor->special1.m->thinker.prev))
            {
                continue;
            }
        }
        // Found a target monster
        P_SetTarget(&actor->target, mo);
        return (true);
    }
    return (false);
}

dboolean Raven_P_LookForPlayers(mobj_t * actor, dboolean allaround)
{
    int c;
    int stop;
    player_t *player;
    angle_t an;
    fixed_t dist;

    if (!netgame && players[0].health <= 0)
    {                           // Single player game and player is dead, look for monsters
        return (Heretic_P_LookForMonsters(actor));
    }
    c = 0;
    stop = (actor->lastlook - 1) & 3;
    for (;; actor->lastlook = (actor->lastlook + 1) & 3)
    {
        if (!playeringame[actor->lastlook])
            continue;

        if (c++ == 2 || actor->lastlook == stop)
            return false;       // done looking

        player = &players[actor->lastlook];

        if (players->cheats & CF_NOTARGET)
          continue; // no target

        if (player->health <= 0)
            continue;           // dead
        if (!P_CheckSight(actor, player->mo))
            continue;           // out of sight

        if (!allaround)
        {
            an = R_PointToAngle2(actor->x, actor->y,
                                 player->mo->x, player->mo->y) - actor->angle;
            if (an > ANG90 && an < ANG270)
            {
                dist = P_AproxDistance(player->mo->x - actor->x,
                                       player->mo->y - actor->y);
                // if real close, react anyway
                if (dist > WAKEUPRANGE)
                    continue;   // behind back
            }
        }
        if (player->mo->flags & MF_SHADOW)
        {                       // Player is invisible
            if ((P_AproxDistance(player->mo->x - actor->x,
                                 player->mo->y - actor->y) > SNEAKRANGE)
                && P_AproxDistance(player->mo->momx, player->mo->momy)
                < 5 * FRACUNIT)
            {                   // Player is sneaking - can't detect
                return (false);
            }
            if (P_Random(pr_heretic) < 225)
            {                   // Player isn't sneaking, but still didn't detect
                return (false);
            }
        }
        if (actor->type == HEXEN_MT_MINOTAUR)
        {
            // hexen_note: minotaur was intended to store the player_t* but doesn't
            if (actor->special1.m == (mobj_t *) player)
            {
                continue;       // Don't target master
            }
        }

        P_SetTarget(&actor->target, player->mo);
        return (true);
    }
    return (false);
}

// hexen

#include "hexen/a_action.h"
#include "hexen/p_acs.h"

extern fixed_t FloatBobOffsets[64];

// Corpse queue for monsters - this should be saved out
#define CORPSEQUEUESIZE	64
mobj_t *corpseQueue[CORPSEQUEUESIZE];
int corpseQueueSlot;

// throw another corpse on the queue
void A_QueueCorpse(mobj_t * actor)
{
    mobj_t *corpse;

    if (corpseQueueSlot >= CORPSEQUEUESIZE)
    {                           // Too many corpses - remove an old one
        corpse = corpseQueue[corpseQueueSlot % CORPSEQUEUESIZE];
        if (corpse)
            P_RemoveMobj(corpse);
    }
    corpseQueue[corpseQueueSlot % CORPSEQUEUESIZE] = actor;
    corpseQueueSlot++;
}

// Remove a mobj from the queue (for resurrection)
void A_DeQueueCorpse(mobj_t * actor)
{
    int slot;

    for (slot = 0; slot < CORPSEQUEUESIZE; slot++)
    {
        if (corpseQueue[slot] == actor)
        {
            corpseQueue[slot] = NULL;
            break;
        }
    }
}

void P_InitCreatureCorpseQueue(dboolean corpseScan)
{
    thinker_t *think;
    mobj_t *mo;

    // Initialize queue
    corpseQueueSlot = 0;
    memset(corpseQueue, 0, sizeof(mobj_t *) * CORPSEQUEUESIZE);

    if (!corpseScan)
        return;

    // Search mobj list for corpses and place them in this queue
    for (think = thinkercap.next; think != &thinkercap; think = think->next)
    {
        if (think->function != P_MobjThinker)
            continue;
        mo = (mobj_t *) think;
        if (!(mo->flags & MF_CORPSE))
            continue;           // Must be a corpse
        if (mo->flags & MF_ICECORPSE)
            continue;           // Not ice corpses
        // Only corpses that call A_QueueCorpse from death routine
        switch (mo->type)
        {
            case HEXEN_MT_CENTAUR:
            case HEXEN_MT_CENTAURLEADER:
            case HEXEN_MT_DEMON:
            case HEXEN_MT_DEMON2:
            case HEXEN_MT_WRAITH:
            case HEXEN_MT_WRAITHB:
            case HEXEN_MT_BISHOP:
            case HEXEN_MT_ETTIN:
            case HEXEN_MT_PIG:
            case HEXEN_MT_CENTAUR_SHIELD:
            case HEXEN_MT_CENTAUR_SWORD:
            case HEXEN_MT_DEMONCHUNK1:
            case HEXEN_MT_DEMONCHUNK2:
            case HEXEN_MT_DEMONCHUNK3:
            case HEXEN_MT_DEMONCHUNK4:
            case HEXEN_MT_DEMONCHUNK5:
            case HEXEN_MT_DEMON2CHUNK1:
            case HEXEN_MT_DEMON2CHUNK2:
            case HEXEN_MT_DEMON2CHUNK3:
            case HEXEN_MT_DEMON2CHUNK4:
            case HEXEN_MT_DEMON2CHUNK5:
            case HEXEN_MT_FIREDEMON_SPLOTCH1:
            case HEXEN_MT_FIREDEMON_SPLOTCH2:
                A_QueueCorpse(mo);      // Add corpse to queue
                break;
            default:
                break;
        }
    }
}

dboolean P_CheckMeleeRange2(mobj_t * actor)
{
    mobj_t *mo;
    fixed_t dist;

    if (!actor->target)
    {
        return (false);
    }
    mo = actor->target;
    dist = P_AproxDistance(mo->x - actor->x, mo->y - actor->y);
    if (dist >= MELEERANGE * 2 || dist < MELEERANGE)
    {
        return (false);
    }
    if (!P_CheckSight(actor, mo))
    {
        return (false);
    }
    if (mo->z > actor->z + actor->height)
    {                           // Target is higher than the attacker
        return (false);
    }
    else if (actor->z > mo->z + mo->height)
    {                           // Attacker is higher
        return (false);
    }
    return (true);
}

void A_SetInvulnerable(mobj_t * actor)
{
    actor->flags2 |= MF2_INVULNERABLE;
}

void A_UnSetInvulnerable(mobj_t * actor)
{
    actor->flags2 &= ~MF2_INVULNERABLE;
}

void A_SetReflective(mobj_t * actor)
{
    actor->flags2 |= MF2_REFLECTIVE;

    if ((actor->type == HEXEN_MT_CENTAUR) || (actor->type == HEXEN_MT_CENTAURLEADER))
    {
        A_SetInvulnerable(actor);
    }
}

void A_UnSetReflective(mobj_t * actor)
{
    actor->flags2 &= ~MF2_REFLECTIVE;

    if ((actor->type == HEXEN_MT_CENTAUR) || (actor->type == HEXEN_MT_CENTAURLEADER))
    {
        A_UnSetInvulnerable(actor);
    }
}

dboolean P_UpdateMorphedMonster(mobj_t * actor, int tics)
{
    mobj_t *fog;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    mobjtype_t moType;
    mobj_t *mo;
    mobj_t oldMonster;

    actor->special1.i -= tics;
    if (actor->special1.i > 0)
    {
        return (false);
    }
    moType = actor->special2.i;
    switch (moType)
    {
        case HEXEN_MT_WRAITHB:       // These must remain morphed
        case HEXEN_MT_SERPENT:
        case HEXEN_MT_SERPENTLEADER:
        case HEXEN_MT_MINOTAUR:
            return (false);
        default:
            break;
    }
    x = actor->x;
    y = actor->y;
    z = actor->z;
    oldMonster = *actor;        // Save pig vars

    map_format.remove_mobj_thing_id(actor);
    P_SetMobjState(actor, HEXEN_S_FREETARGMOBJ);
    mo = P_SpawnMobj(x, y, z, moType);
    dsda_WatchUnMorph(mo);
    if (P_TestMobjLocation(mo) == false)
    {                           // Didn't fit
        P_RemoveMobj(mo);
        mo = P_SpawnMobj(x, y, z, oldMonster.type);
        mo->angle = oldMonster.angle;
        mo->flags = oldMonster.flags;
        mo->health = oldMonster.health;
        P_SetTarget(&mo->target, oldMonster.target);
        mo->special = oldMonster.special;
        mo->special1.i = 5 * 35;  // Next try in 5 seconds
        mo->special2.i = moType;
        mo->tid = oldMonster.tid;
        memcpy(mo->special_args, oldMonster.special_args, SPECIAL_ARGS_SIZE);
        map_format.add_mobj_thing_id(mo, oldMonster.tid);
        dsda_WatchMorph(mo);
        return (false);
    }
    mo->angle = oldMonster.angle;
    P_SetTarget(&mo->target, oldMonster.target);
    mo->tid = oldMonster.tid;
    mo->special = oldMonster.special;
    memcpy(mo->special_args, oldMonster.special_args, SPECIAL_ARGS_SIZE);
    map_format.add_mobj_thing_id(mo, oldMonster.tid);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, HEXEN_MT_TFOG);
    S_StartMobjSound(fog, hexen_sfx_teleport);
    return (true);
}

void A_PigLook(mobj_t * actor)
{
    if (P_UpdateMorphedMonster(actor, 10))
    {
        return;
    }
    A_Look(actor);
}

void A_PigChase(mobj_t * actor)
{
    if (P_UpdateMorphedMonster(actor, 3))
    {
        return;
    }
    A_Chase(actor);
}

void A_PigAttack(mobj_t * actor)
{
    if (P_UpdateMorphedMonster(actor, 18))
    {
        return;
    }
    if (!actor->target)
    {
        return;
    }
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, 2 + (P_Random(pr_hexen) & 1));
        S_StartMobjSound(actor, hexen_sfx_pig_attack);
    }
}

void A_PigPain(mobj_t * actor)
{
    A_Pain(actor);
    if (actor->z <= actor->floorz)
    {
        actor->momz = 3.5 * FRACUNIT;
    }
}

void FaceMovementDirection(mobj_t * actor)
{
    switch (actor->movedir)
    {
        case DI_EAST:
            actor->angle = 0 << 24;
            break;
        case DI_NORTHEAST:
            actor->angle = 32 << 24;
            break;
        case DI_NORTH:
            actor->angle = 64 << 24;
            break;
        case DI_NORTHWEST:
            actor->angle = 96 << 24;
            break;
        case DI_WEST:
            actor->angle = 128 << 24;
            break;
        case DI_SOUTHWEST:
            actor->angle = 160 << 24;
            break;
        case DI_SOUTH:
            actor->angle = 192 << 24;
            break;
        case DI_SOUTHEAST:
            actor->angle = 224 << 24;
            break;
    }
}

//----------------------------------------------------------------------------
//
// Minotaur variables
//
//      special1                pointer to player that spawned it (mobj_t)
//      special2                internal to minotaur AI
//      args[0]                 args[0]-args[3] together make up minotaur start time
//      args[1]                 |
//      args[2]                 |
//      args[3]                 V
//      args[4]                 charge duration countdown
//----------------------------------------------------------------------------

void A_MinotaurFade0(mobj_t * actor)
{
    actor->flags &= ~MF_ALTSHADOW;
    actor->flags |= MF_SHADOW;
}

void A_MinotaurFade1(mobj_t * actor)
{
    // Second level of transparency
    actor->flags &= ~MF_SHADOW;
    actor->flags |= MF_ALTSHADOW;
}

void A_MinotaurFade2(mobj_t * actor)
{
    // Make fully visible
    actor->flags &= ~MF_SHADOW;
    actor->flags &= ~MF_ALTSHADOW;
}

void A_MinotaurLook(mobj_t * actor);

// Check the age of the minotaur and stomp it after MAULATORTICS of time
// have passed. Returns false if killed.
static dboolean CheckMinotaurAge(mobj_t *mo)
{
    byte args[5];
    unsigned int starttime;

    COLLAPSE_SPECIAL_ARGS(args, mo->special_args);
    memcpy(&starttime, args, sizeof(unsigned int));
    if (leveltime - LittleLong(starttime) >= MAULATORTICS)
    {
        P_DamageMobj(mo, NULL, NULL, 10000);
        return false;
    }

    return true;
}

void A_MinotaurRoam(mobj_t * actor)
{
    actor->flags &= ~MF_SHADOW; // In case pain caused him to
    actor->flags &= ~MF_ALTSHADOW;      // skip his fade in.

    if (!CheckMinotaurAge(actor))
    {
        return;
    }

    if (P_Random(pr_hexen) < 30)
        A_MinotaurLook(actor);  // adjust to closest target

    if (P_Random(pr_hexen) < 6)
    {
        //Choose new direction
        actor->movedir = P_Random(pr_hexen) % 8;
        FaceMovementDirection(actor);
    }
    if (!P_Move(actor, false))
    {
        // Turn
        if (P_Random(pr_hexen) & 1)
            actor->movedir = (actor->movedir + 1) % 8;
        else
            actor->movedir = (actor->movedir + 7) % 8;
        FaceMovementDirection(actor);
    }
}

#define MINOTAUR_LOOK_DIST		(16*54*FRACUNIT)

void A_MinotaurLook(mobj_t * actor)
{
    mobj_t *mo = NULL;
    player_t *player;
    thinker_t *think;
    fixed_t dist;
    int i;
    mobj_t *master = actor->special1.m;

    P_SetTarget(&actor->target, NULL);
    if (deathmatch)             // Quick search for players
    {
        for (i = 0; i < g_maxplayers; i++)
        {
            if (!playeringame[i])
                continue;
            player = &players[i];
            mo = player->mo;
            if (mo == master)
                continue;
            if (mo->health <= 0)
                continue;
            dist = P_AproxDistance(actor->x - mo->x, actor->y - mo->y);
            if (dist > MINOTAUR_LOOK_DIST)
                continue;
            P_SetTarget(&actor->target, mo);
            break;
        }
    }

    if (!actor->target)         // Near player monster search
    {
        if (master && (master->health > 0) && (master->player))
            mo = P_RoughTargetSearch(master, 0, 20);
        else
            mo = P_RoughTargetSearch(actor, 0, 20);
        P_SetTarget(&actor->target, mo);
    }

    if (!actor->target)         // Normal monster search
    {
        for (think = thinkercap.next; think != &thinkercap;
             think = think->next)
        {
            if (think->function != P_MobjThinker)
                continue;
            mo = (mobj_t *) think;
            if (!(mo->flags & MF_COUNTKILL))
                continue;
            if (mo->health <= 0)
                continue;
            if (!(mo->flags & MF_SHOOTABLE))
                continue;
            dist = P_AproxDistance(actor->x - mo->x, actor->y - mo->y);
            if (dist > MINOTAUR_LOOK_DIST)
                continue;
            if ((mo == master) || (mo == actor))
                continue;
            if ((mo->type == HEXEN_MT_MINOTAUR) &&
                (mo->special1.m == actor->special1.m))
                continue;
            P_SetTarget(&actor->target, mo);
            break;              // Found mobj to attack
        }
    }

    if (actor->target)
    {
        P_SetMobjStateNF(actor, HEXEN_S_MNTR_WALK1);
    }
    else
    {
        P_SetMobjStateNF(actor, HEXEN_S_MNTR_ROAM1);
    }
}

void A_MinotaurChase(mobj_t * actor)
{
    actor->flags &= ~MF_SHADOW; // In case pain caused him to
    actor->flags &= ~MF_ALTSHADOW;      // skip his fade in.

    if (!CheckMinotaurAge(actor))
    {
        return;
    }

    if (P_Random(pr_hexen) < 30)
        A_MinotaurLook(actor);  // adjust to closest target

    if (!actor->target || (actor->target->health <= 0) ||
        !(actor->target->flags & MF_SHOOTABLE))
    {                           // look for a new target
        P_SetMobjState(actor, HEXEN_S_MNTR_LOOK1);
        return;
    }

    FaceMovementDirection(actor);
    actor->reactiontime = 0;

    // Melee attack
    if (actor->info->meleestate && P_CheckMeleeRange(actor))
    {
        if (actor->info->attacksound)
        {
            S_StartMobjSound(actor, actor->info->attacksound);
        }
        P_SetMobjState(actor, actor->info->meleestate);
        return;
    }

    // Missile attack
    if (actor->info->missilestate && P_CheckMissileRange(actor))
    {
        P_SetMobjState(actor, actor->info->missilestate);
        return;
    }

    // chase towards target
    if (!P_Move(actor, false))
    {
        P_NewChaseDir(actor);
    }

    // Active sound
    if (actor->info->activesound && P_Random(pr_hexen) < 6)
    {
        S_StartMobjSound(actor, actor->info->activesound);
    }
}

void Hexen_A_Scream(mobj_t * actor)
{
    int sound;

    S_StopSound(actor);
    if (actor->player)
    {
        if (actor->player->morphTics)
        {
            S_StartMobjSound(actor, actor->info->deathsound);
        }
        else
        {
            // Handle the different player death screams
            if (actor->momz <= -39 * FRACUNIT)
            {                   // Falling splat
                sound = hexen_sfx_player_falling_splat;
            }
            else if (actor->health > -50)
            {                   // Normal death sound
                switch (actor->player->pclass)
                {
                    case PCLASS_FIGHTER:
                        sound = hexen_sfx_player_fighter_normal_death;
                        break;
                    case PCLASS_CLERIC:
                        sound = hexen_sfx_player_cleric_normal_death;
                        break;
                    case PCLASS_MAGE:
                        sound = hexen_sfx_player_mage_normal_death;
                        break;
                    default:
                        sound = hexen_sfx_None;
                        break;
                }
            }
            else if (actor->health > -100)
            {                   // Crazy death sound
                switch (actor->player->pclass)
                {
                    case PCLASS_FIGHTER:
                        sound = hexen_sfx_player_fighter_crazy_death;
                        break;
                    case PCLASS_CLERIC:
                        sound = hexen_sfx_player_cleric_crazy_death;
                        break;
                    case PCLASS_MAGE:
                        sound = hexen_sfx_player_mage_crazy_death;
                        break;
                    default:
                        sound = hexen_sfx_None;
                        break;
                }
            }
            else
            {                   // Extreme death sound
                switch (actor->player->pclass)
                {
                    case PCLASS_FIGHTER:
                        sound = hexen_sfx_player_fighter_extreme1_death;
                        break;
                    case PCLASS_CLERIC:
                        sound = hexen_sfx_player_cleric_extreme1_death;
                        break;
                    case PCLASS_MAGE:
                        sound = hexen_sfx_player_mage_extreme1_death;
                        break;
                    default:
                        sound = hexen_sfx_None;
                        break;
                }
                sound += P_Random(pr_hexen) % 3;        // Three different extreme deaths
            }
            S_StartMobjSound(actor, sound);
        }
    }
    else
    {
        S_StartMobjSound(actor, actor->info->deathsound);
    }
}

void A_SerpentUnHide(mobj_t * actor)
{
    actor->flags2 &= ~MF2_DONTDRAW;
    actor->floorclip = 24 * FRACUNIT;
}

void A_SerpentHide(mobj_t * actor)
{
    actor->flags2 |= MF2_DONTDRAW;
    actor->floorclip = 0;
}

void A_SerpentChase(mobj_t * actor)
{
    int delta;
    int oldX, oldY, oldFloor;

    if (actor->reactiontime)
    {
        actor->reactiontime--;
    }

    // Modify target threshold
    if (actor->threshold)
    {
        actor->threshold--;
    }

    if (skill_info.flags & SI_FAST_MONSTERS)
    {                           // Monsters move faster in nightmare mode
        actor->tics -= actor->tics / 2;
        if (actor->tics < 3)
        {
            actor->tics = 3;
        }
    }

    //
    // turn towards movement direction if not there yet
    //
    if (actor->movedir < 8)
    {
        actor->angle &= (7 << 29);
        delta = actor->angle - (actor->movedir << 29);
        if (delta > 0)
        {
            actor->angle -= ANG90 / 2;
        }
        else if (delta < 0)
        {
            actor->angle += ANG90 / 2;
        }
    }

    if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
    {                           // look for a new target
        if (P_LookForPlayers(actor, true))
        {                       // got a new target
            return;
        }
        P_SetMobjState(actor, actor->info->spawnstate);
        return;
    }

    //
    // don't attack twice in a row
    //
    if (actor->flags & MF_JUSTATTACKED)
    {
        actor->flags &= ~MF_JUSTATTACKED;
        if (!(skill_info.flags & SI_FAST_MONSTERS))
            P_NewChaseDir(actor);
        return;
    }

    //
    // check for melee attack
    //
    if (actor->info->meleestate && P_CheckMeleeRange(actor))
    {
        if (actor->info->attacksound)
        {
            S_StartMobjSound(actor, actor->info->attacksound);
        }
        P_SetMobjState(actor, actor->info->meleestate);
        return;
    }

    //
    // possibly choose another target
    //
    if (netgame && !actor->threshold && !P_CheckSight(actor, actor->target))
    {
        if (P_LookForPlayers(actor, true))
            return;             // got a new target
    }

    //
    // chase towards player
    //
    oldX = actor->x;
    oldY = actor->y;
    oldFloor = actor->subsector->sector->floorpic;
    if (--actor->movecount < 0 || !P_Move(actor, false))
    {
        P_NewChaseDir(actor);
    }
    if (actor->subsector->sector->floorpic != oldFloor)
    {
        P_TryMove(actor, oldX, oldY, 0);
        P_NewChaseDir(actor);
    }

    //
    // make active sound
    //
    if (actor->info->activesound && P_Random(pr_hexen) < 3)
    {
        S_StartMobjSound(actor, actor->info->activesound);
    }
}

void A_SerpentRaiseHump(mobj_t * actor)
{
    actor->floorclip -= 4 * FRACUNIT;
}

void A_SerpentLowerHump(mobj_t * actor)
{
    actor->floorclip += 4 * FRACUNIT;
}

void A_SerpentHumpDecide(mobj_t * actor)
{
    if (actor->type == HEXEN_MT_SERPENTLEADER)
    {
        if (P_Random(pr_hexen) > 30)
        {
            return;
        }
        else if (P_Random(pr_hexen) < 40)
        {                       // Missile attack
            P_SetMobjState(actor, HEXEN_S_SERPENT_SURFACE1);
            return;
        }
    }
    else if (P_Random(pr_hexen) > 3)
    {
        return;
    }
    if (!P_CheckMeleeRange(actor))
    {                           // The hump shouldn't occur when within melee range
        if (actor->type == HEXEN_MT_SERPENTLEADER && P_Random(pr_hexen) < 128)
        {
            P_SetMobjState(actor, HEXEN_S_SERPENT_SURFACE1);
        }
        else
        {
            P_SetMobjState(actor, HEXEN_S_SERPENT_HUMP1);
            S_StartMobjSound(actor, hexen_sfx_serpent_active);
        }
    }
}

void A_SerpentBirthScream(mobj_t * actor)
{
    S_StartMobjSound(actor, hexen_sfx_serpent_birth);
}

void A_SerpentDiveSound(mobj_t * actor)
{
    S_StartMobjSound(actor, hexen_sfx_serpent_active);
}

void A_SerpentWalk(mobj_t * actor)
{
    int delta;

    if (actor->reactiontime)
    {
        actor->reactiontime--;
    }

    // Modify target threshold
    if (actor->threshold)
    {
        actor->threshold--;
    }

    if (skill_info.flags & SI_FAST_MONSTERS)
    {                           // Monsters move faster in nightmare mode
        actor->tics -= actor->tics / 2;
        if (actor->tics < 3)
        {
            actor->tics = 3;
        }
    }

    //
    // turn towards movement direction if not there yet
    //
    if (actor->movedir < 8)
    {
        actor->angle &= (7 << 29);
        delta = actor->angle - (actor->movedir << 29);
        if (delta > 0)
        {
            actor->angle -= ANG90 / 2;
        }
        else if (delta < 0)
        {
            actor->angle += ANG90 / 2;
        }
    }

    if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
    {                           // look for a new target
        if (P_LookForPlayers(actor, true))
        {                       // got a new target
            return;
        }
        P_SetMobjState(actor, actor->info->spawnstate);
        return;
    }

    //
    // don't attack twice in a row
    //
    if (actor->flags & MF_JUSTATTACKED)
    {
        actor->flags &= ~MF_JUSTATTACKED;
        if (!(skill_info.flags & SI_FAST_MONSTERS))
            P_NewChaseDir(actor);
        return;
    }

    //
    // check for melee attack
    //
    if (actor->info->meleestate && P_CheckMeleeRange(actor))
    {
        if (actor->info->attacksound)
        {
            S_StartMobjSound(actor, actor->info->attacksound);
        }
        P_SetMobjState(actor, HEXEN_S_SERPENT_ATK1);
        return;
    }
    //
    // possibly choose another target
    //
    if (netgame && !actor->threshold && !P_CheckSight(actor, actor->target))
    {
        if (P_LookForPlayers(actor, true))
            return;             // got a new target
    }

    //
    // chase towards player
    //
    if (--actor->movecount < 0 || !P_Move(actor, false))
    {
        P_NewChaseDir(actor);
    }
}

void A_SerpentCheckForAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    if (actor->type == HEXEN_MT_SERPENTLEADER)
    {
        if (!P_CheckMeleeRange(actor))
        {
            P_SetMobjState(actor, HEXEN_S_SERPENT_ATK1);
            return;
        }
    }
    if (P_CheckMeleeRange2(actor))
    {
        P_SetMobjState(actor, HEXEN_S_SERPENT_WALK1);
    }
    else if (P_CheckMeleeRange(actor))
    {
        if (P_Random(pr_hexen) < 32)
        {
            P_SetMobjState(actor, HEXEN_S_SERPENT_WALK1);
        }
        else
        {
            P_SetMobjState(actor, HEXEN_S_SERPENT_ATK1);
        }
    }
}

void A_SerpentChooseAttack(mobj_t * actor)
{
    if (!actor->target || P_CheckMeleeRange(actor))
    {
        return;
    }
    if (actor->type == HEXEN_MT_SERPENTLEADER)
    {
        P_SetMobjState(actor, HEXEN_S_SERPENT_MISSILE1);
    }
}

void A_SerpentMeleeAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(5));
        S_StartMobjSound(actor, hexen_sfx_serpent_meleehit);
    }
    if (P_Random(pr_hexen) < 96)
    {
        A_SerpentCheckForAttack(actor);
    }
}

void A_SerpentMissileAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }

    P_SpawnMissile(actor, actor->target, HEXEN_MT_SERPENTFX);
}

void A_SerpentHeadPop(mobj_t * actor)
{
    P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
                HEXEN_MT_SERPENT_HEAD);
}

void A_SerpentSpawnGibs(mobj_t * actor)
{
    mobj_t *mo;
    int r1, r2;

    r1 = P_Random(pr_hexen);
    r2 = P_Random(pr_hexen);
    mo = P_SpawnMobj(actor->x + ((r2 - 128) << 12),
                     actor->y + ((r1 - 128) << 12),
                     actor->floorz + FRACUNIT, HEXEN_MT_SERPENT_GIB1);
    if (mo)
    {
        mo->momx = (P_Random(pr_hexen) - 128) << 6;
        mo->momy = (P_Random(pr_hexen) - 128) << 6;
        mo->floorclip = 6 * FRACUNIT;
    }
    r1 = P_Random(pr_hexen);
    r2 = P_Random(pr_hexen);
    mo = P_SpawnMobj(actor->x + ((r2 - 128) << 12),
                     actor->y + ((r1 - 128) << 12),
                     actor->floorz + FRACUNIT, HEXEN_MT_SERPENT_GIB2);
    if (mo)
    {
        mo->momx = (P_Random(pr_hexen) - 128) << 6;
        mo->momy = (P_Random(pr_hexen) - 128) << 6;
        mo->floorclip = 6 * FRACUNIT;
    }
    r1 = P_Random(pr_hexen);
    r2 = P_Random(pr_hexen);
    mo = P_SpawnMobj(actor->x + ((r2 - 128) << 12),
                     actor->y + ((r1 - 128) << 12),
                     actor->floorz + FRACUNIT, HEXEN_MT_SERPENT_GIB3);
    if (mo)
    {
        mo->momx = (P_Random(pr_hexen) - 128) << 6;
        mo->momy = (P_Random(pr_hexen) - 128) << 6;
        mo->floorclip = 6 * FRACUNIT;
    }
}

void A_FloatGib(mobj_t * actor)
{
    actor->floorclip -= FRACUNIT;
}

void A_SinkGib(mobj_t * actor)
{
    actor->floorclip += FRACUNIT;
}

void A_DelayGib(mobj_t * actor)
{
    actor->tics -= P_Random(pr_hexen) >> 2;
}

void A_SerpentHeadCheck(mobj_t * actor)
{
    if (actor->z <= actor->floorz)
    {
        if (P_GetThingFloorType(actor) >= FLOOR_LIQUID)
        {
            P_HitFloor(actor);
            P_SetMobjState(actor, HEXEN_S_NULL);
        }
        else
        {
            P_SetMobjState(actor, HEXEN_S_SERPENT_HEAD_X1);
        }
    }
}

void A_CentaurAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, P_Random(pr_hexen) % 7 + 3);
    }
}

void A_CentaurAttack2(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    P_SpawnMissile(actor, actor->target, HEXEN_MT_CENTAUR_FX);
    S_StartMobjSound(actor, hexen_sfx_centaurleader_attack);
}

void A_CentaurDropStuff(mobj_t * actor)
{
    mobj_t *mo;
    angle_t angle;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
                     HEXEN_MT_CENTAUR_SHIELD);
    if (mo)
    {
        angle = actor->angle + ANG90;
        mo->momz = FRACUNIT * 8 + (P_Random(pr_hexen) << 10);
        mo->momx = FixedMul(((P_Random(pr_hexen) - 128) << 11) + FRACUNIT,
                            finecosine[angle >> ANGLETOFINESHIFT]);
        mo->momy = FixedMul(((P_Random(pr_hexen) - 128) << 11) + FRACUNIT,
                            finesine[angle >> ANGLETOFINESHIFT]);
        P_SetTarget(&mo->target, actor);
    }
    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
                     HEXEN_MT_CENTAUR_SWORD);
    if (mo)
    {
        angle = actor->angle - ANG90;
        mo->momz = FRACUNIT * 8 + (P_Random(pr_hexen) << 10);
        mo->momx = FixedMul(((P_Random(pr_hexen) - 128) << 11) + FRACUNIT,
                            finecosine[angle >> ANGLETOFINESHIFT]);
        mo->momy = FixedMul(((P_Random(pr_hexen) - 128) << 11) + FRACUNIT,
                            finesine[angle >> ANGLETOFINESHIFT]);
        P_SetTarget(&mo->target, actor);
    }
}

void A_CentaurDefend(mobj_t * actor)
{
    A_FaceTarget(actor);
    if (P_CheckMeleeRange(actor) && P_Random(pr_hexen) < 32)
    {
        A_UnSetInvulnerable(actor);
        P_SetMobjState(actor, actor->info->meleestate);
    }
}

void A_BishopAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    S_StartMobjSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(4));
        return;
    }
    actor->special1.i = (P_Random(pr_hexen) & 3) + 5;
}

void A_BishopAttack2(mobj_t * actor)
{
    mobj_t *mo;

    if (!actor->target || !actor->special1.i)
    {
        actor->special1.i = 0;
        P_SetMobjState(actor, HEXEN_S_BISHOP_WALK1);
        return;
    }
    mo = P_SpawnMissile(actor, actor->target, HEXEN_MT_BISH_FX);
    if (mo)
    {
        P_SetTarget(&mo->special1.m, actor->target);
        mo->special2.i = 16;      // High word == x/y, Low word == z
    }
    actor->special1.i--;
}

void A_BishopMissileWeave(mobj_t * actor)
{
    fixed_t newX, newY;
    int weaveXY, weaveZ;
    int angle;

    weaveXY = actor->special2.i >> 16;
    weaveZ = actor->special2.i & 0xFFFF;
    angle = (actor->angle + ANG90) >> ANGLETOFINESHIFT;
    newX = actor->x - FixedMul(finecosine[angle],
                               FloatBobOffsets[weaveXY] << 1);
    newY = actor->y - FixedMul(finesine[angle],
                               FloatBobOffsets[weaveXY] << 1);
    weaveXY = (weaveXY + 2) & 63;
    newX += FixedMul(finecosine[angle], FloatBobOffsets[weaveXY] << 1);
    newY += FixedMul(finesine[angle], FloatBobOffsets[weaveXY] << 1);
    P_TryMove(actor, newX, newY, false);
    actor->z -= FloatBobOffsets[weaveZ];
    weaveZ = (weaveZ + 2) & 63;
    actor->z += FloatBobOffsets[weaveZ];
    actor->special2.i = weaveZ + (weaveXY << 16);
}

void A_BishopMissileSeek(mobj_t * actor)
{
    P_SeekerMissile(actor, &actor->special1.m, ANG1 * 2, ANG1 * 3, false);
}

void A_BishopDecide(mobj_t * actor)
{
    if (P_Random(pr_hexen) < 220)
    {
        return;
    }
    else
    {
        P_SetMobjState(actor, HEXEN_S_BISHOP_BLUR1);
    }
}

void A_BishopDoBlur(mobj_t * actor)
{
    actor->special1.i = (P_Random(pr_hexen) & 3) + 3;     // Random number of blurs
    if (P_Random(pr_hexen) < 120)
    {
        P_ThrustMobj(actor, actor->angle + ANG90, 11 * FRACUNIT);
    }
    else if (P_Random(pr_hexen) > 125)
    {
        P_ThrustMobj(actor, actor->angle - ANG90, 11 * FRACUNIT);
    }
    else
    {                           // Thrust forward
        P_ThrustMobj(actor, actor->angle, 11 * FRACUNIT);
    }
    S_StartMobjSound(actor, hexen_sfx_bishop_blur);
}

void A_BishopSpawnBlur(mobj_t * actor)
{
    mobj_t *mo;

    if (!--actor->special1.i)
    {
        actor->momx = 0;
        actor->momy = 0;
        if (P_Random(pr_hexen) > 96)
        {
            P_SetMobjState(actor, HEXEN_S_BISHOP_WALK1);
        }
        else
        {
            P_SetMobjState(actor, HEXEN_S_BISHOP_ATK1);
        }
    }
    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_BISHOPBLUR);
    if (mo)
    {
        mo->angle = actor->angle;
    }
}

void A_BishopChase(mobj_t * actor)
{
    actor->z -= FloatBobOffsets[actor->special2.i] >> 1;
    actor->special2.i = (actor->special2.i + 4) & 63;
    actor->z += FloatBobOffsets[actor->special2.i] >> 1;
}

void A_BishopPuff(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 40 * FRACUNIT,
                     HEXEN_MT_BISHOP_PUFF);
    if (mo)
    {
        mo->momz = FRACUNIT / 2;
    }
}

void A_BishopPainBlur(mobj_t * actor)
{
    mobj_t *mo;
    int r1,r2,r3;

    if (P_Random(pr_hexen) < 64)
    {
        P_SetMobjState(actor, HEXEN_S_BISHOP_BLUR1);
        return;
    }

    r1 = P_SubRandom();
    r2 = P_SubRandom();
    r3 = P_SubRandom();

    mo = P_SpawnMobj(actor->x + (r3 << 12), actor->y
                     + (r2 << 12),
                     actor->z + (r1 << 11),
                     HEXEN_MT_BISHOPPAINBLUR);
    if (mo)
    {
        mo->angle = actor->angle;
    }
}

static void DragonSeek(mobj_t * actor, angle_t thresh, angle_t turnMax)
{
    int dir;
    int dist;
    angle_t delta;
    angle_t angle;
    mobj_t *target;
    int search;
    int i;
    int bestArg;
    angle_t bestAngle;
    angle_t angleToSpot, angleToTarget;
    mobj_t *mo;

    target = actor->special1.m;
    if (target == NULL)
    {
        return;
    }
    dir = P_FaceMobj(actor, target, &delta);
    if (delta > thresh)
    {
        delta >>= 1;
        if (delta > turnMax)
        {
            delta = turnMax;
        }
    }
    if (dir)
    {                           // Turn clockwise
        actor->angle += delta;
    }
    else
    {                           // Turn counter clockwise
        actor->angle -= delta;
    }
    angle = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul(actor->info->speed, finecosine[angle]);
    actor->momy = FixedMul(actor->info->speed, finesine[angle]);
    if (actor->z + actor->height < target->z
        || target->z + target->height < actor->z)
    {
        dist = P_AproxDistance(target->x - actor->x, target->y - actor->y);
        dist = dist / actor->info->speed;
        if (dist < 1)
        {
            dist = 1;
        }
        actor->momz = (target->z - actor->z) / dist;
    }
    else
    {
        dist = P_AproxDistance(target->x - actor->x, target->y - actor->y);
        dist = dist / actor->info->speed;
    }
    if (target->flags & MF_SHOOTABLE && P_Random(pr_hexen) < 64)
    {                           // attack the destination mobj if it's attackable
        mobj_t *oldTarget;

        if (abs((int) actor->angle - (int) R_PointToAngle2(actor->x, actor->y,
                                               target->x,
                                               target->y)) < ANG45 / 2)
        {
            oldTarget = actor->target;
            actor->target = target;
            if (P_CheckMeleeRange(actor))
            {
                P_DamageMobj(actor->target, actor, actor, HITDICE(10));
                S_StartMobjSound(actor, hexen_sfx_dragon_attack);
            }
            else if (P_Random(pr_hexen) < 128 && P_CheckMissileRange(actor))
            {
                P_SpawnMissile(actor, target, HEXEN_MT_DRAGON_FX);
                S_StartMobjSound(actor, hexen_sfx_dragon_attack);
            }
            actor->target = oldTarget;
        }
    }
    if (dist < 4)
    {                           // Hit the target thing
        if (actor->target && P_Random(pr_hexen) < 200)
        {
            bestArg = -1;
            bestAngle = ANGLE_MAX;
            angleToTarget = R_PointToAngle2(actor->x, actor->y,
                                            actor->target->x,
                                            actor->target->y);
            for (i = 0; i < 5; i++)
            {
                if (!target->special_args[i])
                {
                    continue;
                }
                search = -1;
                mo = P_FindMobjFromTID(target->special_args[i], &search);
                angleToSpot = R_PointToAngle2(actor->x, actor->y,
                                              mo->x, mo->y);
                if (abs((int) angleToSpot - (int) angleToTarget) < bestAngle)
                {
                    bestAngle = abs((int) angleToSpot - (int) angleToTarget);
                    bestArg = i;
                }
            }
            if (bestArg != -1)
            {
                search = -1;
                P_SetTarget(
                  &actor->special1.m,
                  P_FindMobjFromTID(target->special_args[bestArg], &search)
                );
            }
        }
        else
        {
            do
            {
                i = (P_Random(pr_hexen) >> 2) % 5;
            }
            while (!target->special_args[i]);
            search = -1;
            P_SetTarget(
              &actor->special1.m,
              P_FindMobjFromTID(target->special_args[i], &search)
            );
        }
    }
}

void A_DragonInitFlight(mobj_t * actor)
{
    int search;

    search = -1;
    do
    {                           // find the first tid identical to the dragon's tid
        P_SetTarget(
          &actor->special1.m,
          P_FindMobjFromTID(actor->tid, &search)
        );
        if (search == -1)
        {
            P_SetMobjState(actor, actor->info->spawnstate);
            return;
        }
    }
    while (actor->special1.m == actor);
    map_format.remove_mobj_thing_id(actor);
}

void A_DragonFlight(mobj_t * actor)
{
    angle_t angle;

    DragonSeek(actor, 4 * ANG1, 8 * ANG1);
    if (actor->target)
    {
        if (!(actor->target->flags & MF_SHOOTABLE))
        {                       // target died
            P_SetTarget(&actor->target, NULL);
            return;
        }
        angle = R_PointToAngle2(actor->x, actor->y, actor->target->x,
                                actor->target->y);
        if (abs((int) actor->angle - (int) angle) < ANG45 / 2
            && P_CheckMeleeRange(actor))
        {
            P_DamageMobj(actor->target, actor, actor, HITDICE(8));
            S_StartMobjSound(actor, hexen_sfx_dragon_attack);
        }
        else if (abs((int) actor->angle - (int) angle) <= ANG1 * 20)
        {
            P_SetMobjState(actor, actor->info->missilestate);
            S_StartMobjSound(actor, hexen_sfx_dragon_attack);
        }
    }
    else
    {
        P_LookForPlayers(actor, true);
    }
}

void A_DragonFlap(mobj_t * actor)
{
    A_DragonFlight(actor);
    if (P_Random(pr_hexen) < 240)
    {
        S_StartMobjSound(actor, hexen_sfx_dragon_wingflap);
    }
    else
    {
        S_StartMobjSound(actor, actor->info->activesound);
    }
}

void A_DragonAttack(mobj_t * actor)
{
    P_SpawnMissile(actor, actor->target, HEXEN_MT_DRAGON_FX);
}

void A_DragonFX2(mobj_t * actor)
{
    mobj_t *mo;
    int i;
    int r1,r2,r3;
    int delay;

    delay = 16 + (P_Random(pr_hexen) >> 3);
    for (i = 1 + (P_Random(pr_hexen) & 3); i; i--)
    {
        r1 = P_Random(pr_hexen);
        r2 = P_Random(pr_hexen);
        r3 = P_Random(pr_hexen);
        mo = P_SpawnMobj(actor->x + ((r3 - 128) << 14),
                         actor->y + ((r2 - 128) << 14),
                         actor->z + ((r1 - 128) << 12),
                         HEXEN_MT_DRAGON_FX2);
        if (mo)
        {
            mo->tics = delay + (P_Random(pr_hexen) & 3) * i * 2;
            P_SetTarget(&mo->target, actor->target);
        }
    }
}

void A_DragonPain(mobj_t * actor)
{
    A_Pain(actor);
    if (!actor->special1.m)
    {                           // no destination spot yet
        P_SetMobjState(actor, HEXEN_S_DRAGON_INIT);
    }
}

void A_DragonCheckCrash(mobj_t * actor)
{
    if (actor->z <= actor->floorz)
    {
        P_SetMobjState(actor, HEXEN_S_DRAGON_CRASH1);
    }
}

void A_DemonAttack1(mobj_t * actor)
{
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(2));
    }
}

void A_DemonAttack2(mobj_t * actor)
{
    mobj_t *mo;
    int fireBall;

    if (actor->type == HEXEN_MT_DEMON)
    {
        fireBall = HEXEN_MT_DEMONFX1;
    }
    else
    {
        fireBall = HEXEN_MT_DEMON2FX1;
    }
    mo = P_SpawnMissile(actor, actor->target, fireBall);
    if (mo)
    {
        mo->z += 30 * FRACUNIT;
        S_StartMobjSound(actor, hexen_sfx_demon_missile_fire);
    }
}

void A_DemonDeath(mobj_t * actor)
{
    mobj_t *mo;
    angle_t angle;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
                     HEXEN_MT_DEMONCHUNK1);
    if (mo)
    {
        angle = actor->angle + ANG90;
        mo->momz = 8 * FRACUNIT;
        mo->momx = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finecosine[angle >> ANGLETOFINESHIFT]);
        mo->momy = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finesine[angle >> ANGLETOFINESHIFT]);
        P_SetTarget(&mo->target, actor);
    }
    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
                     HEXEN_MT_DEMONCHUNK2);
    if (mo)
    {
        angle = actor->angle - ANG90;
        mo->momz = 8 * FRACUNIT;
        mo->momx = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finecosine[angle >> ANGLETOFINESHIFT]);
        mo->momy = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finesine[angle >> ANGLETOFINESHIFT]);
        P_SetTarget(&mo->target, actor);
    }
    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
                     HEXEN_MT_DEMONCHUNK3);
    if (mo)
    {
        angle = actor->angle - ANG90;
        mo->momz = 8 * FRACUNIT;
        mo->momx = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finecosine[angle >> ANGLETOFINESHIFT]);
        mo->momy = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finesine[angle >> ANGLETOFINESHIFT]);
        P_SetTarget(&mo->target, actor);
    }
    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
                     HEXEN_MT_DEMONCHUNK4);
    if (mo)
    {
        angle = actor->angle - ANG90;
        mo->momz = 8 * FRACUNIT;
        mo->momx = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finecosine[angle >> ANGLETOFINESHIFT]);
        mo->momy = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finesine[angle >> ANGLETOFINESHIFT]);
        P_SetTarget(&mo->target, actor);
    }
    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
                     HEXEN_MT_DEMONCHUNK5);
    if (mo)
    {
        angle = actor->angle - ANG90;
        mo->momz = 8 * FRACUNIT;
        mo->momx = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finecosine[angle >> ANGLETOFINESHIFT]);
        mo->momy = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finesine[angle >> ANGLETOFINESHIFT]);
        P_SetTarget(&mo->target, actor);
    }
}

void A_Demon2Death(mobj_t * actor)
{
    mobj_t *mo;
    angle_t angle;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
                     HEXEN_MT_DEMON2CHUNK1);
    if (mo)
    {
        angle = actor->angle + ANG90;
        mo->momz = 8 * FRACUNIT;
        mo->momx = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finecosine[angle >> ANGLETOFINESHIFT]);
        mo->momy = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finesine[angle >> ANGLETOFINESHIFT]);
        P_SetTarget(&mo->target, actor);
    }
    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
                     HEXEN_MT_DEMON2CHUNK2);
    if (mo)
    {
        angle = actor->angle - ANG90;
        mo->momz = 8 * FRACUNIT;
        mo->momx = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finecosine[angle >> ANGLETOFINESHIFT]);
        mo->momy = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finesine[angle >> ANGLETOFINESHIFT]);
        P_SetTarget(&mo->target, actor);
    }
    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
                     HEXEN_MT_DEMON2CHUNK3);
    if (mo)
    {
        angle = actor->angle - ANG90;
        mo->momz = 8 * FRACUNIT;
        mo->momx = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finecosine[angle >> ANGLETOFINESHIFT]);
        mo->momy = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finesine[angle >> ANGLETOFINESHIFT]);
        P_SetTarget(&mo->target, actor);
    }
    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
                     HEXEN_MT_DEMON2CHUNK4);
    if (mo)
    {
        angle = actor->angle - ANG90;
        mo->momz = 8 * FRACUNIT;
        mo->momx = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finecosine[angle >> ANGLETOFINESHIFT]);
        mo->momy = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finesine[angle >> ANGLETOFINESHIFT]);
        P_SetTarget(&mo->target, actor);
    }
    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
                     HEXEN_MT_DEMON2CHUNK5);
    if (mo)
    {
        angle = actor->angle - ANG90;
        mo->momz = 8 * FRACUNIT;
        mo->momx = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finecosine[angle >> ANGLETOFINESHIFT]);
        mo->momy = FixedMul((P_Random(pr_hexen) << 10) + FRACUNIT,
                            finesine[angle >> ANGLETOFINESHIFT]);
        P_SetTarget(&mo->target, actor);
    }
}

dboolean A_SinkMobj(mobj_t * actor)
{
    if (actor->floorclip < actor->info->height)
    {
        switch (actor->type)
        {
            case HEXEN_MT_THRUSTFLOOR_DOWN:
            case HEXEN_MT_THRUSTFLOOR_UP:
                actor->floorclip += 6 * FRACUNIT;
                break;
            default:
                actor->floorclip += FRACUNIT;
                break;
        }
        return false;
    }
    return true;
}

dboolean A_RaiseMobj(mobj_t * actor)
{
    int done = true;

    // Raise a mobj from the ground
    if (actor->floorclip > 0)
    {
        switch (actor->type)
        {
            case HEXEN_MT_WRAITHB:
                actor->floorclip -= 2 * FRACUNIT;
                break;
            case HEXEN_MT_THRUSTFLOOR_DOWN:
            case HEXEN_MT_THRUSTFLOOR_UP:
                actor->floorclip -= actor->special2.i * FRACUNIT;
                break;
            default:
                actor->floorclip -= 2 * FRACUNIT;
                break;
        }
        if (actor->floorclip <= 0)
        {
            actor->floorclip = 0;
            done = true;
        }
        else
        {
            done = false;
        }
    }
    return done;                // Reached target height
}

void A_WraithInit(mobj_t * actor)
{
    actor->z += 48 << FRACBITS;
    actor->special1.i = 0;        // index into floatbob
}

void A_WraithRaiseInit(mobj_t * actor)
{
    actor->flags2 &= ~MF2_DONTDRAW;
    actor->flags2 &= ~MF2_NONSHOOTABLE;
    actor->flags |= MF_SHOOTABLE | MF_SOLID;
    actor->floorclip = actor->info->height;
}

void A_WraithRaise(mobj_t * actor)
{
    if (A_RaiseMobj(actor))
    {
        // Reached it's target height
        P_SetMobjState(actor, HEXEN_S_WRAITH_CHASE1);
    }

    P_SpawnDirt(actor, actor->radius);
}

void A_WraithMelee(mobj_t * actor)
{
    int amount;

    // Steal health from target and give to player
    if (P_CheckMeleeRange(actor) && (P_Random(pr_hexen) < 220))
    {
        amount = HITDICE(2);
        P_DamageMobj(actor->target, actor, actor, amount);
        actor->health += amount;
    }
}

void A_WraithMissile(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMissile(actor, actor->target, HEXEN_MT_WRAITHFX1);
    if (mo)
    {
        S_StartMobjSound(actor, hexen_sfx_wraith_missile_fire);
    }
}

void A_WraithFX2(mobj_t * actor)
{
    mobj_t *mo;
    angle_t angle;
    int i;

    for (i = 0; i < 2; i++)
    {
        mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_WRAITHFX2);
        if (mo)
        {
            if (P_Random(pr_hexen) < 128)
            {
                angle = actor->angle + (P_Random(pr_hexen) << 22);
            }
            else
            {
                angle = actor->angle - (P_Random(pr_hexen) << 22);
            }
            mo->momz = 0;
            mo->momx = FixedMul((P_Random(pr_hexen) << 7) + FRACUNIT,
                                finecosine[angle >> ANGLETOFINESHIFT]);
            mo->momy = FixedMul((P_Random(pr_hexen) << 7) + FRACUNIT,
                                finesine[angle >> ANGLETOFINESHIFT]);
            P_SetTarget(&mo->target, actor);
            mo->floorclip = 10 * FRACUNIT;
        }
    }
}

void A_WraithFX3(mobj_t * actor)
{
    mobj_t *mo;
    int numdropped = P_Random(pr_hexen) % 15;
    int i;

    for (i = 0; i < numdropped; i++)
    {
        mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_WRAITHFX3);
        if (mo)
        {
            mo->x += (P_Random(pr_hexen) - 128) << 11;
            mo->y += (P_Random(pr_hexen) - 128) << 11;
            mo->z += (P_Random(pr_hexen) << 10);
            P_SetTarget(&mo->target, actor);
        }
    }
}

void A_WraithFX4(mobj_t * actor)
{
    mobj_t *mo;
    int chance = P_Random(pr_hexen);
    int spawn4, spawn5;

    if (chance < 10)
    {
        spawn4 = true;
        spawn5 = false;
    }
    else if (chance < 20)
    {
        spawn4 = false;
        spawn5 = true;
    }
    else if (chance < 25)
    {
        spawn4 = true;
        spawn5 = true;
    }
    else
    {
        spawn4 = false;
        spawn5 = false;
    }

    if (spawn4)
    {
        mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_WRAITHFX4);
        if (mo)
        {
            mo->x += (P_Random(pr_hexen) - 128) << 12;
            mo->y += (P_Random(pr_hexen) - 128) << 12;
            mo->z += (P_Random(pr_hexen) << 10);
            P_SetTarget(&mo->target, actor);
        }
    }
    if (spawn5)
    {
        mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_WRAITHFX5);
        if (mo)
        {
            mo->x += (P_Random(pr_hexen) - 128) << 11;
            mo->y += (P_Random(pr_hexen) - 128) << 11;
            mo->z += (P_Random(pr_hexen) << 10);
            P_SetTarget(&mo->target, actor);
        }
    }
}

void A_WraithLook(mobj_t * actor)
{
//  A_WraithFX4(actor);             // too expensive
    A_Look(actor);
}

void A_WraithChase(mobj_t * actor)
{
    int weaveindex = actor->special1.i;
    actor->z += FloatBobOffsets[weaveindex];
    actor->special1.i = (weaveindex + 2) & 63;
    A_Chase(actor);
    A_WraithFX4(actor);
}

void A_EttinAttack(mobj_t * actor)
{
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(2));
    }
}

void A_DropMace(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMobj(actor->x, actor->y,
                     actor->z + (actor->height >> 1), HEXEN_MT_ETTIN_MACE);
    if (mo)
    {
        mo->momx = (P_Random(pr_hexen) - 128) << 11;
        mo->momy = (P_Random(pr_hexen) - 128) << 11;
        mo->momz = FRACUNIT * 10 + (P_Random(pr_hexen) << 10);
        P_SetTarget(&mo->target, actor);
    }
}

void A_FiredSpawnRock(mobj_t * actor)
{
    mobj_t *mo;
    int x, y, z;
    int rtype = 0;

    switch (P_Random(pr_hexen) % 5)
    {
        case 0:
            rtype = HEXEN_MT_FIREDEMON_FX1;
            break;
        case 1:
            rtype = HEXEN_MT_FIREDEMON_FX2;
            break;
        case 2:
            rtype = HEXEN_MT_FIREDEMON_FX3;
            break;
        case 3:
            rtype = HEXEN_MT_FIREDEMON_FX4;
            break;
        case 4:
            rtype = HEXEN_MT_FIREDEMON_FX5;
            break;
    }

    x = actor->x + ((P_Random(pr_hexen) - 128) << 12);
    y = actor->y + ((P_Random(pr_hexen) - 128) << 12);
    z = actor->z + ((P_Random(pr_hexen)) << 11);
    mo = P_SpawnMobj(x, y, z, rtype);
    if (mo)
    {
        P_SetTarget(&mo->target, actor);
        mo->momx = (P_Random(pr_hexen) - 128) << 10;
        mo->momy = (P_Random(pr_hexen) - 128) << 10;
        mo->momz = (P_Random(pr_hexen) << 10);
        mo->special1.i = 2;       // Number bounces
    }

    // Initialize fire demon
    actor->special2.i = 0;
    actor->flags &= ~MF_JUSTATTACKED;
}

void A_FiredRocks(mobj_t * actor)
{
    A_FiredSpawnRock(actor);
    A_FiredSpawnRock(actor);
    A_FiredSpawnRock(actor);
    A_FiredSpawnRock(actor);
    A_FiredSpawnRock(actor);
}

void A_FiredAttack(mobj_t * actor)
{
    mobj_t *mo;
    mo = P_SpawnMissile(actor, actor->target, HEXEN_MT_FIREDEMON_FX6);
    if (mo)
        S_StartMobjSound(actor, hexen_sfx_fired_attack);
}

void A_SmBounce(mobj_t * actor)
{
    // give some more momentum (x,y,&z)
    actor->z = actor->floorz + FRACUNIT;
    actor->momz = (2 * FRACUNIT) + (P_Random(pr_hexen) << 10);
    actor->momx = P_Random(pr_hexen) % 3 << FRACBITS;
    actor->momy = P_Random(pr_hexen) % 3 << FRACBITS;
}

#define FIREDEMON_ATTACK_RANGE	64*8*FRACUNIT

void A_FiredChase(mobj_t * actor)
{
    int weaveindex = actor->special1.i;
    mobj_t *target = actor->target;
    angle_t ang;
    fixed_t dist;

    if (actor->reactiontime)
        actor->reactiontime--;
    if (actor->threshold)
        actor->threshold--;

    // Float up and down
    actor->z += FloatBobOffsets[weaveindex];
    actor->special1.i = (weaveindex + 2) & 63;

    // Insure it stays above certain height
    if (actor->z < actor->floorz + (64 * FRACUNIT))
    {
        actor->z += 2 * FRACUNIT;
    }

    if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
    {                           // Invalid target
        P_LookForPlayers(actor, true);
        return;
    }

    // Strafe
    if (actor->special2.i > 0)
    {
        actor->special2.i--;
    }
    else
    {
        actor->special2.i = 0;
        actor->momx = actor->momy = 0;
        dist = P_AproxDistance(actor->x - target->x, actor->y - target->y);
        if (dist < FIREDEMON_ATTACK_RANGE)
        {
            if (P_Random(pr_hexen) < 30)
            {
                ang =
                    R_PointToAngle2(actor->x, actor->y, target->x, target->y);
                if (P_Random(pr_hexen) < 128)
                    ang += ANG90;
                else
                    ang -= ANG90;
                ang >>= ANGLETOFINESHIFT;
                actor->momx = FixedMul(8 * FRACUNIT, finecosine[ang]);
                actor->momy = FixedMul(8 * FRACUNIT, finesine[ang]);
                actor->special2.i = 3;    // strafe time
            }
        }
    }

    FaceMovementDirection(actor);

    // Normal movement
    if (!actor->special2.i)
    {
        if (--actor->movecount < 0 || !P_Move(actor, false))
        {
            P_NewChaseDir(actor);
        }
    }

    // Do missile attack
    if (!(actor->flags & MF_JUSTATTACKED))
    {
        if (P_CheckMissileRange(actor) && (P_Random(pr_hexen) < 20))
        {
            P_SetMobjState(actor, actor->info->missilestate);
            actor->flags |= MF_JUSTATTACKED;
            return;
        }
    }
    else
    {
        actor->flags &= ~MF_JUSTATTACKED;
    }

    // make active sound
    if (actor->info->activesound && P_Random(pr_hexen) < 3)
    {
        S_StartMobjSound(actor, actor->info->activesound);
    }
}

void A_FiredSplotch(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_FIREDEMON_SPLOTCH1);
    if (mo)
    {
        mo->momx = (P_Random(pr_hexen) - 128) << 11;
        mo->momy = (P_Random(pr_hexen) - 128) << 11;
        mo->momz = FRACUNIT * 3 + (P_Random(pr_hexen) << 10);
    }
    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_FIREDEMON_SPLOTCH2);
    if (mo)
    {
        mo->momx = (P_Random(pr_hexen) - 128) << 11;
        mo->momy = (P_Random(pr_hexen) - 128) << 11;
        mo->momz = FRACUNIT * 3 + (P_Random(pr_hexen) << 10);
    }
}

#include "heretic/sb_bar.h"

void A_FreezeDeath(mobj_t * actor)
{
    int r = P_Random(pr_hexen);
    actor->tics = 75 + r + P_Random(pr_hexen);
    actor->flags |= MF_SOLID | MF_SHOOTABLE | MF_NOBLOOD;
    actor->flags2 |= MF2_PUSHABLE | MF2_TELESTOMP | MF2_PASSMOBJ | MF2_SLIDE;
    actor->height <<= 2;
    S_StartMobjSound(actor, hexen_sfx_freeze_death);

    if (actor->player)
    {
        actor->player->damagecount = 0;
        actor->player->poisoncount = 0;
        actor->player->bonuscount = 0;
        if (actor->player == &players[consoleplayer])
        {
            SB_PaletteFlash(false);
        }
    }
    else if (actor->flags & MF_COUNTKILL && actor->special)
    {
        // Initiate monster death actions.
        map_format.execute_line_special(actor->special, actor->special_args, NULL, 0, actor);
    }
}

void A_IceSetTics(mobj_t * actor)
{
    int floor;

    actor->tics = 70 + (P_Random(pr_hexen) & 63);
    floor = P_GetThingFloorType(actor);
    if (floor == FLOOR_LAVA)
    {
        actor->tics >>= 2;
    }
    else if (floor == FLOOR_ICE)
    {
        actor->tics <<= 1;
    }
}

void A_IceCheckHeadDone(mobj_t * actor)
{
    if (actor->special2.i == 666)
    {
        P_SetMobjState(actor, HEXEN_S_ICECHUNK_HEAD2);
    }
}

void A_FreezeDeathChunks(mobj_t * actor)
{
    int i;
    int r1,r2,r3;
    mobj_t *mo;

    if (actor->momx || actor->momy || actor->momz)
    {
        actor->tics = 105;
        return;
    }
    S_StartMobjSound(actor, hexen_sfx_freeze_shatter);

    for (i = 12 + (P_Random(pr_hexen) & 15); i >= 0; i--)
    {
        r1 = P_Random(pr_hexen);
        r2 = P_Random(pr_hexen);
        r3 = P_Random(pr_hexen);
        mo = P_SpawnMobj(actor->x +
                         (((r3 - 128) * actor->radius) >> 7),
                         actor->y +
                         (((r2 - 128) * actor->radius) >> 7),
                         actor->z + (r1 * actor->height / 255),
                         HEXEN_MT_ICECHUNK);
        P_SetMobjState(mo, mo->info->spawnstate + (P_Random(pr_hexen) % 3));
        mo->momz = FixedDiv(mo->z - actor->z, actor->height) << 2;
        mo->momx = P_SubRandom() << (FRACBITS - 7);
        mo->momy = P_SubRandom() << (FRACBITS - 7);
        A_IceSetTics(mo);   // set a random tic wait
    }
    for (i = 12 + (P_Random(pr_hexen) & 15); i >= 0; i--)
    {
        r1 = P_Random(pr_hexen);
        r2 = P_Random(pr_hexen);
        r3 = P_Random(pr_hexen);
        mo = P_SpawnMobj(actor->x +
                         (((r3 - 128) * actor->radius) >> 7),
                         actor->y +
                         (((r2 - 128) * actor->radius) >> 7),
                         actor->z + (r1 * actor->height / 255),
                         HEXEN_MT_ICECHUNK);
        P_SetMobjState(mo, mo->info->spawnstate + (P_Random(pr_hexen) % 3));
        mo->momz = FixedDiv(mo->z - actor->z, actor->height) << 2;
        mo->momx = P_SubRandom() << (FRACBITS - 7);
        mo->momy = P_SubRandom() << (FRACBITS - 7);
        A_IceSetTics(mo);   // set a random tic wait
    }
    if (actor->player)
    {                           // attach the player's view to a chunk of ice
        mo = P_SpawnMobj(actor->x, actor->y, actor->z + g_viewheight,
                         HEXEN_MT_ICECHUNK);
        P_SetMobjState(mo, HEXEN_S_ICECHUNK_HEAD);
        mo->momz = FixedDiv(mo->z - actor->z, actor->height) << 2;
        mo->momx = P_SubRandom() << (FRACBITS - 7);
        mo->momy = P_SubRandom() << (FRACBITS - 7);
        mo->flags2 |= MF2_ICEDAMAGE;    // used to force blue palette
        mo->flags2 &= ~MF2_FOOTCLIP;
        mo->player = actor->player;
        actor->player = NULL;
        mo->health = actor->health;
        mo->angle = actor->angle;
        mo->player->mo = mo;
        mo->player->lookdir = 0;
    }
    map_format.remove_mobj_thing_id(actor);
    P_SetMobjState(actor, HEXEN_S_FREETARGMOBJ);
    actor->flags2 |= MF2_DONTDRAW;
}

void A_IceGuyLook(mobj_t * actor)
{
    fixed_t dist;
    fixed_t an;

    A_Look(actor);
    if (P_Random(pr_hexen) < 64)
    {
        dist = ((P_Random(pr_hexen) - 128) * actor->radius) >> 7;
        an = (actor->angle + ANG90) >> ANGLETOFINESHIFT;

        P_SpawnMobj(actor->x + FixedMul(dist, finecosine[an]),
                    actor->y + FixedMul(dist, finesine[an]),
                    actor->z + 60 * FRACUNIT,
                    HEXEN_MT_ICEGUY_WISP1 + (P_Random(pr_hexen) & 1));
    }
}

void A_IceGuyChase(mobj_t * actor)
{
    fixed_t dist;
    fixed_t an;
    mobj_t *mo;

    A_Chase(actor);
    if (P_Random(pr_hexen) < 128)
    {
        dist = ((P_Random(pr_hexen) - 128) * actor->radius) >> 7;
        an = (actor->angle + ANG90) >> ANGLETOFINESHIFT;

        mo = P_SpawnMobj(actor->x + FixedMul(dist, finecosine[an]),
                         actor->y + FixedMul(dist, finesine[an]),
                         actor->z + 60 * FRACUNIT,
                         HEXEN_MT_ICEGUY_WISP1 + (P_Random(pr_hexen) & 1));
        if (mo)
        {
            mo->momx = actor->momx;
            mo->momy = actor->momy;
            mo->momz = actor->momz;
            P_SetTarget(&mo->target, actor);
        }
    }
}

void A_IceGuyAttack(mobj_t * actor)
{
    fixed_t an;

    if (!actor->target)
    {
        return;
    }
    an = (actor->angle + ANG90) >> ANGLETOFINESHIFT;
    P_SpawnMissileXYZ(actor->x + FixedMul(actor->radius >> 1,
                                          finecosine[an]),
                      actor->y + FixedMul(actor->radius >> 1, finesine[an]),
                      actor->z + 40 * FRACUNIT, actor, actor->target,
                      HEXEN_MT_ICEGUY_FX);
    an = (actor->angle - ANG90) >> ANGLETOFINESHIFT;
    P_SpawnMissileXYZ(actor->x + FixedMul(actor->radius >> 1,
                                          finecosine[an]),
                      actor->y + FixedMul(actor->radius >> 1, finesine[an]),
                      actor->z + 40 * FRACUNIT, actor, actor->target,
                      HEXEN_MT_ICEGUY_FX);
    S_StartMobjSound(actor, actor->info->attacksound);
}

void A_IceGuyMissilePuff(mobj_t * actor)
{
    P_SpawnMobj(actor->x, actor->y, actor->z + 2 * FRACUNIT, HEXEN_MT_ICEFX_PUFF);
}

void A_IceGuyDie(mobj_t * actor)
{
    actor->momx = 0;
    actor->momy = 0;
    actor->momz = 0;
    actor->height <<= 2;
    A_FreezeDeathChunks(actor);
}

void A_IceGuyMissileExplode(mobj_t * actor)
{
    mobj_t *mo;
    unsigned int i;

    for (i = 0; i < 8; i++)
    {
        mo = P_SpawnMissileAngle(actor, HEXEN_MT_ICEGUY_FX2, i * ANG45,
                                 -0.3 * FRACUNIT);
        if (mo)
        {
            P_SetTarget(&mo->target, actor->target);
        }
    }
}

//============================================================================
//
//      Sorcerer stuff
//
// Sorcerer Variables
//              special1                Angle of ball 1 (all others relative to that)
//              special2                which ball to stop at in stop mode (HEXEN_MT_???)
//              args[0]                 Denfense time
//              args[1]                 Number of full rotations since stopping mode
//              args[2]                 Target orbit speed for acceleration/deceleration
//              args[3]                 Movement mode (see SORC_ macros)
//              args[4]                 Current ball orbit speed
//      Sorcerer Ball Variables
//              special1                Previous angle of ball (for woosh)
//              special2                Countdown of rapid fire (FX4)
//              args[0]                 If set, don't play the bounce sound when bouncing
//============================================================================

#define SORCBALL_INITIAL_SPEED 		7
#define SORCBALL_TERMINAL_SPEED		25
#define SORCBALL_SPEED_ROTATIONS 	5
#define SORC_DEFENSE_TIME			255
#define SORC_DEFENSE_HEIGHT			45
#define BOUNCE_TIME_UNIT			(35/2)
#define SORCFX4_RAPIDFIRE_TIME		(6*3)   // 3 seconds
#define SORCFX4_SPREAD_ANGLE		20

#define SORC_DECELERATE		0
#define SORC_ACCELERATE 	1
#define SORC_STOPPING		2
#define SORC_FIRESPELL		3
#define SORC_STOPPED		4
#define SORC_NORMAL			5
#define SORC_FIRING_SPELL	6

#define BALL1_ANGLEOFFSET	0
#define BALL2_ANGLEOFFSET	(ANGLE_MAX/3)
#define BALL3_ANGLEOFFSET	((ANGLE_MAX/3)*2)

void A_SorcBallOrbit(mobj_t * actor);
void A_SorcSpinBalls(mobj_t * actor);
void A_SpeedBalls(mobj_t * actor);
void A_SlowBalls(mobj_t * actor);
void A_StopBalls(mobj_t * actor);
void A_AccelBalls(mobj_t * actor);
void A_DecelBalls(mobj_t * actor);
void A_SorcBossAttack(mobj_t * actor);
void A_SpawnFizzle(mobj_t * actor);
void A_CastSorcererSpell(mobj_t * actor);
void A_SorcUpdateBallAngle(mobj_t * actor);
void A_BounceCheck(mobj_t * actor);
void A_SorcFX1Seek(mobj_t * actor);
void A_SorcOffense1(mobj_t * actor);
void A_SorcOffense2(mobj_t * actor);

void A_SorcSpinBalls(mobj_t * actor)
{
    mobj_t *mo;
    fixed_t z;

    A_SlowBalls(actor);
    actor->special_args[0] = 0;         // Currently no defense
    actor->special_args[3] = SORC_NORMAL;
    actor->special_args[4] = SORCBALL_INITIAL_SPEED;    // Initial orbit speed
    actor->special1.i = ANG1;
    z = actor->z - actor->floorclip + actor->info->height;

    mo = P_SpawnMobj(actor->x, actor->y, z, HEXEN_MT_SORCBALL1);
    if (mo)
    {
        P_SetTarget(&mo->target, actor);
        mo->special2.i = SORCFX4_RAPIDFIRE_TIME;
    }
    mo = P_SpawnMobj(actor->x, actor->y, z, HEXEN_MT_SORCBALL2);
    if (mo)
        P_SetTarget(&mo->target, actor);
    mo = P_SpawnMobj(actor->x, actor->y, z, HEXEN_MT_SORCBALL3);
    if (mo)
        P_SetTarget(&mo->target, actor);
}

void A_SorcBallOrbit(mobj_t * actor)
{
    int x, y;
    angle_t angle, baseangle;
    int mode = actor->target->special_args[3];
    mobj_t *parent = (mobj_t *) actor->target;
    int dist = parent->radius - (actor->radius << 1);
    angle_t prevangle = actor->special1.i;

    if (actor->target->health <= 0)
        P_SetMobjState(actor, actor->info->painstate);

    baseangle = (angle_t) parent->special1.i;
    switch (actor->type)
    {
        case HEXEN_MT_SORCBALL1:
            angle = baseangle + BALL1_ANGLEOFFSET;
            break;
        case HEXEN_MT_SORCBALL2:
            angle = baseangle + BALL2_ANGLEOFFSET;
            break;
        case HEXEN_MT_SORCBALL3:
            angle = baseangle + BALL3_ANGLEOFFSET;
            break;
        default:
            I_Error("corrupted sorcerer");
            return;
    }
    actor->angle = angle;
    angle >>= ANGLETOFINESHIFT;

    switch (mode)
    {
        case SORC_NORMAL:      // Balls rotating normally
            A_SorcUpdateBallAngle(actor);
            break;
        case SORC_DECELERATE:  // Balls decelerating
            A_DecelBalls(actor);
            A_SorcUpdateBallAngle(actor);
            break;
        case SORC_ACCELERATE:  // Balls accelerating
            A_AccelBalls(actor);
            A_SorcUpdateBallAngle(actor);
            break;
        case SORC_STOPPING:    // Balls stopping
            if ((parent->special2.i == actor->type) &&
                (parent->special_args[1] > SORCBALL_SPEED_ROTATIONS) &&
                (abs((int) angle - (int) (parent->angle >> ANGLETOFINESHIFT)) <
                 (30 << 5)))
            {
                // Can stop now
                actor->target->special_args[3] = SORC_FIRESPELL;
                actor->target->special_args[4] = 0;
                // Set angle so ball angle == sorcerer angle
                switch (actor->type)
                {
                    case HEXEN_MT_SORCBALL1:
                        parent->special1.i = (int) (parent->angle -
                                                    BALL1_ANGLEOFFSET);
                        break;
                    case HEXEN_MT_SORCBALL2:
                        parent->special1.i = (int) (parent->angle -
                                                    BALL2_ANGLEOFFSET);
                        break;
                    case HEXEN_MT_SORCBALL3:
                        parent->special1.i = (int) (parent->angle -
                                                    BALL3_ANGLEOFFSET);
                        break;
                    default:
                        break;
                }
            }
            else
            {
                A_SorcUpdateBallAngle(actor);
            }
            break;
        case SORC_FIRESPELL:   // Casting spell
            if (parent->special2.i == actor->type)
            {
                // Put sorcerer into special throw spell anim
                if (parent->health > 0)
                    P_SetMobjStateNF(parent, HEXEN_S_SORC_ATTACK1);

                if (actor->type == HEXEN_MT_SORCBALL1 && P_Random(pr_hexen) < 200)
                {
                    S_StartVoidSound(hexen_sfx_sorcerer_spellcast);
                    actor->special2.i = SORCFX4_RAPIDFIRE_TIME;
                    actor->special_args[4] = 128;
                    parent->special_args[3] = SORC_FIRING_SPELL;
                }
                else
                {
                    A_CastSorcererSpell(actor);
                    parent->special_args[3] = SORC_STOPPED;
                }
            }
            break;
        case SORC_FIRING_SPELL:
            if (parent->special2.i == actor->type)
            {
                if (actor->special2.i-- <= 0)
                {
                    // Done rapid firing
                    parent->special_args[3] = SORC_STOPPED;
                    // Back to orbit balls
                    if (parent->health > 0)
                        P_SetMobjStateNF(parent, HEXEN_S_SORC_ATTACK4);
                }
                else
                {
                    // Do rapid fire spell
                    A_SorcOffense2(actor);
                }
            }
            break;
        case SORC_STOPPED:     // Balls stopped
        default:
            break;
    }

    if ((angle < prevangle) && (parent->special_args[4] == SORCBALL_TERMINAL_SPEED))
    {
        parent->special_args[1]++;      // Bump rotation counter
        // Completed full rotation - make woosh sound
        S_StartMobjSound(actor, hexen_sfx_sorcerer_ballwoosh);
    }
    actor->special1.i = angle;    // Set previous angle
    x = parent->x + FixedMul(dist, finecosine[angle]);
    y = parent->y + FixedMul(dist, finesine[angle]);
    actor->x = x;
    actor->y = y;
    actor->z = parent->z - parent->floorclip + parent->info->height;
}

void A_SpeedBalls(mobj_t * actor)
{
    actor->special_args[3] = SORC_ACCELERATE;   // speed mode
    actor->special_args[2] = SORCBALL_TERMINAL_SPEED;   // target speed
}

void A_SlowBalls(mobj_t * actor)
{
    actor->special_args[3] = SORC_DECELERATE;   // slow mode
    actor->special_args[2] = SORCBALL_INITIAL_SPEED;    // target speed
}

void A_StopBalls(mobj_t * actor)
{
    int chance = P_Random(pr_hexen);
    actor->special_args[3] = SORC_STOPPING;     // stopping mode
    actor->special_args[1] = 0;         // Reset rotation counter

    if ((actor->special_args[0] <= 0) && (chance < 200))
    {
        actor->special2.i = HEXEN_MT_SORCBALL2; // Blue
    }
    else if ((actor->health < (P_MobjSpawnHealth(actor) >> 1)) &&
             (chance < 200))
    {
        actor->special2.i = HEXEN_MT_SORCBALL3; // Green
    }
    else
    {
        actor->special2.i = HEXEN_MT_SORCBALL1; // Yellow
    }
}

void A_AccelBalls(mobj_t * actor)
{
    mobj_t *sorc = actor->target;

    if (sorc->special_args[4] < sorc->special_args[2])
    {
        sorc->special_args[4]++;
    }
    else
    {
        sorc->special_args[3] = SORC_NORMAL;
        if (sorc->special_args[4] >= SORCBALL_TERMINAL_SPEED)
        {
            // Reached terminal velocity - stop balls
            A_StopBalls(sorc);
        }
    }
}

void A_DecelBalls(mobj_t * actor)
{
    mobj_t *sorc = actor->target;

    if (sorc->special_args[4] > sorc->special_args[2])
    {
        sorc->special_args[4]--;
    }
    else
    {
        sorc->special_args[3] = SORC_NORMAL;
    }
}

void A_SorcUpdateBallAngle(mobj_t * actor)
{
    if (actor->type == HEXEN_MT_SORCBALL1)
    {
        actor->target->special1.i += ANG1 * actor->target->special_args[4];
    }
}

void A_CastSorcererSpell(mobj_t * actor)
{
    mobj_t *mo;
    int spell = actor->type;
    angle_t ang1, ang2;
    fixed_t z;
    mobj_t *parent = actor->target;

    S_StartVoidSound(hexen_sfx_sorcerer_spellcast);

    // Put sorcerer into throw spell animation
    if (parent->health > 0)
        P_SetMobjStateNF(parent, HEXEN_S_SORC_ATTACK4);

    switch (spell)
    {
        case HEXEN_MT_SORCBALL1:     // Offensive
            A_SorcOffense1(actor);
            break;
        case HEXEN_MT_SORCBALL2:     // Defensive
            z = parent->z - parent->floorclip +
                SORC_DEFENSE_HEIGHT * FRACUNIT;
            mo = P_SpawnMobj(actor->x, actor->y, z, HEXEN_MT_SORCFX2);
            parent->flags2 |= MF2_REFLECTIVE | MF2_INVULNERABLE;
            parent->special_args[0] = SORC_DEFENSE_TIME;
            if (mo)
                P_SetTarget(&mo->target, parent);
            break;
        case HEXEN_MT_SORCBALL3:     // Reinforcements
            ang1 = actor->angle - ANG45;
            ang2 = actor->angle + ANG45;
            if (actor->health < (P_MobjSpawnHealth(actor) / 3))
            {                   // Spawn 2 at a time
                mo = P_SpawnMissileAngle(parent, HEXEN_MT_SORCFX3, ang1,
                                         4 * FRACUNIT);
                if (mo)
                    P_SetTarget(&mo->target, parent);
                mo = P_SpawnMissileAngle(parent, HEXEN_MT_SORCFX3, ang2,
                                         4 * FRACUNIT);
                if (mo)
                    P_SetTarget(&mo->target, parent);
            }
            else
            {
                if (P_Random(pr_hexen) < 128)
                    ang1 = ang2;
                mo = P_SpawnMissileAngle(parent, HEXEN_MT_SORCFX3, ang1,
                                         4 * FRACUNIT);
                if (mo)
                    P_SetTarget(&mo->target, parent);
            }
            break;
        default:
            break;
    }
}

void A_SorcOffense1(mobj_t * actor)
{
    mobj_t *mo;
    angle_t ang1, ang2;
    mobj_t *parent = (mobj_t *) actor->target;

    ang1 = actor->angle + ANG1 * 70;
    ang2 = actor->angle - ANG1 * 70;
    mo = P_SpawnMissileAngle(parent, HEXEN_MT_SORCFX1, ang1, 0);
    if (mo)
    {
        P_SetTarget(&mo->target, parent);
        P_SetTarget(&mo->special1.m, parent->target);
        mo->special_args[4] = BOUNCE_TIME_UNIT;
        mo->special_args[3] = 15;       // Bounce time in seconds
    }
    mo = P_SpawnMissileAngle(parent, HEXEN_MT_SORCFX1, ang2, 0);
    if (mo)
    {
        P_SetTarget(&mo->target, parent);
        P_SetTarget(&mo->special1.m, parent->target);
        mo->special_args[4] = BOUNCE_TIME_UNIT;
        mo->special_args[3] = 15;       // Bounce time in seconds
    }
}

void A_SorcOffense2(mobj_t * actor)
{
    angle_t ang1;
    mobj_t *mo;
    int delta, index;
    mobj_t *parent = actor->target;
    mobj_t *dest = parent->target;
    int dist;

    index = actor->special_args[4] << 5;
    actor->special_args[4] += 15;
    actor->special_args[4] &= 0xff;
    delta = (finesine[index]) * SORCFX4_SPREAD_ANGLE;
    delta = (delta >> FRACBITS) * ANG1;
    ang1 = actor->angle + delta;
    mo = P_SpawnMissileAngle(parent, HEXEN_MT_SORCFX4, ang1, 0);
    if (mo)
    {
        mo->special2.i = 35 * 5 / 2;      // 5 seconds
        dist = P_AproxDistance(dest->x - mo->x, dest->y - mo->y);
        dist = dist / mo->info->speed;
        if (dist < 1)
            dist = 1;
        mo->momz = (dest->z - mo->z) / dist;
    }
}

void A_SorcBossAttack(mobj_t * actor)
{
    actor->special_args[3] = SORC_ACCELERATE;
    actor->special_args[2] = SORCBALL_INITIAL_SPEED;
}

void A_SpawnFizzle(mobj_t * actor)
{
    fixed_t x, y, z;
    fixed_t dist = 5 * FRACUNIT;
    angle_t angle = actor->angle >> ANGLETOFINESHIFT;
    fixed_t speed = actor->info->speed;
    angle_t rangle;
    mobj_t *mo;
    int ix;

    x = actor->x + FixedMul(dist, finecosine[angle]);
    y = actor->y + FixedMul(dist, finesine[angle]);
    z = actor->z - actor->floorclip + (actor->height >> 1);
    for (ix = 0; ix < 5; ix++)
    {
        mo = P_SpawnMobj(x, y, z, HEXEN_MT_SORCSPARK1);
        if (mo)
        {
            rangle = angle + ((P_Random(pr_hexen) % 5) << 1);
            mo->momx = FixedMul(P_Random(pr_hexen) % speed, finecosine[rangle]);
            mo->momy = FixedMul(P_Random(pr_hexen) % speed, finesine[rangle]);
            mo->momz = FRACUNIT * 2;
        }
    }
}

//============================================================================
// Yellow spell - offense
//============================================================================

void A_SorcFX1Seek(mobj_t * actor)
{
    A_BounceCheck(actor);
    P_SeekerMissile(actor, &actor->special1.m, ANG1 * 2, ANG1 * 6, false);
}

//============================================================================
// Blue spell - defense
//============================================================================
//
// FX2 Variables
//              special1                current angle
//              special2
//              args[0]         0 = CW,  1 = CCW
//              args[1]
//============================================================================

void A_SorcFX2Split(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_SORCFX2);
    if (mo)
    {
        P_SetTarget(&mo->target, actor->target);
        mo->special_args[0] = 0;        // CW
        mo->special1.i = actor->angle;    // Set angle
        P_SetMobjStateNF(mo, HEXEN_S_SORCFX2_ORBIT1);
    }
    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_SORCFX2);
    if (mo)
    {
        P_SetTarget(&mo->target, actor->target);
        mo->special_args[0] = 1;        // CCW
        mo->special1.i = actor->angle;    // Set angle
        P_SetMobjStateNF(mo, HEXEN_S_SORCFX2_ORBIT1);
    }
    P_SetMobjStateNF(actor, HEXEN_S_NULL);
}

void A_SorcFX2Orbit(mobj_t * actor)
{
    angle_t angle;
    fixed_t x, y, z;
    mobj_t *parent = actor->target;
    fixed_t dist = parent->info->radius;

    if ((parent->health <= 0) ||        // Sorcerer is dead
        (!parent->special_args[0]))     // Time expired
    {
        P_SetMobjStateNF(actor, actor->info->deathstate);
        parent->special_args[0] = 0;
        parent->flags2 &= ~MF2_REFLECTIVE;
        parent->flags2 &= ~MF2_INVULNERABLE;
    }

    if (actor->special_args[0] && (parent->special_args[0]-- <= 0))     // Time expired
    {
        P_SetMobjStateNF(actor, actor->info->deathstate);
        parent->special_args[0] = 0;
        parent->flags2 &= ~MF2_REFLECTIVE;
    }

    // Move to new position based on angle
    if (actor->special_args[0])         // Counter clock-wise
    {
        actor->special1.i += ANG1 * 10;
        angle = ((angle_t) actor->special1.i) >> ANGLETOFINESHIFT;
        x = parent->x + FixedMul(dist, finecosine[angle]);
        y = parent->y + FixedMul(dist, finesine[angle]);
        z = parent->z - parent->floorclip + SORC_DEFENSE_HEIGHT * FRACUNIT;
        z += FixedMul(15 * FRACUNIT, finecosine[angle]);
        // Spawn trailer
        P_SpawnMobj(x, y, z, HEXEN_MT_SORCFX2_T1);
    }
    else                        // Clock wise
    {
        actor->special1.i -= ANG1 * 10;
        angle = ((angle_t) actor->special1.i) >> ANGLETOFINESHIFT;
        x = parent->x + FixedMul(dist, finecosine[angle]);
        y = parent->y + FixedMul(dist, finesine[angle]);
        z = parent->z - parent->floorclip + SORC_DEFENSE_HEIGHT * FRACUNIT;
        z += FixedMul(20 * FRACUNIT, finesine[angle]);
        // Spawn trailer
        P_SpawnMobj(x, y, z, HEXEN_MT_SORCFX2_T1);
    }

    actor->x = x;
    actor->y = y;
    actor->z = z;
}

//============================================================================
// Green spell - spawn bishops
//============================================================================

void A_SpawnBishop(mobj_t * actor)
{
    mobj_t *mo;
    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_BISHOP);
    if (mo)
    {
        if (!P_TestMobjLocation(mo))
        {
            P_SetMobjState(mo, HEXEN_S_NULL);
        }
    }
    P_SetMobjState(actor, HEXEN_S_NULL);
}

void A_SmokePuffExit(mobj_t * actor)
{
    P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_MNTRSMOKEEXIT);
}

void A_SorcererBishopEntry(mobj_t * actor)
{
    P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_SORCFX3_EXPLOSION);
    S_StartMobjSound(actor, actor->info->seesound);
}

void A_SorcFX4Check(mobj_t * actor)
{
    if (actor->special2.i-- <= 0)
    {
        P_SetMobjStateNF(actor, actor->info->deathstate);
    }
}

void A_SorcBallPop(mobj_t * actor)
{
    S_StartVoidSound(hexen_sfx_sorcerer_ballpop);
    actor->flags &= ~MF_NOGRAVITY;
    actor->flags2 |= MF2_LOGRAV;
    actor->momx = ((P_Random(pr_hexen) % 10) - 5) << FRACBITS;
    actor->momy = ((P_Random(pr_hexen) % 10) - 5) << FRACBITS;
    actor->momz = (2 + (P_Random(pr_hexen) % 3)) << FRACBITS;
    actor->special2.i = 4 * FRACUNIT;     // Initial bounce factor
    actor->special_args[4] = BOUNCE_TIME_UNIT;  // Bounce time unit
    actor->special_args[3] = 5;         // Bounce time in seconds
}

void A_BounceCheck(mobj_t * actor)
{
    if (actor->special_args[4]-- <= 0)
    {
        if (actor->special_args[3]-- <= 0)
        {
            P_SetMobjState(actor, actor->info->deathstate);
            switch (actor->type)
            {
                case HEXEN_MT_SORCBALL1:
                case HEXEN_MT_SORCBALL2:
                case HEXEN_MT_SORCBALL3:
                    S_StartVoidSound(hexen_sfx_sorcerer_bigballexplode);
                    break;
                case HEXEN_MT_SORCFX1:
                    S_StartVoidSound(hexen_sfx_sorcerer_headscream);
                    break;
                default:
                    break;
            }
        }
        else
        {
            actor->special_args[4] = BOUNCE_TIME_UNIT;
        }
    }
}

#define CLASS_BOSS_STRAFE_RANGE	64*10*FRACUNIT

void A_FastChase(mobj_t * actor)
{
    int delta;
    fixed_t dist;
    angle_t ang;
    mobj_t *target;

    if (actor->reactiontime)
    {
        actor->reactiontime--;
    }

    // Modify target threshold
    if (actor->threshold)
    {
        actor->threshold--;
    }

    if (skill_info.flags & SI_FAST_MONSTERS)
    {                           // Monsters move faster in nightmare mode
        actor->tics -= actor->tics / 2;
        if (actor->tics < 3)
        {
            actor->tics = 3;
        }
    }

    //
    // turn towards movement direction if not there yet
    //
    if (actor->movedir < 8)
    {
        actor->angle &= (7 << 29);
        delta = actor->angle - (actor->movedir << 29);
        if (delta > 0)
        {
            actor->angle -= ANG90 / 2;
        }
        else if (delta < 0)
        {
            actor->angle += ANG90 / 2;
        }
    }

    if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
    {                           // look for a new target
        if (P_LookForPlayers(actor, true))
        {                       // got a new target
            return;
        }
        P_SetMobjState(actor, actor->info->spawnstate);
        return;
    }

    //
    // don't attack twice in a row
    //
    if (actor->flags & MF_JUSTATTACKED)
    {
        actor->flags &= ~MF_JUSTATTACKED;
        if (!(skill_info.flags & SI_FAST_MONSTERS))
            P_NewChaseDir(actor);
        return;
    }

    // Strafe
    if (actor->special2.i > 0)
    {
        actor->special2.i--;
    }
    else
    {
        target = actor->target;
        actor->special2.i = 0;
        actor->momx = actor->momy = 0;
        dist = P_AproxDistance(actor->x - target->x, actor->y - target->y);
        if (dist < CLASS_BOSS_STRAFE_RANGE)
        {
            if (P_Random(pr_hexen) < 100)
            {
                ang = R_PointToAngle2(actor->x, actor->y,
                                      target->x, target->y);
                if (P_Random(pr_hexen) < 128)
                    ang += ANG90;
                else
                    ang -= ANG90;
                ang >>= ANGLETOFINESHIFT;
                actor->momx = FixedMul(13 * FRACUNIT, finecosine[ang]);
                actor->momy = FixedMul(13 * FRACUNIT, finesine[ang]);
                actor->special2.i = 3;    // strafe time
            }
        }
    }

    //
    // check for missile attack
    //
    if (actor->info->missilestate)
    {
        if (!(skill_info.flags & SI_FAST_MONSTERS) && actor->movecount)
            goto nomissile;
        if (!P_CheckMissileRange(actor))
            goto nomissile;
        P_SetMobjState(actor, actor->info->missilestate);
        actor->flags |= MF_JUSTATTACKED;
        return;
    }
  nomissile:

    //
    // possibly choose another target
    //
    if (netgame && !actor->threshold && !P_CheckSight(actor, actor->target))
    {
        if (P_LookForPlayers(actor, true))
            return;             // got a new target
    }

    //
    // chase towards player
    //
    if (!actor->special2.i)
    {
        if (--actor->movecount < 0 || !P_Move(actor, false))
        {
            P_NewChaseDir(actor);
        }
    }
}

void A_FighterAttack(mobj_t * actor)
{
    extern void A_FSwordAttack2(mobj_t * actor);

    if (!actor->target)
        return;
    A_FSwordAttack2(actor);
}

void A_ClericAttack(mobj_t * actor)
{
    extern void A_CHolyAttack3(mobj_t * actor);

    if (!actor->target)
        return;
    A_CHolyAttack3(actor);
}

void A_MageAttack(mobj_t * actor)
{
    extern void A_MStaffAttack2(mobj_t * actor);

    if (!actor->target)
        return;
    A_MStaffAttack2(actor);
}

void A_ClassBossHealth(mobj_t * actor)
{
    if (netgame && !deathmatch) // co-op only
    {
        if (!actor->special1.i)
        {
            actor->health *= 5;
            actor->special1.i = true;     // has been initialized
        }
    }
}

void A_CheckFloor(mobj_t * actor)
{
    if (actor->z <= actor->floorz)
    {
        actor->z = actor->floorz;
        actor->flags2 &= ~MF2_LOGRAV;
        P_SetMobjState(actor, actor->info->deathstate);
    }
}

//===========================================================================
// Korax Variables
//      special1        last teleport destination
//      special2        set if "below half" script not yet run
//
// Korax Scripts (reserved)
//      249             Tell scripts that we are below half health
//      250-254         Control scripts
//      255             Death script
//
// Korax TIDs (reserved)
//      245             Reserved for Korax himself
//      248             Initial teleport destination
//      249             Teleport destination
//      250-254         For use in respective control scripts
//      255             For use in death script (spawn spots)
//===========================================================================
#define KORAX_SPIRIT_LIFETIME	(5*(35/5))      // 5 seconds
#define KORAX_COMMAND_HEIGHT	(120*FRACUNIT)
#define KORAX_COMMAND_OFFSET	(27*FRACUNIT)

void KoraxFire1(mobj_t * actor, int type);
void KoraxFire2(mobj_t * actor, int type);
void KoraxFire3(mobj_t * actor, int type);
void KoraxFire4(mobj_t * actor, int type);
void KoraxFire5(mobj_t * actor, int type);
void KoraxFire6(mobj_t * actor, int type);
void KSpiritInit(mobj_t * spirit, mobj_t * korax);

#define KORAX_TID					(245)
#define KORAX_FIRST_TELEPORT_TID	(248)
#define KORAX_TELEPORT_TID			(249)

void A_KoraxChase(mobj_t * actor)
{
    mobj_t *spot;
    int lastfound;
    byte args[3] = {0, 0, 0};

    if ((!actor->special2.i) &&
        (actor->health <= (P_MobjSpawnHealth(actor) / 2)))
    {
        lastfound = 0;
        spot = P_FindMobjFromTID(KORAX_FIRST_TELEPORT_TID, &lastfound);
        if (spot)
        {
            P_Teleport(actor, spot->x, spot->y, spot->angle, true);
        }

        CheckACSPresent(249);
        P_StartACS(249, 0, args, actor, NULL, 0);
        actor->special2.i = 1;    // Don't run again

        return;
    }

    if (!actor->target)
        return;
    if (P_Random(pr_hexen) < 30)
    {
        P_SetMobjState(actor, actor->info->missilestate);
    }
    else if (P_Random(pr_hexen) < 30)
    {
        S_StartVoidSound(hexen_sfx_korax_active);
    }

    // Teleport away
    if (actor->health < (P_MobjSpawnHealth(actor) >> 1))
    {
        if (P_Random(pr_hexen) < 10)
        {
            lastfound = actor->special1.i;
            spot = P_FindMobjFromTID(KORAX_TELEPORT_TID, &lastfound);
            actor->special1.i = lastfound;
            if (spot)
            {
                P_Teleport(actor, spot->x, spot->y, spot->angle, true);
            }
        }
    }
}

void A_KoraxStep(mobj_t * actor)
{
    A_Chase(actor);
}

void A_KoraxStep2(mobj_t * actor)
{
    S_StartVoidSound(hexen_sfx_korax_step);
    A_Chase(actor);
}

void A_KoraxBonePop(mobj_t * actor)
{
    mobj_t *mo;
    byte args[5];

    args[0] = args[1] = args[2] = args[3] = args[4] = 0;

    // Spawn 6 spirits equalangularly
    mo = P_SpawnMissileAngle(actor, HEXEN_MT_KORAX_SPIRIT1, ANG60 * 0,
                             5 * FRACUNIT);
    if (mo)
        KSpiritInit(mo, actor);
    mo = P_SpawnMissileAngle(actor, HEXEN_MT_KORAX_SPIRIT2, ANG60 * 1,
                             5 * FRACUNIT);
    if (mo)
        KSpiritInit(mo, actor);
    mo = P_SpawnMissileAngle(actor, HEXEN_MT_KORAX_SPIRIT3, ANG60 * 2,
                             5 * FRACUNIT);
    if (mo)
        KSpiritInit(mo, actor);
    mo = P_SpawnMissileAngle(actor, HEXEN_MT_KORAX_SPIRIT4, ANG60 * 3,
                             5 * FRACUNIT);
    if (mo)
        KSpiritInit(mo, actor);
    mo = P_SpawnMissileAngle(actor, HEXEN_MT_KORAX_SPIRIT5, ANG60 * 4,
                             5 * FRACUNIT);
    if (mo)
        KSpiritInit(mo, actor);
    mo = P_SpawnMissileAngle(actor, HEXEN_MT_KORAX_SPIRIT6, ANG60 * 5,
                             5 * FRACUNIT);
    if (mo)
        KSpiritInit(mo, actor);

    CheckACSPresent(255);
    P_StartACS(255, 0, args, actor, NULL, 0);   // Death script
}

void KSpiritInit(mobj_t * spirit, mobj_t * korax)
{
    int i;
    mobj_t *tail, *next;

    spirit->health = KORAX_SPIRIT_LIFETIME;

    P_SetTarget(&spirit->special1.m, korax);     // Swarm around korax
    spirit->special2.i = 32 + (P_Random(pr_hexen) & 7);   // Float bob index
    spirit->special_args[0] = 10;       // initial turn value
    spirit->special_args[1] = 0;        // initial look angle

    // Spawn a tail for spirit
    tail = P_SpawnMobj(spirit->x, spirit->y, spirit->z, HEXEN_MT_HOLY_TAIL);
    P_SetTarget(&tail->special2.m, spirit);      // parent
    for (i = 1; i < 3; i++)
    {
        next = P_SpawnMobj(spirit->x, spirit->y, spirit->z, HEXEN_MT_HOLY_TAIL);
        P_SetMobjState(next, next->info->spawnstate + 1);
        P_SetTarget(&tail->special1.m, next);
        tail = next;
    }
    P_SetTarget(&tail->special1.m, NULL);         // last tail bit
}

void A_KoraxDecide(mobj_t * actor)
{
    if (P_Random(pr_hexen) < 220)
    {
        P_SetMobjState(actor, HEXEN_S_KORAX_MISSILE1);
    }
    else
    {
        P_SetMobjState(actor, HEXEN_S_KORAX_COMMAND1);
    }
}

void A_KoraxMissile(mobj_t * actor)
{
    int type = P_Random(pr_hexen) % 6;
    int sound = 0;

    S_StartMobjSound(actor, hexen_sfx_korax_attack);

    switch (type)
    {
        case 0:
            type = HEXEN_MT_WRAITHFX1;
            sound = hexen_sfx_wraith_missile_fire;
            break;
        case 1:
            type = HEXEN_MT_DEMONFX1;
            sound = hexen_sfx_demon_missile_fire;
            break;
        case 2:
            type = HEXEN_MT_DEMON2FX1;
            sound = hexen_sfx_demon_missile_fire;
            break;
        case 3:
            type = HEXEN_MT_FIREDEMON_FX6;
            sound = hexen_sfx_fired_attack;
            break;
        case 4:
            type = HEXEN_MT_CENTAUR_FX;
            sound = hexen_sfx_centaurleader_attack;
            break;
        case 5:
            type = HEXEN_MT_SERPENTFX;
            sound = hexen_sfx_centaurleader_attack;
            break;
    }

    // Fire all 6 missiles at once
    S_StartVoidSound(sound);
    KoraxFire1(actor, type);
    KoraxFire2(actor, type);
    KoraxFire3(actor, type);
    KoraxFire4(actor, type);
    KoraxFire5(actor, type);
    KoraxFire6(actor, type);
}

void A_KoraxCommand(mobj_t * actor)
{
    byte args[5];
    fixed_t x, y, z;
    angle_t ang;
    int numcommands;

    S_StartMobjSound(actor, hexen_sfx_korax_command);

    // Shoot stream of lightning to ceiling
    ang = (actor->angle - ANG90) >> ANGLETOFINESHIFT;
    x = actor->x + FixedMul(KORAX_COMMAND_OFFSET, finecosine[ang]);
    y = actor->y + FixedMul(KORAX_COMMAND_OFFSET, finesine[ang]);
    z = actor->z + KORAX_COMMAND_HEIGHT;
    P_SpawnMobj(x, y, z, HEXEN_MT_KORAX_BOLT);

    args[0] = args[1] = args[2] = args[3] = args[4] = 0;

    if (actor->health <= (P_MobjSpawnHealth(actor) >> 1))
    {
        numcommands = 5;
    }
    else
    {
        numcommands = 4;
    }

    switch (P_Random(pr_hexen) % numcommands)
    {
        case 0:
            CheckACSPresent(250);
            P_StartACS(250, 0, args, actor, NULL, 0);
            break;
        case 1:
            CheckACSPresent(251);
            P_StartACS(251, 0, args, actor, NULL, 0);
            break;
        case 2:
            CheckACSPresent(252);
            P_StartACS(252, 0, args, actor, NULL, 0);
            break;
        case 3:
            CheckACSPresent(253);
            P_StartACS(253, 0, args, actor, NULL, 0);
            break;
        case 4:
            CheckACSPresent(254);
            P_StartACS(254, 0, args, actor, NULL, 0);
            break;
    }
}

#define KORAX_DELTAANGLE			(85*ANG1)
#define KORAX_ARM_EXTENSION_SHORT	(40*FRACUNIT)
#define KORAX_ARM_EXTENSION_LONG	(55*FRACUNIT)

#define KORAX_ARM1_HEIGHT			(108*FRACUNIT)
#define KORAX_ARM2_HEIGHT			(82*FRACUNIT)
#define KORAX_ARM3_HEIGHT			(54*FRACUNIT)
#define KORAX_ARM4_HEIGHT			(104*FRACUNIT)
#define KORAX_ARM5_HEIGHT			(86*FRACUNIT)
#define KORAX_ARM6_HEIGHT			(53*FRACUNIT)

// Arm projectiles
//              arm positions numbered:
//                      1       top left
//                      2       middle left
//                      3       lower left
//                      4       top right
//                      5       middle right
//                      6       lower right

void KoraxFire1(mobj_t * actor, int type)
{
    angle_t ang;
    fixed_t x, y, z;

    ang = (actor->angle - KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
    x = actor->x + FixedMul(KORAX_ARM_EXTENSION_SHORT, finecosine[ang]);
    y = actor->y + FixedMul(KORAX_ARM_EXTENSION_SHORT, finesine[ang]);
    z = actor->z - actor->floorclip + KORAX_ARM1_HEIGHT;
    P_SpawnKoraxMissile(x, y, z, actor, actor->target, type);
}

void KoraxFire2(mobj_t * actor, int type)
{
    angle_t ang;
    fixed_t x, y, z;

    ang = (actor->angle - KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
    x = actor->x + FixedMul(KORAX_ARM_EXTENSION_LONG, finecosine[ang]);
    y = actor->y + FixedMul(KORAX_ARM_EXTENSION_LONG, finesine[ang]);
    z = actor->z - actor->floorclip + KORAX_ARM2_HEIGHT;
    P_SpawnKoraxMissile(x, y, z, actor, actor->target, type);
}

void KoraxFire3(mobj_t * actor, int type)
{
    angle_t ang;
    fixed_t x, y, z;

    ang = (actor->angle - KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
    x = actor->x + FixedMul(KORAX_ARM_EXTENSION_LONG, finecosine[ang]);
    y = actor->y + FixedMul(KORAX_ARM_EXTENSION_LONG, finesine[ang]);
    z = actor->z - actor->floorclip + KORAX_ARM3_HEIGHT;
    P_SpawnKoraxMissile(x, y, z, actor, actor->target, type);
}

void KoraxFire4(mobj_t * actor, int type)
{
    angle_t ang;
    fixed_t x, y, z;

    ang = (actor->angle + KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
    x = actor->x + FixedMul(KORAX_ARM_EXTENSION_SHORT, finecosine[ang]);
    y = actor->y + FixedMul(KORAX_ARM_EXTENSION_SHORT, finesine[ang]);
    z = actor->z - actor->floorclip + KORAX_ARM4_HEIGHT;
    P_SpawnKoraxMissile(x, y, z, actor, actor->target, type);
}

void KoraxFire5(mobj_t * actor, int type)
{
    angle_t ang;
    fixed_t x, y, z;

    ang = (actor->angle + KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
    x = actor->x + FixedMul(KORAX_ARM_EXTENSION_LONG, finecosine[ang]);
    y = actor->y + FixedMul(KORAX_ARM_EXTENSION_LONG, finesine[ang]);
    z = actor->z - actor->floorclip + KORAX_ARM5_HEIGHT;
    P_SpawnKoraxMissile(x, y, z, actor, actor->target, type);
}

void KoraxFire6(mobj_t * actor, int type)
{
    angle_t ang;
    fixed_t x, y, z;

    ang = (actor->angle + KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
    x = actor->x + FixedMul(KORAX_ARM_EXTENSION_LONG, finecosine[ang]);
    y = actor->y + FixedMul(KORAX_ARM_EXTENSION_LONG, finesine[ang]);
    z = actor->z - actor->floorclip + KORAX_ARM6_HEIGHT;
    P_SpawnKoraxMissile(x, y, z, actor, actor->target, type);
}

void A_KSpiritWeave(mobj_t * actor)
{
    fixed_t newX, newY;
    int weaveXY, weaveZ;
    int angle;

    weaveXY = actor->special2.i >> 16;
    weaveZ = actor->special2.i & 0xFFFF;
    angle = (actor->angle + ANG90) >> ANGLETOFINESHIFT;
    newX = actor->x - FixedMul(finecosine[angle],
                               FloatBobOffsets[weaveXY] << 2);
    newY = actor->y - FixedMul(finesine[angle],
                               FloatBobOffsets[weaveXY] << 2);
    weaveXY = (weaveXY + (P_Random(pr_hexen) % 5)) & 63;
    newX += FixedMul(finecosine[angle], FloatBobOffsets[weaveXY] << 2);
    newY += FixedMul(finesine[angle], FloatBobOffsets[weaveXY] << 2);
    P_TryMove(actor, newX, newY, false);
    actor->z -= FloatBobOffsets[weaveZ] << 1;
    weaveZ = (weaveZ + (P_Random(pr_hexen) % 5)) & 63;
    actor->z += FloatBobOffsets[weaveZ] << 1;
    actor->special2.i = weaveZ + (weaveXY << 16);
}

void A_KSpiritSeeker(mobj_t * actor, angle_t thresh, angle_t turnMax)
{
    int dir;
    int dist;
    angle_t delta;
    angle_t angle;
    mobj_t *target;
    fixed_t newZ;
    fixed_t deltaZ;

    target = actor->special1.m;
    if (target == NULL)
    {
        return;
    }
    dir = P_FaceMobj(actor, target, &delta);
    if (delta > thresh)
    {
        delta >>= 1;
        if (delta > turnMax)
        {
            delta = turnMax;
        }
    }
    if (dir)
    {                           // Turn clockwise
        actor->angle += delta;
    }
    else
    {                           // Turn counter clockwise
        actor->angle -= delta;
    }
    angle = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul(actor->info->speed, finecosine[angle]);
    actor->momy = FixedMul(actor->info->speed, finesine[angle]);

    if (!(leveltime & 15)
        || actor->z > target->z + (target->info->height)
        || actor->z + actor->height < target->z)
    {
        newZ = target->z + ((P_Random(pr_hexen) * target->info->height) >> 8);
        deltaZ = newZ - actor->z;
        if (abs(deltaZ) > 15 * FRACUNIT)
        {
            if (deltaZ > 0)
            {
                deltaZ = 15 * FRACUNIT;
            }
            else
            {
                deltaZ = -15 * FRACUNIT;
            }
        }
        dist = P_AproxDistance(target->x - actor->x, target->y - actor->y);
        dist = dist / actor->info->speed;
        if (dist < 1)
        {
            dist = 1;
        }
        actor->momz = deltaZ / dist;
    }
    return;
}

void A_KSpiritRoam(mobj_t * actor)
{
    if (actor->health-- <= 0)
    {
        S_StartMobjSound(actor, hexen_sfx_spirit_die);
        P_SetMobjState(actor, HEXEN_S_KSPIRIT_DEATH1);
    }
    else
    {
        if (actor->special1.m)
        {
            A_KSpiritSeeker(actor, actor->special_args[0] * ANG1,
                            actor->special_args[0] * ANG1 * 2);
        }
        A_KSpiritWeave(actor);
        if (P_Random(pr_hexen) < 50)
        {
            S_StartVoidSound(hexen_sfx_spirit_active);
        }
    }
}

void A_KBolt(mobj_t * actor)
{
    // Countdown lifetime
    if (actor->special1.i-- <= 0)
    {
        P_SetMobjState(actor, HEXEN_S_NULL);
    }
}

#define KORAX_BOLT_HEIGHT		48*FRACUNIT
#define KORAX_BOLT_LIFETIME		3

void A_KBoltRaise(mobj_t * actor)
{
    mobj_t *mo;
    fixed_t z;

    // Spawn a child upward
    z = actor->z + KORAX_BOLT_HEIGHT;

    if ((z + KORAX_BOLT_HEIGHT) < actor->ceilingz)
    {
        mo = P_SpawnMobj(actor->x, actor->y, z, HEXEN_MT_KORAX_BOLT);
        if (mo)
        {
            mo->special1.i = KORAX_BOLT_LIFETIME;
        }
    }
    else
    {
        // Maybe cap it off here
    }
}
