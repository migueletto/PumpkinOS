#ifndef PIT_HTTPC_H
#define PIT_HTTPC_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_RES_HEADERS 64

#define TAG_HTTP_CLIENT "http_client"

typedef struct http_client_t {
  char *tag;
  char *user_agent;
  char *buf;
  int buflen;
  int connected;
  conn_filter_t filter;
  secure_provider_t *secure;
  secure_config_t *sc;
  secure_t *s;

  // request
  char *request_method;
  char *request_url;
  char *request_host;
  char *request_path;
  char *request_type;
  char *request_body;
  int request_body_length;
  int request_port;

  // response
  int linelen;
  int response_num_headers;
  char *response_header_name[MAX_RES_HEADERS];
  char *response_header_value[MAX_RES_HEADERS];
  int response_ok;
  int response_code;
  int response_end_header;
  int response_fd;

  // callback
  void *data;
  int (*callback)(int ptr, void *data);
} http_client_t;

int pit_http_get(char *user_agent, char *url, secure_provider_t *secure, int (*callback)(int ptr, void *data), void *data);
int pit_http_post(char *user_agent, char *url, secure_provider_t *secure, char *content_type, char *body, int len, int (*callback)(int ptr, void *data), void *data);
int pit_http_put(char *user_agent, char *url, secure_provider_t *secure, char *content_type, char *body, int len, int (*callback)(int ptr, void *data), void *data);
int pit_http_delete(char *user_agent, char *url, secure_provider_t *secure, int (*callback)(int ptr, void *data), void *data);

#ifdef __cplusplus
}
#endif

#endif
