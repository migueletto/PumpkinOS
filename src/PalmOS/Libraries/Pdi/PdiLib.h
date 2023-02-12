/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PdiLib.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *              Public API of versit lib
 *
 *****************************************************************************/

#ifndef __PDILIB_H__
#define __PDILIB_H__

#ifndef BUILDING_PDI_LIB
#	define PDI_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#else
# 	define PDI_LIB_TRAP(trapNum)
#       define sysLibTrapCustom 0
#endif

#include <PalmTypes.h>
#include <SystemMgr.h>
#include <TextMgr.h>

/*******************************************************************
 *	Unified data access types and macros
 *******************************************************************/

#include <UDAMgr.h>

/*******************************************************************
 *	Pdi library built-in name constants (default dictionary)
 *******************************************************************/

// Constants for vObject Names id, (PR)operties (N)ames id
// for (PA)rameters (N)ames id and (PA)rameters (V)alues id

#include <PdiConst.h>
 
/*******************************************************************
 * Internal library name which can be passed to SysLibFind()
 *******************************************************************/
 
#define		kPdiLibName						"Pdi.lib"	

/*******************************************************************
 * Pdi Library function trap ID's
 *******************************************************************/
 
#define PdiLibTrapReaderNew				(sysLibTrapCustom)
#define PdiLibTrapReaderDelete			(sysLibTrapCustom+1)
#define PdiLibTrapWriterNew				(sysLibTrapCustom+2)
#define PdiLibTrapWriterDelete			(sysLibTrapCustom+3)
#define PdiLibTrapReadProperty			(sysLibTrapCustom+4)
#define PdiLibTrapReadPropertyField		(sysLibTrapCustom+5)
#define PdiLibTrapReadPropertyName		(sysLibTrapCustom+6)
#define PdiLibTrapReadParameter			(sysLibTrapCustom+7)
#define PdiLibTrapDefineResizing		(sysLibTrapCustom+8)
#define PdiLibTrapEnterObject			(sysLibTrapCustom+9)
#define PdiLibTrapWriteBeginObject		(sysLibTrapCustom+10)
#define PdiLibTrapWriteProperty			(sysLibTrapCustom+11)
#define PdiLibTrapWriteParameter		(sysLibTrapCustom+12)
#define PdiLibTrapWritePropertyValue	(sysLibTrapCustom+13)
#define PdiLibTrapWritePropertyFields	(sysLibTrapCustom+14)
#define PdiLibTrapWritePropertyBinaryValue	(sysLibTrapCustom+15)
#define PdiLibTrapSetEncoding			(sysLibTrapCustom+16)
#define PdiLibTrapSetCharset			(sysLibTrapCustom+17)
#define PdiLibTrapWritePropertyStr		(sysLibTrapCustom+18)
#define PdiLibTrapWriteParameterStr		(sysLibTrapCustom+19)
#define PdiLibTrapDefineReaderDictionary	(sysLibTrapCustom+20)
#define PdiLibTrapDefineWriterDictionary	(sysLibTrapCustom+21)

/*******************************************************************
 * Pdi Library result codes
 *******************************************************************/

#define pdiErrRead 							(pdiErrorClass | 1)
#define pdiErrWrite 						(pdiErrorClass | 2)
#define pdiErrNoPropertyName 				(pdiErrorClass | 3)
#define pdiErrNoPropertyValue 				(pdiErrorClass | 4)
#define pdiErrMoreChars 					(pdiErrorClass | 5)
#define pdiErrNoMoreFields 					(pdiErrorClass | 6)
#define pdiErrOpenFailed					(pdiErrorClass | 7)
#define pdiErrCloseFailed					(pdiErrorClass | 8)

/*******************************************************************
 * Pdi library constants
 *******************************************************************/

#define kPdiASCIIEncoding 					0										// consider ascii value 
#define kPdiQPEncoding  					kPdiPAV_ENCODING_QUOTED_PRINTABLE		// value must be QP encoded (write) or is QP encoded (read)
#define kPdiB64Encoding 					kPdiPAV_ENCODING_BASE64					// value must be B64 encoded (write) or is B64 encoded (read)
#define kPdiBEncoding 						kPdiPAV_ENCODING_B						// same as above but ENCODING=B in place of ENCODING=BASE64
#define kPdiEscapeEncoding          		((UInt16) (0x8000))						// special encoding where newline are backslashed
#define kPdiNoEncoding          			((UInt16) (0x8001))						// value must not be encoded (write)

// Constants for structured property values
#define kPdiNoFields						((UInt16) 0)		// Consider property value has just one field
#define kPdiCommaFields						((UInt16) 1)		// Consider property value can have several fields comma separated
#define kPdiSemicolonFields					((UInt16) 2)		// Consider property value can have several fields semicolon separated
#define kPdiDefaultFields					((UInt16) 4)		// Accept default fields definition (dictionary information)
#define kPdiConvertComma					((UInt16) 8)		// Consider property value has just one field, commas are converted to '\n'
#define kPdiConvertSemicolon				((UInt16) 16)		// Consider property value has just one field, semicolons are converted to '\n'

