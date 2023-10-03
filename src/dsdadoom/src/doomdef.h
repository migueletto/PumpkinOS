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
 *  Internally used data structures for virtually everything,
 *   key definitions, lots of other stuff.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __DOOMDEF__
#define __DOOMDEF__

/* use config.h if autoconf made one -- josh */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// killough 4/25/98: Make gcc extensions mean nothing on other compilers
#if !defined(__GNUC__) && !defined(__clang__)
#define __attribute__(x)
#endif

// This must come first, since it redefines malloc(), free(), etc. -- killough:
#include "z_zone.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "m_swap.h"
#include "doomtype.h"

extern dboolean bfgedition;

// Game mode handling - identify IWAD version
//  to handle IWAD dependend animations etc.
typedef enum {
  shareware,    // DOOM 1 shareware, E1, M9
  registered,   // DOOM 1 registered, E3, M27
  commercial,   // DOOM 2 retail, E1 M34  (DOOM 2 german edition not handled)
  retail,       // DOOM 1 retail, E4, M36
  indetermined  // Well, no IWAD found.
} GameMode_t;

// Mission packs - might be useful for TC stuff?
typedef enum {
  doom,         // DOOM 1
  doom2,        // DOOM 2
  pack_tnt,     // TNT mission pack
  pack_plut,    // Plutonia pack
  pack_nerve,   // No Rest For The Living
  hacx,         // HACX - Twitch 'n Kill
  chex,         // Chex Quest
  none
} GameMission_t;

// Identify language to use, software localization.
typedef enum {
  english,
  french,
  german,
  unknown
} Language_t;

//
// For resize of screen, at start of game.
//

#define BASE_WIDTH 320

// It is educational but futile to change this
//  scaling e.g. to 2. Drawing of status bar,
//  menues etc. is tied to the scale implied
//  by the graphics.

#define INV_ASPECT_RATIO   0.625 /* 0.75, ideally */

// killough 2/8/98: MAX versions for maximum screen sizes
// allows us to avoid the overhead of dynamic allocation
// when multiple screen sizes are supported

// SCREENWIDTH and SCREENHEIGHT define the visible size
extern int SCREENWIDTH;
extern int SCREENHEIGHT;
// SCREENPITCH is the size of one line in the buffer and
// can be bigger than the SCREENWIDTH depending on the size
// of one pixel (8, 16 or 32 bit) and the padding at the
// end of the line caused by hardware considerations
extern int SCREENPITCH;

// e6y: wide-res
extern int WIDE_SCREENWIDTH;
extern int WIDE_SCREENHEIGHT;
extern int SCREEN_320x200;

// The maximum number of players, multiplayer/networking.
#define MAX_MAXPLAYERS   8

// killough 2/28/98: A ridiculously large number
// of players, the most you'll ever need in a demo
// or savegame. This is used to prevent problems, in
// case more players in a game are supported later.
#define FUTURE_MAXPLAYERS 32

// phares 5/14/98:
// DOOM Editor Numbers (aka doomednum in mobj_t)

#define DEN_PLAYER5 4001
#define DEN_PLAYER6 4002
#define DEN_PLAYER7 4003
#define DEN_PLAYER8 4004

// State updates, number of tics / second.
#define TICRATE          35

// The current state of the game: whether we are playing, gazing
// at the intermission screen, the game final animation, or a demo.

typedef enum {
  GS_LEVEL,
  GS_INTERMISSION,
  GS_FINALE,
  GS_DEMOSCREEN
} gamestate_t;

//
// Difficulty/skill settings/filters.
//
// These are Thing flags

// Skill flags.
#define MTF_EASY        0x0001
#define MTF_NORMAL      0x0002
#define MTF_HARD        0x0004
// Deaf monsters/do not react to sound.
#define MTF_AMBUSH      0x0008

/* killough 11/98 */
#define MTF_NOTSINGLE   0x0010
#define MTF_NOTDM       0x0020
#define MTF_NOTCOOP     0x0040
#define MTF_FRIEND      0x0080
#define MTF_RESERVED    0x0100

// hexen
#define MTF_DORMANT     0x0010
#define MTF_FIGHTER     0x0020
#define MTF_CLERIC      0x0040
#define MTF_MAGE        0x0080
#define MTF_GSINGLE     0x0100
#define MTF_GCOOP       0x0200
#define MTF_GDEATHMATCH 0x0400

