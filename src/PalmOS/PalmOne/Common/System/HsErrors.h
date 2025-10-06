/**
 * \file HsErrors.h
 * Errors that are not part of any particular component but can be returned by
 * many different system components. 
 *
 * \license
 *
 *    Copyright (c) 2002 Handspring Inc., All Rights Reserved
 *
 * \author Kiran Prasad
 *
 * $Id: //device/handheld/dev/sdk/2.1/incs/common/system/HsErrors.h#4 $
 *
 *****************************************************************************/

#ifndef __HS_ERRORS_H__
#define __HS_ERRORS_H__

#include <Common/HsCommon.h>


#if 0
#pragma mark -------- Radio Errors -------- 
#endif
/**
 * The radio is currently off and you can not perform the action requested.
 **/
#define hsErrRadioOff					  (hsOEMRadioErrorClass | 0x01)

/**
 * The radio is currently not registered and you can not perform the action
 * reuqested.
 **/
#define	hsErrRadioNotRegistered			  (hsOEMRadioErrorClass | 0x02)

/**
 * The radio is currently on an active voice call and so your request can not be
 * handled.
 **/
#define hsErrVoiceCallActive			  (hsOEMRadioErrorClass | 0x03)

/**
 * For GSM Radio Only,
 *   Your SIM is not ready to perform this action. This can either mean there is
 *   no SIM or it is not fully read yet.
 **/
#define hsErrSimNotReady				  (hsOEMRadioErrorClass | 0x04)

/**
 * Your Radio is currently locked and therefore we can perform the action
 * requested.
 **/
#define hsErrPhoneLocked				  (hsOEMRadioErrorClass | 0x05)

/**
 * Phone is not activated. 
 **/
#define hsErrPhoneNotActivated            (hsOEMRadioErrorClass | 0x06)

#endif
