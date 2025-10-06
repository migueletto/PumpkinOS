/*
	HS_HTTPLib68K.h
	Copyright(c) 1996-2002 ACCESS CO., LTD.
	All rights are reserved by ACCESS CO., LTD., whether the whole or
	part of the source code including any modifications.
*/

/**
 * @file 	HS_HTTPLib68K.h
 * @version 2.0
 * @date 	
 *
 * HTTP Library.
 * 
 *
 * <hr>
 */


#ifndef HS_HTTPLIB68K_H__
#define HS_HTTPLIB68K_H__

#include <PalmTypes.h>
#include <LibTraps.h>

/* Make sure to move the HS_inc folder in front of the following headers when merging
 * new Access code drops.
 */
#include <Common/Libraries/HTTP/HS_HTTPLibApp.h>
#include <Common/Libraries/HTTP/HS_HTTPLibConst.h>

/* struct */

/* structure for glueing */

struct HS_HTTPLibSplit_ {
	Char *s;
	Int32 len;
};
typedef struct HS_HTTPLibSplit_ HS_HTTPLibSplit;

struct HS_HTTPLibTime_ {
	Int32 day;	/* days (either backward or forward) since 01 January, 1970 UTC */
	Int32 msec;	/* milliseconds since midnight [0-86399999] */
};
typedef struct HS_HTTPLibTime_ HS_HTTPLibTime;

typedef void *HS_HTTPLibDataHandle;
typedef void *HS_HTTPLibVHandle;
typedef void *HS_HTTPLibString;
typedef void *HS_HTTPLibFixedString;
typedef void *HS_HTTPLibURLString;

struct HS_HTTPLibAuthInfo_ {
	HS_HTTPLibString fUser;
	HS_HTTPLibString fPass;
	HS_HTTPLibString fRealm;
	HS_HTTPLibString fChallenge;
	HS_HTTPLibString fNextNonce;
	Int32 fNC;
};
typedef struct HS_HTTPLibAuthInfo_ HS_HTTPLibAuthInfo;

/* This sets the rules for HttpLib to use to select useragent for each request.
 * The rule should be set as little as possible so that it does not slow down
 * browser performance.
 */
enum {
	httpUASelectTypeHost,
	httpUASelectTypePath
};

typedef struct {
	Int32 fType;
	Char *fMatchString; /* substring to be matched */
	Char *fUserAgent;
} HttpUASelectionRule;


typedef void *HS_HTTPLibStream;
typedef void *HS_HTTPLibSSL;
typedef void *HS_HTTPLibCookies;
typedef void *HS_HTTPLibCertPtr;
typedef void *HS_HTTPLibCertList;
typedef void *HS_HTTPLibClientCertList;
typedef void *HS_HTTPLibSSLClassPtr;

typedef Int32 (*HS_HTTPLibCookieConfirmProc)(HS_HTTPLibCookies in_cookie, HS_HTTPLibURLString in_url, Char *in_header, Int32 in_header_len, HS_HTTPLibOpaque in_aux);
typedef Int32 (*HS_HTTPLibServerCertConfirmProc)(HS_HTTPLibSSL in_ssl, Int32 in_verify_result, HS_HTTPLibCertList in_list, HS_HTTPLibOpaque in_aux);
typedef Int32 (*HS_HTTPLibClientCertSelectProc)(HS_HTTPLibSSL in_ssl, HS_HTTPLibClientCertList in_list, HS_HTTPLibOpaque in_aux);
typedef Int32 (*HS_HTTPLibTunnelingCallbackProc)(HS_HTTPLibStream stream, Char* buf, Int32 len, void *in_aux);
typedef void (*HS_HTTPLibWakeUpCallbackProc)(HS_HTTPLibOpaque in_aux);

/* const */

/* API */
Err HS_HTTPLibOpen(UInt16 refnum)
		SYS_TRAP(kHTTPLibTrapOpen);
Err HS_HTTPLibClose(UInt16 refnum, UInt16* usecountP)
		SYS_TRAP(kHTTPLibTrapClose);
Err HS_HTTPLibSleep(UInt16 refnum)
		SYS_TRAP(kHTTPLibTrapSleep);
Err HS_HTTPLibWake(UInt16 refnum)
		SYS_TRAP(kHTTPLibTrapWake);
UInt16 HS_HTTPLibOpenCount(UInt16 refnum)
		SYS_TRAP(kHTTPLibTrapOpenCount);
