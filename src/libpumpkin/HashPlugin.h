#define hashPluginType 'hash'

typedef struct {
  void *(*init)(UInt32 type);
  uint32_t (*size)(void *ctx);
  void (*update)(void *ctx, UInt8 *input, UInt32 len);
  void (*finalize)(void *ctx, UInt8 *hash);
  void (*free)(void *ctx);
  void *ctx;
} HashPluginType;

