ifeq ($(PUMPKIN_APP),)
LIBS=-L$(BIN) -lpumpkin -lpit
else
CUSTOMPFLAGS=-DPUMPKIN_APP -fvisibility=hidden
STUBS_OPT=$(STUBS).o
endif

ifeq ($(PROGRAM),)
PROGRAM=$(APPNAME)
endif

ifeq ($(APPTYPE),)
APPTYPE=appl
endif

ifeq ($(OBJS),)
ifneq ($(SOURCE),)
OBJS=$(SOURCE:%.c=%.o)
endif
endif

ifneq ($(OBJS),)
DLIB_OPT=$(DLIB)
endif

ifneq ($(RCP),)
RESFLAG=res.flag
endif

PRC=$(VFS)/app_install/$(PROGRAM).prc

$(PROGRAM).prc: $(DLIB_OPT) $(RESFLAG) $(RESOPT)
	@echo Building $(PROGRAM).prc
	@$(PRCDUILD) -f $(PROGRAM).prc -t $(APPTYPE) -c $(APPID) -n "$(APPNAME)" $(DLIB_OPT) resources/*
	@echo Installing $(PROGRAM).prc
	@cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	@cp $(PROGRAM).dlib $(DLIB)
	@$(STRIP) $(DLIB)

$(PROGRAM).dlib: $(OBJS) $(STUBS_OPT)
	@echo Linking $(PROGRAM).dlib
	@$(CC) -shared -o $(PROGRAM).dlib $(OBJS) $(STUBS_OPT) $(LIBS)

$(STUBS_OPT): $(STUBS).c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c $(STUBS).c

$(STUBS).c: $(OBJS)
	@echo Generating $<
	@$(NM) $(OBJS) | sort | uniq > $(STUBS).txt
	@$(LIBPUMPKIN)/gen_stubs.sh $(STUBS).txt $(LIBPUMPKIN)/traps.txt > $(STUBS).c

res.flag: $(RCP) $(RCPDEP)
	@echo Compiling resources
	@mkdir -p resources
	@$(PILRC) $(RCPINC) $(RCP) resources
	@touch res.flag

clean:
	@rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(OBJS) $(DLIB) $(STUBS).c $(STUBS_OPT) $(STUBS).txt resources/*.bin *.flag
