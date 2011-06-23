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

#define PGE_NET_ADHOCCTL_EVENT_ERROR        0
#define PGE_NET_ADHOCCTL_EVENT_CONNECT        1
#define PGE_NET_ADHOCCTL_EVENT_DISCONNECT    2
#define PGE_NET_ADHOCCTL_EVENT_SCAN            3
#define PGE_NET_ADHOCCTL_EVENT_GAMEMODE        4

static int pgeAdhocErrorCode = 0;
static int pgeAdhocEventFlag = 0;

static int pgeAdhocMatchingId = 0;
static int pgeAdhocPtpHostId = 0;
static int pgeAdhocPtpClientId = 0;

static char pgeAdhocctlGroupName[8];

static pgeAdhocPeerEvent pgeAdhocPeerEvents[15];

static void pgeAdhocctlHandler(int event, int error, void *arg)
{
    (void)arg;
    
    if(event == PGE_NET_ADHOCCTL_EVENT_ERROR)
    {
        pgeAdhocErrorCode = error;
        pgeAdhocEventFlag |= PGE_ADHOC_EVENT_ERROR;
    }    
    else if(event == PGE_NET_ADHOCCTL_EVENT_CONNECT)
    {
        pgeAdhocErrorCode = 0;
        pgeAdhocEventFlag |= PGE_ADHOC_EVENT_CONNECT;
    }
    else if(event == PGE_NET_ADHOCCTL_EVENT_DISCONNECT)
    {
        pgeAdhocErrorCode = 0;
        pgeAdhocEventFlag |= PGE_ADHOC_EVENT_DISCONNECT;
    }
    else if(event == PGE_NET_ADHOCCTL_EVENT_SCAN)
    {
        pgeAdhocErrorCode = 0;
        pgeAdhocEventFlag |= PGE_ADHOC_EVENT_SCAN;
    }    
    else if(event == PGE_NET_ADHOCCTL_EVENT_GAMEMODE)
    {
        pgeAdhocErrorCode = 0;
        pgeAdhocEventFlag |= PGE_ADHOC_EVENT_GAMEMODE;
    }
    
    return;
}

static void pgeAdhocMatchingAddEvent(unsigned char *mac, int event, int optlen, char *opt)
{
    int i;
    
    char mac2[20];
    
    sceNetEtherNtostr(mac, mac2);
    
    for(i = 0;i < 15;i++)
    {
        if(pgeAdhocPeerEvents[i].event != PGE_ADHOC_MATCHING_EVENT_NONE)
        {
            if(memcmp(mac2, pgeAdhocPeerEvents[i].mac, 18) == 0)
            {
                pgeAdhocPeerEvents[i].event = event;
                return;
            }
        }
    }
    
    for(i = 0;i < 15;i++)
    {
        if(pgeAdhocPeerEvents[i].event == PGE_ADHOC_MATCHING_EVENT_NONE)
        {
            pgeAdhocPeerEvents[i].event = event;
            sceNetEtherNtostr(mac, pgeAdhocPeerEvents[i].mac);
            sceNetAdhocctlGetNameByAddr(mac, pgeAdhocPeerEvents[i].nickname);
            
            if(optlen > 0)
                strncpy(pgeAdhocPeerEvents[i].hello, opt, optlen);
                
            return;
        }
    }
}
            
static void pgeAdhocMatchingHandler(int id, int event, unsigned char *mac, int optlen, void *opt)
{
    (void)id;
    
    pgeAdhocMatchingAddEvent(mac, event, optlen, (char*)opt);    
}

