#ifndef PIT_FILTER_H
#define PIT_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct conn_filter_t {
  int (*peek)(struct conn_filter_t *filter, uint32_t us);
  int (*read)(struct conn_filter_t *filter, uint8_t *b);
  int (*write)(struct conn_filter_t *filter, uint8_t *b, int n);
  void *data;
  struct conn_filter_t *next;
} conn_filter_t;

conn_filter_t *conn_filter(int fd);
void conn_set(conn_filter_t *filter, conn_filter_t *next);
void conn_close(conn_filter_t *filter);

#ifdef __cplusplus
}
#endif

#endif
