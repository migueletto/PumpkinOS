TARGET	= galasm
OBJS	= galasm.o support.o jedec.o localize.o

CFLAGS	= -Wall -O2 -Iinclude -I.

all: $(TARGET)

$(TARGET): $(OBJS)
	gcc -o $@ $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)
