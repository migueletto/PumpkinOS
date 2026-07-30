#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "mutex.h"
#include "storage.h"
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_DmSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapDmDetachRecord: {
      // Err DmDetachRecord(DmOpenRef dbP, UInt16 index, MemHandle *oldHP)
      uint32_t dbP = ARG32;
      uint16_t index = ARG16;
      uint32_t oldHP = ARG32;
      emupalmos_trap_in(oldHP, trap, 2);
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      MemHandle old;
      Err res = DmDetachRecord(dbRef, index, oldHP ? &old : NULL);
      if (oldHP) m68k_write_memory_32(oldHP, emupalmos_trap_out(old));
      debug(DEBUG_TRACE, "EmuPalmOS", "DmDetachRecord(0x%08X, %d, 0x%08X): %d", dbP, index, oldHP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapDmDetachResource: {
      // Err DmDetachResource(DmOpenRef dbP, UInt16 index, MemHandle *oldHP)
      uint32_t dbP = ARG32;
      uint16_t index = ARG16;
      uint32_t oldHP = ARG32;
      emupalmos_trap_in(oldHP, trap, 2);
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      MemHandle old;
      Err res = DmDetachResource(dbRef, index, oldHP ? &old : NULL);
      if (oldHP) m68k_write_memory_32(oldHP, emupalmos_trap_out(old));
      debug(DEBUG_TRACE, "EmuPalmOS", "DmDetachsource(0x%08X, %d, 0x%08X): %d", dbP, index, oldHP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapDmSearchResource: {
      // UInt16 DmSearchResource(DmResType resType, DmResID resID, MemHandle resH, DmOpenRef *dbPP)
      uint32_t type = ARG32;
      uint16_t resID = ARG16;
      uint32_t ih = ARG32;
      uint32_t dbPP = ARG32;
      MemHandle h = emupalmos_trap_in(ih, trap, 2);
      emupalmos_trap_in(dbPP, trap, 3);
      DmOpenRef dbP;
      UInt16 index = DmSearchResource(type, resID, h, dbPP ? &dbP : NULL);
      if (dbPP) m68k_write_memory_32(dbPP, emupalmos_trap_out(dbP));
      char stype[8];
      pumpkin_id2s(type, stype);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmSearchResource('%s', %d, 0x%08X, 0x%08X): %d", stype, resID, ih, dbPP, index);
      m68k_set_reg(M68K_REG_D0, index);
    }
    break;
    case sysTrapDmGetNextDatabaseByTypeCreator: {
      // Err DmGetNextDatabaseByTypeCreator(Boolean newSearch, DmSearchStatePtr stateInfoP, UInt32 type, UInt32 creator, Boolean onlyLatestVers, UInt16 *cardNoP, LocalID *dbIDP)
      uint8_t newSearch = ARG8;
      uint32_t stateInfoP = ARG32;
      uint32_t type = ARG32;
      uint32_t creator = ARG32;
      uint8_t onlyLatestVers = ARG8;
      uint32_t cardNoP = ARG32;
      uint32_t dbIDP = ARG32;
      emupalmos_trap_in(stateInfoP, trap, 1);
      emupalmos_trap_in(cardNoP, trap, 5);
      emupalmos_trap_in(dbIDP, trap, 6);
      DmSearchStateType stateInfo;
      UInt16 cardNo;
      LocalID dbID = 0;
      if (stateInfoP && !newSearch) {
        uint32_t info = m68k_read_memory_32(stateInfoP);
        stateInfo.p = emupalmos_trap_in(info, trap, -1);
      }
      Err err = DmGetNextDatabaseByTypeCreator(newSearch, stateInfoP ? &stateInfo : NULL, type, creator, onlyLatestVers, cardNoP ? &cardNo : NULL, dbIDP ? &dbID : NULL);
      if (stateInfoP) {
        uint32_t info = emupalmos_trap_out(stateInfo.p);
        m68k_write_memory_32(stateInfoP, info);
      }
      if (cardNoP) m68k_write_memory_16(cardNoP, cardNo);
      if (dbIDP) m68k_write_memory_32(dbIDP, dbID);
      char stype[8];
      char screator[8];
      pumpkin_id2s(type, stype);
      pumpkin_id2s(creator, screator);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmGetNextDatabaseByTypeCreator(%d, 0x%08X, '%s', '%s', %d, 0x%08X, 0x%08X): %d", newSearch, stateInfoP, stype, screator, onlyLatestVers, cardNoP, dbIDP, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapDmInsertionSort: {
      // Err DmInsertionSort(DmOpenRef dbP, DmComparF *comparF, Int16 other)
      uint32_t dbP = ARG32;
      uint32_t comparP = ARG32;
      int16_t other = ARG16;
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      Err res = DmInsertionSort68K(dbRef, comparP, other);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmInsertionSort(0x%08X, 0x%08X, %d): %d", dbP, comparP, other, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapDmQuickSort: {
      // Err DmQuickSort(DmOpenRef dbP, DmComparF *comparF, Int16 other)
      uint32_t dbP = ARG32;
      uint32_t comparP = ARG32;
      int16_t other = ARG16;
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      emupalmos_trap_in(comparP, trap, 1);
      Err res = DmQuickSort68K(dbRef, comparP, other);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmQuickSort(0x%08X, 0x%08X, %d): %d", dbP, comparP, other, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapDmFindSortPositionV10: {
      // UInt16 DmFindSortPositionV10(DmOpenRef dbP, void *newRecord, DmComparF *compar, Int16 other)
      uint32_t dbP = ARG32;
      uint32_t newRecordP = ARG32;
      uint32_t comparP = ARG32;
      int16_t other = ARG16;
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      emupalmos_trap_in(newRecordP, trap, 1);
      emupalmos_trap_in(comparP, trap, 2);
      UInt16 res = DmFindSortPosition68K(dbRef, newRecordP, 0, comparP, other);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmFindSortPositionV10(0x%08X, 0x%08X, 0x%08X, %d): %d", dbP, newRecordP, comparP, other, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapDmFindSortPosition: {
      // UInt16 DmFindSortPosition(DmOpenRef dbP, void *newRecord, SortRecordInfoPtr newRecordInfo, DmComparF *compar, Int16 other)
      uint32_t dbP = ARG32;
      uint32_t newRecordP = ARG32;
      uint32_t newRecordInfoP = ARG32;
      uint32_t comparP = ARG32;
      int16_t other = ARG16;
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      emupalmos_trap_in(newRecordP, trap, 1);
      emupalmos_trap_in(newRecordInfoP, trap, 2);
      emupalmos_trap_in(comparP, trap, 3);
      UInt16 res = DmFindSortPosition68K(dbRef, newRecordP, newRecordInfoP, comparP, other);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmFindSortPosition(0x%08X, 0x%08X, 0x%08X, 0x%08X, %d): %d", dbP, newRecordP, newRecordInfoP, comparP, other, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapDmAttachRecord: {
      // Err DmAttachRecord(DmOpenRef dbP, UInt16 *atP, MemHandle newH, MemHandle *oldHP)
      uint32_t dbP = ARG32;
      uint32_t atP = ARG32;
      uint32_t newH = ARG32;
      uint32_t oldHP = ARG32;
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      emupalmos_trap_in(atP, trap, 1);
      UInt16 at = atP ? m68k_read_memory_16(atP) : 0;
      MemHandle h = emupalmos_trap_in(newH, trap, 2);
      emupalmos_trap_in(oldHP, trap, 3);
      MemHandle old;
      Err res = DmAttachRecord(dbRef, atP ? &at : NULL, h, oldHP ? &old : NULL);
      if (atP) m68k_write_memory_16(atP, at);
      if (oldHP) m68k_write_memory_32(oldHP, emupalmos_trap_out(old));
      debug(DEBUG_TRACE, "EmuPalmOS", "DmAttachRecord(0x%08X, 0x%08X, 0x%08X, 0x%08X): %d", dbP, atP, newH, oldHP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapDmSync:
      // void DmSync(void)
      DmSync();
      debug(DEBUG_TRACE, "EmuPalmOS", "DmSync()");
    break;
    case sysTrapDmSyncDatabase: {
      // Err DmSyncDatabase(DmOpenRef dbRef)
      uint32_t dbP = ARG32;
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      Err res = DmSyncDatabase(dbRef);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmSyncDatabase(0x%08X): %d", dbP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapDmInit: {
      // Err DmInit(void)
      Err res = DmInit();
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmInit(): %d", res);
    }
    break;
    case sysTrapDmCreateDatabase: {
      // Err DmCreateDatabase(UInt16 cardNo, in Char *nameP, UInt32 creator, UInt32 type, Boolean resDB)
      uint16_t cardNo = ARG16;
      uint32_t nameP = ARG32;
      char *s_nameP = emupalmos_trap_in(nameP, trap, 1);
      uint32_t creator = ARG32;
      uint32_t type = ARG32;
      uint8_t resDB = ARG8;
      Err res = DmCreateDatabase(cardNo, nameP ? s_nameP : NULL, creator, type, resDB);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmCreateDatabase(cardNo=%d, nameP=0x%08X [%s], creator=%d, type=%d, resDB=%d): %d", cardNo, nameP, s_nameP, creator, type, resDB, res);
    }
    break;
    case sysTrapDmCreateDatabaseFromImage: {
      // Err DmCreateDatabaseFromImage(MemPtr bufferP)
      uint32_t bufferP = ARG32;
      void *l_bufferP = emupalmos_trap_in(bufferP, trap, 0);
      Err res = DmCreateDatabaseFromImage(bufferP ? l_bufferP : 0);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmCreateDatabaseFromImage(bufferP=0x%08X): %d", bufferP, res);
    }
    break;
    case sysTrapDmDeleteDatabase: {
      // Err DmDeleteDatabase(UInt16 cardNo, LocalID dbID)
      uint16_t cardNo = ARG16;
      LocalID dbID = ARG32;
      Err res = DmDeleteDatabase(cardNo, dbID);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmDeleteDatabase(cardNo=%d, dbID=0x%08X): %d", cardNo, dbID, res);
    }
    break;
    case sysTrapDmNumDatabases: {
      // UInt16 DmNumDatabases(UInt16 cardNo)
      uint16_t cardNo = ARG16;
      UInt16 res = DmNumDatabases(cardNo);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmNumDatabases(cardNo=%d): %d", cardNo, res);
    }
    break;
    case sysTrapDmGetDatabase: {
      // LocalID DmGetDatabase(UInt16 cardNo, UInt16 index)
      uint16_t cardNo = ARG16;
      uint16_t index = ARG16;
      LocalID res = DmGetDatabase(cardNo, index);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmGetDatabase(cardNo=%d, index=%d): 0x%08X", cardNo, index, res);
    }
    break;
    case sysTrapDmFindDatabase: {
      // LocalID DmFindDatabase(UInt16 cardNo, in Char *nameP)
      uint16_t cardNo = ARG16;
      uint32_t nameP = ARG32;
      char *s_nameP = emupalmos_trap_in(nameP, trap, 1);
      LocalID res = DmFindDatabase(cardNo, nameP ? s_nameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmFindDatabase(cardNo=%d, nameP=0x%08X [%s]): 0x%08X", cardNo, nameP, s_nameP, res);
    }
    break;
    case sysTrapDmDatabaseInfo: {
      // Err DmDatabaseInfo(UInt16 cardNo, LocalID dbID, out Char *nameP, out UInt16 *attributesP, out UInt16 *versionP, out UInt32 *crDateP, out UInt32 *modDateP, out UInt32 *bckUpDateP, out UInt32 *modNumP, out LocalID *appInfoIDP, out LocalID *sortInfoIDP, out UInt32 *typeP, out UInt32 *creatorP)
      uint16_t cardNo = ARG16;
      LocalID dbID = ARG32;
      uint32_t nameP = ARG32;
      char *s_nameP = emupalmos_trap_in(nameP, trap, 2);
      uint32_t attributesP = ARG32;
      UInt16 l_attributesP = 0;
      uint32_t versionP = ARG32;
      UInt16 l_versionP = 0;
      uint32_t crDateP = ARG32;
      UInt32 l_crDateP = 0;
      uint32_t modDateP = ARG32;
      UInt32 l_modDateP = 0;
      uint32_t bckUpDateP = ARG32;
      UInt32 l_bckUpDateP = 0;
      uint32_t modNumP = ARG32;
      UInt32 l_modNumP = 0;
      uint32_t appInfoIDP = ARG32;
      LocalID l_appInfoIDP = 0;
      uint32_t sortInfoIDP = ARG32;
      LocalID l_sortInfoIDP = 0;
      uint32_t typeP = ARG32;
      UInt32 l_typeP = 0;
      uint32_t creatorP = ARG32;
      UInt32 l_creatorP = 0;
      Err res = DmDatabaseInfo(cardNo, dbID, nameP ? s_nameP : NULL, attributesP ? &l_attributesP : NULL, versionP ? &l_versionP : NULL, crDateP ? &l_crDateP : NULL, modDateP ? &l_modDateP : NULL, bckUpDateP ? &l_bckUpDateP : NULL, modNumP ? &l_modNumP : NULL, appInfoIDP ? &l_appInfoIDP : NULL, sortInfoIDP ? &l_sortInfoIDP : NULL, typeP ? &l_typeP : NULL, creatorP ? &l_creatorP : NULL);
      if (attributesP) m68k_write_memory_16(attributesP, l_attributesP);
      if (versionP) m68k_write_memory_16(versionP, l_versionP);
      if (crDateP) m68k_write_memory_32(crDateP, l_crDateP);
      if (modDateP) m68k_write_memory_32(modDateP, l_modDateP);
      if (bckUpDateP) m68k_write_memory_32(bckUpDateP, l_bckUpDateP);
      if (modNumP) m68k_write_memory_32(modNumP, l_modNumP);
      if (appInfoIDP) m68k_write_memory_32(appInfoIDP, l_appInfoIDP);
      if (sortInfoIDP) m68k_write_memory_32(sortInfoIDP, l_sortInfoIDP);
      if (typeP) m68k_write_memory_32(typeP, l_typeP);
      if (creatorP) m68k_write_memory_32(creatorP, l_creatorP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmDatabaseInfo(cardNo=%d, dbID=0x%08X, nameP=0x%08X [%s], attributesP=0x%08X [%d], versionP=0x%08X [%d], crDateP=0x%08X [%d], modDateP=0x%08X [%d], bckUpDateP=0x%08X [%d], modNumP=0x%08X [%d], appInfoIDP=0x%08X, sortInfoIDP=0x%08X, typeP=0x%08X [%d], creatorP=0x%08X [%d]): %d", cardNo, dbID, nameP, s_nameP, attributesP, l_attributesP, versionP, l_versionP, crDateP, l_crDateP, modDateP, l_modDateP, bckUpDateP, l_bckUpDateP, modNumP, l_modNumP, appInfoIDP, sortInfoIDP, typeP, l_typeP, creatorP, l_creatorP, res);
    }
    break;
    case sysTrapDmSetDatabaseInfo: {
      // Err DmSetDatabaseInfo(UInt16 cardNo, LocalID dbID, in Char *nameP, in UInt16 *attributesP, in UInt16 *versionP, in UInt32 *crDateP, in UInt32 *modDateP, in UInt32 *bckUpDateP, in UInt32 *modNumP, in LocalID *appInfoIDP, in LocalID *sortInfoIDP, in UInt32 *typeP, in UInt32 *creatorP)
      uint16_t cardNo = ARG16;
      LocalID dbID = ARG32;
      uint32_t nameP = ARG32;
      char *s_nameP = emupalmos_trap_in(nameP, trap, 2);
      uint32_t attributesP = ARG32;
      UInt16 l_attributesP = 0;
      if (attributesP) l_attributesP = m68k_read_memory_16(attributesP);
      uint32_t versionP = ARG32;
      UInt16 l_versionP = 0;
      if (versionP) l_versionP = m68k_read_memory_16(versionP);
      uint32_t crDateP = ARG32;
      UInt32 l_crDateP = 0;
      if (crDateP) l_crDateP = m68k_read_memory_32(crDateP);
      uint32_t modDateP = ARG32;
      UInt32 l_modDateP = 0;
      if (modDateP) l_modDateP = m68k_read_memory_32(modDateP);
      uint32_t bckUpDateP = ARG32;
      UInt32 l_bckUpDateP = 0;
      if (bckUpDateP) l_bckUpDateP = m68k_read_memory_32(bckUpDateP);
      uint32_t modNumP = ARG32;
      UInt32 l_modNumP = 0;
      if (modNumP) l_modNumP = m68k_read_memory_32(modNumP);
      uint32_t appInfoIDP = ARG32;
      LocalID l_appInfoIDP = 0;
      if (appInfoIDP) l_appInfoIDP = m68k_read_memory_32(appInfoIDP);
      uint32_t sortInfoIDP = ARG32;
      LocalID l_sortInfoIDP = 0;
      if (sortInfoIDP) l_sortInfoIDP = m68k_read_memory_32(sortInfoIDP);
      uint32_t typeP = ARG32;
      UInt32 l_typeP = 0;
      if (typeP) l_typeP = m68k_read_memory_32(typeP);
      uint32_t creatorP = ARG32;
      UInt32 l_creatorP = 0;
      if (creatorP) l_creatorP = m68k_read_memory_32(creatorP);
      Err res = DmSetDatabaseInfo(cardNo, dbID, nameP ? s_nameP : NULL, attributesP ? &l_attributesP : NULL, versionP ? &l_versionP : NULL, crDateP ? &l_crDateP : NULL, modDateP ? &l_modDateP : NULL, bckUpDateP ? &l_bckUpDateP : NULL, modNumP ? &l_modNumP : NULL, appInfoIDP ? &l_appInfoIDP : NULL, sortInfoIDP ? &l_sortInfoIDP : NULL, typeP ? &l_typeP : NULL, creatorP ? &l_creatorP : NULL);
      if (attributesP) m68k_write_memory_16(attributesP, l_attributesP);
      if (versionP) m68k_write_memory_16(versionP, l_versionP);
      if (crDateP) m68k_write_memory_32(crDateP, l_crDateP);
      if (modDateP) m68k_write_memory_32(modDateP, l_modDateP);
      if (bckUpDateP) m68k_write_memory_32(bckUpDateP, l_bckUpDateP);
      if (modNumP) m68k_write_memory_32(modNumP, l_modNumP);
      if (typeP) m68k_write_memory_32(typeP, l_typeP);
      if (creatorP) m68k_write_memory_32(creatorP, l_creatorP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmSetDatabaseInfo(cardNo=%d, dbID=0x%08X, nameP=0x%08X [%s], attributesP=0x%08X [%d], versionP=0x%08X [%d], crDateP=0x%08X [%d], modDateP=0x%08X [%d], bckUpDateP=0x%08X [%d], modNumP=0x%08X [%d], appInfoIDP=0x%08X, sortInfoIDP=0x%08X, typeP=0x%08X [%d], creatorP=0x%08X [%d]): %d", cardNo, dbID, nameP, s_nameP, attributesP, l_attributesP, versionP, l_versionP, crDateP, l_crDateP, modDateP, l_modDateP, bckUpDateP, l_bckUpDateP, modNumP, l_modNumP, appInfoIDP, sortInfoIDP, typeP, l_typeP, creatorP, l_creatorP, res);
    }
    break;
    case sysTrapDmDatabaseSize: {
      // Err DmDatabaseSize(UInt16 cardNo, LocalID dbID, out UInt32 *numRecordsP, out UInt32 *totalBytesP, out UInt32 *dataBytesP)
      uint16_t cardNo = ARG16;
      LocalID dbID = ARG32;
      uint32_t numRecordsP = ARG32;
      UInt32 l_numRecordsP = 0;
      uint32_t totalBytesP = ARG32;
      UInt32 l_totalBytesP = 0;
      uint32_t dataBytesP = ARG32;
      UInt32 l_dataBytesP = 0;
      Err res = DmDatabaseSize(cardNo, dbID, numRecordsP ? &l_numRecordsP : NULL, totalBytesP ? &l_totalBytesP : NULL, dataBytesP ? &l_dataBytesP : NULL);
      if (numRecordsP) m68k_write_memory_32(numRecordsP, l_numRecordsP);
      if (totalBytesP) m68k_write_memory_32(totalBytesP, l_totalBytesP);
      if (dataBytesP) m68k_write_memory_32(dataBytesP, l_dataBytesP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmDatabaseSize(cardNo=%d, dbID=0x%08X, numRecordsP=0x%08X [%d], totalBytesP=0x%08X [%d], dataBytesP=0x%08X [%d]): %d", cardNo, dbID, numRecordsP, l_numRecordsP, totalBytesP, l_totalBytesP, dataBytesP, l_dataBytesP, res);
    }
    break;
    case sysTrapDmDatabaseProtect: {
      // Err DmDatabaseProtect(UInt16 cardNo, LocalID dbID, Boolean protect)
      uint16_t cardNo = ARG16;
      LocalID dbID = ARG32;
      uint8_t protect = ARG8;
      Err res = DmDatabaseProtect(cardNo, dbID, protect);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmDatabaseProtect(cardNo=%d, dbID=0x%08X, protect=%d): %d", cardNo, dbID, protect, res);
    }
    break;
    case sysTrapDmOpenDatabase: {
      // DmOpenRef DmOpenDatabase(UInt16 cardNo, LocalID dbID, UInt16 mode)
      uint16_t cardNo = ARG16;
      LocalID dbID = ARG32;
      uint16_t mode = ARG16;
      DmOpenRef res = DmOpenDatabase(cardNo, dbID, mode);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmOpenDatabase(cardNo=%d, dbID=0x%08X, mode=%d): 0x%08X", cardNo, dbID, mode, r_res);
    }
    break;
    case sysTrapDmOpenDatabaseByTypeCreator: {
      // DmOpenRef DmOpenDatabaseByTypeCreator(UInt32 type, UInt32 creator, UInt16 mode)
      uint32_t type = ARG32;
      uint32_t creator = ARG32;
      uint16_t mode = ARG16;
      DmOpenRef res = DmOpenDatabaseByTypeCreator(type, creator, mode);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      char stype[8], screator[8];
      pumpkin_id2s(type, stype);
      pumpkin_id2s(creator, screator);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmOpenDatabaseByTypeCreator(type='%s', creator='%s', mode=%d): 0x%08X", stype, screator, mode, r_res);
    }
    break;
    case sysTrapDmOpenDBNoOverlay: {
      // DmOpenRef DmOpenDBNoOverlay(UInt16 cardNo, LocalID dbID, UInt16 mode)
      uint16_t cardNo = ARG16;
      LocalID dbID = ARG32;
      uint16_t mode = ARG16;
      DmOpenRef res = DmOpenDBNoOverlay(cardNo, dbID, mode);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmOpenDBNoOverlay(cardNo=%d, dbID=0x%08X, mode=%d): 0x%08X", cardNo, dbID, mode, r_res);
    }
    break;
    case sysTrapDmCloseDatabase: {
      // Err DmCloseDatabase(DmOpenRef dbP)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      Err res = DmCloseDatabase(dbP ? l_dbP : 0);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmCloseDatabase(dbP=0x%08X): %d", dbP, res);
    }
    break;
    case sysTrapDmNextOpenDatabase: {
      // DmOpenRef DmNextOpenDatabase(DmOpenRef currentP)
      uint32_t currentP = ARG32;
      DmOpenRef l_currentP = emupalmos_trap_in(currentP, trap, 0);
      DmOpenRef res = DmNextOpenDatabase(currentP ? l_currentP : 0);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmNextOpenDatabase(currentP=0x%08X): 0x%08X", currentP, r_res);
    }
    break;
    case sysTrapDmOpenDatabaseInfo: {
      // Err DmOpenDatabaseInfo(DmOpenRef dbP, out LocalID *dbIDP, out UInt16 *openCountP, out UInt16 *modeP, out UInt16 *cardNoP, out Boolean *resDBP)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint32_t dbIDP = ARG32;
      LocalID l_dbIDP = 0;
      uint32_t openCountP = ARG32;
      UInt16 l_openCountP = 0;
      uint32_t modeP = ARG32;
      UInt16 l_modeP = 0;
      uint32_t cardNoP = ARG32;
      UInt16 l_cardNoP = 0;
      uint32_t resDBP = ARG32;
      Boolean l_resDBP = false;
      Err res = DmOpenDatabaseInfo(dbP ? l_dbP : 0, dbIDP ? &l_dbIDP : NULL, openCountP ? &l_openCountP : NULL, modeP ? &l_modeP : NULL, cardNoP ? &l_cardNoP : NULL, resDBP ? &l_resDBP : NULL);
      if (dbIDP) m68k_write_memory_32(dbIDP, l_dbIDP);
      if (openCountP) m68k_write_memory_16(openCountP, l_openCountP);
      if (modeP) m68k_write_memory_16(modeP, l_modeP);
      if (cardNoP) m68k_write_memory_16(cardNoP, l_cardNoP);
      if (resDBP) m68k_write_memory_8(resDBP, l_resDBP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmOpenDatabaseInfo(dbP=0x%08X, dbIDP=0x%08X, openCountP=0x%08X [%d], modeP=0x%08X [%d], cardNoP=0x%08X [%d], resDBP=0x%08X): %d", dbP, dbIDP, openCountP, l_openCountP, modeP, l_modeP, cardNoP, l_cardNoP, resDBP, res);
    }
    break;
    case sysTrapDmGetAppInfoID: {
      // LocalID DmGetAppInfoID(DmOpenRef dbP)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      LocalID res = DmGetAppInfoID(dbP ? l_dbP : 0);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmGetAppInfoID(dbP=0x%08X): 0x%08X", dbP, res);
    }
    break;
    case sysTrapDmGetDatabaseLockState: {
      // void DmGetDatabaseLockState(DmOpenRef dbR, out UInt8 *highest, out UInt32 *count, out UInt32 *busy)
      uint32_t dbR = ARG32;
      DmOpenRef l_dbR = emupalmos_trap_in(dbR, trap, 0);
      uint32_t highest = ARG32;
      UInt8 l_highest = 0;
      uint32_t count = ARG32;
      UInt32 l_count = 0;
      uint32_t busy = ARG32;
      UInt32 l_busy = 0;
      DmGetDatabaseLockState(dbR ? l_dbR : 0, highest ? &l_highest : NULL, count ? &l_count : NULL, busy ? &l_busy : NULL);
      if (highest) m68k_write_memory_8(highest, l_highest);
      if (count) m68k_write_memory_32(count, l_count);
      if (busy) m68k_write_memory_32(busy, l_busy);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmGetDatabaseLockState(dbR=0x%08X, highest=0x%08X, count=0x%08X [%d], busy=0x%08X [%d])", dbR, highest, count, l_count, busy, l_busy);
    }
    break;
    case sysTrapDmResetRecordStates: {
      // Err DmResetRecordStates(DmOpenRef dbP)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      Err res = DmResetRecordStates(dbP ? l_dbP : 0);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmResetRecordStates(dbP=0x%08X): %d", dbP, res);
    }
    break;
    case sysTrapDmGetLastErr: {
      // Err DmGetLastErr(void)
      Err res = DmGetLastErr();
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmGetLastErr(): %d", res);
    }
    break;
    case sysTrapDmNumRecords: {
      // UInt16 DmNumRecords(DmOpenRef dbP)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      UInt16 res = DmNumRecords(dbP ? l_dbP : 0);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmNumRecords(dbP=0x%08X): %d", dbP, res);
    }
    break;
    case sysTrapDmNumRecordsInCategory: {
      // UInt16 DmNumRecordsInCategory(DmOpenRef dbP, UInt16 category)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t category = ARG16;
      UInt16 res = DmNumRecordsInCategory(dbP ? l_dbP : 0, category);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmNumRecordsInCategory(dbP=0x%08X, category=%d): %d", dbP, category, res);
    }
    break;
    case sysTrapDmRecordInfo: {
      // Err DmRecordInfo(DmOpenRef dbP, UInt16 index, out UInt16 *attrP, out UInt32 *uniqueIDP, out LocalID *chunkIDP)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      uint32_t attrP = ARG32;
      UInt16 l_attrP = 0;
      uint32_t uniqueIDP = ARG32;
      UInt32 l_uniqueIDP = 0;
      uint32_t chunkIDP = ARG32;
      LocalID l_chunkIDP = 0;
      Err res = DmRecordInfo(dbP ? l_dbP : 0, index, attrP ? &l_attrP : NULL, uniqueIDP ? &l_uniqueIDP : NULL, chunkIDP ? &l_chunkIDP : NULL);
      if (attrP) m68k_write_memory_16(attrP, l_attrP);
      if (uniqueIDP) m68k_write_memory_32(uniqueIDP, l_uniqueIDP);
      if (chunkIDP) m68k_write_memory_32(chunkIDP, l_chunkIDP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmRecordInfo(dbP=0x%08X, index=%d, attrP=0x%08X [%d], uniqueIDP=0x%08X [%d], chunkIDP=0x%08X): %d", dbP, index, attrP, l_attrP, uniqueIDP, l_uniqueIDP, chunkIDP, res);
    }
    break;
    case sysTrapDmSetRecordInfo: {
      // Err DmSetRecordInfo(DmOpenRef dbP, UInt16 index, in UInt16 *attrP, in UInt32 *uniqueIDP)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      uint32_t attrP = ARG32;
      UInt16 l_attrP = 0;
      if (attrP) l_attrP = m68k_read_memory_16(attrP);
      uint32_t uniqueIDP = ARG32;
      UInt32 l_uniqueIDP = 0;
      if (uniqueIDP) l_uniqueIDP = m68k_read_memory_32(uniqueIDP);
      Err res = DmSetRecordInfo(dbP ? l_dbP : 0, index, attrP ? &l_attrP : NULL, uniqueIDP ? &l_uniqueIDP : NULL);
      if (attrP) m68k_write_memory_16(attrP, l_attrP);
      if (uniqueIDP) m68k_write_memory_32(uniqueIDP, l_uniqueIDP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmSetRecordInfo(dbP=0x%08X, index=%d, attrP=0x%08X [%d], uniqueIDP=0x%08X [%d]): %d", dbP, index, attrP, l_attrP, uniqueIDP, l_uniqueIDP, res);
    }
    break;
    case sysTrapDmMoveRecord: {
      // Err DmMoveRecord(DmOpenRef dbP, UInt16 from, UInt16 to)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t from = ARG16;
      uint16_t to = ARG16;
      Err res = DmMoveRecord(dbP ? l_dbP : 0, from, to);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmMoveRecord(dbP=0x%08X, from=%d, to=%d): %d", dbP, from, to, res);
    }
    break;
    case sysTrapDmNewRecord: {
      // MemHandle DmNewRecord(DmOpenRef dbP, inout UInt16 *atP, UInt32 size)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint32_t atP = ARG32;
      UInt16 l_atP = 0;
      if (atP) l_atP = m68k_read_memory_16(atP);
      uint32_t size = ARG32;
      MemHandle res = DmNewRecord(dbP ? l_dbP : 0, atP ? &l_atP : NULL, size);
      if (atP) m68k_write_memory_16(atP, l_atP);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmNewRecord(dbP=0x%08X, atP=0x%08X [%d], size=%d): 0x%08X", dbP, atP, l_atP, size, r_res);
    }
    break;
    case sysTrapDmRemoveRecord: {
      // Err DmRemoveRecord(DmOpenRef dbP, UInt16 index)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      Err res = DmRemoveRecord(dbP ? l_dbP : 0, index);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmRemoveRecord(dbP=0x%08X, index=%d): %d", dbP, index, res);
    }
    break;
    case sysTrapDmDeleteRecord: {
      // Err DmDeleteRecord(DmOpenRef dbP, UInt16 index)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      Err res = DmDeleteRecord(dbP ? l_dbP : 0, index);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmDeleteRecord(dbP=0x%08X, index=%d): %d", dbP, index, res);
    }
    break;
    case sysTrapDmArchiveRecord: {
      // Err DmArchiveRecord(DmOpenRef dbP, UInt16 index)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      Err res = DmArchiveRecord(dbP ? l_dbP : 0, index);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmArchiveRecord(dbP=0x%08X, index=%d): %d", dbP, index, res);
    }
    break;
    case sysTrapDmNewHandle: {
      // MemHandle DmNewHandle(DmOpenRef dbP, UInt32 size)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint32_t size = ARG32;
      MemHandle res = DmNewHandle(dbP ? l_dbP : 0, size);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmNewHandle(dbP=0x%08X, size=%d): 0x%08X", dbP, size, r_res);
    }
    break;
    case sysTrapDmRemoveSecretRecords: {
      // Err DmRemoveSecretRecords(DmOpenRef dbP)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      Err res = DmRemoveSecretRecords(dbP ? l_dbP : 0);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmRemoveSecretRecords(dbP=0x%08X): %d", dbP, res);
    }
    break;
    case sysTrapDmFindRecordByID: {
      // Err DmFindRecordByID(DmOpenRef dbP, UInt32 uniqueID, out UInt16 *indexP)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint32_t uniqueID = ARG32;
      uint32_t indexP = ARG32;
      UInt16 l_indexP = 0;
      Err res = DmFindRecordByID(dbP ? l_dbP : 0, uniqueID, indexP ? &l_indexP : NULL);
      if (indexP) m68k_write_memory_16(indexP, l_indexP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmFindRecordByID(dbP=0x%08X, uniqueID=%d, indexP=0x%08X [%d]): %d", dbP, uniqueID, indexP, l_indexP, res);
    }
    break;
    case sysTrapDmQueryRecord: {
      // MemHandle DmQueryRecord(DmOpenRef dbP, UInt16 index)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      MemHandle res = DmQueryRecord(dbP ? l_dbP : 0, index);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmQueryRecord(dbP=0x%08X, index=%d): 0x%08X", dbP, index, r_res);
    }
    break;
    case sysTrapDmGetRecord: {
      // MemHandle DmGetRecord(DmOpenRef dbP, UInt16 index)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      MemHandle res = DmGetRecord(dbP ? l_dbP : 0, index);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmGetRecord(dbP=0x%08X, index=%d): 0x%08X", dbP, index, r_res);
    }
    break;
    case sysTrapDmQueryNextInCategory: {
      // MemHandle DmQueryNextInCategory(DmOpenRef dbP, inout UInt16 *indexP, UInt16 category)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint32_t indexP = ARG32;
      UInt16 l_indexP = 0;
      if (indexP) l_indexP = m68k_read_memory_16(indexP);
      uint16_t category = ARG16;
      MemHandle res = DmQueryNextInCategory(dbP ? l_dbP : 0, indexP ? &l_indexP : NULL, category);
      if (indexP) m68k_write_memory_16(indexP, l_indexP);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmQueryNextInCategory(dbP=0x%08X, indexP=0x%08X [%d], category=%d): 0x%08X", dbP, indexP, l_indexP, category, r_res);
    }
    break;
    case sysTrapDmPositionInCategory: {
      // UInt16 DmPositionInCategory(DmOpenRef dbP, UInt16 index, UInt16 category)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      uint16_t category = ARG16;
      UInt16 res = DmPositionInCategory(dbP ? l_dbP : 0, index, category);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmPositionInCategory(dbP=0x%08X, index=%d, category=%d): %d", dbP, index, category, res);
    }
    break;
    case sysTrapDmSeekRecordInCategory: {
      // Err DmSeekRecordInCategory(DmOpenRef dbP, inout UInt16 *indexP, UInt16 offset, Int16 direction, UInt16 category)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint32_t indexP = ARG32;
      UInt16 l_indexP = 0;
      if (indexP) l_indexP = m68k_read_memory_16(indexP);
      uint16_t offset = ARG16;
      int16_t direction = ARG16;
      uint16_t category = ARG16;
      Err res = DmSeekRecordInCategory(dbP ? l_dbP : 0, indexP ? &l_indexP : NULL, offset, direction, category);
      if (indexP) m68k_write_memory_16(indexP, l_indexP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmSeekRecordInCategory(dbP=0x%08X, indexP=0x%08X [%d], offset=%d, direction=%d, category=%d): %d", dbP, indexP, l_indexP, offset, direction, category, res);
    }
    break;
    case sysTrapDmResizeRecord: {
      // MemHandle DmResizeRecord(DmOpenRef dbP, UInt16 index, UInt32 newSize)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      uint32_t newSize = ARG32;
      MemHandle res = DmResizeRecord(dbP ? l_dbP : 0, index, newSize);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmResizeRecord(dbP=0x%08X, index=%d, newSize=%d): %p", dbP, index, newSize, res);
    }
    break;
    case sysTrapDmReleaseRecord: {
      // Err DmReleaseRecord(DmOpenRef dbP, UInt16 index, Boolean dirty)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      uint8_t dirty = ARG8;
      Err res = DmReleaseRecord(dbP ? l_dbP : 0, index, dirty);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmReleaseRecord(dbP=0x%08X, index=%d, dirty=%d): %d", dbP, index, dirty, res);
    }
    break;
    case sysTrapDmMoveCategory: {
      // Err DmMoveCategory(DmOpenRef dbP, UInt16 toCategory, UInt16 fromCategory, Boolean dirty)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t toCategory = ARG16;
      uint16_t fromCategory = ARG16;
      uint8_t dirty = ARG8;
      Err res = DmMoveCategory(dbP ? l_dbP : 0, toCategory, fromCategory, dirty);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmMoveCategory(dbP=0x%08X, toCategory=%d, fromCategory=%d, dirty=%d): %d", dbP, toCategory, fromCategory, dirty, res);
    }
    break;
    case sysTrapDmDeleteCategory: {
      // Err DmDeleteCategory(DmOpenRef dbR, UInt16 categoryNum)
      uint32_t dbR = ARG32;
      DmOpenRef l_dbR = emupalmos_trap_in(dbR, trap, 0);
      uint16_t categoryNum = ARG16;
      Err res = DmDeleteCategory(dbR ? l_dbR : 0, categoryNum);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmDeleteCategory(dbR=0x%08X, categoryNum=%d): %d", dbR, categoryNum, res);
    }
    break;
    case sysTrapDmWriteCheck: {
      // Err DmWriteCheck(out void *recordP, UInt32 offset, UInt32 bytes)
      uint32_t recordP = ARG32;
      void *s_recordP = emupalmos_trap_in(recordP, trap, 0);
      uint32_t offset = ARG32;
      uint32_t bytes = ARG32;
      Err res = DmWriteCheck(recordP ? s_recordP : NULL, offset, bytes);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmWriteCheck(recordP=0x%08X, offset=%d, bytes=%d): %d", recordP, offset, bytes, res);
    }
    break;
    case sysTrapDmWrite: {
      // Err DmWrite(out void *recordP, UInt32 offset, in void *srcP, UInt32 bytes)
      uint32_t recordP = ARG32;
      void *s_recordP = emupalmos_trap_in(recordP, trap, 0);
      uint32_t offset = ARG32;
      uint32_t srcP = ARG32;
      void *s_srcP = emupalmos_trap_in(srcP, trap, 2);
      uint32_t bytes = ARG32;
      Err res;
      if (emupalmos_check_address(recordP + offset, bytes, 0) && emupalmos_check_address(srcP, bytes, 1)) {
        res = DmWrite(recordP ? s_recordP : NULL, offset, srcP ? s_srcP : NULL, bytes);
      } else {
        res = dmErrInvalidParam;
      }
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmWrite(recordP=0x%08X, offset=%d, srcP=0x%08X, bytes=%d): %d", recordP, offset, srcP, bytes, res);
    }
    break;
    case sysTrapDmStrCopy: {
      // Err DmStrCopy(out void *recordP, UInt32 offset, in Char *srcP)
      uint32_t recordP = ARG32;
      void *s_recordP = emupalmos_trap_in(recordP, trap, 0);
      uint32_t offset = ARG32;
      uint32_t srcP = ARG32;
      char *s_srcP = emupalmos_trap_in(srcP, trap, 2);
      Err res = DmStrCopy(recordP ? s_recordP : NULL, offset, srcP ? s_srcP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmStrCopy(recordP=0x%08X, offset=%d, srcP=0x%08X [%s]): %d", recordP, offset, srcP, s_srcP, res);
    }
    break;
    case sysTrapDmSet: {
      // Err DmSet(out void *recordP, UInt32 offset, UInt32 bytes, UInt8 value)
      uint32_t recordP = ARG32;
      void *s_recordP = emupalmos_trap_in(recordP, trap, 0);
      uint32_t offset = ARG32;
      uint32_t bytes = ARG32;
      uint8_t value = ARG8;
      Err res = DmSet(recordP ? s_recordP : NULL, offset, bytes, value);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmSet(recordP=0x%08X, offset=%d, bytes=%d, value=%d): %d", recordP, offset, bytes, value, res);
    }
    break;
    case sysTrapDmGetResource: {
      // MemHandle DmGetResource(DmResType type, DmResID resID)
      uint32_t type = ARG32;
      char buf[8];
      pumpkin_id2s(type, buf);
      uint16_t resID = ARG16;
      MemHandle res = DmGetResource(type, resID);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmGetResource(type=%s, resID=%d): 0x%08X", buf, resID, r_res);
    }
    break;
    case sysTrapDmGet1Resource: {
      // MemHandle DmGet1Resource(DmResType type, DmResID resID)
      uint32_t type = ARG32;
      char buf[8];
      pumpkin_id2s(type, buf);
      uint16_t resID = ARG16;
      MemHandle res = DmGet1Resource(type, resID);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmGet1Resource(type=%s, resID=%d): 0x%08X", buf, resID, r_res);
    }
    break;
    case sysTrapDmReleaseResource: {
      // Err DmReleaseResource(MemHandle resourceH)
      uint32_t resourceH = ARG32;
      MemHandle l_resourceH = emupalmos_trap_in(resourceH, trap, 0);
      Err res = DmReleaseResource(resourceH ? l_resourceH : 0);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmReleaseResource(resourceH=0x%08X): %d", resourceH, res);
    }
    break;
    case sysTrapDmResizeResource: {
      // MemHandle DmResizeResource(MemHandle resourceH, UInt32 newSize)
      uint32_t resourceH = ARG32;
      MemHandle l_resourceH = emupalmos_trap_in(resourceH, trap, 0);
      uint32_t newSize = ARG32;
      MemHandle res = DmResizeResource(resourceH ? l_resourceH : 0, newSize);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmResizeResource(resourceH=0x%08X, newSize=%d): %p", resourceH, newSize, res);
    }
    break;
    case sysTrapDmNextOpenResDatabase: {
      // DmOpenRef DmNextOpenResDatabase(DmOpenRef dbP)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      DmOpenRef res = DmNextOpenResDatabase(dbP ? l_dbP : 0);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmNextOpenResDatabase(dbP=0x%08X): 0x%08X", dbP, r_res);
    }
    break;
    case sysTrapDmFindResourceType: {
      // UInt16 DmFindResourceType(DmOpenRef dbP, DmResType resType, UInt16 typeIndex)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint32_t resType = ARG32;
      uint16_t typeIndex = ARG16;
      UInt16 res = DmFindResourceType(dbP ? l_dbP : 0, resType, typeIndex);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmFindResourceType(dbP=0x%08X, resType=%d, typeIndex=%d): %d", dbP, resType, typeIndex, res);
    }
    break;
    case sysTrapDmFindResource: {
      // UInt16 DmFindResource(DmOpenRef dbP, DmResType resType, DmResID resID, MemHandle resH)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint32_t resType = ARG32;
      uint16_t resID = ARG16;
      uint32_t resH = ARG32;
      MemHandle l_resH = emupalmos_trap_in(resH, trap, 3);
      UInt16 res = DmFindResource(dbP ? l_dbP : 0, resType, resID, resH ? l_resH : 0);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmFindResource(dbP=0x%08X, resType=%d, resID=%d, resH=%d): %d", dbP, resType, resID, resH, res);
    }
    break;
    case sysTrapDmNumResources: {
      // UInt16 DmNumResources(DmOpenRef dbP)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      UInt16 res = DmNumResources(dbP ? l_dbP : 0);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmNumResources(dbP=0x%08X): %d", dbP, res);
    }
    break;
    case sysTrapDmResourceInfo: {
      // Err DmResourceInfo(DmOpenRef dbP, UInt16 index, out DmResType *resTypeP, out DmResID *resIDP, out LocalID *chunkLocalIDP)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      uint32_t resTypeP = ARG32;
      DmResType l_resTypeP;
      uint32_t resIDP = ARG32;
      DmResID l_resIDP;
      uint32_t chunkLocalIDP = ARG32;
      LocalID l_chunkLocalIDP;
      Err res = DmResourceInfo(dbP ? l_dbP : 0, index, resTypeP ? &l_resTypeP : NULL, resIDP ? &l_resIDP : NULL, chunkLocalIDP ? &l_chunkLocalIDP : NULL);
      if (resTypeP) m68k_write_memory_32(resTypeP, l_resTypeP);
      if (resIDP) m68k_write_memory_16(resIDP, l_resIDP);
      if (chunkLocalIDP) m68k_write_memory_32(chunkLocalIDP, l_chunkLocalIDP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmResourceInfo(dbP=0x%08X, index=%d, resTypeP=0x%08X, resIDP=0x%08X, chunkLocalIDP=0x%08X): %d", dbP, index, resTypeP, resIDP, chunkLocalIDP, res);
    }
    break;
    case sysTrapDmSetResourceInfo: {
      // Err DmSetResourceInfo(DmOpenRef dbP, UInt16 index, in DmResType *resTypeP, in DmResID *resIDP)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      uint32_t resTypeP = ARG32;
      DmResType l_resTypeP;
      if (resTypeP) l_resTypeP = m68k_read_memory_32(resTypeP);
      uint32_t resIDP = ARG32;
      DmResID l_resIDP;
      if (resIDP) l_resIDP = m68k_read_memory_16(resIDP);
      Err res = DmSetResourceInfo(dbP ? l_dbP : 0, index, resTypeP ? &l_resTypeP : NULL, resIDP ? &l_resIDP : NULL);
      if (resTypeP) m68k_write_memory_32(resTypeP, l_resTypeP);
      if (resIDP) m68k_write_memory_16(resIDP, l_resIDP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmSetResourceInfo(dbP=0x%08X, index=%d, resTypeP=0x%08X, resIDP=0x%08X): %d", dbP, index, resTypeP, resIDP, res);
    }
    break;
    case sysTrapDmNewResource: {
      // MemHandle DmNewResource(DmOpenRef dbP, DmResType resType, DmResID resID, UInt32 size)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint32_t resType = ARG32;
      uint16_t resID = ARG16;
      uint32_t size = ARG32;
      MemHandle res = DmNewResource(dbP ? l_dbP : 0, resType, resID, size);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmNewResource(dbP=0x%08X, resType=%d, resID=%d, size=%d): %p", dbP, resType, resID, size, res);
    }
    break;
    case sysTrapDmRemoveResource: {
      // Err DmRemoveResource(DmOpenRef dbP, UInt16 index)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      Err res = DmRemoveResource(dbP ? l_dbP : 0, index);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmRemoveResource(dbP=0x%08X, index=%d): %d", dbP, index, res);
    }
    break;
    case sysTrapDmGetResourceIndex: {
      // MemHandle DmGetResourceIndex(DmOpenRef dbP, UInt16 index)
      uint32_t dbP = ARG32;
      DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
      uint16_t index = ARG16;
      MemHandle res = DmGetResourceIndex(dbP ? l_dbP : 0, index);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmGetResourceIndex(dbP=0x%08X, index=%d): 0x%08X", dbP, index, r_res);
    }
    break;
  }
}
