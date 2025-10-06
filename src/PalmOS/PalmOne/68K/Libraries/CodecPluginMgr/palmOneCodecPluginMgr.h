/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Codec
 */

/**
 * @file 	palmOneCodecPluginMgr.h
 * @version 3.0
 *
 * @brief	The Codec Plugin Manager is used to unify all different type of
 * 			codecs that are available for PalmOne devices and provide a generic
 * 			API for developers to access these codecs.
 *
 * A typical application would use the library the following way:
 * - Check is an input/output format pair is supported [optional]
 * - Create a CodecSession with parameters that match the formats
 * - Call EncodeDecode()
 * - ...
 * - Delete the session.
 *
 */

#ifndef _PALMONECODECPLUGINMGR_H_
#define _PALMONECODECPLUGINMGR_H_

#include <PalmTypes.h>
#include <ErrorBase.h>
#include <SystemMgr.h>
#include <LibTraps.h>
#include <Common/Libraries/CodecPluginMgr/palmOneCodecPluginMgrCommon.h>

/***********************************************************************
 * API Prototypes
 ***********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opens the Codec Plugin Manager library.
 *
 * @param refNum:	IN:	Reference number of the CodecPluginMgr library.
 * @retval Err errNone if no error
 */
Err CodecMgrOpen(UInt16 refNum)
		SYS_TRAP(sysLibTrapOpen);
/**
 * Closes the Codec Plugin Manager library.
 *
 * @param refNum:	IN:	Reference number of the CodecPluginMgr library.
 * @retval Err The library is closed and errNone is returned. If another application is still using the
 *             library, a kCodecMgrLibStillOpen error is returned.
 */
Err CodecMgrClose(UInt16 refNum)
		SYS_TRAP(sysLibTrapClose);

/**
 * Standard Sleep function.
 *
 * @param refNum:	IN: Reference number of the CodecPluginMgr library.
 * @retval Err errNone if no error
 */
Err CodecMgrSleep(UInt16 refNum)
		SYS_TRAP(sysLibTrapSleep);


/**
 * Standard Wake function.
 *
 * @param refNum:	IN: Reference number of the CodecPluginMgr library.
 * @retval Err errNone if no error
 */
Err CodecMgrWake(UInt16 refNum)
		SYS_TRAP(sysLibTrapWake);

/**
 * Return the version of the Codec Plugin Manager library API to which
 * this Codec was written. The API version of the manager that is loading
 * this plugin is passed in, so that the plugin can either handle older
 * managers, or return kCodecMgrLibNotSupported.
 *
 * @param refNum:					IN: Reference number of the CodecPluginMgr library.
 * @param codecMgrAPIVersion:		IN: The largest version of the codec API supported
 * @param codecMgrLibAPIVersionP:	IN:	Receives the API version to which this plugin was written.
 * 										If the plugin supports multiple API versions, this should
 *										be set to the same version as codecMgrAPIVersion.
 * @retval Err errNone if no error, kCodecMgrLibNotSupported if the manager
 *             is too old to handle this plugin
 */
Err CodecMgrLibAPIVersion(UInt16 refNum, UInt32 codecMgrAPIVersion, UInt32 *codecMgrLibAPIVersionP)
		SYS_TRAP(kCodecMgrLibTrapLibAPIVersion);

/**
 * Create a Codec Session.
 * A session is Codec dependent, which means it is up to a Codec to define
 * the structure (if any) pointed by the Session.
 * This function is called by the CodecPluginMgr. At this stage, the CPM
 * has already selected a known format pair from its codec list.
 * The input and output parameters have to match the format they represent.
 * E.g.	palmCodecImageJPEG as input uses PalmJPEGEncodeType
 *		palmCodecImageJPEG as output uses PalmJPEGDecodeType etc.
 *
 * @param refNum:		IN: Reference number of the CodecPluginMgr library.
 * @param inputFormat:	IN:	Input format for this session.
 * @param inputParamP:	IN: Parameter associated with the input format. NULL if not needed.
 * @param outputFormat:	IN: Output format for this session.
 * @param outputParamP:	IN: Parameter associated with the output format. NULL if not needed.
 * @param sessionP		OUT: Receives the reference to the newly created session.  This reference is passed into all future calls for this session.
 *
 * @retval Err errNone if no error.
 */
