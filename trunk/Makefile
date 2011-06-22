TARGET = lua

LUA_OBJS = 			src/luaPlayer.o src/utility.o \
					src/luaControls.o src/luaGraphics.o src/luaSound.o \
					src/lua3d.o src/luaTimer.o src/luaSystem.o src/luaWlan.o \
					src/luaAdhoc.o

ADHOC_OBJS = 		src/libs/adhoc/adhoc.o

AUDIO_OBJS = 		src/libs/aalib/pspaalib.o src/libs/aalib/pspaalibwav.o \
			 		src/libs/aalib/pspaalibscemp3.o src/libs/aalib/pspaalibogg.o \
					src/libs/aalib/pspaalibat3.o src/libs/aalib/pspaalibeffects.o

GRAPHICS_OBJS = 	src/libs/vram/vram.o \
					src/libs/graphics/graphics.o src/libs/graphics/framebuffer.o

INTRAFONT_OBJS = 	src/libs/intraFont/libccc.o src/libs/intraFont/intraFont.o \
					src/pspDveManager.o

SCE_OBJS =			src/libs/sce/msgDialog.o src/libs/sce/osk.o src/libs/sce/browser.o

UNZIP_OBJS =

OBJS =	$(UNZIP_OBJS) $(LUA_OBJS) $(ADHOC_OBJS) $(VFPU_OBJS) $(AUDIO_OBJS) $(GRAPHICS_OBJS) $(TILEMAP_OBJS) $(INTRAFONT_OBJS) $(SCE_OBJS) src/main.o src/loadlib.o src/extralibs.o

CFLAGS = -O3 -frename-registers -ffast-math -fomit-frame-pointer -G0 -Wall -g
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)



LIBDIR =

LDFLAGS =

LIBS = 	-llua -lpsprtc -lpng -ljpeg -lpspgum -lpspgum_vfpu -lpspvfpu -lpspgu -lz -lpspaudio -lpspmp3 -lmad -lpsppower -lvorbisidec -lpspatrac3 -lm \
		-lpspusb -lpspusbstor -lpsppower -lpsphttp -lpspssl -lpspwlan -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet_adhocctl -lpsputility -lcurl \
		-lpspgu -lpsphprm_driver -lpspkubridge -lpspsystemctrl_user -lpspumd




BUILD_PRX = 1

PSP_FW_VERSION = 401

PSP_LARGE_MEMORY = 1



EXTRA_TARGETS = EBOOT.PBP

PSP_EBOOT_TITLE = LuaPlayer Plus
PSP_EBOOT_ICON = Eboot/ICON0.png


PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

src/main.o: src/boot.c

all :
	cp	$(TARGET).prx debug
