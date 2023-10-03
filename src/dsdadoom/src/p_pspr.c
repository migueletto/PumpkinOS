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
 *      Weapon sprite animation, weapon objects.
 *      Action functions for weapons.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "r_main.h"
#include "p_map.h"
#include "p_inter.h"
#include "p_pspr.h"
#include "p_enemy.h"
#include "p_tick.h"
#include "m_random.h"
#include "s_sound.h"
#include "sounds.h"
#include "d_event.h"
#include "smooth.h"
#include "g_game.h"
#include "lprintf.h"
#include "e6y.h"//e6y
#include "dsda.h"

#define LOWERSPEED   (FRACUNIT*6)
#define RAISESPEED   (FRACUNIT*6)
#define WEAPONBOTTOM (FRACUNIT*128)
#define WEAPONTOP    (FRACUNIT*32)

#define BFGCELLS bfgcells        /* Ty 03/09/98 externalized in p_inter.c */

// Checking correctness of input parameters for weapon codepointers
// for avoiding crashes when they are used with player/monster states.
#ifdef PRBOOM_DEBUG
  #define CHECK_WEAPON_CODEPOINTER(codepointer, player)\
    if (!player->mo->player) {\
      I_Error("%s: Weapon codepointers cannot be used with player/monster states (incorrect DEH).", codepointer);\
      return;\
    }
#else
  #define CHECK_WEAPON_CODEPOINTER(codepointer, player)
#endif

extern void P_ForwardThrust(player_t *, angle_t, fixed_t);
extern void P_Thrust(player_t *, angle_t, fixed_t);

// The following array holds the recoil values         // phares

static const int recoil_values[] = {    // phares
  10, // wp_fist
  10, // wp_pistol
  30, // wp_shotgun
  10, // wp_chaingun
  100,// wp_missile
  20, // wp_plasma
  100,// wp_bfg
  0,  // wp_chainsaw
  80  // wp_supershotgun
};

//
// P_SetPsprite
//

void P_SetPsprite(player_t *player, int position, statenum_t stnum)
{
  P_SetPspritePtr(player, &player->psprites[position], stnum);
}

//
// P_SetPspritePtr
//

void P_SetPspritePtr(player_t *player, pspdef_t *psp, statenum_t stnum)
{
  do
  {
    state_t *state;

    if (!stnum)
    {
      // object removed itself
      psp->state = NULL;
      break;
    }

    state = &states[stnum];
    psp->state = state;
    psp->tics = state->tics;        // could be 0

    if (hexen)
    {
      if (state->misc1)
      {                       // Set coordinates.
        psp->sx = state->misc1 << FRACBITS;
      }
      if (state->misc2)
      {
        psp->sy = state->misc2 << FRACBITS;
      }
    }
    else if (state->misc1)
    {
      // coordinate set
      psp->sx = state->misc1 << FRACBITS;
      psp->sy = state->misc2 << FRACBITS;
    }

    // Call action routine.
    // Modified handling.
    if (state->action)
    {
      state->action(player, psp);
      if (!psp->state)
        break;
    }
    stnum = psp->state->nextstate;
  }
  while (!psp->tics);     // an initial state of 0 could cycle through
}

//
// P_BringUpWeapon
// Starts bringing the pending weapon up
// from the bottom of the screen.
// Uses player
//

static void P_BringUpWeapon(player_t *player)
{
  statenum_t newstate;

  if (player->pendingweapon == wp_nochange)
    player->pendingweapon = player->readyweapon;

  if (player->pendingweapon == g_wp_chainsaw)
    S_StartMobjSound(player->mo, g_sfx_sawup);

  if (player->pendingweapon >= NUMWEAPONS)
    lprintf(LO_WARN, "P_BringUpWeapon: weaponinfo overrun has occurred.\n");

  if (player->pclass)
  {
    if (player->pclass == PCLASS_FIGHTER && player->pendingweapon == wp_second
        && player->ammo[MANA_1])
    {
      newstate = HEXEN_S_FAXEUP_G;
    }
    else
    {
      newstate = hexen_weaponinfo[player->pendingweapon][player->pclass].upstate;
    }
  }
  else if (player->powers[pw_weaponlevel2])
  {
    newstate = wpnlev2info[player->pendingweapon].upstate;
  }
  else
  {
    newstate = weaponinfo[player->pendingweapon].upstate;
  }

  player->pendingweapon = wp_nochange;
  // killough 12/98: prevent pistol from starting visibly at bottom of screen:
  player->psprites[ps_weapon].sy =
    mbf_features ? WEAPONBOTTOM + FRACUNIT * 2 : WEAPONBOTTOM;

  P_SetPsprite(player, ps_weapon, newstate);
}

// The first set is where the weapon preferences from             // killough,
// default.cfg are stored. These values represent the keys used   // phares
// in DOOM2 to bring up the weapon, i.e. 6 = plasma gun. These    //    |
// are NOT the wp_* constants.                                    //    V

int weapon_preferences[2][NUMWEAPONS+1] = {
  {6, 9, 4, 3, 2, 8, 5, 7, 1, 0},  // !compatibility preferences
  {6, 9, 4, 3, 2, 8, 5, 7, 1, 0},  //  compatibility preferences
};

// [XA] fixed version of P_SwitchWeapon that correctly
// takes each weapon's ammotype and ammopershot into account,
// instead of blindly assuming both.

static int P_SwitchWeaponMBF21(player_t *player)
{
  int *prefer;
  int currentweapon, newweapon;
  int i;
  weapontype_t checkweapon;
  ammotype_t ammotype;

  prefer = weapon_preferences[0];
  currentweapon = player->readyweapon;
  newweapon = currentweapon;
  i = NUMWEAPONS + 1;

  do
  {
    checkweapon = wp_nochange;
    switch (*prefer++)
    {
      case 1:
        if (!player->powers[pw_strength]) // allow chainsaw override
          break;
        // fallthrough
      case 0:
        checkweapon = wp_fist;
        break;
      case 2:
        checkweapon = wp_pistol;
        break;
      case 3:
        checkweapon = wp_shotgun;
        break;
      case 4:
        checkweapon = wp_chaingun;
        break;
      case 5:
        checkweapon = wp_missile;
        break;
      case 6:
        if (gamemode != shareware)
          checkweapon = wp_plasma;
        break;
      case 7:
        if (gamemode != shareware)
          checkweapon = wp_bfg;
        break;
      case 8:
        checkweapon = wp_chainsaw;
        break;
      case 9:
        if (gamemode == commercial)
          checkweapon = wp_supershotgun;
        break;
    }

    if (checkweapon != wp_nochange && player->weaponowned[checkweapon])
    {
      ammotype = weaponinfo[checkweapon].ammo;
      if (ammotype == am_noammo ||
        player->ammo[ammotype] >= weaponinfo[checkweapon].ammopershot)
        newweapon = checkweapon;
    }
  }
  while (newweapon == currentweapon && --i);
  return newweapon;
}

// P_SwitchWeapon checks current ammo levels and gives you the
// most preferred weapon with ammo. It will not pick the currently
// raised weapon. When called from P_CheckAmmo this won't matter,
// because the raised weapon has no ammo anyway. When called from
// G_BuildTiccmd you want to toggle to a different weapon regardless.

int P_SwitchWeapon(player_t *player)
{
  int *prefer;
  int currentweapon, newweapon;
  int i;

  // [XA] use fixed behavior for mbf21. no need
  // for a discrete compat option for this, as
  // it doesn't impact demo playback (weapon
  // switches are saved in the demo itself)
  if (mbf21)
    return P_SwitchWeaponMBF21(player);

  prefer = weapon_preferences[demo_compatibility != 0]; // killough 3/22/98
  currentweapon = player->readyweapon;
  newweapon = currentweapon;
  i = NUMWEAPONS + 1;   // killough 5/2/98

  // killough 2/8/98: follow preferences and fix BFG/SSG bugs

  do
    switch (*prefer++)
      {
      case 1:
        if (!player->powers[pw_strength])      // allow chainsaw override
          break;
        // fallthrough
      case 0:
        newweapon = wp_fist;
        break;
      case 2:
        if (player->ammo[am_clip])
          newweapon = wp_pistol;
        break;
      case 3:
        if (player->weaponowned[wp_shotgun] && player->ammo[am_shell])
          newweapon = wp_shotgun;
        break;
      case 4:
        if (player->weaponowned[wp_chaingun] && player->ammo[am_clip])
          newweapon = wp_chaingun;
        break;
      case 5:
        if (player->weaponowned[wp_missile] && player->ammo[am_misl])
          newweapon = wp_missile;
        break;
      case 6:
        if (player->weaponowned[wp_plasma] && player->ammo[am_cell] &&
            gamemode != shareware)
          newweapon = wp_plasma;
        break;
      case 7:
        if (player->weaponowned[wp_bfg] && gamemode != shareware &&
            player->ammo[am_cell] >= (demo_compatibility ? 41 : 40))
          newweapon = wp_bfg;
        break;
      case 8:
        if (player->weaponowned[wp_chainsaw])
          newweapon = wp_chainsaw;
        break;
      case 9:
        if (player->weaponowned[wp_supershotgun] && gamemode == commercial &&
            player->ammo[am_shell] >= (demo_compatibility ? 3 : 2))
          newweapon = wp_supershotgun;
        break;
      }
  while (newweapon == currentweapon && --i);          // killough 5/2/98
  return newweapon;
}

// killough 5/2/98: whether consoleplayer prefers weapon w1 over weapon w2.
int P_WeaponPreferred(int w1, int w2)
{
  return
    (weapon_preferences[0][0] != ++w2 && (weapon_preferences[0][0] == ++w1 ||
    (weapon_preferences[0][1] !=   w2 && (weapon_preferences[0][1] ==   w1 ||
    (weapon_preferences[0][2] !=   w2 && (weapon_preferences[0][2] ==   w1 ||
    (weapon_preferences[0][3] !=   w2 && (weapon_preferences[0][3] ==   w1 ||
    (weapon_preferences[0][4] !=   w2 && (weapon_preferences[0][4] ==   w1 ||
    (weapon_preferences[0][5] !=   w2 && (weapon_preferences[0][5] ==   w1 ||
    (weapon_preferences[0][6] !=   w2 && (weapon_preferences[0][6] ==   w1 ||
    (weapon_preferences[0][7] !=   w2 && (weapon_preferences[0][7] ==   w1
   ))))))))))))))));
}

