#include <PalmOS.h>

#include "debug.h"

Int32 HostGetHostVersion(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostGetHostVersion not implemented");
  return 0;
}

HostIDType HostGetHostID(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostGetHostID not implemented");
  return 0;
}

HostPlatformType HostGetHostPlatform(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostGetHostPlatform not implemented");
  return 0;
}

HostBoolType HostIsSelectorImplemented(long selector) {
  debug(DEBUG_ERROR, "PALMOS", "HostIsSelectorImplemented not implemented");
  return 0;
}

HostErrType HostGestalt(long gestSel, long* response) {
  debug(DEBUG_ERROR, "PALMOS", "HostGestalt not implemented");
  return 0;
}

HostBoolType HostIsCallingTrap(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostIsCallingTrap not implemented");
  return 0;
}

HostErrType HostProfileInit(long maxCalls, long maxDepth) {
  debug(DEBUG_ERROR, "PALMOS", "HostProfileInit not implemented");
  return 0;
}

HostErrType HostProfileDetailFn(void* addr, HostBoolType logDetails) {
  debug(DEBUG_ERROR, "PALMOS", "HostProfileDetailFn not implemented");
  return 0;
}

HostErrType HostProfileStart(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostProfileStart not implemented");
  return 0;
}

HostErrType HostProfileStop(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostProfileStop not implemented");
  return 0;
}

HostErrType HostProfileDump(const char* filenameP) {
  debug(DEBUG_ERROR, "PALMOS", "HostProfileDump not implemented");
  return 0;
}

HostErrType HostProfileCleanup(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostProfileCleanup not implemented");
  return 0;
}

long HostProfileGetCycles(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostProfileGetCycles not implemented");
  return 0;
}

long HostErrNo(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostErrNo not implemented");
  return 0;
}

long HostFClose(HostFILEType* fileP) {
  debug(DEBUG_ERROR, "PALMOS", "HostFClose not implemented");
  return 0;
}

long HostFEOF(HostFILEType* fileP) {
  debug(DEBUG_ERROR, "PALMOS", "HostFEOF not implemented");
  return 0;
}

long HostFError(HostFILEType* fileP) {
  debug(DEBUG_ERROR, "PALMOS", "HostFError not implemented");
  return 0;
}

long HostFFlush(HostFILEType* fileP) {
  debug(DEBUG_ERROR, "PALMOS", "HostFFlush not implemented");
  return 0;
}

long HostFGetC(HostFILEType* fileP) {
  debug(DEBUG_ERROR, "PALMOS", "HostFGetC not implemented");
  return 0;
}

long HostFGetPos(HostFILEType* fileP, long* posP) {
  debug(DEBUG_ERROR, "PALMOS", "HostFGetPos not implemented");
  return 0;
}

char* HostFGetS(char* s, long n, HostFILEType* fileP) {
  debug(DEBUG_ERROR, "PALMOS", "HostFGetS not implemented");
  return 0;
}

HostFILEType * HostFOpen(const char* name, const char* mode) {
  debug(DEBUG_ERROR, "PALMOS", "HostFOpen not implemented");
  return 0;
}

long HostFPrintF(HostFILEType* fileP, const char* fmt, ...) {
  debug(DEBUG_ERROR, "PALMOS", "HostFPrintF not implemented");
  return 0;
}

long HostFPutC(long c, HostFILEType* fileP) {
  debug(DEBUG_ERROR, "PALMOS", "HostFPutC not implemented");
  return 0;
}

long HostFPutS(const char* s, HostFILEType* fileP) {
  debug(DEBUG_ERROR, "PALMOS", "HostFPutS not implemented");
  return 0;
}

long HostFRead(void* buffer, long size, long count, HostFILEType* fileP) {
  debug(DEBUG_ERROR, "PALMOS", "HostFRead not implemented");
  return 0;
}

HostFILEType *HostFReopen(const char* name, const char* mode, HostFILEType* fileP) {
  debug(DEBUG_ERROR, "PALMOS", "HostFReopen not implemented");
  return 0;
}

