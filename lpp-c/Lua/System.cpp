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
#include "System.h"

#include "../Libs/Dir/Dir.h"
#include "../Libs/Utils/Utils.h"

#include "../Libs/Graphics/Graphics.h"
#include "../Libs/minIni/minIni.h"

#include "../Libs/Umd/Umd.h"

#include <pspkernel.h>
#include <unistd.h>

static SceUID irda_fd = -1;
static SceUID  sio_fd = -1;

extern "C" {
    #include "../Libs/Kubridge/kubridge.h"
    int RunEboot(const char*);
	int RunUpdate(const char*);
	int LaunchUMD(void);
	SceUID psploadlib(const char *, char *);
	void **findFunction( SceUID, const char *, const char *);
	unsigned int getPommel(void);
	unsigned int getBaryon(void);
	int sceSysregGetTachyonVersion();
	void StartISO(char *, int);
	void LaunchPops(char*);
	#ifdef DEBUG
	extern int dwrite_output(const char*, ...);
	#endif
}

static int luaSystem_curDir(lua_State *L)
{
    Int16 args = lua_gettop(L);
    if(args > 1)
    {
        return luaL_error(L, "System.currentDir([Path]) takes 1 or no arguments.");
    }

    if(args == 1)
    {
        LPP_Chdir(luaL_checkstring(L, 1));
    }

    char path[256] = "";
    LPP_GetCwd(path, 256);

    lua_pushstring(L, path);
    return(1);
}

static int luaSystem_listDir(lua_State *L)
{
    Int16 args = lua_gettop(L);
    if(args > 1)
    {
        return luaL_error(L, "System.listDir([Path]) takes 1 or no arguments.");
    }

    char path[256];

    if(args == 1)
    {
        strncpy(path, luaL_checkstring(L, 1), 256);
    }
    else
    {
        LPP_GetCwd(path, 256);
    }

    LPP_Dir *dir = LPP_DirOpen(path);
    LPP_DirRead(dir);

    lua_newtable(L);

    u32 i = 0;
    while(i < dir->count)
    {
        lua_pushnumber(L, i++);

        lua_newtable(L);
            lua_pushstring(L, "name");
            lua_pushstring(L, dir->entries[i].name);
            lua_settable(L, -3);

            lua_pushstring(L, "size");
            lua_pushnumber(L, dir->entries[i].size);
            lua_settable(L, -3);

            lua_pushstring(L, "isdir");
            lua_pushboolean(L, dir->entries[i].type == LPP_DIR_ENTRY_TYPE_DIR);
            lua_settable(L, -3);

        lua_settable(L, -3);
    }

    LPP_DirClose(dir);

    return 1;
}

static int luaSystem_createDir(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.createDir(Dir) takes 1 argument.");
    }
    LPP_MkDir(luaL_checkstring(L, 1));
    return 0;
}

static int luaSystem_removeDir(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.removeDir(Dir) takes 1 arguments.");
    }
    LPP_RemoveDir(luaL_checkstring(L, 1));
    return 0;
}

static int luaSystem_removeFile(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.removeFile(Filename) takes 1 arguments.");
    }
    LPP_RemoveFile(luaL_checkstring(L, 1));
    return 0;
}

static int luaSystem_renameFile(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "System.renameFile(oldName, newName) takes 2 arguments.");
    }
    LPP_FileRename(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
    return 0;
}

static int luaSystem_renameDir(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "System.renameDir(oldName, newName) takes 2 arguments.");
    }
    LPP_DirRename(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
    return 0;
}

static int luaSystem_powerIsOnline(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.powerIsOnline() takes no arguments.");
    }
    lua_pushboolean(L, scePowerIsBatteryExist());
    return 1;
}

static int luaSystem_powerBatteryExist(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.powerIsBatteryExist() takes no arguments.");
    }
    lua_pushboolean(L, scePowerIsBatteryExist());
    return 1;
}

static int luaSystem_powerIsBatteryCharging(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.powerIsBatteryCharging() takes no arguments.");
    }
    lua_pushnumber(L, scePowerGetBatteryChargingStatus());
    return 1;
}

static int luaSystem_powerBatteryLifePercent(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.powerGetBatteryLifePercent() takes no arguments.");
    }
    lua_pushnumber(L, scePowerGetBatteryLifePercent());
    return 1;
}

static int luaSystem_powerBatteryLifeTime(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.powerGetBatteryLifeTime() takes no arguments.");
    }
    lua_pushnumber(L, scePowerGetBatteryLifeTime());
    return 1;
}

static int luaSystem_powerBatteryTemp(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.powerGetBatteryTemp() takes no arguments.");
    }
    lua_pushnumber(L, scePowerGetBatteryTemp());
    return 1;
}

static int luaSystem_powerGetVolt(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.powerGetVolt() takes no arguments.");
    }
    lua_pushnumber(L, scePowerGetBatteryVolt());
    return 1;
}

static int luaSystem_powerTick(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.powerTick() takes no arguments.");
    }
    scePowerTick(0);
    return 0;
}

static int luaSystem_md5Sum(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.md5Sum(string) takes 1 argument.");
    }

    size_t size;
    u8 digest[16];
    sceKernelUtilsMd5Digest((u8*)luaL_checklstring(L, 1, &size), size, digest);
    char buffer[33];
    int i;
    for(i = 0; i < 16; i++)
    {
        sprintf(buffer + 2 * i, "%02x", digest[i]);
    }
    lua_pushstring(L, buffer);
    return 1;
}

static int luaSystem_getFreeMemory(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.getFreeMemory() takes no arguments.");
    }
    void *buf[64];
    u32 i = 0;
    for(i = 0; i < 64; i++) {
        buf[i] = malloc(1024 * 1024);
        if(!buf[i]) break;
    }

    u32 res = i;
    for(; i >= 0; i--) {
        free(buf[i]);
    }
    lua_pushnumber(L, res * 1024 * 1024);
    return 1;
}

