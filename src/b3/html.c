#include "sys.h"
#include "script.h"
#include "debug.h"

#include "ht.h"
#include "dom.h"
#include "html.h"
#include "css.h"

/*
begin: <tag
begin_close: >
begin_short: />
end: </tag>
*/

#define MAX_CLASSNAME 64

typedef enum {
  TOKEN_IDENT = 0x1000,
  TOKEN_STRING,
  TOKEN_ASSIGN,
  TOKEN_TAG_BEGIN,
  TOKEN_TAG_BEGIN_CLOSE,
  TOKEN_TAG_BEGIN_SHORT,
  TOKEN_TAG_END,
  TOKEN_TEXT,
  TOKEN_DEF,

  TOKEN_ERROR = 0xff00,
  TOKEN_EOF = 0xffff
} html_token_e;

typedef struct html_token_t {
  html_token_e id;
  char *value;
} html_token_t;

typedef struct {
  char *buffer;
  int len;
  int pos;
  int state;
  int script;
  int string;
  int old;
  int closetag;
  char *text;
  uint32_t text_size;
  uint32_t text_pos;
  uint32_t line;
} html_lexer_t;

static char *stoken[] = {
  "IDENT",
  "STRING",
  "ASSGIN",
  "TAG_BEGIN",
  "TAG_BEGIN_CLOSE",
  "TAG_BEGIN_SHORT",
  "TAG_END",
  "TEXT",
  "DEF"
};

static char *auto_links[] = {
  "meta",
  "link",
  "img",
  "hr",
  "br",
  "input",
  "source",
  NULL
};

