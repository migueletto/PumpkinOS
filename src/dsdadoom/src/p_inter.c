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
 *      Handling interactions (i.e., collisions).
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "dstrings.h"
#include "m_random.h"
#include "am_map.h"
#include "r_main.h"
#include "s_sound.h"
#include "smooth.h"
#include "sounds.h"
#include "d_deh.h"  // Ty 03/22/98 - externalized strings
#include "p_tick.h"
#include "lprintf.h"

#include "p_inter.h"
#include "p_enemy.h"
#include "p_spec.h"
#include "p_pspr.h"
#include "p_user.h"

#include "p_inter.h"
#include "e6y.h"//e6y

#include "dsda.h"
#include "dsda/map_format.h"
#include "dsda/mapinfo.h"
#include "dsda/messenger.h"
#include "dsda/skill_info.h"

#include "heretic/def.h"
#include "heretic/sb_bar.h"

#include "hexen/p_acs.h"

// Ty 03/07/98 - add deh externals
// Maximums and such were hardcoded values.  Need to externalize those for
// dehacked support (and future flexibility).  Most var names came from the key
// strings used in dehacked.

int initial_health = 100;
int initial_bullets = 50;
int maxhealth = 100; // was MAXHEALTH as a #define, used only in this module
int maxhealthbonus = 200;
int max_armor = 200;
int green_armor_class = 1;  // these are involved with armortype below
int blue_armor_class = 2;
int max_soul = 200;
int soul_health = 100;
int mega_health = 200;
int god_health = 100;   // these are used in cheats (see st_stuff.c)
int idfa_armor = 200;
int idfa_armor_class = 2;
// not actually used due to pairing of cheat_k and cheat_fa
int idkfa_armor = 200;
int idkfa_armor_class = 2;

int bfgcells = 40;      // used in p_pspr.c
int monsters_infight = 0; // e6y: Dehacked support - monsters infight
// Ty 03/07/98 - end deh externals

// a weapon is found with two clip loads,
// a big item has five clip loads
int maxammo[NUMAMMO]  = {200, 50, 300, 50, 0, 0}; // heretic +2 ammo types
int clipammo[NUMAMMO] = { 10,  4,  20,  1, 0, 0}; // heretic +2 ammo types

//
// GET STUFF
//

// heretic
static weapontype_t GetAmmoChange[] = {
    wp_goldwand,
    wp_crossbow,
    wp_blaster,
    wp_skullrod,
    wp_phoenixrod,
    wp_mace
};

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//

static dboolean P_GiveAmmoAutoSwitch(player_t *player, ammotype_t ammo, int oldammo)
{
  int i;

  if (
    weaponinfo[player->readyweapon].flags & WPF_AUTOSWITCHFROM &&
    weaponinfo[player->readyweapon].ammo != ammo
  )
  {
    for (i = NUMWEAPONS - 1; i > player->readyweapon; --i)
    {
      if (
        player->weaponowned[i] &&
        !(weaponinfo[i].flags & WPF_NOAUTOSWITCHTO) &&
        weaponinfo[i].ammo == ammo &&
        weaponinfo[i].ammopershot > oldammo &&
        weaponinfo[i].ammopershot <= player->ammo[ammo]
      )
      {
        player->pendingweapon = i;
        break;
      }
    }
  }

  return true;
}

static dboolean P_GiveAmmo(player_t *player, ammotype_t ammo, int num)
{
  int oldammo;

  if (ammo == am_noammo)
    return false;

#ifdef RANGECHECK
  if (ammo < 0 || ammo > NUMAMMO)
    I_Error ("P_GiveAmmo: bad type %i", ammo);
#endif

  if ( player->ammo[ammo] == player->maxammo[ammo]  )
    return false;

  if (num)
    num *= clipammo[ammo];
  else
    num = clipammo[ammo]/2;

  if (skill_info.ammo_factor)
    num = FixedMul(num, skill_info.ammo_factor);

  oldammo = player->ammo[ammo];
  player->ammo[ammo] += num;

  if (player->ammo[ammo] > player->maxammo[ammo])
    player->ammo[ammo] = player->maxammo[ammo];

  if (mbf21)
    return P_GiveAmmoAutoSwitch(player, ammo, oldammo);

  // If non zero ammo, don't change up weapons, player was lower on purpose.
  if (oldammo)
    return true;

  // We were down to zero, so select a new weapon.
  // Preferences are not user selectable.

  if (heretic) {
    if (player->readyweapon == wp_staff || player->readyweapon == wp_gauntlets)
    {
        if (player->weaponowned[GetAmmoChange[ammo]])
        {
            player->pendingweapon = GetAmmoChange[ammo];
        }
    }

    return true;
  }

  switch (ammo)
    {
    case am_clip:
      if (player->readyweapon == wp_fist) {
        if (player->weaponowned[wp_chaingun])
          player->pendingweapon = wp_chaingun;
        else
          player->pendingweapon = wp_pistol;
      }
      break;

    case am_shell:
      if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
        if (player->weaponowned[wp_shotgun])
          player->pendingweapon = wp_shotgun;
      break;

      case am_cell:
        if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
          if (player->weaponowned[wp_plasma])
            player->pendingweapon = wp_plasma;
        break;

      case am_misl:
        if (player->readyweapon == wp_fist)
          if (player->weaponowned[wp_missile])
            player->pendingweapon = wp_missile;
    default:
      break;
    }
  return true;
}

//
// P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ored in.
//

dboolean P_GiveWeapon(player_t *player, weapontype_t weapon, dboolean dropped)
{
  dboolean gaveammo;
  dboolean gaveweapon;

  if (heretic) return Heretic_P_GiveWeapon(player, weapon);

  if (netgame && deathmatch!=2 && !dropped)
    {
      // leave placed weapons forever on net games
      if (player->weaponowned[weapon])
        return false;

      player->bonuscount += BONUSADD;
      player->weaponowned[weapon] = true;

      P_GiveAmmo(player, weaponinfo[weapon].ammo, deathmatch ? 5 : 2);

      player->pendingweapon = weapon;
      /* cph 20028/10 - for old-school DM addicts, allow old behavior
       * where only consoleplayer's pickup sounds are heard */
      // displayplayer, not consoleplayer, for viewing multiplayer demos
      if (!comp[comp_sound] || player == &players[displayplayer])
        S_StartSound (player->mo, sfx_wpnup|PICKUP_SOUND); // killough 4/25/98
      return false;
    }

  if (weaponinfo[weapon].ammo != am_noammo)
    {
      // give one clip with a dropped weapon,
      // two clips with a found weapon
      gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, dropped ? 1 : 2);
    }
  else
    gaveammo = false;

  if (player->weaponowned[weapon])
    gaveweapon = false;
  else
    {
      gaveweapon = true;
      player->weaponowned[weapon] = true;
      player->pendingweapon = weapon;
    }
  return gaveweapon || gaveammo;
}

int P_PlayerHealthIncrease(int value)
{
  if (skill_info.health_factor)
    value = FixedMul(value, skill_info.health_factor);

  return value;
}

int P_PlayerArmorIncrease(int value)
{
  if (skill_info.armor_factor)
    value = FixedMul(value, skill_info.armor_factor);

  return value;
}

//
// P_GiveBody
// Returns false if the body isn't needed at all
//

dboolean P_GiveBody(player_t * player, int num)
{
    int max;

    max = maxhealth;
    if (player->chickenTics)
    {
        max = MAXCHICKENHEALTH;
    }
    if (player->morphTics)
    {
        max = MAXMORPHHEALTH;
    }
    if (player->health >= max)
    {
        return (false);
    }
    player->health += P_PlayerHealthIncrease(num);
    if (player->health > max)
    {
        player->health = max;
    }
    player->mo->health = player->health;
    return (true);
}

void P_HealMobj(mobj_t *mo, int num)
{
  player_t *player = mo->player;

  if (mo->health <= 0 || (player && player->playerstate == PST_DEAD))
    return;

  if (player)
  {
    P_GiveBody(player, num);
    return;
  }
  else
  {
    int max = P_MobjSpawnHealth(mo);

    mo->health += num;
    if (mo->health > max)
      mo->health = max;
  }
}

//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//

static dboolean P_GiveArmor(player_t *player, int armortype)
{
  int hits = P_PlayerArmorIncrease(armortype * 100);
  if (player->armorpoints[ARMOR_ARMOR] >= hits)
    return false;   // don't pick up
  player->armortype = armortype;
  player->armorpoints[ARMOR_ARMOR] = hits;
  return true;
}

//
// P_GiveCard
//

void P_GiveCard(player_t *player, card_t card)
{
  if (player->cards[card])
    return;
  player->bonuscount = BONUSADD;
  player->cards[card] = 1;

  if (player == &players[consoleplayer])
    playerkeys |= 1 << card;

  dsda_WatchCard(card);
}

//
// P_GivePower
//
// Rewritten by Lee Killough
//

dboolean P_GivePower(player_t *player, int power)
{
  static const int tics[NUMPOWERS] = {
    INVULNTICS, 1 /* strength */, INVISTICS,
    IRONTICS, 1 /* allmap */, INFRATICS,
    WPNLEV2TICS, FLIGHTTICS, 1 /* shield */, 1 /* health2 */,
    SPEEDTICS, MAULATORTICS
   };

  if (
    raven &&
    tics[power] > 1 &&
    power != pw_ironfeet && power != pw_minotaur &&
    player->powers[power] > BLINKTHRESHOLD
  ) return false;

  switch (power)
  {
    case pw_invulnerability:
      if (hexen)
      {
        player->mo->flags2 |= MF2_INVULNERABLE;
        if (player->pclass == PCLASS_MAGE)
        {
            player->mo->flags2 |= MF2_REFLECTIVE;
        }
      }
      break;
    case pw_invisibility:
      player->mo->flags |= MF_SHADOW;
      break;
    case pw_allmap:
      if (player->powers[pw_allmap])
        return false;
      break;
    case pw_strength:
      P_GiveBody(player,100);
      break;
    case pw_flight:
      player->mo->flags2 |= MF2_FLY;
      player->mo->flags |= MF_NOGRAVITY;
      if (player->mo->z <= player->mo->floorz)
      {
          player->flyheight = 10;     // thrust the player in the air a bit
      }
      break;
  }

  if (hexen && player->powers[power])
    return false;

  // Unless player has infinite duration cheat, set duration (killough)

  if (player->powers[power] >= 0)
    player->powers[power] = tics[power];
  return true;
}

//
// P_TouchSpecialThing
//

static void Heretic_P_TouchSpecialThing(mobj_t * special, mobj_t * toucher);
static void Hexen_P_TouchSpecialThing(mobj_t * special, mobj_t * toucher);