Err CodecMgrCreateSession(UInt16 refNum, PalmCodecFormat inputFormat, void *inputParamP, PalmCodecFormat outputFormat, void *outputParamP, PalmCodecSession *sessionP)
		SYS_TRAP(kCodecMgrLibTrapCreateSession);


/**
 * Create a Codec Session using Creator and Codec  ID.
 * A session is Codec dependent, which means it is
 * up to a Codec to define the structure (if any) pointed by the Session.
 * This function is called by the CodecPluginMgr. At this stage, the CPM
 * has already selected a known format pair from its codec list.
 * The input and output parameters have to match the format they represent.
 * E.g.	palmCodecImageJPEG as input uses PalmJPEGEncodeType
 *		palmCodecImageJPEG as output uses PalmJPEGDecodeType etc.
 *
 * This function is used to create a session when a multiple codecs
 * using the same input and output format are available.
 *
 * @param refNum:		IN: Reference number of the CodecPluginMgr library.
 * @param inputFormat:	IN: Input format for this session.
 * @param inputParamP:	IN: Parameter associated with the input format. NULL if not needed.
 * @param outputFormat:	IN: Output format for this session.
 * @param outputParamP:	IN: Parameter associated with the output format. NULL if not needed.
 * @param creatorID:	IN: Creator ID of the module containing the codec.
 * @param codecID:		IN: Codec ID for a specific codec.
 * @param sessionP:		OUT: Receives the reference to the newly created session.  This reference is passed into all future calls for this session.
 *
 * @return	errNone if no error.
 */
Err CodecMgrCreateSessionByID(UInt16 refNum, PalmCodecFormat inputFormat, void *inputParamP, PalmCodecFormat outputFormat, void *outputParamP, UInt32 creatorID, UInt32 codecID, PalmCodecSession *sessionP)
		SYS_TRAP(kCodecMgrLibTrapCreateSessionByID);


/**
 * Delete a session.
 * This function has to delete any allocated memory and set the
 * session reference to NULL.
 *
 * @param refNum:	IN: Reference number of the CodecPluginMgr library.
 * @param sessionP:	IN: Pointer to the Codec session reference received from CodecCreateSession.
 * @retval Err errNone if no error.
 */
Err CodecMgrDeleteSession(UInt16 refNum, PalmCodecSession *sessionP)
		SYS_TRAP(kCodecMgrLibTrapDeleteSession);

/**
 * Reset a session.
 *
 * @param refNum:	IN: Reference number of the CodecPluginMgr library.
 * @param session:	IN: Session to reset.
 * @retval Err errNone if no error.
 */
Err CodecMgrResetSession(UInt16 refNum, PalmCodecSession session)
		SYS_TRAP(kCodecMgrLibTrapResetSession);

/**
 * Returns the output buffer size required given an input buffer.
 * This function is not required, but rather is provided to try to help the caller
 * allocate the appropriate amount of memory for a call to CodecEncodeDecode
 * that takes the same parameters.  It is recommended that you always return
 * a number equal to or larger then the buffer that will be required to hold
 * the output of the CodecEncodeDecode call.
 *
 * @param refNum:			IN: Reference number of the CodecPluginMgr library.
 * @param session:			IN:	Codec session reference received from CodecCreateSession.
 * @param srcBufferP:		IN:	Input buffer.
 * @param srcBufferSize:	IN: Size of the input buffer in bytes.
 * @param destBufferSizeP:	OUT: Receives that maximum buffer size.
 *
 * @retval Err errNone if no error, kCodecMgrLibNotSupported if no implemented.
 */
