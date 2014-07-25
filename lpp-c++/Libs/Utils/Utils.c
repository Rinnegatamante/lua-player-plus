/** LPP Utils lib by Nanni */

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

#include <pspkernel.h>
#include <psputility.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <stdarg.h>
#include <pspnet_adhocmatching.h>
#include <pspnet_adhoc.h>
#include <psputility_gamesharing.h>
#include <psputility_htmlviewer.h>
#include <psphttp.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <pspreg.h>

#include "../Dir/Dir.h"
#include "../Kubridge/kubridge.h"
#include "Utils.h"

static char lpp_UtilsSavedataSaveName[20]  = "0000";
static pspUtilityHtmlViewerParam lpp_UtilsBrowserParams;
static SceUtilityOskParams lpp_UtilsOskDialogParams;
static SceUtilityOskData lpp_UtilsOskDialogData;
static u16 lpp_UtilsOskOutText[512];
SceUtilitySavedataParam lpp_UtilsSavedataParams;
static void *lpp_UtilsSaveDataData = null;
static pspUtilityNetconfData lpp_UtilsNetconfData;
static struct pspUtilityNetconfAdhoc adhocparam;
static pspUtilityMsgDialogParams lpp_UtilsMsgDialogParams;
static pspUtilityGameSharingParams lpp_UtilsGameShareParams;

typedef struct {
    char signature[4];
    int version;
    u32 offset[8];
} EBOOT_HEADER;

typedef struct __attribute__((packed)) {
    char signature[4];
    char version[4];
    u32 fields_table_offs;
    u32 data_table_offs;
    u32 entry_numbers;
} SFO_HEADER;

typedef struct __attribute__((packed)) {
    u16 field_offs;
	u8 unk;
	u8 type;
	u32 length;
	u32 size;
	u16 val_offs;
	u16 unk4;
} SFO_DIR;

char lpp_UtilsSaveNameMultiple[][20] =
{
    "0000",
    "0001",
    "0002",
    "0003",
    "0004",
    ""
};

#ifdef DEBUG
extern int dwrite_output(const char*, ...);
#endif

int LPP_UtilsBrowserInit(int memorysize, const char *url)
{
    sceHttpInit(0x25800);

    memset(&lpp_UtilsBrowserParams, 0, sizeof(lpp_UtilsBrowserParams));

    lpp_UtilsBrowserParams.base.size = sizeof(lpp_UtilsBrowserParams);
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &lpp_UtilsBrowserParams.base.language);
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &lpp_UtilsBrowserParams.base.buttonSwap);

    lpp_UtilsBrowserParams.base.graphicsThread = 17;
    lpp_UtilsBrowserParams.base.accessThread = 19;
    lpp_UtilsBrowserParams.base.fontThread = 18;
    lpp_UtilsBrowserParams.base.soundThread = 16;

    lpp_UtilsBrowserParams.memsize = memorysize;
    lpp_UtilsBrowserParams.initialurl = (char *)url;
    lpp_UtilsBrowserParams.numtabs = 3;
    lpp_UtilsBrowserParams.textsize = PSP_UTILITY_HTMLVIEWER_TEXTSIZE_SMALL;
    lpp_UtilsBrowserParams.connectmode = PSP_UTILITY_HTMLVIEWER_CONNECTMODE_MANUAL_ALL;
    lpp_UtilsBrowserParams.textsize = PSP_UTILITY_HTMLVIEWER_TEXTSIZE_NORMAL;
    lpp_UtilsBrowserParams.interfacemode = PSP_UTILITY_HTMLVIEWER_INTERFACEMODE_FULL;
    lpp_UtilsBrowserParams.displaymode = PSP_UTILITY_HTMLVIEWER_DISPLAYMODE_SMART_FIT;

    lpp_UtilsBrowserParams.memaddr = malloc(memorysize);
    if(!lpp_UtilsBrowserParams.memaddr) return -1;

    int res = sceUtilityHtmlViewerInitStart(&lpp_UtilsBrowserParams);
    if(res == 0) return 1;

    return res;
}

int LPP_UtilsBrowserUpdate(void)
{
    int res = -1;
    int stat = sceUtilityHtmlViewerGetStatus();

    switch(stat)
    {
    case PSP_UTILITY_DIALOG_NONE :
        {
            res = lpp_UtilsBrowserParams.base.result;
            if(lpp_UtilsBrowserParams.memaddr)
                free(lpp_UtilsBrowserParams.memaddr);
            sceHttpEnd();
        } break;

    case PSP_UTILITY_DIALOG_VISIBLE :
        {
            sceUtilityHtmlViewerUpdate(1);
        } break;

    case PSP_UTILITY_DIALOG_QUIT :
        {
            sceUtilityHtmlViewerShutdownStart();
        } break;

    case PSP_UTILITY_DIALOG_FINISHED :
        break;

    default:
        break;
    }

    return res;
}

int LPP_UtilsNetDialogInit(void)
{
    memset(&lpp_UtilsNetconfData, 0, sizeof(lpp_UtilsNetconfData));
    lpp_UtilsNetconfData.base.size = sizeof(lpp_UtilsNetconfData);
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &lpp_UtilsNetconfData.base.language);
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &lpp_UtilsNetconfData.base.buttonSwap);

    lpp_UtilsNetconfData.base.graphicsThread = 17;
    lpp_UtilsNetconfData.base.accessThread = 19;
    lpp_UtilsNetconfData.base.fontThread = 18;
    lpp_UtilsNetconfData.base.soundThread = 16;
    lpp_UtilsNetconfData.action = 0;

    struct pspUtilityNetconfAdhoc adhocparam;
    memset(&adhocparam, 0, sizeof(adhocparam));
    lpp_UtilsNetconfData.adhocparam = &adhocparam;

    int res = sceUtilityNetconfInitStart(&lpp_UtilsNetconfData);
    if(res == 0) return 1;

    return res;
}

