#include "sys.h"
#include "script.h"
#include "debug.h"

#include "ht.h"
#include "dom.h"
#include "html.h"
#include "css.h"

#define MAX_TOKEN   64
#define MAX_STRING 256

typedef enum {
  TOKEN_SPACE = 0x1000,
  TOKEN_IDENT,
  TOKEN_HASH,
  TOKEN_CLASS,
  TOKEN_NUMBER,
  TOKEN_OPENB,
  TOKEN_CLOSEB,
  TOKEN_COLON,
  TOKEN_SCOLON,
  TOKEN_COMMA,
  TOKEN_CHILD,
  TOKEN_PLUS,
  TOKEN_GSIBLING,
  TOKEN_MINUS,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_OPENSB,
  TOKEN_CLOSESB,
  TOKEN_OPENP,
  TOKEN_CLOSEP,
  TOKEN_EQUAL,
  TOKEN_INCLUDES,
  TOKEN_DASHMATCH,
  TOKEN_STARTSWITH,
  TOKEN_ENDSWITH,
  TOKEN_CONTAINS,
  TOKEN_AT,
  TOKEN_STRING,
  TOKEN_EXCL,
  TOKEN_CM,
  TOKEN_MM,
  TOKEN_IN,
  TOKEN_PT,
  TOKEN_PC,
  TOKEN_PX,
  TOKEN_FR,
  TOKEN_EM,
  TOKEN_REM,
  TOKEN_DEG,
  TOKEN_S,
  TOKEN_DIM,
  TOKEN_PERCENT,
  TOKEN_IMPORTANT,
  TOKEN_PSEUDO,

  TOKEN_ERROR = 0xff00,
  TOKEN_EOF = 0xffff
} css_token_e;

#define CASE_NUMBER \
  case TOKEN_NUMBER: \
  case TOKEN_CM: \
  case TOKEN_MM: \
  case TOKEN_IN: \
  case TOKEN_PT: \
  case TOKEN_PC: \
  case TOKEN_PX: \
  case TOKEN_FR: \
  case TOKEN_EM: \
  case TOKEN_REM: \
  case TOKEN_DEG: \
  case TOKEN_S: \
  case TOKEN_DIM: \
  case TOKEN_PERCENT

typedef struct {
  char *name;
  css_token_e id;
} css_unit_t;

typedef struct {
  css_token_e id;
  char *value;
} css_token_t;

typedef struct {
  int state;
  char *buffer;
  int len;
  int pos;
  char token[MAX_STRING];
  char *text;
  uint32_t text_size;
  uint32_t text_pos;
  uint32_t line;
  ht *colors;
} css_lexer_t;

static char *stoken[] = {
  "SPACE",
  "IDENT",
  "HASH",
  "CLASS",
  "NUMBER",
  "OPENB",
  "CLOSEB",
  "COLON",
  "SCOLON",
  "COMMA",
  "CHILD",
  "PLUS",
  "GSIBLING",
  "MINUS",
  "STAR",
  "SLASH",
  "OPENSB",
  "CLOSESB",
  "OPENP",
  "CLOSEP",
  "EQUAL",
  "INCLUDES",
  "DASHMATCH",
  "STARTSWITH",
  "ENDSWITH",
  "CONTAINS",
  "AT",
  "STRING",
  "EXCL",
  "CM",
  "MM",
  "IN",
  "PT",
  "PC",
  "PX",
  "FR",
  "EM",
  "REM",
  "DEG",
  "S",
  "DIM",
  "PERCENT",
  "COLOR",
  "IMPORTANT",
  "PSEUDO"
};

static css_unit_t units[] = {
  { "cm",  TOKEN_CM  },
  { "mm",  TOKEN_MM  },
  { "in",  TOKEN_IN  },
  { "pt",  TOKEN_PT  },
  { "pc",  TOKEN_PC  },
  { "px",  TOKEN_PX  },
  { "fr",  TOKEN_FR  },
  { "em",  TOKEN_EM  },
  { "rem", TOKEN_REM },
  { "deg", TOKEN_DEG },
  { "s",   TOKEN_S   },
  { "%",   TOKEN_PERCENT },
  { NULL, 0 }
};

static char *color_names[] = {
  "aliceblue", "#f0f8ff",
  "antiquewhite", "#faebd7",
  "aqua", "#00ffff",
  "aquamarine", "#7fffd4",
  "azure", "#f0ffff",
  "beige", "#f5f5dc",
  "bisque", "#ffe4c4",
  "black", "#000000",
  "blanchedalmond", "#ffebcd",
  "blue", "#0000ff",
  "blueviolet", "#8a2be2",
  "brown", "#a52a2a",
  "burlywood", "#deb887",
  "cadetblue", "#5f9ea0",
  "chartreuse", "#7fff00",
  "chocolate", "#d2691e",
  "coral", "#ff7f50",
  "cornflowerblue", "#6495ed",
  "cornsilk", "#fff8dc",
  "crimson", "#dc143c",
  "cyan", "#00ffff",
  "darkblue", "#00008b",
  "darkcyan", "#008b8b",
  "darkgoldenrod", "#b8860b",
  "darkgray", "#a9a9a9",
  "darkgreen", "#006400",
  "darkkhaki", "#bdb76b",
  "darkmagenta", "#8b008b",
  "darkolivegreen", "#556b2f",
  "darkorange", "#ff8c00",
  "darkorchid", "#9932cc",
  "darkred", "#8b0000",
  "darksalmon", "#e9967a",
  "darkseagreen", "#8fbc8f",
  "darkslateblue", "#483d8b",
  "darkslategray", "#2f4f4f",
  "darkturquoise", "#00ced1",
  "darkviolet", "#9400d3",
  "deeppink", "#ff1493",
  "deepskyblue", "#00bfff",
  "dimgray", "#696969",
  "dodgerblue", "#1e90ff",
  "firebrick", "#b22222",
  "floralwhite", "#fffaf0",
  "forestgreen", "#228b22",
  "fuchsia", "#ff00ff",
  "gainsboro", "#dcdcdc",
  "ghostwhite", "#f8f8ff",
  "gold", "#ffd700",
  "goldenrod", "#daa520",
  "gray", "#808080",
  "green", "#008000",
  "greenyellow", "#adff2f",
  "honeydew", "#f0fff0",
  "hotpink", "#ff69b4",
  "indianred", "#cd5c5c",
  "indigo", "#4b0082",
  "ivory", "#fffff0",
  "khaki", "#f0e68c",
  "lavender", "#e6e6fa",
  "lavenderblush", "#fff0f5",
  "lawngreen", "#7cfc00",
  "lemonchiffon", "#fffacd",
  "lightblue", "#add8e6",
  "lightcoral", "#f08080",
  "lightcyan", "#e0ffff",
  "lightgoldenrodyellow", "#fafad2",
  "lightgrey", "#d3d3d3",
  "lightgreen", "#90ee90",
  "lightpink", "#ffb6c1",
  "lightsalmon", "#ffa07a",
  "lightseagreen", "#20b2aa",
  "lightskyblue", "#87cefa",
  "lightslategray", "#778899",
  "lightsteelblue", "#b0c4de",
  "lightyellow", "#ffffe0",
  "lime", "#00ff00",
  "limegreen", "#32cd32",
  "linen", "#faf0e6",
  "magenta", "#ff00ff",
  "maroon", "#800000",
  "mediumaquamarine", "#66cdaa",
  "mediumblue", "#0000cd",
  "mediumorchid", "#ba55d3",
  "mediumpurple", "#9370d8",
  "mediumseagreen", "#3cb371",
  "mediumslateblue", "#7b68ee",
  "mediumspringgreen", "#00fa9a",
  "mediumturquoise", "#48d1cc",
  "mediumvioletred", "#c71585",
  "midnightblue", "#191970",
  "mintcream", "#f5fffa",
  "mistyrose", "#ffe4e1",
  "moccasin", "#ffe4b5",
  "navajowhite", "#ffdead",
  "navy", "#000080",
  "oldlace", "#fdf5e6",
  "olive", "#808000",
  "olivedrab", "#6b8e23",
  "orange", "#ffa500",
  "orangered", "#ff4500",
  "orchid", "#da70d6",
  "palegoldenrod", "#eee8aa",
  "palegreen", "#98fb98",
  "paleturquoise", "#afeeee",
  "palevioletred", "#d87093",
  "papayawhip", "#ffefd5",
  "peachpuff", "#ffdab9",
  "peru", "#cd853f",
  "pink", "#ffc0cb",
  "plum", "#dda0dd",
  "powderblue", "#b0e0e6",
  "purple", "#800080",
  "red", "#ff0000",
  "rosybrown", "#bc8f8f",
  "royalblue", "#4169e1",
  "saddlebrown", "#8b4513",
  "salmon", "#fa8072",
  "sandybrown", "#f4a460",
  "seagreen", "#2e8b57",
  "seashell", "#fff5ee",
  "sienna", "#a0522d",
  "silver", "#c0c0c0",
  "skyblue", "#87ceeb",
  "slateblue", "#6a5acd",
  "slategray", "#708090",
  "snow", "#fffafa",
  "springgreen", "#00ff7f",
  "steelblue", "#4682b4",
  "tan", "#d2b48c",
  "teal", "#008080",
  "thistle", "#d8bfd8",
  "tomato", "#ff6347",
  "turquoise", "#40e0d0",
  "violet", "#ee82ee",
  "wheat", "#f5deb3",
  "white", "#ffffff",
  "whitesmoke", "#f5f5f5",
  "yellow", "#ffff00",
  "yellowgreen", "#9acd32",
  NULL
};

