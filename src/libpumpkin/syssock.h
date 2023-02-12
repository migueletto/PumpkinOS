#ifdef __cplusplus
extern "C" {
#endif

// Initialize the NetSocket glue code globals if not already initialized.
// In particular, this sets up the AppNetRefnum global with the refnum of
//  the NetLib. 
Err NetUInit(void);

// Open up a TCP socket and connect to the given host. If port is 0, the port
//  number will be looked up by the serviceName parameter. Returns socket
//  descriptor or -1 on error
NetSocketRef NetUTCPOpen(const Char *hostName, const Char *serviceName, Int16 port);

// Read N bytes from a descriptor. This call automatically makes repeated read
//  calls to the socket until all N bytes have been received
// Returns number of bytes read or -1 if error
Int32 NetUReadN(NetSocketRef fd, UInt8 *bufP, UInt32 numBytes);

// Write N bytes to a descriptor. This call automatically makes repeated write
//  calls to the socket until all N bytes have been sent
// Returns number of bytes written or -1 if error
Int32 NetUWriteN(NetSocketRef fd, UInt8 * bufP, UInt32 numBytes);

#ifdef __cplusplus 
}
#endif
