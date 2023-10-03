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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#include <winreg.h>
#endif
//#include <SDL_opengl.h>
#include <string.h>
#include <math.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

//#include "SDL.h"
#ifdef _WIN32
#include <SDL_syswm.h>
#endif

#include "hu_lib.h"

#include "doomtype.h"
#include "doomstat.h"
#include "d_main.h"
#include "s_sound.h"
#include "i_system.h"
#include "i_main.h"
#include "i_sound.h"
#include "m_menu.h"
#include "lprintf.h"
#include "m_file.h"
#include "i_system.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"
#include "i_video.h"
#include "info.h"
#include "r_main.h"
#include "r_things.h"
#include "r_sky.h"
#include "am_map.h"
#include "dsda.h"
#include "dsda/settings.h"
//#include "gl_struct.h"
//#include "gl_intern.h"
#include "g_game.h"
#include "d_deh.h"
#include "e6y.h"
#include "m_file.h"
#include "host.h"

#include "dsda/args.h"
#include "dsda/map_format.h"
#include "dsda/mapinfo.h"
#include "dsda/playback.h"
#include "dsda/skip.h"
#include "dsda/stretch.h"

dboolean wasWiped = false;

int secretfound;
int demo_playerscount;
int demo_tics_count;
char demo_len_st[80];

int mouse_handler;
int gl_render_fov = 90;

camera_t walkcamera;

angle_t viewpitch;
float skyscale;
float screen_skybox_zplane;
float tan_pitch;
float skyUpAngle;
float skyUpShift;
float skyXShift;
float skyYShift;

#ifdef _WIN32
const char* WINError(void)
{
  static char *WinEBuff = NULL;
  DWORD err = GetLastError();
  char *ch;

  if (WinEBuff)
  {
    LocalFree(WinEBuff);
  }

  if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR) &WinEBuff, 0, NULL) == 0)
  {
    return "Unknown error";
  }

  if ((ch = strchr(WinEBuff, '\n')) != 0)
    *ch = 0;
  if ((ch = strchr(WinEBuff, '\r')) != 0)
    *ch = 0;

  return WinEBuff;
}
#endif

//--------------------------------------------------

/* ParamsMatchingCheck
 * Conflicting command-line parameters could cause the engine to be confused
 * in some cases. Added checks to prevent this.
 * Example: dsda-doom.exe -record mydemo -playdemo demoname
 */
void ParamsMatchingCheck()
{
  dboolean recording_attempt =
    dsda_Flag(dsda_arg_record) ||
    dsda_Flag(dsda_arg_recordfromto);

  dboolean playbacking_attempt =
    dsda_Flag(dsda_arg_playdemo) ||
    dsda_Flag(dsda_arg_timedemo) ||
    dsda_Flag(dsda_arg_fastdemo);

  if (recording_attempt && playbacking_attempt)
    I_Error("Params are not matching: Can not being played back and recorded at the same time.");
}

