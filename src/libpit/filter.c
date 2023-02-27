#include "sys.h"
#include "filter.h"
#include "xalloc.h"
#include "debug.h"

typedef struct {
  int fd;
} sys_conn_filter_t;

static int conn_filter_peek(struct conn_filter_t *filter, uint32_t us) {
  sys_conn_filter_t *data = (sys_conn_filter_t *)filter->data;

  return sys_select(data->fd, us);
}

static int conn_filter_read(struct conn_filter_t *filter, uint8_t *b) {
  sys_conn_filter_t *data = (sys_conn_filter_t *)filter->data;
  int r = sys_read(data->fd, b, 1);
  return r == 1 ? 1 : -1;
}

static int conn_filter_write(struct conn_filter_t *filter, uint8_t *b, int n) {
  sys_conn_filter_t *data = (sys_conn_filter_t *)filter->data;
  int r = sys_write(data->fd, b, n);
  return r == n ? n : -1;
}

conn_filter_t *conn_filter(int fd) {
  conn_filter_t *filter;
  sys_conn_filter_t *data;

  if ((filter = xcalloc(1, sizeof(conn_filter_t))) != NULL) {
    if ((data = xcalloc(1, sizeof(sys_conn_filter_t))) != NULL) {
      data->fd = fd;
      filter->data = data;
      filter->peek = conn_filter_peek;
      filter->read = conn_filter_read;
      filter->write = conn_filter_write;
    } else {
      xfree(filter);
    }
  }

  return filter;
}

void conn_set(conn_filter_t *filter, conn_filter_t *next) {
  if (filter) filter->next = next;
}

void conn_close(conn_filter_t *filter) {
  sys_conn_filter_t *data;

  if (filter) {
    data = (sys_conn_filter_t *)filter->data;
    if (data) {
      sys_close(data->fd);
      xfree(data);
    }
    xfree(filter);
  }
}
