#include <PalmOS.h>
#include <BtLib.h>
#include <CPMLib.h>
#include <DLServer.h>
#include <ExpansionMgr.h>
#include <FSLib.h>
#include <INetMgr.h>
#include <PceNativeCall.h>
#include <PdiLib.h>
#include <SerialMgrOld.h>
#include <SerialSdrv.h>
#include <SerialVdrv.h>
#include <SlotDrvrLib.h>
#include <SslLib.h>

#include "debug.h"

Err PdiLibOpen(UInt16 libRefnum) {
  debug(DEBUG_ERROR, "PALMOS", "PdiLibOpen not implemented");
  return 0;
}

Err PdiLibClose(UInt16 libRefnum) {
  debug(DEBUG_ERROR, "PALMOS", "PdiLibClose not implemented");
  return 0;
}

PdiReaderType* PdiReaderNew(UInt16 libRefnum, UDAReaderType *input, UInt16 version) {
  debug(DEBUG_ERROR, "PALMOS", "PdiReaderNew not implemented");
  return 0;
}

void PdiReaderDelete(UInt16 libRefnum, PdiReaderType** ioReader) {
  debug(DEBUG_ERROR, "PALMOS", "PdiReaderDelete not implemented");
}

PdiWriterType* PdiWriterNew(UInt16 libRefnum, UDAWriterType *output, UInt16 version) {
  debug(DEBUG_ERROR, "PALMOS", "PdiWriterNew not implemented");
  return 0;
}

void PdiWriterDelete(UInt16 libRefnum, PdiWriterType** ioWriter) {
  debug(DEBUG_ERROR, "PALMOS", "PdiWriterDelete not implemented");
}

Err PdiReadProperty(UInt16 libRefnum, PdiReaderType* ioReader) {
  debug(DEBUG_ERROR, "PALMOS", "PdiReadProperty not implemented");
  return 0;
}

Err PdiReadPropertyField(UInt16 libRefnum, PdiReaderType* ioReader, Char** bufferPP, UInt16 bufferSize, UInt16 readMode) {
  debug(DEBUG_ERROR, "PALMOS", "PdiReadPropertyField not implemented");
  return 0;
}

Err PdiReadPropertyName(UInt16 libRefnum, PdiReaderType* ioReader) {
  debug(DEBUG_ERROR, "PALMOS", "PdiReadPropertyName not implemented");
  return 0;
}

Err PdiReadParameter(UInt16 libRefnum, PdiReaderType* ioReader) {
  debug(DEBUG_ERROR, "PALMOS", "PdiReadParameter not implemented");
  return 0;
}

Err PdiDefineResizing(UInt16 libRefnum, PdiReaderType* ioReader, UInt16 deltaSize, UInt16 maxSize) {
  debug(DEBUG_ERROR, "PALMOS", "PdiDefineResizing not implemented");
  return 0;
}

Err PdiEnterObject(UInt16 libRefnum, PdiReaderType* ioReader) {
  debug(DEBUG_ERROR, "PALMOS", "PdiEnterObject not implemented");
  return 0;
}

Err PdiWriteBeginObject(UInt16 libRefnum, PdiWriterType* ioWriter, UInt16 objectNameID) {
  debug(DEBUG_ERROR, "PALMOS", "PdiWriteBeginObject not implemented");
  return 0;
}

Err PdiWriteProperty(UInt16 libRefnum, PdiWriterType* ioWriter, UInt16 propertyNameID) {
  debug(DEBUG_ERROR, "PALMOS", "PdiWriteProperty not implemented");
  return 0;
}

Err PdiWriteParameter(UInt16 libRefnum, PdiWriterType* ioWriter, UInt16 parameter, Boolean parameterName) {
  debug(DEBUG_ERROR, "PALMOS", "PdiWriteParameter not implemented");
  return 0;
}

Err PdiWritePropertyValue(UInt16 libRefnum, PdiWriterType* ioWriter, Char* buffer, UInt16 options) {
  debug(DEBUG_ERROR, "PALMOS", "PdiWritePropertyValue not implemented");
  return 0;
}

Err PdiWritePropertyFields(UInt16 libRefnum, PdiWriterType* ioWriter, Char* fields[], UInt16 fieldNumber, UInt16 options) {
  debug(DEBUG_ERROR, "PALMOS", "PdiWritePropertyFields not implemented");
  return 0;
}

Err PdiWritePropertyBinaryValue(UInt16 libRefnum, PdiWriterType* ioWriter, const Char* buffer, UInt16 size, UInt16 options) {
  debug(DEBUG_ERROR, "PALMOS", "PdiWritePropertyBinaryValue not implemented");
  return 0;
}

Err PdiSetEncoding(UInt16 libRefnum, PdiWriterType* ioWriter, UInt16 encoding) {
  debug(DEBUG_ERROR, "PALMOS", "PdiSetEncoding not implemented");
  return 0;
}

Err PdiSetCharset(UInt16 libRefnum, PdiWriterType* ioWriter, CharEncodingType charset) {
  debug(DEBUG_ERROR, "PALMOS", "PdiSetCharset not implemented");
  return 0;
}

Err PdiWritePropertyStr(UInt16 libRefnum, PdiWriterType* ioWriter, const Char* propertyName, UInt8 writeMode, UInt8 requiredFields) {
  debug(DEBUG_ERROR, "PALMOS", "PdiWritePropertyStr not implemented");
  return 0;
}

Err PdiWriteParameterStr(UInt16 libRefnum, PdiWriterType* ioWriter , const Char* parameterName, const Char* parameterValue) {
  debug(DEBUG_ERROR, "PALMOS", "PdiWriteParameterStr not implemented");
  return 0;
}

PdiDictionary* PdiDefineReaderDictionary(UInt16 libRefnum, PdiReaderType* ioReader, PdiDictionary* dictionary, Boolean disableMainDictionary) {
  debug(DEBUG_ERROR, "PALMOS", "PdiDefineReaderDictionary not implemented");
  return 0;
}

PdiDictionary* PdiDefineWriterDictionary(UInt16 libRefnum, PdiWriterType* ioWriter, PdiDictionary* dictionary, Boolean disableMainDictionary) {
  debug(DEBUG_ERROR, "PALMOS", "PdiDefineWriterDictionary not implemented");
  return 0;
}

