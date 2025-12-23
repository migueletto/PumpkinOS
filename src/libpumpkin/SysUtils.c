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

  // for random
  int n, i, j;
  uint32_t init[32];
  uint32_t *x;

} sysu_module_t;

static const uint32_t random_init[] = {
  0x00000000, 0x5851f42d, 0xc0b18ccf, 0xcbb5f646,
  0xc7033129, 0x30705b04, 0x20fd5db4, 0x9a8b7f78,
  0x502959d8, 0xab894868, 0x6c0356a7, 0x88cdb7ff,
  0xb477d43f, 0x70a3a52b, 0xa8e4baf1, 0xfd8341fc,
  0x8ae16fd9, 0x742d2f7a, 0x0d1f0796, 0x76035e09,
  0x40f7702c, 0x6fa72ca5, 0xaaa84157, 0x58a0df74,
  0xc74a0364, 0xae533cc4, 0x04185faf, 0x6de3b115,
  0x0cab8628, 0xf043bfa4, 0x398150e9, 0x37521657
};

static void __srandom(sysu_module_t *module, uint32_t seed);

int SysUInitModule(void) {
  sysu_module_t *module;
  int i;

  if ((module = xcalloc(1, sizeof(sysu_module_t))) == NULL) {
    return -1;
  }

  for (i = 0; i < 32; i++) {
    module->init[i] = random_init[i];
  }
  module->n = 31;
  module->i = 3;
  module->j = 0; 
  module->x = module->init + 1;
  __srandom(module, sys_get_clock() & 0xFFFFFFFF);

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

static uint32_t lcg31(uint32_t x) {
  return (1103515245*x + 12345) & 0x7fffffff;
}

static uint64_t lcg64(uint64_t x) {
  return 6364136223846793005ull*x + 1;
}

static void __srandom(sysu_module_t *module, uint32_t seed) {
  uint64_t s = seed;
  int k;

  if (module->n == 0) {
    module->x[0] = s;
    return;
  }
  module->i = module->n == 31 || module->n == 7 ? 3 : 1;
  module->j = 0;
  for (k = 0; k < module->n; k++) {
    s = lcg64(s);
    module->x[k] = s>>32;
  }
  // make sure x contains at least one odd number
  module->x[0] |= 1;
}

static uint32_t __random(sysu_module_t *module) {
  uint32_t k;

  if (module->n == 0) {
    k = module->x[0] = lcg31(module->x[0]);
    goto end;
  }
  module->x[module->i] += module->x[module->j];
  k = module->x[module->i]>>1;
  if (++module->i == module->n)
    module->i = 0;
  if (++module->j == module->n)
    module->j = 0;
end:
  return k;
}

// Return a random number anywhere from 0 to sysRandomMax
Int16 SysRandom(Int32 newSeed) {
  int32_t r;
  r = SysRandom32(newSeed);
  return r % sysRandomMax;
}

Int32 SysRandom32(Int32 newSeed) {
  sysu_module_t *module = (sysu_module_t *)pumpkin_get_local_storage(sysu_key);
  int32_t r;

  if (newSeed) {
    __srandom(module, newSeed);
  }
    
  r = __random(module);
  if (r < 0) r = -r;
  
  return r;
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
    resID = 10000 + (err >> 8);
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
