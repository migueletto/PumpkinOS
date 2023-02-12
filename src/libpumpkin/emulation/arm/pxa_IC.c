//(c) uARM project    https://github.com/uARM-Palm/uARM    uARM@dmitry.gr

#include <stdlib.h>
#include <string.h>
#include "pxa_IC.h"
#include "armem.h"


#define PXA_IC_BASE	0x40D00000UL
#define PXA_IC_SIZE	0x00010000UL


struct SocIc {

	struct ArmCpu *cpu;
	
	uint32_t ICMR[2];	//Mask Registers
	uint32_t ICLR[2];	//Level Registers
	uint32_t ICPR[2];	//Pending registers
	uint32_t ICCR;	//Control Register
	
	uint8_t prio[40];
	uint8_t iccr;
	
	bool wasIrq, wasFiq, gen2;
};


static void socIcPrvHandleChanges(struct SocIc *ic)
{
	bool nowIrq = false, nowFiq = false;
	uint_fast8_t i;
	
	for (i = 0; i < 2; i++) {
		uint32_t unmasked = ic->ICPR[i] & ic->ICMR[i];
		
		nowFiq = nowFiq || (unmasked & ic->ICLR[i]);
		nowIrq = nowIrq || (unmasked & ~ic->ICLR[i]);
	}
	
	if (nowFiq != ic->wasFiq)
		cpuIrq(ic->cpu, true, nowFiq);
	if (nowIrq != ic->wasIrq)
		cpuIrq(ic->cpu, false, nowIrq);

	ic->wasFiq = nowFiq;
	ic->wasIrq = nowIrq;
}

static uint32_t socIcPrvCalcHighestPrio(struct SocIc *ic)
{
	uint32_t activeIrq[2], activeFiq[2], ret = 0x001f001ful;
	uint_fast8_t i;
	
	for (i = 0; i < 2; i++) {
		activeIrq[i] = ic->ICPR[i] & ic->ICMR[i] & ~ic->ICLR[i];
		activeFiq[i] = ic->ICPR[i] & ic->ICMR[i] & ic->ICLR[i];
	}
	
	for (i = 0; i < 40; i++) {
		
		if (ic->prio[i] & 0x80) {
			
			uint_fast8_t periph = ic->prio[i] & 0x3f;
			
			if (!(ret & 0x80000000ul) && ((activeIrq[periph / 32] >> (periph % 32)) & 1)) {
				
				ret |= 0x80000000ul;
				ret &=~ 0x001f0000ul;
				ret |= ((uint32_t)periph) << 16;
			}
			
			if (!(ret & 0x8000) && ((activeFiq[periph / 32] >> (periph % 32)) & 1)) {
				
				ret |= 0x8000;
				ret &=~ 0x001f;
				ret |= periph;
			}
		}
	}
	
	return ret;
}

static uint32_t socIcPrvGetIcip(struct SocIc *ic, uint_fast8_t idx)
{
	return ic->ICPR[idx] & ic->ICMR[idx] & ~ic->ICLR[idx];
}

static uint32_t socIcPrvGetIcfp(struct SocIc *ic, uint_fast8_t idx)
{
	return ic->ICPR[idx] & ic->ICMR[idx] & ic->ICLR[idx];
}

