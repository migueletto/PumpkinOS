#define HTTP_OK		0
#define HTTP_LIBERR	1
#define HTTP_NOTFOUND	2
#define HTTP_CONNECTERR	3
#define HTTP_ERR	4

typedef void (*http_buf)(char *buf, UInt16 n);

Err http_init(UInt32 lookupTimeout, UInt32 connectTimeout, UInt32 appTimeout);
Err http_finish(void);
Err http_get(char *host, UInt16 port, char *path, http_buf f);
char *http_error(Err err);
