include ../common.mak

#CUSTOMFLAGS=-fpack-struct
# https://elm-chan.org/fsw/ff/

PROGRAM=$(BIN)/libfatfs.a

OBJS=ff.o ffunicode.o

$(PROGRAM): $(OBJS)
	$(AR) -cr $(PROGRAM) $(OBJS)

clean:
	rm -f $(PROGRAM) $(OBJS)
