#include "sys.h"
#include "bytes.h"
#include "websocket.h"
#include "sha1.h"
#include "base64.h"
#include "debug.h"

struct websocket_t {
  int fd, handshake;
};

/*
      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+
*/

#define WS_FIN      0x80
#define WS_MASK     0x80

#define WS_OP_CONT  0x00
#define WS_OP_TEXT  0x01
#define WS_OP_BIN   0x02
#define WS_OP_CLOSE 0x08
#define WS_OP_PING  0x09
#define WS_OP_PONG  0x0A

#define HASH_LEN  20

static const char *SEC_WEBSOCKET_KEY = "Sec-WebSocket-Key: ";

static const char *HANDSHAKE_REPLY =
  "HTTP/1.1 101 Switching Protocols\r\n"
  "Upgrade: websocket\r\n"
  "Connection: Upgrade\r\n"
  "Sec-WebSocket-Accept: ";

static const char *GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

static int websocket_write_op(websocket_t *ws, uint8_t op, uint8_t *buf, uint32_t len) {
  uint8_t header[10];
  uint32_t hlen;
  int r = -1;

  // FIN=1, no masking, payload length < 2^32

  sys_memset(header, 0, sizeof(header));
  header[0] = WS_FIN | op;

  if (len <= 125) {
    header[1] = len;
    hlen = 2;
  } else if (len <= 0xFFFF) {
    header[1] = 126;
    put2b(len, header, 2);
    hlen = 4;
  } else {
    header[1] = 127;
    put4b(len, header, 6);
    hlen = 10;
  }

  debug(DEBUG_TRACE, "WSOCK", "websocket_write op %u data %u bytes", op, len);
  debug_bytes(DEBUG_TRACE, "WSOCK", header, hlen);
  if (sys_write(ws->fd, header, hlen) == hlen) {
    if (len <= 16) {
      debug_bytes(DEBUG_TRACE, "WSOCK", buf, len);
    }
    r = len > 0 ? sys_write(ws->fd, buf, len) : 0;
  }

  return r;
}

int websocket_write(websocket_t *ws, uint8_t *buf, uint32_t len) {
  return websocket_write_op(ws, WS_OP_BIN, buf, len);
}

static int websocket_length_mask(websocket_t *ws, uint8_t *header, uint32_t *len, uint8_t *mask) {
  uint8_t buf[12];
  uint8_t len8, masked;
  uint16_t len16;
  uint32_t aux;
  int n;

  masked = header[1] & WS_MASK;
  len8 = header[1] & 0x7F;

  if (len8 <= 125) {
    *len = len8;
  } else if (len8 == 126) {
    if (sys_read_timeout(ws->fd, buf, 2, &n, 0) != 1 || n != 2) {
      debug(DEBUG_ERROR, "WSOCK", "client sent incomplete len header (%d bytes, %d expected)", n, 2);
      return -1;
    }
    get2b(&len16, buf, 0);
    *len = len16;
  } else { // len8 == 127
    if (sys_read_timeout(ws->fd, buf, 8, &n, 0) != 1 || n != 8) {
      debug(DEBUG_ERROR, "WSOCK", "client sent incomplete len header (%d bytes, %d expected)", n, 8);
      return -1;
    }
    get4b(&aux, buf, 0);
    if (aux != 0) {
      debug(DEBUG_ERROR, "WSOCK", "payload length >= 2^32");
      return -1;
    }
    get4b(len, buf, 4);
  }

  if (masked) {
    if (sys_read_timeout(ws->fd, mask, 4, &n, 0) != 1 || n != 4) {
      debug(DEBUG_ERROR, "WSOCK", "client sent incomplete mask header (%d bytes, %d expected)", n, 4);
      return -1;
    }
  } else {
    sys_memset(mask, 0, 4);
  }

  return 0;
}

static void websocket_unmask(uint8_t *buf, uint32_t len, uint8_t *mask) {
  uint32_t i;

  for (i = 0; i < len; i++) {
    buf[i] ^= mask[i % 4];
  }
}

