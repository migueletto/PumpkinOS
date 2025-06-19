gpio_t *gpio_sys_open(void *data);
int gpio_sys_close(gpio_t *gpio);
int gpio_sys_setup(gpio_t *gpio, int pin, int direction);
int gpio_sys_output(gpio_t *gpio, int pin, int value);
int gpio_sys_output_multi(gpio_t *gpio, int n, int *pins, uint32_t values);
int gpio_sys_input(gpio_t *gpio, int pin, int *value);
int gpio_sys_create_monitor(gpio_t *gpio, int pe, int pin, int (*callback)(int pe, int pin, int value, script_ref_t ref, void *data), script_ref_t ref, void *data);
int gpio_sys_destroy_monitor(gpio_t *gpio, int handle);
