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
#-----------------------------------------------------------------------------------------------------------------------*/

//PSP adhoc functions.
// based on pspZorba's sample
// www.pspZorba.com

#include <pspkernel.h>
#include <pspsdk.h>
#include <pspwlan.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>
#include <pspnet_adhocmatching.h>
#include <psputility_sysparam.h>
#include <psputility.h>
#include <string.h>

#include "AdHoc.h"

static Int32 state = LPP_ADHOC_UNINIT;
static u8 myMacAddress[8];
static struct productStruct product;
static Int32 matchingHD = 0x0;
static Int32 pdpHD = 0x0;
static Int32 port = 0x0;

static struct LPP_RemotePSP allRemotePSP[LPP_ADHOC_MAX_REMOTEPSP];
static u32 RemotePSP_Count = 0;

static Int32 getMacAddress(void) {
    return sceWlanGetEtherAddr(myMacAddress);
}

static Int32 loadModules(void) {
    Int32 ret = -1;

	ret = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	if(ret < 0) return ret;

	ret = sceUtilityLoadNetModule(PSP_NET_MODULE_ADHOC);
	if(ret < 0) return ret;

	return 0;
}

static struct LPP_RemotePSP *findByMacAddress(const u8 aMacAddress[6])
{
    int i;
	for(i = 0; i < RemotePSP_Count; i++)
	{
	    if(memcmp(aMacAddress, allRemotePSP[i].macAddress, 6) == 0)
		    return &allRemotePSP[i];
	}

	return 0;
}

static struct LPP_RemotePSP *findByState(Int32 remotePspState)
{
    int i;
	for(i = 0; i < RemotePSP_Count; i++)
	{
	    if(allRemotePSP[i].connectionState == remotePspState)
		{
		    return &allRemotePSP[i];
		}
	}
	return 0;
}

static Int32 removeByMacAddress(const u8 aMacAddress[6])
{
    int i, found;
	for(i = 0; i < RemotePSP_Count; i++)
	{
	    if(memcmp(aMacAddress, allRemotePSP[i].macAddress, 6) == 0)
		{
		    found = 1;
		    RemotePSP_Count--;
		}
		if(found == 1) allRemotePSP[i] = allRemotePSP[i + 1];
	}
	return found;
}


static void matchingCB(Int32 unk1, Int32 event, u8 *macSource, Int32 size, void *data)
{
    struct LPP_RemotePSP *pPsp = findByMacAddress((u8*)macSource);

	switch(event)
	{
	    case LPP_ADHOC_MATCHING_JOINED :
		{
		    if(pPsp == 0L)
			{
			    char buffer[42];
				memcpy(buffer, data, 42);
				buffer[41] = 0;
				struct LPP_RemotePSP newPsp;
				memcpy(newPsp.macAddress, macSource, 8);
				strcpy(newPsp.name, buffer);
				newPsp.connectionState = LPP_ADHOC_JOINED;
				allRemotePSP[RemotePSP_Count++] = newPsp;
			}
		}
		break;

		case LPP_ADHOC_MATCHING_DISCONNECT :
		{
		    if(pPsp != 0) removeByMacAddress(pPsp->macAddress);
		}
		break;

		case LPP_ADHOC_MATCHING_SELECTED :
		{
		    if(pPsp != 0) pPsp->connectionState = LPP_ADHOC_SELECTED;
		}
		break;

		case LPP_ADHOC_MATCHING_REJECTED :
		{
		    if(pPsp != 0)
			{
			    pPsp->connectionState = LPP_ADHOC_REJECTED;
			}
		}
		break;

		case LPP_ADHOC_MATCHING_CANCELED :
		{
		    if(pPsp != 0)
			{
			    pPsp->connectionState = LPP_ADHOC_CANCELED;
			}
		}
		break;

		case LPP_ADHOC_MATCHING_ESTABILISHED :
		{
		    if(pPsp != 0)
			{
			    pPsp->connectionState = LPP_ADHOC_ESTABILISHED;
			}
		}
		break;

		case LPP_ADHOC_MATCHING_ACCEPTED :
		{
		    if(pPsp != 0)
			{
			    pPsp->connectionState = LPP_ADHOC_ACCEPTED;
			}
		}
		break;

		default : break;
	}
}

Int32 LPP_AdhocInit(char *productID)
{
    if(state != LPP_ADHOC_UNINIT) return 0;

	memset(myMacAddress, 0, 8);
	strncpy(product.product, productID, 9);
	product.unknown = 0x0;
	matchingHD = -1;
	pdpHD = -1;
	port =  0x309;
	RemotePSP_Count = 0;

	if(sceWlanDevIsPowerOn() != 1) return LPP_ADHOC_ERROR_WLAN;
	if(getMacAddress() < 0) return LPP_ADHOC_ERROR_MAC;
	if(loadModules() < 0) return LPP_ADHOC_ERROR_MODULES;
	if(sceNetInit(0x20000, 0x20, 0x1000, 0x20, 0x1000) < 0)
	{
	    return LPP_ADHOC_ERROR_NET_INIT;
	}
	if(sceNetAdhocInit() < 0) return LPP_ADHOC_ERROR_INIT;
	if(sceNetAdhocctlInit(0x2000, 0x20, &product) < 0)
	{
	    return LPP_ADHOC_ERROR_CTL_INIT;
	}
	if(sceNetAdhocctlConnect(0L) < 0) return LPP_ADHOC_ERROR_CTL_CONNECT;

	Int32 encore = 1, ret, s;

	while(encore) {
	    ret = sceNetAdhocctlGetState(&s);
		if(s == 1)
		{
		    encore = 0;
		}
		else
		{
		    sceKernelDelayThread(50 * 1000);
		}
	}

	if((pdpHD = sceNetAdhocPdpCreate(myMacAddress, port, 0x400, 0)) < 0)
	{
	    return LPP_ADHOC_ERROR_PDP_CREATE;
	}
	if(sceNetAdhocMatchingInit(0x20000) < 0) return LPP_ADHOC_ERROR_MATCHING_INIT;
	if((matchingHD = sceNetAdhocMatchingCreate(3, 0xa, 0x22b, 0x800, 0x2dc6c0, 0x5b8d80, 3, 0x7a120, matchingCB)) < 0)
	{
	    return LPP_ADHOC_ERROR_MATCHING_CREATE;
	}

	char name[50] = "";
	sceUtilityGetSystemParamString(PSP_SYSTEMPARAM_ID_STRING_NICKNAME, name, 50);

	if(sceNetAdhocMatchingStart(matchingHD, 0x10, 0x2000, 0x10, 0x2000, strlen(name), name) < 0)
	{
	    return LPP_ADHOC_ERROR_MATCHING_START;
	}

	state = LPP_ADHOC_INIT;
	return 0;
}

