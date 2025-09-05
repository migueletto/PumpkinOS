include common.mak

DIRS=bin lib tools vfs/app_card/PALM/Programs vfs/app_install vfs/app_storage registry
BASE_MODULES=pilrc prcbuild libpit lua libpumpkin libos libshell BOOT Launcher Preferences Command Edit LuaSyntax MemoPad AddressBook ToDoList DateBook

ifeq ($(OSNAME),GNU/Linux)
EXTRA_MODULES=liblsdl2 libaalsa linux
else ifeq ($(OSNAME),Msys)
EXTRA_MODULES=windows
else ifeq ($(OSNAME),Emscripten)
MODULES=libpit lua libpumpkin libos BOOT Launcher
EXTRA_MODULES=liblsdl2 emscripten
endif

MODULES=$(BASE_MODULES) $(EXTRA_MODULES)
CLEAN_MODULES=$(addsuffix _clean,$(MODULES))

all: $(DIRS) $(MODULES)

$(DIRS):
	mkdir -p $@

$(MODULES):
	$(MAKE) -C $@

clean: $(CLEAN_MODULES)

$(CLEAN_MODULES):
	$(MAKE) -C $(subst _clean,,$@) clean

.PHONY: all clean $(MODULES) $(CLEAN_MODULES)
