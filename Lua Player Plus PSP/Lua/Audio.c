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

#include "../LPP.h"
#include "Audio.h"

static int luaMp3_load(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Mp3.load(filename, channel) takes 2 arguments.");
    }
    u32 channel = CLAMP(luaL_checkint(L, 2),0,2);
    L_CONST char *path = luaL_checkstring(L, 1);

    if(AalibLoad((char*)path, PSPAALIB_CHANNEL_SCEMP3_1 + channel, 0) != 0)
    {
        return luaL_error(L, "Cannot load the sound '%s'.", path);
    }

    return 0;
}

static int luaMp3_play(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Mp3.play(loop, channel) takes 2 arguments.");
    }
    u32 channel = CLAMP(luaL_checkint(L, 2),0,2);
    AalibPlay(PSPAALIB_CHANNEL_SCEMP3_1 + channel);
    AalibSetAutoloop(PSPAALIB_CHANNEL_SCEMP3_1 + channel, lua_toboolean(L, 1));
    return 0;
}

static int luaMp3_stop(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Mp3.stop(channel) takes 1 argument.");
    }
    u32 channel = CLAMP(luaL_checkint(L,1),0,2);

    AalibStop(PSPAALIB_CHANNEL_SCEMP3_1 + channel);
    AalibRewind(PSPAALIB_CHANNEL_SCEMP3_1 + channel);

    return 0;
}

static int luaMp3_pause(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Mp3.pause(channel) takes 1 argument.");
    }

    AalibPause(PSPAALIB_CHANNEL_SCEMP3_1 + CLAMP(luaL_checkint(L,1),0,2));

    return 0;
}

static int luaMp3_volume(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Mp3.volume(vol, channel) takes 2 arguments.");
    }
    float vol = luaL_checknumber(L, 1);
    u32 channel = CLAMP(luaL_checkint(L, 2),0,2);

    AalibEnable(PSPAALIB_CHANNEL_SCEMP3_1 + channel, PSPAALIB_EFFECT_VOLUME_MANUAL);
    AalibSetVolume((PSPAALIB_CHANNEL_SCEMP3_1 + channel), ((AalibVolume){vol,vol}));

    return 0;
}

static int luaMp3_unload(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Mp3.unload(channel) takes 1 argument.");
    }
    AalibUnload(PSPAALIB_CHANNEL_SCEMP3_1 + CLAMP(luaL_checkint(L, 1),0,2));
    return 0;
}

static int luaMp3_speed(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Mp3.speed(speed, channel) takes 2 arguments.");
    }

    float speed = luaL_checknumber(L, 1);
    u32 channel = CLAMP(luaL_checkint(L, 2),0,2);

    AalibEnable(PSPAALIB_CHANNEL_SCEMP3_1 + channel, PSPAALIB_EFFECT_PLAYSPEED);
    AalibSetPlaySpeed(PSPAALIB_CHANNEL_SCEMP3_1 + channel, speed);

    return 0;
}

static int luaMp3_eos(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Mp3.eos(channel) takes 1 argument.");
    }
    u32 channel = CLAMP(luaL_checkint(L, 1),0,2);

    lua_pushboolean(L, AalibGetStopReason(PSPAALIB_CHANNEL_SCEMP3_1 + channel) == PSPAALIB_WARNING_END_OF_STREAM_REACHED);

    return 1;
}

static L_CONST luaL_reg luaMp3_methods[] = {
    { "load", luaMp3_load },
    { "stop", luaMp3_stop },
    { "pause", luaMp3_pause },
    { "speed", luaMp3_speed },
    { "volume", luaMp3_volume },
    { "eos", luaMp3_eos },
    { "play", luaMp3_play },
    { "unload", luaMp3_unload },
    { 0, 0 }
};

static int luaOgg_load(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Ogg.load(filename, channel) takes 2 arguments.");
    }
    L_CONST char *path = luaL_checkstring(L, 1);
    u32 channel = CLAMP(luaL_checkint(L, 2), 0,2);

    if(AalibLoad((char*)path, PSPAALIB_CHANNEL_OGG_1 + channel, 0) != 0)
    {
        return luaL_error(L, "Cannot load sound '%s'", path);
    }

    return 0;
}

static int luaOgg_play(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Ogg.play(loop, channel) takes 2 arguments.");
    }

    u32 channel = CLAMP(luaL_checkint(L,2),0,2);

    AalibPlay(PSPAALIB_CHANNEL_OGG_1 + channel);
    AalibSetAutoloop(PSPAALIB_CHANNEL_OGG_1 + channel, lua_toboolean(L, 1));

    return 0;
}

static int luaOgg_stop(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Ogg.stop(channel) takes 1 argument.");
    }

    u32 channel = CLAMP(luaL_checkint(L,1),0,2);

    AalibStop(PSPAALIB_CHANNEL_OGG_1 + channel);

    return 0;
}

