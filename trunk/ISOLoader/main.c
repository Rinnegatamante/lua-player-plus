#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdisplay_kernel.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pspkernel.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <psppower.h>
#include <pspdebug.h>
#include <psprtc.h>
#include <pspLoadExec.h>
#include <stdarg.h>
#include <pspgu.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspwlan.h>
#include <pspopenpsid.h>
#include <psploadexec_kernel.h>
#include "../src/include/kubridge.h"
#include "../src/include/systemctrl.h"
#include "systemctrl_se.h"

PSP_MODULE_INFO("ISOLoader", 0x1006, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

u32 k1;

void startISO(char* file,int driver){
struct SceKernelLoadExecVSHParam param;
k1 = pspSdkSetK1(0);
SEConfig config;
sctrlSEGetConfig(&config);
sctrlSESetUmdFile(file);
if (driver == 1){
sctrlSESetBootConfFileIndex(MODE_MARCH33);
}
if (driver == 2){
sctrlSESetBootConfFileIndex(MODE_NP9660);
}
if (driver == 3){
sctrlSESetBootConfFileIndex(MODE_INFERNO);
}
sctrlSESetDiscType(0x10);
memset(&param, 0, sizeof(param));
param.size = sizeof(param);
param.args = strlen("disc0:/PSP_GAME/SYSDIR/EBOOT.BIN") + 1;
param.argp = "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";
if (kuKernelGetModel() == 3){
param.key = "umdemu";
if (strcmp(file,"ef0:/")){
sctrlKernelLoadExecVSHWithApitype(0x125, "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN", &param);
}else{
sctrlKernelLoadExecVSHWithApitype(0x123, "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN", &param);
}
}else{
param.key = "game";
sctrlKernelLoadExecVSHWithApitype(0x120, "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN", &param);
}
    pspSdkSetK1(k1);
}

int module_start(SceSize args, void *argp)
{
   return 0;
}

int module_stop()
{
   return 0;
} 