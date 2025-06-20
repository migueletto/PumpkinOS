#include "sys.h"
#include "script.h"
#include "thread.h"
#include "mutex.h"
#include "gpio.h"
#include "gpiobcm.h"
#include "gpiomonitor.h"
#include "bcm2835.h"
#include "debug.h"

struct gpio_t {
  int a;
};

gpio_t *gpio_bcm_open(void *data) {
  gpio_t *gpio;

  if ((gpio = sys_calloc(1, sizeof(gpio_t))) != NULL) {
  }

  return gpio;
}

int gpio_bcm_close(gpio_t *gpio) {
  int r = -1;

  if (gpio) {
    sys_free(gpio);
    r = 0;
  }

  return r;
}

int gpio_bcm_setup(gpio_t *gpio, int pin, int direction) {
  switch (direction) {
    case GPIO_IN:
      bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
      break;
    case GPIO_OUT:
      bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
      break;
    case GPIO_HIGH_OUT:
      bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
      bcm2835_gpio_write(pin, HIGH);
      break;
    case GPIO_LOW_OUT:
      bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
      bcm2835_gpio_write(pin, LOW);
      break;
    case GPIO_ALT0:
      debug(DEBUG_INFO, "GPIO", "pin %d set to ALT0", pin);
      bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_ALT0);
      break;
    default:
      debug(DEBUG_ERROR, "GPIO", "invalid direction %d for pin %d", direction, pin);
      return -1;
  }

  return 0;
}

int gpio_bcm_output(gpio_t *gpio, int pin, int value) {
  bcm2835_gpio_write(pin, value ? HIGH : LOW);

  return 0;
}

/*
Sets the first 32 GPIO output pins specified in the mask to the value given by value
value: values required for each bit masked in by mask, eg: (1 << RPI_GPIO_P1_03) | (1 << RPI_GPIO_P1_05)
mask: Mask of pins to affect. Use eg: (1 << RPI_GPIO_P1_03) | (1 << RPI_GPIO_P1_05)
void bcm2835_gpio_write_mask(uint32_t value, uint32_t mask);
*/

int gpio_bcm_output_multi(gpio_t *gpio, int n, int *pins, uint32_t values) {
  uint32_t pins_mask = 0, values_mask = 0;
  int i;

  if (n >= 32) n = 32;
  for (i = 0; i < n; i++) {
    pins_mask |= 1 << pins[i];
    values_mask |= (values & 1) << pins[i];
    values >>= 1;
  }
  bcm2835_gpio_write_mask(values_mask, pins_mask);

  return 0;
}

int gpio_bcm_input(gpio_t *gpio, int pin, int *value) {
  *value = bcm2835_gpio_lev(pin) == HIGH ? 1 : 0;

  return 0;
}

static int monitor_action(void *arg) {
  gpio_monitor_t *monitor;
  int value, old, n;

  monitor = (gpio_monitor_t *)arg;
  old = bcm2835_gpio_lev(monitor->pin);
  value = old;

  for (n = 0;; n++) {
    if (n == 200) {
      if (thread_must_end()) break;
      n = 0;
      value = !value;
    }

    value = bcm2835_gpio_lev(monitor->pin);
    if (value == old) {
      sys_usleep(50);
      continue;
    }

    if (monitor->callback(monitor->pe, monitor->pin, value, monitor->ref, monitor->data) != 0) {
      break;
    }

    old = value;
  }

  monitor->callback(monitor->pe, -1, 0, monitor->ref, monitor->data);
  sys_free(monitor);

  return 0;
}

int gpio_bcm_create_monitor(gpio_t *gpio, int pe, int pin, int (*callback)(int pe, int pin, int value, script_ref_t ref, void *data), script_ref_t ref, void *data) {
  gpio_monitor_t *monitor;
  int handle;

  if (callback == NULL) return -1;

  handle = -1;

  if ((monitor = sys_calloc(1, sizeof(gpio_monitor_t))) != NULL) {
    monitor->pin = pin;
    monitor->callback = callback;
    monitor->pe = pe;
    monitor->ref = ref;
    monitor->data = data;

    if ((handle = thread_begin(TAG_GPIO, monitor_action, monitor)) == -1) {
      sys_free(monitor);
    }
  }

  return handle;
}

int gpio_bcm_destroy_monitor(gpio_t *gpio, int handle) {
  return thread_end(TAG_GPIO, handle);
}