int pgeAdhocInit(int type)
{        
    int result;
    
    result = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
    
    if(result < 0)
    {
        printf("pgeAdhoc error: sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON): 0x%08X", result);
        return result;
    }
    
    result = sceUtilityLoadNetModule(PSP_NET_MODULE_ADHOC);
    
    if(result < 0)
    {
        printf("pgeAdhoc error: sceUtilityLoadNetModule(PSP_NET_MODULE_ADHOC); 0x%08X", result);
        return result;
    }
    
    result = sceNetInit(128*1024, 42, 4*1024, 42, 4*1024);
    
    if(result < 0)
    {
        printf("pgeAdhoc error: sceNetInit(); 0x%08X", result);
        return result;
    }
    
    result = sceNetAdhocInit();
    
    if(result < 0)
        printf("pgeAdhoc error: sceNetAdhocInit(): 0x%08X", result);
    
    struct productStruct gameProduct;
    
    gameProduct.unknown = type;
    
    if(type == PGE_ADHOC_TYPE_GAMESHARING)
        memcpy(gameProduct.product, "000000001", 9);
    else
        memcpy(gameProduct.product, "ULUS99999", 9);
    
    result = sceNetAdhocctlInit(32*1024, 0x20, &gameProduct);
    
    if(result < 0)
        printf("pgeAdhoc error: sceNetAdhocctlInit(): 0x%08X", result);
    
    // Register pspnet_adhocctl event handler
    result = sceNetAdhocctlAddHandler(pgeAdhocctlHandler, NULL);
    
    if(result < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocctlAddHandler\n");
        return 0;
    }
    
    return 1;
}

int pgeAdhocShutdown(void)
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

int pgeAdhocGameHost(const char *name)
{
    memset(&pgeAdhocctlGroupName, 0, sizeof(pgeAdhocctlGroupName));
    memcpy(&pgeAdhocctlGroupName, name, strlen(name));
    
    int ret = sceNetAdhocctlCreate(pgeAdhocctlGroupName);
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocctlCreate\n");
        return 0;
    }
    
    return 1;
}

int pgeAdhocConnect(const char *name)
{    
    memset(&pgeAdhocctlGroupName, 0, sizeof(pgeAdhocctlGroupName));
    sprintf(pgeAdhocctlGroupName, name);
    
    int ret = sceNetAdhocctlConnect(pgeAdhocctlGroupName);
        
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocctlConnect\n");
        return 0;
    }
    
    return 1;
}

int pgeAdhocGetState(void)
{
    if(pgeAdhocEventFlag & PGE_ADHOC_EVENT_ERROR)
    {
        pgeAdhocEventFlag &= ~PGE_ADHOC_EVENT_ERROR;
        return(PGE_ADHOC_EVENT_ERROR);
    }
    else if(pgeAdhocEventFlag & PGE_ADHOC_EVENT_CONNECT)
    {
        pgeAdhocEventFlag &= ~PGE_ADHOC_EVENT_CONNECT;
        return(PGE_ADHOC_EVENT_CONNECT);
    }
    else if(pgeAdhocEventFlag & PGE_ADHOC_EVENT_DISCONNECT)
    {
        pgeAdhocEventFlag &= ~PGE_ADHOC_EVENT_DISCONNECT;
        return(PGE_ADHOC_EVENT_DISCONNECT);
    }
    else if(pgeAdhocEventFlag & PGE_ADHOC_EVENT_SCAN)
    {
        pgeAdhocEventFlag &= ~PGE_ADHOC_EVENT_SCAN;
        return(PGE_ADHOC_EVENT_SCAN);
    }
    else if(pgeAdhocEventFlag & PGE_ADHOC_EVENT_GAMEMODE)
    {
        pgeAdhocEventFlag &= ~PGE_ADHOC_EVENT_GAMEMODE;
        return(PGE_ADHOC_EVENT_GAMEMODE);
    }
    else if(pgeAdhocEventFlag & PGE_ADHOC_EVENT_CANCEL)
    {
        pgeAdhocEventFlag &= ~PGE_ADHOC_EVENT_CANCEL;
        return(PGE_ADHOC_EVENT_CANCEL);
    }
    
    return 0;
}

int pgeAdhocPeerExists(const char *mac)
{
    unsigned char peermac[6];
    
    sceNetEtherStrton(mac, peermac);
    
    struct SceNetAdhocctlPeerInfo info;
    
    int ret = sceNetAdhocctlGetPeerInfo(peermac, sizeof(struct SceNetAdhocctlPeerInfo), &info);
    
    if(ret < 0)
        return 0;
        
    return 1;
}    