int P_AmmoPercent(player_t *player, int weapon)
{
  int ammo_i = weaponinfo[weapon].ammo;

  if (!player->maxammo[ammo_i])
    return 0;

  return player->ammo[ammo_i] * 100 / player->maxammo[ammo_i];
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
// (only in demo_compatibility mode -- killough 3/22/98)
//

static dboolean Heretic_P_CheckAmmo(struct player_s * player);
static dboolean P_CheckMana(player_t * player);

dboolean P_CheckAmmo(player_t *player)
{
  ammotype_t ammo;
  int count;  // Regular

  if (heretic) return Heretic_P_CheckAmmo(player);
  if (hexen) return P_CheckMana(player);

  ammo = weaponinfo[player->readyweapon].ammo;
  if (mbf21)
    count = weaponinfo[player->readyweapon].ammopershot;
  else
    if (player->readyweapon == wp_bfg)  // Minimal amount for one shot varies.
      count = BFGCELLS;
    else
      if (player->readyweapon == wp_supershotgun)        // Double barrel.
        count = 2;
      else
        count = 1;

  // Some do not need ammunition anyway.
  // Return if current ammunition sufficient.

  if (player->cheats & CF_INFINITE_AMMO || ammo == am_noammo || player->ammo[ammo] >= count)
    return true;

  // Out of ammo, pick a weapon to change to.
  //
  // killough 3/22/98: for old demos we do the switch here and now;
  // for Boom games we cannot do this, and have different player
  // preferences across demos or networks, so we have to use the
  // G_BuildTiccmd() interface instead of making the switch here.

  if (demo_compatibility)
    {
      player->pendingweapon = P_SwitchWeapon(player);      // phares
      // Now set appropriate weapon overlay.
      P_SetPsprite(player,ps_weapon,weaponinfo[player->readyweapon].downstate);
    }

  return false;
}

//
// P_SubtractAmmo
// Subtracts ammo, w/compat handling. In mbf21, use
// readyweapon's "ammopershot" field if it's explicitly
// defined in dehacked; otherwise use the amount specified
// by the codepointer instead (Doom behavior)
//
// [XA] NOTE: this function is for handling Doom's native
// attack pointers; of note, the new A_ConsumeAmmo mbf21
// codepointer does NOT call this function, since it doesn't
// have to worry about any compatibility shenanigans.
//

void P_SubtractAmmo(struct player_s *player, int vanilla_amount)
{
  int amount;
  ammotype_t ammotype = weaponinfo[player->readyweapon].ammo;

  if (player->cheats & CF_INFINITE_AMMO || (mbf21 && ammotype == am_noammo))
    return; // [XA] hmm... I guess vanilla/boom will go out of bounds then?

  if (mbf21 && (weaponinfo[player->readyweapon].intflags & WIF_ENABLEAPS))
    amount = weaponinfo[player->readyweapon].ammopershot;
  else
    amount = vanilla_amount;

  player->ammo[ammotype] -= amount;

  if (mbf21 && player->ammo[ammotype] < 0)
    player->ammo[ammotype] = 0;
}

//
// P_FireWeapon.
//

static void P_FireWeapon(player_t *player)
{
  statenum_t newstate;

  if (!P_CheckAmmo(player))
    return;

  dsda_WatchWeaponFire(player->readyweapon);

  P_SetMobjState(player->mo, pclass[player->pclass].fire_weapon_state);

  if (heretic)
  {
    weaponinfo_t *wpinfo;

    wpinfo = player->powers[pw_weaponlevel2] ? &wpnlev2info[0]
                                             : &weaponinfo[0];
    newstate = player->refire ? wpinfo[player->readyweapon].holdatkstate
                              : wpinfo[player->readyweapon].atkstate;
  }
  else if (hexen)
  {
    if (player->pclass == PCLASS_FIGHTER && player->readyweapon == wp_second
        && player->ammo[MANA_1] > 0)
    {                           // Glowing axe
      newstate = HEXEN_S_FAXEATK_G1;
    }
    else
    {
      newstate = player->refire ?
        hexen_weaponinfo[player->readyweapon][player->pclass].holdatkstate
        : hexen_weaponinfo[player->readyweapon][player->pclass].atkstate;
    }
  }
  else
  {
    newstate = weaponinfo[player->readyweapon].atkstate;
  }

  P_SetPsprite(player, ps_weapon, newstate);
  if (hexen || !(weaponinfo[player->readyweapon].flags & WPF_SILENT))
    P_NoiseAlert(player->mo, player->mo);

  // heretic_note: does the order matter? can we move it up?
  if (heretic)
  {
    if (player->readyweapon == wp_gauntlets && !player->refire)
    {                           // Play the sound for the initial gauntlet attack
        S_StartMobjSound(player->mo, heretic_sfx_gntuse);
    }
  }
}

//
// P_DropWeapon
// Player died, so put the weapon away.
//

void P_DropWeapon(player_t *player)
{
  statenum_t newstate;
  if (player->powers[pw_weaponlevel2])
  {
    newstate = wpnlev2info[player->readyweapon].downstate;
  }
  else if (player->pclass)
  {
    newstate = hexen_weaponinfo[player->readyweapon][player->pclass].downstate;
  }
  else
  {
    newstate = weaponinfo[player->readyweapon].downstate;
  }
  P_SetPsprite(player, ps_weapon, newstate);
}

//
// A_WeaponReady
// The player can fire the weapon
// or change to another weapon at this time.
// Follows after getting weapon up,
// or after previous attack/fire sequence.
//

void A_WeaponReady(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_WeaponReady", player);

  if (player->chickenTics)
  {                           // Change to the chicken beak
      P_ActivateBeak(player);
      return;
  }

  // get out of attack state
  if (player->mo->state >= &states[pclass[player->pclass].attack_state]
      && player->mo->state <= &states[pclass[player->pclass].attack_end_state])
    P_SetMobjState(player->mo, pclass[player->pclass].normal_state);

  if (heretic)
  {
    if (player->readyweapon == wp_staff
        && psp->state == &states[HERETIC_S_STAFFREADY2_1]
        && P_Random(pr_heretic) < 128)
    {
        S_StartMobjSound(player->mo, heretic_sfx_stfcrk);
    }
  }
  else if (hexen)
  {
    // hexen does nothing here
  }
  else
    if (player->readyweapon == wp_chainsaw && psp->state == &states[S_SAW])
      S_StartMobjSound(player->mo, sfx_sawidl);

  // check for change
  //  if player is dead, put the weapon away

  if (player->pendingweapon != wp_nochange || !player->health)
  {
    // change weapon (pending weapon should already be validated)
    statenum_t newstate;
    if (player->powers[pw_weaponlevel2])
      newstate = wpnlev2info[player->readyweapon].downstate;
    else if (player->pclass)
      newstate = hexen_weaponinfo[player->readyweapon][player->pclass].downstate;
    else
      newstate = weaponinfo[player->readyweapon].downstate;
    P_SetPsprite(player, ps_weapon, newstate);
    return;
  }

  // check for fire
  //  the missile launcher and bfg do not auto fire

  if (player->cmd.buttons & BT_ATTACK)
  {
    if (
      hexen || // hexen_note: why is this different?
      !player->attackdown ||
      !(weaponinfo[player->readyweapon].flags & WPF_NOAUTOFIRE)
    )
    {
      player->attackdown = true;
      P_FireWeapon(player);
      return;
    }
  }
  else
    player->attackdown = false;

  // bob the weapon based on movement speed
  if (!player->morphTics)
  {
    int angle = (128 * leveltime) & FINEMASK;
    psp->sx = FRACUNIT + FixedMul(player->bob, finecosine[angle]);
    angle &= FINEANGLES / 2 - 1;
    psp->sy = WEAPONTOP + FixedMul(player->bob, finesine[angle]);
  }
}

//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//

void A_ReFire(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_ReFire", player);

  // check for fire
  //  (if a weaponchange is pending, let it go through instead)

  if ( (player->cmd.buttons & BT_ATTACK)
       && player->pendingweapon == wp_nochange && player->health)
    {
      player->refire++;
      P_FireWeapon(player);
    }
  else
    {
      player->refire = 0;
      P_CheckAmmo(player);
    }
}

dboolean boom_weapon_state_injection;

void A_CheckReload(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_CheckReload", player);

  if (!P_CheckAmmo(player) && compatibility_level >= prboom_4_compatibility) {
    /* cph 2002/08/08 - In old Doom, P_CheckAmmo would start the weapon lowering
     * immediately. This was lost in Boom when the weapon switching logic was
     * rewritten. But we must tell Doom that we don't need to complete the
     * reload frames for the weapon here. G_BuildTiccmd will set ->pendingweapon
     * for us later on. */
    boom_weapon_state_injection = true;
    P_SetPsprite(player,ps_weapon,weaponinfo[player->readyweapon].downstate);
  }
}

//
// A_Lower
// Lowers current weapon,
//  and changes weapon at bottom.
//

void A_Lower(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_Lower", player);

  if (player->chickenTics || player->morphTics)
  {
      psp->sy = WEAPONBOTTOM;
  }
  else
  {
      psp->sy += LOWERSPEED;
  }

  // Is already down.
  if (psp->sy < WEAPONBOTTOM)
    return;

  // Player is dead.
  if (player->playerstate == PST_DEAD)
  {
    psp->sy = WEAPONBOTTOM;
    return;      // don't bring weapon back up
  }

  // The old weapon has been lowered off the screen,
  // so change the weapon and start raising it

  if (!player->health)
  {      // Player is dead, so keep the weapon off screen.
    P_SetPsprite(player,  ps_weapon, g_s_null);
    return;
  }

  player->readyweapon = player->pendingweapon;

  P_BringUpWeapon(player);
}

//
// A_Raise
//

void A_Raise(player_t *player, pspdef_t *psp)
{
  statenum_t newstate;

  CHECK_WEAPON_CODEPOINTER("A_Raise", player);

  psp->sy -= RAISESPEED;

  if (psp->sy > WEAPONTOP)
    return;

  psp->sy = WEAPONTOP;

  // The weapon has been raised all the way,
  //  so change to the ready state.

  if (player->powers[pw_weaponlevel2])
    newstate = wpnlev2info[player->readyweapon].readystate;
  else if (player->pclass)
  {
    if (player->pclass == PCLASS_FIGHTER && player->readyweapon == wp_second
        && player->ammo[MANA_1])
    {
      newstate = HEXEN_S_FAXEREADY_G;
    }
    else
    {
      newstate = hexen_weaponinfo[player->readyweapon][player->pclass].readystate;
    }
  }
  else
    newstate = weaponinfo[player->readyweapon].readystate;

  P_SetPsprite(player, ps_weapon, newstate);
}


// Weapons now recoil, amount depending on the weapon.              // phares
//                                                                  //   |
// The P_SetPsprite call in each of the weapon firing routines      //   V
// was moved here so the recoil could be synched with the
// muzzle flash, rather than the pressing of the trigger.
// The BFG delay caused this to be necessary.

static void A_FireSomething(player_t* player,int adder)
{
  P_SetPsprite(player, ps_flash,
               weaponinfo[player->readyweapon].flashstate+adder);

  // killough 3/27/98: prevent recoil in no-clipping mode
  if (!(player->mo->flags & MF_NOCLIP))
    if (!compatibility && weapon_recoil) // phares
      P_ForwardThrust(player, ANG180 + player->mo->angle,
                      2048 * recoil_values[player->readyweapon]);
}

//
// A_GunFlash
//

void A_GunFlash(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_GunFlash", player);

  P_SetMobjState(player->mo, S_PLAY_ATK2);

  A_FireSomething(player,0);                                      // phares
}

//
// WEAPON ATTACKS
//

//
// A_Punch
//

void A_Punch(player_t *player, pspdef_t *psp)
{
  angle_t angle;
  int t, slope, damage, range;

  CHECK_WEAPON_CODEPOINTER("A_Punch", player);

  damage = (P_Random(pr_punch)%10+1)<<1;

  if (player->powers[pw_strength])
    damage *= 10;

  angle = player->mo->angle;

  // killough 5/5/98: remove dependence on order of evaluation:
  t = P_Random(pr_punchangle);
  angle += (t - P_Random(pr_punchangle))<<18;

  range = (mbf21 ? player->mo->info->meleerange : MELEERANGE);

  /* killough 8/2/98: make autoaiming prefer enemies */
  if (!mbf_features ||
      (slope = P_AimLineAttack(player->mo, angle, range, MF_FRIEND),
       !linetarget))
    slope = P_AimLineAttack(player->mo, angle, range, 0);

  P_LineAttack(player->mo, angle, range, slope, damage);

  if (!linetarget)
    return;

  S_StartMobjSound(player->mo, sfx_punch);

  // turn to face target

  player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y,
                                      linetarget->x, linetarget->y);
  R_SmoothPlaying_Reset(player); // e6y
}

//
// A_Saw
//

void A_Saw(player_t *player, pspdef_t *psp)
{
  int slope, damage, range;
  angle_t angle;
  int t;

  CHECK_WEAPON_CODEPOINTER("A_Saw", player);

  damage = 2*(P_Random(pr_saw)%10+1);
  angle = player->mo->angle;
  // killough 5/5/98: remove dependence on order of evaluation:
  t = P_Random(pr_saw);
  angle += (t - P_Random(pr_saw))<<18;

  // Use meleerange + 1 so that the puff doesn't skip the flash
  range = (mbf21 ? player->mo->info->meleerange : MELEERANGE) + 1;

  /* killough 8/2/98: make autoaiming prefer enemies */
  if (!mbf_features ||
      (slope = P_AimLineAttack(player->mo, angle, range, MF_FRIEND),
       !linetarget))
    slope = P_AimLineAttack(player->mo, angle, range, 0);

  P_LineAttack(player->mo, angle, range, slope, damage);

  if (!linetarget)
    {
      S_StartMobjSound(player->mo, sfx_sawful);
      return;
    }

  S_StartMobjSound(player->mo, sfx_sawhit);

  // turn to face target
  angle = R_PointToAngle2(player->mo->x, player->mo->y,
                          linetarget->x, linetarget->y);

  if (angle - player->mo->angle > ANG180) {
    if (angle - player->mo->angle < -ANG90/20)
      player->mo->angle = angle + ANG90/21;
    else
      player->mo->angle -= ANG90/20;
  } else {
    if (angle - player->mo->angle > ANG90/20)
      player->mo->angle = angle - ANG90/21;
    else
      player->mo->angle += ANG90/20;
  }

  player->mo->flags |= MF_JUSTATTACKED;
  R_SmoothPlaying_Reset(player); // e6y
}

//
// A_FireMissile
//

void A_FireMissile(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_FireMissile", player);

  P_SubtractAmmo(player, 1);
  P_SpawnPlayerMissile(player->mo, MT_ROCKET);
}

//
// A_FireBFG
//

void A_FireBFG(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_FireBFG", player);

  P_SubtractAmmo(player, BFGCELLS);
  P_SpawnPlayerMissile(player->mo, MT_BFG);
}

//
// A_FireOldBFG
//
// This function emulates Doom's Pre-Beta BFG
// By Lee Killough 6/6/98, 7/11/98, 7/19/98, 8/20/98
//
// This code may not be used in other mods without appropriate credit given.
// Code leeches will be telefragged.

int autoaim = 0;  // killough 7/19/98: autoaiming was not in original beta
void A_FireOldBFG(player_t *player, pspdef_t *psp)
{
  int type = MT_PLASMA1;

  if (compatibility_level < mbf_compatibility)
    return;

  CHECK_WEAPON_CODEPOINTER("A_FireOldBFG", player);

  if (weapon_recoil && !(player->mo->flags & MF_NOCLIP))
    P_ForwardThrust(player, ANG180 + player->mo->angle,
                    512 * recoil_values[wp_plasma]);

  P_SubtractAmmo(player, 1);

  player->extralight = 2;

  do
  {
    mobj_t *th, *mo = player->mo;
    angle_t an = mo->angle;
    angle_t an1 = ((P_Random(pr_bfg)&127) - 64) * (ANG90/768) + an;
    angle_t an2 = ((P_Random(pr_bfg)&127) - 64) * (ANG90/640) + ANG90;
    extern int autoaim;

    if (autoaim/* || !beta_emulation*/)
    {
      // killough 8/2/98: make autoaiming prefer enemies
      uint64_t mask = mbf_features ? MF_FRIEND : 0;
      fixed_t slope;
      do
      {
        slope = P_AimLineAttack(mo, an, 16*64*FRACUNIT, mask);
        if (!linetarget)
          slope = P_AimLineAttack(mo, an += 1<<26, 16*64*FRACUNIT, mask);
        if (!linetarget)
          slope = P_AimLineAttack(mo, an -= 2<<26, 16*64*FRACUNIT, mask);
        if (!linetarget)
          slope = 0, an = mo->angle;
      }
      while (mask && (mask=0, !linetarget));     // killough 8/2/98
      an1 += an - mo->angle;
      an2 += tantoangle[slope >> DBITS];
    }

    th = P_SpawnMobj(mo->x, mo->y, mo->z + 62*FRACUNIT - player->psprites[ps_weapon].sy, type);
    P_SetTarget(&th->target, mo);
    th->angle = an1;
    th->momx = finecosine[an1>>ANGLETOFINESHIFT] * 25;
    th->momy = finesine[an1>>ANGLETOFINESHIFT] * 25;
    th->momz = finetangent[an2>>ANGLETOFINESHIFT] * 25;
    P_CheckMissileSpawn(th);
  }
  while ((type != MT_PLASMA2) && (type = MT_PLASMA2)); //killough: obfuscated!
}

//
// A_FirePlasma
//

void A_FirePlasma(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_FirePlasma", player);

  P_SubtractAmmo(player, 1);

  A_FireSomething(player,P_Random(pr_plasma)&1);              // phares
  P_SpawnPlayerMissile(player->mo, MT_PLASMA);
}

//
// P_BulletSlope
// Sets a slope so a near miss is at aproximately
// the height of the intended target
//

//e6y static
fixed_t bulletslope;

static void P_BulletSlope(mobj_t *mo)
{
  angle_t an = mo->angle;    // see which target is to be aimed at

  if (comperr(comperr_freeaim))
    bulletslope = finetangent[(ANG90 - mo->pitch) >> ANGLETOFINESHIFT];
  else
  {
    /* killough 8/2/98: make autoaiming prefer enemies */
    uint64_t mask = mbf_features ? MF_FRIEND : 0;

    do
    {
      bulletslope = P_AimLineAttack(mo, an, 16*64*FRACUNIT, mask);
      if (!linetarget)
        bulletslope = P_AimLineAttack(mo, an += 1<<26, 16*64*FRACUNIT, mask);
      if (!linetarget)
        bulletslope = P_AimLineAttack(mo, an -= 2<<26, 16*64*FRACUNIT, mask);
      if (heretic && !linetarget)
        bulletslope = (mo->player->lookdir << FRACBITS) / 173;
    }
    while (mask && (mask=0, !linetarget));  /* killough 8/2/98 */
  }
}

//
// P_GunShot
//

static void P_GunShot(mobj_t *mo, dboolean accurate)
{
  int damage = 5*(P_Random(pr_gunshot)%3+1);
  angle_t angle = mo->angle;

  if (!accurate)
    {  // killough 5/5/98: remove dependence on order of evaluation:
      int t = P_Random(pr_misfire);
      angle += (t - P_Random(pr_misfire))<<18;
    }

  P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
}

//
// A_FirePistol
//

void A_FirePistol(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_FirePistol", player);

  S_StartMobjSound(player->mo, sfx_pistol);

  P_SetMobjState(player->mo, S_PLAY_ATK2);
  P_SubtractAmmo(player, 1);

  A_FireSomething(player,0);                                      // phares
  P_BulletSlope(player->mo);
  P_GunShot(player->mo, !player->refire);
}

//
// A_FireShotgun
//

