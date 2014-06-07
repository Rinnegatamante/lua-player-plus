TARGET = lpp

OBJS = utility.o luaGraphics.o luaPlayer.o luaSystem.o luaControls.o main.o

CFLAGS = -O3 -frename-registers -ffast-math -fomit-frame-pointer -G0 -Wall -g
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)



LIBDIR =

LDFLAGS =

LIBS =  -lpsppower -llua -lc -lm  -lpsputility \
      -lpspnet_inet -lpspnet -lpsprtc \
      -lpsprtc_driver -lz -lpspkubridge -lpspsystemctrl_user




BUILD_PRX = 1
PRX_EXPORTS     = exports.exp
PSP_FW_VERSION = 401

PSP_LARGE_MEMORY = 1


PSPSDK  = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

exports:
		psp-build-exports -s exports.exp