include ../common.mak

SOURCE=libwxcb.c
LIBXCB=-lxcb -lxcb-image -lxkbcommon -lxkbcommon-x11 -lxcb-xkb

PROGRAM=$(BIN)/libwxcb$(SOEXT)
OBJS=$(SOURCE:%.c=%.o)
$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit $(LIBXCB)

clean:
	rm -f $(PROGRAM) $(OBJS)