int pgeAdhocMatchingInit(int mode, int type)
{    
    memset(pgeAdhocPeerEvents, 0, sizeof(pgeAdhocPeerEvents));
    
    int ret, maxpeers, intmode, inttype;
    
    if(mode == PGE_ADHOC_MATCHING_MODE_PTP)
    {
        intmode = 3;
        inttype = type;
        maxpeers = 2;
    }
    else
    {
        if(type == PGE_ADHOC_MATCHING_TYPE_HOST)
            intmode = 1;
        else
            intmode = 2;
            
        inttype = type;
        maxpeers = 16;
    }
    
    ret = sceNetAdhocMatchingInit(32*1024);
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocMatchingInit\n");
        return 0;
    }
    
    ret = sceNetAdhocMatchingCreate(3, 2, 1, 2048, 200*1000, 200*1000, 30, 200*1000, pgeAdhocMatchingHandler);
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocMatchingCreate\n");
        return 0;
    }
    
    pgeAdhocMatchingId = ret;
    
    return 1;
}

int pgeAdhocMatchingStart(const char *hello)
{
    int ret;
    
    if(hello == NULL)
        ret = sceNetAdhocMatchingStart(pgeAdhocMatchingId, 16, 32 * 1024, 16, 0, 0, NULL);
    else
        ret = sceNetAdhocMatchingStart(pgeAdhocMatchingId, 16, 32 * 1024, 16, 0, strlen(hello) + 1, (void*)hello);
     
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocMatchingStart\n");
        return 0;
    }
    
    return 1;
}

int pgeAdhocMatchingAccept(const char *mac)
{
    unsigned char mac2[6];
    
    sceNetEtherStrton(mac, mac2);
    
    int ret = sceNetAdhocMatchingSelectTarget(pgeAdhocMatchingId, mac2, 0, 0);
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocMatchingSelectTarget: 0x%08X\n", ret);
        return 0;
    }
        
    return 1;
}

int pgeAdhocMatchingDecline(const char *mac)
{
    unsigned char mac2[6];
    
    sceNetEtherStrton(mac, mac2);
    
    int ret = sceNetAdhocMatchingCancelTarget(pgeAdhocMatchingId, mac2);
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocMatchingCancelTarget\n");
        return 0;
    }
        
    return 1;
}

int pgeAdhocMatchingShutdown(void)
{    
    int ret = sceNetAdhocMatchingStop(pgeAdhocMatchingId);
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocMatchingStop\n");
        return 0;
    }
    
    ret = sceNetAdhocMatchingDelete(pgeAdhocMatchingId);
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocMatchingDelete\n");
        return 0;
    }
    
    ret = sceNetAdhocMatchingTerm();
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocMatchingTerm\n");
        return 0;
    }
    
    return 1;
}

pgeAdhocPeerEvent* pgeAdhocMatchingGetEvents(void)
{
    return(pgeAdhocPeerEvent*) &pgeAdhocPeerEvents[0];
}

int pgeAdhocMatchingClearEvent(pgeAdhocPeerEvent *event)
{
    event->event = PGE_ADHOC_MATCHING_EVENT_NONE;
    
    return 1;
}

int pgeAdhocPtpHostStart(void)
{
    unsigned char mac[6];
    unsigned char clientmac[6];
    unsigned short clientport;
    
    pgeAdhocPtpHostId = -1;
    pgeAdhocPtpClientId = -1;
    
    int ret = sceNetGetLocalEtherAddr(mac);
        
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetGetLocalEtherAddr\n");
        return 0;
    }
    
    ret = sceNetAdhocPtpListen(mac, 1, 8192, 200*1000, 300, 1, 0);
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocPtpListen 0x%08X\n", ret);
        
        if(pgeAdhocPtpHostId > 0)
            sceNetAdhocPtpClose(pgeAdhocPtpHostId, 0);
            
        if(pgeAdhocPtpClientId > 0)
            sceNetAdhocPtpClose(pgeAdhocPtpClientId, 0);
        
        return ret;
    }
    
    pgeAdhocPtpHostId = ret;
        
    ret = sceNetAdhocPtpAccept(pgeAdhocPtpHostId, &clientmac[0], &clientport, 0, 0);
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocPtpAccept\n");
        
        if(pgeAdhocPtpHostId > 0)
            sceNetAdhocPtpClose(pgeAdhocPtpHostId, 0);
            
        if(pgeAdhocPtpClientId > 0)
            sceNetAdhocPtpClose(pgeAdhocPtpClientId, 0);
        
        return ret;
    }
    
    pgeAdhocPtpClientId = ret;
    
    return ret;
}

