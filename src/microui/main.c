#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pumpkin.h"
#include "surface.h"
#include "fontstd.h"
#include "palette.h"
#include "renderer.h"
#include "debug.h"

static surface_t *surface;
static void (*old_setpixel)(void *data, int x, int y, uint32_t color);
static mu_Real bg[3] = { 128, 128, 128 };
static mu_Rect clip = { 0, 0, 9999, 99999 };

static mu_Style style = {
  /* font | size | padding | spacing | indent */
  NULL, { 68, 10 }, 5, 4, 24,
  /* title_height | scrollbar_size | thumb_size */
  24, 12, 8,
  {
    { 230, 230, 230, 255 }, /* MU_COLOR_TEXT */
    { 25,  25,  25,  255 }, /* MU_COLOR_BORDER */
    { 50,  50,  50,  255 }, /* MU_COLOR_WINDOWBG */
    { 25,  25,  25,  255 }, /* MU_COLOR_TITLEBG */
    { 240, 240, 240, 255 }, /* MU_COLOR_TITLETEXT */
    { 0,   0,   0,   0   }, /* MU_COLOR_PANELBG */
    { 75,  75,  75,  255 }, /* MU_COLOR_BUTTON */
    { 95,  95,  95,  255 }, /* MU_COLOR_BUTTONHOVER */
    { 115, 115, 115, 255 }, /* MU_COLOR_BUTTONFOCUS */
    { 30,  30,  30,  255 }, /* MU_COLOR_BASE */
    { 35,  35,  35,  255 }, /* MU_COLOR_BASEHOVER */
    { 40,  40,  40,  255 }, /* MU_COLOR_BASEFOCUS */
    { 43,  43,  43,  255 }, /* MU_COLOR_SCROLLBASE */
    { 30,  30,  30,  255 }  /* MU_COLOR_SCROLLTHUMB */
  }
};

void r_draw_rect(mu_Rect r, mu_Color c) {
  int color = surface_color_rgb(surface->encoding, NULL, 0, c.r, c.g, c.b, 0xff);
  surface_rectangle(surface, r.x, r.y, r.x + r.w - 1, r.y + r.h - 1, 1, color);
}

void r_draw_text(const char *text, mu_Vec2 pos, mu_Color c) {
  int color = surface_color_rgb(surface->encoding, NULL, 0, c.r, c.g, c.b, 0xff);
  surface_print(surface, pos.x, pos.y, (char *)text, &fontstd, 0, color, -1);
}

void r_draw_icon(int id, mu_Rect r, mu_Color c) {
  int x, y, color;

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

int r_get_text_width(const char *text, int len) {
  int i, width = 0;
  for (i = 0; i < len; i++) {
    width += surface_font_char_width(&fontstd, 0, text[i]);
  }
  return width;
}

int r_get_text_height(void) {
  return surface_font_height(&fontstd, 0);
}

void r_set_clip_rect(mu_Rect r) {
  clip = r;
}

void r_clear(mu_Color c) {
  int color = surface_color_rgb(surface->encoding, NULL, 0, c.r, c.g, c.b, 0xff);
  surface_rectangle(surface, 0, 0, surface->width - 1, surface->height - 1, 1, color);
}

void r_present(void) {
  int len;
  uint16_t *buf = surface_buffer(surface, &len);
  pumpkin_screen_copy(buf, 0, surface->height);
}

static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) len = StrLen(text);
  return r_get_text_width(text, len);
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
  IndexedColorType color = UIColorGetTableEntryIndex(index);
  setColorRGB(colorTable->entry[color].r, colorTable->entry[color].g, colorTable->entry[color].b, c);
}

static mu_Context *StartApplication(void) {
  FullColorTableType fcolorTable;
  ColorTableType *colorTable;
  UInt32 width, height;

  colorTable = (ColorTableType *)&fcolorTable;
  MemMove(colorTable->entry, defaultPalette, MAX_PAL * sizeof(RGBColorType));
  colorTable->numEntries = MAX_PAL;

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

  WinScreenGetAttribute(winScreenWidth, &width);
  WinScreenGetAttribute(winScreenHeight, &height);
  surface = surface_create(width, height, pumpkin_get_encoding());
  old_setpixel = surface->setpixel;
  surface->setpixel = surface_setpixel;

  return ctx;
}

static void StopApplication(void) {
  surface_destroy(surface);
}

