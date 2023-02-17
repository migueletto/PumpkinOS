#define GRAIL_CAPS      0x01
#define GRAIL_CAPSLOCK  0x02
#define GRAIL_PUNCT     0x04

int grail_begin(int alpha);
int grail_stroke(int x, int y);
int grail_end(uint32_t *state);
void grail_reset(void);
void grail_draw_stroke(int x1, int y1, int x2, int y2, int alpha);
