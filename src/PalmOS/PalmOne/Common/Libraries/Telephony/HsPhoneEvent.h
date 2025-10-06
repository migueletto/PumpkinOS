/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */


/**
 *
 * @file 	HsPhoneEvent.h
 *
 * @brief  Header file for the phone library (CDMA or GSM)
 *
 * NOTES:
 * Header file for the phone library (CDMA or GSM)
 *
 * This file defines the Notification Events between the Phone Library
 * (either GSM or CDMA) and applications.  These events allow applications
 * to react to changes of state in the Phone Library.
 *
 * Applications wishing to receive notification events from the Phone Library
 * must register using the PhnLibRegister call.  The services parameter to
 * PhnLibRegister is a bitmap identifying the classes of events that are of
 * interest.  To unregister for notifications the app can call PhnLibRegister
 * with the services parameter set to zero.
 *
 * Notifications are sent to applications based on their unique creatorID.
 * Confusion may occur if multiple versions of a single app exist with the same
 * creatorID.  In this case notifications will be sent to the latest version of
 * the app (even if this app was not the one that registered).  In general,
 * you should try to avoid this situation where possible.
 *
 * Each notification event is identified by a unique event code (see PhnEventCode).
 * The event code then determines the contents of the remainder of the
 * event.
 *
 * There are three styles of notifications from the Phone Library:
 *
 *  (1) universal broadcast notifications are sent to all registrants for
 *      a given service.  (for example, a change in the network registration
 *      status)
 *
 *  (2) exclusive broadcast notifications are sent to registrations one at a time.
 *      The first app that acknowledges the notification by setting the
 *      acknowedge flag will be presumed to own that notification.  Only this
 *      app should handle and respond to the notification.  For example, an
 *      incoming call or incoming SMS message will be acknowleged by an app
 *      that wants to answer the call or process the SMS message.
 *
 *  (3) direct notifications are sent directly to a specific app with a known
 *      interest in some occurence in the phone library.  These are typically
 *      used after an application has taken ownership of a particular call,
 *      message, or dialog.  By sending notifications directly to the owning
 *      app we avoid confusion with multiple apps sending competing instructions.
 */

#ifndef HS_PHONEEVENT_H
#define HS_PHONEEVENT_H
#include <PalmOS.h>
#include <PalmTypes.h>
#ifndef __CORE_COMPATIBILITY_H__
#include <PalmCompatibility.h>
	// workaround for differing header files in sdk-3.5 and sdk-internal
	#ifndef __CORE_COMPATIBILITY_H__
	#define __CORE_COMPATIBILITY_H__
	#endif
#endif

// this is not a good struct
//#define USE_PHNSMSMESSAGETYPE

//Keep it same as defined in HICMSMS.h
#define phnAddressDigitsMax	32
#define phnUserDataMax		255

// Notification Types
#define phnNotifySubscriber 'CLIP'  		/**< GSM notification with caller ID  */
#define phnNotifyEquipMode	'Heqp'  	/**< GSM notificaiton for equip mode change  */

// Notification of a phone event
#define	phnLibLaunchCmdEvent	 	0xabad	/**< Phone Event notification. Used
						     to be 0x2bad, which is less than
						     Palm's custom base. Have to get
						     Palm to allow that.	 */

#define phnLibLaunchCmdRegister		0xabae	  /**< Sent to application to register with GSM library  */


/**
 * unique identifier for Indication notification.   This value will be
 * contained in PhnEventType notification events of phnEvtIndication type.
 **/
typedef enum {
	indicationSIMReady, 		/**<		*/
	indicationSIMMessages, 		/**<		*/
    indicationNetworkSearch,    	/**< to display Network search banner */
	indicationPasswordAccepted,	/**<		*/
	indicationNetworkAvailable,	/**<		*/
	indicationStartingRadio,    	/**< to update UI from radio being off */
	indicationPoweringOffRadio, 	/**< notify app that the radio is turning off. */
	indicationResettingRadio,   	/**< tell the app that the radio is resetting. */
	indicationAuthFailure,		/**< notify app that network authentication failed. */
	indicationSIMChanged,		/**< sent when the GSM library detects that the IMSI value has changed */
	indicationInitComplete,		/**< sent when the GSM library is done with it initialization when the radio is powered on */
  indicationOtaspMsg 			/**< sent otasp msg from network */
} PhnIndicationKind;

#define isValidPhnIndicationKind(i) ((i >= indicationSIMReady) && (i <= indicationSIMChanged))	/**<		*/

/**
 * PhnEventCode is unique identifier for notifications between the Phone Library
 * and applications.  It is used in the eventType field in the header of notification
 * events (PhnEventType).
 **/
