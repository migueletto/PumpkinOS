#include <PalmOS.h>
#include <VFSMgr.h>

#include "debug.h"

#define MAX_FEATURES 32

#define sysFtrNumUIHardwareFlags        27           // Additional User Input Hardware (PalmOS 5.x)
#define sysFtrNumDmAutoBackup           31           // Is Data Manager Auto Backup supported?
#define sysFtrNumFiveWayNavVersion      32           // version of the 5-way nav if any

#define sysFtrNumUIHardwareHas5Way      0x00000001   // device has a 5-way rocker
#define sysFtrNumUIHardwareHasJog       0x00000002   // device has a jog wheel
#define sysFtrNumUIHardwareHasJogBack   0x00000004   // device has a jog wheel with a 'back' button
#define sysFtrNumUIHardwareHasKbd       0x00000008   // device has a dedicated keyboard

#define navFtrCreator  'fway'      // Creator ID for 5-Way navigator
#define navFtrVersion  0           // Feature id for 5-Way
#define navVersion     0x00010000  // Version for 5-Way

typedef struct {
  UInt32 creator;
  UInt16 featureNum;
  UInt32 value;
  Boolean ptr;
} feature_t;

typedef struct {
  feature_t features[MAX_FEATURES];
  UInt32 numFeatures;
} ftr_module_t;

int FtrInitModule(void) {
  ftr_module_t *module;

  if ((module = sys_calloc(1, sizeof(ftr_module_t))) == NULL) {
    return -1;
  }

  pumpkin_set_local_storage(ftr_key, module);

  return 0;
}

int FtrFinishModule(void) {
  ftr_module_t *module = (ftr_module_t *)pumpkin_get_local_storage(ftr_key);

  if (module) {
    sys_free(module);
  }

  return 0;
}

Err FtrInit(void) {
  return errNone;
}

static Err FtrGetEx(UInt32 creator, UInt16 featureNum, UInt32 *valueP, Boolean *ptr) {
  ftr_module_t *module = (ftr_module_t *)pumpkin_get_local_storage(ftr_key);
  UInt32 i;
  Int32 osversion;
  Err err = ftrErrNoSuchFeature;

  *valueP = 0;
  if (ptr) *ptr = false;

  switch (creator) {
    case sysFileCSystem:
      switch (featureNum) {
        case sysFtrNumROMVersion:
          osversion = pumpkin_get_osversion();
          *valueP = sysMakeROMVersion(osversion / 10, osversion % 10, 0, sysROMStageRelease, 0);
          err = errNone;
          break;
        case sysFtrNumProcessorID:
#if SYS_CPU == 1
          *valueP = sysFtrNumProcessorARM720T;
          err = errNone;
#elif SYS_CPU == 2
          *valueP = sysFtrNumProcessorx86;
          err = errNone;
#elif SYS_CPU == 3
          *valueP = sysFtrNumProcessorPPC64LE;
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
        case sysFtrNumDisplayDepth:
          if (WinScreenMode(winScreenModeGetDefaults, NULL, NULL, valueP, NULL) == errNone) {
            err = errNone;
          }
          break;
        case sysFtrNumWinVersion:
          *valueP = pumpkin_get_density() == kDensityDouble ? 4 : 3;
          err = errNone;
          break;
        case sysFtrNumOEMCompanyID:
          *valueP = 'Palm';
          err = errNone;
          break;
        case sysFtrNumOEMDeviceID:
          *valueP = 'Pmpk';
          err = errNone;
          break;
        case sysFtrNumOEMHALID:
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
        case sysFtrNumUIHardwareFlags:
          // XXX check if device has a keyboard (a Raspberry PI with just a touch screen should not set these flags)
          *valueP = sysFtrNumUIHardwareHas5Way /*| sysFtrNumUIHardwareHasKbd*/;
          err = errNone;
          break;
        case sysFtrNumDmAutoBackup:
          *valueP = 0;
          err = errNone;
          break;
        case sysFtrNumFiveWayNavVersion:
          *valueP = navVersion;
          err = errNone;
          break;
        default:
          debug(DEBUG_ERROR, "Feature", "FtrGet sysFileCSystem %d not defined", featureNum);
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
          debug(DEBUG_ERROR, "Feature", "FtrGet sysFileCSerialMgr %d not defined", featureNum);
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
          debug(DEBUG_ERROR, "Feature", "FtrGet sysFileCExpansionMgr %d not defined", featureNum);
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
          debug(DEBUG_ERROR, "Feature", "FtrGet sysFileCVFSMgr %d not defined", featureNum);
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
            debug(DEBUG_ERROR, "Feature", "FtrGet pinCreator %d not defined", featureNum);
            err = ftrErrNoSuchFeature;
            break;
        }
      }
      break;
    case navFtrCreator:
      switch (featureNum) {
        case navFtrVersion:
          *valueP = navVersion;
          err = errNone;
          break;
        default:
          debug(DEBUG_ERROR, "Feature", "FtrGet navFtrCreator %d not defined", featureNum);
          err = ftrErrNoSuchFeature;
          break;
      }
      break;
    default:
      if (module->numFeatures > 0) {
        for (i = 0; i < module->numFeatures; i++) {
          if (module->features[i].creator == creator && module->features[i].featureNum == featureNum) {
            *valueP = module->features[i].value;
            if (ptr) *ptr = module->features[i].ptr;
            err = errNone;
            break;
          }
        }
      }
      break;
  }

  return err;
}

