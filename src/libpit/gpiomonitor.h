typedef struct {
  int pin;
  int fd;
  int (*callback)(int pe, int pin, int value, script_ref_t ref, void *data);
  int pe;
  script_ref_t ref;
  void *data;
} gpio_monitor_t;

#define TAG_GPIO_MONITOR  "gpio_monitor"
