/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2004 by
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
 *      Moving object handling. Spawn functions.
 *
 *-----------------------------------------------------------------------------*/

#include "doomdef.h"
#include "doomstat.h"
#include "m_random.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_tick.h"
#include "sounds.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "s_sound.h"
#include "s_advsound.h"
#include "info.h"
#include "g_game.h"
#include "p_inter.h"
#include "p_user.h"
#include "lprintf.h"
#include "smooth.h"
#include "p_enemy.h"
#include "p_spec.h"
#include "g_overflow.h"
#include "e6y.h"//e6y

#include "dsda.h"
#include "dsda/map_format.h"
#include "dsda/mapinfo.h"
#include "dsda/settings.h"
#include "dsda/skill_info.h"
#include "dsda/spawn_number.h"
#include "dsda/thing_id.h"
#include "dsda/tranmap.h"
#include "dsda/utility.h"

#include "heretic/def.h"
#include "heretic/sb_bar.h"

#include "hexen/po_man.h"

// heretic_note: static NUMSTATES arrays here - probably fine?
// NUMSTATES > HERETIC_NUMSTATES

//
// P_SetMobjState
// Returns true if the mobj is still present.
//

dboolean P_SetMobjState(mobj_t* mobj,statenum_t state)
{
  state_t*  st;

  // killough 4/9/98: remember states seen, to detect cycles:

  extern statenum_t* seenstate_tab;           // fast transition table
  statenum_t *seenstate;                      // pointer to table
  static int recursion;                       // detects recursion
  statenum_t i;                               // initial state
  dboolean ret;                               // return value
  statenum_t* tempstate = NULL;               // for use with recursion

  if (raven) return Raven_P_SetMobjState(mobj, state);

  seenstate = seenstate_tab;
  i = state;
  ret = true;

  if (recursion++)                            // if recursion detected,
    seenstate = tempstate = Z_Calloc(num_states, sizeof(statenum_t)); // allocate state table

  do
    {
    if (state == g_s_null)
      {
      mobj->state = NULL;
      P_RemoveMobj (mobj);
      ret = false;
      break;                 // killough 4/9/98
      }

    st = &states[state];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    // Modified handling.
    // Call action functions when the state is set

    if (st->action)
      st->action(mobj);

    seenstate[state] = 1 + st->nextstate;   // killough 4/9/98

    state = st->nextstate;
    } while (!mobj->tics && !seenstate[state]);   // killough 4/9/98

  if (ret && !mobj->tics)  // killough 4/9/98: detect state cycles
    doom_printf("Warning: State Cycle Detected");

  if (!--recursion)
    for (;(state=seenstate[i]);i=state-1)
      seenstate[i] = 0;  // killough 4/9/98: erase memory of states

  if (tempstate)
    Z_Free(tempstate);

  return ret;
}


//
// P_ExplodeMissile
//

void P_ExplodeMissile (mobj_t* mo)
{
  if (heretic && mo->type == HERETIC_MT_WHIRLWIND)
  {
    if (++mo->special2.i < 60)
    {
      return;
    }
  }

  mo->momx = mo->momy = mo->momz = 0;

  P_SetMobjState (mo, mobjinfo[mo->type].deathstate);

  if (!raven)
  {
    mo->tics -= P_Random(pr_explode) & 3;

    if (mo->tics < 1)
      mo->tics = 1;
  }

  mo->flags &= ~MF_MISSILE;

  if (!hexen)
  {
    if (mo->info->deathsound)
      S_StartMobjSound(mo, mo->info->deathsound);
  }
  else
  {
    switch (mo->type)
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
        if (mo->info->deathsound)
          S_StartMobjSound(mo, mo->info->deathsound);
        break;
    }
  }
}


//
// P_XYMovement
//
// Attempts to move something if it has momentum.
//

void P_ApplyCompatibleSectorMovementSpecial(mobj_t *mo, int special)
{
  // nothing in doom
}

// heretic, hexen, and zdoom all use the same values
void P_ApplyHereticSectorMovementSpecial(mobj_t *mo, int special)
{
  static int windTab[3] = { 2048 * 5, 2048 * 10, 2048 * 25 };

  if (mo->flags2 & MF2_WINDTHRUST)
  {
    switch (special)
    {
      case zs_wind_east_weak:
      case zs_wind_east_medium:
      case zs_wind_east_strong:
        P_ThrustMobj(mo, 0, windTab[special - 40]);
        break;
      case zs_wind_north_weak:
      case zs_wind_north_medium:
      case zs_wind_north_strong:
        P_ThrustMobj(mo, ANG90, windTab[special - 43]);
        break;
      case zs_wind_south_weak:
      case zs_wind_south_medium:
      case zs_wind_south_strong:
        P_ThrustMobj(mo, ANG270, windTab[special - 46]);
        break;
      case zs_wind_west_weak:
      case zs_wind_west_medium:
      case zs_wind_west_strong:
        P_ThrustMobj(mo, ANG180, windTab[special - 49]);
        break;
    }
  }
}

static void P_XYMovement (mobj_t* mo)
{
  player_t *player;
  fixed_t xmove, ymove;

  //e6y
  fixed_t   oldx,oldy; // phares 9/10/98: reducing bobbing/momentum on ice

  // heretic
  int special;

  if (!(mo->momx | mo->momy)) // Any momentum?
  {
    if (mo->flags & MF_SKULLFLY)
    {
      statenum_t new_state;
      // the skull slammed into something

      mo->flags &= ~MF_SKULLFLY;
      mo->momz = 0;

      if (raven)
        new_state = mo->info->seestate;
      else
        new_state = mo->info->spawnstate;

      P_SetMobjState (mo, new_state);
    }
    return;
  }

  special = mo->subsector->sector->special;
  map_format.apply_sector_movement_special(mo, special);

  player = mo->player;

  if (mo->momx > MAXMOVE)
    mo->momx = MAXMOVE;
  else if (mo->momx < -MAXMOVE)
    mo->momx = -MAXMOVE;

  if (mo->momy > MAXMOVE)
    mo->momy = MAXMOVE;
  else if (mo->momy < -MAXMOVE)
    mo->momy = -MAXMOVE;

  xmove = mo->momx;
  ymove = mo->momy;

  oldx = mo->x; // phares 9/10/98: new code to reduce bobbing/momentum
  oldy = mo->y; // when on ice & up against wall. These will be compared
                // to your x,y values later to see if you were able to move

  do
  {
    fixed_t ptryx, ptryy;
    angle_t angle;

    // killough 8/9/98: fix bug in original Doom source:
    // Large negative displacements were never considered.
    // This explains the tendency for Mancubus fireballs
    // to pass through walls.
    // CPhipps - compatibility optioned

    if (xmove > MAXMOVE / 2 ||
        ymove > MAXMOVE / 2 ||
        (!comp[comp_moveblock] && (xmove < -MAXMOVE/2 || ymove < -MAXMOVE/2)))
    {
      ptryx = mo->x + xmove / 2;
      ptryy = mo->y + ymove / 2;
      xmove >>= 1;
      ymove >>= 1;
    }
    else
    {
      ptryx = mo->x + xmove;
      ptryy = mo->y + ymove;
      xmove = ymove = 0;
    }

    // killough 3/15/98: Allow objects to drop off

    if (!P_TryMove (mo, ptryx, ptryy, true))
    {
      // blocked move

      // killough 8/11/98: bouncing off walls
      // killough 10/98:
      // Add ability for objects other than players to bounce on ice

      if (
        !(mo->flags & MF_MISSILE) &&
        mbf_features &&
        (
          mo->flags & MF_BOUNCES ||
          (
            !player &&
            blockline &&
            variable_friction &&
            mo->z <= mo->floorz &&
            P_GetFriction(mo, NULL) > ORIG_FRICTION
          )
        )
      )
      {
        if (blockline)
        {
          fixed_t r = ((blockline->dx >> FRACBITS) * mo->momx +
                       (blockline->dy >> FRACBITS) * mo->momy) /
                      ((blockline->dx >> FRACBITS)*(blockline->dx >> FRACBITS)+
                       (blockline->dy >> FRACBITS)*(blockline->dy >> FRACBITS));
          fixed_t x = FixedMul(r, blockline->dx);
          fixed_t y = FixedMul(r, blockline->dy);

          // reflect momentum away from wall

          mo->momx = x*2 - mo->momx;
          mo->momy = y*2 - mo->momy;

          // if under gravity, slow down in
          // direction perpendicular to wall.

          if (!(mo->flags & MF_NOGRAVITY))
          {
            mo->momx = (mo->momx + x)/2;
            mo->momy = (mo->momy + y)/2;
          }
        }
        else
          mo->momx = mo->momy = 0;
      }
      else if (player || mo->flags2 & MF2_SLIDE) // try to slide along it
      {
        if (BlockingMobj == NULL || map_format.zdoom)
        {
          P_SlideMove(mo);
        }
        else
        {
          if (P_TryMove(mo, mo->x, ptryy, 0))
          {
            mo->momx = 0;
          }
          else if (P_TryMove(mo, ptryx, mo->y, 0))
          {
            mo->momy = 0;
          }
          else
          {
            mo->momx = mo->momy = 0;
          }
        }
      }
      else if (mo->flags & MF_MISSILE)
      {
        if (hexen && mo->flags2 & MF2_FLOORBOUNCE)
        {
          if (BlockingMobj)
          {
            if ((BlockingMobj->flags2 & MF2_REFLECTIVE) ||
                ((!BlockingMobj->player) &&
                 (!(BlockingMobj->flags & MF_COUNTKILL))))
            {
              fixed_t speed;

              angle = R_PointToAngle2(BlockingMobj->x, BlockingMobj->y, mo->x, mo->y) +
                      ANG1 * ((P_Random(pr_hexen) % 16) - 8);
              speed = P_AproxDistance(mo->momx, mo->momy);
              speed = FixedMul(speed, 0.75 * FRACUNIT);
              mo->angle = angle;
              angle >>= ANGLETOFINESHIFT;
              mo->momx = FixedMul(speed, finecosine[angle]);
              mo->momy = FixedMul(speed, finesine[angle]);
              if (mo->info->seesound)
              {
                S_StartMobjSound(mo, mo->info->seesound);
              }
              return;
            }
            else
            { // Struck a player/creature
              P_ExplodeMissile(mo);
            }
          }
          else
          { // Struck a wall
            P_BounceWall(mo);
            switch (mo->type)
            {
              case HEXEN_MT_SORCBALL1:
              case HEXEN_MT_SORCBALL2:
              case HEXEN_MT_SORCBALL3:
              case HEXEN_MT_SORCFX1:
                break;
              default:
                if (mo->info->seesound)
                {
                  S_StartMobjSound(mo, mo->info->seesound);
                }
                break;
            }
            return;
          }
        }

        if (BlockingMobj && (BlockingMobj->flags2 & MF2_REFLECTIVE))
        {
          angle = R_PointToAngle2(BlockingMobj->x,
                                  BlockingMobj->y, mo->x, mo->y);

          // Change angle for delflection/reflection
          switch (BlockingMobj->type)
          {
            case HEXEN_MT_CENTAUR:
            case HEXEN_MT_CENTAURLEADER:
              if (abs((int) angle - (int) BlockingMobj->angle) >> 24 > 45)
                goto explode;
              if (mo->type == HEXEN_MT_HOLY_FX)
                goto explode;
              // Drop through to sorcerer full reflection
            case HEXEN_MT_SORCBOSS:
              // Deflection
              if (P_Random(pr_hexen) < 128)
                angle += ANG45;
              else
                angle -= ANG45;
              break;
            default:
              // Reflection
              angle += ANG1 * ((P_Random(pr_hexen) % 16) - 8);
              break;
          }

          // Reflect the missile along angle
          mo->angle = angle;
          angle >>= ANGLETOFINESHIFT;
          mo->momx = FixedMul(mo->info->speed >> 1, finecosine[angle]);
          mo->momy = FixedMul(mo->info->speed >> 1, finesine[angle]);
          if (mo->flags2 & MF2_SEEKERMISSILE)
          {
            P_SetTarget(&mo->special1.m, mo->target);
          }
          P_SetTarget(&mo->target, BlockingMobj);
          return;
        }

      explode:
        // explode a missile
        if (ceilingline &&
            ceilingline->backsector &&
            ceilingline->backsector->ceilingpic == skyflatnum)
        {
          if (raven && mo->type == g_skullpop_mt)
          {
            mo->momx = mo->momy = 0;
            mo->momz = -FRACUNIT;
            return;
          }
          else if (hexen && mo->type == HEXEN_MT_HOLY_FX)
          {
            P_ExplodeMissile(mo);
            return;
          }
          else if (demo_compatibility ||  // killough
                   mo->z > ceilingline->backsector->ceilingheight)
          {
            // Hack to prevent missiles exploding
            // against the sky.
            // Does not handle sky floors.

            P_RemoveMobj(mo);
            return;
          }
        }
        P_ExplodeMissile(mo);
      }
      else // whatever else it is, it is now standing still in (x,y)
      {
        mo->momx = mo->momy = 0;
      }
    }
  } while (xmove || ymove);

  /* no friction for missiles or skulls ever, no friction when airborne */
  if (mo->flags & (MF_MISSILE | MF_SKULLFLY))
    return;

  if (mo->z > mo->floorz &&
      !(mo->flags & MF_FLY) &&
      !(mo->flags2 & MF2_FLY) &&
      !(mo->flags2 & MF2_ONMOBJ) &&
      (!hexen || mo->type != HEXEN_MT_BLASTEFFECT))
    return;

  /* killough 8/11/98: add bouncers
   * killough 9/15/98: add objects falling off ledges
   * killough 11/98: only include bouncers hanging off ledges
   */
  if (
    (
      (mo->flags & MF_BOUNCES && mo->z > mo->dropoffz) ||
       mo->flags & MF_CORPSE ||
       mo->intflags & MIF_FALLING
    ) &&
    (
      mo->momx > FRACUNIT / 4 ||
      mo->momx < -FRACUNIT / 4 ||
      mo->momy > FRACUNIT / 4 ||
      mo->momy < -FRACUNIT / 4
    ) &&
    mo->floorz != mo->subsector->sector->floorheight
  )
    return;  // do not stop sliding if halfway off a step with some momentum

  // killough 11/98:
  // Stop voodoo dolls that have come to rest, despite any
  // moving corresponding player, except in old demos:

  if (
    mo->momx > -STOPSPEED && mo->momx < STOPSPEED &&
    mo->momy > -STOPSPEED && mo->momy < STOPSPEED &&
    (
      !player ||
      !(player->cmd.forwardmove | player->cmd.sidemove) ||
      (
        player->mo != mo &&
        compatibility_level >= lxdoom_1_compatibility &&
        (comp[comp_voodooscroller] || !(mo->intflags & MIF_SCROLLING))
      )
    )
  )
  {
    // if in a walking frame, stop moving

    // killough 10/98:
    // Don't affect main player when voodoo dolls stop, except in old demos:

    if (player)
    {
      if (player->chickenTics)
      {
        if ((unsigned)(player->mo->state - states - HERETIC_S_CHICPLAY_RUN1) < 4)
        {
          P_SetMobjState(player->mo, HERETIC_S_CHICPLAY);
        }
      }
      else
      {
        if ((unsigned)(player->mo->state - states - pclass[player->pclass].run_state) < 4)
        {
          if (raven || player->mo == mo || compatibility_level >= lxdoom_1_compatibility)
          {
            P_SetMobjState(player->mo, pclass[player->pclass].normal_state);
          }
        }
      }
    }

    mo->momx = mo->momy = 0;

    /* killough 10/98: kill any bobbing momentum too (except in voodoo dolls)
     * cph - DEMOSYNC - needs compatibility check?
     */
    if (!raven && player && player->mo == mo)
      player->momx = player->momy = 0;
  }
  else
  {
    /* phares 3/17/98
     *
     * Friction will have been adjusted by friction thinkers for
     * icy or muddy floors. Otherwise it was never touched and
     * remained set at ORIG_FRICTION
     *
     * killough 8/28/98: removed inefficient thinker algorithm,
     * instead using touching_sectorlist in P_GetFriction() to
     * determine friction (and thus only when it is needed).
     *
     * killough 10/98: changed to work with new bobbing method.
     * Reducing player momentum is no longer needed to reduce
     * bobbing, so ice works much better now.
     *
     * cph - DEMOSYNC - need old code for Boom demos?
     */

    //e6y
    if (compatibility_level <= boom_201_compatibility && !prboom_comp[PC_PRBOOM_FRICTION].state)
    {
      if (mo->flags2 & MF2_FLY && !(mo->z <= mo->floorz)
          && !(mo->flags2 & MF2_ONMOBJ))
      {
        mo->momx = FixedMul(mo->momx, FRICTION_FLY);
        mo->momy = FixedMul(mo->momy, FRICTION_FLY);
      }
      else if (
        hexen ? P_GetThingFloorType(mo) == FLOOR_ICE :
        heretic ? special == 15 :
        false
      )
      {
        mo->momx = FixedMul(mo->momx, FRICTION_LOW);
        mo->momy = FixedMul(mo->momy, FRICTION_LOW);
      }
      else
      {
        // phares 3/17/98
        // Friction will have been adjusted by friction thinkers for icy
        // or muddy floors. Otherwise it was never touched and
        // remained set at ORIG_FRICTION
        mo->momx = FixedMul(mo->momx, mo->friction);
        mo->momy = FixedMul(mo->momy, mo->friction);
      }

      mo->friction = ORIG_FRICTION; // reset to normal for next tic
    }
    else if (compatibility_level <= lxdoom_1_compatibility && !prboom_comp[PC_PRBOOM_FRICTION].state)
    {
      // phares 9/10/98: reduce bobbing/momentum when on ice & up against wall

      if ((oldx == mo->x) && (oldy == mo->y)) // Did you go anywhere?
      { // No. Use original friction. This allows you to not bob so much
        // if you're on ice, but keeps enough momentum around to break free
        // when you're mildly stuck in a wall.
        mo->momx = FixedMul(mo->momx,ORIG_FRICTION);
        mo->momy = FixedMul(mo->momy,ORIG_FRICTION);
      }
      else
      { // Yes. Use stored friction.
        mo->momx = FixedMul(mo->momx,mo->friction);
        mo->momy = FixedMul(mo->momy,mo->friction);
      }
      mo->friction = ORIG_FRICTION; // reset to normal for next tic
    }
    else
    {
      fixed_t friction = P_GetFriction(mo, NULL);

      mo->momx = FixedMul(mo->momx, friction);
      mo->momy = FixedMul(mo->momy, friction);

      /* killough 10/98: Always decrease player bobbing by ORIG_FRICTION.
       * This prevents problems with bobbing on ice, where it was not being
       * reduced fast enough, leading to all sorts of kludges being developed.
       */

      if (player && player->mo == mo)     /* Not voodoo dolls */
      {
        player->momx = FixedMul(player->momx, ORIG_FRICTION);
        player->momy = FixedMul(player->momy, ORIG_FRICTION);
      }
    }
  }
}

