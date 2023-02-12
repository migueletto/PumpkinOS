#include <PalmOS.h>

#include "debug.h"

Err GrfInit(void) {
  debug(DEBUG_ERROR, "PALMOS", "GrfInit not implemented");
  return sysErrParamErr;
}

Err GrfFree(void) {
  debug(DEBUG_ERROR, "PALMOS", "GrfFree not implemented");
  return sysErrParamErr;
}

Err GrfBeginStroke(const PointType *startPtP, const RectangleType *boundsP, Boolean liveInk) {
  debug(DEBUG_ERROR, "PALMOS", "GrfBeginStroke not implemented");
  return sysErrParamErr;
}

Err GrfProcessStroke(const PointType *startPtP, const PointType *endPtP, Boolean upShift) {
  debug(DEBUG_ERROR, "PALMOS", "GrfProcessStroke not implemented");
  return sysErrParamErr;
}

Err GrfFieldChange(Boolean resetState, UInt16 *characterToDelete) {
  debug(DEBUG_ERROR, "PALMOS", "GrfFieldChange not implemented");
  return sysErrParamErr;
}

Err GrfGetState(Boolean *capsLockP, Boolean *numLockP, UInt16 *tempShiftP, Boolean *autoShiftedP) {
  debug(DEBUG_ERROR, "PALMOS", "GrfGetState not implemented");
  return sysErrParamErr;
}

Err GrfSetState(Boolean capsLock, Boolean numLock, Boolean upperShift) {
  debug(DEBUG_ERROR, "PALMOS", "GrfSetState not implemented");
  return sysErrParamErr;
}

Err GrfFlushPoints(void) {
  debug(DEBUG_ERROR, "PALMOS", "GrfFlushPoints not implemented");
  return sysErrParamErr;
}

Err GrfAddPoint(PointType *pt) {
  debug(DEBUG_ERROR, "PALMOS", "GrfAddPoint not implemented");
  return sysErrParamErr;
}

Err GrfInitState(void) {
  debug(DEBUG_ERROR, "PALMOS", "GrfInitState not implemented");
  return sysErrParamErr;
}

Err GrfCleanState(void) {
  debug(DEBUG_ERROR, "PALMOS", "GrfCleanState not implemented");
  return sysErrParamErr;
}

Err GrfFilterPoints(void) {
  debug(DEBUG_ERROR, "PALMOS", "GrfFilterPoints not implemented");
  return sysErrParamErr;
}

Err GrfGetNumPoints(UInt16 *numPtsP) {
  debug(DEBUG_ERROR, "PALMOS", "GrfGetNumPoints not implemented");
  return sysErrParamErr;
}

Err GrfGetPoint(UInt16 index, PointType *pointP) {
  debug(DEBUG_ERROR, "PALMOS", "GrfGetPoint not implemented");
  return sysErrParamErr;
}

Err GrfFindBranch(UInt16 flags) {
  debug(DEBUG_ERROR, "PALMOS", "GrfFindBranch not implemented");
  return sysErrParamErr;
}

Err GrfGetMacroName(UInt16 index, Char *nameP) {
  debug(DEBUG_ERROR, "PALMOS", "GrfGetMacroName not implemented");
  return sysErrParamErr;
}

Err GrfDeleteMacro(UInt16 index) {
  debug(DEBUG_ERROR, "PALMOS", "GrfDeleteMacro not implemented");
  return sysErrParamErr;
}

Err GrfMatch(UInt16 *flagsP, void *dataPtrP, UInt16 *dataLenP, UInt16 *uncertainLenP, GrfMatchInfoPtr matchInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "GrfMatch not implemented");
  return sysErrParamErr;
}

Err GrfGetMacro(Char *nameP, UInt8 *macroDataP, UInt16 *dataLenP) {
  debug(DEBUG_ERROR, "PALMOS", "GrfGetMacro not implemented");
  return sysErrParamErr;
}

Err GrfGetAndExpandMacro(Char *nameP, UInt8 *macroDataP, UInt16 *dataLenP) {
  debug(DEBUG_ERROR, "PALMOS", "GrfGetAndExpandMacro not implemented");
  return sysErrParamErr;
}

Err GrfMatchGlyph(GrfMatchInfoPtr matchInfoP, Int16 maxUnCertainty, UInt16 maxMatches) {
  debug(DEBUG_ERROR, "PALMOS", "GrfMatchGlyph not implemented");
  return sysErrParamErr;
}


Err GrfGetGlyphMapping(UInt16 glyphID, UInt16 *flagsP, void *dataPtrP, UInt16 *dataLenP, UInt16 *uncertainLenP) {
  debug(DEBUG_ERROR, "PALMOS", "GrfGetGlyphMapping not implemented");
  return sysErrParamErr;
}

Err GrfAddMacro(const Char *nameP, UInt8 *macroDataP, UInt16 dataLen) {
  debug(DEBUG_ERROR, "PALMOS", "GrfAddMacro not implemented");
  return sysErrParamErr;
}
