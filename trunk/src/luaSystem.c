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
#include <pspwlan.h>
#include <pspopenpsid.h>
#include <psploadexec_kernel.h>
#include "include/luaplayer.h"
#include "include/kubridge.h"
#include "include/systemctrl.h"
#include "libs/graphics/graphics.h"
#include "libs/sce/msgDialog.h"
#include "libs/sce/osk.h"
#include "libs/sce/browser.h"
#include "libs/unzip/unzip.h"
#include "libs/mp4/main.h"
#include <stdio.h>

#define SIO_IOCTL_SET_BAUD_RATE 1
#define LOADMODULE_ARGERROR "Argument error: System.loadModule(module, init) takes a module name and init method as string arguments."

#define bool int
#define TRUE 1
#define FALSE 0

#define LuaCreateUserdataHandlersFix(HANDLE, DATATYPE) \
DATATYPE *to##HANDLE (lua_State *L, int index) \
{ \
  DATATYPE* handle  = (DATATYPE*)lua_touserdata(L, index); \
  if (handle == NULL) luaL_typerror(L, index, #HANDLE); \
  return handle; \
} \
DATATYPE* push##HANDLE(lua_State *L) { \
	DATATYPE* newvalue = (DATATYPE*)lua_newuserdata(L, sizeof(DATATYPE)); \
	luaL_getmetatable(L, #HANDLE); \
	lua_setmetatable(L, -2); \
	return newvalue; \
}
LuaCreateUserdataHandlersFix(Zip, Zip*)
LuaCreateUserdataHandlersFix(ZipFile, ZipFile*)

void startISO(char* file,int driver);

typedef struct
{
	unsigned long		maxclusters;
	unsigned long		freeclusters;
	int					unk1;
	unsigned int		sectorsize;
	u64					sectorcount;
	
} SystemDevCtl;

typedef struct
{
	SystemDevCtl *pdevinf;	
} SystemDevCommand;

static int usbActivated = 0;
SceUID irda_fd = -1;
static SceUID sio_fd = -1;
static const char* sioNotInitialized = "SIO not initialized.";

SceUID psploadlib( const char * name, char * init );
void **findFunction( SceUID id, const char * library, const char * name );
int init( lua_State *L);

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

void sceIoMove(char *oldPath,char *newPath) {
    SceUID oldFile; 
    SceUID newFile;    
    int readSize; 
    char filebuf[0x8000]; 
    oldFile = sceIoOpen(oldPath, PSP_O_RDONLY, 0777); 
    newFile = sceIoOpen(newPath, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777); 
    while ((readSize = sceIoRead(oldFile, filebuf, 0x08000)) > 0) { 
        sceIoWrite(newFile, filebuf, readSize);
    }
    sceIoClose(newFile);
    sceIoClose(oldFile);
    sceIoRemove(oldPath);       
}


void sceIoMvdir(char *oldPath, char *newPath){
	if (Dir_NotExist(newPath)){
    sceIoMkdir(newPath, 0777);
	}
    char *fullOldPath;
    char *fullNewPath;
    SceIoDirent oneDir;
    int oDir = sceIoDopen(oldPath);
    if (oDir < 0){
        return;
    }
    while (1){
        memset(&oneDir, 0, sizeof(SceIoDirent));
        if (sceIoDread(oDir, &oneDir) <= 0)
            break;
        if (!strcmp(oneDir.d_name, ".") || !strcmp(oneDir.d_name, ".."))
            continue;
        if (oldPath[strlen(oldPath)-1] != '/'){
            fullOldPath = (char *)calloc(strlen(oldPath)+strlen(oneDir.d_name)+2,sizeof(char));
            fullNewPath = (char *)calloc(strlen(newPath)+strlen(oneDir.d_name)+2,sizeof(char));
            sprintf(fullOldPath,"%s/%s",oldPath,oneDir.d_name);
            sprintf(fullNewPath,"%s/%s",newPath,oneDir.d_name);
        } else {
            fullOldPath = (char *)calloc(strlen(oldPath)+strlen(oneDir.d_name)+1,sizeof(char));
            fullNewPath = (char *)calloc(strlen(newPath)+strlen(oneDir.d_name)+1,sizeof(char));
            sprintf(fullOldPath,"%s%s",oldPath,oneDir.d_name);
            sprintf(fullNewPath,"%s%s",newPath,oneDir.d_name);
        }
        if (FIO_S_ISDIR(oneDir.d_stat.st_mode)){
            sceIoMvdir(fullOldPath,fullNewPath);
        } else if(FIO_S_ISREG(oneDir.d_stat.st_mode)){
            sceIoMove(fullOldPath,fullNewPath);
        }
        free(fullOldPath);
        free(fullNewPath);
    }
    sceIoDclose(oDir);
    sceIoRmdir(oldPath);
}


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

void RemoveDir(const char* oldPath)
{
    char *fullOldPath;
    SceIoDirent oneDir;
    int oDir = sceIoDopen(oldPath);
    if (oDir < 0){
        return;
    }
    while (1){
        memset(&oneDir, 0, sizeof(SceIoDirent));
        if (sceIoDread(oDir, &oneDir) <= 0)
            break;
        if (!strcmp(oneDir.d_name, ".") || !strcmp(oneDir.d_name, ".."))
            continue;
        if (oldPath[strlen(oldPath)-1] != '/'){
            fullOldPath = (char *)calloc(strlen(oldPath)+strlen(oneDir.d_name)+2,sizeof(char));
            sprintf(fullOldPath,"%s/%s",oldPath,oneDir.d_name);
        } else {
            fullOldPath = (char *)calloc(strlen(oldPath)+strlen(oneDir.d_name)+1,sizeof(char));
            sprintf(fullOldPath,"%s%s",oldPath,oneDir.d_name);
        }
        if (FIO_S_ISDIR(oneDir.d_stat.st_mode)){
            RemoveDir(fullOldPath);
        } else if(FIO_S_ISREG(oneDir.d_stat.st_mode)){
            sceIoRemove(fullOldPath);
        }
        free(fullOldPath);
    }
    sceIoDclose(oDir);
	sceIoRmdir(oldPath);
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

	sceIoMove(oldName, newName);

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
static u64 timeNowFPS;
static u64 timeLastFPS;

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

static int lua_showFPS(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 0)
	{
		return luaL_error(L, "System.getFPS() takes no arguments");
	}
	fps +=1;
	sceRtcGetCurrentTick( &timeNowFPS );

	if( ((timeNowFPS - timeLastFPS)/((float)tickResolution)) >= 1.0f )
	{
		timeLastFPS = timeNowFPS;
	}
	lua_pushnumber(L,fps);
	fps = 0;
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

// Copy/Move File/Dir Functions
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
void sceIoCpdir(char *oldPath, char *newPath){
    sceIoMkdir(newPath, 0777);
    char *fullOldPath;
    char *fullNewPath;
    SceIoDirent oneDir;
    int oDir = sceIoDopen(oldPath);
    if (oDir < 0){
        return;
    }
    while (1){
        memset(&oneDir, 0, sizeof(SceIoDirent));
        if (sceIoDread(oDir, &oneDir) <= 0)
            break;
        if (!strcmp(oneDir.d_name, ".") || !strcmp(oneDir.d_name, ".."))
            continue;
        if (oldPath[strlen(oldPath)-1] != '/'){
            fullOldPath = (char *)calloc(strlen(oldPath)+strlen(oneDir.d_name)+2,sizeof(char));
            fullNewPath = (char *)calloc(strlen(newPath)+strlen(oneDir.d_name)+2,sizeof(char));
            sprintf(fullOldPath,"%s/%s",oldPath,oneDir.d_name);
            sprintf(fullNewPath,"%s/%s",newPath,oneDir.d_name);
        } else {
            fullOldPath = (char *)calloc(strlen(oldPath)+strlen(oneDir.d_name)+1,sizeof(char));
            fullNewPath = (char *)calloc(strlen(newPath)+strlen(oneDir.d_name)+1,sizeof(char));
            sprintf(fullOldPath,"%s%s",oldPath,oneDir.d_name);
            sprintf(fullNewPath,"%s%s",newPath,oneDir.d_name);
        }
        if (FIO_S_ISDIR(oneDir.d_stat.st_mode)){
            sceIoCpdir(fullOldPath,fullNewPath);
        } else if(FIO_S_ISREG(oneDir.d_stat.st_mode)){
            CopiaFile(fullOldPath,fullNewPath);
        }
        free(fullOldPath);
        free(fullNewPath);
    }
    sceIoDclose(oDir);
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
	sceIoCpdir(path,path2);

	return 1;
}
static int lua_moveDir(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	if(!path) return luaL_error(L, "System.copyFile takes a directory name as a string argument.");
	const char *path2 = luaL_checkstring(L, 2);
	if(!path2) return luaL_error(L, "System.copyFile takes a directory name as a string argument.");
	sceIoMvdir(path,path2);

	return 1;
}

// Shutdown and Standby Functions
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

//Get Nickname,PSSID,Language & MAC Address Functions
static int lua_nickname(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
	char nickname[256];
	sceUtilityGetSystemParamString(PSP_SYSTEMPARAM_ID_STRING_NICKNAME, nickname, 128);
	lua_pushstring(L,nickname);
	return 1;
}
void SystemGetMac(unsigned char *mac)
{
	sceWlanGetEtherAddr(mac);
}
static int lua_getmac(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argument error: System.getMacAddress takes no arguments.");
		
	unsigned char mac[8];
	
	SystemGetMac(mac);
	
	char string[30];
	
	sprintf(string, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	lua_pushstring(L, string);
	
	return 1;
}
void SystemGetPsid(unsigned char *psid)
{
	PspOpenPSID thepsid;
	
	sceOpenPSIDGetOpenPSID(&thepsid);
	
	memcpy(psid, &thepsid, 16);
}
static int lua_getPSID(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
	unsigned char psid[16];
	
	SystemGetPsid(psid);
	
	char string[60];
	
	sprintf(string, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", psid[0], psid[1], psid[2], psid[3], psid[4], psid[5], psid[6], psid[7], psid[8], psid[9], psid[10], psid[11], psid[12], psid[13], psid[14], psid[15]);
	
	lua_pushstring(L, (char *)psid);
	
	return 1;
}
static int lua_getLanguage(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
	int language;
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &language);
	lua_pushnumber(L,language);
	return 1;
}

// Get Total/Free Size from Devices Functions
static int lua_getFSize(lua_State *L)
{
	if (lua_gettop(L) != 1) return luaL_error(L, "System.getFreeSize takes one argument.");
	const char *dev = luaL_checkstring(L,1);
	SystemDevCtl devctl;
	memset(&devctl, 0, sizeof(SystemDevCtl));
	SystemDevCommand command;
	command.pdevinf = &devctl;
	sceIoDevctl(dev, 0x02425818, &command, sizeof(SystemDevCommand), NULL, 0);
	u64 freesize = (devctl.freeclusters * devctl.sectorcount) * devctl.sectorsize; 
	lua_pushnumber(L,(float)freesize/1048576.0f);
	return 1;
}
static int lua_getTSize(lua_State *L)
{
	if (lua_gettop(L) != 1) return luaL_error(L, "System.getTotalSize takes one argument.");
	const char *dev = luaL_checkstring(L,1);
	SystemDevCtl devctl;
	memset(&devctl, 0, sizeof(SystemDevCtl));
	SystemDevCommand command;
	command.pdevinf = &devctl;
	sceIoDevctl(dev, 0x02425818, &command, sizeof(SystemDevCommand), NULL, 0);
	u64 size = (devctl.maxclusters * devctl.sectorcount) * devctl.sectorsize;  
	lua_pushnumber(L,(float)size/1048576.0f);
	return 1;
}

// ZIP Funcions
static int lua_ZipOpen(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: ZIP.open(filename) takes one argument.");
		
	const char *filepath = luaL_checkstring(L, 1);
	
	if(!filepath)
		return luaL_error(L, "Argument error: ZIP.open(filename), argument must be a file path.");
		
	lua_gc(L, LUA_GCCOLLECT, 0);
	
	Zip *newZip = ZipOpen(filepath);
	
	if(!newZip)
	{
		lua_pushnil(L);
		return 1;
	}
		
	Zip **luaNewZip = pushZip(L);
	
	*luaNewZip = newZip;
	
	return 1;
}

static int lua_ZipClose(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: ZIP.close(zip) takes one argument.");
	
	Zip **handle = toZip(L, 1);
	
	lua_pushboolean(L, ZipClose(*handle));
	
	return 1;
}

static int lua_ZipFileRead(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 2 && argc != 3)
		return luaL_error(L, "Argument error: ZIP.read(zip, filename, [password]) takes two or three arguments.");
		
	Zip **handle = toZip(L, 1);
	
	const char *filename = luaL_checkstring(L, 2);
	
	const char *password;
	
	if(argc == 2)
		password = NULL;
	else
		password = luaL_checkstring(L, 3);
		
	lua_gc(L, LUA_GCCOLLECT, 0);
		
	ZipFile *newZipFile = ZipFileRead(*handle, filename, password);
	
	if(!newZipFile)
	{
		lua_pushnil(L);
		return 1;
	}
	
	ZipFile **luaNewZipFile = pushZipFile(L);
	
	*luaNewZipFile = newZipFile;
	
	return 1;
}

static int lua_ZipExtract(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 2 && argc != 3)
		return luaL_error(L, "Argument error: ZIP.extract(zip, dirto, [password]) takes two or three arguments.");
	
	const char *password;
	const char *filetoextract = luaL_checkstring(L, 1);
	mkdir("ms0:/tempLPP", 0777);
	sceIoMove(filetoextract,"ms0:/tempLPP/temp.zip");
	Zip *handle = ZipOpen("ms0:/tempLPP/temp.zip");
	if(argc == 3)
		password = luaL_checkstring(L, 3);
	else
		password = NULL;
	
	int result = ZipExtract(handle, password);
	ZipClose(handle);
	const char *dir = luaL_checkstring(L, 2);
	sceIoRemove("ms0:/tempLPP/temp.zip");
	sceIoMvdir("ms0:/tempLPP",dir);
	
	lua_pushboolean(L, result);
	
	return 1;
}

static int lua_ZipFileData(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: ZIP.readFile(zipfile) takes one argument.");
	
	ZipFile *handle = *toZipFile(L, 1);
	
	lua_gc(L, LUA_GCCOLLECT, 0);
	
	lua_pushlstring(L, (char *)handle->data, handle->size);
	
	return 1;
}

static int lua_ZipFileSize(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argument error: ZIP.sizeFile(zipfile) takes one argument.");
	
	ZipFile *handle = *toZipFile(L, 1);
	
	lua_pushinteger(L, handle->size);
	
	return 1;
}

// Video Functions
static int lua_PlayMP4(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1 && argc != 2)
		return luaL_error(L, "Argument error: System.playMP4(filename) takes one or two arguments.");
	
	const char *file = luaL_checkstring(L, 1);
	int debugmode;
	if (argc == 2){
	debugmode = luaL_checkint(L, 2);
	}else{
	debugmode = 0;
	}
	PlayMp4(file,debugmode);
		
	return 1;
}

