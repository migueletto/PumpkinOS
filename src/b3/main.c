#include <PalmOS.h>
#include <VFSMgr.h>

#include "ColorTable.h"

#include "sys.h"
#include "thread.h"
#include "mutex.h"
#include "script.h"
#include "ptr.h"
#include "pumpkin.h"
#include "surface.h"
#include "fontstd.h"
#include "fontbold.h"
#include "palette.h"
#include "renderer.h"
#include "filter.h"
#include "secure.h"
#include "httpc.h"
#include "ht.h"
#include "dom.h"
#include "html.h"
#include "css.h"
#include "util.h"
#include "resource.h"
#include "debug.h"

#define ESCAPE '['
#define Y0 60

#define ERROR_DIALOG 10021
#define INFO_DIALOG  10024

#define HTTP_TIMEOUT 10
#define MAX_THREADS  3

typedef enum {
  CMD_NULL,
  CMD_LINK,
  CMD_ROW,
  CMD_IMG
} block_cmd_t;

typedef struct {
  mu_Font font;
  mu_Color fg;
  mu_Color bg;
  int margin;
  int padding;
} block_style_t;

typedef struct block_t {
  block_cmd_t cmd;
  int count;
  char *content;
  char *arg;
  uint32_t len;
  int width, height;
  block_style_t style;
  void *data;
  struct block_t *next;
} block_t;

typedef struct response_t {
  uint32_t seq;
  uint32_t id;
  char *url;
  int16_t parse_response;
  uint32_t response_size;
  char *response;
  struct response_t *next;
} response_t;

static mutex_t *mutex;
static int task;
static UInt32 screen_width, screen_height;
static surface_t *surface;
static mu_Context *ctx;
static int refresh;
static void (*old_setpixel)(void *data, int x, int y, uint32_t color);
static mu_Rect clip = { 0, 0, 99999, 99999 };

static response_t *resp_queue, *resp_last;
static tree_t *dom;
static block_t *render_tree;

static html_env_t env;

static mu_Style style = {
  // font | size | padding | spacing | indent
  0, { 68, 10 }, 5, 0, 24,
  // title_height | scrollbar_size | thumb_size
  24, 12, 8,
  {
    { 230, 230, 230, 255 }, // MU_COLOR_TEXT
    { 25,  25,  25,  255 }, // MU_COLOR_BORDER
    { 50,  50,  50,  255 }, // MU_COLOR_WINDOWBG
    { 25,  25,  25,  255 }, // MU_COLOR_TITLEBG
    { 240, 240, 240, 255 }, // MU_COLOR_TITLETEXT
    { 0,   0,   0,   0   }, // MU_COLOR_PANELBG
    { 75,  75,  75,  255 }, // MU_COLOR_BUTTON
    { 95,  95,  95,  255 }, // MU_COLOR_BUTTONHOVER
    { 115, 115, 115, 255 }, // MU_COLOR_BUTTONFOCUS
    { 30,  30,  30,  255 }, // MU_COLOR_BASE
    { 35,  35,  35,  255 }, // MU_COLOR_BASEHOVER
    { 40,  40,  40,  255 }, // MU_COLOR_BASEFOCUS
    { 43,  43,  43,  255 }, // MU_COLOR_SCROLLBASE
    { 30,  30,  30,  255 }, // MU_COLOR_SCROLLTHUMB
    { 35,  35,  205, 255 }, // MU_COLOR_LINK
    { 205, 35,  205, 255 }  // MU_COLOR_LINKHOVER
  }
};

void r_draw_rect(mu_Rect r, mu_Color c) {
  uint32_t color = surface_color_rgb(surface->encoding, NULL, 0, c.r, c.g, c.b, 0xff);
  surface_rectangle(surface, r.x, r.y, r.x + r.w - 1, r.y + r.h - 1, 1, color);
}

static void set_font(mu_Font font, font_t **f, int *fnt) {
  switch (font) {
    case 0: // standard
      *f = &fontstd;
      *fnt = 0;
      break;
    case 1: // bold
      *f = &fontbold;
      *fnt = 0;
      break;
    case 2: // monospaced
      *f = NULL;
      *fnt = 5;
      break;
    default:
      *f = &fontstd;
      *fnt = 0;
      break;
  }
}

static int r_get_text_width(mu_Font font, char *text, int len) {
  int i, fnt, width = 0;
  font_t *f;

  set_font(font, &f, &fnt);
  if (len == -1) len = sys_strlen(text);

  for (i = 0; i < len; i++) {
    if (text[i] == ESCAPE) {
      switch (text[i+1]) {
        case 'f':
          font = text[i+2] - '0';
          set_font(font, &f, &fnt);
          break;
      }
      i += 2;
      continue;
    }
    width += surface_font_char_width(f, fnt, text[i]);
  }

  return width;
}

static int r_get_text_height(void) {
  return surface_font_height(&fontstd, 0);
}

void r_draw_text(char *text, mu_Vec2 pos, mu_Font font, mu_Color fg, mu_Color bg) {
  uint32_t fg_color, bg_color;
  int i, fnt, x, y, d;
  char *s;
  font_t *f;
  mu_Color black = { 0x00, 0x00, 0x00, 0xff };
  mu_Color blue  = { 0x00, 0x00, 0xff, 0xff };
  mu_Color red   = { 0xff, 0x00, 0x00, 0xff };

  fg_color = surface_color_rgb(surface->encoding, NULL, 0, fg.r, fg.g, fg.b, 0xff);
  bg_color = surface_color_rgb(surface->encoding, NULL, 0, bg.r, bg.g, bg.b, 0xff);
  set_font(font, &f, &fnt);
  d = font == 2 ? 2 : 0;
  x = pos.x;
  y = pos.y;

  for (i = 0, s = text; s[i]; i++) {
    if (s[i] == ESCAPE) {
      s[i] = 0;
      if (s[0]) {
        surface_print(surface, x, y+d, s, f, fnt, fg_color, bg_color);
        x += r_get_text_width(font, s, i);
      }
      switch (s[i+1]) {
        case 'f':
          font = s[i+2] - '0';
          set_font(font, &f, &fnt);
          d = font == 2 ? 2 : 0;
          break;
        case 'c':
          switch (s[i+2] - '0') {
            case 0: fg = black; break;
            case 1: fg = blue; break;
            case 2: fg = red; break;
          }
          fg_color = surface_color_rgb(surface->encoding, NULL, 0, fg.r, fg.g, fg.b, 0xff);
          break;
      }
      s += i + 3;
      i = -1;
    }
  }

  if (s[0]) {
    surface_print(surface, x, y+d, s, f, fnt, fg_color, bg_color);
  }
}