int LPP_UtilsNetDialogUpdate(void)
{
    int res = -1;

    switch(sceUtilityNetconfGetStatus())
    {
    case PSP_UTILITY_DIALOG_NONE :
        res = lpp_UtilsNetconfData.base.result;
        break;
    case PSP_UTILITY_DIALOG_VISIBLE :
        sceUtilityNetconfUpdate(1);
        break;
    case PSP_UTILITY_DIALOG_QUIT :
        sceUtilityNetconfShutdownStart();
        break;
    case PSP_UTILITY_DIALOG_FINISHED :
        break;
    default:
        break;
    }

    return res;
}

int LPP_AdhocDialogInit(int type, char *name)
{
    memset(&adhocparam, 0, sizeof(adhocparam));
    kuKernelMemcpy(&adhocparam.name, name, sizeof(adhocparam.name));
    adhocparam.timeout = 60;

    memset(&lpp_UtilsNetconfData, 0, sizeof(lpp_UtilsNetconfData));
    lpp_UtilsNetconfData.base.size = sizeof(lpp_UtilsNetconfData);

    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &lpp_UtilsNetconfData.base.language);
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &lpp_UtilsNetconfData.base.buttonSwap);

    lpp_UtilsNetconfData.base.graphicsThread = 17;
    lpp_UtilsNetconfData.base.accessThread = 19;
    lpp_UtilsNetconfData.base.fontThread = 18;
    lpp_UtilsNetconfData.base.soundThread = 16;
    lpp_UtilsNetconfData.action = type;

    lpp_UtilsNetconfData.adhocparam = &adhocparam;

    int res = sceUtilityNetconfInitStart(&lpp_UtilsNetconfData);
    if(res == 0) return 1;

    return res;
}

int LPP_AdhocDialogUpdate(void)
{
    int res = -1;

    switch(sceUtilityNetconfGetStatus())
    {
    case PSP_UTILITY_DIALOG_NONE :
        {
            res = lpp_UtilsNetconfData.base.result;
        } break;
    case PSP_UTILITY_DIALOG_VISIBLE :
        {
            sceUtilityNetconfUpdate(1);
        } break;
    case PSP_UTILITY_DIALOG_QUIT :
        {
            sceUtilityNetconfShutdownStart();
        } break;
        case PSP_UTILITY_DIALOG_FINISHED :
            break;
        default :
            break;
    }

    return(res);
}

int LPP_UtilsGameShareInit(const char *filepath, const char *name)
{
    sceNetAdhocMatchingInit(32*1024);

    memset(&lpp_UtilsGameShareParams, 0, sizeof(lpp_UtilsGameShareParams));
    lpp_UtilsGameShareParams.base.size = sizeof(lpp_UtilsGameShareParams);

    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &lpp_UtilsGameShareParams.base.language);
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &lpp_UtilsGameShareParams.base.buttonSwap);

    lpp_UtilsGameShareParams.base.graphicsThread = 17;
    lpp_UtilsGameShareParams.base.accessThread = 19;
    lpp_UtilsGameShareParams.base.fontThread = 18;
    lpp_UtilsGameShareParams.base.soundThread = 16;

    size_t lsize = 0;

    SceUID fd = sceIoOpen(filepath, PSP_O_RDONLY, 0777);
    if(fd < 0) return -1;

    lsize = sceIoLseek32(fd, 0, PSP_SEEK_END);

    u8 *buffer = (u8*)malloc(lsize);

    if(buffer == null)
    {
        sceIoClose(fd);
        return -1;
    }

    sceIoLseek32(fd, 0, PSP_SEEK_SET);

    int read = sceIoRead(fd, buffer,lsize);

    if(read < lsize)
    {
        sceIoClose(fd);
        free(buffer);
        return -1;
    }

    sceIoClose(fd);

    buffer[276] = 0x57;
    strncpy((char *)&buffer[320], name, 127);

    kuKernelMemcpy(&lpp_UtilsGameShareParams.name, "GameShar", 8);

    lpp_UtilsGameShareParams.mode = 1;
    lpp_UtilsGameShareParams.datatype = 2;

    lpp_UtilsGameShareParams.data = buffer;
    lpp_UtilsGameShareParams.datasize = lsize;

    int res = sceUtilityGameSharingInitStart(&lpp_UtilsGameShareParams);

    if(res == 0) return 1;

    return (res);
}

int LPP_UtilsGameShareUpdate(void)
{
    int res = -1;

    switch(sceUtilityGameSharingGetStatus())
    {
    case PSP_UTILITY_DIALOG_NONE :
        {
            sceNetAdhocMatchingTerm();
            res = lpp_UtilsGameShareParams.base.result;
        } break;
    case PSP_UTILITY_DIALOG_VISIBLE :
        sceUtilityGameSharingUpdate(1);
        break;
    case PSP_UTILITY_DIALOG_QUIT :
        sceUtilityGameSharingShutdownStart();
        break;
    default :
        break;
    }

    return(res);
}

