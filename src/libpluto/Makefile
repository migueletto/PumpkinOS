include ../common.mak
include ../commonp.mak

PROGRAM=$(BIN)/libpluto$(SOEXT)

OBJS=VectorGraphics.o plutosvg.o plutovg-blend.o plutovg-dash.o plutovg-font.o plutovg-geometry.o plutovg-paint.o \
     plutovg-rle.o plutovg.o sw_ft_math.o sw_ft_raster.o sw_ft_stroker.o

CUSTOMPFLAGS=-I.

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit

clean:
	rm -f $(PROGRAM) $(OBJS)
