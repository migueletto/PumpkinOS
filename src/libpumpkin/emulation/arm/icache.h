//(c) uARM project    https://github.com/uARM-Palm/uARM    uARM@dmitry.gr

#ifndef _ICACHE_H_
#define _ICACHE_H_

#include "armem.h"
#include "MMU.h"
#include "CPU.h"

struct icache;

struct icache *icacheInit(struct ArmMem *mem, struct ArmMmu *mmu);
void icacheDeinit(struct icache *ic);
int icacheFetch(struct icache *ic, uint32_t va, uint8_t sz, int priviledged, uint8_t *fsr, void *buf);
void icacheInval(struct icache *ic);
void icacheInvalAddr(struct icache *ic, uint32_t addr);

#endif