HS_HTTPLibHandle HS_HTTPLibInitialize(UInt16 refnum, HS_HTTPLibAppInfo *appInfoP, HS_HTTPLibNetLibInfo *netLibInfoP, HS_HTTPLibPeer *peerP)
		SYS_TRAP(kHTTPLibTrapInitialize);
void HS_HTTPLibFinalize(UInt16 refnum, HS_HTTPLibHandle libH)
		SYS_TRAP(kHTTPLibTrapFinalize);
HS_HTTPLibSSLClassPtr HS_HTTPLibSSLClass(UInt16 refnum, HS_HTTPLibHandle libH)
		SYS_TRAP(kHTTPLibTrapSSLClass);
void HS_HTTPLibRegisterSSLClass(UInt16 refnum, HS_HTTPLibHandle libH, HS_HTTPLibSSLClassPtr sslClassP)
		SYS_TRAP(kHTTPLibTrapRegisterSSLClass);
void HS_HTTPLibClearDNSCache(UInt16 refnum, HS_HTTPLibHandle libH)
		SYS_TRAP(kHTTPLibTrapClearDNSCache);
void HS_HTTPLibCloseAllKeepAlive(UInt16 refnum, HS_HTTPLibHandle libH)
		SYS_TRAP(kHTTPLibTrapCloseAllKeepAlive);
void HS_HTTPLibMakeProxy(UInt16 refnum, HS_HTTPLibHandle libH, HS_HTTPLibFixedString *out_proxy_host, Int32 *out_proxy_http_port, Int32 *out_proxy_https_port, HS_HTTPLibString *out_no_proxy_hosts)
		SYS_TRAP(kHTTPLibTrapMakeProxy);
void HS_HTTPLibSetHTTPPort(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_port)
		SYS_TRAP(kHTTPLibTrapSetHTTPPort);
void HS_HTTPLibSetHTTPSPort(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_port)
		SYS_TRAP(kHTTPLibTrapSetHTTPSPort);
void HS_HTTPLibSetMaxRequestHeader(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_size)
		SYS_TRAP(kHTTPLibTrapSetMaxRequestHeader);
void HS_HTTPLibSetMaxRequestBody(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_size)
		SYS_TRAP(kHTTPLibTrapSetMaxRequestBody);
Boolean HS_HTTPLibSetProxy(UInt16 refnum, HS_HTTPLibHandle libH, Char *in_proxy_host, Int32 in_proxy_host_len, Int32 in_proxy_http_port, Int32 in_proxy_https_port, Char *in_no_proxy_hosts, Int32 in_no_proxy_hosts_len)
		SYS_TRAP(kHTTPLibTrapSetProxy);
Boolean HS_HTTPLibSetUserAgent(UInt16 refnum, HS_HTTPLibHandle libH, Char *in_useragent, Int32 in_useragent_len)
		SYS_TRAP(kHTTPLibTrapSetUserAgent);
Boolean HS_HTTPLibSetDefaultHeaders(UInt16 refnum, HS_HTTPLibHandle libH, Char *in_default_headers, Int32 in_default_headers_len)
		SYS_TRAP(kHTTPLibTrapSetDefaultHeaders);
void HS_HTTPLibSetUseProxy(UInt16 refnum, HS_HTTPLibHandle libH, Boolean in_bool)
		SYS_TRAP(kHTTPLibTrapSetUseProxy);
void HS_HTTPLibSetHTTP11OverProxy(UInt16 refnum, HS_HTTPLibHandle libH, Boolean in_bool)
		SYS_TRAP(kHTTPLibTrapSetHTTP11OverProxy);
void HS_HTTPLibSetSendReferer(UInt16 refnum, HS_HTTPLibHandle libH, Boolean in_bool)
		SYS_TRAP(kHTTPLibTrapSetSendReferer);
void HS_HTTPLibSetSendProxyKeepAlive(UInt16 refnum, HS_HTTPLibHandle libH, Boolean in_bool)
		SYS_TRAP(kHTTPLibTrapSetSendProxyKeepAlive);
void HS_HTTPLibSetSendCookie(UInt16 refnum, HS_HTTPLibHandle libH, Boolean in_bool)
		SYS_TRAP(kHTTPLibTrapSetSendCookie);
void HS_HTTPLibSetConnectTimeOut(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_msec)
		SYS_TRAP(kHTTPLibTrapSetConnectTimeOut);
void HS_HTTPLibSetReqTimeOut(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_msec)
		SYS_TRAP(kHTTPLibTrapSetReqTimeOut);