void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher)
{
  player_t *player;
  int      i;
  int      sound;
  fixed_t  delta = special->z - toucher->z;

  if (heretic) return Heretic_P_TouchSpecialThing(special, toucher);
  if (hexen) return Hexen_P_TouchSpecialThing(special, toucher);

  if (delta > toucher->height || delta < -8*FRACUNIT)
    return;        // out of reach

  sound = sfx_itemup;
  player = toucher->player;

  // Dead thing touching.
  // Can happen with a sliding player corpse.
  if (toucher->health <= 0)
    return;

    // Identify by sprite.
  switch (special->sprite)
    {
      // armor
    case SPR_ARM1:
      if (!P_GiveArmor (player, green_armor_class))
        return;
      dsda_AddPlayerMessage(s_GOTARMOR, player);
      break;

    case SPR_ARM2:
      if (!P_GiveArmor (player, blue_armor_class))
        return;
      dsda_AddPlayerMessage(s_GOTMEGA, player);
      break;

        // bonus items
    case SPR_BON1:
      // can go over 100%
      player->health += P_PlayerHealthIncrease(1);
      if (player->health > (maxhealthbonus))//e6y
        player->health = (maxhealthbonus);//e6y
      player->mo->health = player->health;
      dsda_AddPlayerMessage(s_GOTHTHBONUS, player);
      break;

    case SPR_BON2:
      // can go over 100%
      player->armorpoints[ARMOR_ARMOR] += P_PlayerArmorIncrease(1);
      // e6y
      // Doom 1.2 does not do check of armor points on overflow.
      // If you set the "IDKFA Armor" to MAX_INT (DWORD at 0x00064B5A -> FFFFFF7F)
      // and pick up one or more armor bonuses, your armor becomes negative
      // and you will die after reception of any damage since this moment.
      // It happens because the taken health damage depends from armor points
      // if they are present and becomes equal to very large value in this case
      if (player->armorpoints[ARMOR_ARMOR] > max_armor && compatibility_level != doom_12_compatibility)
        player->armorpoints[ARMOR_ARMOR] = max_armor;
      // e6y
      // We always give armor type 1 for the armor bonuses;
      // dehacked only affects the GreenArmor.
      if (!player->armortype)
        player->armortype =
         ((!demo_compatibility || prboom_comp[PC_APPLY_GREEN_ARMOR_CLASS_TO_ARMOR_BONUSES].state) ?
          green_armor_class : 1);
      dsda_AddPlayerMessage(s_GOTARMBONUS, player);
      break;

    case SPR_SOUL:
      player->health += P_PlayerHealthIncrease(soul_health);
      if (player->health > max_soul)
        player->health = max_soul;
      player->mo->health = player->health;
      dsda_AddPlayerMessage(s_GOTSUPER, player);
      sound = sfx_getpow;
      break;

    case SPR_MEGA:
      if (gamemode != commercial)
        return;
      player->health = mega_health;
      player->mo->health = player->health;
      // e6y
      // We always give armor type 2 for the megasphere;
      // dehacked only affects the MegaArmor.
      P_GiveArmor (player,
         ((!demo_compatibility || prboom_comp[PC_APPLY_BLUE_ARMOR_CLASS_TO_MEGASPHERE].state) ?
          blue_armor_class : 2));
      dsda_AddPlayerMessage(s_GOTMSPHERE, player);
      sound = sfx_getpow;
      break;

        // cards
        // leave cards for everyone
    case SPR_BKEY:
      if (!player->cards[it_bluecard])
        dsda_AddPlayerMessage(s_GOTBLUECARD, player);
      P_GiveCard (player, it_bluecard);
      if (!netgame)
        break;
      return;

    case SPR_YKEY:
      if (!player->cards[it_yellowcard])
        dsda_AddPlayerMessage(s_GOTYELWCARD, player);
      P_GiveCard (player, it_yellowcard);
      if (!netgame)
        break;
      return;

    case SPR_RKEY:
      if (!player->cards[it_redcard])
        dsda_AddPlayerMessage(s_GOTREDCARD, player);
      P_GiveCard (player, it_redcard);
      if (!netgame)
        break;
      return;

    case SPR_BSKU:
      if (!player->cards[it_blueskull])
        dsda_AddPlayerMessage(s_GOTBLUESKUL, player);
      P_GiveCard (player, it_blueskull);
      if (!netgame)
        break;
      return;

    case SPR_YSKU:
      if (!player->cards[it_yellowskull])
        dsda_AddPlayerMessage(s_GOTYELWSKUL, player);
      P_GiveCard (player, it_yellowskull);
      if (!netgame)
        break;
      return;

    case SPR_RSKU:
      if (!player->cards[it_redskull])
        dsda_AddPlayerMessage(s_GOTREDSKULL, player);
      P_GiveCard (player, it_redskull);
      if (!netgame)
        break;
      return;

      // medikits, heals
    case SPR_STIM:
      if (!P_GiveBody (player, 10))
        return;
      dsda_AddPlayerMessage(s_GOTSTIM, player);
      break;

    case SPR_MEDI:
      if (!P_GiveBody (player, 25))
        return;

      if (player->health < 50) // cph - 25 + the 25 just added, thanks to Quasar for reporting this bug
        dsda_AddPlayerMessage(s_GOTMEDINEED, player);
      else
        dsda_AddPlayerMessage(s_GOTMEDIKIT, player);
      break;


      // power ups
    case SPR_PINV:
      if (!P_GivePower (player, pw_invulnerability))
        return;
      dsda_AddPlayerMessage(s_GOTINVUL, player);
      sound = sfx_getpow;
      break;

    case SPR_PSTR:
      if (!P_GivePower (player, pw_strength))
        return;
      dsda_AddPlayerMessage(s_GOTBERSERK, player);
      if (player->readyweapon != wp_fist)
        player->pendingweapon = wp_fist;
      sound = sfx_getpow;
      break;

    case SPR_PINS:
      if (!P_GivePower (player, pw_invisibility))
        return;
      dsda_AddPlayerMessage(s_GOTINVIS, player);
      sound = sfx_getpow;
      break;

    case SPR_SUIT:
      if (!P_GivePower (player, pw_ironfeet))
        return;
      dsda_AddPlayerMessage(s_GOTSUIT, player);
      sound = sfx_getpow;
      break;

    case SPR_PMAP:
      if (!P_GivePower (player, pw_allmap))
        return;
      dsda_AddPlayerMessage(s_GOTMAP, player);
      sound = sfx_getpow;
      break;

    case SPR_PVIS:
      if (!P_GivePower (player, pw_infrared))
        return;
      dsda_AddPlayerMessage(s_GOTVISOR, player);
      sound = sfx_getpow;
      break;

      // ammo
    case SPR_CLIP:
      if (special->flags & MF_DROPPED)
        {
          if (!P_GiveAmmo (player,am_clip,0))
            return;
        }
      else
        {
          if (!P_GiveAmmo (player,am_clip,1))
            return;
        }
      dsda_AddPlayerMessage(s_GOTCLIP, player);
      break;

    case SPR_AMMO:
      if (!P_GiveAmmo (player, am_clip,5))
        return;
      dsda_AddPlayerMessage(s_GOTCLIPBOX, player);
      break;

    case SPR_ROCK:
      if (!P_GiveAmmo (player, am_misl,1))
        return;
      dsda_AddPlayerMessage(s_GOTROCKET, player);
      break;

    case SPR_BROK:
      if (!P_GiveAmmo (player, am_misl,5))
        return;
      dsda_AddPlayerMessage(s_GOTROCKBOX, player);
      break;

    case SPR_CELL:
      if (!P_GiveAmmo (player, am_cell,1))
        return;
      dsda_AddPlayerMessage(s_GOTCELL, player);
      break;

    case SPR_CELP:
      if (!P_GiveAmmo (player, am_cell,5))
        return;
      dsda_AddPlayerMessage(s_GOTCELLBOX, player);
      break;

    case SPR_SHEL:
      if (!P_GiveAmmo (player, am_shell,1))
        return;
      dsda_AddPlayerMessage(s_GOTSHELLS, player);
      break;

    case SPR_SBOX:
      if (!P_GiveAmmo (player, am_shell,5))
        return;
      dsda_AddPlayerMessage(s_GOTSHELLBOX, player);
      break;

    case SPR_BPAK:
      if (!player->backpack)
        {
          for (i=0 ; i<NUMAMMO ; i++)
            player->maxammo[i] *= 2;
          player->backpack = true;
        }
      for (i=0 ; i<NUMAMMO ; i++)
        P_GiveAmmo (player, i, 1);
      dsda_AddPlayerMessage(s_GOTBACKPACK, player);
      break;

        // weapons
    case SPR_BFUG:
      if (!P_GiveWeapon (player, wp_bfg, false) )
        return;
      dsda_AddPlayerMessage(s_GOTBFG9000, player);
      sound = sfx_wpnup;
      break;

    case SPR_MGUN:
      if (!P_GiveWeapon (player, wp_chaingun, (special->flags&MF_DROPPED)!=0) )
        return;
      dsda_AddPlayerMessage(s_GOTCHAINGUN, player);
      sound = sfx_wpnup;
      break;

    case SPR_CSAW:
      if (!P_GiveWeapon (player, wp_chainsaw, false) )
        return;
      dsda_AddPlayerMessage(s_GOTCHAINSAW, player);
      sound = sfx_wpnup;
      break;

    case SPR_LAUN:
      if (!P_GiveWeapon (player, wp_missile, false) )
        return;
      dsda_AddPlayerMessage(s_GOTLAUNCHER, player);
      sound = sfx_wpnup;
      break;

    case SPR_PLAS:
      if (!P_GiveWeapon (player, wp_plasma, false) )
        return;
      dsda_AddPlayerMessage(s_GOTPLASMA, player);
      sound = sfx_wpnup;
      break;

    case SPR_SHOT:
      if (!P_GiveWeapon (player, wp_shotgun, (special->flags&MF_DROPPED)!=0 ) )
        return;
      dsda_AddPlayerMessage(s_GOTSHOTGUN, player);
      sound = sfx_wpnup;
      break;

    case SPR_SGN2:
      if (!P_GiveWeapon(player, wp_supershotgun, (special->flags&MF_DROPPED)!=0))
        return;
      dsda_AddPlayerMessage(s_GOTSHOTGUN2, player);
      sound = sfx_wpnup;
      break;

    default:
      I_Error ("P_SpecialThing: Unknown gettable thing");
    }

  if (special->special)
  {
    map_format.execute_line_special(special->special, special->special_args, NULL, 0, player->mo);
    special->special = 0;
  }

  if (special->flags & MF_COUNTITEM)
    player->itemcount++;

  if (special->flags2 & MF2_COUNTSECRET)
    P_PlayerCollectSecret(player);

  P_RemoveMobj (special);
  player->bonuscount += BONUSADD;

  /* cph 20028/10 - for old-school DM addicts, allow old behavior
   * where only consoleplayer's pickup sounds are heard */
  // displayplayer, not consoleplayer, for viewing multiplayer demos
  if (!comp[comp_sound] || player == &players[displayplayer])
    S_StartSound (player->mo, sound | PICKUP_SOUND);   // killough 4/25/98
}

//
// KillMobj
//

static mobj_t *ActiveMinotaur(player_t * master);

