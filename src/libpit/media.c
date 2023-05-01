#include "sys.h"
#include "script.h"
#include "media.h"
#include "audio.h"
#include "pwindow.h"
#include "thread.h"
#include "ptr.h"
#include "debug.h"
#include "xalloc.h"

#define TAG_MEDIA_NODE  "media_node"

#define MAX_NAME 64
#define MAX_NEXT 8

#define TAG_PLAY  "PLAY"

typedef struct {
  char *tag;
  char name[MAX_NAME];
  node_dispatch_t dispatch;
  void *data;
  int ptr_next[MAX_NEXT];
  int nnext;
  int64_t ts;
  window_provider_t *wp;
  window_t *w;
  texture_t *t;
  audio_provider_t *ap;
  audio_t *a;
  int show;
} media_node_t;

typedef struct {
  int pe;
  script_ref_t ref;
  int ptr_node;
  int destroy;
  window_provider_t *wp;
  audio_provider_t *ap;
} media_play_t;

static char *video_enc_names[] = { "unknown", "I420", "YUYV", "UYVY", "RGB", "RGBA", "RGB565", "GRAY", "MONO", "JPEG", "PNG", "OPAQUE", "BGRA" };

char *video_encoding_name(int encoding) {
  return encoding >= 1 && encoding <= ENC_VIDEO_LAST ? video_enc_names[encoding] : video_enc_names[0];
}

static char *audio_enc_names[] = { "unknown", "PCM", "OPAQUE" };

char *audio_encoding_name(int encoding) {
  return encoding >= 1 && encoding <= ENC_AUDIO_LAST ? audio_enc_names[encoding] : audio_enc_names[0];
}

static char *pcm_names[] = { "unknown", "8 bits signed", "8 bits unsigned", "16 bits signed", "16 bits unsigned", "32 bits signed", "32 bits unsigned", "32 bits float", "64 bits double"};

char *pcm_name(int pcm) {
  return pcm >= 1 && pcm <= PCM_LAST ? pcm_names[pcm] : pcm_names[0];
}

int media_frame_put(media_frame_t *f, unsigned char *frame, int len) {
  int r = -1;

  switch (f->meta.type) {
    case FRAME_TYPE_AUDIO:
    case FRAME_TYPE_VIDEO:
      break;
    default:
      debug(DEBUG_ERROR, "MEDIA", "unknown frame type %d", f->meta.type);
      return -1;
  }

  if (f->frame == NULL) {
    if (len) {
      if ((f->frame = xcalloc(1, len)) != NULL) {
        f->alloc = len;
        f->len = len;
        xmemcpy(f->frame, frame, len);
        r = 0;
      } else {
        f->alloc = 0;
        f->len = 0;
      }
    } else {
      f->alloc = 0;
      f->len = 0;
      r = 0;
    }

  } else if (f->alloc < len) {
    if ((f->frame = xrealloc(f->frame, len)) != NULL) {
      f->alloc = len;
      f->len = len;
      xmemcpy(f->frame, frame, len);
      r = 0;
    } else {
      f->alloc = 0;
      f->len = 0;
    }

  } else {
    f->len = len;
    if (frame && len) xmemcpy(f->frame, frame, len);
    r = 0;
  }

  return r;
}

static void node_destroy_callback(void *p) {
  media_node_t *node;

  if (p) {
    node = (media_node_t *)p;
    if (node->dispatch.destroy) node->dispatch.destroy(node->data);
    if (node->w && node->t) node->wp->destroy_texture(node->w, node->t);
    if (node->w) node->wp->destroy(node->w);
    if (node->a) node->ap->destroy(node->a);
    debug(DEBUG_INFO, "MEDIA", "destroyed node %s", node->name);
    xfree(node);
  }
}

int node_destroy(int ptr) {
  int r = -1;

  if (ptr_free(ptr, TAG_MEDIA_NODE) == 0) {
    r = 0;
  }

  return r;
}

