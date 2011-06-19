////////////////////////////////////////////////
//
//		pspaalib.c
//		Part of the PSP Advanced Audio Library
//		Created by Arshia001
//
//		This file includes all the public functions for the
//		PSPAALIB.
//
////////////////////////////////////////////////

#include "pspaalib.h"

typedef struct
{
	bool effects[7];
	ScePspFVector2 position;
	ScePspFVector2 velocity;
	float playSpeed;
	AalibVolume volume;
	float ampValue;
	float audioStrength;
	bool initialized;
} AalibChannelData;

AalibChannelData channels[49];
int hardwareChannels[8];
SceUID threads[8];
ScePspFVector2 observerPosition={0,0},observerFront={0,1},observerVelocity={0,0};

int GetFreeHardwareChannel()
{
	int i;
	for (i=0;i<8;i++)
	{
		if (!hardwareChannels[i])
		{
			return i;
		}
	}
	return -1;
}

int FreeHardwareChannel(int channel)
{
	int i;
	for (i=0;i<8;i++)
	{
		if (hardwareChannels[i]==channel)
		{
			while (AalibGetStatus(channel)!=PSPAALIB_STATUS_STOPPED)
			{
				sceKernelDelayThread(10);
			}
			sceAudioChRelease(i);
			hardwareChannels[i]=PSPAALIB_CHANNEL_NONE;
			return TRUE;
		}
	}
	return FALSE;
}

