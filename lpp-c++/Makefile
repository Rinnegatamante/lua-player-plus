#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#---------  This File is Part of : -------------------------------------------------------------------------------------#
#------- _  -------------------  ______   _   --------------------------------------------------------------------------#
#------ | | ------------------- (_____ \ | |  --------------------------------------------------------------------------#
#------ | | ---  _   _   ____    _____) )| |  ____  _   _   ____   ____   ----------------------------------------------#
#------ | | --- | | | | / _  |  |  ____/ | | / _  || | | | / _  ) / ___)  ----------------------------------------------#
#------ | |_____| |_| |( ( | |  | |      | |( ( | || |_| |( (/ / | |  --------------------------------------------------#
#------ |_______)\____| \_||_|  |_|      |_| \_||_| \__  | \____)|_|  --------------------------------------------------#
#------------------------------------------------- (____/  -------------------------------------------------------------#
#------------------------   ______   _   -------------------------------------------------------------------------------#
#------------------------  (_____ \ | |  -------------------------------------------------------------------------------#
#------------------------   _____) )| | _   _   ___   ------------------------------------------------------------------#
#------------------------  |  ____/ | || | | | /___)  ------------------------------------------------------------------#
#------------------------  | |      | || |_| ||___ |  ------------------------------------------------------------------#
#------------------------  |_|      |_| \____|(___/   ------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Licensed under the GPL License --------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Copyright (c) Nanni <cusunanni@hotmail.it> --------------------------------------------------------------------------#
#- Copyright (c) Rinnegatamante <rinnegatamante@eternalongju2.com> -----------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Official Forum : http://rinnegatamante.eu/luaplayerplus/forum.php ---------------------------------------------------#
#- For help using LuaPlayerPlus, coding help, and other please visit : http://rinnegatamante.eu/luaplayerplus/forum.php #
#-----------------------------------------------------------------------------------------------------------------------#
#- Credits : -----------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Homemister for LPHM sourcecode --------------------------------------------------------------------------------------#
#- Zack & Shine for LP Euphoria sourcecode -----------------------------------------------------------------------------#
#- ab5000 for support on psp-ita.com -----------------------------------------------------------------------------------#
#- valantin for sceIoMvdir and sceIoCpdir improved functions------------------------------------------------------------#
#- Dark_AleX for usbdevice ---------------------------------------------------------------------------------------------#
#- VirtuosFlame & ColdBird for iso drivers and kuBridge ----------------------------------------------------------------#
#- sakya for Media Engine and OslibMod ---------------------------------------------------------------------------------#
#- Booster & silverspring for EEPROM write/read functions --------------------------------------------------------------#
#- Akind for RemoteJoyLite ---------------------------------------------------------------------------------------------#
#- cooleyes for mpeg4 lib ----------------------------------------------------------------------------------------------#
#- Arshia001 for PSPAALib ----------------------------------------------------------------------------------------------#
#- InsertWittyName & MK2k for PGE sourcecode ---------------------------------------------------------------------------#
#- Youresam for LUA BMPLib ---------------------------------------------------------------------------------------------#
#- Raphael for vram manager code ---------------------------------------------------------------------------------------#
#- Dynodzzo for LSD concepts -------------------------------------------------------------------------------------------#
#- ab_portugal for Image.negative function -----------------------------------------------------------------------------#
#- JiCé for drawCircle function ----------------------------------------------------------------------------------------#
#- Rapper_skull & DarkGiovy for testing LuaPlayer Plus and coming up with some neat ideas for it. ----------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#

TARGET = LPP

