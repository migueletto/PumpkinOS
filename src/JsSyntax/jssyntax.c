#include <PalmOS.h>

#include "sys.h"
#include "pumpkin.h"
#include "syntax.h"
#include "color.h"
#include "xalloc.h"
#include "debug.h"

#define jsSyntaxPluginId '.js'

#define SYNTAX_RESERVED_WORD  1
#define SYNTAX_FUNCTION       2

#define MAX_BUF 256

typedef enum {
  state_initial,
  state_alpha,
  state_number,
  state_double_quote,
  state_single_quote,
  state_double_quote_escape,
  state_single_quote_escape,
  state_slash,
  state_comment,
  state_aster
} state_t;

static const RGBColorType reservedWordRGB = { 0, 0xff, 0xff, 0x20 };
static const RGBColorType functionRGB     = { 0, 0x20, 0xff, 0xff };
static const RGBColorType symbolRGB       = { 0, 0xff, 0xff, 0xff };
static const RGBColorType numberRGB       = { 0, 0xff, 0x20, 0xff };
static const RGBColorType otherRGB        = { 0, 0xff, 0xff, 0xff };
static const RGBColorType stringRGB       = { 0, 0x20, 0xff, 0x20 };
static const RGBColorType commentRGB      = { 0, 0xff, 0x20, 0x20 };

static const char *reservedWords[] = {
  "abstract", "arguments", "await", "boolean",
  "break", "byte", "case", "catch",
  "char", "class", "const", "continue",
  "debugger", "default", "delete", "do",
  "double", "else", "enum", "eval",
  "export", "extends", "false", "final",
  "finally", "float", "for", "function",
  "goto", "if", "implements", "import",
  "in", "instanceof", "int", "interface",
  "let", "long", "native", "new",
  "null", "package", "private", "protected",
  "public", "return", "short", "static",
  "super", "switch", "synchronized", "this",
  "throw", "throws", "transient", "true",
  "try", "typeof", "var", "void",
  "volatile", "while", "with", "yield",
  NULL
};

static const char *objsProps[] = {
  "Array", "Date", "eval", "function",
  "hasOwnProperty", "Infinity", "isFinite", "isNaN",
  "isPrototypeOf", "length", "Math", "NaN",
  "name", "Number", "Object", "prototype",
  "String", "toString", "undefined", "valueOf",
  NULL
};

struct syntax_highlight_t {
  int pe;
  state_t state;
  int comment, line_comment;
  uint32_t backgroundColor;
  uint32_t reservedWordColor;
  uint32_t functionColor;
  uint32_t symbolColor;
  uint32_t numberColor;
  uint32_t otherColor;
  uint32_t stringColor;
  uint32_t commentColor;
  char buf[MAX_BUF];
  int count;
  void (*print)(void *data, uint32_t fg, uint32_t bg, char *s, int n);
  void *data;
};

static syntax_plugin_t plugin;

static syntax_highlight_t *syntax_create(uint32_t bg) {
  syntax_highlight_t *shigh;
  int i;

  debug(DEBUG_TRACE, "SYNTAX", "Javascript syntax highlighting");
  if ((shigh = xcalloc(1, sizeof(syntax_highlight_t))) != NULL) {
    shigh->backgroundColor = bg;
    shigh->pe = pumpkin_script_create();

    for (i = 0; reservedWords[i]; i++) {
      pumpkin_script_global_iconst(shigh->pe, (char *)reservedWords[i], SYNTAX_RESERVED_WORD);
    }

    for (i = 0; objsProps[i]; i++) {
      pumpkin_script_global_iconst(shigh->pe, (char *)objsProps[i], SYNTAX_FUNCTION);
    }

    pumpkin_script_global_iconst(shigh->pe, "screen.width",    SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "screen.height",   SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "screen.rgb",      SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "screen.setpixel", SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "screen.clear",    SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "screen.draw",     SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "screen.line",     SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "screen.rect",     SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "screen.circle",   SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "screen.fill",     SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "db.open",         SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "db.close",        SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "rsrc.get",        SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "rsrc.get1",       SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "rsrc.release",    SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "bitmap.draw",     SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "bitmap.width",    SYNTAX_FUNCTION);
    pumpkin_script_global_iconst(shigh->pe, "bitmap.height",   SYNTAX_FUNCTION);

    shigh->reservedWordColor = RGBToLong((RGBColorType *)&reservedWordRGB);
    shigh->functionColor = RGBToLong((RGBColorType *)&functionRGB);
    shigh->symbolColor = RGBToLong((RGBColorType *)&symbolRGB);
    shigh->numberColor = RGBToLong((RGBColorType *)&numberRGB);
    shigh->otherColor = RGBToLong((RGBColorType *)&otherRGB);
    shigh->stringColor = RGBToLong((RGBColorType *)&stringRGB);
    shigh->commentColor = RGBToLong((RGBColorType *)&commentRGB);
  }

  return shigh;
}