typedef enum
  {
  phnEvtCardInsertion,      /**< 0x0000 < NOT USED.  OBSOLETE */

  /** Network Registration has changed.
   *
   *  The Phone Library is responsible for registering the device to
   *  the carrier network, including handling roaming and losses of service.
   *  This notification occurs when the network registration state has changed.
   *  See PhnRegistrationStatus for possible state values.
   *
   *  This notification will be universally broadcast to applications
   * registered for any service.
   */
  phnEvtRegistration,       /**< 0x0001 < Phone able to find service  */

  /** Error detected
   *
   *  This notification occurs on various error scenarios in the Phone Library.
   *  These may occur when unexpected errors are reported from the radio, or when
   *  an operation in progress has failed.
   *
   *  This notification is universally broadcast to applications in the
   *  service area where the error was detected (e.g. Voice, SMS, ...).
   */
  phnEvtError,              /**< 0x0002 < indicator of something important happens to the phone that needs to bring up alert */

  /** phone-related keypress has occured on device
   *
   *  This notification occurs when the user presses a phone-related key on the device.
   *  Currently the only supported key is the headset button that can be used for
   *  basic call control.  In general, buttons available will be device dependent.
   *
   *  This notification is exclusively broadcast to all applications registered for the
   *  Voice Service.
   */
  phnEvtKeyPress,           /**< 0x0003 < Headset button pressed. */

  /** change in power state of radio
   *
   *  This notification occurs when the power state of the radio is changing.  Applications
   *  may want to change their state, and take action based on the expected availability
   *  of the radio.
   *
   *  This notification is universally broadcast to all registered applications.
   */
  phnEvtPower,              /**< 0x0004  */

  /** Password Dialog Control
   *
   *  This notification helps an application control the entry of a PIN on a
   *  device that is locked.
   *
   *  This notification is exclusively broadcast to all registered applications.
   *  Application that acknowledges the notification is presumed to own handling
   *  the password situation.
   */
  phnEvtPassword,           /**< 0x0005  */

  /** outgoing call progress dialog
   *
   *  This notification is sent to help applications display the progress of an
   *  oubound call operation.
   *
   *  This notification is exclusively broadcast to all registered applications.
   *  Application that acknowledges the notification is presumed to be the only one
   *  to take any action on the outgoing call.
   */
  phnEvtProgress,           /**< 0x0006  */

  /** indication of various changes of state in Phone Library
   *
   *  This notification is sent on various changes of state in the Phone Library.
   *  Most of these are related to activation and registration with the carrier
   *  network.  Applications that display gauges with state info will often need
   *  to be updated after this notification.
   *
   *  Indications are often used to direct GUI interactions in applications
   *  that interact with the user.  For example, the Network Search notification
   *  might cause an app to put up a special Network Search dialog.
   *
   *  See PhnIndicationKind for list of available indications.
   *
   *  The broadcast rules for indications vary depending on the type of indication.
   */
  phnEvtIndication,         /**< 0x0007  */

  /** incoming call received
   *
   *  This notification is exclusively broadcast to applications registered for
   *  the Voice Service.  First application to acknowledge the notification will
   *  be presumed to control the call.  Normally application would answer the call,
   *  or forward it to another destination.
   *
   */
  phnEvtConnectInd,         /**< 0x0008  */

  /** call has connected
   *
   *  Despite the name, this event is used BOTH when a call goes into
   *  connected state (e.g. is answered), and when a call joins a conference.
   *
   *  This notification is exclusively broadcast to applications registered for
   *  the Voice Service.  First application to acknowledge the notification will
   *  be presumed to control the call or conference.
   */
  phnEvtConnectConf,        /**< 0x0009  */

  /** Number/name of call has changed
   *
   *  This notification is universally broadcast to all applications registered for
   *  the Voice Service.
   */
  phnEvtSubscriber,         /**< 0x000A  */

  /** call has disconnected
   *
   *  This notification occurs when the remote party of an active voice call
   *  disconnects.  Note that if the call is in conference (or 3-way calling) mode, then
   *  a different phnEvtDisconnectConf notification will be sent instead (see below).
   *
   *  This notification is universally broadcast to all applications registered for
   *  the Voice or Data Services.
   */
  phnEvtDisconnectInd,      /**< 0x000B  */

  /** call in conference (or 3-way calling) mode has disconnected
   *
   *  This notification occurs when a conference call is in progress, and one of the
   *  remote parties disconnects.
   *
   *  This notification is universally broadcast to all applications registered for
   *  the Voice Service.
   */
  phnEvtDisconnectConf,     /**< 0x000C An ACK for a disconnection command on a specific connection ID is received */

  /** Outbound call attempt failed because remote party (or network) reported busy
   *
   *  This notification is universally broadcast to all applications registered for
   *  the Voice Service.
   */
  phnEvtBusy,               /**< 0x000D  */

  /** Change in Call Status;  Application may need to update its state
   *
   *  This notification is sent on various changes in call state in the Phone Library.
   *  An application receiving this notification should poll for the connection status
   *  of any calls of interest, and update its own state as needed.
   *
   *  This notification is universally broadcast to all applications registered
   *  for the Voice Service.
   *********************************************************************/
  phnEvtUpdate,             /**< 0x000E  */

  /** 3-way call mode
   *
   *  This notification occurs when a call goes into conferrence (or 3-way calling) mode.
   *
   *  This notification is universally broadcast to all applications registered
   *  for the Voice Service.
   */
  phnEvtConference,         /**< 0x000F  */

  /** Incoming VoiceMail Message
   *
   *  This notification occurs when the Phone Library detects that there are
   *  pending VoiceMail messages available.
   *
   *  This notification is universally broadcast to all applications registered
   *  for the Voice Service.
   */
  phnEvtVoiceMail,          /**< 0x0010  */

  /** Incoming SMS message received
   *
   * This notification occurs when the Phone Library receives an
   * incoming SMS (Simple Message Service) message.
   *
   * This notification will be exclusively broadcast to applications
   * registered for the SMS service.  The Application that acknowledges
   * the notification is assumed to own the message, and should take
   * any appropriate actions.
   */
  phnEvtMessageInd,         /**< 0x0011  */

  /** Incoming segment of SMS Message received
   *
   * This notification occurs when the Phone Library receives a segment
   * of a multi-segment SMS (Simple Message Service) message.  Single
   * segment SMS messages will generate a MessageInd notification (see
   * above).
   *
   * This notification will be exclusively broadcast to applications
   * registered for the SMS service.  The Application that acknowledges
   * the notification is assumed to own the segment, and should take
   * any appropriate actions.
   */
  phnEvtSegmentInd,         /**< 0x0012  */

  /** Status of outgoing SMS message has changed
   *
   * This notification occurs when the status of a particular SMS
   * message has changed.  For example, an SMS message might move from
   * the pending queue to the sent queue after being successfully
   * delivered.
   *
   * This notification will be sent directly to the application that
   * created the message, and is NOT broadcast based on app registrations.
   */
  phnEvtMessageStat,        /**< 0x0013  */

  /** SMS message deleted from local DB on device
   *
   * This notification occurs when an SMS message is deleted from the
   * local Database on the device.
   *
   * This notification will be sent directly to the application that
   * created the message, and is NOT broadcast based on app registrations.
   */
  phnEvtMessageDel,         /**< 0x0014  */

  /** SIM Contains SMS Messages
   *
   *  This notification indicates that the SIM (Subscriber Information
   *  Module) on a GSM phone contains SMS (Simple Messaging Service)
   *  messages.  This is not an indication of new messages, just that
   *  messages are stored.  This notification is typically sent during
   *  Phone Library activation.  In many cases, the application may want
   *  to move the SMS messages to a local DB on the device, freeing memory
   *  for future incoming messages.
   *
   *  This notification will use exclusive broadcast to applications
   *  registered for the SMS service.  The application that acknowledges
   *  the notification is responsible for taking any action on moving
   *  the SMS messages on the SIM (if desired).
   */
  phnEvtMessageMoved,       /**< 0x0015  */

  /** SIM Application Toolkit (SAT) event
   *
   * The SIM Application Toolkit allows for simple applications that
   * can be placed on the SIM (Subscriber Identity Module) in a GSM
   * phone.  This notification will be sent when a SAT application is
   * requesting input/output to/from the user.
   *
   * This notification will use exclusive broadcast to applications
   * registered for the SIM Toolkit Service.  The Application that first
   * acknowedges the notification will be presumed to own the SAT session,
   * and should handle needed user dialog as appropriate.
   */
  phnEvtSATNotification,    /**< 0x0016  */

  /** USSD input/output requested from network
   *
   * USSD (Unstructured Supplementary Services Data) is a supplementary
   * service to GSM networks that allows simple text dialogs
   * with applications based in the network.  This notification
   * is sent when the network is requesting USSD input/output to/from the user.
   * (see +CUSD event in radio documentation for more details)
   *
   * This notification will use exclusive broadcast to applications
   * registered for the Voice Service.  The Application that first
   * acknowledges the notification will be presumed to own the USSD session.
   * Typically, that app will want to display any USSD message to the user,
   * and handle any response from the user back to the network.
   */
  phnEvtUSSDInd,            /**< 0x0017  */

  /** Phone Equipment state has changed
   *
   * On communicator devices there are pieces of equipment that directly
   * impact how the radio is used.  Examples of equipment include headset
   * jacks, car kits, lids, etc.  The equipment modes available will vary
   * with the model of communicator being used.  This notification will be
   * sent when ever the Phone Library detects that an available equipment
   * mode has changed.  See the type PhnEquipmentMode for the available
   * set of mode values.
   *
   * This notification will be universally broadcast to all applications
   * registered for any service.
   */
  phnEvtPhoneEquipmentMode, /**< 0x0018  */

  /** GPRS Attach Registration has changed. (GSM/GPRS systems only)
   *
   *  GPRS (General Packet Radio Services) is an extension to the GSM
   *  wireless network that provides higher speed, always on data connections.
   *  The radio must attach to the GPRS network before any GPRS sessions
   *  can be established.  This notification will be sent when ever the
   *  status of the GPRS attach registration changes.   See type
   *  PhnRegistrationStatus for available registration states.
   *
   *  This notification will be universally broadcast to applications
   * registered for any service.
   */
  phnEvtGPRSRegistration,   /**< 0x0019  */

  /** incoming MMS message available
   *
   *  This notification will be sent when the Phone Library receives an
   *  incoming MMS (Multi-media Message Service) message.
   *
   *  This notification will use exclusive broadcast to applications registered
   *  for the MMS service.  Application that first acknowledges the notification
   *  will be presumed to own the message.  Typically, that app will want
   *  to notify the user that there is a message available, and move the
   *  message into a system DB, freeing memory for future messages.
   */
  phnEvtMMSInd,             /**< 0x001A  */

  /************************************************************
  * CDMA Events
  *************************************************************/


  /** SMS Memory Full
   *
   *  This notification indicates that memory for SMS messages is
   *  currently full, and no new SMS messages can be received until
   *  some memory is freed.  A phnEvtMemoryOK notification will be sent
   *  when memory becomes available.
   *
   *  CURRENTLY USED BY CDMA PHONE LIBRARY ONLY
   */
  phnEvtMemoryFull,         /**< 0x001B  */

  /** SMS Memory no longer full
   *
   *  This notification indicates that memory for SMS messages is
   *  no longer full, and new SMS messages can be received.
   *
   *  CURRENTLY USED BY CDMA PHONE LIBRARY ONLY
   */
  phnEvtMemoryOK,           /**< 0x001C - < Memory is not full anymore, can receive messages Description  */

  /** WAP Message received via SMS (WAP in IS-637 standard)
   *
   *  NOT CURRENTLY USED (WAP notifications use standard Palm notifications)
   */
  phnEvtWAPInd,		        /**< 0x001D  */

  /** Radio Debug Info
   *
   *  Periodically sent from Phone Library to Activation App when
   *  in debug mode.
   *
   *  CURRENTLY USED BY CDMA PHONE LIBRARY ONLY
   */
  phnEvtDebugReport,        /**< 0x001E  */

  /** System Selection mode has changed
   *
   *  Radio mode has changed (e.g. to PCS mode)
   *
   *  CURRENTLY USED BY CDMA PHONE LIBRARY ONLY
   */
  phnEvtSSModeChanged,      /**< 0x001F  */

  /** Position Info has changed
   *
   *  indication of change in physical location of device.  This may
   *  be determined by GPS, network triangulation (AFLT), or some other
   *  technique.
   *
   *  \TODO - rename this event & some of its data structures to be cleaner.
   *
   *  CURRENTLY USED BY CDMA PHONE LIBRARY ONLY
   */
  phnEvtPDDataChanged,      /**< 0x0020  */

  /** 1XRTT Status has changed
   *
   *  1XRTT is a high speed data protocol used in CDMA radios.  This
   *  notification indicates that the status of the 1XRTT connection
   *  has changed.
   *
   *  CURRENTLY USED BY CDMA PHONE LIBRARY ONLY
   */
  phnEvtOneXStatus,         /**< 0x0021  */

  /** Mobile IP connection failed
   *
   *  Mobile IP is a flavor of the TCP/IP protocol for use on mobile
   *  devices.  It allows applications to use a single IP address while
   *  in motion.  This notification occurs when a Mobile IP session
   *  fails to be established.
   *
   *  CURRENTLY USED BY CDMA PHONE LIBRARY ONLY
   */
  phnEvtMIPFailed,          /**< 0x0022  */

  /** IOTA Status has changed
   *
   *  Internet Over The Air (IOTA) is a protocol for a network to
   *  update the radio using an internet connection (as opposed to
   *  Over The Air (OTA)).  This notification occurs as an IOTA
   *  session changes (e.g. is established, committed, ended, etc).
   *
   *  CURRENTLY GENERATED BY CDMA PHONE LIBRARY ONLY
   */
  phnEvtIOTAStatus,         /**< 0x0023  */

  /** Data Channel availability has changed
   *
   *  This notification occurs if there is a change in the ability
   *  for an application to send/receive data using the radio.  For
   *  example on Class B radios, data is not available when a voice
   *  call is in progress.
   *
   * \TODO - this notification should be generated by GSM Phone Library too.
   *
   *  CURRENTLY GENERATED BY CDMA PHONE LIBRARY ONLY
   */
  phnEvtDataChannel,        /**< 0x0024  */

  /** Mobile IP Registration successful
   *
   *  Mobile IP is a flavor of the TCP/IP protocol for use on mobile
   *  devices.  It allows applications to use a single IP address while
   *  in motion.  This notification occurs when a Mobile IP session
   *  is successfully established.  The notification phnEvtMIPFailed
   *  will occur if the session fails.
   *
   *  CURRENTLY GENERATED BY CDMA PHONE LIBRARY ONLY
   */
  phnEvtOneXMipRRQInd,      /**< 0x0025  */

  /** 1XRTT Session has failed (mid-session)
   *
   *  1XRTT is a high speed data protocol used in CDMA radios.  This
   *  notification indicates that the 1XRTT session has failed, and
   *  is not available.  Note that 1XRTT session may be left in Dormant
   *  state.
   *
   *  CURRENTLY GENERATED BY CDMA PHONE LIBRARY ONLY
   */
  phnEvtOneXDataFail,       /**< 0x0026  */

  /** Start Dialing of Outbound Call
   *
   *   This notification is sent when an outbound call begins dialing before
   *   the call is connected.  This notification is universally broadcast to
   *   all applications registered for the Voice service.  Notification
   *   uses same notification data as Connect indication.
   */
  phnEvtStartDial,          /**< 0x0027  */

  /** Start of incoming call
   *
   *   This notification is sent when an incoming call if first detected.
   *   This notification is used to perform special pre-call processing
   *   like turning off of MMS playback before a call.  Any processing on
   *   this notification must be very quick (less than a few hundred miliseconds)
   *   as it could potentially delay an incoming call from being answered.
   *
   *   APPLICATIONS MUST NOT ANSWER THE CALL OR DO ANY CALL CONTROL BASED
   *   ON THIS NOTIFICATION.  A separate phnEvtConnectInd notification will
   *   be follow for this purpose.  The phnEvtConnectInd notification is normally
   *   processed by the Phone App, and uses exclusive broadcast.  Thus it may
   *   not be received by all applications.  The phnEvtStartIncomingCall
   *   notification is sent to all applications registered for Voice Service
   *   using Universal Broadcast.  The acknowledgement flag is NOT used.
   */
  phnEvtStartIncomingCall,  /**< 0x0028  */

  /** Alerting/Preconnected of Outbound Call
   *
   *   This notification is sent when an outbound call is alerting or
   *   preconnected.  This notification is universally broadcast to
   *   all applications registered for the Voice service.  Notification
   *   uses same notification data as Connect indication.
   */
  phnEvtAlertingPreConnected,   /**< 0x0029  */

  /** Alerting of Outbound Call (NOT USED, for future implementation)
   *
   *   NOT USED
   *   This notification is sent when an outbound call is alerting.
   *   This notification is universally broadcast to
   *   all applications registered for the Voice service.  Notification
   *   uses same notification data as Connect indication.
   */
  phnEvtAlerting,               /**< 0x002A  */

  /** Preconnected of Outbound Call (NOT USED, for future implementation)
   *
   *   NOT USED
   *   This notification is sent when an outbound call is preconnected.
   *   This notification is universally broadcast to
   *   all applications registered for the Voice service.  Notification
   *   uses same notification data as Connect indication.
   */
  phnEvtPreConnected,           /**< 0x002B  */

  kMaxPhnEvtSupported  /**< Must be at last position*/
} PhnEventCode;