LIBS_OBJS = Libs/Math/Math.o Libs/Graphics/Graphics.o Libs/Vram/vram.o \
Libs/LibnsGif/libnsgif.o Libs/intraFont/intraFont.o Libs/intraFont/libccc.o \
Libs/Timer/Timer.o Libs/Unrar/unrar_misc.o Libs/Unrar/unrar_open.o Libs/Unrar/archive.o \
Libs/Unrar/arcread.o Libs/Unrar/coder.o Libs/Unrar/crc.o Libs/Unrar/encname.o \
Libs/Unrar/extract.o Libs/Unrar/getbits.o Libs/Unrar/model.o Libs/Unrar/rarvm.o \
Libs/Unrar/rarvmtbl.o Libs/Unrar/rawread.o Libs/Unrar/suballoc.o Libs/Unrar/unicode.o \
Libs/Unrar/unpack.o Libs/Unrar/unpack15.o Libs/Unrar/unpack20.o Libs/Unrar/unrar.o \
Libs/7zip/7zAlloc.o Libs/7zip/7zBuf.o Libs/7zip/7zCrc.o Libs/7zip/7zDecode.o \
Libs/7zip/7zExtract.o Libs/7zip/7zHeader.o Libs/7zip/7zIn.o Libs/7zip/7zItem.o \
Libs/7zip/7zStream.o Libs/7zip/Bcj2.o Libs/7zip/Bra86.o Libs/7zip/LzmaDec.o \
Libs/Fex/Binary_Extractor.o Libs/Fex/blargg_common.o Libs/Fex/blargg_errors.o \
Libs/Fex/Data_Reader.o Libs/Fex/fex.o Libs/Fex/File_Extractor.o Libs/Fex/Gzip_Extractor.o \
Libs/Fex/Gzip_Reader.o Libs/Fex/Rar_Extractor.o Libs/Fex/Zip_Extractor.o \
Libs/Fex/Zip7_Extractor.o Libs/Fex/Zlib_Inflater.o Libs/Psar/Psar.o Libs/Dir/Dir.o \
Libs/AdHoc/AdHoc.o Libs/minIni/minIni.o Libs/Utils/Utils.o Libs/Battery/Battery.o \
Libs/Umd/Umd.o Libs/Utils/csoread.o Libs/Utils/gzip.o Libs/Utils/isofs_driver.o \
Libs/mXml/mxml-attr.o Libs/mXml/mxml-entity.o Libs/mXml/mxml-file.o \
Libs/mXml/mxml-get.o Libs/mXml/mxml-index.o Libs/mXml/mxml-node.o Libs/mXml/mxml-private.o \
Libs/mXml/mxml-search.o Libs/mXml/mxml-set.o Libs/mXml/mxml-string.o Libs/Mp4/Mp4.o \
Libs/cooleyesBridge/cooleyesBridge.o Libs/mem64/mem64.o Libs/Mp4/mp4_file.o Libs/Mp4/mp4_read.o \
Libs/Mp4/mp4info.o Libs/Mp4/mpegbase.o Libs/Mp4/time_math.o Libs/Mp4/util.o Libs/Mp4/atom.o \
Libs/freeMem.o Libs/strreplace.o  Libs/danzeff/danzeff.o Libs/Audio/pspaalib.o Libs/Audio/pspaalibat3.o \
Libs/Audio/pspaalibeffects.o Libs/Audio/pspaalibogg.o Libs/Audio/pspaalibscemp3.o Libs/Audio/pspaalibwav.o \
Libs/Cam/Cam.o

LUA_SRC_OBJS = Lua/src/lapi.o Lua/src/lauxlib.o Lua/src/lbaselib.o \
Lua/src/lcode.o Lua/src/ldblib.o Lua/src/ldebug.o Lua/src/ldo.o \
Lua/src/ldump.o Lua/src/lfunc.o Lua/src/lgc.o Lua/src/linit.o \
Lua/src/liolib.o Lua/src/llex.o Lua/src/lmathlib.o Lua/src/lmem.o \
Lua/src/loadlib.o Lua/src/lobject.o Lua/src/lopcodes.o Lua/src/loslib.o \
Lua/src/lparser.o Lua/src/lstate.o Lua/src/lstring.o Lua/src/lstrlib.o \
Lua/src/ltable.o Lua/src/ltablib.o Lua/src/ltm.o Lua/src/lundump.o \
Lua/src/lvm.o Lua/src/lzio.o Lua/src/print.o

LUA_OBJS = $(LUA_SRC_OBJS) Lua/Graphics.o Lua/Controls.o Lua/Archive.o Lua/Timer.o \
Lua/AdHoc.o Lua/Wlan.o Lua/System.o Lua/Mp4.o Lua/Audio.o Lua/Xml.o Lua/Camera.o

OBJS = $(LIBS_OBJS) $(CLASSES_OBJS) $(LUA_OBJS) LPP.o Support/Support.o Stubs.o

INCDIR =
LIBDIR = Libs/
CFLAGS = -O3 -frename-registers -ffast-math -fomit-frame-pointer -G0 -Wall -g -DPSPFW3xx -DRELEASE
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lpspgum -lpspgu -lm -lpspreg -lpsprtc -ljpeg -lpng -lzzip -lz -lstdc++ -lfreetype \
-lpspwlan -lpsphttp -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet_adhocctl \
-lpsppower -lpspkubridge -lpspusbdevice -lpspusb -lpspusbstor -lpspumd -lpspopenpsid \
-lvorbisidec -lpsphprm -lpspaudiocodec -lpspaudiolib -lpspusbcam \
-lpspaudio -lpspmpeg -lpspmp3 -lpspatrac3 -lpspjpeg

PSP_FW_VERSION = 370
BUILD_PRX = 1
PSP_LARGE_MEMORY = 1

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Lua Player Plus
PSP_EBOOT_ICON = "ICON0.png"

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

LPP.o: Support/Support-c.c
Libs/Graphics/Graphics.o : Libs/Graphics/vera.c Libs/Graphics/veraMono.c
Libs/danzeff/danzeff.o: Libs/danzeff/Images.c