long HostFScanF(HostFILEType* fileP, const char* fmt, ...) {
  debug(DEBUG_ERROR, "PALMOS", "HostFScanF not implemented");
  return 0;
}

long HostFSeek(HostFILEType* fileP, long offset, long origin) {
  debug(DEBUG_ERROR, "PALMOS", "HostFSeek not implemented");
  return 0;
}

long HostFSetPos(HostFILEType* fileP, long* pos) {
  debug(DEBUG_ERROR, "PALMOS", "HostFSetPos not implemented");
  return 0;
}

long HostFTell(HostFILEType* fileP) {
  debug(DEBUG_ERROR, "PALMOS", "HostFTell not implemented");
  return 0;
}

long HostFWrite(const void* buffer, long size, long count, HostFILEType* fileP) {
  debug(DEBUG_ERROR, "PALMOS", "HostFWrite not implemented");
  return 0;
}

long HostRemove(const char* nameP) {
  debug(DEBUG_ERROR, "PALMOS", "HostRemove not implemented");
  return 0;
}

long HostRename(const char* oldNameP, const char* newNameP) {
  debug(DEBUG_ERROR, "PALMOS", "HostRename not implemented");
  return 0;
}

HostFILEType* HostTmpFile(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostTmpFile not implemented");
  return 0;
}

char* HostTmpNam(char* nameP) {
  debug(DEBUG_ERROR, "PALMOS", "HostTmpNam not implemented");
  return 0;
}

char* HostGetEnv(const char* nameP) {
  debug(DEBUG_ERROR, "PALMOS", "HostGetEnv not implemented");
  return 0;
}

void* HostMalloc(long size) {
  debug(DEBUG_ERROR, "PALMOS", "HostMalloc not implemented");
  return 0;
}

void* HostRealloc(void* p, long size) {
  debug(DEBUG_ERROR, "PALMOS", "HostRealloc not implemented");
  return 0;
}

void HostFree(void* p) {
  debug(DEBUG_ERROR, "PALMOS", "HostFree not implemented");
}

char* HostAscTime(const HostTmType *p) {
  debug(DEBUG_ERROR, "PALMOS", "HostAscTime not implemented");
  return 0;
}

HostClockType HostClock(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostClock not implemented");
  return 0;
}

char* HostCTime(const HostTimeType *p) {
  debug(DEBUG_ERROR, "PALMOS", "HostCTime not implemented");
  return 0;
}

HostTmType* HostGMTime(const HostTimeType *p) {
  debug(DEBUG_ERROR, "PALMOS", "HostGMTime not implemented");
  return 0;
}

HostTmType* HostLocalTime(const HostTimeType *p) {
  debug(DEBUG_ERROR, "PALMOS", "HostLocalTime not implemented");
  return 0;
}

HostTimeType HostMkTime(HostTmType *p) {
  debug(DEBUG_ERROR, "PALMOS", "HostMkTime not implemented");
  return 0;
}

HostSizeType HostStrFTime(char *s, HostSizeType h, const char *s2, const HostTmType *p) {
  debug(DEBUG_ERROR, "PALMOS", "HostStrFTime not implemented");
  return 0;
}

HostTimeType HostTime(HostTimeType *p) {
  debug(DEBUG_ERROR, "PALMOS", "HostTime not implemented");
  return 0;
}

long HostMkDir(const char *s) {
  debug(DEBUG_ERROR, "PALMOS", "HostMkDir not implemented");
  return 0;
}

long HostRmDir(const char *s) {
  debug(DEBUG_ERROR, "PALMOS", "HostRmDir not implemented");
  return 0;
}

HostDIRType* HostOpenDir(const char *s) {
  debug(DEBUG_ERROR, "PALMOS", "HostOpenDir not implemented");
  return 0;
}

HostDirEntType* HostReadDir(HostDIRType *p) {
  debug(DEBUG_ERROR, "PALMOS", "HostReadDir not implemented");
  return 0;
}

long HostCloseDir(HostDIRType *p) {
  debug(DEBUG_ERROR, "PALMOS", "HostCloseDir not implemented");
  return 0;
}

long HostStat(const char *s, HostStatType *p) {
  debug(DEBUG_ERROR, "PALMOS", "HostStat not implemented");
  return 0;
}