// Constants to manage parser/generator behavior

// Generator behavior
#define kPdiEnableBase64					((UInt16) 0)		// Base64 is the default behavior
#define kPdiEnableFolding					((UInt16) 1)
#define kPdiEnableQuotedPrintable			((UInt16) 2)
#define kPdiEscapeMultiFieldValues			((UInt16) 4) 		// Earlier PalmOS compatiblity
#define kPdiEnableB							((UInt16) 8) 		// New B encoding type (in place of base64)

#define kPdiPalmCompatibility 				(kPdiEscapeMultiFieldValues | kPdiEnableQuotedPrintable)

// Parser behavior, currently the open parser is OK
// Maybe future evolution will declare new constants
#define kPdiOpenParser						((UInt16) 16) 		// Generic parser

// parser/generator behavior
#define kPdiBypassLocaleCharEncoding		((UInt16) 32)		// bypass inbound/outbound default char encoding (determined via locale)


// Constants to manage writting of values
#define kPdiWriteData						((UInt16) 0)		// No charset computation (non text values)
#define	kPdiWriteText						((UInt16) 8)		// charset computation
#define kPdiWriteMultiline					((UInt16) 16)		// if present: must encode else encoding is determinated by charset

// Constant to manage growing buffers
#define kPdiResizableBuffer					((UInt16) 0xFFFF)	// Special value to indicate a resizable buffer (handle based)
#define kPdiDefaultBufferMaxSize 			((UInt16) 0x3FFF)	// Maximum size of a resizable buffer non including terminal 0
#define kPdiDefaultBufferDeltaSize 			((UInt16) 0x0010) 	// Delta (& minimum) size of resizable buffer

// event mask of automata
#define kPdiEOFEventMask 					((UInt16) 1)
#define kPdiGroupNameEventMask				((UInt16) 2)		// A group name is found
#define kPdiPropertyNameEventMask			((UInt16) 4)		// A property name is found
#define kPdiParameterNameEventMask			((UInt16) 8)		// A parameter name is found
#define kPdiParameterValueEventMask		 	((UInt16) 16)		// A parameter value is found
#define kPdiPropertyDefinedEventMask		((UInt16) 32)		// A property definition is found (the ':' separator is reached)
#define kPdiPropertyValueEventMask			((UInt16) 64)		// An entire property value is found
#define kPdiPropertyValueFieldEventMask	 	((UInt16) 128)		// A value field is found (';' separated)
#define kPdiPropertyValueItemEventMask		((UInt16) 256)		// A value item is found (',' separated)
#define kPdiPropertyValueMoreCharsEventMask ((UInt16) 512)		// The application didn't provide a large enought buffer: more chars must be read
#define kPdiBeginObjectEventMask 		 	((UInt16) 1024)		// BEGIN reached
#define kPdiEndObjectEventMask 		 	 	((UInt16) 2048)		// END reached
#define kPdiPropertyValueCRLFEventMask		((UInt16) 4096)  	// A value item is found (',' separated)

/*******************************************************************
 * Public Data structures.
 *******************************************************************/

typedef UInt8 PdiDictionary;

typedef struct PdiReaderTag {
    Err					error;					// last error
    UInt8				encoding;				// Type of encoding of the property value
    UInt8				fieldNum;
    CharEncodingType	charset;				// Charset of property value
    UInt16				written;				// Current number of chars already written in buffer
    UInt16				property;				// ID of the current property
    UInt16				propertyValueType;		// type of property value
    UInt16				parameter;				// ID of the last parsed parameter name
    UInt32				parameterPairs[8];		// set of bits of parsed parameter values
    UInt16  			customFieldNumber;		// Value of X-PALM-CUSTOM (cutom fields)
    void*   			appData;				// General usage app dependent field
    UInt16				pdiRefNum;				// The refNum of the Pdi library
    UInt16				events;					// Mask of events (see kPdiXXXXEventMask constants)
    Char*				groupName;
    Char*				propertyName;
    Char*				parameterName;
    Char*				parameterValue;
    Char*				propertyValue;
} PdiReaderType;

typedef struct _PdiWriter {
	Err					error;					// last error
	UInt16				encoding;				// Type of encoding of the property value
	CharEncodingType	charset;				// Charset of property value
    void*   			appData;				// General usage app dependent field
    UInt16				pdiRefNum;				// The refNum of the Pdi library
} PdiWriterType;

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************
 * Library Open & Close functions
 *******************************************************************/

extern Err PdiLibOpen(UInt16 libRefnum)
				PDI_LIB_TRAP(sysLibTrapOpen);
				
extern Err PdiLibClose(UInt16 libRefnum)
				PDI_LIB_TRAP(sysLibTrapClose);

/*******************************************************************
 * Reader / Writer initialization & finalization functions
 *******************************************************************/

extern PdiReaderType* PdiReaderNew(UInt16 libRefnum, UDAReaderType *input, UInt16 version)
				PDI_LIB_TRAP(PdiLibTrapReaderNew);

extern void PdiReaderDelete(UInt16 libRefnum, PdiReaderType** ioReader)
				PDI_LIB_TRAP(PdiLibTrapReaderDelete);