#define isValidPhnEventCode(e) ((e >= phnEvtCardInsertion) && (e < kMaxPhnEvtSupported)) /**<		*/


/**
 *
 */
typedef struct
  {
    PhoneServiceClassType service;   		/**< type of call being disconnected */
	  Err code;                         	/**< cause of disconnect (0 for normal hangup) */
  }
PhnDisconnectEventType;

/**
 *
 **/
typedef struct
  {
    PhnAddressHandle caller;			/**<		*/
    PhoneServiceClassType service;		/**<		*/
    Int16 lineNumber;				/**<		*/
  }
PhnSubscriberAddrInfo;

/**
 *
 **/
typedef struct
  {
	  PhnConnectionID call1ID;		/**<		*/
	  PhnConnectionID call2ID;		/**<		*/
	  PhnConnectionID conferenceID;		/**<		*/
  }
PhnConferenceDesc;


/**
 *
 **/
typedef struct
  {
	  Err code;		/**<		*/
	  UInt32 id;		/**<		*/
	  // removed Count from CDMA structure to make GSM and CDMA the same. Must update library.
	  // dont think we use the count now anyway. That was added to debug CRC errors as far as
	  // I know
	  UInt16 networkError;  /**< Error code returned by the network. If this
				     field is non-zero it contains that error code
				     returned by the network. The 'code' above will be
				     the Phone Library version of this error. */
  }
