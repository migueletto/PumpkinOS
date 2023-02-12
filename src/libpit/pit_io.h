#ifndef PIT_IO_H
#define PIT_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#define IO_BIND          1
#define IO_BIND_ERROR    2
#define IO_ACCEPT        3
#define IO_CONNECT       4
#define IO_CONNECT_ERROR 5
#define IO_DISCONNECT    6
#define IO_DATA          7
#define IO_LINE          8
#define IO_TIMEOUT       9
#define IO_CLOSE         10
#define IO_CMD           11
#define IO_TIMER         12
#define IO_FILTER        13
#define IO_LAST          99

#define MAX_DEVICE      256
#define MAX_HOST        256
#define MAX_ADDR        32
#define MAX_WORD        4

#define IO_DEVICE_ADDR  1
#define IO_IP_ADDR      2
#define IO_BT_ADDR      3
#define IO_FD_ADDR      4

typedef struct {
  char device[MAX_DEVICE];
  char word[MAX_WORD];
  int baud;
} io_device_t;

typedef struct {
  char host[MAX_HOST];
  int port;
} io_ip_t;

typedef struct {
  char addr[MAX_ADDR];
  int port, type;
} io_bt_t;

typedef struct {
  int fd;
} io_fd_t;

typedef struct {
  int addr_type;
  union {
    io_device_t dev;
    io_ip_t ip;
    io_bt_t bt;
    io_fd_t fd;
  } addr;
} io_addr_t;

typedef struct {
  int event;
  io_addr_t *addr;
  int handle;
  int fd;
  void *data;
  uint8_t *buf;
  int len;
  void *param;
} io_arg_t;

typedef int (*io_callback_f)(io_arg_t *arg);

typedef int (*io_loop_f)(int fd, int ptr);

#define BT_PROVIDER "bluetooth_provider"

#define BT_RFCOMM 1
#define BT_L2CAP  2

typedef struct {
  int (*bind)(char *addr, int port, int type);
  int (*accept)(int sock, char *peer, int *port, int type, struct timeval *tv);
  int (*connect)(char *peer, int port, int type);
} bt_provider_t;

int io_stream_bound_client(char *tag, io_addr_t *src, io_addr_t *addr, io_callback_f callback, void *data, int timeout, bt_provider_t *bt);
int io_stream_client(char *tag, io_addr_t *addr, io_callback_f callback, void *data, int timeout, bt_provider_t *bt);
int io_stream_server(char *tag, io_addr_t *addr, io_callback_f callback, void *data, int timer, bt_provider_t *bt);
int io_stream_close(char *tag, int handle);
int io_simple_server(char *tag, io_addr_t *addr, io_loop_f loop, int ptr, bt_provider_t *bt);
int io_simple_client(char *tag, io_addr_t *addr, io_loop_f loop, int ptr, bt_provider_t *bt);
int io_dgram_server(char *tag, io_addr_t *addr, io_callback_f callback, void *data, bt_provider_t *bt);
int io_dgram_close(char *tag, int handle);
int io_sendto_handle(char *tag, int handle, io_addr_t *addr, unsigned char *buf, unsigned int len);
int io_write_handle(char *tag, int handle, unsigned char *buf, unsigned int len);
int io_fill_addr(char *host, int port, io_addr_t *addr);
int io_connect(char *peer, int port, bt_provider_t *bt);
int io_bind(char *host, int port, int *addr_type, bt_provider_t *bt);
int io_accept(int fd, uint32_t us, int addr_type, bt_provider_t *bt);

#ifdef __cplusplus
}
#endif

#endif