void HS_HTTPLibSetRspTimeOut(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_msec)
		SYS_TRAP(kHTTPLibTrapSetRspTimeOut);
void HS_HTTPLibSetTunnelingCallback(UInt16 refnum, HS_HTTPLibHandle libH, HS_HTTPLibTunnelingCallbackProc in_cb, void *in_aux)
        SYS_TRAP(kHTTPLibTrapSetTunnelingCallback);
Boolean HS_HTTPLibIsUseProxy(UInt16 refnum, HS_HTTPLibHandle libH)
		SYS_TRAP(kHTTPLibTrapIsUseProxy);
Boolean HS_HTTPLibIsHTTP11OverProxy(UInt16 refnum, HS_HTTPLibHandle libH)
		SYS_TRAP(kHTTPLibTrapIsHTTP11OverProxy);
Boolean HS_HTTPLibIsSendReferer(UInt16 refnum, HS_HTTPLibHandle libH)
		SYS_TRAP(kHTTPLibTrapIsSendReferer);
Boolean HS_HTTPLibIsSendCookie(UInt16 refnum, HS_HTTPLibHandle libH)
		SYS_TRAP(kHTTPLibTrapIsSendCookie);
HS_HTTPLibStream HS_HTTPLibStreamNew(UInt16 refnum, HS_HTTPLibHandle libH)
		SYS_TRAP(kHTTPLibTrapStreamNew);
void HS_HTTPLibStreamDelete(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapStreamDelete);
Int32 HS_HTTPLibStreamCreateRequest(UInt16 refnum, HS_HTTPLibStream stream, Int32 in_method, Char *in_url, UInt32 in_url_len,Char *in_header, UInt32 in_header_len,Char *in_referer, UInt32 in_referer_len, Int32 in_version, Boolean in_no_cache,Boolean in_pipeline, Int32 in_ssl_flag)
		SYS_TRAP(kHTTPLibTrapStreamCreateRequest);
Int32 HS_HTTPLibStreamCreateRequestWithChunkEncoding(UInt16 refnum, HS_HTTPLibStream stream, Int32 in_method, Char *in_url, UInt32 in_url_len,Char *in_header, UInt32 in_header_len,Char *in_referer, UInt32 in_referer_len, Int32 in_version, Boolean in_no_cache,Boolean in_pipeline, Int32 in_ssl_flag)
		SYS_TRAP(kHTTPLibTrapStreamCreateRequestWithChunkEncoding);
Int32 HS_HTTPLibStreamSendRequest(UInt16 refnum, HS_HTTPLibStream stream,Int32 *out_sleep)
		SYS_TRAP(kHTTPLibTrapStreamSendRequest);
Int32 HS_HTTPLibStreamSendRequestWithPostData(UInt16 refnum, HS_HTTPLibStream stream, Char *in_buf, Int32 in_len, Int32 *out_len, Int32 *out_sleep)
		SYS_TRAP(kHTTPLibTrapStreamSendRequestWithPostData);
Int32 HS_HTTPLibStreamSendRequestWithChunkEncoding(UInt16 refnum, HS_HTTPLibStream stream, Char *in_buf, Int32 in_offset, Int32 in_len, Boolean isFinish, Int32 *out_len, Int32 *out_sleep)
		SYS_TRAP(kHTTPLibTrapStreamSendRequestWithChunkEncoding);
Int32 HS_HTTPLibStreamReceiveResponse(UInt16 refnum, HS_HTTPLibStream stream, Char *out_buf, Int32 in_len, Int32 *out_len, Int32 *out_sleep)
		SYS_TRAP(kHTTPLibTrapStreamReceiveResponse);
Int32 HS_HTTPLibStreamGetFlag(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapStreamGetFlag);
void HS_HTTPLibStreamSuspendTimer(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapStreamSuspendTimer);
void HS_HTTPLibStreamResumeTimer(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapStreamResumeTimer);
void HS_HTTPLibStreamClose(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapStreamClose);
Boolean HS_HTTPLibStreamIsHeaderReceived(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapStreamIsHeaderReceived);
Int32 HS_HTTPLibStreamSetPostData(UInt16 refnum, HS_HTTPLibStream stream, Int32 in_encoding, Char *in_data, Int32 in_len)
		SYS_TRAP(kHTTPLibTrapStreamSetPostData);