// zdoom
#define MTF_TRANSLUCENT 0x0800
#define MTF_INVISIBLE   0x1000
#define MTF_FRIENDLY    0x2000
#define MTF_STANDSTILL  0x4000
#define MTF_COUNTSECRET 0x8000
#define MTF_SKILL1  0x00010000
#define MTF_SKILL2  0x00020000
#define MTF_SKILL3  0x00040000
#define MTF_SKILL4  0x00080000
#define MTF_SKILL5  0x00100000

//
// Key cards.
//

typedef enum {
  it_bluecard,
  it_yellowcard,
  it_redcard,
  it_blueskull,
  it_yellowskull,
  it_redskull,
  DOOM_NUMCARDS,

  // heretic
  key_blue = 0,
  key_yellow,
  key_green,

  // hexen
  key_1 = 0,
  key_2,
  key_3,
  key_4,
  key_5,
  key_6,
  key_7,
  key_8,
  key_9,
  key_a,
  key_b,
  NUMCARDS
} card_t;

// The defined weapons, including a marker
// indicating user has not changed weapon.
typedef enum {
  wp_fist,
  wp_pistol,
  wp_shotgun,
  wp_chaingun,
  wp_missile,
  wp_plasma,
  wp_bfg,
  wp_chainsaw,
  wp_supershotgun,

  // heretic
  wp_staff = 0,
  wp_goldwand,
  wp_crossbow,
  wp_blaster,
  wp_skullrod,
  wp_phoenixrod,
  wp_mace,
  wp_gauntlets,
  wp_beak,

  NUMWEAPONS,
  wp_nochange,             // No pending weapon change.

  // hexen
  wp_first = 0,
  wp_second,
  wp_third,
  wp_fourth,
  HEXEN_NUMWEAPONS
} weapontype_t;

// Ammunition types defined.
typedef enum {
  am_clip,    // Pistol / chaingun ammo.
  am_shell,   // Shotgun / double barreled shotgun.
  am_cell,    // Plasma rifle, BFG.
  am_misl,    // Missile launcher.
  DOOM_NUMAMMO,

  // heretic
  am_goldwand = 0,
  am_crossbow,
  am_blaster,
  am_skullrod,
  am_phoenixrod,
  am_mace,
  HERETIC_NUMAMMO,

  NUMAMMO = HERETIC_NUMAMMO,
  am_noammo,   // fist, chainsaw, staff, gauntlets

  // hexen
  MANA_1 = 0,
  MANA_2,
  NUMMANA,
  MANA_BOTH,
  MANA_NONE = am_noammo
} ammotype_t;

// Power up artifacts.
typedef enum {
  pw_invulnerability,
  pw_strength,
  pw_invisibility,
  pw_ironfeet,
  pw_allmap,
  pw_infrared,

  // heretic
  pw_weaponlevel2,
  pw_flight,
  pw_shield,
  pw_health2,

  // hexen
  pw_speed,
  pw_minotaur,

  NUMPOWERS
} powertype_t;

// Power up durations (how many seconds till expiration).
typedef enum {
  INVULNTICS   = (30*TICRATE),
  INVISTICS    = (60*TICRATE),
  INFRATICS    = (120*TICRATE),
  IRONTICS     = (60*TICRATE),
  WPNLEV2TICS  = (40*TICRATE),
  FLIGHTTICS   = (60*TICRATE),
  SPEEDTICS    = (45*TICRATE),
  MORPHTICS    = (40*TICRATE),
  MAULATORTICS = (25*TICRATE)
} powerduration_t;

// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).

