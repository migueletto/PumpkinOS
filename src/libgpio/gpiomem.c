#include "sys.h"
#include "script.h"
#include "gpio.h"
#include "gpiomonitor.h"
#include "gpiomem.h"
#include "debug.h"

#define GPIO_MEM "gpio_mem"
#define MAX_PINS 64

struct gpio_t {
  uint8_t dir[MAX_PINS];
  uint8_t value[MAX_PINS];
  gpio_monitor_t monitor[MAX_PINS];
};

gpio_t *gpio_mem_init(void) {
  return sys_calloc(1, sizeof(gpio_t));
}

int gpio_mem_finish(gpio_t *gpio) {
  if (gpio) sys_free(gpio);
  return 0;
}

gpio_t *gpio_mem_open(void *data) {
  return data;
}

int gpio_mem_close(gpio_t *gpio) {
  return 0;
}

static int gpio_check(int pin) {
  int r = 0;

  if (pin < 0 || pin >= MAX_PINS) {
    debug(DEBUG_ERROR, "GPIO", "invalid pin %d", pin);
    r = -1;
  }

  return r;
}

int gpio_mem_setup(gpio_t *gpio, int pin, int direction) {
  int r = -1;

  if (gpio_check(pin) == -1) return -1;

  switch (direction) {
    case GPIO_IN:
    case GPIO_OUT:
    case GPIO_LOW_OUT:
    case GPIO_HIGH_OUT:
      gpio->dir[pin] = direction;
      r = 0;
      break;
    default:
      debug(DEBUG_ERROR, "GPIO", "invalid direction");
      break;
  }

  return r;
}

int gpio_mem_output(gpio_t *gpio, int pin, int value) {
  int r = -1;

  if (gpio_check(pin) == -1) return -1;

  if (gpio->dir[pin] == GPIO_OUT) {
    value = value ? 1 : 0;
    if (gpio->value[pin] != value) {
      if (gpio->monitor[pin].callback) {
        gpio->monitor[pin].callback(gpio->monitor[pin].pe, gpio->monitor[pin].pin, value, gpio->monitor[pin].ref, gpio->monitor[pin].data);
      }
    }
    gpio->value[pin] = value;
  }

  return r;
}

int gpio_mem_output_multi(gpio_t *gpio, int n, int *pins, uint32_t values) {
  int i, r = 0;

  if (n >= 32) n = 32;

  for (i = 0; i < n; i++) {
    if ((r = gpio_mem_output(gpio, pins[i], values & 1)) == -1) break;
    values >>= 1;
  }

  return r;
}

int gpio_mem_input(gpio_t *gpio, int pin, int *value) {
  *value = 0;
  if (gpio_check(pin) == -1) return -1;

  *value = gpio->value[pin];

  return 0;
}

int gpio_mem_create_monitor(gpio_t *gpio, int pe, int pin, int (*callback)(int pe, int pin, int value, script_ref_t ref, void *data), script_ref_t ref, void *data) {
  if (gpio_check(pin) == -1) return -1;
  if (callback == NULL) return -1;

  if (gpio->dir[pin] != GPIO_IN) {
    debug(DEBUG_ERROR, "GPIO", "attempt to monitor from non-input pin %d", pin);
    return -1;
  }

  gpio->monitor[pin].pin = pin;
  gpio->monitor[pin].callback = callback;
  gpio->monitor[pin].pe = pe;
  gpio->monitor[pin].ref = ref;
  gpio->monitor[pin].data = data;

  return pin + 1;
}

int gpio_mem_destroy_monitor(gpio_t *gpio, int handle) {
  int pin = handle - 1;

  if (gpio_check(pin) == -1) return -1;
  sys_memset(&gpio->monitor[pin], 0, sizeof(gpio_monitor_t));
  
  return 0;
}
