//(c) uARM project    https://github.com/uARM-Palm/uARM    uARM@dmitry.gr

#include "cp15.h"

struct ArmCP15 {
	struct ArmCpu* cpu;
	struct ArmMmu* mmu;
	struct icache *ic;
	
	uint32_t control;
	uint32_t ttb;
	uint32_t FSR;	//fault sttaus register
	uint32_t FAR;	//fault address register
	
	union {
		struct {
			uint32_t CPAR;	//coprocessor access register
			uint32_t ACP;	//auxilary control reg for xscale
		};	//xscale
		struct {
			uint8_t cfg, iMin, iMax;
			uint16_t tid;
		};	//omap
	};
	uint32_t mmuSwitchCy;
	
	uint32_t cpuid;
	uint32_t cacheId;
	
	int xscale, omap;
};

void cp15Cycle(struct ArmCP15* cp15)	//mmu on/off lags by a cycle
{
	if (cp15->mmuSwitchCy) {
		
		if (!--cp15->mmuSwitchCy)
			mmuSetTTP(cp15->mmu, (cp15->control & 0x00000001UL) ? cp15->ttb : MMU_DISABLED_TTP);
	}
}

static int cp15prvCoprocRegXferFunc(struct ArmCpu* cpu, void* userData, int two, int read, uint8_t op1, uint8_t Rx, uint8_t CRn, uint8_t CRm, uint8_t op2)
{
	struct ArmCP15 *cp15 = (struct ArmCP15*)userData;
	uint32_t val = 0, tmp;
	
	if (!read)
		val = cpuGetRegExternal(cpu, Rx);
	
	if (op1 != 0 || two)
		goto fail;								//CP15 only accessed with MCR/MRC with op1 == 0
	
	switch (CRn) {
		
		case 0:		//ID codes
		
			if (!read)
				goto fail;						//cannot write to ID codes register
			if (CRm != 0)
				goto fail;						//CRm must be zero for this read
			if (op2 == 0) {						//main ID register
				
				val = cp15->cpuid;
				goto success;
			}
			else if (op2 == 1) {				//cache type register - we lie here
				
				val = cp15->cacheId;
				goto success;	
			}
			break;
			
		case 1:		//control register
		
			if (op2 == 0 && CRm == 0) {
				if (read)
					val = cp15->control;
				else {
					//uint32_t origVal = val;
				
					//some bits ignore writes. pretend they were wirtten as proper
					val |= 0x0070;
					val &=~ 0x0080;
				
					tmp = val ^ cp15->control;		//see what changed and mask off then chack for what we support changing of
					if (tmp & 0x84F0UL) {
						//fprintf(stderr, "cp15: unknown bits changed (0x%08lx) 0x%08lx -> 0x%08lx, halting\n", (unsigned long)(tmp & 0x84F0UL), (unsigned long)cp15->control, (unsigned long)origVal);
						//while (1);
					}
					
					if (tmp & 0x00002000UL) {			// V bit
						
						cpuSetVectorAddr(cp15->cpu, (val & 0x00002000UL) ? 0xFFFF0000UL : 0x00000000UL);
						cp15->control ^= 0x00002000UL;
					}
					if (tmp & 0x00000200UL) {			// R bit
						
						mmuSetR(cp15->mmu, (val & 0x00000200UL) != 0);
						cp15->control ^= 0x00000200UL;
					}
					if (tmp & 0x00000100UL) {			// S bit
						
						mmuSetS(cp15->mmu, (val & 0x00000100UL) != 0);
						cp15->control ^= 0x00000100UL;
					}
					if (tmp & 0x00000001UL) {			// M bit
						
						cp15->mmuSwitchCy = 2;
						cp15->control ^= 0x00000001UL;
					}
					
				}
			}
			else if (CRm == 1) {
				//fprintf(stderr, "sony cr1 bug?\n");
				if (read)
					val = 0;
			}
			else if (cp15->xscale && op2 == 1) {	//PXA-specific thing
				if (read)
					val = cp15->ACP;
				else
					cp15->ACP = val;
			}
			else
				break;
			goto success;
			
		case 2:		//translation tabler base
			if (read)
				val = cp15->ttb;
			else {
				if (cp15->control & 0x00000001UL) {	//mmu is on
					
					mmuSetTTP(cp15->mmu, val);
				}
				cp15->ttb = val;
			}
			goto success;
		
		case 3:		//domain access control
			if (read)
				val = mmuGetDomainCfg(cp15->mmu);
			else {
				mmuSetDomainCfg(cp15->mmu, val);
				//dcacheFlushPermInfo(cp15->dc);
			}
			goto success;
		
		case 5:		//FSR
			if (read)
				val = cp15->FSR;
			else
				cp15->FSR = val;
			goto success;
			
		case 6:		//FAR
			if (read)
				val = cp15->FAR;
			else
				cp15->FAR = val;
			goto success;
		
		case 7:		//cache ops
			if ((CRm == 5 || CRm == 7) && op2 == 0) {
				icacheInval(cp15->ic);		//invalidate entire {icache(5) or both i and dcache(7)}
				if (CRm == 7) {
					
					//dcacheInval(cp15->dc);
				}
			}
			else if ((CRm == 5 || CRm == 7) && op2 == 1) {
				icacheInvalAddr(cp15->ic, val);	//invalidate {icache(5) or both i and dcache(7)} line, given VA
				if (CRm == 7) {
					
					//dcacheInvalAddr(cp15->dc, val);
				}
			}
			else if ((CRm == 5 || CRm == 7) && op2 == 2) {
				icacheInval(cp15->ic);		//invalidate {icache(5) or both i and dcache(7)} line, given set/index. i dont know how to do this, so flush the whole thing
				
				if (CRm == 7) {
					
					//dcacheInvalSetWayRaw(cp15->dc, val);
				}
			}
			else if (CRm == 10 && op2 == 4)
				{/* drain write buffer = nothing */}
			else if (CRm == 10 && op2 == 1) {
				
				//dcacheCleanSetWayRaw(cp15->dc, val);
			}
			else if (CRm == 6 && op2 == 0) {
				
				//dcacheInval(cp15->dc);
			}
			else if (CRm == 6 && op2 == 1) {
				
				//dcacheInvalAddr(cp15->dc, val);
			}
			else if (CRm == 6 && op2 == 2) {
				
				//dcacheInvalSetWayRaw(cp15->dc, val);
			}
			else if (CRm == 2 && op2 == 5) {
				
				//dcacheAllocAddr(cp15->dc, val);
			}
			else if (CRm == 5 && op2 == 6)
				{/* flush btb = nothing */}
			else if (CRm == 0 && op2 == 4)
				{/* idle = nothing */ }
			else if (CRm == 14 && op2 == 2)
				{/* clean and inval d-cache line */}
			else if (CRm == 10 && op2 == 0)
				{/* clean entire d-cache - omap can do this */}
			else if (CRm == 10 && op2 == 2)
				{/* clean d-cache line */}
			else
				break;
			goto success;
		
		case 8:		//TLB ops
			mmuTlbFlush(cp15->mmu);
			//dcacheFlushPermInfo(cp15->dc);
			goto success;
		
		case 9:		//cache lockdown
			if (CRm == 1 && op2 == 0) {
				//fprintf(stderr, "Attempt to lock 0x%08lx+32 in icache\r\n", (unsigned long)val);
			} else if (CRm == 2 && op2 == 0) {
				//fprintf(stderr, "Dcache now %s lock mode\r\n", val ? "in" : "out of");
			} else
				break;
			goto success;
		
		case 10:	//TLB lockdown
			if (!read && CRm == 0 && (op2 == 0 || op2 == 1)) {
				goto success;
			}
			break;
		
		case 13:	//FCSE
			if (read)
				val = cpuGetPid(cp15->cpu);
			else
				cpuSetPid(cp15->cpu, val & 0xfe000000ul);
			goto success;
		
		case 14:	//xscale debug
			if (cp15->xscale) {
				if (CRm == 8 && op2 == 0) {	//ICBR0
					
					val = 0;
					goto success;
				}
				if (CRm == 9 && op2 == 0) {	//ICBR1
					
					val = 0;
					goto success;
				}
				if (CRm == 0 && op2 == 0) {	//DBR0
					
					val = 0;
					goto success;
				}
				if (CRm == 3 && op2 == 0) {	//DBR1
					
					val = 0;
					goto success;
				}
				if (CRm == 4 && op2 == 0) {	//DBCON
					
					val = 0;
					goto success;
				}
			}
			break;
		
		case 15:
			if (cp15->xscale && op2 == 0 && CRm == 1){	//CPAR on xscale
				if (read)
					val = cpuGetCPAR(cp15->cpu);
				else
					cpuSetCPAR(cp15->cpu, val & 0x3FFF);
				goto success;
			}
			else if (cp15->omap) {						//omap shit
				
				if (CRm == 1 && op2 == 0) {
					if (read)
						val = cp15->cfg;
					else
						cp15->cfg = val & 0x87;
					goto success;
				}
				else if (CRm == 2 && op2 == 0) {
					if (read)
						val = cp15->iMax;
					else
						cp15->iMax = val;
					goto success;
				}
				else if (CRm == 3 && op2 == 0) {
					if (read)
						val = cp15->iMin;
					else
						cp15->iMin = val;
					goto success;
				}
				else if (CRm == 4 && op2 == 0) {
					if (read)
						val = cp15->tid;
					else
						cp15->tid = val;
					goto success;
				}
				else if (CRm == 8 && op2 == 0 && read) {
					val = 0;
					goto success;
				}
				else if (CRm == 8 && op2 == 2 && !read) {
					
					//WFI
					goto success;
				}
			}
			break;
	}
	
fail:
	return 0;

success:
	
	if(read)
		cpuSetReg(cpu, Rx, val);
	return 1;
}