static void test_window(mu_Context *ctx) {
  /* do window */
  if (mu_begin_window_ex(ctx, "Demo Window", mu_rect(0, 0, 320, 450), MU_OPT_NORESIZE | MU_OPT_NODRAG)) {
    mu_Container *win = mu_get_current_container(ctx);
    win->rect.w = mu_max(win->rect.w, 240);
    win->rect.h = mu_max(win->rect.h, 300);

    /* window info */
    if (mu_header(ctx, "Window Info")) {
      mu_Container *win = mu_get_current_container(ctx);
      char buf[64];
      mu_layout_row(ctx, 2, (int[]) { 70, -1 }, 0);
      mu_label(ctx,"Position:");
      sys_sprintf(buf, "%d, %d", win->rect.x, win->rect.y); mu_label(ctx, buf);
      mu_label(ctx, "Size:");
      sys_sprintf(buf, "%d, %d", win->rect.w, win->rect.h); mu_label(ctx, buf);
    }

    /* labels + buttons */
    if (mu_header_ex(ctx, "Test Buttons", MU_OPT_EXPANDED)) {
      mu_layout_row(ctx, 3, (int[]) { 130, -110, -1 }, 0);
      mu_label(ctx, "Test buttons 1:");
      mu_button(ctx, "Btn 1");
      mu_button(ctx, "Btn 2");
      mu_label(ctx, "Test buttons 2:");
      mu_button(ctx, "Btn 3");
      if (mu_button(ctx, "Popup")) { mu_open_popup(ctx, "Test Popup"); }
      if (mu_begin_popup(ctx, "Test Popup")) {
        mu_button(ctx, "Hello");
        mu_button(ctx, "World");
        mu_end_popup(ctx);
      }
    }

    /* tree */
    if (mu_header_ex(ctx, "Tree and Text", MU_OPT_EXPANDED)) {
      mu_layout_row(ctx, 2, (int[]) { 145, -1 }, 0);
      mu_layout_begin_column(ctx);
      if (mu_begin_treenode(ctx, "Test 1")) {
        if (mu_begin_treenode(ctx, "Test 1a")) {
          mu_label(ctx, "Hello");
          mu_label(ctx, "world");
          mu_end_treenode(ctx);
        }
        if (mu_begin_treenode(ctx, "Test 1b")) {
          mu_button(ctx, "Btn 1");
          mu_button(ctx, "Btn 2");
          mu_end_treenode(ctx);
        }
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 2")) {
        mu_layout_row(ctx, 2, (int[]) { 50, 50 }, 0);
        mu_button(ctx, "Btn 3");
        mu_button(ctx, "Btn 4");
        mu_button(ctx, "Btn 5");
        mu_button(ctx, "Btn 6");
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 3")) {
        static int checks[3] = { 1, 0, 1 };
        mu_checkbox(ctx, "Checkbox 1", &checks[0]);
        mu_checkbox(ctx, "Checkbox 2", &checks[1]);
        mu_checkbox(ctx, "Checkbox 3", &checks[2]);
        mu_end_treenode(ctx);
      }
      mu_layout_end_column(ctx);

      mu_layout_begin_column(ctx);
      mu_layout_row(ctx, 1, (int[]) { -1 }, 0);
      mu_text(ctx, "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
      mu_layout_end_column(ctx);
    }

    /* background color sliders */
    if (mu_header_ex(ctx, "Background Color", MU_OPT_EXPANDED)) {
      mu_layout_row(ctx, 2, (int[]) { -78, -1 }, 74);
      /* sliders */
      mu_layout_begin_column(ctx);
      mu_layout_row(ctx, 2, (int[]) { 60, -1 }, 0);
      mu_label(ctx, "Red:");   mu_slider(ctx, &bg[0], 0, 255);
      mu_label(ctx, "Green:"); mu_slider(ctx, &bg[1], 0, 255);
      mu_label(ctx, "Blue:");  mu_slider(ctx, &bg[2], 0, 255);
      mu_layout_end_column(ctx);
      /* color preview */
      mu_Rect r = mu_layout_next(ctx);
      mu_draw_rect(ctx, r, mu_color((int)bg[0], (int)bg[1], (int)bg[2], 255));
      char buf[32];
      sys_sprintf(buf, "#%02X%02X%02X", (int)bg[0], (int)bg[1], (int)bg[2]);
      mu_draw_control_text(ctx, buf, r, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
    }

    mu_end_window(ctx);
  }
}

static void EventLoop(mu_Context *ctx) {
  int ev, key, mods, buttons;
  mu_Command *cmd;
  uint64_t last = 0;
  int draw, x = 0, y = 0;

  for (; !thread_must_end();) {
    ev = pumpkin_event(&key, &mods, &buttons, NULL, NULL, 200000);
    switch (ev) {
      case MSG_MOTION:
        x = key;
        y = mods;
        mu_input_mousemove(ctx, x, y);
        break;
      case MSG_BUTTON:
        if (buttons) {
          mu_input_mousedown(ctx, x, y, MU_MOUSE_LEFT);
        } else {
          mu_input_mouseup(ctx, x, y, MU_MOUSE_LEFT);
        }
        break;
      case MSG_KEYDOWN:
        if (key != WINDOW_KEY_HOME) {
          mu_input_keydown(ctx, key);
        }
        break;
      case MSG_KEYUP:
        if (key == WINDOW_KEY_HOME) return;
        mu_input_keyup(ctx, key);
        break;
      case -1:
        return;
    }

    if ((sys_get_clock() - last) > 50000) {
      mu_begin(ctx);
      test_window(ctx);
      mu_end(ctx);

      draw = 0;
      cmd = NULL;
      while (mu_next_command(ctx, &cmd)) {
        switch (cmd->type) {
          case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); draw = 1; break;
          case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); draw = 1; break;
          case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); draw = 1; break;
          case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
        }
      }

      if (draw) {
        r_present();
        last = sys_get_clock();
      }
    }
  }
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  mu_Context *ctx;

  if (cmd == sysAppLaunchCmdNormalLaunch) {
    if ((ctx = StartApplication()) != NULL) {
      EventLoop(ctx);
      StopApplication();
    }
  }

  return 0;
}
