typedef struct VectorGraphicsType VectorGraphicsType;

VectorGraphicsType *VgCreate(char *s, UInt32 size);
void VgSize(VectorGraphicsType *vg, double *width, double *height);
void VgRender(VectorGraphicsType *vg, Coord x, Coord y);
void VgScale(VectorGraphicsType *vg, double factor);
void VgRotate(VectorGraphicsType *vg, double angle);
void VgDestroy(VectorGraphicsType *vg);