long HostTruncate(const char *s, long l) {
  debug(DEBUG_ERROR, "PALMOS", "HostTruncate not implemented");
  return 0;
}

long HostUTime(const char *s, HostUTimeType *p) {
  debug(DEBUG_ERROR, "PALMOS", "HostUTime not implemented");
  return 0;
}

long HostGetFileAttr(const char *s, long *l) {
  debug(DEBUG_ERROR, "PALMOS", "HostGetFileAttr not implemented");
  return 0;
}

long HostSetFileAttr(const char *s, long l) {
  debug(DEBUG_ERROR, "PALMOS", "HostSetFileAttr not implemented");
  return 0;
}

HostBoolType HostGremlinIsRunning(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostGremlinIsRunning not implemented");
  return 0;
}

long HostGremlinNumber(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostGremlinNumber not implemented");
  return 0;
}

long HostGremlinCounter(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostGremlinCounter not implemented");
  return 0;
}

long HostGremlinLimit(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostGremlinLimit not implemented");
  return 0;
}

HostErrType HostGremlinNew(const HostGremlinInfoType *p) {
  debug(DEBUG_ERROR, "PALMOS", "HostGremlinNew not implemented");
  return 0;
}

HostErrType HostImportFile(const char* fileName, long cardNum) {
  debug(DEBUG_ERROR, "PALMOS", "HostImportFile not implemented");
  return 0;
}

HostErrType HostImportFileWithID(const char* fileName, long cardNum, LocalID* newIDP) {
  debug(DEBUG_ERROR, "PALMOS", "HostImportFileWithID not implemented");
  return 0;
}

HostErrType HostExportFile(const char* fileName, long cardNum, const char* dbName) {
  debug(DEBUG_ERROR, "PALMOS", "HostExportFile not implemented");
  return 0;
}

HostErrType HostSaveScreen(const char* fileName) {
  debug(DEBUG_ERROR, "PALMOS", "HostSaveScreen not implemented");
  return 0;
}

Err HostExgLibOpen(UInt16 libRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibOpen not implemented");
  return 0;
}

Err HostExgLibClose(UInt16 libRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibClose not implemented");
  return 0;
}

Err HostExgLibSleep(UInt16 libRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibSleep not implemented");
  return 0;
}

Err HostExgLibWake(UInt16 libRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibWake not implemented");
  return 0;
}

Boolean HostExgLibHandleEvent(UInt16 libRefNum, void* eventP) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibHandleEvent not implemented");
  return 0;
}

Err  HostExgLibConnect(UInt16 libRefNum, void* exgSocketP) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibConnect not implemented");
  return 0;
}

Err HostExgLibAccept(UInt16 libRefNum, void* exgSocketP) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibAccept not implemented");
  return 0;
}

Err HostExgLibDisconnect(UInt16 libRefNum, void* exgSocketP,Err error) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibDisconnect not implemented");
  return 0;
}

Err HostExgLibPut(UInt16 libRefNum, void* exgSocketP) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibPut not implemented");
  return 0;
}

Err HostExgLibGet(UInt16 libRefNum, void* exgSocketP) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibGet not implemented");
  return 0;
}

UInt32  HostExgLibSend(UInt16 libRefNum, void* exgSocketP, const void* const bufP, const UInt32 bufLen, Err* errP) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibSend not implemented");
  return 0;
}

UInt32  HostExgLibReceive(UInt16 libRefNum, void* exgSocketP, void* bufP, const UInt32 bufSize, Err* errP) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibReceive not implemented");
  return 0;
}

Err  HostExgLibControl(UInt16 libRefNum, UInt16 op, void* valueP, UInt16* valueLenP) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibControl not implemented");
  return 0;
}

Err  HostExgLibRequest(UInt16 libRefNum, void* exgSocketP) {
  debug(DEBUG_ERROR, "PALMOS", "HostExgLibRequest not implemented");
  return 0;
}

HostBoolType HostGetPreference(const char *s, char *s2) {
  debug(DEBUG_ERROR, "PALMOS", "HostGetPreference not implemented");
  return 0;
}