fixed_t P_MobjGravity(mobj_t* mo)
{
  return FixedMul(mo->subsector->sector->gravity, mo->gravity);
}

void P_AutoCorrectLookDir(player_t* player)
{
  if (allow_incompatibility && dsda_MouseLook())
  {
    return;
  }

  player->centering = true;
}

//
// P_ZMovement
//
// Attempt vertical movement.

static void P_ZMovement (mobj_t* mo)
{
  fixed_t gravity = P_MobjGravity(mo);

  /* killough 7/11/98:
   * BFG fireballs bounced on floors and ceilings in Pre-Beta Doom
   * killough 8/9/98: added support for non-missile objects bouncing
   * (e.g. grenade, mine, pipebomb)
   */

  if (mo->flags & MF_BOUNCES && mo->momz)
  {
    mo->z += mo->momz;
    if (mo->z <= mo->floorz)                /* bounce off floors */
    {
      mo->z = mo->floorz;
      if (mo->momz < 0)
      {
        mo->momz = -mo->momz;
        if (!(mo->flags & MF_NOGRAVITY)) /* bounce back with decay */
        {
          mo->momz = mo->flags & MF_FLOAT ?   // floaters fall slowly
                     mo->flags & MF_DROPOFF ? // DROPOFF indicates rate
                     FixedMul(mo->momz, (fixed_t)(FRACUNIT*.85)) :
                     FixedMul(mo->momz, (fixed_t)(FRACUNIT*.70)) :
                     FixedMul(mo->momz, (fixed_t)(FRACUNIT*.45)) ;

          /* Bring it to rest below a certain speed */
          if (D_abs(mo->momz) <= mo->info->mass * (gravity * 4 / 256))
            mo->momz = 0;
        }

        /* killough 11/98: touchy objects explode on impact */
        if (mo->flags & MF_TOUCHY && mo->intflags & MIF_ARMED
            && mo->health > 0)
          P_DamageMobj(mo, NULL, NULL, mo->health);
        else if (mo->flags & MF_FLOAT && sentient(mo))
          goto floater;
        return;
      }
    }
    else if (mo->z >= mo->ceilingz - mo->height)
    {
      /* bounce off ceilings */
      mo->z = mo->ceilingz - mo->height;
      if (mo->momz > 0)
      {
        if (mo->subsector->sector->ceilingpic != skyflatnum)
          mo->momz = -mo->momz;    /* always bounce off non-sky ceiling */
        else if (mo->flags & MF_MISSILE)
          P_RemoveMobj(mo);        /* missiles don't bounce off skies */
        else if (mo->flags & MF_NOGRAVITY)
          mo->momz = -mo->momz; // bounce unless under gravity

        if (mo->flags & MF_FLOAT && sentient(mo))
          goto floater;

        return;
      }
    }
    else
    {
      if (!(mo->flags & MF_NOGRAVITY))      /* free-fall under gravity */
        mo->momz -= mo->info->mass * (gravity / 256);

      if (mo->flags & MF_FLOAT && sentient(mo)) goto floater;
      return;
    }

    /* came to a stop */
    mo->momz = 0;

    if (mo->flags & MF_MISSILE)
    {
      if (ceilingline &&
          ceilingline->backsector &&
          ceilingline->backsector->ceilingpic == skyflatnum &&
          mo->z > ceilingline->backsector->ceilingheight)
        P_RemoveMobj(mo);  /* don't explode on skies */
      else
        P_ExplodeMissile(mo);
    }

    if (mo->flags & MF_FLOAT && sentient(mo)) goto floater;
    return;
  }

  /* killough 8/9/98: end bouncing object code */

  // check for smooth step up

  if (mo->player && //e6y: restoring original visual behaviour for demo_compatibility
      (demo_compatibility || mo->player->mo == mo) &&  // killough 5/12/98: exclude voodoo dolls
      mo->z < mo->floorz)
  {
    mo->player->viewheight -= mo->floorz - mo->z;
    mo->player->deltaviewheight = (g_viewheight - mo->player->viewheight) >> 3;
  }

  // adjust altitude

  mo->z += mo->momz;

floater:
  if ((mo->flags & MF_FLOAT) && mo->target)

    // float down towards target if too close

    if (!(mo->flags & MF_SKULLFLY) && !(mo->flags & MF_INFLOAT))
    {
      fixed_t delta;
      if (P_AproxDistance(mo->x - mo->target->x, mo->y - mo->target->y) <
          D_abs(delta = mo->target->z + (mo->height >> 1) - mo->z) * 3)
        mo->z += delta < 0 ? -FLOATSPEED : FLOATSPEED;
    }

  if (mo->player && (mo->flags & MF_FLY) && (mo->z > mo->floorz))
  {
    mo->z += finesine[(FINEANGLES/80*gametic)&FINEMASK]/8;
    mo->momz = FixedMul (mo->momz, FRICTION_FLY);
  }

  if (mo->player && mo->flags2 & MF2_FLY && !(mo->z <= mo->floorz)
      && leveltime & 2)
  {
      mo->z += finesine[(FINEANGLES / 20 * leveltime >> 2) & FINEMASK];
  }

  // clip movement

  if (mo->z <= mo->floorz)
  {
    // hit the floor

    if (raven)
    {
      if (mo->flags & MF_MISSILE)
      {
        mo->z = mo->floorz;
        if (mo->flags2 & MF2_FLOORBOUNCE)
        {
          P_FloorBounceMissile(mo);
          return;
        }
        else if (
          raven && (
            mo->type == HERETIC_MT_MNTRFX2 ||
            mo->type == HEXEN_MT_MNTRFX2 ||
            mo->type == HEXEN_MT_LIGHTNING_FLOOR
          )
        )
        {                   // Minotaur floor fire can go up steps
          return;
        }
        else if (hexen && mo->type == HEXEN_MT_HOLY_FX)
        {                   // The spirit struck the ground
          mo->momz = 0;
          P_HitFloor(mo);
          return;
        }
        else
        {
          if (hexen)
            P_HitFloor(mo);
          P_ExplodeMissile(mo);
          return;
        }
      }

      if (hexen && mo->flags & MF_COUNTKILL)   // Blasted mobj falling
      {
        if (mo->momz < -(23 * FRACUNIT))
        {
          P_DamageMobj(mo, NULL, NULL, 10000);
        }
      }

      if (mo->z - mo->momz > mo->floorz)
      {                       // Spawn splashes, etc.
          P_HitFloor(mo);
      }
    }

    /* Note (id):
     *  somebody left this after the setting momz to 0,
     *  kinda useless there.
     * cph - This was the a bug in the linuxdoom-1.10 source which
     *  caused it not to sync Doom 2 v1.9 demos. Someone
     *  added the above comment and moved up the following code. So
     *  demos would desync in close lost soul fights.
     * cph - revised 2001/04/15 -
     * This was a bug in the Doom/Doom 2 source; the following code
     *  is meant to make charging lost souls bounce off of floors, but it
     *  was incorrectly placed after momz was set to 0.
     *  However, this bug was fixed in Doom95, Final/Ultimate Doom, and
     *  the v1.10 source release (which is one reason why it failed to sync
     *  some Doom2 v1.9 demos)
     * I've added a comp_soul compatibility option to make this behavior
     *  selectable for PrBoom v2.3+. For older demos, we do this here only
     *  if we're in a compatibility level above Doom 2 v1.9 (in which case we
     *  mimic the bug and do it further down instead)
     */

    if (
      mo->flags & MF_SKULLFLY &&
      (
        !comp[comp_soul] ||
      	(
          compatibility_level > doom2_19_compatibility &&
      	  compatibility_level < prboom_4_compatibility
        )
      )
    )
      mo->momz = -mo->momz; // the skull slammed into something

    if (hexen) mo->z = mo->floorz;
    if (mo->momz < 0)
    {
      /* killough 11/98: touchy objects explode on impact */
      if (mo->flags & MF_TOUCHY && mo->intflags & MIF_ARMED && mo->health > 0)
        P_DamageMobj(mo, NULL, NULL, mo->health);
      else
      {
        if (mo->flags2 & MF2_ICEDAMAGE && mo->momz < -gravity * 8)
        {
          mo->tics = 1;
          mo->momx = 0;
          mo->momy = 0;
          mo->momz = 0;
          return;
        }
        // heretic_note: probably not necessary?
        if (!heretic && mo->player)
            mo->player->jumpTics = 7;
        if (
          mo->player && /* killough 5/12/98: exclude voodoo dolls */
          // e6y
          // Restoring original visual behaviour for demo_compatibility.
          // Viewheight of consoleplayer should be decreased for a moment
          // after voodoo doll hits the ground.
          // This additional condition makes sense only for plutonia complevel
          // when voodoo doll falls down after teleporting,
          // but can be applied globally for all demo_compatibility complevels,
          // because original sources do not exclude voodoo dolls from condition above,
          // but Boom does it.
          (demo_compatibility || mo->player->mo == mo) &&
          mo->momz < -gravity * 8 &&
          !(mo->flags2 & MF2_FLY)
        )
        {
          // Squat down.
          // Decrease viewheight for a moment
          // after hitting the ground (hard),
          // and utter appropriate sound.

          mo->player->deltaviewheight = mo->momz >> 3;

          if (heretic)
          {
            S_StartMobjSound(mo, heretic_sfx_plroof);
            P_AutoCorrectLookDir(mo->player);
          }
          else if (hexen)
          {
            if (mo->momz < -23 * FRACUNIT)
            {
              P_FallingDamage(mo->player);
              P_NoiseAlert(mo, mo);
            }
            else if (mo->momz < -gravity * 12 && !mo->player->morphTics)
            {
              S_StartMobjSound(mo, hexen_sfx_player_land);
              switch (mo->player->pclass)
              {
                case PCLASS_FIGHTER:
                  S_StartMobjSound(mo, hexen_sfx_player_fighter_grunt);
                  break;
                case PCLASS_CLERIC:
                  S_StartMobjSound(mo, hexen_sfx_player_cleric_grunt);
                  break;
                case PCLASS_MAGE:
                  S_StartMobjSound(mo, hexen_sfx_player_mage_grunt);
                  break;
                default:
                  break;
              }
            }
            else if (P_GetThingFloorType(mo) < FLOOR_LIQUID && !mo->player->morphTics)
            {
              S_StartMobjSound(mo, hexen_sfx_player_land);
            }
            P_AutoCorrectLookDir(mo->player);
          }
          //e6y: compatibility optioned
          else if (comp[comp_sound] || (mo->health > 0)) /* cph - prevent "oof" when dead */
            S_StartSound (mo, sfx_oof);
        }
        else if (hexen && mo->type >= HEXEN_MT_POTTERY1 && mo->type <= HEXEN_MT_POTTERY3)
        {
          P_DamageMobj(mo, NULL, NULL, 25);
        }
      }
      mo->momz = 0;
    }
    if (!hexen) mo->z = mo->floorz;

    /* cph 2001/04/15 -
     * This is the buggy lost-soul bouncing code referenced above.
     * We've already set momz = 0 normally by this point, so it's useless.
     * However we might still have upward momentum, in which case this will
     * incorrectly reverse it, so we might still need this for demo sync
     */
    if (mo->flags & MF_SKULLFLY &&
	     compatibility_level <= doom2_19_compatibility)
      mo->momz = -mo->momz; // the skull slammed into something

    if (mo->info->crashstate && (mo->flags & MF_CORPSE) && !(mo->flags2 & MF2_ICEDAMAGE))
    {
      P_SetMobjState(mo, mo->info->crashstate);
      return;
    }

    if (!raven && (mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP))
    {
      P_ExplodeMissile (mo);
      return;
    }
  }
  else if (mo->flags2 & MF2_LOGRAV || (mo->type == MT_GIBDTH && allow_incompatibility))
  {
    if (mo->momz == 0)
      mo->momz = -(gravity >> 3) * 2;
    else
      mo->momz -= gravity >> 3;
  }
  else if (!(mo->flags & MF_NOGRAVITY))
  {
    if (mo->momz == 0)
      mo->momz = -gravity;
    mo->momz -= gravity;
  }

  if (mo->z + mo->height > mo->ceilingz)
  {
    /* cph 2001/04/15 -
     * Lost souls were meant to bounce off of ceilings;
     *  new comp_soul compatibility option added
     */
    if (!comp[comp_soul] && mo->flags & MF_SKULLFLY)
      mo->momz = -mo->momz; // the skull slammed into something

    // hit the ceiling

    if (mo->momz > 0)
      mo->momz = 0;

    mo->z = mo->ceilingz - mo->height;

    if (hexen && mo->flags2 & MF2_FLOORBOUNCE)
    {
      if (mo->info->seesound)
      {
        S_StartMobjSound(mo, mo->info->seesound);
      }
      return;
    }

    /* cph 2001/04/15 -
     * We might have hit a ceiling but had downward momentum (e.g. ceiling is
     *  lowering on us), so for old demos we must still do the buggy
     *  momentum reversal here
     */
    if (comp[comp_soul] && mo->flags & MF_SKULLFLY)
      mo->momz = -mo->momz; // the skull slammed into something

    if ((mo->flags & MF_MISSILE) && (hexen || !(mo->flags & MF_NOCLIP)))
    {
      if (hexen && mo->type == HEXEN_MT_LIGHTNING_CEILING)
      {
        return;
      }
      if (raven && mo->subsector->sector->ceilingpic == skyflatnum)
      {
        if (mo->type == g_skullpop_mt)
        {
          mo->momx = mo->momy = 0;
          mo->momz = -FRACUNIT;
        }
        else if (mo->type == HEXEN_MT_HOLY_FX)
        {
          P_ExplodeMissile(mo);
        }
        else
        {
          P_RemoveMobj(mo);
        }
        return;
      }
      P_ExplodeMissile (mo);
      return;
    }
  }
}

