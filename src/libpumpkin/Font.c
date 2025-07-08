#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "mutex.h"
#include "pwindow.h"
#include "vfs.h"
#include "bytes.h"
#include "AppRegistry.h"
#include "storage.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

typedef struct {
  FontType *fonts[256];
  FontTypeV2 *fontsv2[256];
  FontID currentFont;
  UInt16 density;
} fnt_module_t;

static void adjust(Int16 *r) {
  switch (WinGetRealCoordinateSystem()) {
    case kCoordinatesDouble:    *r *= 2; break;
    case kCoordinatesQuadruple: *r *= 4; break;
  }
}

int FntInitModule(UInt16 density) {
  fnt_module_t *module;
  int i;

  if ((module = xcalloc(1, sizeof(fnt_module_t))) == NULL) {
    return -1;
  }

  pumpkin_set_local_storage(fnt_key, module);
  module->density = density;

  // map all system fonts
  for (i = 0; i < 128; i++) {
    if (density == kDensityLow) {
      module->fonts[i] = pumpkin_get_font(i);
    } else {
      module->fontsv2[i] = (FontTypeV2 *)pumpkin_get_font(i);
    }
  }

  return 0;
}

void *FntReinitModule(void *module) {
  fnt_module_t *old = NULL;

  if (module) {
    FntFinishModule();
    pumpkin_set_local_storage(fnt_key, module);
  } else {
    old = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
    FntInitModule(old->density);
  }

  return old;
}

int FntFinishModule(void) {
  fnt_module_t *module = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
  int i;

  if (module) {
    for (i = 128; i < 256; i++) {
      if (module->fonts[i]) FntFreeFont(module->fonts[i]);
      if (module->fontsv2[i]) FntFreeFont((FontPtr)module->fontsv2[i]);
    }
    xfree(module);
  }

  return 0;
}

FontID FntGetFont(void) {
  fnt_module_t *module = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
  return module->currentFont;
}

FontID FntSetFont(FontID font) {
  fnt_module_t *module = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
  FontID prev = module->currentFont;
  module->currentFont = stdFont;
  if (font >= 0 && font < 256) {
    if (module->fonts[font] || module->fontsv2[font]) {
      module->currentFont = font;
    } else {
      debug(DEBUG_ERROR, "Font", "font %d not installed", font);
    }
  }
  return prev;
}

FontPtr FntGetFontPtr(void) {
  fnt_module_t *module = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
  if (module->fonts[module->currentFont]) {
    return module->fonts[module->currentFont];
  }
  return (FontPtr)module->fontsv2[module->currentFont];
}

Int16 FntBaseLine(void) {
  fnt_module_t *module = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
  Int16 r = 0;
  if (module->fonts[module->currentFont]) {
    r = module->fonts[module->currentFont]->ascent;
  } else if (module->fontsv2[module->currentFont]) {
    r = module->fontsv2[module->currentFont]->ascent;
  }
  adjust(&r);
  return r;
}

Int16 FntCharHeight(void) {
  fnt_module_t *module = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
  Int16 r = 0;
  if (module->fonts[module->currentFont]) {
    r = module->fonts[module->currentFont]->fRectHeight;
  } else if (module->fontsv2[module->currentFont]) {
    r = module->fontsv2[module->currentFont]->fRectHeight;
  }
  adjust(&r);
  return r;
}

Int16 FntLineHeight(void) {
  fnt_module_t *module = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
  Int16 r = 0;
  if (module->fonts[module->currentFont]) {
    r = module->fonts[module->currentFont]->fRectHeight + module->fonts[module->currentFont]->leading;
  } else if (module->fontsv2[module->currentFont]) {
    r = module->fontsv2[module->currentFont]->fRectHeight + module->fontsv2[module->currentFont]->leading;
  }
  adjust(&r);
  return r;
}

Int16 FntAverageCharWidth(void) {
  fnt_module_t *module = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
  Int16 r = 0;
  if (module->fonts[module->currentFont]) {
    r = module->fonts[module->currentFont]->fRectWidth;
  } else if (module->fontsv2[module->currentFont]) {
    r = module->fontsv2[module->currentFont]->fRectWidth;
  }
  adjust(&r);
  return r;
}

