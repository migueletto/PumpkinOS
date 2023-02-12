#ifndef PIT_MEDIA_H
#define PIT_MEDIA_H

#ifdef __cplusplus
extern "C" {
#endif

#define FRAME_TYPE_VIDEO 1
#define FRAME_TYPE_AUDIO 2

#define ENC_I420   1  // planar Y U V
#define ENC_YUYV   2  // packed Y1 U Y2 V
#define ENC_UYVY   3  // packed U Y1 V Y2
#define ENC_RGB    4  // R G B
#define ENC_RGBA   5  // R G B A
#define ENC_RGB565 6  // R(5) G(6) B(5)
#define ENC_GRAY   7  // Y (8 bits)
#define ENC_MONO   8  // Y (1 bit)
#define ENC_JPEG   9
#define ENC_PNG    10
#define ENC_OPAQUE_VIDEO 11
#define ENC_BGRA   12
#define ENC_VIDEO_LAST ENC_BGRA

#define ENC_PCM    1
#define ENC_OPAQUE_AUDIO 2
#define ENC_AUDIO_LAST ENC_OPAQUE_AUDIO

#define PCM_S8     1
#define PCM_U8     2
#define PCM_S16    3
#define PCM_U16    4
#define PCM_S32    5
#define PCM_U32    6
#define PCM_FLT    7
#define PCM_DBL    8
#define PCM_LAST   PCM_DBL

#define DEFAULT_JPEG_QUALITY 85

typedef struct {
  int width, height, fwidth, fheight;
  int ar_num, ar_den, tb_num, tb_den;
  int encoding, native_encoding;
} video_meta_t;

typedef struct {
  int pcm, channels, rate;
  int encoding, native_encoding;
  int tb_num, tb_den;
} audio_meta_t;

typedef struct {
  int type;
  union {
    audio_meta_t a;
    video_meta_t v;
  } av;
} media_meta_t;

typedef struct {
  media_meta_t meta;
  int64_t ts;
  int alloc, len;
  unsigned char *frame;
} media_frame_t;

typedef struct {
  int (*process)(media_frame_t *frame, void *data);

  int (*option)(char *name, char *value, void *data);

  int (*next)(void *data);

  int (*destroy)(void *data);
} node_dispatch_t;

int node_create(char *name, node_dispatch_t *dispatch, void *data);

int node_option(int ptr, char *name, char *value);

int node_destroy(int ptr);

int node_destroy_chain(int ptr);

int node_connect(int ptr_source, int ptr_next);

int node_disconnect(int ptr_source, int ptr_next);

int node_show(int ptr, int show);

int node_call(int ptr, char *name, int (*callback)(void *data, void *arg), void *arg);

int media_loop(int ptr, media_frame_t *frame, void *wp, void *ap);

int media_play(int pe, script_ref_t ref, int ptr_node, int destroy);

int media_stop(int handle);

int64_t node_sync(int ptr);

int media_frame_put(media_frame_t *f, unsigned char *frame, int len);

char *video_encoding_name(int encoding);

char *audio_encoding_name(int encoding);

char *pcm_name(int pcm);

#ifdef __cplusplus
}
#endif

#endif
