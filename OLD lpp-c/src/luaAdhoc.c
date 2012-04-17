#include <pspsdk.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <pspnet_adhocmatching.h>
#include <psputility.h>
#include <pspnet.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <string.h>
#include <math.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>
#include <psputility_gamesharing.h>

#include "luaAdhoc.h"
#include "include/luaplayer.h"

#define _NET_ADHOCCTL_EVENT_ERROR		0
#define _NET_ADHOCCTL_EVENT_CONNECT		1
#define _NET_ADHOCCTL_EVENT_DISCONNECT	2
#define _NET_ADHOCCTL_EVENT_SCAN			3
#define _NET_ADHOCCTL_EVENT_GAMEMODE		4

static int AdhocErrorCode = 0;
static int AdhocEventFlag = 0;

static int AdhocMatchingId = 0;
static int AdhocPtpHostId = 0;
static int AdhocPtpClientId = 0;

static char AdhocctlGroupName[8];

static AdhocPeerEvent AdhocPeerEvents[15];

static void AdhocctlHandler(int event, int error, void *arg)
{
	(void)arg;
	
	if(event == _NET_ADHOCCTL_EVENT_ERROR)
	{
		AdhocErrorCode = error;
		AdhocEventFlag |= _ADHOC_EVENT_ERROR;
	}	
	else if(event == _NET_ADHOCCTL_EVENT_CONNECT)
	{
		AdhocErrorCode = 0;
		AdhocEventFlag |= _ADHOC_EVENT_CONNECT;
	}
	else if(event == _NET_ADHOCCTL_EVENT_DISCONNECT)
	{
		AdhocErrorCode = 0;
		AdhocEventFlag |= _ADHOC_EVENT_DISCONNECT;
	}
	else if(event == _NET_ADHOCCTL_EVENT_SCAN)
	{
		AdhocErrorCode = 0;
		AdhocEventFlag |= _ADHOC_EVENT_SCAN;
	}	
	else if(event == _NET_ADHOCCTL_EVENT_GAMEMODE)
	{
		AdhocErrorCode = 0;
		AdhocEventFlag |= _ADHOC_EVENT_GAMEMODE;
	}
	
	return;
}

static void AdhocMatchingAddEvent(unsigned char *mac, int event, int optlen, char *opt)
{
	int i;
	
	char mac2[20];
	
	sceNetEtherNtostr(mac, mac2);
	
	for(i = 0;i < 15;i++)
	{
		if(AdhocPeerEvents[i].event != _ADHOC_MATCHING_EVENT_NONE)
		{
			if(memcmp(mac2, AdhocPeerEvents[i].mac, 18) == 0)
			{
				AdhocPeerEvents[i].event = event;
				return;
			}
		}
	}
	
	for(i = 0;i < 15;i++)
	{
		if(AdhocPeerEvents[i].event == _ADHOC_MATCHING_EVENT_NONE)
		{
			AdhocPeerEvents[i].event = event;
			sceNetEtherNtostr(mac, AdhocPeerEvents[i].mac);
			sceNetAdhocctlGetNameByAddr(mac, AdhocPeerEvents[i].nickname);
			
			if(optlen > 0)
				strncpy(AdhocPeerEvents[i].hello, opt, optlen);
				
			return;
		}
	}
}
			
static void AdhocMatchingHandler(int id, int event, unsigned char *mac, int optlen, void *opt)
{
	(void)id;
	
	AdhocMatchingAddEvent(mac, event, optlen, (char*)opt);	
}

