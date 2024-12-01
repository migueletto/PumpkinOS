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
  appRegistryLast
} AppRegistryID;

typedef struct {
  UInt16 compat;
  UInt16 code;
} AppRegistryCompat;

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

enum {
  appCompatUnknown,
  appCompatOk,
  appCompatMinor,
  appCompatMajor,
  appCompatCrash
};

AppRegistryType *AppRegistryInit(char *regname);
void AppRegistryFinish(AppRegistryType *ar);

void AppRegistrySet(AppRegistryType *ar, UInt32 creator, AppRegistryID id, UInt16 seq, void *p);
Boolean AppRegistryGet(AppRegistryType *ar, UInt32 creator, AppRegistryID id, UInt16 seq, void *p);
void AppRegistrySetPreference(AppRegistryType *ar, UInt32 creator, UInt16 seq, void *p, UInt16 size, Boolean saved);
UInt16 AppRegistryGetPreference(AppRegistryType *ar, UInt32 creator, UInt16 seq, void *p, UInt16 size, Boolean saved);
void AppRegistryEnum(AppRegistryType *ar, void (*callback)(UInt32 creator, UInt16 seq, UInt16 index, UInt16 id, void *p, UInt16 size, void *data), UInt32 creator, AppRegistryID id, void *data);
void AppRegistryDelete(char *regname, UInt16 id);
