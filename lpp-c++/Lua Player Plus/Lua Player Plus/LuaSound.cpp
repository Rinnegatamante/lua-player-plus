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
#include <stdlib.h>
#include <unistd.h>
#include <oslib/oslib.h>
#include "LuaPlayer.h"

UserdataStubs(Sound, OSL_SOUND*)

static int lua_SoundPlay(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "Sound:play(voice) takes 1 argument and must be called with a colon.");
	oslPlaySound(*toSound(L, 1), luaL_checknumber(L, 2));
	return(1);
}

static int lua_StopSound(lua_State *L)
{
	if(lua_gettop(L) != 1)
		return luaL_error(L, "Sound:stop() takes no arguments and must be called with a colon.");
	oslStopSound(*toSound(L, 1));
	return(1);
}

static int lua_PauseSound(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "Sound:pause(pause) takes 1 argument and must be called with a colon.");
	oslPauseSound(*toSound(L, 1), luaL_checknumber(L, 2));
	return(1);
}

static int lua_SounSetLoop(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "Sound:setLoop(loop) takes 2 arguments and must be called with a colon.");
	OSL_SOUND *sound = *toSound(L, 1);
	oslSetSoundLoop(sound, luaL_checknumber(L, 2));
	return(1);
}


/******************************************************************************
 ** Mp3 ***********************************************************************
 *******************************************************************************/

static int lua_Mp3Load(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "Sound.loadMp3(filename, stream) takes 2 arguments.");
	const char *Filename = luaL_checkstring(L, 1);
	OSL_SOUND *sound = oslLoadSoundFileMP3(Filename, luaL_checknumber(L, 2));
	if(sound == NULL) return luaL_error(L, "Cannot load the file '%s'.", Filename);
	OSL_SOUND **luasound = pushSound(L);
	*luasound = sound;
	return(1);
}

/******************************************************************************
 ** At3 ***********************************************************************
 *******************************************************************************/

static int lua_At3Load(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "Sound.loadAt3(filename, stream) takes 2 arguments.");
	const char *Filename = luaL_checkstring(L, 1);
	OSL_SOUND *sound = oslLoadSoundFileAT3(Filename, luaL_checknumber(L, 2));
	if(sound == NULL) return luaL_error(L, "Cannot load the file '%s'.", Filename);
	OSL_SOUND **luasound = pushSound(L);
	*luasound = sound;
	return(1);
}

/******************************************************************************
 ** Mod ***********************************************************************
 *******************************************************************************/

static int lua_ModLoad(lua_State *L)
{
	if(lua_gettop(L) != 2)
		return luaL_error(L, "Sound.loadMod(filename, stream) takes 2 arguments.");
	const char *Filename = luaL_checkstring(L, 1);
	OSL_SOUND *sound = oslLoadSoundFileMOD(Filename, luaL_checknumber(L, 2));
	if(sound == NULL) return luaL_error(L, "Cannot load the file '%s'.", Filename);
	OSL_SOUND **luasound = pushSound(L);
	*luasound = sound;
	return(1);
}

/******************************************************************************
 ** Bgm ***********************************************************************
 *******************************************************************************/

static int lua_BgmLoad(lua_State *L)
{
	if(lua_gettop(L) != 2) 
		return luaL_error(L, "Sound.loadBgm(filename, stream) takes 2 arguments.");
	const char *Filename = luaL_checkstring(L, 1);
	OSL_SOUND *sound = oslLoadSoundFileBGM(Filename, luaL_checknumber(L, 2));
	if(sound == NULL) return luaL_error(L, "Cannot load the file '%s'.", Filename);
	OSL_SOUND **luasound = pushSound(L);
	*luasound = sound;
	return(1);
}

/******************************************************************************
 ** Wav ***********************************************************************
 *******************************************************************************/

static int lua_WavLoad(lua_State *L)
{
	if(lua_gettop(L) != 2) 
		return luaL_error(L, "Sound.loadWav(filename, stream) takes 2 arguments.");
	const char *Filename = luaL_checkstring(L, 1);
	OSL_SOUND *sound = oslLoadSoundFileWAV(Filename, luaL_checknumber(L, 2));
	if(sound == NULL) return luaL_error(L, "Cannot load the file '%s'.", Filename);
	OSL_SOUND **luasound = pushSound(L);
	*luasound = sound;
	return(1);
}

/******************************************************************************
 ** All ***********************************************************************
 *******************************************************************************/

static int lua_SoundLoad(lua_State *L)
{
	if(lua_gettop(L) != 2) 
		return luaL_error(L, "Sound.load(filename, stream) takes 2 arguments.");
	const char *Filename = luaL_checkstring(L, 1);
	OSL_SOUND *sound = oslLoadSoundFile(Filename, luaL_checknumber(L, 2));
	if(sound == NULL) return luaL_error(L, "Cannot load the file '%s'.", Filename);
	OSL_SOUND **luasound = pushSound(L);
	*luasound = sound;
	return(1);
}

static const luaL_Reg Sound_functions[] = {
	{"play", lua_SoundPlay},
	{"pause", lua_PauseSound},
	{"stop", lua_StopSound},
	{"setLoop", lua_SounSetLoop},
	{"load", lua_SoundLoad},
	{"loadMp3", lua_Mp3Load},
	{"loadBgm", lua_BgmLoad},
	{"loadWav", lua_WavLoad},
	{"loadAt3", lua_At3Load},
	{"loadMod", lua_ModLoad},
	{0, 0}
};

static const luaL_Reg Sound_Meta[] = {
	{0, 0}
};

UserdataRegister(Sound, Sound_functions, Sound_Meta);

void lua_SoundInit(lua_State *L)
{
	Sound_register(L);
}