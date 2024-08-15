#include <PalmOS.h>
#include <VFSMgr.h>
    
#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"
    
typedef struct {
  FileRef ref;
} FileRefProxy;

void palmos_filesystemtrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];
    
  switch (sel) {
    case vfsTrapFileOpen: {
      // Err VFSFileOpen(UInt16 volRefNum, in Char *pathNameP, UInt16 openMode, FileRef *fileRefP);
      uint16_t volRefNum = ARG16;
      uint32_t pathNameP = ARG32;
      char *s_pathNameP = emupalmos_trap_sel_in(pathNameP, sysTrapFileSystemDispatch, sel, 0);
      uint16_t openMode = ARG16;
      uint32_t fileRefP = ARG32;
      FileRef l_fileRefP = 0;
      Err res = VFSFileOpen(volRefNum, pathNameP ? s_pathNameP : NULL, openMode, fileRefP ? &l_fileRefP : NULL);
      if (fileRefP && l_fileRefP) {
        FileRefProxy *proxy = pumpkin_heap_alloc(sizeof(FileRefProxy), "FileProxy");
        if (proxy) {
          proxy->ref = l_fileRefP;
          m68k_write_memory_32(fileRefP, emupalmos_trap_out(proxy));
        }
      }
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileOpen(volRefNum=%d, pathNameP=0x%08X [%s], openMode=0x%04X, fileRefP=0x%08X): %d",
        volRefNum, pathNameP, s_pathNameP ? s_pathNameP : "", openMode, fileRefP, res);
      }
      break;
    case vfsTrapFileClose: {
      // Err VFSFileClose(FileRef fileRef);
      uint32_t fileRef = ARG32;
      FileRefProxy *ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      Err res = VFSFileClose(fileRef ? l_fileRef : 0);
      if (ll_fileRef) pumpkin_heap_free(ll_fileRef, "FileProxy");
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileClose(fileRef=%d): %d", fileRef, res);
      }
      break;
    case vfsTrapInit: {
      // Err VFSInit(void);
      Err res = VFSInit();
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSInit(): %d", res);
    }
    break;
    case vfsTrapFileCreate: {
      // Err VFSFileCreate(UInt16 volRefNum, in Char *pathNameP);
      uint16_t volRefNum = ARG16;
      uint32_t pathNameP = ARG32;
      char *s_pathNameP = emupalmos_trap_sel_in(pathNameP, sysTrapFileSystemDispatch, sel, 1);
      Err res = VFSFileCreate(volRefNum, pathNameP ? s_pathNameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileCreate(volRefNum=%d, pathNameP=0x%08X [%s]): %d", volRefNum, pathNameP, s_pathNameP, res);
    }
    break;
    case vfsTrapFileReadData: {
      // Err VFSFileReadData(FileRef fileRef, UInt32 numBytes, out void *bufBaseP, UInt32 offset, out UInt32 *numBytesReadP);
      uint32_t fileRef = ARG32;
      FileRefProxy *ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      uint32_t numBytes = ARG32;
      uint32_t bufBaseP = ARG32;
      void *s_bufBaseP = emupalmos_trap_sel_in(bufBaseP, sysTrapFileSystemDispatch, sel, 0);
      uint32_t offset = ARG32;
      uint32_t numBytesReadP = ARG32;
      UInt32 l_numBytesReadP = 0;
      Err res = VFSFileReadData(fileRef ? l_fileRef : 0, numBytes, bufBaseP ? s_bufBaseP : NULL, offset, numBytesReadP ? &l_numBytesReadP : NULL);
      if (numBytesReadP) m68k_write_memory_32(numBytesReadP, l_numBytesReadP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileReadData(fileRef=%d, numBytes=%d, bufBaseP=0x%08X, offset=%d, numBytesReadP=0x%08X [%d]): %d", fileRef, numBytes, bufBaseP, offset, numBytesReadP, l_numBytesReadP, res);
    }
    break;
    case vfsTrapFileRead: {
      // Err VFSFileRead(FileRef fileRef, UInt32 numBytes, out void *bufP, out UInt32 *numBytesReadP);
      uint32_t fileRef = ARG32;
      FileRefProxy *ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      uint32_t numBytes = ARG32;
      uint32_t bufP = ARG32;
      void *s_bufP = emupalmos_trap_sel_in(bufP, sysTrapFileSystemDispatch, sel, 2);
      uint32_t numBytesReadP = ARG32;
      UInt32 l_numBytesReadP = 0;
      Err res = VFSFileRead(fileRef ? l_fileRef : 0, numBytes, bufP ? s_bufP : NULL, numBytesReadP ? &l_numBytesReadP : NULL);
      if (numBytesReadP) m68k_write_memory_32(numBytesReadP, l_numBytesReadP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileRead(fileRef=%d, numBytes=%d, bufP=0x%08X, numBytesReadP=0x%08X [%d]): %d", fileRef, numBytes, bufP, numBytesReadP, l_numBytesReadP, res);
    }
    break;
    case vfsTrapFileWrite: {
      // Err VFSFileWrite(FileRef fileRef, UInt32 numBytes, in void *dataP, out UInt32 *numBytesWrittenP);
      uint32_t fileRef = ARG32;
      FileRefProxy *ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      uint32_t numBytes = ARG32;
      uint32_t dataP = ARG32;
      void *s_dataP = emupalmos_trap_sel_in(dataP, sysTrapFileSystemDispatch, sel, 2);
      uint32_t numBytesWrittenP = ARG32;
      UInt32 l_numBytesWrittenP = 0;
      Err res = VFSFileWrite(fileRef ? l_fileRef : 0, numBytes, dataP ? s_dataP : NULL, numBytesWrittenP ? &l_numBytesWrittenP : NULL);
      if (numBytesWrittenP) m68k_write_memory_32(numBytesWrittenP, l_numBytesWrittenP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileWrite(fileRef=%d, numBytes=%d, dataP=0x%08X, numBytesWrittenP=0x%08X [%d]): %d", fileRef, numBytes, dataP, numBytesWrittenP, l_numBytesWrittenP, res);
    }
    break;
    case vfsTrapFileDelete: {
      // Err VFSFileDelete(UInt16 volRefNum, in Char *pathNameP);
      uint16_t volRefNum = ARG16;
      uint32_t pathNameP = ARG32;
      char *s_pathNameP = emupalmos_trap_sel_in(pathNameP, sysTrapFileSystemDispatch, sel, 1);
      Err res = VFSFileDelete(volRefNum, pathNameP ? s_pathNameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileDelete(volRefNum=%d, pathNameP=0x%08X [%s]): %d", volRefNum, pathNameP, s_pathNameP, res);
    }
    break;
    case vfsTrapFileRename: {
      // Err VFSFileRename(UInt16 volRefNum, in Char *pathNameP, in Char *newNameP);
      uint16_t volRefNum = ARG16;
      uint32_t pathNameP = ARG32;
      char *s_pathNameP = emupalmos_trap_sel_in(pathNameP, sysTrapFileSystemDispatch, sel, 1);
      uint32_t newNameP = ARG32;
      char *s_newNameP = emupalmos_trap_sel_in(newNameP, sysTrapFileSystemDispatch, sel, 2);
      Err res = VFSFileRename(volRefNum, pathNameP ? s_pathNameP : NULL, newNameP ? s_newNameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileRename(volRefNum=%d, pathNameP=0x%08X [%s], newNameP=0x%08X [%s]): %d", volRefNum, pathNameP, s_pathNameP, newNameP, s_newNameP, res);
    }
    break;
    case vfsTrapFileSeek: {
      // Err VFSFileSeek(FileRef fileRef, FileOrigin origin, Int32 offset);
      uint32_t fileRef = ARG32;
      FileRefProxy *ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      uint16_t origin = ARG16;
      int32_t offset = ARG32;
      Err res = VFSFileSeek(fileRef ? l_fileRef : 0, origin, offset);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileSeek(fileRef=%d, origin=%d, offset=%d): %d", fileRef, origin, offset, res);
    }
    break;
    case vfsTrapFileEOF: {
      // Err VFSFileEOF(FileRef fileRef);
      uint32_t fileRef = ARG32;
      FileRefProxy *ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      Err res = VFSFileEOF(fileRef ? l_fileRef : 0);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileEOF(fileRef=%d): %d", fileRef, res);
    }
    break;
    case vfsTrapFileTell: {
      // Err VFSFileTell(FileRef fileRef, out UInt32 *filePosP);
      uint32_t fileRef = ARG32;
      FileRefProxy *ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      uint32_t filePosP = ARG32;
      UInt32 l_filePosP = 0;
      Err res = VFSFileTell(fileRef ? l_fileRef : 0, filePosP ? &l_filePosP : NULL);
      if (filePosP) m68k_write_memory_32(filePosP, l_filePosP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileTell(fileRef=%d, filePosP=0x%08X [%d]): %d", fileRef, filePosP, l_filePosP, res);
    }
    break;
    case vfsTrapFileSize: {
      // Err VFSFileSize(FileRef fileRef, out UInt32 *fileSizeP);
      uint32_t fileRef = ARG32;
      FileRefProxy *ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      uint32_t fileSizeP = ARG32;
      UInt32 l_fileSizeP = 0;
      Err res = VFSFileSize(fileRef ? l_fileRef : 0, fileSizeP ? &l_fileSizeP : NULL);
      if (fileSizeP) m68k_write_memory_32(fileSizeP, l_fileSizeP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileSize(fileRef=%d, fileSizeP=0x%08X [%d]): %d", fileRef, fileSizeP, l_fileSizeP, res);
    }
    break;
    case vfsTrapFileResize: {
      // Err VFSFileResize(FileRef fileRef, UInt32 newSize);
      uint32_t fileRef = ARG32;
      FileRefProxy *ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      uint32_t newSize = ARG32;
      Err res = VFSFileResize(fileRef ? l_fileRef : 0, newSize);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileResize(fileRef=%d, newSize=%d): %d", fileRef, newSize, res);
    }
    break;
    case vfsTrapFileGetAttributes: {
      // Err VFSFileGetAttributes(FileRef fileRef, out UInt32 *attributesP);
      uint32_t fileRef = ARG32;
      FileRefProxy *ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      uint32_t attributesP = ARG32;
      UInt32 l_attributesP = 0;
      Err res = VFSFileGetAttributes(fileRef ? l_fileRef : 0, attributesP ? &l_attributesP : NULL);
      if (attributesP) m68k_write_memory_32(attributesP, l_attributesP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileGetAttributes(fileRef=%d, attributesP=0x%08X [%d]): %d", fileRef, attributesP, l_attributesP, res);
    }
    break;
    case vfsTrapFileSetAttributes: {
      // Err VFSFileSetAttributes(FileRef fileRef, UInt32 attributes);
      uint32_t fileRef = ARG32;
      FileRefProxy *ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      uint32_t attributes = ARG32;
      Err res = VFSFileSetAttributes(fileRef ? l_fileRef : 0, attributes);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileSetAttributes(fileRef=%d, attributes=%d): %d", fileRef, attributes, res);
    }
    break;
    case vfsTrapFileGetDate: {
      // Err VFSFileGetDate(FileRef fileRef, UInt16 whichDate, out UInt32 *dateP);
      uint32_t fileRef = ARG32;
      FileRefProxy *ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      uint16_t whichDate = ARG16;
      uint32_t dateP = ARG32;
      UInt32 l_dateP = 0;
      Err res = VFSFileGetDate(fileRef ? l_fileRef : 0, whichDate, dateP ? &l_dateP : NULL);
      if (dateP) m68k_write_memory_32(dateP, l_dateP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileGetDate(fileRef=%d, whichDate=%d, dateP=0x%08X [%d]): %d", fileRef, whichDate, dateP, l_dateP, res);
    }
    break;
    case vfsTrapFileSetDate: {
      // Err VFSFileSetDate(FileRef fileRef, UInt16 whichDate, UInt32 date);
      uint32_t fileRef = ARG32;
      FileRefProxy *ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      uint16_t whichDate = ARG16;
      uint32_t date = ARG32;
      Err res = VFSFileSetDate(fileRef ? l_fileRef : 0, whichDate, date);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileSetDate(fileRef=%d, whichDate=%d, date=%d): %d", fileRef, whichDate, date, res);
    }
    break;
    case vfsTrapDirCreate: {
      // Err VFSDirCreate(UInt16 volRefNum, in Char *dirNameP);
      uint16_t volRefNum = ARG16;
      uint32_t dirNameP = ARG32;
      char *s_dirNameP = emupalmos_trap_sel_in(dirNameP, sysTrapFileSystemDispatch, sel, 1);
      Err res = VFSDirCreate(volRefNum, dirNameP ? s_dirNameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSDirCreate(volRefNum=%d, dirNameP=0x%08X [%s]): %d", volRefNum, dirNameP, s_dirNameP, res);
    }
    break;
    case vfsTrapDirEntryEnumerate: {
      // Err VFSDirEntryEnumerate(FileRef dirRef, inout UInt32 *dirEntryIteratorP, inout FileInfoType *infoP);
      uint32_t dirRef = ARG32;
      FileRefProxy *ll_dirRef = (FileRefProxy *)emupalmos_trap_sel_in(dirRef, sysTrapFileSystemDispatch, sel, 0);
      FileRef l_dirRef = ll_dirRef ? ll_dirRef->ref : NULL;
      uint32_t dirEntryIteratorP = ARG32;
      UInt32 l_dirEntryIteratorP;
      if (dirEntryIteratorP) l_dirEntryIteratorP = m68k_read_memory_32(dirEntryIteratorP);
      uint32_t infoP = ARG32;
      FileInfoType l_infoP;
      decode_FileInfoType(infoP, &l_infoP);
      Err res = VFSDirEntryEnumerate(dirRef ? l_dirRef : 0, dirEntryIteratorP ? &l_dirEntryIteratorP : NULL, infoP ? &l_infoP : NULL);
      if (dirEntryIteratorP) m68k_write_memory_32(dirEntryIteratorP, l_dirEntryIteratorP);
      encode_FileInfoType(infoP, &l_infoP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSDirEntryEnumerate(dirRef=%d, dirEntryIteratorP=0x%08X [%d], infoP=0x%08X): %d", dirRef, dirEntryIteratorP, l_dirEntryIteratorP, infoP, res);
    }
    break;
    case vfsTrapGetDefaultDirectory: {
      // Err VFSGetDefaultDirectory(UInt16 volRefNum, in Char *fileTypeStr, out Char *pathStr, inout UInt16 *bufLenP);
      uint16_t volRefNum = ARG16;
      uint32_t fileTypeStr = ARG32;
      char *s_fileTypeStr = emupalmos_trap_sel_in(fileTypeStr, sysTrapFileSystemDispatch, sel, 1);
      uint32_t pathStr = ARG32;
      char *s_pathStr = emupalmos_trap_sel_in(pathStr, sysTrapFileSystemDispatch, sel, 2);
      uint32_t bufLenP = ARG32;
      UInt16 l_bufLenP;
      if (bufLenP) l_bufLenP = m68k_read_memory_16(bufLenP);
      Err res = VFSGetDefaultDirectory(volRefNum, fileTypeStr ? s_fileTypeStr : NULL, pathStr ? s_pathStr : NULL, bufLenP ? &l_bufLenP : NULL);
      if (bufLenP) m68k_write_memory_16(bufLenP, l_bufLenP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSGetDefaultDirectory(volRefNum=%d, fileTypeStr=0x%08X [%s], pathStr=0x%08X [%s], bufLenP=0x%08X [%d]): %d", volRefNum, fileTypeStr, s_fileTypeStr, pathStr, s_pathStr, bufLenP, l_bufLenP, res);
    }
    break;
    case vfsTrapRegisterDefaultDirectory: {
      // Err VFSRegisterDefaultDirectory(in Char *fileTypeStr, UInt32 mediaType, in Char *pathStr);
      uint32_t fileTypeStr = ARG32;
      char *s_fileTypeStr = emupalmos_trap_sel_in(fileTypeStr, sysTrapFileSystemDispatch, sel, 0);
      uint32_t mediaType = ARG32;
      uint32_t pathStr = ARG32;
      char *s_pathStr = emupalmos_trap_sel_in(pathStr, sysTrapFileSystemDispatch, sel, 2);
      Err res = VFSRegisterDefaultDirectory(fileTypeStr ? s_fileTypeStr : NULL, mediaType, pathStr ? s_pathStr : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSRegisterDefaultDirectory(fileTypeStr=0x%08X [%s], mediaType=%d, pathStr=0x%08X [%s]): %d", fileTypeStr, s_fileTypeStr, mediaType, pathStr, s_pathStr, res);
    }
    break;
    case vfsTrapUnregisterDefaultDirectory: {
      // Err VFSUnregisterDefaultDirectory(in Char *fileTypeStr, UInt32 mediaType);
      uint32_t fileTypeStr = ARG32;
      char *s_fileTypeStr = emupalmos_trap_sel_in(fileTypeStr, sysTrapFileSystemDispatch, sel, 0);
      uint32_t mediaType = ARG32;
      Err res = VFSUnregisterDefaultDirectory(fileTypeStr ? s_fileTypeStr : NULL, mediaType);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSUnregisterDefaultDirectory(fileTypeStr=0x%08X [%s], mediaType=%d): %d", fileTypeStr, s_fileTypeStr, mediaType, res);
    }
    break;
    case vfsTrapVolumeUnmount: {
      // Err VFSVolumeUnmount(UInt16 volRefNum);
      uint16_t volRefNum = ARG16;
      Err res = VFSVolumeUnmount(volRefNum);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSVolumeUnmount(volRefNum=%d): %d", volRefNum, res);
    }
    break;
    case vfsTrapVolumeEnumerate: {
      // Err VFSVolumeEnumerate(out UInt16 *volRefNumP, inout UInt32 *volIteratorP);
      uint32_t volRefNumP = ARG32;
      UInt16 l_volRefNumP = 0;
      uint32_t volIteratorP = ARG32;
      UInt32 l_volIteratorP = 0;
      if (volIteratorP) l_volIteratorP = m68k_read_memory_32(volIteratorP);
      Err res = VFSVolumeEnumerate(volRefNumP ? &l_volRefNumP : NULL, volIteratorP ? &l_volIteratorP : NULL);
      if (volRefNumP) m68k_write_memory_16(volRefNumP, l_volRefNumP);
      if (volIteratorP) m68k_write_memory_32(volIteratorP, l_volIteratorP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSVolumeEnumerate(volRefNumP=0x%08X [%d], volIteratorP=0x%08X [%d]): %d", volRefNumP, l_volRefNumP, volIteratorP, l_volIteratorP, res);
    }
    break;
    case vfsTrapVolumeInfo: {
      // Err VFSVolumeInfo(UInt16 volRefNum, out VolumeInfoType *volInfoP);
      uint16_t volRefNum = ARG16;
      uint32_t volInfoP = ARG32;
      VolumeInfoType l_volInfoP;
      Err res = VFSVolumeInfo(volRefNum, volInfoP ? &l_volInfoP : NULL);
      encode_VolumeInfoType(volInfoP, &l_volInfoP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSVolumeInfo(volRefNum=%d, volInfoP=0x%08X): %d", volRefNum, volInfoP, res);
    }
    break;
    case vfsTrapVolumeGetLabel: {
      // Err VFSVolumeGetLabel(UInt16 volRefNum, out Char *labelP, UInt16 bufLen);
      uint16_t volRefNum = ARG16;
      uint32_t labelP = ARG32;
      char *s_labelP = emupalmos_trap_sel_in(labelP, sysTrapFileSystemDispatch, sel, 1);
      uint16_t bufLen = ARG16;
      Err res = VFSVolumeGetLabel(volRefNum, labelP ? s_labelP : NULL, bufLen);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSVolumeGetLabel(volRefNum=%d, labelP=0x%08X [%s], bufLen=%d): %d", volRefNum, labelP, s_labelP, bufLen, res);
    }
    break;
    case vfsTrapVolumeSetLabel: {
      // Err VFSVolumeSetLabel(UInt16 volRefNum, in Char *labelP);
      uint16_t volRefNum = ARG16;
      uint32_t labelP = ARG32;
      char *s_labelP = emupalmos_trap_sel_in(labelP, sysTrapFileSystemDispatch, sel, 1);
      Err res = VFSVolumeSetLabel(volRefNum, labelP ? s_labelP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSVolumeSetLabel(volRefNum=%d, labelP=0x%08X [%s]): %d", volRefNum, labelP, s_labelP, res);
    }
    break;
    case vfsTrapVolumeSize: {
      // Err VFSVolumeSize(UInt16 volRefNum, out UInt32 *volumeUsedP, out UInt32 *volumeTotalP);
      uint16_t volRefNum = ARG16;
      uint32_t volumeUsedP = ARG32;
      UInt32 l_volumeUsedP = 0;
      uint32_t volumeTotalP = ARG32;
      UInt32 l_volumeTotalP = 0;
      Err res = VFSVolumeSize(volRefNum, volumeUsedP ? &l_volumeUsedP : NULL, volumeTotalP ? &l_volumeTotalP : NULL);
      if (volumeUsedP) m68k_write_memory_32(volumeUsedP, l_volumeUsedP);
      if (volumeTotalP) m68k_write_memory_32(volumeTotalP, l_volumeTotalP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSVolumeSize(volRefNum=%d, volumeUsedP=0x%08X [%d], volumeTotalP=0x%08X [%d]): %d", volRefNum, volumeUsedP, l_volumeUsedP, volumeTotalP, l_volumeTotalP, res);
    }
    break;
    case vfsTrapInstallFSLib: {
      // Err VFSInstallFSLib(UInt32 creator, out UInt16 *fsLibRefNumP);
      uint32_t creator = ARG32;
      uint32_t fsLibRefNumP = ARG32;
      UInt16 l_fsLibRefNumP = 0;
      Err res = VFSInstallFSLib(creator, fsLibRefNumP ? &l_fsLibRefNumP : NULL);
      if (fsLibRefNumP) m68k_write_memory_16(fsLibRefNumP, l_fsLibRefNumP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSInstallFSLib(creator=%d, fsLibRefNumP=0x%08X [%d]): %d", creator, fsLibRefNumP, l_fsLibRefNumP, res);
    }
    break;
    case vfsTrapRemoveFSLib: {
      // Err VFSRemoveFSLib(UInt16 fsLibRefNum);
      uint16_t fsLibRefNum = ARG16;
      Err res = VFSRemoveFSLib(fsLibRefNum);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSRemoveFSLib(fsLibRefNum=%d): %d", fsLibRefNum, res);
    }
    break;
    case vfsTrapFileDBInfo: {
      // Err VFSFileDBInfo(FileRef ref, Char *nameP,
      //    UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP,
      //    UInt32 *modDateP, UInt32 *bckUpDateP,
      //    UInt32 *modNumP, MemHandle *appInfoHP,
      //    MemHandle *sortInfoHP, UInt32 *typeP,
      //    UInt32 *creatorP, UInt16 *numRecordsP)
      uint32_t refP = ARG32;
      uint32_t nameP = ARG32;
      uint32_t attributesP = ARG32;
      uint32_t versionP = ARG32;
      uint32_t crDateP = ARG32;
      uint32_t modDateP = ARG32;
      uint32_t bckUpDateP = ARG32;
      uint32_t modNumP = ARG32;
      uint32_t appInfoHP = ARG32;
      uint32_t sortInfoHP = ARG32;
      uint32_t typeP = ARG32;
      uint32_t creatorP = ARG32;
      uint32_t numRecordsP = ARG32;
      char name[dmDBNameLength];
      FileRefProxy *ref = (FileRefProxy *)emupalmos_trap_sel_in(refP, sysTrapFileSystemDispatch, sel, 0);
      FileRef fileRef = ref ? ref->ref : NULL;
      UInt16 attributes, version, numRecords;
      UInt32 crDate, modDate, bckUpDate, modNum, type, creator;
      MemHandle appInfoH, sortInfoH;
      Err res = VFSFileDBInfo(fileRef, name, &attributes, &version, &crDate, &modDate, &bckUpDate, &modNum, &appInfoH, &sortInfoH, &type, &creator, &numRecords);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileDBInfo(fileRef=%d): %d", refP, res);
    }
    break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "FileSystemDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