//
// P_NightmareRespawn
//

static void P_NightmareRespawn(mobj_t* mobj)
{
  fixed_t      x;
  fixed_t      y;
  fixed_t      z;
  subsector_t* ss;
  mobj_t*      mo;
  mapthing_t*  mthing;

  x = mobj->spawnpoint.x;
  y = mobj->spawnpoint.y;

  /* haleyjd: stupid nightmare respawning bug fix
   *
   * 08/09/00: compatibility added, time to ramble :)
   * This fixes the notorious nightmare respawning bug that causes monsters
   * that didn't spawn at level startup to respawn at the point (0,0)
   * regardless of that point's nature. SMMU and Eternity need this for
   * script-spawned things like Halif Swordsmythe, as well.
   *
   * cph - copied from eternity, alias comp_respawnfix
   */
  if(!comp[comp_respawn] && !x && !y)
  {
     // spawnpoint was zeroed out, so use point of death instead
     x = mobj->x;
     y = mobj->y;
  }

  // something is occupying its position?

  if (!P_CheckPosition (mobj, x, y) )
    return; // no respwan

  // spawn a teleport fog at old spot
  // because of removal of the body?

  mo = P_SpawnMobj (mobj->x,
                    mobj->y,
                    mobj->subsector->sector->floorheight + g_telefog_height,
                    g_mt_tfog);

  // initiate teleport sound

  S_StartSound (mo, g_sfx_telept);

  // spawn a teleport fog at the new spot

  ss = R_PointInSubsector (x,y);

  mo = P_SpawnMobj (x, y, ss->sector->floorheight + g_telefog_height, g_mt_tfog);

  S_StartSound (mo, g_sfx_telept);

  // spawn the new monster

  mthing = &mobj->spawnpoint;
  if (mobj->info->flags & MF_SPAWNCEILING)
    z = ONCEILINGZ;
  else
    z = ONFLOORZ;

  // inherit attributes from deceased one

  mo = P_SpawnMobj (x,y,z, mobj->type);
  mo->spawnpoint = mobj->spawnpoint;
  mo->angle = ANG45 * (mthing->angle/45);
  mo->index = mobj->index;

  // "bug" in the respawn code for heretic
  // the chicken's return type is stored in special2.i
  // that value didn't exist in doom so is left uninitialized on respawn
  // we have to set this to the MT zero value for heretic
  if (heretic && mo->type == HERETIC_MT_CHICKEN)
    mo->special2.i = HERETIC_MT_ZERO;

  if (hexen && mo->type == HEXEN_MT_PIG)
    mo->special2.i = HEXEN_MT_ZERO;

  if (mthing->options & MTF_AMBUSH)
    mo->flags |= MF_AMBUSH;

  /* killough 11/98: transfer friendliness from deceased */
  mo->flags = (mo->flags & ~MF_FRIEND) | (mobj->flags & MF_FRIEND);
  mo->flags = mo->flags | MF_RESSURECTED;//e6y

  mo->reactiontime = 18;

  // remove the old monster,

  P_RemoveMobj (mobj);
}

fixed_t FloatBobOffsets[64] = {
    0, 51389, 102283, 152192,
    200636, 247147, 291278, 332604,
    370727, 405280, 435929, 462380,
    484378, 501712, 514213, 521763,
    524287, 521763, 514213, 501712,
    484378, 462380, 435929, 405280,
    370727, 332604, 291278, 247147,
    200636, 152192, 102283, 51389,
    -1, -51390, -102284, -152193,
    -200637, -247148, -291279, -332605,
    -370728, -405281, -435930, -462381,
    -484380, -501713, -514215, -521764,
    -524288, -521764, -514214, -501713,
    -484379, -462381, -435930, -405280,
    -370728, -332605, -291279, -247148,
    -200637, -152193, -102284, -51389
};

//
// P_MobjThinker
//

static void PlayerLandedOnThing(mobj_t * mo, mobj_t * onmobj, fixed_t gravity);

void P_MobjThinker (mobj_t* mobj)
{
  // killough 11/98:
  // removed old code which looked at target references
  // (we use pointer reference counting now)

  if (mobj->type == MT_MUSICSOURCE)
  {
    MusInfoThinker(mobj);
    return;
  }

  mobj->PrevX = mobj->x;
  mobj->PrevY = mobj->y;
  mobj->PrevZ = mobj->z;

  // momentum movement
  BlockingMobj = NULL;
  if (mobj->momx | mobj->momy || mobj->flags & MF_SKULLFLY)
  {
    P_XYMovement(mobj);
    mobj->intflags &= ~MIF_SCROLLING;
    if (mobj->thinker.function != P_MobjThinker) // cph - Must've been removed
      return;       // killough - mobj was removed
  }
  else if (mobj->flags2 & MF2_BLASTED)
  {                           // Reset to not blasted when momentums are gone
    ResetBlasted(mobj);
  }

  if (mobj->flags2 & MF2_FLOATBOB)
  {                           // Floating item bobbing motion
      mobj->z = mobj->floorz +
                (hexen ? mobj->special1.i : 0) + FloatBobOffsets[(mobj->health++) & 63];
  }
  else if (mobj->z != mobj->floorz || mobj->momz || BlockingMobj)
  {
    if (mobj->flags2 & MF2_PASSMOBJ)
    {
      mobj_t *onmo;

      if (!(onmo = P_CheckOnmobj(mobj)))
      {
        P_ZMovement(mobj);

        // This bug is part of the original source
        if (hexen && mobj->player && mobj->flags & MF2_ONMOBJ)
        {
          mobj->flags2 &= ~MF2_ONMOBJ;
        }

        if (map_format.zdoom)
        {
          mobj->flags2 &= ~MF2_ONMOBJ;
        }
      }
      else
      {
        if (mobj->player)
        {
          if (map_format.hexen)
          {
            fixed_t gravity = P_MobjGravity(mobj);

            if (hexen && mobj->momz < -gravity * 8 && !(mobj->flags2 & MF2_FLY))
            {
              PlayerLandedOnThing(mobj, onmo, gravity);
            }
            if (onmo->z + onmo->height - mobj->z <= 24 * FRACUNIT)
            {
              mobj->player->viewheight -= onmo->z + onmo->height - mobj->z;
              mobj->player->deltaviewheight = (g_viewheight - mobj->player->viewheight) >> 3;
              mobj->z = onmo->z + onmo->height;
              mobj->flags2 |= MF2_ONMOBJ;
              mobj->momz = 0;
            }
            else
            {           // hit the bottom of the blocking mobj
              mobj->momz = 0;
            }
          }
          else
          {
            if (mobj->momz < 0)
            {
              mobj->flags2 |= MF2_ONMOBJ;
              mobj->momz = 0;
            }
            if (onmo->player || onmo->type == HERETIC_MT_POD)
            {
              mobj->momx = onmo->momx;
              mobj->momy = onmo->momy;
              if (onmo->z < onmo->floorz)
              {
                mobj->z += onmo->floorz - onmo->z;
                if (onmo->player)
                {
                  onmo->player->viewheight -= onmo->floorz - onmo->z;
                  onmo->player->deltaviewheight = (g_viewheight - onmo->player->viewheight) >> 3;
                }
                onmo->z = onmo->floorz;
              }
            }
          }
        }
        else if (map_format.zdoom)
        {
          mobj->momz = 0;

          if (onmo->z + onmo->height - mobj->z <= 24 * FRACUNIT)
          {
            mobj->z = onmo->z + onmo->height;
            mobj->flags2 |= MF2_ONMOBJ;
          }
        }
      }
    }
    else
      P_ZMovement(mobj);
    if (mobj->thinker.function != P_MobjThinker) // cph - Must've been removed
      return;       // killough - mobj was removed
  }
  // raven_note: are the intflags irrelevant when compatibility is enabled?
  else if (!raven && !(mobj->momx | mobj->momy) && !sentient(mobj))
  {                                  // non-sentient objects at rest
    mobj->intflags |= MIF_ARMED;     // arm a mine which has come to rest

    // killough 9/12/98: objects fall off ledges if they are hanging off
    // slightly push off of ledge if hanging more than halfway off

    if (mobj->z > mobj->dropoffz &&      // Only objects contacting dropoff
        !(mobj->flags & MF_NOGRAVITY) && // Only objects which fall
        !comp[comp_falloff]) // Not in old demos
      P_ApplyTorque(mobj);               // Apply torque
    else
      mobj->intflags &= ~MIF_FALLING, mobj->gear = 0;  // Reset torque
  }

  if (map_format.mobj_in_special_sector(mobj))
    return;

  // cycle through states,
  // calling action functions at transitions
  if (mobj->tics != -1)
  {
    mobj->tics--;

    // you can cycle through multiple states in a tic

    // raven's cycle code is only here (i.e., not in every call to P_SetMobjState)
    // not sure about ramifications of moving loop inside all state calls
    if (raven)
    {
      while (!mobj->tics)
        if (!P_SetMobjState(mobj, mobj->state->nextstate))
          return;     // freed itself
    }
    else
    {
      if (!mobj->tics)
        if (!P_SetMobjState(mobj, mobj->state->nextstate))
          return;     // freed itself
    }
  }
  else
  {
    // check for nightmare respawn

    if (! (mobj->flags & MF_COUNTKILL) )
      return;

    if (!skill_info.respawn_time)
      return;

    mobj->movecount++;

    if (mobj->movecount < skill_info.respawn_time * 35)
      return;

    if (leveltime & 31)
      return;

    if (P_Random(pr_respawn) > 4)
      return;

    P_NightmareRespawn (mobj);
  }
}


