#include "sys.h"
#include "pspi.h"
#include "ads7846.h"
#include "debug.h"

#define CMD_START       0x80
#define CMD_12BIT       0x00
#define CMD_8BIT        0x08
#define CMD_DIFF        0x00
#define CMD_SINGLE      0x04
#define CMD_X_POS       0x10
#define CMD_Z1_POS      0x30
#define CMD_Z2_POS      0x40
#define CMD_Y_POS       0x50
#define CMD_PWD         0x00
#define CMD_ALWAYSON    0x03

static void ads7846_cmd(spi_provider_t *spip, spi_t *spi, uint8_t cmd, int nr, uint8_t *rp) {
  uint8_t b[4], r[4];
  b[0] = cmd;
  b[1] = 0;
  b[2] = 0;
  spip->transfer(spi, b, r, nr+1);
  rp[0] = r[1];
  if (nr == 2) rp[1] = r[2];
}

int ads7846_pressure(spi_provider_t *spip, spi_t *spi) {
  uint8_t a, b;

  ads7846_cmd(spip, spi, CMD_START | CMD_8BIT | CMD_DIFF | CMD_Z1_POS, 1, &a);
  a &= 0x7F;
  ads7846_cmd(spip, spi, CMD_START | CMD_8BIT | CMD_DIFF | CMD_Z2_POS, 1, &b);
  b = (0xFF - b) & 0x7F;
  return a + b;
}

static int ads7846_coord(spi_provider_t *spip, spi_t *spi, uint8_t coord) {
  uint8_t a1, a2, b2, r[2];
  int i;

  for (i = 0; i < 10; i++) {
    ads7846_cmd(spip, spi, CMD_START | CMD_12BIT | CMD_DIFF | coord, 2, r);
    a1 = r[0];
    ads7846_cmd(spip, spi, CMD_START | CMD_12BIT | CMD_DIFF | coord, 2, r);
    a2 = r[0];
    b2 = r[1];
    if (a1 == a2) break;
  }

  return (a2 << 2) | (b2 >> 6);
}

int ads7846_x(spi_provider_t *spip, spi_t *spi) {
  return ads7846_coord(spip, spi, CMD_Y_POS);
}

int ads7846_y(spi_provider_t *spip, spi_t *spi) {
  return ads7846_coord(spip, spi, CMD_X_POS);
}