static int luaSystem_getFPS(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.getFPS() takes no arguments.");
    }
    lua_pushnumber(L, LPPG_GetFPS());
    return 1;
}

static int luaSystem_usbActivate(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.usbDiskModeActivate() takes no arguments.");
    }
    if(LPP_UtilsInitUsbStorage() == 0)
    {
        LPP_UtilsStartUsbStorage();
    }
    return 0;
}

static int luaSystem_usbDeactivate(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.usbDiskModeDeactivate() takes no arguments.");
    }
    if(LPP_UtilsStopUsbStorage() == 0)
    {
        LPP_UtilsDeinitUsbStorage();
    }
    return 0;
}

static int luaSystem_setLow(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L , "System.setLow() takes no arguments.");
    }
    if(scePowerGetCpuClockFrequency() != 100)
    {
        scePowerSetCpuClockFrequency(100);
        scePowerSetBusClockFrequency(50);
    }
    return 0;
}

static int luaSystem_setMed(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.setMed() takes no arguments.");
    }
    if(scePowerGetCpuClockFrequency() != 222)
    {
        scePowerSetCpuClockFrequency(222);
        scePowerSetBusClockFrequency(111);
    }
    return 0;
}

static int luaSystem_setHight(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.setHight() takes no arguments.");
    }
    if(scePowerGetCpuClockFrequency() != 333)
    {
        scePowerSetClockFrequency(333, 333, 166);
    }
    return 0;
}

static int luaSystem_setCpuSpeed(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.setCpuSpeed(speed) takes 1 argument.");
    }
    u32 speed = luaL_checknumber(L, 1);
    if(speed < 222) {
        scePowerSetClockFrequency(100, 100, 100);
    } else
    if(speed < 222) {
        scePowerSetClockFrequency(222, 222, 111);
    } else
    if(speed < 266) {
        scePowerSetClockFrequency(266, 266, 133);
    } else {
        scePowerSetClockFrequency(333, 333, 166);
    }

    return 0;
}

static int luaSystem_msgDialogInit(lua_State *L)
{
    Int16 args = lua_gettop(L);
    if(args != 1 && args != 2)
    {
        return luaL_error(L, "System.msgInit(text, [options]) takes 1 or 2 arguments.");
    }
    lua_pushboolean(L, LPP_UtilsMsgDialogInit((args == 2) ? luaL_checkint(L, 2) : 0, luaL_checkstring(L, 1)));

    return 1;
}

static int luaSystem_msgDialogUpdate(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.msgUpdate() takes no arguments.");
    }
    lua_pushinteger(L, LPP_UtilsMsgDialogUpdate());

    return 1;
}

static int luaSystem_errorDialogInit(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.msgErrorInit(error) takes 1 argument.");
    }
    lua_pushboolean(L, LPP_UtilsMsgDialogErrorInit(luaL_checkint(L, 1)));

    return 1;
}

static int luaSystem_msgAbort(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.msgAbort() takes no arguments.");
    }
    lua_pushboolean(L, LPP_UtilsMsgDialogAbort());

    return 1;
}

static int luaSystem_iniGets(lua_State *L)
{
    if(lua_gettop(L) != 3)
    {
        return luaL_error(L, "System.iniGets(iniPath, Section, Key) takes 3 arguments.");
    }
    char buf[255];
    ini_gets(luaL_checkstring(L, 2), luaL_checkstring(L, 3), "", buf, 255, luaL_checkstring(L, 1));
    lua_pushstring(L, buf);

    return 1;
}

static int luaSystem_iniGetn(lua_State *L)
{
    if(lua_gettop(L) != 3)
    {
        return luaL_error(L, "System.iniGetn(iniPath, Section, Key) takes 3 arguments.");
    }
    long r = ini_getl(luaL_checkstring(L, 2), luaL_checkstring(L, 3), 0, luaL_checkstring(L, 1));
    lua_pushinteger(L, r);

    return 1;
}

static int luaSystem_iniPutn(lua_State *L)
{
    if(lua_gettop(L) != 4)
    {
        return luaL_error(L, "System.iniPutn(iniPath, Section, Key, Value) takes 4 arguments.");
    }
    ini_putl(luaL_checkstring(L, 2), luaL_checkstring(L, 3), luaL_checknumber(L, 4), luaL_checkstring(L, 1));

	FILE *i = fopen(luaL_checkstring(L,1),"wb");
	if(i) {
	    fprintf(i,"\n");
	    fclose(i);
	}

    return 0;
}

static int luaSystem_iniPuts(lua_State *L)
{
    if(lua_gettop(L) != 4)
    {
        return luaL_error(L, "System.iniPuts(iniPath, Section, Key, String) takes 4 arguments.");
    }
    ini_puts(luaL_checkstring(L, 2), luaL_checkstring(L, 3), luaL_checkstring(L, 4), luaL_checkstring(L, 1));

	FILE *i = fopen(luaL_checkstring(L,1),"wb");
	if(i) {
	    fprintf(i,"\n");
	    fclose(i);
	}

    return 0;
}

static int luaSystem_loadStartModule(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "System.loadStartModule(filename) takes 1 argument.");
	}
	lua_pushnumber(L, LPP_UtilsLoadStartModule(luaL_checkstring(L, 1)));

	return 1;
}

static int luaSystem_stopUnloadModule(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "System.stopUnloadModule(Modid) takes 1 argument.");
	}
	LPP_UtilsStopUnloadModule(luaL_checknumber(L, 1));

	return 0;
}

static int luaSystem_browserInit(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.browserInit(url) takes 1 argument.");
    }
    lua_pushboolean(L, LPP_UtilsBrowserInit(4*1024*1024, luaL_checkstring(L, 1)));

    return 1;
}

