#include <PalmOS.h>

#include "sys.h"
#include "script.h"
#include "pumpkin.h"
#include "command.h"
#include "rcpexport.h"
#include "xalloc.h"
#include "debug.h"

static int rcpexport(int pe) {
  char *prc = NULL;
  char *rcp = NULL;
  int r = -1;

  if (script_get_string(pe, 0, &prc) == 0 &&
      script_get_string(pe, 1, &rcp) == 0) {

    r = rcp_export(prc, rcp);
  }

  if (prc) xfree(prc);
  if (rcp) xfree(rcp);

  return r;
}

static void *PluginMain(void *p) {
  command_builtin_t *c = (command_builtin_t *)p;

  if (c) {
    c->name = "rcpexport";
    c->function = rcpexport;
  }

  return c;
}

pluginMainF PluginInit(UInt32 *type, UInt32 *id) {
  *type = commandPluginType;
  *id = 'RcpE';

  return PluginMain;
}
