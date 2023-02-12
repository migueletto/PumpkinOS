/******************************************************************************
 *
 * Copyright (c) 2001-2003 PalmSource, Inc. All rights reserved.
 *
 * File: BtCommVdrv.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *      Bluetooth RfComm Virtual Serial Driver public header file.
 *
 *****************************************************************************/

#ifndef __BTCOMMVDRV_H
#define __BTCOMMVDRV_H

#include <PalmTypes.h>
#include <BtLib.h>

//-----------------------------------------------------------------------------
//  BtVdOpenParams -- Bluetooth RFCOMM Virtual Serial Driver Open() parameters.
//
//  To open an RFCOMM virtual serial port, you must call SrmExtOpen(), passing
//  a pointer to an object of type SrmOpenConfigType, which in turn points to
//  an object of type BtVdOpenParams. The latter contains the information that
//  describes how to establish the RFCOMM connection:
//
//    - whether to play the role of client or server
//    - parameters specific to that role
//    - whether to require link authentication
//    - whether to require link encryption
//
//  Client Role
//  -----------
//  
//  When playing the client role, you must specify the address of the remote
//  bluetooth device, and the service to which to connect on that remote
//  device.
//  
//  The address of the remote device may be set to null (all binary zeros),
//  in which case a bluetooth device discovery operation will be initiated,
//  allowing the end user to choose the device to which to connect.
//  
//  You normally specify the service to which to connect by providing a list
//  of one or more Service Class UUIDs. In that case, SDP queries will be
//  performed, looking successively for each of the given Service Classes.
//  The first Service Class that is found on the remote device will be used.
//  As a convenience, you may specify an empty list of Service Class UUIDs
//  (zero count), in which case a first SDP query will be made for a default
//  Palm-specific Service Class UUID (953D4FBC-8DA3-11D5-AA62-0030657C543C);
//  then if that fails, an SDP query will be made for the SerialPort Service
//  Class UUID.
//  
//  The result of a successful SDP query is the RfComm server channel to
//  which to connect on the remote device. To facilitate testing, you can
//  bypass SDP querying, and directly specify the remote RfComm server
//  channel id.
//  
//  The call to SrmExtOpen() will block until the RFCOMM connection is
//  established, or it is determined that the connection cannot be established.
//  The the driver will display a progress dialog, allowing the user to cancel
//  the connection attempt if she so desires (unless the call to SrmExtOpen()
//  is made on a non-UI thread).
//  
//  SrmExtOpen() returns 0 if and only if the the RFCOMM connection was
//  successfully established.
//
//  Server Role
//  -----------
//  When playing the server role, you must specify the UUID of one service
//  to advertise, and you may also specify a user-readable name for that
//  service.
//
//  As a convenience, you may specify a null UUID (all binary zeros), in which
//  case the default Palm-specific Service Class UUID will be advertised.
//  (953D4FBC-8DA3-11D5-AA62-0030657C543C).
//
//  The call to SrmExtOpen() will return immediately, without waiting for the
//  incoming RFCOMM connection. The caller should thereafter wait for incoming
//  data by periodically calling SrmReceive(), SrmReceiveWait(), or
//  SrmReceiveCheck().
//
//  Example
//  -------
//  Here is an example of a call to SrmExtOpen() that creates a bluetooth
//  RfComm virtual port. The parameters we set up will cause us to:
//  -  play the role of client
//  -  determine the remote device by doing discovery with UI interaction
//  -  connect to the Serial Port service class on the remote device
//
//  Err                err;
//  UInt16             btPortId;
//  SrmOpenConfigType  config;
//  BtVdOpenParams     btParams;
//  BtLibSdpUuidType   sppUuid;
//
//  // Set up a SerialPort service class UUID
//  //
//  MemSet( &sppUuid, sizeof(sppUuid), 0 );
//  sppUuid.size = btLibUuidSize16;
//  sppUuid.UUID[0] = 0x11;
//  sppUuid.UUID[1] = 0x01;
//
//  // Set up a bluetooth parameter block: play client role, use null remote device address,
//  // and use sdp to search for the serial port profile service class uuid.
//  //
//  MemSet( &btParams, sizeof(btParams), 0 );     // note this leaves the remote dev addr null    
//  btParams.role = btVdClient;                   // we are the client side
//  btParams.u.client.method = btVdUseUuidList;   // use SDP to look for service class uuids
//  btParams.u.client.u.uuidList.tab = &sppUuid;  // the list of service uuids to search for
//  btParams.u.client.u.uuidList.len = 1;         // the number of uuids in the list
//
//  // Set up the SrmExtOpen param block to reference our bluetooth param block
//  //
//  MemSet( &config, sizeof(config), 0 );
//  config.function = 0;                       // must be zero
//  config.drvrDataP = (MemPtr)&btParams;      // bluetooth-specific params go here 
//  config.drvrDataSize = sizeof(btParams);    // size of bluetooth-specific params
//
//  // Call SrmExtOpen with the parameters we just set up.
//  //
//  err = SrmExtOpen(
//      sysFileCVirtRfComm, // type of port == bluetooth RfComm
//      &config,            // port configuration params
//      sizeof(config),     // size of port config params
//      &btPortId           // receives the id of this virtual port instance
//  );
//

//  Role -- client or server
//
typedef enum {
    btVdClient=0,        // initiates baseband and RFCOMM connections
    btVdServer           // waits for incoming baseband and RFCOMM connections
} BtVdRole;

// Client Method -- how to determine remote RFCOMM channel to which to connect.
//
typedef enum {
    btVdUseUuidList=0,   // use SDP to look for any one of the given UUIDs
    btVdUseChannelId     // just use the given RFCOMM channel id
} BtVdClientMethod;

//  How to represent a list of UUIDs.
//
typedef struct {
    UInt8                       len;           // length of table == number of UUIDs
    BtLibSdpUuidType*           tab;           // table of UUIDs
} BtVdUuidList;

//  Open parameters, client side
//
typedef struct {
    BtLibDeviceAddressType      remoteDevAddr; // remote bluetooth device address
                                               // (null address => perform discovery with UI dialog)
    BtVdClientMethod            method;        // how to determine remote RFCOMM channel
    union {
        BtLibRfCommServerIdType channelId;     // RFCOMM channel to which to connect
        BtVdUuidList            uuidList;      // list of UUIDs of the service to which to connect
                                               // (null list => connect to default Palm service)
    } u;
} BtVdOpenParamsClient;

//  Open parameters, server side
//
typedef struct {
    BtLibSdpUuidType            uuid;          // UUID of the service to advertise
                                               // (null UUID => advertise default Palm service)
    const Char*                 name;          // optional readable name of the service
} BtVdOpenParamsServer;

//  BtVdOpenParams -- Bluetooth-RFCOMM-specific parameters to SrmExtOpen().
//  The second argument to SrmExtOpen() points to an SrmOpenConfigType object.
//  The field drvrDataP of that object must point to a BtVdOpenParams object.
//
typedef struct {
    BtVdRole                    role;          // client or server?
    union {
        BtVdOpenParamsClient    client;        // client parameters
        BtVdOpenParamsServer    server;        // server parameters
    } u;
    Boolean                     authenticate;  // require link authentication
    Boolean                     encrypt;       // require link encryption
} BtVdOpenParams;

#endif // __BTCOMMVDRV_H
