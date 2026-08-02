include ../common.mak

PROGRAM=libwnull
SOURCE=libwnull.c
EXTCLEAN=display_html.h

include ../lib.mak

libwnull.o: libwnull.c display_html.h
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c -o $@ $<

display_html.h: display.html display.js favicon.ico
	@echo Generating display_html.h
	@xxd -i display.html > display_html.h
	@xxd -i display.js >> display_html.h
	@xxd -i favicon.ico >> display_html.h
