int dbg_init(window_provider_t *wp, int encoding);
void dbg_finish(void);
void dbg_poll(void);

void dbg_add(int row, BitmapType *bmp);
void dbg_delete(BitmapType *bmp);
void dbg_update(BitmapType *bmp);
