#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspsdk.h>
#include <psputility.h>
#include <psprtc.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "libs/graphics/graphics.h"
#include "include/luaplayer.h"
#include "libs/aalib/pspaalib.h"

/* the boot.lua */
#include "boot.c"

// extralibs.lua
#include "extralibs.c"

/* Define the module info section */
PSP_MODULE_INFO("LuaPlayer Plus", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(0);
PSP_HEAP_SIZE_KB(-1024);

// startup path
char path[256];

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

bool debugInitialized = false;

void debugResetScreen()
{
	debugInitialized = false;
}

int debugOutput(const char *format, ...)
{
	va_list opt;
	char buffer[2048];
	int bufsz;

	if (!debugInitialized) {
		disableGraphics();
		pspDebugScreenInit();
		debugInitialized = true;
	}
	va_start(opt, format);
	bufsz = vsnprintf( buffer, (size_t) sizeof(buffer), format, opt);
	return pspDebugScreenPrintData(buffer, bufsz);
}

int user_main( SceSize argc, void *argp )
{

	SetupCallbacks();
	initGraphics();
	pspDebugScreenInit();
	AalibInit();
	initTimer(); //For FPS

	// execute Lua script (according to boot sequence)
	getcwd(path, 256);
	char* bootStringWith0 = (char*) malloc(size_bootString + 1);
	memcpy(bootStringWith0, bootString, size_bootString);
	bootString[size_bootString] = 0;

	while(1)
	{
		chdir(path); // set base path luaplayer/
		getDeltaTime(); //For FPS

		const char *errMsg = runScript(extralibs, true);

		if (errMsg != NULL);
		{
			debugOutput("Error: %s\n", errMsg);
		}
		debugOutput("\nPress start to restart\n");

		SceCtrlData pad; int i;
		sceCtrlReadBufferPositive(&pad, 1);
		for(i = 0; i < 40; i++) sceDisplayWaitVblankStart();
		while(!(pad.Buttons&PSP_CTRL_START)) sceCtrlReadBufferPositive(&pad, 1);

		debugResetScreen();
		initGraphics();
	}

	free(bootStringWith0);

	// wait until user ends the program
	sceKernelSleepThread();

	return 0;
}

int main(SceSize argc, char **argv)
{
	// create user thread, tweek stack size here if necessary
	SceUID thid = sceKernelCreateThread("User Mode Thread", user_main,
	    0x11, // default priority
	    256 * 1024, // stack size (256KB is regular default)
	    PSP_THREAD_ATTR_USER, NULL);
	
	// start user thread, then wait for it to do everything else
	sceKernelStartThread(thid, 0, NULL);
	sceKernelWaitThreadEnd(thid, NULL);

	sceKernelExitGame();
	return 0;
}
