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

#include <stdlib.h>
#include <pspctrl.h>
#include "LuaPlayer.h"

typedef SceCtrlData Controls;

UserdataStubs(Controls, Controls)

	static int Controls_tostring(lua_State*L)
{
	char buff[32];
	sprintf(buff, "%d", toControls(L, 1)->Buttons);
	lua_pushfstring(L, "Controls (%s)", buff);
	return 1;
}

static int Controls_read(lua_State*L)
{
	int argc = lua_gettop(L);
	if(argc) return luaL_error(L, "Argument error: Controls.read() cannot be called from an instance.");
	Controls* pad = pushControls(L);
	sceCtrlReadBufferPositive(pad, 1); 
	return 1;
}

static int Controls_readPeek(lua_State*L)
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
	int argc = lua_gettop(L); \
	if (argc > 1) return luaL_error(L, g_errorMessage); \
	if(argc == 0) { \
	lua_pushinteger(L, BIT); \
	return(1); \
	} \
	Controls *a = toControls(L, 1);\
	lua_pushinteger(L, BIT); \
	lua_pushboolean(L, (a->Buttons & BIT) == BIT); \
	return(1); \
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
	setTableValue(L, (char*)"selectMask", PSP_CTRL_SELECT);
	setTableValue(L, (char*)"startMask", PSP_CTRL_START);
	setTableValue(L, (char*)"upMask", PSP_CTRL_UP);
	setTableValue(L, (char*)"rightMask", PSP_CTRL_RIGHT);
	setTableValue(L, (char*)"downMask", PSP_CTRL_DOWN);
	setTableValue(L, (char*)"leftMask", PSP_CTRL_LEFT);
	setTableValue(L, (char*)"ltriggerMask", PSP_CTRL_LTRIGGER);
	setTableValue(L, (char*)"rtriggerMask", PSP_CTRL_RTRIGGER);
	setTableValue(L, (char*)"triangleMask", PSP_CTRL_TRIANGLE);
	setTableValue(L, (char*)"circleMask", PSP_CTRL_CIRCLE);
	setTableValue(L, (char*)"crossMask", PSP_CTRL_CROSS);
	setTableValue(L, (char*)"squareMask", PSP_CTRL_SQUARE);
	setTableValue(L, (char*)"homeMask", PSP_CTRL_HOME);
	setTableValue(L, (char*)"holdMask", PSP_CTRL_HOLD);
	setTableValue(L, (char*)"noteMask", PSP_CTRL_NOTE);
}