/*
 * tiny vi.c: A small 'vi' clone
 * Copyright (C) 2000, 2001 Sterling Huxley <sterling@europa.com>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */

#include <PalmOS.h>

#include "sys.h"
#include "pumpkin.h"
#include "editor.h"
#include "edit.h"
#include "syntax.h"
#include "color.h"
#include "util.h"
#include "debug.h"

#define editPluginId 'vied'

#define TRUE  1
#define FALSE 0

#define ENABLE_FEATURE_VI_8BIT 1
#define ENABLE_FEATURE_VI_ASK_TERMINAL 0
#define ENABLE_FEATURE_VI_COLON 1
#define ENABLE_FEATURE_VI_DOT_CMD 1
#define ENABLE_FEATURE_VI_READONLY 1
#define ENABLE_FEATURE_VI_SEARCH 1
#define ENABLE_FEATURE_VI_SET 1
#define ENABLE_FEATURE_VI_SETOPTS 1
#define ENABLE_FEATURE_VI_UNDO 1
#define ENABLE_FEATURE_VI_WIN_RESIZE 0
#define ENABLE_FEATURE_VI_YANKMARK 1
#define ENABLE_LOCALE_SUPPORT 1

#define CONFIG_FEATURE_VI_MAX_LEN 4096

#define IF_FEATURE_VI_ASK_TERMINAL(...) __VA_ARGS__
#define IF_FEATURE_VI_SEARCH(...) __VA_ARGS__
#define IF_FEATURE_VI_COLON(...) __VA_ARGS__
#define IF_FEATURE_VI_YANKMARK(...) __VA_ARGS__
#define IF_FEATURE_VI_READONLY(...) __VA_ARGS__

#define ALWAYS_INLINE inline
#define NOINLINE
#define ALIGN1

#define STRERROR_FMT    "%s"
#define STRERROR_ERRNO  ,pumpkin_error_msg(pumpkin_get_lasterr())

#define xzalloc(n) sys_malloc(n)

#define ARRAY_SIZE(x) ((unsigned)(sizeof(x) / sizeof((x)[0])))

#define BB_VER "1.31.1"

enum {
  KEYCODE_UP        =  -2,
  KEYCODE_DOWN      =  -3,
  KEYCODE_RIGHT     =  -4,
  KEYCODE_LEFT      =  -5,
  KEYCODE_HOME      =  -6,
  KEYCODE_END       =  -7,
  KEYCODE_INSERT    =  -8,
  KEYCODE_DELETE    =  -9,
  KEYCODE_PAGEUP    = -10,
  KEYCODE_PAGEDOWN  = -11,
  KEYCODE_BACKSPACE = -12, /* Used only if Alt/Ctrl/Shifted */
  KEYCODE_D         = -13, /* Used only if Alted */
  //KEYCODE_SHIFT_...   = KEYCODE_...   & ~0x80,
  KEYCODE_CTRL_RIGHT    = KEYCODE_RIGHT & ~0x40,
  KEYCODE_CTRL_LEFT     = KEYCODE_LEFT  & ~0x40,
  KEYCODE_ALT_RIGHT     = KEYCODE_RIGHT & ~0x20,
  KEYCODE_ALT_LEFT      = KEYCODE_LEFT  & ~0x20,
  KEYCODE_ALT_BACKSPACE = KEYCODE_BACKSPACE & ~0x20,
  KEYCODE_ALT_D         = KEYCODE_D     & ~0x20,

  KEYCODE_CURSOR_POS = -0x100, /* 0xfff..fff00 */
  KEYCODE_BUFFER_SIZE = 16
};

typedef signed char smallint;
typedef unsigned char smalluint;

static const RGBColorType foregroundColorRGB = { 0, 0xff, 0xff, 0xff };
static const RGBColorType backgroundColorRGB = { 0, 0x13, 0x32, 0x65 };

/*
struct globals *ptr_to_globals;

#define SET_PTR_TO_GLOBALS(x) do { \
  ptr_to_globals = x; \
} while (0)

#define FREE_PTR_TO_GLOBALS() do { \
  if (ENABLE_FEATURE_CLEAN_UP) { \
    xfree(ptr_to_globals); \
  } \
} while (0)
*/

static char *strchrnul(const char *s, int c) {
  char *r = sys_strchr(s, c);
  return r ? r : (char *)&s[sys_strlen(s)];
}

#ifdef WINDOWS
static char *strndup(const char *str, sys_size_t size) {
  char *r = NULL;
  int n;

  n = sys_strlen(str);
  if (n > size) n = size;
  r = sys_malloc(n+1);
  strncpy(r, str, n);

  return r;
}
#endif

static char *last_char_is(const char *s, int c) {
  if (s && *s) {
    sys_size_t sz = sys_strlen(s) - 1;
    s += sz;
    if ( (unsigned char)*s == c)
      return (char*)s;
  }
  return NULL;
}

static char *skip_whitespace(const char *s) {
  /* In POSIX/C locale (the only locale we care about: do we REALLY want
   * to allow Unicode whitespace in, say, .conf files? nuts!)
   * isspace is only these chars: "\t\n\v\f\r" and space.
   * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
   * Use that.
   */
  while (*s == ' ' || (unsigned char)(*s - 9) <= (13 - 9))
    s++;

  return (char *) s;
}

static char *skip_non_whitespace(const char *s) {
  while (*s != '\0' && *s != ' ' && (unsigned char)(*s - 9) > (13 - 9))
    s++;

  return (char *) s;
}

// Should be after libbb.h: on some systems regex.h needs sys/types.h:
#if ENABLE_FEATURE_VI_REGEX_SEARCH
# include <regex.h>
#endif

// the CRASHME code is unmaintained, and doesn't currently build
#define ENABLE_FEATURE_VI_CRASHME 0


#if ENABLE_LOCALE_SUPPORT

#if ENABLE_FEATURE_VI_8BIT
//FIXME: this does not work properly for Unicode anyway
# define Isprint(c) (sys_isprint)(c)
#else
# define Isprint(c) isprint_asciionly(c)
#endif

#else

// 0x9b is Meta-ESC
#if ENABLE_FEATURE_VI_8BIT
# define Isprint(c) ((unsigned char)(c) >= ' ' && (c) != 0x7f && (unsigned char)(c) != 0x9b)
#else
# define Isprint(c) ((unsigned char)(c) >= ' ' && (unsigned char)(c) < 0x7f)
#endif

#endif


enum {
  MAX_TABSTOP = 32, // sanity limit
  // User input len. Need not be extra big.
  // Lines in file being edited *can* be bigger than this.
  MAX_INPUT_LEN = 128,
  // Sanity limits. We have only one buffer of this size.
  MAX_SCR_COLS = CONFIG_FEATURE_VI_MAX_LEN,
  MAX_SCR_ROWS = CONFIG_FEATURE_VI_MAX_LEN,
};

#if ENABLE_FEATURE_VI_DOT_CMD || ENABLE_FEATURE_VI_YANKMARK
// cmds modifying text[]
static const char modifying_cmds[] ALIGN1 = "aAcCdDiIJoOpPrRs""xX<>~";
#endif

enum {
  YANKONLY = FALSE,
  YANKDEL = TRUE,
  FORWARD = 1,  // code depends on "1"  for array index
  BACK = -1,  // code depends on "-1" for array index
  LIMITED = 0,  // char_search() only current line
  FULL = 1,  // char_search() to the end/beginning of entire text

  S_BEFORE_WS = 1,  // used in skip_thing() for moving "dot"
  S_TO_WS = 2,    // used in skip_thing() for moving "dot"
  S_OVER_WS = 3,    // used in skip_thing() for moving "dot"
  S_END_PUNCT = 4,  // used in skip_thing() for moving "dot"
  S_END_ALNUM = 5,  // used in skip_thing() for moving "dot"
};


// vi.c expects chars to be unsigned.
// busybox build system provides that, but it's better
// to audit and fix the source

struct globals {
  // many references - keep near the top of globals
  char *text, *end;       // pointers to the user data in memory
  char *dot;              // where all the action takes place
  int text_size;    // size of the allocated buffer

  // the rest
  smallint vi_setops;
#define VI_AUTOINDENT 1
#define VI_SHOWMATCH  2
#define VI_IGNORECASE 4
#define VI_ERR_METHOD 8
#define autoindent (g->vi_setops & VI_AUTOINDENT)
#define showmatch  (g->vi_setops & VI_SHOWMATCH )
#define ignorecase (g->vi_setops & VI_IGNORECASE)
// indicate error with beep or flash
#define err_method (g->vi_setops & VI_ERR_METHOD)

#if ENABLE_FEATURE_VI_READONLY
  smallint readonly_mode;
#define SET_READONLY_FILE(flags)        ((flags) |= 0x01)
#define SET_READONLY_MODE(flags)        ((flags) |= 0x02)
#define UNSET_READONLY_FILE(flags)      ((flags) &= 0xfe)
#else
#define SET_READONLY_FILE(flags)        ((void)0)
#define SET_READONLY_MODE(flags)        ((void)0)
#define UNSET_READONLY_FILE(flags)      ((void)0)
#endif

  smallint editing;        // >0 while we are editing a file
                           // [code audit says "can be 0, 1 or 2 only"]
  smallint cmd_mode;       // 0=command  1=insert 2=replace
  int modified_count;      // buffer contents changed if !0
  int last_modified_count; // = -1;
  int cmdline_filecnt;     // how many file names on cmd line
  int cmdcnt;              // repetition count
  unsigned rows, columns;   // the terminal screen is this size
//#if ENABLE_FEATURE_VI_ASK_TERMINAL
  int get_rowcol_error;
//#endif
  int crow, ccol;          // cursor is on Crow x Ccol
  int offset;              // chars scrolled off the screen to the left
  int have_status_msg;     // is default edit status needed?
                           // [don't make smallint!]
  int last_status_cksum;   // hash of current status line
  char *current_filename;
  char *screenbegin;       // index into text[], of top line on the screen
  char *screen;            // pointer to the virtual screen buffer
  int screensize;          //            and its size
  int tabstop;
  int last_forward_char;   // last char searched for with 'f' (int because of Unicode)
#if ENABLE_FEATURE_VI_CRASHME
  char last_input_char;    // last char read from user
#endif

#if ENABLE_FEATURE_VI_DOT_CMD
  smallint adding2q;   // are we currently adding user input to q
  int lmc_len;             // length of last_modifying_cmd
  char *ioq, *ioq_start;   // pointer to string for get_one_char to "read"
#endif
#if ENABLE_FEATURE_VI_SEARCH
  char *last_search_pattern; // last pattern from a '/' or '?' search
#endif

  // former statics
#if ENABLE_FEATURE_VI_YANKMARK
  char *edit_file__cur_line;
#endif
  int refresh__old_offset;
  int format_edit_status__tot;

  // a few references only
#if ENABLE_FEATURE_VI_YANKMARK
  smalluint YDreg;//,Ureg;// default delete register and orig line for "U"
#define Ureg 27
  char *reg[28];          // named register a-z, "D", and "U" 0-25,26,27
  char *mark[28];         // user marks points somewhere in text[]-  a-z and previous context ''
  char *context_start, *context_end;
#endif
#if ENABLE_FEATURE_VI_USE_SIGNALS
  sigjmp_buf restart;     // int_handler() jumps to location remembered here
#endif
  //struct termios term_orig; // remember what the cooked mode was
#if ENABLE_FEATURE_VI_COLON
  char *initial_cmds[3];  // currently 2 entries, NULL terminated
#endif
  // Should be just enough to hold a key sequence,
  // but CRASHME mode uses it as generated command buffer too
#if ENABLE_FEATURE_VI_CRASHME
  char readbuffer[128];
#else
  char readbuffer[KEYCODE_BUFFER_SIZE];
#endif
#define STATUS_BUFFER_LEN  200
  char status_buffer[STATUS_BUFFER_LEN]; // messages to the user
#if ENABLE_FEATURE_VI_DOT_CMD
  char last_modifying_cmd[MAX_INPUT_LEN];  // last modifying cmd for "."
#endif
  char get_input_line__buf[MAX_INPUT_LEN]; // former static

  char scr_out_buf[MAX_SCR_COLS + MAX_TABSTOP * 2];

#if ENABLE_FEATURE_VI_UNDO
// undo_push() operations
#define UNDO_INS         0
#define UNDO_DEL         1
#define UNDO_INS_CHAIN   2
#define UNDO_DEL_CHAIN   3
// UNDO_*_QUEUED must be equal to UNDO_xxx ORed with UNDO_QUEUED_FLAG
#define UNDO_QUEUED_FLAG 4
#define UNDO_INS_QUEUED  4
#define UNDO_DEL_QUEUED  5
#define UNDO_USE_SPOS   32
#define UNDO_EMPTY      64
// Pass-through flags for functions that can be undone
#define NO_UNDO          0
#define ALLOW_UNDO       1
#define ALLOW_UNDO_CHAIN 2
# if ENABLE_FEATURE_VI_UNDO_QUEUE
#define ALLOW_UNDO_QUEUED 3
  char undo_queue_state;
  int undo_q;
  char *undo_queue_spos;  // Start position of queued operation
  char undo_queue[CONFIG_FEATURE_VI_UNDO_QUEUE_MAX];
# else
// If undo queuing disabled, don't invoke the missing queue logic
#define ALLOW_UNDO_QUEUED 1
# endif
  struct undo_object {
    struct undo_object *prev;  // Linking back avoids list traversal (LIFO)
    int start;    // Offset where the data should be restored/deleted
    int length;    // total data size
    uint8_t u_type;    // 0=deleted, 1=inserted, 2=swapped
    char undo_text[1];  // text that was deleted (if deletion)
  } *undo_stack_tail;
#endif /* ENABLE_FEATURE_VI_UNDO */

  editor_t *e;
  syntax_highlight_t *shigh;
  syntax_plugin_t *syntax;
  uint32_t foregroundColor, backgroundColor;
};

#define G ptr_to_globals

//#define text           (G->text          )
//#define text_size      (G->text_size     )
//#define end            (G->end           )
//#define dot            (G->dot           )
//#define reg            (G->reg           )

//#define vi_setops               (G->vi_setops          )
//#define editing                 (G->editing            )
//#define cmd_mode                (G->cmd_mode           )
//#define modified_count          (G->modified_count     )
//#define last_modified_count     (G->last_modified_count)
//#define cmdline_filecnt         (G->cmdline_filecnt    )
//#define cmdcnt                  (G->cmdcnt             )
//#define rows                    (G->rows               )
//#define columns                 (G->columns            )
//#define crow                    (G->crow               )
//#define ccol                    (G->ccol               )
//#define offset                  (G->offset             )
//#define status_buffer           (G->status_buffer      )
//#define have_status_msg         (G->have_status_msg    )
//#define last_status_cksum       (G->last_status_cksum  )
//#define current_filename        (G->current_filename   )
//#define screen                  (G->screen             )
//#define screensize              (G->screensize         )
//#define screenbegin             (G->screenbegin        )
//#define tabstop                 (G->tabstop            )
//#define last_forward_char       (G->last_forward_char  )
//#if ENABLE_FEATURE_VI_CRASHME
//#define last_input_char         (G->last_input_char    )
//#endif
//#if ENABLE_FEATURE_VI_READONLY
//#define readonly_mode           (G->readonly_mode      )
//#else
//#define readonly_mode           0
//#endif
//#define adding2q                (G->adding2q           )
//#define lmc_len                 (G->lmc_len            )
//#define ioq                     (G->ioq                )
//#define ioq_start               (G->ioq_start          )
//#define last_search_pattern     (G->last_search_pattern)

//#define edit_file__cur_line     (G->edit_file__cur_line)
//#define refresh__old_offset     (G->refresh__old_offset)
//#define format_edit_status__tot (G->format_edit_status__tot)

//#define YDreg          (G->YDreg         )
//#define Ureg           (G->Ureg          )
//#define mark           (G->mark          )
//#define context_start  (G->context_start )
//#define context_end    (G->context_end   )
#define restart        (G->restart       )
#define term_orig      (G->term_orig     )
//#define initial_cmds   (G->initial_cmds  )
//#define readbuffer     (G->readbuffer    )
//#define scr_out_buf    (G->scr_out_buf   )
//#define last_modifying_cmd  (G->last_modifying_cmd )
//#define get_input_line__buf (G->get_input_line__buf)

#if ENABLE_FEATURE_VI_UNDO
//#define undo_stack_tail  (G->undo_stack_tail )
# if ENABLE_FEATURE_VI_UNDO_QUEUE
//#define undo_queue_state (G->undo_queue_state)
//#define undo_q           (G->undo_q          )
//#define undo_queue       (G->undo_queue      )
//#define undo_queue_spos  (G->undo_queue_spos )
# endif
#endif

#if 0
#define INIT_G() do { \
  SET_PTR_TO_GLOBALS(xzalloc(sizeof(struct globals))); \
  g->last_modified_count = -1; \
  /* "" but has space for 2 chars: */ \
  IF_FEATURE_VI_SEARCH(last_search_pattern = xzalloc(2);) \
} while (0)
#endif

#if ENABLE_FEATURE_VI_CRASHME
static int crashme = 0;
#endif

static void show_status_line(struct globals *g);  // put a message on the bottom line
static void status_line_bold(struct globals *g, const char *, ...);

static void show_help(struct globals *g)
{
  char *msg = "These features are available:"
#if ENABLE_FEATURE_VI_SEARCH
  "\n\tPattern searches with / and ?"
#endif
#if ENABLE_FEATURE_VI_DOT_CMD
  "\n\tLast command repeat with ."
#endif
#if ENABLE_FEATURE_VI_YANKMARK
  "\n\tLine marking with 'x"
  "\n\tNamed buffers with \"x"
#endif
#if ENABLE_FEATURE_VI_READONLY
  //not implemented: "\n\tReadonly if vi is called as \"view\""
  //redundant: usage text says this too: "\n\tReadonly with -R command line arg"
#endif
#if ENABLE_FEATURE_VI_SET
  "\n\tSome colon mode commands with :"
#endif
#if ENABLE_FEATURE_VI_SETOPTS
  "\n\tSettable options with \":set\""
#endif
#if ENABLE_FEATURE_VI_USE_SIGNALS
  "\n\tSignal catching- ^C"
  "\n\tJob suspend and resume with ^Z"
#endif
#if ENABLE_FEATURE_VI_WIN_RESIZE
  "\n\tAdapt to window re-sizes"
#endif
  "\r\n";

  g->e->write(g->e->data, msg, sys_strlen(msg));
}

static void write1(struct globals *g, const char *out)
{
  debug(DEBUG_TRACE, "vi", "write1 [%s]", out);
  g->e->write(g->e->data, (char *)out, sys_strlen(out));
}

static int get_terminal_width_height(struct globals *g, unsigned int *ncolumns, unsigned int *nrows) {
  int nc, nr;

  g->e->window(g->e->data, &nc, &nr);
  *ncolumns = nc;
  *nrows = nr;

  return 0;
}

