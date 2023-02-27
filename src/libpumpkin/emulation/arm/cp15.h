//(c) uARM project    https://github.com/uARM-Palm/uARM    uARM@dmitry.gr

#ifndef _CP15_H_
#define _CP15_H_


#include "sys.h"
#include "icache.h"
#include "CPU.h"
#include "MMU.h"

struct ArmCP15;


struct ArmCP15* cp15Init(struct ArmCpu* cpu, struct ArmMmu* mmu, struct icache *ic, uint32_t cpuid, uint32_t cacheId, int xscale, int omap);
void cp15Deinit(struct ArmCP15 *cp15);
void cp15SetFaultStatus(struct ArmCP15* cp15, uint32_t addr, uint_fast8_t faultStatus);
void cp15Cycle(struct ArmCP15* cp15);

#endif

