    case gpsLibTrapClose: {
      const UInt16 refNum = sys_va_arg(ap, UInt32);
      Err ret = GPSClose(refNum);
      *iret = ret;
      }
      break;

    case gpsLibTrapGetLibAPIVersion: {
      const UInt16 refNum = sys_va_arg(ap, UInt32);
      UInt16 ret = GPSGetLibAPIVersion(refNum);
      *iret = ret;
      }
      break;

    case gpsLibTrapGetMaxSatellites: {
      const UInt16 refNum = sys_va_arg(ap, UInt32);
      UInt8 ret = GPSGetMaxSatellites(refNum);
      *iret = ret;
      }
      break;

    case gpsLibTrapGetPVT: {
      const UInt16 refNum = sys_va_arg(ap, UInt32);
      GPSPVTDataType *pvt = sys_va_arg(ap, GPSPVTDataType *);
      Err ret = GPSGetPVT(refNum, pvt);
      *iret = ret;
      }
      break;

    case gpsLibTrapGetPosition: {
      const UInt16 refNum = sys_va_arg(ap, UInt32);
      GPSPositionDataType *position = sys_va_arg(ap, GPSPositionDataType *);
      Err ret = GPSGetPosition(refNum, position);
      *iret = ret;
      }
      break;

    case gpsLibTrapGetSatellites: {
      const UInt16 refNum = sys_va_arg(ap, UInt32);
      GPSSatDataType *sat = sys_va_arg(ap, GPSSatDataType *);
      Err ret = GPSGetSatellites(refNum, sat);
      *iret = ret;
      }
      break;

    case gpsLibTrapGetStatus: {
      const UInt16 refNum = sys_va_arg(ap, UInt32);
      GPSStatusDataType *status = sys_va_arg(ap, GPSStatusDataType *);
      Err ret = GPSGetStatus(refNum, status);
      *iret = ret;
      }
      break;

    case gpsLibTrapGetTime: {
      const UInt16 refNum = sys_va_arg(ap, UInt32);
      GPSTimeDataType *time = sys_va_arg(ap, GPSTimeDataType *);
      Err ret = GPSGetTime(refNum, time);
      *iret = ret;
      }
      break;

    case gpsLibTrapGetVelocity: {
      const UInt16 refNum = sys_va_arg(ap, UInt32);
      GPSVelocityDataType *velocity = sys_va_arg(ap, GPSVelocityDataType *);
      Err ret = GPSGetVelocity(refNum, velocity);
      *iret = ret;
      }
      break;

    case gpsLibTrapOpen: {
      const UInt16 refNum = sys_va_arg(ap, UInt32);
      Err ret = GPSOpen(refNum);
      *iret = ret;
      }
      break;

