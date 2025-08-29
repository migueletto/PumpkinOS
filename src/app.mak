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

ifneq ($(SOURCE),)
OBJS=$(SOURCE:%.c=%.o)
endif

ifneq ($(OBJS),)
DLIB_OPT=$(DLIB)
endif

ifneq ($(RCP),)
RESFLAG=res.flag
endif

PRC=$(VFS)/app_install/$(PROGRAM).prc

$(PROGRAM).prc: $(DLIB_OPT) $(RESFLAG)
	$(PRCDUILD) -f $(PROGRAM).prc -t $(APPTYPE) -c $(APPID) -n $(APPNAME) $(DLIB_OPT) resources/*
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)
	strip $(DLIB)

$(PROGRAM).dlib: $(OBJS) $(STUBS_OPT)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) $(STUBS_OPT) $(LIBS)

$(STUBS_OPT): $(STUBS).c
	$(CC) $(CFLAGS) -c $(STUBS).c

$(STUBS).c: $(OBJS)
	$(NM) $(OBJS) | sort | uniq > $(STUBS).txt
	$(LIBPUMPKIN)/gen_stubs.sh $(STUBS).txt $(LIBPUMPKIN)/traps.txt > $(STUBS).c

res.flag: $(RCP) $(RCPDEP)
	mkdir -p resources
	$(PILRC) $(RCP) resources
	touch res.flag

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(OBJS) $(DLIB) $(STUBS).c $(STUBS_OPT) $(STUBS).txt resources/*.bin res.flag
