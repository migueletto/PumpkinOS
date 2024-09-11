#define cryptoPluginType 'cryp'

typedef struct {
  void *(*hash_init)(UInt32 type);
  uint32_t (*hash_size)(void *ctx);
  void (*hash_update)(void *ctx, UInt8 *input, UInt32 len);
  void (*hash_finalize)(void *ctx, UInt8 *hash);
  void (*hash_free)(void *ctx);

  void *(*cipher_init)(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, Boolean encrypt);
  void (*cipher_update)(void *ctx, APKeyInfoType *keyInfoP, UInt8 *input, UInt32 inputLen, UInt8 *output, UInt32 *outputLen);
  void (*cipher_free)(void *ctx);

  void *(*key_generate)(APKeyInfoType *keyInfoP, UInt8 *data);
  void (*key_export)(void *ctx, UInt8 *data, UInt32 *len);
  void (*key_release)(void *ctx);
} CryptoPluginType;

