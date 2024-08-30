void allTrapsInit(void);
char *trapName(uint16_t trap, uint16_t *selector, int follow);
void trapHook(uint32_t pc, emu_state_t *state);