int LPP_UtilsMsgDialogInit(int options, const char *text, ...)
{
    char buffer[512];
    va_list ap;

    va_start(ap, text);
    vsnprintf(buffer, 512, text, ap);
    va_end(ap);

    memset(&lpp_UtilsMsgDialogParams, 0, sizeof(lpp_UtilsMsgDialogParams));
    lpp_UtilsMsgDialogParams.base.size = sizeof(lpp_UtilsMsgDialogParams);

    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &lpp_UtilsMsgDialogParams.base.language);
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &lpp_UtilsMsgDialogParams.base.buttonSwap);

    lpp_UtilsMsgDialogParams.base.graphicsThread = 17;
    lpp_UtilsMsgDialogParams.base.accessThread = 19;
    lpp_UtilsMsgDialogParams.base.fontThread = 18;
    lpp_UtilsMsgDialogParams.base.soundThread = 16;

    lpp_UtilsMsgDialogParams.mode = PSP_UTILITY_MSGDIALOG_MODE_TEXT;
    lpp_UtilsMsgDialogParams.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT;
    lpp_UtilsMsgDialogParams.options |= options;

    strcpy(lpp_UtilsMsgDialogParams.message, buffer);

    int res = sceUtilityMsgDialogInitStart(&lpp_UtilsMsgDialogParams);
    if(res == 0) return 1;

    return(res);
}

int LPP_UtilsMsgDialogErrorInit(u32 error)
{
    memset(&lpp_UtilsMsgDialogParams, 0, sizeof(lpp_UtilsMsgDialogParams));
    lpp_UtilsMsgDialogParams.base.size = sizeof(lpp_UtilsMsgDialogParams);

    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &lpp_UtilsMsgDialogParams.base.language);
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &lpp_UtilsMsgDialogParams.base.buttonSwap);

    lpp_UtilsMsgDialogParams.base.graphicsThread = 17;
    lpp_UtilsMsgDialogParams.base.accessThread = 19;
    lpp_UtilsMsgDialogParams.base.fontThread = 18;
    lpp_UtilsMsgDialogParams.base.soundThread = 16;

    lpp_UtilsMsgDialogParams.mode = PSP_UTILITY_MSGDIALOG_MODE_ERROR;
    lpp_UtilsMsgDialogParams.options = PSP_UTILITY_MSGDIALOG_OPTION_ERROR;
    lpp_UtilsMsgDialogParams.errorValue = error;

    int res = sceUtilityMsgDialogInitStart(&lpp_UtilsMsgDialogParams);
    if(res == 0) return 1;

    return(res);
}

int LPP_UtilsMsgDialogUpdate(void)
{
    int res = -1;

    switch(sceUtilityMsgDialogGetStatus())
    {
    case PSP_UTILITY_DIALOG_NONE :
        res = lpp_UtilsMsgDialogParams.buttonPressed;
        break;
    case PSP_UTILITY_DIALOG_VISIBLE:
        sceUtilityMsgDialogUpdate(1);
        break;
    case PSP_UTILITY_DIALOG_QUIT :
        sceUtilityMsgDialogShutdownStart();
        break;
    case PSP_UTILITY_DIALOG_FINISHED:
        break;
    default:
        break;
    }

    if(res != -1)
    {
        if(!(lpp_UtilsMsgDialogParams.options & LPP_UTILS_MSG_DIALOG_YESNO_BUTTONS))
            res = LPP_UTILS_MSG_DIALOG_RESULT_BACK;
    }

    return(res);
}

int LPP_UtilsMsgDialogAbort(void)
{
    int res = sceUtilityMsgDialogAbort();
    if(res == 0) return 1;
    return(res);
}

