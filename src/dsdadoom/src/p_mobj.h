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
 *      Map Objects, MObj, definition and handling.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __P_MOBJ__
#define __P_MOBJ__

// Basics.
#include "tables.h"
#include "m_fixed.h"

// We need the thinker_t stuff.
#include "d_think.h"

// We need the WAD data structure for Map things,
// from the THINGS lump.
#include "doomdata.h"

// States are tied to finite states are
//  tied to animation frames.
// Needs precompiled tables/data structures.
#include "info.h"

//
// NOTES: mobj_t
//
// mobj_ts are used to tell the refresh where to draw an image,
// tell the world simulation when objects are contacted,
// and tell the sound driver how to position a sound.
//
// The refresh uses the next and prev links to follow
// lists of things in sectors as they are being drawn.
// The sprite, frame, and angle elements determine which patch_t
// is used to draw the sprite if it is visible.
// The sprite and frame values are allmost allways set
// from state_t structures.
// The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the
// statescr.txt source file.
// The xyz origin point represents a point at the bottom middle
// of the sprite (between the feet of a biped).
// This is the default origin position for patch_ts grabbed
// with lumpy.exe.
// A walking creature will have its z equal to the floor
// it is standing on.
//
// The sound code uses the x,y, and subsector fields
// to do stereo positioning of any sound effited by the mobj_t.
//
// The play simulation uses the blocklinks, x,y,z, radius, height
// to determine when mobj_ts are touching each other,
// touching lines in the map, or hit by trace lines (gunshots,
// lines of sight, etc).
// The mobj_t->flags element has various bit flags
// used by the simulation.
//
// Every mobj_t is linked into a single sector
// based on its origin coordinates.
// The subsector_t is found with R_PointInSubsector(x,y),
// and the sector_t can be found with subsector->sector.
// The sector links are only used by the rendering code,
// the play simulation does not care about them at all.
//
// Any mobj_t that needs to be acted upon by something else
// in the play world (block movement, be shot, etc) will also
// need to be linked into the blockmap.
// If the thing has the MF_NOBLOCK flag set, it will not use
// the block links. It can still interact with other things,
// but only as the instigator (missiles will run into other
// things, but nothing can run into a missile).
// Each block in the grid is 128*128 units, and knows about
// every line_t that it contains a piece of, and every
// interactable mobj_t that has its origin contained.
//
// A valid mobj_t is a mobj_t that has the proper subsector_t
// filled in for its xy coordinates and is linked into the
// sector from which the subsector was made, or has the
// MF_NOSECTOR flag set (the subsector_t needs to be valid
// even if MF_NOSECTOR is set), and is linked into a blockmap
// block or has the MF_NOBLOCKMAP flag set.
// Links should only be modified by the P_[Un]SetThingPosition()
// functions.
// Do not change the MF_NO? flags while a thing is valid.
//
// Any questions?
//

//
// Misc. mobj flags
//

// Call P_SpecialThing when touched.
#define MF_SPECIAL      0x0000000000000001ull
// Blocks.
#define MF_SOLID        0x0000000000000002ull
// Can be hit.
#define MF_SHOOTABLE    0x0000000000000004ull
// Don't use the sector links (invisible but touchable).
#define MF_NOSECTOR     0x0000000000000008ull
// Don't use the blocklinks (inert but displayable)
#define MF_NOBLOCKMAP   0x0000000000000010ull

// Not to be activated by sound, deaf monster.
#define MF_AMBUSH       0x0000000000000020ull
// Will try to attack right back.
#define MF_JUSTHIT      0x0000000000000040ull
// Will take at least one step before attacking.
#define MF_JUSTATTACKED 0x0000000000000080ull
// On level spawning (initial position),
//  hang from ceiling instead of stand on floor.
#define MF_SPAWNCEILING 0x0000000000000100ull
// Don't apply gravity (every tic),
//  that is, object will float, keeping current height
//  or changing it actively.
#define MF_NOGRAVITY    0x0000000000000200ull

