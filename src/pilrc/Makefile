include ../common.mak

PROGRAM=$(BIN)/pilrc
OBJS=bitmap.o font.o lex.o main.o pilrc.o plex.o restype.o util.o win.o

$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS)

clean:
	rm -f $(PROGRAM) $(OBJS)