static int luaSystem_browserUpdate(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.browserUpdate() takes no arguments.");
    }
    lua_pushinteger(L, LPP_UtilsBrowserUpdate());

    return 1;
}

static int luaSystem_netInit(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.netInit() takes no arguments.");
    }
    lua_pushboolean(L, LPP_UtilsNetDialogInit());

    return 1;
}

static int luaSystem_netUpdate(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.netUpdate() takes no arguments.");
    }
    lua_pushinteger(L, LPP_UtilsNetDialogUpdate());

    return 1;
}

static int luaSystem_adhocDialogInit(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "System.adhocDialogInit(type, name) takes 2 arguments.");
    }
    lua_pushboolean(L, LPP_AdhocDialogInit(luaL_checkint(L, 1), (char *)luaL_checkstring(L, 2)));

    return 1;
}

static int luaSystem_adhocDialogUpdate(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.adhocDialogUpdate() takes no arguments.");
    }
    lua_pushinteger(L, LPP_AdhocDialogUpdate());

    return 1;
}

static int luaSystem_oskInit(lua_State *L)
{
    Int16 args = lua_gettop(L);
    if(args > 2)
    {
        return luaL_error(L, "System.oskInit([Desctiption], [InitialText]) takes zero, one or two arguments.");
    }

    const char *desc = (args > 0) ? luaL_checkstring(L, 1) : null;
    const char *intx = (args == 2) ? luaL_checkstring(L, 2) : null;

    lua_pushboolean(L, LPP_UtilsOskInit(desc, intx));

    return 1;
}

static char *luaOskOutText = null;

static int luaSystem_oskUpdate(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.oskUpdate() takes no arguments.");
    }

    if(luaOskOutText == null)
    {
        luaOskOutText = (char *)malloc(512);
    }

    int res = LPP_UtilsOskUpdate(luaOskOutText);

    if(res == 0 || res == 2)
    {
        lua_pushboolean(L, 1);
        lua_pushstring(L, luaOskOutText);
        free(luaOskOutText);
        luaOskOutText = null;
    }
    else
    if(res == -1)
    {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "");
    }
    else
    {
        lua_pushboolean(L, 1);
        lua_pushstring(L, "");
    }

    return 2;
}

static int luaSaveDataType = -1;

static int luaSystem_listSaveInit(lua_State *L)
{
    Int16 args= lua_gettop(L);
    if(args != 5 && args != 6)
    {
        return luaL_error(L, "System.lisSaveInit(data, name, title, subtitle, detail, [cPath]) takes 5 or 6 arguments.");
    }

    luaSaveDataType = LPP_UTILS_SAVEDATA_TYPE_LISTSAVE;

    const char *data = luaL_checkstring(L, 1);
    u32 data_size = strlen(data);

    u8 key[16];
    sceOpenPSIDGetOpenPSID((PspOpenPSID*)key);

    lua_pushboolean(L, LPP_UtilsSavedataInit(LPP_UTILS_SAVEDATA_TYPE_LISTSAVE, (void *)data, data_size, (args == 6) ? luaL_checkstring(L, 6) : "EBOOT.PBP",
                                              luaL_checkstring(L, 2), (char *)key, luaL_checkstring(L, 3), luaL_checkstring(L, 4), luaL_checkstring(L, 5)));

    return 1;
}

static char *luaSaveDataData = null;

static int luaSystem_listLoadInit(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "System.listLoadInit(datasize, name) takes 2 arguments.");
    }
    u32 datasize = luaL_checkinteger(L, 1);
    datasize++;
    if(luaSaveDataData) free(luaSaveDataData);
    luaSaveDataData = (char *)malloc(datasize);

    if(luaSaveDataData == null)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    u8 key[16];
    sceOpenPSIDGetOpenPSID((PspOpenPSID*)key);

    luaSaveDataType = LPP_UTILS_SAVEDATA_TYPE_LISTLOAD;

    lua_pushboolean(L, LPP_UtilsSavedataInit(LPP_UTILS_SAVEDATA_TYPE_LISTLOAD, luaSaveDataData, datasize, "EBOOT.PBP", luaL_checkstring(L, 2), (char *)key, null, null, null));

    return 1;
}

static int luaSystem_autoLoadInit(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "System.autoLoadInit(datasize, name) takes 2 arguments.");
    }
    u32 datasize = luaL_checkinteger(L, 1);
    datasize++;

    if(luaSaveDataData) free(luaSaveDataData);
    luaSaveDataData = (char *)malloc(datasize);

    if(!luaSaveDataData)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    u8 key[16];
    sceOpenPSIDGetOpenPSID((PspOpenPSID*)key);

    luaSaveDataType = LPP_UTILS_SAVEDATA_TYPE_AUTOLOAD;

    lua_pushboolean(L, LPP_UtilsSavedataInit(LPP_UTILS_SAVEDATA_TYPE_AUTOLOAD, luaSaveDataData, datasize, "EBOOT.PBP", luaL_checkstring(L, 2), (char *)key, null, null, null));

    return 1;
}

static int luaSystem_autoSaveInit(lua_State *L)
{
    Int16 args = lua_gettop(L);
    if(args != 5 && args != 6)
    {
        return luaL_error(L, "System.autoSaveInit(data, name, title, subtitle, detail, [cPath]) takes 5 or 6 arguments.");
    }

    L_CONST char *data = luaL_checkstring(L, 1);
    u32 datalen = strlen(data);

    u8 key[16];
    sceOpenPSIDGetOpenPSID((PspOpenPSID*)key);

    luaSaveDataType = LPP_UTILS_SAVEDATA_TYPE_AUTOSAVE;

    lua_pushboolean(L, LPP_UtilsSavedataInit(LPP_UTILS_SAVEDATA_TYPE_AUTOSAVE, (void *)data, datalen, (args == 6) ? luaL_checkstring(L, 6) : "EBOOT.PBP",
                                             luaL_checkstring(L, 2), (char *)key, luaL_checkstring(L, 3), luaL_checkstring(L, 4), luaL_checkstring(L, 5)));

    return 1;
}