Err CodecMgrGetMaxDestBufferSize(UInt16 refNum, PalmCodecSession session, void *srcBufferP, UInt32 srcBufferSize, UInt32 *destBufferSizeP)
		SYS_TRAP(kCodecMgrLibTrapGetMaxDestBufferSize);
/**
 * Encode or decode a block of data.
 * The format of the source and destination buffers are set in a
 * preceding call to CodecCreateSession and possibly calls to CodecCustomControl.
 * Is is legal for this function to not encode/decode the whole source buffer.  For
 * example, if the source buffer contains a non-round number of encoded blocks, and the
 * encoding format requires whole blocks, this call should encode/decode the even
 * number of blocks, and set srcBufferP appropriately to indicate how much of the
 * source data was actually encoded.  The caller should then pass the un-encoded piece of
 * the last chunk in on the next call to this function with the next chunk of data
 * appended to the end of it.
 *
 * @param refNum:			IN: Reference number of the CodecPluginMgr library.
 * @param session:			IN: Codec session reference received from CodecCreateSession.
 * @param srcBufferP:		IN: The source buffer to be processed.
 * @param srcBufferSizeP:	IN,OUT: On input, the size of srcBufferP in bytes.  If the whole
 *                                  buffer is not processed, on output srcBufferP is set to the amount of
 *                                  source data that was processed.
 * @param destBufferP:		OUT: The destination buffer which receives the encoded/decoded data.
 * @param destBufferSizeP:	IN,OUT: On input, the size of destBufferP in bytes, on output the number of bytes of data written.
 *
 * @retval Err errNone if no error
 */
Err CodecMgrEncodeDecode(UInt16 refNum, PalmCodecSession session, void *srcBufferP, UInt32 *srcBufferSizeP, void *destBufferP, UInt32 *destBufferSizeP)
		SYS_TRAP(kCodecMgrLibTrapEncodeDecode);

/**
 * Handle a custom call.
 *
 * The codec identifies the call and its API by a registered creator code and a selector.
 * This allows codec developers to extend the API by defining selectors for their
 * creator code. It also allows driver developers to support selectors (and custom calls)
 * defined by other driver developers.
 *
 * @param refNum:		IN: Reference number of the CodecPluginMgr library.
 * @param session:		IN: Codec session reference received from CodecCreateSession.
 * @param apiCreator:	IN: Registered creator code.
 * @param apiSelector:	IN: Custom operation to perform.
 * @param valueP:		IN:	Buffer containing data specific to the operation.
 * @param valueLenP:	IN,OUT: Size of the valueP buffer on entry, size of data written to
 *                              valueP on exit. If NULL, valueP is ignored.
 * @retval Err errNone if no error, kCodecMgrLibNotSupported if not supported.
 */
Err CodecMgrCustomControl(UInt16 refNum, PalmCodecSession session, UInt32 apiCreator, UInt32 apiSelector, void *valueP, UInt32 *valueLenP)
		SYS_TRAP(kCodecMgrLibTrapCustomControl);