static int query_screen_dimensions(struct globals *g)
{
  int err = get_terminal_width_height(g, &g->columns, &g->rows);
  if (g->rows > MAX_SCR_ROWS)
    g->rows = MAX_SCR_ROWS;
  if (g->columns > MAX_SCR_COLS)
    g->columns = MAX_SCR_COLS;
  return err;
}

static void fflush_all(void) {
}

// sleep for 'h' 1/100 seconds, return 1/0 if stdin is (ready for read)/(not ready)
static int mysleep(struct globals *g, int hund)
{
  //return g->e->peek(g->e->data, hund*10);
  EvtPumpEvents(hund * 10000);
  return EvtSysEventAvail(true);
}

//----- Set terminal attributes --------------------------------
static void rawmode(void)
{
// XXX
  // no TERMIOS_CLEAR_ISIG: leave ISIG on - allow signals
  //set_termios_to_raw(STDIN_FILENO, &term_orig, TERMIOS_RAW_CRNL);
}

static void cookmode(void)
{
// XXX
  //fflush_all();
  //tcsetattr_stdin_TCSANOW(&term_orig);
}

//----- Terminal Drawing ---------------------------------------
// The terminal is made up of 'rows' line of 'columns' columns.
// classically this would be 24 x 80.
//  screen coordinates
//  0,0     ...     0,79
//  1,0     ...     1,79
//  .       ...     .
//  .       ...     .
//  22,0    ...     22,79
//  23,0    ...     23,79   <- status line

//----- Move the cursor to row x col (count from 0, not 1) -------
static void place_cursor(struct globals *g, int row, int col)
{
  if (row < 0) row = 0;
  if (row >= g->rows) row = g->rows - 1;
  if (col < 0) col = 0;
  if (col >= g->columns) col = g->columns - 1;
  debug(DEBUG_TRACE, "vi", "cursor %2d,%2d", col, row);
  g->e->cursor(g->e->data, col, row);
}

//----- Erase from cursor to end of line -----------------------
static void clear_to_eol(struct globals *g)
{
  g->e->clreol(g->e->data, g->backgroundColor);
}

static void go_bottom_and_clear_to_eol(struct globals *g)
{
  place_cursor(g, g->rows - 1, 0);
  clear_to_eol(g);
}

//----- Start standout mode ------------------------------------
static void standout_start(void)
{
}

//----- End standout mode --------------------------------------
static void standout_end(void)
{
}

//----- Text Movement Routines ---------------------------------
static char *begin_line(struct globals *g, char *p) // return pointer to first char cur line
{
  if (p > g->text) {
    p = sys_memrchr(g->text, '\n', p - g->text);
    if (!p)
      return g->text;
    return p + 1;
  }
  return p;
}

static char *end_line(struct globals *g, char *p) // return pointer to NL of cur line
{
  if (p < g->end - 1) {
    p = sys_memchr(p, '\n', g->end - p - 1);
    if (!p)
      return g->end - 1;
  }
  return p;
}

static char *dollar_line(struct globals *g, char *p) // return pointer to just before NL line
{
  p = end_line(g, p);
  // Try to stay off of the Newline
  if (*p == '\n' && (p - begin_line(g, p)) > 0)
    p--;
  return p;
}

static char *prev_line(struct globals *g, char *p) // return pointer first char prev line
{
  p = begin_line(g, p);  // goto beginning of cur line
  if (p > g->text && p[-1] == '\n')
    p--;      // step to prev line
  p = begin_line(g, p);  // goto beginning of prev line
  return p;
}

static char *next_line(struct globals *g, char *p) // return pointer first char next line
{
  p = end_line(g, p);
  if (p < g->end - 1 && *p == '\n')
    p++;      // step to next line
  return p;
}

//----- Text Information Routines ------------------------------
static char *end_screen(struct globals *g)
{
  char *q;
  int cnt;

  // find new bottom line
  q = g->screenbegin;
  for (cnt = 0; cnt < g->rows - 2; cnt++)
    q = next_line(g, q);
  q = end_line(g, q);
  return q;
}

// count line from start to stop
static int count_lines(struct globals *g, char *start, char *stop)
{
  char *q;
  int cnt;

  if (stop < start) { // start and stop are backwards- reverse them
    q = start;
    start = stop;
    stop = q;
  }
  cnt = 0;
  stop = end_line(g, stop);
  while (start <= stop && start <= g->end - 1) {
    start = end_line(g, start);
    if (*start == '\n')
      cnt++;
    start++;
  }
  return cnt;
}

static char *find_line(struct globals *g, int li)  // find beginning of line #li
{
  char *q;

  for (q = g->text; li > 1; li--) {
    q = next_line(g, q);
  }
  return q;
}

static int next_tabstop(struct globals *g, int col)
{
  return col + ((g->tabstop - 1) - (col % g->tabstop));
}

//----- Erase the Screen[] memory ------------------------------
static void screen_erase(struct globals *g)
{
  sys_memset(g->screen, ' ', g->screensize);  // clear new screen
}

static void new_screen(struct globals *g, int ro, int co)
{
  char *s;

  sys_free(g->screen);
  g->screensize = ro * co + 8;
  s = g->screen = sys_malloc(g->screensize);
  // initialize the new screen. assume this will be a empty file.
  screen_erase(g);
  // non-existent text[] lines start with a tilde (~).
  //screen[(1 * co) + 0] = '~';
  //screen[(2 * co) + 0] = '~';
  //..
  //screen[((ro-2) * co) + 0] = '~';
  ro -= 2;
  while (--ro >= 0) {
    s += co;
    *s = '~';
  }
}

//----- Synchronize the cursor to Dot --------------------------
static NOINLINE void sync_cursor(struct globals *g, char *d, int *row, int *col)
{
  char *beg_cur;  // begin and end of "d" line
  char *tp;
  int cnt, ro, co;

  beg_cur = begin_line(g, d);  // first char of cur line

  if (beg_cur < g->screenbegin) {
    // "d" is before top line on screen
    // how many lines do we have to move
    cnt = count_lines(g, beg_cur, g->screenbegin);
 sc1:
    g->screenbegin = beg_cur;
    if (cnt > (g->rows - 1) / 2) {
      // we moved too many lines. put "dot" in middle of screen
      for (cnt = 0; cnt < (g->rows - 1) / 2; cnt++) {
        g->screenbegin = prev_line(g, g->screenbegin);
      }
    }
  } else {
    char *end_scr;  // begin and end of screen
    end_scr = end_screen(g);  // last char of screen
    if (beg_cur > end_scr) {
      // "d" is after bottom line on screen
      // how many lines do we have to move
      cnt = count_lines(g, end_scr, beg_cur);
      if (cnt > (g->rows - 1) / 2)
        goto sc1;  // too many lines
      for (ro = 0; ro < cnt - 1; ro++) {
        // move screen begin the same amount
        g->screenbegin = next_line(g, g->screenbegin);
        // now, move the end of screen
        end_scr = next_line(g, end_scr);
        end_scr = end_line(g, end_scr);
      }
    }
  }
  // "d" is on screen- find out which row
  tp = g->screenbegin;
  for (ro = 0; ro < g->rows - 1; ro++) {  // drive "ro" to correct row
    if (tp == beg_cur)
      break;
    tp = next_line(g, tp);
  }

  // find out what col "d" is on
  co = 0;
  while (tp < d) { // drive "co" to correct column
    if (*tp == '\n') //vda || *tp == '\0')
      break;
    if (*tp == '\t') {
      // handle tabs like real vi
      if (d == tp && g->cmd_mode) {
        break;
      }
      co = next_tabstop(g, co);
    } else if ((unsigned char)*tp < ' ' || *tp == 0x7f) {
      co++; // display as ^X, use 2 columns
    }
    co++;
    tp++;
  }

  // "co" is the column where "dot" is.
  // The screen has "columns" columns.
  // The currently displayed columns are  0+offset -- columns+ofset
  // |-------------------------------------------------------------|
  //               ^ ^                                ^
  //        offset | |------- columns ----------------|
  //
  // If "co" is already in this range then we do not have to adjust offset
  //      but, we do have to subtract the "offset" bias from "co".
  // If "co" is outside this range then we have to change "offset".
  // If the first char of a line is a tab the cursor will try to stay
  //  in column 7, but we have to set offset to 0.

  if (co < 0 + g->offset) {
    g->offset = co;
  }
  if (co >= g->columns + g->offset) {
    g->offset = co - g->columns + 1;
  }
  // if the first char of the line is a tab, and "dot" is sitting on it
  //  force offset to 0.
  if (d == beg_cur && *d == '\t') {
    g->offset = 0;
  }
  co -= g->offset;

  *row = ro;
  *col = co;
}

//----- Format a text[] line into a buffer ---------------------
static char* format_line(struct globals *g, char *src /*, int li*/)
{
  unsigned char c;
  int co;
  int ofs = g->offset;
  char *dest = g->scr_out_buf; // [MAX_SCR_COLS + MAX_TABSTOP * 2]

  debug(DEBUG_TRACE, "vi", "format_line [%s]", src);
  c = '~'; // char in col 0 in non-existent lines is '~'
  co = 0;
  while (co < g->columns + g->tabstop) {
    // have we gone past the end?
    if (src < g->end) {
      c = *src++;
      if (c == '\n')
        break;
      if ((c & 0x80) && !Isprint(c)) {
        c = '.';
      }
      if (c < ' ' || c == 0x7f) {
        if (c == '\t') {
          c = ' ';
          //      co %    8     !=     7
          while ((co % g->tabstop) != (g->tabstop - 1)) {
            dest[co++] = c;
          }
        } else {
          dest[co++] = '^';
          if (c == 0x7f)
            c = '?';
          else
            c += '@'; // Ctrl-X -> 'X'
        }
      }
    }
    dest[co++] = c;
    // discard scrolled-off-to-the-left portion,
    // in tabstop-sized pieces
    if (ofs >= g->tabstop && co >= g->tabstop) {
      sys_memmove(dest, dest + g->tabstop, co);
      co -= g->tabstop;
      ofs -= g->tabstop;
    }
    if (src >= g->end)
      break;
  }
  // check "short line, gigantic offset" case
  if (co < ofs)
    ofs = co;
  // discard last scrolled off part
  co -= ofs;
  dest += ofs;
  // fill the rest with spaces
  if (co < g->columns)
    sys_memset(&dest[co], ' ', g->columns - co);
  return dest;
}

static void edit_print(void *data, uint32_t fg, uint32_t bg, char *s, int n) {
  struct globals *g = (struct globals *)data;
  
  if (s && n) {
    debug(DEBUG_TRACE, "vi", "sequence fg=0x%08X bg=0x%08X [%.*s]", fg, bg, n, s);
    g->e->color(g->e->data, fg, bg);
    g->e->write(g->e->data, s, n);
    g->e->color(g->e->data, g->foregroundColor, g->backgroundColor);
  }
} 

static void draw_line(struct globals *g, char *line, int len) {
  int i;

  if (g->syntax && g->cmd_mode == 0) {
    g->syntax->syntax_begin_line(g->shigh, edit_print, g);
    for (i = 0; i < len; i++) {
      g->syntax->syntax_char(g->shigh, line[i]);
    }
    g->syntax->syntax_end_line(g->shigh);
  } else {
    edit_print(g, g->foregroundColor, g->backgroundColor, line, len);
  }
}

//----- Refresh the changed screen lines -----------------------
// Copy the source line from text[] into the buffer and note
// if the current screenline is different from the new buffer.
// If they differ then that line needs redrawing on the terminal.
//
static void refresh(struct globals *g, int full_screen)
{
#define old_offset g->refresh__old_offset

  int li, changed;
  char *tp, *sp;    // pointer into text[] and screen[]

  if (ENABLE_FEATURE_VI_WIN_RESIZE IF_FEATURE_VI_ASK_TERMINAL(&& !g->get_rowcol_error) ) {
    unsigned c = g->columns, r = g->rows;
    query_screen_dimensions(g);
#if ENABLE_FEATURE_VI_USE_SIGNALS
    full_screen |= (c - g->columns) | (r - g->rows);
#else
    if (c != g->columns || r != g->rows) {
      full_screen = TRUE;
      // update screen memory since SIGWINCH won't have done it
      new_screen(g, g->rows, g->columns);
    }
#endif
  }
  sync_cursor(g, g->dot, &g->crow, &g->ccol);  // where cursor will be (on "dot")
  tp = g->screenbegin;  // index into text[] of top line

  // compare text[] to screen[] and mark screen[] lines that need updating
  debug(DEBUG_TRACE, "vi", "refresh");
  for (li = 0; li < g->rows - 1; li++) {
    int cs, ce;        // column start & end
    char *out_buf;

    if (!tp[0]) {
      g->e->color(g->e->data, g->foregroundColor, g->backgroundColor);
      if (g->syntax) g->syntax->syntax_reset(g->shigh);
    }
    // format current text line
    out_buf = format_line(g, tp /*, li*/);

    // skip to the end of the current text[] line
    if (tp < g->end) {
      char *t = sys_memchr(tp, '\n', g->end - tp);
      if (!t) t = g->end - 1;
      tp = t + 1;
    }

    // see if there are any changes between virtual screen and out_buf
    changed = FALSE;  // assume no change
    cs = 0;
    ce = g->columns - 1;
    sp = &g->screen[li * g->columns];  // start of screen line
    if (full_screen) {
      // force re-draw of every single column from 0 - columns-1
      goto re0;
    }
    // compare newly formatted buffer with virtual screen
    // look forward for first difference between buf and screen
    for (; cs <= ce; cs++) {
      if (out_buf[cs] != sp[cs]) {
        changed = TRUE;  // mark for redraw
        break;
      }
    }

    // look backward for last difference between out_buf and screen
    for (; ce >= cs; ce--) {
      if (out_buf[ce] != sp[ce]) {
        changed = TRUE;  // mark for redraw
        break;
      }
    }
    // now, cs is index of first diff, and ce is index of last diff

    // if horz offset has changed, force a redraw
    if (g->offset != old_offset) {
 re0:
      changed = TRUE;
    }

    // make a sanity check of columns indexes
    if (cs < 0) cs = 0;
    if (ce > g->columns - 1) ce = g->columns - 1;
    if (cs > ce) { cs = 0; ce = g->columns - 1; }
    // is there a change between virtual screen and out_buf
    if (changed) {
      // copy changed part of buffer to virtual screen
      sys_memcpy(sp+cs, out_buf+cs, ce-cs+1);
      place_cursor(g, li, cs);
      // write line out to terminal
      draw_line(g, &sp[cs], ce - cs + 1);
    }
  }

  place_cursor(g, g->crow, g->ccol);
  if (g->syntax) g->syntax->syntax_reset(g->shigh);

  old_offset = g->offset;
#undef old_offset
}

//----- Force refresh of all Lines -----------------------------
static void redraw(struct globals *g, int full_screen)
{
  // cursor to top,left; clear to the end of screen
  g->e->cls(g->e->data, g->backgroundColor);
  screen_erase(g);    // erase the internal screen buffer
  g->last_status_cksum = 0;  // force status update
  refresh(g, full_screen);  // this will redraw the entire display
  show_status_line(g);
}

//----- Flash the screen  --------------------------------------
static void flash(struct globals *g, int h)
{
  standout_start();
  redraw(g, TRUE);
  mysleep(g, h);
  standout_end();
  redraw(g, TRUE);
}

static void indicate_error(struct globals *g)
{
#if ENABLE_FEATURE_VI_CRASHME
  if (crashme > 0)
    return;
#endif
  flash(g, 10);
}

static uint64_t read_key(struct globals *g, char *buffer, int timeout) {
  unsigned char c;
  uint16_t c16;
  int ch, n, r;

  buffer++; /* saved chars counter is in buffer[-1] now */
  //errno = 0;
  n = (unsigned char)buffer[-1];
  ch = 0;

  if (n == 0) {
    for (;;) {
      r = g->e->read(g->e->data, &c16);
      if (r == -1) return -1;
      if (r == 1) break;
    }
    ch = c16;
    n = 1;
  }

  buffer[0] = ch;
  c = buffer[0];
  n--;
  if (n)
    sys_memmove(buffer, buffer + 1, n);
  buffer[-1] = n;

  return c;
}

static void bb_error_msg_and_die(char *s) {
}

//----- IO Routines --------------------------------------------
static int readit(struct globals *g) // read (maybe cursor) key from stdin
{
  int c;

  fflush_all();

  // Wait for input. TIMEOUT = -1 makes read_key wait even
  // on nonblocking stdin.
  // Note: read_key sets errno to 0 on success.
 //again:
  c = read_key(g, g->readbuffer, /*timeout:*/ -1);
  if (c == -1) { // EOF/error
/*
    if (errno == EAGAIN) // paranoia
      goto again;
*/
    go_bottom_and_clear_to_eol(g);
    cookmode(); // terminal to "cooked"
    bb_error_msg_and_die("can't read user input");
  }
  return c;
}

#if ENABLE_FEATURE_VI_DOT_CMD
static int get_one_char(struct globals *g)
{
  int c;

  if (!g->adding2q) {
    // we are not adding to the q.
    // but, we may be reading from a saved q.
    // (checking "ioq" for NULL is wrong, it's not reset to NULL
    // when done - "ioq_start" is reset instead).
    if (g->ioq_start != NULL) {
      // there is a queue to get chars from.
      // careful with correct sign expansion!
      c = (unsigned char)*g->ioq++;
      if (c != '\0')
        return c;
      // the end of the q
      sys_free(g->ioq_start);
      g->ioq_start = NULL;
      // read from STDIN:
    }
    return readit(g);
  }
  // we are adding STDIN chars to q.
  c = readit(g);
  if (g->lmc_len >= ARRAY_SIZE(g->last_modifying_cmd) - 1) {
    // last_modifying_cmd[] is too small, can't remeber the cmd
    // - drop it
    g->adding2q = 0;
    g->lmc_len = 0;
  } else {
    g->last_modifying_cmd[g->lmc_len++] = c;
  }
  return c;
}
#else
# define get_one_char(g) readit(g)
#endif


static void bb_putchar(struct globals *g, int c) {
  char ch = c;
  g->e->write(g->e->data, &ch, 1);
}

// Get input line (uses "status line" area)
static char *get_input_line(struct globals *g, const char *prompt)
{
  // char [MAX_INPUT_LEN]
#define buf g->get_input_line__buf

  int c;
  int i;

  sys_strcpy(buf, prompt);
  g->last_status_cksum = 0;  // force status update
  go_bottom_and_clear_to_eol(g);
  write1(g, prompt);      // write out the :, /, or ? prompt

  i = sys_strlen(buf);
  while (i < MAX_INPUT_LEN) {
    c = get_one_char(g);
    if (c == '\n' || c == '\r' || c == 27)
      break;    // this is end of input
    if (/*c == term_orig.c_cc[VERASE] ||*/ c == 8 || c == 127) {
      // user wants to erase prev char
      buf[--i] = '\0';
      write1(g, "\b \b"); // erase char on screen
      if (i <= 0) // user backs up before b-o-l, exit
        break;
    } else if (c > 0 && c < 256) { // exclude Unicode
      // (TODO: need to handle Unicode)
      buf[i] = c;
      buf[++i] = '\0';
      bb_putchar(g, c);
    }
  }
  refresh(g, FALSE);
  return buf;
#undef buf
}

