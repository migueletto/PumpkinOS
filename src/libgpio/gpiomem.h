gpio_t *gpio_mem_init(void);
int gpio_mem_finish(gpio_t *gpio);

gpio_t *gpio_mem_open(void *data);
int gpio_mem_close(gpio_t *gpio);
int gpio_mem_setup(gpio_t *gpio, int pin, int direction);
int gpio_mem_output(gpio_t *gpio, int pin, int value);
int gpio_mem_output_multi(gpio_t *gpio, int n, int *pins, uint32_t values);
int gpio_mem_input(gpio_t *gpio, int pin, int *value);
int gpio_mem_create_monitor(gpio_t *gpio, int pe, int pin, int (*callback)(int pe, int pin, int value, script_ref_t ref, void *data), script_ref_t ref, void *data);
int gpio_mem_destroy_monitor(gpio_t *gpio, int handle);