// Movement flags.
// This allows jumps from high places.
#define MF_DROPOFF      0x0000000000000400ull
// For players, will pick up items.
#define MF_PICKUP       0x0000000000000800ull
// Player cheat. ???
#define MF_NOCLIP       0x0000000000001000ull
// Player: keep info about sliding along walls.
#define MF_SLIDE        0x0000000000002000ull
// Allow moves to any height, no gravity.
// For active floaters, e.g. cacodemons, pain elementals.
#define MF_FLOAT        0x0000000000004000ull
// Don't cross lines
//   ??? or look at heights on teleport.
#define MF_TELEPORT     0x0000000000008000ull
// Don't hit same species, explode on block.
// Player missiles as well as fireballs of various kinds.
#define MF_MISSILE      0x0000000000010000ull
// Dropped by a demon, not level spawned.
// E.g. ammo clips dropped by dying former humans.
#define MF_DROPPED      0x0000000000020000ull
// Use fuzzy draw (shadow demons or spectres),
//  temporary player invisibility powerup.
#define MF_SHADOW       0x0000000000040000ull
// Flag: don't bleed when shot (use puff),
//  barrels and shootable furniture shall not bleed.
#define MF_NOBLOOD      0x0000000000080000ull
// Don't stop moving halfway off a step,
//  that is, have dead bodies slide down all the way.
#define MF_CORPSE       0x0000000000100000ull
// Floating to a height for a move, ???
//  don't auto float to target's height.
#define MF_INFLOAT      0x0000000000200000ull

// On kill, count this enemy object
//  towards intermission kill total.
// Happy gathering.
#define MF_COUNTKILL    0x0000000000400000ull

// On picking up, count this item object
//  towards intermission item total.
#define MF_COUNTITEM    0x0000000000800000ull

// Special handling: skull in flight.
// Neither a cacodemon nor a missile.
#define MF_SKULLFLY     0x0000000001000000ull

// Don't spawn this object
//  in death match mode (e.g. key cards).
#define MF_NOTDMATCH    0x0000000002000000ull

// Player sprites in multiplayer modes are modified
//  using an internal color lookup table for re-indexing.
// If 0x4 0x8 or 0xc,
//  use a translation table for player colormaps
#define MF_TRANSLATION  (uint64_t)(0x000000000c000000)
#define MF_TRANSLATION1 0x0000000004000000ull
#define MF_TRANSLATION2 0x0000000008000000ull
// Hmm ???.
#define MF_TRANSSHIFT 26

#define MF_UNUSED2      0x0000000010000000ull
#define MF_UNUSED3      0x0000000020000000ull

    // Translucent sprite?                                          // phares
#define MF_TRANSLUCENT  0x0000000040000000ull

// this is free            0x0000000100000000ull

// these are greater than an int. That's why the flags below are now uint64_t

#define MF_TOUCHY          0x0000000100000000ull
#define MF_BOUNCES         0x0000000200000000ull
#define MF_FRIEND          0x0000000400000000ull

#define MF_RESSURECTED     0x0000001000000000ull
#define MF_NO_DEPTH_TEST   0x0000002000000000ull
#define MF_FOREGROUND      0x0000004000000000ull
#define MF_PLAYERSPRITE    0x0000008000000000ull

// This actor not targetted when it hurts something else
#define MF_NOTARGET        0x0000010000000000ull
// fly mode is active
#define MF_FLY             0x0000020000000000ull

// hexen
#define	MF_ALTSHADOW	0x0000040000000000ull // alternate translucent draw
#define	MF_ICECORPSE	0x0000080000000000ull // a frozen corpse (for blasting)

// hexen_note: MF_TRANSLATION covers doom's (MF_TRANSLATION | MF_UNUSED2)

#define ALIVE(thing) ((thing->health > 0) && ((thing->flags & (MF_COUNTKILL | MF_CORPSE | MF_RESSURECTED)) == MF_COUNTKILL))

// killough 9/15/98: Same, but internal flags, not intended for .deh
// (some degree of opaqueness is good, to avoid compatibility woes)

enum {
  MIF_FALLING = 1,      // Object is falling
  MIF_ARMED = 2,        // Object is armed (for MF_TOUCHY objects)
  MIF_SCROLLING = 4,    // Object is affected by scroller / pusher / puller
  MIF_PLAYER_DAMAGED_BARREL = 8,
  MIF_SPAWNED_BY_ICON = 16,
  MIF_FAKE = 32, // Not a real thing, transient (e.g., for cheats)
};

// heretic
typedef struct
{
    int i;
    struct mobj_s *m;
} specialval_t;

// Map Object definition.
//
//
// killough 2/20/98:
//
// WARNING: Special steps must be taken in p_saveg.c if C pointers are added to
// this mobj_s struct, or else savegames will crash when loaded. See p_saveg.c.
// Do not add "struct mobj_s *fooptr" without adding code to p_saveg.c to
// convert the pointers to ordinals and back for savegames. This was the whole
// reason behind monsters going to sleep when loading savegames (the "target"
// pointer was simply nullified after loading, to prevent Doom from crashing),
// and the whole reason behind loadgames crashing on savegames of AV attacks.
//

