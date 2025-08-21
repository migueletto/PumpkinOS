#include "sys.h"
#include "script.h"
#include "thread.h"
#include "media.h"
#include "audio.h"
#include "ptr.h"
#include "debug.h"

#include <alsa/asoundlib.h>

#define TAG_AUDIO  "audio"

typedef struct {
  char *tag;
  int pcm, rate, channels;
  int (*getaudio)(void *buffer, int len, void *data);
  void *data;
} alsa_audio_t;

static audio_provider_t audio_provider;

static void audio_destructor(void *p) {
  alsa_audio_t *audio = (alsa_audio_t *)p;

  if (audio) {
    sys_free(audio);
  }
}

static audio_t libaalsa_audio_create(int pcm, int channels, int rate, void *data) {
  alsa_audio_t *audio;
  int ptr = -1;

  debug(DEBUG_INFO, "ALSA", "audio_create(%d,%d,%d)", pcm, channels, rate);

  if ((pcm == PCM_U8 || pcm == PCM_S16 || pcm == PCM_S32) && (channels == 1 || channels == 2) && rate > 0) {
    if ((audio = sys_calloc(1, sizeof(alsa_audio_t))) != NULL) {
      audio->tag = TAG_AUDIO;
      audio->pcm = pcm;
      audio->rate = rate;
      audio->channels = channels;
      if ((ptr = ptr_new(audio, audio_destructor)) == -1) {
        sys_free(audio);
      }
    }
  } else {
    debug(DEBUG_ERROR, "ALSA", "audio_create(%d,%d,%d): invalid arguments", pcm, channels, rate);
  }

  return ptr;
}

static int libaalsa_audio_start(int handle, audio_t _audio, int (*getaudio)(void *buffer, int len, void *data), void *data) {
  alsa_audio_t *audio;
  uint32_t ptr;
  int r = -1;

  if (handle && (audio = ptr_lock(_audio, TAG_AUDIO)) != NULL) {
    audio->getaudio = getaudio;
    audio->data = data;
    ptr_unlock(_audio, TAG_AUDIO);
    ptr = _audio;
    r = thread_client_write(handle, (uint8_t *)&ptr, sizeof(uint32_t)) == sizeof(uint32_t) ? 0 : -1;
  }

  return r;
}

static int libaalsa_audio_destroy(audio_t audio) {
  return ptr_free(audio, TAG_AUDIO);
}

static void error_handler(const char *file, int line, const char *function, int err, const char *fmt, ...) {
  sys_va_list ap;

  sys_va_start(ap, fmt);
  debugva_full(file, function, line, DEBUG_ERROR, "ALSA", fmt, ap);
  sys_va_end(ap);
}

static void debug_buffer(char *prefix, snd_output_t *output) {
  sys_size_t n, i;
  char *buf, *s;

  n = snd_output_buffer_string(output, &buf);
  s = buf;

  if (n > 0 && buf) {
    for (i = 0; i < n; i++) {
      if (buf[i] == '\n') {
        buf[i] = 0;
        debug(DEBUG_TRACE, "ALSA", "%s %s", prefix, s);
        s = &buf[i+1];
      }
    }
    if (s[0]) {
      debug(DEBUG_TRACE, "ALSA", "%s %s", prefix, s);
    }
  }

  snd_output_flush(output);
}

