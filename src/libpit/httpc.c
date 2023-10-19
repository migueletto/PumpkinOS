#include "sys.h"
#include "thread.h"
#include "pit_io.h"
#include "filter.h"
#include "secure.h"
#include "httpc.h"
#include "ptr.h"
#include "debug.h"
#include "xalloc.h"

#define DEFAULT_PORT         80
#define DEFAULT_SECURE_PORT  443
#define BUFFER_LEN           (65536*4)
#define TIMEOUT              5

#define TAG_HTTPC  "HTTPC"

static void http_parse_headers(char *buf, http_client_t *hc) {
  int minor, code, len;
  char *p;

  if (!sys_strncmp(buf, "HTTP/1.", 7)) {
    if (sys_sscanf(buf, "HTTP/1.%d %d ", &minor, &code) == 2) {
      hc->response_ok = 1;
      hc->response_code = code;

    } else {
      debug(DEBUG_ERROR, "WEB", "invalid http server reply \"%s\"", buf);
    }

  } else {
    if (!hc->response_end_header && hc->response_num_headers < MAX_RES_HEADERS) {
      p = sys_strstr(buf, ": ");
      if (p && p > buf) {
        len = p - buf;
        hc->response_header_name[hc->response_num_headers] = xcalloc(len+1, 1);
        if (hc->response_header_name[hc->response_num_headers]) {
          sys_strncpy(hc->response_header_name[hc->response_num_headers], buf, len);
          p += 2;
          len = sys_strlen(p);
          if (len) {
            hc->response_header_value[hc->response_num_headers] = xcalloc(len+1, 1);
            if (hc->response_header_value[hc->response_num_headers]) {
              sys_strncpy(hc->response_header_value[hc->response_num_headers], p, len);
              debug(DEBUG_INFO, "WEB", "header \"%s\" = \"%s\"", hc->response_header_name[hc->response_num_headers], hc->response_header_value[hc->response_num_headers]);
            }
          } else {
            debug(DEBUG_INFO, "WEB", "header \"%s\"", hc->response_header_name[hc->response_num_headers]);
          }
          hc->response_num_headers++;
        }
      }
    }
  }
}

static void free_http_client(void *p) {
  http_client_t *hc = (http_client_t *)p;
  int i;

  if (hc) {
    for (i = 0; i < hc->response_num_headers; i++) {
      if (hc->response_header_name[i]) xfree(hc->response_header_name[i]);
      if (hc->response_header_value[i]) xfree(hc->response_header_value[i]);
    }
    if (hc->buf) xfree(hc->buf);
    if (hc->request_url) xfree(hc->request_url);
    if (hc->request_type) xfree(hc->request_type);
    if (hc->request_body) xfree(hc->request_body);
    if (hc->request_path) xfree(hc->request_path);
    if (hc->request_host) xfree(hc->request_host);
    if (hc->response_fd) sys_close(hc->response_fd);
    xfree(hc);
  }
}

static int secure_peek(conn_filter_t *filter, uint32_t us) {
  http_client_t *hc = (http_client_t *)filter->data;
  int r = hc->secure->peek(hc->s, us);
  return r;
}

static int secure_read(conn_filter_t *filter, uint8_t *b) {
  http_client_t *hc = (http_client_t *)filter->data;
  int r = hc->secure->read(hc->s, (char *)b, 1);
  return r;
}

static int secure_write(conn_filter_t *filter, uint8_t *b, int n) {
  http_client_t *hc = (http_client_t *)filter->data;
  int r = hc->secure->write(hc->s, (char *)b, n);
  return r;
}