struct ArmCP15* cp15Init(struct ArmCpu* cpu, struct ArmMmu* mmu, struct icache *ic, uint32_t cpuid, uint32_t cacheId, int xscale, int omap)
{
	struct ArmCP15 *cp15 = (struct ArmCP15*)sys_malloc(sizeof(*cp15));
	struct ArmCoprocessor cp = {
		.regXfer = cp15prvCoprocRegXferFunc,
		.userData = cp15,
	};
	
	sys_memset(cp15, 0, sizeof (*cp15));

	cp15->cpu = cpu;
	cp15->mmu = mmu;
	cp15->ic = ic;
	cp15->control = 0x00004072UL;
	cp15->cpuid = cpuid;
	cp15->cacheId = cacheId;
	cp15->xscale = xscale;
	cp15->omap = omap;
	
	if (omap)
		cp15->iMax = 0xff;
	
	cpuCoprocessorRegister(cpu, 15, &cp);
	
	return cp15;
}

void cp15Deinit(struct ArmCP15 *cp15) {
  if (cp15) {
    sys_free(cp15);
  }
}

void cp15SetFaultStatus(struct ArmCP15* cp15, uint32_t addr, uint_fast8_t faultStatus)
{
	cp15->FAR = addr;
	cp15->FSR = faultStatus;
}
