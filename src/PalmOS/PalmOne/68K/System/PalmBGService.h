/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PalmBGService.h
 * @version 1.0
 * @date 	06/29/2003
 *
 * @brief Defines APIs and structures for the Background Service which 
 *        lets users make a background network connection.
 * 
 *
 * <hr>
 */

#ifndef __PALMBGSERVICE_H__
#define __PALMBGSERVICE_H__

#include <PalmTypes.h>
#include <LibTraps.h>


/**
 * @name Traps
 *
 */
/*@{*/
#define bgTrapAPIVersion			(sysLibTrapCustom+0)	/**<		*/
#define bgTrapCancelRqst			(sysLibTrapCustom+1)	/**<		*/
#define bgTrapINetLibRqst			(sysLibTrapCustom+3)	/**<		*/
#define bgTrapNetLibRqst			(sysLibTrapCustom+4)	/**<		*/
/*@}*/

/**
 * @name Library type and creator
 *
 */
/*@{*/
#define bgServiceLibName		"BGService.Lib" 	/**< Background services library name. */
#define bgServiceCreatorID		'asc9'			/**< Background services Creator ID. */
#define bgServiceType			'appl' 			/**< Background services Type. */
/*@}*/


/**
 * @name Typedefs and constants used in Background service lib
 *
 */
/*@{*/
#define IBG_SUCCESS         0	/**< Return Value indicating success.*/


#define IBG_ERRNOTPOSSIBLE  2	/**< Return Value indicating on i705 that radio is NOT on or WebClipping is
 				  *  NOT configured in i705 Wireless panel.
                                  */

#define IBG_ERRTIMEOUT      3 	/**< Return Value indicating the request made could not be
 				  *  completed because of a network timeout, or request took longer than
 				  *  20 minutes to complete for BGServiceNetRqst() or greater than 2
 				  *  minutes for BGServiceINetRqst().
                                  */

#define IBG_ERRREQUEST      4	/**< Return Value indicating an error occurred processing the request.  If
 				  *  a network error occurred, look at the protocolErr and protocolErrString
 				  *  members in the returned BGResult value for details.
                                  */



#define IBG_ERRRSPTOOBIG    6	/**< Return Value indicating the response handle passed with the request is not
 				  *  large enough to contain the response gotten from the server.  In this case
 				  *  a partial response may be contained in the response handle, but it is
 				  *  truncated.
                                  */

#define IBG_ERRSHUTDOWN     7	/**< Return Value indicating the BGService cannot process the request due to a
 				  *  HotSync or systen reset occurring
                                  */

#define IBG_ERRRQSTDONE     8	/**< Return Value indicating a request cannot be cancelled with BGServiceCancelRqst()
 				  *  because the request was already started.
                                  */

#define IBG_MAXBUFSZ        100   /**< Maximum size of protocolErrString. */
/*@}*/

/**
 * @brief Structure containing Result of response for a protocol request 
 */
typedef struct 
{
	UInt32	majorResult;                      /**< One of the above defined return values */	
	UInt32	protocolErr;                      /**< Any protocol specific error code, if any */
	char 	protocolErrString[IBG_MAXBUFSZ];  /**< Any protocol err response from the server */

} BGResult;


/********************************************************************
 * Prototypes
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opens the Background Service library.
 *	  This function should be called prior to calling the other Background Service functions.
 * 
 * @param libRefNum: 	IN:  Reference number of the Background Service library.
 * @retval The error code returned from the library. If this is errNone, the
 *	   function was sucessful.
 *
 * @code
 * UInt16 bgServiceLibRefNum = 0;
 * err = SysLibFind(bgServiceLibName, &bgServiceLibRefNum);
 * if( err == sysErrLibNotFound )
 * {
 *     err = SysLibLoad(bgServiceType, bgServiceCreatorID, &bgServiceLibRefNum);
 *         if( !err ) {
 *             err = BGServiceOpen(bgServiceLibRefNum);
 *         }
 * } @endcode
 *
 * @see BGServiceClose
 */
extern Err BGServiceOpen( UInt16 libRefNum )
				SYS_TRAP( sysLibTrapOpen );

/**
 * @brief Closes the Background Service library.
 *	  This function should be called after your application has finished with the Background Service 
 * 	  library.
 *
 * @param libRefNum:	IN:  Reference number of the Background Service  library.
 * @retval The error code returned from the library. If this is errNone, the
 *	   function was sucessful.
 * 
 * @code
 * err = BGServiceClose(bgServiceLibRefNum);@endcode
 *
 * @see BGServiceOpen
 */
