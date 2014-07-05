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
#- Copyright (c) Nanni <lpp.nanni@gmail.com> ---------------------------------------------------------------------------#
#- Copyright (c) Rinnegatamante <rinnegatamante@gmail.com> -------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Credits : -----------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Zack & Shine for LP Euphoria sourcecode -----------------------------------------------------------------------------#
#- valantin for sceIoMvdir and sceIoCpdir improved functions------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------*/

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

static int lua_sw(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 2) return luaL_error(L, "wrong number of arguments");
	u32 addr = luaL_checknumber(L, 1);
	u32 val = luaL_checknumber(L, 2);
	_sw(addr,val);
	return 0;
}

static int lua_memcpy(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 3) return luaL_error(L, "wrong number of arguments");
	u32 addr = luaL_checknumber(L, 1);
	u32 source = luaL_checknumber(L, 2);
	int size = luaL_checkint(L, 3);
	memcpy((void *) addr, source, size);
	return 0;
}

static int lua_memset(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 3) return luaL_error(L, "wrong number of arguments");
	u32 addr = luaL_checknumber(L, 1);
	int value = luaL_checknumber(L, 2);
	int size = luaL_checkint(L, 3);
	memset((void *) addr, value, size);
	return 0;
}

static int lua_memcmp(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 3) return luaL_error(L, "wrong number of arguments");
	u32 addr = luaL_checknumber(L, 1);
	u32 addr2 = luaL_checknumber(L, 2);
	int size = luaL_checkint(L, 3);	
	return memcmp((void *) addr,(void *) addr2, size);
}

static int lua_lw(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "Kernel.loadWord(addr) takes 1 argument.");
	}
	lua_pushnumber(L,_lw(luaL_checknumber(L,1)));

	return 1;
}

//Register our Kernel Functions
static const luaL_reg Kernel_functions[] = {
  {"memset",                       lua_memset},
  {"memcpy",                       lua_memcpy},
  {"memcmp",                       lua_memcmp},
  {"storeWord",                    lua_sw},
  {"loadWord",                     lua_lw},
  {0, 0}
};

void luaKernel_init(lua_State *L) {
    luaL_openlib(L, "Kernel", Kernel_functions, 0);

#define PSP_UTILITY_CONSTANT(name)\
    lua_pushstring(L, #name);\
    lua_pushnumber(L, PSP_UTILITY_##name);\
    lua_settable(L, -3);

    lua_pushstring(L, "Kernel");
    lua_gettable(L, LUA_GLOBALSINDEX);
	
}