/**
 * Enumerate all supported input and output format pairs.
 * A pair describes what input format is expected and what output format
 * will come out of the Codec. Codecs can accept multiple format pairs.
 * For example, an MP3 Codec might be able to decode only MP3, MP2, and MP1.
 * In this case, the pairs are:
 *		[in: MP3	out: PCM	codecID: 0]
 *  	[in: MP2	out: PCM	codecID: 0]
 *  	[in: MP1	out: PCM	codecID: 0]
 *
 * The Codec ID is used if multiple codecs within the same PRC have the exact
 * same input and output formats. For example, a DSP filter bank might specify
 * PCM as both the input and output for every codec. Codec ID helps differentiate between
 * these Codecs (Low-Pass, High-Pass, Band-Pass...).
 * A Codec ID set to 0 (palmNULLCodecID) means that there is no need to look for
 * a particular codec (for example, if there is only one codec in the PRC).
 *
 * Pass a formatIteratorP of palmCodecIteratorStart to get the first pair.
 * formatIteratorP will be updated to the next item. When returning the last
 * format pair, the formatIteratorP will be set to palmCodecIteratorStop.
 *
 * Sample:
 *
 * PalmCodecFormat inputFormat, outputFormat;
 * UInt32 codecID;
 * UInt32 formatIterator = palmCodecIteratorStart;
 * while (formatIterator != palmCodecIteratorStop) {
 * 		if ((err = CodecEnumerateSupportedFormats(&formatIterator, &inputFormat, &outputFormat, &codecID)) != errNone) {
 *				// Do something with the format pair
 *		} else {
 *				// handle error... possibly by breaking out of the loop
 *		}
 * }
 *
 * @param refNum:			IN: Reference number of the CodecPluginMgr library.
 * @param formatIteratorP:	IN,OUT:	Reference to the last entry enumerated
 *                                  Pass palmCodecIteratorStart to get the first entry.
 *                                  This is updated on return to reference the next entry
 *                                  or set to palmCodecIteratorStop if last directory entry.
 * @param inputFormatP:		IN: Receives the input format entry specified with formatIteratorP.
 * @param outputFormatP:	IN: Receives the output format entry specified with formatIteratorP.
 * @param creatorIDP:		IN:	Pointer to the location where the creator ID associated with the current
 *								formatIteratorP entry is stored. Pass NULL to ignore this value for each codec.
 * @param codecIDP:			IN: Pointer to the location where the codec ID associated with the current
 *                              formatIteratorP entry is stored. Pass NULL to ignore this value for each codec.
 *
 * @retval Err errNone or kCodecErrBadParam
 */
Err CodecMgrEnumerateFormats(UInt16 refNum, UInt32 *formatIteratorP, PalmCodecFormat *inputFormatP, PalmCodecFormat *outputFormatP, UInt32 *creatorIDP, UInt32 *codecIDP)
		SYS_TRAP(kCodecMgrLibTrapEnumerateFormats);

/**
 * Checks whether an input and output format pair is supported by any codec or by
 * a particular codec.
 *
 * @param refNum:		IN: Reference number of the CodecPluginMgr library.
 * @param inputFormat:	IN: Input format for this session.
 * @param outputFormat:	IN: Output format for this session.
 * @param creatorID:	IN: Creator ID of the codec checked. Pass palmCodecNullID to ignore this parameter
 *                          and check whether any available codec supports the given input and output
 *                          format pair.
 * @param codecID:		IN: Codec ID of the codec checked. Pass palmCodecNullID to ignore this parameter
 *                          and check whether any available codec supports the given input and output
 *                          format pair.
 *
 * @retval Boolean true if the format is supported by a codec or the specified codec, false if the format
 *                 pair is not supported by a codec or the specified codec.
 */
Boolean CodecMgrIsFormatSupported(UInt16 refNum, PalmCodecFormat inputFormat, PalmCodecFormat outputFormat, UInt32 creatorID, UInt32 codecID)
		SYS_TRAP(kCodecMgrLibTrapIsFormatSupported);

/**
 * Get information about a session.
 *
 * @param refNum:			IN: Reference number of the CodecPluginMgr library.
 * @param session:			IN: Session to reset.
 * @param inputFormatP:		OUT: Receives the input format entry specified with formatIteratorP.
 * @param outputFormatP:	OUT: Receives the output format entry specified with formatIteratorP.
 * @param creatorIDP:		OUT: Pointer to the location where the creator ID of the session is returned.
 * @param codecIDP:			OUT: Pointer to the location where the codec ID of the session is returned.
 *
 * @retval Err errNone if no error.
 */
Err CodecMgrGetSessionInfo(UInt16 refNum, PalmCodecSession session, PalmCodecFormat *inputFormatP, PalmCodecFormat *outputFormatP, UInt32 *creatorIDP, UInt32 *codecIDP)
		SYS_TRAP(kCodecMgrLibTrapGetSessionInfo);



#ifdef __cplusplus
}
#endif

#endif  // _PALMCODECPLUGINMGR_H_
