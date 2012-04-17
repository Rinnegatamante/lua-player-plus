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

#include <sys/stat.h>

extern "C" {
#include <kubridge.h>
}

#include "LuaPlayer.h"
#include "Archives.h"
#include "Common.h"


UserdataStubs(Zip, Zip*)
UserdataStubs(ZipFile, ZipFile*)

static int lua_ZipOpen(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Zip.open(filename) takes one argument.");
	const char *Filepath = luaL_checkstring(L, 1);
	if(!Filepath)
		return luaL_error(L, "Cannot open the zip file '%s'.", Filepath);
	lua_gc(L, LUA_GCCOLLECT, 0);
	Zip *newZip = ZipOpen(Filepath);
	if(!newZip) {
		lua_pushnil(L);
		return(0x0);
	}
	Zip **luaNewZip = pushZip(L);
	*luaNewZip = newZip;
	return(1);
}

static int lua_ZipClose(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Zip:Close() takes no arguments and must be called with a colon.");
	Zip **handle = toZip(L, 1);
	lua_pushboolean(L, ZipClose(*handle));
	return(1);
}

static int lua_ZipRead(lua_State *L) {
	int argc = lua_gettop(L);
	if(argc != 2 && argc != 3)
		return luaL_error(L, "Zip:read(filename, [password]) takes 1 or 2 arguments and must be called with a colon.");
	Zip **handle = toZip(L, 1);
	const char *Filename = luaL_checkstring(L, 2);
	const char *Password;
	if(argc == 2)
		Password = NULL;
	else
		Password = luaL_checkstring(L, 3);
	lua_gc(L, LUA_GCCOLLECT, 0);
	ZipFile *newZipFile = ZipFileRead(*handle, Filename, Password);
	if(!newZipFile) {
		lua_pushnil(L);
		return(0x0);
	}
	ZipFile **luaNewZipFile = pushZipFile(L);
	*luaNewZipFile = newZipFile;
	return(1);
}

static int lua_ZipExtract(lua_State *L) {
	int argc = lua_gettop(L);
	if(argc != 2 && argc != 3)
		return luaL_error(L, "Zip.extract(zipfile, dest, [Password) takes 2 or 3 arguments.");
	const char *FileToExtract = luaL_checkstring(L, 1);
	const char *DirTe = luaL_checkstring(L, 2);
	const char *Password = (argc == 3) ? luaL_checkstring(L, 3) : NULL;
	const char *tmpPath = (kuKernelGetModel() == 4) ? "ef0:/tempLPP" : "ms0:/tempLPP";
	const char *tmpFile = (kuKernelGetModel() == 4) ? "ef0:/tempLPP/temp.zip" : "ms0:/tempLPP/temp.zip"; 
	mkdir(tmpPath, 0777);
	sceIoMove((char*)FileToExtract, (char*)tmpFile);
	Zip *handle = ZipOpen(tmpFile);
	int result = ZipExtract(handle, Password);
	ZipClose(handle);
	sceIoRemove(tmpFile);
	sceIoMvdir((char*)tmpPath, (char*)DirTe);
	lua_pushboolean(L, result);
	return(1);
}

static int lua_ZipFileData(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Zip:readFile() takes no arguments and must be called with a colon.");
	ZipFile *handle = *toZipFile(L, 1);
	lua_gc(L, LUA_GCCOLLECT, 0);
	lua_pushlstring(L, (char*)handle->data, handle->size);
	return(1);
}

static int lua_ZipFileSize(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Zip.sizeFile() takes no arguments and must be called with a colon.");
	ZipFile *handle = *toZipFile(L, 1);
	lua_pushinteger(L, handle->size);
	return(1);
}

static const luaL_Reg Zip_functions[] = {
	{"open",							lua_ZipOpen},
	{"close",					lua_ZipClose},
	{"read",							lua_ZipRead},
	{"extract",						lua_ZipExtract},
	{"readFile",						lua_ZipFileData},
	{"sizeFile",						lua_ZipFileSize},
	{0, 0}
};

static const luaL_Reg Zip_Meta[] = {
	{ 0, 0}
};

UserdataRegister(Zip, Zip_functions, Zip_Meta);
UserdataRegister(ZipFile, Zip_functions, Zip_Meta);

void luaZip_init(lua_State *L) {
	Zip_register(L);
	ZipFile_register(L);
	lua_gettable(L, LUA_GLOBALSINDEX);
}
