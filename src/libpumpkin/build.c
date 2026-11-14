#include <PalmOS.h>

#include "pumpkin.h"

#ifdef BUILD_DEBUG
#define D "D"
#else
#define D
#endif

const char *pumpkin_get_build(void) {
#if defined(GIT_COMMIT_ID)
#if defined(GIT_TAG_NAME)
  return GIT_TAG_NAME D;
#else
#if defined(GIT_NOT_CLEAN)
  return GIT_COMMIT_ID "*" D;
#else
  return GIT_COMMIT_ID " " D;
#endif // GIT_NOT_CLEAN
#endif // GIT_TAG_NAME
#else
  return "unknown " D;
#endif // GIT_COMMIT_ID
}