PhnError;

/**
 * SMS Replacement types
 **/
typedef UInt8 PhnSMSType;
typedef enum
{
  defaultType = 0,
  discardType0 = 0x40,			/**< Type 0 messages should be discarded. */
	replaceType1 = 0x41,		/**< Replace Short Message Type 1. */
	replaceType2 = 0x42,		/**< Replace Short Message Type 2. */
	replaceType3 = 0x43,		/**< Replace Short Message Type 3. */
	replaceType4 = 0x44,		/**< Replace Short Message Type 4. */
	replaceType5 = 0x45,		/**< Replace Short Message Type 5. */
	replaceType6 = 0x46,		/**< Replace Short Message Type 6. */
	replaceType7 = 0x47		/**< Replace Short Message Type 7. */

} PhnSMSTypeEnum;

/**
 * SMS Class types
 **/
typedef UInt8 PhnSMSClass;
typedef enum
{
  classUnknown = 0,	/**<		*/
  class0,       	/**< immediate display */
  class1,   		/**<		*/
  class2,		/**<		*/
  class3		/**<		*/
} PhnSMSClassEnum;

/**
 * Struct that is sent when a new sms message event
 *   id                 - Unique id
 *   oldStatus          -
 *   newStatus          -
 *
 *   version            - Version number of the struct
 *   size               - Size of the struct
 *   msgType            - Indicates if message should be discarded or replaced
 *   msgClass           - Indicates msg class
 *   msgInd             - True if msg contains an indicator msg
 *   indicator.store    - True = store msg; False = may delete msg
 *   indicator.activate - True then activate indicator; False = deactivate indicator
 *   indicator.indType  - Type of indicator to display
 *   trasactionID       -
 *   timeStamp          - Time in seconds
 *   serviceCenterNumber- Service Center Number
 *   senderNumber       - Number of sender
 *   callbackAddress    - CallBack number
 *   destPort           -
 *   sourcePort         -
 **/
