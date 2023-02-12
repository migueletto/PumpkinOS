#ifndef PIT_SCALE_H
#define PIT_SCALE_H

#ifdef __cplusplus
extern "C" {
#endif

#define SCALE_PROVIDER "scale_provider"

typedef struct {
  int (*scale)(surface_t *src, surface_t *dst);
} scale_provider_t;

#ifdef __cplusplus
}
#endif

#endif