int pgeAdhocPtpClientStart(const char *servermac)
{
    unsigned char mac[6];
    
    sceNetEtherStrton(servermac, mac);
    
    unsigned char clientmac[6];
    
    int ret = sceNetGetLocalEtherAddr(clientmac);
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetGetLocalEtherAddr\n");
        return ret;
    }
    
    ret = sceNetAdhocPtpOpen(clientmac, 0, mac, 1, 8192, 200*1000, 300, 0);
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocPtpOpen\n");
        
        if(pgeAdhocPtpClientId > 0)
            sceNetAdhocPtpClose(pgeAdhocPtpClientId, 0);
        
        return ret;
    }
    
    pgeAdhocPtpClientId = ret;
    
    while(1)
    {    
        ret = sceNetAdhocPtpConnect(pgeAdhocPtpClientId, 0, 1);
            
        if(ret < 0 && ret != (int)0x80410709)
        {
            printf("pgeAdhoc error: sceNetAdhocPtpConnect 0x%08X\n", ret);
        
            if(pgeAdhocPtpClientId > 0)
                sceNetAdhocPtpClose(pgeAdhocPtpClientId, 0);
        
            return ret;
        }
        
        if(ret == 0)
            break;
        
        sceKernelDelayThread(200*1000);
    }
    
    return ret;
}

int pgeAdhocPtpReceive(void *data, int *length)
{    
    int ret = sceNetAdhocPtpRecv(pgeAdhocPtpClientId, data, length, 1000*1000, 0);
    
    if(ret < 0 && ret != (int)0x80410709)
    {
        printf("pgeAdhoc error: sceNetAdhocPtpRecv\n");
        
        if(pgeAdhocPtpClientId > 0)
            sceNetAdhocPtpClose(pgeAdhocPtpClientId, 0);
        
        return 0;
    }
    
    return 1;
}

int pgeAdhocPtpSend(void *data, int *length)
{
    int ret = sceNetAdhocPtpSend(pgeAdhocPtpClientId, data, length, 0, 0);
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocPtpSend\n");
        
        if(pgeAdhocPtpHostId > 0)
            sceNetAdhocPtpClose(pgeAdhocPtpHostId, 0);
            
        if(pgeAdhocPtpClientId > 0)
            sceNetAdhocPtpClose(pgeAdhocPtpClientId, 0);
        
        return(0);
    }
        
    return 1;
}

int pgeAdhocPtpCheckForData(void)
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

int pgeAdhocPtpFlush(void)
{
    int ret = sceNetAdhocPtpFlush(pgeAdhocPtpClientId, 0, 0);
    
    if(ret < 0)
    {
        printf("pgeAdhoc error: sceNetAdhocPtpFlush\n");
        
        if(pgeAdhocPtpHostId > 0)
            sceNetAdhocPtpClose(pgeAdhocPtpHostId, 0);
            
        if(pgeAdhocPtpClientId > 0)
            sceNetAdhocPtpClose(pgeAdhocPtpClientId, 0);
        
        return 0;
    }
    
    return 1;
}

int pgeAdhocPtpHostShutdown(void)
{
    if(pgeAdhocPtpHostId > 0)
        sceNetAdhocPtpClose(pgeAdhocPtpHostId, 0);
            
    if(pgeAdhocPtpClientId > 0)
        sceNetAdhocPtpClose(pgeAdhocPtpClientId, 0);
    
    return 1;
}