Int32 HS_HTTPLibStreamGetScheme(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapStreamGetScheme);
Int32 HS_HTTPLibStreamGetState(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapStreamGetState);
Int32 HS_HTTPLibStreamGetStatusCode(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapStreamGetStatusCode);
Int32 HS_HTTPLibStreamGetResponseHeaderLength(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapStreamGetResponseHeaderLength);
HS_HTTPLibVHandle HS_HTTPLibStreamGetResponseHeader(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapStreamGetResponseHeader);
Int32 HS_HTTPLibStreamGetContentLength(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapStreamGetContentLength);
Int32 HS_HTTPLibStreamGetEntityLength(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapStreamGetEntityLength);
Boolean HS_HTTPLibStreamAddHeaderLine(UInt16 refnum, HS_HTTPLibStream stream, Char *in_str, Int32 in_len)
		SYS_TRAP(kHTTPLibTrapStreamAddHeaderLine);
Boolean HS_HTTPLibStreamAddHeader(UInt16 refnum, HS_HTTPLibStream stream, Int32 in_type, Char *in_str, Int32 in_len)
		SYS_TRAP(kHTTPLibTrapStreamAddHeader);
Boolean HS_HTTPLibStreamGetHeader(UInt16 refnum, HS_HTTPLibStream stream, Int32 in_type, Int32 *out_off, Int32 *out_len)
		SYS_TRAP(kHTTPLibTrapStreamGetHeader);
HS_HTTPLibCertList HS_HTTPLibMakeCertList(UInt16 refnum, HS_HTTPLibStream stream)
		SYS_TRAP(kHTTPLibTrapMakeCertList);
Boolean HS_HTTPLibHeaderSplitByX(UInt16 refnum, HS_HTTPLibSplit *inout_ss, Int32 in_id, Char *in_cs)
		SYS_TRAP(kHTTPLibTrapHeaderSplitByX);
void HS_HTTPLibAuthInfoTidy(UInt16 refnum, HS_HTTPLibAuthInfo *in_info, Boolean in_new)
		SYS_TRAP(kHTTPLibTrapAuthInfoTidy);
Boolean HS_HTTPLibSetUserPass(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_type, Int32 in_target, HS_HTTPLibSplit *in_challenge, HS_HTTPLibSplit *in_realm, Char *in_user, Int32 in_user_len, Char *in_pass, Int32 in_pass_len, Char *in_url, Int32 in_url_len, Char *in_host, Int32 in_host_len, Int32 in_port)
		SYS_TRAP(kHTTPLibTrapSetUserPass);
Boolean HS_HTTPLibFindAuthInfo(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_target, Char *in_url, Int32 in_url_len, Char *in_host, Int32 in_host_len, Int32 in_port, HS_HTTPLibSplit *in_realm, HS_HTTPLibAuthInfo *out_info)
		SYS_TRAP(kHTTPLibTrapFindAuthInfo);
Boolean HS_HTTPLibMakeAuthInfo(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_target, Char *in_url, Int32 in_url_len, Char *in_host, Int32 in_host_len, Int32 in_port, HS_HTTPLibSplit *in_realm, HS_HTTPLibAuthInfo *out_info)
		SYS_TRAP(kHTTPLibTrapMakeAuthInfo);
HS_HTTPLibString HS_HTTPLibAuthInfoUser(UInt16 refnum, HS_HTTPLibAuthInfo *in_info)
		SYS_TRAP(kHTTPLibTrapAuthInfoUser);
HS_HTTPLibString HS_HTTPLibAuthInfoPass(UInt16 refnum, HS_HTTPLibAuthInfo *in_info)
		SYS_TRAP(kHTTPLibTrapAuthInfoPass);
HS_HTTPLibString HS_HTTPLibAuthInfoRealm(UInt16 refnum, HS_HTTPLibAuthInfo *in_info)
		SYS_TRAP(kHTTPLibTrapAuthInfoRealm);
Boolean HS_HTTPLibAuthCacheGetChallengeTokenSS(UInt16 refnum, HS_HTTPLibSplit *in_challenge_ss, Char *in_token, HS_HTTPLibSplit *out_ss)
		SYS_TRAP(kHTTPLibTrapAuthCacheGetChallengeTokenSS);
void HS_HTTPLibSetCookieMode(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_mode, HS_HTTPLibCookieConfirmProc in_cb, HS_HTTPLibOpaque in_opaque)
		SYS_TRAP(kHTTPLibTrapSetCookieMode);
