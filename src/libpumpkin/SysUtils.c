#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "bytes.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define PALMOS_MODULE "System"

typedef struct {
  CmpFuncPtr comparF;
  Int32 other;
  CmpFuncPPtr comparFP;
  void *otherP;
  UInt32 comparF68k;
} sysu_module_t;

int SysUInitModule(void) {
  sysu_module_t *module;

  if ((module = xcalloc(1, sizeof(sysu_module_t))) == NULL) {
    return -1;
  }

  pumpkin_set_local_storage(sysu_key, module);

  return 0;
}

int SysUFinishModule(void) {
  sysu_module_t *module = (sysu_module_t *)pumpkin_get_local_storage(sysu_key);

  if (module) {
    xfree(module);
  }

  return 0;
}

void SysCopyStringResource(Char *string, Int16 theID) {
  MemHandle h;
  Char *s;

  if (string) {
    if ((h = DmGetResource(strRsc, theID)) != NULL) {
      if ((s = MemHandleLock(h)) != NULL) {
        StrCopy(string, s);
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    } else {
      debug(DEBUG_ERROR, PALMOS_MODULE, "SysCopyStringResource string %d not found", theID);
    }
  }
}

// Form an array of pointers to strings in a block. Useful for setting the items of a list.

MemHandle SysFormPointerArrayToStrings(Char *c, Int16 stringCount) {
  MemHandle h = NULL;
  Char **p;
  Int16 i, j;

  if (c) {
    if ((h = MemHandleNew(stringCount * sizeof(Char *))) != NULL) {
      if ((p = MemHandleLock(h)) != NULL) {
        for (i = 0, j = 0; j < stringCount;) {
          if (c[i] == 0) {
            p[j] = c;
            c = &c[i+1];
            i = 0;
            j++;
          } else {
            i++;
          }
        }
        MemHandleUnlock(h);
      }
    }
  }

  return h;
}

// Return a random number anywhere from 0 to sysRandomMax
Int16 SysRandom(Int32 newSeed) {
  int32_t r;

  if (newSeed) {
    sys_srand(newSeed);
  }

  r = sys_rand();
  if (r < 0) r = -r;

  return r % sysRandomMax;
}

/*
0000000 nul nul  bs   N   e   w  sp   M   o   o   n nul   W   a   x   i
0000020   n   g  sp   c   r   e   s   c   e   n   t nul   F   i   r   s
0000040   t  sp   q   u   a   r   t   e   r nul   W   a   x   i   n   g
0000060  sp   g   i   b   b   o   u   s nul   F   u   l   l  sp   M   o
0000100   o   n nul   W   a   n   i   n   g  sp   g   i   b   b   o   u
0000120   s nul   T   h   i   r   d  sp   q   u   a   r   t   e   r nul
0000140   W   a   n   i   n   g  sp   c   r   e   s   c   e   n   t nul
*/

Char **SysStringArray(UInt16 resID, UInt16 *numStrings) {
  MemHandle h;
  char *prefix, *str, **array = NULL;
  UInt16 len;
  UInt8 *p;
  int i, j;

  if (numStrings) {
    if ((h = DmGetResource(strListRscType, resID)) != NULL) {
      if ((p = MemHandleLock(h)) != NULL) {
        i = 0;
        i += pumpkin_getstr(&prefix, p, i);
        i += get2b(numStrings, p, i);
        if ((array = MemPtrNew(*numStrings * sizeof(char *))) != NULL) {
          for (j = 0; j < *numStrings; j++) {
            i += pumpkin_getstr(&str, p, i);
            len = StrLen(prefix) + StrLen(str) + 1;
            if ((array[j] = MemPtrNew(len)) != NULL) {
              StrPrintF(array[j], "%s%s", prefix, str);
            }
          }
        }
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    }
  }

  return array;
}

Char *SysStringByIndex(UInt16 resID, UInt16 index, Char *strP, UInt16 maxLen) {
  MemHandle h;
  char *prefix, *str;
  UInt16 max;
  UInt8 *p;
  int i, j;

  if (strP) {
    strP[0] = 0;

    if ((h = DmGetResource(strListRscType, resID)) != NULL) {
      if ((p = MemHandleLock(h)) != NULL) {
        i = 0;
        i += pumpkin_getstr(&prefix, p, 0);
        i += get2b(&max, p, i);
        str = NULL;
        for (j = 0; j < max; j++) {
          i += pumpkin_getstr(&str, p, i);
          if (j == index) break;
        }
        if (j < max && prefix && str) {
          sys_snprintf(strP, maxLen, "%s%s", prefix, str);
          debug(DEBUG_TRACE, PALMOS_MODULE, "SysStringByIndex resID %d index %d \"%s\"", resID, index, strP);
        }
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    }
  }

  return strP;
}

Char *SysErrString(Err err, Char *strP, UInt16 maxLen) {
  uint16_t resID, index;

  // The actual string will be of the form: "<error message> (XXXX)"
  // where XXXX is the hexadecimal error number.

  if (strP && maxLen > 8) {
    strP[0] = 0;
    resID = err >> 8;
    index = err & 0xFF;
    if (SysStringByIndex(resID, index, strP, maxLen-8)) {
      StrPrintF(&strP[StrLen(strP)], " (%04X)", err);
    }
  }

  return strP;
}

// system use only

UInt32 HostControl(HostControlTrapNumber selector, ...) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "HostControl not implemented");
  return 0;
}

// Int16 searchF(void const *searchData, void const *arrayData, Int32 other)

Boolean SysBinarySearch(void const *baseP, UInt16 numOfElements, Int16 width, SearchFuncPtr searchF, void const *searchData, Int32 other, Int32 *position, Boolean findFirst) {
  UInt16 i;
  UInt8 *p;
  Boolean found = false;

  debug(DEBUG_INFO, PALMOS_MODULE, "SysBinarySearch not implemented correctly");

  p = (UInt8 *)baseP;

  if (p && searchF && searchData) {
    for (i = 0; i < numOfElements && !found; i++) {
      if (searchF(searchData, p + i*width, other) == 0) {
        if (position) *position = i;
        debug(DEBUG_INFO, PALMOS_MODULE, "SysBinarySearch element found at position %d", i);
        found = true;
      }
    }
  }

  if (!found) {
    debug(DEBUG_INFO, PALMOS_MODULE, "SysBinarySearch element not found");
  }

  return found;
}

static int compare68k(const void *e1, const void *e2) {
  sysu_module_t *module = (sysu_module_t *)pumpkin_get_local_storage(sysu_key);
  return CallCompareFunction(module->comparF68k, (void *)e1, (void *)e2, module->other);
}

void SysQSort68k(void *baseP, UInt16 numOfElements, Int16 width, UInt32 comparF, Int32 other) {
  sysu_module_t *module = (sysu_module_t *)pumpkin_get_local_storage(sysu_key);

  module->comparF68k = comparF;
  module->other = other;
  sys_qsort(baseP, numOfElements, width, compare68k);
  module->comparF68k = 0;
}

static int compare(const void *e1, const void *e2) {
  sysu_module_t *module = (sysu_module_t *)pumpkin_get_local_storage(sysu_key);
  return module->comparF((void *)e1, (void *)e2, module->other);
}

void SysQSort(void *baseP, UInt16 numOfElements, Int16 width, CmpFuncPtr comparF, Int32 other) {
  sysu_module_t *module = (sysu_module_t *)pumpkin_get_local_storage(sysu_key);

  module->comparF = comparF;
  module->other = other;
  sys_qsort(baseP, numOfElements, width, compare);
  module->comparF = NULL;
}

static int compareP(const void *e1, const void *e2) {
  sysu_module_t *module = (sysu_module_t *)pumpkin_get_local_storage(sysu_key);
  return module->comparFP((void *)e1, (void *)e2, module->otherP);
}

void SysQSortP(void *baseP, UInt32 numOfElements, Int32 width, CmpFuncPPtr comparFP, void *otherP) {
  sysu_module_t *module = (sysu_module_t *)pumpkin_get_local_storage(sysu_key);

  module->comparFP = comparFP;
  module->otherP = otherP;
  sys_qsort(baseP, numOfElements, width, compareP);
  module->comparFP = NULL;
}

void SysInsertionSort(void *baseP, UInt16 numOfElements, Int16 width, CmpFuncPtr comparF, Int32 other) {
  SysQSort(baseP, numOfElements, width, comparF, other);
}
