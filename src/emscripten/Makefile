OSNAME=Emscripten

include ../common.mak

PROGRAM=pumpkin

SOURCE=main.c
OBJS=$(SOURCE:%.c=%.wasm)
MODULES=libpit.a libpumpkin.a libscriptlua.a liblsdl2.a libos.a

$(PROGRAM).zip: $(PROGRAM).html
	zip $(PROGRAM).zip $(PROGRAM).wasm $(PROGRAM).data $(PROGRAM).html $(PROGRAM).js $(PROGRAM).worker.js

$(PROGRAM).html: $(OBJS) $(MODULES)
	$(EM_CC) -o $(PROGRAM).html -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s PTHREAD_POOL_SIZE=8 -pthread --preload-file vfs $(OBJS) $(MODULES)

clean:
	rm -f $(OBJS) $(PROGRAM).zip $(PROGRAM).wasm $(PROGRAM).data $(PROGRAM).html $(PROGRAM).js $(PROGRAM).worker.js
