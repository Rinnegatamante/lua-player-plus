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

#include <pspdebug.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pspdisplay.h>
#include <sys/stat.h>
#include "include/luaplayer.h"

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
    char* text = luaL_checkstring(L, 3);
	u32 color = luaL_checknumber(L,4);
	pspDebugScreenSetXY(x,y);
	pspDebugScreenSetTextColor(color);
    pspDebugScreenPrintf(text);
    return 0;
}

static int lua_gradient(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 5) return luaL_error(L, "wrong number of arguments");
    int x = luaL_checkint(L, 1);
    int y = luaL_checkint(L, 2);
	char text[256];
    strcpy(text,luaL_checkstring(L, 3));
	int color_init = luaL_checknumber(L,4);
	int color_end = luaL_checknumber(L,5);
	pspDebugScreenSetXY(x,y);
	int size = strlen(text);
	int init_b = color_init / 65536;
	int end_b = color_end / 65536;
	int init_g = (color_init - (65536 * init_b)) / 256;
	int end_g = (color_end - (65536 * end_b)) / 256;
	int init_r = (color_init - (65536 * init_b) - (256 * init_g));
	int end_r = (color_end - (65536 * end_b) - (256 * end_g));
	int diff_r = end_r - init_r;
	int diff_g = end_g - init_g;
	int diff_b = end_b - init_b;
	int step_r = diff_r / size;
	int step_g = diff_g / size;
	int step_b = diff_b / size;
	int i=0;
	while (i<size){
	int color = (color_init + (i*step_r) + (i*step_g*256) + (i*step_b*65536));
	pspDebugScreenPutChar(x+i*6,y,color,text[i]);		
	i=i+1;
	}
    return 0;
}

static int lua_color(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 3) return luaL_error(L, "wrong number of arguments");
    int r = luaL_checkint(L, 1);
    int g = luaL_checkint(L, 2);
    int b = luaL_checkint(L, 3);
	int real_g = g * 256;
	int real_b = b * 65536;
	int color = r+real_b+real_g;
	lua_pushnumber(L,color);
    return 1;
}

static int lua_getR(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
    int color = luaL_checkint(L, 1);
	while (color > 65535){
	color = color - 65536;
	}
	while (color > 255){
	color = color - 256;
	}
	lua_pushnumber(L,color);
    return 1;
}

static int lua_getG(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
    int color = luaL_checkint(L, 1);
	while (color > 65535){
	color = color - 65536;
	}
	int push = color / 256;
	lua_pushnumber(L,push);
    return 1;
}

static int lua_getB(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
    int color = luaL_checkint(L, 1);
	int push = color / 65536;
	lua_pushnumber(L,push);
    return 1;
}

//Register our Color Functions
static const luaL_reg Color_functions[] = {
  {"new",                lua_color},
  {"getR",				 lua_getR},
  {"getG",				 lua_getG},
  {"getB",				 lua_getB},
  {0, 0}
};

void luaColor_init(lua_State *L) {
    luaL_openlib(L, "Color", Color_functions, 0);

#define PSP_UTILITY_CONSTANT(name)\
    lua_pushstring(L, #name);\
    lua_pushnumber(L, PSP_UTILITY_##name);\
    lua_settable(L, -3);

    lua_pushstring(L, "Color");
    lua_gettable(L, LUA_GLOBALSINDEX);
	
}

//Register our Screen Functions
static const luaL_reg Screen_functions[] = {
  {"waitVblankStart",               lua_waitVblankStart},
  {"debugPrint",                    lua_print},
  {"debugPrintGradient",            lua_gradient},
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