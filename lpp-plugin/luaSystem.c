#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <psppower.h>
#include <pspdebug.h>
#include <psprtc.h>
#include <pspLoadExec.h>
#include <stdarg.h>
#include <ctype.h>
#include <pspgu.h>
#include <pspsdk.h>
#include <psploadexec_kernel.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <pspthreadman.h>
#include <pspctrl.h>
#include <stdio.h>
#include <pspiofilemgr.h>
#include <malloc.h>
#include "include/luaplayer.h"

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

static int lua_createDir(lua_State *L)
{
    const char *path = luaL_checkstring(L, 1);
    if(!path) return luaL_error(L, "System.createDirectory(directory) takes a directory name as a string argument.");

    mkdir(path, 0777);

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

//Register our System Functions
static const luaL_reg System_functions[] = {
  {"resumeThread",               lua_resumeGame},
  {"createDirectory",            lua_createDir},
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
