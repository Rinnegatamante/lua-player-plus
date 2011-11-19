/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspaudiolib.c - Audio library build on top of sceAudio, but to provide
 *                 multiple thread usage and callbacks.
 *
 * Copyright (c) 2005 Adresd
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 *
 * $Id: pspaudiolib.c 1145 2005-10-12 15:32:44Z mrbrown $
 */
#include <stdlib.h>
#include <string.h>
#include <pspthreadman.h>
#include <pspaudio.h>

#include "pspaudiolib.h"

int sceAudioSRCChRelease(void);
int sceAudioSRCOutputBlocking(int volume, void *buffer);
int sceAudioOutput2GetRestSample();

int setFrequency(unsigned short samples, unsigned short freq, char car){
	return sceAudioSRCChReserve(samples, freq, car);
}

int pspReleaseAudio(void){
	while(sceAudioOutput2GetRestSample() > 0);
	return sceAudioSRCChRelease();
}


int audioOutpuBlocking(int volume, void *buffer){
	return sceAudioSRCOutputBlocking(volume, buffer);
}

static int audio_ready=0;
static short audio_sndbuf[PSP_NUM_AUDIO_CHANNELS][2][PSP_NUM_AUDIO_SAMPLES][2];

static psp_audio_channelinfo AudioStatus[PSP_NUM_AUDIO_CHANNELS];

static volatile int audio_terminate=0;

void pspAudioSetVolume(int channel, int left, int right)
{
    if(channel >= PSP_NUM_AUDIO_CHANNELS) return;
    AudioStatus[channel].volumeright = right;
    AudioStatus[channel].volumeleft  = left;
}

void pspAudioChannelThreadCallback(int channel, void *buf, unsigned int reqn)
{
    if(channel >= PSP_NUM_AUDIO_CHANNELS)
        return;
    pspAudioCallback_t callback;
    callback=AudioStatus[channel].callback;
}


void pspAudioSetChannelCallback(int channel, pspAudioCallback_t callback, void *pdata)
{
    if(channel >= PSP_NUM_AUDIO_CHANNELS)
        return;
    volatile psp_audio_channelinfo *pci = &AudioStatus[channel];
    pci->callback=0;
    pci->pdata=pdata;
    pci->callback=callback;
}

int pspAudioOutBlocking(unsigned int channel, unsigned int vol1, unsigned int vol2, void *buf)
{
    if (!audio_ready) return -1;
    if (channel>=PSP_NUM_AUDIO_CHANNELS) return -1;
    if (vol1>PSP_VOLUME_MAX) vol1=PSP_VOLUME_MAX;
    if (vol2>PSP_VOLUME_MAX) vol2=PSP_VOLUME_MAX;
    return audioOutpuBlocking(vol1,buf);
}

static SceUID play_sema;

static int AudioChannelThread(int args, void *argp)
{
    volatile int bufidx=0;
    int channel=*(int *)argp;

    AudioStatus[channel].threadactive = 1;
    while (audio_terminate==0) {
        void *bufptr=&audio_sndbuf[channel][bufidx];
        pspAudioCallback_t callback;
        callback=AudioStatus[channel].callback;
        if (callback) {
            callback(bufptr, PSP_NUM_AUDIO_SAMPLES, AudioStatus[channel].pdata);
        } else {
            unsigned int *ptr=bufptr;
            int i;
            for (i=0; i<PSP_NUM_AUDIO_SAMPLES; ++i) *(ptr++)=0;
        }
        //pspAudioOutBlocking(channel,AudioStatus[channel].volumeleft,AudioStatus[channel].volumeright,bufptr);
        sceKernelWaitSema(play_sema, 1, 0);
        audioOutpuBlocking(AudioStatus[0].volumeright,bufptr);
        sceKernelSignalSema(play_sema, 1);
        bufidx=(bufidx?0:1);
    }
    AudioStatus[channel].threadactive = 0;
    sceKernelExitThread(0);
    return 0;
}



/******************************************************************************/

int pspAudioSetFrequency(unsigned short freq)
{
   int ret = 0;
   switch(freq) {
      case 8000:
      case 12000:
      case 16000:
      case 24000:
      case 32000:
      case 48000:
      case 11025:
      case 22050:
      case 44100:
         break;
      default:
         return -1;
   }
   sceKernelWaitSema(play_sema, 1, 0);
   pspReleaseAudio();
   if(setFrequency(PSP_NUM_AUDIO_SAMPLES,freq,2)<0) ret = -1;
   sceKernelSignalSema(play_sema, 1);
   return ret;
}


int pspAudioInit()
{
    int i,ret;
    int failed=0;
    char str[32];

    audio_terminate=0;
    audio_ready=0;

    play_sema = sceKernelCreateSema("play_sema", 6, 1, 1, 0);
    for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
        AudioStatus[i].handle = -1;
        AudioStatus[i].threadhandle = -1;
        AudioStatus[i].threadactive = 0;
        AudioStatus[i].volumeright = PSP_VOLUME_MAX;
        AudioStatus[i].volumeleft  = PSP_VOLUME_MAX;
        AudioStatus[i].callback = 0;
        AudioStatus[i].pdata = 0;
    }
    for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
      if(pspAudioSetFrequency(44100)<0)
        failed=1;
      else
        AudioStatus[i].handle = 0;
    }
    if (failed) {
        for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
            if (AudioStatus[i].handle != -1)
                pspReleaseAudio();
            AudioStatus[i].handle = -1;
        }
        return -1;
    }
    audio_ready = 1;
    strcpy(str,"audiot0");
    for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
        str[6]='0'+i;
        //AudioStatus[i].threadhandle = sceKernelCreateThread(str,(void*)&AudioChannelThread,0x12,0x10000,0,NULL);
        //sceAudioSetChannelDataLen(i, PSP_NUM_AUDIO_SAMPLES);
        AudioStatus[i].threadhandle = sceKernelCreateThread(str,(void*)&AudioChannelThread,0x12,0x10000,PSP_THREAD_ATTR_USER,NULL);
        if (AudioStatus[i].threadhandle < 0) {
            AudioStatus[i].threadhandle = -1;
            failed=1;
            break;
        }
        ret=sceKernelStartThread(AudioStatus[i].threadhandle,sizeof(i),&i);
        if (ret!=0) {
            failed=1;
            break;
        }
    }
    if (failed) {
        audio_terminate=1;
        for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
            if (AudioStatus[i].threadhandle != -1) {
                //sceKernelWaitThreadEnd(AudioStatus[i].threadhandle,NULL);
                while (AudioStatus[i].threadactive)
                    sceKernelDelayThread(100000);
                sceKernelDeleteThread(AudioStatus[i].threadhandle);
            }
            AudioStatus[i].threadhandle = -1;
        }
        audio_ready=0;
        return -1;
    }
    return 0;
}


void pspAudioEndPre()
{
        audio_ready=0;
        audio_terminate=1;
}


void pspAudioEnd()
{
    int i;
    audio_ready=0;
    audio_terminate=1;

    for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
        if (AudioStatus[i].threadhandle != -1) {
            //sceKernelWaitThreadEnd(AudioStatus[i].threadhandle,NULL);
            while (AudioStatus[i].threadactive)
                sceKernelDelayThread(100000);
            sceKernelDeleteThread(AudioStatus[i].threadhandle);
        }
        AudioStatus[i].threadhandle = -1;
    }

    for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
        if (AudioStatus[i].handle != -1) {
            pspReleaseAudio();
            AudioStatus[i].handle = -1;
        }
    }
    sceKernelDeleteSema(play_sema);
}