extern Err BGServiceClose( UInt16 libRefNum )
				SYS_TRAP( sysLibTrapClose );

/**
 * @brief Returns the version number of Background Service 
 * 
 * @param libRefNum:	IN:  Reference number of the library.
 * @param pversion:	IN:  Pointer to version number returned
 *
 * @retval Error code. Always IBG_SUCCESS
 */
extern Err BGServiceVersion( UInt16 libRefNum, UInt32 *pversion )
				SYS_TRAP( bgTrapAPIVersion );


/**
 * @brief Cancels a current request identified by the Request id
 * 
 * @param libRefNum:	IN:  Reference number of the library.
 * @param rqstId:	IN:  Request ID of the BgRequest to be cancelled.
 *
 * @retval Error code. IBG_SUCCESS on success, otherwise IBG_ERRRQSTDONE if request 
 *         was not found or has begun already.
 */
extern Err BGServiceCancelRqst( UInt16 libRefNum, UInt32 rqstId )
				SYS_TRAP( bgTrapCancelRqst );


/**
 * @brief Background Request's Response Callback function.
 *
 * @param rqstId	IN:  Request id that identifies a particular request the response is for 
 * @param presult	IN:  Contains the response protocol error message if any.
 * @param responseH	IN:  Handle to the response string.
 * @param respSz	IN:  Size of response
 * @param opaque	IN:  Opaque value passed back to the callback, can be used for userdata
 * @retval UInt32  One of the IBG_xxx return codes defined above.  Currently any return
 *         value is ignored.
 */
typedef UInt32 ( * BGRecvResponseFP )( UInt32 rqstId, BGResult *presult, MemHandle *responseH, 
									   UInt32 respSz, UInt32* opaque );


/**
 *
 * @brief Perform a INetLib request via the HTTP POST command
 *
 * @param libRefNum:	IN:  Reference number of the library.
 * @param purl:		IN:  Url of server to issue request to.  Formatted for INetLibURLCrack command
 * @param ppostBuf:    	IN:  Any post data for request, formatted for INetLibSockHTTPReqSend command - 
 *                           No additional formatting is performed.
 * @param precvFunc:	IN:  Response callback function
 * @param opaque:	IN:  Opaque value passed back to callback routine when called
 * @param respH:	IN:  Pre-allocated memory handle to hold response.  Must be big enough to hold
 *                           largest response ( for INet typically less than 10K )
 * @param prqstId:	IN:  Returned id given to request for later identification or canceling
 * 
 * @retval ErrorCode   IBG_SUCCESS if accepted, otherwise another error code defined above
 */
extern Err BGServiceINetRqst( UInt16 libRefNum, char *purl, char *ppostBuf, BGRecvResponseFP precvFunc,
                              UInt32 opaque, MemHandle respH, UInt32 *prqstId )
				SYS_TRAP( bgTrapINetLibRqst );

/**
 *
 * @brief Perform a NetLib request on the default network connection ( for now )
 *
 * @param libRefNum:	IN:  Reference number of the library.
 * @param notUsed:	IN:  Always pass NULL.
 * @param pserver:	IN:  Server to issue request to.  Hostnames are looked up via DNS calls, 
 *                           although slows down requests.  Better to use IP addresses if possible.
 * @param port:		IN:  Port to make call on
 * @param prqst:	IN:  Protocol specific string to make once connection is made to server.
 * @param pterm:	IN:  Server response terminator to look for to know have gotten complete
 *                           response from server, or 0 if don't have a terminator.  Of course
 *                           using a terminator greatly speeds up receiving response.
 * @param precvFunc:	IN:  Response callback function 
 * @param opaque:	IN:  Opaque value passed back to callback routine when called
 * @param reserved:	IN:  Reserved for future use, must always be 0
 * @param respH:	IN:  Pre-allocated memory handle to hold response.  Must be big enough to hold
 *                           largest response ( for INet typically less than 10K )
 * @param prqstId:	IN:  Returned id given to request for later identification or canceling
 * 
 * @retval IBG_SUCCESS if accepted, otherwise another error code defined above
 ***/
extern Err BGServiceNetRqst( UInt16 libRefNum, char *notUsed, char *pserver, UInt16 port,
                             char *prqst, char *pterm, BGRecvResponseFP precvFunc, UInt32 opaque, 
                             UInt8 reserved, MemHandle respH, UInt32 *prqstId )
				SYS_TRAP( bgTrapNetLibRqst );




#ifdef __cplusplus 
}
#endif
	
#endif // __PALMBGSERVICE_H__