// killough 9/8/98: changed some fields to shorts,
// for better memory usage (if only for cache).
/* cph 2006/08/28 - move Prev[XYZ] fields to the end of the struct. Add any
 * other new fields to the end, and make sure you don't break savegames! */

typedef struct mobj_s
{
    // List: thinker links.
    thinker_t           thinker;

    // Info for drawing: position.
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    // More list: links in sector (if needed)
    struct mobj_s*      snext;
    struct mobj_s**     sprev; // killough 8/10/98: change to ptr-to-ptr

    //More drawing info: to determine current sprite.
    angle_t             angle;  // orientation
    spritenum_t         sprite; // used to find patch_t and flip value
    int                 frame;  // might be ORed with FF_FULLBRIGHT

    // Interaction info, by BLOCKMAP.
    // Links in blocks (if needed).
    struct mobj_s*      bnext;
    struct mobj_s**     bprev; // killough 8/11/98: change to ptr-to-ptr

    struct subsector_s* subsector;

    // The closest interval over all contacted Sectors.
    fixed_t             floorz;
    fixed_t             ceilingz;

    // killough 11/98: the lowest floor over all contacted Sectors.
    fixed_t             dropoffz;

    // For movement checking.
    fixed_t             radius;
    fixed_t             height;

    // Momentums, used to update position.
    fixed_t             momx;
    fixed_t             momy;
    fixed_t             momz;

    // If == validcount, already checked.
    int                 validcount;

    mobjtype_t          type;
    mobjinfo_t*         info;   // &mobjinfo[mobj->type]

    int                 tics;   // state tic counter
    state_t*            state;
    uint64_t            flags;
    int                 intflags;  // killough 9/15/98: internal flags
    int                 health;

    // Movement direction, movement generation (zig-zagging).
    short               movedir;        // 0-7
    short               movecount;      // when 0, select a new dir
    short               strafecount;    // killough 9/8/98: monster strafing

    // Thing being chased/attacked (or NULL),
    // also the originator for missiles.
    struct mobj_s*      target;

    // Reaction time: if non 0, don't attack yet.
    // Used by player to freeze a bit after teleporting.
    short               reactiontime;

    // If >0, the current target will be chased no
    // matter what (even if shot by another object)
    short               threshold;

    // killough 9/9/98: How long a monster pursues a target.
    short               pursuecount;

    short               gear; // killough 11/98: used in torque simulation

    // Additional info record for player avatars only.
    // Only valid if type == MT_PLAYER
    struct player_s*    player;

    // Player number last looked for.
    short               lastlook;

    // For nightmare respawn.
    mapthing_t          spawnpoint;

    // Thing being chased/attacked for tracers.
    struct mobj_s*      tracer;

    // new field: last known enemy -- killough 2/15/98
    struct mobj_s*      lastenemy;

    // killough 8/2/98: friction properties part of sectors,
    // not objects -- removed friction properties from here
    // e6y: restored friction properties here
    // Friction values for the sector the object is in
    int friction;                                           // phares 3/17/98
    int movefactor;

    // a linked list of sectors where this object appears
    struct msecnode_s* touching_sectorlist;                 // phares 3/14/98

    fixed_t             PrevX;
    fixed_t             PrevY;
    fixed_t             PrevZ;

    //e6y
    angle_t             pitch;  // orientation
    int index;
    short patch_width;

    int iden_nums;		// hi word stores thing num, low word identifier num

    // heretic
    int damage;                 // For missiles
    uint64_t flags2;           // Heretic & MBF21 flags
    specialval_t special1;      // Special info
    specialval_t special2;      // Special info

    // hexen
    fixed_t floorpic;           // contacted sec floorpic
    fixed_t floorclip;          // value to use for floor clipping
    int archiveNum;             // Identity during archive
    short tid;                  // thing identifier
    int special;                // special
    int special_args[5];        // special arguments

    // zdoom
    fixed_t gravity;
    float alpha;

    // misc
    byte color;
    const byte* tranmap;

    // SEE WARNING ABOVE ABOUT POINTER FIELDS!!!
} mobj_t;

// External declarations (fomerly in p_local.h) -- killough 5/2/98

#define MAXMOVE         (30*FRACUNIT)

#define ONFLOORZ        INT_MIN
#define ONCEILINGZ      INT_MAX
#define FLOATRANDZ     (INT_MAX-1)

// Time interval for item respawning.
#define ITEMQUESIZE     128

#define FLOATSPEED      (FRACUNIT*4)
#define STOPSPEED       (FRACUNIT/16)

// killough 11/98:
// For torque simulation:

