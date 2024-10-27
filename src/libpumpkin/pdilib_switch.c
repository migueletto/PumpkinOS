    case PdiLibTrapDefineReaderDictionary: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiReaderType *ioReader = sys_va_arg(ap, PdiReaderType *);
      PdiDictionary *dictionary = sys_va_arg(ap, PdiDictionary *);
      Boolean disableMainDictionary = sys_va_arg(ap, UInt32);
      PdiDictionary *ret = PdiDefineReaderDictionary(libRefnum, ioReader, dictionary, disableMainDictionary);
      *pret = (void *)ret;
      }
      break;

    case PdiLibTrapDefineResizing: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiReaderType *ioReader = sys_va_arg(ap, PdiReaderType *);
      UInt16 deltaSize = sys_va_arg(ap, UInt32);
      UInt16 maxSize = sys_va_arg(ap, UInt32);
      Err ret = PdiDefineResizing(libRefnum, ioReader, deltaSize, maxSize);
      *iret = ret;
      }
      break;

    case PdiLibTrapDefineWriterDictionary: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiWriterType *ioWriter = sys_va_arg(ap, PdiWriterType *);
      PdiDictionary *dictionary = sys_va_arg(ap, PdiDictionary *);
      Boolean disableMainDictionary = sys_va_arg(ap, UInt32);
      PdiDictionary *ret = PdiDefineWriterDictionary(libRefnum, ioWriter, dictionary, disableMainDictionary);
      *pret = (void *)ret;
      }
      break;

    case PdiLibTrapEnterObject: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiReaderType *ioReader = sys_va_arg(ap, PdiReaderType *);
      Err ret = PdiEnterObject(libRefnum, ioReader);
      *iret = ret;
      }
      break;

    case PdiLibTrapReadParameter: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiReaderType *ioReader = sys_va_arg(ap, PdiReaderType *);
      Err ret = PdiReadParameter(libRefnum, ioReader);
      *iret = ret;
      }
      break;

    case PdiLibTrapReadProperty: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiReaderType *ioReader = sys_va_arg(ap, PdiReaderType *);
      Err ret = PdiReadProperty(libRefnum, ioReader);
      *iret = ret;
      }
      break;

    case PdiLibTrapReadPropertyField: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiReaderType *ioReader = sys_va_arg(ap, PdiReaderType *);
      Char * *bufferPP = sys_va_arg(ap, Char * *);
      UInt16 bufferSize = sys_va_arg(ap, UInt32);
      UInt16 readMode = sys_va_arg(ap, UInt32);
      Err ret = PdiReadPropertyField(libRefnum, ioReader, bufferPP, bufferSize, readMode);
      *iret = ret;
      }
      break;

    case PdiLibTrapReadPropertyName: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiReaderType *ioReader = sys_va_arg(ap, PdiReaderType *);
      Err ret = PdiReadPropertyName(libRefnum, ioReader);
      *iret = ret;
      }
      break;

    case PdiLibTrapReaderDelete: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiReaderType * *ioReader = sys_va_arg(ap, PdiReaderType * *);
      PdiReaderDelete(libRefnum, ioReader);
      }
      break;

    case PdiLibTrapReaderNew: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      UDAReaderType *input = sys_va_arg(ap, UDAReaderType *);
      UInt16 version = sys_va_arg(ap, UInt32);
      PdiReaderType *ret = PdiReaderNew(libRefnum, input, version);
      *pret = (void *)ret;
      }
      break;

    case PdiLibTrapSetCharset: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiWriterType *ioWriter = sys_va_arg(ap, PdiWriterType *);
      CharEncodingType charset = sys_va_arg(ap, UInt32);
      Err ret = PdiSetCharset(libRefnum, ioWriter, charset);
      *iret = ret;
      }
      break;

    case PdiLibTrapSetEncoding: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiWriterType *ioWriter = sys_va_arg(ap, PdiWriterType *);
      UInt16 encoding = sys_va_arg(ap, UInt32);
      Err ret = PdiSetEncoding(libRefnum, ioWriter, encoding);
      *iret = ret;
      }
      break;

    case PdiLibTrapWriteBeginObject: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiWriterType *ioWriter = sys_va_arg(ap, PdiWriterType *);
      UInt16 objectNameID = sys_va_arg(ap, UInt32);
      Err ret = PdiWriteBeginObject(libRefnum, ioWriter, objectNameID);
      *iret = ret;
      }
      break;

    case PdiLibTrapWriteParameter: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiWriterType *ioWriter = sys_va_arg(ap, PdiWriterType *);
      UInt16 parameter = sys_va_arg(ap, UInt32);
      Boolean parameterName = sys_va_arg(ap, UInt32);
      Err ret = PdiWriteParameter(libRefnum, ioWriter, parameter, parameterName);
      *iret = ret;
      }
      break;

    case PdiLibTrapWriteParameterStr: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiWriterType *ioWriter = sys_va_arg(ap, PdiWriterType *);
      const Char *parameterName = sys_va_arg(ap, Char *);
      const Char *parameterValue = sys_va_arg(ap, Char *);
      Err ret = PdiWriteParameterStr(libRefnum, ioWriter, parameterName, parameterValue);
      *iret = ret;
      }
      break;

    case PdiLibTrapWriteProperty: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiWriterType *ioWriter = sys_va_arg(ap, PdiWriterType *);
      UInt16 propertyNameID = sys_va_arg(ap, UInt32);
      Err ret = PdiWriteProperty(libRefnum, ioWriter, propertyNameID);
      *iret = ret;
      }
      break;

    case PdiLibTrapWritePropertyBinaryValue: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiWriterType *ioWriter = sys_va_arg(ap, PdiWriterType *);
      const Char *buffer = sys_va_arg(ap, Char *);
      UInt16 size = sys_va_arg(ap, UInt32);
      UInt16 options = sys_va_arg(ap, UInt32);
      Err ret = PdiWritePropertyBinaryValue(libRefnum, ioWriter, buffer, size, options);
      *iret = ret;
      }
      break;

    case PdiLibTrapWritePropertyFields: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiWriterType *ioWriter = sys_va_arg(ap, PdiWriterType *);
      Char **fields = sys_va_arg(ap, Char * *);
      UInt16 fieldNumber = sys_va_arg(ap, UInt32);
      UInt16 options = sys_va_arg(ap, UInt32);
      Err ret = PdiWritePropertyFields(libRefnum, ioWriter, fields, fieldNumber, options);
      *iret = ret;
      }
      break;

    case PdiLibTrapWritePropertyStr: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiWriterType *ioWriter = sys_va_arg(ap, PdiWriterType *);
      const Char *propertyName = sys_va_arg(ap, Char *);
      UInt8 writeMode = sys_va_arg(ap, UInt32);
      UInt8 requiredFields = sys_va_arg(ap, UInt32);
      Err ret = PdiWritePropertyStr(libRefnum, ioWriter, propertyName, writeMode, requiredFields);
      *iret = ret;
      }
      break;

    case PdiLibTrapWritePropertyValue: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiWriterType *ioWriter = sys_va_arg(ap, PdiWriterType *);
      Char *buffer = sys_va_arg(ap, Char *);
      UInt16 options = sys_va_arg(ap, UInt32);
      Err ret = PdiWritePropertyValue(libRefnum, ioWriter, buffer, options);
      *iret = ret;
      }
      break;

    case PdiLibTrapWriterDelete: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      PdiWriterType * *ioWriter = sys_va_arg(ap, PdiWriterType * *);
      PdiWriterDelete(libRefnum, ioWriter);
      }
      break;

    case PdiLibTrapWriterNew: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      UDAWriterType *output = sys_va_arg(ap, UDAWriterType *);
      UInt16 version = sys_va_arg(ap, UInt32);
      PdiWriterType *ret = PdiWriterNew(libRefnum, output, version);
      *pret = (void *)ret;
      }
      break;

    case sysLibTrapClose: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      Err ret = PdiLibClose(libRefnum);
      *iret = ret;
      }
      break;

    case sysLibTrapOpen: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      Err ret = PdiLibOpen(libRefnum);
      *iret = ret;
      }
      break;

