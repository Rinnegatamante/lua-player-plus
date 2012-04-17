/*----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#------  This File is Part Of : ----------------------------------------------------------------------------------------#
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
#- For help using LuaPlayerPlus, code help, and other please visit : http://rinnegatamante.eu/luaplayerplus/forum.php --#
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
#-----------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
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
#include <psploadexec.h>
#include <pspdisplay.h>
#include <stdarg.h>
#include <pspgu.h>
#include <pspsdk.h>

extern "C" {
#include <pspwlan.h>
#include <pspopenpsid.h>
#include <psploadexec_kernel.h>
#include <kubridge.h>
#include <systemctrl.h>
#include <pspusbdevice.h>
#include <psputility.h>
#include <oslib/browser.h>
#include <oslib/net.h>
}

#include <Mp4Decoder.h>

#include "LuaPlayer.h"
#include "Archives.h"
#include "UmdRip.h"

//Buffers
#include "mpeg_vsh370.cpp"
#include <psphttp.h>

extern OSL_SOUND **toSound(lua_State *L, int index);
extern OSL_IMAGE *toImage(lua_State *L, int arg);

#define cc(a) \
	if(a < 0) return(0x0); 

bool DirExist(const char *Directory)
{
	SceIoStat state;
	sceIoGetstat(Directory, &state);
	if (state.st_mode & FIO_S_IFDIR)
		return false;
	else
		return true;
}

void sceIoMove(char *oldPath, char * newPath)
{
	SceUID oldFile, newFile;
	int readSize = 0; char filebuf[0x8000];
	oldFile = sceIoOpen(oldPath, PSP_O_RDONLY, 0777);
	newFile = sceIoOpen(newPath, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	while((readSize == sceIoRead(oldFile, filebuf, 0x08000)) > 0)
		sceIoWrite(newFile, filebuf, readSize);
	sceIoClose(newFile);
	sceIoClose(oldFile);
	sceIoRemove(oldPath);
}

void sceIoMvdir(char *oldPath, char *newPath){
	if (!DirExist(newPath)){
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

void SystemGetPsid(unsigned char *psid)
{
	PspOpenPSID thepsid;

	sceOpenPSIDGetOpenPSID(&thepsid);

	memcpy(psid, &thepsid, 16);
}

typedef struct {
	unsigned long maxclusters;
	unsigned long freeclusters;
	int unk1;
	unsigned int sectorsize;
	u64 sectorcount;
} SystemDevCtl;

typedef struct {
	SystemDevCtl *pdevinf;
} SystemDevCommand;

typedef struct {
	int FWV;
} System;

static int usbActivated = 0;

SceUID irda_fd = -1;

static SceUID sio_fd = -1;

extern "C" {
	SceUID psploadlib(const char *name, char *init);
	void **findFunction( SceUID id, const char * library, const char * name );
	void StartISO(char *Filename, int Driver);
	u32 getBaryon(void);
	u32 getPommel(void);
	int sceSysregGetTachyonVersion();
	int LauncPops(char *Path);
}

UserdataStubs(System, System*);

static int lua_curdir(lua_State*L)
{
	int argc = lua_gettop(L);
	if(argc != 0 || argc != 1)
		return luaL_error(L, "System.currentDirectory([Path]) takes zero or one arguments.");
	if(argc == 1) {
		chdir(luaL_checkstring(L, 1));
	}
	char path[256];
	getcwd(path, 256);
	lua_pushstring(L, path);
	return(1);
}

SceIoDirent g_dir;

static int lua_dir(lua_State*L)
{
	int argc = lua_gettop(L);
	if(argc != 0 && argc != 1)
		return luaL_error(L, "System.listDirectory([path]) takes zero or one argument.");
	const char *path = "";
	if(argc == 0)
		path = "";
	else
		path = luaL_checkstring(L, 1);
	int fd = sceIoDopen(path);
	if(fd < 0) { lua_pushnil(L); return 1; }
	lua_newtable(L);
	int i = 1;
	while(sceIoDread(fd, &g_dir) > 0){
		lua_pushnumber(L, i++);
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
	}
	sceIoDclose(fd);
	return(1);
}

static int lua_createDir(lua_State*L)
{
	if(lua_gettop(L) == 0)
		return luaL_error(L, "System.createDirectory(directory) takes one argument.");
	mkdir(luaL_checkstring(L, 1), 0777);
	return(1);
}

static int lua_removeDir(lua_State*L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "System.removeDirectory(directory) takes one argument.");
	const char *path = luaL_checkstring(L, 1);
	RemoveDir(path);
	return(1);
}

static int lua_removeFile(lua_State*L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "System.removeFile(filename) takes one argument.");
	const char *path = luaL_checkstring(L, 1);
	sceIoRemove(path);
	return(1);
}

static int lua_rename(lua_State*L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "System.rename(source, destination) takes 2 arguments.");
	const char *oldName = luaL_checkstring(L, 1);
	const char *newName = luaL_checkstring(L, 2);
	sceIoRename(oldName, newName);
	return(1);
}

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
	if(lua_gettop(L) != 2)
		return luaL_error(L, "System.loadModule(module, init) takes 2 arguments.");
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
	lua_pushstring(L, (uid >= 0) ? "init" : "open");
	return(1);
}

static int lua_usbActivate(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.usbDiskModeActivate() takes no arguments.");
	if (usbActivated) return 0;
	static int modulesLoaded = 0;
	if (!modulesLoaded) {
		LoadStartModule((char*)"flash0:/km/semawm.prx");
		LoadStartModule((char*)"flash0:/km/usbstor.prx");
		LoadStartModule((char*)"flash0:/km/usbstormgr.prx");
		LoadStartModule((char*)"flash0:/km/usbstorms.prx");
		LoadStartModule((char*)"flash0:/km/usbstorboot.prx");
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
	return(1);
}

static int lua_usbDeactivate(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.usbDiskModeDeactivate() takes no arguments.");
	if (!usbActivated) return (0x0);

	sceUsbDeactivate( 0 );
	usbActivated = 0;
	return(1);
}

static int lua_powerIsPowerOnline(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.powerIsPowerOnline() takes no arguments.");
	lua_pushboolean(L, scePowerIsPowerOnline());
	return(1);
}

static int lua_powerIsBatteryExist(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.powerIsBatteryExist() takes no arguments.");
	lua_pushboolean(L, scePowerIsBatteryExist());
	return(1);
}

static int lua_powerIsBatteryCharging(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.powerIsBatteryCharging() takes no arguments.");
	lua_pushboolean(L, scePowerIsBatteryCharging());
	return(1);
}

static int lua_powerGetBatteryChargingStatus(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.powerGetBatteryChargingStatus() takes no arguments.");
	lua_pushnumber(L, scePowerGetBatteryChargingStatus());
	return(1);
}

static int lua_powerIsLowBattery(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.powerIsLowBattery() takes no arguments.");
	lua_pushboolean(L, scePowerIsLowBattery());
	return(1);
}

static int lua_powerGetBatteryLifePercent(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.powerGetBatteryLifePercent() takes no arguments.");
	lua_pushnumber(L, scePowerGetBatteryLifePercent());
	return(1);
}

static int lua_powerGetBatteryLifeTime(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.powerGetBatteryLifeTime() takes no arguments.");
	lua_pushnumber(L, scePowerGetBatteryLifeTime());
	return(1);
}

static int lua_powerGetBatteryTemp(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.powerGetBatteryTemp() takes no arguments.");
	lua_pushnumber(L, scePowerGetBatteryTemp());
	return(1);
}

static int lua_powerGetBatteryVolt(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.powerGetBatteryVolt() takes no arguments.");
	lua_pushnumber(L, scePowerGetBatteryVolt());
	return(1);
}

static int lua_powerTick(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.powerTick() takes no arguments.");
	scePowerTick(0);
	return(1);
}

static int lua_md5sum(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "System.md5sum(string) takes one arguments.");
	size_t size;
	const char *string = luaL_checklstring(L, 1, &size);
	u8 digest[16];
	sceKernelUtilsMd5Digest((u8*)string, size, digest);
	int i;
	char result[33];
	for (i = 0; i < 16; i++) sprintf(result + 2 * i, "%02x", digest[i]);
	lua_pushstring(L, result);
	return(1);
}

static int lua_sleep(lua_State *L)
{
	if (lua_gettop(L) != 1)
		return luaL_error(L, "System.sleep(millisenconds) takes one argument.");
	int milliseconds = luaL_checkint(L, 1);
	sceKernelDelayThread(milliseconds * 1000);
	return(1);
}

static int lua_getFreeMemory(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.getFreeMemory() takes no arguments.");
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
	return(1);
}

static u64 timeNow;
static u64 timeLastAsk;
static u32 tickResolution;

static int fps = 0;
static u64 timeNowFPS;
static u64 timeLastFPS;

void initTimer()
{
	sceRtcGetCurrentTick( &timeLastAsk );
	sceRtcGetCurrentTick( &timeLastFPS );
	tickResolution = sceRtcGetTickResolution();
}

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
		return luaL_error(L, "System.getFPS() takes no arguments.");
	fps +=1;
	sceRtcGetCurrentTick( &timeNowFPS );

	if( ((timeNowFPS - timeLastFPS)/((float)tickResolution)) >= 1.0f )
	{
		timeLastFPS = timeNowFPS;
	}
	lua_pushnumber(L,fps);
	fps = 0;
	return(1);
}

static int lua_setLow(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.setLow() takes no arguments.");
	if (scePowerGetCpuClockFrequencyInt() != 100){
		scePowerSetCpuClockFrequency(100);
		scePowerSetBusClockFrequency(50);
	}
	return(1);
}

static int lua_setReg(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.setReg() takes no arguments.");
	if (scePowerGetCpuClockFrequencyInt() != 222){
		scePowerSetCpuClockFrequency(222);
		scePowerSetBusClockFrequency(111);
	}
	return(1);
}

static int lua_setHigh(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.setHight() takes no arguments.");
	if (scePowerGetCpuClockFrequencyInt() != 333){
		scePowerSetClockFrequency(333,333,166);
	}
	return 0;
}

static int lua_setCpuSpeed(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "System.setCpuSpeed(speed) takes one arguments.");
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
	return(1);
}

static int lua_systemQuit(lua_State *L)
{
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.quit() takes no arguments.");
	sceKernelExitGame();
	return(0x0);
}

static int lua_SCEShowMessageDialog(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc > 2 || argc == 0)
		return luaL_error(L, "System.msgDialog(text, [EnableYesNo]) takes a 1 or 2 arguments.");
	int skip = 0, initialize = 0, dialog = OSL_DIALOG_NONE;
	const char *Message = luaL_checkstring(L, 1);
	int Buttons = (argc == 2) ? luaL_checknumber(L, 2) : 1;
	while(!osl_quit) {
		if(!skip) {
			oslStartDrawing();
			if(dialog)
			{
				oslDrawDialog();
				if(oslGetDialogStatus() == PSP_UTILITY_DIALOG_NONE)
				{
						if(oslDialogGetResult() == OSL_DIALOG_CANCEL) { lua_pushnumber(L, 0); break; }
						else if(dialog == OSL_DIALOG_MESSAGE)
						{
							int button = oslGetDialogButtonPressed();
							if(button == PSP_UTILITY_MSGDIALOG_RESULT_YES) { lua_pushnumber(L, 1); break; }
							if(button == PSP_UTILITY_MSGDIALOG_RESULT_NO)  { lua_pushnumber(L, 2); break; }
						}
						oslEndDialog();
				}
				}
				oslEndDrawing();
			}

			if(dialog == OSL_DIALOG_NONE)
			{
				if(initialize == 0)
				{
					oslInitMessageDialog(Message, Buttons);
					initialize = 1;
				}
			}
			oslEndFrame();
			dialog = oslGetDialogType();
			skip = oslSyncFrame();
		}
	return(1);
}

static int lua_SCEOsk(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc > 2 || argc == 0)
       return luaL_error(L, "System.osk(Message, [InitialText]) takes 1 or 2 arguments.");
    int skip = 0, initialize = 0;
    char iText[256] = "";
    const char *Message = luaL_checkstring(L, 1);
    const char *InitialText = (argc == 2) ? luaL_checkstring(L, 2) : "";
    while(!osl_quit) {
        if(!skip)
        {
            oslStartDrawing();
            if(oslOskIsActive()) {
            oslDrawOsk();
            if(oslGetOskStatus() == PSP_UTILITY_DIALOG_NONE) {
                if(oslGetOskStatus() == OSL_OSK_CANCEL) { lua_pushstring(L, ""); return(1); }
            else {
                char userText[256] = "";
                oslOskGetText(userText);
                sprintf(iText, "%s", userText);
            }
            oslEndOsk();
            break;
            }
            }
            oslEndDrawing();
        }

        if(!oslOskIsActive()) {
            if(initialize == 0) {
                oslInitOsk((char*)Message, (char*)InitialText, 256, 20);
                memset((void*)Message, 0, sizeof(Message));
                initialize = 1;
            }
        }
        oslEndFrame();
        skip = oslSyncFrame();
    }
    lua_pushstring(L, iText);
	return(1);
}

static int lua_startPBP(lua_State *L)
{
	if(lua_gettop(L) > 0)
		return luaL_error(L, "System.startPBP() takes one argument.");
	const char *file = luaL_checkstring(L, 1);
	if (!DirExist(file))
	{
		return luaL_error(L, "File doesn't exist");
	}else{
		EseguiPBP(file);
	}
	return 1;
}

static int lua_startUpdate(lua_State *L)
{
	const char *file = luaL_checkstring(L, 1);
	int argc = lua_gettop(L);
	if (argc > 1)
	{
		return luaL_error(L, "System.startUpdate() takes one or none argument.");
	}
	if (!DirExist(file))
	{
		return luaL_error(L, "File doesn't exist");
	}else{
		if (argc == 1)
		{
			EseguiUpdate(file);
		}else{
			if (!DirExist("ms0:/PSP/GAME/UPDATE/EBOOT.PBP"))
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
	return(1);
}

static int lua_startUMD(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 0)
	{
		return luaL_error(L, "System.launchUMD() takes no arguments.");
	}
	if (!DirExist("disc0:/PSP_GAME")){
		return luaL_error(L, "Error: No UMD inserted");
	}else{
		if (!DirExist("disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"))
		{
			sctrlKernelLoadExecVSHDisc("disc0:/PSP_GAME/SYSDIR/BOOT.BIN", NULL);
		}else{
			sctrlKernelLoadExecVSHDisc("disc0:/PSP_GAME/SYSDIR/EBOOT.BIN", NULL);
		}
	}
	return(1);
}

static int lua_startUMDUpdate(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 0)
	{
		return luaL_error(L, "System.startUMDUpdate() takes no arguments.");
	}
	if (!DirExist("disc0:/PSP_GAME")){
		return luaL_error(L, "Error in System.startUMDUpdate(): No UMD inserted.");
	}else{
		sctrlKernelLoadExecVSHDiscUpdater("disc0:/PSP_GAME/SYSDIR/UPDATE/EBOOT.BIN", NULL);
	}
	return(1);
}

static int lua_getModel(lua_State *L)
{
	char stringa[256];
	int argc = lua_gettop(L);
	if (argc > 0)
	{
		return luaL_error(L, "System.getModel() takes no arguments.");
	}
	int model;
	model = ((kuKernelGetModel() + 1) * 1000);
	if (model == 4000){
		sprintf(stringa, "N1000");
	}else{
		sprintf(stringa, "%i", model);
	}
	lua_pushstring(L, stringa);
	return(1);
}

static int lua_Unassign(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "System.unassign(device) takes one argument.");
	int uas1;
	char temp[40];
	const char *device = luaL_checkstring(L, 1);
	uas1 = sceIoUnassign(device);
	if(uas1 < 0)
	{
		sprintf(temp,"Failed to unassign \'%s\'",temp);
		return luaL_error(L,temp);
	}
	return(1);
}

static int lua_Assign(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "System.assign(device) takes one argument.");
	int as1;
	char temp[40];
	const char *dev1=luaL_checkstring(L, 1), *dev2=luaL_checkstring(L, 2), *dev3=luaL_checkstring(L, 3);
	as1  = sceIoAssign(dev1, dev2, dev3, IOASSIGN_RDWR, NULL, 0);
	if(as1 < 0)
	{
		sprintf(temp,"Failed to assign \'%s\'",dev1);
		return luaL_error(L,temp);
	}
	return(1);
}

static int lua_irdaInit(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.irdaInit() takes no arguments.");
	if (irda_fd < 0) irda_fd = sceIoOpen("irda0:", PSP_O_RDWR, 0);
	if (irda_fd < 0) return luaL_error(L, "failed create IRDA handle.");
	return(1);
}

static int lua_irdaWrite(lua_State *L)
{
	if (irda_fd < 0)
		return luaL_error(L, "Irda not initialized.");
	size_t size;
	if(lua_gettop(L) != 1)
		return luaL_error(L, "System.irdaWrite(string) takes 1 argument.");
	const char *string = luaL_checklstring(L, 1, &size);
	sceIoWrite(irda_fd, string, size);
	return(1);
}

static int lua_irdaRead(lua_State *L)
{
	if (irda_fd < 0) return luaL_error(L, "irda not initialized");
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.irdaRead() takes no arguments.");
	char data[256];
	int count = sceIoRead(irda_fd, &data, 256);
	if (count > 0) {
		lua_pushlstring(L, data, count);
	} else {
		lua_pushstring(L, "");
	}
	return(1);
}

static int lua_sioInit(lua_State *L)
{
	if (lua_gettop(L) != 1)
		return luaL_error(L, "System.sioInit(baudRate) takes one argument.");
	int baudRate = luaL_checkint(L, 1);
	if (sio_fd < 0) sio_fd = sceIoOpen("sio:", PSP_O_RDWR, 0);
	if (sio_fd < 0) return luaL_error(L, "failed create SIO handle.");
	sceIoIoctl(sio_fd,1, &baudRate, sizeof(baudRate), NULL, 0);
	return(1);
}

static int lua_sioWrite(lua_State *L)
{
	if (sio_fd < 0)
		return luaL_error(L, "Sio not Initialized.");
	size_t size;
	const char *string = luaL_checklstring(L, 1, &size);
	if (!string) return luaL_error(L, "Argument error: System.sioWrite(string) takes a string as argument.");
	sceIoWrite(sio_fd, string, size);
	return(1);
}

static int lua_sioRead(lua_State *L)
{
	if (sio_fd < 0)
		return luaL_error(L,"Sio not Initialized.");
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.sioRead() takes no arguments.");
	char data[256];
	int count = sceIoRead(sio_fd, data, 256);
	if (count > 0) {
		lua_pushlstring(L, data, count);
	} else {
		lua_pushstring(L, "");
	}
	return(1);
}

static int lua_checkExist(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "System.doesFileExist(filename) and System.doesDirExist(dir) take one argument.");
	const char *dir = luaL_checkstring(L, 1);
	lua_pushboolean(L, DirExist(dir));
	return(1);
}

static int lua_copyFile(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "System.copyFile(source, dest) takes 2 arguments.");
	const char *path = luaL_checkstring(L, 1);
	const char *path2 = luaL_checkstring(L, 2);
	CopiaFile(path,path2);
	return(1);
}


static int lua_copyDir(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "System.copyDir(source, dest) takes 2 arguments.");
	const char *path = luaL_checkstring(L, 1);
	const char *path2 = luaL_checkstring(L, 2);
	sceIoCpdir((char*)path,(char*)path2);
	return(1);
}


static int lua_moveDir(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "System.moveDir(source, dest) takes 2 arguments.");
	const char *path = luaL_checkstring(L, 1);
	const char *path2 = luaL_checkstring(L, 2);
	sceIoMvdir((char*)path,(char*)path2);
	return(1);
}

static int lua_shutdown(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.shutdown() takes no arguments.");
	scePowerRequestStandby();
	return(0x0);
}

static int lua_standby(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.standby() takes no arguments.");
	scePowerRequestSuspend();
	return(0x0);
}

static int lua_startPRX(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "System.startPrx(filename) takes 1 argument.");
	const char *path = luaL_checkstring(L, 1);
	SceUID modid;
	modid = kuKernelLoadModule(path, 0, NULL);
	sceKernelStartModule(modid, 0, NULL, NULL, NULL);
	return(1);
}

static int lua_nickname(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "Stystem.nickname() takes no arguments.");
	char nickname[256];
	sceUtilityGetSystemParamString(PSP_SYSTEMPARAM_ID_STRING_NICKNAME, nickname, 128);
	lua_pushstring(L,nickname);
	return(1);
}

static int lua_getmac(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0)
		return luaL_error(L, "System.getMacAddress() takes no arguments.");
	unsigned char mac[8];
	sceWlanGetEtherAddr(mac);
	char string[30];
	sprintf(string, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	lua_pushstring(L, string);
	return 1;
}

static int lua_getPSID(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.getPSID() takes no arguments.");
	unsigned char psid[16];
	SystemGetPsid(psid);
	char string[60];
	sprintf(string, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", psid[0], psid[1], psid[2], psid[3], psid[4], psid[5], psid[6], psid[7], psid[8], psid[9], psid[10], psid[11], psid[12], psid[13], psid[14], psid[15]);
	lua_pushstring(L, (char *)psid);
	return 1;
}

static int lua_getLanguage(lua_State *L)
{
	if (lua_gettop(L) != 0)
		return luaL_error(L, "System.getLanguage() takes no arguments.");
	int language;
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &language);
	lua_pushnumber(L,language);
	return 1;
}

static int lua_getFSize(lua_State *L)
{
	if (lua_gettop(L) != 1)
		return luaL_error(L, "System.getFreeSize(device) takes one argument.");
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
	if (lua_gettop(L) != 1)
		return luaL_error(L, "System.getTotalSize(device) takes one argument.");
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

static int lua_getCpu(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0)
		return luaL_error(L, "System.getCpuSpeed() takes no arguments.");
	u32 cpu = scePowerGetCpuClockFrequency();
	lua_pushnumber(L,cpu);
	return 1;
}

static int lua_getBus(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0)
		return luaL_error(L, "System.getBusSpeed() takes no arguments.");
	u32 bus = scePowerGetBusClockFrequency();
	lua_pushnumber(L,bus);
	return 1;
}

static int lua_usbdevUMD(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.usbDevUMD() takes no arguments.");
	if(!DirExist("disc0:/PSP_GAME"))
		return luaL_error(L, "No UMD Disk Present!");
	pspSdkLoadStartModule("flas0:/kd/usbdevice.prx", PSP_MEMORY_PARTITION_KERNEL);
	pspUsbDeviceSetDevice(PSP_USBDEVICE_UMD9660, 0, 0);
	return(1);
}

static int lua_checkUMD(lua_State *L)
{
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.checkUMD() takes no arguments.");
	lua_pushboolean(L, (DirExist("disc0:/PSP_GAME")) ? 1 : 0);
	return(1);
}

static int lua_GetTime(lua_State *L) {
	if(lua_gettop(L) != 1)
		return luaL_error(L, "System.getTime(param) takes one argument.");
	int Param = luaL_checknumber(L, 1);
	if(Param > 3 || Param == 0)
		return luaL_error(L, "Invalid argument #1 in 'System.getTime()'. '%d' is not a valid value.", Param);
	pspTime psptime;
	sceRtcGetCurrentClockLocalTime(&psptime);
	lua_pushnumber(L, (Param == 1) ? psptime.hour : (Param == 2) ? psptime.minutes : psptime.seconds);
	return(1);
}

static int lua_GetDate(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "System.getDate(param) takes one argument.");
	int Param = luaL_checknumber(L, 1);
	if(Param > 3 || Param == 0)
		return luaL_error(L, "Invalid argument #1 in 'System.getDate()'. '%d' is not a valid value.", Param);
	pspTime psptime;
	sceRtcGetCurrentClockLocalTime(&psptime);
	lua_pushnumber(L, (Param == 1) ? psptime.day : (Param == 2) ? psptime.month : psptime.year);
	return(1);
}

static int lua_startISO(lua_State*L)
{
	int argc = lua_gettop(L);
	if(argc > 2)
		return luaL_error(L, "System.startISO(File, [Driver]) takes 1 or 2 arguments.");
	const char *isoPath = luaL_checkstring(L, 1);
	int Driver = (argc == 2) ? luaL_checknumber(L, 2) : 3;
	StartISO((char*)isoPath, Driver);
	return(1);
}

static int lua_RarExtract(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2 && argc != 3)
		return luaL_error(L, "Rar.extract(RarFile, DirTo, [Password]) takes 2 or 3 arguments.");
	const char *Archive = luaL_checkstring(L, 1);
	const char *DirTo = luaL_checkstring(L, 2);
	const char *Password = (argc == 3) ? luaL_checkstring(L, 3) : NULL;
	int ret = rarExtract(Archive, DirTo, Password);
	lua_pushboolean(L, ret);
	return(1);
}

static int lua_usbdevflash3(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.usbDevFlash3() takes no arguments.");
	pspSdkLoadStartModule("flash0:/kd/usbdevice.prx", PSP_MEMORY_PARTITION_KERNEL);
	pspUsbDeviceSetDevice(PSP_USBDEVICE_FLASH3, 0, 0);
	return(1);
}

static int lua_usbdevflash2(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.usbDevFlash2() takes no arguments.");
	pspSdkLoadStartModule("flash0:/kd/usbdevice.prx", PSP_MEMORY_PARTITION_KERNEL);
	pspUsbDeviceSetDevice(PSP_USBDEVICE_FLASH2, 0, 0);
	return(1);
}

static int lua_usbdeflash1(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.usbDevFlash1() takes no arguments.");
	pspSdkLoadStartModule("flash0:/kd/usbdevice.prx", PSP_MEMORY_PARTITION_KERNEL);
	pspUsbDeviceSetDevice(PSP_USBDEVICE_FLASH0, 0, 0);
	return(1);
}

static int lua_usbdevflash0(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.usbDevFlash0() takes no arguments.");
	pspSdkLoadStartModule("flash0:/kd/usbdevice.prx", PSP_MEMORY_PARTITION_KERNEL);
	pspUsbDeviceSetDevice(PSP_USBDEVICE_FLASH0, 0, 0);
	return(1);
}

static int lua_CfwVersion(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.getVersion() takes no arguments.");
	int ver = sceKernelDevkitVersion();
	char *aStr; char buffer[16];
	sprintf(buffer, "%x", ver);
	buffer[1] = '.'; buffer[3] = buffer[4]; buffer[4] = 0;
	aStr = buffer;
	lua_pushstring(L, aStr);
	return(1);
}

static int lua_mobo(lua_State*L)
{
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "System.getMotherboard() takes no arguments.");
	unsigned int tachyon = sceSysregGetTachyonVersion();
	unsigned int baryon = getBaryon();
	unsigned int pommel = getPommel();
	char stringaz[256];
	sprintf(stringaz, "%08x%08x%08x", tachyon, baryon, pommel);
	char stringaz2[256];
	sprintf(stringaz2, "%08x%08x", tachyon, baryon);
	lua_pushstring(L,"Unknown");
	// PSP 1000
	if (strstr(stringaz2,"0014000000030600")){
		lua_pushstring(L,"TA-079 v1 (1G)");
		return 1;
	}
	if (strstr(stringaz2,"0020000000030600")){
		lua_pushstring(L,"TA-079 v2 (1G)");
		return 1;
	}
	if (strstr(stringaz2,"0020000000040600")){
		lua_pushstring(L,"TA-079 v3 (1G)");
		return 1;
	}
	if (strstr(stringaz2,"0030000000040600")){
		lua_pushstring(L,"TA-081 (1G)");
		return 1;
	}
	if (strstr(stringaz2,"0040000000114000")){
		lua_pushstring(L,"TA-082 (1G)");
		return 1;
	}
	if (strstr(stringaz2,"0040000000121000")){
		lua_pushstring(L,"TA-086 (1G)");
		return 1;
	}
	// PSP 2000
	if (strstr(stringaz2,"005000000022b200")){
		lua_pushstring(L,"TA-085 v1 (2G)");
		return 1;
	}
	if (strstr(stringaz2,"0050000000234000")){
		lua_pushstring(L,"TA-085 v2 (2G)");
		return 1;
	}
	if (strstr(stringaz,"005000000024300000000123")){
		lua_pushstring(L,"TA-088 v1/v2 (2G)");
		return 1;
	}
	if (strstr(stringaz2,"0060000000243000")){
		lua_pushstring(L,"TA-088 v3 (2G)");
		return 1;
	}
	if (strstr(stringaz,"005000000024300000000132")){
		lua_pushstring(L,"TA-090 v1 (2G)");
		return 1;
	}
	// PSP 3000
	if (strstr(stringaz2,"0060000000263100")){
		lua_pushstring(L,"TA-090 v2 (3G)");
		return 1;
	}
	if (strstr(stringaz2,"0060000000285000")){
		lua_pushstring(L,"TA-092 (3G)");
		return 1;
	}
	if (strstr(stringaz2,"00810000002c4000")){
		lua_pushstring(L,"TA-093 (4G)");
		return 1;
	}
	if (strstr(stringaz2,"00810000002e4000")){
		lua_pushstring(L,"N/A (6G)");
		return 1;
	}
	if (strstr(stringaz2,"0072000000304000")){
		if (kuKernelGetModel() == 3){
			lua_pushstring(L,"TA-091 (5G)");
		}else{
			lua_pushstring(L,"TA-095 (9G)");
		}
		return 1;
	}
	// PSP N1000
	if (strstr(stringaz2,"00800000002a0000")){
		lua_pushstring(L,"TA-094 (5G)");
		return 1;
	}
	return 1;
}

static int lua_baryon(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.getBaryon() takes no arguments.");
	u32 baryon = getBaryon();
	char stringa3[256];
	sprintf(stringa3, "%08x", (unsigned int)baryon);
	for(int i = 0; stringa3[i]; i++)
		stringa3[i] = toupper(stringa3[i]);
	char suffix[10] = "0x";
	strcat(suffix, stringa3);
	lua_pushstring(L, suffix);
	return(1);
}

static int lua_pommel(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.getPommel() takes no arguments.");
	u32 pommel = getPommel();
	char stringa[256];
	sprintf(stringa, "0x%08x", (unsigned int)pommel);
	lua_pushstring(L, stringa);
	return(1);
}

static int lua_tachyon(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.getTachyon() takes no arguments.");
	unsigned int tachyon = sceSysregGetTachyonVersion();
	char stringa2[256];
	sprintf(stringa2, "0x%08x", tachyon);
	lua_pushstring(L, stringa2);
	return(1);
}

static int lua_wait(lua_State *L) {
	if(lua_gettop(L) != 1)
		return luaL_error(L, "System.wait(milliseconds) takes 1 argument.");
	sceKernelDelayThread(luaL_checknumber(L, 1));
	return(1);
}

static int lua_startPSX(lua_State *L) {
	if(lua_gettop(L) != 1)
		return luaL_error(L, "System.startPSX(filename) takes 1 argument.");
	LauncPops((char*)luaL_checkstring(L, 1));
	return(1);
}

static int lua_PlayMp4(lua_State *L) {
	int argc = lua_gettop(L);
	if(argc > 3 || argc == 0)
		return luaL_error(L, "System.playMp4(filename, [StopButton], [AudioFile]) takes a maximum of 3 arguments.");
	int InitResult = 0;
	if(!ClMp4Decoder::initialized) {
		if(!DirExist("./mpeg_vsh370.prx")) {
			FILE * F = fopen("./mpeg_vsh370.prx", "wb");
			if(F == NULL) return luaL_error(L, "Cannot write 'mpeg_vsh370.prx'.");
			fwrite(mpeg_vsh370, 1, size_mpeg_vsh370, F);
			fclose(F);
		}
		InitResult = ClMp4Decoder::init("/mpeg_vsh370.prx");
		if(InitResult != 0)
			return luaL_error(L, "Cannot load the module necessary for mp4 playing.");
	}
	if(argc == 3) {
		OSL_SOUND *s = *toSound(L, 3);
		oslSetSoundLoop(s, 0);
		oslPlaySound(s, 0);
		int ret = ClMp4Decoder::playMp4(luaL_checkstring(L, 1), (argc >= 2) ? luaL_checkint(L, 2) : NULL, PSP_DISPLAY_PIXEL_FORMAT_8888);
		oslStopSound(s);
		return(ret);
	}
	return ClMp4Decoder::playMp4(luaL_checkstring(L, 1), (argc >= 2) ? luaL_checkint(L, 2) : NULL, PSP_DISPLAY_PIXEL_FORMAT_8888);
}

static int lua_WebBrowser(lua_State *L) {
    int argc = lua_gettop(L);
    if(argc != 2)
       return luaL_error(L, "System.webbrowser(url, path) takes 1 argument.");
	const char *Url = luaL_checkstring(L, 1);
	const char *Path = luaL_checkstring(L, 2);
	int skip = 0, browser = 0;
	oslNetInit();
	while(!osl_quit) {
		browser = oslBrowserIsActive();
		if(!skip) {
			oslStartDrawing();
			if(browser) {
				oslDrawBrowser();
				if(oslGetBrowserStatus() == PSP_UTILITY_DIALOG_NONE) { oslEndBrowser(); break; }
			}
			oslEndDrawing();
		}
		oslEndFrame();
		skip = oslSyncFrame();

		if(!browser) {
			oslBrowserInit((char*)Url, (char*)Path, 5*1024*1024,
				PSP_UTILITY_HTMLVIEWER_DISPLAYMODE_SMART_FIT,
				PSP_UTILITY_HTMLVIEWER_DISABLE_STARTUP_LIMITS,
				PSP_UTILITY_HTMLVIEWER_INTERFACEMODE_FULL,
				PSP_UTILITY_HTMLVIEWER_CONNECTMODE_MANUAL_ALL);
		}
	}
	oslNetTerm();
    return(1);
}

UMD *umd = new UMD();

static int lua_GetUmdTitle(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.getUmdTitle() takes no arguments.");
	const char *RetIn = umd->Init();
	if(RetIn != NULL) {
		lua_pushstring(L, RetIn);
		return(0x0);
	}
	umd->GetTitle();
	lua_pushstring(L, umd->Title);
	return(1);
}

static int lua_GetUmdID(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.getUmdID() takes no arguments.");
	const char *RetIn = umd->Init();
	if(RetIn != NULL) {
		lua_pushstring(L, RetIn);
		return(0x0);
	}
	lua_pushstring(L, umd->ID);
	return(1);
}

static int lua_GetUmdSize(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "System.getUmdSize() takes no arguments.");
	const char *RetIn = umd->Init();
	if(RetIn != NULL) {
		lua_pushstring(L, RetIn);
		lua_pushnumber(L, 0);
		return(0x0);
	}
	lua_pushnumber(L, umd->GetSize());
	return(1);
}

static int lua_RipUmd(lua_State *L) {
	if(lua_gettop(L) != 1)
		return luaL_error(L, "System.ripUmd(Outpath) takes one argument.");
	const char *RetIn = umd->Init();
	if(RetIn != NULL) {
		lua_pushstring(L, RetIn);
		lua_pushboolean(L, 0);
		return(0x0);
	}
	unsigned int blocks = umd->GetSize();
	const char *Ret = umd->DumpISO((char*)luaL_checkstring(L, 1), blocks);
	if(Ret != NULL){
		lua_pushstring(L, Ret);
		lua_pushboolean(L, 0);
		return(0x0);
	}
	lua_pushstring(L, "");
	lua_pushboolean(L, 1);
	return(1);
}

typedef struct {
	char signature[4];
	int version;
	int offset[8];
} EbootHeader;

static int lua_SaveData(lua_State *L) {
	int argc = lua_gettop(L);
	if(argc != 4 )
		return luaL_error(L, "System.saveData(Data, Title, Id, SaveName)");
	char nameList[][20] = {"0000","0001","0002",""};
	int skip = 0;
	struct oslSaveLoad saveLoadData;
	int Type = OSL_DIALOG_NONE;

	unsigned char *ICON0Data = NULL, *PIC1Data = NULL;
	int ICON0Data_Size = 0, PIC1Data_Size = 0;

	oslClearScreen(RGB(0, 0, 0));
	while(!osl_quit)
	{
		if (!skip)
		{
			oslStartDrawing();
			if (Type != OSL_DIALOG_NONE) {
				oslDrawSaveLoad();
				if (oslGetLoadSaveStatus() == PSP_UTILITY_DIALOG_NONE) {
					if (oslSaveLoadGetResult() == OSL_SAVELOAD_CANCEL){
						lua_pushboolean(L, 0);
						break;
					}else if (Type == OSL_DIALOG_SAVE) {
						lua_pushboolean(L, 1);
						break;
					}
					oslEndSaveLoadDialog();
				}
				oslEndDrawing();
			}
		}

		if (Type == OSL_DIALOG_NONE) {
			memset(&saveLoadData, 0, sizeof(saveLoadData));
			strcpy(saveLoadData.gameTitle, luaL_checkstring(L, 2));
			strcpy(saveLoadData.gameID, luaL_checkstring(L, 3));
			strcpy(saveLoadData.saveName, luaL_checkstring(L, 4));
			saveLoadData.nameList = nameList;
			saveLoadData.data = (char*)luaL_checkstring(L, 1);
			saveLoadData.dataSize = 256;
			if(argc == 5) {
				EbootHeader ebootHeader;
				SceUID fd;
				int ebootlenght, filesize;
				fd = sceIoOpen("EBOOT.PBP", PSP_O_RDONLY, 0777);
				ebootlenght = sceIoLseek32(fd, 0, PSP_SEEK_END);
				sceIoRead(fd, &ebootHeader, sizeof(ebootHeader));
				filesize = ebootHeader.offset[1 + 1] - ebootHeader.offset[1];
				if(filesize > 0)
				{
					sceIoLseek32(fd, ebootHeader.offset[1], PSP_SEEK_SET);
					sceIoRead(fd, ICON0Data, filesize);
					ICON0Data_Size = filesize;
				}

				filesize = ebootHeader.offset[4 + 1] - ebootHeader.offset[4];
				if(filesize >  0)
				{
					sceIoLseek32(fd, ebootHeader.offset[4], PSP_SEEK_SET);
					sceIoRead(fd, PIC1Data, filesize);
					PIC1Data_Size = filesize;
				}
				sceIoClose(fd);
			}
			saveLoadData.icon0 = (unsigned char*)ICON0Data;
			saveLoadData.size_icon0 = ICON0Data_Size;
			saveLoadData.pic1 = (unsigned char*)PIC1Data;
			saveLoadData.size_pic1 = PIC1Data_Size;
			oslInitSaveDialog(&saveLoadData);
		}
		oslEndFrame();
		Type = oslGetSaveLoadType();
		skip = oslSyncFrame();
	}
	return(1);
}

static int lua_LoadData(lua_State *L) {
	int argc = lua_gettop(L);
	if(argc == 0 || argc > 5 )
		return luaL_error(L, "System.loadData(Id, SaveName)");
	char nameList[][20] = {"0000","0001","0002",""};
	int skip = 0;
	struct oslSaveLoad saveLoadData;
	int Type = OSL_DIALOG_NONE;
	char Data[256] = "";

	oslClearScreen(RGB(0, 0, 0));
	while(!osl_quit)
	{
		if (!skip)
		{
			oslStartDrawing();
			if (Type != OSL_DIALOG_NONE) {
				oslDrawSaveLoad();
				if (oslGetLoadSaveStatus() == PSP_UTILITY_DIALOG_NONE) {
					if (oslSaveLoadGetResult() == OSL_SAVELOAD_CANCEL){
						lua_pushboolean(L, 0);
						break;
					}else if (Type == OSL_DIALOG_LOAD) {
						lua_pushboolean(L, 1);
						break;
					}
					oslEndSaveLoadDialog();
				}
				oslEndDrawing();
			}
		}

		if (Type == OSL_DIALOG_NONE) {
			memset(&saveLoadData, 0, sizeof(saveLoadData));
			strcpy(saveLoadData.gameID, luaL_checkstring(L, 1));
			strcpy(saveLoadData.saveName, luaL_checkstring(L, 2));
			saveLoadData.nameList = nameList;
			saveLoadData.data = Data;
			saveLoadData.dataSize = sizeof(Data);
			oslInitLoadDialog(&saveLoadData);
		}
		oslEndFrame();
		Type = oslGetSaveLoadType();
		skip = oslSyncFrame();
	}
	lua_pushstring(L, Data);
	return(1);
}

/* Don't Work */
static int lua_DownloadFile(lua_State *L) {
	if(lua_gettop(L) != 2)
		return luaL_error(L, "System.download(Url, DownloadPath) takes 2 arguments.");
	oslNetInit();
	const char *Url = luaL_checkstring(L, 1);
	const char *DownloadPath = luaL_checkstring(L, 2);
	lua_pushboolean(L, oslNetGetFile(Url, DownloadPath));
	oslNetTerm();
	return(1);
}

