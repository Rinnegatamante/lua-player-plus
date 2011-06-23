#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "libs/aalib/pspaalib.h"
#include "include/luaplayer.h"
#include "libs/sounds/pspaudiolib.h"
#include "libs/sounds/mp3playerME.h"
#include "libs/sounds/aa3playerME.h"

#define MAX_MUSIC_CHANNELS 2
#define MAX_SOUND_CHANNELS 30
#define false 0
#define true 1
/*
 *
 ---------            MP3 Functions            ---------
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
 ---------            OGG Functions            ---------
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
 ---------            AT3 Functions            ---------
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
 ---------            WAV Functions            ---------
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

// aa3me  functions
bool aa3meplaying = false;

static int aa3me_load(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments");
    
    const char *path = luaL_checkstring(L, 1);
    
    char fullpath[512];
    getcwd(fullpath, 256);
    strcat(fullpath, "/");
    strcat(fullpath, path);

    FILE* soundFile = fopen(fullpath, "rb");
    if (!soundFile) return luaL_error(L, "can't open sound file %s.", fullpath);
    fclose(soundFile);
    lua_gc(L, LUA_GCCOLLECT, 0);
    if (aa3meplaying == false){
    AA3ME_Init(1);
    AA3ME_Load(fullpath);
    sceKernelDelayThread(10000);
    }
    return 0;
}

static int aa3me_play(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
    if (aa3meplaying == false){
    AA3ME_Play();
    aa3meplaying = true;
    }
    return 0;
}
static int aa3me_Stop(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
    if (aa3meplaying == true){
    AA3ME_End();
    aa3meplaying = false;
    }
    return 0;
}
static int aa3me_EndOfStream(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
    if (AA3ME_EndOfStream() == 1) { 
        lua_pushstring(L, "true");
    } else {
        lua_pushstring(L, "false");
    }
    return 1;
}
static int aa3me_getTime(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
    char aa3time[200];
    char aa3timefeed[200];
    if (aa3meplaying == true){
    AA3ME_GetTimeString(aa3time);
    sprintf(aa3timefeed, "%s", aa3time);
    lua_pushstring(L, aa3timefeed);
    }
    return 1;
}
static int aa3me_percent(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
        char percent[200];
    if (aa3meplaying == true){
    AA3ME_GetPercentage();
    lua_pushstring(L, percent);
    }
    return 1;
}
static int aa3me_pause(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
    if (aa3meplaying == true){
    AA3ME_Pause();
    }
    return 0;
}

// MP3ME  functions
int mp3meplaying = false;

static int Mp3me_load(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments");
    
    const char *path = luaL_checkstring(L, 1);
    
    char fullpath[512];
    getcwd(fullpath, 256);
    strcat(fullpath, "/");
    strcat(fullpath, path);

    FILE* soundFile = fopen(fullpath, "rb");
    if (!soundFile) return luaL_error(L, "can't open sound file %s.", fullpath);
    fclose(soundFile);
    lua_gc(L, LUA_GCCOLLECT, 0);
    if (mp3meplaying == false){
    MP3ME_Init(1);
    MP3ME_Load(fullpath);
    sceKernelDelayThread(10000);
    }
    return 0;
}

static int Mp3me_play(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
    if (mp3meplaying == false){
    MP3ME_Play();
    mp3meplaying = true;
    }
    return 0;
}
static int Mp3me_Stop(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
    if (mp3meplaying == true){
    MP3ME_End();
    mp3meplaying = false;
    }
    return 0;
}
static int Mp3me_EndOfStream(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
    if (MP3ME_EndOfStream() == 1) { 
        lua_pushstring(L, "true");
    } else {
        lua_pushstring(L, "false");
    }
    return 1;
}
static int Mp3me_getTime(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
    char mp3time[200];
    char mp3timefeed[200];
    if (mp3meplaying == true){
    MP3ME_GetTimeString(mp3time);
    sprintf(mp3timefeed, "%s", mp3time);
    lua_pushstring(L, mp3timefeed);
    }
    return 1;
}
static int Mp3me_percent(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
        char percent[200];
    if (mp3meplaying == true){
    MP3ME_GetPercentage();
    lua_pushstring(L, percent);
    }
    return 1;
}
static int Mp3me_pause(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
    if (mp3meplaying == true){
    MP3ME_Pause();
    }
    return 0;
}

//Register or MP3Me Functions
static const luaL_reg Mp3me_functions[] = {
  {"load",                Mp3me_load},
  {"play",                Mp3me_play},
  {"stop",                Mp3me_Stop},
  {"eos",                Mp3me_EndOfStream},
  {"gettime",                Mp3me_getTime},
  {"percent",                Mp3me_percent},
  {"pause",                Mp3me_pause},
  {0, 0}
};

//Register our AA3Me Functions
static const luaL_reg Aa3me_functions[] = {
  {"load",                aa3me_load},
  {"play",                aa3me_play},
  {"stop",                aa3me_Stop},
  {"eos",                aa3me_EndOfStream},
  {"gettime",                aa3me_getTime},
  {"percent",                aa3me_percent},
  {"pause",                aa3me_pause},
  {0, 0}
};

//Register our Mp3 Functions
static const luaL_reg MP3Functions[] = {
      {"load",              lua_Mp3Load},
    {"play",               lua_Mp3Play},
    {"stop",             lua_Mp3Stop},
    {"pause",            lua_Mp3Pause},
     {"unload",             lua_Mp3Unload},
     {"volume",             lua_Mp3Volume},
     {"speed",             lua_Mp3SetSpeed},
     {"endOfStream",     lua_Mp3GetEOS},
      {0, 0}
};

//Register our Ogg Functions
static const luaL_reg OGGFunctions[] = {
      {"load",              lua_OggLoad},
    {"play",               lua_OggPlay},
    {"stop",             lua_OggStop},
    {"pause",            lua_OggPause},
     {"unload",             lua_OggUnload},
     {"volume",             lua_OggVolume},
     {"speed",             lua_OggSetSpeed},
     {"endOfStream",     lua_OggGetEOS},
      {0, 0}
};

//Register our At3 Functions
static const luaL_reg AT3Functions[] = {
      {"load",              lua_At3Load},
    {"play",               lua_At3Play},
    {"stop",             lua_At3Stop},
    {"pause",            lua_At3Pause},
     {"unload",             lua_At3Unload},
     {"volume",             lua_At3Volume},
     {"speed",             lua_At3SetSpeed},
     {"endOfStream",     lua_At3GetEOS},
      {0, 0}
};

//Register our Wav Functions
static const luaL_reg WAVFunctions[] = {
      {"load",              lua_WavLoad},
    {"play",               lua_WavPlay},
    {"stop",             lua_WavStop},
    {"pause",            lua_WavPause},
     {"unload",             lua_WavUnload},
     {"volume",             lua_WavVolume},
     {"speed",             lua_WavSetSpeed},
     {"endOfStream",     lua_WavGetEOS},
      {0, 0}
};

void luaSound_init(lua_State *L) {
    luaL_openlib(L, "Mp3", MP3Functions, 0);
    luaL_openlib(L, "Ogg", OGGFunctions, 0);
    luaL_openlib(L, "At3", AT3Functions, 0);
    luaL_openlib(L, "Wav", WAVFunctions, 0);
}

