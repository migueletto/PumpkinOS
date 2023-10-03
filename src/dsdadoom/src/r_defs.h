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
 *      Refresh/rendering module, shared data struct definitions.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __R_DEFS__
#define __R_DEFS__

// Screenwidth.
#include "doomdef.h"

// Some more or less basic data types
// we depend on.
#include "m_fixed.h"

// We rely on the thinker data struct
// to handle sound origins in sectors.
#include "d_think.h"

// SECTORS do store MObjs anyway.
#include "p_mobj.h"

// Silhouette, needed for clipping Segs (mainly)
// and sprites representing things.
#define SIL_NONE    0
#define SIL_BOTTOM  1
#define SIL_TOP     2
#define SIL_BOTH    3

#define MAXDRAWSEGS   256

//
// INTERNAL MAP TYPES
//  used by play and refresh
//

//
// Your plain vanilla vertex.
// Note: transformed values not buffered locally,
// like some DOOM-alikes ("wt", "WebView") do.
//
typedef struct
{
  fixed_t x, y;
  // [crispy] remove slime trails
  // pseudovertexes are dummies that have their coordinates modified to get
  // moved towards the linedef associated with their seg by projecting them
  // using the law of cosines in p_setup.c:P_RemoveSlimeTrails();
  // they are *only* used in rendering
  fixed_t px;
  fixed_t py;
} vertex_t;

// Each sector has a degenmobj_t in its center for sound origin purposes.
typedef struct
{
  thinker_t thinker;  // not used for anything
  fixed_t x, y, z;
} degenmobj_t;

//
// The SECTORS record, at runtime.
// Stores things/mobjs.
//

#define NO_TOPTEXTURES             0x00000001
#define NO_BOTTOMTEXTURES          0x00000002
#define SECTOR_IS_CLOSED           0x00000004
#define NULL_SECTOR                0x00000008
#define MISSING_TOPTEXTURES        0x00000010
#define MISSING_BOTTOMTEXTURES     0x00000020
#define SECF_SECRET                0x00000040
#define SECF_WASSECRET             0x00000080
#define SECF_HIDDEN                0x00000100
#define SECF_ENDGODMODE            0x00000200
#define SECF_ENDLEVEL              0x00000400
#define SECF_DMGTERRAINFX          0x00000800
#define SECF_HAZARD                0x00001000
#define SECF_DMGUNBLOCKABLE        0x00002000
#define SECF_FRICTION              0x00004000
#define SECF_PUSH                  0x00008000
#define SECF_NOATTACK              0x00010000
#define SECF_SILENT                0x00020000
#define SECF_LIGHTFLOORABSOLUTE    0x00040000
#define SECF_LIGHTCEILINGABSOLUTE  0x00080000
#define SECF_DAMAGEFLAGS (SECF_ENDGODMODE|SECF_ENDLEVEL|SECF_DMGTERRAINFX|SECF_HAZARD|SECF_DMGUNBLOCKABLE)
#define SECF_TRANSFERMASK (SECF_SECRET|SECF_WASSECRET|SECF_DAMAGEFLAGS|SECF_FRICTION|SECF_PUSH)

typedef struct
{
  short amount;
  byte leakrate;
  byte interval;
} damage_t;

