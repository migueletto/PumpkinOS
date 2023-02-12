#ifndef MEM_H
#include <stdint.h>

typedef struct mem_chunk_t mem_chunk_t;
typedef void *mem_handle_t;

#define NULL_HANDLE NULL

typedef uint32_t local_id_t;

#if UINTPTR_MAX == 0xffffffff
typedef uint32_t UIntPtr;
#elif UINTPTR_MAX == 0xffffffffffffffff
typedef uint64_t UIntPtr;
#else
#error "Word size not known"
#endif

#define MEM_H
#endif