typedef struct PhnSMSParams
{
  UInt32 id;			/**<		*/
  UInt8  oldStatus;		/**<		*/
  UInt8  newStatus;		/**<		*/

  // new params
  UInt16  version;		/**<		*/
  UInt16  size;			/**<		*/
  PhnSMSType msgType;		/**<		*/
  PhnSMSClass msgClass;		/**<		*/
  Boolean moveRequested;	/**<		*/
  Boolean msgInd;		/**<		*/
  struct {
    Boolean store;		/**<		*/
    Boolean activate;		/**<		*/
    PhnMsgBoxType indType;	/**<		*/
  } indicator;

  UInt8   trasactionID;		/**<		*/
  UInt32 timeStamp;  		/**<		*/

  char serviceCenterNumber[kMaxPhoneNumberLen+1];	/**<		*/
  char senderNumber[kMaxPhoneNumberLen+1];		/**<		*/
  char callbackAddress[kMaxPhoneNumberLen+1];		/**<		*/

  Int32 destPort;		/**<		*/
  Int32 sourcePort;		/**<		*/

  UInt16 networkMsgId;		/**<		*/
  UInt8 priority;		/**<		*/
  UInt8 privacy;		/**<		*/

} PhnSMSParams;

#define PhnSMSParamsVersion 2		/**<		*/


/**
 * Phone buttons
 **/
// <chg 11-04-2002 TRS> eliminate phone & data buttons.  These were from VisorPhone.
typedef enum  {
	phnButtonHeadset,		/**<		*/
	phnButtonRedial,		/**<		*/
	phnButtonAnswerCall,		/**<		*/
	phnButtonRejectCall,		/**<		*/
	phnButtonIgnoreCall,		/**<		*/
	phnButtonHoldCall,		/**<		*/
	phnButtonConferenceCall,	/**<		*/
	phnButtonLast			/**<		*/
} PhnModuleButtonType;

#define isValidPhnModuleButtonType(m) (m < phnButtonLast)	/**<		*/

/**
 *
 **/
typedef struct
  {
    PhnModuleButtonType key;		/**<		*/
    UInt16            modifiers;	/**<		*/
  }
PhnKeyPressedParams;

/**
 *
 **/
typedef struct
  {
    PhnRegistrationStatus status;	/**<		*/
  }
PhnRegistrationType;

/**
 *
 **/
typedef struct
  {
    PhnPowerType    state;		/**<		*/
  }
PhnPowerEventType;

typedef Err (* ContinuePowerOnOffCallback) (UInt16 refNum);	/**<		*/

/**
 *
 **/
typedef struct
  {
    PhnIndicationKind kind;			/**<		*/
    char            filler;			/**<		*/
  	union
		{
		struct
			{
			Boolean state;		/**<		*/
			} simReady;
		struct
			{
			GSMSIMMessagesDialogKind dialog;	/**<		*/
			Boolean moveMessages;			/**<		*/
    	DWord mSIMMessagesAcknowledge;  	/**< creator of move msg dialog for subsequent events */
			} simMessages;
		struct
			{
			PhnPasswordType type;		/**<		*/
			} passwordAccepted;
		struct
			{
			Boolean state;			/**<		*/
			} networkAvailable;
		struct
		    {
			  ContinuePowerOnOffCallback continuePowerOnOff;	/**<		*/
			} powerOnOffContinueCallback;
    struct
		{
			UInt32 indicationCode;	/**<		*/
		} otaspMsg;

	} data;

  }
PhnIndicationType;


typedef char PhnPassword[maxPasswordLen+2];  	/**<		*/

/**
 *
 **/
typedef struct  {
	PhnPasswordType		type;		/**<		*/
	PhnPasswordType		prevType;	/**<		*/
	Err			error;		/**<		*/
	PhnPassword		pin;		/**<		*/
	PhnPassword		puk;		/**<		*/
} PhnPasswordEventType;


/**
 *
 **/
typedef enum  {
	kOpenDialog, 		/**<		*/
	kCloseDialog, 		/**<		*/
	kSetText, 		/**<		*/
	kSetRecipient, 		/**<		*/
	kShowSegment		/**<		*/
} PhnProgressType;

#define isValidPhnProgressType(p) (p <= kShowSegment)		/**<		*/
//#define isValidPhnProgressType(p) ((p >= kOpenDialog) && (p <= kShowSegment))

/**
 *
 **/
typedef enum  {
	kDialogSetBarring, 		/**<		*/
	kDialogGetBarring,		/**<		*/
	kDialogSetForwarding, 		/**<		*/
	kDialogGetForwarding,		/**<		*/
	kDialogSetCallWaiting, 		/**<		*/
	kDialogGetCallWaiting,		/**<		*/
	kDialogGetOperatorList, 	/**<		*/
	kDialogSetOperator,		/**<		*/
	kDialogOperatorSelection,	/**<		*/
	kDialogImmediateSend, 		/**<		*/
	kDialogDeferredSend,		/**<		*/
	kDialogSendUSSD			/**<		*/
} PhnOpenDialogType;

