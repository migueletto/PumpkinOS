#include <PalmOS.h>
#include <VFSMgr.h>

#include "thread.h"
#include "pumpkin.h"
#include "color.h"
#include "syntax.h"
#include "Chat.h"
#include "debug.h"

/*
A simple command that sends its arguments to an UDP server
and reads back the server response, printing it on the terminal.
It can be used to interact with the modified gtp4allj server
shipped with PumpkinOS. Each time this command is invoked,
a single query/reponse interaction is carried on.
Since the server keeps the conversation state, multiple
invocations of this command will continue the conversation.

This command can be invoked in two ways
chat "word1 word2 .. wordN"
char -f filename

In the first case, the arguments are the query;
Example: chat "what is an alpaca?"

In the second case, the query is stored in a file named 'filename'.
Example: char -f query.txt

CommandMain is the command entry point. In the example above,
argc would be 1, and argv would be { "what is an alpaca?" }
*/

#define MAX_BUF   65536
#define MAX_LINE    512
#define MAX_LANG     16

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
  FileRef fileRef;
  UInt32 nread;
  char *buf;
  int i, r = -1;

  // if there is any argument at all
  if (argc > 0) {
    buf = MemPtrNew(MAX_BUF);

    if (argc == 2 && !StrCompare(argv[0], "-f")) {
      // read text from a file into the buffer (at most MAX_BUF-1 chars are read)
      if (VFSFileOpen(1, argv[1], vfsModeRead, &fileRef) == errNone) {
        if (VFSFileRead(fileRef, MAX_BUF-1, buf, &nread) == errNone) {
          buf[nread] = 0;
        } else {
          debug(DEBUG_ERROR, "chat", "VFSFileRead failed");
          buf[0] = 0;
        }
        VFSFileClose(fileRef);
      } else {
        debug(DEBUG_ERROR, "chat", "VFSFileOpen \"%s\" failed", argv[1]);
      }

    } else {
      // concatenate the arguments into the buffer
      for (i = 0; i < argc; i++) {
        if (StrLen(buf) + 1 + StrLen(argv[i]) + 1 < MAX_BUF) {
          // separate each argument with a space
          if (i > 0) StrCat(buf, " ");
          StrCat(buf, argv[i]);
        }
      }
    }

    // if the buffer is not empty
    if (buf[0]) {
      MemSet(&state, sizeof(chat_state_t), 0);

      // change terminal foreground color so that server replies appear in green
      chat_fg(0, 255, 0);

      // initializes Lua syntax highlighting
      if ((state.syntax = syntax_get_plugin("lua")) != NULL) {
        state.shigh = state.syntax->syntax_create(0);
      }

      // call server on localhost port 5555
      ChatQuery("127.0.0.1", 5555, buf, ChatResponse, &state);

      // revert terminal foreground color to the default
      pumpkin_setcolor(0x80000000, 0x80000000);

      if (state.syntax) state.syntax->syntax_destroy(state.shigh);
    } else {
      debug(DEBUG_ERROR, "chat", "nothing to send");
    }

    MemPtrFree(buf);
  } else {
    debug(DEBUG_ERROR, "chat", "no arguments");
  }

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
  return true;
}
