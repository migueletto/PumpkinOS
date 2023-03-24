FileHand OpenLog(char *name, UInt32 creator, UInt32 type, UInt32 mode);
void CloseLog(FileHand);
Err DeleteLog(char *);
Err RenameLog(char *, char *);
Err SeekLog(FileHand f, Int32 offset);
UInt32 LogSize(FileHand);
Err WriteLog(FileHand, void *, UInt32);
Err ReadLog(FileHand, void *, UInt32);
