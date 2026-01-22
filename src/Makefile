include common.mak

DIRS=../bin ../lib ../tools ../vfs/app_card/PALM/Programs ../vfs/app_install ../vfs/app_storage
BASE_MODULES=pilrc prcbuild libpit lua libpumpkin libos libshell BOOT Launcher Preferences Command vi LuaSyntax MemoPad AddressBook ToDoList DateBook

ifeq ($(OSNAME),GNU/Linux)

ifneq ($(wildcard /usr/include/SDL2/SDL.h),)
WINDOW_MODULE=liblsdl2
else ifneq ($(wildcard /usr/include/wayland-client.h),)
WINDOW_MODULE=libwwayland
else ifneq ($(wildcard /usr/include/xcb/xcb_ewmh.h),)
WINDOW_MODULE=libwxcb
else
ERROR=make_error
ERROR_MSG=Could not find SDL2, Wayland or XCB development headers
endif

ifneq ($(wildcard /usr/include/unicorn/unicorn.h),)
UNICORN_MODULE=UnicornArm
endif

EXTRA_MODULES=$(UNICORN_MODULE) $(WINDOW_MODULE) libaalsa linux

else ifeq ($(OSNAME),Msys)

EXTRA_MODULES=windows

else ifeq ($(OSNAME),Emscripten)

EMCC=$(shell which emcc)
ifeq ($(wildcard $(EMCC)),)
ERROR=make_error
ERROR_MSG=Emscripten compiler (emcc) was not found in your PATH
endif
BASE_MODULES=pilrc prcbuild libpit lua libpumpkin libos BOOT Launcher
EXTRA_MODULES=liblsdl2 emscripten

endif

MODULES=$(BASE_MODULES) $(EXTRA_MODULES)
CLEAN_MODULES=$(addsuffix _clean,$(MODULES))

all: messages $(ERROR) $(DIRS) $(MODULES)
	@echo "$(COLOR_CYAN)End$(COLOR_END)"

messages:
	@echo "$(COLOR_CYAN)Building PumpkinOS$(COLOR_END)"
	@echo "$(COLOR_CYAN)Platform:$(COLOR_END) $(OSNAME) $(BITS) bits"
	@echo "$(COLOR_CYAN)Modules:$(COLOR_END) $(MODULES)"
	@echo "$(COLOR_CYAN)Compiler flags:$(COLOR_END) $(CFLAGS)"

make_error:
	@echo "$(COLOR_RED)ERROR: $(ERROR_MSG)$(COLOR_END)"
	@exit 1

$(DIRS):
	@echo "$(COLOR_GREEN)Creating directory $@$(COLOR_END)"
	@mkdir -p $@

$(MODULES):
	@echo "$(COLOR_GREEN)Building $@$(COLOR_END)"
	@$(MAKE) -s --no-print-directory -j -C $@

clean: $(CLEAN_MODULES)

$(CLEAN_MODULES):
	@echo "$(COLOR_GREEN)Cleaning $(subst _clean,,$@)$(COLOR_END)"
	@$(MAKE) -s --no-print-directory -C $(subst _clean,,$@) clean

.PHONY: all clean conf $(MODULES) $(CLEAN_MODULES)
