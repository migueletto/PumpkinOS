//(c) uARM project    https://github.com/uARM-Palm/uARM    uARM@dmitry.gr

#ifndef _CPU_H_
#define _CPU_H_

struct ArmCpu;

#include "sys.h"
#include "armem.h"


#define ARM_SR_N				0x80000000UL
#define ARM_SR_Z				0x40000000UL
#define ARM_SR_C				0x20000000UL
#define ARM_SR_V				0x10000000UL
#define ARM_SR_Q				0x08000000UL
#define ARM_SR_I				0x00000080UL
#define ARM_SR_F				0x00000040UL
#define ARM_SR_T				0x00000020UL
#define ARM_SR_M				0x0000001FUL

#define ARM_SR_MODE_USR			0x00000010UL
#define ARM_SR_MODE_FIQ			0x00000011UL
#define ARM_SR_MODE_IRQ			0x00000012UL
#define ARM_SR_MODE_SVC			0x00000013UL
#define ARM_SR_MODE_ABT			0x00000017UL
#define ARM_SR_MODE_UND			0x0000001BUL
#define ARM_SR_MODE_SYS			0x0000001FUL

#define ARV_VECTOR_OFFT_RST		0x00000000UL
#define ARM_VECTOR_OFFT_UND		0x00000004UL
#define ARM_VECTOR_OFFT_SWI		0x00000008UL
#define ARM_VECTOR_OFFT_P_ABT	0x0000000CUL
#define ARM_VECTOR_OFFT_D_ABT	0x00000010UL
#define ARM_VECTOR_OFFT_UNUSED	0x00000014UL
#define ARM_VECTOR_OFFT_IRQ		0x00000018UL
#define ARM_VECTOR_OFFT_FIQ		0x0000001CUL


//the following are for cpuGetRegExternal() and are generally used for debugging purposes
#define ARM_REG_NUM_CPSR	16
#define ARM_REG_NUM_SPSR	17


typedef int (*ArmCoprocRegXferF)(struct ArmCpu *cpu, void *userData, int two/* MCR2/MRC2 ? */, int MRC, uint8_t op1, uint8_t Rx, uint8_t CRn, uint8_t CRm, uint8_t op2);
typedef int (*ArmCoprocDatProcF)(struct ArmCpu *cpu, void *userData, int two/* CDP2 ? */, uint8_t op1, uint8_t CRd, uint8_t CRn, uint8_t CRm, uint8_t op2);
typedef int (*ArmCoprocMemAccsF)(struct ArmCpu *cpu, void *userData, int two /* LDC2/STC2 ? */, int N, int store, uint8_t CRd, uint8_t addrReg, uint32_t addBefore, uint32_t addAfter, uint8_t *option /* NULL if none */);	///addBefore/addAfter are UNSCALED. spec syas *4, but WMMX has other ideas. exercise caution. writeback is ON YOU!
typedef int (*ArmCoprocTwoRegF)(struct ArmCpu *cpu, void *userData, int MRRC, uint8_t op, uint8_t Rd, uint8_t Rn, uint8_t CRm);


struct ArmCoprocessor {
  ArmCoprocRegXferF regXfer;
  ArmCoprocDatProcF dataProcessing;
  ArmCoprocMemAccsF memAccess;
  ArmCoprocTwoRegF  twoRegF;
  void *userData;
};


struct ArmCpu *cpuInit(uint32_t pc, struct ArmMem *mem, int xscale, int omap, uint32_t cpuid, uint32_t cacheId);
void cpuDeinit(struct ArmCpu *cpu);

void cpuDisasm(struct ArmCpu *cpu, int disasm);
void cpuCycle(struct ArmCpu *cpu);
void cpuIrq(struct ArmCpu *cpu, int fiq, int raise);  //unraise when acknowledged


uint32_t cpuGetRegExternal(struct ArmCpu *cpu, uint8_t reg);
void cpuSetReg(struct ArmCpu *cpu, uint8_t reg, uint32_t val);
int cpuMemOpExternal(struct ArmCpu *cpu, void *buf, uint32_t vaddr, uint8_t size, int write);  //for external use


void cpuCoprocessorRegister(struct ArmCpu *cpu, uint8_t cpNum, struct ArmCoprocessor *coproc);

void cpuSetVectorAddr(struct ArmCpu *cpu, uint32_t adr);
void cpuSetPid(struct ArmCpu *cpu, uint32_t pid);
uint32_t cpuGetPid(struct ArmCpu *cpu);

uint16_t cpuGetCPAR(struct ArmCpu *cpu);
void cpuSetCPAR(struct ArmCpu *cpu, uint16_t cpar);

uint32_t cpuReg(struct ArmCpu *cpu, uint8_t reg);

#endif

