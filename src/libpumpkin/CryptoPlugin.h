#define cryptoPluginType 'cryp'

typedef struct {
  const char *(*info)(UInt32 *flags);

  Boolean (*hash_init)(APHashInfoType *hashInfoP);
  void (*hash_update)(APHashInfoType *hashInfoP, UInt8 *input, UInt32 len);
  void (*hash_finalize)(APHashInfoType *hashInfoP, UInt8 *hash);
  void (*hash_free)(APHashInfoType *hashInfoP);

  Boolean (*cipher_init)(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, Boolean encrypt);
  Boolean (*cipher_update)(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *input, UInt32 inputLen, UInt8 *output, UInt32 *outputLen);
  void (*cipher_free)(APCipherInfoType *cipherInfoP);

  Boolean (*key_generate)(APKeyInfoType *keyInfoP, UInt8 *data);
  Boolean (*key_export)(APKeyInfoType *keyInfoP, UInt8 encoding, UInt8 *data, UInt32 *len);
  Boolean (*key_import)(APKeyInfoType *keyInfoP, UInt8 encoding, UInt8 *data, UInt32 len);
  void (*key_release)(APKeyInfoType *keyInfoP);

  Boolean (*random_get)(UInt8 *data, UInt32 len);
} CryptoPluginType;

