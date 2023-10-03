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

#ifndef _GL_INTERN_H
#define _GL_INTERN_H

#include "v_video.h"
#include "xs_Float.h"

#define MAXCOORD (32767.0f / MAP_COEFF)

#define SMALLDELTA 0.001f

typedef enum
{
  GLDT_UNREGISTERED,
  GLDT_BROKEN,
  GLDT_PATCH,
  GLDT_TEXTURE,
  GLDT_FLAT,
  GLDT_COLORMAP
} GLTexType;

typedef enum
{
  GLTEXTURE_SPRITE    = 0x00000002,
  GLTEXTURE_HASHOLES  = 0x00000004,
  GLTEXTURE_SKY       = 0x00000008,


  GLTEXTURE_CLAMPX    = 0x00000040,
  GLTEXTURE_CLAMPY    = 0x00000080,
  GLTEXTURE_CLAMPXY   = (GLTEXTURE_CLAMPX | GLTEXTURE_CLAMPY),
  GLTEXTURE_INDEXED   = 0x00000100,
  GLTEXTURE_SKYHACK   = 0x00000200,
} GLTexture_flag_t;

typedef struct gl_strip_coords_s
{
  GLfloat v[4][3];

  GLfloat t[4][2];
} gl_strip_coords_t;

#define PLAYERCOLORMAP_COUNT (9)

typedef struct detail_s
{
  GLuint texid;
  int texture_num;
  float width, height;
  float offsetx, offsety;
} detail_t;

typedef struct color_rgb_s
{
  byte r;
  byte g;
  byte b;
} color_rgb_t;

typedef struct
{
  int index;
  int patch_index;
  int width,height;
  int leftoffset,topoffset;
  int tex_width,tex_height;
  int realtexwidth, realtexheight;
  int buffer_width,buffer_height;
  int buffer_size;

  //e6y: support for Boom colormaps
  GLuint ***glTexExID;
  unsigned int texflags[CR_LIMIT+MAX_MAXPLAYERS][PLAYERCOLORMAP_COUNT];
  GLuint *texid_p;
  unsigned int *texflags_p;

  int cm;
  int player_cm;

  GLTexType textype;
  unsigned int flags;
  float scalexfac, scaleyfac; //e6y: right/bottom UV coordinates for patch drawing
} GLTexture;

typedef struct
{
  float x1,x2;
  float z1,z2;
  dboolean fracleft, fracright; //e6y
} GLSeg;

typedef struct
{
  GLSeg *glseg;
  float ytop,ybottom;
  float ul,ur,vt,vb;
  float light;
  float alpha;
  float skypitch;
  float skyyaw;
  float skyoffset;
  float xscale;
  float yscale;
  dboolean anchor_vb;
  GLTexture *gltexture;
  byte flag;
  seg_t *seg;
} GLWall;

typedef enum
{
  GLFLAT_CEILING      = 0x00000001,
  GLFLAT_HAVE_TRANSFORM  = 0x00000002,
} GLFlat_flag_t;

typedef struct
{
  int sectornum;
  float light; // the lightlevel of the flat
  float fogdensity;
  float uoffs,voffs; // the texture coordinates
  float rotation;
  float xscale;
  float yscale;
  float z; // the z position of the flat (height)
  GLTexture *gltexture;
  unsigned int flags;
  float alpha;
} GLFlat;

/* GLLoopDef is the struct for one loop. A loop is a list of vertexes
 * for triangles, which is calculated by the gluTesselator in gld_PrecalculateSector
 * and in gld_PreprocessCarvedFlat
 */
typedef struct
{
  int index;   // subsector index
  GLenum mode; // GL_TRIANGLES, GL_TRIANGLE_STRIP or GL_TRIANGLE_FAN
  int vertexcount; // number of vertexes in this loop
  int vertexindex; // index into vertex list
} GLLoopDef;

// GLSector is the struct for a sector with a list of loops.

#define SECTOR_CLAMPXY   0x00000001
typedef struct
{
  int loopcount; // number of loops for this sector
  GLLoopDef *loops; // the loops itself
  unsigned int flags;
} GLSector;

typedef struct
{
  int loopcount; // number of loops for this sector
  GLLoopDef *loops; // the loops itself
} GLMapSubsector;

typedef struct
{
  GLfloat x;
  GLfloat y;
  GLfloat z;
} GLVertex;

typedef struct
{
  GLfloat u;
  GLfloat v;
} GLTexcoord;

typedef struct
{
  GLLoopDef loop; // the loops itself
} GLSubSector;

typedef struct
{
  float x, y, z;
  float radius;
  float light;
} GLShadow;

typedef enum
{
  health_bar_null,
  health_bar_red,
  health_bar_yellow,
} health_bar_color_t;

typedef struct
{
  health_bar_color_t color;

  float x1, x2, x3;
  float z1, z2, z3;
  float y;
} GLHealthBar;

