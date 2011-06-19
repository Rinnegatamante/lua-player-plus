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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "libs/aalib/pspaalib.h"
#include "include/luaplayer.h"

#define MAX_MUSIC_CHANNELS 2
#define MAX_SOUND_CHANNELS 30

/*
 *
 ---------			MP3 Functions			---------
 *
 */

//Load a Mp3
static int lua_Mp3Load(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "Mp3.load(filename, channel) takes 2 arguments : (The Filename, The Channel)");
	}
	
	const char *path = luaL_checkstring(L, 1);
	unsigned int channel = luaL_checkint(L, 2);
	
	char fullpath[512];
	getcwd(fullpath, 256);
	strcat(fullpath, "/");
	strcat(fullpath, path);
	
	lua_gc(L, LUA_GCCOLLECT, 0);
	
	if (AalibLoad(fullpath, PSPAALIB_CHANNEL_SCEMP3_1 + channel, false) != 0)
	{	
		return luaL_error(L, "Cannot find the requested Mp3 file - Check your path and make sure the file exists");
	}
	
	if (PSPAALIB_CHANNEL_SCEMP3_1 + channel >= PSPAALIB_CHANNEL_SCEMP3_1 + MAX_MUSIC_CHANNELS)
	{
		return luaL_error(L, "Mp3 sound channel unavailable - You are using over the maximum 2 allowed channels for Mp3 files");
	}
	return 0;
}

//Play a Mp3 though the PSP MEDIA ENGINE
static int lua_Mp3Play(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "Mp3.play() takes 2 arguments : (loop - true or false, channel)");
	}
	
	bool loop;
	loop = lua_toboolean(L, 1);
	unsigned int channel = luaL_checkint(L, 2);
	
	AalibPlay(PSPAALIB_CHANNEL_SCEMP3_1 + channel);
	AalibSetAutoloop(PSPAALIB_CHANNEL_SCEMP3_1 + channel, loop);
	
	return 0;
}
	
//Stop a currently playing Mp3
static int lua_Mp3Stop(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "Mp3.stop(channel) takes 1 argument (the channel to stop)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);
	
	AalibStop(PSPAALIB_CHANNEL_SCEMP3_1 + channel);
	AalibRewind(PSPAALIB_CHANNEL_SCEMP3_1 + channel);
	
	return 0;
}

//Pause a currently playing Mp3
static int lua_Mp3Pause(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "Mp3.pause(channel) takes 1 argument (the channel to stop)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);
	
	AalibPause(PSPAALIB_CHANNEL_SCEMP3_1 + channel);
	
	return 0;
}

//Unload a loaded Mp3
static int lua_Mp3Unload(lua_State *L)
{	
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "Mp3.unload() takes 1 argument (the channel to stop)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);
	
	AalibUnload(PSPAALIB_CHANNEL_SCEMP3_1 + channel);
	
	return 0;
}

//Set/Change the Volume of the Mp3
static int lua_Mp3Volume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "Mp3.volume(vol, channel) takes one argument (the volume in float, the channel)");
	}

	float vol = luaL_checknumber(L, 1);	
	unsigned int channel = luaL_checkint(L, 2);
	lua_pushnumber(L, vol);
		
	AalibEnable(PSPAALIB_CHANNEL_SCEMP3_1 + channel, PSPAALIB_EFFECT_VOLUME_MANUAL);
	AalibSetVolume((PSPAALIB_CHANNEL_SCEMP3_1 + channel), ((AalibVolume){(vol,vol)}));
	
	return 1;	
}

//Set the speed/tempo of the Mp3
static int lua_Mp3SetSpeed(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "Mp3.speed(speed) takes two arguments (the speed in float, the channel)");
	}

	float speed = luaL_checknumber(L, 1);	
	unsigned int channel = luaL_checkint(L, 2);
	lua_pushnumber(L, speed);
		
	AalibEnable(PSPAALIB_CHANNEL_SCEMP3_1 + channel, PSPAALIB_EFFECT_PLAYSPEED);
	AalibSetPlaySpeed(PSPAALIB_CHANNEL_SCEMP3_1 + channel, speed);

	return 1;	
}