void A_FireShotgun(player_t *player, pspdef_t *psp)
{
  int i;

  CHECK_WEAPON_CODEPOINTER("A_FireShotgun", player);

  S_StartMobjSound(player->mo, sfx_shotgn);
  P_SetMobjState(player->mo, S_PLAY_ATK2);

  P_SubtractAmmo(player, 1);

  A_FireSomething(player,0);                                      // phares

  P_BulletSlope(player->mo);

  for (i=0; i<7; i++)
    P_GunShot(player->mo, false);
}

//
// A_FireShotgun2
//

void A_FireShotgun2(player_t *player, pspdef_t *psp)
{
  int i;

  CHECK_WEAPON_CODEPOINTER("A_FireShotgun2", player);

  S_StartMobjSound(player->mo, sfx_dshtgn);
  P_SetMobjState(player->mo, S_PLAY_ATK2);
  P_SubtractAmmo(player, 2);

  A_FireSomething(player,0);                                      // phares

  P_BulletSlope(player->mo);

  for (i=0; i<20; i++)
    {
      int damage = 5*(P_Random(pr_shotgun)%3+1);
      angle_t angle = player->mo->angle;
      // killough 5/5/98: remove dependence on order of evaluation:
      int t = P_Random(pr_shotgun);
      angle += (t - P_Random(pr_shotgun))<<19;
      t = P_Random(pr_shotgun);
      P_LineAttack(player->mo, angle, MISSILERANGE, bulletslope +
                   ((t - P_Random(pr_shotgun))<<5), damage);
    }
}

//
// A_FireCGun
//

void A_FireCGun(player_t *player, pspdef_t *psp)
{
  dboolean has_ammo;

  CHECK_WEAPON_CODEPOINTER("A_FireCGun", player);

  has_ammo = player->ammo[weaponinfo[player->readyweapon].ammo] ||
             player->cheats & CF_INFINITE_AMMO;

  if (has_ammo || comp[comp_sound])
    S_StartMobjSound(player->mo, sfx_pistol);

  if (!has_ammo)
    return;

  P_SetMobjState(player->mo, S_PLAY_ATK2);
  P_SubtractAmmo(player, 1);

  A_FireSomething(player,psp->state - &states[S_CHAIN1]);           // phares

  P_BulletSlope(player->mo);

  P_GunShot(player->mo, !player->refire);
}

void A_Light0(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_Light0", player);

  player->extralight = 0;
}

void A_Light1 (player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_Light1", player);

  player->extralight = 1;
}

void A_Light2 (player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_Light2", player);

  player->extralight = 2;
}

//
// A_BFGSpray
// Spawn a BFG explosion on every monster in view
//

void A_BFGSpray(mobj_t *mo)
{
  int i;

  for (i=0 ; i<40 ; i++)  // offset angles from its attack angle
    {
      int j, damage;
      angle_t an = mo->angle - ANG90/2 + ANG90/40*i;

      // mo->target is the originator (player) of the missile

      // killough 8/2/98: make autoaiming prefer enemies
      if (!mbf_features ||
         (P_AimLineAttack(mo->target, an, 16*64*FRACUNIT, MF_FRIEND),
         !linetarget))
        P_AimLineAttack(mo->target, an, 16*64*FRACUNIT, 0);

      if (!linetarget)
        continue;

      P_SpawnMobj(linetarget->x, linetarget->y,
                  linetarget->z + (linetarget->height>>2), MT_EXTRABFG);

      for (damage=j=0; j<15; j++)
        damage += (P_Random(pr_bfg)&7) + 1;

      P_DamageMobj(linetarget, mo->target, mo->target, damage);
    }
}

//
// A_BFGsound
//

void A_BFGsound(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_BFGsound", player);

  S_StartMobjSound(player->mo, sfx_bfg);
}

//
// [XA] New mbf21 codepointers
//

//
// A_WeaponProjectile
// A parameterized player weapon projectile attack. Does not consume ammo.
//   args[0]: Type of actor to spawn
//   args[1]: Angle (degrees, in fixed point), relative to calling player's angle
//   args[2]: Pitch (degrees, in fixed point), relative to calling player's pitch; approximated
//   args[3]: X/Y spawn offset, relative to calling player's angle
//   args[4]: Z spawn offset, relative to player's default projectile fire height
//
void A_WeaponProjectile(player_t *player, pspdef_t *psp)
{
  int type, angle, pitch, spawnofs_xy, spawnofs_z;
  mobj_t *mo;
  int an;

  CHECK_WEAPON_CODEPOINTER("A_WeaponProjectile", player);

  if (!mbf21 || !psp->state || !psp->state->args[0])
    return;

  type        = psp->state->args[0] - 1;
  angle       = psp->state->args[1];
  pitch       = psp->state->args[2];
  spawnofs_xy = psp->state->args[3];
  spawnofs_z  = psp->state->args[4];

  mo = P_SpawnPlayerMissile(player->mo, type);
  if (!mo)
    return;

  // adjust angle
  mo->angle += (unsigned int)(((int64_t)angle << 16) / 360);
  an = mo->angle >> ANGLETOFINESHIFT;
  mo->momx = FixedMul(mo->info->speed, finecosine[an]);
  mo->momy = FixedMul(mo->info->speed, finesine[an]);

  // adjust pitch (approximated, using Doom's ye olde
  // finetangent table; same method as autoaim)
  mo->momz += FixedMul(mo->info->speed, DegToSlope(pitch));

  // adjust position
  an = (player->mo->angle - ANG90) >> ANGLETOFINESHIFT;
  mo->x += FixedMul(spawnofs_xy, finecosine[an]);
  mo->y += FixedMul(spawnofs_xy, finesine[an]);
  mo->z += spawnofs_z;

  // set tracer to the player's autoaim target,
  // so player seeker missiles prioritizing the
  // baddie the player is actually aiming at. ;)
  P_SetTarget(&mo->tracer, linetarget);
}

//
// A_WeaponBulletAttack
// A parameterized player weapon bullet attack. Does not consume ammo.
//   args[0]: Horizontal spread (degrees, in fixed point)
//   args[1]: Vertical spread (degrees, in fixed point)
//   args[2]: Number of bullets to fire; if not set, defaults to 1
//   args[3]: Base damage of attack (e.g. for 5d3, customize the 5); if not set, defaults to 5
//   args[4]: Attack damage modulus (e.g. for 5d3, customize the 3); if not set, defaults to 3
//
void A_WeaponBulletAttack(player_t *player, pspdef_t *psp)
{
  int hspread, vspread, numbullets, damagebase, damagemod;
  int i, damage, angle, slope;

  CHECK_WEAPON_CODEPOINTER("A_WeaponBulletAttack", player);

  if (!mbf21 || !psp->state)
    return;

  hspread    = psp->state->args[0];
  vspread    = psp->state->args[1];
  numbullets = psp->state->args[2];
  damagebase = psp->state->args[3];
  damagemod  = psp->state->args[4];

  P_BulletSlope(player->mo);

  for (i = 0; i < numbullets; i++)
  {
    damage = (P_Random(pr_mbf21) % damagemod + 1) * damagebase;
    angle = (int)player->mo->angle + P_RandomHitscanAngle(pr_mbf21, hspread);
    slope = bulletslope + P_RandomHitscanSlope(pr_mbf21, vspread);

    P_LineAttack(player->mo, angle, MISSILERANGE, slope, damage);
  }
}

//
// A_WeaponMeleeAttack
// A parameterized player weapon melee attack.
//   args[0]: Base damage of attack (e.g. for 2d10, customize the 2); if not set, defaults to 2
//   args[1]: Attack damage modulus (e.g. for 2d10, customize the 10); if not set, defaults to 10
//   args[2]: Berserk damage multiplier (fixed point); if not set, defaults to 1.0 (no change).
//   args[3]: Sound to play if attack hits
//   args[4]: Range (fixed point); if not set, defaults to player mobj's melee range
//
void A_WeaponMeleeAttack(player_t *player, pspdef_t *psp)
{
  int damagebase, damagemod, zerkfactor, hitsound, range;
  angle_t angle;
  int t, slope, damage;

  CHECK_WEAPON_CODEPOINTER("A_WeaponMeleeAttack", player);

  if (!mbf21 || !psp->state)
    return;

  damagebase = psp->state->args[0];
  damagemod  = psp->state->args[1];
  zerkfactor = psp->state->args[2];
  hitsound   = psp->state->args[3];
  range      = psp->state->args[4];

  if (range == 0)
    range = player->mo->info->meleerange;

  damage = (P_Random(pr_mbf21) % damagemod + 1) * damagebase;
  if (player->powers[pw_strength])
    damage = (damage * zerkfactor) >> FRACBITS;

  // slight randomization; weird vanillaism here. :P
  angle = player->mo->angle;

  t = P_Random(pr_mbf21);
  angle += (t - P_Random(pr_mbf21))<<18;

  // make autoaim prefer enemies
  slope = P_AimLineAttack(player->mo, angle, range, MF_FRIEND);
  if (!linetarget)
    slope = P_AimLineAttack(player->mo, angle, range, 0);

  // attack, dammit!
  P_LineAttack(player->mo, angle, range, slope, damage);

  // missed? ah, welp.
  if (!linetarget)
    return;

  // un-missed!
  S_StartMobjSound(player->mo, hitsound);

  // turn to face target
  player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, linetarget->x, linetarget->y);
  R_SmoothPlaying_Reset(player);
}

//
// A_WeaponSound
// Plays a sound. Usable from weapons, unlike A_PlaySound
//   args[0]: ID of sound to play
//   args[1]: If 1, play sound at full volume (may be useful in DM?)
//
void A_WeaponSound(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_WeaponSound", player);

  if (!mbf21 || !psp->state)
    return;

  S_StartMobjSound(psp->state->args[1] ? NULL : player->mo, psp->state->args[0]);
}

//
// A_WeaponAlert
// Alerts monsters to the player's presence. Handy when combined with WPF_SILENT.
//
void A_WeaponAlert(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_WeaponAlert", player);

  if (!mbf21)
    return;

  P_NoiseAlert(player->mo, player->mo);
}

//
// A_WeaponJump
// Jumps to the specified state, with variable random chance.
// Basically the same as A_RandomJump, but for weapons.
//   args[0]: State number
//   args[1]: Chance, out of 255, to make the jump
//
void A_WeaponJump(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_WeaponJump", player);

  if (!mbf21 || !psp->state)
    return;

  if (P_Random(pr_mbf21) < psp->state->args[1])
    P_SetPspritePtr(player, psp, psp->state->args[0]);
}

//
// A_ConsumeAmmo
// Subtracts ammo from the player's "inventory". 'Nuff said.
//   args[0]: Amount of ammo to consume. If zero, use the weapon's ammo-per-shot amount.
//
void A_ConsumeAmmo(player_t *player, pspdef_t *psp)
{
  int amount;
  ammotype_t type;

  CHECK_WEAPON_CODEPOINTER("A_ConsumeAmmo", player);

  if (!mbf21)
    return;

  // don't do dumb things, kids
  type = weaponinfo[player->readyweapon].ammo;
  if (player->cheats & CF_INFINITE_AMMO || !psp->state || type == am_noammo)
    return;

  // use the weapon's ammo-per-shot amount if zero.
  // to subtract zero ammo, don't call this function. ;)
  if (psp->state->args[0] != 0)
    amount = psp->state->args[0];
  else
    amount = weaponinfo[player->readyweapon].ammopershot;

  // subtract ammo, but don't let it get below zero
  if (player->ammo[type] >= amount)
    player->ammo[type] -= amount;
  else
    player->ammo[type] = 0;
}

//
// A_CheckAmmo
// Jumps to a state if the player's ammo is lower than the specified amount.
//   args[0]: State to jump to
//   args[1]: Minimum required ammo to NOT jump. If zero, use the weapon's ammo-per-shot amount.
//
void A_CheckAmmo(player_t *player, pspdef_t *psp)
{
  int amount;
  ammotype_t type;

  CHECK_WEAPON_CODEPOINTER("A_CheckAmmo", player);

  if (!mbf21)
    return;

  type = weaponinfo[player->readyweapon].ammo;
  if (!psp->state || type == am_noammo)
    return;

  if (psp->state->args[1] != 0)
    amount = psp->state->args[1];
  else
    amount = weaponinfo[player->readyweapon].ammopershot;

  if (player->ammo[type] < amount)
    P_SetPspritePtr(player, psp, psp->state->args[0]);
}

//
// A_RefireTo
// Jumps to a state if the player is holding down the fire button
//   args[0]: State to jump to
//   args[1]: If nonzero, skip the ammo check
//
void A_RefireTo(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_RefireTo", player);

  if (!mbf21 || !psp->state)
    return;

  if ((psp->state->args[1] || P_CheckAmmo(player))
  &&  (player->cmd.buttons & BT_ATTACK)
  &&  (player->pendingweapon == wp_nochange && player->health))
    P_SetPspritePtr(player, psp, psp->state->args[0]);
}

//
// A_GunFlashTo
// Sets the weapon flash layer to the specified state.
//   args[0]: State number
//   args[1]: If nonzero, don't change the player actor state
//
void A_GunFlashTo(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_GunFlashTo", player);

  if (!mbf21 || !psp->state)
    return;

  if(!psp->state->args[1])
    P_SetMobjState(player->mo, S_PLAY_ATK2);

  P_SetPsprite(player, ps_flash, psp->state->args[0]);
}

//
// P_SetupPsprites
// Called at start of level for each player.
//

void P_SetupPsprites(player_t *player)
{
  int i;

  // remove all psprites
  for (i=0; i<NUMPSPRITES; i++)
    player->psprites[i].state = NULL;

  // spawn the gun
  player->pendingweapon = player->readyweapon;
  P_BringUpWeapon(player);
}

//
// P_MovePsprites
// Called every tic by player thinking routine.
//

void P_MovePsprites(player_t *player)
{
  pspdef_t *psp = player->psprites;
  int i;

  // a null state means not active
  // drop tic count and possibly change state
  // a -1 tic count never changes

  for (i=0; i<NUMPSPRITES; i++, psp++)
    if (psp->state && psp->tics != -1 && !--psp->tics)
      P_SetPsprite(player, i, psp->state->nextstate);

  player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
  player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}

// heretic

#include "heretic/def.h"
#include "p_user.h"
#include "p_maputl.h"

#define MAGIC_JUNK 1234
#define FLAME_THROWER_TICS 10*35

#define MAX_MACE_SPOTS 8

static int MaceSpotCount;
static struct
{
    fixed_t x;
    fixed_t y;
} MaceSpots[MAX_MACE_SPOTS];

void A_BeakReady(player_t * player, pspdef_t * psp)
{
    if (player->cmd.buttons & BT_ATTACK)
    {                           // Chicken beak attack
        player->attackdown = true;
        P_SetMobjState(player->mo, HERETIC_S_CHICPLAY_ATK1);
        if (player->powers[pw_weaponlevel2])
        {
            P_SetPsprite(player, ps_weapon, HERETIC_S_BEAKATK2_1);
        }
        else
        {
            P_SetPsprite(player, ps_weapon, HERETIC_S_BEAKATK1_1);
        }
        P_NoiseAlert(player->mo, player->mo);
    }
    else
    {
        if (player->mo->state == &states[HERETIC_S_CHICPLAY_ATK1])
        {                       // Take out of attack state
            P_SetMobjState(player->mo, HERETIC_S_CHICPLAY);
        }
        player->attackdown = false;
    }
}

