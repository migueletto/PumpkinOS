typedef struct AppRegistryType AppRegistryType;

typedef enum {
  appRegistryCompat = 1,
  appRegistrySize,
  appRegistryPosition,
  appRegistryNotification,
  appRegistrySavedPref,
  appRegistryUnsavedPref,
  appRegistryOSVersion,
  appRegistryDepth,
  appRegistryFlags,
  appRegistryLast
} AppRegistryID;

typedef struct {
  UInt16 width, height;
} AppRegistrySize;

typedef struct {
  Int16 x, y;
} AppRegistryPosition;

typedef struct {
  UInt32 appCreator;
  UInt32 notifyType;
  UInt32 priority;
} AppRegistryNotification;

typedef struct {
  UInt16 osversion;
} AppRegistryOSVersion;

typedef struct {
  UInt16 depth;
} AppRegistryDepth;

typedef struct {
  UInt32 flags;
} AppRegistryFlags;

#define appRegistryFlagReset 1

AppRegistryType *AppRegistryInit(char *regname);
void AppRegistryFinish(AppRegistryType *ar);

void AppRegistrySet(AppRegistryType *ar, UInt32 creator, AppRegistryID id, UInt16 seq, void *p);
Boolean AppRegistryGet(AppRegistryType *ar, UInt32 creator, AppRegistryID id, UInt16 seq, void *p);
void AppRegistryEnum(AppRegistryType *ar, void (*callback)(UInt32 creator, UInt16 seq, UInt16 index, UInt16 id, void *p, UInt16 size, void *data), UInt32 creator, AppRegistryID id, void *data);
int AppRegistryDeleteByCreator(AppRegistryType *ar, UInt32 creator);
void AppRegistryDelete(char *regname, UInt16 id);
