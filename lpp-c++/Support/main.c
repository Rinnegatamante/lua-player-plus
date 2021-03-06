/** LPP Support by Nanni */

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
#- JiC� for drawCircle function ----------------------------------------------------------------------------------------#
#- Rapper_skull & DarkGiovy for testing LuaPlayer Plus and coming up with some neat ideas for it. ----------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------*/

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pspdisplay_kernel.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pspkernel.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <psppower.h>
#include <pspdebug.h>
#include <psprtc.h>
#include <stdarg.h>
#include <pspgu.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspwlan.h>
#include <pspopenpsid.h>
#include <psploadexec_kernel.h>
#include <pspdisplay.h>

#include "../Libs/Kubridge/kubridge.h"

#include "../Libs/Sysctrl/systemctrl_se.h"
#include "../Libs/Sysctrl/systemctrl.h"

#define SYSCON_CMD_BATTERY_WRITE_NVM    0x73
#define SYSCON_CMD_BATTERY_READ_NVM     0x74
#define DEboot "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"

PSP_MODULE_INFO("Support", 0x1006, 1, 0);
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

int DirExist(const char *Directory) {
	SceIoStat state;
	sceIoGetstat(Directory, &state);
	if (state.st_mode & FIO_S_IFDIR)
	    return 0;
	else
		return 1;
}

void StartISO(char *Filename, int Driver) {
	struct SceKernelLoadExecVSHParam param;
	k1 = pspSdkSetK1(0);
	sctrlSESetUmdFile(Filename);
	sctrlSESetBootConfFileIndex((Driver == 1) ? MODE_MARCH33 : (Driver == 2) ? MODE_NP9660 : MODE_INFERNO);
	memset(&param, 0, sizeof(param));
	param.size = sizeof(param); param.args = strlen(DEboot) + 1;
	param.argp = (void*)DEboot;
	if(kuKernelGetModel() == 3) {
		param.key = "umdemu";
		sctrlKernelLoadExecVSHWithApitype((kuKernelGetModel() == 4) ? 0x125 : 0x123, DEboot, &param);
	} else {
		param.key = "game";
		sctrlKernelLoadExecVSHWithApitype(0x120, DEboot, &param);
	}
	pspSdkSetK1(k1);
}

int LaunchPops(char *Path) {
	struct SceKernelLoadExecVSHParam param;
	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.args = strlen(Path) + 1;
	param.argp = (char*)Path;
	param.key = (char*)"pops";
	int ret = sctrlKernelLoadExecVSHWithApitype((kuKernelGetModel() == 3) ? 0x155 : 0x144, Path, &param);
	return(ret);
}

int RunEboot(const char *Path) {
    struct SceKernelLoadExecVSHParam param;
    char argp[256]; int args;
    strcpy(argp, Path);
    args = strlen(Path) + 1;
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.args = args;
    param.argp = argp;
    param.key  = "game";
    param.vshmain_args_size = 0;
    param.vshmain_args = NULL;
    int Ret = 0;
    switch(kuKernelGetModel()) {
    case 3 : Ret = sctrlKernelLoadExecVSHEf2(Path, &param); break;
    default : Ret = sctrlKernelLoadExecVSHMs2(Path, &param); break;
    }
    return(Ret);
}

int LaunchUMD(void) {
    if (!DirExist("disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"))
		sctrlKernelLoadExecVSHDisc("disc0:/PSP_GAME/SYSDIR/BOOT.BIN", NULL);
	else
	if (!DirExist("disc0:/PSP_GAME/SYSDIR/BOOT.BIN"))
		sctrlKernelLoadExecVSHDisc("disc0:/PSP_GAME/SYSDIR/EBOOT.BIN", NULL);
	else return 0;
	return 1;
}

int RunUpdate(const char *Path) {
	struct SceKernelLoadExecVSHParam param;
	char argp[256];
	int args;
	strcpy(argp, Path);
	args = strlen(Path)+1;
	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.args = args;
	param.argp = argp;
	param.key = NULL;
	param.vshmain_args_size = 0;
	param.vshmain_args = NULL;
	return sctrlKernelLoadExecVSHMs1(Path, &param);
}

