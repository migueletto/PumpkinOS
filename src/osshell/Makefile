include ../common.mak

ifeq ($(OSNAME),GNU/Linux)
SOURCE=os_shell.c linux_shell.c
endif
ifeq ($(OSNAME),Msys)
SOURCE=os_shell.c windows_shell.c
endif

RCP=resource.rcp
APPNAME=osshell
APPID=OShl
APPTYPE=CmdE

include ../commonp.mak
include ../app.mak
