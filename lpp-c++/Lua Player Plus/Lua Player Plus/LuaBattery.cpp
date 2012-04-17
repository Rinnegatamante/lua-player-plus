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

#include <string.h>
#include "LuaPlayer.h"

typedef void Battery;

UserdataStubs(Battery, Battery*);

extern "C" {
u16 Read_eeprom(u8 addr);
u32 Write_eeprom(u8 addr, u16 data);
}


static int lua_BatTeory(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "Battery.getTeoryCapacity() takes no arguments.");
	lua_pushnumber(L, (u16)Read_eeprom(0x00));
	return(1);
}

static int lua_BatReal(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "Battery.getRealCapacity() takes no arguments.");
	lua_pushnumber(L, (u16)Read_eeprom(0x30));
	return(1);
}

static int lua_BatCharge(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "Battery.gatChargeCount() takes no arguments.");
	lua_pushnumber(L, (u16)Read_eeprom(0x11));
	return(1);
}

static int lua_writeEEPROM(lua_State *L) {
	if(lua_gettop(L) != 2)
		return luaL_error(L, "Battery.writeSerial(base, finish) takes 2 arguments.");
	u16 hi = luaL_checkint(L, 1);
	u16 lo = luaL_checkint(L, 2);
	Write_eeprom(0x09, lo);
	Write_eeprom(0x07, hi);
	return(1);
}

static int lua_Pandorize(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "Battery.pandorize() takes no arguments.");
	Write_eeprom(0x07, 0xFFFF);
	Write_eeprom(0x09, 0xFFFF);
	return(1);
}

static int lua_Normalize(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "Battery.normalize() takes no arguments.");
	Write_eeprom(0x07, 0x90CA);
	Write_eeprom(0x09, 0x0815);
	return(1);
}

static int lua_receiveEEPROM(lua_State *L) {
	if(lua_gettop(L) != 0)
		return luaL_error(L, "Battery.getSerial() takes no arguments.");
	u16 hivalue = Read_eeprom(0x07);
	u16 lowvalue = Read_eeprom(0x09);
	u32 serial = (hivalue << 16) | lowvalue;
	char string[256];
	sprintf(string, "%08x", (unsigned int)serial);
	for(int i = 0; string[i]; i++) {
		string[i] = toupper(string[i]);
	}
	char suffix[10] = "0x";
	strcat(suffix, string);
	lua_pushstring(L, suffix);
	return(1);
}

static const luaL_Reg Battery_functions[] = {
	{"getTheoryCapacity", lua_BatTeory},
	{"getRealCapacity", lua_BatReal},
	{"getSerial", lua_receiveEEPROM},
	{"getChargeCount", lua_BatCharge},
	{"writeSerial", lua_writeEEPROM},
	{"normalize", lua_Normalize},
	{"pandorize", lua_Pandorize},
	{ 0, 0}
};

static const luaL_Reg Battery_meta[] = {
	{ 0, 0}
};

UserdataRegister(Battery, Battery_functions, Battery_meta);

void luaBattery_init(lua_State *L) {
	Battery_register(L);
}