static void syntax_destroy(syntax_highlight_t *shigh) {
  if (shigh) {
    if (shigh->pe > 0) pumpkin_script_destroy(shigh->pe);
    xfree(shigh);
  }
}

static uint32_t syntax_color(syntax_highlight_t *shigh, char *s) {
  uint32_t color = 0;
  uint16_t attr;
  int i, alpha, number, dots;

  if (shigh && s) {
    if (s[0]) {
      switch (pumpkin_script_global_iconst_value(shigh->pe, s)) {
        case SYNTAX_RESERVED_WORD:
          color = shigh->reservedWordColor;
          debug(DEBUG_TRACE, "SYNTAX", "reserved word \"%s\"", s);
          break;
        case SYNTAX_FUNCTION:
          color = shigh->functionColor;
          debug(DEBUG_TRACE, "SYNTAX", "function \"%s\"", s);
          break;
        default:
          alpha = number = dots = 0;
          for (i = 0; s[i]; i++) {
            attr = TxtCharAttr(s[i]);
            if (attr & charAttr_DI) number++;
            if (attr & (charAttr_LO | charAttr_UP)) alpha++;
            if (s[i] == '.') dots++;
          }
          if ((alpha + number) == i) {
            if (alpha == i) {
              // all chars are alphabetic
              debug(DEBUG_TRACE, "SYNTAX", "alphabetic symbol \"%s\"", s);
              color = shigh->symbolColor;
            } else if (number == i) {
              // all chars are numeric
              debug(DEBUG_TRACE, "SYNTAX", "numeric symbol \"%s\"", s);
              color = shigh->numberColor;
            } else {
              // all chars are alphanumeric
              if (!sys_strncmp(s, "0x", 2)) {
                debug(DEBUG_TRACE, "SYNTAX", "hexa numeric symbol \"%s\"", s);
                color = shigh->numberColor;
              } else {
                debug(DEBUG_TRACE, "SYNTAX", "alphanumeric symbol \"%s\"", s);
                color = shigh->otherColor;
              }
            }
          } else {
            if (dots == 1 && number + dots == i) {
              // all chars are numeric with a dot somewhere
              debug(DEBUG_TRACE, "SYNTAX", "float numeric symbol \"%s\"", s);
              color = shigh->numberColor;
            } else {
              // some chars are not alphanumeric
              debug(DEBUG_TRACE, "SYNTAX", "other symbol \"%s\"", s);
              color = shigh->otherColor;
            }
          }
          break;
      }
    } else {
      color = shigh->otherColor;
    }
  }

  return color;
}

static void syntax_begin_line(syntax_highlight_t *shigh, void (*print)(void *data, uint32_t fg, uint32_t bg, char *s, int n), void *data) {
  debug(DEBUG_TRACE, "SYNTAX", "begin line");
  shigh->print = print;
  shigh->data = data;
  shigh->count = 0;
  if (!shigh->comment) {
    shigh->state = state_initial;
  }
}