int pgeAdhocPtpClientShutdown(void)
{
    if(pgeAdhocPtpClientId > 0)
        sceNetAdhocPtpClose(pgeAdhocPtpClientId, 0);
    
    return 1;
}

int pgeAdhocGetError(void)
{
    return(pgeAdhocErrorCode);
}


static int lua_pgeAdhocInit(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 0)
        return luaL_error(L, "Argment error: pge.adhoc.init() takes no arguments.");
        
    int result = pgeAdhocInit(PGE_ADHOC_TYPE_NORMAL);
    
    if(result == 1)
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    
    return 1;
}

static int lua_pgeAdhocShutdown(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 0)
        return luaL_error(L, "Argment error: pge.adhoc.shutdown() takes no arguments.");
        
    lua_pushboolean(L, pgeAdhocShutdown());
    
    return 1;
}

static int lua_pgeAdhocConnect(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 1)
        return luaL_error(L, "Argment error: pge.adhoc.connect(name) takes one argument.");
        
    const char *name = luaL_checkstring(L, 1);
        
    lua_pushboolean(L, pgeAdhocConnect(name));
    
    return 1;
}

static int lua_pgeAdhocGetState(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 0)
        return luaL_error(L, "Argment error: pge.adhoc.state() takes no arguments.");
        
    lua_pushnumber(L, pgeAdhocGetState());
    
    return 1;
}

static int lua_pgeAdhocGetError(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 0)
        return luaL_error(L, "Argment error: pge.adhoc.geterror() takes no arguments.");
        
    lua_pushnumber(L, pgeAdhocGetError());
    
    return 1;
}

static int lua_pgeAdhocPeerExists(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 1)
        return luaL_error(L, "Argment error: pge.adhoc.peerexists() takes one argument.");
        
    const char *mac = luaL_checkstring(L, 1);
    
    lua_pushboolean(L, pgeAdhocPeerExists(mac));
    
    return 1;
}

static int lua_pgeAdhocMatchingInit(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 1 && argc != 2)
        return luaL_error(L, "Argment error: pge.adhoc.matching.init(type, [mode]) takes one or two arguments.");
        
    int type = luaL_checkint(L, 1);
    
    int mode = PGE_ADHOC_MATCHING_MODE_PTP;
    
    if(argc == 2)
        mode = luaL_checkint(L, 2);
        
    lua_pushboolean(L, pgeAdhocMatchingInit(mode, type));
    
    return 1;
}

static int lua_pgeAdhocMatchingStart(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 0 && argc != 1)
        return luaL_error(L, "Argment error: pge.adhoc.matching.start([hello]) takes zero or one argument.");
    
    const char *hello = NULL;
    
    if(argc == 1)
        hello = luaL_checkstring(L, 1);
        
    lua_pushboolean(L, pgeAdhocMatchingStart(hello));
    
    return 1;
}

static int lua_pgeAdhocMatchingAccept(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 1)
        return luaL_error(L, "Argment error: pge.adhoc.matching.accept(mac) takes one argument.");
    
    const char *mac = luaL_checkstring(L, 1);
        
    lua_pushboolean(L, pgeAdhocMatchingAccept(mac));
    
    return 1;
}

static int lua_pgeAdhocMatchingDecline(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 1)
        return luaL_error(L, "Argment error: pge.adhoc.matching.decline(mac) takes one argument.");
    
    const char *mac = luaL_checkstring(L, 1);
        
    lua_pushboolean(L, pgeAdhocMatchingDecline(mac));
    
    return 1;
}

static int lua_pgeAdhocMatchingShutdown(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 0)
        return luaL_error(L, "Argment error: pge.adhoc.matching.shutdown() takes no arguments.");
        
    lua_pushboolean(L, pgeAdhocMatchingShutdown());
    
    return 1;
}

static int lua_pgeAdhocMatchingGetEvents(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 0)
        return luaL_error(L, "Argment error: pge.adhoc.matching.events() takes no arguments.");
        
    pgeAdhocPeerEvent *event = pgeAdhocMatchingGetEvents();
        
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