static tag_name_t tag_names[] = {
  { "a", TAG_A },
  { "abbr", TAG_ABBR },
  { "acronym", TAG_ACRONYM },
  { "address", TAG_ADDRESS },
  { "altglyph", TAG_ALTGLYPH },
  { "altglyphdef", TAG_ALTGLYPHDEF },
  { "altglyphitem", TAG_ALTGLYPHITEM },
  { "animatecolor", TAG_ANIMATECOLOR },
  { "animatemotion", TAG_ANIMATEMOTION },
  { "animatetransform", TAG_ANIMATETRANSFORM },
  { "annotation_xml", TAG_ANNOTATION_XML },
  { "applet", TAG_APPLET },
  { "area", TAG_AREA },
  { "article", TAG_ARTICLE },
  { "aside", TAG_ASIDE },
  { "audio", TAG_AUDIO },
  { "b", TAG_B },
  { "base", TAG_BASE },
  { "basefont", TAG_BASEFONT },
  { "bdi", TAG_BDI },
  { "bdo", TAG_BDO },
  { "bgsound", TAG_BGSOUND },
  { "big", TAG_BIG },
  { "blink", TAG_BLINK },
  { "blockquote", TAG_BLOCKQUOTE },
  { "body", TAG_BODY },
  { "br", TAG_BR },
  { "button", TAG_BUTTON },
  { "canvas", TAG_CANVAS },
  { "caption", TAG_CAPTION },
  { "center", TAG_CENTER },
  { "cite", TAG_CITE },
  { "clippath", TAG_CLIPPATH },
  { "code", TAG_CODE },
  { "col", TAG_COL },
  { "colgroup", TAG_COLGROUP },
  { "data", TAG_DATA },
  { "datalist", TAG_DATALIST },
  { "dd", TAG_DD },
  { "del", TAG_DEL },
  { "desc", TAG_DESC },
  { "details", TAG_DETAILS },
  { "dfn", TAG_DFN },
  { "dialog", TAG_DIALOG },
  { "dir", TAG_DIR },
  { "div", TAG_DIV },
  { "dl", TAG_DL },
  { "dt", TAG_DT },
  { "em", TAG_EM },
  { "embed", TAG_EMBED },
  { "feblend", TAG_FEBLEND },
  { "fecolormatrix", TAG_FECOLORMATRIX },
  { "fecomponenttransfer", TAG_FECOMPONENTTRANSFER },
  { "fecomposite", TAG_FECOMPOSITE },
  { "feconvolvematrix", TAG_FECONVOLVEMATRIX },
  { "fediffuselighting", TAG_FEDIFFUSELIGHTING },
  { "fedisplacementmap", TAG_FEDISPLACEMENTMAP },
  { "fedistantlight", TAG_FEDISTANTLIGHT },
  { "fedropshadow", TAG_FEDROPSHADOW },
  { "feflood", TAG_FEFLOOD },
  { "fefunca", TAG_FEFUNCA },
  { "fefuncb", TAG_FEFUNCB },
  { "fefuncg", TAG_FEFUNCG },
  { "fefuncr", TAG_FEFUNCR },
  { "fegaussianblur", TAG_FEGAUSSIANBLUR },
  { "feimage", TAG_FEIMAGE },
  { "femerge", TAG_FEMERGE },
  { "femergenode", TAG_FEMERGENODE },
  { "femorphology", TAG_FEMORPHOLOGY },
  { "feoffset", TAG_FEOFFSET },
  { "fepointlight", TAG_FEPOINTLIGHT },
  { "fespecularlighting", TAG_FESPECULARLIGHTING },
  { "fespotlight", TAG_FESPOTLIGHT },
  { "fetile", TAG_FETILE },
  { "feturbulence", TAG_FETURBULENCE },
  { "fieldset", TAG_FIELDSET },
  { "figcaption", TAG_FIGCAPTION },
  { "figure", TAG_FIGURE },
  { "font", TAG_FONT },
  { "footer", TAG_FOOTER },
  { "foreignobject", TAG_FOREIGNOBJECT },
  { "form", TAG_FORM },
  { "frame", TAG_FRAME },
  { "frameset", TAG_FRAMESET },
  { "glyphref", TAG_GLYPHREF },
  { "h1", TAG_H1 },
  { "h2", TAG_H2 },
  { "h3", TAG_H3 },
  { "h4", TAG_H4 },
  { "h5", TAG_H5 },
  { "h6", TAG_H6 },
  { "head", TAG_HEAD },
  { "header", TAG_HEADER },
  { "hgroup", TAG_HGROUP },
  { "hr", TAG_HR },
  { "html", TAG_HTML },
  { "i", TAG_I },
  { "iframe", TAG_IFRAME },
  { "image", TAG_IMAGE },
  { "img", TAG_IMG },
  { "input", TAG_INPUT },
  { "ins", TAG_INS },
  { "isindex", TAG_ISINDEX },
  { "kbd", TAG_KBD },
  { "keygen", TAG_KEYGEN },
  { "label", TAG_LABEL },
  { "legend", TAG_LEGEND },
  { "li", TAG_LI },
  { "lineargradient", TAG_LINEARGRADIENT },
  { "link", TAG_LINK },
  { "listing", TAG_LISTING },
  { "main", TAG_MAIN },
  { "malignmark", TAG_MALIGNMARK },
  { "map", TAG_MAP },
  { "mark", TAG_MARK },
  { "marquee", TAG_MARQUEE },
  { "math", TAG_MATH },
  { "menu", TAG_MENU },
  { "meta", TAG_META },
  { "meter", TAG_METER },
  { "mfenced", TAG_MFENCED },
  { "mglyph", TAG_MGLYPH },
  { "mi", TAG_MI },
  { "mn", TAG_MN },
  { "mo", TAG_MO },
  { "ms", TAG_MS },
  { "mtext", TAG_MTEXT },
  { "multicol", TAG_MULTICOL },
  { "nav", TAG_NAV },
  { "nextid", TAG_NEXTID },
  { "nobr", TAG_NOBR },
  { "noembed", TAG_NOEMBED },
  { "noframes", TAG_NOFRAMES },
  { "noscript", TAG_NOSCRIPT },
  { "object", TAG_OBJECT },
  { "ol", TAG_OL },
  { "optgroup", TAG_OPTGROUP },
  { "option", TAG_OPTION },
  { "output", TAG_OUTPUT },
  { "p", TAG_P },
  { "param", TAG_PARAM },
  { "path", TAG_PATH },
  { "picture", TAG_PICTURE },
  { "plaintext", TAG_PLAINTEXT },
  { "pre", TAG_PRE },
  { "progress", TAG_PROGRESS },
  { "q", TAG_Q },
  { "radialgradient", TAG_RADIALGRADIENT },
  { "rb", TAG_RB },
  { "rp", TAG_RP },
  { "rt", TAG_RT },
  { "rtc", TAG_RTC },
  { "ruby", TAG_RUBY },
  { "s", TAG_S },
  { "samp", TAG_SAMP },
  { "script", TAG_SCRIPT },
  { "section", TAG_SECTION },
  { "select", TAG_SELECT },
  { "slot", TAG_SLOT },
  { "small", TAG_SMALL },
  { "source", TAG_SOURCE },
  { "spacer", TAG_SPACER },
  { "span", TAG_SPAN },
  { "strike", TAG_STRIKE },
  { "strong", TAG_STRONG },
  { "style", TAG_STYLE },
  { "sub", TAG_SUB },
  { "summary", TAG_SUMMARY },
  { "sup", TAG_SUP },
  { "svg", TAG_SVG },
  { "table", TAG_TABLE },
  { "tbody", TAG_TBODY },
  { "td", TAG_TD },
  { "template", TAG_TEMPLATE },
  { "textarea", TAG_TEXTAREA },
  { "textpath", TAG_TEXTPATH },
  { "tfoot", TAG_TFOOT },
  { "th", TAG_TH },
  { "thead", TAG_THEAD },
  { "time", TAG_TIME },
  { "title", TAG_TITLE },
  { "tr", TAG_TR },
  { "track", TAG_TRACK },
  { "tt", TAG_TT },
  { "u", TAG_U },
  { "ul", TAG_UL },
  { "var", TAG_VAR },
  { "video", TAG_VIDEO },
  { "wbr", TAG_WBR },
  { "xmp", TAG_XMP },
  { NULL, 0 }
};

