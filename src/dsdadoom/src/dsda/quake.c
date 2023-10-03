//
// Copyright(C) 2022 by Ryan Krafnick
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
//	DSDA Quake
//

#include "doomstat.h"
#include "m_random.h"
#include "p_maputl.h"
#include "p_inter.h"
#include "p_spec.h"
#include "p_tick.h"

#include "dsda/global.h"

// TODO: move here
extern int localQuakeHappening[MAX_MAXPLAYERS];

// The unit of quake distance is 64 (2^6)
#define DISTBITS 6

void dsda_ResetQuakes(void) {
  memset(localQuakeHappening, 0, g_maxplayers * sizeof(*localQuakeHappening));
}

void dsda_UpdateQuakeIntensity(int player_num, int intensity) {
  localQuakeHappening[player_num] = intensity;
}

void dsda_UpdateQuake(quake_t* quake) {
  int i;

  for (i = 0; i < g_maxplayers; ++i) {
    mobj_t* mo;
    fixed_t dist;

    if (!playeringame[i] || players[i].cheats & CF_NOCLIP)
      continue;

    mo = players[i].mo;
    dist = P_AproxDistance(quake->location->x - mo->x,
                           quake->location->y - mo->y) >> (FRACBITS + DISTBITS);

    // TODO: This won't get changed until a quake ends if we leave the radius
    if (dist < quake->tremor_radius)
      dsda_UpdateQuakeIntensity(i, quake->intensity);

    if (dist < quake->damage_radius && mo->z <= mo->floorz) {
      angle_t an;

      if (P_Random(pr_hexen) < 50)
        P_DamageMobj(mo, NULL, NULL, HITDICE(1));

      an = P_Random(pr_hexen) << 24;
      P_ThrustMobj(mo, an, quake->intensity << (FRACBITS - 1));
    }
  }

  --quake->duration;
  if (quake->duration <= 0) {
    dsda_ResetQuakes();
    P_RemoveThinker(&quake->thinker);
  }
}

void dsda_SpawnQuake(mobj_t* location, int intensity, int duration,
                     int damage_radius, int tremor_radius) {
  quake_t* quake;

  quake = Z_MallocLevel(sizeof(*quake));
  memset(quake, 0, sizeof(*quake));
  P_AddThinker(&quake->thinker);
  quake->thinker.function = dsda_UpdateQuake;
  P_SetTarget(&quake->location, location);
  quake->intensity = intensity;
  quake->duration = duration;
  quake->damage_radius = damage_radius;
  quake->tremor_radius = tremor_radius;
}