int node_destroy_chain(int ptr) {
  media_node_t *node;
  int i, n, next[MAX_NEXT];

  if ((node = (media_node_t *)ptr_lock(ptr, TAG_MEDIA_NODE)) != NULL) {
    n = node->nnext;
    for (i = 0; i < n; i++) {
      next[i] = node->ptr_next[i];
    }
    ptr_unlock(ptr, TAG_MEDIA_NODE);
    ptr_free(ptr, TAG_MEDIA_NODE);

    for (i = 0; i < n; i++) {
      node_destroy_chain(next[i]);
    }
  }

  return 0;
}

int node_connect(int ptr_source, int ptr_next) {
  media_node_t *node;
  int r = -1;

  if ((node = (media_node_t *)ptr_lock(ptr_source, TAG_MEDIA_NODE)) != NULL) {
    if (node->nnext < MAX_NEXT) {
      debug(DEBUG_INFO, "MEDIA", "connect node %d to node %d", ptr_source, ptr_next);
      node->ptr_next[node->nnext++] = ptr_next;
    } else {
      debug(DEBUG_ERROR, "MEDIA", "connect limit reached for node %d", ptr_source);
    }
    ptr_unlock(ptr_source, TAG_MEDIA_NODE);
    r = 0;
  }

  return r;
}

int node_disconnect(int ptr_source, int ptr_next) {
  media_node_t *node;
  int i, r = -1;

  if ((node = (media_node_t *)ptr_lock(ptr_source, TAG_MEDIA_NODE)) != NULL) {
    for (i = 0; i < node->nnext; i++) {
      if (node->ptr_next[i] == ptr_next) break;
    }
    if (i < node->nnext) {
      debug(DEBUG_INFO, "MEDIA", "disconnect node %d from node %d", ptr_source, ptr_next);
      node->nnext--;
      for (; i < node->nnext; i++) {
        node->ptr_next[i] = node->ptr_next[i+1];
      }
      r = 0;
    }
    ptr_unlock(ptr_source, TAG_MEDIA_NODE);
  }

  return r;
}

int node_show(int ptr, int show) {
  media_node_t *node;
  int r = -1;

  if ((node = (media_node_t *)ptr_lock(ptr, TAG_MEDIA_NODE)) != NULL) {
    node->show = show;
    r = 0;
    ptr_unlock(ptr, TAG_MEDIA_NODE);
  }

  return r;
}

int node_create(char *name, node_dispatch_t *dispatch, void *data) {
  media_node_t *node;
  int ptr = -1;

  if ((node = xcalloc(1, sizeof(media_node_t))) != NULL) {
    node->tag = TAG_MEDIA_NODE;
    sys_strncpy(node->name, name, MAX_NAME-1);
    node->dispatch.process = dispatch->process;
    node->dispatch.option = dispatch->option;
    node->dispatch.next = dispatch->next;
    node->dispatch.destroy = dispatch->destroy;
    node->data = data;

    if ((ptr = ptr_new(node, node_destroy_callback)) == -1) {
      xfree(node);
      if (dispatch->destroy) dispatch->destroy(data);
    } else {
      debug(DEBUG_INFO, "MEDIA", "created node %d (%s)", ptr, node->name);
    }
  } else {
    if (dispatch->destroy) dispatch->destroy(data);
  }

  return ptr;
}

int node_option(int ptr, char *name, char *value) {
  media_node_t *node;
  int r = -1;

  if ((node = (media_node_t *)ptr_lock(ptr, TAG_MEDIA_NODE)) != NULL) {
    if (node->dispatch.option) {
      if (node->dispatch.option(name, value, node->data) == 0) {
        if (value) {
          debug(DEBUG_INFO, "MEDIA", "node %d (%s) option '%s' set to '%s'", ptr, node->name, name, value);
        } else {
          debug(DEBUG_INFO, "MEDIA", "node %d (%s) option '%s' performed", ptr, node->name, name);
        }
        r = 0;
      } else {
        debug(DEBUG_ERROR, "MEDIA", "node %d (%s) option '%s' failed", ptr, node->name, name);
      }
    } else {
      debug(DEBUG_ERROR, "MEDIA", "node %d (%s) option '%s' not accepted ", ptr, node->name, name);
    }
    ptr_unlock(ptr, TAG_MEDIA_NODE);
  }

  return r;
}

