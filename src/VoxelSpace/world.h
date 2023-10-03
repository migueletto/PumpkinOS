#define MAX_X 1048576.0f
#define MAX_Y 1048576.0f

typedef struct {
  float height_freq;
  int height_depth;
  int height_seed;
  float color_freq;
  int color_depth;
  int color_seed;
  int sea_level;

  uint8_t palette[(2 + 2 + 64 + 64) * 3];
} world_t;
