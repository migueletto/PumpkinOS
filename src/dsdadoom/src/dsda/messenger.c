//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA Message
//

#include "d_player.h"
#include "doomstat.h"
#include "z_zone.h"

#include "dsda/settings.h"

#include "messenger.h"

// hexen_note: use FONTAY_S for yellow messages (new font, y_message, etc)

#define MESSAGE_LIFETIME 140

typedef enum {
  message_alert,
  message_normal,
} message_priority_t;

typedef struct message_s {
  char* str;
  message_priority_t priority;
  int tics;
  struct message_s* next_message;
} message_t;

static message_t* current_message;
static message_t* last_message;

static void dsda_FreeMessage(message_t* message) {
  Z_Free(message->str);
  Z_Free(message);
}

static void dsda_ClearMessages(void) {
  message_t* message_p;

  while (current_message) {
    message_p = current_message->next_message;
    dsda_FreeMessage(current_message);
    current_message = message_p;
  }
}

static void dsda_AppendMessage(message_t* message) {
  if (!current_message)
    current_message = message;
  else {
    message_t* message_p;

    message_p = current_message;
    while (message_p->next_message)
      message_p = message_p->next_message;

    message_p->next_message = message;
  }
}

static void dsda_QueueMessage(const char* str, message_priority_t priority) {
  message_t* new_message;

  if (current_message) {
    if (current_message->priority < priority)
      return;
    else if (current_message->priority > priority)
      dsda_ClearMessages();
    else if (priority == message_normal) {
      Z_Free(current_message->str);

      current_message->str = Z_Strdup(str);
      current_message->tics = MESSAGE_LIFETIME;

      return;
    }
  }

  new_message = Z_Calloc(1, sizeof(*new_message));
  new_message->str = Z_Strdup(str);
  new_message->priority = priority;
  new_message->tics = MESSAGE_LIFETIME;

  dsda_AppendMessage(new_message);
}

void dsda_AddPlayerAlert(const char* str, player_t* player) {
  if (player == &players[displayplayer])
    dsda_QueueMessage(str, message_alert);
}

void dsda_AddAlert(const char* str) {
  dsda_QueueMessage(str, message_alert);
}

void dsda_AddPlayerMessage(const char* str, player_t* player) {
  if (dsda_ShowMessages() && player == &players[displayplayer])
    dsda_QueueMessage(str, message_normal);
}

void dsda_AddMessage(const char* str) {
  if (dsda_ShowMessages())
    dsda_QueueMessage(str, message_normal);
}

void dsda_AddUnblockableMessage(const char* str) {
  dsda_QueueMessage(str, message_normal);
}

void dsda_UpdateMessenger(void) {
  if (!current_message)
    return;

  --current_message->tics;
  if (!current_message->tics) {
    if (last_message)
      dsda_FreeMessage(last_message);
    last_message = current_message;
    current_message = current_message->next_message;
  }
}

void dsda_InitMessenger(void) {
  dsda_ClearMessages();
  if (last_message)
    dsda_FreeMessage(last_message);
  last_message = NULL;
}

void dsda_ReplayMessage(void) {
  if (last_message) {
    dsda_ClearMessages();
    dsda_QueueMessage(last_message->str, last_message->priority);
  }
}

char* dsda_PlayerMessage(void) {
  if (!current_message)
    return NULL;

  return current_message->str;
}