int GetRawBuffer(short* buf,int length,float amp,int channel)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return GetBufferWav(buf,length,amp,channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return GetBufferOgg(buf,length,amp,channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return GetBufferSceMp3(buf,length,amp,channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return GetBufferAt3(buf,length,amp,channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

void GetProcessedBuffer(void* abuf,unsigned int length,int channel)
{
	short* buf=(short*) abuf;
	//Control Volume
	if (channels[channel].effects[PSPAALIB_EFFECT_STEREO_BY_POSITION])
	{
		channels[channel].volume=GetVolumes(channels[channel].position,observerPosition,observerFront);
	}
	else if(!channels[channel].effects[PSPAALIB_EFFECT_VOLUME_MANUAL])
	{
		channels[channel].volume=(AalibVolume){1.0f,1.0f};
	}
	if (channels[channel].effects[PSPAALIB_EFFECT_STRENGTH_BY_POSITION])
	{
		channels[channel].audioStrength=GetStrengthByPosition(channels[channel].position,observerPosition);
	}
	else
	{
		channels[channel].audioStrength=1.0f;
	}
	//Control Play Speed
	if (channels[channel].effects[PSPAALIB_EFFECT_DOPPLER])
	{
		channels[channel].playSpeed=GetDopplerPlaySpeed(channels[channel].position,channels[channel].velocity,observerPosition,observerVelocity);
	}
	else if (!channels[channel].effects[PSPAALIB_EFFECT_PLAYSPEED])
	{
		channels[channel].playSpeed=1.0f;
	}
	if (!channels[channel].effects[PSPAALIB_EFFECT_AMPLIFY])
	{
		channels[channel].ampValue=1.0f;
	}
	//Get Buffer
	if ((channels[channel].effects[PSPAALIB_EFFECT_PLAYSPEED])||(channels[channel].effects[PSPAALIB_EFFECT_DOPPLER])||(channels[channel].effects[PSPAALIB_EFFECT_MIX]))
	{		
		short* tempBuf;
		tempBuf=malloc((int)(length*channels[channel].playSpeed*2*sizeof(short)));
		GetRawBuffer(tempBuf,length*channels[channel].playSpeed,channels[channel].ampValue,channel);
		GetBufferSpeedEffect(buf,tempBuf,length,channels[channel].playSpeed,channels[channel].effects[PSPAALIB_EFFECT_MIX]);
		free(tempBuf);
	}
	else
	{
		GetRawBuffer(buf,length,channels[channel].ampValue,channel);
	}
}

int PlayThread(SceSize argsize, void* args)
{
	if (argsize!=sizeof(int))
	{
		sceKernelExitThread(0);
	}
	int hardwareChannel=*((int*)args);
	int channel=hardwareChannels[hardwareChannel];
	int stopReason;
	void *mainBuf,*backBuf,*tempBuf;
	mainBuf=malloc(4096);
	backBuf=malloc(4096);
	sceAudioChReserve(hardwareChannel,1024,PSP_AUDIO_FORMAT_STEREO);
	while(TRUE)
	{
		stopReason=AalibGetStopReason(channel);
		if (!stopReason)
		{
			goto Play;
		}
		else if (stopReason<0)
		{
			sceKernelDelayThread(10);
			continue;
		}
		else
		{
			goto Release;
		}
	}
Play:
	GetProcessedBuffer(mainBuf,1024,channel);
	while (!AalibGetStopReason(channel))
	{
		sceAudioOutputPanned(hardwareChannel,(unsigned int)(channels[channel].volume.left*channels[channel].audioStrength*PSP_AUDIO_VOLUME_MAX),(unsigned int)(channels[channel].volume.right*channels[channel].audioStrength*PSP_AUDIO_VOLUME_MAX),mainBuf);
		GetProcessedBuffer(backBuf,1024,channel);
		while (sceAudioGetChannelRestLen(hardwareChannel))
		{
			sceKernelDelayThread(100);
		}
		tempBuf=mainBuf;
		mainBuf=backBuf;
		backBuf=tempBuf;
	}
Release:
	FreeHardwareChannel(channel);
	AalibStop(channel);
	sceKernelExitThread(0);
	return 0;
}

int AalibInit()
{
	InitSceMp3();
	InitAt3();
	char c[11];
	int i;
	for (i=0;i<8;i++)
	{
		sprintf(c,"aalibplay%i",i);
		threads[i]=sceKernelCreateThread(c,PlayThread,0x18,0x10000,0,NULL);
		if (threads[i]<0)
		{
			return PSPAALIB_WARNING_CREATE_THREAD;
		}
	}
	return PSPAALIB_SUCCESS;
}

int AalibSetAmplification(int channel,float amplificationValue)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	if (amplificationValue<0)
	{
		return PSPAALIB_ERROR_INVALID_AMPLIFICATION_VALUE;
	}
	channels[channel].ampValue=amplificationValue;
	return PSPAALIB_SUCCESS;
}

int AalibSetVolume(int channel,AalibVolume volume)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	channels[channel].volume=volume;
	return PSPAALIB_SUCCESS;
}

int AalibSetPlaySpeed(int channel,float playSpeed)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	channels[channel].playSpeed=playSpeed;
	return PSPAALIB_SUCCESS;
}

int AalibSetObserverVelocity(ScePspFVector2 velocity)
{
	observerVelocity=velocity;
	return PSPAALIB_SUCCESS;
}

int AalibSetObserverPosition(ScePspFVector2 position)
{
	observerPosition=position;
	return PSPAALIB_SUCCESS;
}

int AalibSetObserverFront(ScePspFVector2 front)
{
	observerFront=front;
	return PSPAALIB_SUCCESS;
}

int AalibSetPosition(int channel,ScePspFVector2 position)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	channels[channel].position=position;
	return PSPAALIB_SUCCESS;
}

int AalibSetVelocity(int channel,ScePspFVector2 velocity)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	channels[channel].velocity=velocity;
	return PSPAALIB_SUCCESS;
}

int AalibEnable(int channel,int effect)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	if ((effect<0)||(effect>6))
	{
		return PSPAALIB_ERROR_INVALID_EFFECT;
	}
	channels[channel].effects[effect]=TRUE;
	return PSPAALIB_SUCCESS;
}

int AalibDisable(int channel,int effect)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	channels[channel].effects[effect]=FALSE;
	return PSPAALIB_SUCCESS;
}

