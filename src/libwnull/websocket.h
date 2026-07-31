typedef struct websocket_t websocket_t;

websocket_t *websocket_create(int fd);
int websocket_destroy(websocket_t *ws);
int websocket_handshake(websocket_t *ws, int wait);
int websocket_write(websocket_t *ws, uint8_t *buf, uint32_t len);
int websocket_read(websocket_t *ws, uint8_t *buf, uint32_t len, int *nread, int wait);