int AdhocInit(int type)
{		
	int result;
	
	result = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	
	if(result < 0)
	{
		printf("Adhoc error: sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON): 0x%08X", result);
		return result;
	}
	
	result = sceUtilityLoadNetModule(PSP_NET_MODULE_ADHOC);
	
	if(result < 0)
	{
		printf("Adhoc error: sceUtilityLoadNetModule(PSP_NET_MODULE_ADHOC); 0x%08X", result);
		return result;
	}
	
	result = sceNetInit(128*1024, 42, 4*1024, 42, 4*1024);
	
	if(result < 0)
	{
		printf("Adhoc error: sceNetInit(); 0x%08X", result);
		return result;
	}
	
	result = sceNetAdhocInit();
	
	if(result < 0)
		printf("Adhoc error: sceNetAdhocInit(): 0x%08X", result);
	
	struct productStruct gameProduct;
	
	gameProduct.unknown = type;
	
	if(type == _ADHOC_TYPE_GAMESHARING)
		memcpy(gameProduct.product, "000000001", 9);
	else
		memcpy(gameProduct.product, "ULUS99999", 9);
	
	result = sceNetAdhocctlInit(32*1024, 0x20, &gameProduct);
	
	if(result < 0)
		printf("Adhoc error: sceNetAdhocctlInit(): 0x%08X", result);
	
	// Register pspnet_adhocctl event handler
	result = sceNetAdhocctlAddHandler(AdhocctlHandler, NULL);
	
	if(result < 0)
	{
		printf("Adhoc error: sceNetAdhocctlAddHandler\n");
		return 0;
	}
	
	return 1;
}

int AdhocShutdown(void)
{	
	sceNetAdhocctlDisconnect();
	
	sceNetAdhocctlDelHandler(0);
	
	sceNetAdhocctlTerm(); 
	
	sceNetAdhocTerm();
	
	sceNetTerm();
	
	sceUtilityUnloadNetModule(PSP_NET_MODULE_ADHOC);
	
	sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
	
	return 1;
}

int AdhocGameHost(const char *name)
{
	memset(&AdhocctlGroupName, 0, sizeof(AdhocctlGroupName));
	memcpy(&AdhocctlGroupName, name, strlen(name));
	
	int ret = sceNetAdhocctlCreate(AdhocctlGroupName);
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocctlCreate\n");
		return 0;
	}
	
	return 1;
}

int AdhocConnect(const char *name)
{	
	memset(&AdhocctlGroupName, 0, sizeof(AdhocctlGroupName));
	sprintf(AdhocctlGroupName, name);
	
	int ret = sceNetAdhocctlConnect(AdhocctlGroupName);
		
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocctlConnect\n");
		return 0;
	}
	
	return 1;
}

int AdhocGetState(void)
{
	if(AdhocEventFlag & _ADHOC_EVENT_ERROR)
	{
		AdhocEventFlag &= ~_ADHOC_EVENT_ERROR;
		return(_ADHOC_EVENT_ERROR);
	}
	else if(AdhocEventFlag & _ADHOC_EVENT_CONNECT)
	{
		AdhocEventFlag &= ~_ADHOC_EVENT_CONNECT;
		return(_ADHOC_EVENT_CONNECT);
	}
	else if(AdhocEventFlag & _ADHOC_EVENT_DISCONNECT)
	{
		AdhocEventFlag &= ~_ADHOC_EVENT_DISCONNECT;
		return(_ADHOC_EVENT_DISCONNECT);
	}
	else if(AdhocEventFlag & _ADHOC_EVENT_SCAN)
	{
		AdhocEventFlag &= ~_ADHOC_EVENT_SCAN;
		return(_ADHOC_EVENT_SCAN);
	}
	else if(AdhocEventFlag & _ADHOC_EVENT_GAMEMODE)
	{
		AdhocEventFlag &= ~_ADHOC_EVENT_GAMEMODE;
		return(_ADHOC_EVENT_GAMEMODE);
	}
	else if(AdhocEventFlag & _ADHOC_EVENT_CANCEL)
	{
		AdhocEventFlag &= ~_ADHOC_EVENT_CANCEL;
		return(_ADHOC_EVENT_CANCEL);
	}
	
	return 0;
}

int AdhocPeerExists(const char *mac)
{
	unsigned char peermac[6];
	
	sceNetEtherStrton(mac, peermac);
	
	struct SceNetAdhocctlPeerInfo info;
	
	int ret = sceNetAdhocctlGetPeerInfo(peermac, sizeof(struct SceNetAdhocctlPeerInfo), &info);
	
	if(ret < 0)
		return 0;
		
	return 1;
}	

