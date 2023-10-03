#include "sys.h"
#include "debug.h"
#include "Zydis/Zydis.h"
#include "i8086disasm.h"

void i8086disasm(uint8_t *data, uint32_t ip, char *buf, int len) {
  ZyanU64 runtime_address = ip;
  ZydisDisassembledInstruction instruction;

  if (ZYAN_SUCCESS(ZydisDisassembleIntel( ZYDIS_MACHINE_MODE_REAL_16, runtime_address, data, 16, &instruction))) {
    sys_strncpy(buf, instruction.text, len);
  } else {
    sys_strncpy(buf, "invalid instruction", len);
  }
}