int LPP_UtilsSavedataInit(int type, void *data, u32 datasize, const char *cPath, const char *gamename, const char *key, const char *title, const char *subtitle, const char *detail)
{
    lpp_UtilsSaveDataData = data;

    PspUtilitySavedataListSaveNewData newData;
    memset(&newData, 0, sizeof(newData));

    memset(&lpp_UtilsSavedataParams, 0, sizeof(lpp_UtilsSavedataParams));
    lpp_UtilsSavedataParams.base.size = sizeof(lpp_UtilsSavedataParams);

    void *icon0data = null;
    size_t icon0size = 0;
    void *pic1data = null;
    size_t pic1size = 0;
    void* snd0data = null;
    size_t snd0size = 0;

    char *titleshow = "New Save";

    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &lpp_UtilsSavedataParams.base.language);
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &lpp_UtilsSavedataParams.base.buttonSwap);

    lpp_UtilsSavedataParams.base.graphicsThread = 17;
    lpp_UtilsSavedataParams.base.accessThread = 19;
    lpp_UtilsSavedataParams.base.fontThread = 18;
    lpp_UtilsSavedataParams.base.soundThread = 16;

    lpp_UtilsSavedataParams.mode = type;

    lpp_UtilsSavedataParams.overwrite = 1;

    if(type == LPP_UTILS_SAVEDATA_TYPE_LISTLOAD)
    {
        lpp_UtilsSavedataParams.focus = PSP_UTILITY_SAVEDATA_FOCUS_LATEST;
    }
    else
    {
        lpp_UtilsSavedataParams.focus = PSP_UTILITY_SAVEDATA_FOCUS_FIRSTEMPTY;
    }

    strncpy(lpp_UtilsSavedataParams.key, key, 16);
    strncpy(lpp_UtilsSavedataParams.gameName, gamename, 9);
    strcpy(lpp_UtilsSavedataParams.saveName, "<>");

    if(type == LPP_UTILS_SAVEDATA_TYPE_AUTOLOAD || type == LPP_UTILS_SAVEDATA_TYPE_AUTOSAVE)
    {
        strcpy(lpp_UtilsSavedataParams.saveName, lpp_UtilsSavedataSaveName);
    }
    else
    {
        lpp_UtilsSavedataParams.saveNameList = lpp_UtilsSaveNameMultiple;
    }

    strcpy(lpp_UtilsSavedataParams.fileName, "DATA.BIN");

    lpp_UtilsSavedataParams.dataBuf = malloc(datasize);
    lpp_UtilsSavedataParams.dataBufSize = datasize;
    lpp_UtilsSavedataParams.dataSize = datasize;

    if(type == LPP_UTILS_SAVEDATA_TYPE_AUTOSAVE || type == LPP_UTILS_SAVEDATA_TYPE_LISTSAVE)
    {
        memset(lpp_UtilsSavedataParams.dataBuf, 0, datasize);
        strncpy(lpp_UtilsSavedataParams.dataBuf, data, datasize);

        strcpy(lpp_UtilsSavedataParams.sfoParam.title, title);
        strcpy(lpp_UtilsSavedataParams.sfoParam.savedataTitle, subtitle);
        strcpy(lpp_UtilsSavedataParams.sfoParam.detail, detail);
        lpp_UtilsSavedataParams.sfoParam.parentalLevel = 0;

        if(type != LPP_UTILS_SAVEDATA_TYPE_AUTOSAVE)
        {
            if(strcmp(cPath, "EBOOT.PBP") == 0 || strcmp(cPath, "EBOOT.PBP/") == 0 || strcmp(cPath, "EBOOT.PBP\\") == 0)
            {
                SceUID fd = sceIoOpen(cPath, PSP_O_RDONLY, 0777);
                if(fd >= 0)
                {
                    EBOOT_HEADER pbpHeader;
                    memset(&pbpHeader, 0, sizeof(pbpHeader));

                    sceIoRead(fd, &pbpHeader, sizeof(pbpHeader));

                    u32 filesize = pbpHeader.offset[2] - pbpHeader.offset[1];
                    if(filesize > 0)
                    {
                        sceIoLseek32(fd, pbpHeader.offset[1], PSP_SEEK_SET);
                        icon0data = malloc(filesize);
                        icon0size = filesize;
                        sceIoRead(fd, icon0data, filesize);
                    }

                    filesize = pbpHeader.offset[5] - pbpHeader.offset[4];
                    if(filesize)
                    {
                        sceIoLseek32(fd, pbpHeader.offset[4], PSP_SEEK_SET);
                        pic1data = malloc(filesize);
                        pic1size = filesize;
                        sceIoRead(fd, pic1data, filesize);
                    }

                    filesize = pbpHeader.offset[6] - pbpHeader.offset[5];
                    if(filesize)
                    {
                        sceIoLseek32(fd, pbpHeader.offset[5], PSP_SEEK_SET);
                        snd0data = malloc(filesize);
                        snd0size = filesize;
                        sceIoRead(fd, snd0data, filesize);
                    }

                    sceIoClose(fd);
                }
            } else {
                char fname[512];

                u8 o = cPath[strlen(cPath) - 1] == '/' || cPath[strlen(cPath) - 1] == '\\';

                sprintf(fname, o ? "%sICON0.PNG" : "%s/ICON0.PNG", cPath);
                if(LPP_FileExists(fname))
                {
                    SceUID fd = sceIoOpen(fname, PSP_O_RDONLY, 0777);
                    icon0size = sceIoLseek32(fd, 0, PSP_SEEK_END);
                    sceIoLseek32(fd, 0, PSP_SEEK_SET);

                    icon0data = malloc(icon0size);
                    sceIoRead(fd, icon0data, icon0size);
                    sceIoClose(fd);
                }

                sprintf(fname, o ? "%sPIC1.PNG" : "%s/PIC1.PNG", cPath);
                if(LPP_FileExists(fname))
                {
                    SceUID fd = sceIoOpen(fname, PSP_O_RDONLY, 0777);
                    pic1size = sceIoLseek32(fd, 0, PSP_SEEK_END);
                    sceIoLseek32(fd, 0, PSP_SEEK_SET);

                    pic1data = malloc(pic1size);
                    sceIoRead(fd, pic1data, pic1size);
                    sceIoClose(fd);
                }

                sprintf(fname, o ? "%sSND0.AT3" : "%s/SND0.AT3", cPath);
                if(LPP_FileExists(fname))
                {
                    SceUID fd = sceIoOpen(fname, PSP_O_RDONLY, 0777);
                    snd0size = sceIoLseek32(fd, 0, PSP_SEEK_END);
                    sceIoLseek32(fd, 0, PSP_SEEK_SET);

                    snd0data = malloc(snd0size);
                    sceIoRead(fd, snd0data, snd0size);
                    sceIoClose(fd);
                }
            }
        }

        lpp_UtilsSavedataParams.icon1FileData.buf = null;
        lpp_UtilsSavedataParams.icon1FileData.bufSize = 0;
        lpp_UtilsSavedataParams.icon1FileData.size = 0;

        lpp_UtilsSavedataParams.pic1FileData.buf = pic1data;
        lpp_UtilsSavedataParams.pic1FileData.bufSize = pic1size;
        lpp_UtilsSavedataParams.pic1FileData.size = pic1size;

        lpp_UtilsSavedataParams.icon0FileData.buf = icon0data;
        lpp_UtilsSavedataParams.icon0FileData.bufSize = icon0size;
        lpp_UtilsSavedataParams.icon0FileData.size = icon0size;

        lpp_UtilsSavedataParams.snd0FileData.buf = snd0data;
        lpp_UtilsSavedataParams.snd0FileData.bufSize = snd0size;
        lpp_UtilsSavedataParams.snd0FileData.size = snd0size;

        newData.title = titleshow;

        lpp_UtilsSavedataParams.newData = &newData;
    }

    int res = sceUtilitySavedataInitStart(&lpp_UtilsSavedataParams);
    if(res == 0) return 1;

    return(res);
}