static char *html_token_name(html_token_e id) {
  return id ? stoken[id - 0x1000] : NULL;
}

static html_lexer_t *html_init(char *buffer, int len) {
  html_lexer_t *l;

  if ((l = sys_calloc(1, sizeof(html_lexer_t))) != NULL) {
    l->buffer = buffer;
    l->len = len;
    l->line = 1;
  }

  return l;
}

static void html_destroy(html_lexer_t *l) {
  if (l) {
    sys_free(l);
  }
}

static uint16_t tok(html_lexer_t *l, uint16_t token, html_token_t *t) {
  t->id = token;
  t->value = NULL;

  switch (token) {
    case TOKEN_IDENT:
    case TOKEN_STRING:
    case TOKEN_TAG_BEGIN:
    case TOKEN_TAG_END:
    case TOKEN_TEXT:
    case TOKEN_DEF:
      t->value = sys_strdup(l->text);
      break;
    case TOKEN_ERROR:
      debug(DEBUG_ERROR, "HTML", "lexer error at line %d (state %d)", l->line, l->state);
      break;
  }

  return token;
}

static int next(html_lexer_t *l) {
  if (l->pos == l->len) return 0;
  return l->buffer[l->pos++];
}

static void back(html_lexer_t *l) {
  if (l->pos) l->pos--;
}

