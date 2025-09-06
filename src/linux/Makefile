include ../common.mak

PROGRAM=$(ROOT)/pumpkin

OBJS=main.o

$(PROGRAM): $(OBJS)
	@echo Linking pumpkin
	@$(CC) -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit -lpthread

clean:
	@rm -f $(PROGRAM) $(OBJS)
