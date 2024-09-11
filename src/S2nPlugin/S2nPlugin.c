#include <PalmOS.h>
#include <CPMLib.h>

#include "sys.h"
#include "pumpkin.h"
#include "CryptoPlugin.h"
#include "debug.h"

#include "crypto/s2n_hash.h"
#include "crypto/s2n_cipher.h"
#include "utils/s2n_mem.h"

#define pluginId 's2nC'

static CryptoPluginType plugin;

typedef struct {
  UInt32 type;
  struct s2n_hash_state state;
} hash_ctx_t;

typedef struct {
  UInt32 type;
  UInt32 usage;
  UInt32 keyclass;
  UInt32 length;
  struct s2n_blob key;
} key_ctx_t;

typedef struct {
  Boolean encrypt;
  UInt32 type;
  struct s2n_session_key key;
  struct s2n_blob iv;
  struct s2n_blob in;
  struct s2n_blob out;
} cipher_ctx_t;

static void *hash_init(UInt32 type) {
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

static uint32_t hash_size(void *_ctx) {
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

static void hash_update(void *_ctx, UInt8 *input, UInt32 len) {
  hash_ctx_t *ctx = (hash_ctx_t *)ctx;

  if (ctx && input) {
    s2n_hash_update(&ctx->state, input, len);
  }
}

static void hash_finalize(void *_ctx, UInt8 *hash) {
  hash_ctx_t *ctx = (hash_ctx_t *)_ctx;

  if (ctx && hash) {
    s2n_hash_digest(&ctx->state, hash, hash_size(ctx));
  }
}

static void hash_free(void *_ctx) {
  hash_ctx_t *ctx = (hash_ctx_t *)_ctx;

  if (ctx) {
    s2n_hash_free(&ctx->state);
    MemPtrFree(ctx);
  }
}

static void *key_generate(APKeyInfoType *keyInfoP, UInt8 *data) {
  key_ctx_t *ctx = NULL;

  if (keyInfoP) {
    if ((ctx = MemPtrNew(sizeof(key_ctx_t))) != NULL) {
      if (keyInfoP->keyclass == apKeyClassUnspecified) {
        keyInfoP->keyclass = apKeyClassSymmetric;
      }
      if (keyInfoP->usage == apKeyUsageUnspecified) {
        keyInfoP->usage = apKeyUsageEncryption;
      }
      ctx->type = keyInfoP->type;
      ctx->usage = keyInfoP->usage;
      ctx->keyclass = keyInfoP->keyclass;
      ctx->length = keyInfoP->length;
      if (s2n_alloc(&ctx->key, keyInfoP->length) == S2N_SUCCESS) {
        if (data) {
          MemMove(ctx->key.data, data, keyInfoP->length);
        }
      } else {
        MemPtrFree(ctx);
        ctx = NULL;
      }
    }
  }

  return ctx;
}

static void key_export(void *_ctx, UInt8 *data, UInt32 *len) {
  key_ctx_t *ctx = (key_ctx_t *)_ctx;
  UInt32 n;

  if (ctx && len) {
    if (data == NULL) {
      *len = ctx->length;
    } else {
      n = *len <= ctx->length ? *len : ctx->length;
      MemMove(data, ctx->key.data, n);
    }
  }
}

static void key_release(void *_ctx) {
  key_ctx_t *ctx = (key_ctx_t *)_ctx;

  if (ctx) {
    s2n_free(&ctx->key);
    MemPtrFree(ctx);
  }
}

static const struct s2n_cipher *get_cipher(UInt32 type, UInt32 keyLen) {
  const struct s2n_cipher *cipher = NULL;

  switch (type) {
    case apAlgorithmTypeUnspecified:
      // test with null cipher
      cipher = &s2n_null_cipher;
      break;
    case apSymmetricTypeRijndael:
      switch (keyLen) {
        case 16:
          cipher = &s2n_aes128;
          break;
        case 32:
          cipher = &s2n_aes256;
          break;
        default:
          debug(DEBUG_ERROR, "s2n", "unsupported Rijndael key size %d (%d bits)", keyLen, keyLen * 8);
          cipher = NULL;
          break;
      }
      break;
    default:
      debug(DEBUG_ERROR, "s2n", "unsupported cipher type 0x%02X", type);
      cipher = NULL;
      break;
  }

  return cipher;
}

static void *cipher_init(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, Boolean encrypt) {
  cipher_ctx_t *ctx = NULL;
  key_ctx_t *key_ctx;
  const struct s2n_cipher *cipher;
  int ok;

  if (keyInfoP && cipherInfoP) {
    key_ctx = (key_ctx_t *)keyInfoP->providerContext.localContext;
    cipher = get_cipher(cipherInfoP->type, key_ctx->length);

    if (cipher && cipher->is_available()) {
      if (key_ctx->keyclass == apKeyClassSymmetric && key_ctx->usage == apKeyUsageEncryption) {
        if ((ctx = MemPtrNew(sizeof(cipher_ctx_t))) != NULL) {
          ctx->encrypt = encrypt;
          ctx->type = cipherInfoP->type;
          s2n_session_key_alloc(&ctx->key);
          if (s2n_alloc(&ctx->iv, cipher->io.cbc.record_iv_size) == S2N_SUCCESS) {
            s2n_blob_zero(&ctx->iv);
          }
          if (s2n_alloc(&ctx->in, cipher->io.cbc.block_size) == S2N_SUCCESS) {
            s2n_blob_zero(&ctx->in);
          }
          if (s2n_alloc(&ctx->out, cipher->io.cbc.block_size) == S2N_SUCCESS) {
            s2n_blob_zero(&ctx->out);
          }
          if (s2n_result_is_ok(cipher->init(&ctx->key))) {
            if (encrypt) {
              ok = s2n_result_is_ok(cipher->set_encryption_key(&ctx->key, &key_ctx->key));
            } else {
              ok = s2n_result_is_ok(cipher->set_decryption_key(&ctx->key, &key_ctx->key));
            }
            if (!ok) {
              debug(DEBUG_ERROR, "s2n", "cipher set key failed");
              s2n_session_key_free(&ctx->key);
              s2n_free(&ctx->iv);
              s2n_free(&ctx->in);
              s2n_free(&ctx->out);
              MemPtrFree(ctx);
              ctx = NULL;
            }
          } else {
            debug(DEBUG_ERROR, "s2n", "cipher type 0x%02X init failed", cipherInfoP->type);
            s2n_session_key_free(&ctx->key);
            s2n_free(&ctx->iv);
            s2n_free(&ctx->in);
            s2n_free(&ctx->out);
            MemPtrFree(ctx);
            ctx = NULL;
          }
        }
      } else {
        debug(DEBUG_ERROR, "s2n", "key class 0x%02X or usage 0x%02X not supported", key_ctx->keyclass, key_ctx->usage);
      }
    } else {
      debug(DEBUG_ERROR, "s2n", "cipher type 0x%02X not available", cipherInfoP->type);
    }
  }

  return ctx;
}

static void cipher_update(void *_ctx, APKeyInfoType *keyInfoP, UInt8 *input, UInt32 inputLen, UInt8 *output, UInt32 *outputLen) {
  cipher_ctx_t *ctx = (cipher_ctx_t *)_ctx;
  key_ctx_t *key_ctx;
  UInt32 len, total;
  Int32 r;
  struct s2n_blob in, out;
  const struct s2n_cipher *cipher;

  if (ctx && keyInfoP && output && outputLen) {
    key_ctx = (key_ctx_t *)keyInfoP->providerContext.localContext;
    cipher = get_cipher(ctx->type, key_ctx->length);

    if (cipher && cipher->is_available()) {
      if (key_ctx->keyclass == apKeyClassSymmetric && key_ctx->usage == apKeyUsageEncryption) {
        if (input) {
            switch (cipher->type) {
              case S2N_STREAM:
                if (s2n_blob_init(&in, input, inputLen) == S2N_SUCCESS && s2n_blob_init(&out, output, *outputLen) == S2N_SUCCESS) {
                  if (ctx->encrypt) {
                    cipher->io.stream.encrypt(&ctx->key, &in, &out);
                  } else {
                    cipher->io.stream.decrypt(&ctx->key, &in, &out);
                  }
                  *outputLen = inputLen;
                } else {
                  debug(DEBUG_ERROR, "s2n", "stream s2n_blob_init failed");
                  *outputLen = 0;
                }
                break;
              case S2N_CBC:
                for (total = 0;;) {
                  if (inputLen == 0) break;
                  MemSet(ctx->in.data, cipher->io.cbc.block_size, 0);
                  MemSet(ctx->out.data, cipher->io.cbc.block_size, 0);
                  len = inputLen < cipher->io.cbc.block_size ? inputLen : cipher->io.cbc.block_size;
                  MemMove(ctx->in.data, input, len);
                  if (ctx->encrypt) {
                    r = cipher->io.cbc.encrypt(&ctx->key, &ctx->iv, &ctx->in, &ctx->out);
                    debug(DEBUG_TRACE, "s2n", "stream encrypt %d bytes %s", len, r == S2N_SUCCESS ? "success" : s2n_strerror(s2n_errno, NULL));
                  } else {
                    r = cipher->io.cbc.decrypt(&ctx->key, &ctx->iv, &ctx->in, &ctx->out);
                    debug(DEBUG_TRACE, "s2n", "stream decrypt %d bytes %s", len, r == S2N_SUCCESS ? "success" : s2n_strerror(s2n_errno, NULL));
                  }
                  MemMove(output, ctx->out.data, cipher->io.cbc.block_size);
                  output += cipher->io.cbc.block_size;
                  total += cipher->io.cbc.block_size;
                  inputLen -= len;
                  input += len;
                }
                *outputLen = total;
                break;
              default:
                debug(DEBUG_ERROR, "s2n", "s2n cipher type %d not supported", cipher->type);
                break;
            }
        } else {
          *outputLen = 0;
        }
      } else {
        debug(DEBUG_ERROR, "s2n", "key class 0x%02X or usage 0x%02X not supported", key_ctx->keyclass, key_ctx->usage);
      }
    } else {
      debug(DEBUG_ERROR, "s2n", "cipher type 0x%02X not available", ctx->type);
    }
  }
}

static void cipher_free(void *_ctx) {
  cipher_ctx_t *ctx = (cipher_ctx_t *)_ctx;

  if (ctx) {
    s2n_session_key_free(&ctx->key);
    s2n_free(&ctx->iv);
    s2n_free(&ctx->in);
    s2n_free(&ctx->out);
    MemPtrFree(ctx);
  }
}

static void *PluginMain(void *p) {
  MemSet(&plugin, sizeof(CryptoPluginType), 0);

  plugin.hash_init = hash_init;
  plugin.hash_size = hash_size;
  plugin.hash_update = hash_update;
  plugin.hash_finalize = hash_finalize;
  plugin.hash_free = hash_free;

  plugin.cipher_init = cipher_init;
  plugin.cipher_update = cipher_update;
  plugin.cipher_free = cipher_free;

  plugin.key_generate = key_generate;
  plugin.key_export = key_export;
  plugin.key_release = key_release;

  s2n_init();
  s2n_mem_init();

  return &plugin;
}

pluginMainF PluginInit(UInt32 *type, UInt32 *id) {
  *type = cryptoPluginType;
  *id   = pluginId;

  return PluginMain;
}