static int luaSystem_listDeleteInit(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.listDeleteInit(name) takes 1 arguments.");
    }

    u8 key[16];
    sceOpenPSIDGetOpenPSID((PspOpenPSID*)key);

    luaSaveDataType = LPP_UTILS_SAVEDATA_TYPE_LISTDELETE;

    lua_pushboolean(L, LPP_UtilsSavedataInit(LPP_UTILS_SAVEDATA_TYPE_LISTDELETE, null, 0, "EBOOT.PBP", luaL_checkstring(L, 1), (char *)key, null, null, null));

    return 1;
}

static int luaSystem_saveDataUpdate(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.saveDataUpdate() takes no arguments.");
    }
    int res = LPP_UtilsSavedataUpdate();

    lua_pushinteger(L, res);

    if(luaSaveDataType == LPP_UTILS_SAVEDATA_TYPE_LISTSAVE ||
       luaSaveDataType == LPP_UTILS_SAVEDATA_TYPE_LISTSAVE ||
       luaSaveDataType == LPP_UTILS_SAVEDATA_TYPE_LISTDELETE) return 1;

    if(res == 0)
    {
        lua_pushstring(L, luaSaveDataData);
        free(luaSaveDataData);
    }
    else
    {
        lua_pushnil(L);
    }

    return 2;
}

static int luaSystem_umdInit(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.umdInit() takes no arguments.");
    }
    lua_pushboolean(L, UMD::Init() >= 0);

    return 1;
}

static int luaSystem_umdGetTitle(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.umdGetTitle() takes no arguments.");
    }
    char title[11] = "";
    int ret = UMD::GetTitle(title);
    lua_pushstring(L, title);

    lua_pushboolean(L, ret >= 0);

    return 2;
}

static int luaSystem_umdGetSize(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.umdGetSize() takes no arguments.");
    }
    lua_pushnumber(L, UMD::GetSize());

    return 1;
}

static int luaSystem_umdDumpISO(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.umdDumpISO(isoName) takes 1 arguments.");
    }
    int ret = UMD::DumpISO(luaL_checkstring(L, 1));

    lua_pushboolean(L, ret >= 0);

    return 1;
}

static int luaSystem_umdDumpCSO(lua_State *L)
{
    Int16 args = lua_gettop(L);
    if(args != 1 && args != 2)
    {
        return luaL_error(L, "System.umdDumpCSO(csoName, [CompressionLevel]) takes 1 or 2 arguments.");
    }
    int ret = UMD::DumpCSO(luaL_checkstring(L, 1), (args == 2) ? CLAMP(luaL_checkinteger(L, 2), 0, 9) : 9);

    lua_pushboolean(L, ret >= 0);

    return 1;
}

static int luaSystem_buttonPressed(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.buttonPressed() takes no arguments.");
    }
    SceCtrlData pad;
    sceCtrlReadBufferPositive(&pad, 1);
    lua_pushboolean(L, pad.Buttons);

    return 1;
}

static int luaSystem_memclean(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.memclean() takes no arguments.");
    }
    lua_gc(L, LUA_GCCOLLECT, null);

    return 0;
}

static int luaSystem_loadModule(lua_State *L)
{
    if(lua_gettop(L) != 2)
	{
	    return luaL_error(L, "System.loadModule(Module, Init) takes 2 arguments.");
	}
	SceUID id = psploadlib(luaL_checkstring(L, 1), null);
	if(id >= 0) {
	    lua_CFunction f = (lua_CFunction) *(findFunction(id, luaL_checkstring(L,1), luaL_checkstring(L,2)));
		if(f != null) {
		    lua_pushlightuserdata(L, (void *)id);
			lua_pushcclosure(L, f, 1);
			return 1;
		}
	}

	lua_pushnil(L);
	lua_pushstring(L, (id >= 0) ? "init" : "open");

	return 1;
}

static int luaSystem_runEboot(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "System.runEboot(filename) takes 1 argument.");
	}
	RunEboot(luaL_checkstring(L, 1));

	return 0;
}

static int luaSystem_runUpdate(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "System.runUpdate(filename) takes 1 argument.");
	}
	RunUpdate(luaL_checkstring(L, 1));

	return 0;
}

static int luaSystem_launchUMD(lua_State *L)
{
    if(lua_gettop(L) != 0)
	{
	    return luaL_error(L, "System.launchUMD() takes no arguments.");
	}
	LaunchUMD();

	return 0;
}

static int luaSystem_getModel(lua_State *L)
{
    if(lua_gettop(L) != 0)
	{
	    return luaL_error(L, "System.getModel() takes no arguments.");
	}
	unsigned int model = ((kuKernelGetModel() + 1) * 1000);

	if(model == 4000)
	    lua_pushstring(L, "N1000");
	else
	    lua_pushfstring(L, "%i", model);

	return 1;
}

static int luaSystem_Assign(lua_State *L)
{
    if(lua_gettop(L) != 3)
	{
	    return luaL_error(L, "System.assign(Dev1, Dev2, Dev3) takes 3 arguments.");
	}
	lua_pushboolean(L, sceIoAssign(luaL_checkstring(L,1),luaL_checkstring(L,2),luaL_checkstring(L,3),IOASSIGN_RDWR, null, 0) >= 0);

	return 1;
}

static int luaSystem_UnAssign(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "System.unassign(Device) takes 1 argument.");
	}
	lua_pushboolean(L, !(sceIoUnassign(luaL_checkstring(L,1)) < 0));

	return 1;
}

