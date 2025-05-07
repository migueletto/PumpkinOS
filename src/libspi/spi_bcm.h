int spi_bcm_begin(void);
int spi_bcm_end(void);
spi_t *spi_bcm_open(int modeh, int cs, int speedh, void *data);
int spi_bcm_close(spi_t *spi);
int spi_bcm_transfer(spi_t *spi, uint8_t *txbuf, uint8_t *rxbuf, int len);

