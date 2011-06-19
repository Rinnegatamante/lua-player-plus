/*
 * LuaPlayer Euphoria
 * ------------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE for details.
 *
 * Copyright (c) 2005 Frank Buss <fb@frank-buss.de> (aka Shine)
 * Copyright (c) 2009 Danny Glover <danny86@live.ie> (aka Zack) 
 *
 * Official Forum : http://www.retroemu.com/forum/forumdisplay.php?f=148
 * For help using LuaPlayer, code help, tutorials etc please visit the official site : http://www.retroemu.com/forum/forumdisplay.php?f=148
 *
 * Credits:
 * 
 * (from Shine/Zack) 
 *
 *   many thanks to the authors of the PSPSDK from http://forums.ps2dev.org
 *   and to the hints and discussions from #pspdev on freenode.net
 *
 * (from Zack Only)
 *
 * Thanks to Brunni for the Swizzle/UnSwizzle code (taken from oslib). 
 * Thanks to Arshia001 for AALIB. It is the sound engine used in LuaPlayer Euphoria. 
 * Thanks to HardHat for being a supportive friend and advisor.
 * Thanks to Benhur for IntraFont.
 * Thanks to Jono for the moveToVram code.
 * Thanks to Raphael for the Vram manager code.
 * Thanks to Osgeld, Dan369 & Cmbeke for testing LuaPlayer Euphoria for me and coming up with some neat ideas for it.
 * Thanks to the entire LuaPlayer Euphoria userbase, for using it and for supporting it's development. You guys rock!
 *
 *
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pspkernel.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <psppower.h>
#include <pspdebug.h>
#include <psprtc.h>
#include <stdarg.h>
#include <pspgu.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include "include/luaplayer.h"
#include "libs/graphics/graphics.h"
#include "libs/sce/msgDialog.h"
#include "libs/sce/osk.h"
#include "libs/sce/browser.h"

/*
 *
 ---------			Directory Functions			---------
 *
 */

static int lua_getCurrentDirectory(lua_State *L)
{
	char path[256];
	getcwd(path, 256);
	lua_pushstring(L, path);
	
	return 1;
}

static int lua_setCurrentDirectory(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	if(!path) return luaL_error(L, "System.currentDirectory(file) takes a filename as a string argument.");

	lua_getCurrentDirectory(L);
	chdir(path);
	
	return 1;
}

static int lua_curdir(lua_State *L) {
	int argc = lua_gettop(L);
	if(argc == 0) return lua_getCurrentDirectory(L);
	if(argc == 1) return lua_setCurrentDirectory(L);
	return luaL_error(L, "System.currentDirectory([file]) takes zero or one argument");
}


// Move g_dir to the stack and MEET CERTAIN DOOM! If the SceIoDirent is found on the STACK instead of among the globals, the PSP WILL CRASH IN A VERY WEIRD WAY. You have been WARNED.
SceIoDirent g_dir;

static int lua_dir(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 0 && argc != 1) return luaL_error(L, "System.listDirectory([path]) takes zero or one argument");
	

	const char *path = "";
	if (argc == 0) {
		path = "";
	} else {
		path = luaL_checkstring(L, 1);
	}
	int fd = sceIoDopen(path);

	if (fd < 0) {
		lua_pushnil(L);  /* return nil */
		return 1;
	}
	lua_newtable(L);
	int i = 1;
	while (sceIoDread(fd, &g_dir) > 0) {
		lua_pushnumber(L, i++);  /* push key for file entry */

		lua_newtable(L);
			lua_pushstring(L, "name");
			lua_pushstring(L, g_dir.d_name);
			lua_settable(L, -3);
	
			lua_pushstring(L, "size");
			lua_pushnumber(L, g_dir.d_stat.st_size);
			lua_settable(L, -3);
	
			lua_pushstring(L, "directory");
			lua_pushboolean(L, FIO_S_ISDIR(g_dir.d_stat.st_mode));
			lua_settable(L, -3);

		lua_settable(L, -3);
	}

	sceIoDclose(fd);

	return 1;  /* table is already on top */
}

static int lua_createDir(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	if(!path) return luaL_error(L, "System.createDirectory(directory) takes a directory name as a string argument.");

	mkdir(path, 0777);
	
	return 0;
}