static void websocket_send_close(websocket_t *ws, uint8_t *buf, uint32_t len) {
  websocket_write_op(ws, WS_OP_CLOSE, buf, len);
}

static void websocket_send_pong(websocket_t *ws, uint8_t *buf, uint32_t len) {
  websocket_write_op(ws, WS_OP_PONG, buf, len);
}

int websocket_handshake(websocket_t *ws, int wait) {
  SHA1_CTX sha;
  uint8_t hash[HASH_LEN];
  char buf[1024], *key, *b64, *s;
  int n, r;

  if ((r = sys_read_timeout(ws->fd, (uint8_t *)buf, sizeof(buf), &n, wait)) < 0) {
    debug(DEBUG_ERROR, "WSOCK", "error reading handshake from client");
    return r;
  }

  if (r == 0) {
    // nothing to read (timeout)
    return r;
  }

  if (n == 0) {
    debug(DEBUG_ERROR, "WSOCK", "client closed connection during handshake");
    return -1;
  }

  debug(DEBUG_TRACE, "WSOCK", "received client handshake");
  debug_bytes(DEBUG_TRACE, "WSOCK", (uint8_t *)buf, n);

/*
  GET /chat HTTP/1.1
  Host: server.example.com
  Upgrade: websocket
  Connection: Upgrade
  Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
  Origin: http://example.com
  Sec-WebSocket-Protocol: chat, superchat
  Sec-WebSocket-Version: 13
*/

  if (sys_strstr(buf, "Upgrade: websocket") == NULL ||
      sys_strstr(buf, "Connection: Upgrade") == NULL ||
      sys_strstr(buf, "Sec-WebSocket-Version: 13") == NULL ||
      (key = sys_strstr(buf, SEC_WEBSOCKET_KEY)) == NULL) {

    debug(DEBUG_ERROR, "WSOCK", "invalid handshake headers");
    return -1;
  }

  key += sys_strlen(SEC_WEBSOCKET_KEY);
  if ((s = sys_strstr(key, "\r\n")) == NULL) {
    debug(DEBUG_ERROR, "WSOCK", "invalid %s header", SEC_WEBSOCKET_KEY);
    return -1;
  }

  n = s - key;
  if (n <= 0) {
    debug(DEBUG_ERROR, "WSOCK", "invalid %s header", SEC_WEBSOCKET_KEY);
    return -1;
  }

  *s = 0;
  if ((key = sys_strdup(key)) == NULL) {
    return -1;
  }
  debug(DEBUG_TRACE, "WSOCK", "key: %s", key);

  sys_memset(buf, 0, sizeof(buf));
  sys_strcpy(buf, key);
  sys_strcat(buf, GUID);
  sys_free(key);
  debug(DEBUG_TRACE, "WSOCK", "key and GUID: %s", buf);

  SHA1Init(&sha);
  SHA1Update(&sha, (uint8_t *)buf, sys_strlen(buf));
  SHA1Final(hash, &sha);
  debug(DEBUG_TRACE, "WSOCK", "SHA1 hash");
  debug_bytes(DEBUG_TRACE, "WSOCK", hash, HASH_LEN);

  if ((b64 = base64_encode(hash, HASH_LEN)) == NULL) {
    debug(DEBUG_ERROR, "WSOCK", "base64 encode failed");
    return -1;
  }
  debug(DEBUG_TRACE, "WSOCK", "SHA1 hash base64: %s", b64);

  sys_memset(buf, 0, sizeof(buf));
  sys_strcpy(buf, HANDSHAKE_REPLY);
  sys_strcat(buf, b64);
  sys_strcat(buf, "\r\n\r\n");
  n = sys_strlen(buf);
  sys_free(b64);

  debug(DEBUG_TRACE, "WSOCK", "sending handshake to client");
  debug_bytes(DEBUG_TRACE, "WSOCK", (uint8_t *)buf, n);

  if (sys_write(ws->fd, (uint8_t *)buf, n) != n) {
    debug(DEBUG_ERROR, "WSOCK", "error sending handshake to client");
    return -1;
  }

  ws->handshake = 1;
  return 0;
}