// Certain functions assume that a mobj_t pointer is non-NULL,
// causing a crash in some situations where it is NULL.  Vanilla
// Doom did not crash because of the lack of proper memory
// protection. This function substitutes NULL pointers for
// pointers to a dummy mobj, to avoid a crash.
mobj_t *P_SubstNullMobj(mobj_t *mobj)
{
    if (mobj == NULL)
    {
        static mobj_t dummy_mobj;

        dummy_mobj.x = 0;
        dummy_mobj.y = 0;
        dummy_mobj.z = 0;
        dummy_mobj.flags = 0;

        mobj = &dummy_mobj;
    }

    return mobj;
}

/*
 * P_FindDoomedNum
 *
 * Finds a mobj type with a matching doomednum
 *
 * killough 8/24/98: rewrote to use hashing
 */

static dboolean P_IsTypeMatch(unsigned doomednum, int type)
{
  return (unsigned) mobjinfo[type].doomednum == doomednum &&
         mobjinfo[type].visibility & map_format.visibility;
}

static PUREFUNC int P_FindDoomedNum(unsigned type)
{
  static struct { int first, next; } *hash;
  register int i;

  if (!hash)
  {
    hash = Z_Malloc(sizeof(*hash) * num_mobj_types);

    for (i = 0; i < num_mobj_types; i++)
      hash[i].first = num_mobj_types;

    for (i = mobj_types_zero; i < num_mobj_types; i++)
      if (mobjinfo[i].doomednum != -1)
      {
        unsigned h = (unsigned) mobjinfo[i].doomednum % num_mobj_types;
        hash[i].next = hash[h].first;
        hash[h].first = i;
      }
  }

  i = hash[type % num_mobj_types].first;
  while (i < num_mobj_types && !P_IsTypeMatch(type, i))
    i = hash[i].next;

  return i;
}

dboolean P_SpawnProjectile(short thing_id, mobj_t *source, int spawn_num, angle_t angle,
	                         fixed_t speed, fixed_t vspeed, short dest_id, mobj_t *forcedest,
                           int gravity, short new_thing_id)
{
  int type;
  dboolean is_monster;
  dboolean success = false;
  thing_id_search_t search;
  thing_id_search_t dest_search;
  mobj_t *spawn_location;
  mobj_t *destination;
  mobj_t *new_mobj;

  type = dsda_ThingTypeFromSpawnNumber(spawn_num);

  if (type == num_mobj_types)
    return false;

  is_monster = (type == MT_SKULL || (mobjinfo[type].flags & MF_COUNTKILL));

  if (nomonsters && is_monster)
    return false;

  dsda_ResetThingIDSearch(&search);
  while ((spawn_location = dsda_FindMobjFromThingIDOrMobj(thing_id, source, &search)))
  {
    dsda_ResetThingIDSearch(&dest_search);
    destination = dsda_FindMobjFromThingIDOrMobj(dest_id, forcedest, &dest_search);

    if (!dest_id || destination)
    {
      do
      {
        new_mobj = P_SpawnMobj(spawn_location->x, spawn_location->y, spawn_location->z, type);
        if (new_mobj)
        {
          if (new_thing_id)
            dsda_AddMobjThingID(new_mobj, new_thing_id);

          if (new_mobj->info->seesound)
          {
            S_StartMobjSound(new_mobj, new_mobj->info->seesound);
          }

          if (gravity)
          {
            new_mobj->flags &= ~MF_NOGRAVITY;
            if (!is_monster && gravity == 1)
            {
              new_mobj->flags2 |= MF2_LOGRAV;
            }
          }
          else
          {
            new_mobj->flags |= MF_NOGRAVITY;
          }

          P_SetTarget(&new_mobj->target, spawn_location);

          if (destination)
          {
            fixed_t aimx, aimy, aimz;
            fixed_t slope;

            aimx = destination->x - new_mobj->x;
            aimy = destination->y - new_mobj->y;
            aimz = destination->z + destination->height / 2 - new_mobj->z;
            slope = FixedDiv(aimz, P_AproxDistance(aimx, aimy));
            new_mobj->angle = R_PointToAngle2(0, 0, aimx, aimy);
            new_mobj->momx = FixedMul(speed, finecosine[new_mobj->angle >> ANGLETOFINESHIFT]);
            new_mobj->momy = FixedMul(speed, finesine[new_mobj->angle >> ANGLETOFINESHIFT]);
            new_mobj->momz = FixedMul(speed, slope);
          }
          else
          {
            new_mobj->angle = angle;
            new_mobj->momx = FixedMul(speed, finecosine[new_mobj->angle >> ANGLETOFINESHIFT]);
            new_mobj->momy = FixedMul(speed, finesine[new_mobj->angle >> ANGLETOFINESHIFT]);
            new_mobj->momz = vspeed;
          }

          new_mobj->flags |= MF_DROPPED; // Don't respawn

          if (new_mobj->flags & MF_MISSILE)
          {
            if (P_CheckMissileSpawn(new_mobj))
            {
              success = true;
            }
          }
          else if (P_TestMobjLocation(new_mobj))
          {
            success = true;
          }
          else
          {
            dsda_WatchFailedSpawn(new_mobj);
            P_RemoveMobj(new_mobj);
          }
        }
      } while (dest_id && (destination = dsda_FindMobjFromThingIDOrMobj(dest_id, forcedest, &dest_search)));
    }
  }

  return success;
}

dboolean P_SpawnThing(short thing_id, mobj_t *source, int spawn_num,
                      angle_t angle, dboolean fog, short new_thing_id)
{
  int type;
  dboolean success = false;
  thing_id_search_t search;
  mobj_t *spawn_location;
  mobj_t *new_mobj;

  type = dsda_ThingTypeFromSpawnNumber(spawn_num);

  if (type == num_mobj_types)
    return false;

  if (nomonsters && (type == MT_SKULL || (mobjinfo[type].flags & MF_COUNTKILL)))
    return false;

  dsda_ResetThingIDSearch(&search);
  while ((spawn_location = dsda_FindMobjFromThingIDOrMobj(thing_id, source, &search)))
  {
    new_mobj = P_SpawnMobj(spawn_location->x, spawn_location->y, spawn_location->z, type);
    if (!P_TestMobjLocation(new_mobj))
    {
      dsda_WatchFailedSpawn(new_mobj);
      P_RemoveMobj(new_mobj);
    }
    else
    {
      new_mobj->angle = angle == ANGLE_MAX ? spawn_location->angle : angle;
      if (fog)
      {
        mobj_t *fog_mobj;
        fog_mobj = P_SpawnMobj(spawn_location->x, spawn_location->y,
                              spawn_location->z + TELEFOGHEIGHT, g_mt_tfog);
        S_StartMobjSound(fog_mobj, g_sfx_telept);
      }
      if (new_thing_id)
        dsda_AddMobjThingID(new_mobj, new_thing_id);
      new_mobj->flags |= MF_DROPPED; // Don't respawn
      success = true;
    }
  }

  return success;
}

int P_MobjSpawnHealth(const mobj_t* mobj)
{
  int result;

  if (mobj->type == MT_SKULL || mobj->flags & MF_COUNTKILL)
  {
    if (mobj->flags & MF_FRIEND)
    {
      if (skill_info.friend_health_factor)
      {
        result = FixedMul(mobj->info->spawnhealth, skill_info.friend_health_factor);

        return result > 0 ? result : 1;
      }
    }
    else if (skill_info.monster_health_factor)
    {
      result = FixedMul(mobj->info->spawnhealth, skill_info.monster_health_factor);

      return result > 0 ? result : 1;
    }
  }

  return mobj->info->spawnhealth;
}