static int luaOgg_pause(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Ogg.pause(channel) takes 1 argument.");
    }
    u32 channel = CLAMP(luaL_checkint(L,1),0,2);

    AalibPause(PSPAALIB_CHANNEL_OGG_1 + channel);

    return 0;
}

static int luaOgg_unload(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Ogg.unload(channel) takes 1 argument.");
    }

    u32 channel = CLAMP(luaL_checkint(L,1),0,2);
    AalibUnload(PSPAALIB_CHANNEL_OGG_1 + channel);

    return 0;
}

static int luaOgg_volume(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Ogg.volume(volume, channel) takes 2 arguments.");
    }

    float volume = luaL_checknumber(L, 1);
    u32 channel = CLAMP(luaL_checkint(L, 2), 0, 2);

    AalibEnable(PSPAALIB_CHANNEL_OGG_1 + channel, PSPAALIB_EFFECT_VOLUME_MANUAL);
    AalibSetVolume(PSPAALIB_CHANNEL_OGG_1 + channel, ((AalibVolume){volume, volume }));

    return 0;
}

static int luaOgg_speed(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Ogg.speed(speed, channel) takes 2 arguments.");
    }

    float speed = luaL_checknumber(L, 1);
    u32 channel = CLAMP(luaL_checkint(L,2), 0, 2);

    AalibEnable(PSPAALIB_CHANNEL_OGG_1 + channel, PSPAALIB_EFFECT_PLAYSPEED);
	AalibSetPlaySpeed(PSPAALIB_CHANNEL_OGG_1 + channel, speed);

	return 0;
}

static int luaOgg_eos(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Ogg.eos(channel) takes 1 argument.");
    }
    u32 channel = CLAMP(luaL_checkint(L,1),0,2);

    lua_pushboolean(L, AalibGetStopReason(PSPAALIB_CHANNEL_OGG_1 + channel) == PSPAALIB_WARNING_END_OF_STREAM_REACHED);

    return 1;
}

static L_CONST luaL_reg luaOgg_methods[] = {
    { "load", luaOgg_load },
    { "stop", luaOgg_stop },
    { "pause", luaOgg_pause },
    { "speed", luaOgg_speed },
    { "volume", luaOgg_volume },
    { "eos", luaOgg_eos },
    { "play", luaOgg_play },
    { "unload", luaOgg_unload },
    { 0, 0 }
};

static int luaAt3_load(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "At3.load(filename, channel) takes 2 arguments.");
    }

    u32 channel = CLAMP(luaL_checkint(L, 2), 0, 2);
    L_CONST char *path = luaL_checkstring(L, 1);

    if(AalibLoad((char*)path, PSPAALIB_CHANNEL_AT3_1 + channel, 0) != 0)
    {
        return luaL_error(L, "Cannot load the sound '%s'.", path);
    }

    return 0;
}

static int luaAt3_play(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "At3.play(loop, channel) takes 2 arguments.");
    }

    u32 channel = CLAMP(luaL_checkint(L, 2), 0, 2);

    AalibPlay(PSPAALIB_CHANNEL_AT3_1 + channel);
    AalibSetAutoloop(PSPAALIB_CHANNEL_AT3_1 + channel, lua_toboolean(L, 1));

    return 0;
}

static int luaAt3_stop(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "At3.stop(channel) takes 1 argument.");
    }

    u32 channel = CLAMP(luaL_checkint(L, 1), 0 , 2);

    AalibStop(PSPAALIB_CHANNEL_AT3_1 + channel);

    return 0;
}

static int luaAt3_pause(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "At3.pause(channel) takes 1 argument.");
    }
    u32 channel = CLAMP(luaL_checkint(L, 1), 0, 2);

    AalibPause(PSPAALIB_CHANNEL_AT3_1 + channel);

    return 0;
}

static int luaAt3_unload(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "At3.unload(channel) takes 1 argument.");
    }
    u32 channel = CLAMP(luaL_checkint(L, 1), 0, 2);

    AalibUnload(PSPAALIB_CHANNEL_AT3_1 + channel);

    return 0;
}

static int luaAt3_volume(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "At3.volume(volume, channel) takes 2 arguments.");
    }

    float vol = luaL_checknumber(L, 1);
    u32 channel = CLAMP(luaL_checkint(L, 2), 0, 2);

    AalibEnable(PSPAALIB_CHANNEL_AT3_1 + channel, PSPAALIB_EFFECT_VOLUME_MANUAL);
    AalibSetVolume(PSPAALIB_CHANNEL_AT3_1 + channel, ((AalibVolume){vol,vol}));

    return 1;
}

static int luaAt3_speed(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "At3.speed(speed, channel) takes 2 arguments.");
    }

    float speed = luaL_checknumber(L, 1);
    u32 channel = CLAMP(luaL_checkint(L,2), 0, 2);

    AalibEnable(PSPAALIB_CHANNEL_AT3_1 + channel, PSPAALIB_EFFECT_PLAYSPEED);
    AalibSetPlaySpeed(PSPAALIB_CHANNEL_AT3_1 + channel, speed);

    return 0;
}

