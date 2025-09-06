include ../common.mak

PROGRAM=$(TOOLS)/pilrc
SOURCE=bitmap.c font.c lex.c main.c pilrc.c plex.c restype.c util.c win.c
OBJS=$(SOURCE:%.c=%.o)
CC=$(HOSTCC)

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	@echo Linking pilrc
	@$(CC) -o $(PROGRAM) $(OBJS)

clean:
	@rm -f $(PROGRAM) $(OBJS)
