struct FileRecType {
  UInt16 cardNo;
  LocalID dbID;
  UInt32 size;
  char name[dmDBNameLength];
};
typedef struct FileRecType FileRecType;

struct FileType {
  Int16 n;
  char **fname;
  FileRecType *rec;
};
typedef struct FileType FileType;

Int16 CountFiles(UInt32, UInt32, char *);
Int16 ListFiles(UInt32, UInt32, char *, Int16, FileRecType *);
Err CreateFileList(UInt32, UInt32, FileType *, char *);
Err DestroyFileList(FileType *);
