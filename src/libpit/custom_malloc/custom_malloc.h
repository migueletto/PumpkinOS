#ifndef _CUSTOM_MALLOC_H_
#define _CUSTOM_MALLOC_H_

#pragma pack(4)
typedef struct BD_t {
  struct BD_t *prev;
  struct BD_t *next;
  uint32_t blkSize;
} BD_t;

typedef struct HEAP_INFO_t {
  uint8_t *pHeap;
  uint32_t heapSize;
  uint32_t allocSize;
  BD_t *alloc_dll;
  BD_t *avail_dll;
  BD_t dummy1;
  BD_t dummy2;
} HEAP_INFO_t;

/*
 * Initializes the heap. It internally created a BD and places at start of the
 * heap. The BD is added to avail_dll.
 * @param pHeapInfo: Pointer to the current heap info.
 **/
void CustomMallocInit(HEAP_INFO_t *pHeapInfo);

/*
 * Mimics the malloc function(i.e provides the requested memory). Returns NULL in
 * case is it not possible to allocate the requested memory.
 * @param pHeapInfo: Pointer to the current heap info.
 * @param sz: The size of the requested memory.
 **/
void *CustomMalloc(HEAP_INFO_t *pHeapInfo, uint32_t size);

/*
 * Mimics the free function(i.e frees an block that was already allocated)
 * @param pHeapInfo: Pointer to the current heap info.
 * @param blkPtr: Pointer to the block to be freed.
 **/
void CustomFree(HEAP_INFO_t *pHeapInfo, void *p);

uint32_t CustomBlockSize(HEAP_INFO_t *pHeapInfo, void *p);

#endif	