static int lua_removeDir(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	if(!path) return luaL_error(L, "System.removeDirectory(directory) takes a directory name as a string argument.");

	rmdir(path);
	
	return 0;
}

static int lua_removeFile(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	if(!path) return luaL_error(L, "System.removeFile(filename) takes a filename as a string argument.");

	remove(path);
	
	return 0;
}

static int lua_rename(lua_State *L)
{
	const char *oldName = luaL_checkstring(L, 1);
	const char *newName = luaL_checkstring(L, 2);
	if(!oldName || !newName)
		return luaL_error(L, "System.rename(source, destination) takes two filenames as string arguments");

	// TODO: looks like the stdio "rename" doesn't work
	sceIoRename(oldName, newName);
	
	return 0;
}

/*
 *
 ---------			USB Functions			---------
 *
 */
 
SceUID modules[8];

void StopUnloadModule(SceUID modID)
{
	int status = 0;
	sceKernelStopModule(modID, 0, NULL, &status, NULL);
	sceKernelUnloadModule(modID);
}

int InitUsbStorage() 
{
	u32 retVal = 0;

	//start necessary drivers
	modules[0] = pspSdkLoadStartModule("flash0:/kd/chkreg.prx", PSP_MEMORY_PARTITION_KERNEL);
	modules[1] = pspSdkLoadStartModule("flash0:/kd/npdrm.prx", PSP_MEMORY_PARTITION_KERNEL);
	modules[2] = pspSdkLoadStartModule("flash0:/kd/semawm.prx", PSP_MEMORY_PARTITION_KERNEL);
	modules[3] = pspSdkLoadStartModule("flash0:/kd/usbstor.prx", PSP_MEMORY_PARTITION_KERNEL);
	modules[4] = pspSdkLoadStartModule("flash0:/kd/usbstormgr.prx", PSP_MEMORY_PARTITION_KERNEL);
	modules[5] = pspSdkLoadStartModule("flash0:/kd/usbstorms.prx", PSP_MEMORY_PARTITION_KERNEL);
	modules[6] = pspSdkLoadStartModule("flash0:/kd/usbstorboot.prx", PSP_MEMORY_PARTITION_KERNEL);
	modules[7] = pspSdkLoadStartModule("flash0:/kd/usbdevice.prx", PSP_MEMORY_PARTITION_KERNEL);

	//setup USB drivers
	retVal = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
	if (retVal != 0)
	return -6;

	retVal = sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
	if (retVal != 0)
	return -7;

	retVal = sceUsbstorBootSetCapacity(0x800000);
	if (retVal != 0)
	return -8;

	return 0;
}

int StartUsbStorage() 
{
	return sceUsbActivate(0x1c8);
}

int StopUsbStorage() 
{
	int retVal = sceUsbDeactivate(0x1c8);
	sceIoDevctl("fatms0:", 0x0240D81E, NULL, 0, NULL, 0 ); //Avoid corrupted files
	
	return retVal;
}

int DeinitUsbStorage() 
{
	int i;
	unsigned long state = sceUsbGetState();
	if (state & PSP_USB_ACTIVATED)
	StopUsbStorage();
	sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0);
	sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
	
	for (i=7; i>=0; i--)
	if (modules[i] >= 0)
	StopUnloadModule(modules[i]);
	
return 0;
}

static int lua_usbActivate(lua_State *L)
{
	if (lua_gettop(L) != 0) 
	{
		return luaL_error(L, "System.usbDiskModeActivate() takes no arguments");
	}
	InitUsbStorage();
	StartUsbStorage();
	
	return 0;
}

static int lua_usbDeactivate(lua_State *L)
{
	if (lua_gettop(L) != 0)
	{
		return luaL_error(L, "System.usbDiskModeDeactivate() takes no arguments");
	}
	StopUsbStorage();
	DeinitUsbStorage();
	
	return 0;
}

/*
 *
 ---------			BATTERY Functions			---------
 *
 */
 
static int lua_powerIsPowerOnline(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "System.powerIsPowerOnline() takes no arguments");
	lua_pushboolean(L, scePowerIsPowerOnline());
	return 1;
}

static int lua_powerIsBatteryExist(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "System.powerIsBatteryExist() takes no arguments");
	lua_pushboolean(L, scePowerIsBatteryExist());
	return 1;
}

