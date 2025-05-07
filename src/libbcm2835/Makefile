include ../common.mak

OBJS=bcm2835.o

ifeq ($(OSNAME),Kernel)

PROGRAM=$(BIN)/libbcm2835.a
$(PROGRAM): $(OBJS)
	$(AR) -cr $(PROGRAM) $(OBJS)

else

PROGRAM=$(BIN)/libbcm2835$(SOEXT)
$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN)

endif

clean:
	rm -f $(PROGRAM) *.o