// killough 11/98: make static
static void P_KillMobj(mobj_t *source, mobj_t *target)
{
  mobjtype_t item;
  mobj_t     *mo;
  int xdeath_limit;

  target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY);

  if (target->type != MT_SKULL)
    target->flags &= ~MF_NOGRAVITY;

  target->flags |= MF_CORPSE|MF_DROPOFF;
  target->height >>= 2;

  // heretic
  target->flags2 &= ~MF2_PASSMOBJ;

  if (
    mbf21 || (
      compatibility_level == mbf_compatibility &&
      !prboom_comp[PC_MBF_REMOVE_THINKER_IN_KILLMOBJ].state
    )
  )
  {
    // killough 8/29/98: remove from threaded list
    P_UpdateThinker(&target->thinker);
  }

  if (!((target->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    totallive--;

  dsda_WatchDeath(target);

  if (map_format.hexen && target->special)
  {
    if (!hexen || (target->flags & MF_COUNTKILL || target->type == HEXEN_MT_ZBELL))
    {                           // Initiate monster death actions
        if (hexen && target->type == HEXEN_MT_SORCBOSS)
        {
            byte dummyArgs[3] = {0, 0, 0};
            P_StartACS(target->special, 0, dummyArgs, target, NULL, 0);
        }
        else
        {
            map_format.execute_line_special(
              target->special, target->special_args, NULL, 0,
              map_info.flags & MI_ACTIVATE_OWN_DEATH_SPECIALS ? target : source
            );
        }
    }
  }

  if (source && source->player)
  {
    // count for intermission
    if (target->flags & MF_COUNTKILL)
    {
      dsda_WatchKill(source->player, target);
    }
    if (target->player)
    {
      source->player->frags[target->player-players]++;

      if (heretic && target != source)
      {
        if (source->player == &players[consoleplayer])
        {
          S_StartVoidSound(heretic_sfx_gfrag);
        }
        if (source->player->chickenTics)
        {               // Make a super chicken
          P_GivePower(source->player, pw_weaponlevel2);
        }
      }
    }
  }
  else
    if (target->flags & MF_COUNTKILL) { /* Add to kills tally */
      if ((compatibility_level < lxdoom_1_compatibility) || !netgame) {
        if (!netgame)
        {
          dsda_WatchKill(&players[0], target);
        }
        else
        {
          if (!deathmatch) {
            if (target->lastenemy && target->lastenemy->health > 0 && target->lastenemy->player)
            {
              dsda_WatchKill(target->lastenemy->player, target);
            }
            else
            {
              unsigned int player;
              for (player = 0; player < g_maxplayers; player++)
              {
                if (playeringame[player])
                {
                  dsda_WatchKill(&players[player], target);
                  break;
                }
              }
            }
          }
        }
      }
      else
        if (!deathmatch) {
          // try and find a player to give the kill to, otherwise give the
          // kill to a random player.  this fixes the missing monsters bug
          // in coop - rain
          // CPhipps - not a bug as such, but certainly an inconsistency.
          if (target->lastenemy && target->lastenemy->health > 0 && target->lastenemy->player) // Fighting a player
          {
            dsda_WatchKill(target->lastenemy->player, target);
          }
          else {
            // cph - randomely choose a player in the game to be credited
            //  and do it uniformly between the active players
            unsigned int activeplayers = 0, player, i;

            for (player = 0; player < g_maxplayers; player++)
              if (playeringame[player])
                activeplayers++;

            if (activeplayers) {
              player = P_Random(pr_friends) % activeplayers;

              for (i = 0; i < g_maxplayers; i++)
                if (playeringame[i])
                  if (!player--)
                  {
                    dsda_WatchKill(&players[i], target);
                  }
            }
          }
        }
    }

  if (target->player)
  {
    // count environment kills against you
    if (!source)
      target->player->frags[target->player-players]++;

    target->flags &= ~MF_SOLID;

    // heretic
    target->flags2 &= ~MF2_FLY;
    target->player->powers[pw_flight] = 0;
    target->player->powers[pw_weaponlevel2] = 0;

    target->player->playerstate = PST_DEAD;
    P_DropWeapon (target->player);

    // heretic
    if (target->flags2 & MF2_FIREDAMAGE)
    {                       // Player flame death
      switch (target->player->pclass)
      {
        case PCLASS_NULL: // heretic
          P_SetMobjState(target, HERETIC_S_PLAY_FDTH1);
          return;
        case PCLASS_FIGHTER:
          S_StartMobjSound(target, hexen_sfx_player_fighter_burn_death);
          P_SetMobjState(target, HEXEN_S_PLAY_F_FDTH1);
          return;
        case PCLASS_CLERIC:
          S_StartMobjSound(target, hexen_sfx_player_cleric_burn_death);
          P_SetMobjState(target, HEXEN_S_PLAY_C_FDTH1);
          return;
        case PCLASS_MAGE:
          S_StartMobjSound(target, hexen_sfx_player_mage_burn_death);
          P_SetMobjState(target, HEXEN_S_PLAY_M_FDTH1);
          return;
        default:
          break;
      }
    }

    // hexen
    if (target->flags2 & MF2_ICEDAMAGE)
    {                       // Player ice death
      target->flags &= ~(7 << MF_TRANSSHIFT);     //no translation
      target->flags |= MF_ICECORPSE;
      switch (target->player->pclass)
      {
        case PCLASS_FIGHTER:
          P_SetMobjState(target, HEXEN_S_FPLAY_ICE);
          return;
        case PCLASS_CLERIC:
          P_SetMobjState(target, HEXEN_S_CPLAY_ICE);
          return;
        case PCLASS_MAGE:
          P_SetMobjState(target, HEXEN_S_MPLAY_ICE);
          return;
        case PCLASS_PIG:
          P_SetMobjState(target, HEXEN_S_PIG_ICE);
          return;
        default:
            break;
      }
    }

    if (target->player == &players[consoleplayer] && automap_active)
      AM_Stop(true);    // don't die in auto map; switch view prior to dying
  }

  if (hexen)
  {
    if (target->flags2 & MF2_FIREDAMAGE)
    {
      if (target->type == HEXEN_MT_FIGHTER_BOSS
          || target->type == HEXEN_MT_CLERIC_BOSS
          || target->type == HEXEN_MT_MAGE_BOSS)
      {
        switch (target->type)
        {
          case HEXEN_MT_FIGHTER_BOSS:
            S_StartMobjSound(target, hexen_sfx_player_fighter_burn_death);
            P_SetMobjState(target, HEXEN_S_PLAY_F_FDTH1);
            return;
          case HEXEN_MT_CLERIC_BOSS:
            S_StartMobjSound(target, hexen_sfx_player_cleric_burn_death);
            P_SetMobjState(target, HEXEN_S_PLAY_C_FDTH1);
            return;
          case HEXEN_MT_MAGE_BOSS:
            S_StartMobjSound(target, hexen_sfx_player_mage_burn_death);
            P_SetMobjState(target, HEXEN_S_PLAY_M_FDTH1);
            return;
          default:
            break;
        }
      }
      else if (target->type == HEXEN_MT_TREEDESTRUCTIBLE)
      {
        P_SetMobjState(target, HEXEN_S_ZTREEDES_X1);
        target->height = 24 * FRACUNIT;
        S_StartMobjSound(target, hexen_sfx_tree_explode);
        return;
      }
    }
    if (target->flags2 & MF2_ICEDAMAGE)
    {
      target->flags |= MF_ICECORPSE;
      switch (target->type)
      {
        case HEXEN_MT_BISHOP:
          P_SetMobjState(target, HEXEN_S_BISHOP_ICE);
          return;
        case HEXEN_MT_CENTAUR:
        case HEXEN_MT_CENTAURLEADER:
          P_SetMobjState(target, HEXEN_S_CENTAUR_ICE);
          return;
        case HEXEN_MT_DEMON:
        case HEXEN_MT_DEMON2:
          P_SetMobjState(target, HEXEN_S_DEMON_ICE);
          return;
        case HEXEN_MT_SERPENT:
        case HEXEN_MT_SERPENTLEADER:
          P_SetMobjState(target, HEXEN_S_SERPENT_ICE);
          return;
        case HEXEN_MT_WRAITH:
        case HEXEN_MT_WRAITHB:
          P_SetMobjState(target, HEXEN_S_WRAITH_ICE);
          return;
        case HEXEN_MT_ETTIN:
          P_SetMobjState(target, HEXEN_S_ETTIN_ICE1);
          return;
        case HEXEN_MT_FIREDEMON:
          P_SetMobjState(target, HEXEN_S_FIRED_ICE1);
          return;
        case HEXEN_MT_FIGHTER_BOSS:
          P_SetMobjState(target, HEXEN_S_FIGHTER_ICE);
          return;
        case HEXEN_MT_CLERIC_BOSS:
          P_SetMobjState(target, HEXEN_S_CLERIC_ICE);
          return;
        case HEXEN_MT_MAGE_BOSS:
          P_SetMobjState(target, HEXEN_S_MAGE_ICE);
          return;
        case HEXEN_MT_PIG:
          P_SetMobjState(target, HEXEN_S_PIG_ICE);
          return;
        default:
          target->flags &= ~MF_ICECORPSE;
          break;
      }
    }

    if (target->type == HEXEN_MT_MINOTAUR)
    {
      mobj_t *master = target->special1.m;
      if (master->health > 0)
      {
        if (!ActiveMinotaur(master->player))
        {
          master->player->powers[pw_minotaur] = 0;
        }
      }
    }
    else if (target->type == HEXEN_MT_TREEDESTRUCTIBLE)
    {
      target->height = 24 * FRACUNIT;
    }
    if (target->health < -(P_MobjSpawnHealth(target) >> 1)
        && target->info->xdeathstate)
    {                           // Extreme death
      P_SetMobjState(target, target->info->xdeathstate);
    }
    else
    {                           // Normal death
      if ((target->type == HEXEN_MT_FIREDEMON) &&
          (target->z <= target->floorz + 2 * FRACUNIT) &&
          (target->info->xdeathstate))
      {
        // This is to fix the imps' staying in fall state
        P_SetMobjState(target, target->info->xdeathstate);
      }
      else
      {
        P_SetMobjState(target, target->info->deathstate);
      }
    }
  }
  else
  {
    xdeath_limit = heretic ? (P_MobjSpawnHealth(target) >> 1) : P_MobjSpawnHealth(target);
    if (target->health < -xdeath_limit && target->info->xdeathstate)
      P_SetMobjState (target, target->info->xdeathstate);
    else
      P_SetMobjState (target, target->info->deathstate);
  }

  target->tics -= P_Random(pr_killtics)&3;

  if (raven) return;

  if (target->tics < 1)
    target->tics = 1;

  // In Chex Quest, monsters don't drop items.
  if (gamemission == chex)
  {
    return;
  }

  // Drop stuff.
  // This determines the kind of object spawned
  // during the death frame of a thing.

  if (target->info->droppeditem != MT_NULL)
  {
    item = target->info->droppeditem;
  }
  else return;

  mo = P_SpawnMobj (target->x,target->y,ONFLOORZ, item);
  mo->flags |= MF_DROPPED;    // special versions of items

  if (target->momx == 0 && target->momy == 0)
  {
    target->flags |= MF_FOREGROUND;
  }
}

//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//

static dboolean P_InfightingImmune(mobj_t *target, mobj_t *source)
{
  return // not default behaviour, and same group
    mobjinfo[target->type].infighting_group != IG_DEFAULT &&
    mobjinfo[target->type].infighting_group == mobjinfo[source->type].infighting_group;
}

static dboolean P_MorphMonster(mobj_t * actor);

void P_DamageMobj(mobj_t *target,mobj_t *inflictor, mobj_t *source, int damage)
{
  player_t *player;
  dboolean justhit = false;          /* killough 11/98 */

  /* killough 8/31/98: allow bouncers to take damage */
  if (!(target->flags & (MF_SHOOTABLE | MF_BOUNCES)))
    return; // shouldn't happen...

  if (target->health <= 0)
  {
    // hexen
    if (inflictor && inflictor->flags2 & MF2_ICEDAMAGE)
    {
        return;
    }
    else if (target->flags & MF_ICECORPSE)  // frozen
    {
        target->tics = 1;
        target->momx = target->momy = 0;
    }
    return;
  }

  // hexen has a different order of checks
  if (hexen)
  {
    if ((target->flags2 & MF2_INVULNERABLE) && damage < 10000)
    {                           // mobj is invulnerable
      if (target->player)
        return;             // for player, no exceptions
      if (inflictor)
      {
        switch (inflictor->type)
        {
              // These inflictors aren't foiled by invulnerability
          case HEXEN_MT_HOLY_FX:
          case HEXEN_MT_POISONCLOUD:
          case HEXEN_MT_FIREBOMB:
            break;
          default:
            return;
        }
      }
      else
      {
        return;
      }
    }
    if (target->player)
    {
      if (damage < 1000 && ((target->player->cheats & CF_GODMODE)
                            || target->player->powers[pw_invulnerability]))
      {
        return;
      }
    }
  }

  if (target->flags & MF_SKULLFLY)
  {
    if (heretic && target->type == HERETIC_MT_MINOTAUR) return;
    target->momx = target->momy = target->momz = 0;
  }

  if (target->flags2 & MF2_DORMANT)
  {
    // Invulnerable, and won't wake up
    return;
  }

  player = target->player;
  if (player && skill_info.damage_factor)
    damage = FixedMul(damage, skill_info.damage_factor);

  // Special damage types
  if (raven && inflictor)
    switch (inflictor->type)
    {
      case HERETIC_MT_EGGFX:
        if (player)
        {
          P_ChickenMorphPlayer(player);
        }
        else
        {
          P_ChickenMorph(target);
        }
        return;         // Always return
      case HERETIC_MT_WHIRLWIND:
        P_TouchWhirlwind(target);
        return;
      case HERETIC_MT_MINOTAUR:
        if (inflictor->flags & MF_SKULLFLY)
        {               // Slam only when in charge mode
          P_MinotaurSlam(inflictor, target);
          return;
        }
        break;
      case HERETIC_MT_MACEFX4:   // Death ball
        if ((target->flags2 & MF2_BOSS) || target->type == HERETIC_MT_HEAD)
        {               // Don't allow cheap boss kills
          break;
        }
        else if (target->player)
        {               // Player specific checks
          if (target->player->powers[pw_invulnerability])
          {           // Can't hurt invulnerable players
            break;
          }
          if (P_AutoUseChaosDevice(target->player))
          {           // Player was saved using chaos device
            return;
          }
        }
        damage = 10000; // Something's gonna die
        break;
      case HERETIC_MT_PHOENIXFX2:        // Flame thrower
        if (target->player && P_Random(pr_heretic) < 128)
        {               // Freeze player for a bit
          target->reactiontime += 4;
        }
        break;
      case HERETIC_MT_RAINPLR1:  // Rain missiles
      case HERETIC_MT_RAINPLR2:
      case HERETIC_MT_RAINPLR3:
      case HERETIC_MT_RAINPLR4:
        if (target->flags2 & MF2_BOSS)
        {               // Decrease damage for bosses
          damage = (P_Random(pr_heretic) & 7) + 1;
        }
        break;
      case HERETIC_MT_HORNRODFX2:
      case HERETIC_MT_PHOENIXFX1:
        if (target->type == HERETIC_MT_SORCERER2 && P_Random(pr_heretic) < 96)
        {               // D'Sparil teleports away
          P_DSparilTeleport(target);
          return;
        }
        break;
      case HERETIC_MT_BLASTERFX1:
      case HERETIC_MT_RIPPER:
        if (target->type == HERETIC_MT_HEAD)
        {               // Less damage to Ironlich bosses
          damage = P_Random(pr_heretic) & 1;
          if (!damage)
          {
            return;
          }
        }
        break;
      case HEXEN_MT_EGGFX:
        if (player)
        {
          P_MorphPlayer(player);
        }
        else
        {
          P_MorphMonster(target);
        }
        return;         // Always return
      case HEXEN_MT_TELOTHER_FX1:
      case HEXEN_MT_TELOTHER_FX2:
      case HEXEN_MT_TELOTHER_FX3:
      case HEXEN_MT_TELOTHER_FX4:
      case HEXEN_MT_TELOTHER_FX5:
        if ((target->flags & MF_COUNTKILL) &&
            (target->type != HEXEN_MT_SERPENT) &&
            (target->type != HEXEN_MT_SERPENTLEADER) &&
            (!(target->flags2 & MF2_BOSS)))
        {
          P_TeleportOther(target);
        }
        return;
      case HEXEN_MT_MINOTAUR:
        if (inflictor->flags & MF_SKULLFLY)
        {               // Slam only when in charge mode
          P_MinotaurSlam(inflictor, target);
          return;
        }
        break;
      case HEXEN_MT_BISH_FX:
        // Bishops are just too nasty
        damage >>= 1;
        break;
      case HEXEN_MT_SHARDFX1:
        switch (inflictor->special2.i)
        {
          case 3:
            damage <<= 3;
            break;
          case 2:
            damage <<= 2;
            break;
          case 1:
            damage <<= 1;
            break;
          default:
            break;
        }
        break;
      case HEXEN_MT_CSTAFF_MISSILE:
        // Cleric Serpent Staff does poison damage
        if (target->player)
        {
          P_PoisonPlayer(target->player, source, 20);
          damage >>= 1;
        }
        break;
      case HEXEN_MT_ICEGUY_FX2:
        damage >>= 1;
        break;
      case HEXEN_MT_POISONDART:
        if (target->player)
        {
          P_PoisonPlayer(target->player, source, 20);
          damage >>= 1;
        }
        break;
      case HEXEN_MT_POISONCLOUD:
        if (target->player)
        {
          if (target->player->poisoncount < 4)
          {
            P_PoisonDamage(target->player, source, 15 + (P_Random(pr_hexen) & 15), false);  // Don't play painsound
            P_PoisonPlayer(target->player, source, 50);
            S_StartMobjSound(target, hexen_sfx_player_poisoncough);
          }
          return;
        }
        else if (!(target->flags & MF_COUNTKILL))
        {               // only damage monsters/players with the poison cloud
          return;
        }
        break;
      case HEXEN_MT_FSWORD_MISSILE:
        if (target->player)
        {
          damage -= damage >> 2;
        }
        break;
      default:
        break;
    }

  // Some close combat weapons should not
  // inflict thrust and push the victim out of reach,
  // thus kick away unless using the chainsaw.

  if (
    inflictor &&
    !(target->flags & MF_NOCLIP) && // hexen_note: not done in hexen, does it matter?
    !(
      source &&
      source->player &&
      (hexen || weaponinfo[source->player->readyweapon].flags & WPF_NOTHRUST)
    ) &&
    !(inflictor->flags2 & MF2_NODMGTHRUST)
  )
  {
    unsigned ang = R_PointToAngle2 (inflictor->x, inflictor->y,
                                    target->x,    target->y);

    fixed_t thrust = damage * (FRACUNIT >> 3) * g_thrust_factor / target->info->mass;

    // make fall forwards sometimes
    if ( damage < 40 && damage > target->health
         && target->z - inflictor->z > 64*FRACUNIT
         && P_Random(pr_damagemobj) & 1)
    {
      ang += ANG180;
      thrust *= 4;
    }

    ang >>= ANGLETOFINESHIFT;

    if (source && source->player && (source == inflictor)
        && source->player->powers[pw_weaponlevel2]
        && source->player->readyweapon == wp_staff)
    {
      // Staff power level 2
      target->momx += FixedMul(10 * FRACUNIT, finecosine[ang]);
      target->momy += FixedMul(10 * FRACUNIT, finesine[ang]);
      if (!(target->flags & MF_NOGRAVITY))
      {
          target->momz += 5 * FRACUNIT;
      }
    }
    else
    {
      target->momx += FixedMul(thrust, finecosine[ang]);
      target->momy += FixedMul(thrust, finesine[ang]);
    }

    /* killough 11/98: thrust objects hanging off ledges */
    if (target->intflags & MIF_FALLING && target->gear >= MAXGEAR)
      target->gear = 0;
  }

  // player specific
  if (player)
  {
    // end of game hell hack
    if (!raven && target->subsector->sector->special == 11 && damage >= target->health)
      damage = target->health - 1;

    // Below certain threshold,
    // ignore damage in GOD mode, or with INVUL power.
    // killough 3/26/98: make god mode 100% god mode in non-compat mode

    if (
      !hexen &&
      (damage < 1000 || (!comp[comp_god] && (player->cheats & CF_GODMODE))) &&
      (player->cheats & CF_GODMODE || player->powers[pw_invulnerability])
    )
      return;

    if (hexen)
    {
      int i;
      int saved;
      fixed_t savedPercent = pclass[player->pclass].auto_armor_save
                             + player->armorpoints[ARMOR_ARMOR]
                             + player->armorpoints[ARMOR_SHIELD]
                             + player->armorpoints[ARMOR_HELMET]
                             + player->armorpoints[ARMOR_AMULET];
      if (savedPercent)
      {                       // armor absorbed some damage
        if (savedPercent > 100 * FRACUNIT)
        {
          savedPercent = 100 * FRACUNIT;
        }
        for (i = 0; i < NUMARMOR; i++)
        {
          if (player->armorpoints[i])
          {
            player->armorpoints[i] -= FixedDiv(
              FixedMul(damage << FRACBITS, pclass[player->pclass].armor_increment[i]),
              300 * FRACUNIT
            );
            if (player->armorpoints[i] < 2 * FRACUNIT)
            {
              player->armorpoints[i] = 0;
            }
          }
        }
        saved = FixedDiv(
          FixedMul(damage << FRACBITS, savedPercent),
          100 * FRACUNIT
        );
        if (saved > savedPercent * 2)
        {
          saved = savedPercent * 2;
        }
        damage -= saved >> FRACBITS;
      }
    }
    else
    {
      if (player->armortype)
      {
        int saved;

        if (heretic)
          saved = player->armortype == 1 ? (damage >> 1) : (damage >> 1) + (damage >> 2);
        else
          saved = player->armortype == 1 ? damage / 3 : damage / 2;

        if (player->armorpoints[ARMOR_ARMOR] <= saved)
        {
          // armor is used up
          saved = player->armorpoints[ARMOR_ARMOR];
          player->armortype = 0;
        }
        player->armorpoints[ARMOR_ARMOR] -= saved;
        damage -= saved;
      }
    }

    if (
      raven &&
      damage >= player->health &&
      (skill_info.flags & SI_AUTO_USE_HEALTH || deathmatch) &&
      !player->chickenTics && !player->morphTics
    )
    {                       // Try to use some inventory health
      P_AutoUseHealth(player, damage - player->health + 1);
    }

    player->health -= damage;       // mirror mobj health here for Dave
    if (player->health < 0)
      player->health = 0;

    player->attacker = source;
    player->damagecount += damage;  // add damage after armor / invuln

    if (player->damagecount > 100)
      player->damagecount = 100;  // teleport stomp does 10k points...

    if (raven && player == &players[consoleplayer])
    {
      SB_PaletteFlash(false);
    }
  }

  dsda_WatchDamage(target, inflictor, source, damage);

  // do the damage
  target->health -= damage;
  if (target->health <= 0)
  {
    if (heretic) {
      target->special1.i = damage;
      if (target->type == HERETIC_MT_POD && source && source->type != HERETIC_MT_POD)
      {                       // Make sure players get frags for chain-reaction kills
        P_SetTarget(&target->target, source);
      }
      if (player && inflictor && !player->chickenTics)
      {                       // Check for flame death
        if ((inflictor->flags2 & MF2_FIREDAMAGE)
            || ((inflictor->type == HERETIC_MT_PHOENIXFX1)
                && (target->health > -50) && (damage > 25)))
        {
          target->flags2 |= MF2_FIREDAMAGE;
        }
      }
    }
    else if (hexen)
    {
      if (inflictor)
      {                       // check for special fire damage or ice damage deaths
        if (inflictor->flags2 & MF2_FIREDAMAGE)
        {
          if (player && !player->morphTics)
          {               // Check for flame death
            if (target->health > -50 && damage > 25)
            {
              target->flags2 |= MF2_FIREDAMAGE;
            }
          }
          else
          {
            target->flags2 |= MF2_FIREDAMAGE;
          }
        }
        else if (inflictor->flags2 & MF2_ICEDAMAGE)
        {
          target->flags2 |= MF2_ICEDAMAGE;
        }
      }
      if (source && (source->type == HEXEN_MT_MINOTAUR))
      {                       // Minotaur's kills go to his master
        mobj_t *master = source->special1.m;
        // Make sure still alive and not a pointer to fighter head
        if (master->player && (master->player->mo == master))
        {
          source = master;
        }
      }
      if (source && (source->player) &&
          (source->player->readyweapon == wp_fourth))
      {
        // Always extreme death from fourth weapon
        target->health = -5000;
      }
    }

    P_KillMobj (source, target);
    return;
  }

  // killough 9/7/98: keep track of targets so that friends can help friends
  if (mbf_features)
  {
    /* If target is a player, set player's target to source,
     * so that a friend can tell who's hurting a player
     */
    if (player) P_SetTarget(&target->target, source);

    /* killough 9/8/98:
     * If target's health is less than 50%, move it to the front of its list.
     * This will slightly increase the chances that enemies will choose to
     * "finish it off", but its main purpose is to alert friends of danger.
     */
    if (target->health*2 < P_MobjSpawnHealth(target))
    {
      thinker_t *cap = &thinkerclasscap[target->flags & MF_FRIEND ?
               th_friends : th_enemies];
      (target->thinker.cprev->cnext = target->thinker.cnext)->cprev =
        target->thinker.cprev;
      (target->thinker.cnext = cap->cnext)->cprev = &target->thinker;
      (target->thinker.cprev = cap)->cnext = &target->thinker;
    }
  }

  if (!(skill_info.flags & SI_NO_PAIN) &&
      P_Random(pr_painchance) < target->info->painchance &&
      !(target->flags & MF_SKULLFLY)) //killough 11/98: see below
  {
    if (hexen && inflictor && inflictor->type >= HEXEN_MT_LIGHTNING_FLOOR &&
                              inflictor->type <= HEXEN_MT_LIGHTNING_ZAP)
    {
      if (P_Random(pr_hexen) < 96)
      {
        target->flags |= MF_JUSTHIT;    // fight back!
        P_SetMobjState(target, target->info->painstate);
      }
      else
      {                   // "electrocute" the target
        target->frame |= FF_FULLBRIGHT;
        if (target->flags & MF_COUNTKILL && P_Random(pr_hexen) < 128
            && !S_GetSoundPlayingInfo(target, hexen_sfx_puppybeat))
        {
          if ((target->type == HEXEN_MT_CENTAUR) ||
              (target->type == HEXEN_MT_CENTAURLEADER) ||
              (target->type == HEXEN_MT_ETTIN))
          {
            S_StartMobjSound(target, hexen_sfx_puppybeat);
          }
        }
      }
    }
    else
    {
      if (mbf_features)
        justhit = true;
      else
        target->flags |= MF_JUSTHIT;    // fight back!

      P_SetMobjState(target, target->info->painstate);

      if (hexen && inflictor && inflictor->type == HEXEN_MT_POISONCLOUD)
      {
        if (target->flags & MF_COUNTKILL && P_Random(pr_hexen) < 128
            && !S_GetSoundPlayingInfo(target, hexen_sfx_puppybeat))
        {
          if ((target->type == HEXEN_MT_CENTAUR) ||
              (target->type == HEXEN_MT_CENTAURLEADER) ||
              (target->type == HEXEN_MT_ETTIN))
          {
            S_StartMobjSound(target, hexen_sfx_puppybeat);
          }
        }
      }
    }
  }

  target->reactiontime = 0;           // we're awake now...

  /* killough 9/9/98: cleaned up, made more consistent: */
  //e6y: Monsters could commit suicide in Doom v1.2 if they damaged themselves by exploding a barrel
  if (
    source &&
    (source != target || compatibility_level == doom_12_compatibility) &&
    !(source->flags2 & MF2_DMGIGNORED) &&
    (!target->threshold || target->flags2 & MF2_NOTHRESHOLD) &&
    ((source->flags ^ target->flags) & MF_FRIEND || monster_infighting || !mbf_features) &&
    !(
      raven && (
        source->flags2 & MF2_BOSS ||
        (target->type == HERETIC_MT_SORCERER2 && source->type == HERETIC_MT_WIZARD) ||
        target->type == HEXEN_MT_BISHOP ||
        target->type == HEXEN_MT_MINOTAUR ||
        (target->type == HEXEN_MT_CENTAUR && source->type == HEXEN_MT_CENTAURLEADER) ||
        (target->type == HEXEN_MT_CENTAURLEADER && source->type == HEXEN_MT_CENTAUR)
      )
    ) &&
    !P_InfightingImmune(target, source)
  )
  {
    /* if not intent on another player, chase after this one
     *
     * killough 2/15/98: remember last enemy, to prevent
     * sleeping early; 2/21/98: Place priority on players
     * killough 9/9/98: cleaned up, made more consistent:
     */

    if (
      !target->lastenemy ||
      target->lastenemy->health <= 0 ||
      (
        !mbf_features ?
        !target->lastenemy->player :
        !((target->flags ^ target->lastenemy->flags) & MF_FRIEND) && target->target != source
      )
    ) // remember last enemy - killough
      P_SetTarget(&target->lastenemy, target->target);

    P_SetTarget(&target->target, source);       // killough 11/98
    target->threshold = BASETHRESHOLD;
    if (target->state == &states[target->info->spawnstate]
        && target->info->seestate != g_s_null)
      P_SetMobjState (target, target->info->seestate);
  }

  /* killough 11/98: Don't attack a friend, unless hit by that friend.
   * cph 2006/04/01 - implicitly this is only if mbf_features */
  if(!demo_compatibility) //e6y
    if (justhit && (target->target == source || !target->target ||
        !(target->flags & target->target->flags & MF_FRIEND)))
      target->flags |= MF_JUSTHIT;    // fight back!
}

// heretic

#include "p_user.h"

#define CHICKENTICS (40*35)

void A_RestoreArtifact(mobj_t * arti)
{
    arti->flags |= MF_SPECIAL;
    P_SetMobjState(arti, arti->info->spawnstate);
    S_StartMobjSound(arti, g_sfx_respawn);
}

void A_RestoreSpecialThing1(mobj_t * thing)
{
    if (thing->type == HERETIC_MT_WMACE)
    {                           // Do random mace placement
        P_RepositionMace(thing);
    }
    thing->flags2 &= ~MF2_DONTDRAW;
    S_StartMobjSound(thing, g_sfx_respawn);
}

void A_RestoreSpecialThing2(mobj_t * thing)
{
    thing->flags |= MF_SPECIAL;
    P_SetMobjState(thing, thing->info->spawnstate);
}

// heretic

void P_SetMessage(player_t * player, const char *message, dboolean ultmsg)
{
    dsda_AddPlayerMessage(message, player);
    player->yellowMessage = false;
}

static void Heretic_P_TouchSpecialThing(mobj_t * special, mobj_t * toucher)
{
    int i;
    player_t *player;
    fixed_t delta;
    int sound;

    delta = special->z - toucher->z;
    if (delta > toucher->height || delta < -32 * FRACUNIT)
    {                           // Out of reach
        return;
    }
    if (toucher->health <= 0)
    {                           // Toucher is dead
        return;
    }
    sound = heretic_sfx_itemup;
    player = toucher->player;

    switch (special->sprite)
    {
            // Items
        case HERETIC_SPR_PTN1:         // Item_HealingPotion
            if (!P_GiveBody(player, 10))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_ITEMHEALTH, false);
            break;
        case HERETIC_SPR_SHLD:         // Item_Shield1
            if (!P_GiveArmor(player, 1))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_ITEMSHIELD1, false);
            break;
        case HERETIC_SPR_SHD2:         // Item_Shield2
            if (!P_GiveArmor(player, 2))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_ITEMSHIELD2, false);
            break;
        case HERETIC_SPR_BAGH:         // Item_BagOfHolding
            if (!player->backpack)
            {
                for (i = 0; i < NUMAMMO; i++)
                {
                    player->maxammo[i] *= 2;
                }
                player->backpack = true;
            }
            P_GiveAmmo(player, am_goldwand, AMMO_GWND_WIMPY);
            P_GiveAmmo(player, am_blaster, AMMO_BLSR_WIMPY);
            P_GiveAmmo(player, am_crossbow, AMMO_CBOW_WIMPY);
            P_GiveAmmo(player, am_skullrod, AMMO_SKRD_WIMPY);
            P_GiveAmmo(player, am_phoenixrod, AMMO_PHRD_WIMPY);
            P_SetMessage(player, HERETIC_TXT_ITEMBAGOFHOLDING, false);
            break;
        case HERETIC_SPR_SPMP:         // Item_SuperMap
            if (!P_GivePower(player, pw_allmap))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_ITEMSUPERMAP, false);
            break;

            // Keys
        case HERETIC_SPR_BKYY:         // Key_Blue
            if (!player->cards[key_blue])
            {
                P_SetMessage(player, HERETIC_TXT_GOTBLUEKEY, false);
            }
            P_GiveCard(player, key_blue);
            sound = heretic_sfx_keyup;
            if (!netgame)
            {
                break;
            }
            return;
        case HERETIC_SPR_CKYY:         // Key_Yellow
            if (!player->cards[key_yellow])
            {
                P_SetMessage(player, HERETIC_TXT_GOTYELLOWKEY, false);
            }
            sound = heretic_sfx_keyup;
            P_GiveCard(player, key_yellow);
            if (!netgame)
            {
                break;
            }
            return;
        case HERETIC_SPR_AKYY:         // Key_Green
            if (!player->cards[key_green])
            {
                P_SetMessage(player, HERETIC_TXT_GOTGREENKEY, false);
            }
            sound = heretic_sfx_keyup;
            P_GiveCard(player, key_green);
            if (!netgame)
            {
                break;
            }
            return;

            // Artifacts
        case HERETIC_SPR_PTN2:         // Arti_HealingPotion
            if (P_GiveArtifact(player, arti_health, special))
            {
                P_SetMessage(player, HERETIC_TXT_ARTIHEALTH, false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_SOAR:         // Arti_Fly
            if (P_GiveArtifact(player, arti_fly, special))
            {
                P_SetMessage(player, HERETIC_TXT_ARTIFLY, false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_INVU:         // Arti_Invulnerability
            if (P_GiveArtifact(player, arti_invulnerability, special))
            {
                P_SetMessage(player, HERETIC_TXT_ARTIINVULNERABILITY, false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_PWBK:         // Arti_TomeOfPower
            if (P_GiveArtifact(player, arti_tomeofpower, special))
            {
                P_SetMessage(player, HERETIC_TXT_ARTITOMEOFPOWER, false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_INVS:         // Arti_Invisibility
            if (P_GiveArtifact(player, arti_invisibility, special))
            {
                P_SetMessage(player, HERETIC_TXT_ARTIINVISIBILITY, false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_EGGC:         // Arti_Egg
            if (P_GiveArtifact(player, arti_egg, special))
            {
                P_SetMessage(player, HERETIC_TXT_ARTIEGG, false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_SPHL:         // Arti_SuperHealth
            if (P_GiveArtifact(player, arti_superhealth, special))
            {
                P_SetMessage(player, HERETIC_TXT_ARTISUPERHEALTH, false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_TRCH:         // Arti_Torch
            if (P_GiveArtifact(player, arti_torch, special))
            {
                P_SetMessage(player, HERETIC_TXT_ARTITORCH, false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_FBMB:         // Arti_FireBomb
            if (P_GiveArtifact(player, arti_firebomb, special))
            {
                P_SetMessage(player, HERETIC_TXT_ARTIFIREBOMB, false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_ATLP:         // Arti_Teleport
            if (P_GiveArtifact(player, arti_teleport, special))
            {
                P_SetMessage(player, HERETIC_TXT_ARTITELEPORT, false);
                P_SetDormantArtifact(special);
            }
            return;

            // Ammo
        case HERETIC_SPR_AMG1:         // Ammo_GoldWandWimpy
            if (!P_GiveAmmo(player, am_goldwand, special->health))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_AMMOGOLDWAND1, false);
            break;
        case HERETIC_SPR_AMG2:         // Ammo_GoldWandHefty
            if (!P_GiveAmmo(player, am_goldwand, special->health))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_AMMOGOLDWAND2, false);
            break;
        case HERETIC_SPR_AMM1:         // Ammo_MaceWimpy
            if (!P_GiveAmmo(player, am_mace, special->health))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_AMMOMACE1, false);
            break;
        case HERETIC_SPR_AMM2:         // Ammo_MaceHefty
            if (!P_GiveAmmo(player, am_mace, special->health))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_AMMOMACE2, false);
            break;
        case HERETIC_SPR_AMC1:         // Ammo_CrossbowWimpy
            if (!P_GiveAmmo(player, am_crossbow, special->health))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_AMMOCROSSBOW1, false);
            break;
        case HERETIC_SPR_AMC2:         // Ammo_CrossbowHefty
            if (!P_GiveAmmo(player, am_crossbow, special->health))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_AMMOCROSSBOW2, false);
            break;
        case HERETIC_SPR_AMB1:         // Ammo_BlasterWimpy
            if (!P_GiveAmmo(player, am_blaster, special->health))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_AMMOBLASTER1, false);
            break;
        case HERETIC_SPR_AMB2:         // Ammo_BlasterHefty
            if (!P_GiveAmmo(player, am_blaster, special->health))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_AMMOBLASTER2, false);
            break;
        case HERETIC_SPR_AMS1:         // Ammo_SkullRodWimpy
            if (!P_GiveAmmo(player, am_skullrod, special->health))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_AMMOSKULLROD1, false);
            break;
        case HERETIC_SPR_AMS2:         // Ammo_SkullRodHefty
            if (!P_GiveAmmo(player, am_skullrod, special->health))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_AMMOSKULLROD2, false);
            break;
        case HERETIC_SPR_AMP1:         // Ammo_PhoenixRodWimpy
            if (!P_GiveAmmo(player, am_phoenixrod, special->health))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_AMMOPHOENIXROD1, false);
            break;
        case HERETIC_SPR_AMP2:         // Ammo_PhoenixRodHefty
            if (!P_GiveAmmo(player, am_phoenixrod, special->health))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_AMMOPHOENIXROD2, false);
            break;

            // Weapons
        case HERETIC_SPR_WMCE:         // Weapon_Mace
            if (!P_GiveWeapon(player, wp_mace, false))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_WPNMACE, false);
            sound = heretic_sfx_wpnup;
            break;
        case HERETIC_SPR_WBOW:         // Weapon_Crossbow
            if (!P_GiveWeapon(player, wp_crossbow, false))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_WPNCROSSBOW, false);
            sound = heretic_sfx_wpnup;
            break;
        case HERETIC_SPR_WBLS:         // Weapon_Blaster
            if (!P_GiveWeapon(player, wp_blaster, false))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_WPNBLASTER, false);
            sound = heretic_sfx_wpnup;
            break;
        case HERETIC_SPR_WSKL:         // Weapon_SkullRod
            if (!P_GiveWeapon(player, wp_skullrod, false))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_WPNSKULLROD, false);
            sound = heretic_sfx_wpnup;
            break;
        case HERETIC_SPR_WPHX:         // Weapon_PhoenixRod
            if (!P_GiveWeapon(player, wp_phoenixrod, false))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_WPNPHOENIXROD, false);
            sound = heretic_sfx_wpnup;
            break;
        case HERETIC_SPR_WGNT:         // Weapon_Gauntlets
            if (!P_GiveWeapon(player, wp_gauntlets, false))
            {
                return;
            }
            P_SetMessage(player, HERETIC_TXT_WPNGAUNTLETS, false);
            sound = heretic_sfx_wpnup;
            break;
        default:
            I_Error("Heretic_P_TouchSpecialThing: Unknown gettable thing");
    }
    if (special->flags & MF_COUNTITEM)
    {
        player->itemcount++;
    }
    if (deathmatch && !(special->flags & MF_DROPPED))
    {
        P_HideSpecialThing(special);
    }
    else
    {
        P_RemoveMobj(special);
    }
    player->bonuscount += BONUSADD;
    if (player == &players[consoleplayer])
    {
        S_StartVoidSound(sound);
        SB_PaletteFlash(false);
    }
}