static uint16_t html_next(html_lexer_t *l, html_token_t *t) {
  char c;

  for (;;) {
    c = next(l);
    if (c == -1) return tok(l, TOKEN_ERROR, t);
    if (c == 0) return tok(l, TOKEN_EOF, t);

    if (!l->text) {
      l->text_size = 1024;
      l->text = sys_calloc(1, l->text_size);
    } else if (l->text_pos == l->text_size - 1) {
      l->text_size += 1024;
      l->text = sys_realloc(l->text, l->text_size);
    }

    debug(DEBUG_TRACE, "HTML", "char [%c] state %d", c, l->state);

    switch (l->state) {
      case 0: // initial
        switch (c) {
          case ' ':
          case '\t':
          case '\n':
          case '\r':
            if (l->text_pos > 0 && l->text[l->text_pos - 1] != ' ') {
              l->text[l->text_pos++] = ' ';
            }
            break;
          case '<':
            if (l->script) {
              l->text[l->text_pos++] = c;
              l->state = 31;
            } else {
              l->state = 2;
              if (l->text_pos > 0) {
                l->text[l->text_pos] = 0;
                l->text_pos = 0;
                return tok(l, TOKEN_TEXT, t);
              }
            }
            break;
          default:
            if (l->script) {
              switch (c) {
                case '"':
                  l->string = 1;
                  l->state = 21;
                  break;
                case '\'':
                  l->string = 1;
                  l->state = 22;
                  break;
                case '/':
                  l->state = 23;
                  break;
              }
            }
            l->text[l->text_pos++] = c;
            break;
        }
        break;
      case 2: // <
        switch (c) {
          case '/':
            l->state = 3;
            break;
          case '!':
            l->state = 41;
            break;
          default:
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
              l->text[l->text_pos++] = c;
              l->state = 4;
            } else {
              return tok(l, TOKEN_ERROR, t);
            }
        }
        break;
      case 3: // </
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
              l->text[l->text_pos++] = c;
              l->state = 5;
            } else {
              return tok(l, TOKEN_ERROR, t);
            }
            break;
      case 4: // <a
            switch (c) {
              case '>':
                back(l);
                l->state = 6;
                l->text[l->text_pos] = 0;
                l->text_pos = 0;
                l->script = !sys_strcasecmp(l->text, "script");
                return tok(l, TOKEN_TAG_BEGIN, t);
              case '/':
                l->state = 7;
                l->text[l->text_pos] = 0;
                l->text_pos = 0;
                return tok(l, TOKEN_TAG_BEGIN, t);
              case '\n': l->line++; // fall through
              case ' ':
              case '\t':
              case '\r':
                l->state = 8;
                l->text[l->text_pos] = 0;
                l->text_pos = 0;
                l->script = !sys_strcasecmp(l->text, "script");
                return tok(l, TOKEN_TAG_BEGIN, t);
              default:
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
                  l->text[l->text_pos++] = c;
                } else {
                  return tok(l, TOKEN_ERROR, t);
                }
            }
            break;
      case 5: // </a
            if (c == '>') {
              l->state = 0;
              l->text[l->text_pos] = 0;
              l->text_pos = 0;
              return tok(l, TOKEN_TAG_END, t);
            } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
              l->text[l->text_pos++] = c;
            }
            break;
      case 6:
            if (c == '>') {
              l->state = 0;
              l->text[l->text_pos] = 0;
              l->text_pos = 0;
              return tok(l, TOKEN_TAG_BEGIN_CLOSE, t);
            } else {
              return tok(l, TOKEN_ERROR, t);
            }
            break;
      case 7:
            if (c == '>') {
              l->state = 0;
              l->text[l->text_pos] = 0;
              l->text_pos = 0;
              return tok(l, TOKEN_TAG_BEGIN_SHORT, t);
            } else {
              return tok(l, TOKEN_ERROR, t);
            }
            break;
      case 8: // "<abc "
            switch (c) {
              case '>':
                back(l);
                l->state = 6;
                l->text[l->text_pos] = 0;
                l->text_pos = 0;
                break;
              case '/':
                l->state = 7;
                l->text[l->text_pos] = 0;
                l->text_pos = 0;
                break;
              case '\n': l->line++; // fall through
              case ' ':
              case '\t':
              case '\r':
                break;
              default:
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
                  l->state = 9;
                  l->text[l->text_pos++] = c;
                } else {
                  return tok(l, TOKEN_ERROR, t);
                }
            }
            break;
      case 9: // <abc x
            switch (c) {
              case '\n': l->line++; // fall through
              case ' ':
              case '\t':
              case '\r':
                l->state = 8;
                l->text[l->text_pos] = 0;
                l->text_pos = 0;
                return tok(l, TOKEN_IDENT, t);
              case '=':
                back(l);
                l->state = 14;
                l->text[l->text_pos] = 0;
                l->text_pos = 0;
                return tok(l, TOKEN_IDENT, t);
              default:
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_') {
                  l->text[l->text_pos++] = c;
                } else {
                  back(l);
                  l->state = 8;
                  l->text[l->text_pos] = 0;
                  l->text_pos = 0;
                  return tok(l, TOKEN_IDENT, t);
                }
            }
            break;
      case 10: // <abc x=
            switch (c) {
              case '"':
                l->string = 1;
                l->state = 11;
                break;
              case '\'':
                l->string = 1;
                l->state = 12;
                break;
              default:
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                  l->text[l->text_pos++] = c;
                  l->state = 13;
                }
                break;
            }
            break;
      case 11: // <abc x="
            if (c == '"') {
              l->state = 8;
              l->text[l->text_pos] = 0;
              l->text_pos = 0;
              return tok(l, TOKEN_STRING, t);
            } else {
              l->text[l->text_pos++] = c;
            }
            break;
      case 12: // <abc x='
            if (c == '\'') {
              l->state = 8;
              l->text[l->text_pos] = 0;
              l->text_pos = 0;
              return tok(l, TOKEN_STRING, t);
            } else {
              l->text[l->text_pos++] = c;
            }
            break;
      case 13: // <abc x=a
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_') {
              l->text[l->text_pos++] = c;
            } else {
              back(l);
              l->state = 8;
              if (l->text_pos > 0) {
                l->text[l->text_pos] = 0;
                l->text_pos = 0;
                return tok(l, TOKEN_IDENT, t);
              }
            }
            break;
      case 14:
        if (c == '=') {
          l->state = 10;
          return tok(l, TOKEN_ASSIGN, t);
        } else {
          return tok(l, TOKEN_ERROR, t);
        }
        break;
      case 21:
        switch (c) {
          case '"':
            l->string--;
            if (l->string == 0) {
              l->state = 0;
            } else {
              l->state = 22;
            }
            break;
          case '\'':
            if (l->string == 1) {
              l->string = 2;
              l->state = 22;
            }
            break;
          case '\\':
            l->old = l->state;
            l->state = 27;
            break;
        }
        l->text[l->text_pos++] = c;
        break;
      case 22:
        switch (c) {
          case '\'':
            l->string--;
            if (l->string == 0) {
              l->state = 0;
            } else {
              l->state = 21;
            }
            break;
          case '"':
            if (l->string == 1) {
              l->string = 2;
              l->state = 21;
            }
            break;
          case '\\':
            l->old = l->state;
            l->state = 27;
            break;
        }
        l->text[l->text_pos++] = c;
        break;
      case 23:
        switch (c) {
          case '/': l->state = 24; break;
          case '*': l->state = 25; break;
          default: l->state = 0; break;
        }
        l->text[l->text_pos++] = c;
        break;
      case 24:
        if (c == '\n') l->state = 0;
        l->text[l->text_pos++] = c;
        break;
      case 25:
        if (c == '*') l->state = 26;
        l->text[l->text_pos++] = c;
        break;
      case 26:
        switch (c) {
          case '\n': l->state = 25; break;
          case '/': l->state = 0; break;
        }
        l->text[l->text_pos++] = c;
        break;
      case 27:
        l->text[l->text_pos++] = c;
        l->state = l->old;
        break;
      case 31:
        if (c == '/' && l->text_pos > 0 && l->text[l->text_pos - 1] == '<') {
          l->script = 0;
          l->state = 3;
          l->text_pos--;
          if (l->text_pos > 0) {
            l->text[l->text_pos] = 0;
            l->text_pos = 0;
            return tok(l, TOKEN_TEXT, t);
          }
        } else {
          l->text[l->text_pos++] = c;
          l->state = 0;
        }
        break;
      case 41: // <!
        if (c == '-') {
          l->state = 42;
        } else {
          l->text[l->text_pos++] = c;
          l->state = 51;
        }
        break;
      case 42: // <!-
        if (c == '-') {
          l->state = 43;
        } else {
          return tok(l, TOKEN_ERROR, t);
        }
        break;
      case 43: // inside comment
        if (c == '-') l->state = 44;
        break;
      case 44: // -
        switch (c) {
          case '-': l->state = 45; break;
          default: l->state = 43; break;
        }
        break;
      case 45: // --
        switch (c) {
          case '>': l->state = 0; break;
          default: l->state = 43; break;
        }
        break;
      case 51:
        if (c == '>') {
          l->state = 0;
          if (l->text_pos > 0) {
            l->text[l->text_pos] = 0;
            l->text_pos = 0;
            return tok(l, TOKEN_DEF, t);
          }
        } else {
          l->text[l->text_pos++] = c;
        }
        break;
    }
    if (c == '\n') l->line++;
  }

  return tok(l, TOKEN_ERROR, t);
}