void A_BeakRaise(player_t * player, pspdef_t * psp)
{
    psp->sy = WEAPONTOP;
    P_SetPsprite(player, ps_weapon,
                 wpnlev1info[player->readyweapon].readystate);
}

extern mobjtype_t PuffType;

void A_BeakAttackPL1(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;

    damage = 1 + (P_Random(pr_heretic) & 3);
    angle = player->mo->angle;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE, 0);
    PuffType = HERETIC_MT_BEAKPUFF;
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
    if (linetarget)
    {
        player->mo->angle = R_PointToAngle2(player->mo->x,
                                            player->mo->y, linetarget->x,
                                            linetarget->y);
    }
    S_StartMobjSound(player->mo, heretic_sfx_chicpk1 + (P_Random(pr_heretic) % 3));
    player->chickenPeck = 12;
    psp->tics -= P_Random(pr_heretic) & 7;
}

void A_BeakAttackPL2(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;

    damage = HITDICE(4);
    angle = player->mo->angle;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE, 0);
    PuffType = HERETIC_MT_BEAKPUFF;
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
    if (linetarget)
    {
        player->mo->angle = R_PointToAngle2(player->mo->x,
                                            player->mo->y, linetarget->x,
                                            linetarget->y);
    }
    S_StartMobjSound(player->mo, heretic_sfx_chicpk1 + (P_Random(pr_heretic) % 3));
    player->chickenPeck = 12;
    psp->tics -= P_Random(pr_heretic) & 3;
}

void A_StaffAttackPL1(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;

    damage = 5 + (P_Random(pr_heretic) & 15);
    angle = player->mo->angle;
    angle += P_SubRandom() << 18;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE, 0);
    PuffType = HERETIC_MT_STAFFPUFF;
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
    if (linetarget)
    {
        //S_StartMobjSound(player->mo, sfx_stfhit);
        // turn to face target
        player->mo->angle = R_PointToAngle2(player->mo->x,
                                            player->mo->y, linetarget->x,
                                            linetarget->y);
        R_SmoothPlaying_Reset(player); // e6y
    }
}

void A_StaffAttackPL2(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;

    // P_inter.c:P_DamageMobj() handles target momentums
    damage = 18 + (P_Random(pr_heretic) & 63);
    angle = player->mo->angle;
    angle += P_SubRandom() << 18;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE, 0);
    PuffType = HERETIC_MT_STAFFPUFF2;
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
    if (linetarget)
    {
        //S_StartMobjSound(player->mo, sfx_stfpow);
        // turn to face target
        player->mo->angle = R_PointToAngle2(player->mo->x,
                                            player->mo->y, linetarget->x,
                                            linetarget->y);
        R_SmoothPlaying_Reset(player); // e6y
    }
}

void A_FireBlasterPL1(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;
    angle_t angle;
    int damage;

    mo = player->mo;
    S_StartMobjSound(mo, heretic_sfx_gldhit);
    player->ammo[am_blaster] -= USE_BLSR_AMMO_1;
    P_BulletSlope(mo);
    damage = HITDICE(4);
    angle = mo->angle;
    if (player->refire)
    {
        angle += P_SubRandom() << 18;
    }
    PuffType = HERETIC_MT_BLASTERPUFF1;
    P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
    S_StartMobjSound(player->mo, heretic_sfx_blssht);
}

void A_FireBlasterPL2(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    player->ammo[am_blaster] -=
        deathmatch ? USE_BLSR_AMMO_1 : USE_BLSR_AMMO_2;
    mo = P_SpawnPlayerMissile(player->mo, HERETIC_MT_BLASTERFX1);
    if (mo)
    {
        mo->thinker.function = P_BlasterMobjThinker;
    }
    S_StartMobjSound(player->mo, heretic_sfx_blssht);
}

void A_FireGoldWandPL1(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;
    angle_t angle;
    int damage;

    mo = player->mo;
    player->ammo[am_goldwand] -= USE_GWND_AMMO_1;
    P_BulletSlope(mo);
    damage = 7 + (P_Random(pr_heretic) & 7);
    angle = mo->angle;
    if (player->refire)
    {
        angle += P_SubRandom() << 18;
    }
    PuffType = HERETIC_MT_GOLDWANDPUFF1;
    P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
    S_StartMobjSound(player->mo, heretic_sfx_gldhit);
}

void A_FireGoldWandPL2(player_t * player, pspdef_t * psp)
{
    int i;
    mobj_t *mo;
    angle_t angle;
    int damage;
    fixed_t momz;

    mo = player->mo;
    player->ammo[am_goldwand] -=
        deathmatch ? USE_GWND_AMMO_1 : USE_GWND_AMMO_2;
    PuffType = HERETIC_MT_GOLDWANDPUFF2;
    P_BulletSlope(mo);
    momz = FixedMul(mobjinfo[HERETIC_MT_GOLDWANDFX2].speed, bulletslope);
    P_SpawnMissileAngle(mo, HERETIC_MT_GOLDWANDFX2, mo->angle - (ANG45 / 8), momz);
    P_SpawnMissileAngle(mo, HERETIC_MT_GOLDWANDFX2, mo->angle + (ANG45 / 8), momz);
    angle = mo->angle - (ANG45 / 8);
    for (i = 0; i < 5; i++)
    {
        damage = 1 + (P_Random(pr_heretic) & 7);
        P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
        angle += ((ANG45 / 8) * 2) / 4;
    }
    S_StartMobjSound(player->mo, heretic_sfx_gldhit);
}

void A_FireMacePL1B(player_t * player, pspdef_t * psp)
{
    mobj_t *pmo;
    mobj_t *ball;
    angle_t angle;

    if (player->ammo[am_mace] < USE_MACE_AMMO_1)
    {
        return;
    }
    player->ammo[am_mace] -= USE_MACE_AMMO_1;
    pmo = player->mo;

    // Vanilla bug here:
    // Original code here looks like:
    //   (pmo->flags2 & MF2_FEETARECLIPPED != 0)
    // C's operator precedence interprets this as:
    //   (pmo->flags2 & (MF2_FEETARECLIPPED != 0))
    // Which simplifies to:
    //   (pmo->flags2 & 1)
    ball = P_SpawnMobj(pmo->x, pmo->y, pmo->z + 28 * FRACUNIT
                       - FOOTCLIPSIZE * (pmo->flags2 & 1), HERETIC_MT_MACEFX2);

    ball->momz = 2 * FRACUNIT + ((player->lookdir) << (FRACBITS - 5));
    angle = pmo->angle;
    P_SetTarget(&ball->target, pmo);
    ball->angle = angle;
    ball->z += (player->lookdir) << (FRACBITS - 4);
    angle >>= ANGLETOFINESHIFT;
    ball->momx = (pmo->momx >> 1)
        + FixedMul(ball->info->speed, finecosine[angle]);
    ball->momy = (pmo->momy >> 1)
        + FixedMul(ball->info->speed, finesine[angle]);
    S_StartMobjSound(ball, heretic_sfx_lobsht);
    P_CheckMissileSpawn(ball);
}

void A_FireMacePL1(player_t * player, pspdef_t * psp)
{
    mobj_t *ball;

    if (P_Random(pr_heretic) < 28)
    {
        A_FireMacePL1B(player, psp);
        return;
    }
    if (player->ammo[am_mace] < USE_MACE_AMMO_1)
    {
        return;
    }
    player->ammo[am_mace] -= USE_MACE_AMMO_1;
    psp->sx = ((P_Random(pr_heretic) & 3) - 2) * FRACUNIT;
    psp->sy = WEAPONTOP + (P_Random(pr_heretic) & 3) * FRACUNIT;
    ball = P_SPMAngle(player->mo, HERETIC_MT_MACEFX1, player->mo->angle
                      + (((P_Random(pr_heretic) & 7) - 4) << 24));
    if (ball)
    {
        ball->special1.i = 16;    // tics till dropoff
    }
}

void A_MacePL1Check(mobj_t * ball)
{
    angle_t angle;

    if (ball->special1.i == 0)
    {
        return;
    }
    ball->special1.i -= 4;
    if (ball->special1.i > 0)
    {
        return;
    }
    ball->special1.i = 0;
    ball->flags2 |= MF2_LOGRAV;
    angle = ball->angle >> ANGLETOFINESHIFT;
    ball->momx = FixedMul(7 * FRACUNIT, finecosine[angle]);
    ball->momy = FixedMul(7 * FRACUNIT, finesine[angle]);
    ball->momz -= ball->momz >> 1;
}

void A_MaceBallImpact(mobj_t * ball)
{
    if ((ball->z <= ball->floorz) && (P_HitFloor(ball) != FLOOR_SOLID))
    {                           // Landed in some sort of liquid
        P_RemoveMobj(ball);
        return;
    }
    if ((ball->health != MAGIC_JUNK) && (ball->z <= ball->floorz)
        && ball->momz)
    {                           // Bounce
        ball->health = MAGIC_JUNK;
        ball->momz = (ball->momz * 192) >> 8;
        ball->flags2 &= ~MF2_FLOORBOUNCE;
        P_SetMobjState(ball, ball->info->spawnstate);
        S_StartMobjSound(ball, heretic_sfx_bounce);
    }
    else
    {                           // Explode
        ball->flags |= MF_NOGRAVITY;
        ball->flags2 &= ~MF2_LOGRAV;
        S_StartMobjSound(ball, heretic_sfx_lobhit);
    }
}

void A_MaceBallImpact2(mobj_t * ball)
{
    mobj_t *tiny;
    angle_t angle;

    if ((ball->z <= ball->floorz) && (P_HitFloor(ball) != FLOOR_SOLID))
    {                           // Landed in some sort of liquid
        P_RemoveMobj(ball);
        return;
    }
    if ((ball->z != ball->floorz) || (ball->momz < 2 * FRACUNIT))
    {                           // Explode
        ball->momx = ball->momy = ball->momz = 0;
        ball->flags |= MF_NOGRAVITY;
        ball->flags2 &= ~(MF2_LOGRAV | MF2_FLOORBOUNCE);
    }
    else
    {                           // Bounce
        ball->momz = (ball->momz * 192) >> 8;
        P_SetMobjState(ball, ball->info->spawnstate);

        tiny = P_SpawnMobj(ball->x, ball->y, ball->z, HERETIC_MT_MACEFX3);
        angle = ball->angle + ANG90;
        P_SetTarget(&tiny->target, ball->target);
        tiny->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        tiny->momx = (ball->momx >> 1) + FixedMul(ball->momz - FRACUNIT,
                                                  finecosine[angle]);
        tiny->momy = (ball->momy >> 1) + FixedMul(ball->momz - FRACUNIT,
                                                  finesine[angle]);
        tiny->momz = ball->momz;
        P_CheckMissileSpawn(tiny);

        tiny = P_SpawnMobj(ball->x, ball->y, ball->z, HERETIC_MT_MACEFX3);
        angle = ball->angle - ANG90;
        P_SetTarget(&tiny->target, ball->target);
        tiny->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        tiny->momx = (ball->momx >> 1) + FixedMul(ball->momz - FRACUNIT,
                                                  finecosine[angle]);
        tiny->momy = (ball->momy >> 1) + FixedMul(ball->momz - FRACUNIT,
                                                  finesine[angle]);
        tiny->momz = ball->momz;
        P_CheckMissileSpawn(tiny);
    }
}

void A_FireMacePL2(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    player->ammo[am_mace] -= deathmatch ? USE_MACE_AMMO_1 : USE_MACE_AMMO_2;
    mo = P_SpawnPlayerMissile(player->mo, HERETIC_MT_MACEFX4);
    if (mo)
    {
        mo->momx += player->mo->momx;
        mo->momy += player->mo->momy;
        mo->momz = 2 * FRACUNIT + ((player->lookdir) << (FRACBITS - 5));
        if (linetarget)
        {
            P_SetTarget(&mo->special1.m, linetarget);
        }
    }
    S_StartMobjSound(player->mo, heretic_sfx_lobsht);
}

void A_DeathBallImpact(mobj_t * ball)
{
    int i;
    mobj_t *target;
    angle_t angle;
    dboolean newAngle;

    if ((ball->z <= ball->floorz) && (P_HitFloor(ball) != FLOOR_SOLID))
    {                           // Landed in some sort of liquid
        P_RemoveMobj(ball);
        return;
    }
    if ((ball->z <= ball->floorz) && ball->momz)
    {                           // Bounce
        newAngle = false;
        target = (mobj_t *) ball->special1.m;
        if (target)
        {
            if (!(target->flags & MF_SHOOTABLE))
            {                   // Target died
                P_SetTarget(&ball->special1.m, NULL);
            }
            else
            {                   // Seek
                angle = R_PointToAngle2(ball->x, ball->y,
                                        target->x, target->y);
                newAngle = true;
            }
        }
        else
        {                       // Find new target
            angle = 0;
            for (i = 0; i < 16; i++)
            {
                P_AimLineAttack(ball, angle, 10 * 64 * FRACUNIT, 0);
                if (linetarget && ball->target != linetarget)
                {
                    P_SetTarget(&ball->special1.m, linetarget);
                    angle = R_PointToAngle2(ball->x, ball->y,
                                            linetarget->x, linetarget->y);
                    newAngle = true;
                    break;
                }
                angle += ANG45 / 2;
            }
        }
        if (newAngle)
        {
            ball->angle = angle;
            angle >>= ANGLETOFINESHIFT;
            ball->momx = FixedMul(ball->info->speed, finecosine[angle]);
            ball->momy = FixedMul(ball->info->speed, finesine[angle]);
        }
        P_SetMobjState(ball, ball->info->spawnstate);
        S_StartMobjSound(ball, heretic_sfx_pstop);
    }
    else
    {                           // Explode
        ball->flags |= MF_NOGRAVITY;
        ball->flags2 &= ~MF2_LOGRAV;
        S_StartMobjSound(ball, heretic_sfx_phohit);
    }
}

void A_SpawnRippers(mobj_t * actor)
{
    unsigned int i;
    angle_t angle;
    mobj_t *ripper;

    for (i = 0; i < 8; i++)
    {
        ripper = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_RIPPER);
        angle = i * ANG45;
        P_SetTarget(&ripper->target, actor->target);
        ripper->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        ripper->momx = FixedMul(ripper->info->speed, finecosine[angle]);
        ripper->momy = FixedMul(ripper->info->speed, finesine[angle]);
        P_CheckMissileSpawn(ripper);
    }
}

void A_FireCrossbowPL1(player_t * player, pspdef_t * psp)
{
    mobj_t *pmo;

    pmo = player->mo;
    player->ammo[am_crossbow] -= USE_CBOW_AMMO_1;
    P_SpawnPlayerMissile(pmo, HERETIC_MT_CRBOWFX1);
    P_SPMAngle(pmo, HERETIC_MT_CRBOWFX3, pmo->angle - (ANG45 / 10));
    P_SPMAngle(pmo, HERETIC_MT_CRBOWFX3, pmo->angle + (ANG45 / 10));
}

