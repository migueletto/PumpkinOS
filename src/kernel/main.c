#include "sys.h"
#include "dlm.h"
#include "thread.h"
#include "ptr.h"
#include "vfs.h"
#include "vfslocal.h"
#include "disk.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "script.h"
#include "debug.h"
#ifdef RPI
#include "gpio.h"
#include "pspi.h"
#include "bcm2835.h"
#endif

extern int libos_app_init(int pe);
extern int libos_start(int pe);

#ifdef RPI
extern int libgpio_load(void);
extern int gpio_select(int pe, char *type);

extern int libspi_load(void);
extern int libspi_init(int pe, script_ref_t obj);

extern surface_t *libili9486_create_surface(int spi_cs, int rs_pin, int rst_pin, int spi_speed, int width, int height, spi_provider_t *spip, gpio_provider_t *gpiop);
extern void surface_destructor(void *p);
extern int libwsurface_load(void);
extern int libwsurface_init(int pe, script_ref_t obj);
extern void libwsurface_set_surface(int ptr);

#else

extern int liblsdl2_load(void);
extern int liblsdl2_init(int pe, script_ref_t obj);

#endif

int main(int argc, char *argv[]) {
#ifdef RPI
  gpio_provider_t *gpiop;
  spi_provider_t *spip;
  surface_t *s;
  int ptr;
#endif

  dlm_init(NULL);
  sys_init();
  debug_setsyslevel(NULL, DEBUG_INFO);
  //debug_setsyslevel("STOR", DEBUG_TRACE);
  debug_init(NULL);
  ptr_init();
  thread_init();

  disk_init();
  vfs_init();

#ifdef RPI
  bcm2835_init();
  libgpio_load();
  gpio_select(0, "sys");

  libspi_load();
  libspi_init(0, 0);

  gpiop = script_get_pointer(0, GPIO_PROVIDER);
  spip = script_get_pointer(0, SPI_PROVIDER);

  libwsurface_load();
  libwsurface_init(0, 0);

  s = libili9486_create_surface(0, 536, 537, 16000000, 320, 480, spip, gpiop);
  ptr = ptr_new(s, surface_destructor);
  libwsurface_set_surface(ptr);
#else
  liblsdl2_load();
  liblsdl2_init(0, 0);
#endif

  vfs_local_mount("./vfs/", "/");
  libos_app_init(0);
  libos_start(0);

#ifdef RPI
  ptr_free(ptr, s->tag);
  bcm2835_close();
#endif

  vfs_finish();
  thread_close();
  debug_close();

  return 0;
}
