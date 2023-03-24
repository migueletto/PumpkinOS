#include <PalmOS.h>

#include "file.h"

static char name[dmDBNameLength];

static Int16 compare(void *e1, void *e2, Int32 other)
{
  return StrCompare(((FileRecType *)e1)->name, ((FileRecType *)e2)->name);
}

Err CreateFileList(UInt32 creator, UInt32 type, FileType *file, char *exclude)
{
  Int16 i;

  if (!file)
    return -1;

  file->n = CountFiles(creator, type, exclude);
  file->fname = NULL;
  file->rec = NULL;

  if (file->n) {
    if ((file->fname = MemPtrNew(file->n * sizeof(char *))) == NULL) {
      file->n = 0;
      return -1;
    }
    if ((file->rec = MemPtrNew(file->n * sizeof(FileRecType))) == NULL) {
      file->n = 0;
      MemPtrFree(file->fname);
      file->fname = NULL;
      return -1;
    }
    if (ListFiles(creator, type, exclude, file->n, file->rec) != file->n) {
      file->n = 0;
      MemPtrFree(file->rec);
      file->rec = NULL;
      MemPtrFree(file->fname);
      file->fname = NULL;
      return -1;
    }

    SysQSort(file->rec, file->n, sizeof(FileRecType), compare, 0);

    for (i = 0; i < file->n; i++)
      file->fname[i] = file->rec[i].name;
  }

  return 0;
}

Err DestroyFileList(FileType *file)
{
  if (!file)
    return -1;

  if (file->fname) 
    MemPtrFree(file->fname);
  if (file->rec)
    MemPtrFree(file->rec);

  file->n = 0;
  file->fname = NULL;
  file->rec = NULL;

  return 0;
}

Int16 CountFiles(UInt32 creator, UInt32 type, char *exclude)
{
  Boolean newSearch;
  DmSearchStateType stateInfo;
  UInt16 cardNo;
  LocalID dbID;
  Int16 i;
 
  for (i = 0, newSearch = true; ; newSearch = false) {
    if (DmGetNextDatabaseByTypeCreator(newSearch, &stateInfo, type, creator,
          false, &cardNo, &dbID) != 0)
      break;
    MemSet(name, dmDBNameLength, 0);
    if (DmDatabaseInfo(cardNo, dbID, name,
          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != 0)
      return i;
    if (exclude && !StrCompare(exclude, name))
      continue;
    i++;
  }
 
  return i;
}

Int16 ListFiles(UInt32 creator, UInt32 type, char *exclude,
                Int16 n, FileRecType *rec) 
{
  Boolean newSearch;
  DmSearchStateType stateInfo;
  LocalID dbID;
  UInt32 size;
  UInt16 cardNo;
  Int16 i;
 
  for (i = 0, newSearch = true; i < n; newSearch = false) {
    if (DmGetNextDatabaseByTypeCreator(newSearch, &stateInfo, type, creator,
          false, &cardNo, &dbID) != 0)
      break;
    MemSet(name, dmDBNameLength, 0);
    if (DmDatabaseInfo(cardNo, dbID, name,
          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != 0)
      return -1;
    if (exclude && !StrCompare(exclude, name))
      continue;
    DmDatabaseSize(cardNo, dbID, NULL, NULL, &size);
    rec[i].cardNo = cardNo;
    rec[i].dbID = dbID;
    rec[i].size = size;
    MemSet(rec[i].name, dmDBNameLength, 0);
    StrNCopy(rec[i].name, name, dmDBNameLength-1);
    i++;
  }
 
  return i;
}
