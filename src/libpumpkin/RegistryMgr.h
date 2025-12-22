typedef struct RegMgrType RegMgrType;

#define regOsID         1
#define regFlagsID      2
#define regPositionID   3
#define regDimensionID  4
#define regDisplayID    5
#define regFeatureID    6
#define regNotifID      7
#define regEndianID     8
#define regSoundID      9

#define regFlagReset    1

typedef struct {
  UInt16 version;
} RegOsType;

typedef struct {
  UInt32 flags;
} RegFlagsType;

typedef struct {
  UInt16 x;
  UInt16 y;
} RegPositionType;

typedef struct {
  UInt16 width;
  UInt16 height;
} RegDimensionType;

typedef struct {
  UInt16 density;
  UInt16 depth;
} RegDisplayType;

typedef struct {
  UInt32 creator;
  UInt32 number;
  UInt32 value;
} RegFeatureType;

typedef struct {
  UInt32 creator;
  UInt32 notifyType;
  UInt32 priority;
} RegNotificationType;

typedef struct {
  UInt16 littleEndian;
} RegDisplayEndianType;

typedef struct {
  UInt16 enableSound;
} RegSoundType;

RegMgrType *RegInit(void);
void RegFinish(RegMgrType *rm);

void *RegGet(RegMgrType *rm, DmResType type, UInt16 id, UInt32 *size);
void *RegGetById(RegMgrType *rm, UInt16 id, UInt32 *size);
Err RegSet(RegMgrType *rm, DmResType type, UInt16 id, void *p, UInt32 size);
Err RegDelete(RegMgrType *rm, DmResType type);
