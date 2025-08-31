#include "sys.h"
#include "script.h"
#include "thread.h"
#include "media.h"
#include "audio.h"
#include "ptr.h"
#include "debug.h"

#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>

#define TAG_AUDIO  "audio"

typedef struct {
  char *tag;
  int pcm, rate, channels, stride;
  int (*getaudio)(void *buffer, int len, void *data);
  void *data;
  struct pw_main_loop *loop;
  struct pw_stream *stream;
} pipewire_audio_t;

static audio_provider_t audio_provider;

static void audio_destructor(void *p) {
  pipewire_audio_t *audio = (pipewire_audio_t *)p;

  if (audio) {
    sys_free(audio);
  }
}

static audio_t libapipewire_audio_create(int pcm, int channels, int rate, void *data) {
  pipewire_audio_t *audio;
  int ptr = -1;

  debug(DEBUG_INFO, "PIPEWIRE", "audio_create(%d,%d,%d)", pcm, channels, rate);

  if ((pcm == PCM_U8 || pcm == PCM_S16 || pcm == PCM_S32) && (channels == 1 || channels == 2) && rate > 0) {
    if ((audio = sys_calloc(1, sizeof(pipewire_audio_t))) != NULL) {
      audio->tag = TAG_AUDIO;
      audio->pcm = pcm;
      audio->rate = rate;
      audio->channels = channels;
      if ((ptr = ptr_new(audio, audio_destructor)) == -1) {
        sys_free(audio);
      }
    }
  } else {
    debug(DEBUG_ERROR, "PIPEWIRE", "audio_create(%d,%d,%d): invalid arguments", pcm, channels, rate);
  }

  return ptr;
}

static int libapipewire_audio_destroy(audio_t audio) {
  return ptr_free(audio, TAG_AUDIO);
}

static int libapipewire_audio_start(int handle, audio_t _audio, int (*getaudio)(void *buffer, int len, void *data), void *data) {
  pipewire_audio_t *audio;
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

static void on_process(void *userdata) {
  pipewire_audio_t *audio = userdata;
  struct pw_buffer *b;
  struct spa_buffer *buf;
  int num_frames, len;
  uint8_t *p;

  if ((b = pw_stream_dequeue_buffer(audio->stream)) == NULL) {
    debug(DEBUG_INFO, "PIPEWIRE", "out of buffers");
    return;
  }

  buf = b->buffer;
  if ((p = buf->datas[0].data) == NULL) return;

  num_frames = buf->datas[0].maxsize / audio->stride;
  if (b->requested) num_frames = SPA_MIN((int)b->requested, num_frames);

  len = audio->getaudio(p, num_frames * audio->stride, audio->data);
  debug(DEBUG_TRACE, "PIPEWIRE", "get audio len=%d bytes (max %d, req %d)", len, buf->datas[0].maxsize, (int)b->requested * audio->stride);
  if (len <= 0) {
    pw_main_loop_quit(audio->loop);
    return;
  }

  buf->datas[0].chunk->offset = 0;
  buf->datas[0].chunk->stride = audio->stride;
  buf->datas[0].chunk->size = len;

  pw_stream_queue_buffer(audio->stream, b);
}

static int audio_action(void *arg) {
  pipewire_audio_t *audio;
  struct pw_stream_events stream_events;
  const struct spa_pod *params[1];
  uint8_t buffer[4096];
  struct pw_properties *props;
  struct spa_pod_builder b;
  unsigned char *msg;
  unsigned int msglen;
  uint32_t ptr;
  int format, r;

  debug(DEBUG_INFO, "PIPEWIRE", "audio thread starting");

  for (; !thread_must_end();) {
    if ((r = thread_server_read_timeout(2000, &msg, &msglen)) == -1) {
      break;
    }

    if (r == 1) {
      if (msg) {
        if (msglen == sizeof(uint32_t)) {
          ptr = *((uint32_t *)msg);
          debug(DEBUG_TRACE, "PIPEWIRE", "received ptr %d", ptr);

          if ((audio = ptr_lock(ptr, TAG_AUDIO)) != NULL) {
            debug(DEBUG_TRACE, "PIPEWIRE", "audio pcm=%d channels=%d rate=%d", audio->pcm, audio->channels, audio->rate);
            switch (audio->pcm) {
              case PCM_U8:
                format = SPA_AUDIO_FORMAT_U8;
		audio->stride = 1;
                break;
              case PCM_S16:
                format = SPA_AUDIO_FORMAT_S16_LE;
		audio->stride = 2;
                break;
              case PCM_S32:
                format = SPA_AUDIO_FORMAT_S32_LE;
		audio->stride = 4;
                break;
              default:
                debug(DEBUG_ERROR, "PIPEWIRE", "invalid pcm %d", audio->pcm);
                format = -1;
                break;
            }

	    if (format != -1) {
	      audio->stride *= audio->channels;
              audio->loop = pw_main_loop_new(NULL);
    
              props = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio",
                  PW_KEY_MEDIA_CATEGORY, "Playback",
                  PW_KEY_MEDIA_ROLE, "Music",
                  NULL);

              sys_memset(&stream_events, 0, sizeof(stream_events));
              stream_events.version = PW_VERSION_STREAM_EVENTS;
              stream_events.process = on_process;

              audio->stream = pw_stream_new_simple(
                 pw_main_loop_get_loop(audio->loop),
                 "audio-src",
                 props,
                 &stream_events,
                 audio);

              b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

              params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat,
                &SPA_AUDIO_INFO_RAW_INIT( 
                .format = format,
                .channels = audio->channels,
                .rate = audio->rate));

              pw_stream_connect(audio->stream,
                  PW_DIRECTION_OUTPUT,
                  PW_ID_ANY,
                  PW_STREAM_FLAG_AUTOCONNECT |
                  PW_STREAM_FLAG_MAP_BUFFERS /*|
                  PW_STREAM_FLAG_RT_PROCESS*/,
                  params, 1);

              pw_main_loop_run(audio->loop);
	      pw_stream_destroy(audio->stream);
              pw_main_loop_destroy(audio->loop);
	    }
            ptr_unlock(ptr, TAG_AUDIO);
          }
          debug(DEBUG_TRACE, "PIPEWIRE", "handled ptr %d", ptr);
          ptr_free(ptr, TAG_AUDIO);
        } else {
          debug(DEBUG_ERROR, "PIPEWIRE", "invalid message size %d", msglen);
        }
        sys_free(msg);
      }
    }
  }


  debug(DEBUG_INFO, "PIPEWIRE", "audio thread exiting");

  return 0;
}

static int libapipewire_audio_init(int pcm, int channels, int rate) {
  return thread_begin("AUDIO", audio_action, NULL);
}

static int libapipewire_audio_finish(int handle) {
  return thread_end("AUDIO", handle);
}

int libapipewire_load(void) {
  int argc = 0;
  char **argv = NULL;

  pw_init(&argc, &argv);

  sys_memset(&audio_provider, 0, sizeof(audio_provider));
  audio_provider.init = libapipewire_audio_init;
  audio_provider.finish = libapipewire_audio_finish;
  audio_provider.create = libapipewire_audio_create;
  audio_provider.start = libapipewire_audio_start;
  audio_provider.destroy = libapipewire_audio_destroy;

  return 0;
}

int libapipewire_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, "PIPEWIRE", "registering provider %s", AUDIO_PROVIDER);
  script_set_pointer(pe, AUDIO_PROVIDER, &audio_provider);

  return 0;
}

int libapipewire_unload(void) {
  pw_deinit();

  return 0;
}
