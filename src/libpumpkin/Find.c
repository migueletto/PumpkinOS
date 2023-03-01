//#define ALLOW_ACCESS_TO_INTERNALS_OF_FINDPARAMS

#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

void Find(GoToParamsPtr goToP) {
  debug(DEBUG_ERROR, "PALMOS", "Find not implemented");
}

// Returns the bounds of the next available line for displaying a match in the Find results dialog.
void FindGetLineBounds(const FindParamsType *findParams, RectanglePtr r) {
  if (findParams && r) {
    r->topLeft.x = findParams->rect.topLeft.x;
    r->extent.x = findParams->rect.extent.x;
    r->extent.y = findParams->rect.extent.y / maxFinds;
    r->topLeft.y = findParams->rect.topLeft.y + findParams->lineNumber * r->extent.y;
  }
}

// Draw the header line that separates, by application, the list of found items.
// Returns true if Find screen is filled up. Applications should exit from the search if this occurs.
Boolean FindDrawHeader(FindParamsPtr findParams, Char const *title) {
  RectangleType rect;
  UInt16 len, dx, x;
  Boolean full = false;

  if (findParams && title) {
    FindGetLineBounds(findParams, &rect);
    WinDrawLine(rect.topLeft.x, rect.topLeft.y + rect.extent.y / 2, rect.topLeft.x + rect.extent.x, rect.topLeft.y + rect.extent.y / 2);
    len = StrLen(title);
    if (len) {
      dx = FntCharsWidth(title, len);
      x = (rect.extent.x - dx) / 2;
      rect.topLeft.x = x - 2;
      rect.extent.x = dx + 4;
      WinEraseRectangle(&rect, 0);
      WinDrawChars(title, len, x, rect.topLeft.y);
    }
    findParams->lineNumber++;
  }

  return full;
}

Boolean FindStrInStr(Char const *strToSearch, Char const *strToFind, UInt16 *posP) {
  Boolean found = false;
  UInt16 i, n;

  if (strToSearch && strToFind && posP) {
    n = StrLen(strToFind);
    for (i = 0;;) {
      for (; strToSearch[i] && strToSearch[i] <= 32; i++);
      if (!strToSearch[i]) break;
      if (StrNCaselessCompare(&strToSearch[i], strToFind, n) == 0) {
        *posP = i;
        found = true;
        break;
      }
      for (; strToSearch[i] && strToSearch[i] > 32; i++);
      if (!strToSearch[i]) break;
    }
  }

  return found;
}

Boolean FindSaveMatch(FindParamsPtr findParams, UInt16 recordNum, UInt16 pos, UInt16 fieldNum, UInt32 appCustom, UInt16 cardNo, LocalID dbID) {
  Boolean r = false;

  if (findParams) {
    if (findParams->numMatches < maxFinds) {
      findParams->idx[findParams->lineNumber] = findParams->numMatches;
      findParams->match[findParams->numMatches].appCardNo = 0;            // card number of the database record was found in
      findParams->match[findParams->numMatches].appDbID = pumpkin_get_app_localid(); // LocalID of the application
      //Boolean                 foundInCaller;        // true if found in app that called Find
      findParams->match[findParams->numMatches].dbCardNo = cardNo;         // card number of the database record was found in
      findParams->match[findParams->numMatches].dbID = dbID;               // LocalID of the database record was found in
      findParams->match[findParams->numMatches].recordNum = recordNum;     // index of record that contain a match
      findParams->match[findParams->numMatches].matchPos = pos;            // postion in record of the match.
      findParams->match[findParams->numMatches].matchFieldNum = fieldNum;  // field number
      findParams->match[findParams->numMatches].matchCustom = appCustom;   // app specific data
      findParams->numMatches++;
    }
    findParams->recordNum = recordNum;
    r = findParams->numMatches == maxFinds;
  }

  return r;
}