ht *css_color_names(void) {
  ht *colors;
  int i;

  colors = ht_create();
  for (i = 0; color_names[i]; i += 2) {
    ht_set(colors, color_names[i], color_names[i+1]);
  }

  return colors;
}

static char *css_token_name(css_token_e id) {
  return id ? stoken[id - 0x1000] : NULL;
}

static css_lexer_t *css_create(char *buffer, int len) {
  css_lexer_t *l;

  if ((l = sys_calloc(1, sizeof(css_lexer_t))) != NULL) {
    l->buffer = buffer;
    l->len = len;
    l->line = 1;
  }

  return l;
}

static void css_destroy(css_lexer_t *l) {
  if (l) {
    sys_free(l);
  }
}

static uint16_t tok(css_lexer_t *l, uint16_t token, css_token_t *t) {
  t->id = token;
  t->value = NULL;

  switch (token) {
    case TOKEN_IDENT:
    case TOKEN_HASH:
    case TOKEN_CLASS:
    case TOKEN_STRING:
    CASE_NUMBER:
      t->value = sys_strdup(l->text);
      break;
  }

  return token;
}

static uint16_t check_unit(css_lexer_t *l) {
  int i, m, n;

  m = sys_strlen(l->text);
  for (i = 0; units[i].name; i++) {
    n = sys_strlen(units[i].name);
    if (!sys_strcmp(&l->text[m - n], units[i].name)) {
      l->text[m - n] = 0;
      return units[i].id;
    }
  }

  return TOKEN_DIM;
}

static int next(css_lexer_t *l) {
  if (l->pos == l->len) return 0;
  return l->buffer[l->pos++];
}

static void back(css_lexer_t *l) {
  if (l->pos) l->pos--;
}

