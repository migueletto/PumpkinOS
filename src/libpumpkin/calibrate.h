typedef struct {
  int32_t a, b, c, d, e, f, div;
} calibration_t;

void pumpkin_calibrate(window_provider_t *wp, window_t *w, int width, int height, calibration_t *c);
