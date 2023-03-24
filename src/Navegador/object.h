void ObjectInit(Boolean highDensity);
void ObjectDefine(UInt16 num, DmOpenRef dbRef,
                  UInt16 newForm, UInt16 editForm, UInt16 seekForm,
                  Int16 (*getobjname)(void *, char *),
                  Int16 (*getobjdetail)(void *, char *),
                  UInt32 (*getobjdata)(void *),
                  UInt32 (*getobjdynamic)(void *),
                  void (*_getobjcenter)(void *, double *, double *),
                  Int16 _compare(void *, void *, Int32));
void ObjectSelect(UInt16 num);
void ObjectRefresh(FormPtr frm);
void ObjectSetTop(UInt16 i);
void ObjectPositionValid(Boolean valid);