int LPP_UtilsSavedataUpdate(void)
{
    int res = -1;

    switch(sceUtilitySavedataGetStatus())
    {
    case PSP_UTILITY_DIALOG_NONE:
        break;
    case PSP_UTILITY_DIALOG_VISIBLE:
        sceUtilitySavedataUpdate(1);
        break;
    case PSP_UTILITY_DIALOG_QUIT:
        sceUtilitySavedataShutdownStart();
        strcpy(lpp_UtilsSavedataSaveName, lpp_UtilsSavedataParams.saveName);

        if(lpp_UtilsSavedataParams.mode == LPP_UTILS_SAVEDATA_TYPE_AUTOLOAD || lpp_UtilsSavedataParams.mode == LPP_UTILS_SAVEDATA_TYPE_LISTLOAD)
        {
            kuKernelMemcpy(lpp_UtilsSaveDataData, lpp_UtilsSavedataParams.dataBuf, lpp_UtilsSavedataParams.dataBufSize);
        }

        if(lpp_UtilsSavedataParams.pic1FileData.buf != null)
        {
            free(lpp_UtilsSavedataParams.pic1FileData.buf);
        }

        if(lpp_UtilsSavedataParams.icon0FileData.buf != null)
        {
            free(lpp_UtilsSavedataParams.icon0FileData.buf);
        }

        if(lpp_UtilsSavedataParams.snd0FileData.buf != null)
        {
            free(lpp_UtilsSavedataParams.snd0FileData.buf);
        }

        if(lpp_UtilsSavedataParams.dataBuf != null)
        {
            free(lpp_UtilsSavedataParams.dataBuf);
        }

        res = lpp_UtilsSavedataParams.base.result;
        break;
    default:
        break;
    }

    return(res);
}

u16 *LPP_UtilsConvertToUni(char *text)
{
    u16 *out = (u16*)malloc((strlen(text) + 1) << 1);
    if(!out)
    {
        return null;
    }

    u8 *txtPtr = (u8*)text;
    u16 *outPtr = out;

    while(txtPtr[0] != 0) *(outPtr++) = *(txtPtr++);
    out[strlen(text)] = 0;

    return(out);
}

size_t LPP_UtilsUniStrlen(u16 *text)
{
    u16 *txt_ptr = text;
    size_t out = 0;

    while(txt_ptr[0] != 0)
    {
        out++;
        txt_ptr++;
    }

    return(out);
}

void LPP_UtilsConvertToAscii(char *outtext, u16 *text)
{
    u8 *out_ptr = (u8*)outtext;
    u16 *txt_ptr = text;

    while(txt_ptr[0] != 0) *(out_ptr++) = (*(txt_ptr++) & 0x00FF);
    outtext[LPP_UtilsUniStrlen(text)] = 0;
}

int LPP_UtilsOskInit(const char *description, const char *initialtext)
{
    memset(&lpp_UtilsOskDialogParams, 0, sizeof(lpp_UtilsOskDialogParams));
    memset(&lpp_UtilsOskDialogData, 0, sizeof(lpp_UtilsOskDialogData));

    lpp_UtilsOskDialogData.language = PSP_UTILITY_OSK_LANGUAGE_DEFAULT;
    lpp_UtilsOskDialogData.lines = 1;
    lpp_UtilsOskDialogData.unk_24 = 1;
    lpp_UtilsOskDialogData.inputtype = PSP_UTILITY_OSK_INPUTTYPE_ALL;

    if(description != null)
    {
        lpp_UtilsOskDialogData.desc = LPP_UtilsConvertToUni((char*)description);
    }

    if(initialtext != null)
    {
        lpp_UtilsOskDialogData.intext = LPP_UtilsConvertToUni((char*)initialtext);
    }

    lpp_UtilsOskDialogData.outtextlength = 512;
    lpp_UtilsOskDialogData.outtextlimit = 512;
    lpp_UtilsOskDialogData.outtext = lpp_UtilsOskOutText;

    lpp_UtilsOskDialogParams.base.size = sizeof(lpp_UtilsOskDialogParams);

    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &lpp_UtilsOskDialogParams.base.language);
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &lpp_UtilsOskDialogParams.base.buttonSwap);

    lpp_UtilsOskDialogParams.base.graphicsThread = 17;
    lpp_UtilsOskDialogParams.base.accessThread = 19;
    lpp_UtilsOskDialogParams.base.fontThread = 18;
    lpp_UtilsOskDialogParams.base.soundThread = 16;

    lpp_UtilsOskDialogParams.datacount = 1;
    lpp_UtilsOskDialogParams.data = &lpp_UtilsOskDialogData;

    int res = sceUtilityOskInitStart(&lpp_UtilsOskDialogParams);
    if(res == 0) return 1;
    return(res);
}

