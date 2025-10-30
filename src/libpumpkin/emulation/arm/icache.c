//(c) uARM project    https://github.com/uARM-Palm/uARM    uARM@dmitry.gr

#include "sys.h"
#include "MMU.h"
#include "CPU.h"
#include "debug.h"


#define ICACHE_L		5	//line size is 2^L bytes
#define ICACHE_S		11	//number of sets is 2^S
#define ICACHE_A		1	//set associativity (less for speed)


#define ICACHE_LINE_SZ		(1 << ICACHE_L)
#define ICACHE_BUCKET_NUM	(1 << ICACHE_S)
#define ICACHE_BUCKET_SZ	(ICACHE_A)


#define ICACHE_ADDR_MASK	((uint32_t)-ICACHE_LINE_SZ)
#define ICACHE_USED_MASK	1UL
#define ICACHE_PRIV_MASK	2UL

struct icacheLine {

	uint32_t info;	//addr, masks
	uint8_t data[ICACHE_LINE_SZ];
};

struct icache {

	struct ArmMem* mem;
	struct ArmMmu* mmu;
	
	struct icacheLine lines[ICACHE_BUCKET_NUM][ICACHE_BUCKET_SZ];
	uint8_t ptr[ICACHE_BUCKET_NUM];
};


void icacheInval(struct icache *ic)
{
	uint_fast16_t i, j;
	
	for (i = 0; i < ICACHE_BUCKET_NUM; i++) {
		for(j = 0; j < ICACHE_BUCKET_SZ; j++)
			ic->lines[i][j].info = 0;
		ic->ptr[i] = 0;
	}
}

struct icache* icacheInit(struct ArmMem* mem, struct ArmMmu *mmu)
{
	struct icache *ic = (struct icache*)sys_malloc(sizeof(*ic));
	
	if (ic) {
		sys_memset(ic, 0, sizeof (*ic));
	
		ic->mem = mem;
		ic->mmu = mmu;
	
		icacheInval(ic);
	}
	
	return ic;	
}

void icacheDeinit(struct icache *ic) {
  if (ic) {
    sys_free(ic);
  }
}

static uint_fast16_t icachePrvHash(uint32_t addr)
{
	addr >>= ICACHE_L;
	addr &= (1UL << ICACHE_S) - 1UL;

	return addr;
}

void icacheInvalAddr(struct icache* ic, uint32_t va)
{
	uint32_t off = va % ICACHE_LINE_SZ;
	int_fast16_t i, j, bucket;
	struct icacheLine *lines;
	
	va -= off;

	bucket = icachePrvHash(va);
	lines = ic->lines[bucket];
	
	for (i = 0, j = ic->ptr[bucket]; i < ICACHE_BUCKET_SZ; i++) {
		
		if (--j == -1)
			j = ICACHE_BUCKET_SZ - 1;
		
		if ((lines[j].info & (ICACHE_ADDR_MASK | ICACHE_USED_MASK)) == (va | ICACHE_USED_MASK))	//found it!
			lines[j].info = 0;
	}
}

int icacheFetch(struct icache* ic, uint32_t va, uint_fast8_t sz, int priviledged, uint_fast8_t* fsrP, void* buf)
{
	struct icacheLine *lines, *line = NULL;
	uint32_t off = va % ICACHE_LINE_SZ;
	int_fast16_t i, j, bucket;
	int needRead = 0;
	
	va -= off;
	
	if (va & (sz - 1)) {	//alignment issue
		
		if (fsrP)
			*fsrP = 3;
debug(1, "XXX", "icacheFetch alignment issue");
		return 0;
	}

	bucket = icachePrvHash(va);
	lines = ic->lines[bucket];
	
	for (i = 0, j = ic->ptr[bucket]; i < ICACHE_BUCKET_SZ; i++) {
		
		if (--j == -1)
			j = ICACHE_BUCKET_SZ - 1;
		
		if ((lines[j].info & (ICACHE_ADDR_MASK | ICACHE_USED_MASK)) == (va | ICACHE_USED_MASK)) {	//found it!
		
			if (!priviledged && (lines[j].info & ICACHE_PRIV_MASK)) {	//we found a line but it was cached as priviledged and we are not sure if unpriv can access it
				
				//attempt a re-read. if it passes, remove priv flag
				needRead = 1;
			}
			
			line = &lines[j];
			break;
		}
	}
	
	if (!line) {
		
		needRead = 1;
		
		j = ic->ptr[bucket]++;
		if (ic->ptr[bucket] == ICACHE_BUCKET_SZ)
			ic->ptr[bucket] = 0;
		line = lines + j;
	}
	
	if (needRead) {
	
		uint8_t data[ICACHE_LINE_SZ], mappingInfo;
		uint32_t pa;
	
		//if we're here, we found nothing - maybe time to populate the cache
		
		if (!mmuTranslate(ic->mmu, va, priviledged, 0, &pa, fsrP, &mappingInfo)) {
debug(1, "XXX", "icacheFetch mmuTranslate");
			return 0;
		}
		
		if (!mmuIsOn(ic->mmu) || !(mappingInfo & MMU_MAPPING_CACHEABLE)) {	//uncacheable mapping or mmu is off - just do the read we were asked to and do not fill the line
			
			if (!memAccess(ic->mem, pa + off, sz, MEM_ACCESS_TYPE_READ, buf)) {
				
				if (fsrP)
					*fsrP = 0x0d;	//perm error
				
debug(1, "XXX", "icacheFetch memAccess (mmuIsOn %d, mapping %d)", mmuIsOn(ic->mmu), (mappingInfo & MMU_MAPPING_CACHEABLE) ? 1 : 0);
				return 0;
			}
			
			return 1;
		}
		
		if (!memAccess(ic->mem, pa, ICACHE_LINE_SZ, MEM_ACCESS_TYPE_READ, data)) {
			
			if (fsrP)
				*fsrP = 0x0d;	//perm error
			
debug(1, "XXX", "icacheFetch memAccess2");
			return 0;
		}
	
		sys_memcpy(line->data, data, ICACHE_LINE_SZ);
		line->info = va | (priviledged ? ICACHE_PRIV_MASK : 0) | ICACHE_USED_MASK;
	}
		
	if (sz == 4)
		*(uint32_t*)buf = *(uint32_t*)(line->data + off);
	else if (sz == 2) {
		//icache reads in words, but code requests may come in halfwords
		//on BE hosts this means we need to swap the order of halfwords
		// (to unswap what he had already swapped)
		#if SYS_ENDIAN == 2
			*(uint16_t*)buf = *(uint16_t*)(line->data + (off ^ 2));
		#elif SYS_ENDIAN == 1
			*(uint16_t*)buf = *(uint16_t*)(line->data + off);
		#else
			#error "WTF"
		#endif
	}
	else
		sys_memcpy(buf, line->data + off, sz);
	
debug(1, "XXX", "icacheFetch return %d", priviledged || !(line->info & ICACHE_PRIV_MASK) ? 1 : 0);
	return priviledged || !(line->info & ICACHE_PRIV_MASK);
}
