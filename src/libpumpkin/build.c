#include <PalmOS.h>

#include "pumpkin.h"

const char *pumpkin_get_build(void) {
#if defined(GIT_COMMIT_ID)
#if defined(GIT_TAG_NAME)
  return GIT_TAG_NAME;
#else
#if defined(GIT_NOT_CLEAN)
  return GIT_COMMIT_ID "*";
#else
  return GIT_COMMIT_ID;
#endif // GIT_NOT_CLEAN
#endif // GIT_TAG_NAME
#else
  return "<unknown>";
#endif // GIT_COMMIT_ID
}