Int16 FntCharWidth(Char ch) {
  fnt_module_t *module = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
  UInt8 uc = (UInt8)ch;
  Int16 r = MISSING_SYMBOL_WIDTH;

  if (module->fonts[module->currentFont] && uc >= module->fonts[module->currentFont]->firstChar && uc <= module->fonts[module->currentFont]->lastChar) {
    r = module->fonts[module->currentFont]->width[uc - module->fonts[module->currentFont]->firstChar];
  } else if (module->fontsv2[module->currentFont] && uc >= module->fontsv2[module->currentFont]->firstChar && uc <= module->fontsv2[module->currentFont]->lastChar) {
    r = module->fontsv2[module->currentFont]->width[uc - module->fontsv2[module->currentFont]->firstChar];
  }
  adjust(&r);

  return r;
}

Int16 FntFontCharWidth(FontType *f, Char ch) {
  FontTypeV2 *f2 = (FontTypeV2 *)f;
  UInt8 uc = (UInt8)ch;
  Int16 r = MISSING_SYMBOL_WIDTH;

  if (f->v == 1 && uc >= f->firstChar && uc <= f->lastChar) {
    r = f->width[uc - f->firstChar];
  } else if (f->v == 2 && uc >= f2->firstChar && uc <= f2->lastChar) {
    r = f2->width[uc - f2->firstChar];
  }
  adjust(&r);

  return r;
}

Int16 FntCharsWidth(Char const *chars, Int16 len) {
  FontType *f;
  UInt32 wch;
  char ch;
  Int16 i, r = 0;

  for (i = 0; i < len;) {
    i += pumpkin_next_char((UInt8 *)chars, i, len, &wch);
    ch = pumpkin_map_char(wch, &f);
    r += FntFontCharWidth(f, ch);
  }

  return r;
}

// Gets the width of the specified character. If the specified character
// does not exist within the current font, the missing character symbol is substituted.

Int16 FntWCharWidth(WChar iChar) {
  fnt_module_t *module = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
  UInt8 uc = (UInt16)iChar; // XXX
  Int16 r = MISSING_SYMBOL_WIDTH;
  if (module->fonts[module->currentFont] && uc >= module->fonts[module->currentFont]->firstChar && uc <= module->fonts[module->currentFont]->lastChar) {
    r = module->fonts[module->currentFont]->width[uc - module->fonts[module->currentFont]->firstChar];
  } else if (module->fontsv2[module->currentFont] && uc >= module->fontsv2[module->currentFont]->firstChar && uc <= module->fontsv2[module->currentFont]->lastChar) {
    r = module->fontsv2[module->currentFont]->width[uc - module->fontsv2[module->currentFont]->firstChar];
  }
  adjust(&r);
  return r;
}

Int16 FntDescenderHeight(void) {
  fnt_module_t *module = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
  Int16 r = 0;
  if (module->fonts[module->currentFont]) {
    r = module->fonts[module->currentFont]->descent;
  } else if (module->fontsv2[module->currentFont]) {
    r = module->fontsv2[module->currentFont]->descent;
  }
  adjust(&r);
  return r;
}

// Gets the width of the specified line of text, taking tab characters into
// account. The function assumes that the characters passed are left-
// aligned and that the first character in the string is the first character
// drawn on a line. In other words, this routine doesn’t work for
// characters that don’t start at the beginning of a line.

Int16 FntLineWidth(Char const *pChars, UInt16 length) {
  // XXX does not take into account tab characters
  return FntCharsWidth(pChars, length);
}

// Given a string, determines how many bytes of text can be displayed
// within the specified width with a line break at a tab or space character.

UInt16 FntWordWrap(Char const *chars, UInt16 maxWidth) {
  LineInfoType lineInfo[8];
  RectangleType rect;
  UInt16 totalLines, r = 0;

  if (chars) {
    RctSetRectangle(&rect, 0, 0, maxWidth, FntCharHeight());
    WinDrawCharBox((char *)chars, StrLen(chars), FntGetFont(), &rect, false, NULL, &totalLines, NULL, lineInfo, 8);
    if (totalLines > 0) {
      r = lineInfo[0].length;
    }
  }

  return r;
}