static int luaSystem_irdaInit(lua_State *L)
{
    if(lua_gettop(L) != 0)
	{
	    return luaL_error(L, "System.irdaInit() takes no arguments.");
	}
	if(irda_fd < 0) irda_fd = sceIoOpen("irda0:", PSP_O_RDWR, 0);
	lua_pushboolean(L, !(irda_fd < 0));

	return 1;
}

static int luaSystem_irdaWrite(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "System.irdaWrite(string) takes 1 argument.");
	}
	if(irda_fd < 0)
    {
        lua_pushboolean(L, 0);
        return 1;
    }
	size_t size;
	const char *string = luaL_checklstring(L, 1, &size);
	lua_pushboolean(L, !(sceIoWrite(irda_fd, string, size) < (int)size));

	return 1;
}

static int luaSystem_irdaRead(lua_State *L)
{
    if(lua_gettop(L) != 0)
	{
	    return luaL_error(L, "System.irdaRead() takes no arguments.");
	}
	if(irda_fd < 0)
    {
        lua_pushstring(L, "");
        return 1;
    }
	char buff[256];
	int count = sceIoRead(irda_fd, &buff, 256);
	if(count > 0)
	    lua_pushlstring(L, buff, count);
	else
	    lua_pushstring(L, "");

	return 1;
}

static int luaSystem_sioInit(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "System.sioInit(baudRate) takes 1 argument.");
	}
	int baudRate = luaL_checkint(L,1);
	if(sio_fd < 0) sio_fd = sceIoOpen("sio:", PSP_O_RDWR, 0);
	if(sio_fd < 0)
	{
	    lua_pushboolean(L, 0);
		return 1;
	}
	sceIoIoctl(sio_fd, 1, &baudRate, sizeof(baudRate), null, 0);
	lua_pushboolean(L, 1);

	return 1;
}

static int luaSystem_sioWrite(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.sioWrite(string) takes 1 argument.");
    }
    if(sio_fd < 0)
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    size_t size;
    const char *string = luaL_checklstring(L, 1, &size);
    lua_pushboolean(L, !(sceIoWrite(sio_fd, string, size) < (int)size));

    return 1;
}

static int luaSystem_sioRead(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.sioRead() takes no arguments.");
    }
    if(sio_fd < 0)
    {
        lua_pushstring(L, "");
        return 1;
    }
    char buffer[256];
    int count = sceIoRead(sio_fd, buffer, 256);
    if(count > 0)
        lua_pushlstring(L, buffer, count);
    else
        lua_pushstring(L, "");

    return 1;
}

static int luaSystem_fileExtists(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.fileExist(filename) takes 1 argument.");
    }
    lua_pushboolean(L, LPP_FileExists(luaL_checkstring(L, 1)));

    return 1;
}

static int luaSystem_dirExists(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.dirExist(path) takes 1 argument.");
    }
    lua_pushboolean(L, LPP_DirExists(luaL_checkstring(L, 1)));

    return 1;
}

static int luaSystem_copyFile(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "System.copyFile(source, dest) takes 2 arguments.");
    }
    LPP_CopyFile(luaL_checkstring(L, 1), luaL_checkstring(L, 2));

    return 0;
}

static int luaSystem_copyDir(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "System.copyDir(source, dest) takes 2 arguments.");
    }
    LPP_CopyDir(luaL_checkstring(L,1),luaL_checkstring(L,2));

    return 0;
}

static int luaSystem_moveFile(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "System.moveFile(source, dest) takes 2 arguments.");
    }
    LPP_MoveFile(luaL_checkstring(L, 1), luaL_checkstring(L, 2));

    return 0;
}

static int luaSystem_moveDir(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "System.moveDir(source, dest) takes 2 arguments.");
    }
    LPP_MoveDir(luaL_checkstring(L,1),luaL_checkstring(L,2));

    return 0;
}

static int luaSystem_shutdown(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.shutdown() takes no arguments.");
    }
    scePowerRequestStandby();

    return 0;
}

static int luaSystem_standby(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.standby() takes no arguments.");
    }
    scePowerRequestSuspend();

    return 0;
}

static int luaSystem_getNickname(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.nickname() takes no arguments.");
    }
    char nick[128];
    sceUtilityGetSystemParamString(PSP_SYSTEMPARAM_ID_STRING_NICKNAME, nick, 128);
    lua_pushstring(L, nick);

    return 1;
}

static int luaSystem_getMac(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.getMacAddress() takes no arguments.");
    }
    u8 Mac[8];
    sceWlanGetEtherAddr(Mac);
    lua_pushfstring(L, "%02X:%02X:%02X:%02X:%02X:%02X", Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5]);

    return 1;
}

static int luaSystem_getPSID(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.getPSID() takes no arguments.");
    }
    u8 PSID[16];
    sceOpenPSIDGetOpenPSID((PspOpenPSID*)PSID);
    lua_pushfstring(L, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
    PSID[0], PSID[1], PSID[2], PSID[3], PSID[4], PSID[5], PSID[6], PSID[7], PSID[8], PSID[9], PSID[10],
    PSID[11], PSID[12], PSID[13], PSID[14], PSID[15]);

    return 1;
}

static int luaSystem_getLanguage(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.getLanguage() takes no arguments.");
    }
    int lang = -1;
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &lang);
    lua_pushnumber(L, lang);

    return 1;
}

static int luaSystem_getFreeSize(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.getFreeSize(Device) takes 1 argument.");
    }
    SystemDevCtl devctl;
    memset(&devctl, 0, sizeof(SystemDevCtl));
    SystemDevCommand command;
    command.pdevinf = &devctl;
    sceIoDevctl(luaL_checkstring(L, 1), 0x02425818, &command, sizeof(SystemDevCommand), null, 0);
    u64 freesize = (devctl.freeclusters * devctl.sectorcount) * devctl.sectorsize;
    lua_pushnumber(L, (double)freesize / 1048576.0);

    return 1;
}

