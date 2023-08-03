include ../common.mak

CC=$(HOSTCC)

all: $(TOOLS)/prcbuild $(TOOLS)/prclist

$(TOOLS)/prcbuild: prcbuild.o pdb.o
	$(CC) -o $(TOOLS)/prcbuild prcbuild.o pdb.o

$(TOOLS)/prclist: prclist.o pdb.o
	$(CC) -o $(TOOLS)/prclist prclist.o pdb.o

clean:
	rm -f $(TOOLS)/prcbuild $(TOOLS)/prclist *.o