static void Hit_Return(struct globals *g)
{
  int c;

  standout_start();
  write1(g, "[Hit return to continue]");
  standout_end();
  while ((c = get_one_char(g)) != '\n' && c != '\r')
    continue;
  redraw(g, TRUE);    // force redraw all
}

//----- Draw the status line at bottom of the screen -------------
// show file status on status line
static int format_edit_status(struct globals *g)
{
  static const char cmd_mode_indicator[] ALIGN1 = "-IR-";

#define tot g->format_edit_status__tot

  int cur, percent, ret, trunc_at;

  // modified_count is now a counter rather than a flag.  this
  // helps reduce the amount of line counting we need to do.
  // (this will cause a mis-reporting of modified status
  // once every MAXINT editing operations.)

  // it would be nice to do a similar optimization here -- if
  // we haven't done a motion that could have changed which line
  // we're on, then we shouldn't have to do this count_lines()
  cur = count_lines(g, g->text, g->dot);

  // count_lines() is expensive.
  // Call it only if something was changed since last time
  // we were here:
  if (g->modified_count != g->last_modified_count) {
    tot = cur + count_lines(g, g->dot, g->end - 1) - 1;
    g->last_modified_count = g->modified_count;
  }

  //    current line         percent
  //   -------------    ~~ ----------
  //    total lines            100
  if (tot > 0) {
    percent = (100 * cur) / tot;
  } else {
    cur = tot = 0;
    percent = 100;
  }

  trunc_at = g->columns < STATUS_BUFFER_LEN-1 ?
    g->columns : STATUS_BUFFER_LEN-1;

  ret = sys_snprintf(g->status_buffer, trunc_at+1,
#if ENABLE_FEATURE_VI_READONLY
    "%c %s%s%s %d/%d %d%%",
#else
    "%c %s%s %d/%d %d%%",
#endif
    cmd_mode_indicator[g->cmd_mode & 3],
    (g->current_filename != NULL ? g->current_filename : "No file"),
#if ENABLE_FEATURE_VI_READONLY
    (g->readonly_mode ? " [Readonly]" : ""),
#endif
    (g->modified_count ? " [Modified]" : ""),
    cur, tot, percent);

  if (ret >= 0 && ret < trunc_at)
    return ret;  // it all fit

  return trunc_at;  // had to truncate
#undef tot
}

static int bufsum(char *buf, int count)
{
  int sum = 0;
  char *e = buf + count;
  while (buf < e)
    sum += (unsigned char) *buf++;
  return sum;
}

static void show_status_line(struct globals *g)
{
  int cnt = 0, cksum = 0;

  // either we already have an error or status message, or we
  // create one.
  if (!g->have_status_msg) {
    cnt = format_edit_status(g);
    cksum = bufsum(g->status_buffer, cnt);
  }
  if (g->have_status_msg || ((cnt > 0 && g->last_status_cksum != cksum))) {
    g->last_status_cksum = cksum;    // remember if we have seen this line
    go_bottom_and_clear_to_eol(g);
    write1(g, g->status_buffer);
    if (g->have_status_msg) {
      if (((int)sys_strlen(g->status_buffer) - (g->have_status_msg - 1)) >
          (g->columns - 1) ) {
        g->have_status_msg = 0;
        Hit_Return(g);
      }
      g->have_status_msg = 0;
    }
    place_cursor(g, g->crow, g->ccol);  // put cursor back in correct place
  }
  fflush_all();
}

//----- format the status buffer, the bottom line of screen ------
static void status_line(struct globals *g, const char *format, ...)
{
  sys_va_list args;

  sys_va_start(args, format);
  sys_vsnprintf(g->status_buffer, STATUS_BUFFER_LEN, format, args);
  sys_va_end(args);

  g->have_status_msg = 1;
}
static void status_line_bold(struct globals *g, const char *format, ...)
{
  sys_va_list args;

  sys_va_start(args, format);
  sys_vsnprintf(g->status_buffer, STATUS_BUFFER_LEN, format, args);
  sys_va_end(args);

  g->have_status_msg = 1;
}
static void status_line_bold_errno(struct globals *g, const char *fn)
{
  status_line_bold(g, "'%s' "STRERROR_FMT, fn STRERROR_ERRNO);
}

// copy s to buf, convert unprintable
static void print_literal(char *buf, const char *s)
{
  char *d;
  unsigned char c;

  buf[0] = '\0';
  if (!s[0])
    s = "(NULL)";

  d = buf;
  for (; *s; s++) {
    int c_is_no_print;

    c = *s;
    c_is_no_print = (c & 0x80) && !Isprint(c);
    if (c_is_no_print) {
      c = '.';
    }
    if (c < ' ' || c == 0x7f) {
      *d++ = '^';
      c |= '@'; // 0x40
      if (c == 0x7f)
        c = '?';
    }
    *d++ = c;
    *d = '\0';
    if (c_is_no_print) {
    }
    if (*s == '\n') {
      *d++ = '$';
      *d = '\0';
    }
    if (d - buf > MAX_INPUT_LEN - 10) // paranoia
      break;
  }
}
static void not_implemented(struct globals *g, const char *s)
{
  char buf[MAX_INPUT_LEN];
  print_literal(buf, s);
  status_line_bold(g, "'%s' is not implemented", buf);
}

//----- Block insert/delete, undo ops --------------------------
#if ENABLE_FEATURE_VI_YANKMARK
static char *text_yank(struct globals *g, char *p, char *q, int dest)  // copy text into a register
{
  int cnt = q - p;
  if (cnt < 0) {    // they are backwards- reverse them
    p = q;
    cnt = -cnt;
  }
  sys_free(g->reg[dest]);  //  if already a yank register, free it
  g->reg[dest] = sys_strndup(p, cnt + 1);
  return p;
}

static char what_reg(struct globals *g)
{
  char c;

  c = 'D';      // default to D-reg
  if (g->YDreg <= 25)
    c = 'a' + (char) g->YDreg;
  if (g->YDreg == 26)
    c = 'D';
  if (g->YDreg == 27)
    c = 'U';
  return c;
}

static void check_context(struct globals *g, char cmd)
{
  // A context is defined to be "modifying text"
  // Any modifying command establishes a new context.

  if (g->dot < g->context_start || g->dot > g->context_end) {
    if (sys_strchr(modifying_cmds, cmd) != NULL) {
      // we are trying to modify text[]- make this the current context
      g->mark[27] = g->mark[26];  // move cur to prev
      g->mark[26] = g->dot;  // move local to cur
      g->context_start = prev_line(g, prev_line(g, g->dot));
      g->context_end = next_line(g, next_line(g, g->dot));
      //loiter= start_loiter= now;
    }
  }
}

static char *swap_context(struct globals *g, char *p) // goto new context for '' command make this the current context
{
  char *tmp;

  // the current context is in mark[26]
  // the previous context is in mark[27]
  // only swap context if other context is valid
  if (g->text <= g->mark[27] && g->mark[27] <= g->end - 1) {
    tmp = g->mark[27];
    g->mark[27] = p;
    g->mark[26] = p = tmp;
    g->context_start = prev_line(g, prev_line(g, prev_line(g, p)));
    g->context_end = next_line(g, next_line(g, next_line(g, p)));
  }
  return p;
}
#endif /* FEATURE_VI_YANKMARK */

#if ENABLE_FEATURE_VI_UNDO
static void undo_push(struct globals *g, char *, unsigned, unsigned char);
#endif

// open a hole in text[]
// might reallocate text[]! use p += text_hole_make(p, ...),
// and be careful to not use pointers into potentially freed text[]!
static uintptr_t text_hole_make(struct globals *g, char *p, int size)  // at "p", make a 'size' byte hole
{
  uintptr_t bias = 0;

  if (size <= 0)
    return bias;
  g->end += size;    // adjust the new END
  if (g->end >= (g->text + g->text_size)) {
    char *new_text;
    g->text_size += g->end - (g->text + g->text_size) + 10240;
    new_text = sys_realloc(g->text, g->text_size);
    bias = (new_text - g->text);
    g->screenbegin += bias;
    g->dot         += bias;
    g->end         += bias;
    p           += bias;
#if ENABLE_FEATURE_VI_YANKMARK
    {
      int i;
      for (i = 0; i < ARRAY_SIZE(g->mark); i++)
        if (g->mark[i])
          g->mark[i] += bias;
    }
#endif
    g->text = new_text;
  }
  sys_memmove(p + size, p, g->end - size - p);
  sys_memset(p, ' ', size);  // clear new hole
  return bias;
}

// close a hole in text[] - delete "p" through "q", inclusive
// "undo" value indicates if this operation should be undo-able
#if !ENABLE_FEATURE_VI_UNDO
#define text_hole_delete(a,b,c) text_hole_delete(a,b)
#endif
static char *text_hole_delete(struct globals *g, char *p, char *q, int undo)
{
  char *src, *dest;
  int cnt, hole_size;

  // move forwards, from beginning
  // assume p <= q
  src = q + 1;
  dest = p;
  if (q < p) {    // they are backward- swap them
    src = p + 1;
    dest = q;
  }
  hole_size = q - p + 1;
  cnt = g->end - src;
#if ENABLE_FEATURE_VI_UNDO
  switch (undo) {
    case NO_UNDO:
      break;
    case ALLOW_UNDO:
      undo_push(g, p, hole_size, UNDO_DEL);
      break;
    case ALLOW_UNDO_CHAIN:
      undo_push(g, p, hole_size, UNDO_DEL_CHAIN);
      break;
# if ENABLE_FEATURE_VI_UNDO_QUEUE
    case ALLOW_UNDO_QUEUED:
      undo_push(g, p, hole_size, UNDO_DEL_QUEUED);
      break;
# endif
  }
  g->modified_count--;
#endif
  if (src < g->text || src > g->end)
    goto thd0;
  if (dest < g->text || dest >= g->end)
    goto thd0;
  g->modified_count++;
  if (src >= g->end)
    goto thd_atend;  // just delete the end of the buffer
  sys_memmove(dest, src, cnt);
 thd_atend:
  g->end = g->end - hole_size;  // adjust the new END
  if (dest >= g->end)
    dest = g->end - 1;  // make sure dest in below end-1
  if (g->end <= g->text)
    dest = g->end = g->text;  // keep pointers valid
 thd0:
  return dest;
}

#if ENABLE_FEATURE_VI_UNDO

# if ENABLE_FEATURE_VI_UNDO_QUEUE
// Flush any queued objects to the undo stack
static void undo_queue_commit(struct globals *g)
{
  // Pushes the queue object onto the undo stack
  if (g->undo_q > 0) {
    // Deleted character undo events grow from the end
    undo_push(g, g->undo_queue + CONFIG_FEATURE_VI_UNDO_QUEUE_MAX - g->undo_q,
      g->undo_q,
      (g->undo_queue_state | UNDO_USE_SPOS)
    );
    g->undo_queue_state = UNDO_EMPTY;
    g->undo_q = 0;
  }
}
# else
#  define undo_queue_commit(g) ((void)0)
# endif

static void flush_undo_data(struct globals *g)
{
  struct undo_object *undo_entry;

  while (g->undo_stack_tail) {
    undo_entry = g->undo_stack_tail;
    g->undo_stack_tail = undo_entry->prev;
    sys_free(undo_entry);
  }
}

// Undo functions and hooks added by Jody Bruchon (jody@jodybruchon.com)
// Add to the undo stack
static void undo_push(struct globals *g, char *src, unsigned length, uint8_t u_type)
{
  struct undo_object *undo_entry;

  // "u_type" values
  // UNDO_INS: insertion, undo will remove from buffer
  // UNDO_DEL: deleted text, undo will restore to buffer
  // UNDO_{INS,DEL}_CHAIN: Same as above but also calls undo_pop() when complete
  // The CHAIN operations are for handling multiple operations that the user
  // performs with a single action, i.e. REPLACE mode or find-and-replace commands
  // UNDO_{INS,DEL}_QUEUED: If queuing feature is enabled, allow use of the queue
  // for the INS/DEL operation. The raw values should be equal to the values of
  // UNDO_{INS,DEL} ORed with UNDO_QUEUED_FLAG

# if ENABLE_FEATURE_VI_UNDO_QUEUE
  // This undo queuing functionality groups multiple character typing or backspaces
  // into a single large undo object. This greatly reduces calls to malloc() for
  // single-character operations while typing and has the side benefit of letting
  // an undo operation remove chunks of text rather than a single character.
  switch (u_type) {
  case UNDO_EMPTY:  // Just in case this ever happens...
    return;
  case UNDO_DEL_QUEUED:
    if (length != 1)
      return;  // Only queue single characters
    switch (g->undo_queue_state) {
    case UNDO_EMPTY:
      g->undo_queue_state = UNDO_DEL;
    case UNDO_DEL:
      g->undo_queue_spos = src;
      g->undo_q++;
      g->undo_queue[CONFIG_FEATURE_VI_UNDO_QUEUE_MAX - g->undo_q] = *src;
      // If queue is full, dump it into an object
      if (g->undo_q == CONFIG_FEATURE_VI_UNDO_QUEUE_MAX)
        undo_queue_commit(g);
      return;
    case UNDO_INS:
      // Switch from storing inserted text to deleted text
      undo_queue_commit(g);
      undo_push(g, src, length, UNDO_DEL_QUEUED);
      return;
    }
    break;
  case UNDO_INS_QUEUED:
    if (length < 1)
      return;
    switch (g->undo_queue_state) {
    case UNDO_EMPTY:
      g->undo_queue_state = UNDO_INS;
      g->undo_queue_spos = src;
    case UNDO_INS:
      while (length--) {
        g->undo_q++;  // Don't need to save any data for insertions
        if (g->undo_q == CONFIG_FEATURE_VI_UNDO_QUEUE_MAX)
          undo_queue_commit(g);
      }
      return;
    case UNDO_DEL:
      // Switch from storing deleted text to inserted text
      undo_queue_commit(g);
      undo_push(g, src, length, UNDO_INS_QUEUED);
      return;
    }
    break;
  }
# else
  // If undo queuing is disabled, ignore the queuing flag entirely
  u_type = u_type & ~UNDO_QUEUED_FLAG;
# endif

  // Allocate a new undo object
  if (u_type == UNDO_DEL || u_type == UNDO_DEL_CHAIN) {
    // For UNDO_DEL objects, save deleted text
    if ((g->text + length) == g->end)
      length--;
    // If this deletion empties text[], strip the newline. When the buffer becomes
    // zero-length, a newline is added back, which requires this to compensate.
    undo_entry = xzalloc(OffsetOf(struct undo_object, undo_text) + length);
    sys_memcpy(undo_entry->undo_text, src, length);
  } else {
    undo_entry = xzalloc(sizeof(*undo_entry));
  }
  undo_entry->length = length;
# if ENABLE_FEATURE_VI_UNDO_QUEUE
  if ((u_type & UNDO_USE_SPOS) != 0) {
    undo_entry->start = g->undo_queue_spos - g->text;  // use start position from queue
  } else {
    undo_entry->start = src - g->text;  // use offset from start of text buffer
  }
  u_type = (u_type & ~UNDO_USE_SPOS);
# else
  undo_entry->start = src - g->text;
# endif
  undo_entry->u_type = u_type;

  // Push it on undo stack
  undo_entry->prev = g->undo_stack_tail;
  g->undo_stack_tail = undo_entry;
  g->modified_count++;
}

static void undo_push_insert(struct globals *g, char *p, int len, int undo)
{
  switch (undo) {
  case ALLOW_UNDO:
    undo_push(g, p, len, UNDO_INS);
    break;
  case ALLOW_UNDO_CHAIN:
    undo_push(g, p, len, UNDO_INS_CHAIN);
    break;
# if ENABLE_FEATURE_VI_UNDO_QUEUE
  case ALLOW_UNDO_QUEUED:
    undo_push(g, p, len, UNDO_INS_QUEUED);
    break;
# endif
  }
}

// Undo the last operation
static void undo_pop(struct globals *g)
{
  int repeat;
  char *u_start, *u_end;
  struct undo_object *undo_entry;

  // Commit pending undo queue before popping (should be unnecessary)
  undo_queue_commit(g);

  undo_entry = g->undo_stack_tail;
  // Check for an empty undo stack
  if (!undo_entry) {
    status_line(g, "Already at oldest change");
    return;
  }

  switch (undo_entry->u_type) {
  case UNDO_DEL:
  case UNDO_DEL_CHAIN:
    // make hole and put in text that was deleted; deallocate text
    u_start = g->text + undo_entry->start;
    text_hole_make(g, u_start, undo_entry->length);
    sys_memcpy(u_start, undo_entry->undo_text, undo_entry->length);
    status_line(g, "Undo [%d] %s %d chars at position %d",
      g->modified_count, "restored",
      undo_entry->length, undo_entry->start
    );
    break;
  case UNDO_INS:
  case UNDO_INS_CHAIN:
    // delete what was inserted
    u_start = undo_entry->start + g->text;
    u_end = u_start - 1 + undo_entry->length;
    text_hole_delete(g, u_start, u_end, NO_UNDO);
    status_line(g, "Undo [%d] %s %d chars at position %d",
      g->modified_count, "deleted",
      undo_entry->length, undo_entry->start
    );
    break;
  }
  repeat = 0;
  switch (undo_entry->u_type) {
  // If this is the end of a chain, lower modification count and refresh display
  case UNDO_DEL:
  case UNDO_INS:
    g->dot = (g->text + undo_entry->start);
    refresh(g, FALSE);
    break;
  case UNDO_DEL_CHAIN:
  case UNDO_INS_CHAIN:
    repeat = 1;
    break;
  }
  // Deallocate the undo object we just processed
  g->undo_stack_tail = undo_entry->prev;
  sys_free(undo_entry);
  g->modified_count--;
  // For chained operations, continue popping all the way down the chain.
  if (repeat) {
    undo_pop(g);  // Follow the undo chain if one exists
  }
}

#else
# define flush_undo_data(g)   ((void)0)
# define undo_queue_commit(g) ((void)0)
#endif /* ENABLE_FEATURE_VI_UNDO */

//----- Dot Movement Routines ----------------------------------
static void dot_left(struct globals *g)
{
  undo_queue_commit(g);
  if (g->dot > g->text && g->dot[-1] != '\n')
    g->dot--;
}

static void dot_right(struct globals *g)
{
  undo_queue_commit(g);
  if (g->dot < g->end - 1 && *g->dot != '\n')
    g->dot++;
}