static uint16_t css_next(css_lexer_t *l, css_token_t *t) {
  char c;

  for (;;) {
    c = next(l);
    if (c == -1) return tok(l, TOKEN_ERROR, t);
    if (c == 0) return tok(l, TOKEN_EOF, t);
    c = sys_tolower(c);

    if (!l->text) {
      l->text_size = 1024;
      l->text = sys_calloc(1, l->text_size);
    } else if (l->text_pos == l->text_size - 1) {
      l->text_size += 1024;
      l->text = sys_realloc(l->text, l->text_size);
    }

    debug(DEBUG_TRACE, "CSS", "char [%c] state %d", c, l->state);

    switch (l->state) {
      case 0:
        switch (c) {
          case '\n':
            l->line++;
            l->state = 22;
            break;
          case '\r':
          case '\t':
          case ' ':
            l->state = 22;
            break;
          case ':': return tok(l, TOKEN_COLON, t);
          case '{': return tok(l, TOKEN_OPENB, t);
          case '}': return tok(l, TOKEN_CLOSEB, t);
          case ',': return tok(l, TOKEN_COMMA, t);
          case ';': return tok(l, TOKEN_SCOLON, t);
          case '[': return tok(l, TOKEN_OPENSB, t);
          case ']': return tok(l, TOKEN_CLOSESB, t);
          case '(': return tok(l, TOKEN_OPENP, t);
          case ')': return tok(l, TOKEN_CLOSEP, t);
          case '!': return tok(l, TOKEN_EXCL, t);
          case '>': return tok(l, TOKEN_CHILD, t);
          case '@': return tok(l, TOKEN_AT, t);
          case '=': return tok(l, TOKEN_EQUAL, t);
          case '+': return tok(l, TOKEN_PLUS, t);
          case '~': l->state = 17; break;
          case '|': l->state = 18; break;
          case '^': l->state = 19; break;
          case '$': l->state = 20; break;
          case '*': l->state = 21; break;
          case '\'': l->state = 2; break;
          case '"': l->state = 3; break;
          case '/': l->state = 4; break;
          case '#':
            l->text[l->text_pos++] = c;
            l->state = 10;
            break;
          case '.':
            l->text[l->text_pos++] = c;
            l->state = 11;
            break;
          default:
            if ((c >= 'a' && c <= 'z') || c == '_') {
              l->text[l->text_pos++] = c;
              l->state = 1;
            } else if (c >= '0' && c <= '9') {
              l->text[l->text_pos++] = c;
              l->state = 15;
            } else if (c == '-') {
              l->text[l->text_pos++] = c;
              l->state = 16;
            } else {
              return tok(l, TOKEN_ERROR, t);
            }
            break;
        }
        break;
      case 1:
        switch (c) {
          case '-':
          case '_':
            l->text[l->text_pos++] = c;
            break;
          default:
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
              l->text[l->text_pos++] = c;
            } else {
              back(l);
              l->text[l->text_pos] = 0;
              l->text_pos = 0;
              if (!sys_strcmp(l->text, "url")) {
                l->state = 7;
                return tok(l, TOKEN_IDENT, t);
              }
              l->state = 0;
              return tok(l, TOKEN_IDENT, t);
            }
            break;
        }
        break;
      case 2:
        if (c == '\'') {
          l->text[l->text_pos] = 0;
          l->text_pos = 0;
          l->state = 0;
          return tok(l, TOKEN_STRING, t);
        }
        l->text[l->text_pos++] = c;
        break;
      case 3:
        if (c == '"') {
          l->text[l->text_pos] = 0;
          l->text_pos = 0;
          l->state = 0;
          return tok(l, TOKEN_STRING, t);
        }
        l->text[l->text_pos++] = c;
        break;
      case 4:
        switch (c) {
          case '*':
            l->state = 5;
            break;
          default:
            back(l);
            l->state = 0;
            return tok(l, TOKEN_SLASH, t);
        }
        break;
      case 5:
        if (c == '*') l->state = 6;
        break;
      case 6:
        if (c == '/') l->state = 0; else l->state = 5;
        break;
      case 7:
        if (c != '(') return tok(l, TOKEN_ERROR, t);
        l->state = 8;
        break;
      case 8:
        if (c == ')') {
          l->text[l->text_pos] = 0;
          l->text_pos = 0;
          l->state = 0;
          return tok(l, TOKEN_STRING, t);
        }
        l->text[l->text_pos++] = c;
        break;
      case 10:
        switch (c) {
          case '-':
          case '_':
            l->text[l->text_pos++] = c;
            break;
          default:
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
              l->text[l->text_pos++] = c;
            } else {
              back(l);
              l->text[l->text_pos] = 0;
              l->text_pos = 0;
              l->state = 0;
              return tok(l, TOKEN_HASH, t);
            }
            break;
        }
        break;
      case 11:
        switch (c) {
          case '-':
          case '_':
            l->text[l->text_pos++] = c;
            l->state = 12;
            break;
          default:
            if (c >= 'a' && c <= 'z') {
              l->text[l->text_pos++] = c;
              l->state = 12;
            } else if (c >= '0' && c <= '9') {
              l->text[l->text_pos++] = c;
              l->state = 13;
            } else {
              return tok(l, TOKEN_ERROR, t);
            }
            break;
        }
        break;
      case 12:
        switch (c) {
          case '-':
          case '_':
            l->text[l->text_pos++] = c;
            break;
          default:
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
              l->text[l->text_pos++] = c;
            } else {
              back(l);
              l->text[l->text_pos] = 0;
              l->text_pos = 0;
              l->state = 0;
              return tok(l, TOKEN_CLASS, t);
            }
            break;
        }
        break;
      case 13:
        // num [0-9]+|[0-9]*"."[0-9]+
        if (c >= '0' && c <= '9') {
          l->text[l->text_pos++] = c;
        } else if (c >= 'a' && c <= 'z') {
          l->text[l->text_pos++] = c;
          l->state = 14;
        } else if (c == '%') {
          l->text[l->text_pos] = 0;
          l->text_pos = 0;
          l->state = 0;
          return tok(l, TOKEN_PERCENT, t);
        } else {
          back(l);
          l->text[l->text_pos] = 0;
          l->text_pos = 0;
          l->state = 0;
          return tok(l, TOKEN_NUMBER, t);
        }
        break;
      case 14:
        if (c >= 'a' && c <= 'z') {
          l->text[l->text_pos++] = c;
        } else {
          back(l);
          l->text[l->text_pos] = 0;
          l->text_pos = 0;
          l->state = 0;
          return tok(l, check_unit(l), t);
        }
        break;
      case 15:
        if (c >= '0' && c <= '9') {
          l->text[l->text_pos++] = c;
        } else if (c == '.') {
          l->text[l->text_pos++] = c;
          l->state = 13;
        } else if (c >= 'a' && c <= 'z') {
          l->text[l->text_pos++] = c;
          l->state = 14;
        } else if (c == '%') {
          l->text[l->text_pos] = 0;
          l->text_pos = 0;
          l->state = 0;
          return tok(l, TOKEN_PERCENT, t);
        } else {
          back(l);
          l->text[l->text_pos] = 0;
          l->text_pos = 0;
          l->state = 0;
          return tok(l, TOKEN_NUMBER, t);
        }
        break;
      case 16:
        if (c >= '0' && c <= '9') {
          l->text[l->text_pos++] = c;
          l->state = 15;
        } else if (c == '.') {
          l->text[l->text_pos++] = c;
          l->state = 13;
        } else if ((c >= 'a' && c <= 'z') || c == '-') {
          l->text[l->text_pos++] = c;
          l->state = 1;
        } else {
          back(l);
          l->text[l->text_pos] = 0;
          l->text_pos = 0;
          l->state = 0;
          return tok(l, TOKEN_MINUS, t);
        }
        break;
      case 17:
        l->state = 0;
        if (c == '=') return tok(l, TOKEN_INCLUDES, t);
        return tok(l, TOKEN_GSIBLING, t);
      case 18:
        if (c == '=') {
          l->state = 0;
          return tok(l, TOKEN_DASHMATCH, t);
        }
        return tok(l, TOKEN_ERROR, t);
      case 19:
        if (c == '=') {
          l->state = 0;
          return tok(l, TOKEN_STARTSWITH, t);
        }
        return tok(l, TOKEN_ERROR, t);
      case 20:
        if (c == '=') {
          l->state = 0;
          return tok(l, TOKEN_ENDSWITH, t);
        }
        return tok(l, TOKEN_ERROR, t);
      case 21:
        l->state = 0;
        if (c == '=') return tok(l, TOKEN_CONTAINS, t);
        return tok(l, TOKEN_STAR, t);
      case 22:
        switch (c) {
          case '\n':
            l->line++;
            break;
          case '\r':
          case '\t':
          case ' ':
            break;
          default:
            back(l);
            l->state = 0;
            return tok(l, TOKEN_SPACE, t);
        }
        break;
    }
  }

  return tok(l, TOKEN_ERROR, t);
}

void css_test(char *buffer, int len) {
  css_lexer_t *l;
  css_token_t t;

  l = css_create(buffer, len);
  for (;;) {
    css_next(l, &t);
    if (t.id == TOKEN_ERROR) {
      debug(1, "CSS", "error state %d line %d", l->state, l->line);
      break;
    }
    if (t.id == TOKEN_EOF) {
      debug(1, "CSS", "eof line %d", l->line);
      break;
    }
    if (t.value) {
      debug(1, "CSS", "token [%s] [%s] line %d", css_token_name(t.id), t.value, l->line);
    } else {
      debug(1, "CSS", "token [%s] line %d", css_token_name(t.id), l->line);
    }
  }
  css_destroy(l);
}

static int syntax_error(css_lexer_t *l, css_token_t *t, int state) {
  debug(DEBUG_ERROR, "CSS", "invalid token %s at line %d (state %d)", css_token_name(t->id), l->line, state);
  return -1;
}

