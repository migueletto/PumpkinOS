/*
	HS_HTTPLibConst.h
	Copyright(c) 1996-2002 ACCESS CO., LTD.
	All rights are reserved by ACCESS CO., LTD., whether the whole or
	part of the source code including any modifications.
*/

/**
 * @file 	HS_HTTPLibConst.h
 * @version 2.0
 * @date 	
 */
#ifndef HS_HTTPLIBCONST_H__
#define HS_HTTPLIBCONST_H__

/* constants */

/* const */
#define kHTTPLibTrapOpen sysLibTrapOpen
#define kHTTPLibTrapClose sysLibTrapClose
#define kHTTPLibTrapSleep sysLibTrapSleep
#define kHTTPLibTrapWake sysLibTrapWake
#define kHTTPLibTrapOpenCount  (sysLibTrapCustom + 0)
#define kHTTPLibTrapInitialize  (sysLibTrapCustom + 1)
#define kHTTPLibTrapFinalize  (sysLibTrapCustom + 2)
#define kHTTPLibTrapSSLClass  (sysLibTrapCustom + 3)
#define kHTTPLibTrapRegisterSSLClass  (sysLibTrapCustom + 4)
#define kHTTPLibTrapClearDNSCache  (sysLibTrapCustom + 5)
#define kHTTPLibTrapCloseAllKeepAlive  (sysLibTrapCustom + 6)
#define kHTTPLibTrapMakeProxy  (sysLibTrapCustom + 7)
#define kHTTPLibTrapSetHTTPPort  (sysLibTrapCustom + 8)
#define kHTTPLibTrapSetHTTPSPort  (sysLibTrapCustom + 9)
#define kHTTPLibTrapSetMaxRequestHeader  (sysLibTrapCustom + 10)
#define kHTTPLibTrapSetMaxRequestBody  (sysLibTrapCustom + 11)
#define kHTTPLibTrapSetProxy  (sysLibTrapCustom + 12)
#define kHTTPLibTrapSetUserAgent  (sysLibTrapCustom + 13)
#define kHTTPLibTrapSetDefaultHeaders  (sysLibTrapCustom + 14)
#define kHTTPLibTrapSetUseProxy  (sysLibTrapCustom + 15)
#define kHTTPLibTrapSetHTTP11OverProxy  (sysLibTrapCustom + 16)
#define kHTTPLibTrapSetSendReferer  (sysLibTrapCustom + 17)
#define kHTTPLibTrapSetSendProxyKeepAlive  (sysLibTrapCustom + 18)
#define kHTTPLibTrapSetSendCookie  (sysLibTrapCustom + 19)
#define kHTTPLibTrapSetConnectTimeOut  (sysLibTrapCustom + 20)
#define kHTTPLibTrapSetReqTimeOut  (sysLibTrapCustom + 21)
#define kHTTPLibTrapSetRspTimeOut  (sysLibTrapCustom + 22)
#define kHTTPLibTrapIsUseProxy  (sysLibTrapCustom + 23)
#define kHTTPLibTrapIsHTTP11OverProxy  (sysLibTrapCustom + 24)
#define kHTTPLibTrapIsSendReferer  (sysLibTrapCustom + 25)
#define kHTTPLibTrapIsSendCookie  (sysLibTrapCustom + 26)
#define kHTTPLibTrapStreamNew  (sysLibTrapCustom + 27)
#define kHTTPLibTrapStreamDelete  (sysLibTrapCustom + 28)
#define kHTTPLibTrapStreamCreateRequest  (sysLibTrapCustom + 29)
#define kHTTPLibTrapStreamCreateRequestWithChunkEncoding  (sysLibTrapCustom + 30)
#define kHTTPLibTrapStreamSendRequest  (sysLibTrapCustom + 31)
#define kHTTPLibTrapStreamSendRequestWithPostData  (sysLibTrapCustom + 32)
#define kHTTPLibTrapStreamSendRequestWithChunkEncoding  (sysLibTrapCustom + 33)
#define kHTTPLibTrapStreamReceiveResponse  (sysLibTrapCustom + 34)
#define kHTTPLibTrapStreamGetFlag  (sysLibTrapCustom + 35)
#define kHTTPLibTrapStreamClose  (sysLibTrapCustom + 36)
#define kHTTPLibTrapStreamIsHeaderReceived  (sysLibTrapCustom + 37)
#define kHTTPLibTrapStreamSetPostData  (sysLibTrapCustom + 38)
#define kHTTPLibTrapStreamGetScheme  (sysLibTrapCustom + 39)
#define kHTTPLibTrapStreamGetState  (sysLibTrapCustom + 40)
#define kHTTPLibTrapStreamGetStatusCode  (sysLibTrapCustom + 41)
#define kHTTPLibTrapStreamGetResponseHeaderLength  (sysLibTrapCustom + 42)
#define kHTTPLibTrapStreamGetResponseHeader  (sysLibTrapCustom + 43)
#define kHTTPLibTrapStreamGetContentLength  (sysLibTrapCustom + 44)
#define kHTTPLibTrapStreamGetEntityLength  (sysLibTrapCustom + 45)
#define kHTTPLibTrapStreamAddHeaderLine  (sysLibTrapCustom + 46)
#define kHTTPLibTrapStreamAddHeader  (sysLibTrapCustom + 47)
#define kHTTPLibTrapStreamGetHeader  (sysLibTrapCustom + 48)
#define kHTTPLibTrapMakeCertList  (sysLibTrapCustom + 49)
#define kHTTPLibTrapHeaderSplitByX  (sysLibTrapCustom + 50)
#define kHTTPLibTrapAuthInfoTidy  (sysLibTrapCustom + 51)
#define kHTTPLibTrapSetUserPass  (sysLibTrapCustom + 52)
#define kHTTPLibTrapFindAuthInfo  (sysLibTrapCustom + 53)
#define kHTTPLibTrapMakeAuthInfo  (sysLibTrapCustom + 54)
#define kHTTPLibTrapAuthInfoUser  (sysLibTrapCustom + 55)
#define kHTTPLibTrapAuthInfoPass  (sysLibTrapCustom + 56)
#define kHTTPLibTrapAuthInfoRealm  (sysLibTrapCustom + 57)
#define kHTTPLibTrapAuthCacheGetChallengeTokenSS  (sysLibTrapCustom + 58)
#define kHTTPLibTrapSetCookieMode  (sysLibTrapCustom + 59)
#define kHTTPLibTrapSetCookieMax  (sysLibTrapCustom + 60)
#define kHTTPLibTrapSetCookie  (sysLibTrapCustom + 61)
#define kHTTPLibTrapMakeCookiesString  (sysLibTrapCustom + 62)
#define kHTTPLibTrapSaveCookiesEntries  (sysLibTrapCustom + 63)
#define kHTTPLibTrapLoadCookiesEntries  (sysLibTrapCustom + 64)
#define kHTTPLibTrapCookieMakeEmpty  (sysLibTrapCustom + 65)
#define kHTTPLibTrapSetDNSMaxRetry  (sysLibTrapCustom + 66)
#define kHTTPLibTrapSetDNSRetryInterval  (sysLibTrapCustom + 67)
#define kHTTPLibTrapSetSSLVersionFlag  (sysLibTrapCustom + 68)
#define kHTTPLibTrapSSLTimeout  (sysLibTrapCustom + 69)
#define kHTTPLibTrapSSLOpenCertDB  (sysLibTrapCustom + 70)
#define kHTTPLibTrapSSLCloseCertDB  (sysLibTrapCustom + 71)
#define kHTTPLibTrapCertListNew  (sysLibTrapCustom + 72)
#define kHTTPLibTrapCertListDelete  (sysLibTrapCustom + 73)
#define kHTTPLibTrapCertListClone  (sysLibTrapCustom + 74)
#define kHTTPLibTrapCertListLock  (sysLibTrapCustom + 75)
#define kHTTPLibTrapCertListUnlock  (sysLibTrapCustom + 76)
#define kHTTPLibTrapCertListType  (sysLibTrapCustom + 77)
#define kHTTPLibTrapCertListLength  (sysLibTrapCustom + 78)
#define kHTTPLibTrapCertListBeginDecode  (sysLibTrapCustom + 79)
#define kHTTPLibTrapCertListEndDecode  (sysLibTrapCustom + 80)
#define kHTTPLibTrapCertListImportCert  (sysLibTrapCustom + 81)
#define kHTTPLibTrapCertPeekVersion  (sysLibTrapCustom + 82)
#define kHTTPLibTrapCertPeekSerialNumber  (sysLibTrapCustom + 83)
#define kHTTPLibTrapCertPeekSignAlgo  (sysLibTrapCustom + 84)
#define kHTTPLibTrapCertBeginPeekIssuerRDN  (sysLibTrapCustom + 85)
#define kHTTPLibTrapCertPeekIssuerRDN  (sysLibTrapCustom + 86)
#define kHTTPLibTrapCertPeekIssuerRDNStrType  (sysLibTrapCustom + 87)
#define kHTTPLibTrapCert_EndPeekIssuerRDN  (sysLibTrapCustom + 88)
#define kHTTPLibTrapCertBeginPeekSubjectRDN  (sysLibTrapCustom + 89)
#define kHTTPLibTrapCertPeekSubjectRDN  (sysLibTrapCustom + 90)
#define kHTTPLibTrapCertPeekSubjectRDNStrType  (sysLibTrapCustom + 91)
#define kHTTPLibTrapCert_EndPeekSubjectRDN  (sysLibTrapCustom + 92)
#define kHTTPLibTrapCert_PeekValidityStart  (sysLibTrapCustom + 93)
#define kHTTPLibTrapCert_PeekValidityEnd  (sysLibTrapCustom + 94)
#define kHTTPLibTrapCert_PeekRSAPublicKeyBits  (sysLibTrapCustom + 95)
#define kHTTPLibTrapClientCertListNew  (sysLibTrapCustom + 96)
#define kHTTPLibTrapClientCertListClone  (sysLibTrapCustom + 97)
#define kHTTPLibTrapClientCertListDelete  (sysLibTrapCustom + 98)
#define kHTTPLibTrapClientCertListLength  (sysLibTrapCustom + 99)
#define kHTTPLibTrapClientCertListIsValid  (sysLibTrapCustom + 100)
#define kHTTPLibTrapClientCertListSetValid  (sysLibTrapCustom + 101)
#define kHTTPLibTrapSetSSLServerCertConfirmProc  (sysLibTrapCustom + 102)
#define kHTTPLibTrapServerCertConfirm  (sysLibTrapCustom + 103)
#define kHTTPLibTrapSetSSLClientCertSelectProc  (sysLibTrapCustom + 104)
#define kHTTPLibTrapClientCertSelect  (sysLibTrapCustom + 105)
#define kHTTPLibTrapSetNetLibProc  (sysLibTrapCustom + 106)
#define kHTTPLibTrapDataHandle_Length  (sysLibTrapCustom + 107)
#define kHTTPLibTrapDataHandle_Lock  (sysLibTrapCustom + 108)
#define kHTTPLibTrapDataHandle_Unlock  (sysLibTrapCustom + 109)
#define kHTTPLibTrapDataHandle_Delete  (sysLibTrapCustom + 110)
#define kHTTPLibTrapVHandle_Length  (sysLibTrapCustom + 111)
#define kHTTPLibTrapVHandle_Lock  (sysLibTrapCustom + 112)
#define kHTTPLibTrapVHandle_Unlock  (sysLibTrapCustom + 113)
#define kHTTPLibTrapVHandle_Delete  (sysLibTrapCustom + 114)
#define kHTTPLibTrapString_Length  (sysLibTrapCustom + 115)
#define kHTTPLibTrapStringNew  (sysLibTrapCustom + 116)
#define kHTTPLibTrapString_Lock  (sysLibTrapCustom + 117)
#define kHTTPLibTrapString_Unlock  (sysLibTrapCustom + 118)
#define kHTTPLibTrapString_Delete  (sysLibTrapCustom + 119)
#define kHTTPLibTrapFixedString_Length  (sysLibTrapCustom + 120)
#define kHTTPLibTrapFixedStringNew  (sysLibTrapCustom + 121)
#define kHTTPLibTrapFixedString_Lock  (sysLibTrapCustom + 122)
#define kHTTPLibTrapFixedString_Unlock  (sysLibTrapCustom + 123)
#define kHTTPLibTrapFixedString_Delete  (sysLibTrapCustom + 124)
#define kHTTPLibTrapURLString_Length  (sysLibTrapCustom + 125)
#define kHTTPLibTrapURLStringNew  (sysLibTrapCustom + 126)
#define kHTTPLibTrapURLString_Lock  (sysLibTrapCustom + 127)
#define kHTTPLibTrapURLString_Unlock  (sysLibTrapCustom + 128)
#define kHTTPLibTrapURLString_Delete  (sysLibTrapCustom + 129)
#define kHTTPLibTrapCertListGetCipherInfo  (sysLibTrapCustom + 130)
#define kHTTPLibTrapSetCookieX  (sysLibTrapCustom + 131)
#define kHTTPLibTrapSetTunnelingCallback (sysLibTrapCustom + 132)
#define kHTTPLibTrapSetMaxKeepAliveTimeout (sysLibTrapCustom + 133)
#define kHTTPLibTrapStreamResumeTimer (sysLibTrapCustom + 134)
#define kHTTPLibTrapStreamSuspendTimer (sysLibTrapCustom + 135)
#define kHTTPLibTrapStreamGetRequestSent (sysLibTrapCustom + 136)
#define kHTTPLibTrapStreamGetHeaderWithOffset (sysLibTrapCustom + 137) 
#define kHTTPLibTrapSetUserPassX (sysLibTrapCustom + 138)
#define kHTTPLibTrapFindAuthCandidate (sysLibTrapCustom + 139)
#define kHTTPLibTrapSetWakeUpCallback (sysLibTrapCustom + 140)
#define kHTTPLibTrapSetUseragentSelectionRule (sysLibTrapCustom + 141)