#define KEYD_RIGHTARROW 0xae
#define KEYD_LEFTARROW  0xac
#define KEYD_UPARROW    0xad
#define KEYD_DOWNARROW  0xaf
#define KEYD_ESCAPE     27
#define KEYD_ENTER      13
#define KEYD_TAB        9
#define KEYD_F1         (0x80+0x3b)
#define KEYD_F2         (0x80+0x3c)
#define KEYD_F3         (0x80+0x3d)
#define KEYD_F4         (0x80+0x3e)
#define KEYD_F5         (0x80+0x3f)
#define KEYD_F6         (0x80+0x40)
#define KEYD_F7         (0x80+0x41)
#define KEYD_F8         (0x80+0x42)
#define KEYD_F9         (0x80+0x43)
#define KEYD_F10        (0x80+0x44)
#define KEYD_F11        (0x80+0x57)
#define KEYD_F12        (0x80+0x58)
#define KEYD_BACKSPACE  127
#define KEYD_PAUSE      0xff
#define KEYD_EQUALS     0x3d
#define KEYD_MINUS      0x2d
#define KEYD_RSHIFT     (0x80+0x36)
#define KEYD_RCTRL      (0x80+0x1d)
#define KEYD_RALT       (0x80+0x38)
#define KEYD_LALT       KEYD_RALT
#define KEYD_CAPSLOCK   0xba                                        // phares
#define KEYD_PRINTSC    0xfe

// phares 3/2/98:
#define KEYD_INSERT     0xd2
#define KEYD_HOME       0xc7
#define KEYD_PAGEUP     0xc9
#define KEYD_PAGEDOWN   0xd1
#define KEYD_DEL        0xc8
#define KEYD_END        0xcf
#define KEYD_SCROLLLOCK 0xc6
#define KEYD_SPACEBAR   0x20
// phares 3/2/98

#define KEYD_NUMLOCK    0xC5                 // killough 3/6/98

// cph - Add the numeric keypad keys, as suggested by krose 4/22/99:
// The way numbers are assigned to keys is a mess, but it's too late to
// change that easily. At least these additions are don neatly.
// Codes 0x100-0x200 are reserved for number pad

#define KEYD_KEYPAD0      (0x100 + '0')
#define KEYD_KEYPAD1      (0x100 + '1')
#define KEYD_KEYPAD2      (0x100 + '2')
#define KEYD_KEYPAD3      (0x100 + '3')
#define KEYD_KEYPAD4      (0x100 + '4')
#define KEYD_KEYPAD5      (0x100 + '5')
#define KEYD_KEYPAD6      (0x100 + '6')
#define KEYD_KEYPAD7      (0x100 + '7')
#define KEYD_KEYPAD8      (0x100 + '8')
#define KEYD_KEYPAD9      (0x100 + '9')
#define KEYD_KEYPADENTER  (0x100 + KEYD_ENTER)
#define KEYD_KEYPADDIVIDE (0x100 + '/')
#define KEYD_KEYPADMULTIPLY (0x100 + '*')
#define KEYD_KEYPADMINUS  (0x100 + '-')
#define KEYD_KEYPADPLUS   (0x100 + '+')
#define KEYD_KEYPADPERIOD (0x100 + '.')

// haleyjd: virtual keys
#define KEYD_MOUSE1     (0x80 + 0x60)
#define KEYD_MOUSE2     (0x80 + 0x61)
#define KEYD_MOUSE3     (0x80 + 0x62)
#define KEYD_MWHEELUP   (0x80 + 0x6b)
#define KEYD_MWHEELDOWN (0x80 + 0x6c)

// phares 3/20/98:
//
// Player friction is variable, based on controlling
// linedefs. More friction can create mud, sludge,
// magnetized floors, etc. Less friction can create ice.

#define MORE_FRICTION_MOMENTUM 15000       // mud factor based on momentum
#define ORIG_FRICTION          0xE800      // original value
#define ORIG_FRICTION_FACTOR   2048        // original value
#define FRICTION_FLY           0xeb00

extern dboolean raven;

// heretic

#define FRICTION_LOW 0xf900
#define TELEFOGHEIGHT (32*FRACUNIT)
#define ANG1_X          0x01000000

#define FOOTCLIPSIZE 10*FRACUNIT

// Any floor type >= FLOOR_LIQUID will floorclip sprites (hexen)
typedef enum {
  FLOOR_SOLID,
  FLOOR_ICE,
  FLOOR_LIQUID,
  FLOOR_WATER,
  FLOOR_LAVA,
  FLOOR_SLUDGE
} floortype_t;

