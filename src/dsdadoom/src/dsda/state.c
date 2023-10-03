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

#include <stdlib.h>
#include <string.h>

#include "state.h"

state_t* states;
int num_states;
byte* defined_codeptr_args;
statenum_t* seenstate_tab;

static actionf_t* deh_codeptr;

static void dsda_ResetStates(int from, int to) {
  int i;

  for (i = from; i < to; ++i) {
    states[i].sprite = SPR_TNT1;
    states[i].tics = -1;
    states[i].nextstate = i;
  }
}

static void dsda_PrepAllocation(void) {
  static int first_allocation = true;

  if (first_allocation) {
    state_t* source = states;

    first_allocation = false;
    states = malloc(num_states * sizeof(*states));
    memcpy(states, source, num_states * sizeof(*states));
  }
}

static void dsda_EnsureCapacity(int limit) {
  while (limit >= num_states) {
    int old_num_states = num_states;

    dsda_PrepAllocation();

    num_states *= 2;

    states = realloc(states, num_states * sizeof(*states));
    memset(states + old_num_states, 0, (num_states - old_num_states) * sizeof(*states));

    deh_codeptr = realloc(deh_codeptr, num_states * sizeof(*deh_codeptr));
    memset(deh_codeptr + old_num_states, 0, (num_states - old_num_states) * sizeof(*deh_codeptr));

    defined_codeptr_args =
      realloc(defined_codeptr_args, num_states * sizeof(*defined_codeptr_args));
    memset(defined_codeptr_args + old_num_states, 0,
      (num_states - old_num_states) * sizeof(*defined_codeptr_args));

    seenstate_tab = realloc(seenstate_tab, num_states * sizeof(*seenstate_tab));
    memset(seenstate_tab + old_num_states, 0,
      (num_states - old_num_states) * sizeof(*seenstate_tab));

    dsda_ResetStates(old_num_states, num_states);
  }
}

dsda_deh_state_t dsda_GetDehState(int index) {
  dsda_deh_state_t deh_state;

  dsda_EnsureCapacity(index);

  deh_state.state = &states[index];
  deh_state.codeptr = &deh_codeptr[index];
  deh_state.defined_codeptr_args = &defined_codeptr_args[index];

  return deh_state;
}

void dsda_InitializeStates(state_t* source, int count) {
  int i;
  extern int raven;

  num_states = count;

  states = source;

  if (raven) return;

  seenstate_tab = calloc(num_states, sizeof(*seenstate_tab));

  deh_codeptr = malloc(num_states * sizeof(*deh_codeptr));
  for (i = 0; i < num_states; i++)
    deh_codeptr[i] = states[i].action;

  defined_codeptr_args = calloc(num_states, sizeof(*defined_codeptr_args));
}

void dsda_FreeDehStates(void) {
  free(defined_codeptr_args);
  free(deh_codeptr);
}
