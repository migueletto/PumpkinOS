typedef struct editor_t {
  // called by editor
  int (*cursor)(void *data, int col, int row);
  int (*cls)(void *data, uint32_t bg);
  int (*clreol)(void *data, uint32_t bg);
  int (*peek)(void *data, int ms);
  int (*read)(void *data, uint16_t *c);
  int (*write)(void *data, char *buf, int len);
  int (*color)(void *data, uint32_t fg, uint32_t bg);
  int (*window)(void *data, int *ncols, int *nrows);
  void *(*fopen)(void *data, char *name, int writing);
  int (*fclose)(void *data, void *f);
  int (*fread)(void *data, void *f, void *buf, int n);
  int (*fwrite)(void *data, void *f, void *buf, int n);
  int (*fsize)(void *data, void *f);
  int (*seek)(void *data, void *f, int orig, int offset);
  int (*truncate)(void *data, void *f, int offset);

  // host data
  void *data;

  // called by host
  int (*edit)(struct editor_t *e, char *filename);
  void (*destroy)(struct editor_t *e);
} editor_t;
