/** LPP Mp4 Lib by Nanni */

#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <psputility.h>
#include <pspgu.h>
#include <psppower.h>
#include <stdio.h>
#include <stdlib.h>
#include <psprtc.h>
#include <pspsdk.h>
#include <string.h>
#include <pspgu.h>

#include "Mp4.h"
#include "../Utils/Utils.h"

extern void sceMpegGetAvcNalAu(SceMpeg*,Mp4AvcNalStruct*,SceMpegAu*);
extern void sceMpegAvcDecodeDetail2(SceMpeg*,Mp4AvcDetail2Struct**);
extern int sceMpegBaseCscAvc(void*,unsigned,unsigned,Mp4AvcCscStruct*);
extern unsigned int *LPPG_GetFrameBuffer(void);

#ifdef DEBUG
extern int dwrite_output(const char*, ...);
#endif

extern u32 __attribute__((aligned(64))) dList[2][(262144) >> 2];

SceUID cbridgeid = 0;
SceUID mpegvshid = 0;

int devkitVersion = 0;

int LPP_Mp4Init(const char *mpvPath, const char *cbPath)
{
    devkitVersion = sceKernelDevkitVersion();

    if(sceUtilityLoadAvModule(0))
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : error in sceUtilityLoadAvModule.\n", __FUNCTION__, __LINE__);
		#endif
	    return -1;
	}
	if((mpegvshid = LPP_UtilsLoadStartModule(mpvPath)) <= 0)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot load the module '%s.\n'", __FUNCTION__, __LINE__, mpvPath);
		#endif
	    return -1;
	}
	if((cbridgeid = LPP_UtilsLoadStartModule(cbPath)) <= 0)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot load the module '%s'.\n", __FUNCTION__, __LINE__, cbPath);
		#endif
	    return -1;
	}
	if(sceMpegInit())
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : error in sceMpegInit.\n", __FUNCTION__, __LINE__);
		#endif
		return -1;
	}
	return 0;
}

LPP_Mp4* LPP_Mp4Load(const char *filename)
{
    LPP_Mp4 *tmp = (LPP_Mp4*)malloc(sizeof(LPP_Mp4));
	if(!tmp) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'tmp' to memory.\n", __FUNCTION__, __LINE__);
		#endif
		return 0;
	}
	tmp->avc = (Mp4AvcDecoderStruct*)malloc(sizeof(Mp4AvcDecoderStruct));
	if(!tmp->avc) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'tmp'->avc to memory.\n", __FUNCTION__, __LINE__);
		#endif
		free(tmp);
		return 0;
	}
	tmp->csc = (Mp4AvcCscStruct*)malloc(sizeof(Mp4AvcCscStruct));
	if(!tmp->csc) {
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'tmp' to memory.\n", __FUNCTION__, __LINE__);
		#endif
		free(tmp->avc);
		free(tmp);
		return 0;
	}
	mp4_read_safe_constructor(&(tmp->reader));
	memset(tmp->avc, 0, sizeof(Mp4AvcDecoderStruct));

	if(mp4_read_open(&(tmp->reader), (char*)filename))
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : mp4_read_open error.\n", __FUNCTION__, __LINE__);
		#endif
		free(tmp->csc);
		free(tmp->avc);
		free(tmp);
		return 0;
	}

	if(tmp->reader.file.info->tracks[tmp->reader.file.video_track_id]->avc_profile == 0x4D && (tmp->reader.file.video_width > 480 || tmp->reader.file.video_height > 272))
	{
	    cooleyesMeBootStart(devkitVersion, 1);
		tmp->avc->mpeg_mode = 5;
	} else if(tmp->reader.file.info->tracks[tmp->reader.file.video_track_id]->avc_profile == 0x4D)
	{
	    cooleyesMeBootStart(devkitVersion, 3);
		tmp->avc->mpeg_mode = 4;
	} else if(tmp->reader.file.info->tracks[tmp->reader.file.video_track_id]->avc_profile == 0x42)
	{
	    cooleyesMeBootStart(devkitVersion, 4);
		tmp->avc->mpeg_mode = 4;
	}

	tmp->avc->mpeg_ddrtop = memalign(0x400000, 0x400000);
	if(!tmp->avc->mpeg_ddrtop)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'tmp->avc->mpeg_ddrtop' to memory.\n", __FUNCTION__, __LINE__);
		#endif
		free(tmp->csc);
		free(tmp->avc);
		free(tmp);
		return 0;
	}
	tmp->avc->mpeg_au_buffer = tmp->avc->mpeg_ddrtop + 0x10000;

	int res = sceMpegQueryMemSize(tmp->avc->mpeg_mode);
	if(res < 0)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : res < 0.\n", __FUNCTION__, __LINE__);
		#endif
		free(tmp->csc);
		free(tmp->avc->mpeg_ddrtop);
		free(tmp->avc);
		free(tmp);
		return 0;
	}
	tmp->avc->mpeg_buffer_size = res;

	if(!(res & 0xF)) res = (res & 0xFFFFFFF0) + 16;

	tmp->avc->mpeg_buffer = malloc_64(res);
	if(!tmp->avc->mpeg_buffer)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'tmp->avc->mpeg_buffer' to memory.\n", __FUNCTION__, __LINE__);
		#endif
		free(tmp->csc);
		free(tmp->avc->mpeg_ddrtop);
		free(tmp->avc);
		free(tmp);
		return 0;
	}

	if(sceMpegCreate(&(tmp->avc->mpeg), tmp->avc->mpeg_buffer, tmp->avc->mpeg_buffer_size, &(tmp->avc->mpeg_ringbuffer), 512, tmp->avc->mpeg_mode, (SceInt32)tmp->avc->mpeg_ddrtop))
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Error in sceMpegCreate.\n", __FUNCTION__, __LINE__);
		#endif
		LPP_Mp4Close(tmp);
		return 0;
	}

	tmp->avc->mpeg_au = (SceMpegAu*)malloc_64(64);
	memset(tmp->avc->mpeg_au, 0xFF, 64);

	if(sceMpegInitAu(&(tmp->avc->mpeg), tmp->avc->mpeg_au_buffer, tmp->avc->mpeg_au))
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Error in sceMpegInitAu.\n", __FUNCTION__, __LINE__);
		#endif
		LPP_Mp4Close(tmp);
		return 0;
	}

	tmp->avc->mpeg_sps_size = tmp->reader.file.info->tracks[tmp->reader.file.video_track_id]->avc_sps_size;
	tmp->avc->mpeg_pps_size = tmp->reader.file.info->tracks[tmp->reader.file.video_track_id]->avc_pps_size;
	tmp->avc->mpeg_nal_prefix_size = tmp->reader.file.info->tracks[tmp->reader.file.video_track_id]->avc_nal_prefix_size;
	tmp->avc->mpeg_sps_pps_buffer = malloc_64(tmp->avc->mpeg_sps_size + tmp->avc->mpeg_pps_size);

	if(!tmp->avc->mpeg_sps_pps_buffer)
	{
	    #ifdef DEBUG
		dwrite_output("Function %s Line %d : Cannot allocate 'tmp->avc->mpeg_sps_pps_buffer' to memory.\n", __FUNCTION__, __LINE__);
		#endif
		LPP_Mp4Close(tmp);
		return 0;
	}

	memcpy(tmp->avc->mpeg_sps_pps_buffer, tmp->reader.file.info->tracks[tmp->reader.file.video_track_id]->avc_sps, tmp->avc->mpeg_sps_size);
	memcpy(tmp->avc->mpeg_sps_pps_buffer + tmp->avc->mpeg_sps_size, tmp->reader.file.info->tracks[tmp->reader.file.video_track_id]->avc_pps, tmp->avc->mpeg_pps_size);

	return (tmp);
}