void r_draw_icon(int id, mu_Rect r, mu_Color c) {
  uint32_t color;
  int x, y;

  color = surface_color_rgb(surface->encoding, NULL, 0, c.r, c.g, c.b, 0xff);
  y = r.y + (r.h - surface_font_height(&fontstd, 0)) / 2;

  switch (id) {
    case MU_ICON_CLOSE:
      break;
    case MU_ICON_CHECK:
      x = r.x + (r.w - surface_font_char_width(&fontstd, 0, 'X')) / 2;
      surface_print(surface, x, y, "X", &fontstd, 0, color, -1);
      break;
    case MU_ICON_COLLAPSED:
      x = r.x + (r.w - surface_font_char_width(&fontstd, 0, '>')) / 2;
      surface_print(surface, x, y, ">", &fontstd, 0, color, -1);
      break;
    case MU_ICON_EXPANDED:
      x = r.x + (r.w - surface_font_char_width(&fontstd, 0, 'v')) / 2;
      surface_print(surface, x, y, "v", &fontstd, 0, color, -1);
      break;
  }
}

void r_draw_image(void *img, mu_Rect rect) {
  surface_t *src = (surface_t *)img;
  if (src) {
    surface_draw(surface, rect.x, rect.y, src, 0, 0, src->width, src->height);
  }
}

void r_set_clip_rect(mu_Rect r) {
  clip = r;
}

void r_clear(mu_Color c) {
  uint32_t color = surface_color_rgb(surface->encoding, NULL, 0, c.r, c.g, c.b, 0xff);
  surface_rectangle(surface, 0, 0, surface->width - 1, surface->height - 1, 1, color);
}

void r_present(void) {
  int len;
  uint16_t *buf = surface_buffer(surface, &len);
  pumpkin_screen_copy(buf, Y0, surface->height);
}

static int text_width(mu_Font font, char *text, int len) {
  if (len == -1) len = StrLen(text);
  return r_get_text_width(font, text, len);
}

static int text_height(mu_Font font) {
  return r_get_text_height();
}

static void surface_setpixel(void *data, int x, int y, uint32_t color) {
  if (color != -1 && x >= clip.x && y >= clip.y && x < clip.x+clip.w && y < clip.y+clip.h) {
    old_setpixel(data, x, y, color);
  }
}

static void setColorRGB(UInt8 r, UInt8 g, UInt8 b, int c) {
  style.colors[c].r = r;
  style.colors[c].g = g;
  style.colors[c].b = b;
}

static void setColor(UInt16 index, int c, ColorTableType *colorTable) {
  RGBColorType rgb;
  IndexedColorType color = UIColorGetTableEntryIndex(index);
  CtbGetEntry(colorTable, color, &rgb);
  setColorRGB(rgb.r, rgb.g, rgb.b, c);
}

typedef enum {
  CSS_AT,
  CSS_LIST,
  CSS_STYLE
} css_type_t;

typedef struct css_t {
  css_type_t type;
  struct css_t *parent;
  struct css_t *children;
  struct css_t *next;
  char *name;
} css_t;

static int get_color(ht *table, char *name, ht *colors, mu_Color *c) {
  int red, green, blue;
  char *color;
  int r = 0;

  color = ht_get(table, name);
  if (color) {
    if (color[0] != '#' && colors) {
      color = ht_get(colors, color);
    }
    if (color && sys_sscanf(color, "#%2X%2X%2X", &red, &green, &blue) == 3) {
      c->r = red;
      c->g = green;
      c->b = blue;
      r = 1;
    }
  }

  return r;
}

static int get_prop(ht *table, char *name, int *v) {
  char *value;
  int r = 0;

  value = ht_get(table, name);
  if (value) {
  }

  return r;
}

