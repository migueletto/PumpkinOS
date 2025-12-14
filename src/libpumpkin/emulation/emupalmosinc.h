#define EMUPALMOS_INVALID_ADDRESS     1
#define EMUPALMOS_INVALID_INSTRUCTION 2
#define EMUPALMOS_INVALID_TRAP        3
#define EMUPALMOS_INVALID_XREF        4
#define EMUPALMOS_HEAP_EXHAUSTED      5
#define EMUPALMOS_UNDECODED_EVENT     6
#define EMUPALMOS_GENERIC_ERROR       9

int emupalmos_init(logtrap_def *def);
uint32_t emupalmos_main(uint16_t code, void *param, uint16_t flags);
uint8_t *emupalmos_ram(void);
void emupalmos_finish(int f);
int emupalmos_finished(void);
void emupalmos_panic(char *msg, int code);
uint32_t emupalmos_arm_syscall(uint32_t group, uint32_t function, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp);
int emupalmos_check_address(uint32_t address, uint32_t size, int read);
void emupalmos_disasm(int m68k, int arm);

void cpu_pulse_reset(void);
