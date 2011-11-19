#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pspdisplay_kernel.h>
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
#define SYSCON_CMD_BATTERY_WRITE_NVM    0x73     
#define SYSCON_CMD_BATTERY_READ_NVM     0x74  

PSP_MODULE_INFO("KernelFunctions", 0x1006, 1, 1);
PSP_MAIN_THREAD_ATTR(0);
   
//THIS STRUCTURE WAS CREATED BY BOOSTER AND I DOWNLOADED IT FROM HERE: http://forums.maxconsole.net/showthread.php?t=78436     
typedef struct sceSysconPacket     
{     
    u8  unk00[4];       // +0x00 ?(0x00,0x00,0x00,0x00)     
    u8  unk04[2];       // +0x04 ?(arg2)     
    u8  status;         // +0x06     
    u8  unk07;          // +0x07 ?(0x00)     
    u8  unk08[4];       // +0x08 ?(0xff,0xff,0xff,0xff)     
// transmit data     
    u8  tx_cmd;         // +0x0C command code     
    u8  tx_len;         // +0x0D number of transmit bytes     
    u8  tx_data[14];    // +0x0E transmit parameters     
// receive data     
    u8  rx_sts;         // +0x1C generic status     
    u8  rx_len;         // +0x1D receive length     
    u8  rx_response;    // +0x1E response code(tx_cmd or status code)     
    u8  rx_data[9];     // +0x1F receive parameters     
// ?     
    u32 unk28;          // +0x28     
// user callback (when finish an access?)     
    void (*callback)(struct sceSysconPacket *,u32); // +0x2c     
    u32 callback_r28;   // +0x30     
    u32 callback_arg2;  // +0x34 arg2 of callback (arg4 of sceSysconCmdExec)     
    
    u8  unk38[0x0D];    // +0x38     
    u8  old_sts;        // +0x45 old     rx_sts     
    u8  cur_sts;        // +0x46 current rx_sts     
    u8  unk47[0x21];    // +0x47     
} sceSysconPacket;                  
//prototipo per sceSysconCmdExec     
int sceSysconCmdExec(sceSysconPacket *param, int unk);   
u32 k1;
int sceSyscon_driver_7EC5A957(u32 *baryon);
u32 sceSyscon_driver_E7E87741(u32 *pommel);

void startISO(char* file,int driver){
struct SceKernelLoadExecVSHParam param;
k1 = pspSdkSetK1(0);
sctrlSESetUmdFile(file);
SceUID pFile;
if (driver == 1){
sctrlSESetBootConfFileIndex(MODE_MARCH33);
}
if (driver == 2){
sctrlSESetBootConfFileIndex(MODE_NP9660);
}
if (driver == 3){
sctrlSESetBootConfFileIndex(MODE_INFERNO);
}
memset(&param, 0, sizeof(param));
param.size = sizeof(param);
param.args = strlen("disc0:/PSP_GAME/SYSDIR/EBOOT.BIN") + 1;
param.argp = "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";
if (kuKernelGetModel() == 3){
param.key = "umdemu";
char *to;
strncpy(to, file, 5);
if (strcmp(to,"ef0:/")){
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

int launch_pops(char *path)
{
        struct SceKernelLoadExecVSHParam param;
        int apitype, ret;
        const char *mode;
		if (kuKernelGetModel() == 3){
        apitype = 0x155;
		}else{
		apitype = 0x144;
		}
        mode = "pops";
        memset(&param, 0, sizeof(param));
        param.size = sizeof(param);
        param.args = strlen(path) + 1;
        param.argp = (char *) path;
        param.key = mode;
        ret = sctrlKernelLoadExecVSHWithApitype(apitype, path, &param);
        return ret;
}

u16 read_eeprom(u8 addr){ // reversed function by silverspring (more info: http://my.malloc.us/silverspring/2007/12/19/380-and-pandora/)
	if(addr>0x7F)
		return(0);
	u8 param[0x60];
	param[0x0C] = 0x74; // read battery eeprom command
	param[0x0D] = 3;	// tx packet length
	param[0x0E] = addr;	// tx data
 	u32 k1 = pspSdkSetK1(0);
	int res = sceSysconCmdExec(param, 0);
	pspSdkSetK1(k1);
	if (res < 0)
		return(res);
	return((param[0x21]<<8) | param[0x20]);// rx data
}

u32 write_eeprom(u8 addr, u16 data){ // reversed function by silverspring (more info: http://my.malloc.us/silverspring/2007/12/19/380-and-pandora/)
	u32 k1 = pspSdkSetK1(0);
	int res;
	u8 param[0x60];
	if (addr > 0x7F)
		return(0x80000102);
	param[0x0C] = 0x73; // write battery eeprom command
	param[0x0D] = 5;	// tx packet length
	param[0x0E] = addr;// tx data
	param[0x0F] = data;
	param[0x10] = data>>8;
	res = sceSysconCmdExec(param, 0);
	if (res < 0)
		return(res);
	pspSdkSetK1(k1);
	return 0;
}

u32 getBaryon(){
	u32 k1 = pspSdkSetK1(0);
	u32 baryon;
	sceSyscon_driver_7EC5A957(&baryon);
	pspSdkSetK1(k1);
	return(baryon);
}

u32 getPommel(){
	u32 k1 = pspSdkSetK1(0);
	u32 pommel;
	sceSyscon_driver_E7E87741(&pommel);
	pspSdkSetK1(k1);
	return(pommel);
}

int g_tachyon_ver = -1;            // 0x40

int sceSysregGetTachyonVersion()
{
	u32 k1 = pspSdkSetK1(0);
    if (g_tachyon_ver != -1)
        return(g_tachyon_ver);

    u32 intr = sceKernelCpuSuspendIntr();

    // on devkit?
    if (*(volatile u32*)(0xBC100068) & 0xFFFF0000)
    {
        // date of devkit preipl?
        // retail preipl date: 0x20040420
        g_tachyon_ver = (*(volatile u32*)(0xBFC00FFC) > 0x20040224) ? 0x00010000 : 0;
    }
    else
    {
        if (*(u32*)(0xBC100040) & 0xFF000000) // single mem read, no need to volatile
            g_tachyon_ver = (*(u32*)(0xBC100040) & 0xFF000000) >> 8;
        else
            g_tachyon_ver = 0x00100000;

    }

    sceKernelCpuResumeIntr(intr);
	pspSdkSetK1(k1);
    return(g_tachyon_ver);
}

int module_start(SceSize args, void *argp)
{
   return 0;
}

int module_stop()
{
   return 0;
} 