static void add_block(block_cmd_t cmd, char *content, int len, void *data, block_t **block, block_t **curr, tree_t *node) {
  block_t *blk;
  ht *table;
  int color, bgcolor, margin, padding;
  char *class_name, *id;

  blk = sys_calloc(1, sizeof(block_t));
  blk->cmd = cmd;
  blk->data = data;

  if (content && len) {
    blk->content = sys_calloc(len + 1, 1);
    sys_memcpy(blk->content, content, len);
    blk->len = len;
  }

  // block default colors
  blk->style.fg.r = 0x00;
  blk->style.fg.g = 0x00;
  blk->style.fg.b = 0x00;
  blk->style.bg.r = 0xff;
  blk->style.bg.g = 0xff;
  blk->style.bg.b = 0xff;

  if (cmd == CMD_NULL && node) {
    color = bgcolor = 0;

    // search for "style" attribute in enclosing tag
    if (node->style) {
      // found, search for declarations in style
      color   = get_color(node->style, CSS_COLOR, env.colors, &blk->style.fg);
      bgcolor = get_color(node->style, CSS_BGCOLOR, env.colors, &blk->style.bg);
      margin  = get_prop(node->style, CSS_MARGIN, &blk->style.margin);
      padding = get_prop(node->style, CSS_PADDING, &blk->style.padding);
    }

    if (node->attributes && (!color || !bgcolor)) {
      // search for "id" attribute in enclosing tag
      if (env.ids) {
        id = ht_get(node->attributes, "id");
        if (id) {
          // found, search for the id in global css stylesheet
          table = ht_get(env.ids, id);
          if (table) {
            // found, search for declarations for that id
            if (!color)   color   = get_color(table, CSS_COLOR, env.colors, &blk->style.fg);
            if (!bgcolor) bgcolor = get_color(table, CSS_BGCOLOR, env.colors, &blk->style.bg);
            if (!margin)  margin  = get_prop(table, CSS_MARGIN, &blk->style.margin);
            if (!padding) padding = get_prop(table, CSS_PADDING, &blk->style.padding);
          }
        }
      }

      if (env.classes && (!color || !bgcolor)) {
        // search for "class" attribute in enclosing tag
        class_name = ht_get(node->attributes, "class");
        if (class_name) {
          // found, search for the class in global css stylesheet
          table = ht_get(env.classes, class_name);
          if (table) {
            // found, search for declarations for that class
            if (!color)   color   = get_color(table, CSS_COLOR, env.colors, &blk->style.fg);
            if (!bgcolor) bgcolor = get_color(table, CSS_BGCOLOR, env.colors, &blk->style.bg);
            if (!margin)  margin  = get_prop(table, CSS_MARGIN, &blk->style.margin);
            if (!padding) padding = get_prop(table, CSS_PADDING, &blk->style.padding);
          }
        }
      }
    }

    if (env.tags) {
      while (node && node->type == DOM_TAG && (!color || !bgcolor)) {
        // search for the tag in global css stylesheet
        table = ht_get(env.tags, node->name);
        if (table) {
          // found, search for color declarations for that tag
          if (!color)   color   = get_color(table, CSS_COLOR, env.colors, &blk->style.fg);
          if (!bgcolor) bgcolor = get_color(table, CSS_BGCOLOR, env.colors, &blk->style.bg);
          if (!margin)  margin  = get_prop(table, CSS_MARGIN, &blk->style.margin);
          if (!padding) padding = get_prop(table, CSS_PADDING, &blk->style.padding);
        }
        // move up to the parent tag
        node = node->parent;
      }
    }
  }

  if (*block) {
    (*curr)->next = blk;
    *curr = blk;
  } else {
    *block = *curr = blk;
  }
}

static void walk_dom(tree_t *node, block_t **block, block_t **curr) {
  tree_t *child;
  block_t *first;
  ht *table;
  char *start, *end, *s;

  switch (node->type) {
    case DOM_DOCUMENT:
      debug(DEBUG_INFO, "b3", "begin document");
      if (node->children && node->children->tag == TAG_HTML) {
        walk_dom(node->children, block, curr);
      }

      if (env.tags) {
        table = ht_get(env.tags, "body");
        if (table) {
          get_color(table, CSS_BGCOLOR, env.colors, &style.colors[MU_COLOR_WINDOWBG]);
        }
      }

      debug(DEBUG_INFO, "b3", "end document");
      break;
    case DOM_TAG:
      debug(DEBUG_INFO, "b3", "begin tag %s", node->name);
      switch (node->tag) {
        case TAG_HTML:
          for (child = node->children; child; child = child->next) {
            if (child->tag == TAG_BODY) {
              walk_dom(child, block, curr);
              break;
            }
          }
          break;
        case TAG_IMG:
          debug(DEBUG_INFO, "b3", "image id %d data %p", node->id, node->data);
          add_block(CMD_IMG, NULL, 0, node->data, block, curr, node);
          break;
        case TAG_A:
          child = node->children;
          if (child && !child->next && child->type == DOM_TEXT) {
            add_block(CMD_LINK, child->name, sys_strlen(child->name), NULL, block, curr, node);
            first = *curr;
            first->arg = ht_get(node->attributes, "href");
          }
          break;
        case TAG_TH:
        case TAG_TR:
          if (node->children) {
            first = NULL;
            for (child = node->children; child; child = child->next) {
              if (child->tag == TAG_TD) {
                walk_dom(child, block, curr);
                if (first == NULL) {
                  first = *curr;
                  first->cmd = CMD_ROW;
                }
                first->count++;
              }
            }
          }
          break;
        default:
          for (child = node->children; child; child = child->next) {
            walk_dom(child, block, curr);
          }
          break;
      }
      debug(DEBUG_INFO, "b3", "end tag %s", node->name);
      break;
    case DOM_TEXT:
      debug(DEBUG_INFO, "b3", "text [%s]", node->name);
      if (node->name && node->name[0]) {
        for (start = node->name; *start && *start <= 32; start++);
        for (end = &node->name[sys_strlen(node->name)-1]; end > start && *end <= 32; end--);
        if (end >= start) {
          for (s = start; s <= end; s++) {
            if (*s < 32) *s = 32;
          }
          add_block(CMD_NULL, start, end - start + 1, NULL, block, curr, node->parent);
        }
      }
      break;
  }
}

static void define_function(int pe, script_ref_t obj, char *name, int (*f)(int pe, void *data), void *data) {
  script_arg_t value;

  if (obj) {
    script_add_function_data(pe, obj, name, f, data);
  } else {
    value.type = SCRIPT_ARG_FUNCTION;
    value.value.r = script_create_function_data(pe, f, data);
    script_global_set(pe, name, &value);
  }
}

static int generic_walk_dom(int pe, tree_t *node, int (*callback)(int pe, tree_t *node, void *data), void *data) {
  tree_t *child;

  switch (node->type) {
    case DOM_DOCUMENT:
      if (node->children) {
        return generic_walk_dom(pe, node->children, callback, data);
      }
      break;
    case DOM_TAG:
      if (callback(pe, node, data)) return 1;
      for (child = node->children; child; child = child->next) {
        if (generic_walk_dom(pe, child, callback, data)) return 1;
      }
      break;
    case DOM_TEXT:
      break;
  }

  return 0;
}

static int getElementById(int pe, void *data);

static script_ref_t add_obj(int pe, script_ref_t obj, char *name) {
  script_arg_t key, value;

  key.type = SCRIPT_ARG_STRING;
  key.value.s = "style";
  key.type = SCRIPT_ARG_OBJECT;
  key.value.r = script_create_object(pe);
  script_object_set(pe, obj, &key, &value);

  return key.value.r;
}

