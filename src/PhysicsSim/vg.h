void *VgCreate(void *data, uint32_t size);
void VgSize(void *vg, double *width, double *height);
void VgRender(void *vg, int16_t x, int16_t y);
void VgScale(void *vg, double factor);
void VgRotate(void *vg, double angle, double cx, double cy);
void VgFreeze(void *vg);
void VgDestroy(void *vg);
