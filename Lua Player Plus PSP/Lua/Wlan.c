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

#include <pspsdk.h>
#include <psputility.h>
#include <pspnet_apctl.h>
#include "../LPP.h"
#include "Wlan.h"

#define MAX_PICK 5

typedef struct
{
	SOCKET sock;
	struct sockaddr_in addrTo;
	u8 serverSocket;
} Socket;

UserdataStubs(Socket, Socket*)

static const char* wlanNotInitialized = "WLAN not initialized.";
static u8 wlanInitialized = 0;
static char resolverBuffer[1024];
static int resolverId;

int sceNetResolverCreate(int *rid, void *buf, SceSize buflen);
int sceNetResolverStartNtoA(int rid, const char *hostname, u32* in_addr, unsigned int timeout, int retry);
int sceNetResolverStartAtoN(int rid, const u32* in_addr, char *hostname, SceSize hostname_len, unsigned int timeout, int retry);
int sceNetResolverStop(int rid);
int sceNetInetInetAton(const char* host, u32* in_addr);

static int Wlan_init(lua_State* L)
{
	if (lua_gettop(L) != 0) 
	    return luaL_error(L, "Wlan.init() takes no arguments.");
	if (wlanInitialized) return 0;
	int err = pspSdkInetInit();
	if (err != 0) return luaL_error(L, "pspSdkInetInit failed.");
	err = sceNetResolverCreate(&resolverId, resolverBuffer, sizeof(resolverBuffer));
	wlanInitialized = 1;
	return 1;
}

static int Wlan_term(lua_State* L)
{
	if (lua_gettop(L) != 0) 
	    return luaL_error(L, "Wlan.term() takes no arguments.");
	if (!wlanInitialized) return 0;
	sceNetApctlDisconnect();
	pspSdkInetTerm();
	wlanInitialized = 0;
	return 1;
}

static int Wlan_getConnectionConfigs(lua_State* L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	if (lua_gettop(L) != 0) 
	    return luaL_error(L, "Wlan.getConnectionConfigs() takes no arguments.");

	lua_newtable(L);

	struct
	{
		int index;
		char name[64];
	} picks[MAX_PICK];
	int pick_count = 0;

	int iNetIndex;
	for (iNetIndex = 1; iNetIndex < 100; iNetIndex++) 
	{
		if (sceUtilityCheckNetParam(iNetIndex) != 0) break;  
		sceUtilityGetNetParam(iNetIndex, 0, (netData*) picks[pick_count].name);
		picks[pick_count].index = iNetIndex;
		pick_count++;
		lua_pushnumber(L, pick_count);
		lua_pushstring(L, picks[pick_count - 1].name);
		lua_settable(L, -3);
		if (pick_count >= MAX_PICK) break;  
	}

	return 1;  
}

static int Wlan_useConnectionConfig(lua_State* L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L);
	if (argc != 1) 
	    return luaL_error(L, "Wlan.useConnectionConfig(config) takes one argument.");

	int connectionConfig = luaL_checkint(L, 1) - 1;
	int result = sceNetApctlConnect(connectionConfig);

	int state = 0;

	while (1) {
		sceKernelDelayThread(200*1000); // 200ms

		int err = sceNetApctlGetState(&state);
		if (err != 0 || state == 0) {
			// conncection failed
			return 0;
		}

		if (state == 4) {
			//connection succeeded
			result = 1;
			break;
		}
	}

	lua_pushnumber(L, result);

	return 1;
}

static int Wlan_getIPAddress(lua_State* L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L);
	if (argc != 0) 
	    return luaL_error(L, "Wlan.getIPAddress() takes no arguments.");

	char szMyIPAddr[32];
	if(sceNetApctlGetInfo(8, (union SceNetApctlInfo*)szMyIPAddr) != 0) return 0;
	lua_pushstring(L, szMyIPAddr);
	return 1;
}