int LPP_Mp4Shutdown(void)
{
    sceMpegFinish();
    if(cbridgeid) LPP_UtilsStopUnloadModule(cbridgeid);
    if(mpegvshid) LPP_UtilsStopUnloadModule(mpegvshid);
    sceUtilityUnloadAvModule(0);
    return 0;
}

void LPP_Mp4Close(LPP_Mp4 *vd)
{
    if(vd->avc->mpeg) sceMpegDelete(vd->avc->mpeg);
    if(vd->avc->mpeg_ddrtop) free(vd->avc->mpeg_ddrtop);
	if(vd->avc->mpeg_buffer) free(vd->avc->mpeg_buffer);
	if(vd->avc->mpeg_sps_pps_buffer) free(vd->avc->mpeg_sps_pps_buffer);
    if(vd->avc) free(vd->avc);
	if(vd->csc) free(vd->csc);
	mp4_read_close(&(vd->reader));
	if(vd) free(vd);
}

int LPP_Mp4Play(LPP_Mp4 *vd, unsigned int frame_count) {
    if(frame_count > vd->reader.file.number_of_video_frames) return -1;

    Mp4AvcDecoderStruct *avc = vd->avc;

    vd->nal.sps_buffer = avc->mpeg_sps_pps_buffer;
	vd->nal.sps_size        = avc->mpeg_sps_size;
	vd->nal.pps_buffer      = avc->mpeg_sps_pps_buffer+avc->mpeg_sps_size;
	vd->nal.pps_size        = avc->mpeg_pps_size;
	vd->nal.nal_prefix_size = avc->mpeg_nal_prefix_size;

	if (mp4_read_get_video(&(vd->reader), frame_count, &(vd->v_packet)))
	{
	    return (-1);
	}

	vd->nal.nal_buffer = vd->v_packet.video_buffer;
	vd->nal.nal_size = vd->v_packet.video_length;

	frame_count ? (vd->nal.mode=0) : (vd->nal.mode=3);

	sceMpegGetAvcNalAu(&avc->mpeg, &vd->nal, avc->mpeg_au);
	sceMpegAvcDecode(&avc->mpeg, avc->mpeg_au, 512, 0, &avc->mpeg_pic_num);
	sceMpegAvcDecodeDetail2(&avc->mpeg, &avc->mpeg_detail2);

	if (avc->mpeg_pic_num > 0) {
	    int i;
		for(i = 0; i < avc->mpeg_pic_num; i++) {
		    Mp4AvcCscStruct csc;

			csc.height = (avc->mpeg_detail2->info_buffer->height+15) >> 4;
			csc.width = (avc->mpeg_detail2->info_buffer->width+15) >> 4;
			csc.mode0 = 0;
			csc.mode1 = 0;
			csc.buffer0 = avc->mpeg_detail2->yuv_buffer->buffer0;
			csc.buffer1 = avc->mpeg_detail2->yuv_buffer->buffer1;
			csc.buffer2 = avc->mpeg_detail2->yuv_buffer->buffer2;
			csc.buffer3 = avc->mpeg_detail2->yuv_buffer->buffer3;
			csc.buffer4 = avc->mpeg_detail2->yuv_buffer->buffer4;
			csc.buffer5 = avc->mpeg_detail2->yuv_buffer->buffer5;
			csc.buffer6 = avc->mpeg_detail2->yuv_buffer->buffer6;
			csc.buffer7 = avc->mpeg_detail2->yuv_buffer->buffer7;
			if(!sceMpegBaseCscAvc(LPPG_GetFrameBuffer(),0,(avc->mpeg_mode == 4) ? 512 : 768, &csc));
		}
	}

	return (frame_count + 1);
}