static int lua_powerIsBatteryCharging(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "System.powerIsBatteryCharging() takes no arguments");
	lua_pushboolean(L, scePowerIsBatteryCharging());
	return 1;
}

static int lua_powerGetBatteryChargingStatus(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "System.powerGetBatteryChargingStatus() takes no arguments");
	lua_pushnumber(L, scePowerGetBatteryChargingStatus());
	return 1;
}

static int lua_powerIsLowBattery(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "System.powerIsLowBattery() takes no arguments");
	lua_pushboolean(L, scePowerIsLowBattery());
	return 1;
}

static int lua_powerGetBatteryLifePercent(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "System.powerGetBatteryLifePercent() takes no arguments");
	lua_pushnumber(L, scePowerGetBatteryLifePercent());
	return 1;
}

static int lua_powerGetBatteryLifeTime(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "System.powerGetBatteryLifeTime() takes no arguments");
	lua_pushnumber(L, scePowerGetBatteryLifeTime());
	return 1;
}

static int lua_powerGetBatteryTemp(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "System.powerGetBatteryTemp() takes no arguments");
	lua_pushnumber(L, scePowerGetBatteryTemp());
	return 1;
}

static int lua_powerGetBatteryVolt(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "System.powerGetBatteryVolt() takes no arguments");
	lua_pushnumber(L, scePowerGetBatteryVolt());
	return 1;
}

static int lua_powerTick(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "System.powerTick() takes no arguments");
	scePowerTick(0);
	return 0;
}

static int lua_md5sum(lua_State *L)
{
	size_t size;
	const char *string = luaL_checklstring(L, 1, &size);
	if (!string) return luaL_error(L, "System.md5sum() takes a string as argument.");

	u8 digest[16];
	sceKernelUtilsMd5Digest((u8*)string, size, digest);
	int i;
	char result[33];
	for (i = 0; i < 16; i++) sprintf(result + 2 * i, "%02x", digest[i]);
	lua_pushstring(L, result);
	
	return 1;
}

static int lua_sleep(lua_State *L)
{
	if (lua_gettop(L) != 1) return luaL_error(L, "System.sleep() accepts only milliseconds as an argument");
	int milliseconds = luaL_checkint(L, 1);
	sceKernelDelayThread(milliseconds * 1000);
	return 0;
}

static int lua_getFreeMemory(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "System.getFreeMemory() takes no arguments");
	
	// for a realistic number, try to allocate 1 mb blocks until out of memory
	void* buf[64];
	int i = 0;
	for (i = 0; i < 64; i++) {
		buf[i] = malloc(1024 * 1024);
		if (!buf[i]) break;
	}
	int result = i;
	for (; i >= 0; i--) {
		free(buf[i]);
	}
	lua_pushnumber(L, result * 1024 * 1024);
	return 1;
}

/*
 *
 ---------			GU Functions			---------
 *
 */

 //Start the GU
static int lua_StartGu(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 0)
	{
		return luaL_error(L, "System.draw() takes no arguments");
	}
	
	guStart();
	return 0;
}

//End the gu and sync
static int lua_EndGu(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 0)
	{
		return luaL_error(L, "System.endDraw() takes no arguments");
	}
	
	sceGuTexMode(GU_PSM_8888,0,0,GU_TRUE);
	guEnd();
	return 0;
}

/*
 *
 ---------			FPS Functions			---------
 *
 */

static u64 timeNow;
static u64 timeLastAsk;
static u32 tickResolution;

static int fps = 0;		// for calculating the frames per second
static char fpsDisplay[200];
static u64 timeNowFPS;
static u64 timeLastFPS;

//## Set draw buffer for debug printing
static void setDrawBuffer()
{
	static int drawBuffer=0;
	drawBuffer=0x88000-drawBuffer; 
	pspDebugScreenSetOffset((int)drawBuffer);
}

//## Print Debug Messages ##
static void printDebug(int x, int y, const char *format, ...)
{
	setDrawBuffer();
	pspDebugScreenSetXY(x,y);
	
	va_list	opt;
	char     buff[2048];
	int		bufsz;
   
	va_start(opt, format);
	bufsz = vsnprintf( buff, (size_t) sizeof(buff), format, opt);
	(void) pspDebugScreenPrintData(buff, bufsz);
}