static void dot_begin(struct globals *g)
{
  undo_queue_commit(g);
  g->dot = begin_line(g, g->dot);  // return pointer to first char cur line
}

static void dot_end(struct globals *g)
{
  undo_queue_commit(g);
  g->dot = end_line(g, g->dot);  // return pointer to last char cur line
}

static char *move_to_col(struct globals *g, char *p, int l)
{
  int co;

  p = begin_line(g, p);
  co = 0;
  while (co < l && p < g->end) {
    if (*p == '\n') //vda || *p == '\0')
      break;
    if (*p == '\t') {
      co = next_tabstop(g, co);
    } else if (*p < ' ' || *p == 127) {
      co++; // display as ^X, use 2 columns
    }
    co++;
    p++;
  }
  return p;
}

static void dot_next(struct globals *g)
{
  undo_queue_commit(g);
  g->dot = next_line(g, g->dot);
}

static void dot_prev(struct globals *g)
{
  undo_queue_commit(g);
  g->dot = prev_line(g, g->dot);
}

static void dot_skip_over_ws(struct globals *g)
{
  // skip WS
  while (sys_isspace(*g->dot) && *g->dot != '\n' && g->dot < g->end - 1)
    g->dot++;
}

static void dot_scroll(struct globals *g, int cnt, int dir)
{
  char *q;

  undo_queue_commit(g);
  for (; cnt > 0; cnt--) {
    if (dir < 0) {
      // scroll Backwards
      // ctrl-Y scroll up one line
      g->screenbegin = prev_line(g, g->screenbegin);
    } else {
      // scroll Forwards
      // ctrl-E scroll down one line
      g->screenbegin = next_line(g, g->screenbegin);
    }
  }
  // make sure "dot" stays on the screen so we dont scroll off
  if (g->dot < g->screenbegin)
    g->dot = g->screenbegin;
  q = end_screen(g);  // find new bottom line
  if (g->dot > q)
    g->dot = begin_line(g, q);  // is dot is below bottom line?
  dot_skip_over_ws(g);
}

static char *bound_dot(struct globals *g, char *p) // make sure  text[0] <= P < "end"
{
  if (p >= g->end && g->end > g->text) {
    p = g->end - 1;
    indicate_error(g);
  }
  if (p < g->text) {
    p = g->text;
    indicate_error(g);
  }
  return p;
}

#if ENABLE_FEATURE_VI_DOT_CMD
static void start_new_cmd_q(struct globals *g, char c)
{
  // get buffer for new cmd
  // if there is a current cmd count put it in the buffer first
  if (g->cmdcnt > 0) {
    g->lmc_len = sys_sprintf(g->last_modifying_cmd, "%u%c", g->cmdcnt, c);
  } else { // just save char c onto queue
    g->last_modifying_cmd[0] = c;
    g->lmc_len = 1;
  }
  g->adding2q = 1;
}
static void end_cmd_q(struct globals *g)
{
# if ENABLE_FEATURE_VI_YANKMARK
  g->YDreg = 26;      // go back to default Yank/Delete reg
# endif
  g->adding2q = 0;
}
#else
# define end_cmd_q(g) ((void)0)
#endif /* FEATURE_VI_DOT_CMD */

// copy text into register, then delete text.
// if dist <= 0, do not include, or go past, a NewLine
//
#if !ENABLE_FEATURE_VI_UNDO
#define yank_delete(a,b,c,d,e) yank_delete(a,b,c,d)
#endif
static char *yank_delete(struct globals *g, char *start, char *stop, int dist, int yf, int undo)
{
  char *p;

  // make sure start <= stop
  if (start > stop) {
    // they are backwards, reverse them
    p = start;
    start = stop;
    stop = p;
  }
  if (dist <= 0) {
    // we cannot cross NL boundaries
    p = start;
    if (*p == '\n')
      return p;
    // dont go past a NewLine
    for (; p + 1 <= stop; p++) {
      if (p[1] == '\n') {
        stop = p;  // "stop" just before NewLine
        break;
      }
    }
  }
  p = start;
#if ENABLE_FEATURE_VI_YANKMARK
  text_yank(g, start, stop, g->YDreg);
#endif
  if (yf == YANKDEL) {
    p = text_hole_delete(g, start, stop, undo);
  }          // delete lines
  return p;
}

static int full_read(struct globals *g, void *fd, char *p, int size) {
  return g->e->fread(g->e->data, fd, p, size);
}

// might reallocate text[]!
static int file_insert(struct globals *g, const char *fn, char *p, int initial)
{
  int cnt = -1;
  int size;
  void *fd;

  if (p < g->text)
    p = g->text;
  if (p > g->end)
    p = g->end;

  fd = g->e->fopen(g->e->data, (char *)fn, 0);
  if (fd == NULL) {
    if (!initial)
      status_line_bold_errno(g, fn);
    return cnt;
  }

  #define MAX_FILE_SIZE 256*1024

  size = g->e->fsize(g->e->data, fd);
  if (size > MAX_FILE_SIZE) size = MAX_FILE_SIZE;

  p += text_hole_make(g, p, size);
  cnt = full_read(g, fd, p, size);
  if (cnt < 0) {
    status_line_bold_errno(g, fn);
    p = text_hole_delete(g, p, p + size - 1, NO_UNDO);  // un-do buffer insert
  } else if (cnt < size) {
    // There was a partial read, shrink unused space
    p = text_hole_delete(g, p + cnt, p + size - 1, NO_UNDO);
    status_line_bold(g, "can't read '%s'", fn);
  }
  g->e->fclose(g->e->data, fd);

#if ENABLE_FEATURE_VI_READONLY
#if 0
  if (initial
   && ((access(fn, W_OK) < 0) ||
    // root will always have access()
    // so we check fileperms too
    !(statbuf.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH))
      )
  ) {
    SET_READONLY_FILE(readonly_mode);
  }
#endif
/*
  if (!ent->wr) {
    SET_READONLY_FILE(readonly_mode);
  }
*/
#endif
  return cnt;
}

// find matching char of pair  ()  []  {}
// will crash if c is not one of these
static char *find_pair(struct globals *g, char *p, const char c)
{
  const char *braces = "()[]{}";
  char match;
  int dir, level;

  dir = sys_strchr(braces, c) - braces;
  dir ^= 1;
  match = braces[dir];
  dir = ((dir & 1) << 1) - 1; // 1 for ([{, -1 for )\}

  // look for match, count levels of pairs  (( ))
  level = 1;
  for (;;) {
    p += dir;
    if (p < g->text || p >= g->end)
      return NULL;
    if (*p == c)
      level++;  // increase pair levels
    if (*p == match) {
      level--;  // reduce pair level
      if (level == 0)
        return p; // found matching pair
    }
  }
}

#if ENABLE_FEATURE_VI_SETOPTS
// show the matching char of a pair,  ()  []  {}
static void showmatching(struct globals *g, char *p)
{
  char *q, *save_dot;

  // we found half of a pair
  q = find_pair(g, p, *p);  // get loc of matching char
  if (q == NULL) {
    indicate_error(g);  // no matching char
  } else {
    // "q" now points to matching pair
    save_dot = g->dot;  // remember where we are
    g->dot = q;    // go to new loc
    refresh(g, FALSE);  // let the user see it
    mysleep(g, 40);  // give user some time
    g->dot = save_dot;  // go back to old loc
    refresh(g, FALSE);
  }
}
#endif /* FEATURE_VI_SETOPTS */

// might reallocate text[]! use p += stupid_insert(p, ...),
// and be careful to not use pointers into potentially freed text[]!
static uintptr_t stupid_insert(struct globals *g, char *p, char c) // stupidly insert the char c at 'p'
{
  uintptr_t bias;
  bias = text_hole_make(g, p, 1);
  p += bias;
  *p = c;
  return bias;
}

static int xstrspn(const char *s, const char *accept) {
  int i, j;

  for (i = 0; s[i]; i++) {
    for (j = 0; accept[j]; j++) {
      if (s[i] == accept[j]) break;
    }
    if (!accept[j]) break;
  }

  return i;
}

#if !ENABLE_FEATURE_VI_UNDO
#define char_insert(a,b,c) char_insert(a,b)
#endif
static char *char_insert(struct globals *g, char *p, char c, int undo) // insert the char c at 'p'
{
  if (c == 22) {    // Is this an ctrl-V?
    p += stupid_insert(g, p, '^');  // use ^ to indicate literal next
    refresh(g, FALSE);  // show the ^
    c = get_one_char(g);
    *p = c;
#if ENABLE_FEATURE_VI_UNDO
    undo_push_insert(g, p, 1, undo);
#else
    g->modified_count++;
#endif
    p++;
  } else if (c == 27) {  // Is this an ESC?
    g->cmd_mode = 0;
    undo_queue_commit(g);
    g->cmdcnt = 0;
    end_cmd_q(g);  // stop adding to q
    g->last_status_cksum = 0;  // force status update
    if ((p[-1] != '\n') && (g->dot > g->text)) {
      p--;
    }
    refresh(g, 1); // YYY
  } else if (/*c == term_orig.c_cc[VERASE] ||*/ c == 8 || c == 127) { // Is this a BS
    if (p > g->text) {
      p--;
      p = text_hole_delete(g, p, p, ALLOW_UNDO_QUEUED);  // shrink buffer 1 char
    }
  } else {
    // insert a char into text[]
    if (c == 13)
      c = '\n';  // translate \r to \n
#if ENABLE_FEATURE_VI_UNDO
# if ENABLE_FEATURE_VI_UNDO_QUEUE
    if (c == '\n')
      undo_queue_commit(g);
# endif
    undo_push_insert(g, p, 1, undo);
#else
    g->modified_count++;
#endif
    p += 1 + stupid_insert(g, p, c);  // insert the char
#if ENABLE_FEATURE_VI_SETOPTS
    if (showmatch && sys_strchr(")]}", c) != NULL) {
      showmatching(g, p - 1);
    }
    if (autoindent && c == '\n') {  // auto indent the new line
      char *q;
      sys_size_t len;
      q = prev_line(g, p);  // use prev line as template
      len = xstrspn(q, " \t"); // space or tab
      if (len) {
        uintptr_t bias;
        bias = text_hole_make(g, p, len);
        p += bias;
        q += bias;
#if ENABLE_FEATURE_VI_UNDO
        undo_push_insert(g, p, len, undo);
#endif
        sys_memcpy(p, q, len);
        p += len;
      }
    }
#endif
  }
  return p;
}

// read text from file or create an empty buf
// will also update current_filename
static int init_text_buffer(struct globals *g, char *fn)
{
  int rc;

  // allocate/reallocate text buffer
  sys_free(g->text);
  g->text_size = 10240;
  g->screenbegin = g->dot = g->end = g->text = xzalloc(g->text_size);

  if (fn != g->current_filename) {
    sys_free(g->current_filename);
    g->current_filename = sys_strdup(fn);
  }
  rc = file_insert(g, fn, g->text, 1);
  if (rc < 0) {
    // file doesnt exist. Start empty buf with dummy line
    char_insert(g, g->text, '\n', NO_UNDO);
  }

  flush_undo_data(g);
  g->modified_count = 0;
  g->last_modified_count = -1;
#if ENABLE_FEATURE_VI_YANKMARK
  // init the marks
  sys_memset(g->mark, 0, sizeof(g->mark));
#endif
  return rc;
}

#if ENABLE_FEATURE_VI_YANKMARK \
 || (ENABLE_FEATURE_VI_COLON && ENABLE_FEATURE_VI_SEARCH) \
 || ENABLE_FEATURE_VI_CRASHME
// might reallocate text[]! use p += string_insert(p, ...),
// and be careful to not use pointers into potentially freed text[]!
# if !ENABLE_FEATURE_VI_UNDO
#  define string_insert(a,b,c) string_insert(a,b)
# endif
static uintptr_t string_insert(struct globals *g, char *p, const char *s, int undo) // insert the string at 'p'
{
  uintptr_t bias;
  int i;

  i = sys_strlen(s);
#if ENABLE_FEATURE_VI_UNDO
  undo_push_insert(g, p, i, undo);
#endif
  bias = text_hole_make(g, p, i);
  p += bias;
  sys_memcpy(p, s, i);
#if ENABLE_FEATURE_VI_YANKMARK
  {
    int cnt;
    for (cnt = 0; *s != '\0'; s++) {
      if (*s == '\n')
        cnt++;
    }
    status_line(g, "Put %d lines (%d chars) from [%c]", cnt, i, what_reg(g));
  }
#endif
  return bias;
}
#endif

static int full_write(struct globals *g, void *fd, char *first, int cnt) {
  return g->e->fwrite(g->e->data, fd, first, cnt);
}

static int file_write(struct globals *g, char *fn, char *first, char *last)
{
  void *fd;
  int cnt, charcnt;

  if (fn == 0) {
    status_line_bold(g, "No current filename");
    return -2;
  }
  fd = g->e->fopen(g->e->data, fn, 1);
  if (fd == NULL)
    return -1;
  cnt = last - first + 1;
  charcnt = full_write(g, fd, first, cnt);
  if (charcnt == cnt) {
    // good write
    //modified_count = FALSE;
  } else {
    charcnt = 0;
  }
  g->e->fclose(g->e->data, fd);
  return charcnt;
}

#if ENABLE_FEATURE_VI_SEARCH
# if ENABLE_FEATURE_VI_REGEX_SEARCH
// search for pattern starting at p
static char *char_search(struct globals *g, char *p, const char *pat, int dir_and_range)
{
  struct re_pattern_buffer preg;
  const char *err;
  char *q;
  int i;
  int size;
  int range;

  re_syntax_options = RE_SYNTAX_POSIX_EXTENDED;
  if (ignorecase)
    re_syntax_options = RE_SYNTAX_POSIX_EXTENDED | RE_ICASE;

  sys_memset(&preg, 0, sizeof(preg));
  err = re_compile_pattern(pat, sys_strlen(pat), &preg);
  if (err != NULL) {
    status_line_bold(g, "bad search pattern '%s': %s", pat, err);
    return p;
  }

  range = (dir_and_range & 1);
  q = g->end - 1; // if FULL
  if (range == LIMITED)
    q = next_line(g, p);
  if (dir_and_range < 0) { // BACK?
    q = g->text;
    if (range == LIMITED)
      q = prev_line(g, p);
  }

  // RANGE could be negative if we are searching backwards
  range = q - p;
  q = p;
  size = range;
  if (range < 0) {
    size = -size;
    q = p - size;
    if (q < g->text)
      q = g->text;
  }
  // search for the compiled pattern, preg, in p[]
  // range < 0: search backward
  // range > 0: search forward
  // 0 < start < size
  // re_search() < 0: not found or error
  // re_search() >= 0: index of found pattern
  //           struct pattern   char     int   int    int    struct reg
  // re_search(*pattern_buffer, *string, size, start, range, *regs)
  i = re_search(&preg, q, size, /*start:*/ 0, range, /*struct re_registers*:*/ NULL);
  regfree(&preg);
  if (i < 0)
    return NULL;
  if (dir_and_range > 0) // FORWARD?
    p = p + i;
  else
    p = p - i;
  return p;
}
# else
#  if ENABLE_FEATURE_VI_SETOPTS
static int mycmp(struct globals *g, const char *s1, const char *s2, int len)
{
  if (ignorecase) {
    return sys_strncasecmp(s1, s2, len);
  }
  return sys_strncmp(s1, s2, len);
}
#  else
#   define mycmp strncmp
#  endif
static char *char_search(struct globals *g, char *p, const char *pat, int dir_and_range)
{
  char *start, *stop;
  int len;
  int range;

  len = sys_strlen(pat);
  range = (dir_and_range & 1);
  if (dir_and_range > 0) { //FORWARD?
    stop = g->end - 1;  // assume range is p..end-1
    if (range == LIMITED)
      stop = next_line(g, p);  // range is to next line
    for (start = p; start < stop; start++) {
      if (mycmp(g, start, pat, len) == 0) {
        return start;
      }
    }
  } else { //BACK
    stop = g->text;  // assume range is text..p
    if (range == LIMITED)
      stop = prev_line(g, p);  // range is to prev line
    for (start = p - len; start >= stop; start--) {
      if (mycmp(g, start, pat, len) == 0) {
        return start;
      }
    }
  }
  // pattern not found
  return NULL;
}
# endif
#endif /* FEATURE_VI_SEARCH */

//----- The Colon commands -------------------------------------
#if ENABLE_FEATURE_VI_COLON
static char *get_one_address(struct globals *g, char *p, int *addr)  // get colon addr, if present
{
  int st;
  char *q;
  IF_FEATURE_VI_YANKMARK(char c;)
  IF_FEATURE_VI_SEARCH(char *pat;)

  *addr = -1;      // assume no addr
  if (*p == '.') {  // the current line
    p++;
    q = begin_line(g, g->dot);
    *addr = count_lines(g, g->text, q);
  }
#if ENABLE_FEATURE_VI_YANKMARK
  else if (*p == '\'') {  // is this a mark addr
    p++;
    c = sys_tolower(*p);
    p++;
    if (c >= 'a' && c <= 'z') {
      // we have a mark
      c = c - 'a';
      q = g->mark[(unsigned char) c];
      if (q != NULL) {  // is mark valid
        *addr = count_lines(g, g->text, q);
      }
    }
  }
#endif
#if ENABLE_FEATURE_VI_SEARCH
  else if (*p == '/') {  // a search pattern
    q = strchrnul(++p, '/');
    pat = sys_strndup(p, q - p); // save copy of pattern
    p = q;
    if (*p == '/')
      p++;
    q = char_search(g, g->dot, pat, (FORWARD << 1) | FULL);
    if (q != NULL) {
      *addr = count_lines(g, g->text, q);
    }
    sys_free(pat);
  }
#endif
  else if (*p == '$') {  // the last line in file
    p++;
    q = begin_line(g, g->end - 1);
    *addr = count_lines(g, g->text, q);
  } else if (sys_isdigit(*p)) {  // specific line number
    sys_sscanf(p, "%d%n", addr, &st);
    p += st;
  } else {
    // unrecognized address - assume -1
    *addr = -1;
  }
  return p;
}

static char *get_address(struct globals *g, char *p, int *b, int *e)  // get two colon addrs, if present
{
  //----- get the address' i.e., 1,3   'a,'b  -----
  // get FIRST addr, if present
  while (sys_isblank(*p))
    p++;        // skip over leading spaces
  if (*p == '%') {      // alias for 1,$
    p++;
    *b = 1;
    *e = count_lines(g, g->text, g->end-1);
    goto ga0;
  }
  p = get_one_address(g, p, b);
  while (sys_isblank(*p))
    p++;
  if (*p == ',') {      // is there a address separator
    p++;
    while (sys_isblank(*p))
      p++;
    // get SECOND addr, if present
    p = get_one_address(g, p, e);
  }
 ga0:
  while (sys_isblank(*p))
    p++;        // skip over trailing spaces
  return p;
}