typedef struct sector_s
{
  int iSectorID; // proff 04/05/2000: needed for OpenGL and used in debugmode by the HUD to draw sectornum
  unsigned int flags;    //e6y: instead of .no_toptextures and .no_bottomtextures
  fixed_t floorheight;
  fixed_t ceilingheight;
  byte soundtraversed;   // 0 = untraversed, 1,2 = sndlines-1
  mobj_t *soundtarget;   // thing that made a sound (or null)
  int blockbox[4];       // mapblock bounding box for height changes
  int bbox[4];           // bounding box in map units
  degenmobj_t soundorg;  // origin for any sounds played by the sector
  int validcount;        // if == validcount, already checked
  // Needed by GL path to register flats in sector for rendering only once
  int gl_validcount;
  mobj_t *thinglist;     // list of mobjs in sector

  /* killough 8/28/98: friction is a sector property, not an mobj property.
   * these fields used to be in mobj_t, but presented performance problems
   * when processed as mobj properties. Fix is to make them sector properties.
   */
  int friction,movefactor;

  // thinker_t for reversable actions
  void *floordata;    // jff 2/22/98 make thinkers on
  void *ceilingdata;  // floors, ceilings, and lights independent
  void *lightingdata;

  // jff 2/26/98 lockout machinery for stairbuilding
  signed char stairlock; // -2 on first locked -1 after thinker done 0 normally
  int prevsec;     // -1 or number of sector for previous step
  int nextsec;     // -1 or number of next step sector

  // killough 3/7/98: support flat heights drawn at another sector's heights
  int heightsec;    // other sector, or -1 if no other sector

  int bottommap, midmap, topmap; // killough 4/4/98: dynamic colormaps

  // list of mobjs that are at least partially in the sector
  // thinglist is a subset of touching_thinglist
  struct msecnode_s *touching_thinglist;               // phares 3/14/98

  int linecount;
  struct line_s **lines;

  // killough 10/98: support skies coming from sidedefs. Allows scrolling
  // skies and other effects. No "level info" kind of lump is needed,
  // because you can use an arbitrary number of skies per level with this
  // method. This field only applies when skyflatnum is used for floorpic
  // or ceilingpic, because the rest of Doom needs to know which is sky
  // and which isn't, etc.

  int sky;

  // killough 3/7/98: floor and ceiling texture offsets
  fixed_t   floor_xoffs,   floor_yoffs;
  fixed_t ceiling_xoffs, ceiling_yoffs;

  // killough 4/11/98: support for lightlevels coming from another sector
  int floorlightsec, ceilinglightsec;

  short floorpic;
  short ceilingpic;
  short lightlevel;
  short special;
  short tag;

  // [kb] For R_FixWiggle
  int cachedheight;
  int scaleindex;

  //e6y
  int INTERP_SectorFloor;
  int INTERP_SectorCeiling;
  int INTERP_FloorPanning;
  int INTERP_CeilingPanning;
  int fakegroup[2];

  // hexen
  seqtype_t seqType;          // stone, metal, heavy, etc...

  // zdoom
  fixed_t gravity;
  damage_t damage;
  short lightlevel_floor;
  short lightlevel_ceiling;
  angle_t floor_rotation;
  angle_t ceiling_rotation;
  fixed_t floor_xscale;
  fixed_t floor_yscale;
  fixed_t ceiling_xscale;
  fixed_t ceiling_yscale;
} sector_t;

//
// The SideDef.
//

#define SF_LIGHTABSOLUTE       0x0001
// #define SF_LIGHTFOG            0x0002
#define SF_NOFAKECONTRAST      0x0004
#define SF_SMOOTHLIGHTING      0x0008
#define SF_CLIPMIDTEX          0x0010
#define SF_WRAPMIDTEX          0x0020
// #define SF_NODECALS            0x0040
#define SF_LIGHTABSOLUTETOP    0x0080
#define SF_LIGHTABSOLUTEMID    0x0100
#define SF_LIGHTABSOLUTEBOTTOM 0x0200

typedef struct
{
  fixed_t textureoffset; // add this to the calculated texture column
  fixed_t rowoffset;     // add this to the calculated texture top
  short toptexture;      // Texture indices. We do not maintain names here.
  short bottomtexture;
  short midtexture;
  sector_t* sector;      // Sector the SideDef is facing.

  // killough 4/4/98, 4/11/98: highest referencing special linedef's type,
  // or lump number of special effect. Allows texture names to be overloaded
  // for other functions.

  int special;

  int INTERP_WallPanning;
  int skybox_index;

  fixed_t textureoffset_top;
  fixed_t textureoffset_mid;
  fixed_t textureoffset_bottom;
  fixed_t rowoffset_top;
  fixed_t rowoffset_mid;
  fixed_t rowoffset_bottom;

  fixed_t scalex_top;
  fixed_t scaley_top;
  fixed_t scalex_mid;
  fixed_t scaley_mid;
  fixed_t scalex_bottom;
  fixed_t scaley_bottom;

  int lightlevel;
  int lightlevel_top;
  int lightlevel_mid;
  int lightlevel_bottom;
  unsigned short flags;
} side_t;

//
// Move clipping aid for LineDefs.
//
typedef enum
{
  ST_HORIZONTAL,
  ST_VERTICAL,
  ST_POSITIVE,
  ST_NEGATIVE
} slopetype_t;

typedef byte r_flags_t;
#define RF_TOP_TILE 0x01 // Upper texture needs tiling
#define RF_MID_TILE 0x02 // Mid texture needs tiling
#define RF_BOT_TILE 0x04 // Lower texture needs tiling
#define RF_IGNORE   0x08 // Renderer can skip this line
#define RF_CLOSED   0x10 // Line blocks view
#define RF_ISOLATED 0x20 // Isolated line

typedef enum
{
  ams_default,
  ams_one_sided,
  ams_two_sided,
  ams_floor_diff,
  ams_ceiling_diff,
  ams_extra_floor,
  ams_special,
  ams_secret,
  ams_unseen,
  ams_locked,
  ams_teleport,
  ams_exit,
  ams_unseen_secret,
  ams_portal,

  ams_invisible,
  ams_revealed_secret,
  ams_closed_door,
} automap_style_t;

