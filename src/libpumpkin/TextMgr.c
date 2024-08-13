#include <PalmOS.h>

#include "debug.h"

#define PALMOS_MODULE "Text"

UInt8 TxtByteAttr(UInt8 inByte) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "TxtByteAttr not implemented");
  return 0;
}

/*
#define charAttr_XA   0x0200  // extra alphabetic
#define charAttr_XS   0x0100  // extra space
#define charAttr_BB   0x0080  // BEL, BS, etc.
#define charAttr_CN   0x0040  // CR, FF, HT, NL, VT
#define charAttr_DI   0x0020  // '0'-'9'
#define charAttr_LO   0x0010  // 'a'-'z' and lowercase extended chars.
#define charAttr_PU   0x0008  // punctuation
#define charAttr_SP   0x0004  // space
#define charAttr_UP   0x0002  // 'A'-'Z' and uppercase extended chars.
#define charAttr_XD   0x0001  // '0'-'9', 'A'-'F', 'a'-'f'

#define charAttrPrint       (charAttr_DI|charAttr_LO|charAttr_PU|charAttr_SP|charAttr_UP|charAttr_XA)
#define charAttrSpace       (charAttr_CN|charAttr_SP|charAttr_XS)
#define charAttrAlNum       (charAttr_DI|charAttr_LO|charAttr_UP|charAttr_XA)
#define charAttrAlpha       (charAttr_LO|charAttr_UP|charAttr_XA)
#define charAttrCntrl       (charAttr_BB|charAttr_CN)
#define charAttrGraph       (charAttr_DI|charAttr_LO|charAttr_PU|charAttr_UP|charAttr_XA)
#define charAttrDelim       (charAttr_SP|charAttr_PU)
*/

UInt16 TxtCharAttr(WChar inChar) {
  UInt16 attr = 0;

  if (inChar == ' ' || inChar == '\t' || inChar == '\r' || inChar == '\n') {
    attr = charAttrSpace;
  } else if (inChar < 32) {
    attr = charAttrCntrl;
  } else if (inChar >= '0' && inChar <= '9') {
    attr = charAttr_XA | charAttr_DI;
  } else if (inChar >= 'a' && inChar <= 'z') {
    attr = charAttr_XA | charAttr_LO;
  } else if (inChar >= 'A' && inChar <= 'Z') {
    attr = charAttr_XA | charAttr_UP;
  } else if (inChar >= 32 && inChar < 128) {
    attr = charAttr_XA | charAttr_PU;
  }
  // XXX missing: charAttrGraph, charAttrDelim

  return attr;
}

UInt16 TxtCharXAttr(WChar inChar) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "TxtCharXAttr not implemented");
  return 0;
}

UInt16 TxtCharSize(WChar inChar) {
  return inChar & 0xFF00 ? 2 : 1;
}

Int16 TxtCharWidth(WChar inChar) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "TxtCharWidth not implemented");
  return 0;
}

UInt16 TxtGetPreviousChar(const Char *inText, UInt32 inOffset, WChar *outChar) {
  if (inText && outChar) {
    *outChar = inText[inOffset];
  }
  return 1;
}

// Retrieve the character starting at the specified offset within a text buffer.
// Returns the size in bytes of the character at inOffset. If outChar
// is not NULL upon entry, it points to the character at inOffset upon return.

UInt16 TxtGetNextChar(const Char *inText, UInt32 inOffset, WChar *outChar) {
  if (inText && outChar) {
    *outChar = inText[inOffset];
  }
  return 1;
}

WChar TxtGetChar(const Char *inText, UInt32 inOffset) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "TxtGetChar not implemented");
  return 0;
}

UInt16 TxtSetNextChar(Char *ioText, UInt32 inOffset, WChar inChar) {
  if (ioText) {
    ioText[inOffset] = inChar;
  }
  return 1;
}

