#ifndef PIT_GPIO_H
#define PIT_GPIO_H

#define GPIO_IN        0
#define GPIO_OUT       1
#define GPIO_LOW_OUT   2
#define GPIO_HIGH_OUT  3
#define GPIO_ALT0      4

#define GPIO_PROVIDER "gpio_provider"

#define TAG_GPIO "gpio"

typedef struct gpio_t gpio_t;

typedef struct {
  gpio_t *(*open)(void *data);
  int (*close)(gpio_t *gpio);
  int (*setup)(gpio_t *gpio, int pin, int direction);
  int (*output)(gpio_t *gpio, int pin, int value);
  int (*output_multi)(gpio_t *gpio, int n, int *pins, uint32_t values);
  int (*input)(gpio_t *gpio, int pin, int *value);
  int (*create_monitor)(gpio_t *gpio, int pe, int pin, int (*callback)(int pe, int pin, int value, script_ref_t ref, void *data), script_ref_t ref, void *data);
  int (*destroy_monitor)(gpio_t *gpio, int handle);
  void *data;
} gpio_provider_t;

int gpio_init(void);
int gpio_finish(void);
gpio_t *gpio_open(void *data);
int gpio_close(gpio_t *gpio);
int gpio_setup(gpio_t *gpio, int pin, int direction);
int gpio_output(gpio_t *gpio, int pin, int value);
int gpio_output_multi(gpio_t *gpio, int n, int *pins, uint32_t values);
int gpio_input(gpio_t *gpio, int pin, int *value);
int gpio_create_monitor(gpio_t *gpio, int pe, int pin, int (*callback)(int pe, int pin, int value, script_ref_t ref, void *data), script_ref_t ref, void *data);
int gpio_destroy_monitor(gpio_t *gpio, int handle);

int gpio_map(int pin1, int pin2);

#endif