void A_FireCrossbowPL2(player_t * player, pspdef_t * psp)
{
    mobj_t *pmo;

    pmo = player->mo;
    player->ammo[am_crossbow] -=
        deathmatch ? USE_CBOW_AMMO_1 : USE_CBOW_AMMO_2;
    P_SpawnPlayerMissile(pmo, HERETIC_MT_CRBOWFX2);
    P_SPMAngle(pmo, HERETIC_MT_CRBOWFX2, pmo->angle - (ANG45 / 10));
    P_SPMAngle(pmo, HERETIC_MT_CRBOWFX2, pmo->angle + (ANG45 / 10));
    P_SPMAngle(pmo, HERETIC_MT_CRBOWFX3, pmo->angle - (ANG45 / 5));
    P_SPMAngle(pmo, HERETIC_MT_CRBOWFX3, pmo->angle + (ANG45 / 5));
}

void A_BoltSpark(mobj_t * bolt)
{
    mobj_t *spark;

    if (P_Random(pr_heretic) > 50)
    {
        spark = P_SpawnMobj(bolt->x, bolt->y, bolt->z, HERETIC_MT_CRBOWFX4);
        spark->x += P_SubRandom() << 10;
        spark->y += P_SubRandom() << 10;
    }
}

void A_FireSkullRodPL1(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    if (player->ammo[am_skullrod] < USE_SKRD_AMMO_1)
    {
        return;
    }
    player->ammo[am_skullrod] -= USE_SKRD_AMMO_1;
    mo = P_SpawnPlayerMissile(player->mo, HERETIC_MT_HORNRODFX1);
    // Randomize the first frame
    if (mo && P_Random(pr_heretic) > 128)
    {
        P_SetMobjState(mo, HERETIC_S_HRODFX1_2);
    }
}

void A_FireSkullRodPL2(player_t * player, pspdef_t * psp)
{
    player->ammo[am_skullrod] -=
        deathmatch ? USE_SKRD_AMMO_1 : USE_SKRD_AMMO_2;
    P_SpawnPlayerMissile(player->mo, HERETIC_MT_HORNRODFX2);
    // Use MissileMobj instead of the return value from
    // P_SpawnPlayerMissile because we need to give info to the mobj
    // even if it exploded immediately.
    if (netgame)
    {                           // Multi-player game
        MissileMobj->special2.i = P_GetPlayerNum(player);
    }
    else
    {                           // Always use red missiles in single player games
        MissileMobj->special2.i = 2;
    }
    if (linetarget)
    {
        P_SetTarget(&MissileMobj->special1.m, linetarget);
    }
    S_StartMobjSound(MissileMobj, heretic_sfx_hrnpow);
}

void A_SkullRodPL2Seek(mobj_t * actor)
{
    P_SeekerMissile(actor, &actor->special1.m, ANG1_X * 10, ANG1_X * 30, false);
}

void A_AddPlayerRain(mobj_t * actor)
{
    int playerNum;
    player_t *player;

    playerNum = netgame ? actor->special2.i : 0;
    if (!playeringame[playerNum])
    {                           // Player left the game
        return;
    }
    player = &players[playerNum];
    if (player->health <= 0)
    {                           // Player is dead
        return;
    }
    if (player->rain1 && player->rain2)
    {                           // Terminate an active rain
        if (player->rain1->health < player->rain2->health)
        {
            if (player->rain1->health > 16)
            {
                player->rain1->health = 16;
            }
            player->rain1 = NULL;
        }
        else
        {
            if (player->rain2->health > 16)
            {
                player->rain2->health = 16;
            }
            player->rain2 = NULL;
        }
    }
    // Add rain mobj to list
    if (player->rain1)
    {
        player->rain2 = actor;
    }
    else
    {
        player->rain1 = actor;
    }
}

void A_SkullRodStorm(mobj_t * actor)
{
    fixed_t x;
    fixed_t y;
    mobj_t *mo;
    int playerNum;
    player_t *player;

    if (actor->health-- == 0)
    {
        P_SetMobjState(actor, g_s_null);
        playerNum = netgame ? actor->special2.i : 0;
        if (!playeringame[playerNum])
        {                       // Player left the game
            return;
        }
        player = &players[playerNum];
        if (player->health <= 0)
        {                       // Player is dead
            return;
        }
        if (player->rain1 == actor)
        {
            player->rain1 = NULL;
        }
        else if (player->rain2 == actor)
        {
            player->rain2 = NULL;
        }
        return;
    }
    if (P_Random(pr_heretic) < 25)
    {                           // Fudge rain frequency
        return;
    }
    x = actor->x + ((P_Random(pr_heretic) & 127) - 64) * FRACUNIT;
    y = actor->y + ((P_Random(pr_heretic) & 127) - 64) * FRACUNIT;
    mo = P_SpawnMobj(x, y, ONCEILINGZ, HERETIC_MT_RAINPLR1 + actor->special2.i);
    P_SetTarget(&mo->target, actor->target);
    mo->momx = 1;               // Force collision detection
    mo->momz = -mo->info->speed;
    mo->special2.i = actor->special2.i;     // Transfer player number
    P_CheckMissileSpawn(mo);
    if (!(actor->special1.i & 31))
    {
        S_StartMobjSound(actor, heretic_sfx_ramrain);
    }
    actor->special1.i++;
}

void A_RainImpact(mobj_t * actor)
{
    if (actor->z > actor->floorz)
    {
        P_SetMobjState(actor, HERETIC_S_RAINAIRXPLR1_1 + actor->special2.i);
    }
    else if (P_Random(pr_heretic) < 40)
    {
        P_HitFloor(actor);
    }
}

void A_HideInCeiling(mobj_t * actor)
{
    actor->z = actor->ceilingz + 4 * FRACUNIT;
}

void A_FirePhoenixPL1(player_t * player, pspdef_t * psp)
{
    angle_t angle;

    player->ammo[am_phoenixrod] -= USE_PHRD_AMMO_1;
    P_SpawnPlayerMissile(player->mo, HERETIC_MT_PHOENIXFX1);
    angle = player->mo->angle + ANG180;
    angle >>= ANGLETOFINESHIFT;
    player->mo->momx += FixedMul(4 * FRACUNIT, finecosine[angle]);
    player->mo->momy += FixedMul(4 * FRACUNIT, finesine[angle]);
}

void A_PhoenixPuff(mobj_t * actor)
{
    mobj_t *puff;
    angle_t angle;

    P_SeekerMissile(actor, &actor->special1.m, ANG1_X * 5, ANG1_X * 10, false);
    puff = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_PHOENIXPUFF);
    angle = actor->angle + ANG90;
    angle >>= ANGLETOFINESHIFT;
    puff->momx = FixedMul((fixed_t)(FRACUNIT * 1.3), finecosine[angle]);
    puff->momy = FixedMul((fixed_t)(FRACUNIT * 1.3), finesine[angle]);
    puff->momz = 0;
    puff = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_PHOENIXPUFF);
    angle = actor->angle - ANG90;
    angle >>= ANGLETOFINESHIFT;
    puff->momx = FixedMul((fixed_t)(FRACUNIT * 1.3), finecosine[angle]);
    puff->momy = FixedMul((fixed_t)(FRACUNIT * 1.3), finesine[angle]);
    puff->momz = 0;
}

//
// This function was present in the Heretic 1.0 executable for the
// removed "secondary phoenix flash" object (MT_PHOENIXFX_REMOVED).
// The purpose of this object is unknown, as is this function.
//

void A_RemovedPhoenixFunc(mobj_t *actor)
{
    return;
}

void A_InitPhoenixPL2(player_t * player, pspdef_t * psp)
{
    player->flamecount = FLAME_THROWER_TICS;
}

void A_FirePhoenixPL2(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;
    mobj_t *pmo;
    angle_t angle;
    fixed_t x, y, z;
    fixed_t slope;

    if (--player->flamecount == 0)
    {                           // Out of flame
        P_SetPsprite(player, ps_weapon, HERETIC_S_PHOENIXATK2_4);
        player->refire = 0;
        return;
    }
    pmo = player->mo;
    angle = pmo->angle;
    x = pmo->x + (P_SubRandom() << 9);
    y = pmo->y + (P_SubRandom() << 9);
    z = pmo->z + 26 * FRACUNIT + ((player->lookdir) << FRACBITS) / 173;
    if (pmo->flags2 & MF2_FEETARECLIPPED)
    {
        z -= FOOTCLIPSIZE;
    }
    slope = ((player->lookdir) << FRACBITS) / 173 + (FRACUNIT / 10);
    mo = P_SpawnMobj(x, y, z, HERETIC_MT_PHOENIXFX2);
    P_SetTarget(&mo->target, pmo);
    mo->angle = angle;
    mo->momx = pmo->momx + FixedMul(mo->info->speed,
                                    finecosine[angle >> ANGLETOFINESHIFT]);
    mo->momy = pmo->momy + FixedMul(mo->info->speed,
                                    finesine[angle >> ANGLETOFINESHIFT]);
    mo->momz = FixedMul(mo->info->speed, slope);
    if (!player->refire || !(leveltime % 38))
    {
        S_StartMobjSound(player->mo, heretic_sfx_phopow);
    }
    P_CheckMissileSpawn(mo);
}

void A_ShutdownPhoenixPL2(player_t * player, pspdef_t * psp)
{
    player->ammo[am_phoenixrod] -= USE_PHRD_AMMO_2;
}

void A_FlameEnd(mobj_t * actor)
{
    actor->momz += (fixed_t)(1.5 * FRACUNIT);
}

void A_FloatPuff(mobj_t * puff)
{
    puff->momz += (fixed_t)(1.8 * FRACUNIT);
}

void A_GauntletAttack(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;
    int randVal;
    fixed_t dist;

    psp->sx = ((P_Random(pr_heretic) & 3) - 2) * FRACUNIT;
    psp->sy = WEAPONTOP + (P_Random(pr_heretic) & 3) * FRACUNIT;
    angle = player->mo->angle;
    if (player->powers[pw_weaponlevel2])
    {
        damage = HITDICE(2);
        dist = 4 * MELEERANGE;
        angle += P_SubRandom() << 17;
        PuffType = HERETIC_MT_GAUNTLETPUFF2;
    }
    else
    {
        damage = HITDICE(2);
        dist = MELEERANGE + 1;
        angle += P_SubRandom() << 18;
        PuffType = HERETIC_MT_GAUNTLETPUFF1;
    }
    slope = P_AimLineAttack(player->mo, angle, dist, 0);
    P_LineAttack(player->mo, angle, dist, slope, damage);
    if (!linetarget)
    {
        if (P_Random(pr_heretic) > 64)
        {
            player->extralight = !player->extralight;
        }
        S_StartMobjSound(player->mo, heretic_sfx_gntful);
        return;
    }
    randVal = P_Random(pr_heretic);
    if (randVal < 64)
    {
        player->extralight = 0;
    }
    else if (randVal < 160)
    {
        player->extralight = 1;
    }
    else
    {
        player->extralight = 2;
    }
    if (player->powers[pw_weaponlevel2])
    {
        P_GiveBody(player, damage >> 1);
        S_StartMobjSound(player->mo, heretic_sfx_gntpow);
    }
    else
    {
        S_StartMobjSound(player->mo, heretic_sfx_gnthit);
    }
    // turn to face target
    angle = R_PointToAngle2(player->mo->x, player->mo->y,
                            linetarget->x, linetarget->y);
    if (angle - player->mo->angle > ANG180)
    {
        if (angle - player->mo->angle < -ANG90 / 20)
            player->mo->angle = angle + ANG90 / 21;
        else
            player->mo->angle -= ANG90 / 20;
    }
    else
    {
        if (angle - player->mo->angle > ANG90 / 20)
            player->mo->angle = angle - ANG90 / 21;
        else
            player->mo->angle += ANG90 / 20;
    }
    player->mo->flags |= MF_JUSTATTACKED;
    R_SmoothPlaying_Reset(player); // e6y
}

void P_RepositionMace(mobj_t * mo)
{
    int spot;
    subsector_t *ss;

    P_UnsetThingPosition(mo);
    spot = P_Random(pr_heretic) % MaceSpotCount;
    mo->x = MaceSpots[spot].x;
    mo->y = MaceSpots[spot].y;
    ss = R_PointInSubsector(mo->x, mo->y);
    mo->z = mo->floorz = ss->sector->floorheight;
    mo->ceilingz = ss->sector->ceilingheight;
    P_SetThingPosition(mo);
}

void P_ActivateBeak(player_t * player)
{
    player->pendingweapon = wp_nochange;
    player->readyweapon = wp_beak;
    player->psprites[ps_weapon].sy = WEAPONTOP;
    P_SetPsprite(player, ps_weapon, HERETIC_S_BEAKREADY);
}

void P_PostChickenWeapon(player_t * player, weapontype_t weapon)
{
    if (weapon == wp_beak)
    {                           // Should never happen
        weapon = wp_staff;
    }
    player->pendingweapon = wp_nochange;
    player->readyweapon = weapon;
    player->psprites[ps_weapon].sy = WEAPONBOTTOM;
    P_SetPsprite(player, ps_weapon, wpnlev1info[weapon].upstate);
}

void P_UpdateBeak(player_t * player, pspdef_t * psp)
{
    psp->sy = WEAPONTOP + (player->chickenPeck << (FRACBITS - 1));
}

static dboolean Heretic_P_CheckAmmo(player_t * player)
{
    ammotype_t ammo;
    weaponinfo_t *checkweaponinfo;
    int count;

    ammo = wpnlev1info[player->readyweapon].ammo;
    if (player->powers[pw_weaponlevel2] && !deathmatch)
    {
      checkweaponinfo = wpnlev2info;
    }
    else
    {
      checkweaponinfo = wpnlev1info;
    }
    count = checkweaponinfo[player->readyweapon].ammopershot;
    if (ammo == am_noammo || player->ammo[ammo] >= count)
    {
        return (true);
    }
    // out of ammo, pick a weapon to change to
    do
    {
        if (player->weaponowned[wp_skullrod]
            && player->ammo[am_skullrod] > checkweaponinfo[wp_skullrod].ammopershot)
        {
            player->pendingweapon = wp_skullrod;
        }
        else if (player->weaponowned[wp_blaster]
                 && player->ammo[am_blaster] > checkweaponinfo[wp_blaster].ammopershot)
        {
            player->pendingweapon = wp_blaster;
        }
        else if (player->weaponowned[wp_crossbow]
                 && player->ammo[am_crossbow] > checkweaponinfo[wp_crossbow].ammopershot)
        {
            player->pendingweapon = wp_crossbow;
        }
        else if (player->weaponowned[wp_mace]
                 && player->ammo[am_mace] > checkweaponinfo[wp_mace].ammopershot)
        {
            player->pendingweapon = wp_mace;
        }
        else if (player->ammo[am_goldwand] > checkweaponinfo[wp_goldwand].ammopershot)
        {
            player->pendingweapon = wp_goldwand;
        }
        else if (player->weaponowned[wp_gauntlets])
        {
            player->pendingweapon = wp_gauntlets;
        }
        else if (player->weaponowned[wp_phoenixrod]
                 && player->ammo[am_phoenixrod] > checkweaponinfo[wp_phoenixrod].ammopershot)
        {
            player->pendingweapon = wp_phoenixrod;
        }
        else
        {
            player->pendingweapon = wp_staff;
        }
    }
    while (player->pendingweapon == wp_nochange);
    if (player->powers[pw_weaponlevel2])
    {
        P_SetPsprite(player, ps_weapon,
                     wpnlev2info[player->readyweapon].downstate);
    }
    else
    {
        P_SetPsprite(player, ps_weapon,
                     wpnlev1info[player->readyweapon].downstate);
    }
    return (false);
}

