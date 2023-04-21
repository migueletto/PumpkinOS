#include <stdio.h>
#include <string>

#include "test_hw.h"

void test_hw(void) {
    static bool avx = __builtin_cpu_supports("avx");
    static bool avx2 = __builtin_cpu_supports("avx2");
    static bool fma = __builtin_cpu_supports("fma");
    static bool sse3 = __builtin_cpu_supports("sse3");
    static std::string s;
    s  = "gpt4all hardware test results:\n";
    s += "  AVX  = "         + std::to_string(avx)         + "\n";
    s += "  AVX2 = "        + std::to_string(avx2)        + "\n";
    s += "  FMA  = "         + std::to_string(fma)         + "\n";
    s += "  SSE3 = "        + std::to_string(sse3)        + "\n";
    fprintf(stderr, "%s", s.c_str());
    fprintf(stderr, "your hardware supports the \"");
    if (avx2)
        fprintf(stderr, "full");
    else if (avx && fma)
        fprintf(stderr, "avx_only");
    else
        fprintf(stderr, "bare_minimum");
    fprintf(stderr, "\" version of gpt4all.\n\n");
}