// Get CPU/BUS Speed
static int lua_getCpu(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argument error: System.getCpuSpeed takes no argument.");
	
	u32 cpu = scePowerGetCpuClockFrequency();
	lua_pushnumber(L,cpu);
	
	return 1;
}
static int lua_getBus(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argument error: System.getBusSpeed takes no argument.");
	
    u32 bus = scePowerGetBusClockFrequency();
	lua_pushnumber(L,bus);
		
	return 1;
}

// ISO Loader function
static int lua_startISO(lua_State *L)
{
pspSdkLoadStartModule("ISOLoader.prx", PSP_MEMORY_PARTITION_KERNEL); 
int argc = lua_gettop(L);
if(argc != 2)
		return luaL_error(L, "Argument error: System.startISO(filename,driver) takes two arguments.");
const char *file = luaL_checkstring(L, 1);
int driver = luaL_checkint(L, 2);
startISO(file,driver);
return 0;
}

// 
//Register our Zip Functions
static const luaL_reg Zip_functions[] = {
  {"open",							lua_ZipOpen},
  {"close",							lua_ZipClose},
  {"read",							lua_ZipFileRead},
  {"extract",						lua_ZipExtract},
  {"readFile",						lua_ZipFileData},
  {"sizeFile",						lua_ZipFileSize},
  {0, 0}
};
//Register our System Functions
static const luaL_reg System_functions[] = {
  {"startISO",						lua_startISO},
  {"getCpuSpeed",					lua_getBus},
  {"getBusSpeed",					lua_getCpu},
  {"playMP4",						lua_PlayMP4},
  {"getFreeSize",					lua_getFSize},
  {"getTotalSize",					lua_getTSize},
  {"getLanguage",					lua_getLanguage},
  {"getPSID",						lua_getPSID},
  {"getMacAddress",					lua_getmac},
  {"nickname",						lua_nickname},
  {"renameDir",						lua_moveDir},
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
  {"getFPS",						lua_showFPS},
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
	luaL_openlib(L, "ZIP", Zip_functions, 0);

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