static int syntax_error(html_lexer_t *l, html_token_t *t, int state) {
  debug(DEBUG_ERROR, "HTML", "invalid token %s at line %d (state %d)", html_token_name(t->id), l->line, state);
  return -1;
}

static void print_tree(tree_t *node) {
  tree_t *child;
  hti i;

  switch (node->type) {
    case DOM_DOCUMENT:
      debug(DEBUG_INFO, "HTML", "document begin");
      debug_indent(2);
      print_tree(node->children);
      debug_indent(-2);
      debug(DEBUG_INFO, "HTML", "document end");
      break;
    case DOM_TAG:
      debug(DEBUG_INFO, "HTML", "tag [%s] begin", node->name);
      debug_indent(2);
      i = ht_iterator(node->attributes);
      while (ht_next(&i)) {
        debug(DEBUG_INFO, "HTML", "attr %s=\"%s\"", i.key, i.value);
      }
      for (child = node->children; child; child = child->next) {
        print_tree(child);
      }
      debug_indent(-2);
      debug(DEBUG_INFO, "HTML", "tag [%s] end", node->name);
      break;
    case DOM_TEXT:
      debug_indent(2);
      debug(DEBUG_INFO, "HTML", "text [%s]", node->name);
      debug_indent(-2);
      break;
  }
}