//Return end of Mp3 steam
static int lua_Mp3GetEOS(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "Mp3.endOfStream() takes one argument (the channel)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);

	if (AalibGetStopReason(PSPAALIB_CHANNEL_SCEMP3_1 + channel) == PSPAALIB_WARNING_END_OF_STREAM_REACHED)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
 *
 ---------			OGG Functions			---------
 *
 */

//Load a Ogg
static int lua_OggLoad(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "Ogg.load(filename, channel) takes 2 arguments : (The Filename, The Channel)");
	}
	
	const char *path = luaL_checkstring(L, 1);
	unsigned int channel = luaL_checkint(L, 2);
	
	char fullpath[512];
	getcwd(fullpath, 256);
	strcat(fullpath, "/");
	strcat(fullpath, path);
	
	lua_gc(L, LUA_GCCOLLECT, 0);
	
	if (AalibLoad(fullpath, PSPAALIB_CHANNEL_OGG_1 + channel, false) != 0)
	{	
		return luaL_error(L, "Cannot find the requested Ogg file - Check your path and make sure the file exists");
	}
	
	if (PSPAALIB_CHANNEL_OGG_1 + channel >= PSPAALIB_CHANNEL_OGG_1 + MAX_MUSIC_CHANNELS)
	{
		return luaL_error(L, "Ogg sound channel unavailable - You are using over the maximum 2 allowed channels for Ogg files");
	}
	return 0;
}

//Play a Ogg though the PSP MEDIA ENGINE
static int lua_OggPlay(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "Ogg.play() takes 2 arguments : (loop - true or false, channel)");
	}
	
	bool loop;
	loop = lua_toboolean(L, 1);
	unsigned int channel = luaL_checkint(L, 2);
	
	AalibPlay(PSPAALIB_CHANNEL_OGG_1 + channel);
	AalibSetAutoloop(PSPAALIB_CHANNEL_OGG_1 + channel, loop);
	
	return 0;
}
	
//Stop a currently playing Ogg
static int lua_OggStop(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "Ogg.stop(channel) takes 1 argument (the channel to stop)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);
	
	AalibStop(PSPAALIB_CHANNEL_OGG_1 + channel);
	
	return 0;
}

//Pause a currently playing Ogg
static int lua_OggPause(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "Ogg.pause(channel) takes 1 argument (the channel to stop)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);
	
	AalibPause(PSPAALIB_CHANNEL_OGG_1 + channel);
	
	return 0;
}

//Unload a loaded Ogg
static int lua_OggUnload(lua_State *L)
{	
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "Ogg.unload() takes 1 argument (the channel to stop)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);
	
	AalibUnload(PSPAALIB_CHANNEL_OGG_1 + channel);
	
	return 0;
}

//Set/Change the Volume of the Ogg
static int lua_OggVolume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "Ogg.volume(vol, channel) takes one argument (the volume in float, the channel)");
	}

	float vol = luaL_checknumber(L, 1);	
	unsigned int channel = luaL_checkint(L, 2);
	lua_pushnumber(L, vol);
		
	AalibEnable(PSPAALIB_CHANNEL_OGG_1 + channel, PSPAALIB_EFFECT_VOLUME_MANUAL);
	AalibSetVolume(PSPAALIB_CHANNEL_OGG_1 + channel, ((AalibVolume){(vol,vol)}));
	
	return 1;	
}

//Set the speed/tempo of the Ogg
static int lua_OggSetSpeed(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "Ogg.speed(speed) takes two arguments (the speed in float, the channel)");
	}

	float speed = luaL_checknumber(L, 1);	
	unsigned int channel = luaL_checkint(L, 2);
	lua_pushnumber(L, speed);
		
	AalibEnable(PSPAALIB_CHANNEL_OGG_1 + channel, PSPAALIB_EFFECT_PLAYSPEED);
	AalibSetPlaySpeed(PSPAALIB_CHANNEL_OGG_1 + channel, speed);

	return 1;	
}

//Return end of Ogg steam
static int lua_OggGetEOS(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "Ogg.endOfStream() takes one argument (the channel)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);

	if (AalibGetStopReason(PSPAALIB_CHANNEL_OGG_1 + channel) == PSPAALIB_WARNING_END_OF_STREAM_REACHED)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
 *
 ---------			AT3 Functions			---------
 *
 */

//Load a At3
static int lua_At3Load(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "At3.load(filename, channel) takes 2 arguments : (The Filename, The Channel)");
	}
	
	const char *path = luaL_checkstring(L, 1);
	unsigned int channel = luaL_checkint(L, 2);
	
	char fullpath[512];
	getcwd(fullpath, 256);
	strcat(fullpath, "/");
	strcat(fullpath, path);
	
	lua_gc(L, LUA_GCCOLLECT, 0);
	
	if (AalibLoad(fullpath, PSPAALIB_CHANNEL_AT3_1 + channel, false) != 0)
	{	
		return luaL_error(L, "Cannot find the requested At3 file - Check your path and make sure the file exists");
	}
	
	if (PSPAALIB_CHANNEL_AT3_1 + channel >= PSPAALIB_CHANNEL_AT3_1 + MAX_MUSIC_CHANNELS)
	{
		return luaL_error(L, "At3 sound channel unavailable - You are using over the maximum 2 allowed channels for At3 files");
	}
	return 0;
}

