#include <PalmOS.h>

#include "gps.h"
#include "app.h"
#include "list.h"
#include "ddb.h"

typedef struct {
  RecordType *record;
  char **name;
  Int16 numrecs;
  Int16 selection;
} RecordListType;

static RecordListType list[4];
static char buf[128], buf2[128];

static RecordType *RecGetList(DmOpenRef dbRef, Int16 *num,
                      Int16 (*getrecname)(void *, char *),
                      Int16 (*getrecdetail)(void *, char *),
                      UInt32 (*getrecdata)(void *),
                      UInt32 (*getrecdyn)(void *), Err *err);
static void RecFreeList(RecordType *record, Int16 num);
static Int16 search(void const *e1, void const *e2, Int32 len);
static Int16 searchc(void const *e1, void const *e2, Int32 len);
static void *lrealloc(void *ptr, UInt32 old_size, UInt32 size);

void ListInit(void)
{
  MemSet(list, sizeof(list), 0);
}

Int16 BuildRecList(Int16 id, DmOpenRef dbRef,
                   Int16 (*getrecname)(void *, char *),
                   Int16 (*getrecdetail)(void *, char *),
                   UInt32 (*getrecdata)(void *),
                   UInt32 (*getrecdyn)(void *),
                   Int16 (*compare)(void *, void *, Int32))
{
  Err err;
  Int16 i;

  FreeRecList(id);

  if ((list[id].record = RecGetList(dbRef, &list[id].numrecs, getrecname,
       getrecdetail, getrecdata, getrecdyn, &err)) == NULL || !list[id].numrecs)
    return 0;
   
  if ((list[id].name = MemPtrNew(list[id].numrecs*sizeof(char *))) == NULL)  
    return 0;

  SysQSort(list[id].record, list[id].numrecs, sizeof(RecordType), compare, 0);
  
  for (i = 0; i < list[id].numrecs; i++)
    list[id].name[i] = list[id].record[i].name;

  return list[id].numrecs;
}

Int16 GetRecNum(Int16 id)
{
  return list[id].numrecs;
}

Int16 GetRecSelection(Int16 id)
{
  return list[id].selection;
}

void SetRecSelection(Int16 id, Int16 sel)
{
  list[id].selection = sel;
}

Int16 GetRecIndex(Int16 id, Int16 i)
{
  return list[id].record[i].index;
}

char *GetRecName(Int16 id, Int16 i)
{
  return list[id].record[i].name;
}

char *GetRecDetail(Int16 id, Int16 i)
{
  return list[id].record[i].detail;
}

UInt32 GetRecData(Int16 id, Int16 i)
{
  return list[id].record[i].data;
}

UInt32 GetRecDynamic(Int16 id, Int16 i)
{
  return list[id].record[i].dynamic;
}

void FreeRecList(Int16 id)
{
  if (list[id].name) MemPtrFree(list[id].name);
  RecFreeList(list[id].record, list[id].numrecs);
  list[id].name = NULL;
  list[id].record = NULL;
  list[id].numrecs = 0;
}

Int16 FindRec(Int16 id, char *name, Int16 initial, Boolean caseless, Int16 len)
{
  RecordType rec;
  Int32 pos;

  if (initial >= list[id].numrecs)
    return -1;

  rec.name = name;
  rec.index = 0;
  pos = 0;

  if (!SysBinarySearch(&list[id].record[initial], list[id].numrecs - initial,
                       sizeof(RecordType), caseless ? searchc : search,
                       &rec, len, &pos, true))
    return -1;

  return pos - initial;
}

static RecordType *RecGetList(DmOpenRef dbRef, Int16 *num,
                              Int16 (*getrecname)(void *, char *),
                              Int16 (*getrecdetail)(void *, char *),
                              UInt32 (*getrecdata)(void *),
                              UInt32 (*getrecdyn)(void *), Err *err)
{
  char *rec;
  UInt16 index;
  Int16 iname, len, len2;
  UInt32 numRecs, data, dynamic;
  RecordType *record;

  *num = 0;
  *err = 0;

  if (!dbRef)
    return NULL;

  numRecs = DmNumRecords(dbRef);
  if (numRecs == 0)
    return NULL;

  if ((record = (RecordType *)MemPtrNew(numRecs*sizeof(RecordType))) == NULL) {
    *err = memErrNotEnoughSpace;
    return NULL;
  }

  MemSet(record, numRecs*sizeof(RecordType), 0);

  for (index = 0, iname = 0; index < numRecs; index++) {
    if ((rec = DbOpenRec(dbRef, index, err)) != NULL) {
      MemSet(buf, sizeof(buf), 0);
      len = getrecname(rec, buf);
      len2 = getrecdetail ? getrecdetail(rec, buf2) : 0;
      data = getrecdata ? getrecdata(rec) : 0xFFFFFFFFL;
      dynamic = getrecdyn ? getrecdyn(rec) : 0xFFFFFFFFL;
      DbCloseRec(dbRef, index, rec);

      record[iname].name = MemPtrNew(len);
      if (record[iname].name) {
        MemSet(record[iname].name, len, 0);
        MemMove(record[iname].name, buf, len);
        record[iname].index = index;
        record[iname].data = data;
        record[iname].dynamic = dynamic;

        record[iname].detail = len2 ? MemPtrNew(len2) : NULL;
        if (record[iname].detail) {
          MemSet(record[iname].detail, len2, 0);
          MemMove(record[iname].detail, buf2, len2);
        }

        iname++;
      }
    }
  }

  *err = 0;

  if (!iname) {
    MemPtrFree(record);
    return NULL;
  }

  record = lrealloc(record, numRecs*sizeof(RecordType), iname*sizeof(RecordType));

  *num = iname;
  return record;
}

static void RecFreeList(RecordType *record, Int16 num)
{
  if (record) {
    Int16 i;
    for (i = 0; i < num; i++)
      if (record[i].name)
        MemPtrFree(record[i].name);

    MemPtrFree(record);
  }
}

static void *lrealloc(void *ptr, UInt32 old_size, UInt32 size)
{
  void *new_ptr = MemPtrNew(size);

  if (!new_ptr) return NULL;
  MemSet(new_ptr, size, 0);

  if (ptr) {
    MemMove(new_ptr, ptr, size >= old_size ? old_size : size);
    MemPtrFree(ptr);
  }

  return new_ptr;
}

static Int16 search(void const *e1, void const *e2, Int32 len)
{
  Int16 r;

  if (len)
    r = StrNCompare(((RecordType *)e1)->name, ((RecordType *)e2)->name, len);
  else
    r = StrCompare(((RecordType *)e1)->name, ((RecordType *)e2)->name);

  return r;
}

static Int16 searchc(void const *e1, void const *e2, Int32 len)
{
  Int16 r;
    
  if (len)
    r = StrNCaselessCompare(((RecordType *)e1)->name,
                            ((RecordType *)e2)->name, len);
  else
    r = StrCaselessCompare(((RecordType *)e1)->name,
                           ((RecordType *)e2)->name);

  return r;
}
