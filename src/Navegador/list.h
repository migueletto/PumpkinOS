typedef struct {
  char *name;
  char *detail;
  Int16 index;
  UInt32 data;
  UInt32 dynamic;
} RecordType;

void ListInit(void);
Int16 BuildRecList(Int16 id, DmOpenRef dbRef,
                   Int16 (*getrecname)(void *, char *),
                   Int16 (*getrecdetail)(void *, char *),
                   UInt32 (*getrecdata)(void *),
                   UInt32 (*getrecdyn)(void *),
                   Int16 (*compare)(void *, void *, Int32));
void FreeRecList(Int16 id);
Int16 GetRecNum(Int16 id);
Int16 GetRecSelection(Int16 id);
void SetRecSelection(Int16 id, Int16 sel);
Int16 GetRecIndex(Int16 id, Int16 i);
char *GetRecName(Int16 id, Int16 i);
char *GetRecDetail(Int16 id, Int16 i);
UInt32 GetRecData(Int16 id, Int16 i);
UInt32 GetRecDynamic(Int16 id, Int16 i);
Int16 FindRec(Int16 id, char *name, Int16 initial, Boolean caseless, Int16 len);