#define isValidPhnOpenDialogType(o) (o <= kDialogSendUSSD)		/**<		*/
//#define isValidPhnOpenDialogType(o) ((o >= kDialogSetBarring) && (o <= kDialogSendUSSD))

/**
 *
 **/
typedef struct {
	PhnProgressType		progress;	/**<		*/
	PhnOpenDialogType	dialog;		/**<		*/
	DWord			data;		/**< only for SMS progress */
} PhnProgressEventType;

/**
 *
 **/
typedef struct {
	PhnDatabaseID msgID;	/**<		*/
	DWord msgOwner;		/**<		*/
	Err error;		/**<		*/
	UInt8 event;		/**<		*/
} PhnMovedMsgDescType;

/**
 *
 **/
typedef struct {
	UInt16 count;			/**<		*/
	PhnMovedMsgDescType* list;	/**<		*/
} PhnMovedMsgsParamsType;

/**
 *
 **/
typedef struct {
	long	result;		/**<		*/
	CharPtr	string;		/**<		*/
} PhnUSSDEventType;

/**
 *
 **/
typedef struct {
	MemHandle	dataH;			/**<		*/
	UInt32		notificationType;	/**<		*/
} PhnSATEventType;

// for notificationType = kSATDisplayText
/**
 *
 **/
typedef struct {
	UInt32	signature;		/**<		*/
	UInt32	notificationType;	/**<		*/

	UInt32	textOffset;		/**<		*/
	UInt16	priority;		/**<		*/
	UInt16	clearMode;		/**<		*/
} PhnSATEventDisplayText;

// for notificationType = kSATGetInkey
/**
 *
 **/
typedef struct {
	UInt32	signature;		/**<		*/
	UInt32	notificationType;	/**<		*/

	UInt32	textOffset;		/**<		*/
	UInt16	format;			/**< See GSMSATEventInputFormatEnum */
	UInt16	helpInfo;		/**<		*/
} PhnSATEventGetInkey;

// header for all SATEvents
/**
 *
 **/
typedef struct PhnSATEventHeader		PhnSATEventHeader;	/**<		*/
struct PhnSATEventHeader {
	UInt32	signature;		/**<		*/
	UInt32	notificationType;	/**<		*/
};

// for notificationType = kSATGetInput
/**
 *
 **/
typedef struct {
	UInt32	signature;		/**<		*/
	UInt32	notificationType;	/**<		*/

	UInt32	textOffset;		/**<		*/
	UInt16	format;			/**< See GSMSATEventInputFormatEnum */
	UInt16	helpInfo;		/**<		*/
	UInt16	echoMode;		/**<		*/
	UInt16	minSize;		/**<		*/
	UInt16	maxSize;		/**<		*/
	UInt32  defaultOffset;		/**< offset of default value string at end of cmd block (0 if none) <chg 05-13-2002 TRS> bug # 13829 */
} PhnSATEventGetInput;

// for notificationType = kSATSetupCall
/**
 *
 **/
typedef struct {
	UInt32	signature;		/**<		*/
	UInt32	notificationType;	/**<		*/

	UInt32	numberOffset;		/**<		*/
	UInt32	promptOffset;		/**<		*/
	UInt16	type;			/**<		*/
	UInt16	callType;
} PhnSATEventSetupCall;

// for notificationType = kSATPlayTone
/**
 *
 **/
typedef struct {
	UInt32	signature;		/**<		*/
	UInt32	notificationType;	/**<		*/

	UInt32	textOffset;		/**<		*/
	UInt16	toneType;		/**<		*/
	UInt16	timeUnit;		/**< valid only if timeInterval > 0 */
	UInt16	timeInterval;		/**<		*/
} PhnSATEventPlayTone;

// contain info for one menu item
/**
 *
 **/
typedef struct {
	UInt32	labelOffset;	/**<		*/
	UInt32	features;	/**<		*/
	UInt16	id;		/**<		*/
	UInt16	nextAction;	/**<		*/
} PhnSATEventMenuRec;

// for notificationType = kSATDisplaySubmenus
/**
 *
 **/
typedef struct {
	UInt32	signature;		/**<		*/
	UInt32	notificationType;	/**<		*/

	UInt32	titleOffset;		/**<		*/
	UInt32	defaultItem;		/**< only valid in submenu */
	UInt32	recordCount;		/**<		*/
	PhnSATEventMenuRec	record[1];	/**<		*/
} PhnSATEventMenu;


// for notificationType = kSATSendSS, kSATSendSMS, and kSATSendUSSD
/**
 *
 **/
typedef struct {
	UInt32	signature;		/**<		*/
	UInt32	notificationType;	/**<		*/

	UInt32	textOffset;		/**<		*/
} PhnSATEventSendSMS;


// for notificationType = kSATRefresh
/**
 *
 **/
typedef struct {
    UInt32	signature;		/**<		*/
    UInt32	notificationType;	/**<		*/
    UInt16  refreshType;		/**<		*/
    UInt32  EFListOffset;		/**<		*/
  } PhnSATEventRefresh;


// for notificationType = kSATLaunchBrowser
/**
 *
 **/
typedef struct {
	UInt32	signature;		/**<		*/
	UInt32	notificationType;	/**<		*/
    UInt32  action;			/**<		*/
    UInt32  urlOffset;			/**<		*/
    UInt32  bearerOffset;		/**<		*/
    UInt32  provOffset;			/**<		*/
    UInt32  gatewayProxyOffset;		/**<		*/
    UInt32  alphaIdOffset;		/**<		*/
  } PhnSATEventLaunchBrowser;


/**
 *
 **/
typedef struct {
	long	mode;		/**<		*/
} PhnPhoneEquipmentMode;


/**
 *
 **/
typedef enum
  {
    eDbgUpdateNone,		/**<		*/
    eDbgUpdateYes,		/**<		*/
    eDbgUpdateQuit		/**<		*/
  }
PhnMiscDbgUpdateKind;

#define isValidPhnMiscDbgUpdateKind(x) ((x >= eDbgUpdateNone) && (x <= eDbgUpdateQuit))	/**<		*/

/**
 *
 **/
typedef struct
  {
    PhnMiscDbgUpdateKind miscUpdate;	/**<		*/
  }
PhnMiscDbgRptType;


/**
 *
 **/
