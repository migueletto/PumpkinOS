//(c) uARM project    https://github.com/uARM-Palm/uARM    uARM@dmitry.gr

#ifndef _RAM_H_
#define _RAM_H_

#include "sys.h"

struct ArmRam;

struct ArmRam *ramInit(struct ArmMem *mem, uint32_t adr, uint32_t sz, uint32_t *buf);
void ramDeinit(struct ArmRam *ram);

#endif