#define USE_GWND_AMMO_1 1
#define USE_GWND_AMMO_2 1
#define USE_CBOW_AMMO_1 1
#define USE_CBOW_AMMO_2 1
#define USE_BLSR_AMMO_1 1
#define USE_BLSR_AMMO_2 5
#define USE_SKRD_AMMO_1 1
#define USE_SKRD_AMMO_2 5
#define USE_PHRD_AMMO_1 1
#define USE_PHRD_AMMO_2 1
#define USE_MACE_AMMO_1 1
#define USE_MACE_AMMO_2 5

#define TOCENTER -8

#define BLINKTHRESHOLD (4*32)

// TODO_HEXEN: BLINKTHRESHOLD is (4*35)

extern dboolean heretic;

//hexen

// The top 3 bits of the artifact field in the ticcmd_t struct are used
//              as additional flags
#define AFLAG_MASK    0x3F
#define AFLAG_SUICIDE 0x40
#define AFLAG_JUMP    0x80

typedef enum
{
  ARMOR_ARMOR,
  ARMOR_SHIELD,
  ARMOR_HELMET,
  ARMOR_AMULET,
  NUMARMOR
} armortype_t;

typedef enum
{
  PCLASS_NULL,
  PCLASS_FIGHTER,
  PCLASS_CLERIC,
  PCLASS_MAGE,
  PCLASS_PIG,
  NUMCLASSES
} pclass_t;

typedef ammotype_t manatype_t;

#define MAX_MANA 200

#define WPIECE1 1
#define WPIECE2 2
#define WPIECE3 4

enum
{
  SEQ_PLATFORM,
  SEQ_PLATFORM_HEAVY,         // same script as a normal platform
  SEQ_PLATFORM_METAL,
  SEQ_PLATFORM_CREAK,         // same script as a normal platform
  SEQ_PLATFORM_SILENCE,
  SEQ_PLATFORM_LAVA,
  SEQ_PLATFORM_WATER,
  SEQ_PLATFORM_ICE,
  SEQ_PLATFORM_EARTH,
  SEQ_PLATFORM_METAL2,
  SEQ_DOOR_STONE,
  SEQ_DOOR_HEAVY,
  SEQ_DOOR_METAL,
  SEQ_DOOR_CREAK,
  SEQ_DOOR_SILENCE,
  SEQ_DOOR_LAVA,
  SEQ_DOOR_WATER,
  SEQ_DOOR_ICE,
  SEQ_DOOR_EARTH,
  SEQ_DOOR_METAL2,
  SEQ_ESOUND_WIND,
  SEQ_NUMSEQ
};

typedef enum
{
  SEQTYPE_STONE,
  SEQTYPE_HEAVY,
  SEQTYPE_METAL,
  SEQTYPE_CREAK,
  SEQTYPE_SILENCE,
  SEQTYPE_LAVA,
  SEQTYPE_WATER,
  SEQTYPE_ICE,
  SEQTYPE_EARTH,
  SEQTYPE_METAL2,
  SEQTYPE_NUMSEQ
} seqtype_t;

#define MAX_INTRMSN_MESSAGE_SIZE 1024

// Puzzle artifacts

#define TXT_ARTIPUZZSKULL      "YORICK'S SKULL"
#define TXT_ARTIPUZZGEMBIG     "HEART OF D'SPARIL"
#define TXT_ARTIPUZZGEMRED     "RUBY PLANET"
#define TXT_ARTIPUZZGEMGREEN1  "EMERALD PLANET"
#define TXT_ARTIPUZZGEMGREEN2  "EMERALD PLANET"
#define TXT_ARTIPUZZGEMBLUE1   "SAPPHIRE PLANET"
#define TXT_ARTIPUZZGEMBLUE2   "SAPPHIRE PLANET"
#define TXT_ARTIPUZZBOOK1      "DAEMON CODEX"
#define TXT_ARTIPUZZBOOK2      "LIBER OSCURA"
#define TXT_ARTIPUZZSKULL2     "FLAME MASK"
#define TXT_ARTIPUZZFWEAPON    "GLAIVE SEAL"
#define TXT_ARTIPUZZCWEAPON    "HOLY RELIC"
#define TXT_ARTIPUZZMWEAPON    "SIGIL OF THE MAGUS"
#define TXT_ARTIPUZZGEAR       "CLOCK GEAR"
#define TXT_USEPUZZLEFAILED    "YOU CANNOT USE THIS HERE"

extern dboolean hexen;

#endif          // __DOOMDEF__
