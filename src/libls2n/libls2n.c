#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <poll.h>

#include <s2n.h>

#include "script.h"
#include "thread.h"
#include "sys.h"
#include "vfs.h"
#include "pit_io.h"
#include "filter.h"
#include "secure.h"
#include "debug.h"
#include "xalloc.h"

struct secure_config_t {
  struct s2n_config *config;
};

struct secure_t {
  struct s2n_connection *conn;
  int fd;
};

static char dhparams[] =
    "-----BEGIN DH PARAMETERS-----\n"
    "MIIBCAKCAQEAy1+hVWCfNQoPB+NA733IVOONl8fCumiz9zdRRu1hzVa2yvGseUSq\n"
    "Bbn6k0FQ7yMED6w5XWQKDC0z2m0FI/BPE3AjUfuPzEYGqTDf9zQZ2Lz4oAN90Sud\n"
    "luOoEhYR99cEbCn0T4eBvEf9IUtczXUZ/wj7gzGbGG07dLfT+CmCRJxCjhrosenJ\n"
    "gzucyS7jt1bobgU66JKkgMNm7hJY4/nhR5LWTCzZyzYQh2HM2Vk4K5ZqILpj/n0S\n"
    "5JYTQ2PVhxP+Uu8+hICs/8VvM72DznjPZzufADipjC7CsQ4S6x/ecZluFtbb+ZTv\n"
    "HI5CnYmkAwJ6+FSWGaZQDi8bgerFk9RWwwIBAg==\n"
    "-----END DH PARAMETERS-----\n";

static secure_provider_t provider;

static void libls2n_error(char *s) {
  debug(DEBUG_ERROR, "SECURE", "%s: '%s' : '%s'", s, s2n_strerror(s2n_errno, "EN"), s2n_strerror_debug(s2n_errno, "EN"));
}

static int wait_for_event(int fd, s2n_blocked_status blocked) {
  struct pollfd reader = { .fd = fd, .events = 0 };

  switch (blocked) {
    case S2N_NOT_BLOCKED:
      return S2N_SUCCESS;
    case S2N_BLOCKED_ON_READ:
      reader.events |= POLLIN;
      break;
    case S2N_BLOCKED_ON_WRITE:
      reader.events |= POLLOUT;
      break;
    case S2N_BLOCKED_ON_EARLY_DATA:
    case S2N_BLOCKED_ON_APPLICATION_INPUT:
      /* This case is not encountered by the s2nc/s2nd applications,
       * but is detected for completeness */
      return S2N_SUCCESS;
  }

  if (poll(&reader, 1, -1) < 0) {
    debug_errno("SECURE", "poll");
  }

  return S2N_SUCCESS;
}

