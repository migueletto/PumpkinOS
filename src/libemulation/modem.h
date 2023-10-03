conn_filter_t *modem_filter(conn_filter_t *next, void *p, int (*dial)(void *p, char *number));
void modem_close(conn_filter_t *filter);
void modem_set(conn_filter_t *filter, conn_filter_t *next);
char modem_getstate(conn_filter_t *filter);