static int luaAt3_eos(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "At3.eos(channel) takes 1 argument.");
    }
    u32 channel = CLAMP(luaL_checkint(L,1),0,2);

    lua_pushboolean(L, AalibGetStopReason(PSPAALIB_CHANNEL_AT3_1 + channel) == PSPAALIB_WARNING_END_OF_STREAM_REACHED);

    return 1;
}

static L_CONST luaL_reg luaAt3_methods[] = {
    { "load", luaAt3_load },
    { "stop", luaAt3_stop },
    { "pause", luaAt3_pause },
    { "speed", luaAt3_speed },
    { "volume", luaAt3_volume },
    { "eos", luaAt3_eos },
    { "play", luaAt3_play },
    { "unload", luaAt3_unload },
    { 0, 0 }
};

static int luaWav_load(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Wav.load(filename, channel) takes 2 arguments.");
    }
    L_CONST char *path = luaL_checkstring(L, 1);
    u32 channel = CLAMP(luaL_checkint(L, 2), 0, 30);

    if(AalibLoad((char*)path, PSPAALIB_CHANNEL_WAV_1 + channel, 0) != 0)
    {
        return luaL_error(L, "Cannot load the sound '%s'.", path);
    }

    return 0;
}

static int luaWav_play(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Wav.play(loop, channel) takes 2 arguments.");
    }

    u32 channel = CLAMP(luaL_checkint(L, 2), 0, 30);

    AalibPlay(PSPAALIB_CHANNEL_WAV_1 + channel);
    AalibSetAutoloop(PSPAALIB_CHANNEL_WAV_1 + channel, lua_toboolean(L,1));

    return 0;
}

static int luaWav_stop(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Wav.stop(channel) takes 1 argument.");
    }
    u32 channel = CLAMP(luaL_checkint(L,1),0,30);

    AalibStop(PSPAALIB_CHANNEL_WAV_1 + channel);

    return 0;
}

static int luaWav_pause(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Wav.pause(channel) takes 1 argument.");
    }

    u32 channel = CLAMP(luaL_checkint(L,1),0,30);

    AalibPause(PSPAALIB_CHANNEL_WAV_1 + channel);

    return 0;
}

static int luaWav_speed(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Wav.speed(speed, channel) takes 2 arguments.");
    }
    u32 channel = CLAMP(luaL_checkint(L,2),0,30);
    float speed = luaL_checknumber(L,1);

    AalibEnable(PSPAALIB_CHANNEL_WAV_1 + channel, PSPAALIB_EFFECT_PLAYSPEED);
    AalibSetPlaySpeed(PSPAALIB_CHANNEL_WAV_1 + channel, speed);

    return 0;
}

static int luaWav_volume(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Wav.volume(volume, channel) takes 2 arguments.");
    }
    u32 channel = CLAMP(luaL_checkint(L,2),0,30);
    float vol = luaL_checknumber(L,1);

    AalibEnable(PSPAALIB_CHANNEL_WAV_1 + channel, PSPAALIB_EFFECT_VOLUME_MANUAL);
    AalibSetVolume(PSPAALIB_CHANNEL_WAV_1 + channel, ((AalibVolume){vol,vol}));

    return 0;
}

static int luaWav_eos(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Wav.eos(channel) takes 1 argument.");
    }
    u32 channel = CLAMP(luaL_checkint(L,1),0,30);
    lua_pushboolean(L, AalibGetStopReason(PSPAALIB_CHANNEL_WAV_1 + channel) == PSPAALIB_WARNING_END_OF_STREAM_REACHED);

    return 1;
}

static int luaWav_unload(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Wav.unload(channel) takes 1 argument.");
    }
    u32 channel = CLAMP(luaL_checkint(L,1),0,30);
    AalibUnload(PSPAALIB_CHANNEL_WAV_1 + channel);

    return 0;
}

static L_CONST luaL_reg luaWav_methods[] = {
    { "load", luaWav_load },
    { "stop", luaWav_stop },
    { "pause", luaWav_pause },
    { "speed", luaWav_speed },
    { "volume", luaWav_volume },
    { "eos", luaWav_eos },
    { "play", luaWav_play },
    { "unload", luaWav_unload },
    { 0, 0 }
};

void luaSound_Init(lua_State *L)
{
    lua_newtable(L);
    luaL_register(L, 0, luaMp3_methods);
    lua_setglobal(L, "Mp3");

    lua_newtable(L);
    luaL_register(L, 0, luaOgg_methods);
    lua_setglobal(L, "Ogg");

    lua_newtable(L);
    luaL_register(L, 0, luaAt3_methods);
    lua_setglobal(L, "At3");

    lua_newtable(L);
    luaL_register(L, 0, luaWav_methods);
    lua_setglobal(L, "Wav");
}
