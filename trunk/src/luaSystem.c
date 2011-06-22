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
#include <pspLoadExec.h>
#include <stdarg.h>
#include <pspgu.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <psploadexec_kernel.h>
#include "include/luaplayer.h"
#include "include/kubridge.h"
#include "include/systemctrl.h"
#include "libs/graphics/graphics.h"
#include "libs/sce/msgDialog.h"
#include "libs/sce/osk.h"
#include "libs/sce/browser.h"
#include "libs/unrar/unrarlib.h"
#include "libs/vfile/VirtualFile.h"
#include "ISODriver/march33.h"

#define SIO_IOCTL_SET_BAUD_RATE 1
#define LOADMODULE_ARGERROR "Argument error: System.loadModule(module, init) takes a module name and init method as string arguments."

#define bool int
#define TRUE 1
#define FALSE 0

const OSL_VIRTUALFILENAME __iso_Driver_module[] =
{
    {"ram:/Media/march33.prx", (void*)march33, sizeof(march33), &VF_MEMORY},
};

static int usbActivated = 0;
SceUID irda_fd = -1;
static SceUID sio_fd = -1;
static const char* sioNotInitialized = "SIO not initialized.";

SceUID psploadlib( const char * name, char * init );
void **findFunction( SceUID id, const char * library, const char * name );
int init( lua_State *L);

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

void RemoveDir(const char* path)
{
int fd = sceIoDopen(path);

	if (fd < 0) {

	}
	while (sceIoDread(fd, &g_dir) > 0) {
		if (g_dir.d_stat.st_attr & FIO_SO_IFDIR)
		{
		RemoveDir(g_dir.d_name);
		}else{
		remove(g_dir.d_name);
		}
	}

	sceIoDclose(fd);
	rmdir(path);
}

