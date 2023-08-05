#include <PalmOS.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "script.h"
#include "pwindow.h"
#include "pumpkin.h"
#include "sys.h"
#include "debug.h"
#include "resource.h"

#include "display.h"
#include "input.h"
#include "player.h"
#include "gifload.h"
#include "resource.h"

#define MAP_N 1024
#define Z_FAR 1000.0
#define VERTICAL_SCALE_FACTOR 35.0

#define Y0 30

#define GifResource 'Gifm'

static uint16_t *framebuffer = NULL;
static uint8_t *heightmap = NULL;
static uint8_t *colormap = NULL;
static uint16_t palette[256*3];
static uint64_t extKeyMask[2];
static bool is_running = false;

static player_t player = {
  .x           = 512.0,
  .y           = 512.0,
  .height      = 130.0,
  .angle       =   0.0,
  .pitch       =   0.0,
  .forward_vel =   0.0,   .forward_acc = 0.06,   .forward_brk = 0.06,   .forward_max = 3.0,
  .pitch_vel   =   0.0,   .pitch_acc   = 0.06,   .pitch_brk   = 0.10,   .pitch_max   = 2.0,
  .yaw_vel     =   0.0,   .yaw_acc     = 0.10,   .yaw_brk     = 0.10,   .yaw_max     = 2.0,
  .lift_vel    =   0.0,   .lift_acc    = 0.06,   .lift_brk    = 0.07,   .lift_max    = 2.0,
  .strafe_vel  =   0.0,   .strafe_acc  = 0.05,   .strafe_brk  = 0.09,   .strafe_max  = 2.0,
  .roll_vel    =   0.0,   .roll_acc    = 0.04,   .roll_brk    = 0.09,   .roll_max    = 2.0
};

static uint8_t *load_gifmap(UInt32 type, UInt16 id, int *pal_count, uint8_t *gif_palette) {
  MemHandle h;
  uint8_t *data, *map = NULL;

  if ((h = DmGetResource(type, id)) != NULL) {
    if ((data = MemHandleLock(h)) != NULL) {
      map = load_gifm(data, MemHandleSize(h), pal_count, gif_palette);
      MemHandleUnlock(h);
      DmReleaseResource(h);
    }
  }

  return map;
}

static void load_map(void) {
  uint8_t gif_palette[256 * 3];
  int pal_count;

  colormap = load_gifmap(GifResource, colorMap, &pal_count, gif_palette);
  heightmap = load_gifmap(GifResource, heightMap, NULL, NULL);

  for (int i = 0; i < pal_count; i++) {
    uint16_t r = gif_palette[3 * i + 0];
    uint16_t g = gif_palette[3 * i + 1];
    uint16_t b = gif_palette[3 * i + 2];
    r = (r & 63) << 2;
    g = (g & 63) << 2;
    b = (b & 63) << 2;
    palette[i] = htobe16(surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, r, g, b, 0xFF));
  }
}

static void init_framebuffer(void) {
  WinHandle wh;
  BitmapType *bmp;

  wh = WinGetDisplayWindow();
  bmp = WinGetBitmap(wh);
  framebuffer = BmpGetBits(bmp);
  framebuffer += Y0 * bmp->width;
}

static void clear_framebuffer(uint16_t color) {
  for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
    framebuffer[i] = color;
  }
}

