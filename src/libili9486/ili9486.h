int ili9486_begin(spi_provider_t *spip, spi_t *spi, gpio_provider_t *gpiop, gpio_t *gpio, int dc_pin, int rst_pin);
int ili9486_enable(spi_provider_t *spip, spi_t *spi, gpio_provider_t *gpiop, gpio_t *gpio, int dc_pin, int enable);
int ili9486_draw(spi_provider_t *spip, spi_t *spi, gpio_provider_t *gpiop, gpio_t *gpio, int dc_pin, uint16_t *pic, int x, int y, int width, int height, uint16_t *aux);
uint16_t ili9486_color565(uint8_t r, uint8_t g, uint8_t b);
void ili9486_rgb565(uint32_t c, uint8_t *r, uint8_t *g, uint8_t *b);