FontPtr FntCopyFont(FontPtr f) {
  FontType *f1;
  FontTypeV2 *f2, *ff;
  uint8_t *bits;
  uint32_t j, glyph_len, glyph_offset;
  Err err;

  if (!f) return NULL;

  if (f->v == 1) {
    f1 = xcalloc(1, sizeof(FontType));
    xmemcpy(f1, f, sizeof(FontType));
    f1->column = xcalloc(f->lastChar - f->firstChar + 1, sizeof(uint16_t));
    xmemcpy(f1->column, f->column, (f->lastChar - f->firstChar + 1) * sizeof(uint16_t));
    f1->width = xcalloc(f->lastChar - f->firstChar + 1, sizeof(uint8_t));
    xmemcpy(f1->width, f->width, (f->lastChar - f->firstChar + 1) * sizeof(uint8_t));
    f1->data = xcalloc(1, f->pitch * f->fRectHeight);
    xmemcpy(f1->data, f->data, f->pitch * f->fRectHeight);
    f1->bmp = BmpCreate3(f->pitch*8, f->fRectHeight, 0, kDensityLow, 1, true, 0, NULL, &err);
    bits = BmpGetBits(f1->bmp);
    xmemcpy(bits, f->data, f->fRectHeight * f->pitch);
    f = f1;
  } else {
    ff = (FontTypeV2 *)f;
    f2 = xcalloc(1, sizeof(FontTypeV2));
    xmemcpy(f2, ff, sizeof(FontTypeV2));
    f2->densities = xcalloc(ff->densityCount, sizeof(FontDensityType));
    xmemcpy(f2->densities, ff->densities, ff->densityCount * sizeof(FontDensityType));
    f2->pitch = xcalloc(ff->densityCount, sizeof(uint16_t));
    xmemcpy(f2->pitch, ff->pitch, ff->densityCount * sizeof(uint16_t));
    f2->column = xcalloc(ff->lastChar - ff->firstChar + 1, sizeof(uint16_t));
    xmemcpy(f2->column, ff->column, (ff->lastChar - ff->firstChar + 1) * sizeof(uint16_t));
    f2->width = xcalloc(ff->lastChar - ff->firstChar + 1, sizeof(uint8_t));
    xmemcpy(f2->width, ff->width, (ff->lastChar - ff->firstChar + 1) * sizeof(uint8_t));
    f2->data = xcalloc(ff->densityCount, sizeof(uint8_t *));
    f2->bmp = xcalloc(ff->densityCount, sizeof(BitmapType *));

    for (j = 0; j < ff->densityCount; j++) {
      glyph_offset = ff->densities[j].glyphBitsOffset;

      switch (ff->densities[j].density) {
        case kDensityLow:
          glyph_len = ff->densities[1].glyphBitsOffset - ff->densities[0].glyphBitsOffset;
          f2->data[j] = xcalloc(1, glyph_len);
          xmemcpy(f2->data[j], ff->data[j], glyph_len);
          f2->bmp[j] = BmpCreate3(ff->pitch[j]*8, ff->fRectHeight, 0, kDensityLow, 1, true, 0, NULL, &err);
          bits = BmpGetBits(f2->bmp[j]);
          xmemcpy(bits, ff->data[j], ff->fRectHeight * ff->pitch[j]);
          break;
        case kDensityDouble:
          glyph_len = ff->size - glyph_offset;
          f2->data[j] = xcalloc(1, glyph_len);
          xmemcpy(f2->data[j], ff->data[j], glyph_len);
          f2->bmp[j] = BmpCreate3(ff->pitch[j]*8, ff->fRectHeight*2, 0, kDensityDouble, 1, true, 0, NULL, &err);
          bits = BmpGetBits(f2->bmp[j]);
          xmemcpy(bits, ff->data[j], ff->fRectHeight * 2 * ff->pitch[j]);
          break;
      }
    }

    f = (FontPtr)f2;
  }

  return f;
}