dboolean P_GiveArtifact(player_t * player, artitype_t arti, mobj_t * mo)
{
    int i;
    dboolean slidePointer;

    slidePointer = false;
    i = 0;
    while (player->inventory[i].type != arti && i < player->inventorySlotNum)
    {
        i++;
    }
    if (i == player->inventorySlotNum)
    {
        if (hexen && arti < hexen_arti_firstpuzzitem)
        {
            i = 0;
            while (player->inventory[i].type < hexen_arti_firstpuzzitem
                   && i < player->inventorySlotNum)
            {
                i++;
            }
            if (i != player->inventorySlotNum)
            {
                int j;
                for (j = player->inventorySlotNum; j > i; j--)
                {
                    player->inventory[j].count =
                        player->inventory[j - 1].count;
                    player->inventory[j].type = player->inventory[j - 1].type;
                    slidePointer = true;
                }
            }
        }
        player->inventory[i].count = 1;
        player->inventory[i].type = arti;
        player->inventorySlotNum++;
    }
    else
    {
        if (hexen && arti >= hexen_arti_firstpuzzitem && netgame && !deathmatch)
        {                       // Can't carry more than 1 puzzle item in coop netplay
            return false;
        }
        if (player->inventory[i].count >= g_arti_limit)
        {                       // Player already has 16 of this item
            return (false);
        }
        player->inventory[i].count++;
    }
    if (player->artifactCount == 0)
    {
        player->readyArtifact = arti;
    }
    else if (player == &players[consoleplayer] && slidePointer && i <= inv_ptr)
    {
        inv_ptr++;
        curpos++;
        if (curpos > 6)
        {
            curpos = 6;
        }
    }
    player->artifactCount++;
    if (mo && (mo->flags & MF_COUNTITEM))
    {
        player->itemcount++;
    }
    return (true);
}

