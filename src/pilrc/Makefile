include ../common.mak

PROGRAM=$(TOOLS)/pilrc
OBJS=bitmap.o font.o lex.o main.o pilrc.o plex.o restype.o util.o win.o
CC=$(HOSTCC)

$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS)

clean:
	rm -f $(PROGRAM) $(OBJS)
