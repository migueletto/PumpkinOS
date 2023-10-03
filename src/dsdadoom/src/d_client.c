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
 *    Contains the main wait loop, waiting for the next tic.
 *    Rewritten for LxDoom, but based around bits of the old code.
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include "doomtype.h"
#include "doomstat.h"
#include "d_net.h"
#include "z_zone.h"

#include "d_main.h"
#include "g_game.h"
#include "m_menu.h"

#include "i_system.h"
#include "i_main.h"
#include "i_video.h"
#include "r_fps.h"
#include "lprintf.h"
#include "e6y.h"

#include "dsda/args.h"
#include "dsda/settings.h"
#include "dsda/time.h"

ticcmd_t local_cmds[MAX_MAXPLAYERS][BACKUPTICS];
int maketic;
int solo_net = 0;

void D_InitFakeNetGame (void)
{
  int i;

  consoleplayer = displayplayer = 0;
  solo_net = dsda_Flag(dsda_arg_solo_net);
  coop_spawns = dsda_Flag(dsda_arg_coop_spawns);
  netgame = solo_net;

  playeringame[0] = true;
  for (i = 1; i < g_maxplayers; i++)
    playeringame[i] = false;
}

void FakeNetUpdate(void)
{
  static int lastmadetic;

  if (isExtraDDisplay)
    return;

  { // Build new ticcmds
    int newtics = dsda_GetTick() - lastmadetic;
    lastmadetic += newtics;

    while (newtics--) {
      I_StartTic();
      if (maketic - gametic > BACKUPTICS/2) break;

      // e6y
      // Eliminating the sudden jump of six frames(BACKUPTICS/2)
      // after change of game_speed.
      if (maketic - gametic && gametic <= force_singletics_to && dsda_GameSpeed() < 200) break;

      G_BuildTiccmd(&local_cmds[0][maketic%BACKUPTICS]);
      maketic++;
    }
  }
}

// Implicitly tracked whenever we check the current tick
int ms_to_next_tick;

void TryRunTics (void)
{
  int runtics;
  int entertime = dsda_GetTick();

  // Wait for tics to run
  while (1) {
    FakeNetUpdate();
    runtics = maketic - gametic;
    if (!runtics) {
      if (!movement_smooth /*|| !window_focused*/) {
          I_uSleep(ms_to_next_tick*1000);
      }
      if (dsda_GetTick() - entertime > 10) {
        M_Ticker(); return;
      }

      if (gametic > 0)
      {
        WasRenderedInTryRunTics = true;
        if (movement_smooth && gamestate==wipegamestate)
        {
          isExtraDDisplay = true;
          D_Display(-1);
          isExtraDDisplay = false;
        }
      }
    } else break;
  }

  while (runtics--) {
    if (advancedemo)
      D_DoAdvanceDemo ();
    M_Ticker ();
    G_Ticker ();
    gametic++;
    FakeNetUpdate();
  }
}