int LPP_UtilsOskUpdate(char *outtext)
{
    int res = -1;

    switch(sceUtilityOskGetStatus())
    {
    case PSP_UTILITY_DIALOG_NONE:
        res = lpp_UtilsOskDialogData.result;

        if(lpp_UtilsOskDialogData.desc) free(lpp_UtilsOskDialogData.desc);
        if(lpp_UtilsOskDialogData.intext) free(lpp_UtilsOskDialogData.intext);

        LPP_UtilsConvertToAscii(outtext, lpp_UtilsOskDialogParams.data->outtext);
        break;

    case PSP_UTILITY_DIALOG_VISIBLE:
        sceUtilityOskUpdate(1);
        break;
    case PSP_UTILITY_DIALOG_QUIT:
        sceUtilityOskShutdownStart();
        break;
    case PSP_UTILITY_DIALOG_FINISHED:
        break;
    default:
        break;
    }

    return(res);
}

SceUID LPP_UtilsLoadStartModule(const char *filename)
{
    SceUID r = kuKernelLoadModule(filename, 0, null);
    if(r <= 0) return 0;

    int status;

    int res = sceKernelStartModule(r, 0, null, &status, null);

    if(res < 0)
    {
        return 0;
    }

    return (r);
}

int LPP_UtilsStopUnloadModule(SceUID modid)
{
    int status;

    int res = sceKernelStopModule(modid, 0, null, &status, null);

    if(res < 0)
    {
        return 0;
    }

    res = sceKernelUnloadModule(modid);

    return (res);
}

int usb1, usb2, usb3, usb4, usb5;
u8 usbStarted = 0;

int LPP_UtilsInitUsbStorage(void)
{
    usb1 = LPP_UtilsLoadStartModule("flash0:/kd/semawm.prx");
    if(usb1 < 0)
    {
        return -1;
    }
    usb2 = LPP_UtilsLoadStartModule("flash0/:kd/usbstor.prx");
    if(usb2 < 0)
    {
        LPP_UtilsStopUnloadModule(usb1);
        return -1;
    }
    usb3 = LPP_UtilsLoadStartModule("flash0:/kd/usbstormgr.prx");
    if(usb3 < 0)
    {
        LPP_UtilsStopUnloadModule(usb1);
        LPP_UtilsStopUnloadModule(usb2);
        return -1;
    }
    usb4 = LPP_UtilsLoadStartModule("flash0:/kd/usbstorms.prx");
    if(usb4 < 0)
    {
        LPP_UtilsStopUnloadModule(usb1);
        LPP_UtilsStopUnloadModule(usb2);
        LPP_UtilsStopUnloadModule(usb3);
        return -1;
    }
    usb5 = LPP_UtilsLoadStartModule("flash0:/kd/usbstorboot.prx");
    if(usb5 < 0)
    {
        LPP_UtilsStopUnloadModule(usb1);
        LPP_UtilsStopUnloadModule(usb2);
        LPP_UtilsStopUnloadModule(usb3);
        LPP_UtilsStopUnloadModule(usb4);
        return -1;
    }

    return 0;
}

int LPP_UtilsStartUsbStorage(void)
{
    if(usb1 < 0 || usb2 < 0 || usb3 < 0 || usb4 < 0 || usb5 < 0)
    {
        return -1;
    }
    if(usbStarted) return 1;

    sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
    sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
    sceUsbstorBootSetCapacity(0x800000);
    sceUsbActivate(0x1c8);

    usbStarted = 1;

    return 0;

}

int LPP_UtilsStopUsbStorage(void)
{
    if(!usbStarted) return 1;

    sceUsbDeactivate(0x1c8);
    sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0);
    sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
    usbStarted = 0;

    return 0;
}

int LPP_UtilsDeinitUsbStorage(void)
{
    if(usb1 < 0 || usb2 < 0 || usb3 < 0 || usb4 < 0 || usb5 < 0)
    {
        return -1;
    }
    if(usbStarted) return -1;

    LPP_UtilsStopUnloadModule(usb1);
    LPP_UtilsStopUnloadModule(usb2);
    LPP_UtilsStopUnloadModule(usb3);
    LPP_UtilsStopUnloadModule(usb4);
    LPP_UtilsStopUnloadModule(usb5);

    return 0;
}

int LPP_UtilsGetEboot(const char *ebp, const char *out, short type)
{
    u8 *buf = null;
	size_t off1, off2, size;
	EBOOT_HEADER header;
	memset(&header, 0, sizeof(EBOOT_HEADER));

	if(type < 0) type = 0;
	if(type > 7) type = 7;
	FILE *fp = fopen(ebp, "rb");
	if(fp == null)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open the file '%s'.\n", __FUNCTION__, __LINE__, ebp);
		#endif
		return -1;
	}
	fread(&header, sizeof(EBOOT_HEADER), 1, fp);
	if(header.signature[1] != 'P' || header.signature[2] != 'B' || header.signature[3] != 'P')
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Input file is not a valid PBP file.\n", __FUNCTION__, __LINE__);
		#endif
		fclose(fp);
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	off1 = header.offset[type];
	off2 = (type == 7 ? ftell(fp) : header.offset[type + 1]);
	size = off2 - off1;
	fseek(fp, off1, SEEK_SET);
	if(size <= 0)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Size is < or = to 0. (%u)\n", __FUNCTION__, __LINE__, size);
		#endif
	    fclose(fp);
		return -1;
	}
	FILE *fo = fopen(out, "wb");
	if(fo == null)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open the file '%s'.\n", __FUNCTION__, __LINE__, out);
		#endif
	    fclose(fp);
		return -1;
	}
	size_t towrite = size;
	size_t readsize = 0;
	while(towrite)
	{
	    if(towrite > 0x8000)
		    readsize = 0x8000;
		else
		    readsize = towrite;
		buf = (u8*)malloc(readsize);
		fread(buf, readsize, 1, fp);
		fwrite(buf, readsize, 1, fo);
		free(buf);
		towrite -= readsize;
	}
	fclose(fp);
	fclose(fo);

	return 0;
}

