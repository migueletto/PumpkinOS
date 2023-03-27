#include <PalmOS.h>

#include "sys.h"
#include "script.h"
#include "pumpkin.h"
#include "command.h"
#include "rcpexport.h"
#include "xalloc.h"
#include "debug.h"

static int cmain(int argc, char *argv[]) {
  int r = -1;

  if (argc == 2) {
    r = rcp_export(argv[0], argv[1]);
  }

  return r;
}

static void *PluginMain(void *p) {
  command_builtin_t *c = (command_builtin_t *)p;

  if (c) {
    c->name = "rcpexport";
    c->main = cmain;
  }

  return c;
}

pluginMainF PluginInit(UInt32 *type, UInt32 *id) {
  *type = commandPluginType;
  *id = 'RcpE';

  return PluginMain;
}