static int Socket_free(lua_State *L)
{
    if(lua_gettop(L) != 1)
	   return luaL_error(L, "Socket:free() takes no arguments and must be called with a colon.");
	Socket* socket = *toSocket(L, 1);
	sceNetInetClose(socket->sock);
	free(socket);
	return 0;
}

unsigned short htons(unsigned short wIn)
{
	u8 bHi = (wIn >> 8) & 0xFF;
	u8 bLo = wIn & 0xFF;
	return ((unsigned short)bLo << 8) | bHi;
}

int setSockNoBlock(SOCKET s, u32 val)
{
	return sceNetInetSetsockopt(s, SOL_SOCKET, 0x1009, (const char*)&val, sizeof(u32));
}

static int Socket_connect(lua_State *L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L);
	if (argc != 2) 
	    return luaL_error(L, "Socket.connect() takes no arguments.");

	sceKernelDelayThread(50*1000);

	Socket** luaSocket = pushSocket(L);
	Socket* socket = (Socket*) malloc(sizeof(Socket));
	*luaSocket = socket;
	socket->serverSocket = 0;

	const char *host = luaL_checkstring(L, 1);
	int port = luaL_checkint(L, 2);
	socket->addrTo.sin_family = AF_INET;
	socket->addrTo.sin_port = htons(port);
	int err = sceNetInetInetAton(host, &socket->addrTo.sin_addr);
	if (err == 0) {
		err = sceNetResolverStartNtoA(resolverId, host, &socket->addrTo.sin_addr, 2, 3);
		if (err < 0) return luaL_error(L, "Socket:connect: DNS resolving failed.");
	}

	socket->sock = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
	if (socket->sock & 0x80000000) {
		return luaL_error(L, "invalid socket.");
	}
	setSockNoBlock(socket->sock, 1);

	err = sceNetInetConnect(socket->sock, &socket->addrTo, sizeof(socket->addrTo));

	int inetErr = sceNetInetGetErrno();
	if (err == -1 ) {
		lua_pushnumber(L, inetErr);
	} else {
		lua_pushnumber(L, 0);
	}

	return 2;
}

static int Socket_isConnected(lua_State *L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L);
	if (argc != 1) 
	    return luaL_error(L, "Socket:isConnected() takes no arguments and must be called with a colon.");

	Socket* socket = *toSocket(L, 1);
	if (socket->serverSocket) {
		lua_pushboolean(L, 1);
		return 1;
	}
	int err = sceNetInetConnect(socket->sock, &socket->addrTo, sizeof(socket->addrTo));
	if (err == 0 || (err == -1 && sceNetInetGetErrno() == 0x7F)) {
		lua_pushboolean(L, 1);
		return 1;
	}
	lua_pushboolean(L, 0);
	return 1;
}

static int Socket_createServerSocket(lua_State *L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L);
	if (argc != 1) 
	return luaL_error(L, "Socket.createServerSocket(port) takes one argument.");

	int port = luaL_checkint(L, 1);

	Socket** luaSocket = pushSocket(L);
	Socket* socket = (Socket*) malloc(sizeof(Socket));
	*luaSocket = socket;
	socket->serverSocket = 1;

	socket->sock = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
	if (socket->sock <= 0) {
		return luaL_error(L, "invalid socket.");
	}

	socket->addrTo.sin_family = AF_INET;
	socket->addrTo.sin_port = htons(port);
	socket->addrTo.sin_addr = 0;

	int err = sceNetInetBind(socket->sock, &socket->addrTo, sizeof(socket->addrTo));
	if (err != 0) {
		return luaL_error(L, "bind error.");
	}

	setSockNoBlock(socket->sock, 1);

	err = sceNetInetListen(socket->sock, 1);
	if (err != 0) {
		return luaL_error(L, "listen error.");
	}

	return 1;
}