int LPP_UtilsCheckEboot(const char *ebp, short type)
{
	size_t off1, off2, size;
	EBOOT_HEADER header;
	memset(&header, 0, sizeof(EBOOT_HEADER));

	if(type < 0) type = 0;
	if(type > 7) type = 7;
	FILE *fp = fopen(ebp, "rb");
	if(fp == null)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open the file '%s'.\n", __FUNCTION__, __LINE__, ebp);
		#endif
		return -1;
	}
	fread(&header, sizeof(EBOOT_HEADER), 1, fp);
	if(header.signature[1] != 'P' || header.signature[2] != 'B' || header.signature[3] != 'P')
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Input file is not a valid PBP file.\n", __FUNCTION__, __LINE__);
		#endif
		fclose(fp);
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	off1 = header.offset[type];
	off2 = (type == 7 ? ftell(fp) : header.offset[type + 1]);
	size = off2 - off1;
	fseek(fp, off1, SEEK_SET);
	if(size <= 0)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Size is < or = to 0. (%u)\n", __FUNCTION__, __LINE__, size);
		#endif
	    fclose(fp);
		return -1;
	}
	fclose(fp);

	return 0;
}

int LPP_UtilsGetISO(const char *isopath, const char *input, const char *output)
{
    FILE *fd = fopen(isopath,"rb");
	if(fd == null)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open the iso '%s'.\n", __FUNCTION__, __LINE__, isopath);
		#endif
		return -1;
	}
    IsoOpen(fd);
	File *entry = GetFile((char*)input);
	if((u32)entry & 0x80000000)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot get the iso entry '%s'.\n", __FUNCTION__, __LINE__, input);
		#endif
		fclose(fd);
		return -1;
	}
	if(entry->size >= 1048576) // Max 1MB
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Entry size is too big.\n", __FUNCTION__, __LINE__);
		#endif
		fclose(fd);
		return -1;
	}
	FILE *out = fopen(output,"wb");
	if(out == null)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open the file '%s' to write.\n", __FUNCTION__, __LINE__, output);
		#endif
		fclose(fd);
		return -1;
	}
	void *buf = malloc(1048576);
	if(!buf)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'buf' to memory.\n", __FUNCTION__, __LINE__);
		#endif
		fclose(fd);
		fclose(out);
		return -1;
	}
	ReadFile(entry, buf);
	fwrite(buf,entry->size,1,out);

	if(buf) free(buf);
	if(entry) free(entry);
	fclose(fd);
	fclose(out);

	return 0;
}

int LPP_UtilsCheckISO(const char *isopath, const char *input)
{
    FILE *fd = fopen(isopath,"rb");
	IsoOpen(fd);
	if(fd == null)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot open the iso '%s'.\n", __FUNCTION__, __LINE__, isopath);
		#endif
		return -1;
	}
	File *entry = GetFile((char*)input);
	if((u32)entry & 0x80000000)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot get the iso entry '%s'.\n", __FUNCTION__, __LINE__, input);
		#endif
		fclose(fd);
		return -1;
	}

	if(entry) free(entry);
	fclose(fd);

	return 0;
}

int LPP_UtilsIsPBP(const char *filename)
{
    FILE *source = fopen(filename, "rb");
	if(source == null)
	{
	    return 0;
	}
	char sig[4];
	fread(sig, 4, 1, source);
	fclose(source);
	
	if(sig[1] == 'P' && sig[2] == 'B' && sig[3] == 'P')
	{
	    return 1;
	}
	
	return 0;
}

int LPP_UtilsIsISO(const char *filename)
{
    if(strstr(filename, ".iso") || strstr(filename, ".ISO"))
	{
	    return 1;
	}
	
    FILE *source = fopen(filename, "rb");
	if(source == null)
	{
	    return 0;
	}
	char sig[4];
	fread(sig, 4, 1, source);
	fclose(source);
	
	if(strcmp((const char*)sig, "CISO") == 0)
	{
	    return 1;
	}
	
	return 0;
}

