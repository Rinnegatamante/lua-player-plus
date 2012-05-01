/** LPP Cam lib by Nanni */

#include "Cam.h"
#include <pspsdk.h>
#include <pspuser.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <psputility_usbmodules.h>
#include <psputility_avmodules.h>
#include <pspusb.h>
#include <pspusbacc.h>
#include <pspusbcam.h>
#include <pspjpeg.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Graphics/Graphics.h"

#ifdef DEBUG
extern int dwrite_output(const char*, ...);
#endif

static u8 buffer[LPP_CAM_MAX_STILL_IMAGE_SIZE] __attribute__((aligned(64)));
static u8 work[69632] __attribute__((aligned(64)));
static u32 *framebuffer[PSP_SCREEN_WIDTH * PSP_SCREEN_HEIGHT] __attribute__((aligned(64)));

static SceUID waitphoto;

int LPP_CamLoadModules(void)
{
    int ret = sceUtilityLoadUsbModule(PSP_USB_MODULE_ACC);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUtilityLoadUsbModule.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceUtilityLoadUsbModule(PSP_USB_MODULE_CAM);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUtilityLoadUsbModule.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
    if(ret < 0 && ret != 0 && ret != 0x80110f02)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUtilityLoadAvModule.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    return 0;
}

int LPP_CamUnLoadModules(void)
{
    int ret = sceUtilityUnloadUsbModule(PSP_USB_MODULE_CAM);
     if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUtilityUnloadUsbModule.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceUtilityUnloadUsbModule(PSP_USB_MODULE_ACC);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUtilityUnloadUsbModule.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceUtilityUnloadAvModule(PSP_AV_MODULE_AVCODEC);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUtilityUnloadAvModule.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    return 0;
}

int LPP_CamStartUsb(void)
{
    int ret = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbStart.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceUsbStart(PSP_USBACC_DRIVERNAME, 0, 0);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbStart.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceUsbStart(PSP_USBCAM_DRIVERNAME, 0, 0);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbStart.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceUsbStart(PSP_USBCAMMIC_DRIVERNAME, 0, 0);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbStart.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    return 0;
}

int LPP_CamStopUsb(void)
{
    int ret = sceUsbStop(PSP_USBCAMMIC_DRIVERNAME, 0, 0);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbStop.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceUsbStop(PSP_USBCAM_DRIVERNAME, 0, 0);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbStop.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceUsbStop(PSP_USBACC_DRIVERNAME, 0, 0);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbStop.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbStop.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    return 0;
}

int LPP_CamInitJpegDecoder(void)
{
    int ret = sceJpegInitMJpeg();
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceJpegInitMJpeg.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceJpegCreateMJpeg(480, 272);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceJpegCreateMJpeg.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    return 0;
}

int LPP_CamShutdownDecoder(void)
{
    int ret = sceJpegDeleteMJpeg();
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceJpegDeleteMJpeg.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceJpegFinishMJpeg();
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceJpegFinishMJpeg.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    return 0;
}

int LPP_CamTakePhotoImpl(const char *filepath, int compLevel, int resolution)
{
    PspUsbCamSetupStillParam param;

    int ret;
    memset(&param, 0, sizeof(param));

    param.size = sizeof(param);
    param.resolution = resolution;
    param.jpegsize = LPP_CAM_MAX_STILL_IMAGE_SIZE;
    param.delay = PSP_USBCAM_NODELAY;
    param.complevel = compLevel;

    ret = sceUsbCamSetupStill(&param);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbCamSetupStill.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    ret = sceUsbCamStillInputBlocking(buffer, LPP_CAM_MAX_STILL_IMAGE_SIZE);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbCamStillInputBlocking.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    FILE *fp = fopen(filepath, "wb");
    if(!fp)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Cannot open the file '%s'.\n", __FUNCTION__, __LINE__, filepath);
        #endif
        return -1;
    }

    fwrite(buffer, ret, 1, fp);
    fclose(fp);

    return 0;
}

int LPP_CamTakePhoto(const char *filepath, int compLevel, int resolution)
{
    int ret = sceUsbCamStopVideo();
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbCamStopVideo.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = LPP_CamTakePhotoImpl(filepath, compLevel, resolution);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in LPP_CamTakePhotoImpl.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceUsbCamStartVideo();
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbCamStartVideo.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    sceKernelSignalSema(waitphoto, 1);
    return 0;
}

int LPP_CamInitVideo(int resolution)
{
     int ret;
     PspUsbCamSetupVideoParam param;

    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.resolution = resolution;
    param.framerate = PSP_USBCAM_FRAMERATE_15_FPS;
    param.wb = PSP_USBCAM_WB_AUTO;
    param.saturation = 125;
    param.brightness = 128;
    param.contrast = 64;
    param.sharpness = 0;
    param.effectmode = PSP_USBCAM_EFFECTMODE_NORMAL;
    param.framesize = LPP_CAM_MAX_VIDEO_FRAME_SIZE;
    param.evlevel = PSP_USBCAM_EVLEVEL_0_0;

    ret = sceUsbCamSetupVideo(&param, work, sizeof(work));
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbCamSetupVideo.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    sceUsbCamAutoImageReverseSW(1);
    ret = sceUsbCamStartVideo();
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbCamStartVideo.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    return 0;
}

int LPP_CamRenderScreen(void)
{
    int i, j, m, n, ret;
    ret = sceUsbCamReadVideoFrameBlocking(buffer, LPP_CAM_MAX_VIDEO_FRAME_SIZE);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbCamReadVideoFrameBlocking.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    ret = sceJpegDecodeMJpeg(buffer, ret, framebuffer, 0);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceJpegDecodeMJpeg.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    for(i = 0; i < PSP_SCREEN_HEIGHT; i++)
    {
        m = i * PSP_SCREEN_WIDTH;
        n = i * PSP_LINE_SIZE;
        for(j = 0; j < PSP_SCREEN_WIDTH; j++)
        {
            u32 *vram = LPPG_GetFrameBuffer();
            vram[n + j] = (u32)framebuffer[m + j];
        }
    }
    return 0;
}

int LPP_CamStopAll(void)
{
    int ret = sceUsbDeactivate(PSP_USBCAM_PID);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbDeactivate.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = LPP_CamStopUsb();
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in LPP_CamStopUsb.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = LPP_CamShutdownDecoder();
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in LPP_CamShutdownDecoder.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = LPP_CamUnLoadModules();
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in LPP_CamUnLoadModules.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    return 0;
}

int LPP_CamStartAll(void)
{
    int ret = LPP_CamLoadModules();
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in LPP_CamLoadModules.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = LPP_CamStartUsb();
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in LPP_CamStartUsb.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = sceUsbActivate(PSP_USBCAM_PID);
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceUsbActivate.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }
    ret = LPP_CamInitJpegDecoder();
    if(ret < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in LPP_CamInitJpegDecoder.\n", __FUNCTION__, __LINE__, ret);
        #endif
        return ret;
    }

    while((sceUsbGetState() & 0xF) != PSP_USB_CONNECTION_ESTABLISHED);

    waitphoto = sceKernelCreateSema("lppCamWaitPhotoSema", 0, 0, 1, NULL);
    if(waitphoto < 0)
    {
        #ifdef DEBUG
        dwrite_output("Function %s Line %d : Error 0x%08X in sceKernelCreateSema.\n", __FUNCTION__, __LINE__, waitphoto);
        #endif
        return waitphoto;
    }

    return 0;
}