static void fill_node(int pe, tree_t *node) {
  script_ref_t style;
  hti i;

  if (node->obj == 0) {
    node->obj = script_create_object(pe);
    script_add_pointer(pe, node->obj, "_node", node);
    script_add_sconst(pe, node->obj, "innerHTML", node->name);
    define_function(env.pe, node->obj, "getElementById", getElementById, node);

    i = ht_iterator(node->attributes);
    while (ht_next(&i)) {
      script_add_sconst(pe, node->obj, (char *)i.key, i.value);
    }

    style = add_obj(pe, node->obj, "style");
    script_add_sconst(pe, style, "color", "#000000");
  }
}

static int getElementById_callback(int pe, tree_t *node, void *data) {
  char *id;
  int r = 0;

  if (node->attributes) {
    if ((id = ht_get(node->attributes, "id")) != NULL) {
      if (sys_strcmp(id, (char *)data) == 0) {
        fill_node(pe, node);
        r = script_push_integer(pe, node->obj);
      }
    }
  }

  return r;
}

static int getElementById(int pe, void *data) {
  char *name = NULL;
  int r = -1;

  if (script_get_string(pe, 0, &name) == 0) {
    if (dom) {
      r = generic_walk_dom(pe, dom, getElementById_callback, name);
    }
  }

  if (name) sys_free(name);

  return r;
}

static int createElement(int pe, void *data) {
  char *name = NULL;
  tree_t *node;
  tag_name_t *tag;
  int r = -1;

  if (script_get_string(pe, 0, &name) == 0) {
    if ((tag = ht_get(env.tagnames, name)) != NULL) {
      node = sys_calloc(1, sizeof(tree_t));
      node->root = dom;
      node->type = DOM_TAG;
      node->tag = tag->id;
      fill_node(pe, node);
      r = script_push_object(pe, node->obj);
    }
  }

  if (name) sys_free(name);

  return r;
}

static int script_alert(int pe, void *data) {
  char *str = NULL;

  if (script_get_string(pe, 0, &str) == 0) {
    FrmCustomAlert(INFO_DIALOG, str, "", "");
  }

  if (str) sys_free(str);

  return 0;
}

static void clear_env(void) {
  if (env.classes) ht_destroy(env.classes);
  if (env.ids) ht_destroy(env.ids);
  if (env.tags) ht_destroy(env.tags);
  env.classes = ht_create();
  env.ids = ht_create();
  env.tags = ht_create();
  env.pe = -1;
}

static int add_text(response_t *resp) {
  block_t *curr;

  clear_env();
  render_tree = curr = NULL;
  add_block(CMD_NULL, resp->response, resp->response_size, NULL, &render_tree, &curr, NULL);
  refresh = 1;

  return 0;
}

typedef struct {
  uint32_t id;
  surface_t *dst;
} add_image_t;

static int add_image_callback(int pe, tree_t *node, void *data) {
  add_image_t *ai = (add_image_t *)data;

  debug(DEBUG_INFO, "b3", "testing node [%s]", node->name);
  if (node && node->id == ai->id) {
    debug(DEBUG_INFO, "b3", "found image id %d %dx%d", ai->id, ai->dst->width, ai->dst->height);
    node->data = ai->dst;
    return 1;
  }

  return 0;
}

static int add_image(response_t *resp) {
  surface_t *src, *dst;
  block_t *curr;
  add_image_t ai;
  int width, height;

  if ((src = surface_load_mem((uint8_t *)resp->response, resp->response_size, SURFACE_ENCODING_ARGB)) != NULL) {
    if (src->width > screen_width) {
      width = screen_width;
      height = (src->width * screen_height) / screen_width;
      dst = surface_create(width, height, src->encoding);
      surface_scale(src, dst);
      surface_destroy(src);
    } else {
      dst = src;
    }

    if (resp->id) {
      ai.id = resp->id;
      ai.dst = dst;
      debug(DEBUG_INFO, "b3", "searching for image id %d %dx%d", ai.id, dst->width, dst->height);
      generic_walk_dom(0, dom, add_image_callback, &ai);
      render_tree = NULL;
      curr = NULL;
      walk_dom(dom, &render_tree, &curr);
      refresh = 1;
    } else {
      clear_env();
      render_tree = curr = NULL;
      add_block(CMD_IMG, NULL, 0, dst, &render_tree, &curr, NULL);
      refresh = 1;
    }
  }

  return 0;
}

static int parse_html(response_t *resp) {
  tree_t *root;
  block_t *curr;
  MemHandle h;
  UInt32 len;
  script_arg_t value;
  char *s, *script;

  clear_env();

  root = sys_calloc(1, sizeof(tree_t));
  root->root = root;
  root->type = DOM_DOCUMENT;

  dom = html_parse(root, &env, resp->url, resp->response, resp->response_size);

  render_tree = NULL;
  curr = NULL;
  walk_dom(dom, &render_tree, &curr);
  refresh = 1;

  if (env.engine) {
    if (env.pe > 0) {
      script_destroy(env.pe);
    }
    if ((env.pe = script_create(env.engine)) != -1) {
      if ((h = DmGet1Resource(scriptEngineJS, 1)) != NULL) {
        if ((s = MemHandleLock(h)) != NULL) {
          len = MemHandleSize(h);
          script = sys_calloc(len + 1, 1);
          sys_memcpy(script, s, len);
          if (script_run(env.pe, script, 0, NULL, 1) == -1) {
            debug(DEBUG_ERROR, "HTML", "builtin script failed");
          }
          MemHandleUnlock(h);
        }
        DmReleaseResource(h);
      }

      dom->obj = script_create_object(env.pe);
      value.type = SCRIPT_ARG_OBJECT;
      value.value.r = dom->obj;
      script_global_set(env.pe, "document", &value);

      define_function(env.pe, 0, "alert", script_alert, NULL);

      script_add_pointer(env.pe, dom->obj, "_node", dom);
      define_function(env.pe, dom->obj, "getElementById", getElementById, dom);
      define_function(env.pe, dom->obj, "createElement", createElement, NULL);
    }
  }

  return 0;
}

static int parse_css(response_t *resp) {
  if (mutex_lock(mutex) == 0) {
    if (resp->seq == env.seq) {
      css_parse(resp->response, resp->response_size, 1, &env, NULL, NULL);
    } else {
      debug(DEBUG_ERROR, "b3", "received last response for '%s' (%d != %d)", resp->url, resp->seq, env.seq);
    }
    mutex_unlock(mutex);
  }

  return 0;
}