//
// P_SpawnMobj
//
mobj_t* P_SpawnMobj(fixed_t x,fixed_t y,fixed_t z,mobjtype_t type)
{
  mobj_t*     mobj;
  state_t*    st;
  mobjinfo_t* info;

  mobj = Z_MallocLevel (sizeof(*mobj));
  memset (mobj, 0, sizeof (*mobj));
  info = &mobjinfo[type];
  mobj->type = type;
  mobj->info = info;
  mobj->x = x;
  mobj->y = y;
  mobj->radius = info->radius;
  mobj->height = info->height;                                      // phares
  mobj->flags  = info->flags;
  mobj->flags2 = info->flags2;
  if (raven) mobj->damage = info->damage;

  /* killough 8/23/98: no friends, bouncers, or touchy things in old demos */
  if (!mbf_features)
    mobj->flags &= ~(MF_BOUNCES | MF_FRIEND | MF_TOUCHY);
  else
    if (type == g_mt_player)         // Except in old demos, players
      mobj->flags |= MF_FRIEND;    // are always friends.

  if (map_info.flags & MI_PASSOVER && mobj->flags & MF_SOLID)
    mobj->flags2 |= MF2_PASSMOBJ;

  mobj->health = P_MobjSpawnHealth(mobj);

  if (!(skill_info.flags & SI_INSTANT_REACTION))
    mobj->reactiontime = info->reactiontime;

  mobj->lastlook = P_Random (pr_lastlook) % g_maxplayers;

  // do not set the state with P_SetMobjState,
  // because action routines can not be called yet

  st = &states[info->spawnstate];

  mobj->state  = st;
  mobj->tics   = st->tics;
  mobj->sprite = st->sprite;
  mobj->frame  = st->frame;
  mobj->touching_sectorlist = NULL; // NULL head of sector list // phares 3/13/98

  // set subsector and/or block links

  P_SetThingPosition (mobj);

  mobj->dropoffz =           /* killough 11/98: for tracking dropoffs */
  mobj->floorz   = mobj->subsector->sector->floorheight;
  mobj->ceilingz = mobj->subsector->sector->ceilingheight;

  if (z == ONFLOORZ)
  {
      mobj->z = mobj->floorz;
  }
  else if (z == ONCEILINGZ)
  {
      mobj->z = mobj->ceilingz - mobj->height;
  }
  else if (raven && z == FLOATRANDZ)
  {
    fixed_t space;

    space = ((mobj->ceilingz) - (mobj->height)) - mobj->floorz;
    if (space > 48 * FRACUNIT)
    {
      space -= 40 * FRACUNIT;
      mobj->z =
        ((space * P_Random(pr_heretic)) >> 8) + mobj->floorz + 40 * FRACUNIT;
    }
    else
    {
      mobj->z = mobj->floorz;
    }
  }
  else if (hexen && mobj->flags2 & MF2_FLOATBOB)
  {
    mobj->z = mobj->floorz + z;     // artifact z passed in as height
  }
  else
  {
    mobj->z = z;
  }

  if (hexen)
  {
    if (mobj->flags2 & MF2_FOOTCLIP
        && P_GetThingFloorType(mobj) >= FLOOR_LIQUID
        && mobj->z == mobj->subsector->sector->floorheight)
    {
      mobj->floorclip = 10 * FRACUNIT;
    }
    else
    {
      mobj->floorclip = 0;
    }
  }
  else
  {
    if (mobj->flags2 & MF2_FOOTCLIP
        && P_GetThingFloorType(mobj) != FLOOR_SOLID
        && mobj->floorz == mobj->subsector->sector->floorheight)
    {
        mobj->flags2 |= MF2_FEETARECLIPPED;
    }
    else
    {
        mobj->flags2 &= ~MF2_FEETARECLIPPED;
    }
  }

  mobj->PrevX = mobj->x;
  mobj->PrevY = mobj->y;
  mobj->PrevZ = mobj->z;

  mobj->thinker.function = P_MobjThinker;

  //e6y
  mobj->friction = ORIG_FRICTION;                        // phares 3/17/98
  mobj->gravity = map_info.gravity;
  mobj->alpha = 1.f;
  mobj->index = -1;

  mobj->target = mobj->tracer = mobj->lastenemy = NULL;
  P_AddThinker(&mobj->thinker);
  if (!((mobj->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    totallive++;

  dsda_WatchSpawn(mobj);

  return mobj;
}


static mapthing_t itemrespawnque[ITEMQUESIZE];
static int        itemrespawntime[ITEMQUESIZE];
int        iquehead;
int        iquetail;


//
// P_RemoveMobj
//

void P_RemoveMobj (mobj_t* mobj)
{
  if (raven) // so short, just putting it here
  {
    if (hexen)
    {
      // Remove from creature queue
      if (mobj->flags & MF_COUNTKILL && mobj->flags & MF_CORPSE)
      {
        A_DeQueueCorpse(mobj);
      }
    }

    if (map_format.thing_id && mobj->tid)
    {
      map_format.remove_mobj_thing_id(mobj);
    }

    P_UnsetThingPosition(mobj);
    if (sector_list)
    {
      P_DelSeclist(sector_list);
      sector_list = NULL;
    }
    S_StopSound(mobj);
    P_RemoveThinker((thinker_t *) mobj);
    return;
  }

  if ((mobj->flags & MF_SPECIAL)
      && !(mobj->flags & MF_DROPPED)
      && (mobj->type != MT_INV)
      && (mobj->type != MT_INS))
    {
    itemrespawnque[iquehead] = mobj->spawnpoint;
    itemrespawntime[iquehead] = leveltime;
    iquehead = (iquehead+1)&(ITEMQUESIZE-1);

    // lose one off the end?

    if (iquehead == iquetail)
      iquetail = (iquetail+1)&(ITEMQUESIZE-1);
    }

  if (map_format.thing_id && mobj->tid)
  {
    map_format.remove_mobj_thing_id(mobj);
  }

  // unlink from sector and block lists

  P_UnsetThingPosition (mobj);

  // Delete all nodes on the current sector_list               phares 3/16/98

  if (sector_list)
    {
    P_DelSeclist(sector_list);
    sector_list = NULL;
    }

  // stop any playing sound

  // [FG] removed map objects may finish their sounds
  if (full_sounds)
    S_UnlinkSound(mobj);
  else
    S_StopSound(mobj);

  // killough 11/98:
  //
  // Remove any references to other mobjs.
  //
  // Older demos might depend on the fields being left alone, however,
  // if multiple thinkers reference each other indirectly before the
  // end of the current tic.
  // CPhipps - only leave dead references in old demos; I hope lxdoom_1 level
  // demos are rare and don't rely on this. I hope.

  if (compatibility_level >= lxdoom_1_compatibility || allow_incompatibility) {
    P_SetTarget(&mobj->target,    NULL);
    P_SetTarget(&mobj->tracer,    NULL);
    P_SetTarget(&mobj->lastenemy, NULL);
  }
  // free block

  P_RemoveThinker (&mobj->thinker);
}

void P_RemoveMonsters(void)
{
  thinker_t *th;
  mobj_t *mobj;

  for (th = thinkercap.next; th != &thinkercap; th = th->next)
  {
    if (th->function != P_MobjThinker)
      continue;

    mobj = (mobj_t *) th;
    if (mobj->player)
      continue;

    if (
      mobj->type == MT_SKULL ||
      mobj->flags & MF_COUNTKILL ||
      (mobj->target && !mobj->target->player)
    )
      P_RemoveMobj(mobj);
  }

  P_CleanThinkers();
}

//
// P_RespawnSpecials
//

void P_RespawnSpecials (void)
{
  fixed_t       x;
  fixed_t       y;
  fixed_t       z;
  subsector_t*  ss;
  mobj_t*       mo;
  mapthing_t*   mthing;
  int           i;

  // only respawn items in deathmatch

  if (deathmatch != 2)
    return;

  // nothing left to respawn?

  if (iquehead == iquetail)
    return;

  // wait at least 30 seconds

  if (leveltime - itemrespawntime[iquetail] < 30*35)
    return;

  mthing = &itemrespawnque[iquetail];

  x = mthing->x;
  y = mthing->y;

  // spawn a teleport fog at the new spot

  ss = R_PointInSubsector (x,y);
  mo = P_SpawnMobj (x, y, ss->sector->floorheight , MT_IFOG);
  S_StartSound (mo, sfx_itmbk);

  // find which type to spawn

  /* killough 8/23/98: use table for faster lookup */
  i = P_FindDoomedNum(mthing->type);

  // spawn it

  if (mobjinfo[i].flags & MF_SPAWNCEILING)
    z = ONCEILINGZ;
  else
    z = ONFLOORZ;

  mo = P_SpawnMobj (x,y,z, i);
  mo->spawnpoint = *mthing;
  mo->angle = ANG45 * (mthing->angle/45);

  // pull it from the queue

  iquetail = (iquetail+1)&(ITEMQUESIZE-1);
}

//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//

extern byte playernumtotrans[MAX_MAXPLAYERS];

void P_SpawnPlayer (int n, const mapthing_t* mthing)
{
  player_t* p;
  fixed_t   x;
  fixed_t   y;
  fixed_t   z;
  mobj_t*   mobj;
  int       i;

  // e6y
  // playeringame overflow detection
  // it detects and emulates overflows on vex6d.wad\bug_wald(toke).lmp, etc.
  // http://www.doom2.net/doom2/research/runningbody.zip
  if (PlayeringameOverrun(mthing))
    return;

  // not playing?

  if (!playeringame[n])
    return;

  p = &players[n];

  if (p->playerstate == PST_REBORN)
    G_PlayerReborn (n);

  /* cph 2001/08/14 - use the options field of memorised player starts to
   * indicate whether the start really exists in the level.
   */
  if (!mthing->options)
    I_Error("P_SpawnPlayer: attempt to spawn player at unavailable start point");

  x    = mthing->x;
  y    = mthing->y;
  z    = ONFLOORZ;

  if (hexen)
  {
    if (randomclass && deathmatch)
    {
      p->pclass = 1 + P_Random(pr_hexen) % 3;
      if (p->pclass == PlayerClass[mthing->type - 1])
      {
        p->pclass = 1 + ((p->pclass + 1) % 3);
      }
      PlayerClass[mthing->type - 1] = p->pclass;
      SB_SetClassData();
    }
    else
    {
      p->pclass = PlayerClass[mthing->type - 1];
    }
    switch (p->pclass)
    {
      case PCLASS_FIGHTER:
        mobj = P_SpawnMobj(x, y, z, HEXEN_MT_PLAYER_FIGHTER);
        break;
      case PCLASS_CLERIC:
        mobj = P_SpawnMobj(x, y, z, HEXEN_MT_PLAYER_CLERIC);
        break;
      case PCLASS_MAGE:
        mobj = P_SpawnMobj(x, y, z, HEXEN_MT_PLAYER_MAGE);
        break;
      default:
        I_Error("P_SpawnPlayer: Unknown class type");
        return;
    }
  }
  else
    mobj = P_SpawnMobj(x,y,z, g_mt_player);

  if (map_info.flags & MI_USE_PLAYER_START_Z)
    mobj->z += mthing->height;

  // set color translations for player sprites
  if (hexen)
  {
    if (p->pclass == PCLASS_FIGHTER && (mthing->type == 1 || mthing->type == 3))
    {
      // The first type should be blue, and the third should be the
      // Fighter's original gold color
      if (mthing->type == 1)
      {
        mobj->flags |= MF_TRANSLATION2;
      }
    }
    else if (mthing->type > 1)
    {                           // Set color translation bits for player sprites
      mobj->flags |= (mthing->type - 1) << MF_TRANSSHIFT;
    }
  }
  else
    mobj->flags |= playernumtotrans[n]<<MF_TRANSSHIFT;

  if (leave_data.flags & LF_SET_ANGLE)
    mobj->angle = leave_data.angle;
  else
    mobj->angle = ANG45 * (mthing->angle/45);

  mobj->player     = p;
  mobj->health     = p->health;
  mobj->player->prev_viewangle = mobj->angle;

  p->mo            = mobj;
  p->playerstate   = PST_LIVE;
  p->refire        = 0;
  p->damagecount   = 0;
  p->bonuscount    = 0;
  p->poisoncount   = 0;
  p->chickenTics   = 0;
  p->morphTics     = 0;
  p->rain1         = NULL;
  p->rain2         = NULL;
  p->extralight    = 0;
  p->fixedcolormap = 0;
  p->viewheight    = g_viewheight;

  p->momx = p->momy = 0;   // killough 10/98: initialize bobbing to 0.

  // setup gun psprite

  P_SetupPsprites (p);

  // give all cards in death match mode

  if (deathmatch)
  {
    for (i = 0 ; i < NUMCARDS ; i++)
      p->cards[i] = true;
    if (p == &players[consoleplayer])
      playerkeys = 7;
  }
  else if (p == &players[consoleplayer])
    playerkeys = 0;

  R_SmoothPlaying_Reset(p); // e6y
}

/*
 * P_IsDoomnumAllowed()
 * Based on code taken from P_LoadThings() in src/p_setup.c  Return TRUE
 * if the thing in question is expected to be available in the gamemode used.
 */

dboolean P_IsDoomnumAllowed(int doomnum)
{
  // Do not spawn cool, new monsters if !commercial
  if (!raven && gamemode != commercial)
    switch(doomnum)
      {
      case 64:  // Archvile
      case 65:  // Former Human Commando
      case 66:  // Revenant
      case 67:  // Mancubus
      case 68:  // Arachnotron
      case 69:  // Hell Knight
      case 71:  // Pain Elemental
      case 84:  // Wolf SS
      case 88:  // Boss Brain
      case 89:  // Boss Shooter
        return false;
      }

  return true;
}

//
// P_SpawnMapThing
// The fields of the mapthing should
// already be in host byte order.
//

static dboolean P_ShouldSpawnPlayer(const mapthing_t* mthing)
{
  return !deathmatch && (map_format.zdoom ?
                         mthing->special_args[0] == leave_data.position :
                         !mthing->special_args[0]);
}

static dboolean P_ShouldSpawnMapThing(int options)
{
  unsigned int spawnMask;
  dboolean spawn_multi;

  spawn_multi = skill_info.flags & SI_SPAWN_MULTI;

  if (map_format.hexen)
  {
    // Check current game type with spawn flags
    if (netgame == false)
    {
      spawnMask = MTF_GSINGLE;

      if (spawn_multi)
        spawnMask |= MTF_GCOOP;
    }
    else if (deathmatch)
    {
      spawnMask = MTF_GDEATHMATCH;
    }
    else
    {
      spawnMask = MTF_GCOOP;
    }

    if (!(options & spawnMask))
    {
      return false;
    }
  }
  else
  {
    /* jff "not single" thing flag */
    if (!spawn_multi && !netgame && options & MTF_NOTSINGLE)
      return false;

    //jff 3/30/98 implement "not deathmatch" thing flag
    if (netgame && deathmatch && options & MTF_NOTDM)
      return false;

    //jff 3/30/98 implement "not cooperative" thing flag
    if ((spawn_multi || netgame) && !deathmatch && options & MTF_NOTCOOP)
      return false;
  }

  // check for appropriate skill level
  if (
    skill_info.spawn_filter == 1 ? !(options & MTF_SKILL1) :
    skill_info.spawn_filter == 2 ? !(options & MTF_SKILL2) :
    skill_info.spawn_filter == 3 ? !(options & MTF_SKILL3) :
    skill_info.spawn_filter == 4 ? !(options & MTF_SKILL4) :
                                   !(options & MTF_SKILL5)
  )
    return false;

  if (hexen)
  {
    static unsigned int classFlags[] = {
      0, // null class
      MTF_FIGHTER,
      MTF_CLERIC,
      MTF_MAGE
    };

    int i;

    // Check current character classes with spawn flags
    if (netgame == false)
    {                           // Single player
      if ((options & classFlags[PlayerClass[0]]) == 0)
      {                       // Not for current class
        return false;
      }
    }
    else if (deathmatch == false)
    {                           // Cooperative
      spawnMask = 0;
      for (i = 0; i < g_maxplayers; i++)
      {
        if (playeringame[i])
        {
          spawnMask |= classFlags[PlayerClass[i]];
        }
      }
      if ((options & spawnMask) == 0)
      {
        return false;
      }
    }
  }

  return true;
}

void P_TrySpawnPlayer(const mapthing_t *mthing, int player)
{
  mapthing_t *player_start;

  player_start = &playerstarts[mthing->special_args[0]][player];
  *player_start = *mthing;
  player_start->type = player + 1;

  /* cph 2006/07/24 - use the otherwise-unused options field to flag that
   * this start is present (so we know which elements of the array are filled
   * in, in effect). Also note that the call below to P_SpawnPlayer must use
   * the playerstarts version with this field set */
  player_start->options = 1;

  if (P_ShouldSpawnPlayer(mthing))
    P_SpawnPlayer(player, player_start);
}

static int P_TypeToPlayer(int type)
{
  if (type <= 4 && type > 0)
    return type - 1;

  if (map_format.hexen && type >= 9100 && type <= 9103)
    return 4 + type - 9100;

  return -1;
}

mobj_t* P_SpawnMapThing (const mapthing_t* mthing, int index)
{
  int     i;
  int player;
  mobj_t* mobj;
  fixed_t x;
  fixed_t y;
  fixed_t z;
  int options = mthing->options; /* cph 2001/07/07 - make writable copy */
  short thingtype = mthing->type;
  int iden_num = 0;

  // killough 2/26/98: Ignore type-0 things as NOPs
  // phares 5/14/98: Ignore Player 5-8 starts (for now)

  if (!hexen)
  {
    switch(thingtype)
    {
      case 0:
      case DEN_PLAYER5:
      case DEN_PLAYER6:
      case DEN_PLAYER7:
      case DEN_PLAYER8:
        return NULL;
    }
  }

  // killough 11/98: clear flags unused by Doom
  //
  // We clear the flags unused in Doom if we see flag mask 256 set, since
  // it is reserved to be 0 under the new scheme. A 1 in this reserved bit
  // indicates it's a Doom wad made by a Doom editor which puts 1's in
  // bits that weren't used in Doom (such as HellMaker wads). So we should
  // then simply ignore all upper bits.

  if (
    !map_format.hexen &&
    (
      demo_compatibility ||
      (
        compatibility_level >= lxdoom_1_compatibility &&
        options & MTF_RESERVED
      )
    )
  )
  {
    if (!demo_compatibility) // cph - Add warning about bad thing flags
      lprintf(LO_WARN, "P_SpawnMapThing: correcting bad flags (%u) (thing type %d)\n", options, thingtype);
    options &= MTF_SKILL1|MTF_SKILL2|MTF_SKILL3|MTF_SKILL4|MTF_SKILL5|MTF_AMBUSH|MTF_NOTSINGLE;
  }

  // count deathmatch start positions

  // doom2.exe has at most 10 deathmatch starts
  if (thingtype == 11)
  {
    if (compatibility && deathmatch_p - deathmatchstarts >= 10)
    {
  		return NULL;
    }
    else
    {
      // 1/11/98 killough -- new code removes limit on deathmatch starts:

      size_t offset = deathmatch_p - deathmatchstarts;

      if (offset >= num_deathmatchstarts)
      {
        num_deathmatchstarts = num_deathmatchstarts ?
                               num_deathmatchstarts * 2 : 16;
        deathmatchstarts = Z_Realloc(deathmatchstarts,
                                   num_deathmatchstarts *
                                   sizeof(*deathmatchstarts));
        deathmatch_p = deathmatchstarts + offset;
      }
      memcpy(deathmatch_p++, mthing, sizeof(*mthing));
      (deathmatch_p - 1)->options = 1;

      return NULL;
  	}
  }

  if (PO_Detect(mthing->type))
  {
    return NULL;
  }

  // check for players specially
  if ((player = P_TypeToPlayer(thingtype)) >= 0)
  {
    if (map_info.flags & MI_FILTER_STARTS)
      if (!P_ShouldSpawnMapThing(options))
        return NULL;

    // killough 7/19/98: Marine's best friend :)
    if (
      !netgame &&
      player > 0 && player <= dogs &&
      !players[player].secretcount
    )
    {  // use secretcount to avoid multiple dogs in case of multiple starts
      players[player].secretcount = 1;

      // killough 10/98: force it to be a friend
      options |= (map_format.zdoom ? MTF_FRIENDLY : MTF_FRIEND);
      if (HelperThing != -1) // haleyjd 9/22/99: deh substitution
      {
        int type = HelperThing - 1;
        if (type >= 0 && type < num_mobj_types)
        {
          i = type;
        }
        else
        {
          doom_printf("Invalid value %i for helper, ignored.", HelperThing);
          i = MT_DOGS;
        }
      }
      else {
        i = MT_DOGS;
      }
      goto spawnit;
    }

    P_TrySpawnPlayer(mthing, player);

    return NULL;
  }

  // MAP_FORMAT_TODO: need to verify heretic types
  if (heretic)
  {
    // Ambient sound sequences
    if (mthing->type >= 1200 && mthing->type < 1300)
    {
      P_AddAmbientSfx(mthing->type - 1200);
      return NULL;
    }

    // Check for boss spots
    if (mthing->type == 56)     // Monster_BossSpot
    {
      P_AddBossSpot(mthing->x, mthing->y,
                    ANG45 * (mthing->angle / 45));
      return NULL;
    }
  }

  if (map_format.hexen)
  {
    if (mthing->type >= 1400 && mthing->type < 1410)
    {
      R_PointInSubsector(
        mthing->x, mthing->y
      )->sector->seqType = mthing->type - 1400;
      return NULL;
    }
  }

  if (!P_ShouldSpawnMapThing(options))
    return NULL;

  if (!raven && thingtype >= 14100 && thingtype <= 14164)
  {
    // Use the ambient number
    iden_num = thingtype - 14100; // Mus change
    thingtype = 14164;            // MT_MUSICSOURCE
  }

  if (!raven && thingtype == 14165 && map_format.hexen)
  {
    // Use the ambient number
    iden_num = BETWEEN(0, 64, mthing->special_args[0]); // Mus change
    thingtype = 14164;            // MT_MUSICSOURCE
  }

  // find which type to spawn

  // killough 8/23/98: use table for faster lookup
  i = P_FindDoomedNum(thingtype);

  // phares 5/16/98:
  // Do not abort because of an unknown thing. Ignore it, but post a
  // warning message for the player.

  if (i == num_mobj_types)
  {
    lprintf(LO_INFO, "P_SpawnMapThing: Unknown Thing type %i at (%i, %i)\n", thingtype, mthing->x, mthing->y);
    return NULL;
  }

  // don't spawn keycards and players in deathmatch

  if (deathmatch && mobjinfo[i].flags & MF_NOTDMATCH)
    return NULL;

  // don't spawn any monsters if -nomonsters

  if (nomonsters && (i == MT_SKULL || (mobjinfo[i].flags & MF_COUNTKILL)))
    return NULL;

  // spawn it
spawnit:

  if (heretic && i == HERETIC_MT_WMACE)
  {
    P_AddMaceSpot(mthing);
    return NULL;
  }

  x = mthing->x;
  y = mthing->y;

  if (mobjinfo[i].flags & MF_SPAWNCEILING)
    z = ONCEILINGZ;
  else if (mobjinfo[i].flags2 & MF2_SPAWNFLOAT)
    z = FLOATRANDZ;
  else if (hexen && mobjinfo[i].flags2 & MF2_FLOATBOB)
    z = mthing->height;
  else
    z = ONFLOORZ;

  if (hexen && i == HEXEN_MT_ZLYNCHED_NOHEART)
    P_SpawnMobj(x, y, ONFLOORZ, HEXEN_MT_BLOODPOOL);

  mobj = P_SpawnMobj (x, y, z, i);

  if (!(mobj->flags & MF_FRIEND) &&
      options & (map_format.zdoom ? MTF_FRIENDLY : MTF_FRIEND) &&
      mbf_features)
  {
    mobj->flags |= MF_FRIEND;            // killough 10/98:
    P_UpdateThinker(&mobj->thinker);     // transfer friendliness flag

    // Friends can have a different spawn health
    mobj->health = P_MobjSpawnHealth(mobj);
  }

  if (mthing->health != FRACUNIT)
  {
    if (mthing->health < 0)
      mobj->health = -mthing->health >> FRACBITS;
    else
      mobj->health = FixedMul(mobj->health, mthing->health);
  }

  if (mthing->gravity != FRACUNIT)
  {
    if (mthing->gravity < 0)
      mobj->gravity = -mthing->gravity;
    else
      mobj->gravity = FixedMul(map_info.gravity, mthing->gravity);
  }

  mobj->alpha = mthing->alpha;
  if (mobj->alpha < 1.f)
    mobj->tranmap = dsda_TranMap(dsda_FloatToPercent(mobj->alpha));
  else
    mobj->tranmap = NULL;

  mobj->spawnpoint = *mthing; // heretic_note: this is only done with totalkills++ in heretic
  mobj->index = index;//e6y
  mobj->iden_nums = iden_num;

  if (map_format.hexen)
  {
    if (z == ONFLOORZ)
    {
      mobj->z += mthing->height;
    }
    else if (z == ONCEILINGZ)
    {
      mobj->z -= mthing->height;
    }
    mobj->tid = mthing->tid;
    mobj->special = mthing->special;
    mobj->special_args[0] = mthing->special_args[0];
    mobj->special_args[1] = mthing->special_args[1];
    mobj->special_args[2] = mthing->special_args[2];
    mobj->special_args[3] = mthing->special_args[3];
    mobj->special_args[4] = mthing->special_args[4];
  }

  if (mobj->flags2 & MF2_FLOATBOB)
  {                           // Seed random starting index for bobbing motion
    mobj->health = P_Random(pr_heretic);
    if (hexen) mobj->special1.i = mthing->height;
  }

  if (mobj->tics > 0)
    mobj->tics = 1 + (P_Random(pr_spawnthing) % mobj->tics);

  if (map_format.zdoom)
  {
    if (options & MTF_TRANSLUCENT)
      mobj->flags |= MF_TRANSLUCENT;

    if (options & MTF_INVISIBLE)
    {
      P_UnsetThingPosition(mobj);
      mobj->flags |= MF_NOSECTOR;
      P_SetThingPosition(mobj);
    }

    if (options & MTF_COUNTSECRET)
      P_AddMobjSecret(mobj);
  }

  /* killough 7/20/98: exclude friends */
  if (!((mobj->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    totalkills++;

  if (mobj->flags & MF_COUNTITEM)
    totalitems++;

  if (map_format.hexen)
  {
    if (mobj->flags & MF_COUNTKILL)
    {
      // Quantize angle to 45 degree increments
      mobj->angle = ANG45 * (mthing->angle / 45);
    }
    else
    {
      // Scale angle correctly (source is 0..359)
      mobj->angle = ((mthing->angle << 8) / 360) << 24;
    }
  }
  else
    mobj->angle = ANG45 * (mthing->angle / 45);

  if (options & MTF_AMBUSH)
    mobj->flags |= MF_AMBUSH;

  if (map_format.hexen && mthing->options & MTF_DORMANT)
  {
      mobj->flags2 |= MF2_DORMANT;
      if (hexen && mobj->type == HEXEN_MT_ICEGUY)
      {
        P_SetMobjState(mobj, HEXEN_S_ICEGUY_DORMANT);
      }
      mobj->tics = -1;
  }

  return mobj;
}

//
// GAME SPAWN FUNCTIONS
//

//
// P_SpawnPuff
//

extern fixed_t attackrange;

void P_SpawnPuff(fixed_t x,fixed_t y,fixed_t z)
{
  mobj_t* th;
  int t;

  if (raven) return Raven_P_SpawnPuff(x, y, z);

  // killough 5/5/98: remove dependence on order of evaluation:
  t = P_Random(pr_spawnpuff);
  z += (t - P_Random(pr_spawnpuff))<<10;

  th = P_SpawnMobj (x,y,z, MT_PUFF);
  th->momz = FRACUNIT;
  th->tics -= P_Random(pr_spawnpuff)&3;

  if (th->tics < 1)
    th->tics = 1;

  // don't make punches spark on the wall

  if (attackrange == MELEERANGE)
    P_SetMobjState (th, S_PUFF3);
}


//
// P_SpawnBlood
//
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage, mobj_t *bleeder)
{
  mobj_t* th;
  // killough 5/5/98: remove dependence on order of evaluation:
  int t = P_Random(pr_spawnblood);
  z += (t - P_Random(pr_spawnblood))<<10;
  th = P_SpawnMobj(x,y,z, MT_BLOOD);
  th->momz = FRACUNIT*2;
  th->tics -= P_Random(pr_spawnblood)&3;
  th->color = bleeder->info->bloodcolor;

  if (th->tics < 1)
    th->tics = 1;

  if (damage <= 12 && damage >= 9)
    P_SetMobjState (th,S_BLOOD2);
  else if (damage < 9)
    P_SetMobjState (th,S_BLOOD3);
}


//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//

dboolean P_CheckMissileSpawn (mobj_t* th)
{
  if (!raven) {
    th->tics -= P_Random(pr_missile)&3;
    if (th->tics < 1)
      th->tics = 1;
  }

  // move a little forward so an angle can
  // be computed if it immediately explodes

  if (heretic && th->type == HERETIC_MT_BLASTERFX1) {
    // Ultra-fast ripper spawning missile
    th->x += (th->momx >> 3);
    th->y += (th->momy >> 3);
    th->z += (th->momz >> 3);
  }
  else {
    th->x += (th->momx>>1);
    th->y += (th->momy>>1);
    th->z += (th->momz>>1);
  }

  // killough 8/12/98: for non-missile objects (e.g. grenades)
  if (!(th->flags & MF_MISSILE) && mbf_features)
    return true;

  // killough 3/15/98: no dropoff (really = don't care for missiles)

  if (!P_TryMove (th, th->x, th->y, false)) {
    P_ExplodeMissile (th);
    return false;
  }

  return true;
}


//
// P_SpawnMissile
//

mobj_t* P_SpawnMissile(mobj_t* source,mobj_t* dest,mobjtype_t type)
{
  fixed_t z;
  mobj_t* th;
  angle_t an;
  int     dist;

  if (!raven)
  {
    z = source->z + 32 * FRACUNIT;
  }
  else
  {
    switch (type)
    {
      case HERETIC_MT_MNTRFX1:       // Minotaur swing attack missile
      case HEXEN_MT_MNTRFX1:         // Minotaur swing attack missile
      case HEXEN_MT_ICEGUY_FX:
      case HEXEN_MT_HOLY_MISSILE:
        z = source->z + 40 * FRACUNIT;
        break;
      case HERETIC_MT_MNTRFX2:       // Minotaur floor fire missile
        z = ONFLOORZ;
        break;
      case HEXEN_MT_MNTRFX2:         // Minotaur floor fire missile
        z = ONFLOORZ + source->floorclip;
        break;
      case HERETIC_MT_SRCRFX1:       // Sorcerer Demon fireball
        z = source->z + 48 * FRACUNIT;
        break;
      case HERETIC_MT_KNIGHTAXE:     // Knight normal axe
      case HERETIC_MT_REDAXE:        // Knight red power axe
        z = source->z + 36 * FRACUNIT;
        break;
      case HEXEN_MT_CENTAUR_FX:
        z = source->z + 45 * FRACUNIT;
        break;
      default:
        z = source->z + 32 * FRACUNIT;
        break;
    }

    if (hexen)
      z -= source->floorclip;
    else if (source->flags2 & MF2_FEETARECLIPPED)
      z -= FOOTCLIPSIZE;
  }

  th = P_SpawnMobj(source->x, source->y, z, type);

  if (th->info->seesound)
    S_StartMobjSound(th, th->info->seesound);

  P_SetTarget(&th->target, source);    // where it came from
  an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);

  // fuzzy player
  if (dest->flags & MF_SHADOW)
  {  // killough 5/5/98: remove dependence on order of evaluation:
    int t = P_Random(pr_shadow);
    an += (t - P_Random(pr_shadow)) << g_fuzzy_aim_shift;
  }

  th->angle = an;
  an >>= ANGLETOFINESHIFT;
  th->momx = FixedMul(th->info->speed, finecosine[an]);
  th->momy = FixedMul(th->info->speed, finesine[an]);

  dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
  dist = dist / th->info->speed;

  if (dist < 1)
    dist = 1;

  th->momz = (dest->z - source->z) / dist;

  if (!raven)
  {
    P_CheckMissileSpawn(th);
    return th;
  }

  return (P_CheckMissileSpawn(th) ? th : NULL);
}


//
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster
//

mobj_t* P_SpawnPlayerMissile(mobj_t* source, mobjtype_t type)
{
  mobj_t *th;
  fixed_t x, y, z, slope = 0;

  // see which target is to be aimed at

  angle_t an = source->angle;

  // killough 7/19/98: autoaiming was not in original beta
  if (comperr(comperr_freeaim))
    slope = finetangent[(ANG90 - source->pitch) >> ANGLETOFINESHIFT];
  else
  {
    // killough 8/2/98: prefer autoaiming at enemies
    uint64_t mask = mbf_features ? MF_FRIEND : 0;

    do
    {
      slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, mask);
      if (!linetarget)
        slope = P_AimLineAttack(source, an += 1 << 26, 16 * 64 * FRACUNIT, mask);
      if (!linetarget)
        slope = P_AimLineAttack(source, an -= 2 << 26, 16 * 64 * FRACUNIT, mask);
      if (!linetarget) {
        an = source->angle;
        slope = 0;

        if (raven) slope = ((source->player->lookdir) << FRACBITS) / 173;
      }
    }
    while (mask && (mask=0, !linetarget));  // killough 8/2/98
  }

  x = source->x;
  y = source->y;

  if (!raven)
  {
    z = source->z + 4 * 8 * FRACUNIT;
  }
  else
  {
    if (type == HEXEN_MT_LIGHTNING_FLOOR)
    {
      z = ONFLOORZ;
      slope = 0;
    }
    else if (type == HEXEN_MT_LIGHTNING_CEILING)
    {
      z = ONCEILINGZ;
      slope = 0;
    }
    else
    {
      z = source->z + 4 * 8 * FRACUNIT;

      z += ((source->player->lookdir) << FRACBITS) / 173;

      if (hexen)
      {
        z -= source->floorclip;
      }
      else if (source->flags2 & MF2_FEETARECLIPPED)
      {
        z -= FOOTCLIPSIZE;
      }
    }
  }

  // heretic global MissileMobj
  MissileMobj = th = P_SpawnMobj(x, y, z, type);

  if (!hexen && th->info->seesound)
    S_StartMobjSound(th, th->info->seesound);

  P_SetTarget(&th->target, source);
  th->angle = an;
  th->momx = FixedMul(th->info->speed, finecosine[an>>ANGLETOFINESHIFT]);
  th->momy = FixedMul(th->info->speed, finesine[an>>ANGLETOFINESHIFT]);
  th->momz = FixedMul(th->info->speed, slope);

  if (hexen)
  {
    if (th->type == HEXEN_MT_MWAND_MISSILE || th->type == HEXEN_MT_CFLAME_MISSILE)
    {                           // Ultra-fast ripper spawning missile
      th->x += (th->momx >> 3);
      th->y += (th->momy >> 3);
      th->z += (th->momz >> 3);
    }
    else
    {                           // Normal missile
      th->x += (th->momx >> 1);
      th->y += (th->momy >> 1);
      th->z += (th->momz >> 1);
    }
    if (!P_TryMove(th, th->x, th->y, false))
    {                           // Exploded immediately
      P_ExplodeMissile(th);
      return (NULL);
    }
    return (th);
  }

  // heretic - return missile if it's ok
  return P_CheckMissileSpawn(th) ? th : NULL;
}

// heretic

#include "p_spec.h"

mobjtype_t PuffType;
mobj_t *MissileMobj;

void P_BlasterMobjThinker(mobj_t * mobj)
{
    int i;
    fixed_t xfrac;
    fixed_t yfrac;
    fixed_t zfrac;
    fixed_t z;
    dboolean changexy;

    mobj->PrevX = mobj->x;
    mobj->PrevY = mobj->y;
    mobj->PrevZ = mobj->z;

    // Handle movement
    if (mobj->momx || mobj->momy || (mobj->z != mobj->floorz) || mobj->momz)
    {
        xfrac = mobj->momx >> 3;
        yfrac = mobj->momy >> 3;
        zfrac = mobj->momz >> 3;
        changexy = xfrac || yfrac;
        for (i = 0; i < 8; i++)
        {
            if (changexy)
            {
                if (!P_TryMove(mobj, mobj->x + xfrac, mobj->y + yfrac, false))
                {               // Blocked move
                    P_ExplodeMissile(mobj);
                    return;
                }
            }
            mobj->z += zfrac;
            if (mobj->z <= mobj->floorz)
            {                   // Hit the floor
                mobj->z = mobj->floorz;
                P_HitFloor(mobj);
                P_ExplodeMissile(mobj);
                return;
            }
            if (mobj->z + mobj->height > mobj->ceilingz)
            {                   // Hit the ceiling
                mobj->z = mobj->ceilingz - mobj->height;
                P_ExplodeMissile(mobj);
                return;
            }
            if (changexy)
            {
                if (hexen)
                {
                    if (mobj->type == HEXEN_MT_MWAND_MISSILE && (P_Random(pr_hexen) < 128))
                    {
                        z = mobj->z - 8 * FRACUNIT;
                        if (z < mobj->floorz)
                        {
                            z = mobj->floorz;
                        }
                        P_SpawnMobj(mobj->x, mobj->y, z, HEXEN_MT_MWANDSMOKE);
                    }
                    else if (!--mobj->special1.i)
                    {
                        mobj_t *mo;
                        mobj->special1.i = 4;
                        z = mobj->z - 12 * FRACUNIT;
                        if (z < mobj->floorz)
                        {
                            z = mobj->floorz;
                        }
                        mo = P_SpawnMobj(mobj->x, mobj->y, z, HEXEN_MT_CFLAMEFLOOR);
                        if (mo)
                        {
                            mo->angle = mobj->angle;
                        }
                    }
                }
                else if (P_Random(pr_heretic) < 64)
                {
                    z = mobj->z - 8 * FRACUNIT;
                    if (z < mobj->floorz)
                    {
                        z = mobj->floorz;
                    }
                    P_SpawnMobj(mobj->x, mobj->y, z, HERETIC_MT_BLASTERSMOKE);
                }
            }
        }
    }
    // Advance the state
    if (mobj->tics != -1)
    {
        mobj->tics--;
        while (!mobj->tics)
        {
            if (!P_SetMobjState(mobj, mobj->state->nextstate))
            {                   // mobj was removed
                return;
            }
        }
    }
}

void A_ContMobjSound(mobj_t * actor)
{
    switch (actor->type)
    {
        case HERETIC_MT_KNIGHTAXE:
            S_StartMobjSound(actor, heretic_sfx_kgtatk);
            break;
        case HERETIC_MT_MUMMYFX1:
            S_StartMobjSound(actor, heretic_sfx_mumhed);
            break;
        case HEXEN_MT_SERPENTFX:
            S_StartMobjSound(actor, hexen_sfx_serpentfx_continuous);
            break;
        case HEXEN_MT_HAMMER_MISSILE:
            S_StartMobjSound(actor, hexen_sfx_fighter_hammer_continuous);
            break;
        case HEXEN_MT_QUAKE_FOCUS:
            S_StartMobjSound(actor, hexen_sfx_earthquake);
            break;
        default:
            break;
    }
}

mobj_t *P_SpawnMissileAngle(mobj_t * source, mobjtype_t type, angle_t angle, fixed_t momz)
{
    fixed_t z;
    mobj_t *mo;

    switch (type)
    {
        case HERETIC_MT_MNTRFX1:       // Minotaur swing attack missile
        case HEXEN_MT_MNTRFX1:         // Minotaur swing attack missile
        case HEXEN_MT_MSTAFF_FX2:
            z = source->z + 40 * FRACUNIT;
            break;
        case HEXEN_MT_MNTRFX2:       // Minotaur floor fire missile
            z = ONFLOORZ + source->floorclip;
            break;
        case HERETIC_MT_MNTRFX2:       // Minotaur floor fire missile
            z = ONFLOORZ;
            break;
        case HERETIC_MT_SRCRFX1:       // Sorcerer Demon fireball
            z = source->z + 48 * FRACUNIT;
            break;
        case HEXEN_MT_ICEGUY_FX2:    // Secondary Projectiles of the Ice Guy
            z = source->z + 3 * FRACUNIT;
            break;
        default:
            z = source->z + 32 * FRACUNIT;
            break;
    }

    if (hexen)
    {
        z -= source->floorclip;
    }
    else if (source->flags2 & MF2_FEETARECLIPPED)
    {
        z -= FOOTCLIPSIZE;
    }

    mo = P_SpawnMobj(source->x, source->y, z, type);
    if (mo->info->seesound)
    {
        S_StartMobjSound(mo, mo->info->seesound);
    }
    P_SetTarget(&mo->target, source); // Originator
    mo->angle = angle;
    angle >>= ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[angle]);
    mo->momy = FixedMul(mo->info->speed, finesine[angle]);
    mo->momz = momz;
    return (P_CheckMissileSpawn(mo) ? mo : NULL);
}

dboolean P_SetMobjStateNF(mobj_t * mobj, statenum_t state)
{
    state_t *st;

    if (state == g_s_null)
    {                           // Remove mobj
        mobj->state = NULL;
        P_RemoveMobj(mobj);
        return (false);
    }
    st = &states[state];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;
    return (true);
}

void P_ThrustMobj(mobj_t * mo, angle_t angle, fixed_t move)
{
    angle >>= ANGLETOFINESHIFT;
    mo->momx += FixedMul(move, finecosine[angle]);
    mo->momy += FixedMul(move, finesine[angle]);
}

dboolean P_SeekerMissile(mobj_t * actor, mobj_t ** seekTarget, angle_t thresh, angle_t turnMax, dboolean seekcenter)
{
    int dir;
    int dist;
    angle_t delta;
    angle_t angle;
    mobj_t *target;

    target = *seekTarget;
    if (target == NULL)
    {
        return (false);
    }
    if (!(target->flags & MF_SHOOTABLE))
    {                           // Target died
        *seekTarget = NULL;
        return (false);
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
    if (actor->z + actor->height < target->z ||
        target->z + target->height < actor->z || seekcenter)
    {                           // Need to seek vertically
        dist = P_AproxDistance(target->x - actor->x, target->y - actor->y);
        dist = dist / actor->info->speed;
        if (dist < 1)
        {
            dist = 1;
        }
        if (hexen)
          actor->momz = (target->z + (target->height >> 1) - (actor->z + (actor->height >> 1))) / dist;
        else
          actor->momz = (target->z + (seekcenter ? target->height/2 : 0) - actor->z) / dist;
    }
    return (true);
}

mobj_t *P_SPMAngle(mobj_t * source, mobjtype_t type, angle_t angle)
{
    mobj_t *th;
    angle_t an;
    fixed_t x, y, z, slope;

    //
    // see which target is to be aimed at
    //
    an = angle;
    slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, 0);
    if (!linetarget)
    {
        an += 1 << 26;
        slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, 0);
        if (!linetarget)
        {
            an -= 2 << 26;
            slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, 0);
        }
        if (!linetarget)
        {
            an = angle;
            slope = ((source->player->lookdir) << FRACBITS) / 173;
        }
    }
    x = source->x;
    y = source->y;
    z = source->z + 4 * 8 * FRACUNIT +
        ((source->player->lookdir) << FRACBITS) / 173;
    if (hexen)
    {
        z -= source->floorclip;
    }
    else if (source->flags2 & MF2_FEETARECLIPPED)
    {
        z -= FOOTCLIPSIZE;
    }
    th = P_SpawnMobj(x, y, z, type);
    if (!hexen && th->info->seesound)
    {
        S_StartMobjSound(th, th->info->seesound);
    }
    P_SetTarget(&th->target, source);
    th->angle = an;
    th->momx = FixedMul(th->info->speed, finecosine[an >> ANGLETOFINESHIFT]);
    th->momy = FixedMul(th->info->speed, finesine[an >> ANGLETOFINESHIFT]);
    th->momz = FixedMul(th->info->speed, slope);
    return (P_CheckMissileSpawn(th) ? th : NULL);
}