/*
ruleset: selector [ , selector ]* '{'
           declaration? [ ; declaration? ]*
         '}'

selector: simple_selector [ combinator selector | [ combinator? selector ]? ]?

simple_selector: element_name [ hash | class | attrib | pseudo ]* |
                 [ hash | class | attrib | pseudo ]+

combinator: '+' | '>'

element_name: IDENT | '*'

hash: '#' IDENT

class: . IDENT

attrib: '[' IDENT [
              [ = | '~=' | '|=' ]
              [ IDENT | STRING ]
            ]?
        ']'

pseudo: ':' [ IDENT | IDENT '(' [ IDENT ]? ')' ]
*/


enum {
  PARSER_STATE_FILTER_START = 100,
  PARSER_STATE_FILTER_NEXT,
  PARSER_STATE_FILTER_PSEUDO,
  PARSER_STATE_FILTER_PSEUDO2,
  PARSER_STATE_FILTER_ATTR,
  PARSER_STATE_FILTER_ATTR_OP,
  PARSER_STATE_FILTER_ATTR_VALUE,
  PARSER_STATE_FILTER_ATTR_CLOSE,
  PARSER_STATE_DECLARATIONS,
  PARSER_STATE_DECL_IDENT,
  PARSER_STATE_DECL_VALUE, // 110
  PARSER_STATE_DECL_VALUE_VAR,
  PARSER_STATE_DECL_VALUE_VAR_CLOSE,
  PARSER_STATE_DECL_VALUE_FUNCTION,
  PARSER_STATE_DECL_VALUE_RGBA,
  PARSER_STATE_IMPORTANT,
  PARSER_STATE_IMPORTANT_END,
  PARSER_STATE_AT_RULE,
  PARSER_STATE_MEDIA,
  PARSER_STATE_MEDIA_AND,
  PARSER_STATE_MEDIA_OPENP,
  PARSER_STATE_ATRULE_OPENB,
  PARSER_STATE_KEYFRAMES
};

static void parse_color(char *s, css_decl_value_t *value) {
  int red, green, blue;

  if (s[0] == '#' && sys_sscanf(s, "#%2x%2x%2x", &red, &green, &blue) == 3) {
    value->data.color = (0xff << 24) | (red << 16) | (green << 8) | blue;
    value->type = CSS_DECL_COLOR;
  }
}

static void map_color(char *s, css_decl_value_t *value, html_env_t *env) {
  s = ht_get(env->colors, s);

  if (s) {
    parse_color(s, value);
  }
}

// 1 in = 96 px
// 1 pt = 96/72 px
// 1 pc = 16 px
// 1 cm = 37.8 px
// 1 mm = 3.78 px
// 1 rem = 16 px

static void parse_number(char *value, css_number_t *number) {
  if (sys_strchr(value, '.')) {
    number->integer = 0;
    number->value.d = sys_atof(value);
    switch (number->unit) {
      case CSS_UNIT_CM:
        number->integer = 1;
        number->value.i = (int)(37.8 * number->value.d + 0.5);
        number->unit = CSS_UNIT_PX;
        break;
      case CSS_UNIT_MM:
        number->integer = 1;
        number->value.i = (int)(3.78 * number->value.d + 0.5);
        number->unit = CSS_UNIT_PX;
        break;
      case CSS_UNIT_IN:
        number->integer = 1;
        number->value.i = (int)(96.0 * number->value.d + 0.5);
        number->unit = CSS_UNIT_PX;
        break;
      case CSS_UNIT_PT:
        number->integer = 1;
        number->value.i = (int)((96.0 * number->value.d) / 72.0 + 0.5);
        number->unit = CSS_UNIT_PX;
        break;
      case CSS_UNIT_PC:
        number->integer = 1;
        number->value.i = (int)(16.0 * number->value.d + 0.5);
        number->unit = CSS_UNIT_PX;
        break;
      default:
        break;
    }
  } else {
    number->integer = 1;
    number->value.i = sys_atoi(value);
    switch (number->unit) {
      case CSS_UNIT_CM:
        number->value.i = (int)(37.8 * number->value.i + 0.5);
        number->unit = CSS_UNIT_PX;
        break;
      case CSS_UNIT_MM:
        number->value.i = (int)(3.78 * number->value.i + 0.5);
        number->unit = CSS_UNIT_PX;
        break;
      case CSS_UNIT_IN:
        number->value.i *= 96;
        number->unit = CSS_UNIT_PX;
        break;
      case CSS_UNIT_PT:
        number->value.i = (int)((96.0 * number->value.i) / 72.0 + 0.5);
        number->unit = CSS_UNIT_PX;
        break;
      case CSS_UNIT_PC:
        number->value.i *= 16;
        number->unit = CSS_UNIT_PX;
        break;
      default:
        break;
    }
  }
}

#define CALLBACK if (callback) callback(curr_decl, data)

#define PARSE_NUMBER(v,t,tt,u) \
  case t: \
    if (!atrule) { \
    if (!inlist) { \
      curr_decl->value.type = tt; \
      curr_decl->value.data.number.unit = u; \
      parse_number(v, &curr_decl->value.data.number); \
    } else { \
      if (list == NULL) { \
        list = init_list(&curr_decl->value); \
      } \
      list->elements[list->len].value.type = tt; \
      list->elements[list->len].value.data.number.unit = u; \
      list->elements[list->len].separator = separator; \
      parse_number(v, &list->elements[list->len].value.data.number); \
      list->len++; \
    } \
    CALLBACK; \
    } \
    inlist = 1;

#define PARSE_NUMBERS \
  PARSE_NUMBER(t.value, TOKEN_NUMBER, CSS_DECL_NUMBER, CSS_UNIT_NONE); \
  break; \
  PARSE_NUMBER(t.value, TOKEN_CM, CSS_DECL_NUMBER, CSS_UNIT_CM); \
  break; \
  PARSE_NUMBER(t.value, TOKEN_MM, CSS_DECL_NUMBER, CSS_UNIT_MM); \
  break; \
  PARSE_NUMBER(t.value, TOKEN_IN, CSS_DECL_NUMBER, CSS_UNIT_IN); \
  break; \
  PARSE_NUMBER(t.value, TOKEN_PT, CSS_DECL_NUMBER, CSS_UNIT_PT); \
  break; \
  PARSE_NUMBER(t.value, TOKEN_PC, CSS_DECL_NUMBER, CSS_UNIT_PC); \
  break; \
  PARSE_NUMBER(t.value, TOKEN_PX, CSS_DECL_NUMBER, CSS_UNIT_PX); \
  break; \
  PARSE_NUMBER(t.value, TOKEN_EM, CSS_DECL_NUMBER, CSS_UNIT_EM); \
  break; \
  PARSE_NUMBER(t.value, TOKEN_REM, CSS_DECL_NUMBER, CSS_UNIT_REM); \
  break; \
  PARSE_NUMBER(t.value, TOKEN_DEG, CSS_DECL_NUMBER, CSS_UNIT_DEG) \
  break; \
  PARSE_NUMBER(t.value, TOKEN_PERCENT, CSS_DECL_NUMBER, CSS_UNIT_PERCENT) \
  break; \
  PARSE_NUMBER(t.value, TOKEN_S, CSS_DECL_NUMBER, CSS_UNIT_S)