static int parse_script(response_t *resp) {
  if (mutex_lock(mutex) == 0) {
    if (resp->seq == env.seq) {
      if (env.pe != -1) {
        debug(DEBUG_INFO, "HTML", "running external script '%s'", resp->url);
        if (script_run(env.pe, resp->response, 0, NULL, 1) == -1) {
          debug(DEBUG_ERROR, "HTML", "script failed");
        }
      }
    } else {
      debug(DEBUG_ERROR, "b3", "received last response for '%s' (%d != %d)", resp->url, resp->seq, env.seq);
    }
    mutex_unlock(mutex);
  }

  return 0;
}

static void process_response(response_t *resp) {
  block_t *curr;

  switch (resp->parse_response) {
    case -1:
      FrmCustomAlert(ERROR_DIALOG, "Connection error", "", "");
      break;
    case 0:
      debug(DEBUG_INFO, "b3", "adding text");
      add_text(resp);
      break;
    case 1:
      debug(DEBUG_INFO, "b3", "parsing html");
      parse_html(resp);
      break;
    case 2:
      debug(DEBUG_INFO, "b3", "parsing css");
      parse_css(resp);
      if (dom) {
        render_tree = NULL;
        curr = NULL;
        walk_dom(dom, &render_tree, &curr);
        refresh = 1;
      }
      break;
    case 3:
      debug(DEBUG_INFO, "b3", "parsing script");
      parse_script(resp);
      break;
    case 4:
      debug(DEBUG_INFO, "b3", "adding image");
      add_image(resp);
      break;
  }

  if (resp->response) sys_free(resp->response);
  if (resp->url) sys_free(resp->url);
}

static int http_callback(int ptr, void *data) {
  response_t *resp = (response_t *)data;
  http_client_t *hc;
  EventType event;
  int i, chunked, chunk_len, rs;
  char *end, *p, *r, *rep;

  if ((hc = ptr_lock(ptr, TAG_HTTP_CLIENT)) != NULL) {
    if (hc->response_error) {
      resp->parse_response = -1;

    } else {
      if (hc->response_fd > 0 && (rs = sys_seek(hc->response_fd, 0, SYS_SEEK_END)) != -1) {
        resp->response_size = rs;
        sys_seek(hc->response_fd, 0, SYS_SEEK_SET);
        resp->response = sys_calloc(resp->response_size + 1, 1);
        sys_read(hc->response_fd, (uint8_t *)resp->response, resp->response_size);
        debug(DEBUG_INFO, "b3", "response is %d bytes", resp->response_size);
      }

      for (i = 0, chunked = 0; i < hc->response_num_headers; i++) {
        if (!sys_strcasecmp(hc->response_header_name[i], "Transfer-Encoding")) {
          if (!sys_strcasecmp(hc->response_header_value[i], "chunked")) {
            debug(DEBUG_INFO, "b3", "response is chunked");
            chunked = 1;
          }
          continue;
        }
        if (!sys_strcasecmp(hc->response_header_name[i], "Content-Type")) {
          if (!sys_strcasecmp(hc->response_header_value[i], "text/html")) {
            debug(DEBUG_INFO, "b3", "response is html");
            resp->parse_response = 1;
          } else if (!sys_strcasecmp(hc->response_header_value[i], "text/css")) {
            debug(DEBUG_INFO, "b3", "response is css");
            resp->parse_response = 2;
          } else if (!sys_strcasecmp(hc->response_header_value[i], "text/javascript")) {
            debug(DEBUG_INFO, "b3", "response is javascript");
            resp->parse_response = 3;
          } else if (!sys_strcasecmp(hc->response_header_value[i], "text/plain")) {
            debug(DEBUG_INFO, "b3", "response is text");
            resp->parse_response = 0;
          } else if (!sys_strcasecmp(hc->response_header_value[i], "image/jpeg") ||
                     !sys_strcasecmp(hc->response_header_value[i], "image/png")) {
            debug(DEBUG_INFO, "b3", "response is image");
            resp->parse_response = 4;
          }
          continue;
        }
      }

      if (chunked) {
        // 1f78<0D><0A>...<0D><0A>
        rep = resp->response;
        for (p = resp->response, end = resp->response + resp->response_size, resp->response_size = 0; p < end;) {
          debug(DEBUG_INFO, "b3", "chunk start");
          debug_bytes(DEBUG_INFO, "b3", (uint8_t *)p, 16);
          chunk_len = sys_strtoul(p, &r, 16);
          if (!r || (r[0] && r[0] != '\r') || (r[1] && r[1] != '\n')) break;
          debug(DEBUG_INFO, "b3", "chunk is %d bytes", chunk_len);
          if (!chunk_len) break;
          sys_memcpy(rep, r + 2, chunk_len);
          resp->response_size += chunk_len;
          rep += chunk_len;
          p = r + 2 + chunk_len + 2;
        }
        debug(DEBUG_INFO, "b3", "new response is %d bytes", resp->response_size);
        resp->response[resp->response_size] = 0;
      }

    }
    ptr_unlock(ptr, TAG_HTTP_CLIENT);

    debug(DEBUG_INFO, "b3", "sending response event to main task %d", task);
    sys_memset(&event, 0, sizeof(EventType));
    event.eType = firstUserEvent;
    sys_memcpy(event.data.buffer, resp, sizeof(response_t));
    pumpkin_forward_event(task, &event);
  }

  sys_free(resp);

  if (mutex_lock(mutex) == 0) {
    env.nthreads--;
    mutex_unlock(mutex);
  }

  return 0;
}