typedef unsigned short line_activation_t;
typedef unsigned int line_flags_t;

typedef struct line_s
{
  int iLineID;           // proff 04/05/2000: needed for OpenGL
  vertex_t *v1, *v2;     // Vertices, from v1 to v2.
  fixed_t dx, dy;        // Precalculated v2 - v1 for side checking.
  float texel_length;
  line_flags_t flags;           // Animation related.
  short special;
  short tag;
  unsigned short sidenum[2];        // Visual appearance: SideDefs.
  fixed_t bbox[4];       // A bounding box, for the linedef's extent
  slopetype_t slopetype; // To aid move clipping.
  sector_t *frontsector; // Front and back sector.
  sector_t *backsector;
  int validcount;        // if == validcount, already checked
  int validcount2;
  void *specialdata;     // thinker_t for reversable actions
  int r_validcount;      // cph: if == gametic, r_flags already done
  r_flags_t r_flags;     // cph
  degenmobj_t soundorg;  // sound origin for switches/buttons

  // dsda
  byte player_activations;

  // hexen
  int special_args[5];

  // zdoom
  line_activation_t activation;
  byte locknumber;
  automap_style_t automap_style;
  int health;
  int healthgroup;
  const byte* tranmap;
  float alpha;
} line_t;

#define LINE_ARG_COUNT 5
#define SPECIAL_ARGS_SIZE (5 * sizeof(int))
#define COLLAPSE_SPECIAL_ARGS(dest, source) { dest[0] = source[0]; \
                                              dest[1] = source[1]; \
                                              dest[2] = source[2]; \
                                              dest[3] = source[3]; \
                                              dest[4] = source[4]; }

// phares 3/14/98
//
// Sector list node showing all sectors an object appears in.
//
// There are two threads that flow through these nodes. The first thread
// starts at touching_thinglist in a sector_t and flows through the m_snext
// links to find all mobjs that are entirely or partially in the sector.
// The second thread starts at touching_sectorlist in an mobj_t and flows
// through the m_tnext links to find all sectors a thing touches. This is
// useful when applying friction or push effects to sectors. These effects
// can be done as thinkers that act upon all objects touching their sectors.
// As an mobj moves through the world, these nodes are created and
// destroyed, with the links changed appropriately.
//
// For the links, NULL means top or end of list.

typedef struct msecnode_s
{
  sector_t          *m_sector; // a sector containing this object
  struct mobj_s     *m_thing;  // this object
  struct msecnode_s *m_tprev;  // prev msecnode_t for this thing
  struct msecnode_s *m_tnext;  // next msecnode_t for this thing
  struct msecnode_s *m_sprev;  // prev msecnode_t for this sector
  struct msecnode_s *m_snext;  // next msecnode_t for this sector
  dboolean visited; // killough 4/4/98, 4/7/98: used in search algorithms
} msecnode_t;

//
// The LineSeg.
//
typedef struct
{
  vertex_t *v1, *v2;
  side_t* sidedef;
  line_t* linedef;
  // Sector references.
  // Could be retrieved from linedef, too
  // (but that would be slower -- killough)
  // backsector is NULL for one sided lines
  sector_t *frontsector, *backsector;
  fixed_t offset;
  angle_t angle;
  angle_t pangle; // re-calculated angle used for rendering
  uint32_t halflength; // fix long wall wobble
} seg_t;

typedef struct ssline_s
{
  seg_t *seg;
  line_t *linedef;
  fixed_t x1, y1;
  fixed_t x2, y2;
  fixed_t bbox[4];
} ssline_t;

//
// A SubSector.
// References a Sector.
// Basically, this is a list of LineSegs,
//  indicating the visible walls that define
//  (all or some) sides of a convex BSP leaf.
//

struct polyobj_s;

typedef struct subsector_s
{
  sector_t *sector;
  // e6y: support for extended nodes
  // 'int' instead of 'short'
  int numlines, firstline;

  // hexen
  struct polyobj_s *poly;
} subsector_t;


//
// BSP node.
//
typedef struct
{
  fixed_t  x,  y, dx, dy;        // Partition line.
  fixed_t bbox[2][4];            // Bounding box for each child.
  //unsigned short children[2];    // If NF_SUBSECTOR its a subsector.
  int children[2];    // If NF_SUBSECTOR its a subsector.
} node_t;

//
// OTHER TYPES
//

// This could be wider for >8 bit display.
// Indeed, true color support is posibble
// precalculating 24bpp lightmap/colormap LUT.
// from darkening PLAYPAL to all black.
// Could use even more than 32 levels.

typedef byte  lighttable_t;

//
// Masked 2s linedefs
//

