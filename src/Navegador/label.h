void LabelInit(void);
void LabelFinish(Boolean highDensity);
int LabelAdd(Int16 type, Int16 x, Int16 y, FontID font, UInt16 color, float angle, char *s);
gpc_polygon *LabelBox(gpc_polygon *p, Int16 x, Int16 y, Int16 dx, Int16 dy, Int16 *ok);