static void serve_url(void) {
  response_t *resp;
  int doit, handle;

  if (resp_queue) {
    doit = 0;

    if (mutex_lock(mutex) == 0) {
      if (env.nthreads < MAX_THREADS) {
        env.nthreads++;
        doit = 1;
      }
      mutex_unlock(mutex);
    }

    if (doit) {
      resp = resp_queue;
      resp_queue = resp->next;
      resp->next = NULL;
      if (resp_queue == NULL) resp_last = NULL;

      debug(DEBUG_INFO, "b3", "serving url %s", resp->url);
      if ((handle = pumpkin_http_get(resp->url, HTTP_TIMEOUT, http_callback, resp)) == -1) {
        if (mutex_lock(mutex) == 0) {
          env.nthreads--;
          mutex_unlock(mutex);
        }
        sys_free(resp->url);
        sys_free(resp);
      } else {
        debug(DEBUG_INFO, "b3", "new thread handle %d", handle);
      }
    }
  }
}

static void set_button(UInt16 id) {
  FormType *frm;
  ControlType *ctl;
  UInt16 index;

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, goBtn);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetGraphics(ctl, id, id);
}

static int fetch_url(char *url, uint32_t id) {
  EventType event;
  FileRef fileRef;
  UInt32 size, nread;
  response_t *resp, fileResp;
  char *ext;
  int r = -1;

  if (!sys_strncmp(url, "http", 4)) {
    debug(DEBUG_INFO, "b3", "queueing url %s", url);
    set_button(stopId);
    resp = sys_calloc(1, sizeof(response_t));
    resp->seq = env.seq;
    resp->id = id;
    resp->url = sys_strdup(url);
    if (resp_queue == NULL) {
      resp_queue = resp;
    } else {
      resp_last->next = resp;
    }
    resp_last = resp;
    r = 0;

  } else {
    debug(DEBUG_INFO, "b3", "reading file %s", url);
    sys_memset(&fileResp, 0, sizeof(response_t));

    ext = getext(url);
    if (!sys_strcmp(ext, "html")) {
      fileResp.parse_response = 1;
    } else if (!sys_strcmp(ext, "css")) {
      fileResp.parse_response = 2;
    } else if (!sys_strcmp(ext, "js")) {
      fileResp.parse_response = 3;
    } else if (!sys_strcmp(ext, "txt")) {
      fileResp.parse_response = 0;
    } else if (!sys_strcmp(ext, "png") || !sys_strcmp(ext, "jpg")) {
      fileResp.parse_response = 4;
    } else {
      fileResp.parse_response = -1;
    }
   
    if (fileResp.parse_response != -1 && VFSFileOpen(1, url, vfsModeRead, &fileRef) == errNone) {
      VFSFileSize(fileRef, &size);
      if (size > 0) {
        fileResp.seq = env.seq;
        fileResp.id = id;
        fileResp.url = sys_strdup(url);
        fileResp.response_size = size;
        fileResp.response = sys_calloc(1, fileResp.response_size + 1);
        VFSFileRead(fileRef, size, fileResp.response, &nread);
        debug(DEBUG_INFO, "b3", "sending response event");
        sys_memset(&event, 0, sizeof(EventType));
        event.eType = firstUserEvent;
        sys_memcpy(event.data.buffer, &fileResp, sizeof(response_t));
        EvtAddEventToQueue(&event);
        r = 0;
      }
      VFSFileClose(fileRef);
    }
  }

  return r;
}

static mu_Context *StartApplication(void) {
  ColorTableType *colorTable;

  colorTable = pumpkin_defaultcolorTable();

  setColor(UIObjectFill, MU_COLOR_WINDOWBG, colorTable);
  setColor(UIObjectForeground, MU_COLOR_TEXT, colorTable);
  setColor(UIObjectSelectedFill, MU_COLOR_TITLEBG, colorTable);
  setColor(UIObjectSelectedForeground, MU_COLOR_TITLETEXT, colorTable);
  setColor(UIObjectFrame, MU_COLOR_BORDER, colorTable);
  setColor(UIObjectFill, MU_COLOR_PANELBG, colorTable);
  setColorRGB(0xE0, 0xE0, 0xF0, MU_COLOR_BUTTON);
  setColorRGB(0x90, 0x90, 0xD0, MU_COLOR_BUTTONHOVER);
  setColor(UIObjectSelectedFill, MU_COLOR_BUTTONFOCUS, colorTable);
  setColorRGB(0xc0, 0xc0, 0xc0, MU_COLOR_SCROLLBASE);
  setColorRGB(0x60, 0x60, 0x60, MU_COLOR_SCROLLTHUMB);
  setColor(UIObjectFill, MU_COLOR_BASE, colorTable);
  setColor(UIObjectFill, MU_COLOR_BASEHOVER, colorTable);
  setColor(UIObjectFill, MU_COLOR_BASEFOCUS, colorTable);

  mu_Context *ctx = MemPtrNew(sizeof(mu_Context));
  mu_init(ctx, &style);
  ctx->text_width = text_width;
  ctx->text_height = text_height;

  WinScreenGetAttribute(winScreenWidth, &screen_width);
  WinScreenGetAttribute(winScreenHeight, &screen_height);
  surface = surface_create(screen_width, screen_height, pumpkin_get_encoding());
  old_setpixel = surface->setpixel;
  surface->setpixel = surface_setpixel;

  task = pumpkin_get_current();
  debug(DEBUG_INFO, "b3", "current task %d", task);

  dom = NULL;
  render_tree = NULL;
  resp_queue = resp_last = NULL;

  mutex = mutex_create("browser");
  sys_memset(&env, 0, sizeof(html_env_t));
  env.fetch = fetch_url;
  env.colors = css_color_names();

  if ((env.engine = script_load_engine("libscriptjs")) != NULL) {
    if (script_init(env.engine) != -1) {
      debug(DEBUG_INFO, "b3", "script engine initialized");
    } else {
      debug(DEBUG_INFO, "b3", "script engine initialization failed");
    }
  } else {
    debug(DEBUG_INFO, "b3", "script engine not found");
  }

  EvtReturnPenMove(true);
  FrmCenterDialogs(true);
  FrmGotoForm(MainForm);

  return ctx;
}

static void StopApplication(void) {
  if (env.engine) {
    if (env.pe > 0) script_destroy(env.pe);
    script_finish(env.engine);
  }

  surface_destroy(surface);
  mutex_destroy(mutex);
}