static void render_framebuffer(void) {
  pumpkin_screen_dirty(WinGetDisplayWindow(), 0, Y0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

static void draw(void) {
  uint16_t color = htobe16(surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0x36, 0xBB, 0xE0, 0xFF));
  clear_framebuffer(color);

  // Pre-compute sine and cosine of the rotation angle
  float sin_angle = sys_sin(player.angle);
  float cos_angle = sys_cos(player.angle);

  // Left-most point of the FOV
  float pl_x = cos_angle * Z_FAR + sin_angle * Z_FAR;
  float pl_y = sin_angle * Z_FAR - cos_angle * Z_FAR;

  // Right-most point of the FOV
  float pr_x = cos_angle * Z_FAR - sin_angle * Z_FAR;
  float pr_y = sin_angle * Z_FAR + cos_angle * Z_FAR;

  // Loop 320 rays from left to right
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    float delta_x = (pl_x + (pr_x - pl_x) / SCREEN_WIDTH * i) / Z_FAR;
    float delta_y = (pl_y + (pr_y - pl_y) / SCREEN_WIDTH * i) / Z_FAR;

    // Ray (x,y) coords
    float r_x = player.x;
    float r_y = player.y;

    // Store the tallest projected height per-ray
    float tallest_height = SCREEN_HEIGHT;

    // Loop all depth units until the zfar distance limit
    for (int z = 1; z < Z_FAR; z++) {
      r_x += delta_x;
      r_y += delta_y;

      // Find the offset that we have to go and fetch values from the heightmap
      int map_offset = ((MAP_N * ((int)(r_y) & (MAP_N - 1))) + ((int)(r_x) & (MAP_N - 1)));

      // Project height values and find the height on-screen
      int proj_height = (int)((player.height - heightmap[map_offset]) / z * VERTICAL_SCALE_FACTOR + player.pitch);

      // Only draw pixels if the new projected height is taller than the previous tallest height
      if (proj_height < tallest_height) {
        float tilt = (player.roll_vel * (i / (float)SCREEN_WIDTH - 0.5) + 0.5) * SCREEN_HEIGHT / 6.0;

        // Draw pixels from previous max-height until the new projected height
        for (int y = (proj_height + tilt); y < (tallest_height + tilt); y++) {
          if (y >= 0) {
            draw_pixel(framebuffer, i, y, palette[colormap[map_offset]]);
          }
        }
        tallest_height = proj_height;
      }
    }
  }

/*
  // Draw HUD lines on top of the terrain
  color = htobe16(surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0x00, 0xFF, 0x00, 0xFF));
  int mid_width = SCREEN_WIDTH / 2;
  int mid_height = SCREEN_HEIGHT / 2;
  draw_pixel(framebuffer, mid_width, mid_height, color);
  draw_line(framebuffer, mid_width,      mid_height - 10, mid_width,      mid_height - 40, color);
  draw_line(framebuffer, mid_width,      mid_height + 10, mid_width,      mid_height + 40, color);
  draw_line(framebuffer, mid_width - 10, mid_height,      mid_width - 40, mid_height,      color);
  draw_line(framebuffer, mid_width + 10, mid_height,      mid_width + 40, mid_height,      color);
*/

  render_framebuffer();
}

static void process_ext_keys(uint64_t oldMask, uint64_t newMask, int offset) {
  uint64_t diff;
  int i, key;

  diff = oldMask ^ newMask;
  if (diff) {
    for (i = 0; i < 64; i++) {
      if (diff & 1) {
        key = offset + i;
        if (newMask & 1) {
          key_down(key);
        } else {
          key_up(key);
        }
      }
      newMask >>= 1;
      diff >>= 1;
    }
  }
}

static void process_input(void) {
  uint32_t _keyMask, _modMask;
  uint64_t _extKeyMask[2];

  pumpkin_status(NULL, NULL, &_keyMask, &_modMask, NULL, _extKeyMask);

  process_ext_keys(extKeyMask[0], _extKeyMask[0], 0);
  extKeyMask[0] = _extKeyMask[0];

  process_ext_keys(extKeyMask[1], _extKeyMask[1], 64);
  extKeyMask[1] = _extKeyMask[1];
}

static void ShowForm(UInt16 formId) {
  FormType *frm, *previous;

  if ((frm = FrmInitForm(formId)) != NULL) {
    previous = FrmGetActiveForm();
    FrmDoDialog(frm);
    FrmDeleteForm(frm);
    FrmSetActiveForm(previous);
  }
}

static void MenuEvent(UInt16 id) {
  switch (id) {
    case menuAbout:
      ShowForm(AboutForm);
      break;
  }
}

static Boolean MainFormHandleEvent(EventPtr event) {
  FormType *frm;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      handled = true;
      break;
    case nilEvent:
      process_input();
      player_move(&player);
      frm = FrmGetActiveForm();
      if (FrmGetWindowHandle(frm) == WinGetActiveWindow()) {
        draw();
      }
      handled = true;
      break;
    case menuEvent:
      MenuEvent(event->data.menu.itemID);
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventPtr event) {
  FormPtr frm;
  UInt16 form;
  Boolean handled = false;

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

static void EventLoop(void) {
  EventType event;
  Err err;

  is_running = true;

  do {
    EvtGetEvent(&event, 2);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);
  } while (event.eType != appStopEvent && is_running);
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  if (cmd == sysAppLaunchCmdNormalLaunch) {
    load_map();
    init_framebuffer();
    FrmGotoForm(MainForm);
    EventLoop();
    FrmCloseAllForms();
  }

  return 0;
}