static int Hexen_P_HitFloor(mobj_t * thing);

int P_HitFloor(mobj_t * thing)
{
    mobj_t *mo;

    if (thing->floorz != thing->subsector->sector->floorheight)
    {                           // don't splash if landing on the edge above water/lava/etc....
        return (FLOOR_SOLID);
    }

    if (hexen) return Hexen_P_HitFloor(thing);

    switch (P_GetThingFloorType(thing))
    {
        case FLOOR_WATER:
            P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HERETIC_MT_SPLASHBASE);
            mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HERETIC_MT_SPLASH);
            P_SetTarget(&mo->target, thing);
            mo->momx = P_SubRandom() << 8;
            mo->momy = P_SubRandom() << 8;
            mo->momz = 2 * FRACUNIT + (P_Random(pr_heretic) << 8);
            S_StartMobjSound(mo, heretic_sfx_gloop);
            return (FLOOR_WATER);
        case FLOOR_LAVA:
            P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HERETIC_MT_LAVASPLASH);
            mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HERETIC_MT_LAVASMOKE);
            mo->momz = FRACUNIT + (P_Random(pr_heretic) << 7);
            S_StartMobjSound(mo, heretic_sfx_burn);
            return (FLOOR_LAVA);
        case FLOOR_SLUDGE:
            P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HERETIC_MT_SLUDGESPLASH);
            mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HERETIC_MT_SLUDGECHUNK);
            P_SetTarget(&mo->target, thing);
            mo->momx = P_SubRandom() << 8;
            mo->momy = P_SubRandom() << 8;
            mo->momz = FRACUNIT + (P_Random(pr_heretic) << 8);
            return (FLOOR_SLUDGE);
    }
    return (FLOOR_SOLID);
}