#define OVERDRIVE 6
#define MAXGEAR (OVERDRIVE+16)

// killough 11/98:
// Whether an object is "sentient" or not. Used for environmental influences.
#define sentient(mobj) ((mobj)->health > 0 && (mobj)->info->seestate)

extern int iquehead;
extern int iquetail;

int P_MobjSpawnHealth(const mobj_t* mobj);
mobj_t* P_SubstNullMobj (mobj_t* th);
void    P_RespawnSpecials(void);
mobj_t  *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);
void    P_RemoveMobj(mobj_t *th);
dboolean P_SetMobjState(mobj_t *mobj, statenum_t state);
void    P_MobjThinker(mobj_t *mobj);
void    P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z);
void    P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage, mobj_t *bleeder);
mobj_t  *P_SpawnMissile(mobj_t *source, mobj_t *dest, mobjtype_t type);
mobj_t  *P_SpawnPlayerMissile(mobj_t *source, mobjtype_t type);
dboolean P_IsDoomnumAllowed(int doomnum);
mobj_t* P_SpawnMapThing (const mapthing_t*  mthing, int index);
void    P_SpawnPlayer(int n, const mapthing_t *mthing);
dboolean P_CheckMissileSpawn(mobj_t*);  // killough 8/2/98
void    P_ExplodeMissile(mobj_t*);    // killough

void P_RemoveMonsters(void);

// heretic

// --- mobj.flags2 ---

#define MF2_LOGRAV         0x00000001ull  // alternate gravity setting
#define MF2_WINDTHRUST     0x00000002ull  // gets pushed around by the wind
#define MF2_FLOORBOUNCE    0x00000004ull  // bounces off the floor
#define MF2_THRUGHOST      0x00000008ull  // missile will pass through ghosts
#define MF2_FLY            0x00000010ull  // fly mode is active
#define MF2_FOOTCLIP       0x00000020ull  // if feet are allowed to be clipped
#define MF2_SPAWNFLOAT     0x00000040ull  // spawn random float z
#define MF2_NOTELEPORT     0x00000080ull  // does not teleport
#define MF2_RIP            0x00000100ull  // missile rips through solid
#define MF2_PUSHABLE       0x00000200ull  // can be pushed by other moving
#define MF2_SLIDE          0x00000400ull  // slides against walls
#define MF2_ONMOBJ         0x00000800ull  // mobj is resting on top of another
#define MF2_PASSMOBJ       0x00001000ull  // Enable z block checking (pass over / under)
#define MF2_CANNOTPUSH     0x00002000ull  // cannot push other pushable mobjs
#define MF2_FEETARECLIPPED 0x00004000ull  // a mobj's feet are now being cut
#define MF2_BOSS           0x00008000ull  // mobj is a major boss
#define MF2_FIREDAMAGE     0x00010000ull  // does fire damage
#define MF2_NODMGTHRUST    0x00020000ull  // does not thrust target when
#define MF2_TELESTOMP      0x00040000ull  // mobj can stomp another
#define MF2_FLOATBOB       0x00080000ull  // use float bobbing z movement
#define MF2_DONTDRAW       0x00100000ull  // don't generate a vissprite

// mbf21
#define MF2_SHORTMRANGE    0x00200000ull // has short missile range (archvile)
#define MF2_DMGIGNORED     0x00400000ull // other things ignore its attacks (archvile)
#define MF2_NORADIUSDMG    0x00800000ull // Doesn't take damage from blast radii
#define MF2_FORCERADIUSDMG 0x01000000ull // Does radius damage to everything, no exceptions
#define MF2_HIGHERMPROB    0x02000000ull // min prob. of miss. att. = 37.5% vs 22%
#define MF2_RANGEHALF      0x04000000ull // use half actual distance for missile attack probability
#define MF2_NOTHRESHOLD    0x08000000ull // has no target threshold
#define MF2_MAP07BOSS1     0x10000000ull // is a MAP07 boss type 1 (666)
#define MF2_MAP07BOSS2     0x20000000ull // is a MAP07 boss type 2 (667)
#define MF2_E1M8BOSS       0x40000000ull // is an E1M8 boss
#define MF2_E2M8BOSS       0x80000000ull // is an E2M8 boss
#define MF2_E3M8BOSS  0x0000000100000000ull // is an E3M8 boss
#define MF2_E4M6BOSS  0x0000000200000000ull // is an E4M6 boss
#define MF2_E4M8BOSS  0x0000000400000000ull // is an E4M8 boss
#define MF2_LONGMELEE 0x0000000800000000ull // has long melee range (revenant)
#define MF2_FULLVOLSOUNDS 0x0000001000000000ull // full volume see / death sound

