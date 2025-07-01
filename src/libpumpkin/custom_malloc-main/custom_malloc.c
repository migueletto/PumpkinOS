#include "sys.h"
#include "debug.h"
#include "custom_malloc.h"

void CustomMallocInit(HEAP_INFO_t *pHeapInfo) {
  sys_memset(pHeapInfo->pHeap, 0, pHeapInfo->heapSize);
  pHeapInfo->alloc_dll = &(pHeapInfo->dummy1);
  pHeapInfo->alloc_dll->next = NULL;
  pHeapInfo->alloc_dll->prev = NULL;
  pHeapInfo->avail_dll = &(pHeapInfo->dummy2);
  pHeapInfo->avail_dll->prev = NULL;
  pHeapInfo->avail_dll->next = (BD_t *)pHeapInfo->pHeap;
  pHeapInfo->avail_dll->next->blkSize = pHeapInfo->heapSize - sizeof(BD_t);
  pHeapInfo->avail_dll->next->next = NULL;
  pHeapInfo->avail_dll->next->prev = pHeapInfo->avail_dll;
  pHeapInfo->allocSize = 0;
}

static void printLists(HEAP_INFO_t *hp) {
  BD_t *iter;

  debug(DEBUG_INFO, "Heap", "heap size %u, used %u", hp->heapSize, hp->allocSize);

  iter = hp->avail_dll->next;
  if (iter) debug(DEBUG_INFO, "Heap", "available list");
  while (iter != NULL) {
    debug(DEBUG_INFO, "Heap", "block size=%u offset=%u", iter->blkSize, (uint32_t)((uint8_t *)iter - hp->pHeap));
    iter = iter->next;
  }

/*
  iter = hp->alloc_dll->next;
  if (iter) debug(DEBUG_INFO, "Heap", "used list");
  while (iter != NULL) {
    debug(DEBUG_INFO, "Heap", "block size=%u offset=%u", iter->blkSize, (uint32_t)((uint8_t *)iter - hp->pHeap));
    iter = iter->next;
  }
*/
}

void *CustomMalloc(HEAP_INFO_t *pHeapInfo, uint32_t size) {
  BD_t *iter, *newFreeBlk;
  uint32_t rem;
  int32_t residueMemory;
  void *p;

  iter = pHeapInfo->avail_dll->next;
  rem = size & 3;
  if (rem) size += 4 - rem;

  debug(DEBUG_TRACE, "Heap", "CustomMalloc %u bytes", size);

  while (iter != NULL) {
    residueMemory = iter->blkSize - size;

    if (residueMemory > 0) {
      if ((uint32_t)residueMemory > sizeof(BD_t)) {
        // update the BD moved to alloc_dll
        iter->blkSize = size;

        newFreeBlk = (BD_t *)((uint8_t *)iter + sizeof(BD_t) + size);
        newFreeBlk->blkSize = residueMemory - sizeof(BD_t);

        // remove iter and add the new BD to the avail_dll
        iter->prev->next = newFreeBlk;
        newFreeBlk->next = iter->next;
        newFreeBlk->prev = iter->prev;
        if (iter->next != NULL) {
          iter->next->prev = newFreeBlk;
        }
      } else {
        // remove the current BD from avail_dll
        iter->prev->next = iter->next;
        if (iter->next != NULL) {
          iter->next->prev = iter->prev;
        }
      }

      // add iter to the alloc_dll

      // establish forward link
      iter->next = pHeapInfo->alloc_dll->next;
      pHeapInfo->alloc_dll->next = iter;

      // establish backward link
      if (iter->next != NULL) {
        iter->next->prev = iter;
      }

      iter->prev = pHeapInfo->alloc_dll;
      pHeapInfo->allocSize += sizeof(BD_t) + iter->blkSize;
      p =  (uint8_t *)iter + sizeof(BD_t);
      sys_memset(p, 0, iter->blkSize);
      return p;
    }

    iter = iter->next;
  }

  debug(DEBUG_ERROR, "Heap", "CustomMalloc %u bytes failed", size);
  printLists(pHeapInfo);

  return NULL;
}

BD_t *CustomBlock(HEAP_INFO_t *pHeapInfo, void *p) {
  if (!pHeapInfo || !p) return 0;
  return p - sizeof(BD_t);
}

void CustomFree(HEAP_INFO_t *pHeapInfo, void *p) {
  BD_t *bd, *iter;
  uint32_t currSize;

  if (!pHeapInfo || !p) return;

  bd = p - sizeof(BD_t);
  pHeapInfo->allocSize -= sizeof(BD_t) + bd->blkSize;
  sys_memset(p, 0, bd->blkSize);

  // remove from alloc_dll
  bd->prev->next = bd->next;
  if (bd->next != NULL) {
    bd->next->prev = bd->prev;
  }

  // add bd to avail_dll
  iter = pHeapInfo->avail_dll;
  while (iter->next != NULL) {
    // check whether the preceding BD is found
    if (iter->next > bd) break;
    iter = iter->next;
  }

  // re-map the forward links
  bd->next = iter->next;
  iter->next = bd;

  // re-map the reverse links
  bd->prev = iter;
  if (bd->next != NULL) {
    bd->next->prev = bd;
  }

  // check whether the blocks can be merged

  // try merging current block and next block
  currSize = sizeof(BD_t) + bd->blkSize;
  if (((uint8_t *)bd + currSize) == (uint8_t *)(bd->next)) {
    bd->blkSize += sizeof(BD_t) + bd->next->blkSize;

    bd->next = bd->next->next;
    if (bd->next != NULL) {
      bd->next->prev = bd;
    }
    sys_memset((uint8_t *)bd + currSize, 0, sizeof(BD_t));
  }

  // try merging previous block and current block
  currSize = sizeof(BD_t) + iter->blkSize;
  if (((uint8_t *)iter + currSize) == (uint8_t *)bd) {
    iter->blkSize += sizeof(BD_t) + bd->blkSize;

    iter->next = bd->next;
    if (iter->next != NULL) {
      iter->next->prev = iter;
    }
    sys_memset((uint8_t *)iter + currSize, 0, sizeof(BD_t));
  }
}