#define httpStreamRead		0x00000001
#define httpStreamWrite		0x00000002
#define httpStreamReadWrite	(httpStreamRead|httpStreamWrite)

enum httpAuthType_ {
	httpAuthTypeNone = -1,
	httpAuthTypeBasic = 0,
	httpAuthTypeDigest,
	httpAuthTypeTypes
};
typedef enum httpAuthType_ httpAuthTypeEnum;

enum httpAuthTarget_ {
	httpAuthTargetPage = 0,
	httpAuthTargetProxy,
	httpAuthTargetTargets
};
typedef enum httpAuthTarget_ httpAuthTargetEnum;

enum httpAuthPath_ {
	httpAuthPathSame,
	httpAuthPathAncestor,
	httpAuthPathDescendant,
	httpAuthPathCousin
};
typedef enum httpAuthPath_ httpAuthPathEnum;

#define httpStreamTextCRLF				"\015\012"
#define httpStreamTextCRLFCRLF			"\015\012\015\012"
#define httpStreamTextCRCR				"\015\015"
#define httpStreamTextLFLF				"\012\012"

/* errors */
enum httpError_ {
	
	httpErrorOK 			= 0,
	httpErrorGeneric 		= -1,
	httpErrorWouldBlock 	= -2,
	httpErrorIOSleep 		= -3,
	httpErrorNoMem 			= -6,
	httpErrorInval			= -8,
	
