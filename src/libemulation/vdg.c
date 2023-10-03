#include "sys.h"
#include "vdg.h"

// mode:
// bit 0: GM0
// bit 1: GM1
// bit 2: GM2
// bit 3: CSS
// bit 4: INT/EXT
// bit 5: A/G

void vdg_byte(vdg_t *v, uint8_t mode, uint16_t a, uint8_t b) {
  uint8_t c1, c2, bg;
  uint32_t x, y;
  int i, j, c;

  if (mode & VDG_AG) {
    // graphic mode
    switch (mode & 0x07) {
      case 0:
        if (a < 1024) {
          x = (a & 0x0F) << 4;
          y = ((a >> 4) & 0x3F) * 3;
          for (i = 0, j = 12; i < 4; i++, j -= 4) {
            c = (b & 0x03) + 1;
            if (mode & VDG_CSS) c += 4;
            v->vdg_clear(v->p, c, x + j, x + j + 4, y, y + 3);
            b >>= 2;
          }
        }
        break;
      case 1:
        if (a < 1024) {
          x = (a & 0x0F) << 4;
          y = ((a >> 4) & 0x3F) * 3;
          for (i = 0, j = 14; i < 8; i++, j -= 2) {
            c = b & 0x01;
            if (c && (mode & VDG_CSS)) c += 4;
            v->vdg_clear(v->p, c, x + j, x + j + 2, y, y + 3);
            b >>= 1;
          }
        }
        break;
      case 2:
        if (a < 2048) {
          x = (a & 0x1F) << 3;
          y = ((a >> 5) & 0x3F) * 3;
          for (i = 0, j = 6; i < 4; i++, j -= 2) {
            c = (b & 0x03) + 1;
            if (mode & VDG_CSS) c += 4;
            v->vdg_clear(v->p, c, x + j, x + j + 2, y, y + 3);
            b >>= 2;
          }
        }
        break;
      case 3:
        if (a < 1536) {
          x = (a & 0x0F) << 4;
          y = ((a >> 4) & 0x7F) * 2;
          if (y < 192) {
            for (i = 0, j = 14; i < 8; i++, j -= 2) {
              c = b & 0x01;
              if (c && (mode & VDG_CSS)) c += 4;
              v->vdg_clear(v->p, c, x + j, x + j + 2, y, y + 2);
              b >>= 1;
            }
          }
        }
        break;
      case 4:
        if (a < 3072) {
          x = (a & 0x1F) << 3;
          y = ((a >> 5) & 0x7F) * 2;
          if (y < 192) {
            for (i = 0, j = 6; i < 4; i++, j -= 2) {
              c = (b & 0x03) + 1;
              if (mode & VDG_CSS) c += 4;
              v->vdg_clear(v->p, c, x + j, x + j + 2, y, y + 2);
              b >>= 2;
            }
          }
        }
        break;
      case 5:
        if (a < 3072) {
          x = (a & 0x0F) << 4;
          y = ((a >> 4) & 0xFF);
          if (y < 192) {
            for (i = 0, j = 14; i < 8; i++, j -= 2) {
              c = b & 0x01;
              if (c && (mode & VDG_CSS)) c += 4;
              v->vdg_clear(v->p, c, x + j, x + j + 2, y, y + 1);
              b >>= 1;
            }
          }
        }
        break;
      case 6:
        if (a < 6144) {
          x = (a & 0x1F) << 3;
          y = ((a >> 5) & 0xFF);
          if (y < 192) {
            for (i = 0, j = 6; i < 4; i++, j -= 2) {
              c = (b & 0x03) + 1;
              if (mode & VDG_CSS) c += 4;
              v->vdg_clear(v->p, c, x + j, x + j + 2, y, y + 1);
              b >>= 2;
            }
          }
        }
        break;
      case 7:
        if (a < 6144) {
          x = (a & 0x1F) << 3;
          y = ((a >> 5) & 0xFF);
          if (y < 192) {
            for (i = 0, j = 7; i < 8; i++, j--) {
              c = b & 0x01;
              if (c && (mode & VDG_CSS)) c += 4;
              v->vdg_clear(v->p, c, x + j, x + j + 1, y, y + 1);
              b >>= 1;
            }
          }
        }
        break;
    }
  } else {
    // text mode
    if (a < 512) {
      bg = (mode & VDG_CSS) ? 8 : 1;
      if (b < 32) {
        b += 32;
        c1 = bg;
        c2 = 0;
      } else if (b < 64) {
        b -= 32;
        c1 = bg;
        c2 = 0;
      } else if (b < 96) {
        b -= 32;
        c1 = 0;
        c2 = bg;
      } else if (b < 128) {
        b -= 96;
        c1 = 0;
        c2 = bg;
      } else {
        if (mode & 0x10) {
          // semi graphics 6
          c1 = ((b >> 6) | ((mode >> 1) & 0x04)) + 1;
          c2 = 0;
          b &= 0xBF;
          b += 32;
        } else {
          // semi graphics 4
          c1 = ((b >> 4) & 7) + 1;
          c2 = 0;
          b = (b & 0x0F) + 96;
        }
      }
      v->vdg_char(v->p, b, c1, c2, a & 0x1F, a >> 5);
    }
  }
}
