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
#- Copyright (c) Nanni <lpp.nanni@gmail.com> ---------------------------------------------------------------------------#
#- Copyright (c) Rinnegatamante <rinnegatamante@gmail.com> -------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Credits : -----------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Zack & Shine for LP Euphoria sourcecode -----------------------------------------------------------------------------#
#- valantin for sceIoMvdir and sceIoCpdir improved functions------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------*/

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
#include "include/kubridge.h"
#include "include/luaPlayer.h"

// Needed libs
#include "extralibs.c"

#define MAX_THREAD 64
/* Define the module info section */
PSP_MODULE_INFO("LuaPlayer Plus", 1, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

// Important Plugin definitions
static thid1 = -1;
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
        u32 keycombination;
    SceCtrlData pad;
    u32 oldButtons = 0;
       
         keycombination = PSP_CTRL_RTRIGGER; //Button to start interpreter
       
        while(1){
            oldButtons = pad.Buttons;
            if (go==0){
             sceCtrlPeekBufferPositive(&pad, 1);
                          if(oldButtons != pad.Buttons)
                          {
            if(pad.Buttons & keycombination)
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
       
        int go2=1;
        SceUID id;
        if (((kuKernelGetModel() + 1) == 4) || ((kuKernelGetModel() + 1) == 5)){
        id = sceIoDopen("ef0:/seplugins/script");
        }else{
        id = sceIoDopen("ms0:/seplugins/script");
        }
                                 SceIoDirent entry;
                                 int script_files = -2;
                                 memset(&entry, 0, sizeof(SceIoDirent));
                             while (sceIoDread(id, &entry) > 0)
                                {
                                        script_files = script_files+1;
                                        memset(&entry, 0, sizeof(SceIoDirent));
                                }
                                sceIoDclose(id);
                                char script[256];
                                if (((kuKernelGetModel() + 1) == 4) || ((kuKernelGetModel() + 1) == 5)){
                                strcpy(script,"ef0:/seplugins/script/index.lua");
                                }else{
                                 strcpy(script,"ms0:/seplugins/script/index.lua");
                                 }
        while(go2==1)
        {
                                 
                                const char *errMsg;
                                if (script_files>1){            
                                errMsg = runScript(extralibs, true);
                                }else{
    SceUID fp = sceIoOpen(script, PSP_O_RDONLY,0777);  
    int size = sceIoLseek(fp, 0, SEEK_END);
    sceIoLseek(fp, 0, SEEK_SET);
    unsigned char *buffer;
    buffer = malloc((size+1) * sizeof (char));
    sceIoRead(fp, buffer, size);
        buffer[size]=0;
    sceIoClose(fp);
    errMsg = runScript(buffer, true);
    free(buffer);
    }
    // Temp replacing for loadfile/dofile functions: System.protodofile
    if (strstr(errMsg, "lpp_open")){ ;
    char dum1[20], dum2[20], dum3[20];
    char script_path2[256];
    sscanf( errMsg, "%s %s %s %s", dum1, dum2, dum3, script_path2 );
    strcpy(script,script_path2);
    script_files=1;
    // End System.protodofile sources
                                }else if (strstr(errMsg, "resumeThread")){
                                go=0;
                                go2=0;
                                }else{
                if (errMsg != NULL);
                {
                                                pspDebugScreenClear();
                                                pspDebugScreenSetTextColor(0xffffff);
                        debugOutput("\nError: %s\n", errMsg);
                }
                debugOutput("\nPress start to restart\nPress select to resume thread\n");
                SceCtrlData pad;
               
                                int restore = 0;
                while(restore==0){
                                sceCtrlPeekBufferPositive(&pad, 1);
                                if (pad.Buttons&PSP_CTRL_START){
                                restore=1;
                                go2=0;
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
