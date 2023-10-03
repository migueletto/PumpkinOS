#include <PalmOS.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "script.h"
#include "pwindow.h"
#include "pumpkin.h"
#include "sys.h"
#include "endianness.h"
#include "debug.h"
#include "resource.h"

#include "display.h"
#include "input.h"
#include "player.h"
#include "resource.h"
#include "noise.h"
#include "world.h"

#define Z_FAR 400.0
#define VERTICAL_SCALE_FACTOR 35.0

#define Y0 30

#define MARGIN 32

static uint16_t *framebuffer = NULL;
static uint16_t palette[256];
static uint64_t extKeyMask[2];
static bool is_running = false;

static world_t world = {
  .height_freq = 0.02,
  .height_depth = 4,
  .height_seed = 7,
  .color_freq = 0.02,
  .color_depth = 4,
  .color_seed = 17,
  .sea_level = 100,

  .palette = {
     // water
     0x00, 0x40, 0xff,
     0x00, 0x48, 0xff,
     // sand
     0x80, 0x80, 0x00,
     0x78, 0x78, 0x00,
  }
};

static player_t player = {
  .x           =   0.0,
  .y           =   0.0,
  .height      = 200.0,
  .angle       =   0.0,
  .pitch       =   0.0,
  .forward_vel =   0.0,   .forward_acc = 0.06,   .forward_brk = 0.06,   .forward_max = 3.0,
  .pitch_vel   =   0.0,   .pitch_acc   = 0.06,   .pitch_brk   = 0.10,   .pitch_max   = 2.0,
  .yaw_vel     =   0.0,   .yaw_acc     = 0.10,   .yaw_brk     = 0.10,   .yaw_max     = 2.0,
  .lift_vel    =   0.0,   .lift_acc    = 0.06,   .lift_brk    = 0.07,   .lift_max    = 2.0,
  .strafe_vel  =   0.0,   .strafe_acc  = 0.05,   .strafe_brk  = 0.09,   .strafe_max  = 2.0,
  .roll_vel    =   0.0,   .roll_acc    = 0.04,   .roll_brk    = 0.09,   .roll_max    = 2.0
};

static void init_palette(void) {
  int pal_count = 0;

  // water
  palette[pal_count++] = sys_htobe16(surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0x00, 0x40, 0xff, 0xff));
  palette[pal_count++] = sys_htobe16(surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0x00, 0x48, 0xff, 0xff));

  // sand
  palette[pal_count++] = sys_htobe16(surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0x80, 0x80, 0x00, 0xff));
  palette[pal_count++] = sys_htobe16(surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0x78, 0x78, 0x00, 0xff));

  // terrain 1
  for (int i = 0; i < 64; i++) {
    palette[pal_count++] = sys_htobe16(surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0x10, 0x80-i, 0x00, 0xff));
  }

  // terrain 2
  for (int i = 0; i < 64; i++) {
    palette[pal_count++] = sys_htobe16(surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0x40+i, 0x20+i, 0x00, 0xff));
  }

  // sky
  palette[pal_count++] = sys_htobe16(surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0x36, 0xBB, 0xE0, 0xFF));
}

static uint8_t get_heightmap(int x, int y) {
  return (uint8_t)(noise(x, y, world.height_freq, world.height_depth, world.height_seed) * 255);
}

static uint8_t get_colormap(int x, int y, uint8_t h) {
  uint8_t m;

  if (h <= world.sea_level) {
    // water
    m = h & 1;
  } else if (h < world.sea_level+4) {
    // sand
    m = 2 + (h & 1);
  } else {
    // terrain
    m = 4 + (uint8_t)(noise(x, y, world.color_freq, world.color_depth, world.color_seed) * 127);
  }

  return m;
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
  for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT/2; i++) {
    framebuffer[i] = color;
  }
}

static void render_framebuffer(void) {
  pumpkin_screen_dirty(WinGetDisplayWindow(), 0, Y0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

static void draw(void) {
  clear_framebuffer(palette[132]);

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

      int ix;
      if (r_x >= MAX_X) {
        ix = r_x - MAX_X;
      } else if (r_x < 0) {
        ix = r_x + MAX_X;
      } else {
        ix = r_x;
      }

      int iy;
      if (r_y >= MAX_Y) {
        iy = r_y - MAX_Y;
      } else if (r_y < 0) {
        iy = r_y + MAX_Y;
      } else {
        iy = r_y;
      }

      // Project height values and find the height on-screen
      uint8_t h0 = get_heightmap(ix, iy);
      if (h0 > world.sea_level) {
        if (r_x >= 0 && r_x < MARGIN) h0 = world.sea_level + (uint8_t)(((h0 - world.sea_level) * r_x) / MARGIN) - 4;
        else if (r_x > -MARGIN && r_x <= 0) h0 = world.sea_level + (uint8_t)(((h0 - world.sea_level) * -r_x) / MARGIN) - 4;
      }
      if (h0 > world.sea_level) {
        if (r_y >= 0 && r_y < MARGIN) h0 = world.sea_level + (uint8_t)(((h0 - world.sea_level) * r_y) / MARGIN) - 4;
        else if (r_y > -MARGIN && r_y <= 0) h0 = world.sea_level + (uint8_t)(((h0 - world.sea_level) * -r_y) / MARGIN) - 4;
      }
      uint8_t h = (h0 < world.sea_level) ? world.sea_level : h0;
      int proj_height = (int)((player.height - h) / z * VERTICAL_SCALE_FACTOR + player.pitch);

      // Only draw pixels if the new projected height is taller than the previous tallest height
      if (proj_height < tallest_height) {
        float tilt = (player.roll_vel * (i / (float)SCREEN_WIDTH - 0.5) + 0.5) * SCREEN_HEIGHT / 6.0;

        // Draw pixels from previous max-height until the new projected height
        uint16_t c = palette[get_colormap(ix, iy, h0)];
        for (int y = (proj_height + tilt); y < (tallest_height + tilt); y++) {
          if (y >= 0) {
            draw_pixel(framebuffer, i, y, c);
          }
        }
        tallest_height = proj_height;
      }
    }
  }

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
    EvtGetEvent(&event, 5);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);
  } while (event.eType != appStopEvent && is_running);
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  if (cmd == sysAppLaunchCmdNormalLaunch) {
    init_palette();
    init_framebuffer();
    FrmGotoForm(MainForm);
    EventLoop();
    FrmCloseAllForms();
  }

  return 0;
}
