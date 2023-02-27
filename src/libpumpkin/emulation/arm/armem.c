//(c) uARM project    https://github.com/uARM-Palm/uARM    uARM@dmitry.gr

#include "armem.h"


#define  NUM_MEM_REGIONS    128

struct ArmMemRegion {
  uint32_t pa;
  uint32_t sz;
  ArmMemAccessF aF;
  void *uD;
};

struct ArmMem {
  struct ArmMemRegion regions[NUM_MEM_REGIONS];
};


struct ArmMem *memInit(void) {
  struct ArmMem *mem = (struct ArmMem*)sys_malloc(sizeof(*mem));
  
  if (mem) sys_memset(mem, 0, sizeof (*mem));
  
  return mem;
}


void memDeinit(struct ArmMem *mem) {
  if (mem) {
    sys_free(mem);
  }
}

int memRegionAdd(struct ArmMem *mem, uint32_t pa, uint32_t sz, ArmMemAccessF aF, void *uD) {
  uint8_t i;
  
  //check for intersection with another region
  
  for (i = 0; i < NUM_MEM_REGIONS; i++) {
    
    if (!mem->regions[i].sz)
      continue;
    if ((mem->regions[i].pa <= pa && mem->regions[i].pa + mem->regions[i].sz > pa) || (pa <= mem->regions[i].pa && pa + sz > mem->regions[i].pa))
      return 0;    //intersection -> fail
  }
  
  
  //find a free region and put it there
  
  for (i = 0; i < NUM_MEM_REGIONS; i++) {
    if (mem->regions[i].sz == 0) {
    
      mem->regions[i].pa = pa;
      mem->regions[i].sz = sz;
      mem->regions[i].aF = aF;
      mem->regions[i].uD = uD;
    
      return 1;
    }
  }
  
  //fail miserably
  
  return 0;  
}

int memAccess(struct ArmMem *mem, uint32_t addr, uint8_t size, uint8_t accessType, void *buf) {
  int ret = 0, wantWrite = !!(accessType &~ MEM_ACCCESS_FLAG_NOERROR);
  uint8_t i;
  
  for (i = 0; i < NUM_MEM_REGIONS; i++) {
    
    if (mem->regions[i].pa <= addr && mem->regions[i].pa + mem->regions[i].sz > addr) {
      
      ret = mem->regions[i].aF(mem->regions[i].uD, addr, size, wantWrite, buf);
      break;
    }
  }
  
  return ret;
}
