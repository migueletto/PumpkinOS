#include "sys.h"
#include "pspi.h"
#include "bcm2835.h"
#include "debug.h"

struct spi_t {
  int cs;
  int speed;
  int fd;
};

int spi_bcm_begin(void) {
  debug(DEBUG_INFO, "SPI", "using lib bcm2835");

  if (!bcm2835_spi_begin()) {
    debug(DEBUG_ERROR, "SPI", "failed to init bcm2835 spi");
    return -1;
  }

  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);

  return 0;
}

int spi_bcm_end(void) {
  bcm2835_spi_end();

  return 0;
}

spi_t *spi_bcm_open(int modeh, int cs, int speedh, void *data) {
  spi_t *spi;
  uint8_t mode;

  if ((spi = sys_calloc(1, sizeof(spi_t))) == NULL) {
    return NULL;
  }
  spi->cs = cs;
  spi->speed = speedh;

  switch (modeh) {
    case 0:  mode = BCM2835_SPI_MODE0; break;
    case 1:  mode = BCM2835_SPI_MODE0; break;
    case 2:  mode = BCM2835_SPI_MODE0; break;
    default: mode = BCM2835_SPI_MODE0;
  }

  bcm2835_spi_setDataMode(mode);

  return spi;
}

int spi_bcm_close(spi_t *spi) {
  if (spi) {
    sys_free(spi);
  }

  return 0;
}

static int spi_bcm_select(spi_t *spi) {
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

int spi_bcm_transfer(spi_t *spi, uint8_t *txbuf, uint8_t *rxbuf, int len) {
  spi_bcm_select(spi);
  bcm2835_spi_transfernb((char *)txbuf, (char *)rxbuf, len);

  return len;
}