int AdhocMatchingInit(int mode, int type)
{	
	memset(AdhocPeerEvents, 0, sizeof(AdhocPeerEvents));
	
	int ret, maxpeers, intmode, inttype;
	
	if(mode == _ADHOC_MATCHING_MODE_PTP)
	{
		intmode = 3;
		inttype = type;
		maxpeers = 2;
	}
	else
	{
		if(type == _ADHOC_MATCHING_TYPE_HOST)
			intmode = 1;
		else
			intmode = 2;
			
		inttype = type;
		maxpeers = 16;
	}
	
	ret = sceNetAdhocMatchingInit(32*1024);
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocMatchingInit\n");
		return 0;
	}
	
	ret = sceNetAdhocMatchingCreate(3, 2, 1, 2048, 200*1000, 200*1000, 30, 200*1000, AdhocMatchingHandler);
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocMatchingCreate\n");
		return 0;
	}
	
	AdhocMatchingId = ret;
	
	return 1;
}

int AdhocMatchingStart(const char *hello)
{
	int ret;
	
	if(hello == NULL)
		ret = sceNetAdhocMatchingStart(AdhocMatchingId, 16, 32 * 1024, 16, 0, 0, NULL);
	else
		ret = sceNetAdhocMatchingStart(AdhocMatchingId, 16, 32 * 1024, 16, 0, strlen(hello) + 1, (void*)hello);
	 
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocMatchingStart\n");
		return 0;
	}
	
	return 1;
}

int AdhocMatchingAccept(const char *mac)
{
	unsigned char mac2[6];
	
	sceNetEtherStrton(mac, mac2);
	
	int ret = sceNetAdhocMatchingSelectTarget(AdhocMatchingId, mac2, 0, 0);
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocMatchingSelectTarget: 0x%08X\n", ret);
		return 0;
	}
		
	return 1;
}

int AdhocMatchingDecline(const char *mac)
{
	unsigned char mac2[6];
	
	sceNetEtherStrton(mac, mac2);
	
	int ret = sceNetAdhocMatchingCancelTarget(AdhocMatchingId, mac2);
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocMatchingCancelTarget\n");
		return 0;
	}
		
	return 1;
}

int AdhocMatchingShutdown(void)
{	
	int ret = sceNetAdhocMatchingStop(AdhocMatchingId);
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocMatchingStop\n");
		return 0;
	}
	
	ret = sceNetAdhocMatchingDelete(AdhocMatchingId);
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocMatchingDelete\n");
		return 0;
	}
	
	ret = sceNetAdhocMatchingTerm();
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocMatchingTerm\n");
		return 0;
	}
	
	return 1;
}

AdhocPeerEvent* AdhocMatchingGetEvents(void)
{
	return(AdhocPeerEvent*) &AdhocPeerEvents[0];
}

int AdhocMatchingClearEvent(AdhocPeerEvent *event)
{
	event->event = _ADHOC_MATCHING_EVENT_NONE;
	
	return 1;
}

int AdhocPtpHostStart(void)
{
	unsigned char mac[6];
	unsigned char clientmac[6];
	unsigned short clientport;
	
	AdhocPtpHostId = -1;
	AdhocPtpClientId = -1;
	
	int ret = sceNetGetLocalEtherAddr(mac);
		
	if(ret < 0)
	{
		printf("Adhoc error: sceNetGetLocalEtherAddr\n");
		return 0;
	}
	
	ret = sceNetAdhocPtpListen(mac, 1, 8192, 200*1000, 300, 1, 0);
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocPtpListen 0x%08X\n", ret);
		
		if(AdhocPtpHostId > 0)
			sceNetAdhocPtpClose(AdhocPtpHostId, 0);
			
		if(AdhocPtpClientId > 0)
			sceNetAdhocPtpClose(AdhocPtpClientId, 0);
		
		return ret;
	}
	
	AdhocPtpHostId = ret;
		
	ret = sceNetAdhocPtpAccept(AdhocPtpHostId, &clientmac[0], &clientport, 0, 0);
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocPtpAccept\n");
		
		if(AdhocPtpHostId > 0)
			sceNetAdhocPtpClose(AdhocPtpHostId, 0);
			
		if(AdhocPtpClientId > 0)
			sceNetAdhocPtpClose(AdhocPtpClientId, 0);
		
		return ret;
	}
	
	AdhocPtpClientId = ret;
	
	return ret;
}