static int libls2n_negotiate(int fd, struct s2n_connection *conn) {
  s2n_blocked_status blocked;
  int client_hello_version;
  int client_protocol_version;
  int server_protocol_version;
  int actual_protocol_version;
  const uint8_t *status;
  const char *s;
  uint32_t length;

  debug(DEBUG_INFO, "SECURE", "negotiating ...");

  for (; !thread_must_end();) {
    if (s2n_negotiate(conn, &blocked) == S2N_SUCCESS) break;
    if (wait_for_event(fd, blocked) == S2N_SUCCESS) break;
  }

  if ((client_hello_version = s2n_connection_get_client_hello_version(conn)) < 0) {
    debug(DEBUG_ERROR, "SECURE", "could not get client hello version");
    return -1;
  }

  if ((client_protocol_version = s2n_connection_get_client_protocol_version(conn)) < 0) {
    debug(DEBUG_ERROR, "SECURE", "could not get client protocol version");
    return -1;
  }

  if ((server_protocol_version = s2n_connection_get_server_protocol_version(conn)) < 0) {
    debug(DEBUG_ERROR, "SECURE", "could not get server protocol version");
    return -1;
  }

  if ((actual_protocol_version = s2n_connection_get_actual_protocol_version(conn)) < 0) {
    debug(DEBUG_ERROR, "SECURE", "could not get actual protocol version");
    return -1;
  }

  debug(DEBUG_INFO, "SECURE", "client hello version: %d", client_hello_version);
  debug(DEBUG_INFO, "SECURE", "client protocol version: %d", client_protocol_version);
  debug(DEBUG_INFO, "SECURE", "server protocol version: %d", server_protocol_version);
  debug(DEBUG_INFO, "SECURE", "actual protocol version: %d", actual_protocol_version);

  s = s2n_get_server_name(conn);
  if (s) {
    debug(DEBUG_INFO, "SECURE", "server name: %s", s);
  } else {
    debug(DEBUG_ERROR, "SECURE", "no server name");
  }

  s = s2n_get_application_protocol(conn);
  if (s) {
    debug(DEBUG_INFO, "SECURE", "application protocol: %s", s);
  } else {
    debug(DEBUG_ERROR, "SECURE", "no application protocol");
  }

  debug(DEBUG_INFO, "SECURE", "curve: %s", s2n_connection_get_curve(conn));

  status = s2n_connection_get_ocsp_response(conn, &length);
  if (status && length > 0) {
    debug(DEBUG_INFO, "SECURE", "OCSP response received, length %u", length);
  }

  debug(DEBUG_INFO, "SECURE", "cipher negotiated: %s", s2n_connection_get_cipher(conn));

  if (s2n_connection_is_session_resumed(conn)) {
    debug(DEBUG_INFO, "SECURE", "resumed session");
  }

  debug(DEBUG_INFO, "SECURE", "negotiation finished");
  return 0;
}

static secure_config_t *libls2n_new(char *cert, char *key) {
  struct s2n_cert_chain_and_key *chain_and_key;
  secure_config_t *c;
  struct s2n_config *config;

  if ((config = s2n_config_new()) == NULL) {
    libls2n_error("error getting new s2n config");
    return NULL;
  }

  if (cert && key) {
    if ((chain_and_key = s2n_cert_chain_and_key_new()) == NULL) {
      libls2n_error("error creating chain");
      return NULL;
    }

    if (s2n_cert_chain_and_key_load_pem(chain_and_key, cert, key) < 0) {
      libls2n_error("error getting certificate/key");
      return NULL;
    }

    if (s2n_config_add_cert_chain_and_key_to_store(config, chain_and_key) < 0) {
      libls2n_error("error setting certificate/key");
      return NULL;
    }
  }

  if (s2n_config_add_dhparams(config, dhparams) < 0) {
    libls2n_error("error adding DH parameters");
    return NULL;
  }

  if (s2n_config_set_cipher_preferences(config, "default") < 0) {
    libls2n_error("error setting cipher prefs");
    return NULL;
  }

  if (s2n_config_disable_x509_verification(config) < 0) {
    libls2n_error("error disabling X.509 validation");
    return NULL;
  }

  if ((c = xcalloc(1, sizeof(secure_config_t))) == NULL) {
    return NULL;
  }

  c->config = config;
  debug(DEBUG_INFO, "SECURE", "configuration created");

  return c;
}

static int libls2n_destroy(secure_config_t *c) {
  int r = -1;

  if (c) {
    xfree(c);
    r = 0;
  }

  return r;
}

static secure_t *libls2n_connect(secure_config_t *c, char *host, int port, int fd) {
  struct s2n_connection *conn;
  secure_t *s;

  // secure connection works in blocking I/O  mode
#ifndef WINDOWS
  int flags;
  flags = fcntl(fd, F_GETFL, 0);
  flags &= ~O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) != 0) {
    debug_errno("SECURE", "fcntl");
    return NULL;
  }
