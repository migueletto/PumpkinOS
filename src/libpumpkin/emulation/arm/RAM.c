//(c) uARM project    https://github.com/uARM-Palm/uARM    uARM@dmitry.gr

#include "sys.h"
#include "endianness.h"
#include "armem.h"
#include "RAM.h"
#include "debug.h"

struct ArmRam {
	uint32_t adr;
	uint32_t sz;
	uint32_t* buf;
};

static int ramAccessF(void* userData, uint32_t pa, uint8_t size, int write, void* bufP)
{
	struct ArmRam *ram = (struct ArmRam*)userData;
	uint8_t *addr = (uint8_t*)ram->buf;
	
	pa -= ram->adr;
	if (pa > ram->sz - size) {
		debug(DEBUG_ERROR, "ARM", "invalid address 0x%08X (arm)", pa);
		return 0;
	}
	
	addr += pa;
	
	if (write) {
		//debug(DEBUG_TRACE, "Heap", "write %p to %p", addr, addr + size - 1);
		switch (size) {
			case 1:
//{
//uint8_t value = *(uint8_t*)bufP;
//debug(1, "XXX", "write byte 0x%02X to 0x%08X", value, pa);
//}
				*addr = *(uint8_t*)bufP;	//our memory system is little-endian
				break;
			case 2:
//{
//uint16_t value = sys_htole16(*(uint16_t*)bufP);
//debug(1, "XXX", "write word 0x%04X to 0x%08X", value, pa);
//}
				*((uint16_t*)addr) = sys_htole16(*(uint16_t*)bufP);	//our memory system is little-endian
				break;
			case 4:
//{
//uint32_t value = sys_htole32(*(uint32_t*)bufP);
//debug(1, "XXX", "write long 0x%08X to 0x%08X", value, pa);
//}
				*((uint32_t*)addr) = sys_htole32(*(uint32_t*)bufP);
				break;
			case 8:
				*((uint32_t*)(addr + 0)) = sys_htole32(((uint32_t*)bufP)[0]);
				*((uint32_t*)(addr + 4)) = sys_htole32(((uint32_t*)bufP)[1]);
				break;
			case 32:
				*((uint32_t*)(addr + 0)) = sys_htole32(((uint32_t*)bufP)[0]);
				*((uint32_t*)(addr + 4)) = sys_htole32(((uint32_t*)bufP)[1]);
				*((uint32_t*)(addr + 8)) = sys_htole32(((uint32_t*)bufP)[2]);
				*((uint32_t*)(addr + 12)) = sys_htole32(((uint32_t*)bufP)[3]);
				*((uint32_t*)(addr + 16)) = sys_htole32(((uint32_t*)bufP)[4]);
				*((uint32_t*)(addr + 20)) = sys_htole32(((uint32_t*)bufP)[5]);
				*((uint32_t*)(addr + 24)) = sys_htole32(((uint32_t*)bufP)[6]);
				*((uint32_t*)(addr + 28)) = sys_htole32(((uint32_t*)bufP)[7]);
				break;
			default:
				return 0;
		}
	} else {
		switch (size) {
			case 1:
				*(uint8_t*)bufP = *((uint8_t*)addr);
				break;
			case 2:
				*(uint16_t*)bufP = sys_le16toh(*((uint16_t*)addr));
				break;
			case 4:
				*(uint32_t*)bufP = sys_le32toh(*((uint32_t*)addr));
//{
//uint32_t value = *(uint32_t*)bufP;
//debug(1, "XXX", "read  0x%08X 0x%08X", value, pa);
//}
				break;
			case 64:
				((uint32_t*)bufP)[ 8] = sys_le32toh(*((uint32_t*)(addr + 32)));
				((uint32_t*)bufP)[ 9] = sys_le32toh(*((uint32_t*)(addr + 36)));
				((uint32_t*)bufP)[10] = sys_le32toh(*((uint32_t*)(addr + 40)));
				((uint32_t*)bufP)[11] = sys_le32toh(*((uint32_t*)(addr + 44)));
				((uint32_t*)bufP)[12] = sys_le32toh(*((uint32_t*)(addr + 48)));
				((uint32_t*)bufP)[13] = sys_le32toh(*((uint32_t*)(addr + 52)));
				((uint32_t*)bufP)[14] = sys_le32toh(*((uint32_t*)(addr + 56)));
				((uint32_t*)bufP)[15] = sys_le32toh(*((uint32_t*)(addr + 60)));
				//fallthrough
			case 32:
				((uint32_t*)bufP)[4] = sys_le32toh(*((uint32_t*)(addr + 16)));
				((uint32_t*)bufP)[5] = sys_le32toh(*((uint32_t*)(addr + 20)));
				((uint32_t*)bufP)[6] = sys_le32toh(*((uint32_t*)(addr + 24)));
				((uint32_t*)bufP)[7] = sys_le32toh(*((uint32_t*)(addr + 28)));
				//fallthrough
			case 16:
				((uint32_t*)bufP)[2] = sys_le32toh(*((uint32_t*)(addr +  8)));
				((uint32_t*)bufP)[3] = sys_le32toh(*((uint32_t*)(addr + 12)));
				//fallthrough
			case 8:
				((uint32_t*)bufP)[0] = sys_le32toh(*((uint32_t*)(addr +  0)));
				((uint32_t*)bufP)[1] = sys_le32toh(*((uint32_t*)(addr +  4)));
				break;
			default:
				return 0;
		}
	}
	
	return 1;
}

struct ArmRam *ramInit(struct ArmMem *mem, uint32_t adr, uint32_t sz, uint32_t* buf)
{
	struct ArmRam *ram = (struct ArmRam*)sys_malloc(sizeof(*ram));
	
	if (ram) {
		sys_memset(ram, 0, sizeof (*ram));
		ram->adr = adr;
		ram->sz = sz;
		ram->buf = buf;
		memRegionAdd(mem, adr, sz, ramAccessF, ram);
	}
	
	return ram;
}

void ramDeinit(struct ArmRam *ram) {
  if (ram) {
    sys_free(ram);
  }
}
