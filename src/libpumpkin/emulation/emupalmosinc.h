#define EMUPALMOS_INVALID_ADDRESS     1
#define EMUPALMOS_INVALID_INSTRUCTION 2
#define EMUPALMOS_INVALID_TRAP        3
#define EMUPALMOS_INVALID_XREF        4
#define EMUPALMOS_HEAP_EXHAUSTED      5
#define EMUPALMOS_GENERIC_ERROR       9

int emupalmos_init(void);
uint32_t emupalmos_main(uint16_t code, void *param, uint16_t flags);
uint8_t *emupalmos_ram(void);
void emupalmos_finish(int f);
int emupalmos_finished(void);
void emupalmos_panic(char *msg, int code);

void cpu_pulse_reset(void);
