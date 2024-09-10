#define cipherPluginType 'ciph'

typedef struct {
  int (*init)(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP);
  void (*update)(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *input, UInt32 inputLen, UInt8 *output, UInt32 *outputLen);
  void (*finalize)(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *input, UInt32 inputLen, UInt8 *output, UInt32 *outputLen);
  void (*free)(APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP);
} CipherPluginType;

