include ../common.mak

PROGRAM=$(TOOLS)/prcbuild
OBJS=prcbuild.o pdb.o
CC=$(HOSTCC)

$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS)

clean:
	rm -f $(PROGRAM) $(OBJS)
