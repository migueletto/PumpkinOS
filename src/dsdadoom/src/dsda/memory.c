//
// Copyright(C) 2020 by Ryan Krafnick
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
//	DSDA Memory
//

#include "i_sound.h"
#include "sounds.h"
#include "w_wad.h"
#include "lprintf.h"

#include "dsda/time.h"

#include "memory.h"

void dsda_CacheSoundLumps(void) {
  int i;

  for (i = 0; i < num_sfx; ++i) {
    sfxinfo_t *sfx = &S_sfx[i];
    sfx->lumpnum = I_GetSfxLumpNum(sfx);

    if (sfx->lumpnum >= 0)
      W_LockLumpNum(sfx->lumpnum);
  }
}