static int32_t alsa_format(snd_pcm_t *snd, int format, int channels, int rate) {
  snd_pcm_hw_params_t *params;
  snd_pcm_sw_params_t *swparams;
  snd_pcm_uframes_t buffer_size;
  snd_pcm_uframes_t chunk_size;
  unsigned int buffer_time, period_time;
  snd_output_t *output;
  int r, exact_rate;

  snd_pcm_hw_params_malloc(&params);

  if ((r = snd_pcm_hw_params_any(snd, params)) < 0) {
    debug(DEBUG_ERROR, "ALSA", "snd_pcm_hw_params_any failed: %s", snd_strerror(r));
    snd_pcm_hw_params_free(params);
    return -1;
  }

  if ((r = snd_pcm_hw_params_set_access(snd, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    debug(DEBUG_ERROR, "ALSA", "snd_pcm_hw_params_set_access failed: %s", snd_strerror(r));
    snd_pcm_hw_params_free(params);
    return -1;
  }

  if ((r = snd_pcm_hw_params_set_format(snd, params, format)) < 0) {
    debug(DEBUG_ERROR, "ALSA", "snd_pcm_hw_params_set_format failed: %s", snd_strerror(r));
    snd_pcm_hw_params_free(params);
    return -1;
  }

  if ((r = snd_pcm_hw_params_set_channels(snd, params, channels)) < 0) {
    debug(DEBUG_ERROR, "ALSA", "snd_pcm_hw_params_set_channels failed: %s", snd_strerror(r));
    snd_pcm_hw_params_free(params);
    return -1;
  }

  exact_rate = rate;
  if ((r = snd_pcm_hw_params_set_rate_near(snd, params, (unsigned int *)&rate, 0)) < 0) {
    debug(DEBUG_ERROR, "ALSA", "snd_pcm_hw_params_set_rate_near failed: %s", snd_strerror(r));
    snd_pcm_hw_params_free(params);
    return -1;
  }

  if (exact_rate != rate) {
    debug(DEBUG_ERROR, "ALSA", "exact rate could not be set, using %d instead of: %d", rate, exact_rate);
  }

  if ((r = snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, 0)) < 0) {
    debug(DEBUG_ERROR, "ALSA", "snd_pcm_hw_params_get_buffer_time_max failed: %s", snd_strerror(r));
    snd_pcm_hw_params_free(params);
    return -1;
  }

  if (buffer_time > 500000) buffer_time = 500000;
  period_time = buffer_time / 4;

  if ((r = snd_pcm_hw_params_set_period_time_near(snd, params, &period_time, 0)) < 0) {
    debug(DEBUG_ERROR, "ALSA", "snd_pcm_hw_params_set_period_time_near failed: %s", snd_strerror(r));
    snd_pcm_hw_params_free(params);
    return -1;
  }

  if ((r = snd_pcm_hw_params_set_buffer_time_near(snd, params, &buffer_time, 0)) < 0) {
    debug(DEBUG_ERROR, "ALSA", "snd_pcm_hw_params_set_buffer_time_near failed: %s", snd_strerror(r));
    snd_pcm_hw_params_free(params);
    return -1;
  }

  snd_output_buffer_open(&output);
  if (debug_getsyslevel("ALSA") == DEBUG_TRACE) {
    snd_pcm_hw_params_dump(params, output);
    debug_buffer("HW param", output);
  }

  if ((r = snd_pcm_hw_params(snd, params)) < 0) {
    debug(DEBUG_ERROR, "ALSA", "snd_pcm_hw_params failed: (%d) %s", r, snd_strerror(r));
    snd_pcm_hw_params_free(params);
    snd_output_close(output);
    return -1;
  }

  snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
  snd_pcm_hw_params_get_buffer_size(params, &buffer_size);

  snd_pcm_hw_params_free(params);
  debug(DEBUG_TRACE, "ALSA", "set format=%d, channels=%d, rate=%d", format, channels, rate);
  debug(DEBUG_TRACE, "ALSA", "chunk_size %ld", chunk_size);
  debug(DEBUG_TRACE, "ALSA", "buffer_size %ld", buffer_size);

  snd_pcm_sw_params_malloc(&swparams);
  snd_pcm_sw_params_current(snd, swparams);

  if ((r = snd_pcm_sw_params_set_avail_min(snd, swparams, chunk_size)) < 0) {
    debug(DEBUG_ERROR, "ALSA", "snd_pcm_sw_params_set_avail_min failed: (%d) %s", r, snd_strerror(r));
    snd_pcm_sw_params_free(swparams);
    snd_output_close(output);
    return -1;
  }

  if ((r = snd_pcm_sw_params_set_start_threshold(snd, swparams, buffer_size)) < 0) {
    debug(DEBUG_ERROR, "ALSA", "snd_pcm_sw_params_set_start_threshold failed: (%d) %s", r, snd_strerror(r));
    snd_pcm_sw_params_free(swparams);
    snd_output_close(output);
    return -1;
  }

  if ((r = snd_pcm_sw_params_set_stop_threshold(snd, swparams, buffer_size)) < 0) {
    debug(DEBUG_ERROR, "ALSA", "snd_pcm_sw_params_set_stop_threshold failed: (%d) %s", r, snd_strerror(r));
    snd_pcm_sw_params_free(swparams);
    snd_output_close(output);
    return -1;
  }

  if ((r = snd_pcm_sw_params(snd, swparams)) < 0) {
    debug(DEBUG_ERROR, "ALSA", "snd_pcm_sw_params failed: (%d) %s", r, snd_strerror(r));
    snd_pcm_sw_params_free(swparams);
    snd_output_close(output);
    return -1;
  }

  if (debug_getsyslevel("ALSA") == DEBUG_TRACE) {
    snd_pcm_sw_params_dump(swparams, output);
    debug_buffer("SW param", output);
  }
  snd_output_close(output);

  snd_pcm_sw_params_free(swparams);

  return buffer_size;
}

static int audio_action(void *arg) {
  alsa_audio_t *audio;
  snd_pcm_t *snd;
  unsigned char *msg;
  unsigned int msglen;
  int32_t bsize, len;
  uint32_t ptr;
  uint8_t *buffer, *buf;
  int frame_size, samples, try, i;
  int format, r;

  debug(DEBUG_INFO, "ALSA", "audio thread starting");
  buffer = NULL;
  bsize = 0;
  snd = NULL;

  for (; !thread_must_end();) {
    if ((r = thread_server_read_timeout(2000, &msg, &msglen)) == -1) {
      break;
    }

    if (r == 1) {
      if (msg) {
        if (msglen == sizeof(uint32_t)) {
          ptr = *((uint32_t *)msg);
          debug(DEBUG_TRACE, "ALSA", "received ptr %d", ptr);

          if ((audio = ptr_lock(ptr, TAG_AUDIO)) != NULL) {
            debug(DEBUG_TRACE, "ALSA", "audio pcm=%d channels=%d rate=%d", audio->pcm, audio->channels, audio->rate);
            switch (audio->pcm) {
              case PCM_U8:
                format = SND_PCM_FORMAT_U8;
                frame_size = audio->channels;
                break;
              case PCM_S16:
                format = SND_PCM_FORMAT_S16_LE;
                frame_size = audio->channels * 2;
                break;
              case PCM_S32:
                format = SND_PCM_FORMAT_S32_LE;
                frame_size = audio->channels * 4;
                break;
              default:
                debug(DEBUG_ERROR, "ALSA", "invalid pcm %d", audio->pcm);
                format = -1;
                break;
            }

            if (snd == NULL) {
              debug(DEBUG_INFO, "ALSA", "open device for playback");
              if ((r = snd_pcm_open(&snd, "default", SND_PCM_STREAM_PLAYBACK, 0)) != 0) {
                debug(DEBUG_ERROR, "ALSA", "snd_pcm_open failed: (%d) %s", r, snd_strerror(r));
                snd = NULL;
              }
            }

            if (snd) {
              if ((bsize = alsa_format(snd, format, audio->channels, audio->rate)) <= 0) {
                ptr_unlock(ptr, TAG_AUDIO);
              }
            }

            if (bsize) {
              buffer = sys_calloc(1, bsize);
              if (buffer == NULL) bsize = 0;
            }

            if (buffer) {
              for (; !thread_must_end();) {
                len = audio->getaudio(buffer, bsize, audio->data);
                debug(DEBUG_TRACE, "ALSA", "get audio len=%d bytes", len);
                if (len <= 0) break;
                buf = buffer;
                for (samples = len / frame_size, i = 0, try = 0; samples > 0;) {
                  debug(DEBUG_TRACE, "ALSA", "writing %d samples", samples);
                  r = snd_pcm_writei(snd, buf, samples);
                  if (r < 0) {
                    debug(DEBUG_ERROR, "ALSA", "write failed (%d): (%d) %s", try, r, snd_strerror(r));
                    if (try == 3) break;
                    try++;
                    continue;
                  } else {
                    debug(DEBUG_TRACE, "ALSA", "writen %d samples", r);
                  }
                  i += r;
                  samples -= r;
                  buf += r * frame_size;
                }
                if (len != bsize) break;
              }
              sys_free(buffer);
              buffer = NULL;
              bsize = 0;
            }
            ptr_unlock(ptr, TAG_AUDIO);
          }
          debug(DEBUG_TRACE, "ALSA", "handled ptr %d", ptr);
          ptr_free(ptr, TAG_AUDIO);
        } else {
          debug(DEBUG_ERROR, "ALSA", "invalid message size %d", msglen);
        }
        sys_free(msg);
      }
    }
  }

  if (snd) {
    debug(DEBUG_INFO, "ALSA", "close device");
    snd_pcm_close(snd);
  }

  debug(DEBUG_INFO, "ALSA", "audio thread exiting");

  return 0;
}

static int libaalsa_audio_init(int pcm, int channels, int rate) {
  return thread_begin("AUDIO", audio_action, NULL);
}

static int libaalsa_audio_finish(int handle) {
  return thread_end("AUDIO", handle);
}

int libaalsa_load(void) {
  snd_lib_error_set_handler(error_handler);

  sys_memset(&audio_provider, 0, sizeof(audio_provider));
  audio_provider.init = libaalsa_audio_init;
  audio_provider.finish = libaalsa_audio_finish;
  audio_provider.create = libaalsa_audio_create;
  audio_provider.start = libaalsa_audio_start;
  audio_provider.destroy = libaalsa_audio_destroy;

  return 0;
}

int libaalsa_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, "ALSA", "registering provider %s", AUDIO_PROVIDER);
  script_set_pointer(pe, AUDIO_PROVIDER, &audio_provider);

  return 0;
}