void P_SetDormantArtifact(mobj_t * arti)
{
    arti->flags &= ~MF_SPECIAL;
    if (deathmatch && (arti->type != HERETIC_MT_ARTIINVULNERABILITY)
        && (arti->type != HERETIC_MT_ARTIINVISIBILITY))
    {
        P_SetMobjState(arti, HERETIC_S_DORMANTARTI1);
    }
    else
    {                           // Don't respawn
        P_SetMobjState(arti, HERETIC_S_DEADARTI1);
    }
    S_StartMobjSound(arti, heretic_sfx_artiup);
}

int GetWeaponAmmo[NUMWEAPONS] = {
    0,                          // staff
    25,                         // gold wand
    10,                         // crossbow
    30,                         // blaster
    50,                         // skull rod
    2,                          // phoenix rod
    50,                         // mace
    0,                          // gauntlets
    0                           // beak
};

int WeaponValue[] = {
    1,                          // staff
    3,                          // goldwand
    4,                          // crossbow
    5,                          // blaster
    6,                          // skullrod
    7,                          // phoenixrod
    8,                          // mace
    2,                          // gauntlets
    0                           // beak
};

dboolean Heretic_P_GiveWeapon(player_t * player, weapontype_t weapon)
{
    dboolean gaveAmmo;
    dboolean gaveWeapon;

    if (netgame && !deathmatch)
    {                           // Cooperative net-game
        if (player->weaponowned[weapon])
        {
            return (false);
        }
        player->bonuscount += BONUSADD;
        player->weaponowned[weapon] = true;
        P_GiveAmmo(player, wpnlev1info[weapon].ammo, GetWeaponAmmo[weapon]);
        player->pendingweapon = weapon;
        if (player == &players[consoleplayer])
        {
            S_StartVoidSound(heretic_sfx_wpnup);
        }
        return (false);
    }
    gaveAmmo = P_GiveAmmo(player, wpnlev1info[weapon].ammo,
                          GetWeaponAmmo[weapon]);
    if (player->weaponowned[weapon])
    {
        gaveWeapon = false;
    }
    else
    {
        gaveWeapon = true;
        player->weaponowned[weapon] = true;
        if (WeaponValue[weapon] > WeaponValue[player->readyweapon])
        {                       // Only switch to more powerful weapons
            player->pendingweapon = weapon;
        }
    }
    return (gaveWeapon || gaveAmmo);
}

void P_HideSpecialThing(mobj_t * thing)
{
    thing->flags &= ~MF_SPECIAL;
    thing->flags2 |= MF2_DONTDRAW;
    P_SetMobjState(thing, g_hide_state);
}

void P_MinotaurSlam(mobj_t * source, mobj_t * target)
{
    angle_t angle;
    fixed_t thrust;

    angle = R_PointToAngle2(source->x, source->y, target->x, target->y);
    angle >>= ANGLETOFINESHIFT;
    thrust = 16 * FRACUNIT + (P_Random(pr_heretic) << 10);
    target->momx += FixedMul(thrust, finecosine[angle]);
    target->momy += FixedMul(thrust, finesine[angle]);
    if (hexen)
    {
        P_DamageMobj(target, NULL, source, HITDICE(4));
    }
    else
    {
        P_DamageMobj(target, NULL, NULL, HITDICE(6));
    }
    if (target->player)
    {
        target->reactiontime = 14 + (P_Random(pr_heretic) & 7);
    }
    source->special_args[0] = 0;        // Stop charging
}