#if ENABLE_FEATURE_VI_SET && ENABLE_FEATURE_VI_SETOPTS
static void setops(struct globals *g, const char *args, const char *opname, int flg_no,
      const char *short_opname, int opt)
{
  const char *a = args + flg_no;
  int l = sys_strlen(opname) - 1; // opname have + ' '

  // maybe strncmp? we had tons of erroneous strncasecmp's...
  if (sys_strncasecmp(a, opname, l) == 0
   || sys_strncasecmp(a, short_opname, 2) == 0
  ) {
    if (flg_no)
      g->vi_setops &= ~opt;
    else
      g->vi_setops |= opt;
  }
}
#endif

#endif /* FEATURE_VI_COLON */

// buf must be no longer than MAX_INPUT_LEN!
static void colon(struct globals *g, char *buf)
{
#if !ENABLE_FEATURE_VI_COLON
  // Simple ":cmd" handler with minimal set of commands
  char *p = buf;
  int cnt;

  if (*p == ':')
    p++;
  cnt = sys_strlen(p);
  if (cnt == 0)
    return;
  if (sys_strncmp(p, "quit", cnt) == 0
   || sys_strncmp(p, "q!", cnt) == 0
  ) {
    if (g->modified_count && p[1] != '!') {
      status_line_bold(g, "No write since last change (:%s! overrides)", p);
    } else {
      g->editing = 0;
    }
    return;
  }
  if (sys_strncmp(p, "write", cnt) == 0
   || sys_strncmp(p, "wq", cnt) == 0
   || sys_strncmp(p, "wn", cnt) == 0
   || (p[0] == 'x' && !p[1])
  ) {
    if (g->modified_count != 0 || p[0] != 'x') {
      cnt = file_write(g, g->current_filename, g->text, g->end - 1);
    }
    if (cnt < 0) {
      if (cnt == -1)
        status_line_bold(g, "Write error: "STRERROR_FMT STRERROR_ERRNO);
    } else {
      g->modified_count = 0;
      g->last_modified_count = -1;
      status_line(g, "'%s' %uL, %uC",
        g->current_filename,
        count_lines(g, g->text, g->end - 1), cnt
      );
      if (p[0] == 'x'
       || p[1] == 'q' || p[1] == 'n'
       || p[1] == 'Q' || p[1] == 'N'
      ) {
        g->editing = 0;
      }
    }
    return;
  }
  if (sys_strncmp(p, "file", cnt) == 0) {
    g->last_status_cksum = 0;  // force status update
    return;
  }
  if (sys_sscanf(p, "%d", &cnt) > 0) {
    g->dot = find_line(g, cnt);
    g->dot_skip_over_ws(g);
    return;
  }
  not_implemented(g, p);
#else

  char c, *buf1, *q, *r;
  char *fn, cmd[MAX_INPUT_LEN], args[MAX_INPUT_LEN];
  int i, l, li, b, e;
  int useforce;
# if ENABLE_FEATURE_VI_SEARCH || ENABLE_FEATURE_ALLOW_EXEC
  char *orig_buf;
# endif

  // :3154  // if (-e line 3154) goto it  else stay put
  // :4,33w! foo  // write a portion of buffer to file "foo"
  // :w    // write all of buffer to current file
  // :q    // quit
  // :q!    // quit- dont care about modified file
  // :'a,'z!sort -u   // filter block through sort
  // :'f    // goto mark "f"
  // :'fl    // list literal the mark "f" line
  // :.r bar  // read file "bar" into buffer before dot
  // :/123/,/abc/d    // delete lines from "123" line to "abc" line
  // :/xyz/  // goto the "xyz" line
  // :s/find/replace/ // substitute pattern "find" with "replace"
  // :!<cmd>  // run <cmd> then return
  //

  if (!buf[0])
    goto ret;
  if (*buf == ':')
    buf++;      // move past the ':'

  li = i = 0;
  b = e = -1;
  q = g->text;      // assume 1,$ for the range
  r = g->end - 1;
  li = count_lines(g, g->text, g->end - 1);
  fn = g->current_filename;

  // look for optional address(es)  :.  :1  :1,9   :'q,'a   :%
  buf = get_address(g, buf, &b, &e);

# if ENABLE_FEATURE_VI_SEARCH || ENABLE_FEATURE_ALLOW_EXEC
  // remember orig command line
  orig_buf = buf;
# endif

  // get the COMMAND into cmd[]
  buf1 = cmd;
  while (*buf != '\0') {
    if (sys_isspace(*buf))
      break;
    *buf1++ = *buf++;
  }
  *buf1 = '\0';
  // get any ARGuments
  while (sys_isblank(*buf))
    buf++;
  sys_strcpy(args, buf);
  useforce = FALSE;
  buf1 = last_char_is(cmd, '!');
  if (buf1) {
    useforce = TRUE;
    *buf1 = '\0';   // get rid of !
  }
  if (b >= 0) {
    // if there is only one addr, then the addr
    // is the line number of the single line the
    // user wants. So, reset the end
    // pointer to point at end of the "b" line
    q = find_line(g, b);  // what line is #b
    r = end_line(g, q);
    li = 1;
  }
  if (e >= 0) {
    // we were given two addrs.  change the
    // end pointer to the addr given by user.
    r = find_line(g, e);  // what line is #e
    r = end_line(g, r);
    li = e - b + 1;
  }
  // ------------ now look for the command ------------
  i = sys_strlen(cmd);
  if (i == 0) {    // :123CR goto line #123
    if (b >= 0) {
      g->dot = find_line(g, b);  // what line is #b
      dot_skip_over_ws(g);
    }
  }
# if ENABLE_FEATURE_ALLOW_EXEC
  else if (cmd[0] == '!') {  // run a cmd
    int retcode;
    // :!ls   run the <cmd>
    go_bottom_and_clear_to_eol(g);
    cookmode();
    retcode = system(orig_buf + 1);  // run the cmd
    if (retcode)
      printf("\nshell returned %i\n\n", retcode);
    rawmode();
    Hit_Return(g);      // let user see results
  }
# endif
  else if (cmd[0] == '=' && !cmd[1]) {  // where is the address
    if (b < 0) {  // no addr given- use defaults
      b = e = count_lines(g, g->text, g->dot);
    }
    status_line(g, "%d", b);
  } else if (sys_strncmp(cmd, "delete", i) == 0) {  // delete lines
    if (b < 0) {  // no addr given- use defaults
      q = begin_line(g, g->dot);  // assume .,. for the range
      r = end_line(g, g->dot);
    }
    g->dot = yank_delete(g, q, r, 1, YANKDEL, ALLOW_UNDO);  // save, then delete lines
    dot_skip_over_ws(g);
  } else if (sys_strncmp(cmd, "edit", i) == 0) {  // Edit a file
    int size;

    // don't edit, if the current file has been modified
    if (g->modified_count && !useforce) {
      status_line_bold(g, "No write since last change (:%s! overrides)", cmd);
      goto ret;
    }
    if (args[0]) {
      // the user supplied a file name
      fn = args;
    } else if (g->current_filename && g->current_filename[0]) {
      // no user supplied name- use the current filename
      // fn = current_filename;  was set by default
    } else {
      // no user file name, no current name- punt
      status_line_bold(g, "No current filename");
      goto ret;
    }

    size = init_text_buffer(g, fn);

# if ENABLE_FEATURE_VI_YANKMARK
    if (Ureg >= 0 && Ureg < 28) {
      sys_free(g->reg[Ureg]);  //   free orig line reg- for 'U'
      g->reg[Ureg] = NULL;
    }
    /*if (YDreg < 28) - always true*/ {
      sys_free(g->reg[g->YDreg]);  //   free default yank/delete register
      g->reg[g->YDreg] = NULL;
    }
# endif
    // how many lines in text[]?
    li = count_lines(g, g->text, g->end - 1);
    status_line(g, "'%s'%s"
      IF_FEATURE_VI_READONLY("%s")
      " %uL, %uC",
      g->current_filename,
      (size < 0 ? " [New file]" : ""),
      IF_FEATURE_VI_READONLY(
        ((g->readonly_mode) ? " [Readonly]" : ""),
      )
      li, (int)(g->end - g->text)
    );
  } else if (sys_strncmp(cmd, "file", i) == 0) {  // what File is this
    if (b != -1 || e != -1) {
      status_line_bold(g, "No address allowed on this command");
      goto ret;
    }
    if (args[0]) {
      // user wants a new filename
      sys_free(g->current_filename);
      g->current_filename = sys_strdup(args);
    } else {
      // user wants file status info
      g->last_status_cksum = 0;  // force status update
    }
  } else if (sys_strncmp(cmd, "features", i) == 0) {  // what features are available
    // print out values of all features
    go_bottom_and_clear_to_eol(g);
    cookmode();
    show_help(g);
    rawmode();
    Hit_Return(g);
  } else if (sys_strncmp(cmd, "list", i) == 0) {  // literal print line
    if (b < 0) {  // no addr given- use defaults
      q = begin_line(g, g->dot);  // assume .,. for the range
      r = end_line(g, g->dot);
    }
    go_bottom_and_clear_to_eol(g);
    //puts("\r");
    write1(g, "\r\n");
    for (; q <= r; q++) {
      int c_is_no_print;

      c = *q;
      c_is_no_print = (c & 0x80) && !Isprint(c);
      if (c_is_no_print) {
        c = '.';
        standout_start();
      }
      if (c == '\n') {
        write1(g, "$\r");
      } else if (c < ' ' || c == 127) {
        bb_putchar(g, '^');
        if (c == 127)
          c = '?';
        else
          c += '@';
      }
      bb_putchar(g, c);
      if (c_is_no_print)
        standout_end();
    }
    Hit_Return(g);
  } else if (sys_strncmp(cmd, "quit", i) == 0 // quit
          || sys_strncmp(cmd, "next", i) == 0 // edit next file
          || sys_strncmp(cmd, "prev", i) == 0 // edit previous file
  ) {
    int n;
    if (useforce) {
      if (*cmd == 'q') {
        // force end of argv list
        //optind = g->cmdline_filecnt;
      }
      g->editing = 0;
      goto ret;
    }
    // don't exit if the file been modified
    if (g->modified_count) {
      status_line_bold(g, "No write since last change (:%s! overrides)", cmd);
      goto ret;
    }
    // are there other file to edit
    //n = g->cmdline_filecnt - optind - 1;
    n = g->cmdline_filecnt - 1;
    if (*cmd == 'q' && n > 0) {
      status_line_bold(g, "%u more file(s) to edit", n);
      goto ret;
    }
    if (*cmd == 'n' && n <= 0) {
      status_line_bold(g, "No more files to edit");
      goto ret;
    }
    if (*cmd == 'p') {
      // are there previous files to edit
      //if (optind < 1) {
        status_line_bold(g, "No previous files to edit");
        goto ret;
      //}
      //optind -= 2;
    }
    g->editing = 0;
  } else if (sys_strncmp(cmd, "read", i) == 0) {  // read file into text[]
    int size;

    fn = args;
    if (!fn[0]) {
      status_line_bold(g, "No filename given");
      goto ret;
    }
    if (b < 0) {  // no addr given- use defaults
      q = begin_line(g, g->dot);  // assume "dot"
    }
    // read after current line- unless user said ":0r foo"
    if (b != 0) {
      q = next_line(g, q);
      // read after last line
      if (q == g->end-1)
        ++q;
    }
    { // dance around potentially-reallocated text[]
      uintptr_t ofs = q - g->text;
      size = file_insert(g, fn, q, 0);
      q = g->text + ofs;
    }
    if (size < 0)
      goto ret;  // nothing was inserted
    // how many lines in text[]?
    li = count_lines(g, q, q + size - 1);
    status_line(g, "'%s'"
      IF_FEATURE_VI_READONLY("%s")
      " %uL, %uC",
      fn,
      IF_FEATURE_VI_READONLY((g->readonly_mode ? " [Readonly]" : ""),)
      li, size
    );
    if (size > 0) {
      // if the insert is before "dot" then we need to update
      if (q <= g->dot)
        g->dot += size;
    }
  } else if (sys_strncmp(cmd, "rewind", i) == 0) {  // rewind cmd line args
    if (g->modified_count && !useforce) {
      status_line_bold(g, "No write since last change (:%s! overrides)", cmd);
    } else {
      // reset the filenames to edit
      //optind = -1; // start from 0th file
      g->editing = 0;
    }
# if ENABLE_FEATURE_VI_SET
  } else if (sys_strncmp(cmd, "set", i) == 0) {  // set or clear features
#  if ENABLE_FEATURE_VI_SETOPTS
    char *argp;
#  endif
    i = 0;      // offset into args
    // only blank is regarded as args delimiter. What about tab '\t'?
    if (!args[0] || sys_strcasecmp(args, "all") == 0) {
      // print out values of all options
#  if ENABLE_FEATURE_VI_SETOPTS
      status_line_bold(g,
        "%sautoindent "
        "%sflash "
        "%signorecase "
        "%sshowmatch "
        "tabstop=%u",
        autoindent ? "" : "no",
        err_method ? "" : "no",
        ignorecase ? "" : "no",
        showmatch ? "" : "no",
        g->tabstop
      );
#  endif
      goto ret;
    }
#  if ENABLE_FEATURE_VI_SETOPTS
    argp = args;
    while (*argp) {
      if (sys_strncmp(argp, "no", 2) == 0)
        i = 2;    // ":set noautoindent"
      setops(g, argp, "autoindent ", i, "ai", VI_AUTOINDENT);
      setops(g, argp, "flash "     , i, "fl", VI_ERR_METHOD);
      setops(g, argp, "ignorecase ", i, "ic", VI_IGNORECASE);
      setops(g, argp, "showmatch " , i, "sm", VI_SHOWMATCH );
      if (sys_strncmp(argp + i, "tabstop=", 8) == 0) {
        int t = 0;
        sys_sscanf(argp + i+8, "%u", &t);
        if (t > 0 && t <= MAX_TABSTOP)
          g->tabstop = t;
      }
      argp = skip_non_whitespace(argp);
      argp = skip_whitespace(argp);
    }
#  endif /* FEATURE_VI_SETOPTS */
# endif /* FEATURE_VI_SET */

# if ENABLE_FEATURE_VI_SEARCH
  } else if (cmd[0] == 's') {  // substitute a pattern with a replacement pattern
    char *F, *R, *flags;
    sys_size_t len_F, len_R;
    int gflag;    // global replace flag
#  if ENABLE_FEATURE_VI_UNDO
    int dont_chain_first_item = ALLOW_UNDO;
#  endif

    // F points to the "find" pattern
    // R points to the "replace" pattern
    // replace the cmd line delimiters "/" with NULs
    c = orig_buf[1];  // what is the delimiter
    F = orig_buf + 2;  // start of "find"
    R = sys_strchr(F, c);  // middle delimiter
    if (!R)
      goto colon_s_fail;
    len_F = R - F;
    *R++ = '\0';  // terminate "find"
    flags = sys_strchr(R, c);
    if (!flags)
      goto colon_s_fail;
    len_R = flags - R;
    *flags++ = '\0';  // terminate "replace"
    gflag = *flags;

    q = begin_line(g, q);
    if (b < 0) {  // maybe :s/foo/bar/
      q = begin_line(g, g->dot);      // start with cur line
      b = count_lines(g, g->text, q); // cur line number
    }
    if (e < 0)
      e = b;    // maybe :.s/foo/bar/

    for (i = b; i <= e; i++) {  // so, :20,23 s \0 find \0 replace \0
      char *ls = q;    // orig line start
      char *found;
 vc4:
      found = char_search(g, q, F, (FORWARD << 1) | LIMITED);  // search cur line only for "find"
      if (found) {
        uintptr_t bias;
        // we found the "find" pattern - delete it
        // For undo support, the first item should not be chained
        text_hole_delete(g, found, found + len_F - 1, dont_chain_first_item);
#  if ENABLE_FEATURE_VI_UNDO
        dont_chain_first_item = ALLOW_UNDO_CHAIN;
#  endif
        // insert the "replace" patern
        bias = string_insert(g, found, R, ALLOW_UNDO_CHAIN);
        found += bias;
        ls += bias;
        //q += bias; - recalculated anyway
        // check for "global"  :s/foo/bar/g
        if (gflag == 'g') {
          if ((found + len_R) < end_line(g, ls)) {
            q = found + len_R;
            goto vc4;  // don't let q move past cur line
          }
        }
      }
      q = next_line(g, ls);
    }
# endif /* FEATURE_VI_SEARCH */
  } else if (sys_strncmp(cmd, "version", i) == 0) {  // show software version
    status_line(g, BB_VER);
  } else if (sys_strncmp(cmd, "write", i) == 0  // write text to file
          || sys_strncmp(cmd, "wq", i) == 0
          || sys_strncmp(cmd, "wn", i) == 0
          || (cmd[0] == 'x' && !cmd[1])
  ) {
    int size;
    //int forced = FALSE;

    // is there a file name to write to?
    if (args[0]) {
      fn = args;
    }
# if ENABLE_FEATURE_VI_READONLY
    if (g->readonly_mode && !useforce) {
      status_line_bold(g, "'%s' is read only", fn);
      goto ret;
    }
# endif
    //if (useforce) {
      // if "fn" is not write-able, chmod u+w
      // sprintf(syscmd, "chmod u+w %s", fn);
      // system(syscmd);
      // forced = TRUE;
    //}
    if (g->modified_count != 0 || cmd[0] != 'x') {
      size = r - q + 1;
      l = file_write(g, fn, q, r);
    } else {
      size = 0;
      l = 0;
    }
    //if (useforce && forced) {
      // chmod u-w
      // sprintf(syscmd, "chmod u-w %s", fn);
      // system(syscmd);
      // forced = FALSE;
    //}
    if (l < 0) {
      if (l == -1)
        status_line_bold_errno(g, fn);
    } else {
      // how many lines written
      li = count_lines(g, q, q + l - 1);
      status_line(g, "'%s' %uL, %uC", fn, li, l);
      if (l == size) {
        if (q == g->text && q + l == g->end) {
          g->modified_count = 0;
          g->last_modified_count = -1;
        }
        if (cmd[0] == 'x'
         || cmd[1] == 'q' || cmd[1] == 'n'
         || cmd[1] == 'Q' || cmd[1] == 'N'
        ) {
          g->editing = 0;
        }
      }
    }
# if ENABLE_FEATURE_VI_YANKMARK
  } else if (sys_strncmp(cmd, "yank", i) == 0) {  // yank lines
    if (b < 0) {  // no addr given- use defaults
      q = begin_line(g, g->dot);  // assume .,. for the range
      r = end_line(g, g->dot);
    }
    text_yank(g, q, r, g->YDreg);
    li = count_lines(g, q, r);
    status_line(g, "Yank %d lines (%d chars) into [%c]",
        li, sys_strlen(g->reg[g->YDreg]), what_reg(g));
# endif
  } else {
    // cmd unknown
    not_implemented(g, cmd);
  }
 ret:
  g->dot = bound_dot(g, g->dot);  // make sure "dot" is valid
  return;
# if ENABLE_FEATURE_VI_SEARCH
 colon_s_fail:
  status_line(g, ":s expression missing delimiters");
# endif
#endif /* FEATURE_VI_COLON */
}

