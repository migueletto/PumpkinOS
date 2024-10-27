    case sysLibTrapOpen: {
      UInt16 uRefNum = sys_va_arg(ap, UInt32);
      Err ret = GPDOpen(uRefNum);
      *iret = ret;
      }
      break;

    case sysLibTrapClose: {
      UInt16 uRefNum = sys_va_arg(ap, UInt32);
      UInt32 *dwRefCountP = sys_va_arg(ap, UInt32 *);
      Err ret = GPDClose(uRefNum, dwRefCountP);
      *iret = ret;
      }
      break;

    case GPDTrapGetVersion: {
      UInt16 uRefNum = sys_va_arg(ap, UInt32);
      UInt32 *dwVerP = sys_va_arg(ap, UInt32 *);
      Err ret = GPDGetVersion(uRefNum, dwVerP);
      *iret = ret;
      }
      break;

    case GPDTrapRead: {
      UInt16 uRefNum = sys_va_arg(ap, UInt32);
      UInt8 *resultP = sys_va_arg(ap, UInt8 *);
      Err ret = GPDReadInstant(uRefNum, resultP);
      *iret = ret;
      }
      break;