static int lua_GetFileSize(lua_State *L) {
	if(lua_gettop(L) != 1)
		return luaL_error(L, "System.getFileSize(filename) takes 1 argument.");
	FILE *f = fopen(luaL_checkstring(L, 1), "rb");
	if(f ==  NULL) { lua_pushnumber(L, 0); return(1); }
	fseek(f, 0, SEEK_END);
	unsigned int size = ftell(f);
	fclose(f);
	lua_pushnumber(L, size);
	return(1);
}

static const luaL_reg System_functions[] = {
	{"getFileSize", lua_GetFileSize},
	{"download", lua_DownloadFile},
    {"loadData", lua_LoadData},
    {"saveData", lua_SaveData},
    {"ripUmd", lua_RipUmd},
    {"getUmdSize", lua_GetUmdSize},
    {"getUmdID", lua_GetUmdID},
    {"getUmdTitle", lua_GetUmdTitle},
    {"webbrowser", lua_WebBrowser},
	{"playMp4", lua_PlayMp4},
	{"startPSX", lua_startPSX},
	{"wait", lua_wait},
	{"getTachyon", lua_tachyon},
	{"getPommel", lua_pommel},
	{"getBaryon", lua_baryon},
	{"getMotherboard", lua_mobo},
	{"getVersion", lua_CfwVersion},
	{"usbDevFlash0", lua_usbdevflash0},
	{"usbDevFlash1", lua_usbdeflash1},
	{"usbDevFlash2", lua_usbdevflash2},
	{"usbDevFlash3", lua_usbdevflash3},
	{"usbDevUMD", lua_usbdevUMD},
	{"checkUMD", lua_checkUMD},
	{"getDate", lua_GetDate},
	{"getTime", lua_GetTime},
	{"extractRar", lua_RarExtract},
	{"startISO",    lua_startISO},
	{"getCpuSpeed",	lua_getBus},
	{"getBusSpeed",	lua_getCpu},
	{"getFreeSize",	lua_getFSize},
	{"getTotalSize", lua_getTSize},
	{"getLanguage",	lua_getLanguage},
	{"getPSID",	lua_getPSID},
	{"getMacAddress", lua_getmac},
	{"nickname", lua_nickname},
	{"renameDir", lua_moveDir},
	{"startELF",  lua_startPBP},
	{"startPRX",  lua_startPRX},
	{"shutdown", lua_shutdown},
	{"suspend",	lua_standby},
	{"setLow",	lua_setLow},
	{"setReg", lua_setReg},
	{"setHigh", lua_setHigh},
	{"copyFile", lua_copyFile},
	{"copyDir", lua_copyDir},
	{"doesDirExist", lua_checkExist},
	{"doesFileExist", lua_checkExist},
	{"loadlib", lua_loadModule},
	{"startUMDUpdate", lua_startUMDUpdate},
	{"sioInit", lua_sioInit},
	{"sioRead", lua_sioRead},
	{"sioWrite", lua_sioWrite},
	{"irdaInit", lua_irdaInit},
	{"irdaRead", lua_irdaRead},
	{"irdaWrite", lua_irdaWrite},
	{"unassign", lua_Unassign},
	{"assign", lua_Assign},
	{"getModel", lua_getModel},
	{"launchUMD", lua_startUMD},
	{"startUpdate",	lua_startUpdate},
	{"startPBP", lua_startPBP},
	{"setCpuSpeed", lua_setCpuSpeed},
	{"getFPS", lua_showFPS},
	{"quit",   lua_systemQuit},
	{"msgDialog", lua_SCEShowMessageDialog},
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

static const luaL_reg System_meta[] = {
	{ 0, 0 }
};

UserdataRegister(System, System_functions, System_meta);

void luaSystem_init(lua_State *L) {
	System_register(L);
#define PSP_UTILITY_CONSTANT(name)\
	lua_pushstring(L, #name);\
	lua_pushnumber(L, PSP_UTILITY_##name);\
	lua_settable(L, -3);

#define MODE_MARCH33 1
#define MODE_NP9660  2
#define MODE_INFERNO 3

#define ISO_CONSTANT(name) \
	lua_pushstring(L, #name); \
	lua_pushnumber(L, MODE_##name); \
	lua_settable(L, -3);

	ISO_CONSTANT(MARCH33);
	ISO_CONSTANT(NP9660);
	ISO_CONSTANT(INFERNO);
}