int P_GetThingFloorType(mobj_t * thing)
{
    if (hexen && thing->floorpic)
    {
        return (TerrainTypes[thing->floorpic]);
    }
    else
    {
        return (TerrainTypes[thing->subsector->sector->floorpic]);
    }
}

// Returns 1 if 'source' needs to turn clockwise, or 0 if 'source' needs
// to turn counter clockwise.  'delta' is set to the amount 'source'
// needs to turn.
int P_FaceMobj(mobj_t * source, mobj_t * target, angle_t * delta)
{
    angle_t diff;
    angle_t angle1;
    angle_t angle2;

    angle1 = source->angle;
    angle2 = R_PointToAngle2(source->x, source->y, target->x, target->y);
    if (angle2 > angle1)
    {
        diff = angle2 - angle1;
        if (diff > ANG180)
        {
            *delta = ANGLE_MAX - diff;
            return (0);
        }
        else
        {
            *delta = diff;
            return (1);
        }
    }
    else
    {
        diff = angle1 - angle2;
        if (diff > ANG180)
        {
            *delta = ANGLE_MAX - diff;
            return (1);
        }
        else
        {
            *delta = diff;
            return (0);
        }
    }
}

dboolean Raven_P_SetMobjState(mobj_t * mobj, statenum_t state)
{
    state_t *st;

    if (state == g_s_null)
    {                           // Remove mobj
        mobj->state = NULL;
        P_RemoveMobj(mobj);
        return (false);
    }
    st = &states[state];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;
    if (st->action)
    {                           // Call action function
        st->action(mobj);
    }
    return (true);
}

void P_FloorBounceMissile(mobj_t * mo)
{
    if (hexen)
    {
        if (P_HitFloor(mo) >= FLOOR_LIQUID)
        {
            switch (mo->type)
            {
                case HEXEN_MT_SORCFX1:
                case HEXEN_MT_SORCBALL1:
                case HEXEN_MT_SORCBALL2:
                case HEXEN_MT_SORCBALL3:
                    break;
                default:
                    P_RemoveMobj(mo);
                    return;
            }
        }
        switch (mo->type)
        {
            case HEXEN_MT_SORCFX1:
                mo->momz = -mo->momz;       // no energy absorbed
                break;
            case HEXEN_MT_SGSHARD1:
            case HEXEN_MT_SGSHARD2:
            case HEXEN_MT_SGSHARD3:
            case HEXEN_MT_SGSHARD4:
            case HEXEN_MT_SGSHARD5:
            case HEXEN_MT_SGSHARD6:
            case HEXEN_MT_SGSHARD7:
            case HEXEN_MT_SGSHARD8:
            case HEXEN_MT_SGSHARD9:
            case HEXEN_MT_SGSHARD0:
                mo->momz = FixedMul(mo->momz, -0.3 * FRACUNIT);
                if (abs(mo->momz) < (FRACUNIT / 2))
                {
                    P_SetMobjState(mo, HEXEN_S_NULL);
                    return;
                }
                break;
            default:
                mo->momz = FixedMul(mo->momz, -0.7 * FRACUNIT);
                break;
        }
        mo->momx = 2 * mo->momx / 3;
        mo->momy = 2 * mo->momy / 3;
        if (mo->info->seesound)
        {
            switch (mo->type)
            {
                case HEXEN_MT_SORCBALL1:
                case HEXEN_MT_SORCBALL2:
                case HEXEN_MT_SORCBALL3:
                    if (!mo->special_args[0])
                        S_StartMobjSound(mo, mo->info->seesound);
                    break;
                default:
                    S_StartMobjSound(mo, mo->info->seesound);
                    break;
            }
            S_StartMobjSound(mo, mo->info->seesound);
        }
    }
    else
    {
      mo->momz = -mo->momz;
      P_SetMobjState(mo, mobjinfo[mo->type].deathstate);
    }
}

