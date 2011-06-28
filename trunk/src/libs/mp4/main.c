#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <pspgu.h>
#include <pspdebug.h>
#include <psppower.h>
#include <stdio.h>
#include <stdlib.h>
#include <psprtc.h>
#include <pspsdk.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <psputility_avmodules.h>
#include "mem64.h"
#include "../../include/kubridge.h"
#include <pspmpeg.h>
#include <mp4ff.h>

SceCtrlData input;

typedef struct {
   ScePVoid sps_buffer;
   SceInt32 sps_size;
   ScePVoid pps_buffer;
   SceInt32 pps_size;
   SceInt32 unkown0;
   ScePVoid nal_buffer;
   SceInt32 nal_size;
   SceInt32 mode;
} AvcNalStruct;

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
} AvcInfoStruct;

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
} AvcYuvStruct;


typedef struct {
   SceInt32 unknown0;
   SceInt32 unknown1;
   SceInt32 unknown2;
   SceInt32 unknown3;
   AvcInfoStruct* info_buffer;
   SceInt32 unknown5;
   SceInt32 unknown6;
   SceInt32 unknown7;
   SceInt32 unknown8;
   SceInt32 unknown9;
   SceInt32 unknown10;
   AvcYuvStruct* yuv_buffer;
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
} AvcCodecStruct;

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
} AvcCscStruct;

typedef struct {
   ScePVoid mpeg_buffer;
   SceMpeg mpeg;
   SceMpegRingbuffer mpeg_ringbuffer;
   SceMpegAu* mpeg_au;
   SceInt32 mpeg_mode;
   SceInt32 mpeg_buffer_size;
   ScePVoid mpeg_ddrtop;
   ScePVoid mpeg_au_buffer;
   AvcNalStruct mpeg_nal;
   AvcCodecStruct* mpeg_codec_buffer;
   AvcYuvStruct* mpeg_yuv_buffer;
   AvcInfoStruct* mpeg_info_buffer;
} AvcDecodeStruct;

static uint32_t mp4_read(void *user_data, void *buffer, uint32_t length) {
   FILE** fp = (FILE**)user_data;
   uint32_t res = fread(buffer, length, 1, *fp);
   return (res*length);
}

static uint32_t mp4_seek(void *user_data, uint64_t position) {
   FILE** fp = (FILE**)user_data;
   return fseek(*fp, position, PSP_SEEK_SET );
}

AvcDecodeStruct avc_struct;
AvcDecodeStruct* avc = &avc_struct;
AvcCscStruct csc_struct;
AvcCscStruct* csc = &csc_struct;
//unsigned char RGBBuffer[4*512*512];
//unsigned char RGBBuffer0[4*512*272];
//unsigned char RGBBuffer1[4*512*272];
/* ENHANCEMENT */
//the framebuffer need to point to addresses in VRAM
unsigned char* FrameBuffer[] = {0x44000000, (0x44000000 | 0x88000)};
int frame_index = 0;
char filename[1024];

mp4ff_callback_t mp4_callback = {mp4_read, 0, mp4_seek, 0, 0};

