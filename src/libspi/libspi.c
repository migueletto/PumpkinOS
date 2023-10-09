#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "sys.h"
#include "script.h"
#include "pspi.h"
#include "ptr.h"
#include "debug.h"
#include "xalloc.h"

#define TAG_SPI "spi"

// cat /sys/module/spidev/parameters/bufsiz
// /boot/cmdline.txt: spidev.bufsiz=256000

typedef struct {
  char *tag;
  spi_provider_t *spip;
  spi_t *spi;
} libspi_t;

static spi_provider_t provider;

static void spi_destructor(void *p) {
  libspi_t *spi;

  spi = (libspi_t *)p;
  if (spi) {
    if (spi->spip && spi->spip->close) spi->spip->close(spi->spi);
    xfree(spi);
  }
}

static int libspi_open(int pe) {
  script_int_t cs, speed;
  libspi_t *spi;
  int ptr, r;

  r = -1;

  if (script_get_integer(pe, 0, &cs) == 0 &&
      script_get_integer(pe, 1, &speed) == 0) {

    if ((spi = xcalloc(1, sizeof(libspi_t))) != NULL) {
      spi->spip = script_get_pointer(pe, SPI_PROVIDER);
      if (spi->spip && spi->spip->open && (spi->spi = spi->spip->open(cs, speed, spi->spip->data)) != NULL) {
        spi->tag = TAG_SPI;

        if ((ptr = ptr_new(spi, spi_destructor)) != -1) {
          r = script_push_integer(pe, ptr);
        } else {
          spi_close(spi->spi);
          xfree(spi);
        }
      } else {
        xfree(spi);
      }
    }
  }

  return r;
}

static int libspi_close(int pe) {
  script_int_t ptr;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    if (ptr_free(ptr, TAG_SPI) == 0) {
      r = script_push_boolean(pe, 1);
    }
  }

  return r;
}

static int libspi_transfer(int pe) {
  script_int_t ptr;
  spi_provider_t *spip;
  libspi_t *spi;
  char *txbuf = NULL, *rxbuf;
  int len, n, r = -1;

  if ((spip = script_get_pointer(pe, SPI_PROVIDER)) != NULL) {
    if (script_get_integer(pe, 0, &ptr) == 0 &&
        script_get_lstring(pe, 1, &txbuf, &len) == 0) {

      if ((rxbuf = xcalloc(1, len)) != NULL) {
        if ((spi = ptr_lock(ptr, TAG_SPI)) != NULL) {
          n = spip->transfer ? spip->transfer(spi->spi, (uint8_t *)txbuf, (uint8_t *)rxbuf, len) : -1;
          ptr_unlock(ptr, TAG_SPI);
          if (n == len) {
            r = script_push_lstring(pe, rxbuf, len);
          }
        }
        xfree(rxbuf);
      }
    }
    if (txbuf) xfree(txbuf);
  }

  return r;
}

int libspi_load(void) {
  xmemset(&provider, 0, sizeof(spi_provider_t));
  provider.open = spi_open;
  provider.close = spi_close;
  provider.transfer = spi_transfer;

  return 0;
}

int libspi_init(int pe, script_ref_t obj) {
  if (script_get_pointer(pe, SPI_PROVIDER) == NULL) {
    if (spi_begin() == -1) {
      debug(DEBUG_ERROR, "SPI", "failed to init spi");
      xmemset(&provider, 0, sizeof(spi_provider_t));
      return -1;
    }
    debug(DEBUG_INFO, "SPI", "registering %s", SPI_PROVIDER);
    script_set_pointer(pe, SPI_PROVIDER, &provider);
  }

  script_add_function(pe, obj, "open",     libspi_open);
  script_add_function(pe, obj, "close",    libspi_close);
  script_add_function(pe, obj, "transfer", libspi_transfer);

  return 0;
}

int libspi_unload(void) {
  if (provider.open) {
    spi_end();
  }
  return 0;
}
