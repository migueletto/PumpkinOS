#define LABEL_N		0x01
#define LABEL_S		0x02
#define LABEL_W		0x04
#define LABEL_E		0x08
#define LABEL_VALUE	0x10

void DrawCompass(Int16 x, Int16 y, Int16 r, UInt8 labels);
void DrawArrow(Int16 x, Int16 y, Int16 r, Int16 h, double g, Boolean full, Boolean draw);
void DrawCircle(Int16 x, Int16 y, Int16 r);
