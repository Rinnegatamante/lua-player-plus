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

#ifndef __UTILSL_H_
#define __UTILSL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../Types.h"
#include "isoloader.h"

#define LPP_UTILS_NET_DIALOG_ADHOC_CONNECT (2)
#define LPP_UTILS_NET_DIALOG_ADHOC_CREATE (4)
#define LPP_UTILS_NET_DIALOG_ADHOC_JOIN (5)
#define LPP_UTILS_ADHOC_GAMESHARE_NAME "GameShar"

#define LPP_UTILS_MSG_DIALOG_NO_OPTIONS 0x00000000
#define LPP_UTILS_MSG_DIALOG_YESNO_BUTTONS 0x00000010
#define LPP_UTILS_MSG_DIALOG_DEFAULT_BUTTON_NO 0x00000100

#define LPP_UTILS_MSG_DIALOG_RESULT_YES	(1)
#define LPP_UTILS_MSG_DIALOG_RESULT_NO (2)
#define LPP_UTILS_MSG_DIALOG_RESULT_BACK (3)

#define LPP_UTILS_SAVEDATA_TYPE_AUTOLOAD (0)
#define LPP_UTILS_SAVEDATA_TYPE_AUTOSAVE (1)
#define LPP_UTILS_SAVEDATA_TYPE_LISTLOAD (4)
#define LPP_UTILS_SAVEDATA_TYPE_LISTSAVE (5)
#define LPP_UTILS_SAVEDATA_TYPE_LISTDELETE (7)

/*#define LPP_UTILS_DRIVER_INFERNO (3)
#define LPP_UTILS_DRIVER_NP9660 */

#define LPP_UTILS_DIALOG_RUNNING (-1)

SceUID LPP_UtilsLoadStartModule(const char *filename);

int LPP_UtilsStopUnloadModule(SceUID modid);

int LPP_UtilsInitUsbStorage(void);

int LPP_UtilsStartUsbStorage(void);

int LPP_UtilsStopUsbStorage(void);

int LPP_UtilsDeinitUsbStorage(void);

int LPP_UtilsBrowserInit(int memorysize, const char *url);

int LPP_UtilsBrowserUpdate(void);

int LPP_UtilsNetDialogInit(void);

int LPP_UtilsNetDialogUpdate(void);

int LPP_AdhocDialogInit(int type, char *name);

int LPP_AdhocDialogUpdate(void);

int LPP_UtilsGameShareInit(const char *filepath, const char *name);

int LPP_UtilsGameShareUpdate(void);

int LPP_UtilsMsgDialogInit(int options, const char *text, ...);

int LPP_UtilsMsgDialogErrorInit(u32 error);

int LPP_UtilsMsgDialogUpdate(void);

int LPP_UtilsMsgDialogAbort(void);

int LPP_UtilsSavedataInit(int type, void *data, u32 datasize, const char *cPath, const char *gamename, const char *key, const char *title, const char *subtitle, const char *detail);

int LPP_UtilsSavedataUpdate(void);

u16 *LPP_UtilsConvertToUni(char *text);

size_t LPP_UtilsUniStrlen(u16 *text);

void LPP_UtilsConvertToAscii(char *outtext, u16 *text);

int LPP_UtilsOskInit(const char *description, const char *initialtext);

int LPP_UtilsOskUpdate(char *outtext);

int LPP_UtilsGetEboot(const char *ebp, const char *out, short type);

int LPP_UtilsCheckEboot(const char *ebp, short type);

int LPP_UtilsGetISO(const char *isopath, const char *input, const char *output);

int LPP_UtilsCheckISO(const char *isopath, const char *input);

int LPP_UtilsGetParamTitle(const char *filepath, char *title);

#ifdef __cplusplus
}
#endif

#endif /* __UTILSL_H_ */