u16 Read_eeprom(u8 addr) {
	if(addr > 0x7F) return(0x0);
	u8 param[0x60];
	param[0x0C] = 0x74;
	param[0x0D] = 3;
	param[0x0E] = addr;
	u32 k1 = pspSdkSetK1(0x0);
	int res = sceSysconCmdExec((sceSysconPacket*)param, 0);
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
	res = sceSysconCmdExec((sceSysconPacket*)param, 0x0);
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

void setDisplayBrightness(int value)
{
    sceDisplaySetBrightness(value, 0);
    pspSdkSetK1(k1);
}

int getDisplayBrightness()
{
    int value = 0, unk;
    sceDisplayGetBrightness(&value, &unk);
    return value;
}

int getMbr(const char *out)
{
    SceUID fd = sceIoOpen("msstor:", PSP_O_RDONLY, 0777);
    if(fd <= 0)
    {
        return -1;
    }
    unsigned char *mbr = (unsigned char*)malloc(512);
    if(!mbr)
    {
        sceIoClose(fd);
        return -1;
    }
    sceIoRead(fd, mbr, 512);
    sceIoClose(fd);

    fd = sceIoOpen(out, PSP_O_WRONLY | PSP_O_CREAT, 0777);
    if(fd <= 0)
    {
        free(mbr);
        return -1;
    }
    sceIoWrite(fd, mbr, 512);
    sceIoClose(fd);

    free(mbr);

    return 0;
}

int setMbr(const char *in)
{
    SceUID fd = sceIoOpen(in, PSP_O_RDONLY, 0777);
    if(fd <= 0)
    {
        return -1;
    }
    unsigned char *mbr = (unsigned char*)malloc(512);
    if(!mbr)
    {
        sceIoClose(fd);
        return -1;
    }
    sceIoRead(fd, mbr, 512);
    sceIoClose(fd);

    fd = sceIoOpen("msstor:", PSP_O_WRONLY, 0777);
    if(fd <= 0)
    {
        free(mbr);
        return -1;
    }
    sceIoWrite(fd, mbr, 512);
    sceIoClose(fd);

    free(mbr);

    return 0;
}

int getIpl(const char *out)
{
    SceUID fd = sceIoOpen("msstor:", PSP_O_RDONLY, 0777);
    if(fd <= 0)
    {
        return -1;
    }
    unsigned char *mbr = (unsigned char*)malloc(512);
    if(!mbr)
    {
        sceIoClose(fd);
        return -1;
    }
    sceIoRead(fd, mbr, 512);
    unsigned int part = (mbr[454] | mbr[455] << 8 | mbr[456] << 16 | mbr[457] << 24);
    free(mbr);

    unsigned char *buffer = (unsigned char*)malloc((part << 9) - 0x2000);
    if(!buffer)
    {
        sceIoClose(fd);
        return -1;
    }
    sceIoLseek(fd, 0x2000, 0);
    sceIoRead(fd, buffer, (part << 9) - 0x2000);
    sceIoClose(fd);

    fd = sceIoOpen(out, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_NBLOCK, 0777);
    if(fd <= 0)
    {
        free(buffer);
        return -1;
    }
    sceIoWrite(fd, buffer, ((part << 9) - 0x2000));
    sceIoClose(fd);
    free(buffer);

    return 0;
}

int setIpl(const char *in)
{
    SceUID fd = sceIoOpen(in, PSP_O_RDONLY, 0777);
    if(fd <= 0)
    {
        return -1;
    }
    int size = sceIoLseek(fd, 0, PSP_SEEK_END);
    unsigned char *buffer = (unsigned char*)malloc(size);
    sceIoLseek(fd, 0, PSP_SEEK_SET);
    sceIoRead(fd, buffer, size);
    sceIoClose(fd);

    if(size != (size/512)*512)
    {
        size = (size + 512) & 0xFFFFFE00;
    }
    if(size < 4096)
    {
        size = 4096;
    }
    fd = sceIoOpen("msstor:", PSP_O_RDWR, 0777);
    if(fd <= 0)
    {
        free(buffer);
        return -1;
    }
    unsigned char *mbr = (unsigned char*)malloc(512);
    if(!mbr)
    {
        free(buffer);
        sceIoClose(fd);
        return -1;
    }
    sceIoRead(fd, mbr, 512);
    unsigned int part = (mbr[454] | mbr[455] << 8 | mbr[456] << 16 | mbr[457] << 24);
    free(mbr);

    if(((part << 9) - 0x2000) < size)
    {
        free(buffer);
        sceIoClose(fd);
        return -1;
    }
    sceIoLseek(fd, 0x2000, PSP_SEEK_SET);
    sceIoWrite(fd, buffer, size);
    sceIoClose(fd);
    free(buffer);

    return 0;
}

int getHen(void)
{
    return sctrlHENGetVersion();
}

int isSe(void)
{
    return sctrlHENIsSE();
}

int isDevhook(void)
{
    return sctrlHENIsDevhook();
}

int module_start(SceSize args, void *argp)
{
    (void)args;
	(void)argp;
	return(0x0);
}

int module_stop(void)
{
	return(0x0);
}
