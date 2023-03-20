typedef struct wman_t wman_t;

wman_t *wman_init(window_provider_t *wp, window_t *w, int width, int height);
int wman_set_background(wman_t *wm, int depth, uint8_t r, uint8_t g, uint8_t b);
int wman_set_border(wman_t *wm, int depth, int size, uint8_t rsel, uint8_t gsel, uint8_t bsel, uint8_t r, uint8_t g, uint8_t b);
int wman_add(wman_t *wm, int id, texture_t *t, int x, int y, int w, int h);
int wman_texture(wman_t *wm, int id, texture_t *t, int w, int h);
int wman_update(wman_t *wm, int id, int x, int y, int w, int h);
int wman_raise(wman_t *wm, int id);
int wman_move(wman_t *wm, int id, int dx, int dy);
int wman_remove(wman_t *wm, int id, int remove);
int wman_clicked(wman_t *wm, int x, int y, int *tx, int *ty);
int wman_xy(wman_t *wm, int id, int *x, int *y);
int wman_finish(wman_t *wm);
