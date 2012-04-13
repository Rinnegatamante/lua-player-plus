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
#-----------------------------------------------------------------------------------------------------------------------*/

#ifndef __LPP_HPP_
#define __LPP_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <psppower.h>
#include <pspctrl.h>
#include <pspsdk.h>
#include <psputility.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <pspiofilemgr.h>
#include <math.h>
#include <psprtc.h>
#include <pspopenpsid.h>
#include <sys/stat.h>
#include <pspumd.h>
#include <zzip/lib.h>
#include <zzip/plugin.h>
#include <pspgu.h>
#include <pspgum.h>
//#include <kubridge.h>

#include "Lua/src/lua.h"
#include "Lua/src/lauxlib.h"
#include "Lua/src/lualib.h"

#include <unistd.h>
#include <pspwlan.h>
#include <pspopenpsid.h>
//#include <pspusbdevice.h>
#include <pspinit.h>

#ifndef MODULE_NAME
    #define MODULE_NAME ("Lua Player Plus")
#endif

#ifndef MODULE_VERSION_MAJOR
    #define MODULE_VERSION_MAJOR (1)
#endif

#ifndef MODULE_VERSION_MINOR
    #define MODULE_VERSION_MINOR (0)
#endif

#ifndef MODULE_ATTR
    #define MODULE_ATTR (0)
#endif

#define PSP_SCREEN_WIDTH  (480)
#define PSP_SCREEN_HEIGHT (272)
#define PSP_SCREEN_BPP    (32)

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#include "Libs/Types.h"

int Support_IsLoaded(void);

int InitSupport(void);

#define UserdataStubs(HANDLE, DATATYPE) \
        DATATYPE *to##HANDLE (lua_State *L, int index) \
{ \
        DATATYPE* handle  = (DATATYPE*)lua_touserdata(L, index); \
        if (handle == NULL) luaL_typerror(L, index, #HANDLE); \
        return handle; \
} \
        DATATYPE* push##HANDLE(lua_State *L) { \
        DATATYPE * newvalue = (DATATYPE*)lua_newuserdata(L, sizeof(DATATYPE)); \
        luaL_getmetatable(L, #HANDLE); \
        lua_pushvalue(L, -1); \
        lua_setmetatable(L, -3); \
        lua_pushstring(L, "__index"); \
        lua_pushstring(L, #HANDLE); \
        lua_gettable(L, LUA_GLOBALSINDEX); \
        lua_settable(L, -3); \
        lua_pop(L, 1); \
        return newvalue; \
}

#define UserdataRegister(HANDLE, METHODS, METAMETHODS) \
        int HANDLE##_register(lua_State *L) { \
        luaL_newmetatable(L, #HANDLE);  \
        lua_pushliteral(L, "__index"); \
        lua_pushvalue(L, -2);  \
        lua_rawset(L, -3); \
        \
        luaL_openlib(L, 0, METAMETHODS, 0); \
        luaL_openlib(L, #HANDLE, METHODS, 0); \
        \
        lua_pushstring(L, #HANDLE); \
        lua_gettable(L, LUA_GLOBALSINDEX); \
        luaL_getmetatable(L, #HANDLE); \
        lua_setmetatable(L, -2); \
        return 1; \
}

#ifdef __cplusplus
}
#endif

#endif /* __LPP_HPP_ */