static int luaSystem_getTotalSize(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.getTotalSize(Device) takes 1 argument.");
    }
    SystemDevCtl devctl;
    memset(&devctl, 0, sizeof(SystemDevCtl));
    SystemDevCommand command;
    command.pdevinf = &devctl;
    sceIoDevctl(luaL_checkstring(L, 1), 0x02425818, &command, sizeof(SystemDevCommand), null, 0);
    u64 freesize = (devctl.maxclusters * devctl.sectorcount) * devctl.sectorsize;
    lua_pushnumber(L, (double)freesize / 1048576.0);

    return 1;
}

static int luaSystem_getCpu(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.getCpuSpeed() takes no arguments.");
    }
    lua_pushnumber(L, scePowerGetCpuClockFrequency());

    return 1;
}

static int luaSystem_getBus(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.getBusSpeed() takes no arguments.");
    }
    lua_pushnumber(L, scePowerGetBusClockFrequency());

    return 1;
}

static int luaSystem_getTime(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.getTime() takes no arguments.");
    }
    pspTime psptime;
    sceRtcGetCurrentClockLocalTime(&psptime);

    lua_newtable(L);

    lua_pushstring(L, "hour");
    lua_pushnumber(L, psptime.hour);
    lua_settable(L, -3);

    lua_pushstring(L, "minutes");
    lua_pushnumber(L, psptime.minutes);
    lua_settable(L, -3);

    lua_pushstring(L, "seconds");
    lua_pushnumber(L, psptime.seconds);
    lua_settable(L, -3);

    lua_pushstring(L, "microseconds");
    lua_pushnumber(L, psptime.microseconds);
    lua_settable(L, -3);

    lua_pushstring(L, "year");
    lua_pushnumber(L, psptime.year);
    lua_settable(L, -3);

    lua_pushstring(L, "month");
    lua_pushnumber(L, psptime.month);
    lua_settable(L, -3);

    lua_pushstring(L, "day");
    lua_pushnumber(L, psptime.day);
    lua_settable(L, -3);

    return 1;
}

static int luaSystem_getMobo(lua_State *L)
{
    if(lua_gettop(L) != 0)
    {
        return luaL_error(L, "System.getMobo() takes no arguments.");
    }
    unsigned int Tachyon = sceSysregGetTachyonVersion();
    unsigned int Baryon = getBaryon();
    unsigned int Pommel = getPommel();
    char Buff[256];
    sprintf(Buff, "%08x%08x%08x", Tachyon, Baryon, Pommel);
    char Buff2[256];
    sprintf(Buff2, "%08x%08x", Tachyon, Baryon);
    if(strstr(Buff2, "0014000000030600"))
	{
	    lua_pushstring(L, "TA-079 v1 (1G)");
		return 1;
	}
    if(strstr(Buff2, "0020000000030600")) 
	{
	    lua_pushstring(L, "TA-079 v2 (1G)");
		return 1;
	}
    if(strstr(Buff2, "0020000000040600"))
	{
	    lua_pushstring(L, "TA-079 v3 (1G)");
		return 1;
	}
    if(strstr(Buff2, "0030000000040600"))
	{
	    lua_pushstring(L, "TA-081 (1G)");
		return 1;
	}
    if(strstr(Buff2, "0040000000114000"))
	{
	    lua_pushstring(L, "TA-082 (1G)");
		return 1;
	}
    if(strstr(Buff2, "0040000000121000"))
	{
	    lua_pushstring(L, "TA-086 (1G)");
		return 1;
	}
    if(strstr(Buff2, "005000000022b200"))
	{
	    lua_pushstring(L, "TA-085 v1 (2G)");
		return 1;
	}
    if(strstr(Buff2, "0050000000234000"))
	{
	    lua_pushstring(L, "TA-085 v2 (2G)");
		return 1;
	}
    if(strstr(Buff, "005000000024300000000123"))
	{
	    lua_pushstring(L, "TA-088 v1/v2 (2G)");
		return 1;
	}
    if(strstr(Buff2, "0060000000243000"))
	{
	    lua_pushstring(L, "TA-088 v3 (2G)");
		return 1;
	}
    if(strstr(Buff, "005000000024300000000132"))
	{
	    lua_pushstring(L, "TA-090 v1 (2G)");
		return 1;
	}
    if(strstr(Buff2, "0060000000263100")) 
	{
	    lua_pushstring(L, "TA-090 v2 (3G)");
		return 1;
	}
    if(strstr(Buff2, "0060000000285000")) 
	{
	    lua_pushstring(L, "TA-092 (3G)");
		return 1;
	}
    if(strstr(Buff2, "00810000002c4000"))
	{
	    lua_pushstring(L, "TA-093 (4G)");
		return 1;
	}
    if(strstr(Buff2, "00810000002e4000"))
	{
	    lua_pushstring(L, "N/A (6G)");
		return 1;
	}
    if(strstr(Buff2, "0072000000304000")) {
        if(kuKernelGetModel() == 3) {
            lua_pushstring(L, "TA-091 (5G)");
			return 1;
        } else {
            lua_pushstring(L, "TA-095 (9G)");
			return 1;
        }
    }
    if(strstr(Buff2, "00800000002a0000")) 
	{
	    lua_pushstring(L, "TA-094 (5G)");
		return 1;
	}
    lua_pushstring(L, "Unknow");

    return 1;
}

static int luaSystem_runISO(lua_State *L)
{
    Int32 args = lua_gettop(L);
    if(args != 1 && args != 2)
    {
        return luaL_error(L, "System.runISO(filename, [Driver]) takes 1 or 2 arguments.");
    }
    StartISO((char *)luaL_checkstring(L,1), (args == 2) ? CLAMP(luaL_checkinteger(L, 2), 0, 3) : 3);

    return 0;
}