static void css_print_filter(css_filter_t *f) {
  switch (f->type) {
    case CSS_FILTER_TAG:
      debug(DEBUG_INFO, "CSS", "tag [%s]", f->data.tag);
      break;
    case CSS_FILTER_ID:
      debug(DEBUG_INFO, "CSS", "id [%s]", f->data.id);
      break;
    case CSS_FILTER_CLASS:
      debug(DEBUG_INFO, "CSS", "class [%s]", f->data.cclass);
      break;
    case CSS_FILTER_PSEUDO:
      debug(DEBUG_INFO, "CSS", "pseudo [%s]", f->data.pseudo);
      break;
    case CSS_FILTER_ATTR:
      switch (f->data.attr.op) {
        case CSS_ATTR_OP_EXISTS:
          debug(DEBUG_INFO, "CSS", "attribute [%s]", f->data.attr.name);
          break;
        case CSS_ATTR_OP_EQUALS:
          debug(DEBUG_INFO, "CSS", "attribute [%s] = [%s]", f->data.attr.name, f->data.attr.value);
          break;
        case CSS_ATTR_OP_INCLUDES:
          debug(DEBUG_INFO, "CSS", "attribute [%s] ~= [%s]", f->data.attr.name, f->data.attr.value);
          break;
        case CSS_ATTR_OP_DASHMATCH:
          debug(DEBUG_INFO, "CSS", "attribute [%s] |= [%s]", f->data.attr.name, f->data.attr.value);
          break;
        case CSS_ATTR_OP_STARTS:
          debug(DEBUG_INFO, "CSS", "attribute [%s] ^= [%s]", f->data.attr.name, f->data.attr.value);
          break;
        case CSS_ATTR_OP_ENDS:
          debug(DEBUG_INFO, "CSS", "attribute [%s] $= [%s]", f->data.attr.name, f->data.attr.value);
          break;
        case CSS_ATTR_OP_CONTAINS:
          debug(DEBUG_INFO, "CSS", "attribute [%s] *= [%s]", f->data.attr.name, f->data.attr.value);
          break;
      }
      break;
    case CSS_FILTER_ANY:
      debug(DEBUG_INFO, "CSS", "any");
      break;
    case CSS_FILTER_OTHER:
      debug(DEBUG_INFO, "CSS", "other [%s]", f->data.other);
      break;
  }
}

static void css_print_selector(css_selector_t *s) {
  css_filter_t *f;

  switch (s->combinator) {
    case CSS_COMBINATOR_NONE:
      break;
    case CSS_COMBINATOR_DESCENDANT:
      debug(DEBUG_INFO, "CSS", "selector descendant");
      break;
    case CSS_COMBINATOR_CHILD:
      debug(DEBUG_INFO, "CSS", "selector child");
      break;
    case CSS_COMBINATOR_ADJ_SIBLING:
      debug(DEBUG_INFO, "CSS", "selector adjacent sibling");
      break;
    case CSS_COMBINATOR_GEN_SIBLING:
      debug(DEBUG_INFO, "CSS", "selector general sibling");
      break;
    case CSS_COMBINATOR_COMMA:
      debug(DEBUG_INFO, "CSS", "selector comma");
      break;
  }

  for (f = s->group; f; f = f->next) {
    css_print_filter(f);
  }
}

static void css_print_decl_value(char *name, css_decl_value_t *value) {
  char *u;
  int i;

  switch (value->type) {
    case CSS_DECL_NONE:
      break;
    case CSS_DECL_IDENT:
      debug(DEBUG_INFO, "CSS", "%s: %s", name, value->data.ident);
      break;
    case CSS_DECL_STRING:
      debug(DEBUG_INFO, "CSS", "%s: \"%s\"", name, value->data.string);
      break;
    case CSS_DECL_COLOR:
      debug(DEBUG_INFO, "CSS", "%s: #%08X", name, value->data.color);
      break;
    case CSS_DECL_NUMBER:
      switch (value->data.number.unit) {
        case CSS_UNIT_CM: u = "cm"; break;
        case CSS_UNIT_MM: u = "mm"; break;
        case CSS_UNIT_IN: u = "in"; break;
        case CSS_UNIT_PT: u = "pt"; break;
        case CSS_UNIT_PC: u = "pc"; break;
        case CSS_UNIT_PX: u = "px"; break;
        case CSS_UNIT_EM: u = "em"; break;
        case CSS_UNIT_REM: u = "rem"; break;
        case CSS_UNIT_DEG: u = "deg"; break;
        case CSS_UNIT_PERCENT: u = "%"; break;
        case CSS_UNIT_NONE: u = ""; break;
        default: u = "???"; break;
      }
      if (value->data.number.integer) {
        debug(DEBUG_INFO, "CSS", "%s: %d%s", name, value->data.number.value.i, u);
      } else {
        debug(DEBUG_INFO, "CSS", "%s: %6.4f%s", name, value->data.number.value.d, u);
      }
      break;
    case CSS_DECL_LIST:
      debug(DEBUG_INFO, "CSS", "%s: list(%d)", name, value->data.list.len);
      for (i = 0; i < value->data.list.len; i++) {
        debug_indent(2);
        if (value->data.list.elements[i].separator) debug(DEBUG_INFO, "CSS", "separator '%c'", value->data.list.elements[i].separator);
        css_print_decl_value("", &value->data.list.elements[i].value);
        debug_indent(-2);
      }
      break;
    case CSS_DECL_VAR:
      debug(DEBUG_INFO, "CSS", "%s: var(%s)", name, value->data.var);
      break;
    case CSS_DECL_FUNCTION:
      debug(DEBUG_INFO, "CSS", "%s: %s() nargs=%d", name, value->data.function.name, value->data.function.args.len);
      break;
  }
}

static void css_print_rule(css_rule_t *rule) {
  css_selector_t *s;
  css_declaration_t *d;

  debug(DEBUG_INFO, "CSS", "rule begin");

  for (s = rule->selectors; s; s = s->next) {
    debug_indent(2);
    css_print_selector(s);
    debug_indent(-2);
  }

  for (d = rule->declarations; d; d = d->next) {
    debug_indent(2);
    css_print_decl_value(d->name, &d->value);
    debug_indent(-2);
  }

  debug(DEBUG_INFO, "CSS", "rule end");
}

static void css_print(css_rule_t *ruleset) {
  css_rule_t *rule;

  for (rule = ruleset; rule; rule = rule->next) {
    css_print_rule(rule);
  }
}

static css_list_t *init_list(css_decl_value_t *value) {
  css_decl_value_t aux;
  css_list_t *list;

  list = &value->data.list;
  aux = *value;
  value->type = CSS_DECL_LIST;
  list->len = 1;
  list->size = 16;
  list->elements = sys_calloc(list->size, sizeof(css_list_element_t));
  list->elements[0].value = aux;

  return list;
}