//----- Char Routines --------------------------------------------
// Chars that are part of a word-
//    0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz
// Chars that are Not part of a word (stoppers)
//    !"#$%&'()*+,-./:;<=>?@[\]^`{|}~
// Chars that are WhiteSpace
//    TAB NEWLINE VT FF RETURN SPACE
// DO NOT COUNT NEWLINE AS WHITESPACE

static int st_test(char *p, int type, int dir, char *tested)
{
  char c, c0, ci;
  int test, inc;

  inc = dir;
  c = c0 = p[0];
  ci = p[inc];
  test = 0;

  if (type == S_BEFORE_WS) {
    c = ci;
    test = (!sys_isspace(c) || c == '\n');
  }
  if (type == S_TO_WS) {
    c = c0;
    test = (!sys_isspace(c) || c == '\n');
  }
  if (type == S_OVER_WS) {
    c = c0;
    test = sys_isspace(c);
  }
  if (type == S_END_PUNCT) {
    c = ci;
    test = sys_ispunct(c);
  }
  if (type == S_END_ALNUM) {
    c = ci;
    test = (sys_isalnum(c) || c == '_');
  }
  *tested = c;
  return test;
}

static char *skip_thing(struct globals *g, char *p, int linecnt, int dir, int type)
{
  char c;

  while (st_test(p, type, dir, &c)) {
    // make sure we limit search to correct number of lines
    if (c == '\n' && --linecnt < 1)
      break;
    if (dir >= 0 && p >= g->end - 1)
      break;
    if (dir < 0 && p <= g->text)
      break;
    p += dir;    // move to next char
  }
  return p;
}

#if ENABLE_FEATURE_VI_USE_SIGNALS
static void winch_handler(struct globals *g, int sig UNUSED_PARAM)
{
  int save_errno = errno;
  // FIXME: do it in main loop!!!
  signal(SIGWINCH, winch_handler);
  query_screen_dimensions(g);
  new_screen(g, g->rows, g->columns);  // get memory for virtual screen
  redraw(g, TRUE);    // re-draw the screen
  errno = save_errno;
}
static void tstp_handler(int sig UNUSED_PARAM)
{
  int save_errno = errno;

  // ioctl inside cookmode() was seen to generate SIGTTOU,
  // stopping us too early. Prevent that:
  signal(SIGTTOU, SIG_IGN);

  go_bottom_and_clear_to_eol(g);
  cookmode(); // terminal to "cooked"

  // stop now
  //signal(SIGTSTP, SIG_DFL);
  //raise(SIGTSTP);
  raise(SIGSTOP); // avoid "dance" with TSTP handler - use SIGSTOP instead
  //signal(SIGTSTP, tstp_handler);

  // we have been "continued" with SIGCONT, restore screen and termios
  rawmode(); // terminal to "raw"
  g->last_status_cksum = 0; // force status update
  redraw(g, TRUE); // re-draw the screen

  errno = save_errno;
}
static void int_handler(int sig)
{
  signal(SIGINT, int_handler);
  siglongjmp(restart, sig);
}
#endif /* FEATURE_VI_USE_SIGNALS */

static void do_cmd(struct globals *g, int c);

static int find_range(struct globals *g, char **start, char **stop, char c)
{
  char *save_dot, *p, *q, *t;
  int cnt, multiline = 0, forward;

  save_dot = g->dot;
  p = q = g->dot;

  // will a 'G' command move forwards or backwards?
  forward = g->cmdcnt == 0 || g->cmdcnt > count_lines(g, g->text, g->dot);

  if (sys_strchr("cdy><", c)) {
    // these cmds operate on whole lines
    p = q = begin_line(g, p);
    for (cnt = 1; cnt < g->cmdcnt; cnt++) {
      q = next_line(g, q);
    }
    q = end_line(g, q);
  } else if (sys_strchr("^%$0bBeEfth\b\177", c)) {
    // These cmds operate on char positions
    do_cmd(g, c);    // execute movement cmd
    q = g->dot;
  } else if (sys_strchr("wW", c)) {
    do_cmd(g, c);    // execute movement cmd
    // if we are at the next word's first char
    // step back one char
    // but check the possibilities when it is true
    if (g->dot > g->text && ((sys_isspace(g->dot[-1]) && !sys_isspace(g->dot[0]))
        || (sys_ispunct(g->dot[-1]) && !sys_ispunct(g->dot[0]))
        || (sys_isalnum(g->dot[-1]) && !sys_isalnum(g->dot[0]))))
      g->dot--;    // move back off of next word
    if (g->dot > g->text && *g->dot == '\n')
      g->dot--;    // stay off NL
    q = g->dot;
  } else if (sys_strchr("H-k{", c) || (c == 'G' && !forward)) {
    // these operate on multi-lines backwards
    q = end_line(g, g->dot);  // find NL
    do_cmd(g, c);    // execute movement cmd
    dot_begin(g);
    p = g->dot;
  } else if (sys_strchr("L+j}\r\n", c) || (c == 'G' && forward)) {
    // these operate on multi-lines forwards
    p = begin_line(g, g->dot);
    do_cmd(g, c);    // execute movement cmd
    dot_end(g);    // find NL
    q = g->dot;
  } else {
    // nothing -- this causes any other values of c to
    // represent the one-character range under the
    // cursor.  this is correct for ' ' and 'l', but
    // perhaps no others.
    //
  }
  if (q < p) {
    t = q;
    q = p;
    p = t;
  }

  // backward char movements don't include start position
  if (q > p && sys_strchr("^0bBh\b\177", c)) q--;

  multiline = 0;
  for (t = p; t <= q; t++) {
    if (*t == '\n') {
      multiline = 1;
      break;
    }
  }

  *start = p;
  *stop = q;
  g->dot = save_dot;
  return multiline;
}

//---------------------------------------------------------------------
//----- the Ascii Chart -----------------------------------------------
//  00 nul   01 soh   02 stx   03 etx   04 eot   05 enq   06 ack   07 bel
//  08 bs    09 ht    0a nl    0b vt    0c np    0d cr    0e so    0f si
//  10 dle   11 dc1   12 dc2   13 dc3   14 dc4   15 nak   16 syn   17 etb
//  18 can   19 em    1a sub   1b esc   1c fs    1d gs    1e rs    1f us
//  20 sp    21 !     22 "     23 #     24 $     25 %     26 &     27 '
//  28 (     29 )     2a *     2b +     2c ,     2d -     2e .     2f /
//  30 0     31 1     32 2     33 3     34 4     35 5     36 6     37 7
//  38 8     39 9     3a :     3b ;     3c <     3d =     3e >     3f ?
//  40 @     41 A     42 B     43 C     44 D     45 E     46 F     47 G
//  48 H     49 I     4a J     4b K     4c L     4d M     4e N     4f O
//  50 P     51 Q     52 R     53 S     54 T     55 U     56 V     57 W
//  58 X     59 Y     5a Z     5b [     5c \     5d ]     5e ^     5f _
//  60 `     61 a     62 b     63 c     64 d     65 e     66 f     67 g
//  68 h     69 i     6a j     6b k     6c l     6d m     6e n     6f o
//  70 p     71 q     72 r     73 s     74 t     75 u     76 v     77 w
//  78 x     79 y     7a z     7b {     7c |     7d }     7e ~     7f del
//---------------------------------------------------------------------