#endif

  if ((conn = s2n_connection_new(S2N_CLIENT)) == NULL) {
    libls2n_error("error getting new s2n connection");
    return NULL;
  }
  debug(DEBUG_INFO, "SECURE", "s2n_connection_new ok");

  if (s2n_connection_set_config(conn, c->config) < 0) {
    libls2n_error("error setting configuration");
    s2n_connection_free(conn);
    return NULL;
  }
  debug(DEBUG_INFO, "SECURE", "s2n_connection_set_config ok");

  if (s2n_connection_set_fd(conn, fd) < 0) {
    libls2n_error("error setting file descriptor");
    s2n_connection_free(conn);
    return NULL;
  }
  debug(DEBUG_INFO, "SECURE", "s2n_connection_set_fd %d ok", fd);

  if (libls2n_negotiate(fd, conn) == -1) {
    s2n_connection_free(conn);
    return NULL;
  }

  if ((s = xcalloc(1, sizeof (secure_t))) != NULL) {
    s->conn = conn;
    s->fd = fd;
    debug(DEBUG_INFO, "SECURE", "connected on fd %d", fd);
  } else {
    s2n_connection_free(conn);
  }

  return s;
}

static int libls2n_peek(secure_t *s, uint32_t us) {
  int r;

  r = s2n_peek(s->conn);
  if (r) return r;

  return sys_select(s->fd, us);
}

static int libls2n_read(secure_t *s, char *buf, int len) {
  s2n_blocked_status blocked;
  int nread = 0;
  int r;

  do {
    r = s2n_recv(s->conn, buf + nread, len - nread, &blocked);
    if (r < 0) {
      debug(DEBUG_ERROR, "SECURE", "s2n_recv failed");
      break;
    }
    nread += r;
    if (nread == len) break;
    if (thread_must_end()) break;
  } while (blocked == S2N_BLOCKED_ON_READ);

  return nread;
}

static int libls2n_write(secure_t *s, char *buf, int len) {
  s2n_blocked_status blocked;
  int nwritten = 0;
  int r;

  do {
    r = s2n_send(s->conn, buf + nwritten, len - nwritten, &blocked);
    if (r < 0) {
      debug(DEBUG_ERROR, "SECURE", "s2n_send failed");
      break;
    }
    nwritten += r;
    if (nwritten == len) break;
    if (thread_must_end()) break;
  } while (blocked == S2N_BLOCKED_ON_WRITE);

  return nwritten;
}

static int libls2n_close(secure_t *s) {
  s2n_blocked_status blocked;
  int r = -1;

  if (s) {
    s2n_shutdown(s->conn, &blocked);
    s2n_connection_wipe(s->conn);
    s2n_connection_free(s->conn);
    xfree(s);
    r = 0;
  }

  return r;
}

int libls2n_cert(int pe) {
  char *cert = NULL;
  int len, r = -1;

  if (script_get_lstring(pe, 0, &cert, &len) == 0) {
    provider.cert = xstrdup(cert);
    r = 0;
  }

  if (cert) xfree(cert);

  return script_push_boolean(pe, r == 0);
}

int libls2n_key(int pe) {
  char *key = NULL;
  int len, r = -1;

  if (script_get_lstring(pe, 0, &key, &len) == 0) {
    provider.key = xstrdup(key);
    r = 0;
  }

  if (key) xfree(key);

  return script_push_boolean(pe, r == 0);
}

int libls2n_load(void) {
  if (s2n_init() == S2N_FAILURE) {
    debug(DEBUG_ERROR, "SECURE", "s2n_init failed");
    return -1;
  }

  provider.new = libls2n_new;
  provider.destroy = libls2n_destroy;
  provider.connect = libls2n_connect;
  provider.peek = libls2n_peek;
  provider.read = libls2n_read;
  provider.write = libls2n_write;
  provider.close = libls2n_close;

  return 0;
}

int libls2n_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, "SECURE", "registering provider %s", SECURE_PROVIDER);
  script_set_pointer(pe, SECURE_PROVIDER, &provider);

  script_add_function(pe,obj, "cert", libls2n_cert);
  script_add_function(pe,obj, "key",  libls2n_key);

  return 0;
}