int AdhocPtpClientStart(const char *servermac)
{
	unsigned char mac[6];
	
	sceNetEtherStrton(servermac, mac);
	
	unsigned char clientmac[6];
	
	int ret = sceNetGetLocalEtherAddr(clientmac);
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetGetLocalEtherAddr\n");
		return ret;
	}
	
	ret = sceNetAdhocPtpOpen(clientmac, 0, mac, 1, 8192, 200*1000, 300, 0);
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocPtpOpen\n");
		
		if(AdhocPtpClientId > 0)
			sceNetAdhocPtpClose(AdhocPtpClientId, 0);
		
		return ret;
	}
	
	AdhocPtpClientId = ret;
	
	while(1)
	{	
		ret = sceNetAdhocPtpConnect(AdhocPtpClientId, 0, 1);
			
		if(ret < 0 && ret != (int)0x80410709)
		{
			printf("Adhoc error: sceNetAdhocPtpConnect 0x%08X\n", ret);
		
			if(AdhocPtpClientId > 0)
				sceNetAdhocPtpClose(AdhocPtpClientId, 0);
		
			return ret;
		}
		
		if(ret == 0)
			break;
		
		sceKernelDelayThread(200*1000);
	}
	
	return ret;
}

int AdhocPtpReceive(void *data, int *length)
{	
	int ret = sceNetAdhocPtpRecv(AdhocPtpClientId, data, length, 1000*1000, 0);
	
	if(ret < 0 && ret != (int)0x80410709)
	{
		printf("Adhoc error: sceNetAdhocPtpRecv\n");
		
		if(AdhocPtpClientId > 0)
			sceNetAdhocPtpClose(AdhocPtpClientId, 0);
		
		return 0;
	}
	
	return 1;
}

int AdhocPtpSend(void *data, int *length)
{
	int ret = sceNetAdhocPtpSend(AdhocPtpClientId, data, length, 0, 0);
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocPtpSend\n");
		
		if(AdhocPtpHostId > 0)
			sceNetAdhocPtpClose(AdhocPtpHostId, 0);
			
		if(AdhocPtpClientId > 0)
			sceNetAdhocPtpClose(AdhocPtpClientId, 0);
		
		return(0);
	}
		
	return 1;
}

int AdhocPtpCheckForData(void)
{
	//int torecv = 0;
	
	int ret, buflen;
	
	//ptpStatStruct *buf, *ptr;
	
	ret = sceNetAdhocGetPtpStat(&buflen, NULL);
	
	if(ret < 0)
		return 0;
	else if (buflen == 0)
		return 0;
		
/*buf = malloc(buflen);
if (buf == NULL)
{
	// Could not allocate memory
	return;
}

ret = sceNetAdhocGetPtpStat(&buflen, buf);
if (ret < 0)
{
	// Error handling
}
else if (buflen == 0)
{
	// No data
}        
else
{
	for (ptr = buf; ptr != NULL; ptr = ptr->next)
	{
		// Process acquired control block
		...
	}
}*/
	
	return 1;
}

int AdhocPtpFlush(void)
{
	int ret = sceNetAdhocPtpFlush(AdhocPtpClientId, 0, 0);
	
	if(ret < 0)
	{
		printf("Adhoc error: sceNetAdhocPtpFlush\n");
		
		if(AdhocPtpHostId > 0)
			sceNetAdhocPtpClose(AdhocPtpHostId, 0);
			
		if(AdhocPtpClientId > 0)
			sceNetAdhocPtpClose(AdhocPtpClientId, 0);
		
		return 0;
	}
	
	return 1;
}