//----- Execute a Vi Command -----------------------------------
static void do_cmd(struct globals *g, int c)
{
  char *p, *q, *save_dot;
  char buf[12];
  int dir;
  int cnt, i, j;
  int c1;

//  c1 = c; // quiet the compiler
//  cnt = yf = 0; // quiet the compiler
//  p = q = save_dot = buf; // quiet the compiler
  sys_memset(buf, '\0', sizeof(buf));

  show_status_line(g);

  // if this is a cursor key, skip these checks
  switch (c) {
    case KEYCODE_UP:
    case KEYCODE_DOWN:
    case KEYCODE_LEFT:
    case KEYCODE_RIGHT:
    case KEYCODE_HOME:
    case KEYCODE_END:
    case KEYCODE_PAGEUP:
    case KEYCODE_PAGEDOWN:
    case KEYCODE_DELETE:
      goto key_cmd_mode;
  }

  if (g->cmd_mode == 2) {
    //  flip-flop Insert/Replace mode
    if (c == KEYCODE_INSERT)
      goto dc_i;
    // we are 'R'eplacing the current *dot with new char
    if (*g->dot == '\n') {
      // don't Replace past E-o-l
      g->cmd_mode = 1;  // convert to insert
      undo_queue_commit(g);
    } else {
      if (1 <= c || Isprint(c)) {
        if (c != 27)
          g->dot = yank_delete(g, g->dot, g->dot, 0, YANKDEL, ALLOW_UNDO);  // delete char
        g->dot = char_insert(g, g->dot, c, ALLOW_UNDO_CHAIN);  // insert new char
      }
      goto dc1;
    }
  }
  if (g->cmd_mode == 1) {
    // hitting "Insert" twice means "R" replace mode
    if (c == KEYCODE_INSERT) goto dc5;
    // insert the char c at "dot"
    if (1 <= c || Isprint(c)) {
      g->dot = char_insert(g, g->dot, c, ALLOW_UNDO_QUEUED);
    }
    goto dc1;
  }

 key_cmd_mode:
  switch (c) {
    //case 0x01:  // soh
    //case 0x09:  // ht
    //case 0x0b:  // vt
    //case 0x0e:  // so
    //case 0x0f:  // si
    //case 0x10:  // dle
    //case 0x11:  // dc1
    //case 0x13:  // dc3
#if ENABLE_FEATURE_VI_CRASHME
  case 0x14:      // dc4  ctrl-T
    crashme = (crashme == 0) ? 1 : 0;
    break;
#endif
    //case 0x16:  // syn
    //case 0x17:  // etb
    //case 0x18:  // can
    //case 0x1c:  // fs
    //case 0x1d:  // gs
    //case 0x1e:  // rs
    //case 0x1f:  // us
    //case '!':  // !-
    //case '#':  // #-
    //case '&':  // &-
    //case '(':  // (-
    //case ')':  // )-
    //case '*':  // *-
    //case '=':  // =-
    //case '@':  // @-
    //case 'F':  // F-
    //case 'K':  // K-
    //case 'Q':  // Q-
    //case 'S':  // S-
    //case 'T':  // T-
    //case 'V':  // V-
    //case '[':  // [-
    //case '\\':  // \-
    //case ']':  // ]-
    //case '_':  // _-
    //case '`':  // `-
    //case 'v':  // v-
  default:      // unrecognized command
    buf[0] = c;
    buf[1] = '\0';
    not_implemented(g, buf);
    end_cmd_q(g);  // stop adding to q
  case 0x00:      // nul- ignore
    break;
  case 2:      // ctrl-B  scroll up   full screen
  case KEYCODE_PAGEUP:  // Cursor Key Page Up
    dot_scroll(g, g->rows - 2, -1);
    break;
  case 4:      // ctrl-D  scroll down half screen
    dot_scroll(g, (g->rows - 2) / 2, 1);
    break;
  case 5:      // ctrl-E  scroll down one line
    dot_scroll(g, 1, 1);
    break;
  case 6:      // ctrl-F  scroll down full screen
  case KEYCODE_PAGEDOWN:  // Cursor Key Page Down
    dot_scroll(g, g->rows - 2, 1);
    break;
  case 7:      // ctrl-G  show current status
    g->last_status_cksum = 0;  // force status update
    break;
  case 'h':      // h- move left
  case KEYCODE_LEFT:  // cursor key Left
  case 8:    // ctrl-H- move left    (This may be ERASE char)
  case 0x7f:  // DEL- move left   (This may be ERASE char)
    do {
      dot_left(g);
    } while (--g->cmdcnt > 0);
    break;
  case 10:      // Newline ^J
  case 'j':      // j- goto next line, same col
  case KEYCODE_DOWN:  // cursor key Down
    do {
      dot_next(g);    // go to next B-o-l
      // try stay in same col
      g->dot = move_to_col(g, g->dot, g->ccol + g->offset);
    } while (--g->cmdcnt > 0);
    break;
  case 12:      // ctrl-L  force redraw whole screen
  case 18:      // ctrl-R  force redraw
    redraw(g, TRUE);  // this will redraw the entire display
    break;
  case 13:      // Carriage Return ^M
  case '+':      // +- goto next line
    do {
      dot_next(g);
      dot_skip_over_ws(g);
    } while (--g->cmdcnt > 0);
    break;
  case 21:      // ctrl-U  scroll up half screen
    dot_scroll(g, (g->rows - 2) / 2, -1);
    break;
  case 25:      // ctrl-Y  scroll up one line
    dot_scroll(g, 1, -1);
    break;
  case 27:      // esc
    if (g->cmd_mode == 0)
      indicate_error(g);
    g->cmd_mode = 0;  // stop inserting
    undo_queue_commit(g);
    end_cmd_q(g);
    g->last_status_cksum = 0;  // force status update
    break;
  case ' ':      // move right
  case 'l':      // move right
  case KEYCODE_RIGHT:  // Cursor Key Right
    do {
      dot_right(g);
    } while (--g->cmdcnt > 0);
    break;
#if ENABLE_FEATURE_VI_YANKMARK
  case '"':      // "- name a register to use for Delete/Yank
    c1 = (get_one_char(g) | 0x20) - 'a'; // | 0x20 is tolower()
    if ((unsigned)c1 <= 25) { // a-z?
      g->YDreg = c1;
    } else {
      indicate_error(g);
    }
    break;
  case '\'':      // '- goto a specific mark
    c1 = (get_one_char(g) | 0x20);
    if ((unsigned)(c1 - 'a') <= 25) { // a-z?
      c1 = (c1 - 'a');
      // get the b-o-l
      q = g->mark[c1];
      if (g->text <= q && q < g->end) {
        g->dot = q;
        dot_begin(g);  // go to B-o-l
        dot_skip_over_ws(g);
      }
    } else if (c1 == '\'') {  // goto previous context
      g->dot = swap_context(g, g->dot);  // swap current and previous context
      dot_begin(g);  // go to B-o-l
      dot_skip_over_ws(g);
    } else {
      indicate_error(g);
    }
    break;
  case 'm':      // m- Mark a line
    // this is really stupid.  If there are any inserts or deletes
    // between text[0] and dot then this mark will not point to the
    // correct location! It could be off by many lines!
    // Well..., at least its quick and dirty.
    c1 = (get_one_char(g) | 0x20) - 'a';
    if ((unsigned)c1 <= 25) { // a-z?
      // remember the line
      g->mark[c1] = g->dot;
    } else {
      indicate_error(g);
    }
    break;
  case 'P':      // P- Put register before
  case 'p':      // p- put register after
    p = g->reg[g->YDreg];
    if (p == NULL) {
      status_line_bold(g, "Nothing in register %c", what_reg(g));
      break;
    }
    // are we putting whole lines or strings
    if (sys_strchr(p, '\n') != NULL) {
      if (c == 'P') {
        dot_begin(g);  // putting lines- Put above
      }
      if (c == 'p') {
        // are we putting after very last line?
        if (end_line(g, g->dot) == (g->end - 1)) {
          g->dot = g->end;  // force dot to end of text[]
        } else {
          dot_next(g);  // next line, then put before
        }
      }
    } else {
      if (c == 'p')
        dot_right(g);  // move to right, can move to NL
    }
    string_insert(g, g->dot, p, ALLOW_UNDO);  // insert the string
    end_cmd_q(g);  // stop adding to q
    break;
  case 'U':      // U- Undo; replace current line with original version
    if (g->reg[Ureg] != NULL) {
      p = begin_line(g, g->dot);
      q = end_line(g, g->dot);
      p = text_hole_delete(g, p, q, ALLOW_UNDO);  // delete cur line
      p += string_insert(g, p, g->reg[Ureg], ALLOW_UNDO_CHAIN);  // insert orig line
      g->dot = p;
      dot_skip_over_ws(g);
    }
    break;
#endif /* FEATURE_VI_YANKMARK */
#if ENABLE_FEATURE_VI_UNDO
  case 'u':  // u- undo last operation
    undo_pop(g);
    break;
#endif
  case '$':      // $- goto end of line
  case KEYCODE_END:    // Cursor Key End
    for (;;) {
      g->dot = end_line(g, g->dot);
      if (--g->cmdcnt <= 0)
        break;
      dot_next(g);
    }
    break;
  case '%':      // %- find matching char of pair () [] {}
    for (q = g->dot; q < g->end && *q != '\n'; q++) {
      if (sys_strchr("()[]{}", *q) != NULL) {
        // we found half of a pair
        p = find_pair(g, q, *q);
        if (p == NULL) {
          indicate_error(g);
        } else {
          g->dot = p;
        }
        break;
      }
    }
    if (*q == '\n')
      indicate_error(g);
    break;
  case 'f':      // f- forward to a user specified char
    g->last_forward_char = get_one_char(g);  // get the search char
    //
    // dont separate these two commands. 'f' depends on ';'
    //
    //**** fall through to ... ';'
  case ';':      // ;- look at rest of line for last forward char
    do {
      if (g->last_forward_char == 0)
        break;
      q = g->dot + 1;
      while (q < g->end - 1 && *q != '\n' && *q != g->last_forward_char) {
        q++;
      }
      if (*q == g->last_forward_char)
        g->dot = q;
    } while (--g->cmdcnt > 0);
    break;
  case ',':           // repeat latest 'f' in opposite direction
    if (g->last_forward_char == 0)
      break;
    do {
      q = g->dot - 1;
      while (q >= g->text && *q != '\n' && *q != g->last_forward_char) {
        q--;
      }
      if (q >= g->text && *q == g->last_forward_char)
        g->dot = q;
    } while (--g->cmdcnt > 0);
    break;

  case '-':      // -- goto prev line
    do {
      dot_prev(g);
      dot_skip_over_ws(g);
    } while (--g->cmdcnt > 0);
    break;
#if ENABLE_FEATURE_VI_DOT_CMD
  case '.':      // .- repeat the last modifying command
    // Stuff the last_modifying_cmd back into stdin
    // and let it be re-executed.
    if (g->lmc_len != 0) {
      g->ioq = g->ioq_start = sys_strndup(g->last_modifying_cmd, g->lmc_len);
    }
    break;
#endif
#if ENABLE_FEATURE_VI_SEARCH
  case '?':      // /- search for a pattern
  case '/':      // /- search for a pattern
    buf[0] = c;
    buf[1] = '\0';
    q = get_input_line(g, buf);  // get input line- use "status line"
    if (q[0] && !q[1]) {
      if (g->last_search_pattern[0])
        g->last_search_pattern[0] = c;
      goto dc3; // if no pat re-use old pat
    }
    if (q[0]) {       // strlen(q) > 1: new pat- save it and find
      // there is a new pat
      sys_free(g->last_search_pattern);
      g->last_search_pattern = sys_strdup(q);
      goto dc3;  // now find the pattern
    }
    // user changed mind and erased the "/"-  do nothing
    break;
  case 'N':      // N- backward search for last pattern
    dir = BACK;    // assume BACKWARD search
    p = g->dot - 1;
    if (g->last_search_pattern[0] == '?') {
      dir = FORWARD;
      p = g->dot + 1;
    }
    goto dc4;    // now search for pattern
    break;
  case 'n':      // n- repeat search for last pattern
    // search rest of text[] starting at next char
    // if search fails return orignal "p" not the "p+1" address
    do {
      const char *msg;
 dc3:
      dir = FORWARD;  // assume FORWARD search
      p = g->dot + 1;
      if (g->last_search_pattern[0] == '?') {
        dir = BACK;
        p = g->dot - 1;
      }
 dc4:
      q = char_search(g, p, g->last_search_pattern + 1, (dir << 1) | FULL);
      if (q != NULL) {
        g->dot = q;  // good search, update "dot"
        msg = NULL;
        goto dc2;
      }
      // no pattern found between "dot" and "end"- continue at top
      p = g->text;
      if (dir == BACK) {
        p = g->end - 1;
      }
      q = char_search(g, p, g->last_search_pattern + 1, (dir << 1) | FULL);
      if (q != NULL) {  // found something
        g->dot = q;  // found new pattern- goto it
        msg = "search hit BOTTOM, continuing at TOP";
        if (dir == BACK) {
          msg = "search hit TOP, continuing at BOTTOM";
        }
      } else {
        msg = "Pattern not found";
      }
 dc2:
      if (msg)
        status_line_bold(g, "%s", msg);
    } while (--g->cmdcnt > 0);
    break;
  case '{':      // {- move backward paragraph
    q = char_search(g, g->dot, "\n\n", (BACK << 1) | FULL);
    if (q != NULL) {  // found blank line
      g->dot = next_line(g, q);  // move to next blank line
    }
    break;
  case '}':      // }- move forward paragraph
    q = char_search(g, g->dot, "\n\n", (FORWARD << 1) | FULL);
    if (q != NULL) {  // found blank line
      g->dot = next_line(g, q);  // move to next blank line
    }
    break;
#endif /* FEATURE_VI_SEARCH */
  case '0':      // 0- goto beginning of line
  case '1':      // 1-
  case '2':      // 2-
  case '3':      // 3-
  case '4':      // 4-
  case '5':      // 5-
  case '6':      // 6-
  case '7':      // 7-
  case '8':      // 8-
  case '9':      // 9-
    if (c == '0' && g->cmdcnt < 1) {
      dot_begin(g);  // this was a standalone zero
    } else {
      g->cmdcnt = g->cmdcnt * 10 + (c - '0');  // this 0 is part of a number
    }
    break;
  case ':':      // :- the colon mode commands
    p = get_input_line(g, ":");  // get input line- use "status line"
    colon(g, p);    // execute the command
    break;
  case '<':      // <- Left  shift something
  case '>':      // >- Right shift something
    cnt = count_lines(g, g->text, g->dot);  // remember what line we are on
    c1 = get_one_char(g);  // get the type of thing to delete
    find_range(g, &p, &q, c1);
    yank_delete(g, p, q, 1, YANKONLY, NO_UNDO);  // save copy before change
    p = begin_line(g, p);
    q = end_line(g, q);
    i = count_lines(g, p, q);  // # of lines we are shifting
    for ( ; i > 0; i--, p = next_line(g, p)) {
      if (c == '<') {
        // shift left- remove tab or 8 spaces
        if (*p == '\t') {
          // shrink buffer 1 char
          text_hole_delete(g, p, p, NO_UNDO);
        } else if (*p == ' ') {
          // we should be calculating columns, not just SPACE
          for (j = 0; *p == ' ' && j < g->tabstop; j++) {
            text_hole_delete(g, p, p, NO_UNDO);
          }
        }
      } else if (c == '>') {
        // shift right -- add tab or 8 spaces
        char_insert(g, p, '\t', ALLOW_UNDO);
      }
    }
    g->dot = find_line(g, cnt);  // what line were we on
    dot_skip_over_ws(g);
    end_cmd_q(g);  // stop adding to q
    break;
  case 'A':      // A- append at e-o-l
    dot_end(g);    // go to e-o-l
    //**** fall through to ... 'a'
  case 'a':      // a- append after current char
    g->e->color(g->e->data, g->foregroundColor, g->backgroundColor);
    if (*g->dot != '\n')
      g->dot++;
    goto dc_i;
    break;
  case 'B':      // B- back a blank-delimited Word
  case 'E':      // E- end of a blank-delimited word
  case 'W':      // W- forward a blank-delimited word
    dir = FORWARD;
    if (c == 'B')
      dir = BACK;
    do {
      if (c == 'W' || sys_isspace(g->dot[dir])) {
        g->dot = skip_thing(g, g->dot, 1, dir, S_TO_WS);
        g->dot = skip_thing(g, g->dot, 2, dir, S_OVER_WS);
      }
      if (c != 'W')
        g->dot = skip_thing(g, g->dot, 1, dir, S_BEFORE_WS);
    } while (--g->cmdcnt > 0);
    break;
  case 'C':      // C- Change to e-o-l
  case 'D':      // D- delete to e-o-l
    save_dot = g->dot;
    g->dot = dollar_line(g, g->dot);  // move to before NL
    // copy text into a register and delete
    g->dot = yank_delete(g, save_dot, g->dot, 0, YANKDEL, ALLOW_UNDO);  // delete to e-o-l
    if (c == 'C')
      goto dc_i;  // start inserting
#if ENABLE_FEATURE_VI_DOT_CMD
    if (c == 'D')
      end_cmd_q(g);  // stop adding to q
#endif
    break;
  case 'g': // 'gg' goto a line number (vim) (default: very first line)
    c1 = get_one_char(g);
    if (c1 != 'g') {
      buf[0] = 'g';
      // c1 < 0 if the key was special. Try "g<up-arrow>"
      // TODO: if Unicode?
      buf[1] = (c1 >= 0 ? c1 : '*');
      buf[2] = '\0';
      not_implemented(g, buf);
      break;
    }
    if (g->cmdcnt == 0)
      g->cmdcnt = 1;
    // fall through
  case 'G':    // G- goto to a line number (default= E-O-F)
    g->dot = g->end - 1;        // assume E-O-F
    if (g->cmdcnt > 0) {
      g->dot = find_line(g, g->cmdcnt);  // what line is #cmdcnt
    }
    dot_skip_over_ws(g);
    break;
  case 'H':      // H- goto top line on screen
    g->dot = g->screenbegin;
    if (g->cmdcnt > (g->rows - 1)) {
      g->cmdcnt = (g->rows - 1);
    }
    if (--g->cmdcnt > 0) {
      do_cmd(g, '+');
    }
    dot_skip_over_ws(g);
    break;
  case 'I':      // I- insert before first non-blank
    dot_begin(g);  // 0
    dot_skip_over_ws(g);
    //**** fall through to ... 'i'
  case 'i':      // i- insert before current char
  case KEYCODE_INSERT:  // Cursor Key Insert
 dc_i:
    g->e->color(g->e->data, g->foregroundColor, g->backgroundColor);
    g->cmd_mode = 1;  // start inserting
    undo_queue_commit(g);  // commit queue when cmd_mode changes
    break;
  case 'J':      // J- join current and next lines together
    do {
      dot_end(g);    // move to NL
      if (g->dot < g->end - 1) {  // make sure not last char in text[]
#if ENABLE_FEATURE_VI_UNDO
        undo_push(g, g->dot, 1, UNDO_DEL);
        *g->dot++ = ' ';  // replace NL with space
        undo_push(g, (g->dot - 1), 1, UNDO_INS_CHAIN);
#else
        *g->dot++ = ' ';
        g->modified_count++;
#endif
        while (sys_isblank(*g->dot)) {  // delete leading WS
          text_hole_delete(g, g->dot, g->dot, ALLOW_UNDO_CHAIN);
        }
      }
    } while (--g->cmdcnt > 0);
    end_cmd_q(g);  // stop adding to q
    break;
  case 'L':      // L- goto bottom line on screen
    g->dot = end_screen(g);
    if (g->cmdcnt > (g->rows - 1)) {
      g->cmdcnt = (g->rows - 1);
    }
    if (--g->cmdcnt > 0) {
      do_cmd(g, '-');
    }
    dot_begin(g);
    dot_skip_over_ws(g);
    break;
  case 'M':      // M- goto middle line on screen
    g->dot = g->screenbegin;
    for (cnt = 0; cnt < (g->rows-1) / 2; cnt++)
      g->dot = next_line(g, g->dot);
    break;
  case 'O':      // O- open a empty line above
    //    0i\n ESC -i
    g->e->color(g->e->data, g->foregroundColor, g->backgroundColor);
    p = begin_line(g, g->dot);
    if (p[-1] == '\n') {
      dot_prev(g);
  case 'o':      // o- open a empty line below; Yes, I know it is in the middle of the "if (..."
      g->e->color(g->e->data, g->foregroundColor, g->backgroundColor);
      dot_end(g);
      g->dot = char_insert(g, g->dot, '\n', ALLOW_UNDO);
    } else {
      dot_begin(g);  // 0
      g->dot = char_insert(g, g->dot, '\n', ALLOW_UNDO);  // i\n ESC
      dot_prev(g);  // -
    }
    goto dc_i;
    break;
  case 'R':      // R- continuous Replace char
 dc5:
    g->cmd_mode = 2;
    undo_queue_commit(g);
    break;
  case KEYCODE_DELETE:
    if (g->dot < g->end - 1)
      g->dot = yank_delete(g, g->dot, g->dot, 1, YANKDEL, ALLOW_UNDO);
    break;
  case 'X':      // X- delete char before dot
  case 'x':      // x- delete the current char
  case 's':      // s- substitute the current char
    dir = 0;
    if (c == 'X')
      dir = -1;
    do {
      if (g->dot[dir] != '\n') {
        if (c == 'X')
          g->dot--;  // delete prev char
        g->dot = yank_delete(g, g->dot, g->dot, 0, YANKDEL, ALLOW_UNDO);  // delete char
      }
    } while (--g->cmdcnt > 0);
    end_cmd_q(g);  // stop adding to q
    if (c == 's')
      goto dc_i;  // start inserting
    break;
  case 'Z':      // Z- if modified, {write}; exit
    // ZZ means to save file (if necessary), then exit
    c1 = get_one_char(g);
    if (c1 != 'Z') {
      indicate_error(g);
      break;
    }
    if (g->modified_count) {
      if (ENABLE_FEATURE_VI_READONLY && g->readonly_mode) {
        status_line_bold(g, "'%s' is read only", g->current_filename);
        break;
      }
      cnt = file_write(g, g->current_filename, g->text, g->end - 1);
      if (cnt < 0) {
        if (cnt == -1)
          status_line_bold(g, "Write error: "STRERROR_FMT STRERROR_ERRNO);
      } else if (cnt == (g->end - 1 - g->text + 1)) {
        g->editing = 0;
      }
    } else {
      g->editing = 0;
    }
    break;
  case '^':      // ^- move to first non-blank on line
    dot_begin(g);
    dot_skip_over_ws(g);
    break;
  case 'b':      // b- back a word
  case 'e':      // e- end of word
    dir = FORWARD;
    if (c == 'b')
      dir = BACK;
    do {
      if ((g->dot + dir) < g->text || (g->dot + dir) > g->end - 1)
        break;
      g->dot += dir;
      if (sys_isspace(*g->dot)) {
        g->dot = skip_thing(g, g->dot, (c == 'e') ? 2 : 1, dir, S_OVER_WS);
      }
      if (sys_isalnum(*g->dot) || *g->dot == '_') {
        g->dot = skip_thing(g, g->dot, 1, dir, S_END_ALNUM);
      } else if (sys_ispunct(*g->dot)) {
        g->dot = skip_thing(g, g->dot, 1, dir, S_END_PUNCT);
      }
    } while (--g->cmdcnt > 0);
    break;
  case 'c':      // c- change something
  case 'd':      // d- delete something
#if ENABLE_FEATURE_VI_YANKMARK
  case 'y':      // y- yank   something
  case 'Y':      // Y- Yank a line
#endif
  {
    int yf, ml, whole = 0;
    yf = YANKDEL;  // assume either "c" or "d"
#if ENABLE_FEATURE_VI_YANKMARK
    if (c == 'y' || c == 'Y')
      yf = YANKONLY;
#endif
    c1 = 'y';
    if (c != 'Y')
      c1 = get_one_char(g);  // get the type of thing to delete
    // determine range, and whether it spans lines
    ml = find_range(g, &p, &q, c1);
    place_cursor(g, 0, 0);
    if (c1 == 27) {  // ESC- user changed mind and wants out
      c = c1 = 27;  // Escape- do nothing
    } else if (sys_strchr("wW", c1)) {
      ml = 0;  // multi-line ranges aren't allowed for words
      if (c == 'c') {
        // don't include trailing WS as part of word
        while (sys_isspace(*q) && q > p) {
          q--;
        }
      }
      g->dot = yank_delete(g, p, q, ml, yf, ALLOW_UNDO);  // delete word
    } else if (sys_strchr("^0bBeEft%$ lh\b\177", c1)) {
      // partial line copy text into a register and delete
      g->dot = yank_delete(g, p, q, ml, yf, ALLOW_UNDO);  // delete word
    } else if (sys_strchr("cdykjGHL+-{}\r\n", c1)) {
      // whole line copy text into a register and delete
      g->dot = yank_delete(g, p, q, ml, yf, ALLOW_UNDO);  // delete lines
      whole = 1;
    } else {
      // could not recognize object
      c = c1 = 27;  // error-
      ml = 0;
      indicate_error(g);
    }
    if (ml && whole) {
      if (c == 'c') {
        g->dot = char_insert(g, g->dot, '\n', ALLOW_UNDO_CHAIN);
        // on the last line of file don't move to prev line
        if (whole && g->dot != (g->end-1)) {
          dot_prev(g);
        }
      } else if (c == 'd') {
        dot_begin(g);
        dot_skip_over_ws(g);
      }
    }
    if (c1 != 27) {
      // if CHANGING, not deleting, start inserting after the delete
      if (c == 'c') {
        sys_strcpy(buf, "Change");
        goto dc_i;  // start inserting
      }
      if (c == 'd') {
        sys_strcpy(buf, "Delete");
      }
#if ENABLE_FEATURE_VI_YANKMARK
      if (c == 'y' || c == 'Y') {
        sys_strcpy(buf, "Yank");
      }
      p = g->reg[g->YDreg];
      q = p + sys_strlen(p);
      for (cnt = 0; p <= q; p++) {
        if (*p == '\n')
          cnt++;
      }
      status_line(g, "%s %u lines (%u chars) using [%c]",
        buf, cnt, (unsigned)sys_strlen(g->reg[g->YDreg]), what_reg(g));
#endif
      end_cmd_q(g);  // stop adding to q
    }
    break;
  }
  case 'k':      // k- goto prev line, same col
  case KEYCODE_UP:    // cursor key Up
    do {
      dot_prev(g);
      g->dot = move_to_col(g, g->dot, g->ccol + g->offset);  // try stay in same col
    } while (--g->cmdcnt > 0);
    break;
  case 'r':      // r- replace the current char with user input
    c1 = get_one_char(g);  // get the replacement char
    if (*g->dot != '\n') {
      g->dot = text_hole_delete(g, g->dot, g->dot, ALLOW_UNDO);
      g->dot = char_insert(g, g->dot, c1, ALLOW_UNDO_CHAIN);
      dot_left(g);
    }
    end_cmd_q(g);  // stop adding to q
    break;
  case 't':      // t- move to char prior to next x
    g->last_forward_char = get_one_char(g);
    do_cmd(g, ';');
    if (*g->dot == g->last_forward_char)
      dot_left(g);
    g->last_forward_char = 0;
    break;
  case 'w':      // w- forward a word
    do {
      if (sys_isalnum(*g->dot) || *g->dot == '_') {  // we are on ALNUM
        g->dot = skip_thing(g, g->dot, 1, FORWARD, S_END_ALNUM);
      } else if (sys_ispunct(*g->dot)) {  // we are on PUNCT
        g->dot = skip_thing(g, g->dot, 1, FORWARD, S_END_PUNCT);
      }
      if (g->dot < g->end - 1)
        g->dot++;    // move over word
      if (sys_isspace(*g->dot)) {
        g->dot = skip_thing(g, g->dot, 2, FORWARD, S_OVER_WS);
      }
    } while (--g->cmdcnt > 0);
    break;
  case 'z':      // z-
    c1 = get_one_char(g);  // get the replacement char
    cnt = 0;
    if (c1 == '.')
      cnt = (g->rows - 2) / 2;  // put dot at center
    if (c1 == '-')
      cnt = g->rows - 2;  // put dot at bottom
    g->screenbegin = begin_line(g, g->dot);  // start dot at top
    dot_scroll(g, cnt, -1);
    break;
  case '|':      // |- move to column "cmdcnt"
    g->dot = move_to_col(g, g->dot, g->cmdcnt - 1);  // try to move to column
    break;
  case '~':      // ~- flip the case of letters   a-z -> A-Z
    do {
#if ENABLE_FEATURE_VI_UNDO
      if (sys_islower(*g->dot)) {
        undo_push(g, g->dot, 1, UNDO_DEL);
        *g->dot = sys_toupper(*g->dot);
        undo_push(g, g->dot, 1, UNDO_INS_CHAIN);
      } else if (sys_isupper(*g->dot)) {
        undo_push(g, g->dot, 1, UNDO_DEL);
        *g->dot = sys_tolower(*g->dot);
        undo_push(g, g->dot, 1, UNDO_INS_CHAIN);
      }
#else
      if (sys_islower(*g->dot)) {
        *g->dot = sys_toupper(*g->dot);
        g->modified_count++;
      } else if (sys_isupper(*g->dot)) {
        *g->dot = sys_tolower(*g->dot);
        g->modified_count++;
      }
#endif
      dot_right(g);
    } while (--g->cmdcnt > 0);
    end_cmd_q(g);  // stop adding to q
    break;
    //----- The Cursor and Function Keys -----------------------------
  case KEYCODE_HOME:  // Cursor Key Home
    dot_begin(g);
    break;
    // The Fn keys could point to do_macro which could translate them
#if 0
  case KEYCODE_FUN1:  // Function Key F1
  case KEYCODE_FUN2:  // Function Key F2
  case KEYCODE_FUN3:  // Function Key F3
  case KEYCODE_FUN4:  // Function Key F4
  case KEYCODE_FUN5:  // Function Key F5
  case KEYCODE_FUN6:  // Function Key F6
  case KEYCODE_FUN7:  // Function Key F7
  case KEYCODE_FUN8:  // Function Key F8
  case KEYCODE_FUN9:  // Function Key F9
  case KEYCODE_FUN10:  // Function Key F10
  case KEYCODE_FUN11:  // Function Key F11
  case KEYCODE_FUN12:  // Function Key F12
    break;
#endif
  }

 dc1:
  // if text[] just became empty, add back an empty line
  if (g->end == g->text) {
    char_insert(g, g->text, '\n', NO_UNDO);  // start empty buf with dummy line
    g->dot = g->text;
  }
  // it is OK for dot to exactly equal to end, otherwise check dot validity
  if (g->dot != g->end) {
    g->dot = bound_dot(g, g->dot);  // make sure "dot" is valid
  }
#if ENABLE_FEATURE_VI_YANKMARK
  check_context(g, c);  // update the current context
#endif

  if (!sys_isdigit(c))
    g->cmdcnt = 0;    // cmd was not a number, reset cmdcnt
  cnt = g->dot - begin_line(g, g->dot);
  // Try to stay off of the Newline
  if (*g->dot == '\n' && cnt > 0 && g->cmd_mode == 0)
    g->dot--;
}