//#Is required before calls to getDeltaTime()
void initTimer()
{
	sceRtcGetCurrentTick( &timeLastAsk );
	sceRtcGetCurrentTick( &timeLastFPS );
	tickResolution = sceRtcGetTickResolution();
}

//#Gets the time since the last call measured in seconds
float getDeltaTime()
{
	sceRtcGetCurrentTick( &timeNow );
	float dt = ( timeNow - timeLastAsk ) / ((float) tickResolution );
	timeLastAsk = timeNow;
 
	return dt;
}

static void showFPS()
{
	fps +=1;
	sceRtcGetCurrentTick( &timeNowFPS );
	
	if( ((timeNowFPS - timeLastFPS)/((float)tickResolution)) >= 1.0f )
	{
		timeLastFPS = timeNowFPS;
		sprintf(fpsDisplay, "%d FPS", fps);
		fps = 0;
	}
	printDebug(0,0, fpsDisplay);
}

//The lua part...
static int lua_showFPS(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 0)
	{
		return luaL_error(L, "System.showFPS() takes no arguments");
	}
	showFPS();
	
	return 0;
}

/*
 *
 ---------			CPU Functions			---------
 *
 */

//Set CPU Speed...
static int lua_setCpuSpeed(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 1) return luaL_error(L, "System.setCpuSpeed(speed) takes only 1 argument (ie. System.setCpuSpeed(333) )");
	
	unsigned int speed = luaL_checkint(L, 1);
	
	switch (speed)
	{
		case 100 :
			scePowerSetClockFrequency(100, 100, 100);
		break;
		
		case 222 : 
			scePowerSetClockFrequency(222, 222, 111);
		break;
		
		case 266 : 
			scePowerSetClockFrequency(266, 266, 133);
		break;
		
		case 333 : 
			scePowerSetClockFrequency(333, 333, 166);
		break;
	}
	
	return 0;
}

//Exit back to xmb
static int lua_systemQuit(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 0) 
	{
		return luaL_error(L, "System.quit() takes no arguments");
	}
	
	sceKernelExitGame();
	
	return 0;
}

/*
 *
 ---------			SCE Functions			---------
 *
 */

//SCE Message Dialog
static int lua_SCEShowMessageDialog(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 1 || argc > 3) return luaL_error(L, "System.msgDialog() takes 1, 2 or 3 arguments");
	
	
	pspUtilityMsgDialogParams dialog;
	
	int opts = PSP_UTILITY_MSGDIALOG_OPTION_TEXT;
	if (argc>1 && lua_toboolean(L, 2))
		opts |= PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS;		
		
	dialog_create(&dialog, luaL_checkstring(L, 1), PSP_UTILITY_MSGDIALOG_MODE_TEXT, opts);

    int draw=1;
	while(draw) 
	{
		guStart();
		
		if (argc>=3 && lua_isfunction(L, 3))
		{
			lua_pushvalue(L, 3);   /* push drawfunc */
			lua_call(L, 0, 0);     /* call it*/
		}
		else clearScreen(0xff554433);
		
		guEnd();

		draw = dialog_update();		
		
		sceDisplayWaitVblankStart();	
		flipScreen();
	}
	
	lua_pushnumber(L, dialog.buttonPressed);

	return 1;
}

//SCE Osk
static int lua_SCEOsk(lua_State *L)
{
	int argc = lua_gettop(L);
	if (2 > argc || argc > 4) 
	{
		return luaL_error(L, "System.osk(description, intialText [, inputType [, drawFunc]]) takes 2, 3 or 4 arguments");
	}
	
	int inputtype = 0; // Allow all input types
	
	if (argc>=3) inputtype = luaL_checkint(L, 3);
	
	SceUtilityOskData data;
	unsigned short* desc = memalign(16, sizeof(unsigned short)*strlen(luaL_checkstring(L, 1)));
	char2UShort(luaL_checkstring(L, 1), desc);
	unsigned short* intext = memalign(16, sizeof(unsigned short)*strlen(luaL_checkstring(L, 2)));
	char2UShort(luaL_checkstring(L, 2), intext);
	
	
	osk_create(&data, desc, intext, inputtype);
	
	int draw = 1;
	while (draw)
	{
		guStart();
		
		if (argc>=4 && lua_isfunction(L, 4))
		{
			lua_pushvalue(L, 4);   /* push drawfunc */
			lua_call(L, 0, 0);     /* call it*/
		}
		else
			clearScreen(0xff554433);
			
		guEnd();
		
		draw = osk_update();	
		
		sceDisplayWaitVblankStart();	
		flipScreen();
	}
	
	char *outtext = (char*) memalign(16, data.outtextlength*sizeof(char)+1);
	
	int i;
	for (i=0; i<data.outtextlength; ++i)
		outtext[i] = data.outtext[i];
	
	lua_pushfstring(L, outtext);
	lua_pushnumber(L, data.result);
	
	
	free(outtext);
	free(intext);
	free(desc);
	
	return 2;
}

