#include "sys.h"
#include "thread.h"
#include "ptr.h"
#include "vfs.h"
#include "vfslocal.h"
#include "disk.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "pwindow.h"
#include "script.h"
#include "debug.h"
#ifdef RPI
#include "gpio.h"
#include "pspi.h"
#include "bcm2835.h"
#endif

extern int libos_app_init(int pe);
extern int libos_start(int pe);
extern void malloc_init(void);

#ifdef RPI
extern int libgpio_load(void);
extern int gpio_select(int pe, char *type);

extern int libspi_load(void);
extern int libspi_unload(void);
extern int libspi_init(int pe, script_ref_t obj);

extern surface_t *libili9486_create_surface(int spi_cs, int rs_pin, int rst_pin, int spi_speed, int width, int height, spi_provider_t *spip, gpio_provider_t *gpiop);
extern void surface_destructor(void *p);
extern int libwsurface_load(void);
extern int libwsurface_init(int pe, script_ref_t obj);
extern void libwsurface_set_surface(int ptr);
extern int libwsurface_window_erase(window_t *_window, uint32_t bg);

static void blink(void) {
  bcm2835_gpio_write(16, HIGH);
  bcm2835_delay(1000);
  bcm2835_gpio_write(16, LOW);
  bcm2835_delay(1000);
}

#else
extern int liblsdl2_load(void);
extern int liblsdl2_init(int pe, script_ref_t obj);

static void blink(void) {
}
#endif

int main(void) {
#ifdef RPI
  gpio_provider_t *gpiop;
  spi_provider_t *spip;
  gpio_t *gpio;
  surface_t *s;
  int ptr;

  bcm2835_init();

  bcm2835_gpio_fsel(16, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_write(16, LOW);
#endif

  blink();
  blink();
  malloc_init();
  blink();
  debug_setsyslevel(NULL, DEBUG_INFO);
  blink();
  //debug_setsyslevel("STOR", DEBUG_TRACE);
  debug_init(NULL);
  blink();
  ptr_init();
  blink();
  thread_init();
  blink();

  disk_init();
  blink();
  vfs_init();
  blink();

#ifdef RPI
  libgpio_load();
  gpio_select(0, "bcm");
  gpiop = script_get_pointer(0, GPIO_PROVIDER);
  blink();

  gpio = gpiop->open(gpiop->data);
  gpiop->setup(gpio, 16, GPIO_OUT);
  blink();

  libspi_load();
  libspi_init(0, 0);
  spip = script_get_pointer(0, SPI_PROVIDER);
  blink();

  libwsurface_load();
  blink();
  libwsurface_init(0, 0);
  blink();

  s = libili9486_create_surface(0, 24, 25, 16000000, 320, 480, spip, gpiop);
  blink();
  ptr = ptr_new(s, surface_destructor);
  blink();
  libwsurface_set_surface(ptr);
  blink();
#else
  liblsdl2_load();
  liblsdl2_init(0, 0);
#endif

  vfs_local_mount("./vfs/", "/");
  blink();
  libos_app_init(0);
  blink();

  libos_start(0);

#ifdef RPI
  libwsurface_window_erase(NULL, 0);
  ptr_free(ptr, s->tag);
  libspi_unload();
  bcm2835_close();
#endif

  vfs_finish();
  thread_close();
  debug_close();

  return 0;
}
