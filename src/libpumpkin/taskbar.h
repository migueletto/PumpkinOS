typedef struct taskbar_t taskbar_t;

taskbar_t *taskbar_create(window_provider_t *wp, window_t *w, uint16_t density, uint32_t x, uint32_t y, uint32_t width, uint32_t height, int encoding);
void taskbar_destroy(taskbar_t *tb);
void taskbar_add(taskbar_t *tb, Int32 taskId, LocalID dbID, UInt32 creator, char *name);
void taskbar_remove(taskbar_t *tb, LocalID dbID);
void taskbar_update(taskbar_t *tb);
void taskbar_draw(taskbar_t *tb);
Int32 taskbar_clicked(taskbar_t *tb, int x);

Boolean taskbar_add_widget(taskbar_t *tb, Int32 taskId, UInt32 id, LocalID dbID, UInt16 bmpID);
Boolean taskbar_remove_widget(taskbar_t *tb, UInt32 id);
Int32 taskbar_widget_clicked(taskbar_t *tb, int x, UInt32 *id);
