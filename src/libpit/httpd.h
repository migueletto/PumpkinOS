#ifndef PIT_HTTPD_H
#define PIT_HTTPD_H

#define MAX_PACKET 1024
#define MAX_REQ_HEADERS 64
#define MAX_REQ_PARAMS 64

#define MAX_EXT  16
#define MAX_MIME 256

#define TAG_CONN  "connection"

typedef struct {
  char ext[MAX_EXT];
  char mimetype[MAX_MIME];
} ext_type_t;

typedef struct http_connection_t {
  char *tag;
  int sock;
  int status;
  int keepalive;
  int timeout;
  int commited;
  int remote_port;
  char host[MAX_HOST];
  char buffer[MAX_PACKET];
  char *system;
  char *home;
  char *method;
  char *uri;
  char *protocol;
  char *content_type;
  int content_length;
  char *headers;
  int num_headers;
  char *header_name[MAX_REQ_HEADERS];
  char *header_value[MAX_REQ_HEADERS];
  int num_params;
  char *param_name[MAX_REQ_PARAMS];
  char *param_value[MAX_REQ_PARAMS];
  int num_res_headers;
  char *res_header_name[MAX_REQ_HEADERS];
  char *res_header_value[MAX_REQ_HEADERS];
  char *authorization;
  FILE *body_fd;
  char body_buf[MAX_PACKET];
  char *user;
  char *password;
  secure_provider_t *secure;
  secure_t *s;
  int (*callback)(struct http_connection_t *con);
  void *data;
  int (*response_callback)(struct http_connection_t *con, void *data);
  void *response_data;
  ext_type_t *types;
  int num_types;
} http_connection_t;

int httpd_string(http_connection_t *con, int code, char *str, char *mime);
int httpd_file(http_connection_t *con, char *filename);
int httpd_file_stream(http_connection_t *con, FILE *fd, char *mime, time_t mtime);
int httpd_set_header(http_connection_t *con, char *name, char *value);
int httpd_reply(http_connection_t *con, int code);

int httpd_create(char *host, int port, char *system, char *home, char *user, char *password, secure_provider_t *secure, char *cert, char *key, int (*callback)(http_connection_t *con), void *data);
int httpd_close(int handle);
int httpd_mimetype(int index, char *ext, char *mimetype);

#endif