static int lua_removeDir(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	if(!path) return luaL_error(L, "System.removeDirectory(directory) takes a directory name as a string argument.");

	RemoveDir(path);

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

static int LoadStartModule(char *path)
{
	u32 loadResult;
	u32 startResult;
	int status;

	loadResult = sceKernelLoadModule(path, 0, NULL);
	if (loadResult & 0x80000000)
		return -1;
	else
		startResult =
		sceKernelStartModule(loadResult, 0, NULL, &status, NULL);

	if (loadResult != startResult)
		return -2;

	return 0;
}

static int lua_loadModule(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	const char *init = luaL_checkstring(L, 2);

	SceUID uid = psploadlib( name, NULL );
	if ( uid >= 0 ) {
		lua_CFunction f = (lua_CFunction) *(findFunction( uid, name, init ));
		if (f != NULL)
		{
			lua_pushlightuserdata(L,(void*)uid);
			lua_pushcclosure(L,f,1);
			return 1;
		}
	}

	lua_pushnil(L);
	lua_pushstring(L, LOADMODULE_ARGERROR );
	lua_pushstring(L, (uid >= 0) ? "init" : "open");

	return 3;
}

static int lua_usbActivate(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	if (usbActivated) return 0;

	static int modulesLoaded = 0;
	if (!modulesLoaded) {
		//start necessary drivers
		LoadStartModule("flash0:/km/semawm.prx");
		LoadStartModule("flash0:/km/usbstor.prx");
		LoadStartModule("flash0:/km/usbstormgr.prx");
		LoadStartModule("flash0:/km/usbstorms.prx");
		LoadStartModule("flash0:/km/usbstorboot.prx");

		//setup USB drivers
		int retVal = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
		if (retVal != 0) {
			printf("Error starting USB Bus driver (0x%08X)\n", retVal);
		}
		retVal = sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
		if (retVal != 0) {
			printf("Error starting USB Mass Storage driver (0x%08X)\n", retVal);
		}
		retVal = sceUsbstorBootSetCapacity(0x800000);
		if (retVal != 0) {
			printf("Error setting capacity with USB Mass Storage driver (0x%08X)\n", retVal);
		}
		retVal = 0;
		modulesLoaded = 1;
	}
	sceUsbActivate(0x1c8);
	usbActivated = 1;
	return 0;
}

static int lua_usbDeactivate(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	if (!usbActivated) return 0;

	sceUsbDeactivate( 0 );  // what value here?
	usbActivated = 0;
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
static int lua_setLow(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
	if (scePowerGetCpuClockFrequencyInt() != 100){
		scePowerSetCpuClockFrequency(100);
		scePowerSetBusClockFrequency(50);
	}
	return 0;
}

static int lua_setReg(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
	if (scePowerGetCpuClockFrequencyInt() != 222){
		scePowerSetCpuClockFrequency(222);
		scePowerSetBusClockFrequency(111);
	}
	return 0;
}

static int lua_setHigh(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
	if (scePowerGetCpuClockFrequencyInt() != 333){
		scePowerSetClockFrequency(333,333,166);
	}
	return 0;
}

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

int Dir_NotExist(const char *PathDirectory)
{
   SceIoStat stato;
   sceIoGetstat( PathDirectory, &stato);
   if ( stato.st_mode & FIO_S_IFDIR )
   {
         return 1;
    }else{
         return 0;
    }
}

void EseguiPBP(const char* file)
{
	struct SceKernelLoadExecVSHParam param;
	char argp[256];
	int args;
	strcpy(argp, file);
	args = strlen(file)+1;
	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.args = args;
	param.argp = argp;
	param.key = NULL;
	param.vshmain_args_size = 0;
	param.vshmain_args = NULL;
	if (kuKernelGetModel() == 3){
	sctrlKernelLoadExecVSHEf2(file, &param);
	}else{
	sctrlKernelLoadExecVSHMs2(file, &param);
	}
}

void EseguiUpdate(const char* file)
{
	struct SceKernelLoadExecVSHParam param;
	char argp[256];
	int args;
	strcpy(argp, file);
	args = strlen(file)+1;
	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.args = args;
	param.argp = argp;
	param.key = NULL;
	param.vshmain_args_size = 0;
	param.vshmain_args = NULL;
	sctrlKernelLoadExecVSHMs1(file, &param);
	}

//Start hb from MS
static int lua_startPBP(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 1)
	{
		return luaL_error(L, "System.startPBP() takes only one argument");
	}
	const char *file = luaL_checkstring(L, 1);
	if (Dir_NotExist(file))
	{
		return luaL_error(L, "File doesn't exist");
	}else{
	EseguiPBP(file);
	}
	return 1;
}

//Start updates from MS
static int lua_startUpdate(lua_State *L)
{
    const char *file = luaL_checkstring(L, 1);
	int argc = lua_gettop(L);
	if (argc > 1)
	{
		return luaL_error(L, "System.startUpdate() takes one or none argument");
	}
	if (Dir_NotExist(file))
	{
		return luaL_error(L, "File doesn't exist");
	}else{
	if (argc == 1)
	{
	EseguiUpdate(file);
	}else{
	if (Dir_NotExist("ms0:/PSP/GAME/UPDATE/EBOOT.PBP"))
	{
	return luaL_error(L, "File doesn't exist");
	}else
	{
	if (kuKernelGetModel() == 3){
	EseguiUpdate("ef0:/PSP/GAME/UPDATE/EBOOT.PBP");
	}else{
	EseguiUpdate("ms0:/PSP/GAME/UPDATE/EBOOT.PBP");
	}
	}
	}
	}
	return 1;
}

//Start UMD
static int lua_startUMD(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 0)
	{
		return luaL_error(L, "System.launchUMD() takes no argument");
	}
	if (Dir_NotExist("disc0:/PSP_GAME")){
		return luaL_error(L, "Error: No UMD inserted");
	}else{
	if (Dir_NotExist("disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"))
	{
	sctrlKernelLoadExecVSHDisc("disc0:/PSP_GAME/SYSDIR/BOOT.BIN", NULL);
	}else{
	sctrlKernelLoadExecVSHDisc("disc0:/PSP_GAME/SYSDIR/EBOOT.BIN", NULL);
	}
	}
	return 1;
}

