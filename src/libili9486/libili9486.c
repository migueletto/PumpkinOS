#include "sys.h"
#include "script.h"
#include "ptr.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "gpio.h"
#include "pspi.h"
#include "ili9486.h"
#include "debug.h"

typedef struct {
  spi_provider_t *spip;
  gpio_provider_t *gpiop;
  gpio_t *gpio;
  spi_t *spi;
  int rs_pin;
  int width, height, len;
  int y0, y1;
  uint16_t *buf;
  uint16_t *aux;
} libili9486_t;

static void *libili9486_getbuffer(void *data, int *len) {
  libili9486_t *ili9486 = (libili9486_t *)data;

  if (len) *len = ili9486->len;

  return ili9486->buf;
}

static void libili9486_update(void *data, int y0, int y1) {
  libili9486_t *ili9486 = (libili9486_t *)data;
  int total, dy, y;

  ili9486->y0 = y0;
  ili9486->y1 = y1;

  if (ili9486->y0 < ili9486->y1) {
    total = ili9486->y1 - ili9486->y0 + 1;
    for (y = 0; total > 0; y += dy) {
      dy = total < 256 ? total : 256;
      ili9486_draw(ili9486->spip, ili9486->spi, ili9486->gpiop, ili9486->gpio, ili9486->rs_pin, &ili9486->buf[(ili9486->y0 + y) * ili9486->width], 0, ili9486->y0 + y, ili9486->width, dy, ili9486->aux);
      total -= dy;
    }
  }

  ili9486->y0 = ili9486->height;
  ili9486->y1 = -1;
}

static void libili9486_setpixel(void *data, int x, int y, uint32_t color) {
  libili9486_t *ili9486 = (libili9486_t *)data;
  if (x >= 0 && x < ili9486->width && y >= 0 && y < ili9486->height) {
    if (y < ili9486->y0) ili9486->y0 = y;
    if (y > ili9486->y1) ili9486->y1 = y;
    ili9486->buf[y * ili9486->width + x] = color;
  }
}

static int libili9486_event(void *data, uint32_t us, int *arg1, int *arg2) {
  libili9486_t *ili9486 = (libili9486_t *)data;
  return ili9486->spip->event(ili9486->spi, us, arg1, arg2);
}

static void libili9486_rgb_color(void *data, uint32_t color, int *red, int *green, int *blue, int *alpha) {
  uint8_t r, g, b;
  ili9486_rgb565(color, &r, &g, &b);
  *red = r;
  *green = g;
  *blue = b;
}

static uint32_t libili9486_color_rgb(void *data, int red, int green, int blue, int alpha) {
  return ili9486_color565(red, green, blue);
}

#if !defined(KERNEL)
static
#endif
void surface_destructor(void *p) {
  surface_t *surface = (surface_t *)p;
  libili9486_t *ili9486;

  if (surface) {
    ili9486 = (libili9486_t *)surface->data;
    ili9486_enable(ili9486->spip, ili9486->spi, ili9486->gpiop, ili9486->gpio, ili9486->rs_pin, 0);
    sys_free(ili9486->buf);
    sys_free(ili9486->aux);
    ili9486->spip->close(ili9486->spi);
    ili9486->gpiop->close(ili9486->gpio);
    sys_free(ili9486);
    sys_free(surface);
  }
}

