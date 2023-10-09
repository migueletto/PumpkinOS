#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int32_t a, b, c, d, e, f, div;
} calibration_t;

void calibrate(window_provider_t *wp, window_t *w, int width, int height, int extra_height, calibration_t *c);

#ifdef __cplusplus
}
#endif
