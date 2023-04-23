#include "llama.cpp/ggml.h"
#include "gptj.h"
#include "llamamodel.h"
#include "utils.h"
#include "user.h"
#include "test_hw.h"
#include "param.h"

#include <fstream>
#include <string.h>

#define MAX_BUF 65536

typedef struct {
  LLModel::PromptContext ctx;
  int32_t predict = 1024;
  int32_t top_k = 40;
  float top_p = 0.95f;
  float temp = 0.28f;
  int32_t batch = 9;
} chat_param_t;

static bool response(const std::string &r) {
  if (r.find("### Prompt:") != std::string::npos || r.find("### Response:") != std::string::npos)
    return false;
  fprintf(stderr, "%s", r.c_str());
  if (!user_output(r.c_str())) return false;
  return true;
}

static bool command(char *buf, chat_param_t *param, param_info_t pinfo[]) {
  if (!strcasecmp(buf, "stop")) {
    fprintf(stderr, "stop generating\n");
  } else if (!strcasecmp(buf, "reset")) {
    fprintf(stderr, "reset context\n");
    param->ctx = LLModel::PromptContext();
  } else {
    char *value = strchr(buf, ' ');
    if (value) param_check(pinfo, &buf[1], value+1);
  }

  user_output("#eof");

  return true;
}

int main(int argc, char *argv[]) {
  chat_param_t param;
  param_info_t pinfo[] = {
    { "predict", 'I', &param.predict, 32, 2048, 0, 0, "maximum length of response in tokens" },
    { "top_k",   'I', &param.top_k,    1,  100, 0, 0, "only the top K most likely tokens will be chosen from" },
    { "top_p",   'F', &param.top_p,    0,    0, 0, 1, "only the most likely tokens up to a total probability of top_p can be chosen" },
    { "temp",    'F', &param.temp,     0,    0, 0, 1, "increases the chances of choosing less likely tokens" },
    { "batch",   'I', &param.batch,    1,   20, 0, 0, "amount of prompt tokens to process at once" },
    { 0 }
  };
  LLModel *llmodel;
  char *buf;
  int i, n;

  for (i = 1; i < argc; i += 2) {
    if (argv[i][0] != '-') break;
    if (i == argc-1) break;
    param_check(pinfo, &argv[i][1], argv[i+1]);
  }

  if ((argc - i) != 2) {
    fprintf(stderr, "usage: %s [options] <model> <port>\n", argv[0]);
    fprintf(stderr, "model: model pathname\n");
    fprintf(stderr, "port: UDP port number\n");
    fprintf(stderr, "options:\n");
    param_usage(pinfo);
    return 1;
  }

  test_hw();

  ggml_time_init();
  auto fin = std::ifstream(argv[i], std::ios::binary);

  uint32_t magic;
  fin.read((char *) &magic, sizeof(magic));
  fin.seekg(0);
  bool isGPTJ = (magic == 0x67676d6c);

  if (isGPTJ) {
    llmodel = new GPTJ;
  } else {
    llmodel = new LLamaModel;
  }

  if (!llmodel->loadModel(argv[i], fin)) {
    fprintf(stderr, "failed to load model '%s'\n", argv[i]);
    return 1;
  }

  buf = (char *)calloc(1, MAX_BUF);

  if (user_init(atoi(argv[i+1])) == 0) {
    for (;;) {
      for (;;) {
        if ((n = user_input(buf, MAX_BUF-2)) != 0) break;
      }
      if (n == -1) break;
      if (n > 1 && buf[0] == '#' && command(&buf[1], &param, pinfo)) continue;
      fprintf(stderr, "input: %s\n", buf);
      buf[n++] = '\n';
      buf[n] = 0;

      fprintf(stderr, "predict=%d, top_k=%d, top_p=%.2f, temp=%.2f, batch=%d\n",
        param.predict, param.top_k, param.top_p, param.temp, param.batch);

      std::string prompt = "The prompt below is a question to answer, a task to complete, or a conversation to respond to; decide which and write an appropriate response.\n### Prompt:\n" + std::string(buf) + "\n### Response:\n";
      fprintf(stderr, "output: ");
      llmodel->prompt(prompt, response, param.ctx, param.predict, param.top_k, param.top_p, param.temp, param.batch);
      fprintf(stderr, "\n");
      prompt.clear();
      user_output("#eof");
    }

    user_finish();
  }

  free(buf);

  return 0;
}
