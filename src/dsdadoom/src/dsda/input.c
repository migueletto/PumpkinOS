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
//	DSDA Input
//

#include <string.h>
#include <stdlib.h>

#include "input.h"

int dsda_input_profile;
static dsda_input_t dsda_input[DSDA_INPUT_PROFILE_COUNT][DSDA_INPUT_IDENTIFIER_COUNT];

typedef struct
{
  dboolean on;
  int activated_at;
  int deactivated_at;
  dboolean game_on;
  int game_activated_at;
  int game_deactivated_at;
} dsda_input_state_t;

static int dsda_input_counter; // +1 for each event
static int dsda_input_tick_counter; // +1 for each game tick
static dsda_input_state_t gamekeys[NUMKEYS];
static dsda_input_state_t mousearray[MAX_MOUSE_BUTTONS + 1];
static dsda_input_state_t *mousebuttons = &mousearray[1]; // allow [-1]
static dsda_input_state_t joyarray[MAX_JOY_BUTTONS + 1];
static dsda_input_state_t *joybuttons = &joyarray[1];    // allow [-1]

static void dsda_InputTrackButtons(dsda_input_state_t* buttons, int max, event_t* ev) {
  int i;

  for (i = 0; i < max; ++i) {
    unsigned int button_on = (ev->data1.i & (1 << i)) != 0;

    if (!buttons[i].on && button_on)
      buttons[i].activated_at = dsda_input_counter;

    if (buttons[i].on && !button_on)
      buttons[i].deactivated_at = dsda_input_counter;

    buttons[i].on = button_on;
  }
}

static void dsda_InputTrackKeyDown(event_t* ev) {
  int key = ev->data1.i;

  if (key >= NUMKEYS) return;

  gamekeys[key].activated_at = dsda_input_counter;
  gamekeys[key].on = true;
}

static void dsda_InputTrackKeyUp(event_t* ev) {
  int key = ev->data1.i;

  if (key >= NUMKEYS) return;

  gamekeys[key].deactivated_at = dsda_input_counter;
  gamekeys[key].on = false;
}

void dsda_InputFlushTick(void) {
  dsda_input_tick_counter = dsda_input_counter;
}

void dsda_InputTrackEvent(event_t* ev) {
  ++dsda_input_counter;

  switch (ev->type)
  {
    case ev_keydown:
      dsda_InputTrackKeyDown(ev);
      break;
    case ev_keyup:
      dsda_InputTrackKeyUp(ev);
      break;
    case ev_mouse:
      dsda_InputTrackButtons(mousebuttons, MAX_MOUSE_BUTTONS, ev);
      break;
    case ev_joystick:
      dsda_InputTrackButtons(joybuttons, MAX_JOY_BUTTONS, ev);
      break;
    default:
      break;
  }
}

static void dsda_InputTrackGameButtons(dsda_input_state_t* buttons, int max, event_t* ev) {
  int i;

  for (i = 0; i < max; ++i) {
    unsigned int button_on = (ev->data1.i & (1 << i)) != 0;

    if (!buttons[i].game_on && button_on)
      buttons[i].game_activated_at = dsda_input_counter;

    if (buttons[i].game_on && !button_on)
      buttons[i].game_deactivated_at = dsda_input_counter;

    buttons[i].game_on = button_on;
  }
}

static void dsda_InputTrackGameKeyDown(event_t* ev) {
  int key = ev->data1.i;

  if (key >= NUMKEYS) return;

  gamekeys[key].game_activated_at = dsda_input_counter;
  gamekeys[key].game_on = true;
}

static void dsda_InputTrackGameKeyUp(event_t* ev) {
  int key = ev->data1.i;

  if (key >= NUMKEYS) return;

  gamekeys[key].game_deactivated_at = dsda_input_counter;
  gamekeys[key].game_on = false;
}

