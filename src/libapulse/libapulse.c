#include "sys.h"
#include "script.h"
#include "thread.h"
#include "media.h"
#include "audio.h"
#include "ptr.h"
#include "debug.h"

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#define TAG_AUDIO  "audio"
#define BUFFER_SIZE 4096

typedef struct {
  char *tag;
  int pcm, rate, channels;
  int (*getaudio)(void *buffer, int len, void *data);
  uint8_t buffer[BUFFER_SIZE];
  void *data;
} pulse_audio_t;

static audio_provider_t audio_provider;

static void audio_destructor(void *p) {
  pulse_audio_t *audio = (pulse_audio_t *)p;

  if (audio) {
    sys_free(audio);
  }
}

static audio_t libapulse_audio_create(int pcm, int channels, int rate, void *data) {
  pulse_audio_t *audio;
  int ptr = -1;

  debug(DEBUG_INFO, "PULSE", "audio_create(%d,%d,%d)", pcm, channels, rate);

  if ((pcm == PCM_U8 || pcm == PCM_S16 || pcm == PCM_S32) && (channels == 1 || channels == 2) && rate > 0) {
    if ((audio = sys_calloc(1, sizeof(pulse_audio_t))) != NULL) {
      audio->tag = TAG_AUDIO;
      audio->pcm = pcm;
      audio->rate = rate;
      audio->channels = channels;
      if ((ptr = ptr_new(audio, audio_destructor)) == -1) {
        sys_free(audio);
      }
    }
  } else {
    debug(DEBUG_ERROR, "PULSE", "audio_create(%d,%d,%d): invalid arguments", pcm, channels, rate);
  }

  return ptr;
}

static int libapulse_audio_start(int handle, audio_t _audio, int (*getaudio)(void *buffer, int len, void *data), void *data) {
  pulse_audio_t *audio;
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

static int libapulse_audio_destroy(audio_t audio) {
  return ptr_free(audio, TAG_AUDIO);
}

static int audio_action(void *arg) {
  pulse_audio_t *audio;
  pa_sample_spec ss;
  pa_simple *s;
  unsigned char *msg;
  unsigned int msglen;
  uint32_t ptr;
  int len, format, err, r;

  debug(DEBUG_INFO, "PULSE", "audio thread starting");
  s = NULL;

  for (; !thread_must_end();) {
    if ((r = thread_server_read_timeout(2000, &msg, &msglen)) == -1) {
      break;
    }

    if (r == 1) {
      if (msg) {
        if (msglen == sizeof(uint32_t)) {
          ptr = *((uint32_t *)msg);
          debug(DEBUG_TRACE, "PULSE", "received ptr %d", ptr);

          if ((audio = ptr_lock(ptr, TAG_AUDIO)) != NULL) {
            debug(DEBUG_TRACE, "PULSE", "audio pcm=%d channels=%d rate=%d", audio->pcm, audio->channels, audio->rate);
            switch (audio->pcm) {
              case PCM_U8:
                format = PA_SAMPLE_U8;
                break;
              case PCM_S16:
                format = PA_SAMPLE_S16LE;
                break;
              case PCM_S32:
                format = PA_SAMPLE_S32LE;
                break;
              default:
                debug(DEBUG_ERROR, "PULSE", "invalid pcm %d", audio->pcm);
                format = -1;
                break;
            }

            if (s == NULL) {
              sys_memset(&ss, 0, sizeof(pa_sample_spec));
              ss.format = format;
              ss.channels = audio->channels;
              ss.rate = audio->rate;

              debug(DEBUG_INFO, "PULSE", "open device for playback");
              s = pa_simple_new(
                    NULL,               // server name
                    SYSTEM_NAME,        // application name
                    PA_STREAM_PLAYBACK, // data direction
                    NULL,               // default device
                    "audio",            // stream description
                    &ss,                // sample format
                    NULL,               // default channel map
                    NULL,               // default buffering attributes
                    &err                // error code
                    );

              if (s == NULL) {
                debug(DEBUG_ERROR, "PULSE", "pa_simple_new failed (%d): %s", err, pa_strerror(err));
              }
            }

            if (s) {
              for (; !thread_must_end();) {
                len = audio->getaudio(audio->buffer, BUFFER_SIZE, audio->data);
                debug(DEBUG_TRACE, "PULSE", "get audio len=%d bytes", len);
                if (len <= 0) break;
                debug(DEBUG_TRACE, "PULSE", "writing %d bytes", len);
                if (pa_simple_write(s, audio->buffer, len, &err) < 0) {
                  debug(DEBUG_ERROR, "PULSE", "write failed (%d): %s", r, pa_strerror(err));
                  break;
                } else {
                  debug(DEBUG_TRACE, "PULSE", "writen %d bytes", len);
                }
                if (len != BUFFER_SIZE) break;
              }
            }
            ptr_unlock(ptr, TAG_AUDIO);
          }
          debug(DEBUG_TRACE, "PULSE", "handled ptr %d", ptr);
          ptr_free(ptr, TAG_AUDIO);
        } else {
          debug(DEBUG_ERROR, "PULSE", "invalid message size %d", msglen);
        }
        sys_free(msg);
      }
    }
  }

  if (s) {
    debug(DEBUG_INFO, "PULSE", "close device");
    pa_simple_drain(s, &err);
    pa_simple_free(s);
  }

  debug(DEBUG_INFO, "PULSE", "audio thread exiting");

  return 0;
}

static int libapulse_audio_init(int pcm, int channels, int rate) {
  return thread_begin("AUDIO", audio_action, NULL);
}

static int libapulse_audio_finish(int handle) {
  return thread_end("AUDIO", handle);
}

int libapulse_load(void) {
  sys_memset(&audio_provider, 0, sizeof(audio_provider));
  audio_provider.init = libapulse_audio_init;
  audio_provider.finish = libapulse_audio_finish;
  audio_provider.create = libapulse_audio_create;
  audio_provider.start = libapulse_audio_start;
  audio_provider.destroy = libapulse_audio_destroy;

  return 0;
}

int libapulse_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, "PULSE", "registering provider %s", AUDIO_PROVIDER);
  script_set_pointer(pe, AUDIO_PROVIDER, &audio_provider);

  return 0;
}