void FntFreeFont(FontPtr f) {
  FontTypeV2 *ff;
  uint32_t j;

  if (f) {
    if (f->v == 1) {
      BmpDelete(f->bmp);
      xfree(f->column);
      xfree(f->width);
      xfree(f->data);
      xfree(f);
    } else {
      ff = (FontTypeV2 *)f;
      for (j = 0; j < ff->densityCount; j++) {
        BmpDelete(ff->bmp[j]);
        xfree(ff->data[j]);
      }
      xfree(ff->densities);
      xfree(ff->pitch);
      xfree(ff->width);
      xfree(ff->column);
      xfree(ff->data);
      xfree(ff->bmp);
      xfree(ff);
    }
  }
}

Err FntDefineFont(FontID font, FontPtr fontP) {
  fnt_module_t *module = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
  FontTypeV2 *f2;
  Err err = sysErrParamErr;

  if (fontP == NULL) {
    debug(DEBUG_ERROR, "Font", "FntDefineFont: attempt to define NULL font %d", font);
    return err;
  }

  if (font >= 128 && font < 256) {
    if (fontP->v == 1) {
      fontP = FntCopyFont(fontP);
      if (module->fonts[font]) FntFreeFont(module->fonts[font]);
      if (module->fontsv2[font]) FntFreeFont((FontPtr)module->fontsv2[font]);
      module->fonts[font] = fontP;
      module->fontsv2[font] = NULL;
      debug(DEBUG_TRACE, "Font", "FntDefineFont: font %d version %d %p", font, fontP->v, fontP);
      err = errNone;

    } else if (fontP->v == 2) {
      fontP = FntCopyFont(fontP);
      f2 = (FontTypeV2 *)fontP;
      if (module->fonts[font]) FntFreeFont(module->fonts[font]);
      if (module->fontsv2[font]) FntFreeFont((FontPtr)module->fontsv2[font]);
      module->fonts[font] = NULL;
      module->fontsv2[font] = f2;
      debug(DEBUG_TRACE, "Font", "FntDefineFont: font %d version %d %p", font, fontP->v, fontP);
      err = errNone;

    } else {
      debug(DEBUG_ERROR, "Font", "FntDefineFont: invalid font version %d", fontP->v);
      debug_bytes(DEBUG_ERROR, "Font", (uint8_t *)fontP, 32);
    }
  } else {
    debug(DEBUG_ERROR, "Font", "FntDefineFont: attempt to define system font %d", font);
  }

  return err;
}

// Given a pixel position, gets the offset of the character displayed at that location.
Int16 FntWidthToOffset(Char const *pChars, UInt16 length, Int16 pixelWidth, Boolean *leadingEdge, Int16 *truncWidth) {
  FontType *f;
  Int16 tw, width, offset = 0;
  UInt32 wch;
  char ch;

  if (pChars && length) {
    if (leadingEdge) *leadingEdge = false;
    if (truncWidth) *truncWidth = FntCharsWidth(pChars, length);

    for (offset = 0, width = 0; offset < length;) {
      offset += pumpkin_next_char((UInt8 *)pChars, offset, length, &wch);
      ch = pumpkin_map_char(wch, &f);
      tw = FntFontCharWidth(f, ch);

      if ((width + tw) >= pixelWidth) {
         if (leadingEdge) *leadingEdge = false;
         if (truncWidth) *truncWidth = width;
         break;
      }
      width += tw;
    }
  }

  return offset;
}

// Finds the length in bytes of the characters from a specified string that fit within a passed width.
// Trailing spaces or tabs are stripped by this function.

void FntCharsInWidth(Char const *string, Int16 *stringWidthP, Int16 *stringLengthP, Boolean *fitWithinWidth) {
  FontType *f;
  UInt32 wch;
  int i, tw, lineWidth;
  char ch, *s;

  if (string && stringWidthP && stringLengthP && fitWithinWidth) {
    for (i = *stringLengthP-1; i >= 0; i--) {
      if (string[i] != ' ' || string[i] != '\t') break;
    }
    s = (char *)string;
    *stringLengthP = i + 1;

    for (i = 0, lineWidth = 0; i < *stringLengthP && s[i];) {
      i += pumpkin_next_char((UInt8 *)s, i, *stringLengthP, &wch);
      ch = pumpkin_map_char(wch, &f);
      tw = FntFontCharWidth(f, ch);
      if (lineWidth + tw > *stringWidthP) break;
      lineWidth += tw;
    }

    *stringWidthP = lineWidth;
    *stringLengthP = i;
    *fitWithinWidth = (s[i] == 0);
  }
}