typedef enum
  {
    kMsgIdReserved,		/**<		*/
    kMsgIdDeliverMT,		/**<		*/
    kMsgIdSubmitMO,		/**<		*/
    kMsgIdcancelMO,		/**<		*/
    kMsgIdDeliveryMTAck,	/**<		*/
    kMsgIdUserAck		/**<		*/
  }
PhnMsgIDType;

#define isValidPhnMsgIDType(m) ((m >= kMsgIdReserved) && (m <= kMsgIdUserAck))	/**<		*/


/**
 *
 **/
typedef enum
  {
    phnDataAvailable,		/**<		*/
    phnDataNotAvailable		/**< Used by apps that need to know when they can use the data channel */
  }
PhnDataChannelType;

#define isValidPhnDataChannelType(x) ((x == phnDataAvailable) || (x == phnDataNotAvailable))	/**<		*/


/**
 *
 **/
typedef struct
  {
	Boolean	init_rrq;	     	/**< true if initial registration, false if reregistration */
	DWord	life_timer;  		/**< new lifetime value in seconds */
	DWord	rrq_timer;    		/**< new reregistration time value in seconds */
	DWord	traffic_ck_delay_timer; /**< time in seconds to delay checking for data to trigger re-registration */
  }
PhnOnexMipRRQType, *PhnOnexMipRRQPtr;


/**
 * @name Palm Notifications
 *
 **/
/*@{*/
#define NAITextLength       72				/**<		*/
#define NAITextSize         (NAITextLength + 1)		/**<		*/
#define PasswordLength      16				/**<		*/
#define PasswordSize        (PasswordLength + 1)	/**<		*/
/*@}*/

/**
 * @name WAP defines
 *       Text Header elements
 *
 **/
/*@{*/
#define phnWAPEvent         'Hwap'		/**<		*/

#define WAPMsgLength        160			/**<		*/
#define WAPMsgSize          (WAPMsgLength + 1)	/**<		*/
/*@}*/

/**
 * @name WakeUp notification
 *
 **/
/*@{*/
#define WUHeaderEscapeSeq     "//"            	/**< start sequence of the header */
#define WUTextHeader          "//WU"          	/**< beginning of an wakeup header */
#define WUTextHeaderLength    4               	/**< length of the header */

#define WUFieldDelimiter     ';'              	/**< wakeup message fields are separated by a semi-colon */

#define phnWUEvent         'Hwak'	      	/**<		*/
/*@}*/

/**
 * @name UP stuff
 * Text Header elements
 **/
/*@{*/
#define UPHeaderEscapeSeq     "//"           	/**< start sequence of the header */
#define UPTextHeader          "//UP"         	/**< beginning of an UP header */
#define UPTextHeaderLength    4              	/**< length of the header */

#define phnUPEvent         'Hsup'		/**<		*/

#define UPDataSize            160            	/**< maximum size of the data sent from this message */
/*@}*/

/**
  *  Network Wakeup Stuff
  *  Text Header elements
  **/

/** Structure passed to the callbacks registered for incoming wakeup notifications */
struct WUNotificationEventType
{
	UInt16 version;  	/**< version number to provide future backwards compatibility */

	void *headerP;    	/**< pointer to raw header */
	UInt8 headerLen;  	/**< length of headerP */

    /** Network wakeup fields */
    char  NAI[NAITextSize];       /**< stores the NAI */
    char  _filler1_;              /**< for 16 bit allignment */
    UInt32 netTimeStamp;          /**< timestamp from the message */
    UInt8 reasonCode;             /**< a reason code from the message */
    char  _filler2_;              /**< for 16 bit allignment */
    char  password[PasswordSize]; /**< the password (encrypted) */
    char  _filler3_;              /**< for 16 bit allignment */

	/** SMS related fields */
	UInt32 msgID; /**< ID into the SMS database to reference this
                  * message this ID is not gauranteed to be
                  * valid once the notification callback
                  * returns.  Users should make a copy of the
                  * msg if they want to work on it after the
                  * callback returns.
                  */
	UInt32 datetime;   /**< date/time stamp */
	Int32  reserved2;  /**< reserved*/
	Int32  reserved3;  /**< reserved*/
};

#ifndef _cplusplus
typedef struct WUNotificationEventType WUNotificationEventType;	/**<		*/
#endif


// UP Stuff



/** Structure passed to the callbacks registered for incoming UP notifications */
struct UPNotificationEventType
{
	UInt16 version;  /**< version number to provide future backwards compatibility */

	void *headerP;    /**< pointer to raw header */
	UInt8 headerLen;  /**< length of headerP */

    /** Network wakeup fields */
    char  UPData[UPDataSize + 1];        /**< stores the raw data */
    UInt8 _filler100_;

	/** SMS related fields */
	UInt32 msgID; /**< ID into the SMS database to reference this
                  * message this ID is not gauranteed to be
                  * valid once the notification callback
                  * returns.  Users should make a copy of the
                  * msg if they want to work on it after the
                  * callback returns.
                  */
	UInt32 datetime;   /**< date/time stamp */
	Int32  reserved2;  /**< reserved*/
	Int32  reserved3;  /**< reserved*/
};

#ifndef _cplusplus
typedef struct UPNotificationEventType UPNotificationEventType;	/**<		*/
#endif


// WAP Stuff

/**
 * WAP push message - message content type
 **/
typedef enum {
  kWAPContentTypeUnknown = 0,		/**<		*/
  kWAPContentTypeIOTA,			/**<		*/
  kWAPContentTypeMMS,			/**<		*/
  kWAPContentTypeMMSText,		/**<		*/
  kWAPContentTypeServiceLoad,		/**<		*/
  kWAPContentTypeServiceLoadText,	/**<		*/
  kWAPContentTypeServiceIndicator,	/**<		*/
  kWAPContentTypeServiceIndicatorText	/**<		*/
} PhnWAPPushContentType;

typedef UInt8 PhnWAPPushContent;
/**
 * WAP push message - message type
 * http://www.wapforum.org/wina/push-app-id.htm
 **/
