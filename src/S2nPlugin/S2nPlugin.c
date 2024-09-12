#include <PalmOS.h>
#include <CPMLib.h>

#include "sys.h"
#include "bytes.h"
#include "pumpkin.h"
#include "CryptoPlugin.h"
#include "debug.h"

#include "crypto/s2n_hash.h"
#include "crypto/s2n_cipher.h"
#include "utils/s2n_random.h"
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

static Boolean hash_init(APHashInfoType *hashInfoP) {
  s2n_tls_hash_algorithm alg;
  hash_ctx_t *ctx;
  UInt32 length;
  Boolean r = false;

  switch (hashInfoP->type) {
    case apHashTypeUnspecified:
      // fall-through
    case apHashTypeSHA1:   alg = S2N_TLS_HASH_SHA1;   length = 20; break;
    case apHashTypeSHA256: alg = S2N_TLS_HASH_SHA256; length = 32; break;
    case apHashTypeSHA384: alg = S2N_TLS_HASH_SHA384; length = 48; break;
    case apHashTypeSHA512: alg = S2N_TLS_HASH_SHA512; length = 64; break;
    case apHashTypeMD5:    alg = S2N_TLS_HASH_MD5;    length = 16; break;
    default: alg = -1; break;
  }

  if (alg != -1) {
    if ((ctx = MemPtrNew(sizeof(hash_ctx_t))) != NULL) {
      ctx->type = hashInfoP->type;
      if (s2n_hash_new(&ctx->state) == S2N_SUCCESS) {
        if (s2n_hash_init(&ctx->state, alg) == S2N_SUCCESS) {
          hashInfoP->providerContext.localContext = ctx;
          hashInfoP->length = length;
          r = true;
        } else {
          MemPtrFree(ctx);
        }
      } else {
        MemPtrFree(ctx);
      }
    }
  }

  return r;
}

static void hash_update(APHashInfoType *hashInfoP, UInt8 *input, UInt32 len) {
  hash_ctx_t *ctx = hashInfoP ? (hash_ctx_t *)hashInfoP->providerContext.localContext : NULL;

  if (ctx && input) {
    s2n_hash_update(&ctx->state, input, len);
  }
}

static void hash_finalize(APHashInfoType *hashInfoP, UInt8 *hash) {
  hash_ctx_t *ctx = hashInfoP ? (hash_ctx_t *)hashInfoP->providerContext.localContext : NULL;

  if (ctx && hash) {
    s2n_hash_digest(&ctx->state, hash, hashInfoP->length);
  }
}

static void hash_free(APHashInfoType *hashInfoP) {
  hash_ctx_t *ctx = hashInfoP ? (hash_ctx_t *)hashInfoP->providerContext.localContext : NULL;

  if (ctx) {
    s2n_hash_free(&ctx->state);
    MemPtrFree(ctx);
    hashInfoP->providerContext.localContext = NULL;
    hashInfoP->length = 0;
  }
}

static Boolean random_get(UInt8 *data, UInt32 len) {
  uint64_t output;
  UInt32 i;
  Boolean r = false;

  if (data) {
    for (i = 0; i < len; i++) {
      if (!s2n_result_is_ok(s2n_public_random(255, &output))) break;
      data[i] = output & 0xFF;
    }
    r = i == len;
  }

  return r;
}

static Boolean key_generate(APKeyInfoType *keyInfoP, UInt8 *data) {
  key_ctx_t *ctx;
  Boolean r = false;

  if (keyInfoP) {
    if ((ctx = MemPtrNew(sizeof(key_ctx_t))) != NULL) {
      if (s2n_alloc(&ctx->key, keyInfoP->length) == S2N_SUCCESS) {
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
        keyInfoP->actualLength = keyInfoP->length;
        keyInfoP->exportable = 1;

        if (data) {
          MemMove(ctx->key.data, data, keyInfoP->length);
        } else {
          random_get(ctx->key.data, keyInfoP->length);
        }

        keyInfoP->providerContext.localContext = ctx;
        r = true;

      } else {
        MemPtrFree(ctx);
        ctx = NULL;
      }
    }
  }

  return r;
}