void P_OpenWeapons(void)
{
    MaceSpotCount = 0;
}

void P_AddMaceSpot(const mapthing_t * mthing)
{
    if (MaceSpotCount == MAX_MACE_SPOTS)
    {
        I_Error("Too many mace spots.");
    }
    MaceSpots[MaceSpotCount].x = mthing->x;
    MaceSpots[MaceSpotCount].y = mthing->y;
    MaceSpotCount++;
}

void P_CloseWeapons(void)
{
    int spot;

    if (!MaceSpotCount)
    {                           // No maces placed
        return;
    }
    if (!deathmatch && P_Random(pr_heretic) < 64)
    {                           // Sometimes doesn't show up if not in deathmatch
        return;
    }
    spot = P_Random(pr_heretic) % MaceSpotCount;
    P_SpawnMobj(MaceSpots[spot].x, MaceSpots[spot].y, ONFLOORZ, HERETIC_MT_WMACE);
}

// hexen

#include "heretic/sb_bar.h"

extern fixed_t FloatBobOffsets[64];

static int WeaponManaUse[NUMCLASSES][HEXEN_NUMWEAPONS] = {
    [PCLASS_FIGHTER] = {0, 2, 3, 14},
                       {0, 1, 4, 18},
                       {0, 3, 5, 15},
                       {0, 0, 0, 0}
};

void P_SetPspriteNF(player_t * player, int position, statenum_t stnum)
{
    pspdef_t *psp;
    state_t *state;

    psp = &player->psprites[position];
    do
    {
        if (!stnum)
        {                       // Object removed itself.
            psp->state = NULL;
            break;
        }
        state = &states[stnum];
        psp->state = state;
        psp->tics = state->tics;        // could be 0
        if (state->misc1)
        {                       // Set coordinates.
            psp->sx = state->misc1 << FRACBITS;
        }
        if (state->misc2)
        {
            psp->sy = state->misc2 << FRACBITS;
        }
        stnum = psp->state->nextstate;
    }
    while (!psp->tics);         // An initial state of 0 could cycle through.
}

void P_ActivateMorphWeapon(player_t * player)
{
    player->pendingweapon = wp_nochange;
    player->psprites[ps_weapon].sy = WEAPONTOP;
    player->readyweapon = wp_first;     // Snout is the first weapon
    P_SetPsprite(player, ps_weapon, HEXEN_S_SNOUTREADY);
}

void P_PostMorphWeapon(player_t * player, weapontype_t weapon)
{
    player->pendingweapon = wp_nochange;
    player->readyweapon = weapon;
    player->psprites[ps_weapon].sy = WEAPONBOTTOM;
    P_SetPsprite(player, ps_weapon,
                 hexen_weaponinfo[weapon][player->pclass].upstate);
}

static dboolean P_CheckMana(player_t * player)
{
    manatype_t mana;
    int count;

    mana = hexen_weaponinfo[player->readyweapon][player->pclass].ammo;
    count = WeaponManaUse[player->pclass][player->readyweapon];
    if (mana == MANA_BOTH)
    {
        if (player->ammo[MANA_1] >= count && player->ammo[MANA_2] >= count)
        {
            return true;
        }
    }
    else if (mana == MANA_NONE || player->ammo[mana] >= count)
    {
        return (true);
    }

    // out of mana, pick a weapon to change to
    if (player->weaponowned[wp_third]
        && player->ammo[MANA_2] >= WeaponManaUse[player->pclass][wp_third])
    {
        player->pendingweapon = wp_third;
    }
    else if (player->weaponowned[wp_second]
             && player->ammo[MANA_1] >=
             WeaponManaUse[player->pclass][wp_second])
    {
        player->pendingweapon = wp_second;
    }
    else if (player->weaponowned[wp_fourth]
             && player->ammo[MANA_1] >=
             WeaponManaUse[player->pclass][wp_fourth]
             && player->ammo[MANA_2] >=
             WeaponManaUse[player->pclass][wp_fourth])
    {
        player->pendingweapon = wp_fourth;
    }
    else
    {
        player->pendingweapon = wp_first;
    }

    P_SetPsprite(player, ps_weapon,
                 hexen_weaponinfo[player->readyweapon][player->pclass].downstate);
    return (false);
}

#define MAX_ANGADJUST (5*ANG1)

void AdjustPlayerAngle(mobj_t * pmo)
{
    angle_t angle;
    int difference;

    angle = R_PointToAngle2(pmo->x, pmo->y, linetarget->x, linetarget->y);
    difference = (int) angle - (int) pmo->angle;
    if (abs(difference) > MAX_ANGADJUST)
    {
        pmo->angle += difference > 0 ? MAX_ANGADJUST : -MAX_ANGADJUST;
    }
    else
    {
        pmo->angle = angle;
    }
    R_SmoothPlaying_Reset(pmo->player);
}

mobj_t *PuffSpawned;

void A_SnoutAttack(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;

    damage = 3 + (P_Random(pr_hexen) & 3);
    angle = player->mo->angle;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE, 0);
    PuffType = HEXEN_MT_SNOUTPUFF;
    PuffSpawned = NULL;
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
    S_StartMobjSound(player->mo, hexen_sfx_pig_active1 + (P_Random(pr_hexen) & 1));
    if (linetarget)
    {
        AdjustPlayerAngle(player->mo);
        if (PuffSpawned)
        {                       // Bit something
            S_StartMobjSound(player->mo, hexen_sfx_pig_attack);
        }
    }
}

#define HAMMER_RANGE (MELEERANGE+MELEERANGE/2)

void A_FHammerAttack(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    mobj_t *pmo = player->mo;
    int damage;
    fixed_t power;
    int slope;
    int i;

    damage = 60 + (P_Random(pr_hexen) & 63);
    power = 10 * FRACUNIT;
    PuffType = HEXEN_MT_HAMMERPUFF;
    for (i = 0; i < 16; i++)
    {
        angle = pmo->angle + i * (ANG45 / 32);
        slope = P_AimLineAttack(pmo, angle, HAMMER_RANGE, 0);
        if (linetarget)
        {
            P_LineAttack(pmo, angle, HAMMER_RANGE, slope, damage);
            AdjustPlayerAngle(pmo);
            if (linetarget->flags & MF_COUNTKILL || linetarget->player)
            {
                P_ThrustMobj(linetarget, angle, power);
            }
            pmo->special1.i = false;      // Don't throw a hammer
            goto hammerdone;
        }
        angle = pmo->angle - i * (ANG45 / 32);
        slope = P_AimLineAttack(pmo, angle, HAMMER_RANGE, 0);
        if (linetarget)
        {
            P_LineAttack(pmo, angle, HAMMER_RANGE, slope, damage);
            AdjustPlayerAngle(pmo);
            if (linetarget->flags & MF_COUNTKILL || linetarget->player)
            {
                P_ThrustMobj(linetarget, angle, power);
            }
            pmo->special1.i = false;      // Don't throw a hammer
            goto hammerdone;
        }
    }
    // didn't find any targets in meleerange, so set to throw out a hammer
    PuffSpawned = NULL;
    angle = pmo->angle;
    slope = P_AimLineAttack(pmo, angle, HAMMER_RANGE, 0);
    P_LineAttack(pmo, angle, HAMMER_RANGE, slope, damage);
    if (PuffSpawned)
    {
        pmo->special1.i = false;
    }
    else
    {
        pmo->special1.i = true;
    }
  hammerdone:
    if (player->ammo[MANA_2] < WeaponManaUse[player->pclass][player->readyweapon])
    {                           // Don't spawn a hammer if the player doesn't have enough mana
        pmo->special1.i = false;
    }
    return;
}

void A_FHammerThrow(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    if (!player->mo->special1.i)
    {
        return;
    }
    player->ammo[MANA_2] -= WeaponManaUse[player->pclass][player->readyweapon];
    mo = P_SpawnPlayerMissile(player->mo, HEXEN_MT_HAMMER_MISSILE);
    if (mo)
    {
        mo->special1.i = 0;
    }
}

void A_FSwordAttack(player_t * player, pspdef_t * psp)
{
    mobj_t *pmo;

    player->ammo[MANA_1] -= WeaponManaUse[player->pclass][player->readyweapon];
    player->ammo[MANA_2] -= WeaponManaUse[player->pclass][player->readyweapon];
    pmo = player->mo;
    P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z - 10 * FRACUNIT,
                  HEXEN_MT_FSWORD_MISSILE, pmo->angle + ANG45 / 4);
    P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z - 5 * FRACUNIT,
                  HEXEN_MT_FSWORD_MISSILE, pmo->angle + ANG45 / 8);
    P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z, HEXEN_MT_FSWORD_MISSILE, pmo->angle);
    P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z + 5 * FRACUNIT,
                  HEXEN_MT_FSWORD_MISSILE, pmo->angle - ANG45 / 8);
    P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z + 10 * FRACUNIT,
                  HEXEN_MT_FSWORD_MISSILE, pmo->angle - ANG45 / 4);
    S_StartMobjSound(pmo, hexen_sfx_fighter_sword_fire);
}

void A_FSwordAttack2(mobj_t * actor)
{
    angle_t angle = actor->angle;

    P_SpawnMissileAngle(actor, HEXEN_MT_FSWORD_MISSILE, angle + ANG45 / 4, 0);
    P_SpawnMissileAngle(actor, HEXEN_MT_FSWORD_MISSILE, angle + ANG45 / 8, 0);
    P_SpawnMissileAngle(actor, HEXEN_MT_FSWORD_MISSILE, angle, 0);
    P_SpawnMissileAngle(actor, HEXEN_MT_FSWORD_MISSILE, angle - ANG45 / 8, 0);
    P_SpawnMissileAngle(actor, HEXEN_MT_FSWORD_MISSILE, angle - ANG45 / 4, 0);
    S_StartMobjSound(actor, hexen_sfx_fighter_sword_fire);
}

void A_FSwordFlames(mobj_t * actor)
{
    int i;
    int r1,r2,r3;

    for (i = 1 + (P_Random(pr_hexen) & 3); i; i--)
    {
        r1 = P_Random(pr_hexen);
        r2 = P_Random(pr_hexen);
        r3 = P_Random(pr_hexen);
        P_SpawnMobj(actor->x + ((r3 - 128) << 12), actor->y
                    + ((r2 - 128) << 12),
                    actor->z + ((r1 - 128) << 11), HEXEN_MT_FSWORD_FLAME);
    }
}

void A_MWandAttack(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    mo = P_SpawnPlayerMissile(player->mo, HEXEN_MT_MWAND_MISSILE);
    if (mo)
    {
        mo->thinker.function = P_BlasterMobjThinker;
    }
    S_StartMobjSound(player->mo, hexen_sfx_mage_wand_fire);
}

void A_LightningReady(player_t * player, pspdef_t * psp)
{
    A_WeaponReady(player, psp);
    if (P_Random(pr_hexen) < 160)
    {
        S_StartMobjSound(player->mo, hexen_sfx_mage_lightning_ready);
    }
}

#define ZAGSPEED FRACUNIT

void A_LightningClip(mobj_t * actor)
{
    mobj_t *cMo;
    mobj_t *target = NULL;
    int zigZag;

    if (actor->type == HEXEN_MT_LIGHTNING_FLOOR)
    {
        actor->z = actor->floorz;
        target = actor->special2.m->special1.m;
    }
    else if (actor->type == HEXEN_MT_LIGHTNING_CEILING)
    {
        actor->z = actor->ceilingz - actor->height;
        target = actor->special1.m;
    }
    if (actor->type == HEXEN_MT_LIGHTNING_FLOOR)
    {                           // floor lightning zig-zags, and forces the ceiling lightning to mimic
        cMo = actor->special2.m;
        zigZag = P_Random(pr_hexen);
        if ((zigZag > 128 && actor->special1.i < 2) || actor->special1.i < -2)
        {
            P_ThrustMobj(actor, actor->angle + ANG90, ZAGSPEED);
            if (cMo)
            {
                P_ThrustMobj(cMo, actor->angle + ANG90, ZAGSPEED);
            }
            actor->special1.i++;
        }
        else
        {
            P_ThrustMobj(actor, actor->angle - ANG90, ZAGSPEED);
            if (cMo)
            {
                P_ThrustMobj(cMo, cMo->angle - ANG90, ZAGSPEED);
            }
            actor->special1.i--;
        }
    }
    if (target)
    {
        if (target->health <= 0)
        {
            P_ExplodeMissile(actor);
        }
        else
        {
            actor->angle = R_PointToAngle2(actor->x, actor->y, target->x,
                                           target->y);
            actor->momx = 0;
            actor->momy = 0;
            P_ThrustMobj(actor, actor->angle, actor->info->speed >> 1);
        }
    }
}

void A_LightningZap(mobj_t * actor)
{
    mobj_t *mo;
    fixed_t deltaZ;
    int r1,r2;

    A_LightningClip(actor);

    actor->health -= 8;
    if (actor->health <= 0)
    {
        P_SetMobjState(actor, actor->info->deathstate);
        return;
    }
    if (actor->type == HEXEN_MT_LIGHTNING_FLOOR)
    {
        deltaZ = 10 * FRACUNIT;
    }
    else
    {
        deltaZ = -10 * FRACUNIT;
    }
    r1 = P_Random(pr_hexen);
    r2 = P_Random(pr_hexen);
    mo = P_SpawnMobj(actor->x + ((r2 - 128) * actor->radius / 256),
                     actor->y + ((r1 - 128) * actor->radius / 256),
                     actor->z + deltaZ, HEXEN_MT_LIGHTNING_ZAP);
    if (mo)
    {
        P_SetTarget(&mo->special2.m, actor);
        mo->momx = actor->momx;
        mo->momy = actor->momy;
        P_SetTarget(&mo->target, actor->target);
        if (actor->type == HEXEN_MT_LIGHTNING_FLOOR)
        {
            mo->momz = 20 * FRACUNIT;
        }
        else
        {
            mo->momz = -20 * FRACUNIT;
        }
    }

    if (actor->type == HEXEN_MT_LIGHTNING_FLOOR && P_Random(pr_hexen) < 160)
    {
        S_StartMobjSound(actor, hexen_sfx_mage_lightning_continuous);
    }
}

void A_MLightningAttack2(mobj_t * actor)
{
    mobj_t *fmo, *cmo;

    fmo = P_SpawnPlayerMissile(actor, HEXEN_MT_LIGHTNING_FLOOR);
    cmo = P_SpawnPlayerMissile(actor, HEXEN_MT_LIGHTNING_CEILING);
    if (fmo)
    {
        P_SetTarget(&fmo->special1.m, NULL);
        P_SetTarget(&fmo->special2.m, cmo);
        A_LightningZap(fmo);
    }
    if (cmo)
    {
        P_SetTarget(&cmo->special1.m, NULL);      // mobj that it will track
        P_SetTarget(&cmo->special2.m, fmo);
        A_LightningZap(cmo);
    }
    S_StartMobjSound(actor, hexen_sfx_mage_lightning_fire);
}