typedef enum {
  //URN                   //Number  //  Description
  kWAPAnyApp              = 0x00,   /**< Any Application */
  kWAPPushSIA             = 0x01,   /**< WAP Push SIA  */
  kWAPWmlUa               = 0x02,   /**< WML User Agent  */
  kWAPWtaUa               = 0x03,   /**< WTA User Agent  */
  kWAPMmsUa               = 0x04,   /**< This ID will used for application dispatching  to MMS User Agent in the handling of MMS notfication using WAP Push. See WAP-206-MMSCTR for more detail.  */
  kWAPPushSyncml          = 0x05,   /**< SyncML PUSH Application ID: used to push a SyncML Alert from a SyncML server side. The SyncML Alert is an indication for starting a SyncML session e.g., for data synchronization. Requested by the WAP WAG Synchronisation Drafting Committee.  */
  kWAPLocUa               = 0x06,   /**< This ID is used for application dispatching to Location User Agent in the handling of Location Invocation document. See WAP-257-LOCPROT for details. Requested by the WAP WAG Location Drafting Committee.   */
  kWAPSyncmlDm            = 0x07,   /**< This ID is used for SyncML Device Management. Requested by the SyncML Device Management Expert Group.   */
  kWAPDrmUa               = 0x08,   /**< This ID is used for DRM User Agent. Requested by the WAP WAG Download DC.   */
  kWAPEmnUa               = 0x09,   /**< This ID is used for Email Notification (EMN) User Agent. Requested by the WAP WAG PUSH DC.   */
  kWAPWvUa                = 0x0A,   /**< This ID is used for Wireless Village (EMN) User Agent. Requested by Wireless Village.   */

  // Registered Values -----------------------------
  kWAPMsLocalcontentUa    = 0x8000, /**< An application to receive pages to be stored and access locally. These are not wta channels. Usually the pages will be downloaded as multi-part mime.   */
  kWAPMsIMclientUa        = 0x8001, /**< An application which will act as an instant messaging client on a phone.   */
  kWAPdocomoImodeMailUa   = 0x8002, /**< Used to identify the i-mode mail application user agent on a mobile device.   */
  kWAPdocomoImodeMrUa     = 0x8003, /**< Used to identify the i-mode message request application user agent on a mobile device.   */
  kWAPdocomoImodeMfUa     = 0x8004, /**< Used to identify the i-mode message free application user agent on a mobile device.   */
  kWAPMotorolaLocationUa  = 0x8005, /**< An application that receives message notifications from the location server.   */
  kWAPMotorolaNowUa       = 0x8006, /**< An application that presents personalized information to the user.   */
  kWAPMotorolaOtaprovUa   = 0x8007, /**< An application that receives provisioning messages from the server.   */
  kWAPMotorolaBrowserUa   = 0x8008, /**< A web browsing application.   */
  kWAPMotorolaSplashUa    = 0x8009, /**< An application that receives splash screen content from the server.   */
  kWAPUnused              = 0x800A, /**<		*/
  kWAPNaiMvswCommand      = 0x800B, /**< Used by administrators to issue commands to individual devices.   */
  // unused  0x800C through 0x800F  /**<		*/
  kWAPopenwaveIotaUa      = 0x8010  /**< Used for application dispatching to the IP-based OTA Provisioning Service Agent in the handling of IOTA notfications using WAP Push.   */
} PhnWAPPushAppIdType;


typedef UInt16 PhnWAPPushAppId;

/**
 * Structure passed to the callbacks registered for WAP notifications
 **/
#define WAPNotificationTypeVersion   3		/**<		*/
struct WAPNotificationEventType
{
	UInt16 version;  		/**< version number to provide future backwards compatibility */

    /** Network wakeup fields */
    char  *wapMsg;
    char  _filler1_;                    /**< for 16 bit allignment */

	/** SMS related fields */
	UInt32 length;                      /**< originally the msgID, now it stores the length of the wapMsg */
	UInt32 datetime;                    /**< date/time stamp */

    /** VERSION 3 ADDITION */
	UInt32 id;                      /**< msgID */
    PhnWAPPushContent contentType;  /**< WAP push message content type */
    PhnWAPPushAppId   appId;        /**< WAP push message application id */
};

#ifndef _cplusplus
typedef struct WAPNotificationEventType WAPNotificationEventType;	/**<		*/
#endif


/**
 *  Phone Event Structure
 **/
typedef struct
  {
    UInt8 eventType;  			/**< PhnEventCode  */
    Boolean         acknowledge;	/**<		*/
    UInt16            connectionID;	/**<		*/
    UInt16            launchCode;	/**<		*/
    MemPtr             launchParams;	/**<		*/
    union Data
      {
        PhnDisconnectEventType disconnect;	/**<		*/
        PhnSubscriberAddrInfo info;		/**<		*/
        PhnConferenceDesc conference;		/**<		*/
        PhnError error;				/**<		*/
        PhnSMSParams params;			/**<		*/
        PhnKeyPressedParams keyPressed;		/**<		*/
        PhnRegistrationType registration;	/**<		*/
        PhnMsgBoxDataType msgBox;		/**<		*/
        PhnPowerEventType power;		/**<		*/
        PhnPasswordEventType	password;	/**<		*/
        PhnProgressEventType	progress;	/**<		*/
        PhnIndicationType indication;		/**<		*/
        PhnMovedMsgsParamsType  moved;		/**<		*/
        PhnUSSDEventType		ussd;	/**<		*/
        PhnSATEventType			sat;	/**<		*/
        PhnPhoneEquipmentMode		phoneEquipmentMode;	/**<		*/
        PhnMiscDbgRptType miscDbgRpt;		/**<		*/
        PhnPDDataType     PDDataInfo;         	/**< sent asynchronously by the modem */
        PhnOneXStatus oneXStatus;		/**<		*/
        PhnMIPFailType	mipSessionFailReason;	/**<		*/
        PhnIOTAReportType iotaInfo;		/**<		*/
        PhnDataChannelType dataChannelAvailability;	/**<		*/
        PhnOnexMipRRQType  PhnOnxMipRRQ;	/**<		*/
        PhnOneXDataFailType oneXFailReason;	/**<		*/
        WAPNotificationEventType wapEvtParams;	/**<		*/
	  }
    data;
  }
PhnEventType   ,* PhnEventPtr;

#define PhnEventRecord PhnEventType

#endif
