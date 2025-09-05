OSNAME=Emscripten

include ../common.mak

PROGRAM=pumpkin

SOURCE=main.c
OBJS=$(SOURCE:%.c=%.wasm)
MODULES=libpit.a libpumpkin.a libscriptlua.a liblsdl2.a libos.a

$(PROGRAM).zip: $(PROGRAM).html coi-serviceworker.js
	zip $(PROGRAM).zip $(PROGRAM).wasm $(PROGRAM).data $(PROGRAM).html $(PROGRAM).js coi-serviceworker.js

$(PROGRAM).html: $(OBJS) $(MODULES) template.html vfs/pumpkin.lua
	$(EM_CC) -o $(PROGRAM).js -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s ENVIRONMENT=web,worker -s PTHREAD_POOL_SIZE=4 -s EXPORTED_FUNCTIONS=_main,_cmain,_cexit,_pumpkin_send_deploy -s EXPORTED_RUNTIME_METHODS=ccall -s EXIT_RUNTIME -pthread -lidbfs.js --preload-file vfs $(OBJS) $(MODULES)
	cp template.html $(PROGRAM).html

clean:
	rm -f $(OBJS) $(PROGRAM).zip $(PROGRAM).wasm $(PROGRAM).data $(PROGRAM).html $(PROGRAM).js