extern mobj_t *PuffSpawned;

void Raven_P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z)
{
    mobj_t *puff;

    z += (P_SubRandom() << 10);
    puff = P_SpawnMobj(x, y, z, PuffType);
    if (hexen && linetarget && puff->info->seesound)
    {                           // Hit thing sound
        S_StartMobjSound(puff, puff->info->seesound);
    }
    else if (puff->info->attacksound)
    {
        S_StartMobjSound(puff, puff->info->attacksound);
    }
    switch (PuffType)
    {
        case HERETIC_MT_BEAKPUFF:
        case HERETIC_MT_STAFFPUFF:
        case HEXEN_MT_PUNCHPUFF:
            puff->momz = FRACUNIT;
            break;
        case HERETIC_MT_GAUNTLETPUFF1:
        case HERETIC_MT_GAUNTLETPUFF2:
        case HEXEN_MT_HAMMERPUFF:
            puff->momz = (fixed_t)(.8 * FRACUNIT);
        default:
            break;
    }
    PuffSpawned = puff;
}

void P_BloodSplatter(fixed_t x, fixed_t y, fixed_t z, mobj_t * originator)
{
    mobj_t *mo;

    mo = P_SpawnMobj(x, y, z, g_mt_bloodsplatter);
    P_SetTarget(&mo->target, originator);
    mo->momx = P_SubRandom() << g_bloodsplatter_shift;
    mo->momy = P_SubRandom() << g_bloodsplatter_shift;
    mo->momz = FRACUNIT * g_bloodsplatter_weight;
}

void P_RipperBlood(mobj_t * mo, mobj_t * bleeder)
{
    mobj_t *th;
    fixed_t x, y, z;

    x = mo->x + (P_SubRandom() << 12);
    y = mo->y + (P_SubRandom() << 12);
    z = mo->z + (P_SubRandom() << 12);
    th = P_SpawnMobj(x, y, z, g_mt_blood);
    if (!hexen) th->flags |= MF_NOGRAVITY;
    th->momx = mo->momx >> 1;
    th->momy = mo->momy >> 1;
    th->tics += P_Random(pr_heretic) & 3;
    th->color = bleeder->info->bloodcolor;
}

// hexen

#define MAX_TID_COUNT 200

extern mobj_t LavaInflictor;

static int TIDList[MAX_TID_COUNT + 1];  // +1 for termination marker
static mobj_t *TIDMobj[MAX_TID_COUNT];

mobj_t *P_SpawnMissileAngleSpeed(mobj_t * source, mobjtype_t type,
                                 angle_t angle, fixed_t momz, fixed_t speed)
{
    fixed_t z;
    mobj_t *mo;

    z = source->z;
    z -= source->floorclip;
    mo = P_SpawnMobj(source->x, source->y, z, type);
    P_SetTarget(&mo->target, source); // Originator
    mo->angle = angle;
    angle >>= ANGLETOFINESHIFT;
    mo->momx = FixedMul(speed, finecosine[angle]);
    mo->momy = FixedMul(speed, finesine[angle]);
    mo->momz = momz;
    return (P_CheckMissileSpawn(mo) ? mo : NULL);
}

mobj_t *P_SPMAngleXYZ(mobj_t * source, fixed_t x, fixed_t y,
                      fixed_t z, mobjtype_t type, angle_t angle)
{
    mobj_t *th;
    angle_t an;
    fixed_t slope;

    //
    // see which target is to be aimed at
    //
    an = angle;
    slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, 0);
    if (!linetarget)
    {
        an += 1 << 26;
        slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, 0);
        if (!linetarget)
        {
            an -= 2 << 26;
            slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, 0);
        }
        if (!linetarget)
        {
            an = angle;
            slope = ((source->player->lookdir) << FRACBITS) / 173;
        }
    }
    z += 4 * 8 * FRACUNIT + ((source->player->lookdir) << FRACBITS) / 173;
    z -= source->floorclip;
    th = P_SpawnMobj(x, y, z, type);
    P_SetTarget(&th->target, source);
    th->angle = an;
    th->momx = FixedMul(th->info->speed, finecosine[an >> ANGLETOFINESHIFT]);
    th->momy = FixedMul(th->info->speed, finesine[an >> ANGLETOFINESHIFT]);
    th->momz = FixedMul(th->info->speed, slope);
    return (P_CheckMissileSpawn(th) ? th : NULL);
}

static void PlayerLandedOnThing(mobj_t * mo, mobj_t * onmobj, fixed_t gravity)
{
    mo->player->deltaviewheight = mo->momz >> 3;
    if (mo->momz < -23 * FRACUNIT)
    {
        P_FallingDamage(mo->player);
        P_NoiseAlert(mo, mo);
    }
    else if (mo->momz < -gravity * 12 && !mo->player->morphTics)
    {
        S_StartMobjSound(mo, hexen_sfx_player_land);
        switch (mo->player->pclass)
        {
            case PCLASS_FIGHTER:
                S_StartMobjSound(mo, hexen_sfx_player_fighter_grunt);
                break;
            case PCLASS_CLERIC:
                S_StartMobjSound(mo, hexen_sfx_player_cleric_grunt);
                break;
            case PCLASS_MAGE:
                S_StartMobjSound(mo, hexen_sfx_player_mage_grunt);
                break;
            default:
                break;
        }
    }
    else if (!mo->player->morphTics)
    {
        S_StartMobjSound(mo, hexen_sfx_player_land);
    }
    P_AutoCorrectLookDir(mo->player);
}

void P_CreateTIDList(void)
{
    int i;
    mobj_t *mobj;
    thinker_t *t;

    i = 0;
    for (t = thinkercap.next; t != &thinkercap; t = t->next)
    {                           // Search all current thinkers
        if (t->function != P_MobjThinker)
        {                       // Not a mobj thinker
            continue;
        }
        mobj = (mobj_t *) t;
        if (mobj->tid != 0)
        {                       // Add to list
            if (i == MAX_TID_COUNT)
            {
                I_Error("P_CreateTIDList: MAX_TID_COUNT (%d) exceeded.",
                        MAX_TID_COUNT);
            }
            TIDList[i] = mobj->tid;
            TIDMobj[i++] = mobj;
        }
    }
    // Add termination marker
    TIDList[i] = 0;
}

void P_InsertMobjIntoTIDList(mobj_t * mobj, short tid)
{
    int i;
    int index;

    index = -1;
    for (i = 0; TIDList[i] != 0; i++)
    {
        if (TIDList[i] == -1)
        {                       // Found empty slot
            index = i;
            break;
        }
    }
    if (index == -1)
    {                           // Append required
        if (i == MAX_TID_COUNT)
        {
            I_Error("P_InsertMobjIntoTIDList: MAX_TID_COUNT (%d)"
                    "exceeded.", MAX_TID_COUNT);
        }
        index = i;
        TIDList[index + 1] = 0;
    }
    mobj->tid = tid;
    TIDList[index] = tid;
    TIDMobj[index] = mobj;
}

void P_RemoveMobjFromTIDList(mobj_t * mobj)
{
    int i;

    for (i = 0; TIDList[i] != 0; i++)
    {
        if (TIDMobj[i] == mobj)
        {
            TIDList[i] = -1;
            TIDMobj[i] = NULL;
            mobj->tid = 0;
            return;
        }
    }
    mobj->tid = 0;
}

mobj_t *P_FindMobjFromTID(short tid, int *searchPosition)
{
    int i;

    for (i = *searchPosition + 1; TIDList[i] != 0; i++)
    {
        if (TIDList[i] == tid)
        {
            *searchPosition = i;
            return TIDMobj[i];
        }
    }
    *searchPosition = -1;
    return NULL;
}

void P_BloodSplatter2(fixed_t x, fixed_t y, fixed_t z, mobj_t * originator)
{
    mobj_t *mo;
    int r1, r2;

    r1 = P_Random(pr_hexen);
    r2 = P_Random(pr_hexen);
    mo = P_SpawnMobj(x + ((r2 - 128) << 11),
                     y + ((r1 - 128) << 11), z, HEXEN_MT_AXEBLOOD);
    P_SetTarget(&mo->target, originator);
}

#define SMALLSPLASHCLIP 12<<FRACBITS;

static int Hexen_P_HitFloor(mobj_t * thing)
{
    mobj_t *mo;
    int smallsplash = false;

    if (thing->floorz != thing->subsector->sector->floorheight)
    {                           // don't splash if landing on the edge above water/lava/etc....
        return (FLOOR_SOLID);
    }

    // Things that don't splash go here
    switch (thing->type)
    {
        case HEXEN_MT_LEAF1:
        case HEXEN_MT_LEAF2:
        case HEXEN_MT_SPLASH:
        case HEXEN_MT_SLUDGECHUNK:
            return (FLOOR_SOLID);
        default:
            break;
    }

    // Small splash for small masses
    if (thing->info->mass < 10)
        smallsplash = true;

    switch (P_GetThingFloorType(thing))
    {
        case FLOOR_WATER:
            if (smallsplash)
            {
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HEXEN_MT_SPLASHBASE);
                if (mo)
                    mo->floorclip += SMALLSPLASHCLIP;
                S_StartMobjSound(mo, hexen_sfx_ambient10);        // small drip
            }
            else
            {
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HEXEN_MT_SPLASH);
                P_SetTarget(&mo->target, thing);
                mo->momx = P_SubRandom() << 8;
                mo->momy = P_SubRandom() << 8;
                mo->momz = 2 * FRACUNIT + (P_Random(pr_hexen) << 8);
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HEXEN_MT_SPLASHBASE);
                if (thing->player)
                    P_NoiseAlert(thing, thing);
                S_StartMobjSound(mo, hexen_sfx_water_splash);
            }
            return (FLOOR_WATER);
        case FLOOR_LAVA:
            if (smallsplash)
            {
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HEXEN_MT_LAVASPLASH);
                if (mo)
                    mo->floorclip += SMALLSPLASHCLIP;
            }
            else
            {
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HEXEN_MT_LAVASMOKE);
                mo->momz = FRACUNIT + (P_Random(pr_hexen) << 7);
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HEXEN_MT_LAVASPLASH);
                if (thing->player)
                    P_NoiseAlert(thing, thing);
            }
            S_StartMobjSound(mo, hexen_sfx_lava_sizzle);
            if (thing->player && leveltime & 31)
            {
                P_DamageMobj(thing, &LavaInflictor, NULL, 5);
            }
            return (FLOOR_LAVA);
        case FLOOR_SLUDGE:
            if (smallsplash)
            {
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ,
                                 HEXEN_MT_SLUDGESPLASH);
                if (mo)
                    mo->floorclip += SMALLSPLASHCLIP;
            }
            else
            {
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ,
                                 HEXEN_MT_SLUDGECHUNK);
                P_SetTarget(&mo->target, thing);
                mo->momx = P_SubRandom() << 8;
                mo->momy = P_SubRandom() << 8;
                mo->momz = FRACUNIT + (P_Random(pr_hexen) << 8);
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ,
                                 HEXEN_MT_SLUDGESPLASH);
                if (thing->player)
                    P_NoiseAlert(thing, thing);
            }
            S_StartMobjSound(mo, hexen_sfx_sludge_gloop);
            return (FLOOR_SLUDGE);
    }
    return (FLOOR_SOLID);
}

mobj_t *P_SpawnMissileXYZ(fixed_t x, fixed_t y, fixed_t z,
                          mobj_t * source, mobj_t * dest, mobjtype_t type)
{
    mobj_t *th;
    angle_t an;
    int dist;

    z -= source->floorclip;
    th = P_SpawnMobj(x, y, z, type);
    if (th->info->seesound)
    {
        S_StartMobjSound(th, th->info->seesound);
    }
    P_SetTarget(&th->target, source); // Originator
    an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);
    if (dest->flags & MF_SHADOW)
    {                           // Invisible target
        an += P_SubRandom() << 21;
    }
    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul(th->info->speed, finecosine[an]);
    th->momy = FixedMul(th->info->speed, finesine[an]);
    dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
    dist = dist / th->info->speed;
    if (dist < 1)
    {
        dist = 1;
    }
    th->momz = (dest->z - source->z) / dist;
    return (P_CheckMissileSpawn(th) ? th : NULL);
}

mobj_t *P_SpawnKoraxMissile(fixed_t x, fixed_t y, fixed_t z,
                            mobj_t * source, mobj_t * dest, mobjtype_t type)
{
    mobj_t *th;
    angle_t an;
    int dist;

    z -= source->floorclip;
    th = P_SpawnMobj(x, y, z, type);
    if (th->info->seesound)
    {
        S_StartMobjSound(th, th->info->seesound);
    }
    P_SetTarget(&th->target, source); // Originator
    an = R_PointToAngle2(x, y, dest->x, dest->y);
    if (dest->flags & MF_SHADOW)
    {                           // Invisible target
        an += P_SubRandom() << 21;
    }
    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul(th->info->speed, finecosine[an]);
    th->momy = FixedMul(th->info->speed, finesine[an]);
    dist = P_AproxDistance(dest->x - x, dest->y - y);
    dist = dist / th->info->speed;
    if (dist < 1)
    {
        dist = 1;
    }
    th->momz = (dest->z - z + (30 * FRACUNIT)) / dist;
    return (P_CheckMissileSpawn(th) ? th : NULL);
}
