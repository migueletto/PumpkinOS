CFLAGS=-Wall -DNEED_HSEARCH

OBJ=main.o readobj.o section.o symbol.o fixup.o do_out.o optget.o hsearch.o

all: ld80

ld80: $(OBJ)
	gcc -o ld80 $(OBJ)

clean:
	rm -f ld80 *.o
