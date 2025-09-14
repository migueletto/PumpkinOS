#include <PalmOS.h>

#include "pumpkin.h"

const char *pumpkin_get_build(void) {
#if defined(GIT_COMMIT_ID)
#if defined(GIT_NOT_CLEAN)
  return GIT_COMMIT_ID "*";
#else
  return GIT_COMMIT_ID;
#endif
#else
  return "<unknown>";
#endif
}
