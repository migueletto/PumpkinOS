#include "sys.h"
#include "script.h"
#include "vfs.h"
#include "filter.h"
#include "shell.h"
#include "oshell.h"
#include "debug.h"

static shell_command_t oshell = { "oshell", "oshell", 0, 0, cmd_oshell, "Starts an OS shell.", NULL };

int liboshell_init(int pe, script_ref_t obj) {
  shell_provider_t *p;
  int r = -1;

  if ((p = script_get_pointer(pe, SHELL_PROVIDER)) != NULL) {
    p->add(&oshell);
    r = 0;
  } else {
    debug(DEBUG_ERROR, "OSHELL", "shell provider not found");
  }

  return r;
}