//Start updates from UMD
static int lua_startUMDUpdate(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 0)
	{
		return luaL_error(L, "System.startUMDUpdate() takes no argument");
	}
	if (Dir_NotExist("disc0:/PSP_GAME")){
		return luaL_error(L, "Error: No UMD inserted");
	}else{
	sctrlKernelLoadExecVSHDiscUpdater("disc0:/PSP_GAME/SYSDIR/UPDATE/EBOOT.BIN", NULL);
	}
	return 1;
}

//Get PSP Model
static int lua_getModel(lua_State *L)
{
    char stringa[256];
	int argc = lua_gettop(L);
	if (argc > 0)
	{
		return luaL_error(L, "System.getModel() takes no argument");
	}
	int model;
	model = ((kuKernelGetModel() + 1) * 1000);
	if (model == 4000){
	sprintf(stringa, "N1000");
	}else{
	sprintf(stringa, "%i", model);
	}
	lua_pushstring(L, stringa);
	return 1;
}

// Assign/UnAssign Devices
static int lua_Unassign(lua_State *L)
{
	int uas1;
	char temp[40];
	const char *device = luaL_checkstring(L, 1);
	uas1 = sceIoUnassign(device);
	if(uas1 < 0)
		{
			sprintf(temp,"Failed to unassign \'%s\'",temp);
			return luaL_error(L,temp);
		}
	return 1;

}
static int lua_Assign(lua_State *L)
{
	int as1;
	char temp[40];
	const char *dev1=luaL_checkstring(L, 1), *dev2=luaL_checkstring(L, 2), *dev3=luaL_checkstring(L, 3);
	as1  = sceIoAssign(dev1, dev2, dev3, IOASSIGN_RDWR, NULL, 0);
	if(as1 < 0)
		{
			sprintf(temp,"Failed to assign \'%s\'",dev1);
			return luaL_error(L,temp);
		}
	return 1;
}

// Irda Functions
static int lua_irdaInit(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
	if (irda_fd < 0) irda_fd = sceIoOpen("irda0:", PSP_O_RDWR, 0);
	if (irda_fd < 0) return luaL_error(L, "failed create IRDA handle.");

	return 0;
}

static int lua_irdaWrite(lua_State *L)
{
	if (irda_fd < 0) return luaL_error(L, "irda not initialized");
	size_t size;
	const char *string = luaL_checklstring(L, 1, &size);
	if (!string) return luaL_error(L, "Argument error: System.sioWrite(string) takes a string as argument.");
	sceIoWrite(irda_fd, string, size);

	return 0;
}

static int lua_irdaRead(lua_State *L)
{
	if (irda_fd < 0) return luaL_error(L, "irda not initialized");
	if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
	char data[256];
	int count = sceIoRead(irda_fd, &data, 256);
	if (count > 0) {
		lua_pushlstring(L, data, count);
	} else {
		lua_pushstring(L, "");
	}

	return 1;
}

// SIO Functions
static int lua_sioInit(lua_State *L)
{
	if (lua_gettop(L) != 1) return luaL_error(L, "baud rate expected.");
	int baudRate = luaL_checkint(L, 1);
	if (sio_fd < 0) sio_fd = sceIoOpen("sio:", PSP_O_RDWR, 0);
	if (sio_fd < 0) return luaL_error(L, "failed create SIO handle.");
	sceIoIoctl(sio_fd, SIO_IOCTL_SET_BAUD_RATE, &baudRate, sizeof(baudRate), NULL, 0);

	return 0;
}

static int lua_sioWrite(lua_State *L)
{
	if (sio_fd < 0) return luaL_error(L, sioNotInitialized);
	size_t size;
	const char *string = luaL_checklstring(L, 1, &size);
	if (!string) return luaL_error(L, "Argument error: System.sioWrite(string) takes a string as argument.");
	sceIoWrite(sio_fd, string, size);

	return 0;
}

static int lua_sioRead(lua_State *L)
{
	if (sio_fd < 0) return luaL_error(L, sioNotInitialized);
	if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
	char data[256];
	int count = sceIoRead(sio_fd, data, 256);
	if (count > 0) {
		lua_pushlstring(L, data, count);
	} else {
		lua_pushstring(L, "");
	}

	return 1;
}

