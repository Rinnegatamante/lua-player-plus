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

#include "LPP.h"

#include "Lua/Graphics.h"
#include "Lua/Controls.h"
#include "Lua/Archive.h"
#include "Lua/Timer.h"
#include "Lua/AdHoc.h"
#include "Lua/Wlan.h"
#include "Lua/System.h"
#include "Lua/Audio.h"
#include "Lua/Mp4.h"
#include "Lua/Xml.h"
#include "Lua/Camera.h"

#include "Libs/Graphics/Graphics.h"
#include "Libs/Dir/Dir.h"
#include "Libs/minIni/minIni.h"
#include "Libs/Utils/Utils.h"

#include "Support/Support-c.c"

PSP_MODULE_INFO("Lua Player Plus", PSP_MODULE_USER, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU);
int sce_newlib_heap_kb_size = -1024 * 6;

static char _startup_path[256];
static lua_State *L;
static int debug_is_initialized = 0;

static int running = 1;

static SceUID Support_id = -1;

static char *main_script_path = null;
static unsigned char delete_support;

static int exit_code = EXIT_SUCCESS;

extern int __psp_free_heap(void);

static int exit_callback(int arg1, int arg2, void *common)
{
    (void)arg1;
    (void)arg2;
    (void)common;

    running = 0;

    return 0;
}

static int CallbackThread(SceSize args, void *argp)
{
    (void)args;
    (void)argp;

    int cbid;

    cbid = sceKernelCreateCallback("LppExitCallback", exit_callback, null);
    sceKernelRegisterExitCallback(cbid);

    sceKernelSleepThreadCB();

    return 0;
}

static int SetupCallbacks(void)
{
   int thid = 0;

   thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
   if(thid >= 0) sceKernelStartThread(thid, 0, 0);

   return thid;
}

static int lpp_delay(lua_State *L)
{
    if(lua_gettop(L) != 1)
        return luaL_error(L, "lpp.delay(ms) takes 1 argument.");
    sceKernelDelayThread(luaL_checknumber(L, 1) * 1000);
    return 0;
}

int LPP_Running(void)
{
    return (running);
}

static int lpp_running(lua_State *L)
{
    if(lua_gettop(L) != 0)
        return luaL_error(L, "lpp.running() takes no arguments.");
    lua_pushboolean(L, LPP_Running());

    return 1;
}

static int lpp_exit(lua_State *L)
{
    Int16 argc = lua_gettop(L);
    if(argc > 1)
        return luaL_error(L, "lpp.exit([ExitCode]) takes 1 or no arguments.");
    if(argc == 1) sceKernelExitGame();
	running = 0;
    return 0;
}
static L_CONST luaL_Reg lpp_methods[] = {
    { "delay", lpp_delay },
    { "running", lpp_running },
    { "exit", lpp_exit },
	{ "quit", lpp_exit },
    { 0, 0 }
};

L_CONST char *lpp_run_script(L_CONST char *_script, int _is_buffer)
{
    L = lua_open();

    luaL_openlibs(L);

    lua_newtable(L);
    luaL_register(L, 0, lpp_methods);
    lua_setglobal(L, "lpp");

    luaGraphics_Init(L);
    luaControls_init(L);
	luaArchive_Init(L);
	luaTimer_init(L);
	luaAdhoc_Init(L);
	luaWlan_init(L);
	luaSystem_Init(L);
	luaMp4_Init(L);
	luaSound_Init(L);
	luaXml_Init(L);
	luaCamera_Init(L);

    L_CONST char *_err = null;
    int _s = _is_buffer ?
        luaL_loadbuffer(L, _script, strlen(_script), null)
        :
        luaL_loadfile(L, _script);
    if(_s == 0) _s = lua_pcall(L, 0, LUA_MULTRET, 0);
    if(_s) {
        _err = lua_tostring(L, -1);
        lua_pop(L, 1);
    }
    return (_err);
}

int debug_output(L_CONST char *format, ...)
{
    va_list opt;
    char _buffer[1024 * 2];
    if(!debug_is_initialized) {
        pspDebugScreenInit();
        debug_is_initialized = 1;
    }
    size_t bufsz;
    va_start(opt, format);
    bufsz = vsnprintf(_buffer, sizeof(_buffer), format, opt);
    return pspDebugScreenPrintData(_buffer, bufsz);
}

int dwrite_output(L_CONST char *format, ...)
{
    FILE *out = fopen("lpp_err.log", "ab");
    if(out == null) return -1;
    va_list opt;
    char _buffer[1024 * 2];
    size_t size;
    va_start(opt, format);
    size = vsnprintf(_buffer, sizeof(_buffer), format, opt);
    fprintf(out, _buffer, size);
    fclose(out);
    return 0;
}

int Support_IsLoaded(void) {
    return !(Support_id < 0);
}

int InitSupport(void) {
    if(Support_IsLoaded())
	    return 0;

	char oldpath[256] = "";
	LPP_GetCwd(oldpath, 256);

	LPP_Chdir(_startup_path);
	if(!LPP_FileExists("Support.prx")) {
	    SceUID fd = sceIoOpen("Support.prx", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		if(fd < 0) goto error;
		if(sceIoWrite(fd, Support, size_Support) != size_Support) {
		    sceIoClose(fd);
			goto error;
		}
		sceIoClose(fd);
	}

	Support_id = pspSdkLoadStartModule("Support.prx", PSP_MEMORY_PARTITION_KERNEL);

error:
    LPP_Chdir(oldpath);
	return 0;
}

int UnloadSupport(void) {
    if(!Support_IsLoaded()) return 0;
    int ret = LPP_UtilsStopUnloadModule(Support_id);
	if(delete_support)
	{
	    if(LPP_FileExists("Support.prx"))
		    LPP_RemoveFile("Support.prx");
	}
    return (ret);
}

static void lpp_init_all(void)
{
    SetupCallbacks();
	InitSupport();
    LPPG_Init();
    LPPG_InitTimer();
    AalibInit();
    LPP_PsarDecoder_Init();
    pspDebugScreenInit();
    debug_is_initialized = 1;
}

int main(int argc, char *argv[])
{
    if(argc >= 2)
    {
        main_script_path = strdup(argv[1]);
    }

    delete_support = ini_getl("GENERAL", "DELETE_SUPPORT", GU_FALSE, "lpp.ini");

    if(!main_script_path)
    {
        char buf[255];
        ini_gets("GENERAL", "MAIN_SCRIPT", "index.lua", buf, 255, "lpp.ini");
        if(strcmp(buf, "index.lua"))
        {
            main_script_path = strdup(buf);
        }
    }

    lpp_init_all();

    while(running)
    {
        L_CONST char *_err = lpp_run_script(main_script_path ? main_script_path : "index.lua", GU_FALSE);
        LPPG_ClearScreen(0);
        chdir(_startup_path);

        if(_err != null) {
            debug_output("Error : %s\n", _err);
            dwrite_output("Error : %s\n", _err);
        }

        debug_output("\nPress 'start' to restart. \n");
        SceCtrlData input;
        sceCtrlReadBufferPositive(&input, 1);
        char i;
        for(i = 0; i < 40; i++) sceDisplayWaitVblankStart();
        while(!(input.Buttons & PSP_CTRL_START) && running) sceCtrlReadBufferPositive(&input, 1);

        debug_is_initialized = 0;
    }

	chdir(_startup_path);
    UnloadSupport();
    LPPG_Shutdown();
    lua_close(L);
	if(main_script_path) free(main_script_path);

	__psp_free_heap();
    sceKernelExitGame();
    return exit_code;
}