void P_TouchWhirlwind(mobj_t * target)
{
    int randVal;

    target->angle += P_SubRandom() << 20;
    target->momx += P_SubRandom() << 10;
    target->momy += P_SubRandom() << 10;
    if (leveltime & 16 && !(target->flags2 & MF2_BOSS))
    {
        randVal = P_Random(pr_heretic);
        if (randVal > 160)
        {
            randVal = 160;
        }
        target->momz += randVal << 10;
        if (target->momz > 12 * FRACUNIT)
        {
            target->momz = 12 * FRACUNIT;
        }
    }
    if (!(leveltime & 7))
    {
        P_DamageMobj(target, NULL, NULL, 3);
    }

    if (target->player) R_SmoothPlaying_Reset(target->player); // e6y
}

dboolean P_ChickenMorphPlayer(player_t * player)
{
    mobj_t *pmo;
    mobj_t *fog;
    mobj_t *chicken;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    angle_t angle;
    int oldFlags2;

    if (player->chickenTics)
    {
        if ((player->chickenTics < CHICKENTICS - TICRATE)
            && !player->powers[pw_weaponlevel2])
        {                       // Make a super chicken
            P_GivePower(player, pw_weaponlevel2);
        }
        return (false);
    }
    if (player->powers[pw_invulnerability])
    {                           // Immune when invulnerable
        return (false);
    }
    pmo = player->mo;
    x = pmo->x;
    y = pmo->y;
    z = pmo->z;
    angle = pmo->angle;
    oldFlags2 = pmo->flags2;
    P_SetMobjState(pmo, HERETIC_S_FREETARGMOBJ);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, HERETIC_MT_TFOG);
    S_StartMobjSound(fog, heretic_sfx_telept);
    chicken = P_SpawnMobj(x, y, z, HERETIC_MT_CHICPLAYER);
    chicken->special1.i = player->readyweapon;
    chicken->angle = angle;
    chicken->player = player;
    player->health = chicken->health = MAXCHICKENHEALTH;
    player->mo = chicken;
    player->armorpoints[ARMOR_ARMOR] = player->armortype = 0;
    player->powers[pw_invisibility] = 0;
    player->powers[pw_weaponlevel2] = 0;
    if (oldFlags2 & MF2_FLY)
    {
        chicken->flags2 |= MF2_FLY;
    }
    player->chickenTics = CHICKENTICS;
    P_ActivateBeak(player);
    return (true);
}

dboolean P_ChickenMorph(mobj_t * actor)
{
    mobj_t *fog;
    mobj_t *chicken;
    mobj_t *target;
    mobjtype_t moType;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    angle_t angle;
    int ghost;

    if (actor->player)
    {
        return (false);
    }
    moType = actor->type;
    switch (moType)
    {
        case HERETIC_MT_POD:
        case HERETIC_MT_CHICKEN:
        case HERETIC_MT_HEAD:
        case HERETIC_MT_MINOTAUR:
        case HERETIC_MT_SORCERER1:
        case HERETIC_MT_SORCERER2:
            return (false);
        default:
            break;
    }
    x = actor->x;
    y = actor->y;
    z = actor->z;
    angle = actor->angle;
    ghost = actor->flags & MF_SHADOW;
    target = actor->target;
    P_SetMobjState(actor, HERETIC_S_FREETARGMOBJ);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, HERETIC_MT_TFOG);
    S_StartMobjSound(fog, heretic_sfx_telept);
    chicken = P_SpawnMobj(x, y, z, HERETIC_MT_CHICKEN);
    chicken->special2.i = moType;
    chicken->special1.i = CHICKENTICS + P_Random(pr_heretic);
    chicken->flags |= ghost;
    P_SetTarget(&chicken->target, target);
    chicken->angle = angle;
    dsda_WatchMorph(chicken);
    return (true);
}

dboolean P_AutoUseChaosDevice(player_t * player)
{
    int i;

    for (i = 0; i < player->inventorySlotNum; i++)
    {
        if (player->inventory[i].type == arti_teleport)
        {
            P_PlayerUseArtifact(player, arti_teleport);
            player->health = player->mo->health = (player->health + 1) / 2;
            return (true);
        }
    }
    return (false);
}

void P_AutoUseHealth(player_t * player, int saveHealth)
{
    int i;
    int count;
    int normalCount = 0;
    int normalSlot = 0;
    int superCount = 0;
    int superSlot = 0;

    for (i = 0; i < player->inventorySlotNum; i++)
    {
        if (player->inventory[i].type == g_arti_health)
        {
            normalSlot = i;
            normalCount = player->inventory[i].count;
        }
        else if (player->inventory[i].type == g_arti_superhealth)
        {
            superSlot = i;
            superCount = player->inventory[i].count;
        }
    }
    if (skill_info.flags & SI_AUTO_USE_HEALTH && (normalCount * 25 >= saveHealth))
    {                           // Use quartz flasks
        count = (saveHealth + 24) / 25;
        for (i = 0; i < count; i++)
        {
            player->health += P_PlayerHealthIncrease(25);
            P_PlayerRemoveArtifact(player, normalSlot);
        }
    }
    else if (superCount * 100 >= saveHealth)
    {                           // Use mystic urns
        count = (saveHealth + 99) / 100;
        for (i = 0; i < count; i++)
        {
            player->health += P_PlayerHealthIncrease(100);
            P_PlayerRemoveArtifact(player, superSlot);
        }
    }
    else if (skill_info.flags & SI_AUTO_USE_HEALTH
             && (superCount * 100 + normalCount * 25 >= saveHealth))
    {                           // Use mystic urns and quartz flasks
        count = (saveHealth + 24) / 25;
        saveHealth -= count * 25;
        for (i = 0; i < count; i++)
        {
            player->health += P_PlayerHealthIncrease(25);
            P_PlayerRemoveArtifact(player, normalSlot);
        }
        count = (saveHealth + 99) / 100;
        for (i = 0; i < count; i++)
        {
            player->health += P_PlayerHealthIncrease(100);
            P_PlayerRemoveArtifact(player, normalSlot);
        }
    }
    player->mo->health = player->health;
}

// hexen

#define TXT_MANA_1    "BLUE MANA"
#define TXT_MANA_2    "GREEN MANA"
#define TXT_MANA_BOTH "COMBINED MANA"

#define TXT_KEY_STEEL   "STEEL KEY"
#define TXT_KEY_CAVE    "CAVE KEY"
#define TXT_KEY_AXE     "AXE KEY"
#define TXT_KEY_FIRE    "FIRE KEY"
#define TXT_KEY_EMERALD "EMERALD KEY"
#define TXT_KEY_DUNGEON "DUNGEON KEY"
#define TXT_KEY_SILVER  "SILVER KEY"
#define TXT_KEY_RUSTED  "RUSTED KEY"
#define TXT_KEY_HORN    "HORN KEY"
#define TXT_KEY_SWAMP   "SWAMP KEY"
#define TXT_KEY_CASTLE  "CASTLE KEY"

#define TXT_ARTIINVULNERABILITY "ICON OF THE DEFENDER"
#define TXT_ARTIHEALTH          "QUARTZ FLASK"
#define TXT_ARTISUPERHEALTH     "MYSTIC URN"
#define TXT_ARTIHEALINGRADIUS   "MYSTIC AMBIT INCANT"
#define TXT_ARTISUMMON          "DARK SERVANT"
#define TXT_ARTITORCH           "TORCH"
#define TXT_ARTIEGG             "PORKALATOR"
#define TXT_ARTIFLY             "WINGS OF WRATH"
#define TXT_ARTIBLASTRADIUS     "DISC OF REPULSION"
#define TXT_ARTIPOISONBAG       "FLECHETTE"
#define TXT_ARTITELEPORTOTHER   "BANISHMENT DEVICE"
#define TXT_ARTISPEED           "BOOTS OF SPEED"
#define TXT_ARTIBOOSTMANA       "KRATER OF MIGHT"
#define TXT_ARTIBOOSTARMOR      "DRAGONSKIN BRACERS"
#define TXT_ARTITELEPORT        "CHAOS DEVICE"

#define TXT_ITEMHEALTH        "CRYSTAL VIAL"
#define TXT_ITEMBAGOFHOLDING "BAG OF HOLDING"
#define TXT_ITEMSHIELD1      "SILVER SHIELD"
#define TXT_ITEMSHIELD2      "ENCHANTED SHIELD"
#define TXT_ITEMSUPERMAP     "MAP SCROLL"

#define TXT_ARMOR1 "MESH ARMOR"
#define TXT_ARMOR2 "FALCON SHIELD"
#define TXT_ARMOR3 "PLATINUM HELMET"
#define TXT_ARMOR4 "AMULET OF WARDING"

#define TXT_WEAPON_F2 "TIMON'S AXE"
#define TXT_WEAPON_F3 "HAMMER OF RETRIBUTION"
#define TXT_WEAPON_F4 "QUIETUS ASSEMBLED"
#define TXT_WEAPON_C2 "SERPENT STAFF"
#define TXT_WEAPON_C3 "FIRESTORM"
#define TXT_WEAPON_C4 "WRAITHVERGE ASSEMBLED"
#define TXT_WEAPON_M2 "FROST SHARDS"
#define TXT_WEAPON_M3 "ARC OF DEATH"
#define TXT_WEAPON_M4 "BLOODSCOURGE ASSEMBLED"

#define TXT_QUIETUS_PIECE      "SEGMENT OF QUIETUS"
#define TXT_WRAITHVERGE_PIECE  "SEGMENT OF WRAITHVERGE"
#define TXT_BLOODSCOURGE_PIECE "SEGMENT OF BLOODSCOURGE"

const char *TextKeyMessages[] = {
    TXT_KEY_STEEL,
    TXT_KEY_CAVE,
    TXT_KEY_AXE,
    TXT_KEY_FIRE,
    TXT_KEY_EMERALD,
    TXT_KEY_DUNGEON,
    TXT_KEY_SILVER,
    TXT_KEY_RUSTED,
    TXT_KEY_HORN,
    TXT_KEY_SWAMP,
    TXT_KEY_CASTLE
};

void P_FallingDamage(player_t * player)
{
    int damage;
    int mom;
    int dist;

    mom = abs(player->mo->momz);
    dist = FixedMul(mom, 16 * FRACUNIT / 23);

    if (mom >= 63 * FRACUNIT)
    {                           // automatic death
        P_DamageMobj(player->mo, NULL, NULL, 10000);
        return;
    }
    damage = ((FixedMul(dist, dist) / 10) >> FRACBITS) - 24;
    if (player->mo->momz > -39 * FRACUNIT && damage > player->mo->health
        && player->mo->health != 1)
    {                           // No-death threshold
        damage = player->mo->health - 1;
    }
    S_StartMobjSound(player->mo, hexen_sfx_player_land);
    P_DamageMobj(player->mo, NULL, NULL, damage);
}

void P_PoisonDamage(player_t * player, mobj_t * source, int damage,
                    dboolean playPainSound)
{
    mobj_t *target;
    mobj_t *inflictor;

    target = player->mo;
    inflictor = source;
    if (target->health <= 0)
    {
        return;
    }
    if (target->flags2 & MF2_INVULNERABLE && damage < 10000)
    {                           // mobj is invulnerable
        return;
    }
    if (skill_info.damage_factor)
    {
        damage = FixedMul(damage, skill_info.damage_factor);
    }
    if (damage < 1000 && ((player->cheats & CF_GODMODE)
                          || player->powers[pw_invulnerability]))
    {
        return;
    }
    if (damage >= player->health
        && (skill_info.flags & SI_AUTO_USE_HEALTH || deathmatch) && !player->morphTics)
    {                           // Try to use some inventory health
        P_AutoUseHealth(player, damage - player->health + 1);
    }
    player->health -= damage;   // mirror mobj health here for Dave
    if (player->health < 0)
    {
        player->health = 0;
    }
    player->attacker = source;

    //
    // do the damage
    //
    target->health -= damage;
    if (target->health <= 0)
    {                           // Death
        target->special1.i = damage;
        if (inflictor && !player->morphTics)
        {                       // Check for flame death
            if ((inflictor->flags2 & MF2_FIREDAMAGE)
                && (target->health > -50) && (damage > 25))
            {
                target->flags2 |= MF2_FIREDAMAGE;
            }
            if (inflictor->flags2 & MF2_ICEDAMAGE)
            {
                target->flags2 |= MF2_ICEDAMAGE;
            }
        }
        P_KillMobj(source, target);
        return;
    }
    if (!(leveltime & 63) && playPainSound)
    {
        P_SetMobjState(target, target->info->painstate);
    }
}

dboolean P_GiveMana(player_t * player, manatype_t mana, int count)
{
    int prevMana;

    if (mana == MANA_NONE || mana == MANA_BOTH)
    {
        return (false);
    }
    if ((unsigned int) mana > NUMMANA)
    {
        I_Error("P_GiveMana: bad type %i", mana);
    }
    if (player->ammo[mana] == MAX_MANA)
    {
        return (false);
    }
    if (skill_info.ammo_factor)
    {
        count = FixedMul(count, skill_info.ammo_factor);
    }
    prevMana = player->ammo[mana];

    player->ammo[mana] += count;
    if (player->ammo[mana] > MAX_MANA)
    {
        player->ammo[mana] = MAX_MANA;
    }
    if (player->pclass == PCLASS_FIGHTER && player->readyweapon == wp_second
        && mana == MANA_1 && prevMana <= 0)
    {
        P_SetPsprite(player, ps_weapon, HEXEN_S_FAXEREADY_G);
    }
    return (true);
}