// Word wraps a text string backwards by the number of lines
// specified. The character position of the start of the first line and the
// number of lines that are actually word wrapped are returned.

void FntWordWrapReverseNLines(Char const *const chars, UInt16 maxWidth, UInt16 *linesToScrollP, UInt16 *scrollPosP) {
  debug(DEBUG_ERROR, "Font", "FntWordWrapReverseNLines not implemented");
}

// Gets the values needed to update a scroll bar based on a specified
// string and the position within the string.

void FntGetScrollValues(Char const *chars, UInt16 width, UInt16 scrollPos, UInt16 *linesP, UInt16 *topLine) {
  debug(DEBUG_ERROR, "Font", "FntGetScrollValues not implemented");
}

FontType *pumpkin_create_font(void *h, uint8_t *p, uint32_t size, uint32_t *dsize) {
  FontType *font;
  uint16_t fontType;
  uint16_t firstChar;
  uint16_t lastChar;
  uint16_t maxWidth;
  uint16_t kernMax;
  uint16_t nDescent;
  uint16_t fRectWidth;
  uint16_t fRectHeight;
  uint16_t owTLoc;
  uint16_t ascent;
  uint16_t descent;
  uint16_t leading;
  uint16_t rowWords;
  uint16_t column;
  uint8_t offset, width;
  int glyph_len;
  int i, j, k;
  uint8_t *bits;
  Err err;

  i = 0;
  i += get2b(&fontType, p, i);
  i += get2b(&firstChar, p, i);
  i += get2b(&lastChar, p, i);
  i += get2b(&maxWidth, p, i);
  i += get2b(&kernMax, p, i);
  i += get2b(&nDescent, p, i);
  i += get2b(&fRectWidth, p, i);
  i += get2b(&fRectHeight, p, i);
  k = i;
  i += get2b(&owTLoc, p, i);
  i += get2b(&ascent, p, i);
  i += get2b(&descent, p, i);
  i += get2b(&leading, p, i);
  i += get2b(&rowWords, p, i);

  if ((font = StoNewDecodedResource(h, sizeof(FontType), 0, 0)) != NULL) {
    font->v = 1;
    font->fontType = fontType;
    font->firstChar = firstChar;
    font->lastChar = lastChar;
    font->maxWidth = maxWidth;
    font->kernMax = kernMax;
    font->nDescent = nDescent;
    font->fRectWidth = fRectWidth;
    font->fRectHeight = fRectHeight;
    font->owTLoc = owTLoc;
    font->ascent = ascent;
    font->descent = descent;
    font->leading = leading;
    font->rowWords = rowWords;
    font->column = xcalloc(lastChar - firstChar + 1, sizeof(uint16_t));
    font->width = xcalloc(lastChar - firstChar + 1, sizeof(uint8_t));
    debug(DEBUG_TRACE, "Font", "font chars %d to %d, maxWidth %d, rectWidth %d, rectheight %d, rowWords %d",
      firstChar, lastChar, maxWidth, fRectWidth, fRectHeight, rowWords);

    font->pitch = font->rowWords * 2;
    glyph_len = font->pitch * font->fRectHeight;
    font->data = xcalloc(1, glyph_len);
    xmemcpy(font->data, &p[i], glyph_len);
    i += glyph_len;
    debug(DEBUG_TRACE, "Font", "length %d, pitch %d", glyph_len, font->pitch);

    for (j = 0; j < lastChar - firstChar + 1; j++) {
      i += get2b(&column, p, i);
      font->column[j] = column;
      debug(DEBUG_TRACE, "Font", "char %d: column %d", firstChar + j, column);
    }

    i = owTLoc*2 + k;

    font->totalWidth = 0;
    for (j = 0; j < lastChar - firstChar + 1; j++) {
      i += get1(&offset, p, i);
      i += get1(&width, p, i);
      if (offset == 255 && width == 255) {
        debug(DEBUG_TRACE, "Font", "char %d: missing", firstChar + j);
      } else {
        font->width[j] = width;
        font->totalWidth += width;
        //debug(DEBUG_TRACE, "Font", "char %d: offset %d, width %d", firstChar + j, offset, width);
      }
    }
    debug(DEBUG_TRACE, "Font", "totalWidth %d", font->totalWidth);

    font->bmp = BmpCreate3(font->pitch*8, font->fRectHeight, 0, kDensityLow, 1, true, 0, NULL, &err);
    bits = BmpGetBits(font->bmp);
    xmemcpy(bits, font->data, font->fRectHeight * font->pitch);
  }

  *dsize = sizeof(FontType);
  return font;
}

