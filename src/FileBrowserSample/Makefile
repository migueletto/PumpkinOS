PROGRAM=FileBrowserSample
CUSTOMPFLAGS=-I$(SRC)/PalmOS/PalmOne -I$(SRC)/PalmOS/PalmOne/Common/System -I$(SRC)/PalmOS/PalmOne/68K/System -I$(SRC)/PalmOS/PalmOne/Common/Libraries/filebrowser
SOURCE=src/SampleDB.c src/SampleMain.c src/SampleTransfer.c
RCP=rsc/SampleApp.rcp
RCPINC=-I src -I rsc
RCPDEP=other_res.flag
APPNAME=FileBrowserSample
APPID=SaDo

include ../common.mak
include ../commonp.mak
include ../app.mak

other_res.flag:
	@echo Compiling additional resources
	@$(PILRC) $(RCPINC) rsc/SampleDelete.rcp resources
	@$(PILRC) $(RCPINC) rsc/SampleDetails.rcp resources
	@$(PILRC) $(RCPINC) rsc/SampleEdit.rcp resources
	@$(PILRC) $(RCPINC) rsc/SampleList.rcp resources
	@$(PILRC) $(RCPINC) rsc/SampleMisc.rcp resources
	@$(PILRC) $(RCPINC) rsc/SamplePrefer.rcp resources
	@touch other_res.flag