extern PdiWriterType* PdiWriterNew(UInt16 libRefnum, UDAWriterType *output, UInt16 version)
				PDI_LIB_TRAP(PdiLibTrapWriterNew);

extern void PdiWriterDelete(UInt16 libRefnum, PdiWriterType** ioWriter)
				PDI_LIB_TRAP(PdiLibTrapWriterDelete);
				

/*******************************************************************
 * Read functions group. 
 *******************************************************************/

extern Err PdiReadProperty(UInt16 libRefnum, PdiReaderType* ioReader)
				PDI_LIB_TRAP(PdiLibTrapReadProperty);

extern Err PdiReadPropertyField(UInt16 libRefnum, PdiReaderType* ioReader, Char** bufferPP, UInt16 bufferSize, UInt16 readMode)
				PDI_LIB_TRAP(PdiLibTrapReadPropertyField);

extern Err PdiReadPropertyName(UInt16 libRefnum, PdiReaderType* ioReader)
				PDI_LIB_TRAP(PdiLibTrapReadPropertyName);

extern Err PdiReadParameter(UInt16 libRefnum, PdiReaderType* ioReader)
				PDI_LIB_TRAP(PdiLibTrapReadParameter);
				
extern Err PdiDefineResizing(UInt16 libRefnum, PdiReaderType* ioReader, UInt16 deltaSize, UInt16 maxSize)
				PDI_LIB_TRAP(PdiLibTrapDefineResizing);

#define PdiParameterPairTest(reader, pair) \
	((reader->parameterPairs[(pair) & 7] & ((UInt32) (1) << ((UInt8) (pair) >> 3))) != 0)

/*******************************************************************
 * Recursive objects functions group.
 *******************************************************************/

extern Err PdiEnterObject(UInt16 libRefnum, PdiReaderType* ioReader)
				PDI_LIB_TRAP(PdiLibTrapEnterObject);

/*******************************************************************
 * Write functions group. 
 *******************************************************************/

extern Err PdiWriteBeginObject(UInt16 libRefnum, PdiWriterType* ioWriter, UInt16 objectNameID)
				PDI_LIB_TRAP(PdiLibTrapWriteBeginObject);

#define PdiWriteEndObject PdiWriteBeginObject

extern Err PdiWriteProperty(UInt16 libRefnum, PdiWriterType* ioWriter, UInt16 propertyNameID)
				PDI_LIB_TRAP(PdiLibTrapWriteProperty);

extern Err PdiWriteParameter(UInt16 libRefnum, PdiWriterType* ioWriter, UInt16 parameter, Boolean parameterName)
				PDI_LIB_TRAP(PdiLibTrapWriteParameter);


extern Err PdiWritePropertyValue(UInt16 libRefnum, PdiWriterType* ioWriter, Char* buffer, UInt16 options)
				PDI_LIB_TRAP(PdiLibTrapWritePropertyValue);
				
extern Err PdiWritePropertyFields(UInt16 libRefnum, PdiWriterType* ioWriter, Char* fields[], UInt16 fieldNumber, UInt16 options)
				PDI_LIB_TRAP(PdiLibTrapWritePropertyFields);
				
extern Err PdiWritePropertyBinaryValue(UInt16 libRefnum, PdiWriterType* ioWriter, const Char* buffer, UInt16 size, UInt16 options)
				PDI_LIB_TRAP(PdiLibTrapWritePropertyBinaryValue);

extern Err PdiSetEncoding(UInt16 libRefnum, PdiWriterType* ioWriter, UInt16 encoding)
				PDI_LIB_TRAP(PdiLibTrapSetEncoding);

extern Err PdiSetCharset(UInt16 libRefnum, PdiWriterType* ioWriter, CharEncodingType charset)
				PDI_LIB_TRAP(PdiLibTrapSetCharset);

extern Err PdiWritePropertyStr(UInt16 libRefnum, PdiWriterType* ioWriter, const Char* propertyName, UInt8 writeMode, UInt8 requiredFields)
				PDI_LIB_TRAP(PdiLibTrapWritePropertyStr);

extern Err PdiWriteParameterStr(UInt16 libRefnum, PdiWriterType* ioWriter , const Char* parameterName, const Char* parameterValue)
			 	PDI_LIB_TRAP(PdiLibTrapWriteParameterStr);

/*******************************************************************
 * Customisation functions group
 *******************************************************************/
 
extern PdiDictionary* PdiDefineReaderDictionary(UInt16 libRefnum, PdiReaderType* ioReader, PdiDictionary* dictionary, Boolean disableMainDictionary)
			PDI_LIB_TRAP(PdiLibTrapDefineReaderDictionary);

extern PdiDictionary* PdiDefineWriterDictionary(UInt16 libRefnum, PdiWriterType* ioWriter, PdiDictionary* dictionary, Boolean disableMainDictionary)
			PDI_LIB_TRAP(PdiLibTrapDefineWriterDictionary);
				
#ifdef __cplusplus 
}
#endif


#endif // __PDILIB_H__
