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
 *-----------------------------------------------------------------------------
 */

#ifndef __E6Y__
#define __E6Y__

#include <stdarg.h>

#include "hu_lib.h"

#define GL_COMBINE_ARB                    0x8570
#define GL_RGB_SCALE_ARB                  0x8573

#define FOV_CORRECTION_FACTOR (1.13776f)
#define FOV90 (90)

typedef struct camera_s
{
  fixed_t x;
  fixed_t y;
  fixed_t z;
  fixed_t PrevX;
  fixed_t PrevY;
  fixed_t PrevZ;
  angle_t angle;
  angle_t pitch;
  angle_t PrevAngle;
  angle_t PrevPitch;
  int type;
} camera_t;

extern dboolean wasWiped;

extern int secretfound;
extern int demo_tics_count;
extern int demo_playerscount;
extern char demo_len_st[80];

extern int mouse_handler;

extern int gl_render_fov;
extern float gl_render_ratio;
extern float gl_render_fovratio;
extern float gl_render_fovy;
extern float gl_render_multiplier;
void M_ChangeAspectRatio(void);
void M_ChangeStretch(void);

extern camera_t walkcamera;

extern int PitchSign;
extern angle_t viewpitch;
extern float skyscale;
extern float screen_skybox_zplane;
extern float maxNoPitch[];
extern float tan_pitch;
extern float skyUpAngle;
extern float skyUpShift;
extern float skyXShift;
extern float skyYShift;

void ParamsMatchingCheck();
void e6y_HandleSkip(void);
void e6y_InitCommandLine(void);

void P_WalkTicker ();
void P_SyncWalkcam(dboolean sync_coords, dboolean sync_sight);
void P_ResetWalkcam(void);

extern dboolean sound_inited_once;
void e6y_I_uSleep(unsigned long usecs);
void G_SkipDemoStop(void);
void G_SkipDemoStartCheck(void);
void G_SkipDemoCheck(void);
int G_ReloadLevel(void);
int G_GotoNextLevel(void);

void M_ChangeSkyMode(void);
void M_ChangeMaxViewPitch(void);

void M_ChangeFOV(void);

void M_ChangeSpeed(void);
void M_ChangeScreenMultipleFactor(void);
void M_ChangeInterlacedScanning(void);
void M_MouseMLook(int choice);
void M_MouseAccel(int choice);
void CheckPitch(signed int *pitch);

dboolean HaveMouseLook(void);

extern float viewPitch;

typedef struct prboom_comp_s
{
  unsigned int minver;
  unsigned int maxver;
  dboolean state;
  int arg_id;
} prboom_comp_t;

enum
{
  PC_MONSTER_AVOID_HAZARDS,
  PC_REMOVE_SLIME_TRAILS,
  PC_NO_DROPOFF,
  PC_TRUNCATED_SECTOR_SPECIALS,
  PC_BOOM_BRAINAWAKE,
  PC_PRBOOM_FRICTION,
  PC_REJECT_PAD_WITH_FF,
  PC_FORCE_LXDOOM_DEMO_COMPATIBILITY,
  PC_ALLOW_SSG_DIRECT,
  PC_TREAT_NO_CLIPPING_THINGS_AS_NOT_BLOCKING,
  PC_FORCE_INCORRECT_PROCESSING_OF_RESPAWN_FRAME_ENTRY,
  PC_FORCE_CORRECT_CODE_FOR_3_KEYS_DOORS_IN_MBF,
  PC_UNINITIALIZE_CRUSH_FIELD_FOR_STAIRS,
  PC_FORCE_BOOM_FINDNEXTHIGHESTFLOOR,
  PC_ALLOW_SKY_TRANSFER_IN_BOOM,
  PC_APPLY_GREEN_ARMOR_CLASS_TO_ARMOR_BONUSES,
  PC_APPLY_BLUE_ARMOR_CLASS_TO_MEGASPHERE,
  PC_FORCE_INCORRECT_BOBBING_IN_BOOM,
  PC_BOOM_DEH_PARSER,
  PC_MBF_REMOVE_THINKER_IN_KILLMOBJ,
  PC_DO_NOT_INHERIT_FRIENDLYNESS_FLAG_ON_SPAWN,
  PC_DO_NOT_USE_MISC12_FRAME_PARAMETERS_IN_A_MUSHROOM,
  PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL,
  PC_RESET_MONSTERSPAWNER_PARAMS_AFTER_LOADING,
  PC_MAX
};

extern prboom_comp_t prboom_comp[];

int StepwiseSum(int value, int direction, int minval, int maxval, int defval);

enum
{
  TT_ALLKILL,
  TT_ALLITEM,
  TT_ALLSECRET,

  TT_TIME,
  TT_TOTALTIME,
  TT_TOTALKILL,
  TT_TOTALITEM,
  TT_TOTALSECRET,

  TT_MAX
};

typedef struct timetable_s
{
  char map[16];

  int kill[MAX_MAXPLAYERS];
  int item[MAX_MAXPLAYERS];
  int secret[MAX_MAXPLAYERS];

  int stat[TT_MAX];
} timetable_t;

#ifdef _WIN32
const char* WINError(void);
#endif

extern int stats_level;
extern int stroller;

void e6y_G_DoCompleted(void);
void e6y_WriteStats(void);

void e6y_G_DoTeleportNewMap(void);
void e6y_G_DoWorldDone(void);

void I_ProcessWin32Mouse(void);
void I_StartWin32Mouse(void);
void I_EndWin32Mouse(void);
int AccelerateMouse(int val);
int AccelerateAnalog(float val);
void AccelChanging(void);

extern int mlooky;

void e6y_G_Compatibility(void);

const char* PathFindFileName(const char* pPath);

//extern int viewMaxY;

extern dboolean isskytexture;

extern int levelstarttic;

extern int force_singletics_to;

int HU_DrawDemoProgress(int force);

#ifdef _WIN32
int GetFullPath(const char* FileName, const char* ext, char *Buffer, size_t BufferLength);
#endif

void I_vWarning(const char *message, va_list argList);

#define PRB_MB_OK                       0x00000000
#define PRB_MB_OKCANCEL                 0x00000001
#define PRB_MB_ABORTRETRYIGNORE         0x00000002
#define PRB_MB_YESNOCANCEL              0x00000003
#define PRB_MB_YESNO                    0x00000004
#define PRB_MB_RETRYCANCEL              0x00000005
#define PRB_MB_DEFBUTTON1               0x00000000
#define PRB_MB_DEFBUTTON2               0x00000100
#define PRB_MB_DEFBUTTON3               0x00000200
#define PRB_IDOK                1
#define PRB_IDCANCEL            2
#define PRB_IDABORT             3
#define PRB_IDRETRY             4
#define PRB_IDIGNORE            5
#define PRB_IDYES               6
#define PRB_IDNO                7
int I_MessageBox(const char* text, unsigned int type);

#endif
