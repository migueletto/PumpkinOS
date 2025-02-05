OSNAME=Emscripten

include ../common.mak

PROGRAM=pumpkin

SOURCE=main.c
OBJS=$(SOURCE:%.c=%.wasm)
MODULES=$(BIN)/libpit.wasm $(BIN)/libpumpkin.wasm $(BIN)/libscriptlua.wasm $(BIN)/liblsdl2.wasm $(BIN)/libos.wasm

$(PROGRAM).zip: $(PROGRAM).html
	cp $(MODULES) .
	zip $(PROGRAM).zip $(PROGRAM).wasm $(PROGRAM).data $(PROGRAM).html $(PROGRAM).js libpit.wasm libpumpkin.wasm libscriptlua.wasm liblsdl2.wasm libos.wasm

$(PROGRAM).html: $(OBJS) $(MODULES)
	$(EM_CC) -o $(PROGRAM).html -s MAIN_MODULE=2 -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 --preload-file vfs $(OBJS) $(MODULES)

clean:
	rm -f $(OBJS) $(PROGRAM).zip $(PROGRAM).wasm $(PROGRAM).data $(PROGRAM).html $(PROGRAM).js libpit.wasm libpumpkin.wasm libscriptlua.wasm liblsdl2.wasm libos.wasm