	httpErrorDNSInval 		= -200,
	httpErrorDNSNoServer,
	httpErrorDNSTimedout,
	httpErrorDNSNotFound,
	httpErrorTCPOpen,
	httpErrorTCPConnect,
	httpErrorTCPRead,
	httpErrorTCPWrite,
	httpErrorSSLConnect,
	httpErrorSSLHandShake,
	httpErrorSSLRead,
	httpErrorSSLWrite,
	httpErrorReqTimedout,
	httpErrorRspTimedout,
	httpErrorCacheNone,
	httpErrorCacheExpire,
	httpErrorAuthUnknown,
	httpErrorAuthNoHeader,
	httpErrorAuthFormat,
	httpErrorRedirectFormat,
	httpErrorRedirectCanceled,
	httpErrorReqHeaderSizeOver,	/* method GET */
	httpErrorReqBodySizeOver, /* method POST */
	httpErrorRspHeaderSizeOver,
	httpErrorReqTooManyContinue,
	httpErrorTLSIntolerant
};
typedef enum httpError_ httpErrorEnum;

/* header type */
enum httpHeaderType_ {
	httpHeaderTypeGeneral = 0,
	httpHeaderTypeRequest,
	httpHeaderTypeResponse,
	httpHeaderTypeEntity,
	httpHeaderTypeOther,
	httpHeaderTypeTypes
};
typedef enum httpHeaderType_ httpHeaderTypeEnum;