//Play a At3 though the PSP MEDIA ENGINE
static int lua_At3Play(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "At3.play() takes 2 arguments : (loop - true or false, channel)");
	}
	
	bool loop;
	loop = lua_toboolean(L, 1);
	unsigned int channel = luaL_checkint(L, 2);
	
	AalibPlay(PSPAALIB_CHANNEL_AT3_1 + channel);
	AalibSetAutoloop(PSPAALIB_CHANNEL_AT3_1 + channel, loop);
	
	return 0;
}
	
//Stop a currently playing At3
static int lua_At3Stop(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "At3.stop(channel) takes 1 argument (the channel to stop)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);
	
	AalibStop(PSPAALIB_CHANNEL_AT3_1 + channel);
	
	return 0;
}

//Pause a currently playing At3
static int lua_At3Pause(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "At3.pause(channel) takes 1 argument (the channel to stop)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);
	
	AalibPause(PSPAALIB_CHANNEL_AT3_1 + channel);
	
	return 0;
}

//Unload a loaded At3
static int lua_At3Unload(lua_State *L)
{	
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "At3.unload() takes 1 argument (the channel to stop)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);
	
	AalibUnload(PSPAALIB_CHANNEL_AT3_1 + channel);
	
	return 0;
}

//Set/Change the Volume of the At3
static int lua_At3Volume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "At3.volume(vol, channel) takes one argument (the volume in float, the channel)");
	}

	float vol = luaL_checknumber(L, 1);	
	unsigned int channel = luaL_checkint(L, 2);
	lua_pushnumber(L, vol);
		
	AalibEnable(PSPAALIB_CHANNEL_AT3_1 + channel, PSPAALIB_EFFECT_VOLUME_MANUAL);
	AalibSetVolume(PSPAALIB_CHANNEL_AT3_1 + channel, ((AalibVolume){(vol,vol)}));
	
	return 1;	
}

//Set the speed/tempo of the At3
static int lua_At3SetSpeed(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "At3.speed(speed) takes two arguments (the speed in float, the channel)");
	}

	float speed = luaL_checknumber(L, 1);	
	unsigned int channel = luaL_checkint(L, 2);
	lua_pushnumber(L, speed);
		
	AalibEnable(PSPAALIB_CHANNEL_AT3_1 + channel, PSPAALIB_EFFECT_PLAYSPEED);
	AalibSetPlaySpeed(PSPAALIB_CHANNEL_AT3_1 + channel, speed);

	return 1;	
}

//Return end of At3 steam
static int lua_At3GetEOS(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "At3.endOfStream() takes one argument (the channel)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);

	if (AalibGetStopReason(PSPAALIB_CHANNEL_AT3_1 + channel) == PSPAALIB_WARNING_END_OF_STREAM_REACHED)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
 *
 ---------			WAV Functions			---------
 *
 */

//Load a Wav
static int lua_WavLoad(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "Wav.load(filename, channel) takes 2 arguments : (The Filename, The Channel)");
	}
	
	const char *path = luaL_checkstring(L, 1);
	unsigned int channel = luaL_checkint(L, 2);
	
	char fullpath[512];
	getcwd(fullpath, 256);
	strcat(fullpath, "/");
	strcat(fullpath, path);
	
	lua_gc(L, LUA_GCCOLLECT, 0);
	
	if (AalibLoad(fullpath, PSPAALIB_CHANNEL_WAV_1 + channel, true) != 0)
	{	
		return luaL_error(L, "Cannot find the requested Wav file - Check your path and make sure the file exists");
	}
	
	if (PSPAALIB_CHANNEL_WAV_1 + channel >= PSPAALIB_CHANNEL_WAV_1 + MAX_SOUND_CHANNELS)
	{
		return luaL_error(L, "Wav sound channel unavailable - You are using over the maximum 30 allowed channels for Wav files");
	}
	return 0;
}

//Play a Wav though the PSP MEDIA ENGINE
static int lua_WavPlay(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "Wav.play() takes 2 arguments : (loop - true or false, channel)");
	}
	
	bool loop;
	loop = lua_toboolean(L, 1);
	unsigned int channel = luaL_checkint(L, 2);
	
	AalibPlay(PSPAALIB_CHANNEL_WAV_1 + channel);
	AalibSetAutoloop(PSPAALIB_CHANNEL_WAV_1 + channel, loop);
	
	return 0;
}
	
//Stop a currently playing Wav
static int lua_WavStop(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "Wav.stop(channel) takes 1 argument (the channel to stop)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);
	
	AalibStop(PSPAALIB_CHANNEL_WAV_1 + channel);
	
	return 0;
}

