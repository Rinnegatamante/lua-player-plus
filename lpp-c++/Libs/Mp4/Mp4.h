/** LPP Mp4 Lib by Nanni */

#ifndef __MP4_L_H_
#define __MP4_L_H_

#ifdef __cplusplus
    extern "C" {
#endif

#include "../cooleyesBridge/cooleyesBridge.h"
#include "../mem64/mem64.h"
#include "pspmpeg.h"
#include "mp4_read.h"

typedef struct {
	ScePVoid sps_buffer;
	SceInt32 sps_size;
	ScePVoid pps_buffer;
	SceInt32 pps_size;
	SceInt32 nal_prefix_size;
	ScePVoid nal_buffer;
	SceInt32 nal_size;
	SceInt32 mode;
} Mp4AvcNalStruct;

typedef struct {
	SceInt32 unknown0;
	SceInt32 unknown1;
	SceInt32 width;
	SceInt32 height;
	SceInt32 unknown4;
	SceInt32 unknown5;
	SceInt32 unknown6;
	SceInt32 unknown7;
	SceInt32 unknown8;
	SceInt32 unknown9;
} Mp4AvcInfoStruct;

typedef struct {
	ScePVoid buffer0;
	ScePVoid buffer1;
	ScePVoid buffer2;
	ScePVoid buffer3;
	ScePVoid buffer4;
	ScePVoid buffer5;
	ScePVoid buffer6;
	ScePVoid buffer7;
	SceInt32 unknown0;
	SceInt32 unknown1;
	SceInt32 unknown2;
} Mp4AvcYuvStruct;

typedef struct {
	SceInt32 unknown0;
	SceInt32 unknown1;
	SceInt32 unknown2;
	SceInt32 unknown3;
	Mp4AvcInfoStruct* info_buffer;
	SceInt32 unknown5;
	SceInt32 unknown6;
	SceInt32 unknown7;
	SceInt32 unknown8;
	SceInt32 unknown9;
	SceInt32 unknown10;
	Mp4AvcYuvStruct* yuv_buffer;
	SceInt32 unknown12;
	SceInt32 unknown13;
	SceInt32 unknown14;
	SceInt32 unknown15;
	SceInt32 unknown16;
	SceInt32 unknown17;
	SceInt32 unknown18;
	SceInt32 unknown19;
	SceInt32 unknown20;
	SceInt32 unknown21;
	SceInt32 unknown22;
	SceInt32 unknown23;
} Mp4AvcDetail2Struct;

typedef struct {
	SceInt32 height;
	SceInt32 width;
	SceInt32 mode0;
	SceInt32 mode1;
	ScePVoid buffer0;
	ScePVoid buffer1;
	ScePVoid buffer2;
	ScePVoid buffer3;
	ScePVoid buffer4;
	ScePVoid buffer5;
	ScePVoid buffer6;
	ScePVoid buffer7;
} Mp4AvcCscStruct;

typedef struct {
	int      mpeg_init;
	int      mpeg_create;
	ScePVoid mpeg_buffer;
	SceMpeg mpeg;
	SceMpegRingbuffer mpeg_ringbuffer;
	SceMpegAu* mpeg_au;
	SceInt32 mpeg_mode;
	SceInt32 mpeg_buffer_size;
	ScePVoid mpeg_ddrtop;
	ScePVoid mpeg_au_buffer;
	ScePVoid mpeg_sps_pps_buffer;
	SceInt32 mpeg_sps_size;
	SceInt32 mpeg_pps_size;
	SceInt32 mpeg_nal_prefix_size;
	Mp4AvcDetail2Struct* mpeg_detail2;
	SceInt32 mpeg_pic_num;
} Mp4AvcDecoderStruct;

typedef struct {
    struct mp4_read_struct reader;
    Mp4AvcNalStruct nal;
	struct mp4_video_read_output_struct v_packet;
	Mp4AvcDecoderStruct *avc;
	Mp4AvcCscStruct *csc;
} LPP_Mp4;

int LPP_Mp4Init(const char *mpvPath, const char *cbPath);

int LPP_Mp4Shutdown(void);

LPP_Mp4* LPP_Mp4Load(const char *filename);

void LPP_Mp4Close(LPP_Mp4 *vd);

int LPP_Mp4Play(LPP_Mp4 *vd, unsigned int frame_count);

#ifdef __cplusplus
    }
#endif

#endif /* __MP4_L_H_ */