// hexen
#define MF2_BLASTED	          0x0000002000000000ull // missile will pass through ghosts
#define MF2_IMPACT            0x0000004000000000ull // an MF_MISSILE mobj can activate SPAC_IMPACT
#define MF2_PUSHWALL          0x0000008000000000ull // mobj can push walls
#define MF2_MCROSS            0x0000010000000000ull // can activate monster cross lines
#define MF2_PCROSS            0x0000020000000000ull // can activate projectile cross lines
#define MF2_CANTLEAVEFLOORPIC 0x0000040000000000ull // stay within a certain floor type
#define MF2_NONSHOOTABLE      0x0000080000000000ull // mobj is totally non-shootable, but still considered solid
#define MF2_INVULNERABLE      0x0000100000000000ull // mobj is invulnerable
#define MF2_DORMANT           0x0000200000000000ull // thing is dormant
#define MF2_ICEDAMAGE         0x0000400000000000ull // does ice damage
#define MF2_SEEKERMISSILE     0x0000800000000000ull // is a seeker (for reflection)
#define MF2_REFLECTIVE        0x0001000000000000ull // reflects missiles

// zdoom
#define MF2_CANUSEWALLS       0x0002000000000000ull // can activate use lines
#define MF2_COUNTSECRET       0x0004000000000000ull // picking up counts as a secret

#define AMMO_GWND_WIMPY 10
#define AMMO_GWND_HEFTY 50
#define AMMO_CBOW_WIMPY 5
#define AMMO_CBOW_HEFTY 20
#define AMMO_BLSR_WIMPY 10
#define AMMO_BLSR_HEFTY 25
#define AMMO_SKRD_WIMPY 20
#define AMMO_SKRD_HEFTY 100
#define AMMO_PHRD_WIMPY 1
#define AMMO_PHRD_HEFTY 10
#define AMMO_MACE_WIMPY 20
#define AMMO_MACE_HEFTY 100

extern mobj_t* MissileMobj;

void P_BlasterMobjThinker(mobj_t * mobj);
mobj_t *P_SpawnMissileAngle(mobj_t * source, mobjtype_t type, angle_t angle, fixed_t momz);
dboolean P_SetMobjStateNF(mobj_t * mobj, statenum_t state);
void P_ThrustMobj(mobj_t * mo, angle_t angle, fixed_t move);
dboolean P_SeekerMissile(mobj_t * actor, mobj_t ** seekTarget, angle_t thresh, angle_t turnMax, dboolean seekcenter);
mobj_t *P_SPMAngle(mobj_t * source, mobjtype_t type, angle_t angle);
int P_HitFloor(mobj_t * thing);
int P_GetThingFloorType(mobj_t * thing);
int P_FaceMobj(mobj_t * source, mobj_t * target, angle_t * delta);
void P_BloodSplatter(fixed_t x, fixed_t y, fixed_t z, mobj_t * originator);
void P_RipperBlood(mobj_t * mo, mobj_t * bleeder);
dboolean Raven_P_SetMobjState(mobj_t * mobj, statenum_t state);
void P_FloorBounceMissile(mobj_t * mo);
void Raven_P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z);

// hexen

mobj_t *P_SpawnMissileXYZ(fixed_t x, fixed_t y, fixed_t z,
                          mobj_t * source, mobj_t * dest, mobjtype_t type);
mobj_t *P_SpawnMissileAngleSpeed(mobj_t * source, mobjtype_t type,
                                 angle_t angle, fixed_t momz, fixed_t speed);
mobj_t *P_SPMAngleXYZ(mobj_t * source, fixed_t x, fixed_t y,
                      fixed_t z, mobjtype_t type, angle_t angle);
mobj_t *P_SpawnKoraxMissile(fixed_t x, fixed_t y, fixed_t z,
                            mobj_t * source, mobj_t * dest, mobjtype_t type);
mobj_t *P_FindMobjFromTID(short tid, int *searchPosition);
void P_BloodSplatter2(fixed_t x, fixed_t y, fixed_t z, mobj_t * originator);

// zdoom

fixed_t P_MobjGravity(mobj_t* mo);
dboolean P_SpawnThing(short thing_id, mobj_t *source, int type,
                      angle_t angle, dboolean fog, short new_thing_id);
dboolean P_SpawnProjectile(short thing_id, mobj_t *source, int spawn_num, angle_t angle,
	                         fixed_t speed, fixed_t vspeed, short dest_id, mobj_t *forcedest,
                           int gravity, short new_thing_id);

#endif