int css_parse(char *buffer, int len, int full_stylesheet, html_env_t *env, void (*callback)(css_declaration_t *decl, void *data), void *data) {
  css_lexer_t *l;
  css_token_t t;
  css_rule_t *rule, *curr_rule;
  css_selector_t *curr_selector;
  css_filter_t *curr_filter;
  css_declaration_t *curr_decl;
  css_list_t *list;
  char separator, *last_ident;
  int prev, back, inlist, atrule, n, state = -1;

  if ((l = css_create(buffer, len)) != NULL) {
    rule = sys_calloc(1, sizeof(css_rule_t));
    curr_rule = rule;
    curr_rule->selectors = sys_calloc(1, sizeof(css_selector_t));
    curr_selector = curr_rule->selectors;
    atrule = 0;
    back = 0;
    last_ident = NULL;

    for (state = full_stylesheet ? PARSER_STATE_FILTER_START : PARSER_STATE_DECLARATIONS; state > 0;) {
      if (back) {
        back = 0;
      } else {
        css_next(l, &t);
      }

      if (t.id == TOKEN_ERROR) {
        debug(DEBUG_ERROR, "CSS", "lexer error at line %d", l->line);
        state = -1;
      } else if (t.id == TOKEN_EOF) {
        state = 0;
      } else {
        if (t.value) {
          debug(DEBUG_INFO, "CSS", "%s [%s] state %d, line %d", css_token_name(t.id), t.value, state, l->line);
        } else {
          debug(DEBUG_INFO, "CSS", "%s state %d, line %d", css_token_name(t.id), state, l->line);
        }
        switch (state) {
          case PARSER_STATE_FILTER_START:
            switch (t.id) {
              case TOKEN_SPACE:
                break;
              case TOKEN_AT:
                state = PARSER_STATE_AT_RULE;
                break;
              case TOKEN_CLOSEB:
                if (atrule) {
                  atrule = 0;
                } else {
                  state = syntax_error(l, &t, state);
                }
                break;
              case TOKEN_PERCENT:
                if (!atrule) {
                  state = syntax_error(l, &t, state);
                } else {
                  state = PARSER_STATE_FILTER_NEXT;
                }
                break;
              case TOKEN_CLASS:
                if (!atrule) {
                  curr_selector->group = sys_calloc(1, sizeof(css_filter_t));
                  curr_selector->group->type = CSS_FILTER_CLASS;
                  curr_selector->group->data.cclass = t.value;
                  curr_filter = curr_selector->group;
                }
                state = PARSER_STATE_FILTER_NEXT;
                break;
              case TOKEN_HASH:
                if (!atrule) {
                curr_selector->group = sys_calloc(1, sizeof(css_filter_t));
                curr_selector->group->type = CSS_FILTER_ID;
                curr_selector->group->data.id = t.value;
                curr_filter = curr_selector->group;
                }
                state = PARSER_STATE_FILTER_NEXT;
                break;
              case TOKEN_IDENT:
                if (!atrule) {
                curr_selector->group = sys_calloc(1, sizeof(css_filter_t));
                curr_selector->group->type = CSS_FILTER_TAG;
                curr_selector->group->data.tag = t.value;
                curr_filter = curr_selector->group;
                }
                state = PARSER_STATE_FILTER_NEXT;
                break;
              case TOKEN_PSEUDO:
                if (!atrule) {
                curr_selector->group = sys_calloc(1, sizeof(css_filter_t));
                curr_selector->group->type = CSS_FILTER_PSEUDO;
                curr_selector->group->data.tag = t.value;
                curr_filter = curr_selector->group;
                }
                state = PARSER_STATE_FILTER_NEXT;
                break;
              case TOKEN_COLON:
                prev = state;
                state = PARSER_STATE_FILTER_PSEUDO;
                break;
              case TOKEN_STAR:
                if (!atrule) {
                curr_selector->group = sys_calloc(1, sizeof(css_filter_t));
                curr_selector->group->type = CSS_FILTER_ANY;
                curr_filter = curr_selector->group;
                }
                state = PARSER_STATE_FILTER_NEXT;
                break;
              case TOKEN_OPENSB:
                if (!atrule) {
                curr_selector->group = sys_calloc(1, sizeof(css_filter_t));
                curr_selector->group->type = CSS_FILTER_ATTR;
                curr_filter = curr_selector->group;
                }
                state = PARSER_STATE_FILTER_ATTR;
                break;
              case TOKEN_OPENB:
                if (!atrule) {
                curr_rule->declarations = sys_calloc(1, sizeof(css_declaration_t));
                curr_decl = curr_rule->declarations;
                }
                state = PARSER_STATE_DECLARATIONS;
                break;
              default:
                state = syntax_error(l, &t, state);
                break;
            }
            break;
          case PARSER_STATE_FILTER_NEXT:
            switch (t.id) {
              case TOKEN_SPACE:
                if (!atrule) {
                curr_selector->next = sys_calloc(1, sizeof(css_selector_t));
                curr_selector = curr_selector->next;
                curr_selector->combinator = CSS_COMBINATOR_DESCENDANT;
                }
                state = PARSER_STATE_FILTER_START;
                break;
              case TOKEN_CHILD:
                if (!atrule) {
                curr_selector->next = sys_calloc(1, sizeof(css_selector_t));
                curr_selector = curr_selector->next;
                curr_selector->combinator = CSS_COMBINATOR_CHILD;
                }
                state = PARSER_STATE_FILTER_START;
                break;
              case TOKEN_PLUS:
                if (!atrule) {
                curr_selector->next = sys_calloc(1, sizeof(css_selector_t));
                curr_selector = curr_selector->next;
                curr_selector->combinator = CSS_COMBINATOR_ADJ_SIBLING;
                }
                state = PARSER_STATE_FILTER_START;
                break;
              case TOKEN_GSIBLING:
                if (!atrule) {
                curr_selector->next = sys_calloc(1, sizeof(css_selector_t));
                curr_selector = curr_selector->next;
                curr_selector->combinator = CSS_COMBINATOR_GEN_SIBLING;
                }
                state = PARSER_STATE_FILTER_START;
                break;
              case TOKEN_COMMA:
                if (!atrule) {
                curr_selector->next = sys_calloc(1, sizeof(css_selector_t));
                curr_selector = curr_selector->next;
                curr_selector->combinator = CSS_COMBINATOR_COMMA;
                }
                state = PARSER_STATE_FILTER_START;
                break;
              case TOKEN_CLASS:
                if (!atrule) {
                curr_filter->next = sys_calloc(1, sizeof(css_filter_t));
                curr_filter->next->type = CSS_FILTER_CLASS;
                curr_filter->next->data.cclass = t.value;
                curr_filter = curr_filter->next;
                }
                break;
              case TOKEN_HASH:
                if (!atrule) {
                curr_filter->next = sys_calloc(1, sizeof(css_filter_t));
                curr_filter->next->type = CSS_FILTER_ID;
                curr_filter->next->data.id = t.value;
                curr_filter = curr_filter->next;
                }
                break;
              case TOKEN_IDENT:
                if (!atrule) {
                curr_filter->next = sys_calloc(1, sizeof(css_filter_t));
                curr_filter->next->type = CSS_FILTER_TAG;
                curr_filter->next->data.tag = t.value;
                curr_filter = curr_filter->next;
                }
                break;
              case TOKEN_PSEUDO:
                if (!atrule) {
                curr_filter->next = sys_calloc(1, sizeof(css_filter_t));
                curr_filter->next->type = CSS_FILTER_PSEUDO;
                curr_filter->next->data.tag = t.value;
                curr_filter = curr_filter->next;
                }
                break;
              case TOKEN_COLON:
                prev = state;
                state = PARSER_STATE_FILTER_PSEUDO;
                break;
              case TOKEN_OPENSB:
                if (!atrule) {
                curr_filter->next = sys_calloc(1, sizeof(css_filter_t));
                curr_filter->next->type = CSS_FILTER_ATTR;
                curr_filter = curr_filter->next;
                }
                state = PARSER_STATE_FILTER_ATTR;
                break;
              case TOKEN_OPENB:
                if (!atrule) {
                curr_rule->declarations = sys_calloc(1, sizeof(css_declaration_t));
                curr_decl = curr_rule->declarations;
                }
                state = PARSER_STATE_DECLARATIONS;
                break;
              default:
                state = syntax_error(l, &t, state);
                break;
            }
            break;
          case PARSER_STATE_FILTER_PSEUDO:
            switch (t.id) {
              case TOKEN_COLON:
                state = PARSER_STATE_FILTER_PSEUDO2;
                break;
              case TOKEN_IDENT:
                t.id = TOKEN_PSEUDO;
                back = 1;
                state = prev;
                break;
              default:
                state = syntax_error(l, &t, state);
                break;
            }
            break;
          case PARSER_STATE_FILTER_PSEUDO2:
            switch (t.id) {
              case TOKEN_IDENT:
                t.id = TOKEN_PSEUDO;
                back = 1;
                state = prev;
                break;
              default:
                state = syntax_error(l, &t, state);
                break;
            }
            break;
          case PARSER_STATE_FILTER_ATTR:
            if (t.id == TOKEN_IDENT) {
              if (!atrule) curr_filter->data.attr.name = t.value;
              state = PARSER_STATE_FILTER_ATTR_OP;
            } else {
              state = syntax_error(l, &t, state);
            }
            break;
          case PARSER_STATE_FILTER_ATTR_OP:
            switch (t.id) {
              case TOKEN_EQUAL:
                if (!atrule) curr_filter->data.attr.op = CSS_ATTR_OP_EQUALS;
                state = PARSER_STATE_FILTER_ATTR_VALUE;
                break;
              case TOKEN_INCLUDES:
                if (!atrule) curr_filter->data.attr.op = CSS_ATTR_OP_INCLUDES;
                state = PARSER_STATE_FILTER_ATTR_VALUE;
                break;
              case TOKEN_DASHMATCH:
                if (!atrule) curr_filter->data.attr.op = CSS_ATTR_OP_DASHMATCH;
                state = PARSER_STATE_FILTER_ATTR_VALUE;
                break;
              case TOKEN_STARTSWITH:
                if (!atrule) curr_filter->data.attr.op = CSS_ATTR_OP_STARTS;
                state = PARSER_STATE_FILTER_ATTR_VALUE;
                break;
              case TOKEN_ENDSWITH:
                if (!atrule) curr_filter->data.attr.op = CSS_ATTR_OP_ENDS;
                state = PARSER_STATE_FILTER_ATTR_VALUE;
                break;
              case TOKEN_CONTAINS:
                if (!atrule) curr_filter->data.attr.op = CSS_ATTR_OP_CONTAINS;
                state = PARSER_STATE_FILTER_ATTR_VALUE;
                break;
              case TOKEN_CLOSESB:
                if (!atrule) curr_filter->data.attr.op = CSS_ATTR_OP_EXISTS;
                state = PARSER_STATE_FILTER_NEXT;
                break;
              default:
                state = syntax_error(l, &t, state);
                break;
            }
            break;
          case PARSER_STATE_FILTER_ATTR_VALUE:
            switch (t.id) {
              case TOKEN_IDENT:
              case TOKEN_STRING:
                if (!atrule) curr_filter->data.attr.value = t.value;
                state = PARSER_STATE_FILTER_ATTR_CLOSE;
                break;
              default:
                state = syntax_error(l, &t, state);
                break;
            }
            break;
          case PARSER_STATE_FILTER_ATTR_CLOSE:
            if (t.id == TOKEN_CLOSESB) {
              state = PARSER_STATE_FILTER_NEXT;
            } else {
              state = syntax_error(l, &t, state);
            }
            break;
          case PARSER_STATE_DECLARATIONS:
            switch (t.id) {
              case TOKEN_SPACE:
                break;
              case TOKEN_CLOSEB:
                if (!atrule) {
                curr_rule->next = sys_calloc(1, sizeof(css_rule_t));
                curr_rule = curr_rule->next;
                curr_rule->selectors = sys_calloc(1, sizeof(css_selector_t));
                curr_selector = curr_rule->selectors;
                }
                state = PARSER_STATE_FILTER_START;
                break;
              case TOKEN_IDENT:
                if (!atrule) curr_decl->name = t.value;
                state = PARSER_STATE_DECL_IDENT;
                break;
              default:
                state = syntax_error(l, &t, state);
            }
            break;
          case PARSER_STATE_DECL_IDENT:
            if (t.id == TOKEN_COLON) {
              list = NULL;
              inlist = 0;
              separator = 0;
              state = PARSER_STATE_DECL_VALUE;
            } else {
              state = syntax_error(l, &t, state);
            }
            break;
          case PARSER_STATE_DECL_VALUE:
            switch (t.id) {
              case TOKEN_SPACE:
                if (inlist) separator = ' ';
                break;
              case TOKEN_COMMA:
                separator = ',';
                break;
              case TOKEN_IDENT:
                last_ident = t.value;
                if (!atrule) {
                  if (!inlist) {
                    curr_decl->value.type = CSS_DECL_IDENT;
                    curr_decl->value.data.ident = t.value;
                  } else {
                    if (list == NULL) {
                      list = init_list(&curr_decl->value);
                    }
                    list->elements[list->len].value.type = CSS_DECL_IDENT;
                    list->elements[list->len].value.data.ident = t.value;
                    list->elements[list->len].separator = separator;
                    list->len++;
                  }
                  CALLBACK;
                }
                inlist = 1;
                break;
              case TOKEN_HASH:
                if (!atrule) {
                if (!inlist) {
                  curr_decl->value.type = CSS_DECL_COLOR;
                  parse_color(t.value, &curr_decl->value);
                } else {
                  if (list == NULL) {
                    list = init_list(&curr_decl->value);
                  }
                  list->elements[list->len].value.type = CSS_DECL_COLOR;
                  parse_color(t.value, &list->elements[list->len].value);
                  list->elements[list->len].separator = separator;
                  list->len++;
                }
                CALLBACK;
                }
                inlist = 1;
                break;
              case TOKEN_STRING:
                if (!atrule) {
                if (!inlist) {
                  curr_decl->value.type = CSS_DECL_STRING;
                  curr_decl->value.data.string = t.value;
                } else {
                  if (list == NULL) {
                    list = init_list(&curr_decl->value);
                  }
                  list->elements[list->len].value.type = CSS_DECL_STRING;
                  list->elements[list->len].value.data.string = t.value;
                  list->elements[list->len].separator = separator;
                  list->len++;
                }
                CALLBACK;
                }
                inlist = 1;
                break;
              PARSE_NUMBERS;
                break;
              case TOKEN_OPENP:
                if (last_ident && !sys_strcasecmp(last_ident, "var")) {
                  if (!atrule) curr_decl->value.type = CSS_DECL_VAR;
                  state = PARSER_STATE_DECL_VALUE_VAR;
                } else if (last_ident && !sys_strcasecmp(last_ident, "rgba")) {
                  if (!atrule) curr_decl->value.type = CSS_DECL_COLOR;
                  n = 0;
                  state = PARSER_STATE_DECL_VALUE_RGBA;
                } else {
                  if (!atrule) {
                  curr_decl->value.type = CSS_DECL_FUNCTION;
                  curr_decl->value.data.function.args.len = 0;
                  curr_decl->value.data.function.args.size = 16;
                  curr_decl->value.data.function.args.elements = sys_calloc(16, sizeof(css_list_element_t));
                  }
                  state = PARSER_STATE_DECL_VALUE_FUNCTION;
                }
                break;
              case TOKEN_EXCL:
                state = PARSER_STATE_IMPORTANT;
                break;
              case TOKEN_SCOLON:
                if (!atrule) {
                curr_decl->next = sys_calloc(1, sizeof(css_declaration_t));
                curr_decl = curr_decl->next;
                }
                state = PARSER_STATE_DECLARATIONS;
                break;
              case TOKEN_CLOSEB:
                if (!atrule) {
                curr_rule->next = sys_calloc(1, sizeof(css_rule_t));
                curr_rule = curr_rule->next;
                curr_rule->selectors = sys_calloc(1, sizeof(css_selector_t));
                curr_selector = curr_rule->selectors;
                }
                state = PARSER_STATE_FILTER_START;
                break;
              default:
                state = syntax_error(l, &t, state);
                break;
            }
            break;
          case PARSER_STATE_DECL_VALUE_VAR:
            if (t.id == TOKEN_IDENT) {
              curr_decl->value.data.var = t.value;
              state = PARSER_STATE_DECL_VALUE_VAR_CLOSE;
            } else {
              state = syntax_error(l, &t, state);
            }
            break;
          case PARSER_STATE_DECL_VALUE_VAR_CLOSE:
            if (t.id == TOKEN_CLOSEP) {
              state = PARSER_STATE_DECL_VALUE;
            } else {
              state = syntax_error(l, &t, state);
            }
            break;
          case PARSER_STATE_DECL_VALUE_RGBA:
            switch (t.id) {
              case TOKEN_SPACE:
              case TOKEN_COMMA:
                break;
              case TOKEN_NUMBER:
                if (!atrule) {
                switch (n) {
                  case 0:
                    curr_decl->value.data.color = sys_atoi(t.value) << 16;
                    break;
                  case 1:
                    curr_decl->value.data.color |= sys_atoi(t.value) << 8;
                    break;
                  case 2:
                    curr_decl->value.data.color |= sys_atoi(t.value);
                    break;
                  case 3:
                    curr_decl->value.data.color |= ((int)(sys_atof(t.value) * 256)) << 24;
                    debug(DEBUG_INFO, "CSS", "rgba color #%08X", curr_decl->value.data.color);
                    break;
                }
                }
                n++;
                break;
              case TOKEN_CLOSEP:
                if (n == 4) {
                  state = PARSER_STATE_DECL_VALUE;
                } else {
                  state = syntax_error(l, &t, state);
                }
                break;
              default:
                state = syntax_error(l, &t, state);
                break;
            }
            break;
          case PARSER_STATE_DECL_VALUE_FUNCTION:
            switch (t.id) {
              case TOKEN_SPACE:
              case TOKEN_COMMA:
              case TOKEN_EQUAL:
              case TOKEN_IDENT:
              case TOKEN_MINUS:
              case TOKEN_STAR:
              case TOKEN_SLASH:
              case TOKEN_HASH:
              CASE_NUMBER:
                break;
              case TOKEN_CLOSEP:
                state = PARSER_STATE_DECL_VALUE;
                break;
              default:
                state = syntax_error(l, &t, state);
                break;
            }
            break;
          case PARSER_STATE_IMPORTANT:
            if (t.id == TOKEN_IDENT && !sys_strcasecmp(t.value, "important")) {
              t.id = TOKEN_IMPORTANT;
              state = PARSER_STATE_IMPORTANT_END;
            } else {
              state = syntax_error(l, &t, state);
            }
            break;
          case PARSER_STATE_IMPORTANT_END:
            switch (t.id) {
              case TOKEN_SPACE:
                break;
              case TOKEN_SCOLON:
                if (!atrule) {
                curr_decl->next = sys_calloc(1, sizeof(css_declaration_t));
                curr_decl = curr_decl->next;
                }
                state = PARSER_STATE_DECLARATIONS;
                break;
              case TOKEN_CLOSEB:
                if (!atrule) {
                curr_rule->next = sys_calloc(1, sizeof(css_rule_t));
                curr_rule = curr_rule->next;
                curr_rule->selectors = sys_calloc(1, sizeof(css_selector_t));
                curr_selector = curr_rule->selectors;
                }
                state = PARSER_STATE_FILTER_START;
                break;
              default:
                state = syntax_error(l, &t, state);
                break;
            }
            break;
          case PARSER_STATE_AT_RULE:
            if (t.id == TOKEN_IDENT) {
              n = sys_strlen(t.value);
              if (!sys_strcasecmp(t.value, "media")) {
                state = PARSER_STATE_MEDIA;
              } else if (!sys_strcasecmp(t.value, "keyframes") || (n > 10 && !sys_strcmp(&t.value[n - 10], "-keyframes"))) {
                state = PARSER_STATE_KEYFRAMES;
              } else {
                state = syntax_error(l, &t, state);
              }
            } else {
              state = syntax_error(l, &t, state);
            }
            break;
          case PARSER_STATE_KEYFRAMES:
            if (t.id == TOKEN_IDENT) {
              state = PARSER_STATE_ATRULE_OPENB;
            } else if (t.id != TOKEN_SPACE) {
              state = syntax_error(l, &t, state);
            }
            break;
          case PARSER_STATE_MEDIA:
            if (t.id == TOKEN_IDENT) {
              state = PARSER_STATE_MEDIA_AND;
            } else if (t.id == TOKEN_OPENP) {
              state = PARSER_STATE_MEDIA_OPENP;
            } else if (t.id != TOKEN_SPACE) {
              state = syntax_error(l, &t, state);
            }
            break;
          case PARSER_STATE_MEDIA_AND:
            if (t.id == TOKEN_IDENT && !sys_strcasecmp(t.value, "and")) {
              state = PARSER_STATE_MEDIA_OPENP;
            } else if (t.id != TOKEN_SPACE) {
              state = syntax_error(l, &t, state);
            }
            break;
          case PARSER_STATE_MEDIA_OPENP:
            if (t.id == TOKEN_CLOSEP) {
              state = PARSER_STATE_ATRULE_OPENB;
            }
            break;
          case PARSER_STATE_ATRULE_OPENB:
            if (t.id == TOKEN_OPENB) {
              atrule = 1;
              state = PARSER_STATE_FILTER_START;
            } else if (t.id != TOKEN_SPACE) {
              state = syntax_error(l, &t, state);
            }
        }
      }
    }
    css_print(rule);
    css_destroy(l);
  }

  return state == 0 ? 0 : -1;
}
