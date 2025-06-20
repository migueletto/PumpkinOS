gpio_t *gpio_bcm_open(void *data);
int gpio_bcm_close(gpio_t *gpio);
int gpio_bcm_setmode(gpio_t *gpio, int mode);
int gpio_bcm_setup(gpio_t *gpio, int pin, int direction);
int gpio_bcm_output(gpio_t *gpio, int pin, int value);
int gpio_bcm_output_multi(gpio_t *gpio, int n, int *pins, uint32_t values);
int gpio_bcm_input(gpio_t *gpio, int pin, int *value);
int gpio_bcm_create_monitor(gpio_t *gpio, int pe, int pin, int (*callback)(int pe, int pin, int value, script_ref_t ref, void *data), script_ref_t ref, void *data);
int gpio_bcm_destroy_monitor(gpio_t *gpio, int handle);