static void lower(char *s) {
  int i;

  for (i = 0; s[i]; i++) {
    s[i] = sys_tolower(s[i]);
  }
}

static void css_style_callback(css_declaration_t *decl, void *data) {
  tree_t *node = (tree_t *)data;

  debug(DEBUG_INFO, "HTML", "css style [%s] in [%s]", decl->name, node->name);
}

static void parse_classes(char *classes, tree_t *node) {
  char classname[MAX_CLASSNAME];
  int i, j, s;
  char c;

  s = 0;
  j = 0;
  for (i = 0; classes[i]; i++) {
    c = classes[i];
    switch (s) {
      case 0:
        if (c != ' ') {
          classname[j++] = c;
          s = 1;
        }
        break;
      case 1:
        if (c == ' ') {
          if (j) {
            classname[j] = 0;
            debug(DEBUG_INFO, "HTML", "css class [%s] in [%s]", classname, node->name);
            if (node->classes == NULL) {
              node->classes = sys_calloc(1, sizeof(node_class_t));
              node->classes->name = sys_strdup(classname);
              node->last_class = node->classes;
            } else {
              node->classes->next = sys_calloc(1, sizeof(node_class_t));
              node->classes->next->name = sys_strdup(classname);
              node->last_class = node->classes->next;
            }
            j = 0;
          }
          s = 0;
        } else if (j < MAX_CLASSNAME-1) {
          classname[j++] = c;
        }
        break;
    }
  }
  if (j) {
    classname[j] = 0;
    debug(DEBUG_INFO, "HTML", "css class [%s] in [%s]", classname, node->name);
    if (node->classes == NULL) {
      node->classes = sys_calloc(1, sizeof(node_class_t));
      node->classes->name = sys_strdup(classname);
      node->last_class = node->classes;
    } else {
      node->classes->next = sys_calloc(1, sizeof(node_class_t));
      node->classes->next->name = sys_strdup(classname);
      node->last_class = node->classes->next;
    }
  }
}

static void build_remote_url(html_env_t *env, int https, char *page, char *path, uint32_t id) {
  char *url, *protocol, *host, *p;
  int len;

  protocol = https ? "https://" : "http://";
  len = sys_strlen(protocol);
  host = page;
  p = sys_strchr(host, '/');
  if (p == NULL) {
    if (path[0] == '/') {
      url = sys_calloc(len + sys_strlen(host) + sys_strlen(path) + 1, 1);
      sys_strcpy(url, protocol);
      sys_strcat(url, host);
    } else {
      url = sys_calloc(len + sys_strlen(host) + 1 + sys_strlen(path) + 1, 1);
      sys_strcpy(url, protocol);
      sys_strcat(url, host);
      sys_strcat(url, "/");
    }
    sys_strcat(url, path);
  } else  {
    if (path[0] == '/') {
      url = sys_calloc(len + p - host + sys_strlen(path) + 1, 1);
      sys_strcpy(url, protocol);
      sys_strncpy(&url[len], host, p - host);
    } else {
      url = sys_calloc(len + p - host + 1 + sys_strlen(path) + 1, 1);
      sys_strcpy(url, protocol);
      sys_strncpy(&url[len], host, p - host + 1);
    }
    sys_strcat(url, path);
  }

  env->fetch(url, id);
  sys_free(url);
}

static void build_local_url(html_env_t *env, char *page, char *path, uint32_t id) {
  char *url;
  int i;

  for (i = sys_strlen(page) - 1; i >= 0; i--) {
    if (page[i] == '/') break;
  }

  if (page[i] == '/') {
    if (path[0] == '/') {
      url = sys_calloc(i + sys_strlen(path) + 1, 1);
      sys_strncpy(url, page, i);
    } else {
      url = sys_calloc(i + 1 + sys_strlen(path) + 1, 1);
      sys_strncpy(url, page, i + 1);
    }
    sys_strcat(url, path);
  } else {
    i = sys_strlen(page);
    if (path[0] == '/') {
      url = sys_calloc(i + sys_strlen(path) + 1, 1);
      sys_strncpy(url, page, i);
      sys_strcat(url, path);
      env->fetch(url, id);
      sys_free(url);
    } else {
      env->fetch(path, id);
    }
  }
}

