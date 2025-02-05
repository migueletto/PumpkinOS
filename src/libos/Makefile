include ../common.mak
include ../commonp.mak

SOURCE=libos.c

ifeq ($(OSNAME),Emscripten)

PROGRAM=$(BIN)/libos$(SOEXT)
OBJS=$(SOURCE:%.c=%.wasm)
$(PROGRAM): $(OBJS)
	$(EM_CC) -o $(PROGRAM) -s SIDE_MODULE $(OBJS)

else

PROGRAM=$(BIN)/libos$(SOEXT)
OBJS=$(SOURCE:%.c=%.o)
$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit -lpumpkin

endif

clean:
	rm -f $(PROGRAM) $(OBJS)
