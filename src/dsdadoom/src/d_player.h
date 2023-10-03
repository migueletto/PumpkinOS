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
 *  Player state structure.
 *
 *-----------------------------------------------------------------------------*/


#ifndef __D_PLAYER__
#define __D_PLAYER__

#include "dsda/pclass.h"

// The player data structure depends on a number
// of other structs: items (internal inventory),
// animation states (closely tied to the sprites
// used to represent them, unfortunately).
#include "d_items.h"
#include "p_pspr.h"

// In addition, the player is just a special
// case of the generic moving object/actor.
#include "p_mobj.h"

// Finally, for odd reasons, the player input
// is buffered within the player data struct,
// as commands per game tick.
#include "d_ticcmd.h"

//
// Player states.
//
typedef enum
{
  // Playing or camping.
  PST_LIVE,
  // Dead on the ground, view follows killer.
  PST_DEAD,
  // Ready to restart/respawn???
  PST_REBORN

} playerstate_t;

#define CF_NOCLIP        0x01 // no clipping
#define CF_GODMODE       0x02 // immune to damage
#define CF_INFINITE_AMMO 0x04 // infinite ammo
#define CF_NOTARGET      0x08 // monsters don't target
#define CF_FLY           0x10 // flying player

// heretic
typedef struct
{
    int type;
    int count;
} inventory_t;

// heretic
typedef enum
{
    arti_none,
    arti_invulnerability,
    arti_invisibility,
    arti_health,
    arti_superhealth,
    arti_tomeofpower,
    arti_torch,
    arti_firebomb,
    arti_egg,
    arti_fly,
    arti_teleport,
    NUMARTIFACTS,

    // hexen
    hexen_arti_none = arti_none,
    hexen_arti_invulnerability,
    hexen_arti_health,
    hexen_arti_superhealth,
    hexen_arti_healingradius,
    hexen_arti_summon,
    hexen_arti_torch,
    hexen_arti_egg,
    hexen_arti_fly,
    hexen_arti_blastradius,
    hexen_arti_poisonbag,
    hexen_arti_teleportother,
    hexen_arti_speed,
    hexen_arti_boostmana,
    hexen_arti_boostarmor,
    hexen_arti_teleport,
    // Puzzle artifacts
    hexen_arti_firstpuzzitem,
    hexen_arti_puzzskull = hexen_arti_firstpuzzitem,
    hexen_arti_puzzgembig,
    hexen_arti_puzzgemred,
    hexen_arti_puzzgemgreen1,
    hexen_arti_puzzgemgreen2,
    hexen_arti_puzzgemblue1,
    hexen_arti_puzzgemblue2,
    hexen_arti_puzzbook1,
    hexen_arti_puzzbook2,
    hexen_arti_puzzskull2,
    hexen_arti_puzzfweapon,
    hexen_arti_puzzcweapon,
    hexen_arti_puzzmweapon,
    hexen_arti_puzzgear1,
    hexen_arti_puzzgear2,
    hexen_arti_puzzgear3,
    hexen_arti_puzzgear4,
    HEXEN_NUMARTIFACTS
} artitype_t;

#define NUMINVENTORYSLOTS	HEXEN_NUMARTIFACTS

