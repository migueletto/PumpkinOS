#ifndef PIT_TELNET_H
#define PIT_TELNET_H

#ifdef __cplusplus
extern "C" {
#endif

void telnet_linemode(conn_filter_t *filter, int linemode);

void telnet_echo(conn_filter_t *filter, int echo);

void telnet_naws(conn_filter_t *filter);

int telnet_term(conn_filter_t *filter, char *term, int n, int *cols, int *rows);

conn_filter_t *telnet_filter(conn_filter_t *next);

void telnet_close(conn_filter_t *filter);

#ifdef __cplusplus
}
#endif

#endif