static int lua_pgeAdhocPtpHostStart(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 0)
        return luaL_error(L, "Argment error: pge.adhoc.ptp.hoststart() takes no arguments.");
        
    lua_pushnumber(L, pgeAdhocPtpHostStart());
    
    return 1;
}

static int lua_pgeAdhocPtpClientStart(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 1)
        return luaL_error(L, "Argment error: pge.adhoc.ptp.clientstart(mac) takes one argument.");
        
    const char *mac = luaL_checkstring(L, 1);
        
    lua_pushnumber(L, pgeAdhocPtpClientStart(mac));
    
    return 1;
}

static int lua_pgeAdhocPtpSend(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 1)
        return luaL_error(L, "Argment error: pge.adhoc.ptp.send(data) takes one argument.");
        
    size_t size;
        
    char *data = (char *)luaL_checklstring(L, 1, &size);
        
    lua_pushboolean(L, pgeAdhocPtpSend(data, (int *)&size));
    
    return 1;
}

static int lua_pgeAdhocPtpReceive(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 0)
        return luaL_error(L, "Argment error: pge.adhoc.ptp.receive() takes no arguments.");
        
    int size = 128;
    
    char data[size];
    
    memset(data, 0, size);
    
    pgeAdhocPtpReceive(data, &size);
        
    lua_pushstring(L, data);
    
    return 1;
}

static int lua_pgeAdhocPtpCheckForData(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 0)
        return luaL_error(L, "Argment error: pge.adhoc.ptp.checkfordata() takes no arguments.");
        
    lua_pushboolean(L, pgeAdhocPtpCheckForData());
    
    return 1;
}

static int lua_pgeAdhocPtpFlush(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 0)
        return luaL_error(L, "Argment error: pge.adhoc.ptp.flush() takes no arguments.");
        
    lua_pushboolean(L, pgeAdhocPtpFlush());
    
    return 1;
}

static int lua_pgeAdhocPtpHostShutdown(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 0)
        return luaL_error(L, "Argment error: pge.adhoc.ptp.hostshutdown() takes no arguments.");
        
    lua_pushboolean(L, pgeAdhocPtpHostShutdown());
    
    return 1;
}

static int lua_pgeAdhocPtpClientShutdown(lua_State *L)
{
    int argc = lua_gettop(L);
    
    if(argc != 0)
        return luaL_error(L, "Argment error: pge.adhoc.ptp.clientshutdown() takes no arguments.");
        
    lua_pushboolean(L, pgeAdhocPtpClientShutdown());
    
    return 1;
}

static const luaL_reg lua_pgeAdhoc_functions[] =
{
    {"init",            lua_pgeAdhocInit},
    {"shutdown",        lua_pgeAdhocShutdown},
    {"connect",            lua_pgeAdhocConnect},
    {"state",            lua_pgeAdhocGetState},
    {"geterror",        lua_pgeAdhocGetError},
    {"peerexists",        lua_pgeAdhocPeerExists},
    {0, 0}
};

static const luaL_reg lua_pgeAdhocMatching_functions[] =
{
    {"init",        lua_pgeAdhocMatchingInit},
    {"start",        lua_pgeAdhocMatchingStart},
    {"accept",        lua_pgeAdhocMatchingAccept},
    {"decline",        lua_pgeAdhocMatchingDecline},
    {"shutdown",    lua_pgeAdhocMatchingShutdown},
    {"events",        lua_pgeAdhocMatchingGetEvents},
    {0, 0}
};

static const luaL_reg lua_pgeAdhocPtp_functions[] =
{
    {"hoststart",        lua_pgeAdhocPtpHostStart},
    {"clientstart",        lua_pgeAdhocPtpClientStart},
    {"receive",            lua_pgeAdhocPtpReceive},
    {"send",            lua_pgeAdhocPtpSend},
    {"checkfordata",    lua_pgeAdhocPtpCheckForData},
    {"flush",            lua_pgeAdhocPtpFlush},
    {"hostshutdown",    lua_pgeAdhocPtpHostShutdown},
    {"clientshutdown",    lua_pgeAdhocPtpClientShutdown},
    {0, 0}
};

