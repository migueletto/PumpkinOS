#ifndef PIT_SOCK_H
#define PIT_SOCK_H

#ifdef __cplusplus
extern "C" {
#endif

int sock_stream_server(char *tag, int pe, io_callback_f cb, int index, void *data, bt_provider_t *bt);

int sock_stream_close(char *tag, int pe);

int sock_client(char *tag, int pe, io_callback_f cb, int index, void *data, bt_provider_t *bt);

int sock_write(char *tag, int pe);

int sock_dgram_server(char *tag, int pe, io_callback_f cb, int index, void *data);

int sock_dgram_close(char *tag, int pe);

int sock_sendto(char *tag, int pe);

#ifdef __cplusplus
}
#endif

#endif