int AalibUnload(int channel)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	channels[channel].initialized=FALSE;
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return UnloadWav(channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return UnloadOgg(channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return UnloadSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return UnloadAt3(channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibLoad(char* filename,int channel,bool loadToRam)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	memset(channels[channel].effects,0,7);
	channels[channel].position=(ScePspFVector2){0.0f,0.0f};
	channels[channel].velocity=(ScePspFVector2){0.0f,0.0f};
	channels[channel].playSpeed=1.0f;
	channels[channel].volume=(AalibVolume){1.0f,1.0f};
	channels[channel].ampValue=1.0f;
	channels[channel].initialized=TRUE;
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return LoadWav(filename,channel-PSPAALIB_CHANNEL_WAV_1,loadToRam);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return LoadOgg(filename,channel-PSPAALIB_CHANNEL_OGG_1,loadToRam);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return LoadSceMp3(filename,channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return LoadAt3(filename,channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibPlay(int channel)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	if (!channels[channel].initialized)
	{
		return PSPAALIB_ERROR_UNINITIALIZED_CHANNEL;
	}
	if (!AalibGetStopReason(channel))
	{
		return PSPAALIB_SUCCESS;
	}
	int hardwareChannel=GetFreeHardwareChannel();
	if (hardwareChannel==-1)
	{
		return PSPAALIB_WARNING_NO_FREE_CHANNELS;
	}
	hardwareChannels[hardwareChannel]=channel;
	sceKernelStartThread(threads[hardwareChannel],sizeof(int),(void*)(&hardwareChannel));
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return PlayWav(channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return PlayOgg(channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return PlaySceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return PlayAt3(channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibStop(int channel)
{
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return StopWav(channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return StopOgg(channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return StopSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return StopAt3(channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibPause(int channel)
{
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return PauseWav(channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return PauseOgg(channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return PauseSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return PauseAt3(channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibRewind(int channel)
{
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return RewindWav(channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return RewindOgg(channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return RewindSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return RewindAt3(channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibSeek(int channel,int time)
{
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return SeekWav(channel-PSPAALIB_CHANNEL_WAV_1,time);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return SeekOgg(channel-PSPAALIB_CHANNEL_OGG_1,time);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibSetAutoloop(int channel,bool autoloop)
{
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return SetAutoloopWav(channel-PSPAALIB_CHANNEL_WAV_1,autoloop);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return SetAutoloopOgg(channel-PSPAALIB_CHANNEL_OGG_1,autoloop);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return SetAutoloopSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1,autoloop);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return SetAutoloopAt3(channel-PSPAALIB_CHANNEL_AT3_1,autoloop);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibGetStopReason(int channel)
{
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return GetStopReasonWav(channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return GetStopReasonOgg(channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return GetStopReasonSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return GetStopReasonAt3(channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibGetStatus(int channel)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		if (GetStopReasonWav(channel-PSPAALIB_CHANNEL_WAV_1))
		{
			return PSPAALIB_STATUS_STOPPED;
		}
		else if (GetPausedWav(channel-PSPAALIB_CHANNEL_WAV_1))
		{
			return PSPAALIB_STATUS_PAUSED;
		}
		else
		{
			return PSPAALIB_STATUS_PLAYING;
		}
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		if (GetStopReasonOgg(channel-PSPAALIB_CHANNEL_OGG_1))
		{
			return PSPAALIB_STATUS_STOPPED;
		}
		else if (GetPausedOgg(channel-PSPAALIB_CHANNEL_OGG_1))
		{
			return PSPAALIB_STATUS_PAUSED;
		}
		else
		{
			return PSPAALIB_STATUS_PLAYING;
		}
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		if (GetStopReasonSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1))
		{
			return PSPAALIB_STATUS_STOPPED;
		}
		else if (GetPausedSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1))
		{
			return PSPAALIB_STATUS_PAUSED;
		}
		else
		{
			return PSPAALIB_STATUS_PLAYING;
		}
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		if (GetStopReasonAt3(channel-PSPAALIB_CHANNEL_AT3_1))
		{
			return PSPAALIB_STATUS_STOPPED;
		}
		else if (GetPausedAt3(channel-PSPAALIB_CHANNEL_AT3_1))
		{
			return PSPAALIB_STATUS_PAUSED;
		}
		else
		{
			return PSPAALIB_STATUS_PLAYING;
		}
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}
