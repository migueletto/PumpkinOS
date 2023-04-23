OBJS=chat.o gptj.o llamamodel.o llama.o llama_common.o ggml.o utils.o user.o param.o test_hw.o

CFLAGS=-O3 -I./llama.cpp -Wno-multichar -mavx -mavx2 -mfma -mf16c -msse3 -pthread
CXXFLAGS=-O3 -I./llama.cpp -Wno-multichar -pthread

chat: $(OBJS)
	g++ -o chat -pthread $(OBJS)

llama.o: llama.cpp/llama.cpp
	gcc $(CFLAGS) -c llama.cpp/llama.cpp

ggml.o: llama.cpp/ggml.c
	gcc $(CFLAGS) -c llama.cpp/ggml.c

clean:
	rm -f chat $(OBJS)