int node_call(int ptr, char *name, int (*callback)(void *data, void *arg), void *arg) {
  media_node_t *node;
  int r = -1;

  if ((node = (media_node_t *)ptr_lock(ptr, TAG_MEDIA_NODE)) != NULL) {
    if (!sys_strcmp(node->name, name)) {
      r = callback(node->data, arg);
    } else {
      debug(DEBUG_ERROR, "MEDIA", "called node %d (%s) as if it were (%s)", ptr, node->name, name);
    }
    ptr_unlock(ptr, TAG_MEDIA_NODE);
  }

  return r;
}

int64_t node_sync(int ptr) {
  media_node_t *node;
  int64_t ts = 0;

  if ((node = (media_node_t *)ptr_lock(ptr, TAG_MEDIA_NODE)) != NULL) {
    ts = node->ts;
    ptr_unlock(ptr, TAG_MEDIA_NODE);
  }

  return ts;
}

// media_loop returns:
// -1: error, abort action
//  0: all nodes ended processing, abort action
//  1: at least one node is still processing, continue action

int media_loop(int ptr, media_frame_t *frame, void *_wp, void *_ap) {
  media_node_t *node;
  int i, r, width, height, active, ptr_next;
  int arg1, arg2;
  window_provider_t *wp;
  audio_provider_t *ap;

  active = 0;
  wp = (window_provider_t *)_wp;
  ap = (audio_provider_t *)_ap;

  for (; !thread_must_end();) {
    if ((node = (media_node_t *)ptr_lock(ptr, TAG_MEDIA_NODE)) == NULL) {
      return -1;
    }

    if (frame->meta.type == FRAME_TYPE_VIDEO) {
      debug(DEBUG_TRACE, "MEDIA", "processing node %d (%s) video %s %d", ptr, node->name, video_encoding_name(frame->meta.av.v.encoding), frame->len);
    } else if (frame->meta.type == FRAME_TYPE_AUDIO) {
      debug(DEBUG_TRACE, "MEDIA", "processing node %d (%s) audio %s %d", ptr, node->name, audio_encoding_name(frame->meta.av.a.encoding), frame->len);
    } else {
      debug(DEBUG_TRACE, "MEDIA", "processing node %d (%s)", ptr, node->name);
    }

    if ((r = node->dispatch.process(frame, node->data)) < 0) {
      debug(DEBUG_ERROR, "MEDIA", "node %d (%s) process failed", ptr, node->name);
      ptr_unlock(ptr, TAG_MEDIA_NODE);
      return -1;
    }

    if (r == 1) {
      node->ts = frame->ts;
      //debug(DEBUG_INFO, "MEDIA", "node %d (%s) %c ts %lld", ptr, node->name, frame->meta.type == FRAME_TYPE_VIDEO ? 'v' : 'a', node->ts);
      if (frame->meta.type == FRAME_TYPE_VIDEO) {
        if (node->show) {
          node->wp = wp;
          if (!node->w && node->wp) {
            width = frame->meta.av.v.width;
            height = frame->meta.av.v.height;
            node->w = node->wp->create(frame->meta.av.v.encoding, &width, &height, 1, 1, 0, 0, 0, node->wp->data);
            node->t = node->w ? node->wp->create_texture(node->w, width, height) : NULL;
          }
          if (node->t) {
            node->wp->update_texture(node->w, node->t, frame->frame);
            node->wp->draw_texture(node->w, node->t, 0, 0);
            node->wp->render(node->w);
            node->wp->event2(node->w, 0, &arg1, &arg2);
          }
        } else {
          if (node->w) {
            if (node->t) node->wp->destroy_texture(node->w, node->t);
            if (node->w) node->wp->destroy(node->w);
            node->w = NULL;
          }
        }
      } else if (frame->meta.type == FRAME_TYPE_AUDIO && frame->meta.av.a.encoding == ENC_PCM) {
        if (node->show) {
          node->ap = ap;
          if (!node->a && node->ap) {
            node->a = node->ap->create(frame->meta.av.a.pcm, frame->meta.av.a.channels, frame->meta.av.a.rate, node->ap->data);
          }
          if (node->a) {
            node->ap->play(node->a, frame->frame, frame->len);
          }
        } else {
          if (node->a) {
            node->ap->destroy(node->a);
            node->a = NULL;
          }
        }
      }
    }

    active += r;

    if (frame->meta.type == FRAME_TYPE_VIDEO) {
      debug(DEBUG_TRACE, "MEDIA", "processed  node %d (%s) video %s %d r=%d", ptr, node->name, video_encoding_name(frame->meta.av.v.encoding), frame->len, r);
    } else if (frame->meta.type == FRAME_TYPE_AUDIO) {
      debug(DEBUG_TRACE, "MEDIA", "processed  node %d (%s) audio %s %d r=%d", ptr, node->name, audio_encoding_name(frame->meta.av.a.encoding), frame->len, r);
    } else {
      debug(DEBUG_TRACE, "MEDIA", "processed  node %d (%s)", ptr, node->name);
    }

    if (node->nnext > 1 && node->dispatch.next) {
      if ((i = node->dispatch.next(node->data)) < 0) {
        debug(DEBUG_ERROR, "MEDIA", "node %d (%s) next failed", ptr, node->name);
        ptr_unlock(ptr, TAG_MEDIA_NODE);
        return -1;
      }
      ptr_next = node->ptr_next[i % node->nnext];
    } else {
      ptr_next = node->ptr_next[0];
    }

    ptr_unlock(ptr, TAG_MEDIA_NODE);
 
    if (ptr_next == 0) break;
    ptr = ptr_next;
  }

  return active ? 1 : 0;
}

