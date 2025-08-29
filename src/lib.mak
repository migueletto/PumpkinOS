ifeq ($(STATIC),1)
OBJS=$(SOURCE:%.c=%.o)
EXT=.a
ARCMD=$(AR) -cr
else ifeq ($(STATIC),2)
OBJS=$(SOURCE:%.c=%.wasm)
EXT=.a
ARCMD=$(EM_AR) -cru
else
OBJS=$(SOURCE:%.c=%.o)
EXT=$(SOEXT)
endif

LIB=$(BIN)/$(PROGRAM)

$(LIB)$(EXT):

$(LIB)$(SOEXT): $(EXTDEPS) $(OBJS)
	$(CC) -shared -o $(LIB)$(SOEXT) $(OBJS) -L$(BIN) -lpit $(LIBS)

$(LIB).a: $(OBJS)
	$(ARCMD) $(LIB).a $(OBJS)

clean: $(EXTCLEANDEPS)
	rm -f $(LIB)$(SOEXT) $(LIB).a $(OBJS) $(EXTCLEAN)