u8 *LPP_Adhoc_GetMacAddress(void)
{
    return myMacAddress;
}

Int32 LPP_Adhoc_GetRemotePspCount(void)
{
    return RemotePSP_Count;
}

struct LPP_RemotePSP *LPP_Adhoc_GetPspByMac(const u8 aMacAddress[6])
{
    return findByMacAddress(aMacAddress);
}

struct LPP_RemotePSP *LPP_Adhoc_GetPspByIndex(u32 index)
{
    if(index < RemotePSP_Count) return &allRemotePSP[index];
	else return 0;
}

Int32 LPP_Adhoc_GetConnectionState(void)
{
    return state;
}

Int32 LPP_AdhocRequestConnection(struct LPP_RemotePSP *aPsp, int timeOut, int (*requestConnectionCB)(int aPspState))
{
    Int32 ret = sceNetAdhocMatchingSelectTarget(matchingHD, (u8*)aPsp->macAddress, 0, 0);
	if(ret < 0) return ret;

	int quit = 0;
	time_t startTime;
	time_t currentTime;
	sceKernelLibcTime(&startTime);

	while(!quit)
	{
	    if(aPsp == 0) return -1;
		if(requestConnectionCB != 0)
		    quit = requestConnectionCB(aPsp->connectionState);
		if((aPsp->connectionState == LPP_ADHOC_ACCEPTED) || (aPsp->connectionState == LPP_ADHOC_ESTABILISHED))
		{
		    return aPsp->connectionState;
		}

		if(aPsp->connectionState == LPP_ADHOC_REJECTED)
		{
		    aPsp->connectionState = LPP_ADHOC_JOINED;
			return aPsp->connectionState;
		}
		sceKernelLibcTime(&currentTime);
		if(timeOut > 0 && currentTime - startTime >= timeOut) break;
		sceKernelDelayThread(1000 * 1000);
	}

	return -1;
}

Int32 LPP_Adhoc_SendData(struct LPP_RemotePSP *pPsp, void *data, u32 lenData)
{
    return sceNetAdhocPdpSend(pdpHD, pPsp->macAddress, port, data, lenData, 0, 0);
}

Int32 LPP_Adhoc_ReceiveData(struct LPP_RemotePSP *pPsp, void *data, u32 maxLen)
{
    pdpStatStruct aStat;
	aStat.next = 0;
	int sizeStat = sizeof(pdpStatStruct);
	u32 sizeData = maxLen;

	Int32 ret = sceNetAdhocGetPdpStat(&sizeStat, &aStat);
	if(ret < 0) return ret;

	if(aStat.rcvdData > 0)
	{
	    ret = sceNetAdhocPdpRecv(pdpHD, pPsp->macAddress, &(aStat.port), data, &sizeData, 0, 0);
		if(ret < 0) return ret;
		return aStat.rcvdData;
	}
	else
	{
	    return 0;
	}
}

struct LPP_RemotePSP *LPP_Adhoc_GetConnectionRequest(void)
{
    return findByState(LPP_ADHOC_SELECTED);
}

void LPP_Adhoc_RejectConnection(struct LPP_RemotePSP *aPsp)
{
    sceNetAdhocMatchingCancelTarget(matchingHD, (u8*)aPsp->macAddress);
	aPsp->connectionState = LPP_ADHOC_JOINED;
}

void LPP_Adhoc_AcceptConnection(struct LPP_RemotePSP *aPsp)
{
    sceNetAdhocMatchingSelectTarget(matchingHD, (u8*)aPsp->macAddress, 0, 0);
}

void LPP_Adhoc_Term(void)
{
    if(state != LPP_ADHOC_INIT) return;

	sceUtilityUnloadNetModule(PSP_NET_MODULE_ADHOC);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);

	sceNetAdhocctlDisconnect();
	sceNetAdhocPdpDelete(pdpHD, 0);
    sceNetAdhocMatchingStop(matchingHD);
    sceNetAdhocMatchingDelete( matchingHD);
    sceNetAdhocMatchingTerm();
    sceNetAdhocctlTerm();
    sceNetAdhocTerm();
    sceNetTerm();

	memset(myMacAddress, 0, 8);
	matchingHD = 0;
	pdpHD = 0;
	port = 0;
	state = LPP_ADHOC_UNINIT;
}
