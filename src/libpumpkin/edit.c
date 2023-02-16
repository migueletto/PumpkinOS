#include <PalmOS.h>
#include <time.h>
#include <sys/time.h>

#include "pumpkin.h"
#include "editor.h"
#include "edit.h"

int editor_get_plugin(editor_t *e, UInt32 id) {
  pumpkin_plugin_t *plugin;
  int r = -1;

  if (e) {
    if ((plugin = pumpkin_get_plugin(editPluginType, id)) != NULL) {
      plugin->pluginMain(e);
      r = 0;
    }
  }

  return r;
}
