# https://developer.arm.com/documentation/ka004751/latest/

SOURCE=addtf3.c ashlti3.c clzti2.c comparetf2.c divtf3.c eprintf.c extenddftf2.c extendhftf2.c extendsftf2.c fixtfdi.c fixtfsi.c fixtfti.c fixunstfdi.c fixunstfsi.c fixunstfti.c floatditf.c floatsitf.c floatunditf.c floatunsitf.c fp_mode.c int_util.c lshrti3.c multf3.c powitf2.c subtf3.c trunctfdf2.c trunctfhf2.c trunctfsf2.c
OBJS=$(SOURCE:%.c=%.o)

CFLAGS=-DCOMPILER_RT_HAS_FLOAT16 -march=armv8-a

float.a: $(OBJS)
	ar -cr float.a $(OBJS)