static UInt32 der_get_integer(UInt32 *value, UInt8 *data, UInt32 i) {
  UInt8 type, length;

  i += get1(&type, data, i);   // type
  if (type != 2) return 0;

  i += get1(&length, data, i); // length
  if (length != 4) return 0;

  i += get4l(value, data, i);  // value

  return i;
}

static UInt32 der_get_octet_string(UInt32 len, UInt8 *data, UInt32 i) {
  UInt8 type, length;

  i += get1(&type, data, i);   // type
  if (type != 4) return 0;

  i += get1(&length, data, i); // length
  if (length != len) return 0;

  i += length;

  return i;
}

static Boolean key_import(APKeyInfoType *keyInfoP, UInt8 encoding, UInt8 *data, UInt32 len) {
  key_ctx_t *ctx;
  UInt32 i;
  Boolean r = false;

  if (keyInfoP && data) {
    if ((ctx = MemPtrNew(sizeof(key_ctx_t))) != NULL) {
      switch (encoding) {
        case IMPORT_EXPORT_TYPE_RAW:
          if (len > 4 * sizeof(UInt32)) {
            i  = get4l(&ctx->type,     data, 0);
            i += get4l(&ctx->usage,    data, i);
            i += get4l(&ctx->keyclass, data, i);
            i += get4l(&ctx->length,   data, i);
          }
          break;
        case IMPORT_EXPORT_TYPE_DER:
          if (len <= 4 * 6 + 2 ||
              !(i = der_get_integer(&ctx->type,     data, 0)) ||
              !(i = der_get_integer(&ctx->usage,    data, i)) ||
              !(i = der_get_integer(&ctx->keyclass, data, i)) ||
              !(i = der_get_integer(&ctx->length,   data, i)) ||
              !der_get_octet_string(ctx->length, data, i)) {

            debug(DEBUG_ERROR, "s2n", "invalid DER encoded key (%u %u %u %u %u)",
              len, ctx->type, ctx->usage, ctx->keyclass, ctx->length);
            ctx->length = 0;
          }
          break;
        default:
          debug(DEBUG_ERROR, "s2n", "invalid import encoding %u", encoding);
          break;
      }

      if (ctx->usage == apKeyUsageEncryption && ctx->keyclass == apKeyClassSymmetric &&
          ctx->length > 0 && s2n_alloc(&ctx->key, ctx->length) == S2N_SUCCESS) {

        keyInfoP->type = ctx->type;
        keyInfoP->usage = ctx->usage;
        keyInfoP->keyclass = ctx->keyclass;
        keyInfoP->length = ctx->length;
        keyInfoP->actualLength = keyInfoP->length;
        keyInfoP->exportable = 1;

        MemMove(ctx->key.data, data + i, keyInfoP->length);
        r = true;

      } else {
        MemPtrFree(ctx);
        ctx = NULL;
      }
    }
  }

  return r;
}

static UInt32 der_put_integer(UInt32 value, UInt8 *data, UInt32 i) {
  i += put1(2, data, i);       // type: Integer
  i += put1(4, data, i);       // length
  i += put4l(value, data, i);  // value

  return i;
}

static UInt32 der_put_octet_string(UInt8 *value, UInt32 len, UInt8 *data, UInt32 i) {
  i += put1(4, data, i);       // type: Octet String
  i += put1(len, data, i);       // length
  MemMove(data + i, value, len);
  i += len;

  return i;
}