void luaAdhoc_init(lua_State *L)
{
    luaL_openlib(L, "Adhoc", lua_pgeAdhoc_functions, 0);
    luaL_openlib(L, "AdhocMatching", lua_pgeAdhocMatching_functions, 0);
    luaL_openlib(L, "AdhocPtp", lua_pgeAdhocPtp_functions, 0);
    lua_pushnumber(L, PGE_ADHOC_EVENT_ERROR); lua_setglobal(L, "PGE_ADHOC_EVENT_ERROR");
    lua_pushnumber(L, PGE_ADHOC_EVENT_CONNECT); lua_setglobal(L, "PGE_ADHOC_EVENT_CONNECT");
    lua_pushnumber(L, PGE_ADHOC_EVENT_DISCONNECT); lua_setglobal(L, "PGE_ADHOC_EVENT_DISCONNECT");
    lua_pushnumber(L, PGE_ADHOC_EVENT_SCAN); lua_setglobal(L, "PGE_ADHOC_EVENT_SCAN");
    lua_pushnumber(L, PGE_ADHOC_EVENT_GAMEMODE); lua_setglobal(L, "PGE_ADHOC_EVENT_GAMEMODE");
    lua_pushnumber(L, PGE_ADHOC_EVENT_CANCEL); lua_setglobal(L, "PGE_ADHOC_EVENT_CANCEL");
    
    lua_pushnumber(L, PGE_ADHOC_MATCHING_EVENT_HELLO); lua_setglobal(L, "PGE_ADHOC_MATCHING_EVENT_HELLO");    
    lua_pushnumber(L, PGE_ADHOC_MATCHING_EVENT_REQUEST); lua_setglobal(L, "PGE_ADHOC_MATCHING_EVENT_REQUEST");
    lua_pushnumber(L, PGE_ADHOC_MATCHING_EVENT_LEAVE); lua_setglobal(L, "PGE_ADHOC_MATCHING_EVENT_LEAVE");
    lua_pushnumber(L, PGE_ADHOC_MATCHING_EVENT_DENY); lua_setglobal(L, "PGE_ADHOC_MATCHING_EVENT_DENY");
    lua_pushnumber(L, PGE_ADHOC_MATCHING_EVENT_CANCEL); lua_setglobal(L, "PGE_ADHOC_MATCHING_EVENT_CANCEL");
    lua_pushnumber(L, PGE_ADHOC_MATCHING_EVENT_ACCEPT); lua_setglobal(L, "PGE_ADHOC_MATCHING_EVENT_ACCEPT");
    lua_pushnumber(L, PGE_ADHOC_MATCHING_EVENT_ESTABLISHED); lua_setglobal(L, "PGE_ADHOC_MATCHING_EVENT_ESTABLISHED");
    lua_pushnumber(L, PGE_ADHOC_MATCHING_EVENT_TIMEOUT); lua_setglobal(L, "PGE_ADHOC_MATCHING_EVENT_TIMEOUT");
    lua_pushnumber(L, PGE_ADHOC_MATCHING_EVENT_ERROR); lua_setglobal(L, "PGE_ADHOC_MATCHING_EVENT_ERROR");
    lua_pushnumber(L, PGE_ADHOC_MATCHING_EVENT_BYE); lua_setglobal(L, "PGE_ADHOC_MATCHING_EVENT_BYE");
    
    lua_pushnumber(L, PGE_ADHOC_MATCHING_MODE_PTP); lua_setglobal(L, "PGE_ADHOC_MATCHING_MODE_PTP");
    
    lua_pushnumber(L, PGE_ADHOC_MATCHING_TYPE_HOST); lua_setglobal(L, "PGE_ADHOC_MATCHING_TYPE_HOST");
    lua_pushnumber(L, PGE_ADHOC_MATCHING_TYPE_CLIENT); lua_setglobal(L, "PGE_ADHOC_MATCHING_TYPE_CLIENT");
}
