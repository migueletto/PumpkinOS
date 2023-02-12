DmOpenRef DbOpen(LocalID dbID, UInt16 mode, Err *err);
DmOpenRef DbOpenByName(char *name, UInt16 mode, Err *err);
Err DbClose(DmOpenRef dbRef);
Err DbCreate(char *name, UInt32 type, UInt32 creator);
Err DbResCreate(char *name, UInt32 type, UInt32 creator);
Err DbGetTypeCreator(char *name, UInt32 *type, UInt32 *creator);
Err DbGetAttributes(DmOpenRef dbRef, UInt16 *attr);
Err DbSetAttributes(DmOpenRef dbRef, UInt16 attr);

char *DbOpenRec(DmOpenRef dbRef, UInt16 index, Err *err);
Err DbCloseRec(DmOpenRef dbRef, UInt16 index, char *rec);
Err DbCreateRec(DmOpenRef dbRef, UInt16 *index, UInt16 size, UInt16 category);
Err DbDeleteRec(DmOpenRef dbRef, UInt16 index);
Err DbRemoveRec(DmOpenRef dbRef, UInt16 index);
Err DbResizeRec(DmOpenRef dbRef, UInt16 index, UInt32 size);
Err DbGetRecAttributes(DmOpenRef dbRef, UInt16 index, UInt16 *attr);
Err DbSetRecAttributes(DmOpenRef dbRef, UInt16 index, UInt16 attr);
Err DbGetRecID(DmOpenRef dbRef, UInt16 index, UInt32 *uid);
UInt16 DbNumRecords(DmOpenRef dbRef);

char *DbOpenAppInfo(DmOpenRef dbRef);
void DbCloseAppInfo(char *info);