// Does Dir/File Exist Function
static int lua_checkExist(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 1) return luaL_error(L, "This function takes one argument");
	const char *dir = luaL_checkstring(L, 1);
	if (Dir_NotExist(dir)){
	lua_pushboolean(L, FALSE);
	}else{
	lua_pushboolean(L, TRUE);
	}
	return 1;
}

// Copy File/Dir Functions
void CopiaFile(const char *path1,const char *path2)
{
    SceUID file1;
    SceUID file2;
    int readSize;
    char filebuf[0x8000];
    file1 = sceIoOpen(path1, PSP_O_RDONLY, 0777);
            file2 = sceIoOpen(path2, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
            while ((readSize = sceIoRead(file1, filebuf, 0x08000)) > 0)
            {
                sceIoWrite(file2, filebuf, readSize);
            }
            sceIoClose(file2);
            sceIoClose(file1);
}
void CopiaDir(const char* path,const char* path2)
{
int fd = sceIoDopen(path);
	char dest[256];
	if (fd < 0) {

	}
	while (sceIoDread(fd, &g_dir) > 0) {
		if (strcmp(g_dir.d_name, ".") & strcmp(g_dir.d_name, "..") & strcmp(g_dir.d_name, "")){
		if (g_dir.d_stat.st_attr & FIO_SO_IFDIR)
		{
		sprintf(dest,"%s/%s",path2,g_dir.d_name);
		sceIoMkdir(dest, 0777);
		CopiaDir(g_dir.d_name,dest);
		}else{
		sprintf(dest,"%s/%s",path2,g_dir.d_name);
		CopiaFile(g_dir.d_name,dest);
		}
		}
	}

	sceIoDclose(fd);
	rmdir(path);
}

static int lua_copyFile(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	if(!path) return luaL_error(L, "System.copyFile takes a file name as a string argument.");
	const char *path2 = luaL_checkstring(L, 2);
	if(!path2) return luaL_error(L, "System.copyFile takes a file name as a string argument.");
	CopiaFile(path,path2);

	return 1;
}
static int lua_copyDir(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	if(!path) return luaL_error(L, "System.copyFile takes a directory name as a string argument.");
	const char *path2 = luaL_checkstring(L, 2);
	if(!path2) return luaL_error(L, "System.copyFile takes a directory name as a string argument.");
	CopiaDir(path,path2);

	return 1;
}

static int lua_shutdown(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
	scePowerRequestStandby();
	return 0;
}
static int lua_standby(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
	scePowerRequestSuspend();
	return	0;
}
// PRX Load Function (Kernel Mode)
static int lua_startPRX(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	if(!path) return luaL_error(L, "System.copyFile takes a file name as a string argument.");
	SceUID modid;
	modid = kuKernelLoadModule(path, 0, NULL);
	sceKernelStartModule(modid, 0, NULL, NULL, NULL);
	return 1;
}

//Register our System Functions
static const luaL_reg System_functions[] = {
  {"startELF",						lua_startPBP},
  {"startPRX",						lua_startPRX},
  {"shutdown",						lua_shutdown},
  {"suspend",						lua_standby},
  {"setLow",						lua_setLow},
  {"setReg",						lua_setReg},
  {"setHigh",						lua_setHigh},
  {"copyFile",                		lua_copyFile},
  {"copyDir",                       lua_copyDir},
  {"doesDirExist",                	lua_checkExist},
  {"doesFileExist",                 lua_checkExist},
  {"loadlib",                		lua_loadModule},
  {"startUMDUpdate",                lua_startUMDUpdate},
  {"sioInit",                       lua_sioInit},
  {"sioRead",                       lua_sioRead},
  {"sioWrite",                      lua_sioWrite},
  {"irdaInit",                      lua_irdaInit},
  {"irdaRead",                      lua_irdaRead},
  {"irdaWrite",                     lua_irdaWrite},
  {"unassign",						lua_Unassign},
  {"assign",						lua_Assign},
  {"getModel",						lua_getModel},
  {"launchUMD",						lua_startUMD},
  {"startUpdate",					lua_startUpdate},
  {"startPBP",						lua_startPBP},
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