static int luaSystem_getBaryon(lua_State *L)
{
    if(lua_gettop(L) != 0)
	{
	    return luaL_error(L, "System.getBaryon() takes no arguments.");
	}
	unsigned int bar = getBaryon();
	char buf[256];
	sprintf(buf, "%08x", bar);
	int i = 0;
	while(buf[i++]) buf[i] = toupper(buf[i]);
	lua_pushfstring(L, "0x%s", buf);

	return 1;
}

static int luaSystem_getPommel(lua_State *L)
{
    if(lua_gettop(L) != 0)
	{
	    return luaL_error(L, "System.getPommel() takes no arguments.");
	}
	unsigned int pom = getPommel();
	char buf[256];
	sprintf(buf, "%08x", pom);
	int i = 0;
	while(buf[i++]) buf[i] = toupper(buf[i]);
	lua_pushfstring(L, "0x%s", buf);

	return 1;
}

static int luaSystem_getTachyon(lua_State *L)
{
    if(lua_gettop(L) != 0)
	{
	    return luaL_error(L, "System.getTachyon() takes no arguments.");
	}
	unsigned int tac = sceSysregGetTachyonVersion();
	char buf[256];
	sprintf(buf, "%08x", tac);
	int i = 0;
	while(buf[i++]) buf[i] = toupper(buf[i]);
	lua_pushfstring(L, "0x%s", buf);

	return 1;
}

static int luaSystem_runPSX(lua_State *L)
{
    if(lua_gettop(L) != 1)
	{
	    return luaL_error(L, "System.runPSX(filename) takes 1 argument.");
	}
	LaunchPops((char*)luaL_checkstring(L, 1));

	return 0;
}

static int luaSystem_getEboot(lua_State *L)
{
    if(lua_gettop(L) != 3)
	{
	    return luaL_error(L, "System.getEboot(ebootPath, Output, Type) takes 3 arguments.");
	}
	lua_pushboolean(L, !(LPP_UtilsGetEboot(luaL_checkstring(L,1),luaL_checkstring(L,2),luaL_checkinteger(L,3))<0));

	return 1;
}

static int luaSystem_checkEboot(lua_State *L)
{
    if(lua_gettop(L) != 2)
	{
	    return luaL_error(L, "System.checkEboot(ebootPath, Type) takes 2 arguments.");
	}
	lua_pushboolean(L, !(LPP_UtilsCheckEboot(luaL_checkstring(L,1),luaL_checkinteger(L,2)) < 0));

	return 1;
}

static int luaSystem_getCFWVersion(lua_State *L)
{
    if(lua_gettop(L) != 0)
	{
	    return luaL_error(L, "System.getVersion() takes no arguments.");
	}
	char buf[16];
	sprintf(buf, "%x", sceKernelDevkitVersion());
	buf[1] = '.'; buf[3] = buf[4];
	buf[4] = 0;
	lua_pushstring(L, buf);

	return 1;
}

static int luaSystem_getISO(lua_State *L)
{
    if(lua_gettop(L) != 3)
	{
	    return luaL_error(L, "System.getISO(isoPath, fileName, output) takes 3 arguments.");
	}
	lua_pushboolean(L, !(LPP_UtilsGetISO(luaL_checkstring(L,1),luaL_checkstring(L,2),luaL_checkstring(L,3))<0));

	return 1;
}

static int luaSystem_checkISO(lua_State *L)
{
    if(lua_gettop(L) != 2)
	{
	    return luaL_error(L, "System.checkISO(isoPath, fileName) takes 2 arguments.");
	}
	lua_pushboolean(L, !(LPP_UtilsCheckISO(luaL_checkstring(L,1),luaL_checkstring(L,2))<0));

	return 1;
}

static int luaSystem_getParamTitle(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "System.getParamTitle(filename) takes 1 argument.");
    }
    char title[100] = "";
    LPP_UtilsGetParamTitle(luaL_checkstring(L, 1), title);
    lua_pushstring(L, title);

    return 1;
}

