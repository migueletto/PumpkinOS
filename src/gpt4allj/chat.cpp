#include "ggml/ggml.h"
#include "gptj.h"
#include "llmodel.h"
#include "utils.h"
#include "user.h"

#include <fstream>

static bool response(const std::string &r) {
  user_output(r.c_str());
  return true;
}

int main(int argc, char *argv[]) {
  LLModel::PromptContext ctx;
  GPTJ gptj;
  char buf[256];
  int n;

  if (argc != 3) {
    fprintf(stderr, "usage: %s <model> <port>\n", argv[0]);
    return 1;
  }

  ggml_time_init();
  auto fin = std::ifstream(argv[1], std::ios::binary);

  if (!gptj.loadModel(argv[1], fin)) {
    fprintf(stderr, "failed to load model '%s'\n", argv[1]);
    return 1;
  }

  if (user_init(atoi(argv[2])) == 0) {
    for (;;) {
      for (;;) {
        if ((n = user_input(buf, sizeof(buf)-2)) > 0) break;
      }
      if (n == 1 && buf[0] == 0) break;
      buf[n++] = '\n';
      buf[n] = 0;

      gptj.prompt(buf, response, ctx, 200, 40, 0.9f, 0.9f, 9);
      user_output("EOF");
    }

    user_finish();
  }

  return 0;
}