static void fetch(html_env_t *env, char *page_url, char *url, uint32_t id) {
  if (!sys_strncmp(url, "http://", 7) || !sys_strncmp(url, "https://", 8)) {
    // url is remote (http or https)
    env->fetch(url, id);
  } else if (!sys_strncmp(page_url, "http://", 7)) {
    // url is relative, page url is remote (http)
    build_remote_url(env, 0, page_url+7, url, id);
  } else if (!sys_strncmp(page_url, "https://", 8)) {
    // url is relative, page url is remote (https)
    build_remote_url(env, 1, page_url+8, url, id);
  } else {
    // url is relative, page url is local
    build_local_url(env, page_url, url, id);
  }
}

static void check_external_refs(html_env_t *env, char *url, tree_t *current) {
  char *href, *rel, *src;

  if (current->tag == TAG_STYLE && current->nchildren == 1 && current->children->type == DOM_TEXT) {
    debug(DEBUG_INFO, "HTML", "parsing css style");
    css_parse(current->children->name, sys_strlen(current->children->name), 1, env, NULL, NULL);
  } else if (current->tag == TAG_LINK) {
    if ((href = ht_get(current->attributes, "href")) != NULL &&
        (rel = ht_get(current->attributes, "rel")) != NULL && !sys_strcasecmp(rel, "stylesheet")) {
      if (href[0]) fetch(env, url, href, 0);
    }
  } else if (current->tag == TAG_SCRIPT && env->pe > 0) {
    if ((src = ht_get(current->attributes, "src")) != NULL) {
      if (src[0]) fetch(env, url, src, 0);
    } else if (current->nchildren == 1 && current->children->type == DOM_TEXT) {
      debug(DEBUG_INFO, "HTML", "running inline script");
      if (script_run(env->pe, current->children->name, 0, NULL, 1) == -1) {
        debug(DEBUG_ERROR, "HTML", "script failed");
      }
    }
  } else if (current->tag == TAG_IMG) {
    if ((src = ht_get(current->attributes, "src")) != NULL) {
      if (src[0]) {
        current->id = ++env->nextid;
        fetch(env, url, src, current->id);
      }
    }
  }
}