/* constant for http stream state */
enum httpStreamState_ {
	httpStreamStateNew = 0,
	httpStreamStateDormant,
	httpStreamStateReqCreated,
	httpStreamStateReqResolve,
	httpStreamStateReqResolving,
	httpStreamStateReqOpen,
	httpStreamStateReqConnect,
	httpStreamStateReqConnecting,
	httpStreamStateReqProxyConnect,
	httpStreamStateReqProxyConnecting,
	httpStreamStateReqSSLHandShaking,
	httpStreamStateReqSSLHandShaked,
	httpStreamStateReqSending,
	httpStreamStateReqSent,
	httpStreamStateRspHeaderWaiting,
	httpStreamStateRspContinue,
	httpStreamStateRspHeaderReceiving,
	httpStreamStateRspHeaderReceived,
	httpStreamStateRspEntityReceiving,
	httpStreamStateRspEntityReceived,
	httpStreamStateObsolete,
	httpStreamStateDelete,
	httpStreamStateStates
};
typedef enum httpStreamState_ httpStreamStateEnum;

/* constant for http stream protocol scheme */
enum httpScheme_ {
	httpSchemeHTTP = 0,
	httpSchemeHTTPS,
	httpSchemeSchemes
};
typedef enum httpScheme_ httpSchemeEnum;

