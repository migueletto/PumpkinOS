void FontInit(Boolean highDensity);
void FontFinish(void);
Err FontSet(FontID f);
void FontStringSize(char *s, Int16 *dx, Int16 *dy);
void FontDrawString(char *s, Int16 x, Int16 y, Int16 angle);
