#ifndef PIT_LOGIN_H
#define PIT_LOGIN_H

#ifdef __cplusplus
extern "C" {
#endif

int login_loop(conn_filter_t *next, char *login, char *password);

#ifdef __cplusplus
}
#endif

#endif