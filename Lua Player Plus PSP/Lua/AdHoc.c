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

#include "../Libs/AdHoc/AdHoc.h"
#include "../LPP.h"

UserdataStubs(Adhoc, struct LPP_RemotePSP*);

static int luaAdhoc_init(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "Adhoc.init(productID) takes 1 argument.");
	}
	lua_pushnumber(L, LPP_AdhocInit((char*)luaL_checkstring(L, 1)));
	return(1);
}

static int luaAdhoc_getMacAddr(lua_State *L)
{
    if(lua_gettop(L) != 0)
	{
	    return luaL_error(L, "Adhoc.getMacAddr() takes no arguments.");
	}
	lua_pushstring(L, (L_CONST char*)LPP_Adhoc_GetMacAddress());
	return(1);
}

static int luaAdhoc_getRemotePSPCount(lua_State *L)
{
    if(lua_gettop(L) != 0)
	{
	    return luaL_error(L, "Adhoc.getRemotePSPCount() takes no arguments.");
	}
	lua_pushnumber(L, LPP_Adhoc_GetRemotePspCount());
	return(1);
}

static int luaAdhoc_getPSPByMac(lua_State *L)
{
    if(lua_gettop(L) != 0)
	{
	    return luaL_error(L, "Adhoc.getPSPByMac(mac) takes 1 arguments.");
	}
	struct LPP_RemotePSP *r = LPP_Adhoc_GetPspByMac((u8*)luaL_checkstring(L, 1));
	if(r == null)
	{
	    return luaL_error(L, "Cannot get a remote psp with this mac address : %s", luaL_checkstring(L, 1));
	}
	*pushAdhoc(L) = r;
	return(1);
}

static int luaAdhoc_getPSPByIndex(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "Adhoc.getPSPByIndex(index) takes 1 argument.");
	}
	struct LPP_RemotePSP *r = LPP_Adhoc_GetPspByIndex(luaL_checknumber(L, 1));
	if(r == null)
	{
	    return luaL_error(L, "Cannot get a remote psp with this index : %d", luaL_checknumber(L, 1));
	}
	*pushAdhoc(L) = r;
	return(1);
}

static int luaAdhoc_requestConnection(lua_State *L)
{
    if(lua_gettop(L) != 2)
	{
	    return luaL_error(L, "Adhoc.requestConnection(remotePSP, timeout) takes 2 arguments.");
	}
	lua_pushnumber(L, LPP_AdhocRequestConnection(*toAdhoc(L, 1), luaL_checknumber(L, 2), null));
	return(1);
}

static int luaAdhoc_sendData(lua_State *L)
{
    if(lua_gettop(L) != 2)
	{
	    return luaL_error(L, "Adhoc.sendData(remotePSP, data) takes 2 arguments.");
	}
	L_CONST char *data = luaL_checkstring(L, 2);
	u32 dataLen = strlen(data);
	
	lua_pushnumber(L, LPP_Adhoc_SendData(*toAdhoc(L, 1), (void*)data, dataLen));
	return(1);
}

static int luaAdhoc_recvData(lua_State *L)
{
    if(lua_gettop(L) != 2)
	{
	    return luaL_error(L, "Adhoc.recvData(remotePSP, maxLen) takes 2 arguments.");
	}
	L_CONST char *data = null;
	LPP_Adhoc_ReceiveData(*toAdhoc(L,1), (void*)data, luaL_checknumber(L, 2));
	lua_pushstring(L, data);
	return(1);
}

static int luaAdhoc_getConnectionRequest(lua_State *L)
{
    if(lua_gettop(L) != 0)
	{
	    return luaL_error(L, "Adhoc.getConnectionRequest() takes no arguments.");
	}
	*pushAdhoc(L) = LPP_Adhoc_GetConnectionRequest();
	return(1);
}

static int luaAdhoc_rejectConnection(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L ,"Adhoc.rejectConnection(remotePSP) takes 1 argument.");
	}
	LPP_Adhoc_RejectConnection(*toAdhoc(L, 1));
	return 0;
}

static int luaAdhoc_acceptConnection(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "Adhoc.acceptConnection(remotePSP) takes 1 argument.");
	}
	LPP_Adhoc_AcceptConnection(*toAdhoc(L, 1));
	return 0;
}

