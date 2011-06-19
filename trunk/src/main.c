/*
 * LuaPlayer Euphoria
 * ------------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE for details.
 *
 * Copyright (c) 2005 Frank Buss <fb@frank-buss.de> (aka Shine)
 * Copyright (c) 2009 Danny Glover <danny86@live.ie> (aka Zack) 
 *
 * Official Forum : http://www.retroemu.com/forum/forumdisplay.php?f=148
 * For help using LuaPlayer, code help, tutorials etc please visit the official site : http://www.retroemu.com/forum/forumdisplay.php?f=148
 *
 * Credits:
 * 
 * (from Shine/Zack) 
 *
 *   many thanks to the authors of the PSPSDK from http://forums.ps2dev.org
 *   and to the hints and discussions from #pspdev on freenode.net
 *
 * (from Zack Only)
 *
 * Thanks to Brunni for the Swizzle/UnSwizzle code (taken from oslib). 
 * Thanks to Arshia001 for AALIB. It is the sound engine used in LuaPlayer Euphoria. 
 * Thanks to HardHat for being a supportive friend and advisor.
 * Thanks to Benhur for IntraFont.
 * Thanks to Jono for the moveToVram code.
 * Thanks to Raphael for the Vram manager code.
 * Thanks to Osgeld, Dan369 & Cmbeke for testing LuaPlayer Euphoria for me and coming up with some neat ideas for it.
 * Thanks to the entire LuaPlayer Euphoria userbase, for using it and for supporting it's development. You guys rock!
 *
 *
 */

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
#include "libs/intraFont/intraFont.h"
#include "libs/aalib/pspaalib.h"

/* the boot.lua */
#include "src/boot.c"

/* Define the module info section */
PSP_MODULE_INFO("LuaPlayer Euphoria", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-256);

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

int main(int argc, char *argv[])
{
	SetupCallbacks();
	initGraphics();
	pspDebugScreenInit();
	intraFontInit();
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

		const char *errMsg = runScript("index.lua", false);
		
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
