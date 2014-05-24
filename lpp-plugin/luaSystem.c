#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <psppower.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <psprtc.h>
#include <pspLoadExec.h>
#include <stdarg.h>
#include <ctype.h>
#include <pspgu.h>
#include <pspsdk.h>
#include <psploadexec_kernel.h>
#include <pspkernel.h>
#include <pspopenpsid.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <pspthreadman.h>
#include <pspctrl.h>
#include <stdio.h>
#include <pspiofilemgr.h>
#include <malloc.h>
#include "include/luaplayer.h"
#include "include/kubridge.h"
#include "include/systemctrl.h"
#include "include/libUnrar.h"

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

typedef struct
{
    unsigned long        maxclusters;
    unsigned long        freeclusters;
    int                    unk1;
    unsigned int        sectorsize;
    u64                    sectorcount;
    
} SystemDevCtl;

typedef struct
{
    SystemDevCtl *pdevinf;    
} SystemDevCommand;

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

/*
 *
 ---------            BATTERY Functions            ---------
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

static int lua_createDir(lua_State *L)
{
    const char *path = luaL_checkstring(L, 1);
    if(!path) return luaL_error(L, "System.createDirectory(directory) takes a directory name as a string argument.");

    mkdir(path, 0777);

    return 0;
}

static int lua_clearScreen(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	pspDebugScreenClear();
	return 0;
}

static int lua_waitVblankStart(lua_State *L)
{
    int argc = lua_gettop(L), t = 0;
    if (argc != 0 && argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments"); // can be called as both screen.wait...() and screen:wait...()
    if (argc) t = lua_type(L, 1);
    if (argc == 0 || t != LUA_TNUMBER) {
        sceDisplayWaitVblankStart();
    } else {
        int count = (t == LUA_TNUMBER)?luaL_checkint(L, 1):luaL_checkint(L, 2);
        int i;
        for (i = 0; i < count; i++) sceDisplayWaitVblankStart();
    }
    return 0;
}

static int lua_print(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 4) return luaL_error(L, "wrong number of arguments");
    int x = luaL_checkint(L, 1);
    int y = luaL_checkint(L, 2);
    const char* text = luaL_checkstring(L, 3);
	u32 color = luaL_checknumber(L,4);
	pspDebugScreenSetXY(x,y);
	pspDebugScreenSetTextColor(color);
    pspDebugScreenPrintf(text);
    return 0;
}

#define MAX_THREAD 64
static int thread_count_start, thread_count_now;
static SceUID pauseuid = -1, thread_buf_start[MAX_THREAD], thread_buf_now[MAX_THREAD], thid1 = -1;
void pauseGame(SceUID thid)
{
if(pauseuid >= 0)
return;
pauseuid = thid;
sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, thread_buf_now, MAX_THREAD, &thread_count_now);
int x, y, match;
for(x = 0; x < thread_count_now; x++)
{
match = 0;
SceUID tmp_thid = thread_buf_now[x];
for(y = 0; y < thread_count_start; y++)
{
if((tmp_thid == thread_buf_start[y]) || (tmp_thid == thid1))
{
match = 1;
break;
}
}
if(match == 0)
sceKernelSuspendThread(tmp_thid);
}
}

void resumeGame(SceUID thid)
{
if(pauseuid != thid)
return;
pauseuid = -1;
int x, y, match;
for(x = 0; x < thread_count_now; x++)
{
match = 0;
SceUID tmp_thid = thread_buf_now[x];
for(y = 0; y < thread_count_start; y++)
{
if((tmp_thid == thread_buf_start[y]) || (tmp_thid == thid1))
{
match = 1;
break;
}
}
if(match == 0)
sceKernelResumeThread(tmp_thid);
}
}

static int lua_resumeGame(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	resumeGame(thid1);
	return luaL_error(L, "resumeThread");
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
//Start hb from MS
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
    sctrlKernelLoadExecVSHMs2(file, &param);
}

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

// Other functions
static int lua_cfwver(lua_State *L)
{
    if (lua_gettop(L) != 0) return luaL_error(L, "no arguments expected.");
    int ver = sceKernelDevkitVersion();
    char *aStr;
    char buffer[16];
    sprintf(buffer,"%x",ver);
    buffer[1]='.';     buffer[3]= buffer[4];     buffer[4]=0;
    aStr =buffer;
    lua_pushstring(L, aStr);
    return 1;
}
static int lua_GetDate(lua_State *L)
{
if (lua_gettop(L) != 1) return luaL_error(L, "System.getDate takes one argument (1 for the Day , 2 for the month , 3 for the year).");
int Param = luaL_checkint(L, 1);
if (Param > 3)
{
char* Error;
sprintf(Error, "Invalid argument #1 in 'System.getDate'. %d is not a valid value.", Param);
return luaL_error(L, Error);
}
pspTime psptime;
int Day, Month, Year;
sceRtcGetCurrentClockLocalTime(&psptime);
if (Param == 1) { 
Day = psptime.day;
lua_pushnumber(L, Day);
return 1;
}
if (Param == 2) {
Month = psptime.month;
lua_pushnumber(L, Month);
return 1;
}
if (Param == 3) {
Year = psptime.year;
lua_pushnumber(L, Year);
return 1;
}
return 0;
}

static int lua_GetTime(lua_State *L)
{
if (lua_gettop(L) != 1) return luaL_error(L, "System.getTime takes one argument (1 for the Hour , 2 for the Minutes , 3 for the Seconds).");
int Param = luaL_checkint(L, 1);
if (Param > 3)
{
char* Error;
sprintf(Error, "Invalid argument #1 in 'System.getTime'. %d is not a valid value.", Param);
return luaL_error(L, Error);
}
pspTime psptime;
int Hour, Minutes, Seconds;
sceRtcGetCurrentClockLocalTime(&psptime);
if (Param == 1) { 
Hour = psptime.hour;
lua_pushnumber(L, Hour);
return 1;
}
if (Param == 2) {
Minutes = psptime.minutes;
lua_pushnumber(L, Minutes);
return 1;
}
if (Param == 3) {
Seconds = psptime.seconds;
lua_pushnumber(L, Seconds);
return 1;
}
return 0;
}

//Register our System Functions
static const luaL_reg System_functions[] = {
  {"getTime",                       lua_GetTime},
  {"getDate",                       lua_GetDate},
  {"getVersion",               		lua_cfwver},
  {"startPBP",               		lua_startPBP},
  {"resumeThread",               	lua_resumeGame},
  {"createDirectory",            	lua_createDir},
  {"renameFile",            	 	lua_rename},
  {"renameDir",            	 	 	lua_moveDir},
  {"getFreeSize",                	lua_getFSize},
  {"getTotalSize",               	lua_getTSize},
  {"removeFile",            		lua_removeFile},
  {"removeDir",            	 		lua_removeDir},
  {"copyFile",            	 		lua_copyFile},
  {"copyDir",            	 		lua_copyDir},
  {"powerIsPowerOnline",            lua_powerIsPowerOnline},
  {"powerIsBatteryExist",           lua_powerIsBatteryExist},
  {"powerIsBatteryCharging",        lua_powerIsBatteryCharging},
  {"powerGetBatteryChargingStatus", lua_powerGetBatteryChargingStatus},
  {"powerIsLowBattery",             lua_powerIsLowBattery},
  {"powerGetBatteryLifePercent",    lua_powerGetBatteryLifePercent},
  {"powerGetBatteryLifeTime",       lua_powerGetBatteryLifeTime},
  {"powerGetBatteryTemp",           lua_powerGetBatteryTemp},
  {"powerGetBatteryVolt",           lua_powerGetBatteryVolt},
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
	
}

//Register our Screen Functions
static const luaL_reg Screen_functions[] = {
  {"waitVblankStart",               lua_waitVblankStart},
  {"debugPrint",                    lua_print},
  {"clear",							lua_clearScreen},
  {0, 0}
};

void luaScreen_init(lua_State *L) {
    luaL_openlib(L, "Screen", Screen_functions, 0);

#define PSP_UTILITY_CONSTANT(name)\
    lua_pushstring(L, #name);\
    lua_pushnumber(L, PSP_UTILITY_##name);\
    lua_settable(L, -3);

    lua_pushstring(L, "Screen");
    lua_gettable(L, LUA_GLOBALSINDEX);
	
}