void A_MLightningAttack(player_t * player, pspdef_t * psp)
{
    A_MLightningAttack2(player->mo);
    player->ammo[MANA_2] -= WeaponManaUse[player->pclass][player->readyweapon];
}

void A_ZapMimic(mobj_t * actor)
{
    mobj_t *mo;

    mo = actor->special2.m;
    if (mo)
    {
        if (mo->state >= &states[mo->info->deathstate]
            || mo->state == &states[HEXEN_S_FREETARGMOBJ])
        {
            P_ExplodeMissile(actor);
        }
        else
        {
            actor->momx = mo->momx;
            actor->momy = mo->momy;
        }
    }
}

void A_LastZap(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_LIGHTNING_ZAP);
    if (mo)
    {
        P_SetMobjState(mo, HEXEN_S_LIGHTNING_ZAP_X1);
        mo->momz = 40 * FRACUNIT;
    }
}

void A_LightningRemove(mobj_t * actor)
{
    mobj_t *mo;

    mo = actor->special2.m;
    if (mo)
    {
        P_SetTarget(&mo->special2.m, NULL);
        P_ExplodeMissile(mo);
    }
}

void MStaffSpawn(mobj_t * pmo, angle_t angle)
{
    mobj_t *mo;

    mo = P_SPMAngle(pmo, HEXEN_MT_MSTAFF_FX2, angle);
    if (mo)
    {
        P_SetTarget(&mo->target, pmo);
        P_SetTarget(&mo->special1.m, P_RoughTargetSearch(mo, 0, 10));
    }
}

void A_MStaffAttack(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    mobj_t *pmo;

    player->ammo[MANA_1] -= WeaponManaUse[player->pclass][player->readyweapon];
    player->ammo[MANA_2] -= WeaponManaUse[player->pclass][player->readyweapon];
    pmo = player->mo;
    angle = pmo->angle;

    MStaffSpawn(pmo, angle);
    MStaffSpawn(pmo, angle - ANG1 * 5);
    MStaffSpawn(pmo, angle + ANG1 * 5);
    S_StartMobjSound(player->mo, hexen_sfx_mage_staff_fire);
    if (player == &players[consoleplayer])
    {
        player->damagecount = 0;
        player->bonuscount = 0;
        V_SetPalette(STARTSCOURGEPAL);
        SB_Start();
    }
}

void A_MStaffPalette(player_t * player, pspdef_t * psp)
{
    int pal;

    if (player == &players[consoleplayer])
    {
        pal = STARTSCOURGEPAL + psp->state - (&states[HEXEN_S_MSTAFFATK_2]);
        if (pal == STARTSCOURGEPAL + 3)
        {                       // reset back to original playpal
            pal = 0;
        }
        V_SetPalette(pal);
        SB_Start();
    }
}

void A_MStaffWeave(mobj_t * actor)
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
    weaveXY = (weaveXY + 6) & 63;
    newX += FixedMul(finecosine[angle], FloatBobOffsets[weaveXY] << 2);
    newY += FixedMul(finesine[angle], FloatBobOffsets[weaveXY] << 2);
    P_TryMove(actor, newX, newY, 0);
    actor->z -= FloatBobOffsets[weaveZ] << 1;
    weaveZ = (weaveZ + 3) & 63;
    actor->z += FloatBobOffsets[weaveZ] << 1;
    if (actor->z <= actor->floorz)
    {
        actor->z = actor->floorz + FRACUNIT;
    }
    actor->special2.i = weaveZ + (weaveXY << 16);
}

void A_MStaffTrack(mobj_t * actor)
{
    if ((actor->special1.m == NULL) && (P_Random(pr_hexen) < 50))
    {
        P_SetTarget(&actor->special1.m, P_RoughTargetSearch(actor, 0, 10));
    }
    P_SeekerMissile(actor, &actor->special1.m, ANG1 * 2, ANG1 * 10, false);
}

void MStaffSpawn2(mobj_t * actor, angle_t angle)
{
    mobj_t *mo;

    mo = P_SpawnMissileAngle(actor, HEXEN_MT_MSTAFF_FX2, angle, 0);
    if (mo)
    {
        P_SetTarget(&mo->target, actor);
        P_SetTarget(&mo->special1.m, P_RoughTargetSearch(mo, 0, 10));
    }
}

void A_MStaffAttack2(mobj_t * actor)
{
    angle_t angle;
    angle = actor->angle;
    MStaffSpawn2(actor, angle);
    MStaffSpawn2(actor, angle - ANG1 * 5);
    MStaffSpawn2(actor, angle + ANG1 * 5);
    S_StartMobjSound(actor, hexen_sfx_mage_staff_fire);
}

void A_FPunchAttack(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;
    mobj_t *pmo = player->mo;
    fixed_t power;
    int i;

    damage = 40 + (P_Random(pr_hexen) & 15);
    power = 2 * FRACUNIT;
    PuffType = HEXEN_MT_PUNCHPUFF;
    for (i = 0; i < 16; i++)
    {
        angle = pmo->angle + i * (ANG45 / 16);
        slope = P_AimLineAttack(pmo, angle, 2 * MELEERANGE, 0);
        if (linetarget)
        {
            player->mo->special1.i++;
            if (pmo->special1.i == 3)
            {
                damage <<= 1;
                power = 6 * FRACUNIT;
                PuffType = HEXEN_MT_HAMMERPUFF;
            }
            P_LineAttack(pmo, angle, 2 * MELEERANGE, slope, damage);
            if (linetarget->flags & MF_COUNTKILL || linetarget->player)
            {
                P_ThrustMobj(linetarget, angle, power);
            }
            AdjustPlayerAngle(pmo);
            goto punchdone;
        }
        angle = pmo->angle - i * (ANG45 / 16);
        slope = P_AimLineAttack(pmo, angle, 2 * MELEERANGE, 0);
        if (linetarget)
        {
            pmo->special1.i++;
            if (pmo->special1.i == 3)
            {
                damage <<= 1;
                power = 6 * FRACUNIT;
                PuffType = HEXEN_MT_HAMMERPUFF;
            }
            P_LineAttack(pmo, angle, 2 * MELEERANGE, slope, damage);
            if (linetarget->flags & MF_COUNTKILL || linetarget->player)
            {
                P_ThrustMobj(linetarget, angle, power);
            }
            AdjustPlayerAngle(pmo);
            goto punchdone;
        }
    }
    // didn't find any creatures, so try to strike any walls
    pmo->special1.i = 0;

    angle = pmo->angle;
    slope = P_AimLineAttack(pmo, angle, MELEERANGE, 0);
    P_LineAttack(pmo, angle, MELEERANGE, slope, damage);

  punchdone:
    if (pmo->special1.i == 3)
    {
        pmo->special1.i = 0;
        P_SetPsprite(player, ps_weapon, HEXEN_S_PUNCHATK2_1);
        S_StartMobjSound(pmo, hexen_sfx_fighter_grunt);
    }
    return;
}

#define AXERANGE 2.25*MELEERANGE

void A_FAxeAttack(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    mobj_t *pmo = player->mo;
    fixed_t power;
    int damage;
    int slope;
    int i;
    int useMana;
    int r;

    r = P_Random(pr_hexen);
    damage = 40 + (r & 15) + (P_Random(pr_hexen) & 7);
    power = 0;
    if (player->ammo[MANA_1] > 0)
    {
        damage <<= 1;
        power = 6 * FRACUNIT;
        PuffType = HEXEN_MT_AXEPUFF_GLOW;
        useMana = 1;
    }
    else
    {
        PuffType = HEXEN_MT_AXEPUFF;
        useMana = 0;
    }
    for (i = 0; i < 16; i++)
    {
        angle = pmo->angle + i * (ANG45 / 16);
        slope = P_AimLineAttack(pmo, angle, AXERANGE, 0);
        if (linetarget)
        {
            P_LineAttack(pmo, angle, AXERANGE, slope, damage);
            if (linetarget->flags & MF_COUNTKILL || linetarget->player)
            {
                P_ThrustMobj(linetarget, angle, power);
            }
            AdjustPlayerAngle(pmo);
            useMana++;
            goto axedone;
        }
        angle = pmo->angle - i * (ANG45 / 16);
        slope = P_AimLineAttack(pmo, angle, AXERANGE, 0);
        if (linetarget)
        {
            P_LineAttack(pmo, angle, AXERANGE, slope, damage);
            if (linetarget->flags & MF_COUNTKILL)
            {
                P_ThrustMobj(linetarget, angle, power);
            }
            AdjustPlayerAngle(pmo);
            useMana++;
            goto axedone;
        }
    }
    // didn't find any creatures, so try to strike any walls
    pmo->special1.i = 0;

    angle = pmo->angle;
    slope = P_AimLineAttack(pmo, angle, MELEERANGE, 0);
    P_LineAttack(pmo, angle, MELEERANGE, slope, damage);

  axedone:
    if (useMana == 2)
    {
        player->ammo[MANA_1] -=
            WeaponManaUse[player->pclass][player->readyweapon];
        if (player->ammo[MANA_1] <= 0)
        {
            P_SetPsprite(player, ps_weapon, HEXEN_S_FAXEATK_5);
        }
    }
    return;
}

void A_CMaceAttack(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;
    int i;

    damage = 25 + (P_Random(pr_hexen) & 15);
    PuffType = HEXEN_MT_HAMMERPUFF;
    for (i = 0; i < 16; i++)
    {
        angle = player->mo->angle + i * (ANG45 / 16);
        slope = P_AimLineAttack(player->mo, angle, 2 * MELEERANGE, 0);
        if (linetarget)
        {
            P_LineAttack(player->mo, angle, 2 * MELEERANGE, slope, damage);
            AdjustPlayerAngle(player->mo);

            goto macedone;
        }
        angle = player->mo->angle - i * (ANG45 / 16);
        slope = P_AimLineAttack(player->mo, angle, 2 * MELEERANGE, 0);
        if (linetarget)
        {
            P_LineAttack(player->mo, angle, 2 * MELEERANGE, slope, damage);
            AdjustPlayerAngle(player->mo);

            goto macedone;
        }
    }
    // didn't find any creatures, so try to strike any walls
    player->mo->special1.i = 0;

    angle = player->mo->angle;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE, 0);
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
  macedone:
    return;
}

void A_CStaffCheck(player_t * player, pspdef_t * psp)
{
    mobj_t *pmo;
    int damage;
    int newLife;
    angle_t angle;
    int slope;
    int i;

    pmo = player->mo;
    damage = 20 + (P_Random(pr_hexen) & 15);
    PuffType = HEXEN_MT_CSTAFFPUFF;
    for (i = 0; i < 3; i++)
    {
        angle = pmo->angle + i * (ANG45 / 16);
        slope = P_AimLineAttack(pmo, angle, 1.5 * MELEERANGE, 0);
        if (linetarget)
        {
            P_LineAttack(pmo, angle, 1.5 * MELEERANGE, slope, damage);
            pmo->angle = R_PointToAngle2(pmo->x, pmo->y,
                                         linetarget->x, linetarget->y);
            if ((linetarget->player || linetarget->flags & MF_COUNTKILL)
                && (!(linetarget->flags2 & (MF2_DORMANT + MF2_INVULNERABLE))))
            {
                newLife = player->health + (damage >> 3);
                newLife = newLife > 100 ? 100 : newLife;
                pmo->health = player->health = newLife;
                P_SetPsprite(player, ps_weapon, HEXEN_S_CSTAFFATK2_1);
            }
            player->ammo[MANA_1] -=
                WeaponManaUse[player->pclass][player->readyweapon];
            break;
        }
        angle = pmo->angle - i * (ANG45 / 16);
        slope = P_AimLineAttack(player->mo, angle, 1.5 * MELEERANGE, 0);
        if (linetarget)
        {
            P_LineAttack(pmo, angle, 1.5 * MELEERANGE, slope, damage);
            pmo->angle = R_PointToAngle2(pmo->x, pmo->y,
                                         linetarget->x, linetarget->y);
            if (linetarget->player || linetarget->flags & MF_COUNTKILL)
            {
                newLife = player->health + (damage >> 4);
                newLife = newLife > 100 ? 100 : newLife;
                pmo->health = player->health = newLife;
                P_SetPsprite(player, ps_weapon, HEXEN_S_CSTAFFATK2_1);
            }
            player->ammo[MANA_1] -=
                WeaponManaUse[player->pclass][player->readyweapon];
            break;
        }
    }
    R_SmoothPlaying_Reset(player); // e6y
}

void A_CStaffAttack(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;
    mobj_t *pmo;

    player->ammo[MANA_1] -= WeaponManaUse[player->pclass][player->readyweapon];
    pmo = player->mo;
    mo = P_SPMAngle(pmo, HEXEN_MT_CSTAFF_MISSILE, pmo->angle - (ANG45 / 15));
    if (mo)
    {
        mo->special2.i = 32;
    }
    mo = P_SPMAngle(pmo, HEXEN_MT_CSTAFF_MISSILE, pmo->angle + (ANG45 / 15));
    if (mo)
    {
        mo->special2.i = 0;
    }
    S_StartMobjSound(player->mo, hexen_sfx_cleric_cstaff_fire);
}

void A_CStaffMissileSlither(mobj_t * actor)
{
    fixed_t newX, newY;
    int weaveXY;
    int angle;

    weaveXY = actor->special2.i;
    angle = (actor->angle + ANG90) >> ANGLETOFINESHIFT;
    newX = actor->x - FixedMul(finecosine[angle], FloatBobOffsets[weaveXY]);
    newY = actor->y - FixedMul(finesine[angle], FloatBobOffsets[weaveXY]);
    weaveXY = (weaveXY + 3) & 63;
    newX += FixedMul(finecosine[angle], FloatBobOffsets[weaveXY]);
    newY += FixedMul(finesine[angle], FloatBobOffsets[weaveXY]);
    P_TryMove(actor, newX, newY, 0);
    actor->special2.i = weaveXY;
}

void A_CStaffInitBlink(player_t * player, pspdef_t * psp)
{
    player->mo->special1.i = (P_Random(pr_hexen) >> 1) + 20;
}

void A_CStaffCheckBlink(player_t * player, pspdef_t * psp)
{
    if (!--player->mo->special1.i)
    {
        P_SetPsprite(player, ps_weapon, HEXEN_S_CSTAFFBLINK1);
        player->mo->special1.i = (P_Random(pr_hexen) + 50) >> 2;
    }
}

#define FLAMESPEED  (0.45*FRACUNIT)
#define CFLAMERANGE (12*64*FRACUNIT)

void A_CFlameAttack(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    mo = P_SpawnPlayerMissile(player->mo, HEXEN_MT_CFLAME_MISSILE);
    if (mo)
    {
        mo->thinker.function = P_BlasterMobjThinker;
        mo->special1.i = 2;
    }

    player->ammo[MANA_2] -= WeaponManaUse[player->pclass][player->readyweapon];
    S_StartMobjSound(player->mo, hexen_sfx_cleric_flame_fire);
}

void A_CFlamePuff(mobj_t * actor)
{
    A_UnHideThing(actor);
    actor->momx = 0;
    actor->momy = 0;
    actor->momz = 0;
    S_StartMobjSound(actor, hexen_sfx_cleric_flame_explode);
}

