/*
 * LuaPlayer Euphoria
 * ------------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE for details.
 *
 * Copyright (c) 2005 Frank Buss <fb@frank-buss.de> (aka Shine)
 * Copyright (c) 2009 Danny Glover <danny86@live.ie> (aka Zack) 
 *
 * Official Forum : http://www.retroemu.com/forum/forumdisplay.php?f=148
 * For help using LuaPlayer, code help, tutorials etc please visit the official site : http://www.retroemu.com/forum/forumdisplay.php?f=148
 *
 * Credits:
 * 
 * (from Shine/Zack) 
 *
 *   many thanks to the authors of the PSPSDK from http://forums.ps2dev.org
 *   and to the hints and discussions from #pspdev on freenode.net
 *
 * (from Zack Only)
 *
 * Thanks to Brunni for the Swizzle/UnSwizzle code (taken from oslib). 
 * Thanks to Arshia001 for AALIB. It is the sound engine used in LuaPlayer Euphoria. 
 * Thanks to HardHat for being a supportive friend and advisor.
 * Thanks to Benhur for IntraFont.
 * Thanks to Jono for the moveToVram code.
 * Thanks to Raphael for the Vram manager code.
 * Thanks to Osgeld, Dan369 & Cmbeke for testing LuaPlayer Euphoria for me and coming up with some neat ideas for it.
 * Thanks to the entire LuaPlayer Euphoria userbase, for using it and for supporting it's development. You guys rock!
 *
 *
 */

#include <stdlib.h>
#include <pspctrl.h>
#include "include/luaplayer.h"

typedef SceCtrlData Controls;

// The "Controls" userdata object.
// ------------------------------
UserdataStubs(Controls, Controls)

static int Controls_tostring (lua_State *L)
{
	char buff[32];
	sprintf(buff, "%d", toControls(L, 1)->Buttons);
	lua_pushfstring(L, "Controls (%s)", buff);
	return 1;
}

static int Controls_read(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc) return luaL_error(L, "Argument error: Controls.read() cannot be called from an instance.");
	Controls* pad = pushControls(L);
	sceCtrlReadBufferPositive(pad, 1); 
	return 1;
}

static int Controls_readPeek(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc) return luaL_error(L, "Argument error: Controls.read() cannot be called from an instance.");
	Controls* pad = pushControls(L);
	sceCtrlPeekBufferPositive(pad, 1); 
	return 1;
}

const char* g_errorMessage = "Argument error: The Controls functions take no arguments (and also, must be called with a colon from an instance: e g mycontrols:left().";

#define CHECK_CTRL(NAME, BIT) \
static int NAME(lua_State *L) \
{ \
	if (lua_gettop(L) != 1) return luaL_error(L, g_errorMessage); \
	Controls *a = toControls(L, 1);\
	lua_pushboolean(L, (a->Buttons & BIT) == BIT); \
	return 1; \
}

CHECK_CTRL(Controls_select, PSP_CTRL_SELECT)
CHECK_CTRL(Controls_start, PSP_CTRL_START)
CHECK_CTRL(Controls_up, PSP_CTRL_UP)
CHECK_CTRL(Controls_right, PSP_CTRL_RIGHT)
CHECK_CTRL(Controls_down, PSP_CTRL_DOWN)
CHECK_CTRL(Controls_left, PSP_CTRL_LEFT)
CHECK_CTRL(Controls_l, PSP_CTRL_LTRIGGER)
CHECK_CTRL(Controls_r, PSP_CTRL_RTRIGGER)
CHECK_CTRL(Controls_triangle, PSP_CTRL_TRIANGLE)
CHECK_CTRL(Controls_circle, PSP_CTRL_CIRCLE)
CHECK_CTRL(Controls_cross, PSP_CTRL_CROSS)
CHECK_CTRL(Controls_square, PSP_CTRL_SQUARE)
CHECK_CTRL(Controls_home, PSP_CTRL_HOME)
CHECK_CTRL(Controls_hold, PSP_CTRL_HOLD)
CHECK_CTRL(Controls_note, PSP_CTRL_NOTE)

static int Controls_buttons(lua_State *L)
{
	if (lua_gettop(L) != 1) return luaL_error(L, g_errorMessage);
	Controls *a = toControls(L, 1);
	lua_pushnumber(L, a->Buttons);
	return 1;
}

static int Controls_analogX(lua_State *L) 
{ 
	if (lua_gettop(L) != 1) return luaL_error(L, "Argument error: The Controls functions take no arguments."); 
	Controls *a = toControls(L, 1);
	lua_pushnumber(L, a->Lx -128); 
	return 1; 
}
static int Controls_analogY(lua_State *L) 
{ 
	if (lua_gettop(L) != 1) return luaL_error(L, "Argument error: The Controls functions take no arguments."); 
	Controls *a = toControls(L, 1);
	lua_pushnumber(L, a->Ly -128); 
	return 1; 
}

static int Controls_equal(lua_State *L) {
	Controls* a = toControls(L, 1);
	Controls* b = toControls(L, 2);
	lua_pushboolean(L, a->Buttons == b->Buttons );
	return 1;
}

	
static const luaL_reg Controls_methods[] = {
	{"read",      Controls_read},
	{"readPeek",  Controls_readPeek},
	{"select",    Controls_select},
	{"start",     Controls_start},
	{"up",        Controls_up},
	{"right",     Controls_right},
	{"down",      Controls_down},
	{"left",      Controls_left},
	{"l",         Controls_l},
	{"r",         Controls_r},
	{"triangle",  Controls_triangle},
	{"circle",    Controls_circle},
	{"cross",     Controls_cross},
	{"square",    Controls_square},
	{"home",      Controls_home},
	{"hold",      Controls_hold},
	{"note",      Controls_note},
	{"analogX",   Controls_analogX},
	{"analogY",   Controls_analogY},
	{"buttons",   Controls_buttons},
  {0, 0}
};

static const luaL_reg Controls_meta[] = {
  {"__tostring", Controls_tostring},
  {"__eq", Controls_equal},
  {0, 0}
};

UserdataRegister(Controls, Controls_methods, Controls_meta)

void setTableValue(lua_State *L, char* name, int value)
{
	lua_pushstring(L, name);
	lua_pushnumber(L, value);
	lua_settable(L, -3);
}

void luaControls_init(lua_State *L) {
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	Controls_register(L);

	lua_pushstring(L, "Controls");
	lua_gettable(L, LUA_GLOBALSINDEX); \
	setTableValue(L, "selectMask", PSP_CTRL_SELECT);
	setTableValue(L, "startMask", PSP_CTRL_START);
	setTableValue(L, "upMask", PSP_CTRL_UP);
	setTableValue(L, "rightMask", PSP_CTRL_RIGHT);
	setTableValue(L, "downMask", PSP_CTRL_DOWN);
	setTableValue(L, "leftMask", PSP_CTRL_LEFT);
	setTableValue(L, "ltriggerMask", PSP_CTRL_LTRIGGER);
	setTableValue(L, "rtriggerMask", PSP_CTRL_RTRIGGER);
	setTableValue(L, "triangleMask", PSP_CTRL_TRIANGLE);
	setTableValue(L, "circleMask", PSP_CTRL_CIRCLE);
	setTableValue(L, "crossMask", PSP_CTRL_CROSS);
	setTableValue(L, "squareMask", PSP_CTRL_SQUARE);
	setTableValue(L, "homeMask", PSP_CTRL_HOME);
	setTableValue(L, "holdMask", PSP_CTRL_HOLD);
	setTableValue(L, "noteMask", PSP_CTRL_NOTE);
}