int PlayMp4(const char* filenamevideomp4,int debugmode)
{
   pspDebugScreenInit();
   pspDebugScreenSetXY(0, 2);
   //scePowerSetClockFrequency(333,333,166);
   //scePowerSetCpuClockFrequency(333);
   //scePowerSetBusClockFrequency(166);
   u32 cpu = scePowerGetCpuClockFrequency();
   u32 bus = scePowerGetBusClockFrequency();
   if (debugmode == 1){
   pspDebugScreenPrintf("cpu=%d, bus=%d\n", cpu, bus);
   }
   strcat(filename,"flash0:/kd/mpeg_vsh.prx");
   if (debugmode == 1){
   pspDebugScreenPrintf("%s\n", filename);
   }
   FILE* mp4_file = fopen(filenamevideomp4, "rb");
   mp4_callback.user_data = &mp4_file;
   mp4ff_t* mp4_handle = mp4ff_open_read(&mp4_callback);
   u32 total_tracks = mp4ff_total_tracks(mp4_handle);
   if (debugmode == 1){
   pspDebugScreenPrintf("total_tracks=%d\n", total_tracks);
   }
   int ii;
   for(ii = 0; ii < total_tracks; ii++) {
   if (debugmode == 1){
      pspDebugScreenPrintf("track%d : type 0x%08X\n", ii, mp4ff_get_track_type(mp4_handle, ii));
      pspDebugScreenPrintf("track%d : %d samples\n", ii, mp4ff_num_samples(mp4_handle, ii));
   }
   }
   u32 total_samples = mp4ff_num_samples(mp4_handle, 0);
   unsigned char* sps_pps_buffer;
   unsigned int sps_size, pps_size;

   if ( mp4ff_get_avc_sps_pps(mp4_handle, 0, &sps_pps_buffer, &sps_size, &pps_size) != 0 || sps_size == 0 || pps_size == 0 )  {
      if (debugmode == 1){
      pspDebugScreenPrintf("\nerr: get_avc_sps_pps\n");
      }
      goto wait;
   }

   //pspDebugScreenPrintf("sps_size=%d, pps_size=%d\n", sps_size, pps_size);

   int result;
   //result = sceUtilityLoadAvModule(0);
   //if ( result < 0 ) {
   //   pspDebugScreenPrintf("\nerr: sceUtilityLoadAvModule(0)\n");
   //   goto wait;
   //}
   SceUID modid;
   int status;

   modid = kuKernelLoadModule(filename, 0, NULL);
   if(modid >= 0) {
      modid = sceKernelStartModule(modid, 0, 0, &status, NULL);
   }
   else {
    if (debugmode == 1){
      pspDebugScreenPrintf("\nerr=0x%08X : sceKernelLoadModule\n", modid);
    }
      goto wait;
   }

//   result = pspSdkLoadStartModule("ms0:/mpeg_vsh330.prx", PSP_MEMORY_PARTITION_USER);
//   result = sceUtilityLoadAvModule(3);
//   if ( result < 0 ){
//      pspDebugScreenPrintf("\nerr=0x%08X : sceUtilityLoadAvModule(3)\n", result);
//      goto wait;
//   }

   result = sceMpegInit();
   if ( result != 0 ){
   if (debugmode == 1){
      pspDebugScreenPrintf("\nerr: sceMpegInit\n");
   }
      goto wait;
   }

   avc->mpeg_mode = 4;
//   avc->mpeg_ddrtop = 0x09400000;
//   avc->mpeg_au_buffer = 0x09410000;
   avc->mpeg_ddrtop =  memalign(0x400000, 0x400000);
   avc->mpeg_au_buffer = avc->mpeg_ddrtop + 0x10000;

//   pspDebugScreenPrintf("\naddress=0x%08X\n", avc->mpeg_au_buffer);


   result = sceMpegQueryMemSize(avc->mpeg_mode);
   if ( result < 0 ){
   if (debugmode == 1){
      pspDebugScreenPrintf("\nerr: sceMpegQueryMemSize(0x%08X)\n", avc->mpeg_mode);
      }
      goto wait;
   }

//   pspDebugScreenPrintf("\n%d\n", result);

   avc->mpeg_buffer_size = result;

   if ( (result & 0xF) != 0 )
      result = (result & 0xFFFFFFF0) + 16;

   avc->mpeg_buffer = malloc_64(result);
   if ( avc->mpeg_buffer == 0 ) {
   if (debugmode == 1){
      pspDebugScreenPrintf("\nerr: alloc\n");
      }
      goto wait;
   }

   result = sceMpegCreate(&avc->mpeg, avc->mpeg_buffer, avc->mpeg_buffer_size, &avc->mpeg_ringbuffer, 512, avc->mpeg_mode, avc->mpeg_ddrtop);
   if ( result != 0){
   if (debugmode == 1){
      pspDebugScreenPrintf("\nerr: sceMpegCreate\n");
      }
      goto wait;
   }

   avc->mpeg_au = (SceMpegAu*)malloc_64(64);
   if ( avc->mpeg_au == 0 ) {
   if (debugmode == 1){
      pspDebugScreenPrintf("\nerr: alloc\n");
      }
      goto wait;
   }
   memset(avc->mpeg_au, 0xFF, 64);
   if ( sceMpegInitAu(&avc->mpeg, avc->mpeg_au_buffer, avc->mpeg_au) != 0 ){
   if (debugmode == 1){
      pspDebugScreenPrintf("\nerr: sceMpegInitAu\n");
      }
      goto wait;
   }

   unsigned char* nal_buffer = (unsigned char*)malloc_64(1024*1024);

//   unsigned char sps_pps_buffer[27];
//
//   FILE* fp;
//   fp = fopen("ms0:/sps_pps.dat", "rb");
//   fread(sps_pps_buffer, 27, 1, fp);
//   fclose(fp);
//   sps_size = 23;
//   pps_size = 4;

   //---------------------------------------------------------------------------------//
   float curr_ms = 1.0f;
   u64 last_tick;
   sceRtcGetCurrentTick(&last_tick);
   u32 tick_frequency = sceRtcGetTickResolution();
   int frame_count = 0;
   sceCtrlReadBufferPositive(&input, 1);

   int pic_num;

   sceDisplayWaitVblankStart();
   sceDisplaySetFrameBuf(FrameBuffer[frame_index], 512, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_IMMEDIATE);
while(!(input.Buttons & PSP_CTRL_TRIANGLE)) {
   float curr_fps = 1.0f / curr_ms;
   avc->mpeg_nal.sps_buffer = (&sps_pps_buffer[0]);
   avc->mpeg_nal.sps_size = sps_size;
   avc->mpeg_nal.pps_buffer = (&sps_pps_buffer[sps_size]);
   avc->mpeg_nal.pps_size = pps_size;
   avc->mpeg_nal.unkown0 = 4;
   memset(nal_buffer, 0, 1024*1024);
   mp4ff_read_sample_v2(mp4_handle, 0, frame_count, nal_buffer);
   avc->mpeg_nal.nal_buffer = nal_buffer;
   avc->mpeg_nal.nal_size = mp4ff_read_sample_getsize(mp4_handle, 0, frame_count);//size1 ;
   if ( frame_count == 0 )
      avc->mpeg_nal.mode = 3;
   else
      avc->mpeg_nal.mode = 0;
   FILE* fp1;
//   memset(filename,0,1024);
//   sprintf(filename, "ms0:/mpeg%d_0.dat", frame_count);
//   fp1 = fopen(filename, "wb");
//   fwrite(p1, 512, 1, fp1);
//   fclose(fp1);

   result = sceMpegGetAvcNalAu(&avc->mpeg, &avc->mpeg_nal, avc->mpeg_au);
   //result = sceMpegGetAvcAu(&avc->mpeg, &avc->mpeg_nal.nal_buffer, avc->mpeg_au, 0);
   memset(filename,0,1024);
//   sprintf(filename, "ms0:/au%d_1.dat", frame_count);
//   fp1 = fopen(filename, "wb");
//   fwrite(au0, 64, 1, fp1);
//   fclose(fp1);
//   pspDebugScreenPrintf(" GetAvcNalAu=0x%08X\n", result);

   result = sceMpegAvcDecode(&avc->mpeg, avc->mpeg_au, 512, 0, &pic_num);
//   pspDebugScreenPrintf(" AvcDecode=0x%08X,0x%08X\n", result, pic_num);
   result = sceMpegAvcDecodeDetail2(&avc->mpeg, &avc->mpeg_codec_buffer);
//   pspDebugScreenPrintf(" AvcDecodeDetail2=0x%08X\n", result);


   if ( result == 0 ) {
      avc->mpeg_yuv_buffer = avc->mpeg_codec_buffer->yuv_buffer;
      avc->mpeg_info_buffer = avc->mpeg_codec_buffer->info_buffer;

      if ( pic_num > 0 ) {
         int i;
         for(i=0;i<pic_num;i++) {
            int csc_mode = 0;//i % 2;
            csc->height = avc->mpeg_info_buffer->height >> 4;
            csc->width = avc->mpeg_info_buffer->width >> 4;
            csc->mode0 = csc_mode;
            csc->mode1 = csc_mode;
            if ( csc_mode == 0 ) {
               csc->buffer0 = avc->mpeg_yuv_buffer->buffer0 ;
               csc->buffer1 = avc->mpeg_yuv_buffer->buffer1 ;
               csc->buffer2 = avc->mpeg_yuv_buffer->buffer2 ;
               csc->buffer3 = avc->mpeg_yuv_buffer->buffer3 ;
               csc->buffer4 = avc->mpeg_yuv_buffer->buffer4 ;
               csc->buffer5 = avc->mpeg_yuv_buffer->buffer5 ;
               csc->buffer6 = avc->mpeg_yuv_buffer->buffer6 ;
               csc->buffer7 = avc->mpeg_yuv_buffer->buffer7 ;
            }
            else {
               csc->buffer0 = avc->mpeg_yuv_buffer->buffer2 ;
               csc->buffer1 = avc->mpeg_yuv_buffer->buffer3 ;
               csc->buffer2 = avc->mpeg_yuv_buffer->buffer0 ;
               csc->buffer3 = avc->mpeg_yuv_buffer->buffer1 ;
               csc->buffer4 = avc->mpeg_yuv_buffer->buffer6 ;
               csc->buffer5 = avc->mpeg_yuv_buffer->buffer7 ;
               csc->buffer6 = avc->mpeg_yuv_buffer->buffer4 ;
               csc->buffer7 = avc->mpeg_yuv_buffer->buffer5 ;
            }

            result = sceMpegBaseCscAvc(FrameBuffer[frame_index],0,512,csc);
//            pspDebugScreenPrintf(" BaseCscAvc=0x%08X\n", result);
            if ( result == 0 ) {
//               memset(filename,0,1024);
//               sprintf(filename, "ms0:/RGB%d.%d.dat", frame_count,i);
//               fp1 = fopen(filename, "wb");
//               fwrite(RGBBuffer, 4*512*512, 1, fp1);
//               fclose(fp1);
               sceDisplayWaitVblankStart();
               int resFb = sceDisplaySetFrameBuf(FrameBuffer[frame_index], 512, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_IMMEDIATE);
               frame_index = (frame_index+1) % 2;
               sceKernelDelayThread(10000);
            }
         }
      }
   }
//   memset(filename,0,1024);
//   sprintf(filename, "ms0:/mpeg%d.dat", frame_count);
//   fp1 = fopen(filename, "wb");
//   fwrite(p1, 512, 1, fp1);
//   fclose(fp1);
   ++frame_count;
   if ( frame_count >= total_samples ){
   sceDisplayWaitVblankStart();
    return 1;
    }
   u64 curr_tick;
   sceRtcGetCurrentTick(&curr_tick);
   if ((curr_tick-last_tick) >= tick_frequency)
   {
      float time_span = ((int)(curr_tick-last_tick)) / (float)tick_frequency;
      curr_ms = time_span / frame_count;
      //frame_count = 0;
      sceRtcGetCurrentTick(&last_tick);
   }
   sceCtrlReadBufferPositive(&input, 1);
}
//   fclose(fp);

wait:
   mp4ff_close(mp4_handle);
   fclose(mp4_file);
   if (debugmode == 0){
   pspDebugScreenPrintf("An error was occurred!\n");
   pspDebugScreenPrintf("Please, use Debug Mode to see Errorcode.\n");
   }else{
   pspDebugScreenPrintf("\n");
   }
   pspDebugScreenPrintf("Press triangle to exit...\n");
   sceCtrlReadBufferPositive(&input, 1);
   while(!(input.Buttons & PSP_CTRL_TRIANGLE))
   {
      sceKernelDelayThread(10000);   // wait 10 milliseconds
      sceCtrlReadBufferPositive(&input, 1);
   }
   sceKernelStopModule(modid, 0, NULL, &status, NULL);
   sceKernelUnloadModule(modid);
   return 1;
}