int AdhocPtpHostShutdown(void)
{
	if(AdhocPtpHostId > 0)
		sceNetAdhocPtpClose(AdhocPtpHostId, 0);
			
	if(AdhocPtpClientId > 0)
		sceNetAdhocPtpClose(AdhocPtpClientId, 0);
	
	return 1;
}

int AdhocPtpClientShutdown(void)
{
	if(AdhocPtpClientId > 0)
		sceNetAdhocPtpClose(AdhocPtpClientId, 0);
	
	return 1;
}

int AdhocGetError(void)
{
	return(AdhocErrorCode);
}


static int lua_AdhocInit(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argment error: Adhoc.init() takes no arguments.");
		
	int result = AdhocInit(_ADHOC_TYPE_NORMAL);
	
	if(result == 1)
		lua_pushboolean(L, 1);
	else
		lua_pushboolean(L, 0);
	
	return 1;
}

static int lua_AdhocShutdown(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argment error: Adhoc.shutdown() takes no arguments.");
		
	lua_pushboolean(L, AdhocShutdown());
	
	return 1;
}

static int lua_AdhocConnect(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argment error: Adhoc.connect(name) takes one argument.");
		
	const char *name = luaL_checkstring(L, 1);
		
	lua_pushboolean(L, AdhocConnect(name));
	
	return 1;
}

static int lua_AdhocGetState(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argment error: Adhoc.state() takes no arguments.");
		
	lua_pushnumber(L, AdhocGetState());
	
	return 1;
}

static int lua_AdhocGetError(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argment error: Adhoc.geterror() takes no arguments.");
		
	lua_pushnumber(L, AdhocGetError());
	
	return 1;
}

static int lua_AdhocPeerExists(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argment error: Adhoc.peerexists() takes one argument.");
		
	const char *mac = luaL_checkstring(L, 1);
	
	lua_pushboolean(L, AdhocPeerExists(mac));
	
	return 1;
}

static int lua_AdhocMatchingInit(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1 && argc != 2)
		return luaL_error(L, "Argment error: AdhocMatching.init(type, [mode]) takes one or two arguments.");
		
	int type = luaL_checkint(L, 1);
	
	int mode = _ADHOC_MATCHING_MODE_PTP;
	
	if(argc == 2)
		mode = luaL_checkint(L, 2);
		
	lua_pushboolean(L, AdhocMatchingInit(mode, type));
	
	return 1;
}

static int lua_AdhocMatchingStart(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0 && argc != 1)
		return luaL_error(L, "Argment error: AdhocMatching.start([hello]) takes zero or one argument.");
	
	const char *hello = NULL;
	
	if(argc == 1)
		hello = luaL_checkstring(L, 1);
		
	lua_pushboolean(L, AdhocMatchingStart(hello));
	
	return 1;
}

static int lua_AdhocMatchingAccept(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argment error: AdhocMatching.accept(mac) takes one argument.");
	
	const char *mac = luaL_checkstring(L, 1);
		
	lua_pushboolean(L, AdhocMatchingAccept(mac));
	
	return 1;
}

static int lua_AdhocMatchingDecline(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argment error: AdhocMatching.decline(mac) takes one argument.");
	
	const char *mac = luaL_checkstring(L, 1);
		
	lua_pushboolean(L, AdhocMatchingDecline(mac));
	
	return 1;
}

static int lua_AdhocMatchingShutdown(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argment error: AdhocMatching.shutdown() takes no arguments.");
		
	lua_pushboolean(L, AdhocMatchingShutdown());
	
	return 1;
}

