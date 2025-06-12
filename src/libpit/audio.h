#ifndef PIT_AUDIO_H
#define PIT_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_PROVIDER "audio_provider"

typedef int audio_t;

typedef struct {
  int (*init)(int pcm, int channels, int rate);
  int (*finish)(int handle);
  audio_t (*create)(int pcm, int channels, int rate, void *data);
  int (*start)(int handle, audio_t audio, int (*getaudio)(void *buffer, int len, void *data), void *data);
  int (*play)(audio_t audio, uint8_t *raw, int len);
  int (*destroy)(audio_t audio);

  int (*mixer_init)(void);
  int (*mixer_play)(uint8_t *buf, uint32_t len, int volume);
  int (*mixer_stop)(void);

  void *data;
} audio_provider_t;

#ifdef __cplusplus
}
#endif

#endif