static int luaAdhoc_term(lua_State *L)
{
    if(lua_gettop(L) != 0)
	{
	    return luaL_error(L, "Adhoc.term() takes no arguments.");
	}
	LPP_Adhoc_Term();
	return 0;
}

static int luaAdhoc_name(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "remotePSP:name() takes no arguments and must be called with a colon.");
	}
	struct LPP_RemotePSP *r = *toAdhoc(L, 1);
	lua_pushstring(L, r->name);
	return(1);
}

static int luaAdhoc_mac(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "remotePSP:mac() takes no arguments and must be called with a colon.");
	}
	struct LPP_RemotePSP *r = *toAdhoc(L, 1);
	lua_pushstring(L, (L_CONST char*)r->macAddress);
	return(1);
}

static int luaAdhoc_connectionState(lua_State *L)
{
    Int16 arg = lua_gettop(L);
    if(arg != 0 && arg != 1)
	{
	    return luaL_error(L, "remotePSP:connectionState() ( or Adhoc.connectionState() ) takes no arguments and must be called with a colon.");
	}
	
	if(arg == 1) {
	    struct LPP_RemotePSP *r = *toAdhoc(L, 1);
	    lua_pushnumber(L, r->connectionState);
	} else {
	    lua_pushnumber(L, LPP_Adhoc_GetConnectionState());
	}
	return(1);
}

static int luaAdhoc__tostring(lua_State *L)
{
    struct LPP_RemotePSP *r = *toAdhoc(L ,1);
	lua_pushstring(L, r->name);
	return(1);
}

static L_CONST luaL_reg luaAdhoc_methods[] = {
    { "init", luaAdhoc_init },
	{ "getMacAddr", luaAdhoc_getMacAddr },
	{ "getRemotePSPCount", luaAdhoc_getRemotePSPCount },
	{ "getPSPByMac", luaAdhoc_getPSPByMac },
	{ "getPSPByIndex", luaAdhoc_getPSPByIndex },
	{ "requestConnection", luaAdhoc_requestConnection },
	{ "sendData", luaAdhoc_sendData },
	{ "recvData", luaAdhoc_recvData },
	{ "getConnectionRequest", luaAdhoc_getConnectionRequest },
	{ "rejectConnection", luaAdhoc_rejectConnection },
	{ "acceptConnection", luaAdhoc_acceptConnection },
	{ "term", luaAdhoc_term },
	{ "name", luaAdhoc_name },
	{ "mac", luaAdhoc_mac },
	{ "connectionState", luaAdhoc_connectionState },
	{ 0 , 0 }
};

static L_CONST luaL_reg luaAdhoc_meta[] = {
    { "__tostring", luaAdhoc__tostring },
	{ 0, 0 }
};

UserdataRegister(Adhoc, luaAdhoc_methods, luaAdhoc_meta);

void luaAdhoc_Init(lua_State *L)
{
    Adhoc_register(L);
	
	#define ADHOC_CONSTANT(name)\
    lua_pushstring(L, #name);\
    lua_pushnumber(L, LPP_ADHOC_##name);\
    lua_settable(L, -3);
	
	ADHOC_CONSTANT(ERROR_WLAN)
	ADHOC_CONSTANT(ERROR_MAC)
	ADHOC_CONSTANT(ERROR_MODULES)
	ADHOC_CONSTANT(ERROR_NET_INIT)
	ADHOC_CONSTANT(ERROR_INIT)
	ADHOC_CONSTANT(ERROR_CTL_INIT)
	ADHOC_CONSTANT(ERROR_CTL_CONNECT)
	ADHOC_CONSTANT(ERROR_PDP_CREATE)
	ADHOC_CONSTANT(ERROR_MATCHING_INIT)
	ADHOC_CONSTANT(ERROR_MATCHING_CREATE)
	ADHOC_CONSTANT(ERROR_MATCHING_START)
	
	ADHOC_CONSTANT(DISCONNECTED)
	ADHOC_CONSTANT(JOINED)
	ADHOC_CONSTANT(SELECTED)
	ADHOC_CONSTANT(REJECTED)
	ADHOC_CONSTANT(CANCELED)
	ADHOC_CONSTANT(ACCEPTED)
	ADHOC_CONSTANT(ESTABILISHED)
}