static void browser_window(mu_Context *ctx) {
  block_t *row;
  surface_t *s;
  int i, n, widths[16];

  if (mu_begin_window_ex(ctx, "b3", mu_rect(0, 0, screen_width, screen_height - Y0), MU_OPT_NOTITLE | MU_OPT_NORESIZE | MU_OPT_NODRAG)) {
    n = 0;
    widths[0] = -1;
    mu_layout_row(ctx, 1, widths, 0);

    for (row = render_tree; row; row = row->next) {
      if (row->cmd == CMD_ROW) {
        n = row->count <= 16 ? row->count : 16;
        for (i = 0; i < n; i++) {
          widths[i] = screen_width / n - 10;
        }
        mu_layout_row(ctx, n, widths, 0);

      } 

      switch (row->cmd) {
        case CMD_NULL:
          if (row->content) {
            mu_text_ex(ctx, row->content, 0, row->style.fg, row->style.bg);
          }
          break;
        case CMD_LINK:
          if (row->content) {
            mu_link_ex(ctx, row->content, 0);
debug(1, "XXX", "link href [%s]", row->arg);
          }
          break;
        case CMD_IMG:
          if ((s = (surface_t *)row->data) != NULL) {
            mu_image(ctx, s, s->width, s->height);
          }
          break;
        default:
          break;
      }

      if (n > 0) {
        n--;
        if (n == 0) {
          widths[0] = -1;
          mu_layout_row(ctx, 1, widths, 0);
        }
      }
    }

    mu_end_window(ctx);
  }
}

static void MenuEvent(UInt16 id) {
  switch (id) {
    case aboutCmd:
      AbtShowAboutPumpkin(pumpkin_get_app_creator());
      break;
  }
}

static void resize(FormType *frm) {
  WinHandle wh;
  RectangleType rect;
  UInt32 swidth, sheight;
  UInt16 objIndex;

  WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
  wh = FrmGetWindowHandle(frm);
  RctSetRectangle(&rect, 0, 0, swidth, sheight);
  WinSetBounds(wh, &rect);

  objIndex = FrmGetObjectIndex(frm, urlFld);
  FrmGetObjectBounds(frm, objIndex, &rect);
  rect.extent.x += swidth - 160;
  FrmSetObjectBounds(frm, objIndex, &rect);

  objIndex = FrmGetObjectIndex(frm, goBtn);
  FrmGetObjectBounds(frm, objIndex, &rect);
  rect.topLeft.x = swidth - rect.extent.x - 1;
  FrmSetObjectBounds(frm, objIndex, &rect);
}

#if 0
static void rounded_rectangle(RectangleType *rect, int r, uint32_t color) {
  int x1, y1, x2, y2, x3, y3, x4, y4;

  x1 = rect->topLeft.x;
  y1 = rect->topLeft.y;
  x2 = rect->topLeft.x + rect->extent.x - 1;
  y2 = rect->topLeft.y;
  x3 = rect->topLeft.x + rect->extent.x - 1;
  y3 = rect->topLeft.y + rect->extent.y - 1;
  x4 = rect->topLeft.x;
  y4 = rect->topLeft.y + rect->extent.y - 1;
  surface_line(surface, x1+r, y1, x2-r, y2, color);
  surface_line(surface, x2, y2+r, x3, y3-r, color);
  surface_line(surface, x3-r, y3, x4+r, y4, color);
  surface_line(surface, x4, y4-r, x1, y1+r, color);

  if (r > 0) {
    surface_curve(surface, x1, y1+r, x1, y1, x1+r, y1, color);
    surface_curve(surface, x2-r, y2, x2, y2, x2, y2+r, color);
    surface_curve(surface, x3, y3-r, x3, y3, x3-r, y3, color);
    surface_curve(surface, x4+r, y4, x4, y4, x4, y4-r, color);
  }
}

typedef struct {
  uint32_t bg;
  uint32_t color;
  int x, y;
  int a;
} path_t;

static void walk(path_t *p, int d, int r, int a, int e) {
  int dx, dy, dxr, dyr, x1, y1, x2, y2, x3, y3;
  double angle, dcos, dsin, pi;

  pi = sys_pi();
  angle = p->a * pi / 180.0;
  dcos = sys_cos(angle);
  dsin = sys_sin(angle);
  dx = (int)(d * dcos + 0.5);
  dy = (int)(d * dsin + 0.5);
  x1 = p->x + dx;
  y1 = p->y + dy;
  surface_line(surface, p->x, p->y, x1, y1, p->color);
  if (a == 0) {
    p->x = x1;
    p->y = y1;
  } else {
    dxr = (int)(r * dcos + 0.5);
    dyr = (int)(r * dsin + 0.5);
    x2 = x1 + dxr;
    y2 = y1 + dyr;
    p->a += a;
    if (p->a >= 360) p->a -= 360;
    else if (p->a < 0) p->a += 360;
    angle = p->a * pi / 180.0;
    dcos = sys_cos(angle);
    dsin = sys_sin(angle);
    dxr = (int)(r * dcos + 0.5);
    dyr = (int)(r * dsin + 0.5);
    x3 = x2 + dxr;
    y3 = y2 + dyr;
    if (e) surface_ellipse(surface, x1, y1+e, r, r, 1, p->bg);
    surface_curve(surface, x1, y1, x2, y2, x3, y3, p->color);
    p->x = x3;
    p->y = y3;
  }
}
#endif