void A_CFlameMissile(mobj_t * actor)
{
    int i;
    int an;
    fixed_t dist;
    mobj_t *mo;

    A_UnHideThing(actor);
    S_StartMobjSound(actor, hexen_sfx_cleric_flame_explode);
    if (BlockingMobj && BlockingMobj->flags & MF_SHOOTABLE)
    {                           // Hit something, so spawn the flame circle around the thing
        dist = BlockingMobj->radius + 18 * FRACUNIT;
        for (i = 0; i < 4; i++)
        {
            an = (i * ANG45) >> ANGLETOFINESHIFT;
            mo = P_SpawnMobj(BlockingMobj->x + FixedMul(dist, finecosine[an]),
                             BlockingMobj->y + FixedMul(dist, finesine[an]),
                             BlockingMobj->z + 5 * FRACUNIT, HEXEN_MT_CIRCLEFLAME);
            if (mo)
            {
                mo->angle = an << ANGLETOFINESHIFT;
                P_SetTarget(&mo->target, actor->target);
                mo->momx = mo->special1.i =
                    FixedMul(FLAMESPEED, finecosine[an]);
                mo->momy = mo->special2.i = FixedMul(FLAMESPEED, finesine[an]);
                mo->tics -= P_Random(pr_hexen) & 3;
            }
            mo = P_SpawnMobj(BlockingMobj->x - FixedMul(dist, finecosine[an]),
                             BlockingMobj->y - FixedMul(dist, finesine[an]),
                             BlockingMobj->z + 5 * FRACUNIT, HEXEN_MT_CIRCLEFLAME);
            if (mo)
            {
                mo->angle = ANG180 + (an << ANGLETOFINESHIFT);
                P_SetTarget(&mo->target, actor->target);
                mo->momx = mo->special1.i = FixedMul(-FLAMESPEED,
                                                     finecosine[an]);
                mo->momy = mo->special2.i = FixedMul(-FLAMESPEED, finesine[an]);
                mo->tics -= P_Random(pr_hexen) & 3;
            }
        }
        P_SetMobjState(actor, HEXEN_S_FLAMEPUFF2_1);
    }
}

#define FLAMEROTSPEED 2*FRACUNIT

void A_CFlameRotate(mobj_t * actor)
{
    int an;

    an = (actor->angle + ANG90) >> ANGLETOFINESHIFT;
    actor->momx = actor->special1.i + FixedMul(FLAMEROTSPEED, finecosine[an]);
    actor->momy = actor->special2.i + FixedMul(FLAMEROTSPEED, finesine[an]);
    actor->angle += ANG90 / 15;
}

void A_CHolyAttack3(mobj_t * actor)
{
    P_SpawnMissile(actor, actor->target, HEXEN_MT_HOLY_MISSILE);
    S_StartMobjSound(actor, hexen_sfx_choly_fire);
}

void A_CHolyAttack2(mobj_t * actor)
{
    int j;
    int i;
    int r;
    mobj_t *mo;
    mobj_t *tail, *next;

    for (j = 0; j < 4; j++)
    {
        mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_HOLY_FX);
        if (!mo)
        {
            continue;
        }
        switch (j)
        {                       // float bob index
            case 0:
                mo->special2.i = P_Random(pr_hexen) & 7;  // upper-left
                break;
            case 1:
                mo->special2.i = 32 + (P_Random(pr_hexen) & 7);   // upper-right
                break;
            case 2:
                mo->special2.i = (32 + (P_Random(pr_hexen) & 7)) << 16;   // lower-left
                break;
            case 3:
                r = P_Random(pr_hexen);
                mo->special2.i =
                    ((32 + (r & 7)) << 16) + 32 + (P_Random(pr_hexen) & 7);
                break;
        }
        mo->z = actor->z;
        mo->angle = actor->angle + (ANG45 + ANG45 / 2) - ANG45 * j;
        P_ThrustMobj(mo, mo->angle, mo->info->speed);
        P_SetTarget(&mo->target, actor->target);
        mo->special_args[0] = 10;       // initial turn value
        mo->special_args[1] = 0;        // initial look angle
        if (deathmatch)
        {                       // Ghosts last slightly less longer in DeathMatch
            mo->health = 85;
        }
        if (linetarget)
        {
            P_SetTarget(&mo->special1.m, linetarget);
            mo->flags |= MF_NOCLIP | MF_SKULLFLY;
            mo->flags &= ~MF_MISSILE;
        }
        tail = P_SpawnMobj(mo->x, mo->y, mo->z, HEXEN_MT_HOLY_TAIL);
        P_SetTarget(&tail->special2.m, mo);      // parent
        for (i = 1; i < 3; i++)
        {
            next = P_SpawnMobj(mo->x, mo->y, mo->z, HEXEN_MT_HOLY_TAIL);
            P_SetMobjState(next, next->info->spawnstate + 1);
            P_SetTarget(&tail->special1.m, next);
            tail = next;
        }
        P_SetTarget(&tail->special1.m, NULL);     // last tail bit
    }
}

void A_CHolyAttack(player_t * player, pspdef_t * psp)
{
    player->ammo[MANA_1] -= WeaponManaUse[player->pclass][player->readyweapon];
    player->ammo[MANA_2] -= WeaponManaUse[player->pclass][player->readyweapon];
    P_SpawnPlayerMissile(player->mo, HEXEN_MT_HOLY_MISSILE);
    if (player == &players[consoleplayer])
    {
        player->damagecount = 0;
        player->bonuscount = 0;
        V_SetPalette(STARTHOLYPAL);
        SB_Start();
    }
    S_StartMobjSound(player->mo, hexen_sfx_choly_fire);
}

void A_CHolyPalette(player_t * player, pspdef_t * psp)
{
    int pal;

    if (player == &players[consoleplayer])
    {
        pal = STARTHOLYPAL + psp->state - (&states[HEXEN_S_CHOLYATK_6]);
        if (pal == STARTHOLYPAL + 3)
        {                       // reset back to original playpal
            pal = 0;
        }
        V_SetPalette(pal);
        SB_Start();
    }
}

static void CHolyFindTarget(mobj_t * actor)
{
    mobj_t *target;

    target = P_RoughTargetSearch(actor, 0, 6);
    if (target != NULL)
    {
        P_SetTarget(&actor->special1.m, target);
        actor->flags |= MF_NOCLIP | MF_SKULLFLY;
        actor->flags &= ~MF_MISSILE;
    }
}

static void CHolySeekerMissile(mobj_t * actor, angle_t thresh,
                               angle_t turnMax)
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
    if (!(target->flags & MF_SHOOTABLE)
        || (!(target->flags & MF_COUNTKILL) && !target->player))
    {                           // Target died/target isn't a player or creature
        P_SetTarget(&actor->special1.m, NULL);
        actor->flags &= ~(MF_NOCLIP | MF_SKULLFLY);
        actor->flags |= MF_MISSILE;
        CHolyFindTarget(actor);
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
        || actor->z > target->z + (target->height)
        || actor->z + actor->height < target->z)
    {
        newZ = target->z + ((P_Random(pr_hexen) * target->height) >> 8);
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

static void CHolyWeave(mobj_t * actor)
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
    P_TryMove(actor, newX, newY, 0);
    actor->z -= FloatBobOffsets[weaveZ] << 1;
    weaveZ = (weaveZ + (P_Random(pr_hexen) % 5)) & 63;
    actor->z += FloatBobOffsets[weaveZ] << 1;
    actor->special2.i = weaveZ + (weaveXY << 16);
}

void A_CHolySeek(mobj_t * actor)
{
    actor->health--;
    if (actor->health <= 0)
    {
        actor->momx >>= 2;
        actor->momy >>= 2;
        actor->momz = 0;
        P_SetMobjState(actor, actor->info->deathstate);
        actor->tics -= P_Random(pr_hexen) & 3;
        return;
    }
    if (actor->special1.m)
    {
        CHolySeekerMissile(actor, actor->special_args[0] * ANG1,
                           actor->special_args[0] * ANG1 * 2);
        if (!((leveltime + 7) & 15))
        {
            actor->special_args[0] = 5 + (P_Random(pr_hexen) / 20);
        }
    }
    CHolyWeave(actor);
}

static void CHolyTailFollow(mobj_t * actor, fixed_t dist)
{
    mobj_t *child;
    int an;
    fixed_t oldDistance, newDistance;

    child = actor->special1.m;
    if (child)
    {
        an = R_PointToAngle2(actor->x, actor->y, child->x,
                             child->y) >> ANGLETOFINESHIFT;
        oldDistance =
            P_AproxDistance(child->x - actor->x, child->y - actor->y);
        if (
          P_TryMove(
            child,
            actor->x + FixedMul(dist, finecosine[an]),
            actor->y + FixedMul(dist, finesine[an]),
            0
          )
        )
        {
            newDistance = P_AproxDistance(child->x - actor->x,
                                          child->y - actor->y) - FRACUNIT;
            if (oldDistance < FRACUNIT)
            {
                if (child->z < actor->z)
                {
                    child->z = actor->z - dist;
                }
                else
                {
                    child->z = actor->z + dist;
                }
            }
            else
            {
                child->z = actor->z + FixedMul(FixedDiv(newDistance,
                                                        oldDistance),
                                               child->z - actor->z);
            }
        }
        CHolyTailFollow(child, dist - FRACUNIT);
    }
}

static void CHolyTailRemove(mobj_t * actor)
{
    mobj_t *child;

    child = actor->special1.m;
    if (child)
    {
        CHolyTailRemove(child);
    }
    P_RemoveMobj(actor);
}

void A_CHolyTail(mobj_t * actor)
{
    mobj_t *parent;

    parent = actor->special2.m;

    if (parent)
    {
        if (parent->state >= &states[parent->info->deathstate])
        {                       // Ghost removed, so remove all tail parts
            CHolyTailRemove(actor);
            return;
        }
        else if (
          P_TryMove(
            actor,
            parent->x - FixedMul(14 * FRACUNIT, finecosine[parent->angle >> ANGLETOFINESHIFT]),
            parent->y - FixedMul(14 * FRACUNIT, finesine[parent->angle >> ANGLETOFINESHIFT]),
            0
          )
        )
        {
            actor->z = parent->z - 5 * FRACUNIT;
        }
        CHolyTailFollow(actor, 10 * FRACUNIT);
    }
}

void A_CHolyCheckScream(mobj_t * actor)
{
    A_CHolySeek(actor);
    if (P_Random(pr_hexen) < 20)
    {
        S_StartMobjSound(actor, hexen_sfx_spirit_active);
    }
    if (!actor->special1.m)
    {
        CHolyFindTarget(actor);
    }
}

void A_CHolySpawnPuff(mobj_t * actor)
{
    P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_HOLY_MISSILE_PUFF);
}

#define SHARDSPAWN_LEFT  1
#define SHARDSPAWN_RIGHT 2
#define SHARDSPAWN_UP    4
#define SHARDSPAWN_DOWN  8

void A_FireConePL1(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int i;
    mobj_t *pmo, *mo;
    int conedone = false;

    pmo = player->mo;
    player->ammo[MANA_1] -= WeaponManaUse[player->pclass][player->readyweapon];
    S_StartMobjSound(pmo, hexen_sfx_mage_shards_fire);

    damage = 90 + (P_Random(pr_hexen) & 15);
    for (i = 0; i < 16; i++)
    {
        angle = pmo->angle + i * (ANG45 / 16);
        P_AimLineAttack(pmo, angle, MELEERANGE, 0);
        if (linetarget)
        {
            pmo->flags2 |= MF2_ICEDAMAGE;
            P_DamageMobj(linetarget, pmo, pmo, damage);
            pmo->flags2 &= ~MF2_ICEDAMAGE;
            conedone = true;
            break;
        }
    }

    // didn't find any creatures, so fire projectiles
    if (!conedone)
    {
        mo = P_SpawnPlayerMissile(pmo, HEXEN_MT_SHARDFX1);
        if (mo)
        {
            mo->special1.i = SHARDSPAWN_LEFT | SHARDSPAWN_DOWN | SHARDSPAWN_UP
                | SHARDSPAWN_RIGHT;
            mo->special2.i = 3;   // Set sperm count (levels of reproductivity)
            P_SetTarget(&mo->target, pmo);
            mo->special_args[0] = 3;    // Mark Initial shard as super damage
        }
    }
}

void A_ShedShard(mobj_t * actor)
{
    mobj_t *mo;
    int spawndir = actor->special1.i;
    int spermcount = actor->special2.i;

    if (spermcount <= 0)
        return;                 // No sperm left
    actor->special2.i = 0;
    spermcount--;

    // every so many calls, spawn a new missile in it's set directions
    if (spawndir & SHARDSPAWN_LEFT)
    {
        mo = P_SpawnMissileAngleSpeed(actor, HEXEN_MT_SHARDFX1,
                                      actor->angle + (ANG45 / 9), 0,
                                      (20 + 2 * spermcount) << FRACBITS);
        if (mo)
        {
            mo->special1.i = SHARDSPAWN_LEFT;
            mo->special2.i = spermcount;
            mo->momz = actor->momz;
            P_SetTarget(&mo->target, actor->target);
            mo->special_args[0] = (spermcount == 3) ? 2 : 0;
        }
    }
    if (spawndir & SHARDSPAWN_RIGHT)
    {
        mo = P_SpawnMissileAngleSpeed(actor, HEXEN_MT_SHARDFX1,
                                      actor->angle - (ANG45 / 9), 0,
                                      (20 + 2 * spermcount) << FRACBITS);
        if (mo)
        {
            mo->special1.i = SHARDSPAWN_RIGHT;
            mo->special2.i = spermcount;
            mo->momz = actor->momz;
            P_SetTarget(&mo->target, actor->target);
            mo->special_args[0] = (spermcount == 3) ? 2 : 0;
        }
    }
    if (spawndir & SHARDSPAWN_UP)
    {
        mo = P_SpawnMissileAngleSpeed(actor, HEXEN_MT_SHARDFX1, actor->angle,
                                      0, (15 + 2 * spermcount) << FRACBITS);
        if (mo)
        {
            mo->momz = actor->momz;
            mo->z += 8 * FRACUNIT;
            if (spermcount & 1) // Every other reproduction
                mo->special1.i =
                    SHARDSPAWN_UP | SHARDSPAWN_LEFT | SHARDSPAWN_RIGHT;
            else
                mo->special1.i = SHARDSPAWN_UP;
            mo->special2.i = spermcount;
            P_SetTarget(&mo->target, actor->target);
            mo->special_args[0] = (spermcount == 3) ? 2 : 0;
        }
    }
    if (spawndir & SHARDSPAWN_DOWN)
    {
        mo = P_SpawnMissileAngleSpeed(actor, HEXEN_MT_SHARDFX1, actor->angle,
                                      0, (15 + 2 * spermcount) << FRACBITS);
        if (mo)
        {
            mo->momz = actor->momz;
            mo->z -= 4 * FRACUNIT;
            if (spermcount & 1) // Every other reproduction
                mo->special1.i =
                    SHARDSPAWN_DOWN | SHARDSPAWN_LEFT | SHARDSPAWN_RIGHT;
            else
                mo->special1.i = SHARDSPAWN_DOWN;
            mo->special2.i = spermcount;
            P_SetTarget(&mo->target, actor->target);
            mo->special_args[0] = (spermcount == 3) ? 2 : 0;
        }
    }
}