static L_CONST luaL_reg luaSystem_methods[] = {
    { "currentDir", luaSystem_curDir },
    { "listDir", luaSystem_listDir },
    { "createDir", luaSystem_createDir },
    { "removeDir", luaSystem_removeDir },
    { "removeFile", luaSystem_removeFile },
    { "renameFile", luaSystem_renameFile },
    { "renameDir", luaSystem_renameDir },
    { "powerIsOnline", luaSystem_powerIsOnline },
    { "powerIsBatteryExist", luaSystem_powerBatteryExist },
    { "powerIsBatteryCharging", luaSystem_powerIsBatteryCharging },
    { "powerGetBatteryLifePercent", luaSystem_powerBatteryLifePercent },
    { "powerGetBatteryLifeTime", luaSystem_powerBatteryLifeTime },
    { "powerGetBatteryTemp", luaSystem_powerBatteryTemp },
    { "powerGetBatteryVolt", luaSystem_powerGetVolt },
    { "powerTick", luaSystem_powerTick },
    { "md5Sum", luaSystem_md5Sum },
    { "getFreeMemory", luaSystem_getFreeMemory },
    { "getFPS", luaSystem_getFPS },
    { "usbDiskModeActivate", luaSystem_usbActivate },
    { "usbDiskModeDeactivate", luaSystem_usbDeactivate },
    { "setLow", luaSystem_setLow },
    { "setMed", luaSystem_setMed },
    { "setHight", luaSystem_setHight },
    { "setCpuSpeed", luaSystem_setCpuSpeed },
    { "msgInit", luaSystem_msgDialogInit },
    { "msgUpdate", luaSystem_msgDialogUpdate },
    { "msgErrorInit", luaSystem_errorDialogInit },
    { "msgAbort", luaSystem_msgAbort },
    { "iniGets", luaSystem_iniGets },
    { "iniGetn", luaSystem_iniGetn },
    { "iniPutn", luaSystem_iniPutn },
    { "iniPuts", luaSystem_iniPuts },
	{ "loadStartModule", luaSystem_loadStartModule },
	{ "stopUnloadModule", luaSystem_stopUnloadModule },
	{ "browserInit", luaSystem_browserInit },
	{ "browserUpdate", luaSystem_browserUpdate },
	{ "netInit", luaSystem_netInit },
	{ "netUpdate", luaSystem_netUpdate },
	{ "adhocDialogInit", luaSystem_adhocDialogInit },
	{ "ahodDialogUpdate", luaSystem_adhocDialogUpdate },
	{ "oskInit", luaSystem_oskInit },
	{ "oskUpdate", luaSystem_oskUpdate },
	{ "listSaveInit", luaSystem_listSaveInit },
	{ "listLoadInit", luaSystem_listLoadInit },
	{ "autoLoadInit", luaSystem_autoLoadInit },
	{ "autoSaveInit", luaSystem_autoSaveInit },
	{ "listDeleteInit", luaSystem_listDeleteInit },
	{ "saveDataUpdate", luaSystem_saveDataUpdate },
	{ "umdInit", luaSystem_umdInit },
	{ "umdGetTitle", luaSystem_umdGetTitle },
	{ "umdGetSize", luaSystem_umdGetSize },
	{ "umdDumpISO", luaSystem_umdDumpISO },
	{ "umdDumpCSO", luaSystem_umdDumpCSO },
	{ "buttonPressed", luaSystem_buttonPressed },
	{ "memclean", luaSystem_memclean },
	{ "loadModule", luaSystem_loadModule },
	{ "runEboot", luaSystem_runEboot },
	{ "launchUMD", luaSystem_launchUMD },
	{ "getModel", luaSystem_getModel },
	{ "assign", luaSystem_Assign },
	{ "unassign", luaSystem_UnAssign },
	{ "irdaInit", luaSystem_irdaInit },
	{ "irdaWrite", luaSystem_irdaWrite },
	{ "irdaRead", luaSystem_irdaRead },
	{ "runUpdate", luaSystem_runUpdate },
	{ "sioInit", luaSystem_sioInit },
	{ "sioWrite", luaSystem_sioWrite },
	{ "sioRead", luaSystem_sioRead },
	{ "fileExist", luaSystem_fileExtists },
	{ "dirExist", luaSystem_dirExists },
	{ "copyFile", luaSystem_copyFile },
	{ "copyDir", luaSystem_copyDir },
	{ "moveFile", luaSystem_moveFile },
	{ "moveDir", luaSystem_moveDir },
	{ "shutdown", luaSystem_shutdown },
	{ "standby", luaSystem_standby },
	{ "nickname", luaSystem_getNickname },
	{ "getMacAddress", luaSystem_getMac },
	{ "getPSID", luaSystem_getPSID },
	{ "getLanguage", luaSystem_getLanguage },
	{ "getFreeSize", luaSystem_getFreeSize },
	{ "getTotalSize", luaSystem_getTotalSize },
	{ "getCpuSpeed", luaSystem_getCpu },
	{ "getBusSpeed", luaSystem_getBus },
	{ "getTime", luaSystem_getTime },
	{ "getMobo", luaSystem_getMobo },
	{ "runISO", luaSystem_runISO },
	{ "getBaryon", luaSystem_getBaryon },
	{ "getPommel", luaSystem_getPommel },
	{ "getTachyon", luaSystem_getTachyon },
	{ "runPSX", luaSystem_runPSX },
	{ "getEboot", luaSystem_getEboot },
	{ "checkEboot", luaSystem_checkEboot },
	{ "getVersion", luaSystem_getCFWVersion },
	{ "getISO", luaSystem_getISO },
	{ "checkISO", luaSystem_checkISO },
	{ "getParamTitle", luaSystem_getParamTitle },
    { 0, 0 }
};

void luaSystem_Init(lua_State *L)
{
    luaL_openlib(L, "System", luaSystem_methods, 0);

    #define SYS_UTILS_CONSTANT(name)\
    lua_pushstring(L, #name);\
    lua_pushnumber(L, LPP_UTILS_##name);\
    lua_settable(L, -3);

    SYS_UTILS_CONSTANT(NET_DIALOG_ADHOC_CONNECT)
    SYS_UTILS_CONSTANT(NET_DIALOG_ADHOC_CREATE)
    SYS_UTILS_CONSTANT(NET_DIALOG_ADHOC_JOIN)
    SYS_UTILS_CONSTANT(MSG_DIALOG_NO_OPTIONS)
    SYS_UTILS_CONSTANT(MSG_DIALOG_YESNO_BUTTONS)
    SYS_UTILS_CONSTANT(MSG_DIALOG_DEFAULT_BUTTON_NO)
    SYS_UTILS_CONSTANT(MSG_DIALOG_RESULT_YES)
    SYS_UTILS_CONSTANT(MSG_DIALOG_RESULT_NO)
    SYS_UTILS_CONSTANT(MSG_DIALOG_RESULT_BACK)
    SYS_UTILS_CONSTANT(SAVEDATA_TYPE_AUTOLOAD)
    SYS_UTILS_CONSTANT(SAVEDATA_TYPE_AUTOSAVE)
    SYS_UTILS_CONSTANT(SAVEDATA_TYPE_LISTLOAD)
    SYS_UTILS_CONSTANT(SAVEDATA_TYPE_LISTSAVE)
    SYS_UTILS_CONSTANT(SAVEDATA_TYPE_LISTDELETE)
    SYS_UTILS_CONSTANT(DIALOG_RUNNING)
}
