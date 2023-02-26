#include <PalmOS.h>

#include "sys.h"
#include "pumpkin.h"
#include "syntax.h"

syntax_plugin_t *syntax_get_plugin(char *ext) {
  syntax_plugin_t *syntax = NULL;
  pumpkin_plugin_t *plugin;
  UInt32 syntaxId;
  char buf[8];

  if (ext && ext[0]) {
    buf[0] = '.';
    buf[1] = ext[0];
    buf[2] = ext[1] ? ext[1] : ' ';
    buf[3] = ext[2] ? ext[2] : ' ';
    pumpkin_s2id(&syntaxId, buf);

    if ((plugin = pumpkin_get_plugin(syntaxPluginType, syntaxId)) != NULL) {
      syntax = plugin->pluginMain(NULL);
    }
  }

  return syntax;
}
