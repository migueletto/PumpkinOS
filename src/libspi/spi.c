#include "sys.h"
#include "pspi.h"
#include "pit_io.h"
#include "debug.h"

#if defined(LINUX) || defined(RPI)
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#endif

#define SPI_MASK "/dev/spidev0.%d"

struct spi_t {
  int cs;
  int speed;
  int bits_per_word;
  int fd;
};

spi_t *spi_open(int modeh, int cs, int speedh, void *data) {
  spi_t *spi;
#if defined(LINUX) || defined(RPI)
  char buf[64];
  uint8_t mode, bits;
  uint32_t speed;

  if ((spi = sys_calloc(1, sizeof(spi_t))) == NULL) {
    return NULL;
  }
  spi->cs = cs;
  spi->speed = speedh;
  spi->bits_per_word = 8;

  sys_sprintf(buf, SPI_MASK, cs);
  if ((spi->fd = sys_open(buf, SYS_RDWR)) == -1) {
    debug_errno("SPI", "open \"%s\"", buf);
    sys_free(spi);
    return NULL;
  }
  debug(DEBUG_INFO, "SPI", "device \"%s\" open", buf);

  switch (modeh) {
    case 0:  mode = SPI_MODE_0; break;
    case 1:  mode = SPI_MODE_1; break;
    case 2:  mode = SPI_MODE_2; break;
    default: mode = SPI_MODE_0;
  }

  if (ioctl(spi->fd, SPI_IOC_WR_MODE, &mode) == -1) {
    debug_errno("SPI", "write mode");
    sys_close(spi->fd);
    sys_free(spi);
    return NULL;
  }

  if (ioctl(spi->fd, SPI_IOC_RD_MODE, &mode) == -1) {
    debug_errno("SPI", "read mode");
    sys_close(spi->fd);
    sys_free(spi);
    return NULL;
  }
  debug(DEBUG_INFO, "SPI", "mode set to %d", modeh);

  bits = spi->bits_per_word;
  if (ioctl(spi->fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1) {
    debug_errno("SPI", "set bits per word");
    sys_close(spi->fd);
    sys_free(spi);
    return NULL;
  }

  bits = spi->bits_per_word;
  if (ioctl(spi->fd, SPI_IOC_RD_BITS_PER_WORD, &bits) == -1) {
    debug_errno("SPI", "get bits per word");
    sys_close(spi->fd);
    sys_free(spi);
    return NULL;
  }
  debug(DEBUG_INFO, "SPI", "bits per word set to %d", bits);

  speed = spi->speed; // Hz
  if (ioctl(spi->fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
    debug_errno("SPI", "set max speed hz");
    sys_close(spi->fd);
    sys_free(spi);
    return NULL;
  }

  speed = spi->speed; // Hz
  if (ioctl(spi->fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) == -1) {
    debug_errno("SPI", "get max speed hz");
    sys_close(spi->fd);
    sys_free(spi);
    return NULL;
  }
  debug(DEBUG_INFO, "SPI", "speed set to %d", speed);
#else
  spi = NULL;
#endif

  return spi;
}

int spi_close(spi_t *spi) {
  if (spi) {
#if defined(LINUX) || defined(RPI)
    if (spi->fd) {
      sys_close(spi->fd);
    }
#endif
    sys_free(spi);
  }

  return 0;
}

int spi_transfer(spi_t *spi, uint8_t *txbuf, uint8_t *rxbuf, int len) {
#if defined(LINUX) || defined(RPI)
/*
  struct spi_ioc_transfer xfer[2];

  sys_memset(xfer, 0, sizeof(xfer));
  xfer[0].tx_buf = (unsigned long)txbuf;
  xfer[0].len = len;
  xfer[0].speed_hz = spi->speed;
  xfer[0].bits_per_word = spi->bits_per_word;
  xfer[0].cs_change = 1;
  
  // XXX for SSD1331 rxbuf must be null
  xfer[1].rx_buf = (unsigned long)rxbuf;
  xfer[1].len = len;
  xfer[1].speed_hz = spi->speed;
  xfer[1].bits_per_word = spi->bits_per_word;
  xfer[1].cs_change = 1;

  if (ioctl(spi->fd, SPI_IOC_MESSAGE((rxbuf ? 2 : 1)), xfer) == -1) {
    debug_errno("SPI", "transfer ioctl");
    return -1;
  }
*/
  struct spi_ioc_transfer xfer;
  
  sys_memset(&xfer, 0, sizeof(xfer));
  xfer.tx_buf = (uintptr_t)txbuf;
  xfer.rx_buf = (uintptr_t)rxbuf;
  xfer.len = len;
  
  if (ioctl(spi->fd, SPI_IOC_MESSAGE(1), &xfer) == -1) {
    debug_errno("SPI", "transfer ioctl");
    return -1;
  }
  debug(DEBUG_TRACE, "SPI", "transfer ioctl %d bytes", len);

  return len;
#else
  return -1;
#endif
}