dboolean Hexen_P_GiveArmor(player_t * player, armortype_t armortype, int amount)
{
    int hits;
    int totalArmor;

    if (amount == -1)
    {
        hits = pclass[player->pclass].armor_increment[armortype];
        if (player->armorpoints[armortype] >= hits)
        {
            return false;
        }
        else
        {
            player->armorpoints[armortype] = hits;
        }
    }
    else
    {
        hits = amount * 5 * FRACUNIT;
        totalArmor = player->armorpoints[ARMOR_ARMOR]
            + player->armorpoints[ARMOR_SHIELD]
            + player->armorpoints[ARMOR_HELMET]
            + player->armorpoints[ARMOR_AMULET]
            + pclass[player->pclass].auto_armor_save;
        if (totalArmor < pclass[player->pclass].armor_max)
        {
            player->armorpoints[armortype] += hits;
        }
        else
        {
            return false;
        }
    }
    return true;
}

void P_SetYellowMessage(player_t * player, const char *message, dboolean ultmsg)
{
    dsda_AddPlayerMessage(message, player);
    player->yellowMessage = true;
}

void TryPickupWeapon(player_t * player, pclass_t weaponClass,
                     weapontype_t weaponType, mobj_t * weapon,
                     const char *message)
{
    dboolean remove;
    dboolean gaveMana;
    dboolean gaveWeapon;

    remove = true;
    if (player->pclass != weaponClass)
    {                           // Wrong class, but try to pick up for mana
        if (netgame && !deathmatch)
        {                       // Can't pick up weapons for other classes in coop netplay
            return;
        }
        if (weaponType == wp_second)
        {
            if (!P_GiveMana(player, MANA_1, 25))
            {
                return;
            }
        }
        else
        {
            if (!P_GiveMana(player, MANA_2, 25))
            {
                return;
            }
        }
    }
    else if (netgame && !deathmatch)
    {                           // Cooperative net-game
        if (player->weaponowned[weaponType])
        {
            return;
        }
        player->weaponowned[weaponType] = true;
        if (weaponType == wp_second)
        {
            P_GiveMana(player, MANA_1, 25);
        }
        else
        {
            P_GiveMana(player, MANA_2, 25);
        }
        player->pendingweapon = weaponType;
        remove = false;
    }
    else
    {                           // Deathmatch or single player game
        if (weaponType == wp_second)
        {
            gaveMana = P_GiveMana(player, MANA_1, 25);
        }
        else
        {
            gaveMana = P_GiveMana(player, MANA_2, 25);
        }
        if (player->weaponowned[weaponType])
        {
            gaveWeapon = false;
        }
        else
        {
            gaveWeapon = true;
            player->weaponowned[weaponType] = true;
            if (weaponType > player->readyweapon)
            {                   // Only switch to more powerful weapons
                player->pendingweapon = weaponType;
            }
        }
        if (!(gaveWeapon || gaveMana))
        {                       // Player didn't need the weapon or any mana
            return;
        }
    }

    P_SetMessage(player, message, false);
    if (weapon->special)
    {
        map_format.execute_line_special(weapon->special, weapon->special_args, NULL, 0, player->mo);
        weapon->special = 0;
    }

    if (remove && !(weapon->intflags & MIF_FAKE))
    {
        if (deathmatch && !(weapon->flags & MF_DROPPED))
        {
            P_HideSpecialThing(weapon);
        }
        else
        {
            P_RemoveMobj(weapon);
        }
    }

    player->bonuscount += BONUSADD;
    if (player == &players[consoleplayer])
    {
        S_StartVoidSound(hexen_sfx_pickup_weapon);
        SB_PaletteFlash(false);
    }
}

static void TryPickupWeaponPiece(player_t * player, pclass_t matchClass,
                                 int pieceValue, mobj_t * pieceMobj)
{
    dboolean remove;
    dboolean checkAssembled;
    dboolean gaveWeapon;
    int gaveMana;
    static const char *fourthWeaponText[] = {
        0,
        TXT_WEAPON_F4,
        TXT_WEAPON_C4,
        TXT_WEAPON_M4
    };
    static const char *weaponPieceText[] = {
        0,
        TXT_QUIETUS_PIECE,
        TXT_WRAITHVERGE_PIECE,
        TXT_BLOODSCOURGE_PIECE
    };
    static int pieceValueTrans[] = {
        0,                      // 0: never
        WPIECE1 | WPIECE2 | WPIECE3,    // WPIECE1 (1)
        WPIECE2 | WPIECE3,      // WPIECE2 (2)
        0,                      // 3: never
        WPIECE3                 // WPIECE3 (4)
    };

    remove = true;
    checkAssembled = true;
    gaveWeapon = false;
    if (player->pclass != matchClass)
    {                           // Wrong class, but try to pick up for mana
        if (netgame && !deathmatch)
        {                       // Can't pick up wrong-class weapons in coop netplay
            return;
        }
        checkAssembled = false;
        gaveMana = P_GiveMana(player, MANA_1, 20) +
            P_GiveMana(player, MANA_2, 20);
        if (!gaveMana)
        {                       // Didn't need the mana, so don't pick it up
            return;
        }
    }
    else if (netgame && !deathmatch)
    {                           // Cooperative net-game
        if (player->pieces & pieceValue)
        {                       // Already has the piece
            return;
        }
        pieceValue = pieceValueTrans[pieceValue];
        P_GiveMana(player, MANA_1, 20);
        P_GiveMana(player, MANA_2, 20);
        remove = false;
    }
    else
    {                           // Deathmatch or single player game
        gaveMana = P_GiveMana(player, MANA_1, 20) +
            P_GiveMana(player, MANA_2, 20);
        if (player->pieces & pieceValue)
        {                       // Already has the piece, check if mana needed
            if (!gaveMana)
            {                   // Didn't need the mana, so don't pick it up
                return;
            }
            checkAssembled = false;
        }
    }

    // Pick up the weapon piece
    if (pieceMobj->special)
    {
        map_format.execute_line_special(pieceMobj->special, pieceMobj->special_args, NULL, 0, player->mo);
        pieceMobj->special = 0;
    }
    if (remove)
    {
        if (deathmatch && !(pieceMobj->flags & MF_DROPPED))
        {
            P_HideSpecialThing(pieceMobj);
        }
        else
        {
            P_RemoveMobj(pieceMobj);
        }
    }
    player->bonuscount += BONUSADD;
    if (player == &players[consoleplayer])
    {
      SB_PaletteFlash(false);
    }

    // Check if fourth weapon assembled
    if (checkAssembled)
    {
        player->pieces |= pieceValue;
        if (player->pieces == (WPIECE1 | WPIECE2 | WPIECE3))
        {
            gaveWeapon = true;
            player->weaponowned[wp_fourth] = true;
            player->pendingweapon = wp_fourth;
        }
    }

    if (gaveWeapon)
    {
        P_SetMessage(player, fourthWeaponText[matchClass], false);
        // Play the build-sound full volume for all players
        S_StartVoidSound(hexen_sfx_weapon_build);
    }
    else
    {
        P_SetMessage(player, weaponPieceText[matchClass], false);
        if (player == &players[consoleplayer])
        {
            S_StartVoidSound(hexen_sfx_pickup_weapon);
        }
    }
}

int P_GiveKey(player_t * player, card_t key)
{
    if (player->cards[key])
    {
        return false;
    }
    player->bonuscount += BONUSADD;
    player->cards[key] = true;

    if (player == &players[consoleplayer])
      playerkeys |= 1 << key;

    return true;
}

static void SetDormantArtifact(mobj_t * arti)
{
    arti->flags &= ~MF_SPECIAL;
    if (deathmatch && !(arti->flags & MF_DROPPED))
    {
        if (arti->type == HEXEN_MT_ARTIINVULNERABILITY)
        {
            P_SetMobjState(arti, HEXEN_S_DORMANTARTI3_1);
        }
        else if (arti->type == HEXEN_MT_SUMMONMAULATOR || arti->type == HEXEN_MT_ARTIFLY)
        {
            P_SetMobjState(arti, HEXEN_S_DORMANTARTI2_1);
        }
        else
        {
            P_SetMobjState(arti, HEXEN_S_DORMANTARTI1_1);
        }
    }
    else
    {                           // Don't respawn
        P_SetMobjState(arti, HEXEN_S_DEADARTI1);
    }
}

static void TryPickupArtifact(player_t * player, artitype_t artifactType, mobj_t * artifact)
{
    static const char *artifactMessages[HEXEN_NUMARTIFACTS] = {
        NULL,
        TXT_ARTIINVULNERABILITY,
        TXT_ARTIHEALTH,
        TXT_ARTISUPERHEALTH,
        TXT_ARTIHEALINGRADIUS,
        TXT_ARTISUMMON,
        TXT_ARTITORCH,
        TXT_ARTIEGG,
        TXT_ARTIFLY,
        TXT_ARTIBLASTRADIUS,
        TXT_ARTIPOISONBAG,
        TXT_ARTITELEPORTOTHER,
        TXT_ARTISPEED,
        TXT_ARTIBOOSTMANA,
        TXT_ARTIBOOSTARMOR,
        TXT_ARTITELEPORT,
        TXT_ARTIPUZZSKULL,
        TXT_ARTIPUZZGEMBIG,
        TXT_ARTIPUZZGEMRED,
        TXT_ARTIPUZZGEMGREEN1,
        TXT_ARTIPUZZGEMGREEN2,
        TXT_ARTIPUZZGEMBLUE1,
        TXT_ARTIPUZZGEMBLUE2,
        TXT_ARTIPUZZBOOK1,
        TXT_ARTIPUZZBOOK2,
        TXT_ARTIPUZZSKULL2,
        TXT_ARTIPUZZFWEAPON,
        TXT_ARTIPUZZCWEAPON,
        TXT_ARTIPUZZMWEAPON,
        TXT_ARTIPUZZGEAR,       // All gear pickups use the same text
        TXT_ARTIPUZZGEAR,
        TXT_ARTIPUZZGEAR,
        TXT_ARTIPUZZGEAR
    };

    if (gamemode == shareware)
    {
        artifactMessages[hexen_arti_blastradius] = TXT_ARTITELEPORT;
        artifactMessages[hexen_arti_teleport] = TXT_ARTIBLASTRADIUS;
    }

    if (P_GiveArtifact(player, artifactType, artifact))
    {
        if (artifact->special)
        {
            map_format.execute_line_special(artifact->special, artifact->special_args, NULL, 0, NULL);
            artifact->special = 0;
        }
        player->bonuscount += BONUSADD;
        if (artifactType < hexen_arti_firstpuzzitem)
        {
            SetDormantArtifact(artifact);
            S_StartMobjSound(artifact, hexen_sfx_pickup_artifact);
            P_SetMessage(player, artifactMessages[artifactType], false);
        }
        else
        {                       // Puzzle item
            S_StartVoidSound(hexen_sfx_pickup_item);
            P_SetMessage(player, artifactMessages[artifactType], true);
            if (!netgame || deathmatch)
            {                   // Remove puzzle items if not cooperative netplay
                P_RemoveMobj(artifact);
            }
        }
    }
}