extern GLSeg *gl_segs;
extern GLSeg *gl_lines;

#define GLDWF_TOP 1
#define GLDWF_M1S 2
#define GLDWF_M2S 3
#define GLDWF_BOT 4
#define GLDWF_TOPFLUD 5 //e6y: project the ceiling plane into the gap
#define GLDWF_BOTFLUD 6 //e6y: project the floor plane into the gap
#define GLDWF_SKY 7
#define GLDWF_SKYFLIP 8

typedef struct
{
  int cm;
  float x,y,z;
  float vt,vb;
  float ul,ur;
  float x1,y1;
  float x2,y2;
  float light;
  float alpha;
  fixed_t scale;
  GLTexture *gltexture;
  uint64_t flags;
  int index;
  int id;
  int xy;
  fixed_t fx,fy;
} GLSprite;

typedef enum
{
  GLDIT_NONE,

  GLDIT_WALL,    // opaque wall
  GLDIT_MWALL,   // opaque mid wall
  GLDIT_FWALL,   // projected wall
  GLDIT_TWALL,   // transparent walls
  GLDIT_SWALL,   // sky walls

  GLDIT_AWALL,   // animated wall
  GLDIT_FAWALL,  // animated projected wall

  GLDIT_CEILING, // ceiling
  GLDIT_FLOOR,   // floor

  GLDIT_ACEILING, // animated ceiling
  GLDIT_AFLOOR,   // animated floor

  GLDIT_SPRITE,  // opaque sprite
  GLDIT_TSPRITE, // transparent sprites
  GLDIT_ASPRITE,

  GLDIT_SHADOW,

  GLDIT_HBAR,

  GLDIT_TYPES
} GLDrawItemType;

typedef struct GLDrawItem_s
{
  union
  {
    void *item;
    GLWall *wall;
    GLFlat *flat;
    GLSprite *sprite;
    GLShadow *shadow;
    GLHealthBar *hbar;
  } item;
} GLDrawItem;

typedef struct GLDrawDataItem_s
{
  byte *data;
  int maxsize;
  int size;
} GLDrawDataItem_t;

typedef struct
{
  GLDrawDataItem_t *data;
  int maxsize;
  int size;

  GLDrawItem *items[GLDIT_TYPES];
  int num_items[GLDIT_TYPES];
  int max_items[GLDIT_TYPES];
} GLDrawInfo;

void gld_AddDrawItem(GLDrawItemType itemtype, void *itemdata);

void gld_DrawTriangleStrip(GLWall *wall, gl_strip_coords_t *c);

extern float roll;
extern float yaw;
extern float inv_yaw;
extern float pitch;

extern int gl_preprocessed; //e6y

extern GLDrawInfo gld_drawinfo;
void gld_FreeDrawInfo(void);
void gld_ResetDrawInfo(void);

extern GLSector *sectorloops;
extern GLMapSubsector *subsectorloops;

extern GLfloat gl_texture_filter_anisotropic;
void gld_SetTexFilters(GLTexture *gltexture);

extern float xCamera,yCamera,zCamera;

//
//detail
//

void gld_InitDetail(void);

void gld_PreprocessDetail(void);

extern GLuint* last_glTexID;
GLTexture *gld_RegisterTexture(int texture_num, dboolean mipmap, dboolean force, dboolean indexed, dboolean sky);
void gld_BindTexture(GLTexture *gltexture, unsigned int flags, dboolean sky);
GLTexture *gld_RegisterPatch(int lump, int cm, dboolean is_sprite, dboolean indexed);
void gld_BindPatch(GLTexture *gltexture, int cm);
GLTexture *gld_RegisterRaw(int lump, int width, int height, dboolean mipmap, dboolean indexed);
void gld_BindRaw(GLTexture *gltexture, unsigned int flags);
#define gld_RegisterFlat(lump, mipmap, indexed) \
  gld_RegisterRaw((firstflat+lump), 64, 64, (mipmap), (indexed))
#define gld_BindFlat(gltexture, flags) \
  gld_BindRaw((gltexture), (flags))
GLTexture *gld_RegisterSkyTexture(int texture_num, dboolean force);
void gld_BindSkyTexture(GLTexture *gltexture);
GLTexture *gld_RegisterColormapTexture(int palette_index, int gamma_level, dboolean fullbright);
void gld_BindColormapTexture(GLTexture *gltexture, int palette_index, int gamma_level, dboolean fullbright);
void gld_InitColormapTextures(dboolean fullbright);
void gld_InitFuzzTexture(void);
int gld_GetTexDimension(int value);
void gld_SetIndexedPalette(int palette_index);
void gld_Precache(void);

void SetFrameTextureMode(void);

//gl_vertex
void gld_SplitLeftEdge(const GLWall *wall);
void gld_SplitRightEdge(const GLWall *wall);
void gld_RecalcVertexHeights(const vertex_t *v);