static void next_state(syntax_highlight_t *shigh, char c) {
  uint16_t attr;

  switch (c) {
    case '"':
      shigh->print(shigh->data, shigh->stringColor, shigh->backgroundColor, &c, 1);
      shigh->state = state_double_quote;
      break;
    case '\'':
      shigh->print(shigh->data, shigh->stringColor, shigh->backgroundColor, &c, 1);
      shigh->state = state_single_quote;
      break;
    case '/':
      shigh->state = state_slash;
      break;
    default:
      if (c == '_') {
        shigh->state = state_alpha;
        shigh->count = 0;
        shigh->buf[shigh->count++] = c;
        debug(DEBUG_TRACE, "SYNTAX", "begin alpha buffer \"%.*s\"", shigh->count, shigh->buf);
      } else {
        attr = TxtCharAttr(c);
        if (attr & charAttr_DI) {
          shigh->state = state_number;
          shigh->count = 0;
          shigh->buf[shigh->count++] = c;
          debug(DEBUG_TRACE, "SYNTAX", "begin number buffer \"%.*s\"", shigh->count, shigh->buf);
        } else if (attr & (charAttr_LO | charAttr_UP)) {
          shigh->state = state_alpha;
          shigh->count = 0;
          shigh->buf[shigh->count++] = c;
          debug(DEBUG_TRACE, "SYNTAX", "begin alpha buffer \"%.*s\"", shigh->count, shigh->buf);
        } else {
          shigh->print(shigh->data, shigh->otherColor, shigh->backgroundColor, &c, 1);
          shigh->state = state_initial;
        }
      }
      break;
  }
  debug(DEBUG_TRACE, "SYNTAX", "next state %d", shigh->state);
}