#if !defined(KERNEL)
static
#endif
surface_t *libili9486_create_surface(int spi_cs, int rs_pin, int rst_pin, int spi_speed, int width, int height, spi_provider_t *spip, gpio_provider_t *gpiop) {
  surface_t *surface;
  libili9486_t *ili9486;

  if ((surface = sys_calloc(1, sizeof(surface_t))) != NULL) {
    if ((ili9486 = sys_calloc(1, sizeof(libili9486_t))) != NULL) {
      if ((ili9486->spi = spip->open(0, spi_cs, spi_speed, spip->data)) != NULL) {
        ili9486->gpiop = gpiop;
        ili9486->spip = spip;
        ili9486->gpio = gpiop->open(gpiop->data);
        gpiop->setup(ili9486->gpio, rs_pin, GPIO_OUT);
        gpiop->setup(ili9486->gpio, rst_pin, GPIO_OUT);

        if (ili9486_begin(ili9486->spip, ili9486->spi, gpiop, ili9486->gpio, rs_pin, rst_pin) == 0) {
          ili9486->width = width;
          ili9486->height = height;
          ili9486->rs_pin = rs_pin;
          ili9486->len = ili9486->width * ili9486->height * sizeof(uint16_t);
          ili9486->buf = sys_calloc(1, ili9486->len);
          ili9486->aux = sys_calloc(1, ili9486->len);
          ili9486->y0 = ili9486->height;
          ili9486->y1 = -1;
          surface->width = ili9486->width;
          surface->height = ili9486->height;
          surface->encoding = SURFACE_ENCODING_RGB565;
          surface->data = ili9486;
          surface->setpixel = libili9486_setpixel;
          surface->rgb_color = libili9486_rgb_color;
          surface->color_rgb = libili9486_color_rgb;
          surface->getbuffer = libili9486_getbuffer;
          surface->update = libili9486_update;
          surface->event = spip->event ? libili9486_event : NULL;
          surface->tag = TAG_SURFACE;
        } else {
          debug(DEBUG_ERROR, "ILI9486", "ili9486_begin failed");
          gpiop->open(ili9486->gpio);
          spip->close(ili9486->spi);
          sys_free(ili9486);
          sys_free(surface);
          surface = NULL;
        }
      } else {
        sys_free(ili9486);
        sys_free(surface);
        surface = NULL;
      }
    } else {
      sys_free(surface);
      surface = NULL;
    }
  }

  return surface;
}

#if !defined(KERNEL)
static int libili9486_surface(int pe) {
  spi_provider_t *spip;
  gpio_provider_t *gpiop;
  surface_t *surface;
  script_int_t spi_cs, rs_pin, rst_pin, spi_speed, width, height;
  int ptr, r = -1;

  if (script_get_integer(pe, 0, &spi_cs) == 0 &&
      script_get_integer(pe, 1, &rs_pin) == 0 &&
      script_get_integer(pe, 2, &rst_pin) == 0 &&
      script_get_integer(pe, 3, &spi_speed) == 0 &&
      script_get_integer(pe, 4, &width) == 0 &&
      script_get_integer(pe, 5, &height) == 0) {

    gpiop = script_get_pointer(pe, GPIO_PROVIDER);
    spip = script_get_pointer(pe, SPI_PROVIDER);

    if (!gpiop) {
      debug(DEBUG_ERROR, "ILI9486", "gpio provider not found");
      return -1;
    }

    if (!spip) {
      debug(DEBUG_ERROR, "ILI9486", "spi provider not found");
      return -1;
    }

    surface = libili9486_create_surface(spi_cs, rs_pin, rst_pin, spi_speed, width, height, spip, gpiop);

    if (surface != NULL) {
      if ((ptr = ptr_new(surface, surface_destructor)) != -1) {
        r = script_push_integer(pe, ptr);
      } else {
        surface_destructor(surface);
      }
    }
  }

  return r;
}

int libili9486_init(int pe, script_ref_t obj) {
  script_add_function(pe, obj, "surface", libili9486_surface);

  return 0;
}
#endif

/*
Raspberry Pi 26-pins GPIO header assignment:

1, 17: 3.3V power
2, 4: 5V power
3, 5, 7, 8, 10, 12, 13, 15, 16: not connected
6, 9, 14, 20, 25: ground
11: TP_IRQ, touch panel interrupt, low if the touch Panel detects touching
18: LCD_RS, LCD command/data register selection
19: LCD_SI/TP_SI, SPI data input (LCD and touch panel)
21: TP_SO, SPI data output (touch Panel)
22: reset
23: LCD_SCK/TP_SCK, SPI clock
24: LCD_CS, LCD chip selection, active low
26: TP_CS, touch panel chip selection, active low
*/
