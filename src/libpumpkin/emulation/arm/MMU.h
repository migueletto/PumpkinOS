//(c) uARM project    https://github.com/uARM-Palm/uARM    uARM@dmitry.gr

#ifndef _MMU_H_
#define _MMU_H_


#include "armem.h"


struct ArmMmu;

#define MMU_DISABLED_TTP		0xFFFFFFFFUL

#define MMU_MAPPING_CACHEABLE	0x0001
#define MMU_MAPPING_BUFFERABLE	0x0002
#define MMU_MAPPING_UR			0x0004
#define MMU_MAPPING_UW			0x0008
#define MMU_MAPPING_SR			0x0010
#define MMU_MAPPING_SW			0x0020


struct ArmMmu* mmuInit(struct ArmMem *mem, int xscaleMode);
void mmuDeinit(struct ArmMmu *mmu);
int mmuTranslate(struct ArmMmu *mmu, uint32_t va, int priviledged, int write, uint32_t* paP, uint8_t* fsrP, uint8_t *mappingInfoP);

int mmuIsOn(struct ArmMmu *mmu);

uint32_t mmuGetTTP(struct ArmMmu *mmu);
void mmuSetTTP(struct ArmMmu *mmu, uint32_t ttp);

void mmuSetS(struct ArmMmu *mmu, int on);
void mmuSetR(struct ArmMmu *mmu, int on);
int mmuGetS(struct ArmMmu *mmu);
int mmuGetR(struct ArmMmu *mmu);

uint32_t mmuGetDomainCfg(struct ArmMmu *mmu);
void mmuSetDomainCfg(struct ArmMmu *mmu, uint32_t val);

void mmuTlbFlush(struct ArmMmu *mmu);

void mmuDump(struct ArmMmu *mmu);		//for calling in GDB :)

#endif
