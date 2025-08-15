include ../common.mak

SOURCE=libwwayland.c xdg-shell-protocol.c shm.c
LIBWAYLAND=-lwayland-client -lwayland-cursor -lxkbcommon

WAYLAND_SCANNER=/usr/bin/wayland-scanner
XDG_SHELL_PROTOCOL=/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml

PROGRAM=$(BIN)/libwwayland$(SOEXT)
OBJS=$(SOURCE:%.c=%.o)
$(PROGRAM): xdg-shell-client-protocol.h $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit $(LIBWAYLAND)

xdg-shell-client-protocol.h:
	$(WAYLAND_SCANNER) client-header $(XDG_SHELL_PROTOCOL) xdg-shell-client-protocol.h

xdg-shell-protocol.c:
	$(WAYLAND_SCANNER) private-code $(XDG_SHELL_PROTOCOL) xdg-shell-protocol.c

clean:
	rm -f $(PROGRAM) $(OBJS) xdg-shell-client-protocol.h xdg-shell-protocol.c