static bool socIcPrvMemAccessF(void* userData, uint32_t pa, uint_fast8_t size, bool write, void* buf)
{
	struct SocIc *ic = (struct SocIc*)userData;
	uint32_t val = 0;
	
	if (size != 4) {
		fprintf(stderr, "%s: Unexpected %s of %u bytes to 0x%08lx\n", __func__, write ? "write" : "read", size, (unsigned long)pa);
		return false;
	}
	
	pa = (pa - PXA_IC_BASE) >> 2;
	
	
	if (write)
		val = *(uint32_t*)buf;
	
	switch (pa) {
		case 0x9C / 4:
			if (!ic->gen2)
				return false;
			pa -= 0x9C / 4;
			pa += 0x00 / 4;
			pa += 1;
			//fallthrough
		case 0x00 / 4:
			if (write)
				;		//ignored
			else
				val = socIcPrvGetIcip(ic, pa - 0x00 / 4);
			break;
		
		case 0xA0 / 4:
			if (!ic->gen2)
				return false;
			pa -= 0xA0 / 4;
			pa += 0x04 / 4;
			pa += 1;
			//fallthrough
		case 0x04 / 4:
			if (write)
				ic->ICMR[pa - 0x04 / 4] = val;
			else
				val = ic->ICMR[pa - 0x04 / 4];
			break;
		
		case 0xA4 / 4:
			if (!ic->gen2)
				return false;
			pa -= 0xA4 / 4;
			pa += 0x08 / 4;
			pa += 1;
			//fallthrough
		case 0x08 / 4:
			if (write)
				ic->ICLR[pa - 0x08 / 4] = val;
			else
				val = ic->ICLR[pa - 0x08 / 4];
			break;
		
		case 0xA8 / 4:
			if (!ic->gen2)
				return false;
			pa -= 0xA8 / 4;
			pa += 0x0C / 4;
			pa += 1;
			//fallthrough
		case 0x0C / 4:
			if (write)
				;		//ignored
			else
				val = socIcPrvGetIcfp(ic, pa - 0x0C / 4);
			break;
		
		case 0xAC / 4:
			if (!ic->gen2)
				return false;
			pa -= 0xAC / 4;
			pa += 0x10 / 4;
			pa += 1;
			//fallthrough
		case 0x10 / 4:
			if (write)
				;		//ignored
			else
				val = ic->ICPR[pa - 0x10 / 4];
			break;
		
		case 0x14 / 4:
			if (write)
				ic->iccr = val & 1;
			else
				val = ic->iccr;
			break;
		
		case 0x18 / 4:
			if (!ic->gen2)
				return false;
			if (write)
				;		//ignored
			else
				val = socIcPrvCalcHighestPrio(ic);
			break;
		
		case 0xb0 / 4 ... 0xcc / 4:
			pa -= 0xb0 / 4;
			pa += 0x98 / 4;
			pa += 1;
			//fallthrough
		case 0x1c / 4 ... 0x98 / 4:
			if (write)
				ic->prio[pa - 0x1c / 4] = (val & 0x3f) | ((val >> 24) & 0x80);
			else
				val = (ic->prio[pa - 0x1c / 4] & 0x3f) | ((ic->prio[pa - 0x1c / 4] & 0x80) ? 0x80000000ul : 0);
			break;
		
		default:
			return false;
	}
	
	if (write)
		socIcPrvHandleChanges(ic);
	else
		*(uint32_t*)buf = val;

	return true;
}

bool pxa270icPrvCoprocAccess(struct ArmCpu* cpu, void* userData, bool two/* MCR2/MRC2 ? */, bool MRC, uint8_t op1, uint8_t Rx, uint8_t CRn, uint8_t CRm, uint8_t op2)
{
	struct SocIc *ic = (struct SocIc*)userData;
	bool write = !MRC;
	uint32_t val = 0;
	
	if (CRm || op1 || op2 || two)
		return false;
	
	if (write)
		val = cpuGetRegExternal(cpu, Rx);
	
	switch (CRn) {
		
		case 6:
			CRn -= 5;
			//fallthrough
		case 0:
			if (write)
				return false;
			else
				val = socIcPrvGetIcip(ic, CRn - 0);
			break;
		
		case 7:
			CRn -= 5;
			//fallthrough
		case 1:
			if (write)
				ic->ICMR[CRn - 1] = val;
			else
				val = ic->ICMR[CRn - 1];
			break;
		
		case 8:
			CRn -= 5;
			//fallthrough
		case 2:
			if (write)
				ic->ICLR[CRn - 2] = val;
			else
				val = ic->ICLR[CRn - 2];
			break;
		
		case 9:
			CRn -= 5;
			//fallthrough
		case 3:
			if (write)
				return false;
			else
				val = socIcPrvGetIcfp(ic, CRn - 3);
			break;
		
		case 10:
			CRn -= 5;
			//fallthrough
		case 4:
			if (write)
				return false;
			else
				val = ic->ICPR[CRn - 4];
			break;
		
		case 5:
			if (write)
				return false;
			else
				val = socIcPrvCalcHighestPrio(ic);
			break;
		
		default:
			return false;
	}
	
	if (write)
		socIcPrvHandleChanges(ic);
	else
		cpuSetReg(cpu, Rx, val);
	
	return true;
}

struct SocIc* socIcInit(struct ArmCpu *cpu, struct ArmMem *physMem, uint_fast8_t socRev)
{
	struct SocIc *ic = (struct SocIc*)malloc(sizeof(*ic));
	struct ArmCoprocessor cp = {
		.regXfer = pxa270icPrvCoprocAccess,
		.userData = ic,
	};
	
	memset(ic, 0, sizeof (*ic));
	
	ic->cpu = cpu;
	ic->gen2 = socRev == 2;
	
	memRegionAdd(physMem, PXA_IC_BASE, PXA_IC_SIZE, socIcPrvMemAccessF, ic);
	
	if (ic->gen2)
		cpuCoprocessorRegister(cpu, 6, &cp);
	
	return ic;
}

void socIcDeinit(struct SocIc *ic) {
  if (ic) {
    free(ic);
  }
}

void socIcInt(struct SocIc *ic, uint_fast8_t intNum, bool raise)		//interrupt caused by emulated hardware
{
	uint32_t old = ic->ICPR[intNum / 32];
	
	if (raise)
		ic->ICPR[intNum / 32] |= (1UL << (intNum % 32));
	else
		ic->ICPR[intNum / 32] &=~ (1UL << (intNum % 32));
	
	if (ic->ICPR[intNum / 32] != old)
		socIcPrvHandleChanges(ic);
}
