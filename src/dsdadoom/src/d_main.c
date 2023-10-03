/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2004 by
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
 *  DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
 *  plus functions to determine game mode (shareware, registered),
 *  parse command line parameters, configure game parameters (turbo),
 *  and call the startup functions.
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include "SDL_timer.h"

#ifdef _MSC_VER
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>

#include "doomdef.h"
#include "doomtype.h"
#include "doomstat.h"
#include "d_net.h"
#include "dstrings.h"
#include "sounds.h"
#include "z_zone.h"
#include "w_wad.h"
#include "s_sound.h"
#include "v_video.h"
#include "f_finale.h"
#include "f_wipe.h"
#include "m_file.h"
#include "m_misc.h"
#include "m_menu.h"
#include "i_main.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "p_setup.h"
#include "r_draw.h"
#include "r_main.h"
#include "r_fps.h"
#include "d_main.h"
#include "d_deh.h"  // Ty 04/08/98 - Externalizations
#include "lprintf.h"  // jff 08/03/98 - declaration of lprintf
#include "am_map.h"
#include "e6y.h"
#include "stricmp.h"

#include "dsda/args.h"
#include "dsda/configuration.h"
#include "dsda/demo.h"
#include "dsda/exdemo.h"
#include "dsda/features.h"
#include "dsda/global.h"
#include "dsda/save.h"
#include "dsda/data_organizer.h"
#include "dsda/map_format.h"
#include "dsda/mapinfo.h"
#include "dsda/mobjinfo.h"
#include "dsda/options.h"
#include "dsda/pause.h"
#include "dsda/playback.h"
#include "dsda/preferences.h"
#include "dsda/render_stats.h"
#include "dsda/settings.h"
#include "dsda/signal_context.h"
#include "dsda/skill_info.h"
#include "dsda/skip.h"
#include "dsda/sndinfo.h"
#include "dsda/time.h"
#include "dsda/utility.h"
#include "dsda/wad_stats.h"
#include "dsda/zipfile.h"
//#include "dsda/gl/render_scale.h"

#include "heretic/mn_menu.h"
#include "heretic/sb_bar.h"

#include "hexen/sn_sonix.h"

// NSM
#include "i_capture.h"

#include "i_glob.h"

#include "debug.h"

static void D_PageDrawer(void);

// jff 1/24/98 add new versions of these variables to remember command line
dboolean clnomonsters;   // checkparm of -nomonsters
dboolean clrespawnparm;  // checkparm of -respawn
dboolean clfastparm;     // checkparm of -fast
// jff 1/24/98 end definition of command line version of play mode switches

dboolean nomonsters;     // working -nomonsters
dboolean respawnparm;    // working -respawn
dboolean fastparm;       // working -fast

dboolean randomclass;

dboolean singletics = false; // debug flag to cancel adaptiveness

//jff 1/22/98 parms for disabling music and sound
dboolean nosfxparm;
dboolean nomusicparm;

//jff 4/18/98
extern dboolean inhelpscreens;
extern dboolean BorderNeedRefresh;

int     startskill;
int     startepisode;
int     startmap;
dboolean autostart;
FILE    *debugfile;
int     hires;

dboolean advancedemo;

static int finish;

//jff 4/19/98 list of standard IWAD names
const char *const standard_iwads[]=
{
  "doom2f.wad",
  "doom2.wad",
  "plutonia.wad",
  "tnt.wad",

  "doom.wad",
  "doom1.wad",
  "doomu.wad", /* CPhipps - alow doomu.wad */

  "freedoom2.wad", /* wart@kobold.org:  added freedoom for Fedora Extras */
  "freedoom1.wad",
  "freedm.wad",

  "hacx.wad",
  "chex.wad",
  "rekkrsa.wad",

  "bfgdoom2.wad",
  "bfgdoom.wad",

  "heretic.wad",
  "hexen.wad"
};
//e6y static
const int nstandard_iwads = sizeof standard_iwads/sizeof*standard_iwads;

/*
 * D_PostEvent - Event handling
 *
 * Called by I/O functions when an event is received.
 * Try event handlers for each code area in turn.
 * cph - in the true spirit of the Boom source, let the
 *  short ciruit operator madness begin!
 */

void D_PostEvent(event_t *ev)
{
  dsda_InputTrackEvent(ev);

  // Allow only sensible keys during skipping
  if (dsda_SkipMode())
  {
    if (dsda_InputActivated(dsda_input_quit))
    {
      // Immediate exit if quit key is pressed in skip mode
      I_SafeExit(0);
    }
    else
    {
      // use key is used for seeing the current frame
      if (
        !dsda_InputActivated(dsda_input_use) && !dsda_InputActivated(dsda_input_demo_skip) &&
        (ev->type == ev_keydown || ev->type == ev_keyup) // is this condition important?
      )
      {
        return;
      }
    }
  }

  if (M_Responder(ev))
    dsda_InputFlushTick(); // If the menu used the event, make it invisible
  else
    G_Responder(ev);
}

//
// D_Wipe
//
// CPhipps - moved the screen wipe code from D_Display to here
// The screens to wipe between are already stored, this just does the timing
// and screen updating