void HS_HTTPLibSetCookieMax(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_max_total_cookies, Int32 in_max_cookies_per_domain, Int32 in_max_len_per_cookie)
		SYS_TRAP(kHTTPLibTrapSetCookieMax);
Int32 HS_HTTPLibSetCookie(UInt16 refnum, HS_HTTPLibHandle libH, HS_HTTPLibURLString in_url, Char *in_header, Int32 in_header_len)
		SYS_TRAP(kHTTPLibTrapSetCookie);
Int32 HS_HTTPLibSetCookieX(UInt16 refnum, HS_HTTPLibHandle libH, HS_HTTPLibURLString in_url, Char *in_header, Int32 in_header_len)
		SYS_TRAP(kHTTPLibTrapSetCookieX);
HS_HTTPLibString HS_HTTPLibMakeCookiesString(UInt16 refnum, HS_HTTPLibHandle libH, Char *in_url, Int32 in_urllen, Boolean in_secure, Int32 *out_len)
		SYS_TRAP(kHTTPLibTrapMakeCookiesString);
HS_HTTPLibString HS_HTTPLibSaveCookiesEntries(UInt16 refnum, HS_HTTPLibHandle libH, Int32 *out_len)
		SYS_TRAP(kHTTPLibTrapSaveCookiesEntries);
Boolean HS_HTTPLibLoadCookiesEntries(UInt16 refnum, HS_HTTPLibHandle libH, Char *in_s, Int32 in_slen)
		SYS_TRAP(kHTTPLibTrapLoadCookiesEntries);
void HS_HTTPLibCookieMakeEmpty(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_flag)
		SYS_TRAP(kHTTPLibTrapCookieMakeEmpty);
void HS_HTTPLibSetDNSMaxRetry(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_retry)
		SYS_TRAP(kHTTPLibTrapSetDNSMaxRetry);
void HS_HTTPLibSetDNSRetryInterval(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_interval)
		SYS_TRAP(kHTTPLibTrapSetDNSRetryInterval);
void HS_HTTPLibSetSSLVersionFlag(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_flag, Boolean in_on)
		SYS_TRAP(kHTTPLibTrapSetSSLVersionFlag);
Int32 HS_HTTPLibSSLTimeout(UInt16 refnum, HS_HTTPLibHandle libH)
		SYS_TRAP(kHTTPLibTrapSSLTimeout);
Int32 HS_HTTPLibSSLOpenCertDB(UInt16 refnum, HS_HTTPLibHandle libH, HS_HTTPLibCertList *out_list)
		SYS_TRAP(kHTTPLibTrapSSLOpenCertDB);
void HS_HTTPLibSSLCloseCertDB(UInt16 refnum, HS_HTTPLibHandle libH)
		SYS_TRAP(kHTTPLibTrapSSLCloseCertDB);
HS_HTTPLibCertList HS_HTTPLibCertListNew(UInt16 refnum, Int32 in_type)
		SYS_TRAP(kHTTPLibTrapCertListNew);
void HS_HTTPLibCertListDelete(UInt16 refnum, HS_HTTPLibCertList in_list)
		SYS_TRAP(kHTTPLibTrapCertListDelete);
HS_HTTPLibCertList HS_HTTPLibCertListClone(UInt16 refnum, HS_HTTPLibCertList in_list)
		SYS_TRAP(kHTTPLibTrapCertListClone);
void *HS_HTTPLibCertListLock(UInt16 refnum, HS_HTTPLibCertList in_list)
		SYS_TRAP(kHTTPLibTrapCertListLock);
void HS_HTTPLibCertListUnlock(UInt16 refnum, HS_HTTPLibCertList in_list)
		SYS_TRAP(kHTTPLibTrapCertListUnlock);
Int32 HS_HTTPLibCertListType(UInt16 refnum, HS_HTTPLibCertList in_list)
		SYS_TRAP(kHTTPLibTrapCertListType);
Int32 HS_HTTPLibCertListLength(UInt16 refnum, HS_HTTPLibCertList in_list)
		SYS_TRAP(kHTTPLibTrapCertListLength);
Int32 HS_HTTPLibCertListBeginDecode(UInt16 refnum, HS_HTTPLibCertList in_list, Int32 in_index, HS_HTTPLibCertPtr *out_cert)
		SYS_TRAP(kHTTPLibTrapCertListBeginDecode);
void HS_HTTPLibCertListEndDecode(UInt16 refnum, HS_HTTPLibCertList in_list, HS_HTTPLibCertPtr in_cert)
		SYS_TRAP(kHTTPLibTrapCertListEndDecode);
