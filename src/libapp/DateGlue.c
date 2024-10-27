#include <PalmOS.h>

UInt16 DateGlueTemplateToAscii(const Char *templateP, UInt8 months, UInt8 days, UInt16 years, Char *stringP, Int16 stringSize) {
  return DateTemplateToAscii(templateP, months, days, years, stringP, stringSize);
}

void DateGlueToDOWDMFormat(UInt8 month, UInt8 day, UInt16 year, DateFormatType dateFormat, Char *pString) {
  DateToDOWDMFormat(month, day, year, dateFormat, pString);
}
