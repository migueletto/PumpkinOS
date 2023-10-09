#include "sys.h"
#include "pspi.h"
#include "pit_io.h"
#include "debug.h"
#include "xalloc.h"

#ifdef LINUX
#ifdef BCM2835
#include "bcm2835.h"
#else
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#endif
#else
#define ioctl(a,b,c) -1
#define SPI_MODE_0 0
#endif

#define SPI_MASK "/dev/spidev0.%d"

#define SIM_SPI  "spi"

struct spi_t {
  int cs;
  int speed;
  int fd;
};

int spi_begin(void) {
#ifdef BCM2835
  if (!bcm2835_init()) {
    debug(DEBUG_ERROR, "SPI", "failed to init lib bcm2835");
    return -1;
  }
  debug(DEBUG_INFO, "SPI", "using lib bcm2835");

  if (!bcm2835_spi_begin()) {
    debug(DEBUG_ERROR, "SPI", "failed to init bcm2835 spi");
    return -1;
  }

  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
#else
  debug(DEBUG_INFO, "SPI", "using spidev");
#endif

  return 0;
}

int spi_end(void) {
#ifdef BCM2835
  bcm2835_spi_end();
#endif

  return 0;
}

spi_t *spi_open(int cs, int speedh, void *data) {
  spi_t *spi;
#ifndef BCM2835
  char buf[64];
  uint8_t mode, bits;
  uint32_t speed;
#endif

  if ((spi = xcalloc(1, sizeof(spi_t))) == NULL) {
    return NULL;
  }
  spi->cs = cs;
  spi->speed = speedh;

#ifndef BCM2835
  sprintf(buf, SPI_MASK, cs);
  if ((spi->fd = sys_open(buf, SYS_RDWR)) == -1) {
    debug_errno("SPI", "open \"%s\"", buf);
    xfree(spi);
    return NULL;
  }
  debug(DEBUG_INFO, "SPI", "device \"%s\" open", buf);

  mode = SPI_MODE_0;
  if (ioctl(spi->fd, SPI_IOC_WR_MODE, &mode) == -1) {
    debug_errno("SPI", "set mode");
    sys_close(spi->fd);
    xfree(spi);
    return NULL;
  }

  if (ioctl(spi->fd, SPI_IOC_RD_MODE, &mode) == -1) {
    debug_errno("SPI", "get mode");
    sys_close(spi->fd);
    xfree(spi);
    return NULL;
  }
  debug(DEBUG_INFO, "SPI", "mode set to %d", mode);

  bits = 8;
  if (ioctl(spi->fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1) {
    debug_errno("SPI", "set bits per word");
    sys_close(spi->fd);
    xfree(spi);
    return NULL;
  }

  if (ioctl(spi->fd, SPI_IOC_RD_BITS_PER_WORD, &bits) == -1) {
    debug_errno("SPI", "get bits per word");
    sys_close(spi->fd);
    xfree(spi);
    return NULL;
  }
  debug(DEBUG_INFO, "SPI", "bits per word set to %d", bits);

  speed = speedh; // Hz
  if (ioctl(spi->fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
    debug_errno("SPI", "set max speed hz");
    sys_close(spi->fd);
    xfree(spi);
    return NULL;
  }

  if (ioctl(spi->fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) == -1) {
    debug_errno("SPI", "get max speed hz");
    sys_close(spi->fd);
    xfree(spi);
    return NULL;
  }
  debug(DEBUG_INFO, "SPI", "speed set to %d", speed);
#endif

  return spi;
}

int spi_close(spi_t *spi) {
  if (spi) {
#ifndef BCM2835
    if (spi->fd) {
      sys_close(spi->fd);
    }
#endif
    xfree(spi);
  }

  return 0;
}

#ifdef BCM2835
static int spi_select(spi_t *spi) {
  switch (spi->cs) {
    case 0:
      bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
      return 0;
    case 1:
      bcm2835_spi_chipSelect(BCM2835_SPI_CS1);
      return 0;
  }

  if (spi->speed >= 64000000) {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4);
  } else if (spi->speed >= 32000000) {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_8);
  } else if (spi->speed >= 16000000) {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16);
  } else if (spi->speed >= 8000000) {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);
  } else if (spi->speed >= 4000000) {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);
  } else if (spi->speed >= 2000000) {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);
  } else if (spi->speed >= 1000000) {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);
  } else if (spi->speed >= 512000) {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_512);
  } else if (spi->speed >= 256000) {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_1024);
  } else if (spi->speed >= 128000) {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_2048);
  } else if (spi->speed >= 64000) {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4096);
  } else if (spi->speed >= 32000) {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_8192);
  } else if (spi->speed >= 16000) {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16384);
  } else {
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32768);
  }

  return -1;
}
#endif

int spi_transfer(spi_t *spi, uint8_t *txbuf, uint8_t *rxbuf, int len) {
#ifdef LINUX
#ifdef BCM2835
  // XXX testar
  spi_select(spi);
  bcm2835_spi_transfernb((char *)txbuf, (char *)rxbuf, len);
  return len;
#else
  struct spi_ioc_transfer xfer[2];

  memset(xfer, 0, sizeof(xfer));
  xfer[0].tx_buf = (unsigned long)txbuf;
  xfer[0].len = len;
  xfer[0].speed_hz = spi->speed;
  xfer[0].bits_per_word = 8;
  xfer[0].cs_change = 1;
  
  // XXX for SSD1331 rxbuf must be null
  xfer[1].rx_buf = (unsigned long)rxbuf;
  xfer[1].len = len;
  xfer[1].speed_hz = spi->speed;
  xfer[1].bits_per_word = 8;
  xfer[1].cs_change = 1;

  if (ioctl(spi->fd, SPI_IOC_MESSAGE((rxbuf ? 2 : 1)), xfer) == -1) {
    debug_errno("SPI", "transfer ioctl");
    return -1;
  }
  debug(DEBUG_TRACE, "SPI", "transfer ioctl %d bytes", len);

  return len;
#endif
#else
  return -1;
#endif
}
