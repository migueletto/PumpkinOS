#include "llamamodel.h"

#include "llama.cpp/examples/common.h"
#include "llama.cpp/llama.h"
//#include "llama.cpp/ggml.h"

#include <cassert>
#include <vector>

// TODO: not great allocating this every time
std::vector<llama_token> llama_tokenize(struct llama_context * ctx, const std::string & text, bool add_bos) {
    // initialize to prompt numer of chars, since n_tokens <= n_prompt_chars
    std::vector<llama_token> res(text.size() + (int)add_bos);
    int n = llama_tokenize(ctx, text.c_str(), res.data(), res.size(), add_bos);
    assert(n >= 0);
    res.resize(n);

    return res;
}
