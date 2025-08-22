include ../common.mak

ifneq (,$(wildcard /usr/include/libdecor-0/libdecor.h))
LIBDECOR_LIB=-ldecor-0
LIBDECOR_FLAGS=-I/usr/include/libdecor-0 -DLIBDECOR
endif

SOURCE=libwwayland.c xdg-shell-protocol.c xdg-decoration-protocol.c xdg-toplevel-icon-protocol.c shm.c
LIBWAYLAND=-lwayland-client -lwayland-cursor -lxkbcommon $(LIBDECOR_LIB)
CUSTOMFLAGS=$(LIBDECOR_FLAGS)

WAYLAND_SCANNER=/usr/bin/wayland-scanner
XDG_SHELL_PROTOCOL=/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
XDG_DECORATION_PROTOCOL=/usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
XDG_TOPLEVEL_ICON_PROTOCOL=xdg-toplevel-icon-v1.xml

PROGRAM=$(BIN)/libwwayland$(SOEXT)
OBJS=$(SOURCE:%.c=%.o)
$(PROGRAM): xdg-shell-client-protocol.h xdg-decoration-client-protocol.h xdg-toplevel-icon-client-protocol.h $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit $(LIBWAYLAND)

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

clean:
	rm -f $(PROGRAM) $(OBJS) xdg-shell-client-protocol.h xdg-shell-protocol.c xdg-decoration-client-protocol.h xdg-decoration-protocol.c xdg-toplevel-icon-client-protocol.h xdg-toplevel-icon-protocol.c
