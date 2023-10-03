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
//	DSDA Mouse
//

#include "dsda/configuration.h"
#include "dsda/features.h"

#include "mouse.h"

static int quickstart_cache_tics;
static int quickstart_queued;
static signed short angleturn_cache[35];
static unsigned int angleturn_cache_index;

void dsda_InitQuickstartCache(void) {
  quickstart_cache_tics = dsda_IntConfig(dsda_config_quickstart_cache_tics);
}

void dsda_ApplyQuickstartMouseCache(int* mousex) {
  int i;

  if (!quickstart_cache_tics) return;

  if (quickstart_queued) {
    signed short result = 0;

    quickstart_queued = false;

    dsda_TrackFeature(uf_quickstartcache);

    for (i = 0; i < quickstart_cache_tics; ++i)
      result += angleturn_cache[i];

    *mousex = result;
  }
  else {
    angleturn_cache[angleturn_cache_index] = *mousex;
    ++angleturn_cache_index;
    if (angleturn_cache_index >= quickstart_cache_tics)
      angleturn_cache_index = 0;
  }
}

void dsda_QueueQuickstart(void) {
  quickstart_queued = true;
}
