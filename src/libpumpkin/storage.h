#ifndef PIT_STORAGE_H
#define PIT_STORAGE_H

int StoInit(char *path, mutex_t *mutex);
int StoRefresh(void);
int StoFinish(void);
Int32 StoFileSeek(DmOpenRef dbP, UInt32 offset, Int32 whence);
Int32 StoFileRead(DmOpenRef dbP, void *p, Int32 size);
Int32 StoFileWrite(DmOpenRef dbP, void *p, Int32 size);
void *StoNewDecodedResource(void *h, UInt32 size, DmResType resType, DmResID resID);
void StoHeapWalk(uint32_t *p, uint32_t size, uint32_t task);

MemHandle MemLocalIDToHandle(LocalID local);
MemPtr MemHandleLockEx(MemHandle h, Boolean decoded);
void *DmResourceLoadLib(DmOpenRef dbP, DmResType resType, Boolean *firstLoad);
MemHandle DmNewResourceEx(DmOpenRef dbP, DmResType resType, DmResID resID, UInt32 size, void *p);
Err DmSetDirty(MemHandle handle);
Err DmCreateDatabaseEx(const Char *nameP, UInt32 creator, UInt32 type, UInt16 attr, UInt32 uniqueIDSeed, Boolean overwrite);
UInt16 DmFindSortPosition68K(DmOpenRef dbP, UInt32 newRecord, UInt32 newRecordInfo, UInt32 compar, Int16 other);
Err DmInsertionSort68K(DmOpenRef dbP, UInt32 comparF, Int16 other);
Err DmQuickSort68K(DmOpenRef dbP, UInt32 comparF, Int16 other);

#endif
