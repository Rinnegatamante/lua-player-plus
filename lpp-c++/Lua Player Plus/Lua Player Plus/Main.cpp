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
#- For help using LuaPlayerPlus, code help, and other please visit : http://rinnegatamante.eu/luaplayerplus/forum.php --#
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

// Includes
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <psppower.h>
#include <pspctrl.h>
#include <pspsdk.h>
#include <psputility.h>
#include <psprtc.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <pspiofilemgr.h>

extern "C" {
#include <kubridge.h>
}

#include "Common.h"
#include "LuaPlayer.h"

/* Buffers*/
#include "Boot.cpp"
#include "KernelFunctions.cpp"
#include "Extralibs.cpp"

/* Module Info */
#if !defined(MODULE_NAME)
#define MODULE_NAME "Lua Player Plus"
#endif

#if !defined(MODULE_VERSION_MAJOR)
#define MODULE_VERSION_MAJOR 1
#endif

#if !defined(MODULE_VERSION_MINOR)
#define MODULE_VERSION_MINOR 0
#endif

#if !defined(MODULE_ATTR)
#define MODULE_ATTR 0
#endif

PSP_MODULE_INFO(MODULE_NAME, MODULE_ATTR, MODULE_VERSION_MAJOR, MODULE_VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(20 * 1024);

char _StartupPath[256];

/* Exit Callback */
int exit_callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();
	return(0x0);
}

/* Callback Thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;
	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
	return(0x0);
}

/* Sets up the callback thread and returns it Thread id */
int SetupCallbacks(void)
{
	int thid = 0;
	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}
	return(thid);
}

bool DebugInitialized = false;

void DebugResetScreen(void)
{
	DebugInitialized = false;
}

int DebugOutput(const char *format, ...)
{
	va_list opt;
	char buffer[2048];
	int bufsz;
	if(!DebugInitialized) {
		pspDebugScreenInit();
		DebugInitialized = true;
	}
	va_start(opt, format);
	bufsz = vsnprintf(buffer, (size_t)sizeof(buffer), format, opt);
	return pspDebugScreenPrintData(buffer, bufsz);
}

SceUID kId = 0x0;

void Inizializza(void)
{
	SetupCallbacks();
	oslInit(1);
	oslInitGfx(OSL_PF_8888, 1);
	oslInitAudio();
	oslInitAudioME(1);
	oslIntraFontInit(INTRAFONT_CACHE_MED);
	oslSetDrawBuffer(OSL_SECONDARY_BUFFER);
	initTimer();
	pspDebugScreenInit();
	FILE *kf;
	const char *prxPath = (kuKernelGetModel() == 4) ? "ef0:/Kf.prx" : "ms0:/Kf.prx";
	if((kf = fopen(prxPath, "wb")) == NULL) return;
	fwrite(KernelFunctions, 1, size_KernelFunctions, kf);
	fclose(kf);
	kId = pspSdkLoadStartModule(prxPath, PSP_MEMORY_PARTITION_KERNEL);
	sceIoRemove(prxPath);
}

int main(int argc, char* argv[])
{
	Inizializza();
	getcwd(_StartupPath, 256);
	char *bootStringWith0 = (char*)malloc(size_bootString + 1);
	memcpy(bootStringWith0, bootString, size_bootString);
	bootString[size_bootString - 1] = 0;
	while(!osl_quit)
	{
		getDeltaTime();
		chdir(_StartupPath);
		const char *ErrMsg = RunScript((const char*)extralibs, true);
		if(ErrMsg != NULL)
		{
			DebugOutput("Error : %s\n", ErrMsg);
		}
		DebugOutput("\nPress Start to Restart. \n");
		SceCtrlData Input;
		sceCtrlReadBufferPositive(&Input, 1);
		for(int i = 0; i < 40; i++) sceDisplayWaitVblankStart();
		while(!(Input.Buttons & PSP_CTRL_START)) sceCtrlReadBufferPositive(&Input, 1);
		DebugResetScreen();
	}
	free(bootStringWith0);
	int status = 0;
	sceKernelStopModule(kId, 0, NULL, &status, NULL);
	sceKernelUnloadModule(kId);
	sceKernelExitGame();
	return(0x0);
}