static Boolean key_export(APKeyInfoType *keyInfoP, UInt8 encoding, UInt8 *data, UInt32 *len) {
  key_ctx_t *ctx = keyInfoP ? (key_ctx_t *)keyInfoP->providerContext.localContext : NULL;
  UInt32 encoded_len, i;
  Boolean r = false;

  if (ctx && len) {
    switch (encoding) {
      case IMPORT_EXPORT_TYPE_RAW:
        encoded_len = 4 * sizeof(UInt32) + keyInfoP->length;
        if (data == NULL) {
          *len = encoded_len;
        } else if (*len >= encoded_len) {
          i  = put4l(ctx->type,     data, 0);
          i += put4l(ctx->usage,    data, i);
          i += put4l(ctx->keyclass, data, i);
          i += put4l(ctx->length,   data, i);
          MemMove(data + i, ctx->key.data, keyInfoP->length);
        }
        r = true;
        break;
      case IMPORT_EXPORT_TYPE_DER:
        encoded_len = 4 * 6 + 2 + keyInfoP->length;
        if (data == NULL) {
          *len = encoded_len;
        } else if (*len >= encoded_len) {
          i = der_put_integer(ctx->type, data, 0);
          i = der_put_integer(ctx->usage, data, i);
          i = der_put_integer(ctx->keyclass, data, i);
          i = der_put_integer(ctx->length, data, i);
          i = der_put_octet_string(ctx->key.data, keyInfoP->length, data, i);
        }
        r = true;
        break;
      default:
        debug(DEBUG_ERROR, "s2n", "invalid export encoding %u", encoding);
        break;
    }
  }

  return r;
}

static void key_release(APKeyInfoType *keyInfoP) {
  key_ctx_t *ctx = keyInfoP ? (key_ctx_t *)keyInfoP->providerContext.localContext : NULL;

  if (ctx) {
    s2n_free(&ctx->key);
    MemPtrFree(ctx);
    keyInfoP->providerContext.localContext = NULL;
  }
}

static const struct s2n_cipher *get_cipher(UInt32 type, UInt32 keyLen) {
  const struct s2n_cipher *cipher = NULL;

  switch (type) {
    case apAlgorithmTypeUnspecified:
      // test with null cipher
      cipher = &s2n_null_cipher;
      break;
    case apSymmetricTypeARC4:
      if (keyLen == s2n_rc4.key_material_size) {
        cipher = &s2n_rc4;
      } else {
        debug(DEBUG_ERROR, "s2n", "unsupported RC4 key size %d (%d bits)", keyLen, keyLen * 8);
        cipher = NULL;
      }
      break;
    case apSymmetricType3DES_EDE3:
      if (keyLen == s2n_3des.key_material_size) {
        cipher = &s2n_3des;
      } else {
        debug(DEBUG_ERROR, "s2n", "unsupported 3DES key size %d (%d bits)", keyLen, keyLen * 8);
        cipher = NULL;
      }
      break;
    case apSymmetricTypeRijndael:
      if (keyLen == s2n_aes128.key_material_size) {
        cipher = &s2n_aes128;
      } else if (keyLen == s2n_aes256.key_material_size) {
        cipher = &s2n_aes256;
      } else {
        debug(DEBUG_ERROR, "s2n", "unsupported Rijndael key size %d (%d bits)", keyLen, keyLen * 8);
        cipher = NULL;
      }
      break;
    default:
      debug(DEBUG_ERROR, "s2n", "unsupported cipher type 0x%02X", type);
      cipher = NULL;
      break;
  }

  return cipher;
}

static Boolean cipher_init(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, Boolean encrypt) {
  cipher_ctx_t *ctx;
  key_ctx_t *key_ctx;
  const struct s2n_cipher *cipher;
  int ok;
  Boolean r = false;

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
            if (ok) {
              cipherInfoP->providerContext.localContext = ctx;
              r = true;
            } else {
              debug(DEBUG_ERROR, "s2n", "cipher set key failed");
              s2n_session_key_free(&ctx->key);
              s2n_free(&ctx->iv);
              s2n_free(&ctx->in);
              s2n_free(&ctx->out);
              MemPtrFree(ctx);
            }
          } else {
            debug(DEBUG_ERROR, "s2n", "cipher type 0x%02X init failed", cipherInfoP->type);
            s2n_session_key_free(&ctx->key);
            s2n_free(&ctx->iv);
            s2n_free(&ctx->in);
            s2n_free(&ctx->out);
            MemPtrFree(ctx);
          }
        }
      } else {
        debug(DEBUG_ERROR, "s2n", "key class 0x%02X or usage 0x%02X not supported", key_ctx->keyclass, key_ctx->usage);
      }
    } else {
      debug(DEBUG_ERROR, "s2n", "cipher type 0x%02X not %s", cipherInfoP->type, cipher ? "available" : "supported");
    }
  }

  return r;
}

