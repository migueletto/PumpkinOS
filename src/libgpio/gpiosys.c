#include "sys.h"
#include "script.h"
#include "gpio.h"
#include "gpiomonitor.h"
#include "gpiosys.h"
#include "thread.h"
#include "debug.h"

#define GPIO_MASK "/sys/class/gpio/gpio%d/%s"

#define GPIO_MAX 1024

struct gpio_t {
  int pin_direction[GPIO_MAX+1];
};

gpio_t *gpio_sys_open(void *data) {
  gpio_t *gpio;
  int i;

  if ((gpio = sys_calloc(1, sizeof(gpio_t))) == NULL) {
    return NULL;
  }

  for (i = 0; i <= GPIO_MAX; i++) {
    gpio->pin_direction[i] = -1;
  }

  return gpio;
}

int gpio_sys_close(gpio_t *gpio) {
  int r = -1;

  if (gpio) {
    sys_free(gpio);
    r = 0;
  }

  return r;
}

static int open_pin(int pin, int mode, char *op) {
  char buf[64];
  int fd;

  sys_snprintf(buf, sizeof(buf)-1, GPIO_MASK, pin, op);
  if ((fd = sys_open(buf, mode)) == -1) {
    debug_errno("GPIO", "open \"%s\" for %s", buf, mode == SYS_READ ? "reading" : "writing");
  }

  return fd;
}

static int write_pin(int fd, int pin, char *type, char *cmd) {
  int n, r;

  n = sys_strlen(cmd);
  r = sys_write(fd, (uint8_t *)cmd, n);
  if (r == -1) {
    debug_errno("GPIO", "write %d bytes to pin %d %s", n, pin, type);
  }

  return r == n ? 0 : -1;
}

static int gpio_write(int pin, char *type, char *cmd) {
  int fd, r;

  r = -1;
  if ((fd = open_pin(pin, SYS_WRITE, type)) != -1) {
    r = write_pin(fd, pin, type, cmd);
    sys_close(fd);
  }

  return r;
}

static int gpio_value(int r, char *v, int *value) {
  if (r == 2 && v[1] == '\n') {
    r = 0;
    if (v[0] == '0') *value = 0;
    else if (v[0] == '1') *value = 1;
    else r = -1;

  } else {
    r = -1;
  }

  return r;
}

static int gpio_read_value(int pin, int *value) {
  char v[2];
  int fd, r;

  *value = 0;
  r = -1;

  if ((fd = open_pin(pin, SYS_READ, "value")) != -1) {
    r = sys_read(fd, (uint8_t *)v, 2);
    r = gpio_value(r, v, value);
    sys_close(fd);
  }

  return r;
}

static int gpio_check(int pin) {
  int r = 0;

  if (pin < 0 || pin > GPIO_MAX) {
    debug(DEBUG_ERROR, "GPIO", "invalid pin %d", pin);
    r = -1;
  }

  return r;
}

int gpio_sys_setup(gpio_t *gpio, int pin, int direction) {
  char *cmd;
  int r;

  if (gpio_check(pin) == -1) return -1;

  switch (direction) {
    case GPIO_IN:
      cmd = "in\n";
      break;
    case GPIO_OUT:
      cmd = "out\n";
      break;
    case GPIO_LOW_OUT:
      cmd = "low\n";
      direction = GPIO_OUT;
      break;
    case GPIO_HIGH_OUT:
      cmd = "high\n";
      direction = GPIO_OUT;
      break;
    default:
      debug(DEBUG_ERROR, "GPIO", "invalid direction");
      return -1;
  }

  r = gpio_write(pin, "direction", cmd);
  debug(DEBUG_INFO, "GPIO", "set pin %d direction to %s : %s", pin, direction == GPIO_IN ? "input" : "output", r ? "error" : "ok");

  if (r == 0) {
    gpio->pin_direction[pin] = direction;
  }

  return r;
}