static int io_callback(io_arg_t *arg) {
  http_client_t *hc;
  char header[1024];
  int ptr, wr, r, i;

  hc = (http_client_t *)arg->data;
  r = 0;

  switch (arg->event) {
    case IO_CONNECT:
      sys_memset(header, 0, sizeof(header));

      if (hc->request_type) {
        sys_snprintf(header, sizeof(header)-1, "%s %s HTTP/1.1\r\nUser-Agent: %s\r\nAccept: */*\r\nHost: %s:%d\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", hc->request_method, hc->request_path, hc->user_agent, hc->request_host, hc->request_port, hc->request_type, hc->request_body_length);
      } else {
        sys_snprintf(header, sizeof(header)-1, "%s %s HTTP/1.1\r\nUser-Agent: %s\r\nAccept: */*\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", hc->request_method, hc->request_path, hc->user_agent, hc->request_host, hc->request_port);
      }

      if (hc->s) {
        debug(DEBUG_INFO, "WEB", "sending secure header");
        wr = hc->secure->write(hc->s, header, sys_strlen(header));
        debug(DEBUG_INFO, "WEB", "result: %d", wr);
        if (hc->request_body) {
          debug(DEBUG_INFO, "WEB", "sending secure body");
          wr = hc->secure->write(hc->s, hc->request_body, hc->request_body_length);
          debug(DEBUG_INFO, "WEB", "result: %d", wr);
        }
      } else {
        debug(DEBUG_INFO, "WEB", "sending header");
        sys_write(arg->fd, (uint8_t *)header, sys_strlen(header));
        if (hc->request_body) {
          sys_write(arg->fd, (uint8_t *)hc->request_body, hc->request_body_length);
        }
      }

      hc->connected = 1;
      break;

    case IO_FILTER:
      if (hc->secure) {
        r = 1;
        debug(DEBUG_INFO, "WEB", "creating secure configuration");
        if ((hc->sc = hc->secure->new(hc->secure->cert, hc->secure->key)) != NULL) {
          debug(DEBUG_INFO, "WEB", "creating secure connection");
          if ((hc->s = hc->secure->connect(hc->sc, hc->request_host, hc->request_port, arg->fd)) != NULL) {
            debug(DEBUG_INFO, "WEB", "secure connection created");
            hc->filter.peek = secure_peek;
            hc->filter.read = secure_read;
            hc->filter.write = secure_write;
            hc->filter.data = hc;
            arg->param = &hc->filter;
            r = 0;
          } else {
            hc->secure->destroy(hc->sc);
          }
        }
      }
      break;

    case IO_CONNECT_ERROR:
      r = 1;
      break;

    case IO_DATA:
      debug(DEBUG_INFO, "WEB", "%d bytes received", arg->len);

      if (hc->response_end_header) {
        if (arg->len > 0) {
          debug(DEBUG_INFO, "WEB", "writing %d bytes to response file", arg->len);
          sys_write(hc->response_fd, arg->buf, arg->len);
        }
      } else {
        for (i = 0; i < arg->len; i++) {
          if (arg->buf[i] == '\n') {
            if (hc->linelen && hc->buf[hc->linelen-1] == '\r') {
              hc->linelen--;
            }
            if (hc->linelen == 0) {
              hc->response_end_header = 1;
              if ((hc->response_fd = sys_mkstemp()) == -1) {
                r = 1;
              } else {
                if (arg->len - (i+1) > 0) {
                  debug(DEBUG_INFO, "WEB", "writing %d bytes to response file", arg->len - (i+1));
                  sys_write(hc->response_fd, arg->buf + i+1, arg->len - (i+1));
                  i = arg->len;
                }
              }
            } else {
              hc->buf[hc->linelen] = 0;
              http_parse_headers(hc->buf, hc);
              hc->linelen = 0;
            }
          } else {
            if (hc->linelen < BUFFER_LEN-1) {
              hc->buf[hc->linelen++] = arg->buf[i];
            }
          }
        }
      }
      break;

    case IO_TIMEOUT:
      debug(DEBUG_INFO, "WEB", "timeout reading from http server");
      r = 1;
      break;

    case IO_DISCONNECT:
      if (!hc->response_ok) debug(DEBUG_ERROR, "WEB", "http server reply code not found");
      if (!hc->response_end_header) debug(DEBUG_ERROR, "WEB", "http server reply header did not end properly");
      if (hc->response_fd) sys_seek(hc->response_fd, 0, SYS_SEEK_SET);
      if (hc->callback) {
        hc->tag = TAG_HTTP_CLIENT;
        if ((ptr = ptr_new(hc, free_http_client)) != -1) {
          debug(DEBUG_INFO, "WEB", "calling data callback");
          hc->callback(ptr, hc->data);
          ptr_free(ptr, TAG_HTTP_CLIENT);
        }
      } else {
        free_http_client(hc);
      }
      break;
  }

  return r;
}

