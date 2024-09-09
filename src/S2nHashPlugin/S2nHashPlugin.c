#include <PalmOS.h>
#include <CPMLib.h>

#include "sys.h"
#include "pumpkin.h"
#include "HashPlugin.h"
#include "debug.h"

#include "crypto/s2n_hash.h"

#define pluginId 's2nH'

static HashPluginType plugin;

typedef struct {
  UInt32 type;
  struct s2n_hash_state state;
} hash_ctx_t;

static void *hashInit(UInt32 type) {
  s2n_tls_hash_algorithm alg;
  hash_ctx_t *ctx = NULL;

  switch (type) {
    case apHashTypeUnspecified:
      // fall-through
    case apHashTypeSHA1:   alg = S2N_TLS_HASH_SHA1; break;
    case apHashTypeSHA256: alg = S2N_TLS_HASH_SHA256; break;
    case apHashTypeSHA384: alg = S2N_TLS_HASH_SHA384; break;
    case apHashTypeSHA512: alg = S2N_TLS_HASH_SHA512; break;
    case apHashTypeMD5:    alg = S2N_TLS_HASH_MD5; break;
    default: alg = -1; break;

    if (alg != -1) {
      if ((ctx = MemPtrNew(sizeof(hash_ctx_t))) != NULL) {
        ctx->type = type;
        if (s2n_hash_new(&ctx->state) == S2N_SUCCESS) {
          if (s2n_hash_init(&ctx->state, alg) != S2N_SUCCESS) {
            MemPtrFree(ctx);
            ctx = NULL;
          }
        } else {
          MemPtrFree(ctx);
          ctx = NULL;
        }
      }
    }
  }

  return ctx;
}

static uint32_t hashSize(void *_ctx) {
  hash_ctx_t *ctx = (hash_ctx_t *)ctx;
  UInt32 size = 0;

  if (ctx) {
    switch (ctx->type) {
      case apHashTypeSHA1:   size = 20; break;
      case apHashTypeSHA256: size = 32; break;
      case apHashTypeSHA384: size = 48; break;
      case apHashTypeSHA512: size = 54; break;
      case apHashTypeMD5:    size = 16; break;
    }
  }

  return size;
}

static void hashUpdate(void *_ctx, UInt8 *input, UInt32 len) {
  hash_ctx_t *ctx = (hash_ctx_t *)ctx;

  if (ctx && input) {
    s2n_hash_update(&ctx->state, input, len);
  }
}

static void hashFinalize(void *_ctx, UInt8 *hash) {
  hash_ctx_t *ctx = (hash_ctx_t *)_ctx;

  if (ctx && hash) {
    s2n_hash_digest(&ctx->state, hash, hashSize(ctx));
  }
}

static void hashFree(void *_ctx) {
  hash_ctx_t *ctx = (hash_ctx_t *)_ctx;

  if (ctx) {
    s2n_hash_free(&ctx->state);
  }
}

static void *PluginMain(void *p) {
  plugin.init = hashInit;
  plugin.size = hashSize;
  plugin.update = hashUpdate;
  plugin.finalize = hashFinalize;
  plugin.free = hashFree;

  return &plugin;
}

pluginMainF PluginInit(UInt32 *type, UInt32 *id) {
  *type = hashPluginType;
  *id   = pluginId;

  return PluginMain;
}