//
// Extended player object info: player_t
//
typedef struct player_s
{
  mobj_t*             mo;
  playerstate_t       playerstate;
  ticcmd_t            cmd;

  // Determine POV,
  //  including viewpoint bobbing during movement.
  // Focal origin above r.z
  fixed_t             viewz;
  // Base height above floor for viewz.
  fixed_t             viewheight;
  // Bob/squat speed.
  fixed_t             deltaviewheight;
  // bounded/scaled total momentum.
  fixed_t             bob;

  // This is only used between levels,
  // mo->health is used during levels.
  int                 health;
  int                 armorpoints[NUMARMOR];
  // Armor type is 0-2.
  int                 armortype;

  // Power ups. invinc and invis are tic counters.
  int                 powers[NUMPOWERS];
  dboolean           cards[NUMCARDS];
  dboolean           backpack;

  // Frags, kills of other players.
  int                 frags[MAX_MAXPLAYERS];
  weapontype_t        readyweapon;

  // Is wp_nochange if not changing.
  weapontype_t        pendingweapon;

  dboolean           weaponowned[NUMWEAPONS];
  int                 ammo[NUMAMMO];
  int                 maxammo[NUMAMMO];

  // True if button down last tic.
  int                 attackdown;
  int                 usedown;

  // See CF flags above.
  int                 cheats;

  // Refired shots are less accurate.
  int                 refire;

   // For intermission stats.
  int                 killcount;
  int                 itemcount;
  int                 secretcount;

  // For screen flashing (red or bright).
  int                 damagecount;
  int                 bonuscount;

  // Who did damage (NULL for floors/ceilings).
  mobj_t*             attacker;

  // So gun flashes light up areas.
  int                 extralight;

  // Current PLAYPAL, ???
  //  can be set to REDCOLORMAP for pain, etc.
  int                 fixedcolormap;

  // Player skin colorshift,
  //  0-3 for which color to draw player.
  int                 colormap;

  // Overlay view sprites (gun, etc).
  pspdef_t            psprites[NUMPSPRITES];

  // True if secret level has been done.
  dboolean           didsecret;

  // e6y
  // All non original (new) fields of player_t struct are moved to bottom
  // for compatibility with overflow (from a deh) of player_t::ammo[NUMAMMO]

  /* killough 10/98: used for realistic bobbing (i.e. not simply overall speed)
   * mo->momx and mo->momy represent true momenta experienced by player.
   * This only represents the thrust that the player applies himself.
   * This avoids anomolies with such things as Boom ice and conveyors.
   */
  fixed_t            momx, momy;      // killough 10/98

  //e6y
  int                 maxkilldiscount;

  fixed_t prev_viewz;
  angle_t prev_viewangle;
  angle_t prev_viewpitch;

  // heretic
  int flyheight;
  int lookdir;
  dboolean centering;
  inventory_t inventory[NUMINVENTORYSLOTS];
  artitype_t readyArtifact;
  int artifactCount;
  int inventorySlotNum;
  int flamecount;             // for flame thrower duration
  int chickenTics;            // player is a chicken if > 0
  int chickenPeck;            // chicken peck countdown
  mobj_t *rain1;              // active rain maker 1
  mobj_t *rain2;              // active rain maker 2

  // hexen
  pclass_t pclass;            // player class type
  int morphTics;              // player is a pig if > 0
  int pieces;                 // Fourth Weapon pieces
  short yellowMessage;
  int poisoncount;            // screen flash for poison damage
  mobj_t *poisoner;           // NULL for non-player mobjs
  unsigned int jumpTics;      // delay the next jump for a moment
  unsigned int worldTimer;    // total time the player's been playing

  // zdoom
  int hazardcount;
  byte hazardinterval;
} player_t;


//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//
typedef struct
{
  dboolean   in;     // whether the player is in game

  // Player stats, kills, collected items etc.
  int         skills;
  int         sitems;
  int         ssecret;
  int         stime;
  int         frags[4];
  int         score;  // current score on entry, modified on return

} wbplayerstruct_t;

typedef struct
{
  int         epsd;   // episode # (0-2)

  // if true, splash the secret level
  dboolean   didsecret;

  // previous and next levels, origin 0
  int         last;
  int         next;
  int         nextep;	// for when MAPINFO progression crosses into another episode.

  int         maxkills;
  int         maxitems;
  int         maxsecret;
  int         maxfrags;

  // the par time
  int         partime;
  int         fake_partime;
  dboolean    modified_partime;

  // index of this player in game
  int         pnum;

  wbplayerstruct_t    plyr[MAX_MAXPLAYERS];

  // CPhipps - total game time for completed levels so far
  int         totaltimes;

} wbstartstruct_t;

angle_t P_PlayerPitch(player_t* player);
fixed_t P_PlayerSpeed(player_t* player);

#endif
