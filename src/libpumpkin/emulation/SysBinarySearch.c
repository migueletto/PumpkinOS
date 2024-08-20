#include <PalmOS.h>

// Int16 searchF(void const *searchData, void const *arrayData, Int32 other)

Boolean SysBinarySearch(void const *baseP, UInt16 numOfElements, Int16 width, SearchFuncPtr searchF, void const *searchData, Int32 other, Int32 *position, Boolean findFirst) {
  UInt16 i;
  UInt8 *p;
  Boolean found = false;

  p = (UInt8 *)baseP;

  if (p && searchF && searchData) {
    for (i = 0; i < numOfElements && !found; i++) {
      if (searchF(searchData, p + i*width, other) == 0) {
        if (position) *position = i;
        found = true;
      }
    }
  }

  return found;
}