static Boolean MainFormHandleEvent(EventType *event) {
  FormType *frm;
  ControlType *ctl;
  FieldType *fld;
  UInt16 index;
  Coord x, y;
  DmResID resId;
  response_t *resp;
  char *url;
  Boolean handled = false;
//uint32_t color;
//RectangleType rect;
//path_t pathl, pathc, pathr;

  x = event->screenX*2;
  y = event->screenY*2;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      resize(frm);
      FrmDrawForm(frm);
#if 0
RctSetRectangle(&rect, 50, 50, 200, 200);
color = surface_color_rgb(surface->encoding, NULL, 0, 0xff, 0xff, 0xff, 0xff);
surface_rectangle(surface, 0, 0, surface->width - 1, surface->height - 1, 1, color);
//rounded_rectangle(&rect, 60, color);

int r = 16;
int w = 32;
pathc.bg = surface_color_rgb(surface->encoding, NULL, 0, 0x80, 0x80, 0x80, 0xff);
pathc.color = surface_color_rgb(surface->encoding, NULL, 0, 0xff, 0x00, 0x00, 0xff);
pathc.x = screen_width / 2;
pathc.y = screen_height / 2 - w;
pathc.a = 0;

pathr.bg = pathc.bg;
pathr.color = surface_color_rgb(surface->encoding, NULL, 0, 0x00, 0x00, 0x00, 0xff);
pathr.x = pathc.x;
pathr.y = pathc.y + w/2;
pathr.a = 0;

pathl.bg = pathc.bg;
pathl.color = surface_color_rgb(surface->encoding, NULL, 0, 0x00, 0x00, 0x00, 0xff);
pathl.x = pathc.x;
pathl.y = pathc.y - w/2;
pathl.a = 0;

walk(&pathr, 100, r,      90, 0);
walk(&pathr, 100, r+w,   -90, 0);
walk(&pathr,  50, r+w,   -90, 0);
walk(&pathr, 200, r+w,   -90, 0);
walk(&pathr, 400, r+w,   -90, 0);
walk(&pathr,  50-w/2, r+w,   -90, 0);
walk(&pathr, 150+w, 0,   0, 0);

walk(&pathc, 100, r+w/2,  90, 0);
walk(&pathc, 100, r+w/2, -90, 0);
walk(&pathc,  50, r+w/2, -90, 0);
walk(&pathc, 200, r+w/2, -90, 0);
walk(&pathc, 400, r+w/2, -90, 0);
walk(&pathc,  50-w/2, r+w/2, -90, 0);
walk(&pathc, 150+w, 0,   0, 0);

walk(&pathl, 100, r+w,    90, r+w);
walk(&pathl, 100, r,     -90, 0);
walk(&pathl,  50, r,     -90, 0);
walk(&pathl, 200, r,     -90, 0);
walk(&pathl, 400, r,     -90, 0);
walk(&pathl,  50-w/2, r,     -90, 0);
walk(&pathl, 150+w, 0,   0, 0);

/*
------...
------. .
      | |
      | |
      . .____
      ...____
*/

r_present();
#endif
      handled = true;
      break;

    case menuEvent:
      MenuEvent(event->data.menu.itemID);
      handled = true;
      break;

    case keyDownEvent:
      fld = FldGetActiveField();
      if (fld == NULL || event->data.keyDown.chr != 10) {
        break;
      }
      // simulate a button click when ENTER is typed
      event->data.ctlSelect.controlID = goBtn;
      // fall through

    case ctlSelectEvent:
      if (event->data.ctlSelect.controlID == goBtn) {
        frm = FrmGetActiveForm();
        index = FrmGetObjectIndex(frm, urlFld);
        fld = FrmGetObjectPtr(frm, index);
        index = FrmGetObjectIndex(frm, goBtn);
        ctl = FrmGetObjectPtr(frm, index);
        CtlGetGraphics(ctl, &resId, NULL);
        url = FldGetTextPtr(fld);

        if (mutex_lock(mutex) == 0) {
          env.seq++;
          mutex_unlock(mutex);
        }

        if (resId == stopId) {
          debug(DEBUG_INFO, "b3", "aborting fetch");
          CtlSetGraphics(ctl, loadId, loadId);

        } else if (url && url[0]) {
          if (fetch_url(url, 0) == -1) {
            FrmCustomAlert(ERROR_DIALOG, "Fetch error", "", "");
          }
        }
        handled = true;
      }
      break;

    case penMoveEvent:
      if (y >= Y0) {
        mu_input_mousemove(ctx, x, y - Y0);
        handled = true;
      }
      break;

    case penDownEvent:
      if (y >= Y0) {
        mu_input_mousedown(ctx, x, y - Y0, MU_MOUSE_LEFT);
        FldSetActiveField(NULL);
        refresh = 1;
        handled = true;
      }
      break;

    case penUpEvent:
      if (y >= Y0) {
        mu_input_mouseup(ctx, x, y - Y0, MU_MOUSE_LEFT);
        handled = true;
      }
      break;

    case firstUserEvent:
      debug(DEBUG_INFO, "b3", "received response event");
      resp = (response_t *)event->data.buffer;
      if (resp->parse_response == -1 || resp->parse_response == 1) {
        set_button(loadId);
      }
      process_response(resp);
      break;

    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventPtr event) {
  FormPtr frm;
  UInt16 form;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      form = event->data.frmLoad.formID;
      frm = FrmInitForm(form);
      FrmSetActiveForm(frm);
      FrmSetEventHandler(frm, MainFormHandleEvent);
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static void EventLoop(mu_Context *ctx) {
  Boolean draw;
  mu_Command *cmd;
  EventType event;
  Err err;

  do {
    refresh = 0;
    EvtGetEvent(&event, 20);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);

    serve_url();

    if (render_tree && refresh) {
      mu_begin(ctx);
      browser_window(ctx);
      mu_end(ctx);

      draw = false;
      cmd = NULL;
      while (mu_next_command(ctx, &cmd)) {
        switch (cmd->type) {
          case MU_COMMAND_TEXT:  r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.font, cmd->text.color, cmd->text.back); draw = true; break;
          case MU_COMMAND_RECT:  r_draw_rect(cmd->rect.rect, cmd->rect.color); draw = true; break;
          case MU_COMMAND_ICON:  r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); draw = true; break;
          case MU_COMMAND_IMAGE: r_draw_image(cmd->image.img, cmd->icon.rect); draw = true; break;
          case MU_COMMAND_CLIP:  r_set_clip_rect(cmd->clip.rect); break;
        }
      }

      if (draw) {
        r_present();
      }
    }
  } while (event.eType != appStopEvent);
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  if (cmd == sysAppLaunchCmdNormalLaunch) {
    if ((ctx = StartApplication()) != NULL) {
      EventLoop(ctx);
      StopApplication();
    }
  }

  return 0;
}
