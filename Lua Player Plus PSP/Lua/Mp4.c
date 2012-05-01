#include "Mp4.h"
#include "../Libs/Mp4/Mp4.h"

UserdataStubs(Mp4, LPP_Mp4*);

static int luaMp4_init(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Mp4.init(mpegVsh, cooleyesBridge) takes 2 arguments.");
    }
    int ret = LPP_Mp4Init(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
    lua_pushboolean(L, !(ret < 0));

    return(1);
}

static int luaMp4_shutdown(lua_State *L)
{
    lua_pushboolean(L, !(LPP_Mp4Shutdown() < 0));
    return 1;
}

static int luaMp4_load(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Mp4.load(filename) takes 1 arguments.");
    }
    LPP_Mp4 *ret = LPP_Mp4Load(luaL_checkstring(L, 1));
    if (ret == null)
    {
        return luaL_error(L, "Cannot load the video '%s'.", luaL_checkstring(L, 1));
    }
    *pushMp4(L) = ret;

    return(1);
}

static int luaMp4_play(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Mp4:play(frameN) takes 2 arguments and must be called with a colon.");
    }
    LPP_Mp4Play(*toMp4(L,1), luaL_checknumber(L, 2));

    return 0;
}

static int luaMp4_getInfo(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Mp4:getInfo() takes no arguments and must be called with a colon.");
    }
    LPP_Mp4 *vd = *toMp4(L, 1);

    lua_newtable(L);

    lua_pushstring(L, "frames_count");
    lua_pushnumber(L, vd->reader.file.number_of_video_frames);
    lua_settable(L, -3);

    lua_pushstring(L, "width");
    lua_pushnumber(L, vd->reader.file.video_width);
    lua_settable(L, -3);

    lua_pushstring(L, "height");
    lua_pushnumber(L, vd->reader.file.video_height);
    lua_settable(L, -3);

    lua_pushstring(L, "type");
    lua_pushnumber(L, vd->reader.file.video_type);
    lua_settable(L, -3);

    lua_pushstring(L, "video_rate");
    lua_pushnumber(L, vd->reader.file.video_rate);
    lua_settable(L, -3);

    lua_pushstring(L, "video_scale");
    lua_pushnumber(L, vd->reader.file.video_scale);
    lua_settable(L, -3);

    lua_pushstring(L, "audio_type");
    lua_pushnumber(L, vd->reader.file.audio_type);
    lua_settable(L, -3);

    return 1;
}

static int luaMp4__gc(lua_State *L)
{
    LPP_Mp4Close(*toMp4(L,1));
    return 0;
}

static L_CONST luaL_reg luaMp4_methods[] = {
    { "init", luaMp4_init },
    { "shutdown", luaMp4_shutdown },
    { "load", luaMp4_load },
    { "play", luaMp4_play },
    { "getInfo", luaMp4_getInfo },
    { 0, 0 }
};

static L_CONST luaL_reg luaMp4_meta[] = {
    { "__gc", luaMp4__gc },
    { 0, 0 }
};

UserdataRegister(Mp4, luaMp4_methods, luaMp4_meta);

void luaMp4_Init(lua_State *L)
{
    Mp4_register(L);
}