static void Hexen_P_TouchSpecialThing(mobj_t * special, mobj_t * toucher)
{
    player_t *player;
    fixed_t delta;
    int sound;
    dboolean respawn;

    delta = special->z - toucher->z;
    if (delta > toucher->height || delta < -32 * FRACUNIT)
    {                           // Out of reach
        return;
    }
    if (toucher->health <= 0)
    {                           // Toucher is dead
        return;
    }
    sound = hexen_sfx_pickup_item;
    player = toucher->player;
    respawn = true;
    switch (special->sprite)
    {
            // Items
        case HEXEN_SPR_PTN1:         // Item_HealingPotion
            if (!P_GiveBody(player, 10))
            {
                return;
            }
            P_SetMessage(player, TXT_ITEMHEALTH, false);
            break;
        case HEXEN_SPR_ARM1:
            if (!Hexen_P_GiveArmor(player, ARMOR_ARMOR, -1))
            {
                return;
            }
            P_SetMessage(player, TXT_ARMOR1, false);
            break;
        case HEXEN_SPR_ARM2:
            if (!Hexen_P_GiveArmor(player, ARMOR_SHIELD, -1))
            {
                return;
            }
            P_SetMessage(player, TXT_ARMOR2, false);
            break;
        case HEXEN_SPR_ARM3:
            if (!Hexen_P_GiveArmor(player, ARMOR_HELMET, -1))
            {
                return;
            }
            P_SetMessage(player, TXT_ARMOR3, false);
            break;
        case HEXEN_SPR_ARM4:
            if (!Hexen_P_GiveArmor(player, ARMOR_AMULET, -1))
            {
                return;
            }
            P_SetMessage(player, TXT_ARMOR4, false);
            break;

            // Keys
        case HEXEN_SPR_KEY1:
        case HEXEN_SPR_KEY2:
        case HEXEN_SPR_KEY3:
        case HEXEN_SPR_KEY4:
        case HEXEN_SPR_KEY5:
        case HEXEN_SPR_KEY6:
        case HEXEN_SPR_KEY7:
        case HEXEN_SPR_KEY8:
        case HEXEN_SPR_KEY9:
        case HEXEN_SPR_KEYA:
        case HEXEN_SPR_KEYB:
            if (!P_GiveKey(player, special->sprite - HEXEN_SPR_KEY1))
            {
                return;
            }
            P_SetMessage(player, TextKeyMessages[special->sprite - HEXEN_SPR_KEY1],
                         true);
            sound = hexen_sfx_pickup_key;

            // Check and process the special now in case the key doesn't
            // get removed for coop netplay
            if (special->special)
            {
                map_format.execute_line_special(special->special, special->special_args, NULL, 0, toucher);
                special->special = 0;
            }

            if (!netgame)
            {                   // Only remove keys in single player game
                break;
            }
            player->bonuscount += BONUSADD;
            if (player == &players[consoleplayer])
            {
                S_StartVoidSound(sound);
                SB_PaletteFlash(false);
            }
            return;

            // Artifacts
        case HEXEN_SPR_PTN2:
            TryPickupArtifact(player, hexen_arti_health, special);
            return;
        case HEXEN_SPR_SOAR:
            TryPickupArtifact(player, hexen_arti_fly, special);
            return;
        case HEXEN_SPR_INVU:
            TryPickupArtifact(player, hexen_arti_invulnerability, special);
            return;
        case HEXEN_SPR_SUMN:
            TryPickupArtifact(player, hexen_arti_summon, special);
            return;
        case HEXEN_SPR_PORK:
            TryPickupArtifact(player, hexen_arti_egg, special);
            return;
        case HEXEN_SPR_SPHL:
            TryPickupArtifact(player, hexen_arti_superhealth, special);
            return;
        case HEXEN_SPR_HRAD:
            TryPickupArtifact(player, hexen_arti_healingradius, special);
            return;
        case HEXEN_SPR_TRCH:
            TryPickupArtifact(player, hexen_arti_torch, special);
            return;
        case HEXEN_SPR_ATLP:
            TryPickupArtifact(player, hexen_arti_teleport, special);
            return;
        case HEXEN_SPR_TELO:
            TryPickupArtifact(player, hexen_arti_teleportother, special);
            return;
        case HEXEN_SPR_PSBG:
            TryPickupArtifact(player, hexen_arti_poisonbag, special);
            return;
        case HEXEN_SPR_SPED:
            TryPickupArtifact(player, hexen_arti_speed, special);
            return;
        case HEXEN_SPR_BMAN:
            TryPickupArtifact(player, hexen_arti_boostmana, special);
            return;
        case HEXEN_SPR_BRAC:
            TryPickupArtifact(player, hexen_arti_boostarmor, special);
            return;
        case HEXEN_SPR_BLST:
            TryPickupArtifact(player, hexen_arti_blastradius, special);
            return;

            // Puzzle artifacts
        case HEXEN_SPR_ASKU:
            TryPickupArtifact(player, hexen_arti_puzzskull, special);
            return;
        case HEXEN_SPR_ABGM:
            TryPickupArtifact(player, hexen_arti_puzzgembig, special);
            return;
        case HEXEN_SPR_AGMR:
            TryPickupArtifact(player, hexen_arti_puzzgemred, special);
            return;
        case HEXEN_SPR_AGMG:
            TryPickupArtifact(player, hexen_arti_puzzgemgreen1, special);
            return;
        case HEXEN_SPR_AGG2:
            TryPickupArtifact(player, hexen_arti_puzzgemgreen2, special);
            return;
        case HEXEN_SPR_AGMB:
            TryPickupArtifact(player, hexen_arti_puzzgemblue1, special);
            return;
        case HEXEN_SPR_AGB2:
            TryPickupArtifact(player, hexen_arti_puzzgemblue2, special);
            return;
        case HEXEN_SPR_ABK1:
            TryPickupArtifact(player, hexen_arti_puzzbook1, special);
            return;
        case HEXEN_SPR_ABK2:
            TryPickupArtifact(player, hexen_arti_puzzbook2, special);
            return;
        case HEXEN_SPR_ASK2:
            TryPickupArtifact(player, hexen_arti_puzzskull2, special);
            return;
        case HEXEN_SPR_AFWP:
            TryPickupArtifact(player, hexen_arti_puzzfweapon, special);
            return;
        case HEXEN_SPR_ACWP:
            TryPickupArtifact(player, hexen_arti_puzzcweapon, special);
            return;
        case HEXEN_SPR_AMWP:
            TryPickupArtifact(player, hexen_arti_puzzmweapon, special);
            return;
        case HEXEN_SPR_AGER:
            TryPickupArtifact(player, hexen_arti_puzzgear1, special);
            return;
        case HEXEN_SPR_AGR2:
            TryPickupArtifact(player, hexen_arti_puzzgear2, special);
            return;
        case HEXEN_SPR_AGR3:
            TryPickupArtifact(player, hexen_arti_puzzgear3, special);
            return;
        case HEXEN_SPR_AGR4:
            TryPickupArtifact(player, hexen_arti_puzzgear4, special);
            return;

            // Mana
        case HEXEN_SPR_MAN1:
            if (!P_GiveMana(player, MANA_1, 15))
            {
                return;
            }
            P_SetMessage(player, TXT_MANA_1, false);
            break;
        case HEXEN_SPR_MAN2:
            if (!P_GiveMana(player, MANA_2, 15))
            {
                return;
            }
            P_SetMessage(player, TXT_MANA_2, false);
            break;
        case HEXEN_SPR_MAN3:         // Double Mana Dodecahedron
            if (!P_GiveMana(player, MANA_1, 20))
            {
                if (!P_GiveMana(player, MANA_2, 20))
                {
                    return;
                }
            }
            else
            {
                P_GiveMana(player, MANA_2, 20);
            }
            P_SetMessage(player, TXT_MANA_BOTH, false);
            break;

            // 2nd and 3rd Mage Weapons
        case HEXEN_SPR_WMCS:         // Frost Shards
            TryPickupWeapon(player, PCLASS_MAGE, wp_second,
                            special, TXT_WEAPON_M2);
            return;
        case HEXEN_SPR_WMLG:         // Arc of Death
            TryPickupWeapon(player, PCLASS_MAGE, wp_third,
                            special, TXT_WEAPON_M3);
            return;

            // 2nd and 3rd Fighter Weapons
        case HEXEN_SPR_WFAX:         // Timon's Axe
            TryPickupWeapon(player, PCLASS_FIGHTER, wp_second,
                            special, TXT_WEAPON_F2);
            return;
        case HEXEN_SPR_WFHM:         // Hammer of Retribution
            TryPickupWeapon(player, PCLASS_FIGHTER, wp_third,
                            special, TXT_WEAPON_F3);
            return;

            // 2nd and 3rd Cleric Weapons
        case HEXEN_SPR_WCSS:         // Serpent Staff
            TryPickupWeapon(player, PCLASS_CLERIC, wp_second,
                            special, TXT_WEAPON_C2);
            return;
        case HEXEN_SPR_WCFM:         // Firestorm
            TryPickupWeapon(player, PCLASS_CLERIC, wp_third,
                            special, TXT_WEAPON_C3);
            return;

            // Fourth Weapon Pieces
        case HEXEN_SPR_WFR1:
            TryPickupWeaponPiece(player, PCLASS_FIGHTER, WPIECE1, special);
            return;
        case HEXEN_SPR_WFR2:
            TryPickupWeaponPiece(player, PCLASS_FIGHTER, WPIECE2, special);
            return;
        case HEXEN_SPR_WFR3:
            TryPickupWeaponPiece(player, PCLASS_FIGHTER, WPIECE3, special);
            return;
        case HEXEN_SPR_WCH1:
            TryPickupWeaponPiece(player, PCLASS_CLERIC, WPIECE1, special);
            return;
        case HEXEN_SPR_WCH2:
            TryPickupWeaponPiece(player, PCLASS_CLERIC, WPIECE2, special);
            return;
        case HEXEN_SPR_WCH3:
            TryPickupWeaponPiece(player, PCLASS_CLERIC, WPIECE3, special);
            return;
        case HEXEN_SPR_WMS1:
            TryPickupWeaponPiece(player, PCLASS_MAGE, WPIECE1, special);
            return;
        case HEXEN_SPR_WMS2:
            TryPickupWeaponPiece(player, PCLASS_MAGE, WPIECE2, special);
            return;
        case HEXEN_SPR_WMS3:
            TryPickupWeaponPiece(player, PCLASS_MAGE, WPIECE3, special);
            return;

        default:
            I_Error("P_SpecialThing: Unknown gettable thing");
    }
    if (special->special)
    {
        map_format.execute_line_special(special->special, special->special_args, NULL, 0, toucher);
        special->special = 0;
    }
    if (deathmatch && respawn && !(special->flags & MF_DROPPED))
    {
        P_HideSpecialThing(special);
    }
    else
    {
        P_RemoveMobj(special);
    }
    player->bonuscount += BONUSADD;
    if (player == &players[consoleplayer])
    {
        S_StartVoidSound(sound);
        SB_PaletteFlash(false);
    }
}

// Search thinker list for minotaur
static mobj_t *ActiveMinotaur(player_t * master)
{
    byte args[5];
    mobj_t *mo;
    player_t *plr;
    thinker_t *think;
    unsigned int starttime;

    for (think = thinkercap.next; think != &thinkercap; think = think->next)
    {
        if (think->function != P_MobjThinker)
            continue;
        mo = (mobj_t *) think;
        if (mo->type != HEXEN_MT_MINOTAUR)
            continue;
        if (mo->health <= 0)
            continue;
        if (!(mo->flags & MF_COUNTKILL))
            continue;           // for morphed minotaurs
        if (mo->flags & MF_CORPSE)
            continue;

        COLLAPSE_SPECIAL_ARGS(args, mo->special_args);
        memcpy(&starttime, args, sizeof(unsigned int));
        if (leveltime - LittleLong(starttime) >= MAULATORTICS)
            continue;

        plr = mo->special1.m->player;
        if (plr == master)
            return (mo);
    }
    return (NULL);
}

dboolean P_MorphPlayer(player_t * player)
{
    mobj_t *pmo;
    mobj_t *fog;
    mobj_t *beastMo;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    angle_t angle;
    int oldFlags2;

    if (player->powers[pw_invulnerability])
    {                           // Immune when invulnerable
        return (false);
    }
    if (player->morphTics)
    {                           // Player is already a beast
        return false;
    }
    pmo = player->mo;
    x = pmo->x;
    y = pmo->y;
    z = pmo->z;
    angle = pmo->angle;
    oldFlags2 = pmo->flags2;
    P_SetMobjState(pmo, HEXEN_S_FREETARGMOBJ);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, HEXEN_MT_TFOG);
    S_StartMobjSound(fog, hexen_sfx_teleport);
    beastMo = P_SpawnMobj(x, y, z, HEXEN_MT_PIGPLAYER);
    beastMo->special1.i = player->readyweapon;
    beastMo->angle = angle;
    beastMo->player = player;
    player->health = beastMo->health = MAXMORPHHEALTH;
    player->mo = beastMo;
    memset(&player->armorpoints[0], 0, NUMARMOR * sizeof(int));
    player->pclass = PCLASS_PIG;
    if (oldFlags2 & MF2_FLY)
    {
        beastMo->flags2 |= MF2_FLY;
    }
    player->morphTics = MORPHTICS;
    P_ActivateMorphWeapon(player);
    return (true);
}

static dboolean P_MorphMonster(mobj_t * actor)
{
    mobj_t *master, *monster, *fog;
    mobjtype_t moType;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    mobj_t oldMonster;

    if (actor->player)
        return (false);
    if (!(actor->flags & MF_COUNTKILL))
        return false;
    if (actor->flags2 & MF2_BOSS)
        return false;
    moType = actor->type;
    switch (moType)
    {
        case HEXEN_MT_PIG:
            return (false);
        case HEXEN_MT_FIGHTER_BOSS:
        case HEXEN_MT_CLERIC_BOSS:
        case HEXEN_MT_MAGE_BOSS:
            return (false);
        default:
            break;
    }

    oldMonster = *actor;
    x = oldMonster.x;
    y = oldMonster.y;
    z = oldMonster.z;
    map_format.remove_mobj_thing_id(actor);
    P_SetMobjState(actor, HEXEN_S_FREETARGMOBJ);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, HEXEN_MT_TFOG);
    S_StartMobjSound(fog, hexen_sfx_teleport);
    monster = P_SpawnMobj(x, y, z, HEXEN_MT_PIG);
    monster->special2.i = moType;
    monster->special1.i = MORPHTICS + P_Random(pr_hexen);
    monster->flags |= (oldMonster.flags & MF_SHADOW);
    P_SetTarget(&monster->target, oldMonster.target);
    monster->angle = oldMonster.angle;
    monster->tid = oldMonster.tid;
    monster->special = oldMonster.special;
    map_format.add_mobj_thing_id(monster, oldMonster.tid);
    memcpy(monster->special_args, oldMonster.special_args, SPECIAL_ARGS_SIZE);
    dsda_WatchMorph(monster);

    // check for turning off minotaur power for active icon
    if (moType == HEXEN_MT_MINOTAUR)
    {
        master = oldMonster.special1.m;
        if (master->health > 0)
        {
            if (!ActiveMinotaur(master->player))
            {
                master->player->powers[pw_minotaur] = 0;
            }
        }
    }
    return (true);
}

void P_PoisonPlayer(player_t * player, mobj_t * poisoner, int poison)
{
    if ((player->cheats & CF_GODMODE) || player->powers[pw_invulnerability])
    {
        return;
    }
    player->poisoncount += poison;
    player->poisoner = poisoner;
    if (player->poisoncount > 100)
    {
        player->poisoncount = 100;
    }
}
