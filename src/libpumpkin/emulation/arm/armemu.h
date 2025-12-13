typedef struct arm_emu_t arm_emu_t;

typedef uint32_t (*call68KFunc_f)(uint32_t emulStateP, uint32_t trapOrFunction, uint32_t argsOnStackP, uint32_t argsSizeAndwantA0);

arm_emu_t *uarmInit(uint8_t *buf, uint32_t size);
void uarmFinish(arm_emu_t *arm);
uint32_t uarmGetReg(arm_emu_t *arm, uint32_t reg);
void uarmSetReg(arm_emu_t *arm, uint32_t reg, uint32_t value);
int uarmRun(arm_emu_t *arm, uint32_t n, uint32_t call68KAddr, call68KFunc_f f, uint32_t returnAddr);
void uarmDisasm(arm_emu_t *arm, int disasm);
