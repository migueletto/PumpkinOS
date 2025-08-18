include ../common.mak

SOURCE=libwwayland.c xdg-shell-protocol.c xdg-decoration-protocol.c shm.c
LIBWAYLAND=-lwayland-client -lwayland-cursor -lxkbcommon

WAYLAND_SCANNER=/usr/bin/wayland-scanner
XDG_SHELL_PROTOCOL=/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
XDG_DECORATION_PROTOCOL=/usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml

PROGRAM=$(BIN)/libwwayland$(SOEXT)
OBJS=$(SOURCE:%.c=%.o)
$(PROGRAM): xdg-shell-client-protocol.h xdg-decoration-client-protocol.h $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit $(LIBWAYLAND)

xdg-shell-client-protocol.h:
	$(WAYLAND_SCANNER) client-header $(XDG_SHELL_PROTOCOL) xdg-shell-client-protocol.h

xdg-shell-protocol.c:
	$(WAYLAND_SCANNER) private-code $(XDG_SHELL_PROTOCOL) xdg-shell-protocol.c

xdg-decoration-client-protocol.h:
	$(WAYLAND_SCANNER) client-header $(XDG_DECORATION_PROTOCOL) xdg-decoration-client-protocol.h

xdg-decoration-protocol.c:
	$(WAYLAND_SCANNER) private-code $(XDG_DECORATION_PROTOCOL) xdg-decoration-protocol.c

clean:
	rm -f $(PROGRAM) $(OBJS) xdg-shell-client-protocol.h xdg-shell-protocol.c xdg-decoration-client-protocol.h xdg-decoration-protocol.c
