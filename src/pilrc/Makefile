HOSTCC=gcc

include ../common.mak

PROGRAM=$(TOOLS)/pilrc
SOURCE=bitmap.c font.c lex.c main.c pilrc.c plex.c restype.c util.c win.c
OBJS=$(SOURCE:%.c=%.o)

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	@echo Linking pilrc
	@$(HOSTCC) -o $(PROGRAM) $(OBJS)

clean:
	@rm -f $(PROGRAM) $(OBJS)