Int32 HS_HTTPLibCertListImportCert(UInt16 refnum, HS_HTTPLibCertList in_list, void *in_info, Boolean in_over_write)
		SYS_TRAP(kHTTPLibTrapCertListImportCert);
Int32 HS_HTTPLibCertPeekVersion(UInt16 refnum, HS_HTTPLibCertPtr in_cert)
		SYS_TRAP(kHTTPLibTrapCertPeekVersion);
Int32 HS_HTTPLibCertPeekSerialNumber(UInt16 refnum, HS_HTTPLibCertPtr in_cert, Char **out_str)
		SYS_TRAP(kHTTPLibTrapCertPeekSerialNumber);
Int32 HS_HTTPLibCertPeekSignAlgo(UInt16 refnum, HS_HTTPLibCertPtr in_cert)
		SYS_TRAP(kHTTPLibTrapCertPeekSignAlgo);
Boolean HS_HTTPLibCertBeginPeekIssuerRDN(UInt16 refnum, HS_HTTPLibCertPtr in_cert, Int32 in_attr)
		SYS_TRAP(kHTTPLibTrapCertBeginPeekIssuerRDN);
Int32 HS_HTTPLibCertPeekIssuerRDN(UInt16 refnum, HS_HTTPLibCertPtr in_cert, Char **out_str)
		SYS_TRAP(kHTTPLibTrapCertPeekIssuerRDN);
Int32 HS_HTTPLibCertPeekIssuerRDNStrType(UInt16 refnum, HS_HTTPLibCertPtr in_cert)
		SYS_TRAP(kHTTPLibTrapCertPeekIssuerRDNStrType);
void HS_HTTPLibCertEndPeekIssuerRDN(UInt16 refnum, HS_HTTPLibCertPtr in_cert)
		SYS_TRAP(kHTTPLibTrapCert_EndPeekIssuerRDN);
Boolean HS_HTTPLibCertBeginPeekSubjectRDN(UInt16 refnum, HS_HTTPLibCertPtr in_cert, Int32 in_attr)
		SYS_TRAP(kHTTPLibTrapCertBeginPeekSubjectRDN);
Int32 HS_HTTPLibCertPeekSubjectRDN(UInt16 refnum, HS_HTTPLibCertPtr in_cert, Char **out_str)
		SYS_TRAP(kHTTPLibTrapCertPeekSubjectRDN);
Int32 HS_HTTPLibCertPeekSubjectRDNStrType(UInt16 refnum, HS_HTTPLibCertPtr in_cert)
		SYS_TRAP(kHTTPLibTrapCertPeekSubjectRDNStrType);
void HS_HTTPLibCertEndPeekSubjectRDN(UInt16 refnum, HS_HTTPLibCertPtr in_cert)
		SYS_TRAP(kHTTPLibTrapCert_EndPeekSubjectRDN);
void HS_HTTPLibCertPeekValidityStart(UInt16 refnum, HS_HTTPLibCertPtr in_cert, HS_HTTPLibTime *in_time)
		SYS_TRAP(kHTTPLibTrapCert_PeekValidityStart);
void HS_HTTPLibCertPeekValidityEnd(UInt16 refnum, HS_HTTPLibCertPtr in_cert, HS_HTTPLibTime *in_time)
		SYS_TRAP(kHTTPLibTrapCert_PeekValidityEnd);
Int32 HS_HTTPLibCertPeekRSAPublicKeyBits(UInt16 refnum, HS_HTTPLibCertPtr in_cert)
		SYS_TRAP(kHTTPLibTrapCert_PeekRSAPublicKeyBits);
HS_HTTPLibClientCertList HS_HTTPLibClientCertListNew(UInt16 refnum)
		SYS_TRAP(kHTTPLibTrapClientCertListNew);
HS_HTTPLibClientCertList HS_HTTPLibClientCertListClone(UInt16 refnum, HS_HTTPLibClientCertList in_list)
		SYS_TRAP(kHTTPLibTrapClientCertListClone);
void HS_HTTPLibClientCertListDelete(UInt16 refnum, HS_HTTPLibClientCertList in_list)
		SYS_TRAP(kHTTPLibTrapClientCertListDelete);
Int32 HS_HTTPLibClientCertListLength(UInt16 refnum, HS_HTTPLibClientCertList in_list)
		SYS_TRAP(kHTTPLibTrapClientCertListLength);