//Register our System Functions
static const luaL_reg System_functions[] = {
  {"draw", 						    lua_StartGu},
  {"endDraw",                       lua_EndGu},
  {"setCpuSpeed", 					lua_setCpuSpeed},
  {"showFPS",						lua_showFPS},
  {"quit",                          lua_systemQuit},
  {"msgDialog",						lua_SCEShowMessageDialog},
  {"osk",							lua_SCEOsk},
  {"currentDirectory",              lua_curdir},
  {"listDirectory",           	    lua_dir},
  {"createDirectory",               lua_createDir},
  {"removeDirectory",               lua_removeDir},
  {"removeFile",                    lua_removeFile},
  {"rename",                        lua_rename},
  {"usbDiskModeActivate",           lua_usbActivate},
  {"usbDiskModeDeactivate",    	    lua_usbDeactivate},
  {"powerIsPowerOnline",            lua_powerIsPowerOnline},
  {"powerIsBatteryExist",           lua_powerIsBatteryExist},
  {"powerIsBatteryCharging",        lua_powerIsBatteryCharging},
  {"powerGetBatteryChargingStatus", lua_powerGetBatteryChargingStatus},
  {"powerIsLowBattery",             lua_powerIsLowBattery},
  {"powerGetBatteryLifePercent",    lua_powerGetBatteryLifePercent},
  {"powerGetBatteryLifeTime",       lua_powerGetBatteryLifeTime},
  {"powerGetBatteryTemp",           lua_powerGetBatteryTemp},
  {"powerGetBatteryVolt",           lua_powerGetBatteryVolt},
  {"powerTick",                     lua_powerTick},
  {"md5sum",                        lua_md5sum},
  {"sleep",                         lua_sleep},
  {"getFreeMemory",                 lua_getFreeMemory},
  {0, 0}
};

void luaSystem_init(lua_State *L) {
	luaL_openlib(L, "System", System_functions, 0);
	
#define PSP_UTILITY_CONSTANT(name)\
	lua_pushstring(L, #name);\
	lua_pushnumber(L, PSP_UTILITY_##name);\
	lua_settable(L, -3);

	lua_pushstring(L, "System");
	lua_gettable(L, LUA_GLOBALSINDEX);
	
	PSP_UTILITY_CONSTANT(MSGDIALOG_RESULT_UNKNOWN1);
	PSP_UTILITY_CONSTANT(MSGDIALOG_RESULT_YES);
	PSP_UTILITY_CONSTANT(MSGDIALOG_RESULT_NO);
	PSP_UTILITY_CONSTANT(MSGDIALOG_RESULT_BACK);
	
	PSP_UTILITY_CONSTANT(OSK_RESULT_UNCHANGED);
	PSP_UTILITY_CONSTANT(OSK_RESULT_CANCELLED);
	PSP_UTILITY_CONSTANT(OSK_RESULT_CHANGED);
	
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_ALL);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_LATIN_DIGIT);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_LATIN_SYMBOL);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_LATIN_LOWERCASE);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_LATIN_UPPERCASE);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_JAPANESE_DIGIT);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_JAPANESE_SYMBOL);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_JAPANESE_LOWERCASE);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_JAPANESE_UPPERCASE);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_JAPANESE_HIRAGANA);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_JAPANESE_HALF_KATAKANA);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_JAPANESE_KATAKANA);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_JAPANESE_KANJI);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_RUSSIAN_LOWERCASE);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_RUSSIAN_UPPERCASE);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_KOREAN);
	PSP_UTILITY_CONSTANT(OSK_INPUTTYPE_URL);
}
