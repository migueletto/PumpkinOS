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
//	DSDA State
//

#ifndef __DSDA_STATE__
#define __DSDA_STATE__

#include "info.h"

typedef struct {
  state_t* state;
  actionf_t* codeptr;
  byte* defined_codeptr_args;
} dsda_deh_state_t;

dsda_deh_state_t dsda_GetDehState(int index);
void dsda_InitializeStates(state_t* source, int count);
void dsda_FreeDehStates(void);

#endif