prboom_comp_t prboom_comp[PC_MAX] = {
  {0xffffffff, 0x02020615, 0, dsda_arg_force_monster_avoid_hazards},
  {0x00000000, 0x02040601, 0, dsda_arg_force_remove_slime_trails},
  {0x02020200, 0x02040801, 0, dsda_arg_force_no_dropoff},
  {0x00000000, 0x02040801, 0, dsda_arg_force_truncated_sector_specials},
  {0x00000000, 0x02040802, 0, dsda_arg_force_boom_brainawake},
  {0x00000000, 0x02040802, 0, dsda_arg_force_prboom_friction},
  {0x02020500, 0x02040000, 0, dsda_arg_reject_pad_with_ff},
  {0xffffffff, 0x02040802, 0, dsda_arg_force_lxdoom_demo_compatibility},
  {0x00000000, 0x0202061b, 0, dsda_arg_allow_ssg_direct},
  {0x00000000, 0x02040601, 0, dsda_arg_treat_no_clipping_things_as_not_blocking},
  {0x00000000, 0x02040803, 0, dsda_arg_force_incorrect_processing_of_respawn_frame_entry},
  {0x00000000, 0x02040601, 0, dsda_arg_force_correct_code_for_3_keys_doors_in_mbf},
  {0x00000000, 0x02040601, 0, dsda_arg_uninitialize_crush_field_for_stairs},
  {0x00000000, 0x02040802, 0, dsda_arg_force_boom_findnexthighestfloor},
  {0x00000000, 0x02040802, 0, dsda_arg_allow_sky_transfer_in_boom},
  {0x00000000, 0x02040803, 0, dsda_arg_apply_green_armor_class_to_armor_bonuses},
  {0x00000000, 0x02040803, 0, dsda_arg_apply_blue_armor_class_to_megasphere},
  {0x02020200, 0x02050003, 0, dsda_arg_force_incorrect_bobbing_in_boom},
  {0xffffffff, 0x00000000, 0, dsda_arg_boom_deh_parser},
  {0x00000000, 0x02050007, 0, dsda_arg_mbf_remove_thinker_in_killmobj},
  {0x00000000, 0x02050007, 0, dsda_arg_do_not_inherit_friendlyness_flag_on_spawn},
  {0x00000000, 0x02050007, 0, dsda_arg_do_not_use_misc12_frame_parameters_in_a_mushroom},
  {0x00000000, 0x02050102, 0, dsda_arg_apply_mbf_codepointers_to_any_complevel},
  {0x00000000, 0x02050104, 0, dsda_arg_reset_monsterspawner_params_after_loading},
};

void M_ChangeShorttics(void)
{
  shorttics = dsda_IntConfig(dsda_config_movement_shorttics) || dsda_Flag(dsda_arg_shorttics);
}

void e6y_InitCommandLine(void)
{
  stats_level = dsda_Flag(dsda_arg_levelstat);

  if ((stroller = dsda_Flag(dsda_arg_stroller)))
    dsda_UpdateIntArg(dsda_arg_turbo, "50");

  dsda_ReadCommandLine();

  M_ChangeShorttics();
}

int G_ReloadLevel(void)
{
  int result = false;

  if ((gamestate == GS_LEVEL || gamestate == GS_INTERMISSION) &&
      allow_incompatibility &&
      !menuactive)
  {
    G_DeferedInitNew(gameskill, gameepisode, gamemap);
    result = true;
  }

  if (demoplayback)
  {
    dsda_RestartPlayback();
    result = true;
  }

  dsda_WatchLevelReload(&result);

  return result;
}

int G_GotoNextLevel(void)
{
  int epsd, map;
  int changed = false;

  dsda_NextMap(&epsd, &map);

  if ((gamestate == GS_LEVEL) &&
    allow_incompatibility &&
    !menuactive)
  {
    G_DeferedInitNew(gameskill, epsd, map);
    changed = true;
  }

  return changed;
}

void M_ChangeSpeed(void)
{
  G_SetSpeed(true);
}

void M_ChangeSkyMode(void)
{
#if 0
  int gl_skymode;

  viewpitch = 0;

  R_InitSkyMap();

  gl_skymode = dsda_IntConfig(dsda_config_gl_skymode);

  if (gl_skymode == skytype_auto)
    gl_drawskys = (dsda_MouseLook() ? skytype_skydome : skytype_standard);
  else
    gl_drawskys = gl_skymode;
#endif
}

static int upViewPitchLimit;
static int downViewPitchLimit;

void M_ChangeMaxViewPitch(void)
{
  if (raven || !V_IsOpenGLMode())
  {
    upViewPitchLimit = (int) raven_angle_up_limit;
    downViewPitchLimit = (int) raven_angle_down_limit;
  }
  else
  {
    upViewPitchLimit = -ANG90 + (1 << ANGLETOFINESHIFT);
    downViewPitchLimit = ANG90 - (1 << ANGLETOFINESHIFT);
  }

  CheckPitch((signed int *)&viewpitch);
}

void M_ChangeScreenMultipleFactor(void)
{
  V_ChangeScreenResolution();
}

dboolean HaveMouseLook(void)
{
  return (viewpitch != 0);
}

