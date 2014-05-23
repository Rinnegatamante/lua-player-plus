#include <pspkernel.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <pspgu.h>
#include <pspthreadman.h>
#include <stdio.h>
#include <string.h>
#include <pspmath.h>
#include <psploadcore.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <malloc.h>
#include "include/luaPlayer.h"
#define printfa  pspDebugScreenPrintf

/* the boot.lua */
#include "boot.c"

#define MAX_THREAD 64
/* Define the module info section */
PSP_MODULE_INFO("LuaPlayer Plus", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(0);
PSP_HEAP_SIZE_KB(2048);

// Important Plugins definitions
static int thread_count_start, thread_count_now;
static SceUID pauseuid = -1, thread_buf_start[MAX_THREAD], thread_buf_now[MAX_THREAD], thid1 = -1;

void pauseGame(SceUID thid)
{
if(pauseuid >= 0)
return;
pauseuid = thid;
sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, thread_buf_now, MAX_THREAD, &thread_count_now);
int x, y, match;
for(x = 0; x < thread_count_now; x++)
{
match = 0;
SceUID tmp_thid = thread_buf_now[x];
for(y = 0; y < thread_count_start; y++)
{
if((tmp_thid == thread_buf_start[y]) || (tmp_thid == thid1))
{
match = 1;
break;
}
}
if(match == 0)
sceKernelSuspendThread(tmp_thid);
}
}

void resumeGame(SceUID thid)
{
if(pauseuid != thid)
return;
pauseuid = -1;
int x, y, match;
for(x = 0; x < thread_count_now; x++)
{
match = 0;
SceUID tmp_thid = thread_buf_now[x];
for(y = 0; y < thread_count_start; y++)
{
if((tmp_thid == thread_buf_start[y]) || (tmp_thid == thid1))
{
match = 1;
break;
}
}
if(match == 0)
sceKernelResumeThread(tmp_thid);
}
}

// startup path
char path[256];

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();
	return 0;
}

int debugOutput(const char *format, ...)
{
        va_list opt;
        char buffer[2048];
        int bufsz;
        va_start(opt, format);
        bufsz = vsnprintf( buffer, (size_t) sizeof(buffer), format, opt);
        return pspDebugScreenPrintData(buffer, bufsz);
}


int go=0;
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
                    pspDebugScreenSetXY(2,2);
    pspDebugScreenSetTextColor(0x00FF00);
    printfa(" Beta LPP started successfully!\n");
	
	//Bootstrap
	char* bootStringWith0 = (char*) malloc(size_bootString + 1);
	memcpy(bootStringWith0, bootString, size_bootString);
	bootString[size_bootString] = 0;

        while(1)
        {
				printfa(" Now loading ms0:/seplugins/script/index.lua\n");
                const char *errMsg = runScript("ms0:/seplugins/script/index.lua", false);

                if (errMsg != NULL);
                {
                        debugOutput("Error: %s\n", errMsg);
                }
                debugOutput("\nPress start to restart\n");

                SceCtrlData pad; int i;
                sceCtrlReadBufferPositive(&pad, 1);
                for(i = 0; i < 40; i++) sceDisplayWaitVblankStart();
                while(!(pad.Buttons&PSP_CTRL_START)) sceCtrlReadBufferPositive(&pad, 1);

        }
sceDisplayWaitVblankStart();
free(bootStringWith0);
}
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
