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
 *
 *---------------------------------------------------------------------
 */

#ifndef _GL_STRUCT_H
#define _GL_STRUCT_H

#include <SDL_opengl.h>

extern dboolean use_gl_nodes;

typedef enum {
  skytype_auto,
  skytype_none,
  skytype_standard,
  skytype_skydome,

  skytype_count
} skytype_t;

#define MAX_GLGAMMA 32

enum bleedtype {
  BLEED_NONE = 0x0,
  BLEED_CEILING = 0x1,
  BLEED_OCCLUDE = 0x2
};

extern int gl_drawskys;
extern dboolean gl_ui_lightmode_indexed;
extern dboolean gl_automap_lightmode_indexed;
void gld_FlushTextures(void);

void gld_InitVertexData();
void gld_CleanVertexData();
void gld_UpdateSplitData(sector_t *sector);

void gld_Init(int width, int height);
void gld_InitCommandLine(void);

void gld_BeginUIDraw(void);
void gld_EndUIDraw(void);
void gld_BeginAutomapDraw(void);
void gld_EndAutomapDraw(void);

void gld_DrawNumPatch(int x, int y, int lump, int cm, enum patch_translation_e flags);
void gld_DrawNumPatch_f(float x, float y, int lump, int cm, enum patch_translation_e flags);

void gld_FillRaw(int lump, int x, int y, int src_width, int src_height, int dst_width, int dst_height, enum patch_translation_e flags);
#define gld_FillRawName(name, x, y, src_width, src_height, dst_width, dst_height, flags) \
  gld_FillRaw(W_GetNumForName(name), (x), (y), (src_width), (src_height), (dst_width), (dst_height), (flags))

#define gld_FillFlat(lump, x, y, width, height, flags) \
  gld_FillRaw((firstflat+lump), (x), (y), 64, 64, (width), (height), (flags))
#define gld_FillFlatName(flatname, x, y, width, height, flags) \
  gld_FillFlat(R_FlatNumForName(flatname), (x), (y), (width), (height), (flags))

void gld_FillPatch(int lump, int x, int y, int width, int height, enum patch_translation_e flags);
#define gld_FillPatchName(name, x, y, width, height, flags) \
  gld_FillPatch(W_GetNumForName(name), (x), (y), (width), (height), (flags))

void gld_DrawLine(int x0, int y0, int x1, int y1, int BaseColor);
void gld_DrawLine_f(float x0, float y0, float x1, float y1, int BaseColor);
void gld_DrawWeapon(int weaponlump, vissprite_t *vis, int lightlevel);
void gld_FillBlock(int x, int y, int width, int height, int col);
void gld_SetPalette(int palette);
unsigned char *gld_ReadScreen(void);

void gld_CleanMemory(void);
void gld_CleanStaticMemory(void);
void gld_PreprocessLevel(void);

void gld_Set2DMode();
void gld_InitDrawScene(void);
void gld_StartDrawScene(void);
void gld_AddPlane(int subsectornum, visplane_t *floor, visplane_t *ceiling);
void gld_AddWall(seg_t *seg);
void gld_ProjectSprite(mobj_t* thing, int lightlevel);
void gld_DrawScene(player_t *player);
void gld_EndDrawScene(void);
void gld_Finish();

// wipe
int gld_wipe_doMelt(int ticks, int *y_lookup);
int gld_wipe_exitMelt(int ticks);
int gld_wipe_StartScreen(void);
int gld_wipe_EndScreen(void);

//clipper
dboolean gld_clipper_SafeCheckRange(angle_t startAngle, angle_t endAngle);
void gld_clipper_SafeAddClipRange(angle_t startangle, angle_t endangle);
void gld_FrustumSetup(void);
dboolean gld_SphereInFrustum(float x, float y, float z, float radius);

//missing flats (fake floors and ceilings)
extern dboolean gl_use_stencil;
sector_t* GetBestFake(sector_t *sector, int ceiling, int validcount);
sector_t* GetBestBleedSector(sector_t* source, enum bleedtype type);

void gld_DrawMapLines(void);

//multisampling
void gld_MultisamplingInit(void);
void gld_MultisamplingSet(void);

void gld_ProcessTexturedMap(void);
void gld_ResetTexturedAutomap(void);
void gld_MapDrawSubsectors(player_t *plr, int fx, int fy, fixed_t mx, fixed_t my, int fw, int fh, fixed_t scale);

void gld_Init8InGLMode(void);
void gld_Draw8InGL(void);

// Nice map
enum
{
  am_icon_shadow,

  am_icon_corpse,
  am_icon_normal,
  am_icon_health,
  am_icon_armor,
  am_icon_ammo,
  am_icon_key,
  am_icon_power,
  am_icon_weap,

  am_icon_arrow,
  am_icon_monster,
  am_icon_player,
  am_icon_mark,
  am_icon_bullet,

  am_icon_count
};

typedef struct am_icon_s
{
  GLuint tex_id;
  const char* name;
  int lumpnum;
} am_icon_t;
extern am_icon_t am_icons[];

void gld_InitMapPics(void);
void gld_AddNiceThing(int type, float x, float y, float radius, float angle,
                     unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void gld_DrawNiceThings(int fx, int fy, int fw, int fh);
void gld_ClearNiceThings(void);

extern int gl_render_fov;

#endif // _GL_STRUCT_H
