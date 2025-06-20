#include "sys.h"
#include "script.h"
#include "thread.h"
#include "pit_io.h"
#include "ptr.h"
#include "gpio.h"
#include "gpiomem.h"
#ifdef LINUX
#include "gpiosys.h"
#endif
#ifdef RPI
#include "gpiobcm.h"
#endif
#include "debug.h"

typedef struct {
  char *tag;
  gpio_provider_t *gpiop;
  gpio_t *gpio;
} libgpio_t;

static gpio_provider_t provider_mem;
#ifdef LINUX
static gpio_provider_t provider_sys;
#endif
#ifdef RPI
static gpio_provider_t provider_bcm;
#endif

static void gpio_destructor(void *p) {
  libgpio_t *libgpio;

  libgpio = (libgpio_t *)p;
  if (libgpio) {
    libgpio->gpiop->close(libgpio->gpio);
    sys_free(libgpio);
  }
}

static int libgpio_open(int pe) {
  gpio_provider_t *gpiop;
  libgpio_t *libgpio;
  int ptr, r = -1;

  if ((gpiop = script_get_pointer(pe, GPIO_PROVIDER)) != NULL) {
    if ((libgpio = sys_calloc(1, sizeof(libgpio_t))) != NULL) {
      if ((libgpio->gpio = gpiop->open(gpiop->data)) != NULL) {
        libgpio->gpiop = gpiop;
        libgpio->tag = TAG_GPIO;

        if ((ptr = ptr_new(libgpio, gpio_destructor)) != -1) {
          r = script_push_integer(pe, ptr);
        } else {
          gpiop->close(libgpio->gpio);
          sys_free(libgpio);
        }
      } else {
        sys_free(libgpio);
      }
    }
  }

  return r;
}

static int libgpio_close(int pe) {
  script_int_t ptr;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    if (ptr_free(ptr, TAG_GPIO) == 0) {
      r = script_push_boolean(pe, 1);
    }
  }

  return r;
}

static int libgpio_setup(int pe) {
  script_int_t ptr, pin, direction;
  libgpio_t *libgpio;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &pin) == 0 &&
      script_get_integer(pe, 2, &direction) == 0) {

    if ((libgpio = ptr_lock(ptr, TAG_GPIO)) != NULL) {
      r = libgpio->gpiop->setup ? libgpio->gpiop->setup(libgpio->gpio, pin, direction) : -1;
      ptr_unlock(ptr, TAG_GPIO);
    }
  }

  return script_push_boolean(pe, r == 0);
}

static int libgpio_output(int pe) {
  script_int_t ptr, pin, value;
  libgpio_t *libgpio;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &pin) == 0 &&
      script_get_integer(pe, 2, &value) == 0) {

    if ((libgpio = ptr_lock(ptr, TAG_GPIO)) != NULL) {
      r = libgpio->gpiop->output ? libgpio->gpiop->output(libgpio->gpio, pin, value) : -1;
      ptr_unlock(ptr, TAG_GPIO);
    }
  }

  return script_push_boolean(pe, r == 0);
}

static int libgpio_input(int pe) {
  script_int_t ptr, pin, value;
  libgpio_t *libgpio;
  int r;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &pin) == 0) {

    if ((libgpio = ptr_lock(ptr, TAG_GPIO)) != NULL) {
      r = libgpio->gpiop->input ? libgpio->gpiop->input(libgpio->gpio, pin, &value) : -1;
      if (r == 0) {
        r = script_push_integer(pe, value);
      }
      ptr_unlock(ptr, TAG_GPIO);
    }
  }

  return r;
}

static int gpio_callback(int pe, int pin, int value, script_ref_t ref, void *data) {
  script_arg_t ret;

  if (pin == -1) {
    script_remove_ref(pe, ref);
    return 1;
  }

  return script_call(pe, ref, &ret, "IB", mkint(pin), value);
}

static int libgpio_monitor(int pe) {
  script_int_t ptr, pin, handle;
  script_ref_t ref;
  libgpio_t *libgpio;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_function(pe, 1, &ref) == 0 &&
      script_get_integer(pe, 2, &pin) == 0) {

    if ((libgpio = ptr_lock(ptr, TAG_GPIO)) != NULL) {
      handle = libgpio->gpiop->create_monitor ? libgpio->gpiop->create_monitor(libgpio->gpio, pe, pin, gpio_callback, ref, NULL) : -1;
      if (handle != -1) {
        r = script_push_integer(pe, handle);
      }
      ptr_unlock(ptr, TAG_GPIO);
    }
  }

  return r;
}