Boolean HS_HTTPLibClientCertListIsValid(UInt16 refnum, HS_HTTPLibClientCertList in_list, Int32 in_index)
		SYS_TRAP(kHTTPLibTrapClientCertListIsValid);
void HS_HTTPLibClientCertListSetValid(UInt16 refnum, HS_HTTPLibClientCertList in_list, Int32 in_index, Boolean in_valid)
		SYS_TRAP(kHTTPLibTrapClientCertListSetValid);
void HS_HTTPLibSetSSLServerCertConfirmProc(UInt16 refnum, HS_HTTPLibHandle libH, HS_HTTPLibServerCertConfirmProc in_cb, HS_HTTPLibOpaque in_opaque)
		SYS_TRAP(kHTTPLibTrapSetSSLServerCertConfirmProc);
void HS_HTTPLibServerCertConfirm(UInt16 refnum, HS_HTTPLibSSL in_ssl, Int32 in_verify_result, Boolean in_confirm)
		SYS_TRAP(kHTTPLibTrapServerCertConfirm);
void HS_HTTPLibSetSSLClientCertSelectProc(UInt16 refnum, HS_HTTPLibHandle libH, HS_HTTPLibClientCertSelectProc in_cb, HS_HTTPLibOpaque in_opaque)
		SYS_TRAP(kHTTPLibTrapSetSSLClientCertSelectProc);
void HS_HTTPLibClientCertSelect(UInt16 refnum, HS_HTTPLibSSL in_ssl, Int32 in_index)
		SYS_TRAP(kHTTPLibTrapClientCertSelect);
void HS_HTTPLibSetNetLibProc(UInt16 refnum, HS_HTTPLibHandle libH, UInt16 netLibRefnum, HS_HTTPLibNetLibOpenProc in_open, HS_HTTPLibNetLibCloseProc in_close, HS_HTTPLibNetLibOnlineProc in_online, HS_HTTPLibNetLibOfflineProc in_offline)
		SYS_TRAP(kHTTPLibTrapSetNetLibProc);
Int32 HS_HTTPLibDataHandle_Length(UInt16 refnum, HS_HTTPLibDataHandle in_handle)
		SYS_TRAP(kHTTPLibTrapDataHandle_Length);
void *HS_HTTPLibDataHandle_Lock(UInt16 refnum, HS_HTTPLibDataHandle in_handle)
		SYS_TRAP(kHTTPLibTrapDataHandle_Lock);
void HS_HTTPLibDataHandle_Unlock(UInt16 refnum, HS_HTTPLibDataHandle in_handle)
		SYS_TRAP(kHTTPLibTrapDataHandle_Unlock);
void HS_HTTPLibDataHandle_Delete(UInt16 refnum, HS_HTTPLibDataHandle in_handle)
		SYS_TRAP(kHTTPLibTrapDataHandle_Delete);
Int32 HS_HTTPLibVHandle_Length(UInt16 refnum, HS_HTTPLibVHandle in_vhandle)
		SYS_TRAP(kHTTPLibTrapVHandle_Length);
void *HS_HTTPLibVHandle_Lock(UInt16 refnum, HS_HTTPLibVHandle in_vhandle)
		SYS_TRAP(kHTTPLibTrapVHandle_Lock);
void HS_HTTPLibVHandle_Unlock(UInt16 refnum, HS_HTTPLibVHandle in_vhandle)
		SYS_TRAP(kHTTPLibTrapVHandle_Unlock);
void HS_HTTPLibVHandle_Delete(UInt16 refnum, HS_HTTPLibVHandle in_vhandle)
		SYS_TRAP(kHTTPLibTrapVHandle_Delete);
Int32 HS_HTTPLibString_Length(UInt16 refnum, HS_HTTPLibString in_str)
		SYS_TRAP(kHTTPLibTrapString_Length);
HS_HTTPLibString HS_HTTPLibStringNew(UInt16 refnum, Char *in_str, Int32 in_len)
		SYS_TRAP(kHTTPLibTrapStringNew);
void *HS_HTTPLibString_Lock(UInt16 refnum, HS_HTTPLibString in_str)
		SYS_TRAP(kHTTPLibTrapString_Lock);
void HS_HTTPLibString_Unlock(UInt16 refnum, HS_HTTPLibString in_str)
		SYS_TRAP(kHTTPLibTrapString_Unlock);
void HS_HTTPLibString_Delete(UInt16 refnum, HS_HTTPLibString in_str)
		SYS_TRAP(kHTTPLibTrapString_Delete);
