#define VDG_CSS 0x08
#define VDG_AG  0x20

typedef struct {
  void (*vdg_char)(void *p, uint8_t b, uint8_t fg, uint8_t bg, uint8_t col, uint8_t row);
  void (*vdg_clear)(void *p, uint8_t c, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);
  void *p;
  uint8_t c1, c2, c3;
  uint8_t artifacting;
} vdg_t;

void vdg_byte(vdg_t *v, uint8_t mode, uint16_t a, uint8_t b);
