include ../common.mak

PROGRAM=$(BIN)/libzydis$(SOEXT)

OBJS=MetaInfo.o Mnemonic.o Register.o Segment.o SharedData.o String.o Utils.o Zydis.o Decoder.o DecoderData.o Encoder.o EncoderData.o Disassembler.o Formatter.o FormatterBuffer.o FormatterATT.o FormatterBase.o FormatterIntel.o

CUSTOMFLAGS=-I. -I$(SRC)/libzycore

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lzycore

clean:
	rm -f $(PROGRAM) $(OBJS)