Int32 HS_HTTPLibFixedString_Length(UInt16 refnum, HS_HTTPLibFixedString in_str)
		SYS_TRAP(kHTTPLibTrapFixedString_Length);
HS_HTTPLibFixedString HS_HTTPLibFixedStringNew(UInt16 refnum, Char *in_str, Int32 in_len)
		SYS_TRAP(kHTTPLibTrapFixedStringNew);
void *HS_HTTPLibFixedString_Lock(UInt16 refnum, HS_HTTPLibFixedString in_str)
		SYS_TRAP(kHTTPLibTrapFixedString_Lock);
void HS_HTTPLibFixedString_Unlock(UInt16 refnum, HS_HTTPLibFixedString in_str)
		SYS_TRAP(kHTTPLibTrapFixedString_Unlock);
void HS_HTTPLibFixedString_Delete(UInt16 refnum, HS_HTTPLibFixedString in_str)
		SYS_TRAP(kHTTPLibTrapFixedString_Delete);
Int32 HS_HTTPLibURLString_Length(UInt16 refnum, HS_HTTPLibURLString in_str)
		SYS_TRAP(kHTTPLibTrapURLString_Length);
HS_HTTPLibURLString HS_HTTPLibURLStringNew(UInt16 refnum, Char *in_str, Int32 in_len)
		SYS_TRAP(kHTTPLibTrapURLStringNew);
void *HS_HTTPLibURLString_Lock(UInt16 refnum, HS_HTTPLibURLString in_str)
		SYS_TRAP(kHTTPLibTrapURLString_Lock);
void HS_HTTPLibURLString_Unlock(UInt16 refnum, HS_HTTPLibURLString in_str)
		SYS_TRAP(kHTTPLibTrapURLString_Unlock);
void HS_HTTPLibURLString_Delete(UInt16 refnum, HS_HTTPLibURLString in_str)
		SYS_TRAP(kHTTPLibTrapURLString_Delete);
void *HS_HTTPLibCertListGetCipherInfo(UInt16 refnum, void *iself)
		SYS_TRAP(kHTTPLibTrapCertListGetCipherInfo);
void HS_HTTPLibSetMaxKeepAliveTimeout(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_msec)
        SYS_TRAP(kHTTPLibTrapSetMaxKeepAliveTimeout);
void HS_HTTPLibStreamGetRequestSent(UInt16 refnum, HS_HTTPLibStream stream, Int32 *out_total, Int32 *out_sent)
        SYS_TRAP(kHTTPLibTrapStreamGetRequestSent);
Boolean HS_HTTPLibStreamGetHeaderWithOffset(UInt16 refnum, HS_HTTPLibStream stream, Int32 in_type, Int32 in_ofs, Int32 *out_off, Int32 *out_len)
        SYS_TRAP(kHTTPLibTrapStreamGetHeaderWithOffset);
Boolean HS_HTTPLibSetUserPassX(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_type, Int32 in_target, HS_HTTPLibSplit *in_challenge, HS_HTTPLibSplit *in_realm, Char *in_user, Int32 in_user_len, Char *in_pass, Int32 in_pass_len, Char *in_url, Int32 in_url_len, Char *in_host, Int32 in_host_len, Int32 in_port, Boolean in_keep_user, Boolean in_keep_pass)
        SYS_TRAP(kHTTPLibTrapSetUserPassX);
void HS_HTTPLibFindAuthCandidate(UInt16 refnum, HS_HTTPLibHandle libH, Int32 in_target, Char *in_url, Int32 in_url_len, Char *in_host, Int32 in_host_len, Int32 in_port, Char *in_realm, Int32 in_realm_len, HS_HTTPLibString *out_user, HS_HTTPLibString *out_pass, Boolean *out_keep_user, Boolean *out_keep_pass)
        SYS_TRAP(kHTTPLibTrapFindAuthCandidate);
void HS_HTTPLibSetWakeUpCallback(UInt16 refnum, HS_HTTPLibHandle libH, HS_HTTPLibWakeUpCallbackProc in_proc, HS_HTTPLibOpaque in_opaque)
        SYS_TRAP(kHTTPLibTrapSetWakeUpCallback);
Boolean HS_HTTPLibSetUseragentSelectionRule(UInt16 refnum, HS_HTTPLibHandle libH, HttpUASelectionRule *rules, Int32 len)
		SYS_TRAP(kHTTPLibTrapSetUseragentSelectionRule);



#endif /* HS_HTTPLIB68K_H__ */
