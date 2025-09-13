HOSTCC=gcc

include ../common.mak

all: $(TOOLS)/prcbuild $(TOOLS)/prclist

$(TOOLS)/prcbuild: prcbuild.o pdb.o
	@echo Linking prcbuild
	@$(HOSTCC) -o $(TOOLS)/prcbuild prcbuild.o pdb.o

$(TOOLS)/prclist: prclist.o pdb.o
	@echo Linking prclist
	@$(HOSTCC) -o $(TOOLS)/prclist prclist.o pdb.o

clean:
	@rm -f $(TOOLS)/prcbuild $(TOOLS)/prclist *.o