Err FtrGet(UInt32 creator, UInt16 featureNum, UInt32 *valueP) {
  return FtrGetEx(creator, featureNum, valueP, NULL);
}

// A feature that you define in this manner remains defined until the
// next system reset or until you explicitly undefine the feature with FtrUnregister.
// XXX on PumpkinOS they are not persisted.

static Err FtrSetEx(UInt32 creator, UInt16 featureNum, UInt32 newValue, Boolean ptr) {
  ftr_module_t *module = (ftr_module_t *)pumpkin_get_local_storage(ftr_key);
  UInt32 i;
  Err err = memErrNotEnoughSpace;

  for (i = 0; i < module->numFeatures; i++) {
    if (module->features[i].creator == creator && module->features[i].featureNum == featureNum) {
      module->features[i].value = newValue;
      module->features[i].ptr = ptr;
      err = errNone;
      break;
    }
  }

  if (i == module->numFeatures && module->numFeatures < MAX_FEATURES) {
    module->features[i].creator = creator;
    module->features[i].featureNum = featureNum;
    module->features[i].value = newValue;
    module->features[i].ptr = ptr;
    module->numFeatures++;
    err = errNone;
  }

  return err;
}

Err FtrSet(UInt32 creator, UInt16 featureNum, UInt32 newValue) {
  return FtrSetEx(creator, featureNum, newValue, false);
}

Err FtrUnregister(UInt32 creator, UInt16 featureNum) {
  ftr_module_t *module = (ftr_module_t *)pumpkin_get_local_storage(ftr_key);
  UInt32 i;
  Err err = ftrErrNoSuchFeature;

  if (module->numFeatures > 0) {
    for (i = 0; i < module->numFeatures; i++) {
      if (module->features[i].creator == creator && module->features[i].featureNum == featureNum) {
        if (module->numFeatures > 1 && i < module->numFeatures-1) {
          module->features[i].creator = module->features[module->numFeatures-1].creator;
          module->features[i].featureNum = module->features[module->numFeatures-1].featureNum;
          module->features[i].value = module->features[module->numFeatures-1].value;
          module->features[i].ptr = module->features[module->numFeatures-1].ptr;
        }
        module->numFeatures++;
        err = errNone;
        break;
      }
    }
  }

  return err;
}

// This function is intended for system use only.
Err FtrGetByIndex(UInt16 index, Boolean romTable, UInt32 *creatorP, UInt16 *numP, UInt32 *valueP) {
  debug(DEBUG_ERROR, "Feature", "FtrGetByIndex not implemented");
  return sysErrParamErr;
}

Err FtrPtrNew(UInt32 creator, UInt16 featureNum, UInt32 size, void **newPtrP) {
  UInt8 *p, *ram;
  UInt32 value;
  Err err = memErrInvalidParam;

  if ((p = MemPtrNew(size)) != NULL) {
    ram = pumpkin_heap_base();
    value = p - ram;
    if ((err = FtrSetEx(creator, featureNum, value, true)) == errNone) {
      *newPtrP = p;
      err = errNone;
    } else {
      MemPtrFree(p);
    }
  }

  return err;
}

Err FtrPtrFree(UInt32 creator, UInt16 featureNum) {
  UInt8 *p, *ram;
  UInt32 value;
  Boolean ptr;
  Err err = memErrInvalidParam;

  if ((err = FtrGetEx(creator, featureNum, &value, &ptr)) == errNone) {
    if (ptr) {
      ram = pumpkin_heap_base();
      p = ram + value;
      MemPtrFree(p);
      err = FtrUnregister(creator, featureNum);
    } else {
      err = memErrInvalidParam;
    }
  }

  return err;
}

Err FtrPtrResize(UInt32 creator, UInt16 featureNum, UInt32 newSize, void **newPtrP) {
  UInt8 *p, *ram;
  UInt32 value;
  Boolean ptr;
  Err err = memErrInvalidParam;

  if ((err = FtrGetEx(creator, featureNum, &value, &ptr)) == errNone) {
    if (ptr) {
      ram = pumpkin_heap_base();
      p = ram + value;
      if ((err = MemPtrResize(p, newSize)) == errNone) {
        *newPtrP = p;
      }
    } else {
      err = memErrInvalidParam;
    }
  }

  return err;
}
