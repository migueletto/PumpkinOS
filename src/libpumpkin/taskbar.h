typedef struct taskbar_t taskbar_t;

taskbar_t *taskbar_create(window_provider_t *wp, window_t *w, uint16_t density, uint32_t x, uint32_t y, uint32_t width, uint32_t height, int encoding);
void taskbar_destroy(taskbar_t *tb);
void taskbar_add(taskbar_t *tb, UInt32 taskId, LocalID dbID, UInt32 creator, char *name);
void taskbar_remove(taskbar_t *tb, LocalID dbID);
void taskbar_update(taskbar_t *tb);
void taskbar_draw(taskbar_t *tb);
LocalID taskbar_clicked(taskbar_t *tb, int x, int down);