static int lua_AdhocMatchingGetEvents(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argment error: AdhocMatching.events() takes no arguments.");
		
	AdhocPeerEvent *event = AdhocMatchingGetEvents();
		
	lua_newtable(L);
	
	int i = 0;
	
	for(i = 0;i < 15;i++)
	{
		if(event[i].event > 0)
		{
			lua_pushnumber(L, i + 1);
		
			lua_newtable(L);
		
				lua_pushstring(L, "mac");
				lua_pushstring(L, event[i].mac);
				lua_settable(L, -3);
	
				lua_pushstring(L, "nickname");
				lua_pushstring(L, event[i].nickname);
				lua_settable(L, -3);
			
				lua_pushstring(L, "hello");
				lua_pushstring(L, event[i].hello);
				lua_settable(L, -3);
	
				lua_pushstring(L, "event");
				lua_pushnumber(L, event[i].event);
				lua_settable(L, -3);

			lua_settable(L, -3);
		}
	}
			
	return 1;
}

static int lua_AdhocPtpHostStart(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argment error: AdhocPtp.hoststart() takes no arguments.");
		
	lua_pushnumber(L, AdhocPtpHostStart());
	
	return 1;
}

static int lua_AdhocPtpClientStart(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argment error: AdhocPtp.clientstart(mac) takes one argument.");
		
	const char *mac = luaL_checkstring(L, 1);
		
	lua_pushnumber(L, AdhocPtpClientStart(mac));
	
	return 1;
}

static int lua_AdhocPtpSend(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 1)
		return luaL_error(L, "Argment error: AdhocPtp.send(data) takes one argument.");
		
	size_t size;
		
	char *data = (char *)luaL_checklstring(L, 1, &size);
		
	lua_pushboolean(L, AdhocPtpSend(data, (int *)&size));
	
	return 1;
}

static int lua_AdhocPtpReceive(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argment error: AdhocPtp.receive() takes no arguments.");
		
	int size = 128;
	
	char data[size];
	
	memset(data, 0, size);
	
	AdhocPtpReceive(data, &size);
		
	lua_pushstring(L, data);
	
	return 1;
}

static int lua_AdhocPtpCheckForData(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argment error: AdhocPtp.checkfordata() takes no arguments.");
		
	lua_pushboolean(L, AdhocPtpCheckForData());
	
	return 1;
}

static int lua_AdhocPtpFlush(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argment error: AdhocPtp.flush() takes no arguments.");
		
	lua_pushboolean(L, AdhocPtpFlush());
	
	return 1;
}

static int lua_AdhocPtpHostShutdown(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argment error: AdhocPtp.hostshutdown() takes no arguments.");
		
	lua_pushboolean(L, AdhocPtpHostShutdown());
	
	return 1;
}

static int lua_AdhocPtpClientShutdown(lua_State *L)
{
	int argc = lua_gettop(L);
	
	if(argc != 0)
		return luaL_error(L, "Argment error: AdhocPtp.clientshutdown() takes no arguments.");
		
	lua_pushboolean(L, AdhocPtpClientShutdown());
	
	return 1;
}

static const luaL_reg lua_Adhoc_functions[] =
{
	{"init",			lua_AdhocInit},
	{"shutdown",		lua_AdhocShutdown},
	{"connect",			lua_AdhocConnect},
	{"state",			lua_AdhocGetState},
	{"geterror",		lua_AdhocGetError},
	{"peerexists",		lua_AdhocPeerExists},
	{0, 0}
};

static const luaL_reg lua_AdhocMatching_functions[] =
{
	{"init",		lua_AdhocMatchingInit},
	{"start",		lua_AdhocMatchingStart},
	{"accept",		lua_AdhocMatchingAccept},
	{"decline",		lua_AdhocMatchingDecline},
	{"shutdown",	lua_AdhocMatchingShutdown},
	{"events",		lua_AdhocMatchingGetEvents},
	{0, 0}
};