int gpio_sys_output(gpio_t *gpio, int pin, int value) {
  char cmd[2];
  int r = -1;

  if (gpio_check(pin) == -1) return -1;

  value = value ? 1 : 0;
  cmd[0] = '0' + value;
  cmd[1] = 0;

  if (gpio->pin_direction[pin] != GPIO_OUT) {
    debug(DEBUG_ERROR, "GPIO", "attempt to write to non-output pin %d", pin);
    pin = -1;
  }

  if (pin != -1) {
    r = gpio_write(pin, "value", cmd);
    debug(DEBUG_TRACE, "GPIO", "write %d to pin %d : %s", value, pin, r ? "error" : "ok");
  }

  return r;
}

int gpio_sys_output_multi(gpio_t *gpio, int n, int *pins, uint32_t values) {
  int i, r = 0;

  if (n >= 32) n = 32;

  for (i = 0; i < n; i++) {
    if ((r = gpio_sys_output(gpio, pins[i], values & 1)) == -1) break;
    values >>= 1;
  }

  return r;
}

int gpio_sys_input(gpio_t *gpio, int pin, int *value) {
  int r = -1;

  *value = 0;
  if (gpio_check(pin) == -1) return -1;

  if (gpio->pin_direction[pin] != GPIO_IN) {
    debug(DEBUG_ERROR, "GPIO", "attempt to read from non-input pin %d", pin);
    pin = -1;
  }

  if (pin != -1) {
    r = gpio_read_value(pin, value);

    if (r == -1) {
      debug(DEBUG_ERROR, "GPIO", "error reading from pin %d", pin);
    } else {
      debug(DEBUG_INFO, "GPIO", "read %d from pin %d", *value, pin);
    }
  }

  return r;
}

static int monitor_action(void *arg) {
  gpio_monitor_t *monitor;
  int first, old, value, n, nread;
  char v[2];

  monitor = (gpio_monitor_t *)arg;

  for (first = 1; !thread_must_end();) {
    if ((n = sys_read_timeout(monitor->fd, (uint8_t *)v, 2, &nread, 10000)) < 0) {
      break;
    }

    if (n == 0) {
      // nothing to read, continue
      continue;
    }

    if (n < 0 || nread <= 0) {
      break;
    }

    if (gpio_value(nread, v, &value) != 0) {
      break;
    }

    if (first) {
      old = value;
      first = 0;

    } else if (value != old) {
      old = value;
      if (monitor->callback(monitor->pe, monitor->pin, value, monitor->ref, monitor->data) != 0) {
        break;
      }
    }

    if (sys_seek(monitor->fd, 0, SYS_SEEK_SET) == -1) {
      break;
    }
  }

  monitor->callback(monitor->pe, -1, 0, monitor->ref, monitor->data);
  sys_close(monitor->fd);
  sys_free(monitor);

  return 0;
}

int gpio_sys_create_monitor(gpio_t *gpio, int pe, int pin, int (*callback)(int pe, int pin, int value, script_ref_t ref, void *data), script_ref_t ref, void *data) {
  gpio_monitor_t *monitor;
  int fd, handle;

  if (gpio_check(pin) == -1) return -1;
  if (callback == NULL) return -1;

  monitor = NULL;
  handle = -1;

  if (gpio->pin_direction[pin] != GPIO_IN) {
    debug(DEBUG_ERROR, "GPIO", "attempt to monitor from non-input pin %d", pin);
    pin = -1;
  }

  if (pin != -1) {
    if ((fd = open_pin(pin, SYS_READ, "value")) != -1) {
      if ((monitor = sys_calloc(1, sizeof(gpio_monitor_t))) != NULL) {
        monitor->pin = pin;
        monitor->fd = fd;
        monitor->callback = callback;
        monitor->pe = pe;
        monitor->ref = ref;
        monitor->data = data;
        if ((handle = thread_begin(TAG_GPIO_MONITOR, monitor_action, monitor)) == -1) {
          sys_close(fd);
          sys_free(monitor);
        }
      } else {
        sys_close(fd);
      }
    }
  }

  return handle;
}

int gpio_sys_destroy_monitor(gpio_t *gpio, int handle) {
  return thread_end(TAG_GPIO_MONITOR, handle);
}
