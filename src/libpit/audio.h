#ifndef PIT_AUDIO_H
#define PIT_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_PROVIDER "audio_provider"

typedef void *audio_t;

typedef struct {
  audio_t *(*create)(char *server, int pcm, int channels, int rate);

  int (*play)(audio_t *audio, uint8_t *raw, int len);

  int (*destroy)(audio_t *audio);
} audio_provider_t;

#ifdef __cplusplus
}
#endif

#endif