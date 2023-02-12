#define SECURE_PROVIDER "secure_provider"

typedef struct secure_config_t secure_config_t;
typedef struct secure_t secure_t;

typedef struct {
  secure_config_t *(*new)(char *cert, char *key);
  int (*destroy)(secure_config_t *c);
  secure_t *(*connect)(secure_config_t *c, char *host, int port, int fd);
  int (*peek)(secure_t *s, uint32_t us);
  int (*read)(secure_t *s, char *buf, int len);
  int (*write)(secure_t *s, char *buf, int len);
  int (*close)(secure_t *s);
  char *cert;
  char *key;
} secure_provider_t;