static const luaL_reg lua_AdhocPtp_functions[] =
{
	{"hoststart",		lua_AdhocPtpHostStart},
	{"clientstart",		lua_AdhocPtpClientStart},
	{"receive",			lua_AdhocPtpReceive},
	{"send",			lua_AdhocPtpSend},
	{"checkfordata",	lua_AdhocPtpCheckForData},
	{"flush",			lua_AdhocPtpFlush},
	{"hostshutdown",	lua_AdhocPtpHostShutdown},
	{"clientshutdown",	lua_AdhocPtpClientShutdown},
	{0, 0}
};

void luaAdhoc_init(lua_State *L)
{
	luaL_openlib(L, "Adhoc", lua_Adhoc_functions, 0);
	luaL_openlib(L, "AdhocMatching", lua_AdhocMatching_functions, 0);
	luaL_openlib(L, "AdhocPtp", lua_AdhocPtp_functions, 0);
	lua_pushnumber(L, _ADHOC_EVENT_ERROR); lua_setglobal(L, "_ADHOC_EVENT_ERROR");
	lua_pushnumber(L, _ADHOC_EVENT_CONNECT); lua_setglobal(L, "_ADHOC_EVENT_CONNECT");
	lua_pushnumber(L, _ADHOC_EVENT_DISCONNECT); lua_setglobal(L, "_ADHOC_EVENT_DISCONNECT");
	lua_pushnumber(L, _ADHOC_EVENT_SCAN); lua_setglobal(L, "_ADHOC_EVENT_SCAN");
	lua_pushnumber(L, _ADHOC_EVENT_GAMEMODE); lua_setglobal(L, "_ADHOC_EVENT_GAMEMODE");
	lua_pushnumber(L, _ADHOC_EVENT_CANCEL); lua_setglobal(L, "_ADHOC_EVENT_CANCEL");
	
	lua_pushnumber(L, _ADHOC_MATCHING_EVENT_HELLO); lua_setglobal(L, "_ADHOC_MATCHING_EVENT_HELLO");	
	lua_pushnumber(L, _ADHOC_MATCHING_EVENT_REQUEST); lua_setglobal(L, "_ADHOC_MATCHING_EVENT_REQUEST");
	lua_pushnumber(L, _ADHOC_MATCHING_EVENT_LEAVE); lua_setglobal(L, "_ADHOC_MATCHING_EVENT_LEAVE");
	lua_pushnumber(L, _ADHOC_MATCHING_EVENT_DENY); lua_setglobal(L, "_ADHOC_MATCHING_EVENT_DENY");
	lua_pushnumber(L, _ADHOC_MATCHING_EVENT_CANCEL); lua_setglobal(L, "_ADHOC_MATCHING_EVENT_CANCEL");
	lua_pushnumber(L, _ADHOC_MATCHING_EVENT_ACCEPT); lua_setglobal(L, "_ADHOC_MATCHING_EVENT_ACCEPT");
	lua_pushnumber(L, _ADHOC_MATCHING_EVENT_ESTABLISHED); lua_setglobal(L, "_ADHOC_MATCHING_EVENT_ESTABLISHED");
	lua_pushnumber(L, _ADHOC_MATCHING_EVENT_TIMEOUT); lua_setglobal(L, "_ADHOC_MATCHING_EVENT_TIMEOUT");
	lua_pushnumber(L, _ADHOC_MATCHING_EVENT_ERROR); lua_setglobal(L, "_ADHOC_MATCHING_EVENT_ERROR");
	lua_pushnumber(L, _ADHOC_MATCHING_EVENT_BYE); lua_setglobal(L, "_ADHOC_MATCHING_EVENT_BYE");
	
	lua_pushnumber(L, _ADHOC_MATCHING_MODE_PTP); lua_setglobal(L, "_ADHOC_MATCHING_MODE_PTP");
	
	lua_pushnumber(L, _ADHOC_MATCHING_TYPE_HOST); lua_setglobal(L, "_ADHOC_MATCHING_TYPE_HOST");
	lua_pushnumber(L, _ADHOC_MATCHING_TYPE_CLIENT); lua_setglobal(L, "_ADHOC_MATCHING_TYPE_CLIENT");
}

