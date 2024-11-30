#include "sys.h"
#include "disasm.h"
#include "darm.h"
#include "debug.h"

void disasm(uint32_t addr, uint32_t op, char *extra) {
  darm_t darm;
  darm_str_t str;

  darm_armv7_disasm(&darm, op);
  darm_str(&darm, &str);
  debug(DEBUG_INFO, "ARM", "0x%08X: 0x%08X %s %s", addr, op, str.total, extra ? extra : "");
}