static int Socket_accept(lua_State *L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L);
	if (argc != 1) 
	    return luaL_error(L, "Socket:accept() takes no arguments and must be called with a colon.");

	Socket* socket = *toSocket(L, 1);

	if (!socket->serverSocket) return luaL_error(L, "accept allowed for server sockets only.");

	struct sockaddr_in addrAccept;
	int cbAddrAccept = sizeof(addrAccept);
	SOCKET sockClient = sceNetInetAccept(socket->sock, &addrAccept, &cbAddrAccept);
	if (sockClient <= 0) {
		return 0;
	}

	Socket** luaSocket = pushSocket(L);
	Socket* incomingSocket = (Socket*) malloc(sizeof(Socket));
	*luaSocket = incomingSocket;
	incomingSocket->serverSocket = 0;
	incomingSocket->sock = sockClient;
	incomingSocket->addrTo = addrAccept;

	return 1;
}

static int Socket_recv(lua_State *L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L);
	if (argc != 1) 
	    return luaL_error(L, "Socket:recv() takes no arguments and must be called with a colon.");

	Socket* socket = *toSocket(L, 1);

	if (socket->serverSocket) return luaL_error(L, "recv not allowed for server sockets.");

	char data[256];
	int count = sceNetInetRecv(socket->sock, (u8*) &data, 256, 0);
	if (count > 0) {
		lua_pushlstring(L, data, count);
	} else {
		lua_pushstring(L, "");
	}
	return 1;
}

static int Socket_send(lua_State *L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L);
	if (argc != 2) 
	    return luaL_error(L, "Socket:send(data) takes one argument and must be called with a colon.");

	Socket* socket = *toSocket(L, 1);

	if (socket->serverSocket) return luaL_error(L, "send not allowed for server sockets.");

	size_t size;
	const char *string = luaL_checklstring(L, 2, &size);
	if (!string) return luaL_error(L, "Socket:write expected a string.");
	int result = sceNetInetSend(socket->sock, string, size, 0);
	lua_pushnumber(L, result);
	return 1;
}

static int Socket_close(lua_State *L)
{
	if (!wlanInitialized) return luaL_error(L, wlanNotInitialized);
	int argc = lua_gettop(L);
	if (argc != 1) 
	    return luaL_error(L, "Scoket.close() takes no argument and must be called with a colon.");

	Socket* socket = *toSocket(L, 1);
	sceNetInetClose(socket->sock);
	return 0;
}

static int Socket_tostring(lua_State *L)
{
	Socket* socket = *toSocket(L, 1);
	char buf[128];
	sprintf(buf, "%lu.%lu.%lu.%lu",
		socket->addrTo.sin_addr & 255,
		(socket->addrTo.sin_addr >> 8) & 255,
		(socket->addrTo.sin_addr >> 16) & 255,
		(socket->addrTo.sin_addr >> 24) & 255);
	lua_pushstring(L, buf); 
	return 1;
}

static const luaL_reg Socket_methods[] = {
	{"isConnected", Socket_isConnected},
	{"accept", Socket_accept},
	{"send", Socket_send},
	{"recv", Socket_recv},
	{"close", Socket_close},
	{0,0}
};

static const luaL_reg Socket_meta[] = {
	{"__gc", Socket_free},
	{"__tostring", Socket_tostring},
	{0,0}
};

UserdataRegister(Socket, Socket_methods, Socket_meta)

static const luaL_reg Wlan_functions[] = {
		{"init", Wlan_init},
		{"term", Wlan_term},
		{"getConnectionConfigs", Wlan_getConnectionConfigs},
		{"useConnectionConfig", Wlan_useConnectionConfig},
		{"getIPAddress", Wlan_getIPAddress},
		{0, 0}
};

static const luaL_reg Socket_functions[] = {
	{"connect", Socket_connect},
	{"createServerSocket", Socket_createServerSocket},
	{0, 0}
};

void luaWlan_init(lua_State *L)
{
	luaL_openlib(L, "Wlan", Wlan_functions, 0);
	Socket_register(L);
	luaL_openlib(L, "Socket", Socket_functions, 0);
}