int LPP_UtilsGetParamTitle(const char *filepath, char *title)
{
    u8 *buffer = null;
	
    if(LPP_UtilsIsPBP(filepath))
    {
	    FILE *fd = fopen(filepath, "rb");
		if(!fd)
		{
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot open the file '%s' for read.\n", __FUNCTION__, __LINE__, filepath);
			#endif
			return -1;
		}
		
        EBOOT_HEADER h;
        memset(&h,0,sizeof(h));
        fread(&h,sizeof(h),1,fd);
		
        size_t size = h.offset[1] - h.offset[0];
		
		buffer = (u8*)malloc(size);
		if(!buffer)
		{
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot allocate 'buffer' to memory.\n", __FUNCTION__, __LINE__);
			#endif
			fclose(fd);
			return -1;
		}
        fseek(fd, h.offset[0], SEEK_SET);
        fread(buffer, size, 1, fd);
		
		fclose(fd);
    }
	else if(LPP_UtilsIsISO(filepath))
    {
	    FILE *fd = fopen(filepath, "rb");
		if(!fd)
		{
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot open the file '%s' for read.\n", __FUNCTION__, __LINE__, filepath);
			#endif
			return -1;
		}
		
		u8 *mbuffer = (u8*)malloc(1048576);
		if(!mbuffer)
		{
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot allocate 'mbuffer' to memory.\n", __FUNCTION__, __LINE__);
			#endif
			fclose(fd);
			return -1;
		}
		
        IsoOpen(fd);
        File *entry = GetFile((char*)"/PSP_GAME/PARAM.SFO");
        if((u32)entry & 0x80000000)
        {
            #ifdef DEBUG
            dwrite_output("Function %s Line %d : Cannot get the iso entry '%s'.\n", __FUNCTION__, __LINE__, filepath);
            #endif
			free(buffer);
            fclose(fd);
            return -1;
        }
		
        ReadFile(entry, mbuffer);
		fclose(fd);
		
		buffer = malloc(entry->size);
		if(!buffer)
		{
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d :Cannot allocate 'buffer' to memory.\n", __FUNCTION__, __LINE__);
			#endif
			free(mbuffer);
			return -1;
		}
		
		memcpy(buffer, mbuffer, entry->size);
		free(mbuffer);
		free(entry);
    }
    else
    {
	    FILE *fd = fopen(filepath, "rb");
		if(!fd)
		{
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot open the file '%s' for read.\n", __FUNCTION__, __LINE__, filepath);
			#endif
			free(buffer);
			return -1;
		}
        fseek(fd, 0, SEEK_END);
        size_t size = ftell(fd);
        rewind(fd);
		buffer = (u8*)malloc(size);
		if(!buffer)
		{
		    #ifdef DEBUG
			dwrite_output("Function %s Line %d : Cannot allocate 'buffer' to memory.\n", __FUNCTION__, __LINE__);
			#endif
			fclose(fd);
			return -1;
		}
		
        fread(buffer, size, 1, fd);
		fclose(fd);
    }
	
	SFO_HEADER *header = (SFO_HEADER*)buffer;
	SFO_DIR *entries = (SFO_DIR*)(buffer + 0x14);
	
	if(header->signature[1] != 'P' || header->signature[2] != 'S' || header->signature[3] != 'F')
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : '%s' is not a valid SFO file.\n", __FUNCTION__, __LINE__, filepath);
		#endif
		free(buffer);
		return -1;
	}
	
	u32 i;
	for(i = 0; i < header->entry_numbers; i++)
	{
	    if(memcmp(buffer + header->fields_table_offs + entries[i].field_offs, "TITLE", 5) == 0)
		{
		    char *tt = (char*)buffer + header->data_table_offs + entries[i].val_offs;
			size_t len = strlen(tt);
			len = len > entries[i].size ? entries[i].size : len;
			if(len > 100)
			{
			    #ifdef DEBUG
				dwrite_output("Function %s Line %d : Title is too long.\n", __FUNCTION__, __LINE__);
				#endif
				free(buffer);
			    return -1;
			}
		    memcpy(title, tt, len);
		}
	}
	
    free(buffer);
    return 0;
}

int LPP_UtilsGetSystemReg(const char *dir, const char *name, u32 *val)
{
    struct RegParam reg;
    REGHANDLE handle = 0;
	memset(&reg, 0, sizeof(struct RegParam));
	reg.regtype = 1;
	reg.namelen = 7;
	reg.unk2 = 1;
	reg.unk3 = 1;
	memcpy(reg.name, "/system", reg.namelen);
	
	int ret = -1;
	
	if(sceRegOpenRegistry(&reg, 1, &handle) == 0)
	{
	    REGHANDLE hd;
		if(sceRegOpenCategory(handle, dir, 2, &hd) == 0)
		{
		    REGHANDLE hk;
			u32 type, size;
			
			if(sceRegGetKeyInfo(hd, name, &hk, &type, &size) == 0)
			{
			    if(sceRegGetKeyValue(hd, hk, val, 4) == 0)
				{
				    ret = 0;
					sceRegFlushCategory(hd);
				}
			}
			sceRegCloseCategory(hd);
		}
		sceRegFlushRegistry(handle);
		sceRegCloseRegistry(handle);
	}
	
	return ret;
}

int LPP_UtilsSetSystemReg(const char *dir, const char *name, u32 val)
{
    struct RegParam reg;
	REGHANDLE handle;
	
	memset(&reg, 0, sizeof(struct RegParam));
	reg.regtype = 1;
	reg.namelen = 7;
	reg.unk2 = 1;
	reg.unk3 = 1;
	memcpy(reg.name, "/system", reg.namelen);
	
	int ret = -1;
	
	if(sceRegOpenRegistry(&reg, 2, &handle) == 0)
	{
	    REGHANDLE hd;
		if(sceRegOpenCategory(handle, dir, 2, &hd) == 0)
		{
		    if(sceRegSetKeyValue(hd, name, &val, 4))
		    {
		        ret = 0;
			    sceRegFlushCategory(hd);
		    }
	 	    else
		    {
		        sceRegCreateKey(hd, name, REG_TYPE_INT, 4);
			    sceRegSetKeyValue(hd, name, &val, 4);
			    ret = 0;
			    sceRegFlushCategory(hd);
		    }
		    sceRegCloseCategory(hd);
	    }
	    sceRegFlushRegistry(handle);
	    sceRegCloseRegistry(handle);
	}
	
	return ret;
}