void pumpkin_destroy_font(void *p) {
  FontType *font;

  font = (FontType *)p;
  if (font) {
    if (font->bmp) BmpDelete(font->bmp);
    MemChunkFree(font);
  }
}

FontTypeV2 *pumpkin_create_fontv2(void *h, uint8_t *p, uint32_t size, uint32_t *dsize) {
  FontTypeV2 *font;
  uint16_t fontType;
  uint16_t firstChar;
  uint16_t lastChar;
  uint16_t maxWidth;
  uint16_t kernMax;
  uint16_t nDescent;
  uint16_t fRectWidth;
  uint16_t fRectHeight;
  uint16_t owTLoc;
  uint16_t ascent;
  uint16_t descent;
  uint16_t leading;
  uint16_t rowWords;
  uint16_t version;
  uint16_t densityCount;
  uint16_t density;
  uint32_t glyphBitsOffset;
  uint16_t column;
  uint8_t offset, width;
  int glyph_offset, glyph_len;
  int i, j, k;
  uint8_t *bits;
  Err err;

  i = 0;
  i += get2b(&fontType, p, i);
  i += get2b(&firstChar, p, i);
  i += get2b(&lastChar, p, i);
  i += get2b(&maxWidth, p, i);
  i += get2b(&kernMax, p, i);
  i += get2b(&nDescent, p, i);
  i += get2b(&fRectWidth, p, i);
  i += get2b(&fRectHeight, p, i);
  k = i;
  i += get2b(&owTLoc, p, i);
  i += get2b(&ascent, p, i);
  i += get2b(&descent, p, i);
  i += get2b(&leading, p, i);
  i += get2b(&rowWords, p, i);
  i += get2b(&version, p, i);
  i += get2b(&densityCount, p, i);

  if ((font = StoNewDecodedResource(h, sizeof(FontTypeV2), 0, 0)) != NULL) {
    font->v = 2;
    font->size = size;
    font->fontType = fontType;
    font->firstChar = firstChar;
    font->lastChar = lastChar;
    font->maxWidth = maxWidth;
    font->kernMax = kernMax;
    font->nDescent = nDescent;
    font->fRectWidth = fRectWidth;
    font->fRectHeight = fRectHeight;
    font->owTLoc = owTLoc;
    font->ascent = ascent;
    font->descent = descent;
    font->leading = leading;
    font->rowWords = rowWords;
    font->version = version;
    font->densityCount = densityCount;
    font->densities = xcalloc(densityCount, sizeof(FontDensityType));
    font->pitch = xcalloc(densityCount, sizeof(uint16_t));
    font->column = xcalloc(lastChar - firstChar + 1, sizeof(uint16_t));
    font->width = xcalloc(lastChar - firstChar + 1, sizeof(uint8_t));
    font->data = xcalloc(densityCount, sizeof(uint8_t *));
    font->bmp = xcalloc(densityCount, sizeof(BitmapType *));
    debug(DEBUG_TRACE, "Font", "font version %d, densities %d, chars %d to %d, maxWidth %d, rectWidth %d, rectheight %d, rowWords %d",
      version, densityCount, firstChar, lastChar, maxWidth, fRectWidth, fRectHeight, rowWords);

    for (j = 0; j < densityCount; j++) {
      i += get2b(&density, p, i);
      i += get4b(&glyphBitsOffset, p, i);
      font->densities[j].density = density;
      font->densities[j].glyphBitsOffset = glyphBitsOffset;
      debug(DEBUG_TRACE, "Font", "density %d: %d, offset %d", j, density, glyphBitsOffset);
    }

    for (j = 0; j < lastChar - firstChar + 1; j++) {
      i += get2b(&column, p, i);
      font->column[j] = column;
      debug(DEBUG_TRACE, "Font", "char %d: column %d", firstChar + j, column);
    }

    i = owTLoc*2 + k;

    font->totalWidth = 0;
    for (j = 0; j < lastChar - firstChar + 1; j++) {
      i += get1(&offset, p, i);
      i += get1(&width, p, i);
      if (offset == 255 && width == 255) {
        debug(DEBUG_TRACE, "Font", "char %d: missing", firstChar + j);
      } else {
        font->width[j] = width;
        font->totalWidth += width;
        //debug(DEBUG_TRACE, "Font", "char %d: offset %d, width %d", firstChar + j, offset, width);
      }
    }
    debug(DEBUG_TRACE, "Font", "totalWidth %d", font->totalWidth);

    for (j = 0; j < densityCount; j++) {
      glyph_offset = font->densities[j].glyphBitsOffset;

      switch (font->densities[j].density) {
        case kDensityLow:
          glyph_len = font->densities[1].glyphBitsOffset - font->densities[0].glyphBitsOffset;
          font->pitch[j] = glyph_len / font->fRectHeight;
          font->data[j] = xcalloc(1, glyph_len);
          xmemcpy(font->data[j], &p[glyph_offset], glyph_len);
          font->bmp[j] = BmpCreate3(font->pitch[j]*8, font->fRectHeight, 0, kDensityLow, 1, true, 0, NULL, &err);
          bits = BmpGetBits(font->bmp[j]);
          xmemcpy(bits, font->data[j], font->fRectHeight * font->pitch[j]);
          debug(DEBUG_TRACE, "Font", "single density: offset %d, length %d, pitch %d", glyph_offset, glyph_len, font->pitch[j]);
          break;
        case kDensityDouble:
          glyph_len = size - glyph_offset;
          font->pitch[j] = glyph_len / (font->fRectHeight*2);
          font->data[j] = xcalloc(1, glyph_len);
          xmemcpy(font->data[j], &p[glyph_offset], glyph_len);
          font->bmp[j] = BmpCreate3(font->pitch[j]*8, font->fRectHeight*2, 0, kDensityDouble, 1, true, 0, NULL, &err);
          bits = BmpGetBits(font->bmp[j]);
          xmemcpy(bits, font->data[j], font->fRectHeight * 2 * font->pitch[j]);
          debug(DEBUG_TRACE, "Font", "double density: offset %d, length %d, pitch %d", glyph_offset, glyph_len, font->pitch[j]);
          break;
      }
    }
  }

  *dsize = sizeof(FontTypeV2);
  return font;
}

