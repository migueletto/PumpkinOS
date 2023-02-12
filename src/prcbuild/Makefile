include ../common.mak

PROGRAM=$(BIN)/prcbuild
OBJS=prcbuild.o pdb.o

$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit

clean:
	rm -f $(PROGRAM) $(OBJS)