void CheckPitch(signed int *pitch)
{
  if (*pitch < upViewPitchLimit)
    *pitch = upViewPitchLimit;

  if (*pitch > downViewPitchLimit)
    *pitch = downViewPitchLimit;

  (*pitch) >>= 16;
  (*pitch) <<= 16;
}

float gl_render_ratio;
float gl_render_fovratio;
float gl_render_fovy = FOV90;
float gl_render_multiplier;

void M_ChangeAspectRatio(void)
{
  M_ChangeFOV();

  R_SetViewSize();
}

void M_ChangeStretch(void)
{
  render_stretch_hud = dsda_IntConfig(dsda_config_render_stretch_hud);

  R_SetViewSize();
}

void M_ChangeFOV(void)
{
  //float f1, f2;
  dsda_arg_t* arg;
  int gl_render_aspect_width, gl_render_aspect_height;

  arg = dsda_Arg(dsda_arg_aspect);
  if (
    arg->found &&
    sscanf(arg->value.v_string, "%dx%d", &gl_render_aspect_width, &gl_render_aspect_height) == 2
  )
  {
    SetRatio(SCREENWIDTH, SCREENHEIGHT);
    gl_render_fovratio = (float)gl_render_aspect_width / (float)gl_render_aspect_height;
    gl_render_ratio = RMUL * gl_render_fovratio;
    gl_render_multiplier = 64.0f / gl_render_fovratio / RMUL;
  }
  else
  {
    SetRatio(SCREENWIDTH, SCREENHEIGHT);
#if 0
    gl_render_ratio = gl_ratio;
    gl_render_multiplier = (float)ratio_multiplier;
    if (!tallscreen)
    {
      gl_render_fovratio = 1.6f;
    }
    else
    {
      gl_render_fovratio = gl_render_ratio;
    }
#endif
  }

#if 0
  gl_render_fovy = (float)(2 * RAD2DEG(atan(tan(DEG2RAD(gl_render_fov) / 2) / gl_render_fovratio)));

  screen_skybox_zplane = 320.0f/2.0f/(float)tan(DEG2RAD(gl_render_fov/2));

  f1 = (float)(320.0f / 200.0f * (float)gl_render_fov / (float)FOV90 - 0.2f);
  f2 = (float)tan(DEG2RAD(gl_render_fovy)/2.0f);
  if (f1-f2<1)
    skyUpAngle = (float)-RAD2DEG(asin(f1-f2));
  else
    skyUpAngle = -90.0f;

  skyUpShift = (float)tan(DEG2RAD(gl_render_fovy)/2.0f);

  skyscale = 1.0f / (float)tan(DEG2RAD(gl_render_fov / 2));
#endif
}

float viewPitch;

int StepwiseSum(int value, int direction, int minval, int maxval, int defval)
{
  int newvalue;
  int val = (direction > 0 ? value : value - 1);

  if (direction == 0)
    return defval;

  direction = (direction > 0 ? 1 : -1);

  {
    int exp = 1;
    while (exp * 10 <= val)
      exp *= 10;
    newvalue = direction * (val < exp * 5 && exp > 1 ? exp / 2 : exp);
    newvalue = (value + newvalue) / newvalue * newvalue;
  }

  if (newvalue > maxval) newvalue = maxval;
  if (newvalue < minval) newvalue = minval;

  if ((value < defval && newvalue > defval) || (value > defval && newvalue < defval))
    newvalue = defval;

  return newvalue;
}

void I_vWarning(const char *message, va_list argList)
{
  char msg[1024];
  vsnprintf(msg,sizeof(msg),message,argList);
  lprintf(LO_ERROR, "%s\n", msg);
#ifdef _WIN32
  I_MessageBox(msg, PRB_MB_OK);
#endif
}