/* constant for http method */
enum httpMethod_ {
	httpMethodOPTIONS = 0,
	httpMethodGET,
	httpMethodHEAD,
	httpMethodPOST,
	httpMethodPUT,
	httpMethodDELETE,
	httpMethodTRACE,
	httpMethodCONNECT,
	httpMethodMethods
};
typedef enum httpMethod_ httpMethodEnum;

/* constant for http version */
enum httpVersion_ {
	httpVersion_0_9 = 0,
	httpVersion_1_0,
	httpVersion_1_1,
	httpVersionVersions
};
typedef enum httpVersion_ httpVersionEnum;

/* constant for http status code */
enum httpStatusCode_ {
	httpStatusCodeContinue							= 100,
	httpStatusCodeSwitchingProtocols				= 101,
	httpStatusCodeOK								= 200,
	httpStatusCodeCreated							= 201,
	httpStatusCodeAccepted							= 202,
	httpStatusCodeNonAuthoritativeInformation		= 203,
	httpStatusCodeNoContent							= 204,
	httpStatusCodeResetContent						= 205,
	httpStatusCodePartialContent					= 206,
	httpStatusCodeMultipleChoices					= 300,
	httpStatusCodeMovedPermanently					= 301,
	httpStatusCodeFound								= 302,
	httpStatusCodeSeeOther							= 303,
	httpStatusCodeNotModified						= 304,
	httpStatusCodeUseProxy							= 305,
	httpStatusCodeTemporaryRedirect					= 307,
	httpStatusCodeBadRequest						= 400,
	httpStatusCodeUnauthorized						= 401,
	httpStatusCodePaymentRequired					= 402,
	httpStatusCodeForbidden							= 403,
	httpStatusCodeNotFound							= 404,
	httpStatusCodeMethodNotAllowed					= 405,
	httpStatusCodeNotAcceptable						= 406,
	httpStatusCodeProxyAuthenticationRequired		= 407,
	httpStatusCodeRequestTimeout					= 408,
	httpStatusCodeConflict							= 409,
	httpStatusCodeGone								= 410,
	httpStatusCodeLengthRequired					= 411,
	httpStatusCodePreconditionFailed				= 412,
	httpStatusCodeRequestEntityTooLarge				= 413,
	httpStatusCodeRequestURITooLarge				= 414,
	httpStatusCodeUnsupportedMediaType				= 415,
	httpStatusCodeRequestedRangeNotSatisfiable		= 416,
	httpStatusCodeExpectationFailed					= 417,
	httpStatusCodeInternalServerError				= 500,
	httpStatusCodeNotImplemented					= 501,
	httpStatusCodeBadGateway						= 502,
	httpStatusCodeServiceUnavailable				= 503,
	httpStatusCodeGatewayTimeout					= 504,
	httpStatusCodeHTTPVersionNotSupported			= 505
};
typedef enum httpStatusCode_ httpStatusCodeEnum;

