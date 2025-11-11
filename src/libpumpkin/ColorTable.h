UInt16 CtbGetNumEntries(ColorTableType *colorTableP);
void CtbSetNumEntries(ColorTableType *colorTableP, UInt16 numEntries);
void CtbGetEntry(ColorTableType *colorTableP, UInt8 index, RGBColorType *rgpP);
void CtbSetEntry(ColorTableType *colorTableP, UInt8 index, RGBColorType *rgpP);
Boolean CtbCompare(ColorTableType *colorTableP1, ColorTableType *colorTableP2);