void dsda_InputTrackGameEvent(event_t* ev) {
  switch (ev->type)
  {
    case ev_keydown:
      dsda_InputTrackGameKeyDown(ev);
      break;
    case ev_keyup:
      dsda_InputTrackGameKeyUp(ev);
      break;
    case ev_mouse:
      dsda_InputTrackGameButtons(mousebuttons, MAX_MOUSE_BUTTONS, ev);
      break;
    case ev_joystick:
      dsda_InputTrackGameButtons(joybuttons, MAX_JOY_BUTTONS, ev);
      break;
    default:
      break;
  }
}

dboolean dsda_InputActivated(int identifier) {
  int i;
  dsda_input_t* input;
  input = &dsda_input[dsda_input_profile][identifier];

  for (i = 0; i < input->num_keys; ++i)
    if (gamekeys[input->key[i]].activated_at == dsda_input_counter)
      return true;

  return
    mousebuttons[input->mouseb].activated_at == dsda_input_counter ||
    joybuttons[input->joyb].activated_at == dsda_input_counter;
}

dboolean dsda_InputTickActivated(int identifier) {
  int i;
  dsda_input_t* input;
  input = &dsda_input[dsda_input_profile][identifier];

  for (i = 0; i < input->num_keys; ++i)
    if (gamekeys[input->key[i]].game_activated_at > dsda_input_tick_counter)
      return true;

  return
    mousebuttons[input->mouseb].game_activated_at > dsda_input_tick_counter ||
    joybuttons[input->joyb].game_activated_at > dsda_input_tick_counter;
}

dboolean dsda_InputDeactivated(int identifier) {
  int i;
  dboolean deactivated = false;
  dsda_input_t* input;
  input = &dsda_input[dsda_input_profile][identifier];

  for (i = 0; i < input->num_keys; ++i)
    if (gamekeys[input->key[i]].on)
      return false;
    else if (gamekeys[input->key[i]].deactivated_at == dsda_input_counter)
      deactivated = true;

  return
    !mousebuttons[input->mouseb].on &&
    !joybuttons[input->joyb].on &&
    (
      deactivated ||
      mousebuttons[input->mouseb].deactivated_at == dsda_input_counter ||
      joybuttons[input->joyb].deactivated_at == dsda_input_counter
    );
}

void dsda_InputFlush(void) {
  memset(gamekeys, 0, sizeof(gamekeys));
  memset(mousearray, 0, sizeof(mousearray));
  memset(joyarray, 0, sizeof(joyarray));
  dsda_input_tick_counter = 0;
  dsda_input_counter = 0;
}

dsda_input_t* dsda_Input(int identifier) {
  return &dsda_input[dsda_input_profile][identifier];
}

void dsda_InputCopy(int identifier, dsda_input_t* input[DSDA_INPUT_PROFILE_COUNT]) {
  int i;

  for (i = 0; i < DSDA_INPUT_PROFILE_COUNT; ++i) {
    input[i] = &dsda_input[i][identifier];
  }
}

int dsda_InputMatchKey(int identifier, int value) {
  int i;
  dsda_input_t* p = &dsda_input[dsda_input_profile][identifier];

  for (i = 0; i < p->num_keys; ++i)
    if (p->key[i] == value)
      return true;

  return false;
}

int dsda_InputMatchMouseB(int identifier, int value) {
  return dsda_input[dsda_input_profile][identifier].mouseb == value;
}

int dsda_InputMatchJoyB(int identifier, int value) {
  return dsda_input[dsda_input_profile][identifier].joyb == value;
}

void dsda_InputResetSpecific(int config_index, int identifier) {
  dsda_input_t* p = &dsda_input[config_index][identifier];

  p->num_keys = 0;
  p->mouseb = -1;
  p->joyb = -1;
}

void dsda_InputReset(int identifier) {
  dsda_InputResetSpecific(dsda_input_profile, identifier);
}

void dsda_InputSet(int identifier, dsda_input_default_t input) {
  dsda_InputSetSpecific(dsda_input_profile, identifier, input);
}

