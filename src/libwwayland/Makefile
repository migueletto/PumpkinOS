# additional package(s) to install (example for Ubuntu):
# sudo apt install wayland-protocols libwayland-bin libwayland-dev libxkbcommon-dev libdecor-0-dev

ifneq (,$(wildcard /usr/include/libdecor-0/libdecor.h))
LIBDECOR_LIB=-ldecor-0
LIBDECOR_FLAGS=-I/usr/include/libdecor-0 -DLIBDECOR
endif

PROGRAM=libwwayland
SOURCE=libwwayland.c xdg-shell-protocol.c xdg-decoration-protocol.c xdg-toplevel-icon-protocol.c shm.c
CUSTOMFLAGS=$(LIBDECOR_FLAGS)
EXTDEPS=xdg-shell-client-protocol.h xdg-decoration-client-protocol.h xdg-toplevel-icon-client-protocol.h
LIBS=-lwayland-client -lwayland-cursor -lxkbcommon $(LIBDECOR_LIB)
EXTCLEAN=xdg-shell-client-protocol.h xdg-shell-protocol.c xdg-decoration-client-protocol.h xdg-decoration-protocol.c xdg-toplevel-icon-client-protocol.h xdg-toplevel-icon-protocol.c

include ../common.mak
include ../lib.mak

WAYLAND_SCANNER=/usr/bin/wayland-scanner
XDG_SHELL_PROTOCOL=/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
XDG_DECORATION_PROTOCOL=/usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
XDG_TOPLEVEL_ICON_PROTOCOL=xdg-toplevel-icon-v1.xml

xdg-shell-client-protocol.h:
	$(WAYLAND_SCANNER) client-header $(XDG_SHELL_PROTOCOL) xdg-shell-client-protocol.h

xdg-shell-protocol.c:
	$(WAYLAND_SCANNER) private-code $(XDG_SHELL_PROTOCOL) xdg-shell-protocol.c

xdg-decoration-client-protocol.h:
	$(WAYLAND_SCANNER) client-header $(XDG_DECORATION_PROTOCOL) xdg-decoration-client-protocol.h

xdg-decoration-protocol.c:
	$(WAYLAND_SCANNER) private-code $(XDG_DECORATION_PROTOCOL) xdg-decoration-protocol.c

xdg-toplevel-icon-client-protocol.h:
	$(WAYLAND_SCANNER) client-header $(XDG_TOPLEVEL_ICON_PROTOCOL) xdg-toplevel-icon-client-protocol.h

xdg-toplevel-icon-protocol.c:
	$(WAYLAND_SCANNER) private-code $(XDG_TOPLEVEL_ICON_PROTOCOL) xdg-toplevel-icon-protocol.c
