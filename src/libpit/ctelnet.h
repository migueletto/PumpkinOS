conn_filter_t *telnet_client_filter(conn_filter_t *next, char *term, int cols, int rows);
int telnet_client_echo(conn_filter_t *filter);
void telnet_client_close(conn_filter_t *filter);
char telnet_client_state(conn_filter_t *filter);