static void D_Wipe(void)
{
  dboolean done;
  int wipestart;
  int old_game_speed = 0;

  //e6y
  if (!dsda_RenderWipeScreen() || dsda_SkipWipe())
  {
    if (!raven)
      dsda_TrackFeature(uf_wipescreen);

    // If there's no screen wipe, we still need to refresh the status bar
    SB_Start();
    return;
  }

  if (dsda_GameSpeed() != 100 && dsda_WipeAtFullSpeed())
  {
    old_game_speed = dsda_GameSpeed();
    dsda_UpdateGameSpeed(100);
  }

  wipestart = dsda_GetTick() - 1;

  do
  {
    int nowtime, tics;
    do
    {
      I_uSleep(5000); // CPhipps - don't thrash cpu in this loop
      nowtime = dsda_GetTick();
      tics = nowtime - wipestart;
    }
    while (!tics);

#if 0
    // elim - Enable render-to-texture for GL so "melt" is rendered at same resolution as the game scene
    if (V_IsOpenGLMode())
    {
      dsda_GLLetterboxClear();
      dsda_GLStartMeltRenderTexture();
    }
#endif

    wipestart = nowtime;
    done = wipe_ScreenWipe(tics);

#if 0
    // elim - Render texture to screen
    if (V_IsOpenGLMode())
    {
      dsda_GLEndMeltRenderTexture();
    }
#endif

    M_Drawer();                   // menu is drawn even on top of wipes
    I_FinishUpdate();             // page flip or blit buffer

#if 0
    if (capturing_video && !dsda_SkipMode() && cap_wipescreen)
    {
      I_CaptureFrame();
    }
#endif
  }
  while (!done);

  if (old_game_speed)
  {
    dsda_UpdateGameSpeed(old_game_speed);
  }

  force_singletics_to = gametic + BACKUPTICS;
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t    wipegamestate = GS_DEMOSCREEN;
extern dboolean setsizeneeded;

static void D_DrawPause(void)
{
  if (dsda_PauseMode(PAUSE_BUILDMODE))
    return;

  V_BeginUIDraw();

  if (hexen)
  {
    if (!netgame)
    {
      V_DrawNamePatch(160, 5, 0, "PAUSED", CR_DEFAULT, VPT_STRETCH);
    }
    else
    {
      V_DrawNamePatch(160, 70, 0, "PAUSED", CR_DEFAULT, VPT_STRETCH);
    }
  }
  else if (heretic)
    MN_DrawPause();
  else
    V_DrawNamePatch((320 - V_NamePatchWidth("M_PAUSE")) / 2, 4, 0, "M_PAUSE", CR_DEFAULT, VPT_STRETCH);

  V_EndUIDraw();
}

static dboolean must_fill_back_screen;

void D_MustFillBackScreen(void)
{
  must_fill_back_screen = true;
}

void D_Display (fixed_t frac)
{
  static dboolean isborderstate        = false;
  static dboolean borderwillneedredraw = false;
  static gamestate_t oldgamestate = -1;
  dboolean wipe;
  dboolean viewactive = false, isborder = false;

  // e6y
  if (dsda_SkipMode())
  {
    if (HU_DrawDemoProgress(false))
      I_FinishUpdate();
    if (!dsda_InputActive(dsda_input_use))
      return;

#if 0
    if (V_IsOpenGLMode())
    {
      gld_PreprocessLevel();
    }
#endif
  }

  if (!dsda_SkipMode() || !dsda_InputActive(dsda_input_use))
    if (nodrawers)                    // for comparative timing / profiling
      return;

  if (!I_StartDisplay())
    return;

  if (setsizeneeded) {               // change the view size if needed
    R_ExecuteSetViewSize();
    oldgamestate = -1;            // force background redraw
  }

#if 0
  if (V_IsOpenGLMode() && !exclusive_fullscreen && !nodrawers)
    dsda_GLLetterboxClear();
#endif

  // save the current screen if about to wipe
  if ((wipe = (gamestate != wipegamestate)))
  {
    wipe_StartScreen();
    R_ResetViewInterpolation();
  }

  if (gamestate != GS_LEVEL) { // Not a level
    switch ((int)oldgamestate) {
    case -1:
    case GS_LEVEL:
      V_SetPalette(0); // cph - use default (basic) palette
    default:
      break;
    }

    V_BeginUIDraw();
    switch (gamestate) {
    case GS_INTERMISSION:
      WI_Drawer();
      break;
    case GS_FINALE:
      F_Drawer();
      break;
    case GS_DEMOSCREEN:
      D_PageDrawer();
      break;
    default:
      break;
    }
    V_EndUIDraw();
  }
  else { // In a level
    dboolean redrawborderstuff;

    // Work out if the player view is visible, and if there is a border
    viewactive = automap_off && !inhelpscreens;
    isborder = viewactive ? R_PartialView() : (!inhelpscreens && automap_active);

    if (oldgamestate != GS_LEVEL || must_fill_back_screen) {
      must_fill_back_screen = false;
      R_FillBackScreen ();    // draw the pattern into the back screen
      redrawborderstuff = isborder;
    } else {
      // CPhipps -
      // If there is a border, and either there was no border last time,
      // or the border might need refreshing, then redraw it.
      redrawborderstuff = isborder && (!isborderstate || borderwillneedredraw);
      // The border may need redrawing next time if the border surrounds the screen,
      // and there is a menu being displayed
      borderwillneedredraw = menuactive && isborder && viewactive;
      // e6y
      // I should do it because I call R_RenderPlayerView in all cases,
      // not only if viewactive is true
      borderwillneedredraw = borderwillneedredraw || automap_on;
    }

    if (redrawborderstuff /*|| V_IsOpenGLMode()*/) {
      // elim - Update viewport and scene offsets whenever the view is changed (user hits "-" or "+")
#if 0
      if (redrawborderstuff && V_IsOpenGLMode()) {
        dsda_GLSetRenderViewportParams();
      }
#endif

      R_DrawViewBorder();
    }

#if 0
    // elim - If we go from visible status bar to invisible status bar, update affected viewport params
    if (!isborder && isborderstate) {
      dsda_GLUpdateStatusBarVisible();
    }
#endif

    // e6y
    // Boom colormaps should be applied for everything in R_RenderPlayerView
    use_boom_cm=true;

    if (frac < 0)
      frac = I_GetTimeFrac();

    R_InterpolateView(&players[displayplayer], frac);

    DSDA_ADD_CONTEXT(sf_player_view);
    R_RenderPlayerView(&players[displayplayer]);
    DSDA_REMOVE_CONTEXT(sf_player_view);

    dsda_UpdateRenderStats();

    // e6y
    // but should NOT be applied for automap, statusbar and HUD
    use_boom_cm=false;
    frame_fixedcolormap = 0;

    if (automap_active)
    {
      AM_Drawer(false);
    }

    R_RestoreInterpolations();

    DSDA_ADD_CONTEXT(sf_status_bar);
    ST_Drawer(redrawborderstuff || BorderNeedRefresh);
    DSDA_REMOVE_CONTEXT(sf_status_bar);

    BorderNeedRefresh = false;
    if (V_IsSoftwareMode())
      R_DrawViewBorder();

    DSDA_ADD_CONTEXT(sf_hud);
    HU_Drawer();
    DSDA_REMOVE_CONTEXT(sf_hud);
  }

  isborderstate      = isborder;
  oldgamestate = wipegamestate = gamestate;

  // draw pause pic
  if (dsda_Paused() && (menuactive != mnact_full)) {
    D_DrawPause();
  }

  // menus go directly to the screen
  M_Drawer();          // menu is drawn even on top of everything

  FakeNetUpdate();     // send out any new accumulation

  HU_DrawDemoProgress(true); //e6y

  // normal update
  if (!wipe)
    I_FinishUpdate ();              // page flip or blit buffer
  else {
    // wipe update
    wipe_EndScreen();
    D_Wipe();
  }

  // e6y
  // Don't thrash cpu during pausing or if the window doesnt have focus
  if (dsda_CameraPaused() /*|| !window_focused*/) {
    I_uSleep(5000);
  }

  dsda_LimitFPS();

  I_EndDisplay();
}

static void QuitLoop(void) {
  finish = 1;
}

//
//  D_DoomLoop()
//
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//

static void D_DoomLoop(void)
{
  I_AtExit(QuitLoop, true, "QuitLoop", exit_priority_normal);

  if (dsda_IntConfig(dsda_config_startup_delay_ms) > 0)
    I_uSleep(dsda_IntConfig(dsda_config_startup_delay_ms) * 1000);

  for (finish = 0; !finish;)
  {
    if (I_Interrupted()) {
      I_SafeExit(0);
      break;
    }

    WasRenderedInTryRunTics = false;
    // frame syncronous IO operations
    I_StartFrame ();

    // process one or more tics
    if (singletics)
    {
      I_StartTic ();
      G_BuildTiccmd (&local_cmds[consoleplayer][maketic%BACKUPTICS]);
      if (advancedemo)
        D_DoAdvanceDemo ();
      M_Ticker ();
      G_Ticker ();
      gametic++;
      maketic++;
    }
    else
      TryRunTics (); // will run at least one tic

    // killough 3/16/98: change consoleplayer to displayplayer
    if (players[displayplayer].mo) // cph 2002/08/10
      S_UpdateSounds();// move positional sounds

    // Update display, next frame, with current state.
    if (!movement_smooth || !WasRenderedInTryRunTics || gamestate != wipegamestate)
    {
#if 0
      // NSM
      if (capturing_video && !dsda_SkipMode())
      {
        dboolean first = true;
        int cap_step = TICRATE * FRACUNIT / cap_fps;
        cap_frac += cap_step;
        while(cap_frac <= FRACUNIT)
        {
          isExtraDDisplay = !first;
          first = false;
          D_Display(cap_frac);
          isExtraDDisplay = false;
          I_CaptureFrame();
          cap_frac += cap_step;
        }
        cap_frac -= FRACUNIT + cap_step;
      }
      else
      {
        D_Display(-1);
      }
#endif
      D_Display(-1);
    }
  }
}

//
//  DEMO LOOP
//

static int  demosequence;         // killough 5/2/98: made static
static int  pagetic;
static const char *pagename; // CPhipps - const
dboolean bfgedition = 0;

//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker(void)
{
  if (--pagetic < 0)
    D_AdvanceDemo();
}

//
// D_PageDrawer
//
static void D_PageDrawer(void)
{
  if (raven)
  {
    V_DrawRawScreen(pagename);
    if (demosequence == 1)
    {
      V_DrawNamePatch(4, 160, 0, "ADVISOR", CR_DEFAULT, VPT_STRETCH);
    }
    return;
  }

  // proff/nicolas 09/14/98 -- now stretchs bitmaps to fullscreen!
  // CPhipps - updated for new patch drawing
  // proff - added M_DrawCredits
  if (pagename)
  {
    // e6y: wide-res
    V_ClearBorder();
    V_DrawNamePatch(0, 0, 0, pagename, CR_DEFAULT, VPT_STRETCH);
  }
  else
    M_DrawCredits();
}

//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void)
{
  advancedemo = true;
}

/* killough 11/98: functions to perform demo sequences
 * cphipps 10/99: constness fixes
 */

static void D_SetPageName(const char *name)
{
  if ((bfgedition) && name && !strncmp(name,"TITLEPIC",8))
    pagename = "DMENUPIC";
  else
    pagename = name;
}

void D_SetPage(const char* name, int tics, int music)
{
  if (music)
    S_StartMusic(music);

  if (tics)
    pagetic = tics;

  D_SetPageName(name);
}

static void D_DrawTitle1(const char *name)
{
  D_SetPage(name, TICRATE * 170 / 35, mus_intro);
}

static void D_DrawTitle2(const char *name)
{
  D_SetPage(name, 0, mus_dm2ttl);
}

/* killough 11/98: tabulate demo sequences
 */

extern const demostate_t (*demostates)[4];

const demostate_t doom_demostates[][4] =
{
  {
    {D_DrawTitle1, "TITLEPIC"},
    {D_DrawTitle1, "TITLEPIC"},
    {D_DrawTitle2, "TITLEPIC"},
    {D_DrawTitle1, "TITLEPIC"},
  },

  {
    {G_DeferedPlayDemo, "demo1"},
    {G_DeferedPlayDemo, "demo1"},
    {G_DeferedPlayDemo, "demo1"},
    {G_DeferedPlayDemo, "demo1"},
  },

  {
    {D_SetPageName, NULL},
    {D_SetPageName, NULL},
    {D_SetPageName, NULL},
    {D_SetPageName, NULL},
  },

  {
    {G_DeferedPlayDemo, "demo2"},
    {G_DeferedPlayDemo, "demo2"},
    {G_DeferedPlayDemo, "demo2"},
    {G_DeferedPlayDemo, "demo2"},
  },

  {
    {D_SetPageName, "HELP2"},
    {D_SetPageName, "HELP2"},
    {D_SetPageName, "CREDIT"},
    {D_DrawTitle1,  "TITLEPIC"},
  },

  {
    {G_DeferedPlayDemo, "demo3"},
    {G_DeferedPlayDemo, "demo3"},
    {G_DeferedPlayDemo, "demo3"},
    {G_DeferedPlayDemo, "demo3"},
  },

  {
    {NULL},
    {NULL},
    // e6y
    // Both Plutonia and TNT are commercial like Doom2,
    // but in difference from  Doom2, they have demo4 in demo cycle.
    {G_DeferedPlayDemo, "demo4"},
    {D_SetPageName, "CREDIT"},
  },

  {
    {NULL},
    {NULL},
    {NULL},
    {G_DeferedPlayDemo, "demo4"},
  },

  {
    {NULL},
    {NULL},
    {NULL},
    {NULL},
  }
};

/*
 * This cycles through the demo sequences.
 * killough 11/98: made table-driven
 */

void D_DoAdvanceDemo(void)
{
  players[consoleplayer].playerstate = PST_LIVE;  /* not reborn */
  advancedemo = false;
  dsda_ResetPauseMode();
  gameaction = ga_nothing;

  pagetic = TICRATE * 11;         /* killough 11/98: default behavior */
  gamestate = GS_DEMOSCREEN;

  if (netgame && !demoplayback)
    demosequence = 0;
  else if (!demostates[++demosequence][gamemode].func)
    demosequence = 0;

  // do not even attempt to play DEMO4 if it is not available
  if (demosequence == 6 && gamemode == commercial && !W_LumpNameExists("demo4"))
    demosequence = 0;

  demostates[demosequence][gamemode].func(demostates[demosequence][gamemode].name);
}

//
// D_StartTitle
//
void D_StartTitle (void)
{
  gameaction = ga_nothing;
  in_game = false;
  demosequence = -1;
  D_AdvanceDemo();
}

//
// D_AddFile
//
// Rewritten by Lee Killough
//
// Ty 08/29/98 - add source parm to indicate where this came from
// CPhipps - static, const char* parameter
//         - source is an enum
//         - modified to allocate & use new wadfiles array
void D_AddFile (const char *file, wad_source_t source)
{
  char *gwa_filename=NULL;
  int len;

  // There can only be one iwad source!
  if (source == source_iwad)
  {
    int i;

    for (i = 0; i < numwadfiles; ++i)
      if (wadfiles[i].src == source_iwad)
        wadfiles[i].src = source_skip;
  }

  wadfiles = Z_Realloc(wadfiles, sizeof(*wadfiles)*(numwadfiles+1));
  wadfiles[numwadfiles].name =
    AddDefaultExtension(strcpy(Z_Malloc(strlen(file)+5), file), ".wad");
  wadfiles[numwadfiles].src = source; // Ty 08/29/98
  wadfiles[numwadfiles].handle = 0;

  // No Rest For The Living
  len=strlen(wadfiles[numwadfiles].name);
  if (len>=9 && !strnicmp(wadfiles[numwadfiles].name+len-9,"nerve.wad",9))
    gamemission = pack_nerve;

  numwadfiles++;
  // proff: automatically try to add the gwa files
  // proff - moved from w_wad.c
  gwa_filename=AddDefaultExtension(strcpy(Z_Malloc(strlen(file)+5), file), ".wad");
  if (dsda_HasFileExt(gwa_filename, ".wad"))
  {
    char *ext;
    ext = &gwa_filename[strlen(gwa_filename)-4];
    ext[1] = 'g'; ext[2] = 'w'; ext[3] = 'a';
    wadfiles = Z_Realloc(wadfiles, sizeof(*wadfiles)*(numwadfiles+1));
    wadfiles[numwadfiles].name = gwa_filename;
    wadfiles[numwadfiles].src = source_pwad; // Ty 08/29/98
    wadfiles[numwadfiles].handle = 0;
    numwadfiles++;
  }
}

// killough 10/98: support -dehout filename
// cph - made const, don't cache results
//e6y static
const char *D_dehout(void)
{
  dsda_arg_t* arg;

  arg = dsda_Arg(dsda_arg_dehout);

  return arg->found ? arg->value.v_string : NULL;
}

//
// CheckIWAD
//
// Verify a file is indeed tagged as an IWAD
// Scan its lumps for levelnames and return gamemode as indicated
// Detect missing wolf levels in DOOM II
//
// The filename to check is passed in iwadname, the gamemode detected is
// returned in gmode, hassec returns the presence of secret levels
//
// jff 4/19/98 Add routine to test IWAD for validity and determine
// the gamemode from it. Also note if DOOM II, whether secret levels exist
// CPhipps - const char* for iwadname, made static
//e6y static
void CheckIWAD(const char *iwadname,GameMode_t *gmode,dboolean *hassec)
{
  if (M_ReadAccess(iwadname))
  {
    int ud=0,rg=0,sw=0,cm=0,sc=0,hx=0;
    dg_file_t *fp;

    // Identify IWAD correctly
    if ((fp = M_OpenFile(iwadname, "rb")))
    {
      wadinfo_t header;

      // read IWAD header
      if (DG_read(fp, &header, sizeof(header)) == sizeof(header))
      {
        size_t length;
        filelump_t *fileinfo;

        if (strncmp(header.identification, "IWAD", 4)) // missing IWAD tag in header
        {
          lprintf(LO_WARN,"CheckIWAD: IWAD tag %s not present\n", iwadname);
        }

        // read IWAD directory
        header.numlumps = LittleLong(header.numlumps);
        header.infotableofs = LittleLong(header.infotableofs);
        length = header.numlumps;
        fileinfo = Z_Malloc(length*sizeof(filelump_t));
        if (DG_seek(fp, header.infotableofs, DG_SEEK_SET) ||
            DG_read(fp, fileinfo, sizeof(filelump_t) * length) != sizeof(filelump_t) * length)
        {
          DG_close(fp);
          I_Error("CheckIWAD: failed to read directory %s",iwadname);
        }

        // scan directory for levelname lumps
        while (length--)
        {
          if (fileinfo[length].name[0] == 'E' &&
              fileinfo[length].name[2] == 'M' &&
              fileinfo[length].name[4] == 0)
          {
            if (fileinfo[length].name[1] == '4')
              ++ud;
            else if (fileinfo[length].name[1] == '3')
              ++rg;
            else if (fileinfo[length].name[1] == '2')
              ++rg;
            else if (fileinfo[length].name[1] == '1')
              ++sw;
          }
          else if (fileinfo[length].name[0] == 'M' &&
                    fileinfo[length].name[1] == 'A' &&
                    fileinfo[length].name[2] == 'P' &&
                    fileinfo[length].name[5] == 0)
          {
            ++cm;
            if (fileinfo[length].name[3] == '3')
              if (fileinfo[length].name[4] == '1' ||
                  fileinfo[length].name[4] == '2')
                ++sc;
          }

          if (!strncmp(fileinfo[length].name,"DMENUPIC",8))
            bfgedition++;
          if (!strncmp(fileinfo[length].name,"HACX",4))
            hx++;
        }
        Z_Free(fileinfo);

      }

      DG_close(fp);
    }
    else // error from open call
      I_Error("CheckIWAD: Can't open IWAD %s", iwadname);

    // Determine game mode from levels present
    // Must be a full set for whichever mode is present
    // Lack of wolf-3d levels also detected here

    *gmode = indetermined;
    *hassec = false;
    if (cm>=30 || (cm>=20 && hx))
    {
      *gmode = commercial;
      *hassec = sc>=2;
    }
    else if (ud>=9)
      *gmode = retail;
    else if (rg>=18)
      *gmode = registered;
    else if (sw>=9)
      *gmode = shareware;
  }
  else // error from access call
    I_Error("CheckIWAD: IWAD %s not readable", iwadname);
}

//
// AddIWAD
//
void AddIWAD(const char *iwad)
{
  size_t i;

  if (!(iwad && *iwad))
    return;

  //jff 9/3/98 use logical output routine
  lprintf(LO_DEBUG, "IWAD found: %s\n", iwad); //jff 4/20/98 print only if found
  CheckIWAD(iwad,&gamemode,&haswolflevels);

  /* jff 8/23/98 set gamemission global appropriately in all cases
  * cphipps 12/1999 - no version output here, leave that to the caller
  */
  i = strlen(iwad);

  if (i >= 11 && !strnicmp(iwad + i - 11, "heretic.wad", 11))
  {
    if (!dsda_Flag(dsda_arg_heretic))
      dsda_UpdateFlag(dsda_arg_heretic, true);
  }

  if (i >= 9 && !strnicmp(iwad + i - 9, "hexen.wad", 9))
  {
    if (!dsda_Flag(dsda_arg_hexen))
      dsda_UpdateFlag(dsda_arg_hexen, true);

    gamemode = commercial;
    haswolflevels = false;
  }

  switch(gamemode)
  {
    case retail:
    case registered:
    case shareware:
      gamemission = doom;
      if (i>=8 && !strnicmp(iwad+i-8,"chex.wad",8))
        gamemission = chex;
      break;
    case commercial:
      gamemission = doom2;
      if (i>=10 && !strnicmp(iwad+i-10,"doom2f.wad",10))
        language=french;
      else if (i>=7 && !strnicmp(iwad+i-7,"tnt.wad",7))
        gamemission = pack_tnt;
      else if (i>=12 && !strnicmp(iwad+i-12,"plutonia.wad",12))
        gamemission = pack_plut;
      else if (i>=8 && !strnicmp(iwad+i-8,"hacx.wad",8))
        gamemission = hacx;
      break;
    default:
      gamemission = none;
      break;
  }
  if (gamemode == indetermined)
    //jff 9/3/98 use logical output routine
    lprintf(LO_WARN,"Unknown Game Version, may not work\n");
  D_AddFile(iwad,source_iwad);
}

/*
 * FindIWADFIle
 *
 * Search for one of the standard IWADs
 * CPhipps  - static, proper prototype
 *    - 12/1999 - rewritten to use I_FindFile
 */
static inline dboolean CheckExeSuffix(const char *suffix)
{
  extern char **dsda_argv;

  char *dash;

  if ((dash = strrchr(dsda_argv[0], '-')))
    if (!stricmp(dash, suffix))
      return true;

  return false;
}

static char *FindIWADFile(void)
{
  int   i;
  dsda_arg_t* arg;
  char  * iwad  = NULL;

  arg = dsda_Arg(dsda_arg_iwad);
  if (arg->found)
  {
    iwad = I_FindWad(arg->value.v_string);
  }
  else
  {
    if (dsda_Flag(dsda_arg_heretic) || CheckExeSuffix("-heretic"))
      return I_FindWad("heretic.wad");
    else if (dsda_Flag(dsda_arg_hexen) || CheckExeSuffix("-hexen"))
      return I_FindWad("hexen.wad");

    for (i=0; !iwad && i<nstandard_iwads; i++)
      iwad = I_FindWad(standard_iwads[i]);
  }
  return iwad;
}

static dboolean FileMatchesIWAD(const char *name)
{
  int i;
  int name_length;

  name_length = strlen(name);
  for (i = 0; i < nstandard_iwads; ++i)
  {
    int iwad_length;

    iwad_length = strlen(standard_iwads[i]);
    if (
      name_length >= iwad_length &&
      !stricmp(name + name_length - iwad_length, standard_iwads[i])
    )
      return true;
  }

  return false;
}

//
// IdentifyVersion
//
// Set the location of the defaults file and the savegame root
// Locate and validate an IWAD file
// Determine gamemode from the IWAD
//
// supports IWADs with custom names. Also allows the -iwad parameter to
// specify which iwad is being searched for if several exist in one dir.
// The -iwad parm may specify:
//
// 1) a specific pathname, which must exist (.wad optional)
// 2) or a directory, which must contain a standard IWAD,
// 3) or a filename, which must be found in one of the standard places:
//   a) current dir,
//   b) exe dir
//   c) $DOOMWADDIR
//   d) or $HOME
//
// jff 4/19/98 rewritten to use a more advanced search algorithm

static void IdentifyVersion (void)
{
  char *iwad;

  // why is this here?
  dsda_InitDataDir();
  dsda_InitSaveDir();

  // locate the IWAD and determine game mode from it

  iwad = FindIWADFile();

  if (iwad && *iwad)
  {
    AddIWAD(iwad);
    Z_Free(iwad);
  }
  else
    I_Error("IdentifyVersion: IWAD not found\n");
}

//
// DoLooseFiles
//
// Take any file names on the command line before the first switch parm
// and insert the appropriate -file, -deh or -playdemo switch in front
// of them.
//
// e6y
// Fixed crash if numbers of wads/lmps/dehs is greater than 100
// Fixed bug when length of argname is smaller than 3
// Refactoring of the code to avoid use the static arrays
// The logic of DoLooseFiles has been rewritten in more optimized style
// MAXARGVS has been removed.

static void DoLooseFiles(void)
{
  extern int dsda_argc;
  extern char **dsda_argv;

  int i, k;
  const int loose_wad_index = 0;

  struct {
    const char *ext;
    dsda_arg_identifier_t arg_id;
  } looses[] = {
    { ".wad", dsda_arg_file },
    { ".zip", dsda_arg_file },
    { ".lmp", dsda_arg_playdemo },
    { ".deh", dsda_arg_deh },
    { ".bex", dsda_arg_deh },
    // assume wad if no extension or length of the extention is not equal to 3
    // must be last entry
    { "", dsda_arg_file },
    { 0 }
  };

  for (i = 1; i < dsda_argc; i++)
  {
    size_t arglen, extlen;

    if (*dsda_argv[i] == '-') break;  // quit at first switch

    // so now we must have a loose file.  Find out what kind and store it.
    arglen = strlen(dsda_argv[i]);

    for (k = 0; looses[k].ext; ++k)
    {
      extlen = strlen(looses[k].ext);
      if (arglen >= extlen && !stricmp(&dsda_argv[i][arglen - extlen], looses[k].ext))
      {
        // If a wad is an iwad, we don't want to send it to -file
        if (k == loose_wad_index && FileMatchesIWAD(dsda_argv[i]))
        {
          dsda_UpdateStringArg(dsda_arg_iwad, dsda_argv[i]);
          break;
        }

        dsda_AppendStringArg(looses[k].arg_id, dsda_argv[i]);
        break;
      }
    }
  }
}

const char *port_wad_file;

// CPhipps - misc screen stuff
int desired_screenwidth, desired_screenheight;

// Calculate the path to the directory for autoloaded WADs/DEHs.
// Creates the directory as necessary.

static char *autoload_path = NULL;

static char *GetAutoloadDir(const char *iwadname, dboolean createdir)
{
    char *result;
    int len;

    if (autoload_path == NULL)
    {
        const char* exedir = I_DoomExeDir();
        len = snprintf(NULL, 0, "%s/autoload", exedir);
        autoload_path = Z_Malloc(len+1);
        snprintf(autoload_path, len+1, "%s/autoload", exedir);
    }

    M_MakeDir(autoload_path, false);

    len = snprintf(NULL, 0, "%s/%s", autoload_path, iwadname);
    result = Z_Malloc(len+1);
    snprintf(result, len+1, "%s/%s", autoload_path, iwadname);

    if (createdir)
    {
      M_MakeDir(result, false);
    }

    return result;
}

const char *IWADBaseName(void)
{
  int i;

  for (i = 0; i < numwadfiles; i++)
  {
    if (wadfiles[i].src == source_iwad)
      break;
  }

  if (i == numwadfiles)
    I_Error("IWADBaseName: IWAD not found\n");

  return dsda_BaseName(wadfiles[i].name);
}

typedef struct {
  char **list;
  int count;
  int allocated_count;
} deh_queue_t;

static deh_queue_t autoload_deh_all_queue;
static deh_queue_t autoload_deh_game_queue;
static deh_queue_t autoload_deh_iwad_queue;
static deh_queue_t *autoload_deh_pwad_queue;
static int autoload_deh_pwad_count;

static void D_QueueAutoloadDeh(deh_queue_t *queue, const char *name)
{
  int old_count;

  old_count = queue->count;
  ++queue->count;

  if (queue->count > queue->allocated_count)
  {
    if (!queue->allocated_count)
      queue->allocated_count = 4;

    while (queue->count > queue->allocated_count)
      queue->allocated_count *= 2;

    queue->list = Z_Realloc(queue->list, queue->allocated_count * sizeof(*queue->list));
    memset(&queue->list[old_count], 0, (queue->allocated_count - old_count) * sizeof(*queue->list));
  }

  queue->list[queue->count - 1] = Z_Strdup(name);
}

static void D_ProcessDehAutoloadQueue(deh_queue_t *queue)
{
  int i;

  for (i = 0; i < queue->count; ++i)
  {
    ProcessDehFile(queue->list[i], D_dehout(), 0);
    Z_Free(queue->list[i]);
  }

  Z_Free(queue->list);
}

// Load all WAD files from the given directory.

static void LoadWADsAtPath(const char *path, wad_source_t source)
{
    glob_t *glob;
    const char *filename;

    glob = I_StartMultiGlob(path, GLOB_FLAG_NOCASE|GLOB_FLAG_SORTED,
                            "*.wad", "*.lmp", NULL);
    for (;;)
    {
        filename = I_NextGlob(glob);
        if (filename == NULL)
        {
            break;
        }
        D_AddFile(filename, source);
    }

    I_EndGlob(glob);
}

static void LoadDehackedFilesAtPath(const char *path, dboolean defer_loading, deh_queue_t *deh_queue)
{
    const char *filename;
    glob_t *glob;

    glob = I_StartMultiGlob(path, GLOB_FLAG_NOCASE|GLOB_FLAG_SORTED,
                            "*.deh", "*.bex", NULL);
    for (;;)
    {
        filename = I_NextGlob(glob);
        if (filename == NULL)
        {
            break;
        }

        if (deh_queue)
        {
            D_QueueAutoloadDeh(deh_queue, filename);
        }
        else if (defer_loading)
        {
            dsda_AppendStringArg(dsda_arg_deh, filename);
        }
        else
        {
            ProcessDehFile(filename, D_dehout(), 0);
        }
    }

    I_EndGlob(glob);
}

static void D_AddZip(const char* zipped_file_name, wad_source_t source, deh_queue_t *deh_queue)
{
  char* full_zip_path;
  const char* temporary_directory;

  full_zip_path = I_RequireZip(zipped_file_name);
  temporary_directory = dsda_UnzipFile(full_zip_path);

  LoadWADsAtPath(temporary_directory, source);
  LoadDehackedFilesAtPath(temporary_directory, true, deh_queue);

  Z_Free(full_zip_path);
}

static void LoadZIPsAtPath(const char *path, wad_source_t source, deh_queue_t *deh_queue)
{
    glob_t *glob;
    const char *filename;

    glob = I_StartMultiGlob(path, GLOB_FLAG_NOCASE|GLOB_FLAG_SORTED,
                            "*.zip", NULL);
    for (;;)
    {
        filename = I_NextGlob(glob);
        if (filename == NULL)
        {
            break;
        }
        D_AddZip(filename, source, deh_queue);
    }

    I_EndGlob(glob);
}

static const char *D_AutoLoadGameBase()
{
  return hexen ? "hexen-all" :
         heretic ? "heretic-all" :
         "doom-all";
}

#define ALL_AUTOLOAD "all-all"

// auto-loading of .wad files.

void D_AutoloadIWadDir()
{
  char *autoload_dir;

  // common auto-loaded files for all games
  autoload_dir = GetAutoloadDir(ALL_AUTOLOAD, true);
  LoadWADsAtPath(autoload_dir, source_auto_load);
  LoadZIPsAtPath(autoload_dir, source_auto_load, &autoload_deh_all_queue);
  Z_Free(autoload_dir);

  // common auto-loaded files for the game
  autoload_dir = GetAutoloadDir(D_AutoLoadGameBase(), true);
  LoadWADsAtPath(autoload_dir, source_auto_load);
  LoadZIPsAtPath(autoload_dir, source_auto_load, &autoload_deh_game_queue);
  Z_Free(autoload_dir);

  // auto-loaded files per IWAD
  autoload_dir = GetAutoloadDir(IWADBaseName(), true);
  LoadWADsAtPath(autoload_dir, source_auto_load);
  LoadZIPsAtPath(autoload_dir, source_auto_load, &autoload_deh_iwad_queue);
  Z_Free(autoload_dir);
}

static void D_AutoloadPWadDir()
{
  int i;

  autoload_deh_pwad_count = numwadfiles;
  autoload_deh_pwad_queue = Z_Calloc(autoload_deh_pwad_count, sizeof(*autoload_deh_pwad_queue));

  for (i = 0; i < numwadfiles; ++i)
    if (wadfiles[i].src == source_pwad)
    {
      char *autoload_dir;
      autoload_dir = GetAutoloadDir(dsda_BaseName(wadfiles[i].name), false);
      LoadWADsAtPath(autoload_dir, source_auto_load);
      LoadZIPsAtPath(autoload_dir, source_auto_load, &autoload_deh_pwad_queue[i]);
      Z_Free(autoload_dir);
    }
}

// auto-loading of .deh files.

static void D_AutoloadDehIWadDir()
{
  char *autoload_dir;

  // common auto-loaded files for all games
  autoload_dir = GetAutoloadDir(ALL_AUTOLOAD, true);
  LoadDehackedFilesAtPath(autoload_dir, false, NULL);
  D_ProcessDehAutoloadQueue(&autoload_deh_all_queue);
  Z_Free(autoload_dir);

  // common auto-loaded files for the game
  autoload_dir = GetAutoloadDir(D_AutoLoadGameBase(), true);
  LoadDehackedFilesAtPath(autoload_dir, false, NULL);
  D_ProcessDehAutoloadQueue(&autoload_deh_game_queue);
  Z_Free(autoload_dir);

  // auto-loaded files per IWAD
  autoload_dir = GetAutoloadDir(IWADBaseName(), true);
  LoadDehackedFilesAtPath(autoload_dir, false, NULL);
  D_ProcessDehAutoloadQueue(&autoload_deh_iwad_queue);
  Z_Free(autoload_dir);
}

static void D_AutoloadDehPWadDir()
{
  int i;
  for (i = 0; i < numwadfiles; ++i)
    if (wadfiles[i].src == source_pwad)
    {
      char *autoload_dir;
      autoload_dir = GetAutoloadDir(dsda_BaseName(wadfiles[i].name), false);
      LoadDehackedFilesAtPath(autoload_dir, false, NULL);
      if (i < autoload_deh_pwad_count)
        D_ProcessDehAutoloadQueue(&autoload_deh_pwad_queue[i]);
      Z_Free(autoload_dir);
    }

  Z_Free(autoload_deh_pwad_queue);
}

int warpepisode = -1;
int warpmap = -1;

static void HandleWarp(void)
{
  dsda_arg_t* arg;

  arg = dsda_Arg(dsda_arg_warp);

  if (arg->found)
  {
    autostart = true; // Ty 08/29/98 - move outside the decision tree

    dsda_ResolveWarp(arg->value.v_int_array, arg->count, &warpepisode, &warpmap);

    if (warpmap == -1)
      dsda_FirstMap(&warpepisode, &warpmap);

    startmap = warpmap;
    startepisode = warpepisode;
  }
}

static void HandleClass(void)
{
  int p;
  dsda_arg_t* arg;
  int player_class = PCLASS_FIGHTER;

  if (!hexen) return;

  arg = dsda_Arg(dsda_arg_class);
  if (arg->found)
    player_class = arg->value.v_int + PCLASS_FIGHTER;

  if (
    player_class != PCLASS_FIGHTER &&
    player_class != PCLASS_CLERIC &&
    player_class != PCLASS_MAGE
  )
    player_class = PCLASS_FIGHTER;

  PlayerClass[0] = player_class;
  for (p = 1; p < MAX_MAXPLAYERS; p++)
    PlayerClass[p] = PCLASS_FIGHTER;

  randomclass = dsda_Flag(dsda_arg_randclass);
}

static void HandlePlayback(void)
{
  const char* file;

  file = dsda_ParsePlaybackOptions();

  if (!file)
    return;

  dsda_LoadExDemo(file);
}

const char* doomverstr = "Unknown";

static void EvaluateDoomVerStr(void)
{
  if (heretic)
  {
    doomverstr = "Heretic";
  }
  else if (hexen)
  {
    doomverstr = "Hexen";
  }
  else
  {
    switch ( gamemode )
    {
      case retail:
        switch (gamemission)
        {
          case chex:
            doomverstr = "Chex(R) Quest";
            break;
          default:
            doomverstr = "The Ultimate DOOM";
            break;
        }
        break;
      case shareware:
        doomverstr = "DOOM Shareware";
        break;
      case registered:
        doomverstr = "DOOM Registered";
        break;
      case commercial:  // Ty 08/27/98 - fixed gamemode vs gamemission
        switch (gamemission)
        {
          case pack_plut:
            doomverstr = "Final DOOM - The Plutonia Experiment";
            break;
          case pack_tnt:
            doomverstr = "Final DOOM - TNT: Evilution";
            break;
          case hacx:
            doomverstr = "HACX - Twitch 'n Kill";
            break;
          default:
            doomverstr = "DOOM 2: Hell on Earth";
            break;
        }
        break;
      default:
        doomverstr = "Public DOOM";
        break;
    }
  }

  if (bfgedition)
  {
    char *tempverstr;
    const char bfgverstr[]=" (BFG Edition)";
    tempverstr = Z_Malloc(sizeof(char) * (strlen(doomverstr)+strlen(bfgverstr)+1));
    strcpy (tempverstr, doomverstr);
    strcat (tempverstr, bfgverstr);
    doomverstr = Z_Strdup (tempverstr);
    Z_Free (tempverstr);
  }

#if 0
  /* cphipps - the main display. This shows the copyright and game type */
  lprintf(LO_INFO,
          "%s is released under the GNU General Public license v2.0.\n"
          "You are welcome to redistribute it under certain conditions.\n"
          "It comes with ABSOLUTELY NO WARRANTY. See the file COPYING for details.\n\n",
          PACKAGE_NAME);
#endif

  DG_debug(DEBUG_INFO, "Playing: %s", doomverstr);
}

//
// D_DoomMainSetup
//
// CPhipps - the old contents of D_DoomMain, but moved out of the main
//  line of execution so its stack space can be freed

static void D_DoomMainSetup(void)
{
  int p;
  dsda_arg_t *arg;
  dboolean autoload;

  setbuf(stdout,NULL);

  if (dsda_Flag(dsda_arg_help))
  {
    dsda_PrintArgHelp();
    I_SafeExit(0);
  }

  // figgi 09/18/00-- added switch to force classic bsp nodes
  if (dsda_Flag(dsda_arg_forceoldbsp))
    forceOldBsp = true;

  DoLooseFiles();  // Ty 08/29/98 - handle "loose" files on command line

  IdentifyVersion();

  dsda_InitGlobal();

  // e6y: DEH files preloaded in wrong order
  // http://sourceforge.net/tracker/index.php?func=detail&aid=1418158&group_id=148658&atid=772943
  // The dachaked stuff has been moved below an autoload

  // jff 1/24/98 set both working and command line value of play parms
  nomonsters = clnomonsters = dsda_Flag(dsda_arg_nomonsters);
  respawnparm = clrespawnparm = dsda_Flag(dsda_arg_respawn);
  fastparm = clfastparm = dsda_Flag(dsda_arg_fast);
  // jff 1/24/98 end of set to both working and command line value

  if (dsda_Flag(dsda_arg_altdeath))
    deathmatch = 2;
  else if (dsda_Flag(dsda_arg_deathmatch))
    deathmatch = 1;

  modifiedgame = false;

  // get skill / episode / map from parms

  startskill = dsda_IntConfig(dsda_config_default_skill) - 1;
  startepisode = 1;
  startmap = 1;
  autostart = false;

  arg = dsda_Arg(dsda_arg_skill);
  if (arg->found)
  {
    startskill = arg->value.v_int - 1;
    autostart = true;
  }

  arg = dsda_Arg(dsda_arg_episode);
  if (arg->found)
  {
    startepisode = arg->value.v_int;
    startmap = 1;
    autostart = true;
  }

  HandleClass();

  arg = dsda_Arg(dsda_arg_timer);
  if (arg->found && deathmatch)
  {
    int time = arg->value.v_int;
    //jff 9/3/98 use logical output routine
    lprintf(LO_INFO,"Levels will end after %d minute%s.\n", time, time>1 ? "s" : "");
  }

  //jff 1/22/98 add command line parms to disable sound and music
  {
    int nosound = dsda_Flag(dsda_arg_nosound);
    nomusicparm = nosound || dsda_Flag(dsda_arg_nomusic);
    nosfxparm   = nosound || dsda_Flag(dsda_arg_nosfx);
  }
  //jff end of sound/music command line parms

  // killough 3/2/98: allow -nodraw generally
  nodrawers = dsda_Flag(dsda_arg_nodraw);

  // init subsystems

  G_ReloadDefaults();    // killough 3/4/98: set defaults just loaded.
  // jff 3/24/98 this sets startskill if it was -1

  // proff 04/05/2000: for GL-specific switches
  //gld_InitCommandLine();

  //jff 9/3/98 use logical output routine
  lprintf(LO_DEBUG, "V_Init: allocate screens.\n");
  V_Init();

  hires = 0;
  arg = dsda_Arg(dsda_arg_hires);
  if (arg->found) {
    hires = 1;
  }

  //e6y: Calculate the screen resolution and init all buffers
  I_InitScreenResolution();

  //e6y: some stuff from command-line should be initialised before ProcessDehFile()
  e6y_InitCommandLine();

  // Automatic pistol start when advancing from one level to the next.
  pistolstart = dsda_Flag(dsda_arg_pistolstart);

  // CPhipps - autoloading of wads
  autoload = !dsda_Flag(dsda_arg_noautoload);

  D_AddFile(port_wad_file, source_auto_load);

  HandlePlayback(); // must come before autoload: may detect iwad in footer

  EvaluateDoomVerStr(); // must come after HandlePlayback (may change iwad)

  // add wad files from autoload directory before wads from -file parameter
  if (autoload)
    D_AutoloadIWadDir();

  // add any files specified on the command line with -file wadfile
  // to the wad list

  if ((arg = dsda_Arg(dsda_arg_file))->found)
  {
    int file_i;
    // the parms after p are wadfile/lump names,
    // until end of parms or another - preceded parm
    modifiedgame = true;            // homebrew levels

    for (file_i = 0; file_i < arg->count; ++file_i)
    {
      const char* file_name;
      char *file = NULL;

      file_name = arg->value.v_string_array[file_i];

      if (!dsda_FileExtension(file_name))
      {
        const char *extensions[] = { ".wad", ".lmp", ".zip", ".deh", ".bex", NULL };

        file = I_RequireAnyFile(file_name, extensions);
        file_name = file;
      }

      if (dsda_HasFileExt(file_name, ".deh") || dsda_HasFileExt(file_name, ".bex"))
      {
        dsda_AppendStringArg(dsda_arg_deh, file_name);
      }
      else if (dsda_HasFileExt(file_name, ".zip"))
      {
        D_AddZip(file_name, source_pwad, NULL);
      }
      else if (dsda_HasFileExt(file_name, ".wad") || dsda_HasFileExt(file_name, ".lmp"))
      {
        if (!file)
          file = I_RequireWad(file_name);

        D_AddFile(file, source_pwad);
      }
      else
      {
        I_Error("File type \"%s\" is not supported", dsda_FileExtension(file_name));
      }

      Z_Free(file);
    }
  }

  // add wad files from autoload PWAD directories
  if (autoload)
    D_AutoloadPWadDir();

  D_InitFakeNetGame();

  //jff 9/3/98 use logical output routine
  lprintf(LO_DEBUG, "W_Init: Init WADfiles.\n");
  W_Init(); // CPhipps - handling of wadfiles init changed

  if (hexen)
  {
    if (!W_LumpNameExists("MAP05"))
    {
      I_Error("The Hexen IWAD shareware is not supported.");
      gamemode = shareware;
      g_maxplayers = 4;
    }
    else if (!W_LumpNameExists("CLUS1MSG"))
    {
      I_Error("The Hexen v1.0 IWAD is not supported.");
    }
  }

  lprintf(LO_DEBUG, "G_ReloadDefaults: Checking OPTIONS.\n");
  dsda_ParseOptionsLump();
  G_ReloadDefaults();

  // e6y
  // option to disable automatic loading of dehacked-in-wad lump
  if (!dsda_Flag(dsda_arg_nodeh))
  {
    // MBF-style DeHackEd in wad support: load all lumps, not just the last one
    for (p = -1; (p = W_ListNumFromName("DEHACKED", p)) >= 0; )
      // Split loading DEHACKED lumps into IWAD/autoload and PWADs/others
      if (lumpinfo[p].source == source_iwad
          || lumpinfo[p].source == source_pre
          || lumpinfo[p].source == source_auto_load)
        ProcessDehFile(NULL, D_dehout(), p); // cph - add dehacked-in-a-wad support

    if (bfgedition)
    {
      int lump = W_CheckNumForName2("BFGBEX", ns_prboom);
      if (lump != LUMP_NOT_FOUND)
      {
        ProcessDehFile(NULL, D_dehout(), lump);
      }
    }
    if (gamemission == pack_nerve)
    {
      int lump = W_CheckNumForName2("NERVEBEX", ns_prboom);
      if (lump != LUMP_NOT_FOUND)
      {
        ProcessDehFile(NULL, D_dehout(), lump);
      }
    }
    if (gamemission == chex)
    {
      int lump = W_CheckNumForName2("CHEXDEH", ns_prboom);
      if (lump != LUMP_NOT_FOUND)
      {
        ProcessDehFile(NULL, D_dehout(), lump);
      }
    }
  }

  // process deh files from autoload directory before deh in wads from -file parameter
  if (autoload)
    D_AutoloadDehIWadDir();

  if (!dsda_Flag(dsda_arg_nodeh))
    for (p = -1; (p = W_ListNumFromName("DEHACKED", p)) >= 0; )
      if (!(lumpinfo[p].source == source_iwad
            || lumpinfo[p].source == source_pre
            || lumpinfo[p].source == source_auto_load))
        ProcessDehFile(NULL, D_dehout(), p);

  // process .deh files from PWADs autoload directories
  if (autoload)
    D_AutoloadDehPWadDir();

  // Load command line dehacked patches after WAD dehacked patches

  // e6y: DEH files preloaded in wrong order
  // http://sourceforge.net/tracker/index.php?func=detail&aid=1418158&group_id=148658&atid=772943

  // ty 03/09/98 do dehacked stuff
  // Using -deh in BOOM, others use -dehacked.
  // Ty 03/18/98 also allow .bex extension.  .bex overrides if both exist.

  arg = dsda_Arg(dsda_arg_deh);
  if (arg->found)
  {
    int i;

    // e6y
    // reorganization of the code for looking for bex/deh patches
    // in all standard dirs (%DOOMWADDIR%, etc)
    for (i = 0; i < arg->count; ++i)
    {
      char *file = NULL;

      file = I_RequireDeh(arg->value.v_string_array[i]);

      // during the beta we have debug output to dehout.txt
      ProcessDehFile(file,D_dehout(),0);
      Z_Free(file);
    }
  }

  PostProcessDeh();
  dsda_AppendZDoomMobjInfo();
  dsda_ApplyDefaultMapFormat();

  lprintf(LO_DEBUG, "dsda_InitWadStats: Setting up wad stats.\n");
  dsda_InitWadStats();

  //lprintf(LO_INFO, "\n"); // Separator after file loading

  V_InitColorTranslation(); //jff 4/24/98 load color translation lumps

  //jff 9/3/98 use logical output routine
  lprintf(LO_DEBUG, "M_Init: Init miscellaneous info.\n");
  M_Init();

  dsda_LoadSndInfo();

  if (map_format.sndseq)
  {
    SN_InitSequenceScript();
  }

  //jff 9/3/98 use logical output routine
  lprintf(LO_DEBUG, "R_Init: Init DOOM refresh daemon - ");
  R_Init();

  dsda_LoadWadPreferences();
  dsda_LoadMapInfo();
  dsda_InitSkills();

  //jff 9/3/98 use logical output routine
  lprintf(LO_DEBUG, "\nP_Init: Init Playloop state.\n");
  P_Init();

  // Must be after P_Init
  HandleWarp();

  // Must be after HandleWarp
  dsda_HandleSkip();

  //jff 9/3/98 use logical output routine
  lprintf(LO_DEBUG, "I_Init: Setting up machine state.\n");
  I_Init();

  //jff 9/3/98 use logical output routine
  lprintf(LO_DEBUG, "S_Init: Setting up sound.\n");
  S_Init();

  //jff 9/3/98 use logical output routine
  lprintf(LO_DEBUG, "dsda_InitFont: Loading the hud fonts.\n");
  dsda_InitFont();

  if (!(dsda_Flag(dsda_arg_nodraw) && dsda_Flag(dsda_arg_nosound)))
    I_InitGraphics();

#if 0
  // NSM
  arg = dsda_Arg(dsda_arg_viddump);
  if (arg->found)
  {
    I_CapturePrep(arg->value.v_string);
  }
#endif

  //jff 9/3/98 use logical output routine
  lprintf(LO_DEBUG, "ST_Init: Init status bar.\n");
  ST_Init();

  // start the appropriate game based on parms

  arg = dsda_Arg(dsda_arg_record);
  if (arg->found)
  {
    autostart = true;
    dsda_SetDemoBaseName(arg->value.v_string);
    dsda_InitDemoRecording();
  }

  dsda_ExecutePlaybackOptions();

  if (!userdemo)
  {
    if (autostart || netgame)
    {
      G_InitNew(startskill, startepisode, startmap, true);
      if (demorecording)
        G_BeginRecording();
    }
    else
      D_StartTitle();                 // start up intro loop
  }

  // do not try to interpolate during timedemo
  M_ChangeUncappedFrameRate();

  lprintf(LO_DEBUG, "\n"); // Separator after setup
}

//
// D_DoomMain
//

void D_DoomMain(void)
{
  D_DoomMainSetup(); // CPhipps - setup out of main execution stack

  D_DoomLoop ();  // never returns
}