//Pause a currently playing Wav
static int lua_WavPause(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "Wav.pause(channel) takes 1 argument (the channel to stop)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);
	
	AalibPause(PSPAALIB_CHANNEL_WAV_1 + channel);
	
	return 0;
}

//Unload a loaded Wav
static int lua_WavUnload(lua_State *L)
{	
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "Wav.unload() takes 1 argument (the channel to stop)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);
	
	AalibUnload(PSPAALIB_CHANNEL_WAV_1 + channel);
	
	return 0;
}

//Set/Change the Volume of the Wav
static int lua_WavVolume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "Wav.volume(vol, channel) takes one argument (the volume in float, the channel)");
	}

	float vol = luaL_checknumber(L, 1);	
	unsigned int channel = luaL_checkint(L, 2);
	lua_pushnumber(L, vol);
		
	AalibEnable(PSPAALIB_CHANNEL_WAV_1 + channel, PSPAALIB_EFFECT_VOLUME_MANUAL);
	AalibSetVolume(PSPAALIB_CHANNEL_WAV_1 + channel, ((AalibVolume){(vol,vol)}));
	
	return 1;	
}

//Set the speed/tempo of the Wav
static int lua_WavSetSpeed(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2) 
	{	
		return luaL_error(L, "Wav.speed(speed) takes two arguments (the speed in float, the channel)");
	}

	float speed = luaL_checknumber(L, 1);	
	unsigned int channel = luaL_checkint(L, 2);
	lua_pushnumber(L, speed);
		
	AalibEnable(PSPAALIB_CHANNEL_WAV_1 + channel, PSPAALIB_EFFECT_PLAYSPEED);
	AalibSetPlaySpeed(PSPAALIB_CHANNEL_WAV_1 + channel, speed);

	return 1;	
}

//Return end of Wav steam
static int lua_WavGetEOS(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) 
	{	
		return luaL_error(L, "Wav.endOfStream() takes one argument (the channel)");
	}
	
	unsigned int channel = luaL_checkint(L, 1);

	if (AalibGetStopReason(PSPAALIB_CHANNEL_WAV_1 + channel) == PSPAALIB_WARNING_END_OF_STREAM_REACHED)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}



/*
 *
 ---------			MOD,XM,IT Functions			---------
 *
 */
	
//Register our Mp3 Functions
static const luaL_reg MP3Functions[] = {
  	{"load",          	lua_Mp3Load},
	{"play",           	lua_Mp3Play},
	{"stop", 			lua_Mp3Stop},
	{"pause",			lua_Mp3Pause},
 	{"unload",		 	lua_Mp3Unload},
 	{"volume", 			lua_Mp3Volume},
 	{"speed", 			lua_Mp3SetSpeed},
 	{"endOfStream", 	lua_Mp3GetEOS},
  	{0, 0}
};

//Register our Ogg Functions
static const luaL_reg OGGFunctions[] = {
  	{"load",          	lua_OggLoad},
	{"play",           	lua_OggPlay},
	{"stop", 			lua_OggStop},
	{"pause",			lua_OggPause},
 	{"unload",		 	lua_OggUnload},
 	{"volume", 			lua_OggVolume},
 	{"speed", 			lua_OggSetSpeed},
 	{"endOfStream", 	lua_OggGetEOS},
  	{0, 0}
};

//Register our At3 Functions
static const luaL_reg AT3Functions[] = {
  	{"load",          	lua_At3Load},
	{"play",           	lua_At3Play},
	{"stop", 			lua_At3Stop},
	{"pause",			lua_At3Pause},
 	{"unload",		 	lua_At3Unload},
 	{"volume", 			lua_At3Volume},
 	{"speed", 			lua_At3SetSpeed},
 	{"endOfStream", 	lua_At3GetEOS},
  	{0, 0}
};

//Register our Wav Functions
static const luaL_reg WAVFunctions[] = {
  	{"load",          	lua_WavLoad},
	{"play",           	lua_WavPlay},
	{"stop", 			lua_WavStop},
	{"pause",			lua_WavPause},
 	{"unload",		 	lua_WavUnload},
 	{"volume", 			lua_WavVolume},
 	{"speed", 			lua_WavSetSpeed},
 	{"endOfStream", 	lua_WavGetEOS},
  	{0, 0}
};

void luaSound_init(lua_State *L) {
	luaL_openlib(L, "Mp3", MP3Functions, 0);
	luaL_openlib(L, "Ogg", OGGFunctions, 0);
	luaL_openlib(L, "At3", AT3Functions, 0);
	luaL_openlib(L, "Wav", WAVFunctions, 0);
}
