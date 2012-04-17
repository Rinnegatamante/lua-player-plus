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

#include <kubridge.h>
#include <systemctrl.h>

#include "systemctrl_se.h"

#define SYSCON_CMD_BATTERY_WRITE_NVM    0x73     
#define SYSCON_CMD_BATTERY_READ_NVM     0x74  
#define DEboot "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"

PSP_MODULE_INFO("KernelFunctions", 0x1006, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

typedef struct sceSysconPacket
{
	u8 unk00[4];
	u8 unk04[2];
	u8 status;
	u8 unk07;
	u8 unk08[4];
	u8 tx_cmd;
	u8 tx_len;
	u8 tx_data[14];
	u8 rx_sts;
	u8 rx_len;
	u8 rx_response;
	u8 rx_data[9];
	u32 unk28;
	void (*callback)(struct sceSysconPacket *, u32);
	u32 callback_r28;
	u32 callback_arg2;
	u8 unk38[0x0D];
	u8 old_sts;
	u8 cur_sts;
	u8 unk47[0x21];
} sceSysconPacket;

int sceSysconCmdExec(sceSysconPacket *param, int unk);
u32 k1;
int sceSyscon_driver_7EC5A957(u32 *baryon);
u32 sceSyscon_driver_E7E87741(u32 *pommel);

void StartISO(char *Filename, int Driver) {
	struct SceKernelLoadExecVSHParam param;
	k1 = pspSdkSetK1(0);
	sctrlSESetUmdFile(Filename);
	sctrlSESetBootConfFileIndex((Driver == 1) ? MODE_MARCH33 : (Driver == 2) ? MODE_NP9660 : MODE_INFERNO);
	memset(&param, 0, sizeof(param));
	param.size = sizeof(param); param.args = strlen(DEboot) + 1;
	param.argp = DEboot;
	if(kuKernelGetModel() == 3) {
		param.key = "umdemu";
		sctrlKernelLoadExecVSHWithApitype((kuKernelGetModel() == 4) ? 0x125 : 0x123, DEboot, &param);
	} else {
		param.key = "game";
		sctrlKernelLoadExecVSHWithApitype(0x120, DEboot, &param);
	}
	pspSdkSetK1(k1);
}

int LauncPops(char *Path) {
	struct SceKernelLoadExecVSHParam param;
	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.args = strlen(Path) + 1;
	param.argp = (char*)Path;
	param.key = (char*)"pops";
	int ret = sctrlKernelLoadExecVSHWithApitype((kuKernelGetModel() == 3) ? 0x155 : 0x144, Path, &param);
	return(ret);
}

u16 Read_eeprom(u8 addr) {
	if(addr > 0x7F) return(0x0);
	u8 param[0x60];
	param[0x0C] = 0x74;
	param[0x0D] = 3;
	param[0x0E] = addr;
	u32 k1 = pspSdkSetK1(0x0);
	int res = sceSysconCmdExec(param, 0);
	pspSdkSetK1(k1);
	if(res < 0) return(res);
	return((param[0x21] << 8) | param[0x20]);
}

u32 Write_eeprom(u8 addr, u16 data) {
	u32 k1 = pspSdkSetK1(0x0);
	int res;
	u8 param[0x60];
	if (addr > 0x7F) return(0x80000102);
	param[0x0C] = 0x73;
	param[0x0D] = 5;
	param[0x0E] = addr;
	param[0x0F] = data;
	param[0x10] = data >> 8;
	res = sceSysconCmdExec(param, 0x0);
	if(res < 0x0) return(res);
	pspSdkSetK1(k1);
	return(0x0);
}

u32 getBaryon(void) {
	u32 k1 = pspSdkSetK1(0x0);
	u32 baryon;
	sceSyscon_driver_7EC5A957(&baryon);
	pspSdkSetK1(k1);
	return(baryon);
}

u32 getPommel(void) {
	u32 k1 = pspSdkSetK1(0x0);
	u32 pommel;
	sceSyscon_driver_E7E87741(&pommel);
	pspSdkSetK1(k1);
	return(pommel);
}

int g_tachyon_ver = -1;

int sceSysregGetTachyonVersion()
{
	u32 k1 = pspSdkSetK1(0x0);
	if(g_tachyon_ver != -1) return(g_tachyon_ver);
	u32 intr = sceKernelCpuSuspendIntr();
	if(*(volatile u32*)(0xBC100068) & 0xFFFF0000)
		g_tachyon_ver = (*(volatile u32*)(0xBFC00FFC) > 0x20040224) ? 0x00010000 : 0;
	else
		if(*(u32*)(0xBC100040) & 0xFF000000)
			g_tachyon_ver = (*(u32*)(0xBC100040) & 0xFF000000) >> 8;
		else
			g_tachyon_ver = 0x00100000;
	sceKernelCpuResumeIntr(intr);
	pspSdkSetK1(k1);
	return(g_tachyon_ver);
}

int module_start(SceSize args, void *argp)
{
	return(0x0);
}

int module_stop(void)
{
	return(0x0);
}