static int media_action(void *arg) {
  media_play_t *p;
  media_frame_t frame;
  script_arg_t ret;
  unsigned char *buf;
  unsigned int n;
  int handle, r;

  p = (media_play_t *)arg;
  xmemset(&frame, 0, sizeof(media_frame_t));
  handle = thread_get_handle();

  for (; !thread_must_end();) {
    if ((r = thread_server_read(&buf, &n)) == -1) break;
    if (r == 1) {
      if (buf) xfree(buf);
    }
    if (media_loop(p->ptr_node, &frame, p->wp, p->ap) <= 0) break;
  }

  if (p->ref) {
    script_call(p->pe, p->ref, &ret, "I", handle);
    script_remove_ref(p->pe, p->ref);
  }
  if (frame.frame) xfree(frame.frame);

  if (p->destroy) {
    node_destroy_chain(p->ptr_node);
    thread_end(TAG_PLAY, handle);
  }
  xfree(p);

  return 0;
}

int media_play(int pe, script_ref_t ref, int ptr_node, int destroy) {
  media_play_t *p;
  int handle = -1;

  if ((p = xcalloc(1, sizeof(media_play_t))) != NULL) {
    p->pe = pe;
    p->ref = ref;
    p->ptr_node = ptr_node;
    p->destroy = destroy;
    p->wp = script_get_pointer(pe, WINDOW_PROVIDER);
    p->ap = script_get_pointer(pe, AUDIO_PROVIDER);

    if ((handle = thread_begin(TAG_PLAY, media_action, p)) == -1) {
      xfree(p);
    }
  }

  return handle;
}

int media_stop(int handle) {
  return thread_end(TAG_PLAY, handle);
}