/* constant for cookie mode */
enum httpCookie_ {
	httpCookieNotifyBeforeSet = 0,
	httpCookieAlwaysSet,
	httpCookieNeverSet
};
typedef enum httpCookie_ httpCookieEnum;

/* string encoding in post */
#define httpPostURLENCODED		0x00000001
#define httpPostPLAINTEXT		0x00000002
#define httpPostMULTIPART		0x00000004

enum httpHeaderID_ {
	httpHeaderIDCacheControl,
	httpHeaderIDConnection,
	httpHeaderIDDate,
	httpHeaderIDPragma,
	httpHeaderIDTrailer,
	httpHeaderIDTransferEncoding,
	httpHeaderIDUpgrade,
	httpHeaderIDVia,
	httpHeaderIDWarning,
	httpHeaderIDKeepAlive,
	httpHeaderIDProxyConnection,
	httpHeaderIDAccept,
	httpHeaderIDAcceptCharset,
	httpHeaderIDAcceptEncoding,
	httpHeaderIDAcceptLanguage,
	httpHeaderIDAuthorization,
	httpHeaderIDExpect,
	httpHeaderIDFrom,
	httpHeaderIDHost,
	httpHeaderIDIfMatch,
	httpHeaderIDIfModifiedSince,
	httpHeaderIDIfNoneMatch,
	httpHeaderIDIfRange,
	httpHeaderIDIfUnmodifiedSince,
	httpHeaderIDMaxForwards,
	httpHeaderIDProxyAuthorization,
	httpHeaderIDRange,
	httpHeaderIDReferer,
	httpHeaderIDTE,
	httpHeaderIDUserAgent,
	httpHeaderIDCookie,
	httpHeaderIDAcceptRanges,
	httpHeaderIDAge,
	httpHeaderIDETag,
	httpHeaderIDLocation,
	httpHeaderIDProxyAuthenticate,
	httpHeaderIDRetryAfter,
	httpHeaderIDServer,
	httpHeaderIDVary,
	httpHeaderIDWWWAuthenticate,
	httpHeaderIDSetCookie,
	httpHeaderIDAuthenticationInfo,
	httpHeaderIDProxyAuthenticationInfo,
	httpHeaderIDAllow,
	httpHeaderIDContentEncoding,
	httpHeaderIDContentDisposition,
	httpHeaderIDContentLanguage,
	httpHeaderIDContentLength,
	httpHeaderIDContentLocation,
	httpHeaderIDContentMD5,
	httpHeaderIDContentRange,
	httpHeaderIDContentType,
	httpHeaderIDExpires,
	httpHeaderIDLastModified,
	httpHeaderIDRefresh,
	httpHeaderIDENUMS
};
typedef enum httpHeaderID_ httpHeaderIDEnum;


/* SSL Version */
#define httpSSLFlagConnV2					0x00000001	/* connect by v2 protocol */
#define httpSSLFlagConnV3					0x00000002	/* connect by v3 protocol */
#define httpSSLFlagConnTLS					0x00000004	/* connect by TLS(v3.1) protocol */
#define httpSSLFlagConnV2V3					(httpSSLFlagConnV2 | httpSSLFlagConnV3)
#define httpSSLFlagConnV2TLS				(httpSSLFlagConnV2 | httpSSLFlagConnTLS)
#define httpSSLFlagConnV3TLS				(httpSSLFlagConnV3 | httpSSLFlagConnTLS)
#define httpSSLFlagConnV2V3TLS				(httpSSLFlagConnV2 | httpSSLFlagConnV3 | httpSSLFlagConnTLS)

enum
{
	httpCookieClearAtFinalize,
	httpCookieClearAll,
	httpCookieClearSessionCookiesOnly
};

enum {
	httpDNSModeIPV4Only = 0
};

enum {
	httpCetListTypeUnknown = 0,
	httpCetListTypeServer,
	httpCetListTypeClient,
	httpCetListTypeBrowser,
	httpCetListTypeTypes
};

#endif /* HS_HTTPLIBCONST_H__ */