//e6y
void gld_InitGLVersion(void);
void gld_ResetLastTexture(void);

unsigned char* gld_GetTextureBuffer(GLuint texid, int miplevel, int *width, int *height);

int gld_BuildTexture(GLTexture *gltexture, void *data, dboolean readonly, int width, int height);

GLuint CaptureScreenAsTexID(void);

//progress
void gld_ProgressUpdate(const char * text, int progress, int total);
int gld_ProgressStart(void);
int gld_ProgressEnd(void);

//FBO
extern GLuint glSceneImageFBOTexID;
extern GLuint glSceneImageTextureFBOTexID;

extern dboolean invul_cm;
extern float bw_red;
extern float bw_green;
extern float bw_blue;
extern int SceneInTexture;
void gld_InitFBO(void);
void gld_FreeScreenSizeFBO(void);

extern int imageformats[];

//missing flats (fake floors and ceilings)

void gld_PreprocessFakeSectors(void);

void gld_SetupFloodStencil(GLWall *wall);
void gld_ClearFloodStencil(GLWall *wall);

void gld_SetupFloodedPlaneCoords(GLWall *wall, gl_strip_coords_t *c);
void gld_SetupFloodedPlaneLight(GLWall *wall);

//light
void gld_StaticLightAlpha(float light, float alpha);
#define gld_StaticLight(light) gld_StaticLightAlpha(light, 1.0f)
void gld_InitLightTable(void);
int gld_GetGunFlashLight(void);

float gld_CalcLightLevel(int lightlevel);
float gld_Calc2DLightLevel(int lightlevel);

// SkyBox
#define SKY_NONE    0
#define SKY_CEILING 1
#define SKY_FLOOR   2
typedef struct PalEntry_s
{
  unsigned char r, g, b;
} PalEntry_t;
typedef struct SkyBoxParams_s
{
  int index;
  unsigned int type;
  GLWall wall;
  float x_offset, y_offset;
  // 0 - no colormap; 1 - INVUL inverse colormap
  PalEntry_t FloorSkyColor[2];
  PalEntry_t CeilingSkyColor[2];
} SkyBoxParams_t;
extern SkyBoxParams_t SkyBox;
extern GLfloat gl_whitecolor[];
void gld_InitSky(void);
void gld_AddSkyTexture(GLWall *wall, int sky1, int sky2, int skytype);
void gld_GetSkyCapColors(void);
void gld_InitFrameSky(void);
void gld_DrawStripsSky(void);
void gld_DrawScreenSkybox(void);
void gld_GetScreenSkyScale(GLWall *wall, float *scale_x, float *scale_y);
void gld_DrawDomeSkyBox(void);
void gld_DrawSkyCaps(void);

// VBO
typedef struct vbo_vertex_s
{
  float x, y, z;
  float u, v;
  unsigned char r, g, b, a;
} PACKEDATTR vbo_vertex_t;
#define NULL_VBO_VERTEX ((vbo_vertex_t*)NULL)
#define sky_vbo_x (gl_ext_arb_vertex_buffer_object ? &NULL_VBO_VERTEX->x : &vbo->data[0].x)
#define sky_vbo_u (gl_ext_arb_vertex_buffer_object ? &NULL_VBO_VERTEX->u : &vbo->data[0].u)
#define sky_vbo_r (gl_ext_arb_vertex_buffer_object ? &NULL_VBO_VERTEX->r : &vbo->data[0].r)

typedef struct vbo_xyz_uv_s
{
  float x, y, z;
  float u, v;
} PACKEDATTR vbo_xyz_uv_t;
extern vbo_xyz_uv_t *flats_vbo;
#define NULL_VBO_XYZ_UV ((vbo_xyz_uv_t*)NULL)
#define flats_vbo_x (gl_ext_arb_vertex_buffer_object ? &NULL_VBO_XYZ_UV->x : &flats_vbo[0].x)
#define flats_vbo_u (gl_ext_arb_vertex_buffer_object ? &NULL_VBO_XYZ_UV->u : &flats_vbo[0].u)

typedef struct vbo_xy_uv_rgba_s
{
  float x, y;
  float u, v;
  unsigned char r, g, b, a;
} PACKEDATTR vbo_xy_uv_rgba_t;

// preprocessing
extern byte *segrendered; // true if sector rendered (only here for malloc)
extern int *linerendered[2]; // true if linedef rendered (only here for malloc)
extern int rendermarker;
extern GLuint flats_vbo_id;

void glsl_Init(void);
void glsl_SetTextureDims(int unit, unsigned int width, unsigned int height);
void glsl_PushNullShader(void);
void glsl_PopNullShader(void);
void glsl_PushMainShader(void);
void glsl_PopMainShader(void);
void glsl_PushFuzzShader(int tic, int sprite, float ratio);
void glsl_PopFuzzShader(void);
void glsl_SetLightLevel(float lightlevel);

#endif // _GL_INTERN_H