static int libgpio_destroy(int pe) {
  script_int_t ptr, handle;
  libgpio_t *libgpio;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &handle) == 0) {

    if ((libgpio = ptr_lock(ptr, TAG_GPIO)) != NULL) {
      r = libgpio->gpiop->destroy_monitor ? libgpio->gpiop->destroy_monitor(libgpio->gpio, handle) : -1;
      ptr_unlock(ptr, TAG_GPIO);
    }
  }

  return script_push_boolean(pe, r == 0);
}

#if !defined(KERNEL)
static
#endif
int gpio_select(int pe, char *type) {
  int r = -1;

  debug(DEBUG_INFO, "GPIO", "registering %s %s", GPIO_PROVIDER, type);
  if (!sys_strcmp(type, "mem")) {
    script_set_pointer(pe, GPIO_PROVIDER, &provider_mem);
    r = 0;
#if defined(LINUX) || defined(RPI)
  } else if (!sys_strcmp(type, "sys")) {
    script_set_pointer(pe, GPIO_PROVIDER, &provider_sys);
    r = 0;
#endif
#if defined(RPI)
  } else if (!sys_strcmp(type, "bcm")) {
    script_set_pointer(pe, GPIO_PROVIDER, &provider_bcm);
    r = 0;
#endif
  } else {
    debug(DEBUG_INFO, "GPIO", "invalid %s %s", GPIO_PROVIDER, type);
  }

  return r;
}

static int libgpio_select(int pe) {
  char *type = NULL;
  int r = -1;

  if (script_get_string(pe, 0, &type) == 0) {
    r = gpio_select(pe, type);
  }

  if (type) sys_free(type);

  return r;
}

int libgpio_load(void) {
  sys_memset(&provider_mem, 0, sizeof(gpio_provider_t));
  provider_mem.open = gpio_mem_open;
  provider_mem.close = gpio_mem_close;
  provider_mem.setup = gpio_mem_setup;
  provider_mem.output = gpio_mem_output;
  provider_mem.output_multi = gpio_mem_output_multi;
  provider_mem.input = gpio_mem_input;
  provider_mem.create_monitor = gpio_mem_create_monitor;
  provider_mem.destroy_monitor = gpio_mem_destroy_monitor;
  provider_mem.data = gpio_mem_init();

#ifdef LINUX
  sys_memset(&provider_sys, 0, sizeof(gpio_provider_t));
  provider_sys.open = gpio_sys_open;
  provider_sys.close = gpio_sys_close;
  provider_sys.setup = gpio_sys_setup;
  provider_sys.output = gpio_sys_output;
  provider_sys.output_multi = gpio_sys_output_multi;
  provider_sys.input = gpio_sys_input;
  provider_sys.create_monitor = gpio_sys_create_monitor;
  provider_sys.destroy_monitor = gpio_sys_destroy_monitor;
#endif

#ifdef RPI
  sys_memset(&provider_bcm, 0, sizeof(gpio_provider_t));
  provider_bcm.open = gpio_bcm_open;
  provider_bcm.close = gpio_bcm_close;
  provider_bcm.setup = gpio_bcm_setup;
  provider_bcm.output = gpio_bcm_output;
  provider_bcm.output_multi = gpio_bcm_output_multi;
  provider_bcm.input = gpio_bcm_input;
  provider_bcm.create_monitor = gpio_bcm_create_monitor;
  provider_bcm.destroy_monitor = gpio_bcm_destroy_monitor;
#endif

  return 0;
}

int libgpio_unload(void) {
  gpio_mem_finish(provider_mem.data);

  return 0;
}

int libgpio_init(int pe, script_ref_t obj) {
  if (script_get_pointer(pe, GPIO_PROVIDER) == NULL) {
#ifdef LINUX
    gpio_select(pe, "sys");
#else
    gpio_select(pe, "mem");
#endif
  }

  script_add_iconst(pe, obj, "IN",       GPIO_IN);
  script_add_iconst(pe, obj, "OUT",      GPIO_OUT);
  script_add_iconst(pe, obj, "LOW_OUT",  GPIO_LOW_OUT);
  script_add_iconst(pe, obj, "HIGH_OUT", GPIO_HIGH_OUT);
#ifdef RPI
  script_add_iconst(pe, obj, "ALT0",     GPIO_ALT0);
#endif

  script_add_function(pe, obj, "open",    libgpio_open);
  script_add_function(pe, obj, "close",   libgpio_close);
  script_add_function(pe, obj, "setup",   libgpio_setup);
  script_add_function(pe, obj, "output",  libgpio_output);
  script_add_function(pe, obj, "input",   libgpio_input);
  script_add_function(pe, obj, "monitor", libgpio_monitor);
  script_add_function(pe, obj, "destroy", libgpio_destroy);

  script_add_function(pe, obj, "select",  libgpio_select);

  return 0;
}