int I_MessageBox(const char* text, unsigned int type)
{
#ifdef _WIN32
  int result = PRB_IDCANCEL;

  if (!dsda_Flag(dsda_arg_no_message_box))
  {
    HWND current_hwnd = GetForegroundWindow();
    wchar_t *wtext = ConvertUtf8ToWide(text);
    wchar_t *wpackage = ConvertUtf8ToWide(PACKAGE_NAME);
    result = MessageBoxW(GetDesktopWindow(), wtext, wpackage, type|MB_TASKMODAL|MB_TOPMOST);
    Z_Free(wtext);
    Z_Free(wpackage);
    I_SwitchToWindow(current_hwnd);
    return result;
  }
#endif

  return PRB_IDCANCEL;
}

int stats_level;
int stroller;
int numlevels = 0;
int levels_max = 0;
timetable_t *stats = NULL;

void e6y_G_DoCompleted(void)
{
  int i;

  dsda_EvaluateSkipModeDoCompleted();

  if(!stats_level)
    return;

  if (numlevels >= levels_max)
  {
    levels_max = levels_max ? levels_max*2 : 32;
    stats = Z_Realloc(stats,sizeof(*stats)*levels_max);
  }

  memset(&stats[numlevels], 0, sizeof(timetable_t));

  snprintf(stats[numlevels].map, sizeof(stats[numlevels].map), "%s", dsda_MapLumpName(gameepisode, gamemap));

  if (secretexit)
  {
    size_t end_of_string = strlen(stats[numlevels].map);
    if (end_of_string < 15)
      stats[numlevels].map[end_of_string] = 's';
  }

  stats[numlevels].stat[TT_TIME]        = leveltime;
  stats[numlevels].stat[TT_TOTALTIME]   = totalleveltimes;
  stats[numlevels].stat[TT_TOTALKILL]   = totalkills;
  stats[numlevels].stat[TT_TOTALITEM]   = totalitems;
  stats[numlevels].stat[TT_TOTALSECRET] = totalsecret;

  for (i = 0; i < g_maxplayers; i++)
  {
    if (playeringame[i])
    {
      stats[numlevels].kill[i]   = players[i].killcount - players[i].maxkilldiscount;
      stats[numlevels].item[i]   = players[i].itemcount;
      stats[numlevels].secret[i] = players[i].secretcount;

      stats[numlevels].stat[TT_ALLKILL]   += stats[numlevels].kill[i];
      stats[numlevels].stat[TT_ALLITEM]   += stats[numlevels].item[i];
      stats[numlevels].stat[TT_ALLSECRET] += stats[numlevels].secret[i];
    }
  }

  numlevels++;

  e6y_WriteStats();
}

typedef struct tmpdata_s
{
  char kill[200];
  char item[200];
  char secret[200];
} tmpdata_t;

