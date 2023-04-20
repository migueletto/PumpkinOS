#include "ggml/ggml.h"
#include "gptj.h"
#include "llamamodel.h"
#include "utils.h"
#include "user.h"

#include <fstream>

#define MAX_BUF 65536

static bool response(const std::string &r) {
  if (r.find("### Prompt:") != std::string::npos || r.find("### Response:") != std::string::npos)
    return false;
  user_output(r.c_str());
  return true;
}

int main(int argc, char *argv[]) {
  GPTJ::PromptContext ctx;
  LLModel *llmodel;
  //GPTJ gptj;
  char *buf;
  int n;

  if (argc != 3) {
    fprintf(stderr, "usage: %s <model> <port>\n", argv[0]);
    return 1;
  }

  ggml_time_init();
  auto fin = std::ifstream(argv[1], std::ios::binary);

  uint32_t magic;
  fin.read((char *) &magic, sizeof(magic));
  fin.seekg(0);
  bool isGPTJ = (magic == 0x67676d6c);

  if (isGPTJ) {
    llmodel = new GPTJ;
  } else {
    llmodel = new LLamaModel;
  }

  if (!llmodel->loadModel(argv[1], fin)) {
    fprintf(stderr, "failed to load model '%s'\n", argv[1]);
    return 1;
  }

  buf = (char *)calloc(1, MAX_BUF);

  if (user_init(atoi(argv[2])) == 0) {
    for (;;) {
      for (;;) {
        if ((n = user_input(buf, MAX_BUF-2)) > 0) break;
      }
      if (n == 1 && buf[0] == 0) break;
      buf[n++] = '\n';
      buf[n] = 0;

      std::string prompt = "The prompt below is a question to answer, a task to complete, or a conversation to respond to; decide which and write an appropriate response.\n### Prompt:\n" + std::string(buf) + "### Response:\n";
      llmodel->prompt(prompt, response, ctx, 2048, 40, 0.95, 0.28, 9);
      prompt.clear();
      user_output("EOF");
    }

    user_finish();
  }

  free(buf);

  return 0;
}