UInt16 TxtReplaceStr(Char *ioStr, UInt16 inMaxLen, const Char *inParamStr, UInt16 inParamNum) {
  UInt16 i, j, k, st, num = 0;
  Char *s, c, param;

  if (ioStr && inParamNum <= 9) {
    if ((s = MemPtrNew(inMaxLen+1)) != NULL) {
      param = '0' + inParamNum;
      st = 0;
      for (i = 0, j = 0; ioStr[i]; i++) {
        c = ioStr[i];
        switch (st) {
          case 0:
            if (c == '^') {
              st = 1;
            } else {
              if (j < inMaxLen-1) s[j++] = c;
            }
            break;
          case 1:
            if (c == param) {
              if (inParamStr) {
                for (k = 0; inParamStr[k]; k++) {
                  if (j < inMaxLen-1) s[j++] = inParamStr[k];
                }
              }
              num++;
            } else {
              if (j < inMaxLen-1) s[j++] = '^';
              if (j < inMaxLen-1) s[j++] = c;
            }
            st = 0;
            break;
        }
      }
      if (inParamStr) {
        MemMove(ioStr, s, j);
        ioStr[j] = 0;
      }
      MemPtrFree(s);
    }
  }

  return num;
}

WChar TxtCharBounds(const Char *inText, UInt32 inOffset, UInt32 *outStart, UInt32 *outEnd) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "TxtCharBounds not implemented");
  return 0;
}

UInt32 TxtGetTruncationOffset(const Char *inText, UInt32 inOffset) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "TxtGetTruncationOffset not implemented");
  return 0;
}

UInt32 TxtGetWordWrapOffset(const Char *iTextP, UInt32 iOffset) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "TxtGetWordWrapOffset not implemented");
  return 0;
}

CharEncodingType TxtCharEncoding(WChar inChar) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "TxtCharEncoding not implemented");
  return 0;
}

CharEncodingType TxtStrEncoding(const Char *inStr) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "TxtStrEncoding not implemented");
  return 0;
}

CharEncodingType TxtMaxEncoding(CharEncodingType a, CharEncodingType b) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "TxtMaxEncoding not implemented");
  return 0;
}

CharEncodingType TxtNameToEncoding(const Char* iEncodingName) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "TxtNameToEncoding not implemented");
  return 0;
}

// Determine whether a character is valid character given the Palm OS character encoding.
Boolean TxtCharIsValid(WChar inChar) {
  return inChar < 256;
}

Boolean TxtFindString(const Char *inSourceStr, const Char *inTargetStr, UInt32 *outPos, UInt16 *outLength) {
  Boolean found = false;
  UInt16 i, n;

  if (inSourceStr && inTargetStr && outPos && outLength) {
    *outPos = 0;
    *outLength = 0;
    n = StrLen(inTargetStr);
    for (i = 0;;) {
      for (; inSourceStr[i] && inSourceStr[i] <= 32; i++);
      if (!inSourceStr[i]) break;
      if (StrNCaselessCompare(&inSourceStr[i], inTargetStr, n) == 0) {
        *outPos = i;
        *outLength = n;
        found = true;
        break;
      }
      for (; inSourceStr[i] && inSourceStr[i] > 32; i++);
      if (!inSourceStr[i]) break;
    }

    if (found) {
      debug(DEBUG_TRACE, PALMOS_MODULE, "TxtFindString \"%s\" found at %d", inTargetStr, *outPos);
    } else {
      debug(DEBUG_TRACE, PALMOS_MODULE, "TxtFindString \"%s\" not found", inTargetStr);
    }
  }

  return found;
}