int websocket_read(websocket_t *ws, uint8_t *buf, uint32_t len, int *nread, int wait) {
  uint8_t header[2], mask[4], control_data[256], opcode;
  uint32_t payload_len;
  int n, r;

  if ((r = sys_read_timeout(ws->fd, header, 2, &n, wait)) < 0) {
    debug(DEBUG_ERROR, "WSOCK", "error reading from client");
    return r;
  }

  if (r == 0) {
    // nothing to read (timeout)
    return r;
  }

  if (n == 0) {
    debug(DEBUG_ERROR, "WSOCK", "client closed connection");
    return -1;
  }

  debug(DEBUG_TRACE, "WSOCK", "received header");
  debug_bytes(DEBUG_TRACE, "WSOCK", header, n);

  if (n != 2) {
    debug(DEBUG_ERROR, "WSOCK", "client sent incomplete header (%d bytes)", n);
    return -1;
  }

  if ((header[0] & 0xF0) != WS_FIN) {
    debug(DEBUG_ERROR, "WSOCK", "invalid FIN or extensions (0x%02X)", buf[0]);
    return -1;
  }

  opcode = header[0] & 0x0F;
  switch (opcode) {
    case WS_OP_CONT:
      debug(DEBUG_ERROR, "WSOCK", "continuation frame not expected");
      r = -1;
      break;
    case WS_OP_TEXT:
      debug(DEBUG_ERROR, "WSOCK", "text frame not expected");
      r = -1;
      break;
    case WS_OP_PING:
    case WS_OP_CLOSE:
      buf = control_data;
      len = sizeof(control_data);
      // fall through
    case WS_OP_BIN:
      if (websocket_length_mask(ws, header, &payload_len, mask) == 0) {
        debug(DEBUG_TRACE, "WSOCK", "opcode %u payload length is %u", opcode, payload_len);
        debug(DEBUG_TRACE, "WSOCK", "opcode %u mask is %02X %02X %02X %02X", opcode, mask[0], mask[1], mask[2], mask[3]);
        if (payload_len <= len) {
          n = 0;
          r = payload_len > 0 ? sys_read_timeout(ws->fd, buf, payload_len, &n, 0) : 1;
         
          if (r == 1) {
            if (n == payload_len) {
              if (payload_len > 0 && (mask[0] || mask[1] || mask[2] || mask[3])) {
                websocket_unmask(buf, payload_len, mask);
              }

              switch (opcode) {
                case WS_OP_BIN:
                  *nread = payload_len;
                  r = 1;
                  break;
                case WS_OP_PING:
                  websocket_send_pong(ws, buf, payload_len);
                  r = 0;
                  break;
                case WS_OP_CLOSE:
                  websocket_send_close(ws, buf, payload_len);
                  r = 0;
                  break;
              }
            } else {
              debug(DEBUG_ERROR, "WSOCK", "opcode %u read less than %d bytes from payload", opcode, n);
              r = -1;
            }
          } else {
            debug(DEBUG_ERROR, "WSOCK", "opcode %u error reading payload (%d)", opcode, r);
            r = -1;
          }
        } else {
          debug(DEBUG_ERROR, "WSOCK", "opcode %u payload length %u is bigger than buffer size %u", opcode, payload_len, len);
          r = -1;
        }
      } else {
        debug(DEBUG_ERROR, "WSOCK", "opcode %u invalid payload length or mask", opcode);
        r = -1;
      }
      break;
    case WS_OP_PONG:
      debug(DEBUG_ERROR, "WSOCK", "pong frame not expected");
      r = -1;
      break;
    default:
      debug(DEBUG_ERROR, "WSOCK", "opcode 0x%02X not expected", opcode);
      break;
  }

  return r;
}

websocket_t *websocket_create(int fd) {
  websocket_t *ws;

  if ((ws = sys_calloc(1, sizeof(websocket_t))) != NULL) {
    ws->fd = fd;
  }

  return ws;
}

int websocket_destroy(websocket_t *ws) {
  int r = -1;

  if (ws) {
    if (ws->fd > 0) sys_close(ws->fd);
    sys_free(ws);
    r = 0;
  }

  return r;
}