void pumpkin_destroy_fontv2(void *p) {
  FontTypeV2 *font;
  int i;

  font = (FontTypeV2 *)p;
  if (font) {
    if (font->densities) xfree(font->densities);
    if (font->pitch) xfree(font->pitch);
    if (font->column) xfree(font->column);
    if (font->width) xfree(font->width);

    if (font->bmp) {
      for (i = 0; i < font->densityCount; i++) {
        if (font->bmp[i]) BmpDelete(font->bmp[i]);
      }
      xfree(font->bmp);
    }
    if (font->data) {
      for (i = 0; i < font->densityCount; i++) {
        if (font->data) {
          xfree(font->data[i]);
        }
      }
      xfree(font->data);
    }
    MemChunkFree(font);
  }
}

void FntSaveFontEx(FontPtr font, FontID id) {
  BitmapType *bmp;
  WinHandle wh;
  RectangleType rect;
  Coord width, height;
  UInt16 i, j, k, ch, x, y, w, error;
  char filename[64];

  if (font) {
    if (font->v == 1) {
      width = font->fRectWidth * 16;
      height = font->fRectHeight * 16;
      bmp = BmpCreate3(width, height, 0, kDensityLow, 1, false, 0, NULL, &error);
      wh = WinCreateBitmapWindow(bmp, &error);
      x = y = 0;
      for (i = 0, k = 0, ch = 0; i < 16; i++) {
        x = 0;
        for (j = 0; j < 16; j++, ch++) {
          if (ch >= font->firstChar && ch <= font->lastChar) {
            w = font->width[ch - font->firstChar];
            RctSetRectangle(&rect, k, 0, w, font->fRectHeight);
            WinBlitBitmap(font->bmp, wh, &rect, x + (font->fRectWidth - w) / 2, y, winPaint, false);
            k += w;
          }
          x += font->fRectWidth;
        }
        y += font->fRectHeight;
      }
      StrPrintF(filename, "font_%03d_v1.png", id);
      pumpkin_save_bitmap(bmp, kDensityLow, 0, 0, width, height, filename);
      WinDeleteWindow(wh, false);
      BmpDelete(bmp);
    }
  }
}