static MemHandle TxtParamStringHandle(const Char *inTemplate, const Char *param0, const Char *param1, const Char *param2, const Char *param3) {
  MemHandle h;
  UInt16 i, j, k, max, st;
  Char c, *p, *s;

  debug(DEBUG_TRACE, PALMOS_MODULE, "TxtParamString(\"%s\", \"%s\", \"%s\", \"%s\", \"%s\")",
    inTemplate, param0 ? param0 : "NULL", param1 ? param1 : "NULL", param2 ? param2 : "NULL", param3 ? param3 : "NULL");

  max = 1024;
  if ((h = MemHandleNew(max)) != NULL) {
    s = MemHandleLock(h);
    st = 0;
    for (i = 0, j = 0; inTemplate[i]; i++) {
      c = inTemplate[i];
      switch (st) {
        case 0:
          if (c == '^') {
            st = 1;
          } else {
            if (j < max-1) s[j++] = c;
          }
          break;
        case 1:
            switch (c) {
              case '0': p = (char *)param0; break;
              case '1': p = (char *)param1; break;
              case '2': p = (char *)param2; break;
              case '3': p = (char *)param3; break;
              default : p = NULL;   break;
            }
            if (p) {
              for (k = 0; p[k]; k++) {
                if (j < max-1) s[j++] = p[k];
              }
            }
            st = 0;
          break;
      }
    }
    s[j] = 0;
    MemHandleUnlock(h);
  }

  return h;
}

// Returns a pointer to a locked relocatable chunk in the dynamic heap that contains the appropriate substitutions.
// TxtParamString allocates space for the returned string in the dynamic heap through a call to MemHandleNew, and then returns
// the result of calling MemHandleLock with this handle. Your code is responsible for freeing this memory when it is no longer needed.

Char *TxtParamString(const Char *inTemplate, const Char *param0, const Char *param1, const Char *param2, const Char *param3) {
  MemHandle h;
  char *s = NULL;

  if ((h = TxtParamStringHandle(inTemplate, param0, param1, param2, param3)) != NULL) {
    s = MemHandleLock(h);
  }

  return s;
}

static Int16 TxtCompareAux(const Char *s1, UInt16 s1Len, UInt16 *s1MatchLen, const Char *s2, UInt16 s2Len, UInt16 *s2MatchLen, Boolean caseless) {
  Int16 i, c1, c2, r = -1;

  if (s1 && s2) {
    for (i = 0; i < s1Len && i < s2Len; i++) {
      c1 = s1[i];
      c2 = s2[i];
      if (caseless) {
        if (c1 >= 'a' && c1 <= 'z') c1 -= 32;
        if (c2 >= 'a' && c2 <= 'z') c2 -= 32;
      }
      r = c1 - c2;
      if (r) break;
    }

    if (s1MatchLen) *s1MatchLen = i;
    if (s2MatchLen) *s2MatchLen = i;
  }

  return r;
}

// Compare the first <s1Len> bytes of <s1> with the first <s2Len> bytes
// of <s2>. Return the results of the comparison: < 0 if <s1> sorts before
// <s2>, > 0 if <s1> sorts after <s2>, and 0 if they are equal. Also return
// the number of bytes that matched in <s1MatchLen> and <s2MatchLen>
// (either one of which can be NULL if the match length is not needed).

Int16 TxtCompare(const Char *s1, UInt16 s1Len, UInt16 *s1MatchLen, const Char *s2, UInt16 s2Len, UInt16 *s2MatchLen) {
  return TxtCompareAux(s1, s1Len, s1MatchLen, s2, s2Len, s2MatchLen, false);
}

// Compare the first <s1Len> bytes of <s1> with the first <s2Len> bytes
// of <s2>. Return the results of the comparison: < 0 if <s1> sorts before
// <s2>, > 0 if <s1> sorts after <s2>, and 0 if they are equal. Also return
// the number of bytes that matched in <s1MatchLen> and <s2MatchLen>
// (either one of which can be NULL if the match length is not needed).
// This comparison is "caseless", in the same manner as a find operation,
// thus case, character size, etc. don't matter.

Int16 TxtCaselessCompare(const Char *s1, UInt16 s1Len, UInt16 *s1MatchLen, const Char *s2, UInt16 s2Len, UInt16 *s2MatchLen) {
  return TxtCompareAux(s1, s1Len, s1MatchLen, s2, s2Len, s2MatchLen, true);
}

const Char *TxtEncodingName(CharEncodingType inEncoding) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "TxtEncodingName not implemented");
  return 0;
}

Err TxtTransliterate(const Char *inSrcText, UInt16 inSrcLength, Char *outDstText, UInt16 *ioDstLength, TranslitOpType inOp) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "TxtTransliterate not implemented");
  return 0;
}
