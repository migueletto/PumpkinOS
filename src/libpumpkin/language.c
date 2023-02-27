#include <PalmOS.h>

#include "sys.h"
#include "mutex.h"
#include "pwindow.h"
#include "pumpkin.h"
#include "AppRegistry.h"
#include "language.h"
#include "storage.h"
#include "debug.h"
#include "xalloc.h"

#define LANGUAGE_CREATOR 'lang'

language_t *LanguageInit(UInt32 id) {
  language_t *lang = NULL;
  DmSearchStateType stateInfo;
  Boolean firstLoad;
  UInt32 type;
  UInt16 cardNo;
  LocalID dbID;
  DmOpenRef dbRef;
  langSetF langSet;
  char *langName, stype[8];
  void *lib;
  Err err;

  sys_snprintf(stype, sizeof(stype)-1, "%04d", id);
  pumpkin_s2id(&type, stype);
  langName = PrefLanguageName(id);

  if ((err = DmGetNextDatabaseByTypeCreator(true, &stateInfo, type, LANGUAGE_CREATOR, false, &cardNo, &dbID)) == errNone) {
    if ((dbRef = DmOpenDatabase(cardNo, dbID, dmModeReadOnly)) != NULL) {
      if ((lib = DmResourceLoadLib(dbRef, sysRsrcTypeDlib, &firstLoad)) != NULL) {
        debug(DEBUG_INFO, "Language", "dlib resource loaded (first %d)", firstLoad ? 1 : 0);
        if ((langSet = sys_lib_defsymbol(lib, "LanguageSet", 1)) != NULL) {
          if ((lang = xcalloc(1, sizeof(language_t))) != NULL) {
            if (langSet(lang) == errNone) {
              debug(DEBUG_INFO, "Language", "LanguageSet %d (%s) succeeded", id, langName);
              lang->dbID = dbID;
              lang->dbRef = dbRef;
            } else {
              debug(DEBUG_ERROR, "Language", "LanguageSet %d (%s) failed", id, langName);
              xfree(lang);
              lang = NULL;
            }
          }
        } else {
          debug(DEBUG_ERROR, "Language", "LanguageSet not found in module %d (%s) ", id, langName);
        }
        // sys_lib_close is not being called
      } else {
        debug(DEBUG_ERROR, "Language", "module %d (%s) could not be loaded", id, langName);
      }
      if (!lang) DmCloseDatabase(dbRef);
    } else {
      debug(DEBUG_ERROR, "Language", "module %d (%s) could not be opened", id, langName);
    }
  } else {
    debug(DEBUG_INFO, "Language", "module %d (%s) not installed", id, langName);
  }

  return lang;
}

int LanguageFinish(language_t *lang) {
  if (lang) {
    if (lang->finish) {
      lang->finish(lang->data);
    }
    if (lang->dbRef) {
      DmCloseDatabase(lang->dbRef);
    }
    xfree(lang);
  }

  return 0;
}
