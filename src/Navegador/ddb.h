DmOpenRef DbOpen(LocalID dbID, UInt16 mode, Err *err);
DmOpenRef DbOpenByName(char *name, UInt16 mode, Err *err);
DmOpenRef DbOpenCreateByName(char *name, UInt32 type, UInt32 creator,
                             UInt16 mode, Err *err);
Err DbClose(DmOpenRef dbRef);
Err DbRename(char *oldname, char *newname);
Err DbDelete(char *name);
Err DbCreate(char *name, UInt32 type, UInt32 creator);
Err DbGetTypeCreator(char *name, UInt32 *type, UInt32 *creator);
Err DbGetAttributes(DmOpenRef dbRef, UInt16 *attr);
Err DbSetAttributes(DmOpenRef dbRef, UInt16 attr);

char *DbOpenRec(DmOpenRef dbRef, UInt16 index, Err *err);
Err DbUpdateCloseRec(DmOpenRef dbRef, UInt16 index, char *rec, Boolean update);
Err DbCloseRec(DmOpenRef dbRef, UInt16 index, char *rec);
Err DbCreateRec(DmOpenRef dbRef, UInt16 *index, UInt16 size, UInt16 category);
Err DbDeleteRec(DmOpenRef dbRef, UInt16 index);
Err DbResizeRec(DmOpenRef dbRef, UInt16 index, UInt32 size);
