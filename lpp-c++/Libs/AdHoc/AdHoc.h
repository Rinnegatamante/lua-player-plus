/** LPP AdHoc lib by Nanni. */

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

#ifndef __ADHOCL_H_
#define __ADHOCL_H_

#include "../Types.h"

/* Macros */
#define LPP_ADHOC_ERROR_WLAN (-1)
#define LPP_ADHOC_ERROR_MAC (-2)
#define LPP_ADHOC_ERROR_MODULES (-3)
#define LPP_ADHOC_ERROR_NET_INIT (-4)
#define LPP_ADHOC_ERROR_INIT (-5)

#define LPP_ADHOC_ERROR_CTL_INIT (-6)
#define LPP_ADHOC_ERROR_CTL_CONNECT (-7)

#define LPP_ADHOC_ERROR_PDP_CREATE (-8)

#define LPP_ADHOC_ERROR_MATCHING_INIT (-9)
#define LPP_ADHOC_ERROR_MATCHING_CREATE (-10)
#define LPP_ADHOC_ERROR_MATCHING_START (-11)

#define LPP_ADHOC_MATCHING_JOINED (0x1)
#define LPP_ADHOC_MATCHING_SELECTED (0x2)
#define LPP_ADHOC_MATCHING_REJECTED (0x4)
#define LPP_ADHOC_MATCHING_CANCELED (0x5)
#define LPP_ADHOC_MATCHING_ACCEPTED (0x6)
#define LPP_ADHOC_MATCHING_ESTABILISHED (0x7)

#define LPP_ADHOC_MATCHING_DISCONNECT (0xa)

#define LPP_ADHOC_UNINIT (-1)
#define LPP_ADHOC_INIT (0)

#define LPP_ADHOC_MAX_REMOTEPSP (100)

enum LPP_Adhoc_RemotePSP_State {
    LPP_ADHOC_DISCONNECTED = 0x0,
	LPP_ADHOC_JOINED = 0x2,
	LPP_ADHOC_SELECTED = 0x4,
	LPP_ADHOC_REJECTED = 0x8,
	LPP_ADHOC_CANCELED = 0x16,
	LPP_ADHOC_ACCEPTED = 0x32,
	LPP_ADHOC_ESTABILISHED = 0x64
};

struct LPP_RemotePSP {
    u8 macAddress[8];
	char name[257];
	Int32 connectionState;
};

/**
 * Initialize the AdHoc library.
 *
 * @return < 0 on error.
 *
 */
Int32 LPP_AdhocInit(char *productID);

/**
 * Get the current mac address.
 */
u8 *LPP_Adhoc_GetMacAddress(void);

/**
 * Get the number of remotes psp.
 */
Int32 LPP_Adhoc_GetRemotePspCount(void);

/**
 * Get the remote psp with the given mac address.
 */
struct LPP_RemotePSP *LPP_Adhoc_GetPspByMac(const u8 aMacAddress[6]);

/**
 * Get the remote psp with the given index.
 */
struct LPP_RemotePSP *LPP_Adhoc_GetPspByIndex(u32 index);

/**
 * Return the current Adhoc state (INIT or UNINIT)
 */
Int32 LPP_Adhoc_GetConnectionState(void);

/**
 * Request a connection with a remote psp.
 *
 * @return the remote psp connection state.
 *
 */
Int32 LPP_AdhocRequestConnection(struct LPP_RemotePSP *aPsp, int timeOut, int (*requestConnectionCB)(int aPspState));

/**
 * Sends data to a remote psp.
 *
 */
Int32 LPP_Adhoc_SendData(struct LPP_RemotePSP *pPsp, void *data, u32 lenData);

/**
 * Receives data from a remote psp.
 *
 * @return the lenght of the data received.
 *
 */
Int32 LPP_Adhoc_ReceiveData(struct LPP_RemotePSP *pPsp, void *data, u32 maxLen);

/**
 *  Returns one psp that is requesting a connection.
 *
 */
struct LPP_RemotePSP *LPP_Adhoc_GetConnectionRequest(void);

/**
 * Rejects a connection request from a psp.
 *
 */
void LPP_Adhoc_RejectConnection(struct LPP_RemotePSP *aPsp);

/**
 * Accepts a connection request from a psp.
 *
 */
void LPP_Adhoc_AcceptConnection(struct LPP_RemotePSP *aPsp);

/**
 * Terminates the AdHoc library.
 */
void LPP_Adhoc_Term(void);

#endif /* __ADHOCL_H_ */