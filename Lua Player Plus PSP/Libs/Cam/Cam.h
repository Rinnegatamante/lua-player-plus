/** LPP Cam lib by Nanni */

#ifndef __LPPCAM_LIB_H_
#define __LPPCAM_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../Types.h"

#define LPP_CAM_MAX_VIDEO_FRAME_SIZE (32768)
#define LPP_CAM_MAX_STILL_IMAGE_SIZE (524288)

#ifdef __cplusplus
}
#endif

int LPP_CamLoadModules(void);

int LPP_CamUnLoadModules(void);

int LPP_CamStartUsb(void);

int LPP_CamStopUsb(void);

int LPP_CamInitVideo(int resolution);

int LPP_CamInitJpegDecoder(void);

int LPP_CamShutdownDecoder(void);

int LPP_CamRenderScreen(void);

int LPP_CamTakePhotoImpl(const char *filepath, int compLevel, int resolution);

int LPP_CamTakePhoto(const char *filepath, int compLevel, int resolution);

int LPP_CamStopAll(void);

int LPP_CamStartAll(void);

#endif /* __LPPCAM_LIB_H_ */
