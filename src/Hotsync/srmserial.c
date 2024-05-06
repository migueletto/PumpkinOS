#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <PalmOS.h>

#include "pi-debug.h"
#include "pi-source.h"
#include "pi-serial.h"
#include "pi-error.h"

#include "sys.h"
#include "pumpkin.h"
#include "debug.h"

static int s_open(pi_socket_t *ps, struct pi_sockaddr *addr, size_t addrlen);
static int s_close(pi_socket_t *ps);
static int s_changebaud(pi_socket_t *ps);
static ssize_t s_write(pi_socket_t *ps, const unsigned char *buf, size_t len, int flags);
static ssize_t s_read(pi_socket_t *ps, pi_buffer_t *buf, size_t len, int flags);
static int s_poll(pi_socket_t *ps, int timeout);
static int s_flush(pi_socket_t *ps, int flags);

static int s_open(pi_socket_t *ps, struct pi_sockaddr *addr, size_t addrlen) {
  struct pi_serial_data *data = (struct pi_serial_data *)ps->device->data;
  UInt32 port;
  UInt16 portId;
  int r = -1;
  
  pumpkin_s2id(&port, addr->pi_device);
  LOG((PI_DBG_DEV, PI_DBG_LVL_INFO, "DEV OPEN srmserial '%s' 0x%08X\n", addr->pi_device, port));

  if (SrmOpen(port, 57600, &portId) == errNone) {
    data->portId = portId;
    r = ps->sd;
  } else {
    LOG((PI_DBG_DEV, PI_DBG_LVL_ERR, "DEV OPEN srmserial '%s' 0x%08X failed\n", addr->pi_device, port));
  }

  return r;
}

static int s_close(pi_socket_t *ps) {
  struct pi_serial_data *data = (struct pi_serial_data *)ps->device->data;

  LOG((PI_DBG_DEV, PI_DBG_LVL_INFO, "DEV CLOSE srmserial fd: %d\n", ps->sd));

  close(ps->sd);
  return SrmClose(data->portId) == errNone ? 0 : -1;
}

static int s_poll(pi_socket_t *ps, int timeout) {
  struct pi_serial_data *data = (struct pi_serial_data *)ps->device->data;
  Err err;
  int r;

  // timeout: milliseconds -> ticks
  err = SrmReceiveWait(data->portId, 1, timeout / 10);

  if (err == errNone) {
    LOG((PI_DBG_DEV, PI_DBG_LVL_DEBUG, "DEV POLL srmserial found data on fd: %d\n", ps->sd));
    r = 0;
  } else if (err == serErrTimeOut) {
    r = pi_set_error(ps->sd, PI_ERR_SOCK_TIMEOUT);
  } else {
    r = pi_set_error(ps->sd, PI_ERR_GENERIC_SYSTEM);
  }

  return r;
}

static ssize_t s_write(pi_socket_t *ps, const unsigned char *buf, size_t len, int flags) {
  struct pi_serial_data *data = (struct pi_serial_data *)ps->device->data;
  ssize_t r;
  Err err;

  r = SrmSend(data->portId, buf, len, &err);

  if (err == errNone) {
    LOG((PI_DBG_DEV, PI_DBG_LVL_DEBUG, "DEV TX srmserial wrote %d bytes\n", r));
  } else {
    r = pi_set_error(ps->sd, PI_ERR_SOCK_IO);
  }

  return r;
}

static ssize_t s_read(pi_socket_t *ps, pi_buffer_t *buf, size_t len, int flags) {
  struct pi_serial_data *data = (struct pi_serial_data *)ps->device->data;
  ssize_t r;
  Err err;

  r = SrmReceive(data->portId, buf, len, 0, &err); // XXX timeout == 0

  if (err == errNone) {
    LOG((PI_DBG_DEV, PI_DBG_LVL_DEBUG, "DEV TX srmserial read %d bytes\n", r));
  } else {
    r = pi_set_error(ps->sd, PI_ERR_SOCK_IO);
  }

  return r;
}

static int s_flush(pi_socket_t *ps, int flags) {
  struct pi_serial_data *data = (struct pi_serial_data *) ps->device->data;
  UInt8 b;
  Err err;

  if (flags & PI_FLUSH_INPUT) {
    for (;;) {
      if (SrmReceive(data->portId, &b, 1, 0, &err) == 0 || err) break;
    }

    LOG((PI_DBG_DEV, PI_DBG_LVL_DEBUG, "DEV FLUSH srmserial flushed input buffer\n"));
  }

  return 0;
}

static int s_changebaud(pi_socket_t *ps) {
  struct pi_serial_data *data = (struct pi_serial_data *)ps->device->data;
  UInt32 value;
  UInt16 len;
  Err err;
  int r;

  LOG((PI_DBG_DEV, PI_DBG_LVL_DEBUG, "DEV SPEED srmserial switch to %d bps\n", (int)data->rate));

  value = data->rate;
  len = sizeof(value);
  err = SrmControl(data->portId, srmCtlSetBaudRate, &value, &len);

  if (err == errNone) {
    r = 0;
  } else {
    r = pi_set_error(ps->sd, PI_ERR_GENERIC_SYSTEM);
  }

  return r;
}

void pi_srm_impl_init(struct pi_serial_impl *impl) {
  impl->open       = s_open;
  impl->close      = s_close;
  impl->changebaud = s_changebaud;
  impl->write      = s_write;
  impl->read       = s_read;
  impl->flush      = s_flush;
  impl->poll       = s_poll;
}
