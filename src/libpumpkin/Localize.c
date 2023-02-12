#include <PalmOS.h>

#include "debug.h"

void LocGetNumberSeparators(NumberFormatType numberFormat, Char *thousandSeparator, Char *decimalSeparator) {
  Char t, d;

  switch (numberFormat) {
    case nfCommaPeriod:      t = ',';  d = '.'; break;
    case nfPeriodComma:      t = '.';  d = ','; break;
    case nfSpaceComma:       t = ' ';  d = ','; break;
    case nfApostrophePeriod: t = '\''; d = '.'; break;
    case nfApostropheComma:  t = '\''; d = ','; break;
    default:                 t = ',';  d = '.'; break;
  }

  if (thousandSeparator) *thousandSeparator = t;
  if (decimalSeparator) *decimalSeparator = d;
}
