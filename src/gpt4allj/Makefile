OBJS=chat.o gptj.o ggml.o utils.o user.o

CFLAGS=-O3 -I./ggml/include -I./ggml/include/ggml -I./ggml/examples -mavx -mavx2 -mfma -mf16c -msse3 -pthread
CXXFLAGS=-O3 -I./ggml/include -I./ggml/include/ggml -I./ggml/examples -pthread

chat: $(OBJS)
	g++ -o chat -pthread $(OBJS)

ggml.o: ggml/src/ggml.c
	gcc $(CFLAGS) -c ggml/src/ggml.c

utils.o: ggml/examples/utils.cpp
	g++ $(CXXFLAGS) -c ggml/examples/utils.cpp

clean:
	rm -f chat $(OBJS)
