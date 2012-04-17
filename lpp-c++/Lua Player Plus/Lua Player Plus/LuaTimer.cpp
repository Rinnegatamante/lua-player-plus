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
#- JiC� for drawCircle function ----------------------------------------------------------------------------------------#
#- Rapper_skull & DarkGiovy for testing LuaPlayer Plus and coming up with some neat ideas for it. ----------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------*/

#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pspdisplay.h>
#include "LuaPlayer.h"

typedef struct
{
	clock_t measuredTime;
	clock_t offset;
} Timer;

UserdataStubs(Timer, Timer*)

static clock_t getCurrentMilliseconds()
{
	return clock() / (CLOCKS_PER_SEC / 1000);
}

static int Timer_new(lua_State *L)
{
	int argc = lua_gettop(L); 
	if (argc != 0 && argc != 1) return luaL_error(L, "Timer.new([startTime]) takes zero or one arguments."); 

	Timer** luaTimer = pushTimer(L);
	Timer* timer = (Timer*) malloc(sizeof(Timer));
	*luaTimer = timer;
	timer->measuredTime = getCurrentMilliseconds();
	timer->offset = argc == 1 ? luaL_checkint(L, 1) : 0;
	return 1;
}

static int Timer_start(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 1) return luaL_error(L, "Timer:start() takes no arguments.");
	Timer* timer = *toTimer(L, 1);
	if (timer->measuredTime) {
		clock_t currentTime = getCurrentMilliseconds();
		lua_pushnumber(L, currentTime - timer->measuredTime + timer->offset);
	} else {
		timer->measuredTime = getCurrentMilliseconds();
		lua_pushnumber(L, timer->offset);
	}
	return 1;
}

static int Timer_time(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 1) return luaL_error(L, "Timer:time() takes no arguments.");
	Timer* timer = *toTimer(L, 1);
	if (timer->measuredTime) {
		clock_t currentTime = getCurrentMilliseconds();
		lua_pushnumber(L, currentTime - timer->measuredTime + timer->offset);
	} else {
		lua_pushnumber(L, timer->offset);
	}
	return 1;
}

static int Timer_stop(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 1) return luaL_error(L, "Timer:stop() takes no arguments.");
	Timer* timer = *toTimer(L, 1);
	if (timer->measuredTime) {
		clock_t currentTime = getCurrentMilliseconds();
		timer->offset = currentTime - timer->measuredTime + timer->offset;
		timer->measuredTime = 0;
	}
	lua_pushnumber(L, timer->offset);
	return 1;
}


static int Timer_reset(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc > 2) return luaL_error(L, "Timer:reset() takes zero or one arguments.");
	Timer* timer = *toTimer(L, 1);
	if (timer->measuredTime) {
		clock_t currentTime = getCurrentMilliseconds();
		lua_pushnumber(L, currentTime - timer->measuredTime + timer->offset);
	} else {
		lua_pushnumber(L, timer->offset);
	}
	timer->offset = argc == 2 ? luaL_checkint(L, 2) : 0;
	timer->measuredTime = 0;
	return 1;
}


static int Timer_free(lua_State *L)
{
	free(*toTimer(L, 1));
	return 0;
}

static int Timer_tostring (lua_State *L)
{
	Timer* timer = *toTimer(L, 1);
	clock_t t;
	if (timer->measuredTime) {
		clock_t currentTime = getCurrentMilliseconds();
		t = currentTime - timer->measuredTime + timer->offset;
	} else {
		t = timer->offset;
	}
	lua_pushfstring(L, "%i", t);
	return 1;
}

static const luaL_reg Timer_methods[] = {
	{"new", Timer_new},
	{"start", Timer_start},
	{"time", Timer_time},
	{"stop", Timer_stop},
	{"reset", Timer_reset},
	{0,0}
};

static const luaL_reg Timer_meta[] = {
	{"__gc", Timer_free},
	{"__tostring", Timer_tostring},
	{0,0}
};

UserdataRegister(Timer, Timer_methods, Timer_meta)

void luaTimer_init(lua_State *L)
{
	Timer_register(L);
}