void HostSetPreference(const char *s, const char *s2) {
  debug(DEBUG_ERROR, "PALMOS", "HostSetPreference not implemented");
}

HostFILEType* HostLogFile(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostLogFile not implemented");
  return 0;
}

void HostSetLogFileSize(long l) {
  debug(DEBUG_ERROR, "PALMOS", "HostSetLogFileSize not implemented");
}

HostErrType HostSessionCreate(const char* device, long ramSize, const char* romPath) {
  debug(DEBUG_ERROR, "PALMOS", "HostSessionCreate not implemented");
  return 0;
}

HostErrType HostSessionOpen(const char* psfFileName) {
  debug(DEBUG_ERROR, "PALMOS", "HostSessionOpen not implemented");
  return 0;
}

HostBoolType HostSessionSave(const char* saveFileName) {
  debug(DEBUG_ERROR, "PALMOS", "HostSessionSave not implemented");
  return 0;
}

HostErrType HostSessionClose(const char* saveFileName) {
  debug(DEBUG_ERROR, "PALMOS", "HostSessionClose not implemented");
  return 0;
}

HostErrType HostSessionQuit(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostSessionQuit not implemented");
  return 0;
}

HostErrType HostSignalSend(HostSignalType signalNumber) {
  debug(DEBUG_ERROR, "PALMOS", "HostSignalSend not implemented");
  return 0;
}

HostErrType HostSignalWait(long timeout, HostSignalType* signalNumber) {
  debug(DEBUG_ERROR, "PALMOS", "HostSignalWait not implemented");
  return 0;
}

HostErrType HostSignalResume(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostSignalResume not implemented");
  return 0;
}

void HostTraceInit(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostTraceInit not implemented");
}

void HostTraceClose(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostTraceClose not implemented");
}

void HostTraceOutputT(unsigned short p, const char *s, ...) {
  debug(DEBUG_ERROR, "PALMOS", "HostTraceOutputT not implemented");
}

void HostTraceOutputTL(unsigned short p, const char *s, ...) {
  debug(DEBUG_ERROR, "PALMOS", "HostTraceOutputTL not implemented");
}

void HostTraceOutputVT(unsigned short p, const char *s, char *s2 /*va_list*/) {
  debug(DEBUG_ERROR, "PALMOS", "HostTraceOutputVT not implemented");
}

void HostTraceOutputVTL(unsigned short p, const char *s, char *s2 /*va_list*/) {
  debug(DEBUG_ERROR, "PALMOS", "HostTraceOutputVTL not implemented");
}

void HostTraceOutputB(unsigned short p, const void *p2, HostSizeType p3) {
  debug(DEBUG_ERROR, "PALMOS", "HostTraceOutputB not implemented");
}

HostErr HostDbgSetDataBreak(UInt32 addr, UInt32 size) {
  debug(DEBUG_ERROR, "PALMOS", "HostDbgSetDataBreak not implemented");
  return 0;
}

HostErr HostDbgClearDataBreak(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostDbgClearDataBreak not implemented");
  return 0;
}

const char *HostSlotRoot(long slotNo) {
  debug(DEBUG_ERROR, "PALMOS", "HostSlotRoot not implemented");
  return NULL;
}

long HostSlotMax(void) {
  debug(DEBUG_ERROR, "PALMOS", "HostSlotMax not implemented");
  return 0;
}

HostBoolType HostSlotHasCard(long slotNo) {
  debug(DEBUG_ERROR, "PALMOS", "HostSlotHasCard not implemented");
  return 0;
}

const char *HostPutFile(const char *prompt, const char *defaultDir, const char *defaultName) {
  debug(DEBUG_ERROR, "PALMOS", "HostPutFile not implemented");
  return NULL;
}

const char *HostGetFile(const char *prompt, const char *defaultDir) {
  debug(DEBUG_ERROR, "PALMOS", "HostGetFile not implemented");
  return NULL;
}

const char *HostGetDirectory(const char *prompt, const char *defaultDir) {
  debug(DEBUG_ERROR, "PALMOS", "HostGetDirectory not implemented");
  return NULL;
}