static Boolean cipher_update(void *_ctx, APKeyInfoType *keyInfoP, UInt8 *input, UInt32 inputLen, UInt8 *output, UInt32 *outputLen) {
  cipher_ctx_t *ctx = (cipher_ctx_t *)_ctx;
  key_ctx_t *key_ctx;
  Int32 r;
  struct s2n_blob in, out;
  const struct s2n_cipher *cipher;
  Boolean ret = false;

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
                    r = cipher->io.stream.encrypt(&ctx->key, &in, &out);
                    debug(DEBUG_TRACE, "s2n", "stream encrypt %u bytes %s", inputLen, r == S2N_SUCCESS ? "success" : s2n_strerror(s2n_errno, NULL));
                  } else {
                    r = cipher->io.stream.decrypt(&ctx->key, &in, &out);
                    debug(DEBUG_TRACE, "s2n", "stream decrypt %u bytes %s", inputLen, r == S2N_SUCCESS ? "success" : s2n_strerror(s2n_errno, NULL));
                  }
                  *outputLen = r == S2N_SUCCESS ? inputLen : 0;
                  ret = r == S2N_SUCCESS;
                } else {
                  debug(DEBUG_ERROR, "s2n", "stream s2n_blob_init failed");
                  *outputLen = 0;
                }
                break;
              case S2N_CBC:
                if (inputLen == 0) break;
                if ((inputLen % cipher->io.cbc.block_size) != 0) {
                  debug(DEBUG_ERROR, "s2n", "cbc input (%u bytes) is not a multiple of the block size (%u bytes)", inputLen, cipher->io.cbc.block_size);
                  *outputLen = 0;
                  break;
                }
                if (s2n_blob_init(&in, input, inputLen) == S2N_SUCCESS && s2n_blob_init(&out, output, inputLen) == S2N_SUCCESS) {
                  if (ctx->encrypt) {
                    r = cipher->io.cbc.encrypt(&ctx->key, &ctx->iv, &in, &out);
                    debug(DEBUG_TRACE, "s2n", "cbc encrypt %u bytes %s", inputLen, r == S2N_SUCCESS ? "success" : s2n_strerror(s2n_errno, NULL));
                  } else {
                    r = cipher->io.cbc.decrypt(&ctx->key, &ctx->iv, &in, &out);
                    debug(DEBUG_TRACE, "s2n", "cbc decrypt %u bytes %s", inputLen, r == S2N_SUCCESS ? "success" : s2n_strerror(s2n_errno, NULL));
                  }
                  *outputLen = r == S2N_SUCCESS ? inputLen : 0;
                  ret = r == S2N_SUCCESS;
                } else {
                  debug(DEBUG_ERROR, "s2n", "cbc s2n_blob_init failed");
                  *outputLen = 0;
                }
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

  return ret;
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

static const char *info(UInt32 *flags) {
  *flags = APF_MP | APF_HASH | APF_CIPHER;
  return "s2n bridge";
}

static void *PluginMain(void *p) {
  MemSet(&plugin, sizeof(CryptoPluginType), 0);

  plugin.info = info;

  plugin.hash_init = hash_init;
  plugin.hash_update = hash_update;
  plugin.hash_finalize = hash_finalize;
  plugin.hash_free = hash_free;

  plugin.cipher_init = cipher_init;
  plugin.cipher_update = cipher_update;
  plugin.cipher_free = cipher_free;

  plugin.key_generate = key_generate;
  plugin.key_export = key_export;
  plugin.key_import = key_import;
  plugin.key_release = key_release;

  plugin.random_get = random_get;

  return &plugin;
}

pluginMainF PluginInit(UInt32 *type, UInt32 *id) {
  s2n_init();

  *type = cryptoPluginType;
  *id   = pluginId;

  return PluginMain;
}
