#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "mem.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define MAX_FEATURES 32

typedef struct {
  UInt32 creator;
  UInt16 featureNum;
  UInt32 value;
} feature_t;

typedef struct {
  feature_t features[MAX_FEATURES];
  UInt32 numFeatures;
} ftr_module_t;

extern thread_key_t *ftr_key;

int FtrInitModule(void) {
  ftr_module_t *module;

  if ((module = xcalloc(1, sizeof(ftr_module_t))) == NULL) {
    return -1;
  }

  thread_set(ftr_key, module);

  return 0;
}

int FtrFinishModule(void) {
  ftr_module_t *module = (ftr_module_t *)thread_get(ftr_key);

  if (module) {
    xfree(module);
  }

  return 0;
}

Err FtrInit(void) {
  return errNone;
}

Err FtrGet(UInt32 creator, UInt16 featureNum, UInt32 *valueP) {
  ftr_module_t *module = (ftr_module_t *)thread_get(ftr_key);
  UInt32 i;
  Err err = ftrErrNoSuchFeature;

  *valueP = 0;

  switch (creator) {
    case sysFileCSystem:
      switch (featureNum) {
        case sysFtrNumROMVersion:
          if (pumpkin_default_density() == kDensityDouble) {
            *valueP = sysMakeROMVersion(5, 0, 0, sysROMStageRelease, 0);
          } else {
            *valueP = sysMakeROMVersion(4, 0, 0, sysROMStageRelease, 0);
          }
          err = errNone;
          break;
        case sysFtrNumProcessorID:
#if SYS_CPU == 1
          *valueP = sysFtrNumProcessorARM720T;
          err = errNone;
#elif SYS_CPU == 2
          *valueP = sysFtrNumProcessorx86;
          err = errNone;
#endif
          break;
        case sysFtrNumLanguage:
          *valueP = lEnglish;
          err = errNone;
          break;
        case sysFtrNumNotifyMgrVersion:
          *valueP = sysNotifyVersionNum;
          err = errNone;
          break;
        case sysFtrNumBacklight:
          *valueP = 0;
          err = errNone;
          break;
        case sysFtrNumWinVersion:
          *valueP = pumpkin_default_density() == kDensityDouble ? 4 : 3;
          err = errNone;
          break;
        case sysFtrNumOEMCompanyID:
          *valueP = 1;
          err = errNone;
          break;
        case sysFtrNumOEMDeviceID:
          *valueP = 1;
          err = errNone;
          break;
        case sysFtrNumInputAreaFlags:
          if (pumpkin_dia_enabled()) {
            *valueP = grfFtrInputAreaFlagDynamic /*| grfFtrInputAreaFlagCollapsible*/;
            err = errNone;
          }
          break;
        case sysFtrNumAccessorTrapPresent:
          *valueP = 1;
          err = errNone;
          break;
        default:
          err = ftrErrNoSuchFeature;
          break;
      }
      break;
    case sysFileCSerialMgr:
      switch (featureNum) {
        case sysFtrNewSerialPresent:
          *valueP = 1;
          err = errNone;
          break;
        default:
          err = ftrErrNoSuchFeature;
          break;
      }
      break;
    case sysFileCExpansionMgr:
      switch (featureNum) {
        case expFtrIDVersion:
          *valueP = expMgrVersionNum;
          err = errNone;
          break;
        default:
          err = ftrErrNoSuchFeature;
          break;
      }
      break;
    case sysFileCVFSMgr:
      switch (featureNum) {
        case vfsFtrIDVersion:
          *valueP = vfsMgrVersionNum;
          err = errNone;
          break;
        default:
          err = ftrErrNoSuchFeature;
          break;
      }
      break;
    case pinCreator:
      if (pumpkin_dia_enabled()) {
        switch (featureNum) {
          case pinFtrAPIVersion:
            *valueP = pinAPIVersion1_1;
            err = errNone;
            break;
          default:
            err = ftrErrNoSuchFeature;
            break;
        }
      }
      break;
    default:
      if (module->numFeatures > 0) {
        for (i = 0; i < module->numFeatures; i++) {
          if (module->features[i].creator == creator && module->features[i].featureNum == featureNum) {
            *valueP = module->features[i].value;
            err = errNone;
            break;
          }
        }
      }
      break;
  }

  return err;
}

// A feature that you define in this manner remains defined until the
// next system reset or until you explicitly undefine the feature with FtrUnregister.

Err FtrSet(UInt32 creator, UInt16 featureNum, UInt32 newValue) {
  ftr_module_t *module = (ftr_module_t *)thread_get(ftr_key);
  UInt32 i;
  Err err = memErrNotEnoughSpace;

  for (i = 0; i < module->numFeatures; i++) {
    if (module->features[i].creator == creator && module->features[i].featureNum == featureNum) {
      module->features[i].value = newValue;
      err = errNone;
      break;
    }
  }

  if (i == module->numFeatures && module->numFeatures < MAX_FEATURES) {
    module->features[i].creator = creator;
    module->features[i].featureNum = featureNum;
    module->features[i].value = newValue;
    module->numFeatures++;
    err = errNone;
  }

  return err;
}

Err FtrUnregister(UInt32 creator, UInt16 featureNum) {
  ftr_module_t *module = (ftr_module_t *)thread_get(ftr_key);
  UInt32 i;
  Err err = ftrErrNoSuchFeature;

  if (module->numFeatures > 0) {
    for (i = 0; i < module->numFeatures; i++) {
      if (module->features[i].creator == creator && module->features[i].featureNum == featureNum) {
        if (module->numFeatures > 1 && i < module->numFeatures-1) {
          module->features[i].creator = module->features[module->numFeatures-1].creator;
          module->features[i].featureNum = module->features[module->numFeatures-1].featureNum;
          module->features[i].value = module->features[module->numFeatures-1].value;
        }
        module->numFeatures++;
        err = errNone;
        break;
      }
    }
  }

  return err;
}

Err FtrGetByIndex(UInt16 index, Boolean romTable, UInt32 *creatorP, UInt16 *numP, UInt32 *valueP) {
  debug(DEBUG_ERROR, "PALMOS", "FtrGetByIndex not implemented");
  return sysErrParamErr;
}

Err FtrPtrNew(UInt32 creator, UInt16 featureNum, UInt32 size, void **newPtrP) {
  debug(DEBUG_ERROR, "PALMOS", "FtrPtrNew not implemented");
  return sysErrParamErr;
}

Err FtrPtrFree(UInt32 creator, UInt16 featureNum) {
  debug(DEBUG_ERROR, "PALMOS", "FtrPtrFree not implemented");
  return sysErrParamErr;
}

Err FtrPtrResize(UInt32 creator, UInt16 featureNum, UInt32 newSize, void **newPtrP) {
  debug(DEBUG_ERROR, "PALMOS", "FtrPtrResize not implemented");
  return sysErrParamErr;
}
