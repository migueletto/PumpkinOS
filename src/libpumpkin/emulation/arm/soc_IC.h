//(c) uARM project    https://github.com/uARM-Palm/uARM    uARM@dmitry.gr

#ifndef _SOC_IC_H_
#define _SOC_IC_H_

#include "armem.h"
#include "CPU.h"

struct SocIc;

struct SocIc* socIcInit(struct ArmCpu *cpu, struct ArmMem *physMem, uint_fast8_t socRev);
void socIcDeinit(struct SocIc *ic);

void socIcInt(struct SocIc *ic, uint_fast8_t intNum, int raise);

#endif