void e6y_WriteStats(void)
{
  dg_file_t *f;
  char str[200];
  int i, level, playerscount;
  timetable_t max;
  tmpdata_t tmp;
  tmpdata_t *all;
  size_t allkills_len=0, allitems_len=0, allsecrets_len=0;

  f = M_OpenFile("levelstat.txt", "wb");

  if (f == NULL)
  {
    lprintf(LO_ERROR, "Unable to open levelstat.txt for writing\n");
    return;
  }

  all = Z_Malloc(sizeof(*all) * numlevels);
  memset(&max, 0, sizeof(timetable_t));

  playerscount = 0;
  for (i = 0; i < g_maxplayers; i++)
    if (playeringame[i])
      playerscount++;

  for (level=0;level<numlevels;level++)
  {
    memset(&tmp, 0, sizeof(tmpdata_t));
    for (i = 0; i < g_maxplayers; i++)
    {
      if (playeringame[i])
      {
        char strtmp[200];
        strcpy(str, tmp.kill[0] == '\0' ? "%s%d" : "%s+%d");

        snprintf(strtmp, sizeof(strtmp), str, tmp.kill, stats[level].kill[i]);
        strcpy(tmp.kill, strtmp);

        snprintf(strtmp, sizeof(strtmp), str, tmp.item, stats[level].item[i]);
        strcpy(tmp.item, strtmp);

        snprintf(strtmp, sizeof(strtmp), str, tmp.secret, stats[level].secret[i]);
        strcpy(tmp.secret, strtmp);
      }
    }
    if (playerscount<2)
      memset(&all[level], 0, sizeof(tmpdata_t));
    else
    {
      snprintf(all[level].kill, sizeof(all[level].kill),  " (%s)", tmp.kill  );
      snprintf(all[level].item, sizeof(all[level].item),   " (%s)", tmp.item  );
      snprintf(all[level].secret, sizeof(all[level].secret), " (%s)", tmp.secret);
    }

    if (strlen(all[level].kill) > allkills_len)
      allkills_len = strlen(all[level].kill);
    if (strlen(all[level].item) > allitems_len)
      allitems_len = strlen(all[level].item);
    if (strlen(all[level].secret) > allsecrets_len)
      allsecrets_len = strlen(all[level].secret);

    for(i=0; i<TT_MAX; i++)
      if (stats[level].stat[i] > max.stat[i])
        max.stat[i] = stats[level].stat[i];
  }
  max.stat[TT_TIME] = max.stat[TT_TIME]/TICRATE/60;
  max.stat[TT_TOTALTIME] = max.stat[TT_TOTALTIME]/TICRATE/60;

  for(i=0; i<TT_MAX; i++) {
    snprintf(str, sizeof(str), "%d", max.stat[i]);
    max.stat[i] = strlen(str);
  }

  for (level=0;level<numlevels;level++)
  {
    sprintf(str,
      "%%s - %%%dd:%%05.2f (%%%dd:%%02d)  K: %%%dd/%%-%dd%%%lds  I: %%%dd/%%-%dd%%%lds  S: %%%dd/%%-%dd %%%lds\r\n",
      max.stat[TT_TIME],      max.stat[TT_TOTALTIME],
      max.stat[TT_ALLKILL],   max.stat[TT_TOTALKILL],   (long)allkills_len,
      max.stat[TT_ALLITEM],   max.stat[TT_TOTALITEM],   (long)allitems_len,
      max.stat[TT_ALLSECRET], max.stat[TT_TOTALSECRET], (long)allsecrets_len);

    DG_printf(f, str, stats[level].map,
      stats[level].stat[TT_TIME]/TICRATE/60,
      (float)(stats[level].stat[TT_TIME]%(60*TICRATE))/TICRATE,
      (stats[level].stat[TT_TOTALTIME])/TICRATE/60,
      (stats[level].stat[TT_TOTALTIME]%(60*TICRATE))/TICRATE,
      stats[level].stat[TT_ALLKILL],  stats[level].stat[TT_TOTALKILL],   all[level].kill,
      stats[level].stat[TT_ALLITEM],  stats[level].stat[TT_TOTALITEM],   all[level].item,
      stats[level].stat[TT_ALLSECRET],stats[level].stat[TT_TOTALSECRET], all[level].secret
      );

  }

  Z_Free(all);
  DG_close(f);
}

//--------------------------------------------------

static double mouse_accelfactor;
static double analog_accelfactor;

void AccelChanging(void)
{
  int mouse_acceleration;
  int analog_acceleration;

  mouse_acceleration = dsda_IntConfig(dsda_config_mouse_acceleration);
  mouse_accelfactor = (double) mouse_acceleration / 100.0 + 1.0;

  analog_acceleration = dsda_IntConfig(dsda_config_analog_look_acceleration);
  analog_accelfactor = (double) analog_acceleration / 100.0 + 1.0;
}

int AccelerateMouse(int val)
{
  if (!mouse_accelfactor)
    return val;

  if (val < 0)
    return -AccelerateMouse(-val);

  return M_DoubleToInt(pow((double) val, mouse_accelfactor));
}

int AccelerateAnalog(float val)
{
  if (!analog_accelfactor)
    return val;

  if (val < 0)
    return -AccelerateAnalog(-val);

  return M_DoubleToInt(pow((double) val, analog_accelfactor));
}

int mlooky = 0;

