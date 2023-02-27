//(c) uARM project    https://github.com/uARM-Palm/uARM    uARM@dmitry.gr

#ifndef _MEM_H_
#define _MEM_H_

#include "sys.h"

struct ArmMem;

typedef int (*ArmMemAccessF)(void *userData, uint32_t pa, uint8_t size, int write, void *buf);

#define MEM_ACCESS_TYPE_READ		0
#define MEM_ACCESS_TYPE_WRITE		1
#define MEM_ACCCESS_FLAG_NOERROR	0x80	//for debugger use

struct ArmMem *memInit(void);

void memDeinit(struct ArmMem *mem);
int memRegionAdd(struct ArmMem *mem, uint32_t pa, uint32_t sz, ArmMemAccessF af, void *uD);
int memAccess(struct ArmMem *mem, uint32_t addr, uint8_t size, uint8_t accessType, void *buf);

#endif

