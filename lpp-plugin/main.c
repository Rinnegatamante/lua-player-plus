#include <pspkernel.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <pspthreadman.h>
#include <stdio.h>
#include <string.h>
#include <pspmath.h>
#include <psploadcore.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <malloc.h>
#include "include/luaPlayer.h"

/* the boot.lua */
#include "boot.c"

#define MAX_THREAD 64
/* Define the module info section */
PSP_MODULE_INFO("LuaPlayer Plus", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

// Important Plugins definitions
static int thread_count_start, thread_count_now;
static SceUID pauseuid = -1, thread_buf_start[MAX_THREAD], thread_buf_now[MAX_THREAD], thid1 = -1;

// startup path
char path[256];

int go=0;

int debugOutput(const char *format, ...)
{
        va_list opt;
        char buffer[2048];
        int bufsz;
        va_start(opt, format);
        bufsz = vsnprintf( buffer, (size_t) sizeof(buffer), format, opt);
        return pspDebugScreenPrintData(buffer, bufsz);
}



int main_thread(SceSize args, void *argp) {
sceKernelDelayThread(3000000);
                    SceCtrlData pad;
    u32 oldButtons = 0;
	while(1)
{
            oldButtons = pad.Buttons;
            if (go==0){
             sceCtrlPeekBufferPositive(&pad, 1);
                          if(oldButtons != pad.Buttons)
                          {
            if(pad.Buttons & PSP_CTRL_LTRIGGER)
            {
                pauseGame(thid1);
                    go=1;
					pspDebugScreenInit();
                    pspDebugScreenClear();
                    oldButtons = pad.Buttons;
            }
            }
            }
                if (go ==1){
    pspDebugScreenSetXY(0,0);
    pspDebugScreenSetTextColor(0xffffff);
	
	//Bootstrap
	char* bootStringWith0 = (char*) malloc(size_bootString + 1);
	memcpy(bootStringWith0, bootString, size_bootString);
	bootString[size_bootString] = 0;
	int go2=1;
        while(go2==1)
        {
                const char *errMsg = runScript("ms0:/seplugins/script/index.lua", false);
				if (strstr(errMsg, "resumeThread")){
				go=0;
				go2=0;
				}else{
                if (errMsg != NULL);
                {
						pspDebugScreenClear();
                        debugOutput("\nError: %s\n", errMsg);
                }
                debugOutput("\nPress start to restart\nPress select to resume thread\n");

                SceCtrlData pad; int i;
                
                for(i = 0; i < 40; i++) sceDisplayWaitVblankStart();
				int restore = 0;
                while(restore==0){
				sceCtrlPeekBufferPositive(&pad, 1);
				if (pad.Buttons&PSP_CTRL_START){
				restore=1;
				}
				if (pad.Buttons&PSP_CTRL_SELECT){
				resumeGame(thid1);
				restore=1;
				go=0;
				go2=0;
				}
				}
				}
        }

free(bootStringWith0);
}
sceDisplayWaitVblankStart();
        }
	
	sceKernelSleepThread();
return 0;
}

int module_start(SceSize args, void *argp) {

	int thid;

	/* Create a high priority thread */
	thid = sceKernelCreateThread("LPP", main_thread, 0x18, 0x1000, 0, NULL);//8, 64*1024, PSP_THREAD_ATTR_USER, NULL);
	if(thid >= 0) sceKernelStartThread(thid, args, argp);

	return 0;
}
