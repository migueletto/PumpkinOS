extern Boolean SysLibNewRefNum(UInt32 type, UInt32 creator, UInt16 *refNum) SYS_TRAP(0xA503);
extern Err SysLibRegister(UInt16 refNum, LocalID dbID, void *code, UInt32 size, UInt16 *dispatchTblP, UInt8 *globalsP) SYS_TRAP(0xA504);
extern void SysLibCancelRefNum(UInt16 refNum) SYS_TRAP(0xA505);