tree_t *html_parse(tree_t *root, html_env_t *env, char *url, char *buffer, int len) {
  html_lexer_t *l;
  html_token_t token;
  tree_t *node, *current;
  tag_name_t *tag;
  ht *auto_link;
  char *attr_name;
  int i, state = -1;

  if ((l = html_init(buffer, len)) != NULL) {
    current = root;

    env->tagnames = ht_create();
    for (i = 0; tag_names[i].name; i++) {
      ht_set(env->tagnames, tag_names[i].name, &tag_names[i]);
    }

    auto_link = ht_create();
    for (i = 0; auto_links[i]; i++) {
      ht_set(auto_link, auto_links[i], "");
    }

    for (state = 1; state > 0;) {
      html_next(l, &token);

      if (token.id == TOKEN_ERROR) {
        state = -1;
        break;
      } else if (token.id == TOKEN_EOF) {
        state = 0;
        break;
      }

      if (token.value) {
        debug(DEBUG_TRACE, "HTML", "%s [%s] (line %d)", html_token_name(token.id), token.value, l->line);
      } else {
        debug(DEBUG_TRACE, "HTML", "%s (line %d)", html_token_name(token.id), l->line);
      }

      switch (state) {
        case 1:
          switch (token.id) {
            case TOKEN_DEF:
              break;
            case TOKEN_TAG_BEGIN:
              debug(DEBUG_TRACE, "HTML", "begin tag [%s]", token.value);
              node = sys_calloc(1, sizeof(tree_t));
              node->type = DOM_TAG;
              lower(token.value);
              tag = ht_get(env->tagnames, token.value);
              if (tag) {
                node->tag = tag->id;
              } else {
                debug(DEBUG_ERROR, "HTML", "tag %s not found", token.value);
                node->tag = TAG_UNKNOWN;
              }
              node->name = token.value;
              node->attributes = ht_create();
              node->style = ht_create();
              node->root = root;
              node->parent = current;
              if (node->parent->children == NULL) {
                node->parent->children = node;
                node->parent->last = node;
                node->parent->nchildren = 1;
              } else {
                node->parent->last->next = node;
                node->parent->last = node;
                node->parent->nchildren++;
              }
              current = node;
              state = 2;
              break;
            case TOKEN_TAG_END:
              debug(DEBUG_TRACE, "HTML", "end tag [%s]", token.value);
              check_external_refs(env, url, current);
              if (sys_strcasecmp(token.value, current->name)) {
                debug(DEBUG_ERROR, "HTML", "current [%s] is not [%s]", current->name, token.value);
                state = syntax_error(l, &token, state);
              } else {
                current = current->parent;
              }
              break;
            case TOKEN_TEXT:
              if (current) {
                debug(DEBUG_TRACE, "HTML", "text [%s]", token.value);
                node = sys_calloc(1, sizeof(tree_t));
                node->type = DOM_TEXT;
                node->name = token.value;
                node->root = root;
                node->parent = current;
                if (node->parent->children == NULL) {
                  node->parent->children = node;
                  node->parent->last = node;
                  node->parent->nchildren = 1;
                } else {
                  node->parent->last->next = node;
                  node->parent->last = node;
                  node->parent->nchildren++;
                }
              } else {
                state = syntax_error(l, &token, state);
              }
              break;
            default:
              state = syntax_error(l, &token, state);
              break;
          }
          break;
        case 2:
          switch (token.id) {
            case TOKEN_IDENT:
              attr_name = token.value;
              lower(attr_name);
              state = 3;
              break;
            case TOKEN_TAG_BEGIN_CLOSE:
              if (ht_get(auto_link, current->name)) {
                current = current->parent;
              }
              state = 1;
              break;
            case TOKEN_TAG_BEGIN_SHORT:
              debug(DEBUG_TRACE, "HTML", "end tag short");
              check_external_refs(env, url, current);
              current = current->parent;
              state = 1;
              break;
            default:
              state = syntax_error(l, &token, state);
              break;
          }
          break;
        case 3:
          switch (token.id) {
            case TOKEN_ASSIGN:
              state = 4;
              break;
            case TOKEN_IDENT:
              debug(DEBUG_TRACE, "HTML", "attr [%s]", attr_name);
              ht_set(current->attributes, attr_name, "");
              attr_name = token.value;
              lower(attr_name);
              break;
            case TOKEN_TAG_BEGIN_CLOSE:
              if (attr_name) {
                debug(DEBUG_TRACE, "HTML", "attr [%s]", attr_name);
                ht_set(current->attributes, attr_name, "");
                attr_name = NULL;
              }
              if (ht_get(auto_link, current->name)) {
                current = current->parent;
              }
              state = 1;
              break;
            case TOKEN_TAG_BEGIN_SHORT:
              if (attr_name) {
                debug(DEBUG_TRACE, "HTML", "attr [%s]", attr_name);
                ht_set(current->attributes, attr_name, "");
                attr_name = NULL;
              }
              debug(DEBUG_TRACE, "HTML", "end tag short");
              check_external_refs(env, url, current);
              current = current->parent;
              state = 1;
              break;
            default:
              state = syntax_error(l, &token, state);
              break;
          }
          break;
        case 4:
          switch (token.id) {
            case TOKEN_IDENT:
            case TOKEN_STRING:
              debug(DEBUG_TRACE, "HTML", "attr [%s] = [%s]", attr_name, token.value);
              ht_set(current->attributes, attr_name, token.value);
              if (!sys_strcmp(attr_name, "style")) {
                css_parse(token.value, sys_strlen(token.value), 0, NULL, css_style_callback, current);
              }
              if (!sys_strcmp(attr_name, "class")) {
                debug(DEBUG_INFO, "HTML", "css classes [%s] in [%s]", token.value, node->name);
                parse_classes(token.value, node);
              }
              attr_name = NULL;
              state = 2;
              break;
            default:
              state = syntax_error(l, &token, state);
              break;
          }
          break;
      }
    }
    html_destroy(l);
  }

  print_tree(root);
  ht_destroy(auto_link);

  return state == 0 ? root : NULL;
}
