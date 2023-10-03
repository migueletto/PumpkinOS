//
// Copyright(C) 2021 by Ryan Krafnick
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	DSDA Death
//

#include "doomstat.h"
#include "d_player.h"
#include "v_video.h"

#include "dsda/configuration.h"
#include "dsda/mapinfo.h"
#include "dsda/save.h"
#include "dsda/skill_info.h"

#include "death.h"

extern int inv_ptr;
extern int curpos;
extern int newtorch;
extern int newtorchdelta;

typedef enum {
  death_use_default,
  death_use_nothing,
  death_use_reload,
} death_use_action_t;

static int dsda_DeathUseAction(void)
{
  if (demorecording ||
      demoplayback ||
      map_info.flags & MI_ALLOW_RESPAWN ||
      skill_info.flags & SI_PLAYER_RESPAWN)
    return death_use_default;

  return dsda_IntConfig(dsda_config_death_use_action);
}

void dsda_DeathUse(player_t* player) {
  switch (dsda_DeathUseAction())
  {
    case death_use_default:
    default:
      if (raven)
      {
        if (player == &players[consoleplayer])
        {
          V_SetPalette(0);
          inv_ptr = 0;
          curpos = 0;
          newtorch = 0;
          newtorchdelta = 0;
        }

        if (hexen)
        {
          player->mo->special1.i = player->pclass;
          if (player->mo->special1.i > 2)
          {
            player->mo->special1.i = 0;
          }
        }

        // Let the mobj know the player has entered the reborn state.  Some
        // mobjs need to know when it's ok to remove themselves.
        player->mo->special2.i = 666;
      }

      player->playerstate = PST_REBORN;
      break;
    case death_use_nothing:
      break;
    case death_use_reload:
      {
        extern void G_LoadGame(int slot);
        int slot = dsda_LastSaveSlot();

        if (slot >= 0)
          G_LoadGame(slot);
      }
      break;
  }
}
