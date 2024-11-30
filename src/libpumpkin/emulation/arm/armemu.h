typedef struct arm_emu_t arm_emu_t;

typedef uint32_t (*call68KFunc_f)(uint32_t emulStateP, uint32_t trapOrFunction, uint32_t argsOnStackP, uint32_t argsSizeAndwantA0);

arm_emu_t *armInit(uint8_t *buf, uint32_t size);
void armFinish(arm_emu_t *arm);
uint32_t armGetReg(arm_emu_t *arm, uint32_t reg);
void armSetReg(arm_emu_t *arm, uint32_t reg, uint32_t value);
int armRun(arm_emu_t *arm, uint32_t n, uint32_t call68KAddr, call68KFunc_f f, uint32_t returnAddr, int disasm);