void FntSaveFont(FontPtr font, FontID id) {
  FontTypeV2 *font2;
  Coord width, height;
  char filename[64];

  if (font) {
    if (font->v == 1) {
      width = font->pitch*8;
      height = font->fRectHeight;
      StrPrintF(filename, "font_%03d_v1.png", id);
      pumpkin_save_bitmap(font->bmp, kDensityLow, 0, 0, width, height, filename);
    } else {
      font2 = (FontTypeV2 *)font;
      width = font2->pitch[0]*8;
      height = font2->fRectHeight;
      StrPrintF(filename, "font_%03d_v2_low.png", id);
      pumpkin_save_bitmap(font2->bmp[0], kDensityLow, 0, 0, width, height, filename);
      width = font2->pitch[1]*8;
      height = font2->fRectHeight*2;
      StrPrintF(filename, "font_%03d_v2_dbl.png", id);
      pumpkin_save_bitmap(font2->bmp[1], kDensityDouble, 0, 0, width, height, filename);
    }
  }
}

void FntSaveFonts(void) {
  fnt_module_t *module = (fnt_module_t *)pumpkin_get_local_storage(fnt_key);
  FontID font;

  for (font = 0; font < 256; font++) {
    if (module->fonts[font]) {
      FntSaveFont(module->fonts[font], font);
    } else if (module->fontsv2[font]) {
      FntSaveFont((FontPtr)module->fontsv2[font], font);
    }
  }
}

UInt16 FntGetVersion(FontType *f) {
  return f->v;
}

UInt16 FntGetDensityCount(FontType *f) {
  FontTypeV2 *f2;

  if (f->v == 1) return 1;
  f2 = (FontTypeV2 *)f;
  return f2->densityCount;
}

UInt16 FntGetDensity(FontType *f, UInt16 index) {
  FontTypeV2 *f2;

  if (f->v == 1) return kDensityLow;
  f2 = (FontTypeV2 *)f;
  return index < f2->densityCount ? f2->densities[index].density : kDensityLow;
}

UInt16 FntDrawChar(FontType *f, UInt32 wch, UInt8 ch, UInt16 index, UInt16 mult, Coord x, Coord y) {
  WinHandle drawWindow;
  FontTypeV2 *f2;
  RectangleType rect;
  UInt32 col, width = 0;

  if (f) {
    drawWindow = WinGetDrawWindow();

    if (f->v == 1) {
      if (ch >= f->firstChar && ch <= f->lastChar) {
        col = f->column[ch - f->firstChar];
        RctSetRectangle(&rect, col, 0, f->width[ch - f->firstChar], f->fRectHeight);
        WinBlitBitmap(f->bmp, drawWindow, &rect, x, y, WinGetDrawMode(), true);
        width = f->width[ch - f->firstChar];
      } else {
        debug(DEBUG_ERROR, "Font", "v1 missing symbol 0x%04X", wch);
        width = MISSING_SYMBOL_WIDTH;
      }
    } else {
      f2 = (FontTypeV2 *)f;
      if (ch >= f2->firstChar && ch <= f2->lastChar) {
        col = f2->column[ch - f2->firstChar]*mult;
        RctSetRectangle(&rect, col, 0, f2->width[ch - f2->firstChar]*mult, f2->fRectHeight*mult);
        WinBlitBitmap(f2->bmp[index], drawWindow, &rect, x, y, WinGetDrawMode(), true);
        width = f2->width[ch - f2->firstChar]*mult;
      } else {
        debug(DEBUG_ERROR, "Font", "v2 missing symbol 0x%04X", wch);
        width = MISSING_SYMBOL_WIDTH;
      }
    }
  }

  return width;
}