void dsda_InputSetSpecific(int config_index, int identifier, dsda_input_default_t input) {
  dsda_input_t* p = &dsda_input[config_index][identifier];

  if (p->num_keys == 0)
    p->key = realloc(p->key, sizeof(*p->key));
  if (input.key > 0) {
    p->key[0] = input.key;
    p->num_keys = 1;
  }
  else p->num_keys = 0;

  p->mouseb = input.mouseb;
  p->joyb = input.joyb;
}

static void dsda_InputAddThing(int** list, int* count, int value) {
  int i;

  for (i = 0; i < (*count); ++i)
    if ((*list)[i] == value) return;

  (*count)++;
  (*list) = realloc((*list), (*count) * sizeof(**list));
  (*list)[(*count) - 1] = value;
}

void dsda_InputAddSpecificKey(int config_index, int identifier, int value) {
  dsda_input_t* p = &dsda_input[config_index][identifier];

  if (value < 1 || value >= NUMKEYS) return;

  dsda_InputAddThing(&p->key, &p->num_keys, value);
}

void dsda_InputAddKey(int identifier, int value) {
  dsda_InputAddSpecificKey(dsda_input_profile, identifier, value);
}

void dsda_InputAddSpecificMouseB(int config_index, int identifier, int value) {
  if (value < -1 || value >= MAX_MOUSE_BUTTONS) return;

  dsda_input[config_index][identifier].mouseb = value;
}

void dsda_InputAddMouseB(int identifier, int value) {
  dsda_InputAddSpecificMouseB(dsda_input_profile, identifier, value);
}

void dsda_InputAddSpecificJoyB(int config_index, int identifier, int value) {
  if (value < -1 || value >= MAX_JOY_BUTTONS) return;

  dsda_input[config_index][identifier].joyb = value;
}

void dsda_InputAddJoyB(int identifier, int value) {
  dsda_InputAddSpecificJoyB(dsda_input_profile, identifier, value);
}

static void dsda_InputRemoveThing(int* list, int* count, int value) {
  int i;
  dboolean found = false;

  for (i = 0; i < (*count); ++i)
    if (list[i] == value) {
      found = true;

      for (; i < (*count) - 1; ++i)
        list[i] = list[i + 1];

      break;
    }

  if (!found) return;

  (*count)--;
}

void dsda_InputRemoveKey(int identifier, int value) {
  dsda_input_t* p = &dsda_input[dsda_input_profile][identifier];

  dsda_InputRemoveThing(p->key, &p->num_keys, value);
}

void dsda_InputRemoveMouseB(int identifier, int value) {
  dsda_input[dsda_input_profile][identifier].mouseb = -1;
}

void dsda_InputRemoveJoyB(int identifier, int value) {
  dsda_input[dsda_input_profile][identifier].joyb = -1;
}

dboolean dsda_InputActive(int identifier) {
  int i;
  dsda_input_t* input;
  input = &dsda_input[dsda_input_profile][identifier];

  for (i = 0; i < input->num_keys; ++i)
    if (gamekeys[input->key[i]].game_on)
      return true;

  return (input->mouseb >= 0 && mousebuttons[input->mouseb].game_on) ||
         (input->joyb >= 0   && joybuttons[input->joyb].game_on);
}

dboolean dsda_InputKeyActive(int identifier) {
  int i;
  dsda_input_t* input;
  input = &dsda_input[dsda_input_profile][identifier];

  for (i = 0; i < input->num_keys; ++i)
    if (gamekeys[input->key[i]].game_on)
      return true;

  return false;
}

dboolean dsda_InputMouseBActive(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_profile][identifier];

  return input->mouseb >= 0 && mousebuttons[input->mouseb].game_on;
}

dboolean dsda_InputJoyBActive(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_profile][identifier];

  return input->joyb >= 0 && joybuttons[input->joyb].game_on;
}
