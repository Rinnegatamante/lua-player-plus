#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pspkernel.h>
#include <psppower.h>
#include <pspdebug.h>
#include <psprtc.h>
#include <pspLoadExec.h>
#include <stdarg.h>
#include <ctype.h>
#include <pspgu.h>
#include <pspsdk.h>
#include <psploadexec_kernel.h>
#include "include/luaplayer.h"
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

SceUID psploadlib( const char * name, char * init );
void **findFunction( SceUID id, const char * library, const char * name );
int init( lua_State *L);

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
    if (argc != 3) return luaL_error(L, "wrong number of arguments");
    int x = luaL_checkint(L, 1);
    int y = luaL_checkint(L, 2);
    const char* text = luaL_checkstring(L, 3);
	pspDebugScreenSetXY(x,y);
    pspDebugScreenPrintf(text);
    return 0;
}

//Register our System Functions
static const luaL_reg Screen_functions[] = {
  {"waitVblankStart",               lua_waitVblankStart},
  {"debugPrint",                    lua_print},
  {0, 0}
};

void luaSystem_init(lua_State *L) {
    luaL_openlib(L, "Screen", Screen_functions, 0);

#define PSP_UTILITY_CONSTANT(name)\
    lua_pushstring(L, #name);\
    lua_pushnumber(L, PSP_UTILITY_##name);\
    lua_settable(L, -3);

    lua_pushstring(L, "Screen");
    lua_gettable(L, LUA_GLOBALSINDEX);
	
}
