void SerialInit(void);
Err SerialList(char **, UInt32 *, UInt16 *);
Err SerialOnline(UInt16 *AppSerRefnum, UInt32 baud, UInt16 bits, UInt16 parity,
                 UInt16 stopBits, UInt16 xonXoff, UInt16 rts, UInt16 cts,
                 UInt32 device);
void SerialOffline(UInt16);
Int16 SerialReceive(UInt16, UInt8 *, Int16, Err *);
Int16 SerialReceiveWait(UInt16 AppSerRefnum, UInt8 *buf, Int16 tam,
                        Int32 wait, Err *err);
Int16 SerialSend(UInt16, UInt8 *, Int16, Err *);
UInt16 SerialGetStatus(UInt16);
Err SerialBreak(UInt16);
