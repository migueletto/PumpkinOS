typedef struct RegMgrType RegMgrType;

#define regFlagsID      1
#define regFeatureID    2
#define regDisplayID    3
#define regWindowID     4
#define regNotifID      5

#define regFlagReset    1

typedef struct {
  UInt16 osVersion;
  UInt32 flags;
} RegFlagsType;

typedef struct {
  UInt32 creator;
  UInt32 number;
  UInt32 value;
} RegFeatureType;

typedef struct {
  UInt16 density;
  UInt16 depth;
} RegDisplayType;

typedef struct {
  UInt16 x;
  UInt16 y;
  UInt16 width;
  UInt16 height;
} RegWindowType;

typedef struct {
  UInt32 appCreator;
  UInt32 notifyType;
  UInt32 priority;
} RegNotificationType;

RegMgrType *RegInit(void);
void RegFinish(RegMgrType *rm);

void *RegGet(RegMgrType *rm, DmResType type, UInt16 id, UInt32 *size);
void *RegGetById(RegMgrType *rm, UInt16 id, UInt32 *size);
Err RegSet(RegMgrType *rm, DmResType type, UInt16 id, void *p, UInt32 size);
Err RegDelete(RegMgrType *rm, DmResType type);
