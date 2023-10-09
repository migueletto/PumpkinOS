#include "sys.h"
#include "script.h"
#include "thread.h"
#include "pit_io.h"
#include "ptr.h"
#include "gpio.h"
#ifdef LINUX
#include "gpiobcm.h"
#endif
#include "debug.h"
#include "xalloc.h"

typedef struct {
  char *tag;
  gpio_provider_t *gpiop;
  gpio_t *gpio;
} libgpio_t;

static gpio_provider_t provider;
#ifdef LINUX
static gpio_provider_t provider_bcm;
#endif

static void gpio_destructor(void *p) {
  libgpio_t *libgpio;

  libgpio = (libgpio_t *)p;
  if (libgpio) {
    libgpio->gpiop->close(libgpio->gpio);
    xfree(libgpio);
  }
}

static int libgpio_open(int pe) {
  gpio_provider_t *gpiop;
  libgpio_t *libgpio;
  int ptr, r = -1;

  if ((gpiop = script_get_pointer(pe, GPIO_PROVIDER)) != NULL) {
    if ((libgpio = xcalloc(1, sizeof(libgpio_t))) != NULL) {
      if ((libgpio->gpio = gpiop->open(gpiop->data)) != NULL) {
        libgpio->gpiop = gpiop;
        libgpio->tag = TAG_GPIO;

        if ((ptr = ptr_new(libgpio, gpio_destructor)) != -1) {
          r = script_push_integer(pe, ptr);
        } else {
          gpiop->close(libgpio->gpio);
          xfree(libgpio);
        }
      } else {
        xfree(libgpio);
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

#ifdef LINUX
static int libgpio_bcm(int pe) {
  int bcm, r = -1;

  if (script_get_boolean(pe, 0, &bcm) == 0) {
    debug(DEBUG_INFO, "GPIO", "registering %s (%s)", GPIO_PROVIDER, bcm ? "bcm" : "regular");
    script_set_pointer(pe, GPIO_PROVIDER, bcm ? &provider_bcm : &provider);
    if (bcm) {
      gpio_bcm_init();
    } else {
      gpio_bcm_finish();
    }
  }

  return r;
}
#endif

int libgpio_load(void) {
  xmemset(&provider, 0, sizeof(gpio_provider_t));
  provider.open = gpio_open;
  provider.close = gpio_close;
  provider.setup = gpio_setup;
  provider.output = gpio_output;
  provider.output_multi = gpio_output_multi;
  provider.input = gpio_input;
  provider.create_monitor = gpio_create_monitor;
  provider.destroy_monitor = gpio_destroy_monitor;
  gpio_init();

#ifdef LINUX
  xmemset(&provider_bcm, 0, sizeof(gpio_provider_t));
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
  gpio_finish();
#ifdef LINUX
  gpio_bcm_finish();
#endif

  return 0;
}

int libgpio_init(int pe, script_ref_t obj) {
  if (script_get_pointer(pe, GPIO_PROVIDER) == NULL) {
    debug(DEBUG_INFO, "GPIO", "registering %s", GPIO_PROVIDER);
    script_set_pointer(pe, GPIO_PROVIDER, &provider);
  }

  script_add_iconst(pe, obj, "IN",       GPIO_IN);
  script_add_iconst(pe, obj, "OUT",      GPIO_OUT);
  script_add_iconst(pe, obj, "LOW_OUT",  GPIO_LOW_OUT);
  script_add_iconst(pe, obj, "HIGH_OUT", GPIO_HIGH_OUT);
  script_add_iconst(pe, obj, "ALT0",     GPIO_ALT0);

  script_add_function(pe, obj, "open",    libgpio_open);
  script_add_function(pe, obj, "close",   libgpio_close);
  script_add_function(pe, obj, "setup",   libgpio_setup);
  script_add_function(pe, obj, "output",  libgpio_output);
  script_add_function(pe, obj, "input",   libgpio_input);
  script_add_function(pe, obj, "monitor", libgpio_monitor);
  script_add_function(pe, obj, "destroy", libgpio_destroy);
#ifdef LINUX
  script_add_function(pe, obj, "bcm",     libgpio_bcm);
#endif

  return 0;
}