static int http_request(char *user_agent, char *method, char *url, secure_provider_t *secure, char *content_type, char *body, int len, int (*callback)(int ptr, void *data), void *data) {
  http_client_t *hc;
  io_addr_t addr;
  char *p;

  if ((hc = xcalloc(1, sizeof(http_client_t))) == NULL) {
    return -1;
  }

  hc->tag = TAG_HTTP_CLIENT;
  hc->user_agent = user_agent;
  hc->request_method = method;
  hc->callback = callback;
  hc->data = data;

  if ((hc->buf = xcalloc(BUFFER_LEN, 1)) == NULL) {
    xfree(hc);
    return -1;
  }
  hc->buflen = BUFFER_LEN;

  hc->request_url = xstrdup(url);
  hc->request_type = content_type ? xstrdup(content_type) : NULL;
  hc->request_body = body ? xmalloc(len) : NULL;

  if (!hc->request_url || (body && !hc->request_body)) {
    free_http_client(hc);
    return -1;
  }

  if (body) sys_memcpy(hc->request_body, body, len);
  hc->request_body_length = len;

  if (!sys_strncmp(hc->request_url, "https://", 8)) {
    if (!secure) {
      debug(DEBUG_ERROR, "WEB", "https was used but no secure provider was passed");
      free_http_client(hc);
      return -1;
    }
    hc->request_host = hc->request_url + 8;
    hc->request_port = DEFAULT_SECURE_PORT;
    hc->secure = secure;
  } else if (!sys_strncmp(hc->request_url, "http://", 7)) {
    hc->request_host = hc->request_url + 7;
    hc->request_port = DEFAULT_PORT;
  } else {
    hc->request_host = hc->request_url;
    hc->request_port = DEFAULT_PORT;
  }

  hc->request_host = xstrdup(hc->request_host);
  hc->request_path = NULL;
  p = sys_strchr(hc->request_host, '/');
  if (p == NULL) {
    hc->request_path = xstrdup("/");
  } else  {
    hc->request_path = xstrdup(p);
    p[0] = 0;
  }

  p = sys_strchr(hc->request_host, ':');
  if (p) {
    hc->request_port = sys_atoi(p+1);
    p[0] = 0;
  }

  if (hc->request_host == NULL || hc->request_path == NULL) {
    debug(DEBUG_ERROR, "WEB", "invalid url \"%s\"", hc->request_url);
    free_http_client(hc);
    return -1;
  }

  debug(DEBUG_INFO, "WEB", "method \"%s\"", hc->request_method);
  debug(DEBUG_INFO, "WEB", "host \"%s\"", hc->request_host);
  debug(DEBUG_INFO, "WEB", "port %d (%s)", hc->request_port, hc->secure ? "secure" : "not secure");
  debug(DEBUG_INFO, "WEB", "path \"%s\"", hc->request_path);

  sys_memset(&addr, 0, sizeof(addr));
  addr.addr_type = IO_IP_ADDR;
  sys_strncpy(addr.addr.ip.host, hc->request_host, MAX_HOST-1);
  addr.addr.ip.port = hc->request_port;

  if (io_stream_client(TAG_HTTPC, &addr, io_callback, hc, TIMEOUT, NULL) == -1) {
    free_http_client(hc);
    return -1;
  }

  return 0;
}

int pit_http_get(char *user_agent, char *url, secure_provider_t *secure, int (*callback)(int ptr, void *data), void *data) {
  if (!url) {
    debug(DEBUG_ERROR, "WEB", "invalid http get parameters");
    return -1;
  }

  return http_request(user_agent, "GET", url, secure, NULL, NULL, 0, callback, data);
}

int pit_http_post(char *user_agent, char *url, secure_provider_t *secure, char *content_type, char *body, int len, int (*callback)(int ptr, void *data), void *data) {
  if (!url || !content_type || !body || len <= 0) {
    debug(DEBUG_ERROR, "WEB", "invalid http post parameters");
    return -1;
  }

  return http_request(user_agent, "POST", url, secure, content_type, body, len, callback, data);
}

int pit_http_put(char *user_agent, char *url, secure_provider_t *secure, char *content_type, char *body, int len, int (*callback)(int ptr, void *data), void *data) {
  if (!url || !content_type || !body || len <= 0) {
    debug(DEBUG_ERROR, "WEB", "invalid http put parameters");
    return -1;
  }

  return http_request(user_agent, "PUT", url, secure, content_type, body, len, callback, data);
}

int pit_http_delete(char *user_agent, char *url, secure_provider_t *secure, int (*callback)(int ptr, void *data), void *data) {
  if (!url) {
    debug(DEBUG_ERROR, "WEB", "invalid http delete parameters");
    return -1;
  }

  return http_request(user_agent, "DELETE", url, secure, NULL, NULL, 0, callback, data);
}
