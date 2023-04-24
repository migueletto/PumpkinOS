#include <PalmOS.h>
#include <VFSMgr.h>

#include "thread.h"
#include "pumpkin.h"
#include "color.h"
#include "syntax.h"
#include "Chat.h"
#include "debug.h"

/*
An interactive command that sends queries to an UDP server
and reads back the server response, printing it on the terminal.
It can be used to interact with the modified gtp4allj server
shipped with PumpkinOS.
Since the server keeps the conversation state, multiple
invocations of this command will continue the conversation.

This command accepts two optional parameters, the server
host and the server port number:
chat <host> <port>

If no argument is provided, is uses "127.0.0.1" as the host
and 5555 and the port number.

The prompt will change to "chat>", and everything you type
is sent to the server. Responses will appear on the terminal
when they are sent by the server. To exit the chat command,
just type ENTER on an empty line.
*/

#define MAX_BUF     256
#define MAX_LINE    512
#define MAX_LANG     16

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT "5555"

typedef struct {
  syntax_plugin_t *syntax;
  syntax_highlight_t *shigh;
  int pos, dosyntax, lua, escape, ilang;
  char line[MAX_LINE];
  char language[MAX_LANG];
} chat_state_t;

static Boolean ChatResponse(char *buf, void *data);

static void chat_fg(uint8_t r, uint8_t g, uint8_t b) {
  RGBColorType rgb;
  UInt32 fg;

  rgb.r = r;
  rgb.g = g;
  rgb.b = b;
  fg = RGBToLong(&rgb);
  pumpkin_setcolor(fg, 0x80000000);
}

int CommandMain(int argc, char *argv[]) {
  chat_state_t state;
  ChatType *chat;
  char *host, *buf, *port;
  int r = -1;

  host = (argc >= 1) ? argv[0] : DEFAULT_HOST;
  port = (argc >= 2) ? argv[1] : DEFAULT_PORT;

  MemSet(&state, sizeof(chat_state_t), 0);
  buf = MemPtrNew(MAX_BUF);

  // initializes Lua syntax highlighting
  if ((state.syntax = syntax_get_plugin("lua")) != NULL) {
    state.shigh = state.syntax->syntax_create(0);
  }

  // connect to server
  if ((chat = ChatOpen(host, StrAToI(port))) != NULL) {
    for (;;) {
      // set terminal foreground color to the default
      pumpkin_setcolor(0x80000000, 0x80000000);
      pumpkin_puts("chat>");
      if (pumpkin_gets(buf, MAX_BUF) == 0) break;
      // change terminal foreground color so that server replies appear in green
      chat_fg(0, 255, 0);
      // send que query and print the response
      ChatQuery(chat, buf, ChatResponse, &state);
    }
    ChatClose(chat);
  }

  if (state.syntax) state.syntax->syntax_destroy(state.shigh);

  MemPtrFree(buf);

  return r;
}

// The following functions deal with syntax highlighting for Lua code.
// This can be used if the query asks for Lua code and the server answers
// with a special prefixes indicating where the code starts/ends.
// If the reply does not contain Lua code, it is simply printed as is.

static void syntax_print(void *data, uint32_t fg, uint32_t bg, char *s, int n) {
  int i;

  pumpkin_setcolor(fg, 0x80000000);

  for (i = 0; i < n; i++) {
    if (s[i] == '\n' && (i == 0 || s[i-1] != '\r')) {
      pumpkin_putchar('\r');
    }
    pumpkin_putchar(s[i]);
  }

  pumpkin_setcolor(0x80000000, 0x80000000);
}

static void add_reply(chat_state_t *state, char *buf) {
  int i, j;

  for (i = 0; buf[i]; i++) {
    if (state->pos < MAX_LINE) {
      if (!state->dosyntax) {
        switch (buf[i]) {
          case '\n':
            if (state->escape == 0) {
              pumpkin_putchar('\r');
              pumpkin_putchar(buf[i]);
            } else if (state->escape < 3) {
              for (j = 0; j < state->escape; j++) {
                pumpkin_putchar('`');
              }
              pumpkin_putchar('\r');
              pumpkin_putchar(buf[i]);
            } else if (state->ilang > 0) {
              chat_fg(255, 255, 0);
              state->dosyntax = 1;
              state->lua = 0;
              state->escape = 0;
              state->ilang = 0;
            }
            break;
          case '`':
            if (state->escape < 3) {
              state->escape++;
            }
            break;
          default:
            if (state->escape == 0) {
              pumpkin_putchar(buf[i]);
            } else if (state->escape < 3) {
              for (j = 0; j < state->escape; j++) {
                pumpkin_putchar('`');
              }
              pumpkin_putchar(buf[i]);
              state->escape = 0;
            } else {
              state->language[state->ilang++] = buf[i];
              if (state->ilang == 3 && !StrNCaselessCompare(state->language, "lua", 3)) {
                state->dosyntax = 1;
                state->lua = 1;
                state->escape = 0;
                state->ilang = 0;
                state->syntax->syntax_begin_line(state->shigh, syntax_print, NULL);
              }
            }
            break;
        }
      } else {
        switch (buf[i]) {
          case '`':
            state->escape++;
            if (state->escape == 3) {
              if (state->lua) state->syntax->syntax_end_line(state->shigh);
              state->dosyntax = 0;
              state->lua = 0;
              state->escape = 0;
              chat_fg(0, 255, 0);
            }
            break;
          default:
            for (j = 0; j < state->escape; j++) {
              state->syntax->syntax_char(state->shigh, '`');
            }
            if (state->lua) {
              state->syntax->syntax_char(state->shigh, buf[i]);
            } else {
              if (buf[i] == '\n') pumpkin_putchar('\r');
              pumpkin_putchar(buf[i]);
            }
            state->escape = 0;
            break;
        }
      }
    }
  }
}

static Boolean ChatResponse(char *buf, void *data) {
  chat_state_t *state = (chat_state_t *)data;

  add_reply(state, buf[0] ? buf : "\n");

  return !EvtSysEventAvail(true);
}