void e6y_G_Compatibility(void)
{
  deh_applyCompatibility();

  if (dsda_PlaybackName())
  {
    int i;
    dsda_arg_t* arg;

    //"2.4.8.2" -> 0x02040802
    arg = dsda_Arg(dsda_arg_emulate);
    if (arg->found)
    {
      unsigned int emulated_version = 0;
      int b[4], k = 1;
      memset(b, 0, sizeof(b));
      sscanf(arg->value.v_string, "%d.%d.%d.%d", &b[0], &b[1], &b[2], &b[3]);
      for (i = 3; i >= 0; i--, k *= 256)
      {
#ifdef RANGECHECK
        if (b[i] >= 256)
          I_Error("Wrong version number of package: %s", PACKAGE_VERSION);
#endif
        emulated_version += b[i] * k;
      }

      for (i = 0; i < PC_MAX; i++)
      {
        prboom_comp[i].state =
          (emulated_version >= prboom_comp[i].minver &&
           emulated_version <  prboom_comp[i].maxver);
      }
    }

    for (i = 0; i < PC_MAX; i++)
    {
      if (dsda_Flag(prboom_comp[i].arg_id))
        prboom_comp[i].state = true;
    }
  }

  P_CrossSubsector = P_CrossSubsector_PrBoom;
  if (!prboom_comp[PC_FORCE_LXDOOM_DEMO_COMPATIBILITY].state)
  {
    if (demo_compatibility)
      P_CrossSubsector = P_CrossSubsector_Doom;

    switch (compatibility_level)
    {
    case boom_compatibility_compatibility:
    case boom_201_compatibility:
    case boom_202_compatibility:
    case mbf_compatibility:
    case mbf21_compatibility:
      P_CrossSubsector = P_CrossSubsector_Boom;
    break;
    }
  }
}

const char* PathFindFileName(const char* pPath)
{
  const char* pT = pPath;

  if (pPath)
  {
    for ( ; *pPath; pPath++)
    {
      if ((pPath[0] == '\\' || pPath[0] == ':' || pPath[0] == '/')
        && pPath[1] &&  pPath[1] != '\\'  &&   pPath[1] != '/')
        pT = pPath + 1;
    }
  }

  return pT;
}

int levelstarttic;

int force_singletics_to = 0;

int HU_DrawDemoProgress(int force)
{
  static unsigned int last_update = 0;
  static int prev_len = -1;

  int len, tics_count, diff;
  unsigned int tick, max_period;

  if (gamestate == GS_DEMOSCREEN ||
      !demoplayback ||
      !dsda_IntConfig(dsda_config_hudadd_demoprogressbar))
    return false;

  tics_count = demo_tics_count * demo_playerscount;
  len = MIN(SCREENWIDTH, (int)((int64_t)SCREENWIDTH * dsda_PlaybackTics() / tics_count));

  if (!force)
  {
    max_period = ((tics_count - dsda_PlaybackTics() > 35 * demo_playerscount) ? 500 : 15);

    // Unnecessary updates of progress bar
    // can slow down demo skipping and playback
    tick = DG_GetTicksMs();
    if (tick - last_update < max_period)
      return false;
    last_update = tick;

    // Do not update progress bar if difference is small
    diff = len - prev_len;
    if (diff == 0 || diff == 1) // because of static prev_len
      return false;
  }

  prev_len = len;

  V_FillRect(0, 0, SCREENHEIGHT - 4, len - 0, 4, 4);
  if (len > 4)
    V_FillRect(0, 2, SCREENHEIGHT - 3, len - 4, 2, 0);

  return true;
}

#ifdef _WIN32
int GetFullPath(const char* FileName, const char* ext, char *Buffer, size_t BufferLength)
{
  int i, Result;
  char *p;
  char dir[PATH_MAX];

  for (i=0; i<3; i++)
  {
    switch(i)
    {
    case 0:
      M_getcwd(dir, sizeof(dir));
      break;
    case 1:
      if (!M_getenv("DOOMWADDIR"))
        continue;
      strcpy(dir, M_getenv("DOOMWADDIR"));
      break;
    case 2:
      strcpy(dir, I_DoomExeDir());
      break;
    }

    Result = SearchPath(dir,FileName,ext,BufferLength,Buffer,&p);
    if (Result)
      return Result;
  }

  return false;
}
#endif