typedef struct drawseg_s
{
  seg_t *curline;
  short x1, x2;
  fixed_t scale1, scale2, scalestep;
  int silhouette;                       // 0=none, 1=bottom, 2=top, 3=both
  fixed_t bsilheight;                   // do not clip sprites above this
  fixed_t tsilheight;                   // do not clip sprites below this

  // Pointers to lists for sprite clipping,
  // all three adjusted so [x1] is first value.

  int *sprtopclip, *sprbottomclip, *maskedtexturecol; // dropoff overflow
} drawseg_t;

// proff: Added for OpenGL
typedef struct
{
  int width,height;
  int leftoffset,topoffset;
  int lumpnum;
} patchnum_t;

//
// A vissprite_t is a thing that will be drawn during a refresh.
// i.e. a sprite object that is partly visible.
//

typedef struct vissprite_s
{
  short x1, x2;
  fixed_t gx, gy;              // for line side calculation
  fixed_t gz, gzt;             // global bottom / top for silhouette clipping
  fixed_t startfrac;           // horizontal position of x1
  fixed_t scale;
  fixed_t xiscale;             // negative if flipped
  fixed_t texturemid;
  int patch;
  uint64_t mobjflags;

  // for color translation and shadow draw, maxbright frames as well
  const lighttable_t *colormap;

  // killough 3/27/98: height sector for underwater/fake ceiling support
  int heightsec;

  // hexen
  int pclass;                  // player class (used in translation)
  fixed_t floorclip;

  // zdoom
  const byte* tranmap;

  // misc
  int color;
} vissprite_t;

//
// Sprites are patches with a special naming convention
//  so they can be recognized by R_InitSprites.
// The base name is NNNNFx or NNNNFxFx, with
//  x indicating the rotation, x = 0, 1-7.
// The sprite and frame specified by a thing_t
//  is range checked at run time.
// A sprite is a patch_t that is assumed to represent
//  a three dimensional object and may have multiple
//  rotations pre drawn.
// Horizontal flipping is used to save space,
//  thus NNNNF2F5 defines a mirrored patch.
// Some sprites will only have one picture used
// for all views: NNNNF0
//

typedef struct
{
  // If false use 0 for any position.
  // Note: as eight entries are available,
  //  we might as well insert the same name eight times.
  int rotate;

  // Lump to use for view angles 0-7.
  short lump[16];

  // Flip bit (1 = flip) to use for view angles 0-15.
  unsigned short flip;

} spriteframe_t;

//
// A sprite definition:
//  a number of animation frames.
//

typedef struct
{
  int numframes;
  spriteframe_t *spriteframes;
} spritedef_t;

//
// Now what is a visplane, anyway?
//
// Go to http://classicgaming.com/doom/editing/ to find out -- killough
//

typedef struct visplane
{
  struct visplane *next;        // Next visplane in hash chain -- killough
  int picnum, lightlevel, minx, maxx;
  int special; // heretic
  fixed_t height;
  fixed_t xoffs, yoffs;         // killough 2/28/98: Support scrolling flats
  angle_t rotation;
  fixed_t xscale;
  fixed_t yscale;
  // e6y: resolution limitation is removed
  // bottom and top arrays are dynamically
  // allocated immediately after the visplane
  unsigned short *bottom;
  unsigned short pad1;          // leave pads for [minx-1]/[maxx+1]
  unsigned short top[3];

  // NEW FIELDS MUST BE ADDED **ABOVE** `unsigned short *bottom;`

} visplane_t;

// hexen

typedef struct polyobj_s
{
  int numsegs;
  seg_t **segs;
  degenmobj_t startSpot;
  vertex_t *originalPts;      // used as the base for the rotations
  vertex_t *prevPts;          // use to restore the old point values
  angle_t angle;
  int tag;                    // reference tag assigned in HereticEd
  int bbox[4];
  int validcount;
  int validcount2;
  dboolean crush;              // should the polyobj attempt to crush mobjs?
  dboolean hurt;
  int seqType;
  fixed_t size;               // polyobj size (area of POLY_AREAUNIT == size of FRACUNIT)
  void *specialdata;          // pointer a thinker, if the poly is moving
  subsector_t *subsector;
} polyobj_t;

typedef struct polyblock_s
{
  polyobj_t *polyobj;
  struct polyblock_s *prev;
  struct polyblock_s *next;
} polyblock_t;

#define PO_LINE_START 1         // polyobj line start special
#define PO_LINE_EXPLICIT 5

extern polyobj_t *polyobjs;     // list of all poly-objects on the level
extern int po_NumPolyobjs;

extern int Sky1Texture;
extern int Sky2Texture;
extern fixed_t Sky1ColumnOffset;
extern fixed_t Sky2ColumnOffset;
extern dboolean DoubleSky;

#endif