// NB!  the CRASHME code is unmaintained, and doesn't currently build
#if ENABLE_FEATURE_VI_CRASHME
static int totalcmds = 0;
static int Mp = 85;             // Movement command Probability
static int Np = 90;             // Non-movement command Probability
static int Dp = 96;             // Delete command Probability
static int Ip = 97;             // Insert command Probability
static int Yp = 98;             // Yank command Probability
static int Pp = 99;             // Put command Probability
static int M = 0, N = 0, I = 0, D = 0, Y = 0, P = 0, U = 0;
static const char chars[20] = "\t012345 abcdABCD-=.$";
static const char *const words[20] = {
  "this", "is", "a", "test",
  "broadcast", "the", "emergency", "of",
  "system", "quick", "brown", "fox",
  "jumped", "over", "lazy", "dogs",
  "back", "January", "Febuary", "March"
};
static const char *const lines[20] = {
  "You should have received a copy of the GNU General Public License\n",
  "char c, cm, *cmd, *cmd1;\n",
  "generate a command by percentages\n",
  "Numbers may be typed as a prefix to some commands.\n",
  "Quit, discarding changes!\n",
  "Forced write, if permission originally not valid.\n",
  "In general, any ex or ed command (such as substitute or delete).\n",
  "I have tickets available for the Blazers vs LA Clippers for Monday, Janurary 1 at 1:00pm.\n",
  "Please get w/ me and I will go over it with you.\n",
  "The following is a list of scheduled, committed changes.\n",
  "1.   Launch Norton Antivirus (Start, Programs, Norton Antivirus)\n",
  "Reminder....Town Meeting in Central Perk cafe today at 3:00pm.\n",
  "Any question about transactions please contact Sterling Huxley.\n",
  "I will try to get back to you by Friday, December 31.\n",
  "This Change will be implemented on Friday.\n",
  "Let me know if you have problems accessing this;\n",
  "Sterling Huxley recently added you to the access list.\n",
  "Would you like to go to lunch?\n",
  "The last command will be automatically run.\n",
  "This is too much english for a computer geek.\n",
};
static char *multilines[20] = {
  "You should have received a copy of the GNU General Public License\n",
  "char c, cm, *cmd, *cmd1;\n",
  "generate a command by percentages\n",
  "Numbers may be typed as a prefix to some commands.\n",
  "Quit, discarding changes!\n",
  "Forced write, if permission originally not valid.\n",
  "In general, any ex or ed command (such as substitute or delete).\n",
  "I have tickets available for the Blazers vs LA Clippers for Monday, Janurary 1 at 1:00pm.\n",
  "Please get w/ me and I will go over it with you.\n",
  "The following is a list of scheduled, committed changes.\n",
  "1.   Launch Norton Antivirus (Start, Programs, Norton Antivirus)\n",
  "Reminder....Town Meeting in Central Perk cafe today at 3:00pm.\n",
  "Any question about transactions please contact Sterling Huxley.\n",
  "I will try to get back to you by Friday, December 31.\n",
  "This Change will be implemented on Friday.\n",
  "Let me know if you have problems accessing this;\n",
  "Sterling Huxley recently added you to the access list.\n",
  "Would you like to go to lunch?\n",
  "The last command will be automatically run.\n",
  "This is too much english for a computer geek.\n",
};

// create a random command to execute
static void crash_dummy(struct globals *g)
{
  static int sleeptime;   // how long to pause between commands
  char c, cm, *cmd, *cmd1;
  int i, cnt, thing, rbi, startrbi, percent;

  // "dot" movement commands
  cmd1 = " \n\r\002\004\005\006\025\0310^$-+wWeEbBhjklHL";

  // is there already a command running?
  if (g->readbuffer[0] > 0)
    goto cd1;
 cd0:
  g->readbuffer[0] = 'X';
  startrbi = rbi = 1;
  sleeptime = 0;          // how long to pause between commands
  sys_memset(g->readbuffer, '\0', sizeof(g->readbuffer));
  // generate a command by percentages
  percent = (int) lrand48() % 100;        // get a number from 0-99
  if (percent < Mp) {     //  Movement commands
    // available commands
    cmd = cmd1;
    M++;
  } else if (percent < Np) {      //  non-movement commands
    cmd = "mz<>\'\"";       // available commands
    N++;
  } else if (percent < Dp) {      //  Delete commands
    cmd = "dx";             // available commands
    D++;
  } else if (percent < Ip) {      //  Inset commands
    cmd = "iIaAsrJ";        // available commands
    I++;
  } else if (percent < Yp) {      //  Yank commands
    cmd = "yY";             // available commands
    Y++;
  } else if (percent < Pp) {      //  Put commands
    cmd = "pP";             // available commands
    P++;
  } else {
    // We do not know how to handle this command, try again
    U++;
    goto cd0;
  }
  // randomly pick one of the available cmds from "cmd[]"
  i = (int) lrand48() % sys_strlen(cmd);
  cm = cmd[i];
  if (sys_strchr(":\024", cm))
    goto cd0;               // dont allow colon or ctrl-T commands
  g->readbuffer[rbi++] = cm; // put cmd into input buffer

  // now we have the command-
  // there are 1, 2, and multi char commands
  // find out which and generate the rest of command as necessary
  if (sys_strchr("dmryz<>\'\"", cm)) {        // 2-char commands
    cmd1 = " \n\r0$^-+wWeEbBhjklHL";
    if (cm == 'm' || cm == '\'' || cm == '\"') {    // pick a reg[]
      cmd1 = "abcdefghijklmnopqrstuvwxyz";
    }
    thing = (int) lrand48() % sys_strlen(cmd1); // pick a movement command
    c = cmd1[thing];
    g->readbuffer[rbi++] = c;  // add movement to input buffer
  }
  if (sys_strchr("iIaAsc", cm)) {     // multi-char commands
    if (cm == 'c') {
      // change some thing
      thing = (int) lrand48() % sys_strlen(cmd1); // pick a movement command
      c = cmd1[thing];
      g->readbuffer[rbi++] = c;  // add movement to input buffer
    }
    thing = (int) lrand48() % 4;    // what thing to insert
    cnt = (int) lrand48() % 10;     // how many to insert
    for (i = 0; i < cnt; i++) {
      if (thing == 0) {       // insert chars
        g->readbuffer[rbi++] = chars[((int) lrand48() % sys_strlen(chars))];
      } else if (thing == 1) {        // insert words
        sys_strcat(g->readbuffer, words[(int) lrand48() % 20]);
        sys_strcat(g->readbuffer, " ");
        sleeptime = 0;  // how fast to type
      } else if (thing == 2) {        // insert lines
        sys_strcat(g->readbuffer, lines[(int) lrand48() % 20]);
        sleeptime = 0;  // how fast to type
      } else {        // insert multi-lines
        sys_strcat(g->readbuffer, multilines[(int) lrand48() % 20]);
        sleeptime = 0;  // how fast to type
      }
    }
    sys_strcat(g->readbuffer, ESC);
  }
  g->readbuffer[0] = sys_strlen(g->readbuffer + 1);
 cd1:
  totalcmds++;
  if (sleeptime > 0)
    mysleep(g, sleeptime);      // sleep 1/100 sec
}

// test to see if there are any errors
static void crash_test()
{
  static time_t oldtim;

  time_t tim;
  char d[2], msg[80];

  msg[0] = '\0';
  if (g->end < g->text) {
    sys_strcat(msg, "end<text ");
  }
  if (g->end > g->textend) {
    sys_strcat(msg, "end>textend ");
  }
  if (g->dot < g->text) {
    sys_strcat(msg, "dot<text ");
  }
  if (g->dot > g->end) {
    sys_strcat(msg, "dot>end ");
  }
  if (g->screenbegin < g->text) {
    sys_strcat(msg, "screenbegin<text ");
  }
  if (g->screenbegin > g->end - 1) {
    sys_strcat(msg, "screenbegin>end-1 ");
  }

  if (msg[0]) {
    printf("\n\n%d: \'%c\' %s\n\n\n[Hit return to continue]",
      totalcmds, g->last_input_char, msg);
    fflush_all();
    while (safe_read(STDIN_FILENO, d, 1) > 0) {
      if (d[0] == '\n' || d[0] == '\r')
        break;
    }
  }
  tim = time(NULL);
  if (tim >= (oldtim + 3)) {
    sys_sprintf(g->status_buffer,
        "Tot=%d: M=%d N=%d I=%d D=%d Y=%d P=%d U=%d size=%d",
        totalcmds, M, N, I, D, Y, P, U, g->end - g->text + 1);
    oldtim = tim;
  }
}
#endif

static void edit_file(struct globals *g, char *fn)
{
#if ENABLE_FEATURE_VI_YANKMARK
#define cur_line g->edit_file__cur_line
#endif
  int c;
#if ENABLE_FEATURE_VI_USE_SIGNALS
  int sig;
#endif

  g->editing = 1;  // 0 = exit, 1 = one file, 2 = multiple files
  rawmode();
  g->rows = 24;
  g->columns = 80;
  IF_FEATURE_VI_ASK_TERMINAL(g->get_rowcol_error =) query_screen_dimensions(g);
#if ENABLE_FEATURE_VI_ASK_TERMINAL
  if (g->get_rowcol_error /* TODO? && no input on stdin */) {
    uint64_t k;
    write1(g, ESC"[999;999H" ESC"[6n");
    fflush_all();
    k = read_key(g, g->readbuffer, /*timeout_ms:*/ 100);
    if ((int32_t)k == KEYCODE_CURSOR_POS) {
      uint32_t rc = (k >> 32);
      g->columns = (rc & 0x7fff);
      if (columns > MAX_SCR_COLS)
        g->columns = MAX_SCR_COLS;
      g->rows = ((rc >> 16) & 0x7fff);
      if (g->rows > MAX_SCR_ROWS)
        g->rows = MAX_SCR_ROWS;
    }
  }
#endif
  new_screen(g, g->rows, g->columns);  // get memory for virtual screen
  init_text_buffer(g, fn);

#if ENABLE_FEATURE_VI_YANKMARK
  g->YDreg = 26;      // default Yank/Delete reg
//  Ureg = 27; - const    // hold orig line for "U" cmd
  g->mark[26] = g->mark[27] = g->text;  // init "previous context"
#endif

  g->last_forward_char = '\0';
#if ENABLE_FEATURE_VI_CRASHME
  g->last_input_char = '\0';
#endif
  g->crow = 0;
  g->ccol = 0;

#if ENABLE_FEATURE_VI_USE_SIGNALS
  signal(SIGWINCH, winch_handler);
  signal(SIGTSTP, tstp_handler);
  sig = sigsetjmp(restart, 1);
  if (sig != 0) {
    g->screenbegin = g->dot = g->text;
  }
  // int_handler() can jump to "restart",
  // must install handler *after* initializing "restart"
  signal(SIGINT, int_handler);
#endif

  g->cmd_mode = 0;    // 0=command  1=insert  2='R'eplace
  g->cmdcnt = 0;
  g->tabstop = 8;
  g->offset = 0;      // no horizontal offset
  c = '\0';
#if ENABLE_FEATURE_VI_DOT_CMD
  sys_free(g->ioq_start);
  g->ioq_start = NULL;
  g->lmc_len = 0;
  g->adding2q = 0;
#endif

#if ENABLE_FEATURE_VI_COLON
  {
    char *p, *q;
    int n = 0;

    while ((p = g->initial_cmds[n]) != NULL) {
      do {
        q = p;
        p = sys_strchr(q, '\n');
        if (p)
          while (*p == '\n')
            *p++ = '\0';
        if (*q)
          colon(g, q);
      } while (p);
      sys_free(g->initial_cmds[n]);
      g->initial_cmds[n] = NULL;
      n++;
    }
  }
#endif
  redraw(g, FALSE);      // dont force every col re-draw
  //------This is the main Vi cmd handling loop -----------------------
  while (g->editing > 0) {
#if ENABLE_FEATURE_VI_CRASHME
    if (crashme > 0) {
      if ((g->end - g->text) > 1) {
        crash_dummy();  // generate a random command
      } else {
        crashme = 0;
        string_insert(g, g->text, "\n\n#####  Ran out of text to work on.  #####\n\n", NO_UNDO);
        g->dot = g->text;
        refresh(g, FALSE);
      }
    }
#endif
    c = get_one_char(g);  // get a cmd from user
#if ENABLE_FEATURE_VI_CRASHME
    g->last_input_char = c;
#endif
#if ENABLE_FEATURE_VI_YANKMARK
    // save a copy of the current line- for the 'U" command
    if (begin_line(g, g->dot) != cur_line) {
      cur_line = begin_line(g, g->dot);
      text_yank(g, begin_line(g, g->dot), end_line(g, g->dot), Ureg);
    }
#endif
#if ENABLE_FEATURE_VI_DOT_CMD
    // If c is a command that changes text[],
    // (re)start remembering the input for the "." command.
    if (!g->adding2q
     && g->ioq_start == NULL
     && g->cmd_mode == 0 // command mode
     && c > '\0' // exclude NUL and non-ASCII chars
     && c < 0x7f // (Unicode and such)
     && sys_strchr(modifying_cmds, c)
    ) {
      start_new_cmd_q(g, c);
    }
#endif
    do_cmd(g, c);    // execute the user command

    // poll to see if there is input already waiting. if we are
    // not able to display output fast enough to keep up, skip
    // the display update until we catch up with input.
    if (!g->readbuffer[0] && mysleep(g, 0) == 0) {
      // no input pending - so update output
      refresh(g, FALSE);
      show_status_line(g);
    }
#if ENABLE_FEATURE_VI_CRASHME
    if (crashme > 0)
      crash_test();  // test editor variables
#endif
  }
  //-------------------------------------------------------------------

  go_bottom_and_clear_to_eol(g);
  cookmode();
#undef cur_line
}

/*
static void bb_show_usage(void) {
}
*/

static int vi_main(struct globals *g, int argc, char **argv) {
  g->last_modified_count = -1;
  g->last_search_pattern = xzalloc(2);

#if ENABLE_FEATURE_VI_UNDO
  //undo_stack_tail = NULL; - already is
# if ENABLE_FEATURE_VI_UNDO_QUEUE
  g->undo_queue_state = UNDO_EMPTY;
  //undo_q = 0; - already is
# endif
#endif

#if ENABLE_FEATURE_VI_CRASHME
  srand((long) getpid());
#endif
#ifdef NO_SUCH_APPLET_YET
  // if we aren't "vi", we are "view"
  if (ENABLE_FEATURE_VI_READONLY && applet_name[2]) {
    SET_READONLY_MODE(g->readonly_mode);
  }
#endif

  // autoindent is not default in vim 7.3
  g->vi_setops = /*VI_AUTOINDENT |*/ VI_SHOWMATCH | VI_IGNORECASE;
  //  1-  process $HOME/.exrc file (not inplemented yet)
  //  2-  process EXINIT variable from environment
  //  3-  process command line args
#if 0
#if ENABLE_FEATURE_VI_COLON
  {
    char *p = getenv("EXINIT");
    if (p && *p)
      g->initial_cmds[0] = sys_strndup(p, MAX_INPUT_LEN);
  }
#endif
#endif

#if 0
  while ((c = getopt(argc, argv, "hCRH" IF_FEATURE_VI_COLON("c:"))) != -1)
    switch (c) {
#if ENABLE_FEATURE_VI_CRASHME
    case 'C':
      crashme = 1;
      break;
#endif
#if ENABLE_FEATURE_VI_COLON
    case 'c':    // cmd line vi command
      if (*optarg)
        g->initial_cmds[g->initial_cmds[0] != NULL] = sys_strndup(optarg, MAX_INPUT_LEN);
      break;
#endif
#if ENABLE_FEATURE_VI_READONLY
    case 'R':    // Read-only flag
      SET_READONLY_MODE(g->readonly_mode);
      break;
#endif
    case 'h':
      show_help(g);
      // fall through
    default:
      //bb_show_usage();
      return 1;
    }
  }
#endif

  int optind = 0;
  argv += optind;
  g->cmdline_filecnt = argc - optind;

  g->e->cls(g->e->data, g->backgroundColor);

  // This is the main file handling loop
  optind = 0;
  while (1) {
    edit_file(g, argv[optind]); // might be NULL on 1st iteration
    // NB: optind can be changed by ":next" and ":rewind" commands
    optind++;
    if (optind >= g->cmdline_filecnt)
      break;
  }

  g->e->cls(g->e->data, g->backgroundColor);

  return 0;
}

static int edit_edit(editor_t *e, char *filename) {
  struct globals *g;
  char *argv[1], *ext;
  int r = -1;

  if ((g = sys_calloc(1, sizeof(struct globals))) != NULL) {
    argv[0] = filename;
    g->e = e;
    g->foregroundColor = RGBToLong((RGBColorType *)&foregroundColorRGB);
    g->backgroundColor = RGBToLong((RGBColorType *)&backgroundColorRGB);
    if ((ext = getext(filename)) != NULL) {
      if ((g->syntax = syntax_get_plugin(ext)) != NULL) {
        g->shigh = g->syntax->syntax_create(RGBToLong((RGBColorType *)&backgroundColorRGB));
      } 
    } 

    r = vi_main(g, 1, argv);
    if (g->syntax) g->syntax->syntax_destroy(g->shigh);
    sys_free(g);
  }

  return r;
}

static void edit_destroy(editor_t *e) {
  if (e) {
    e->edit = NULL;
    e->destroy = NULL;
  }
}

static void *PluginMain(void *p) {
  editor_t *e = (editor_t *)p;

  if (e) {
    e->edit = edit_edit;
    e->destroy = edit_destroy;
  }

  return e;
}

pluginMainF PluginInit(UInt32 *type, UInt32 *id) {
  *type = editPluginType;
  *id = editPluginId;

  return PluginMain;
}