static void syntax_char(syntax_highlight_t *shigh, char c) {
  uint32_t color;
  uint16_t attr;
  char slash = '/';
  char aster = '*';

  debug(DEBUG_TRACE, "SYNTAX", "char '%c' (0x%02X) state %d", c, c, shigh->state);

  switch (shigh->state) {
    case state_initial:
      next_state(shigh, c);
      break;
    case state_alpha:
      attr = TxtCharAttr(c);
      if (c == '_' || (attr & (charAttr_LO | charAttr_UP | charAttr_DI))) {
        if (shigh->count < MAX_BUF-1) {
          shigh->buf[shigh->count++] = c;
          debug(DEBUG_TRACE, "SYNTAX", "add alpha buffer \"%.*s\"", shigh->count, shigh->buf);
        }
      } else {
        shigh->buf[shigh->count] = 0;
        color = syntax_color(shigh, shigh->buf);
        debug(DEBUG_TRACE, "SYNTAX", "end alpha buffer \"%s\" color 0x%08X", shigh->buf, color);
        shigh->print(shigh->data, color, shigh->backgroundColor, shigh->buf, shigh->count);
        shigh->count = 0;
        next_state(shigh, c);
      }
      break;
    case state_number:
      attr = TxtCharAttr(c);
      if ((attr & charAttr_DI) || c == 'x' || c == '.' || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
        if (shigh->count < MAX_BUF-1) {
          shigh->buf[shigh->count++] = c;
          debug(DEBUG_TRACE, "SYNTAX", "add number buffer \"%.*s\"", shigh->count, shigh->buf);
        }
      } else {
        shigh->buf[shigh->count] = 0;
        color = syntax_color(shigh, shigh->buf);
        debug(DEBUG_TRACE, "SYNTAX", "end number buffer \"%s\" color 0x%08X", shigh->buf, color);
        shigh->print(shigh->data, shigh->numberColor, shigh->backgroundColor, shigh->buf, shigh->count);
        shigh->count = 0;
        next_state(shigh, c);
      }
      break;
    case state_double_quote:
      switch (c) {
        case '"':
          debug(DEBUG_TRACE, "SYNTAX", "close double quote");
          shigh->print(shigh->data, shigh->stringColor, shigh->backgroundColor, &c, 1);
          shigh->state = state_initial;
          break;
        case '\\':
          shigh->print(shigh->data, shigh->stringColor, shigh->backgroundColor, &c, 1);
          shigh->state = state_double_quote_escape;
          break;
        default:
          shigh->print(shigh->data, shigh->stringColor, shigh->backgroundColor, &c, 1);
          break;
      }
      break;
    case state_double_quote_escape:
      shigh->print(shigh->data, shigh->stringColor, shigh->backgroundColor, &c, 1);
      break;
    case state_single_quote:
      switch (c) {
        case '\'':
          debug(DEBUG_TRACE, "SYNTAX", "close single quote");
          shigh->print(shigh->data, shigh->stringColor, shigh->backgroundColor, &c, 1);
          shigh->state = state_initial;
          break;
        case '\\':
          shigh->print(shigh->data, shigh->stringColor, shigh->backgroundColor, &c, 1);
          shigh->state = state_single_quote_escape;
          break;
        default:
          shigh->print(shigh->data, shigh->stringColor, shigh->backgroundColor, &c, 1);
          break;
      }
      break;
    case state_single_quote_escape:
      shigh->print(shigh->data, shigh->stringColor, shigh->backgroundColor, &c, 1);
      break;
    case state_slash:
      if (c == '/') {
        debug(DEBUG_TRACE, "SYNTAX", "begin line comment");
        shigh->print(shigh->data, shigh->commentColor, shigh->backgroundColor, &slash, 1);
        shigh->print(shigh->data, shigh->commentColor, shigh->backgroundColor, &slash, 1);
        shigh->state = state_comment;
        shigh->line_comment = 1;
        shigh->comment = 1;
      } else if (c == '*') {
        debug(DEBUG_TRACE, "SYNTAX", "begin block comment");
        shigh->print(shigh->data, shigh->commentColor, shigh->backgroundColor, &slash, 1);
        shigh->print(shigh->data, shigh->commentColor, shigh->backgroundColor, &aster, 1);
        shigh->state = state_comment;
        shigh->line_comment = 0;
        shigh->comment = 1;
      } else {
        shigh->print(shigh->data, shigh->otherColor, shigh->backgroundColor, &slash, 1);
        shigh->print(shigh->data, shigh->otherColor, shigh->backgroundColor, &c, 1);
        next_state(shigh, c);
      }
      break;
    case state_comment:
      if (c == '*') {
        if (shigh->line_comment) {
          shigh->print(shigh->data, shigh->commentColor, shigh->backgroundColor, &c, 1);
        } else {
          shigh->state = state_aster;
        }
      } else {
        shigh->print(shigh->data, shigh->commentColor, shigh->backgroundColor, &c, 1);
      }
      break;
    case state_aster:
      if (c == '/') {
        shigh->print(shigh->data, shigh->commentColor, shigh->backgroundColor, &c, 1);
        debug(DEBUG_TRACE, "SYNTAX", "end block comment");
        shigh->line_comment = 0;
        shigh->comment = 0;
        shigh->state = state_initial;
      } else {
        shigh->print(shigh->data, shigh->commentColor, shigh->backgroundColor, &aster, 1);
        shigh->print(shigh->data, shigh->commentColor, shigh->backgroundColor, &c, 1);
        shigh->state = state_comment;
      }
      break;
  }
}

static void syntax_end_line(syntax_highlight_t *shigh) {
  uint32_t color;

  debug(DEBUG_TRACE, "SYNTAX", "end line");

  if (shigh->count > 0) {
    shigh->buf[shigh->count] = 0;
    color = shigh->comment ? shigh->commentColor : syntax_color(shigh, shigh->buf);
    shigh->print(shigh->data, color, shigh->backgroundColor, shigh->buf, shigh->count);
    shigh->print(shigh->data, shigh->otherColor, shigh->backgroundColor, NULL, 0);
    shigh->count = 0;
  }

  if (shigh->line_comment) {
    debug(DEBUG_TRACE, "SYNTAX", "end line comment");
    shigh->line_comment = 0;
    shigh->comment = 0;
  }
}

static void *PluginMain(void *p) {
  plugin.syntax_create = syntax_create;
  plugin.syntax_destroy = syntax_destroy;
  plugin.syntax_begin_line = syntax_begin_line;
  plugin.syntax_char = syntax_char;
  plugin.syntax_end_line = syntax_end_line;

  return &plugin;
}

pluginMainF PluginInit(UInt32 *type, UInt32 *id) {
  *type = syntaxPluginType;
  *id = jsSyntaxPluginId;

  return PluginMain;
}
