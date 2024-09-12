#include <PalmOS.h>
#include <CPMLib.h>

#include "sys.h"
#include "pumpkin.h"
#include "CryptoPlugin.h"
#include "sha1.h"
#include "md5.h"
#include "debug.h"

#define pluginId 'iCrp'

static CryptoPluginType plugin;

typedef struct {
  UInt32 type;
  union {
    SHA1_CTX sha1;
    MD5Context md5;
  } ctx;
} internal_hash_ctx_t;

static Boolean hash_init(APHashInfoType *hashInfoP) {
  internal_hash_ctx_t *ctx;
  Boolean r = false;

  switch (hashInfoP->type) {
    case apHashTypeUnspecified:
      // fall-through
    case apHashTypeSHA1:
      if ((ctx = MemPtrNew(sizeof(internal_hash_ctx_t))) != NULL) {
        ctx->type = hashInfoP->type;
        SHA1Init(&ctx->ctx.sha1);
        hashInfoP->providerContext.localContext = ctx;
        hashInfoP->length = SHA1_HASH_LEN;
        r = true;
      }
      break;
    case apHashTypeMD5:
      if ((ctx = MemPtrNew(sizeof(internal_hash_ctx_t))) != NULL) {
        ctx->type = hashInfoP->type;
        md5Init(&ctx->ctx.md5);
        hashInfoP->providerContext.localContext = ctx;
        hashInfoP->length = MD5_HASH_LEN;
        r = true;
      }
      break;
  }

  return r;
}

static void hash_update(APHashInfoType *hashInfoP, UInt8 *input, UInt32 len) {
  internal_hash_ctx_t *ctx = hashInfoP ? (internal_hash_ctx_t *)hashInfoP->providerContext.localContext : NULL;

  if (ctx && input) {
    switch (ctx->type) {
      case apHashTypeSHA1:
        SHA1Update(&ctx->ctx.sha1, input, len);
        break;
      case apHashTypeMD5:
        md5Update(&ctx->ctx.md5, input, len);
        break;
    }
  }
}

static void hash_finalize(APHashInfoType *hashInfoP, UInt8 *hash) {
  internal_hash_ctx_t *ctx = hashInfoP ? (internal_hash_ctx_t *)hashInfoP->providerContext.localContext : NULL;

  if (ctx && hash) {
    switch (ctx->type) {
      case apHashTypeSHA1:
        SHA1Final(hash, &ctx->ctx.sha1);
        break;
      case apHashTypeMD5:
        md5Finalize(&ctx->ctx.md5);
        MemMove(hash, ctx->ctx.md5.digest, MD5_HASH_LEN);
        break;
    }
  }
}

static void hash_free(APHashInfoType *hashInfoP) {
  internal_hash_ctx_t *ctx = hashInfoP ? (internal_hash_ctx_t *)hashInfoP->providerContext.localContext : NULL;

  if (ctx) {
    MemPtrFree(ctx);
    hashInfoP->providerContext.localContext = NULL;
    hashInfoP->length = 0;
  }
}

static const char *info(UInt32 *flags) {
  *flags = APF_MP | APF_HASH;
  return "Internal provider";
}

static void *PluginMain(void *p) {
  MemSet(&plugin, sizeof(CryptoPluginType), 0);

  plugin.info = info;

  plugin.hash_init = hash_init;
  plugin.hash_update = hash_update;
  plugin.hash_finalize = hash_finalize;
  plugin.hash_free = hash_free;

  return &plugin;
}

pluginMainF PluginInit(UInt32 *type, UInt32 *id) {
  *type = cryptoPluginType;
  *id   = pluginId;

  return PluginMain;
}
