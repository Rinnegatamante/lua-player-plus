////////////////////////////////////////////////
//
//		pspaalibwav.c
//		Part of the PSP Advanced Audio Library
//		Created by Arshia001
//
//		This file includes functions for playing WAV files
//		with different bitrates/frequencies.
//
////////////////////////////////////////////////

#include "pspaalibwav.h"

typedef struct
{
	SceUID file;
	char* data;
	int dataLength;
	int dataLocation;
	int dataPos;
	short sigBytes;
	short numChannels;
	int sampleRate;
	int bytesPerSecond;
	int stopReason;
	bool paused;
	bool loadToRam;
	bool autoloop;
	bool initialized;
} WavFileInfo;

WavFileInfo streamsWav[32];

bool GetPausedWav(int channel)
{
	if ((channel<0)||(channel>31))
	{
		return PSPAALIB_ERROR_WAV_INVALID_CHANNEL;
	}
	if (!streamsWav[channel].initialized)
	{
		return PSPAALIB_ERROR_WAV_UNINITIALIZED_CHANNEL;
	}
	return streamsWav[channel].paused;
}

int SetAutoloopWav(int channel,bool autoloop)
{
	if ((channel<0)||(channel>31))
	{
		return PSPAALIB_ERROR_WAV_INVALID_CHANNEL;
	}
	if (!streamsWav[channel].initialized)
	{
		return PSPAALIB_ERROR_WAV_UNINITIALIZED_CHANNEL;
	}
	streamsWav[channel].autoloop=autoloop;
	return PSPAALIB_SUCCESS;
}

int GetStopReasonWav(int channel)
{
	if ((channel<0)||(channel>31))
	{
		return PSPAALIB_ERROR_WAV_INVALID_CHANNEL;
	}
	if (!streamsWav[channel].initialized)
	{
		return PSPAALIB_ERROR_WAV_UNINITIALIZED_CHANNEL;
	}
	return streamsWav[channel].stopReason;
}

int PlayWav(int channel)
{
	if ((channel<0)||(channel>31))
	{
		return PSPAALIB_ERROR_WAV_INVALID_CHANNEL;
	}
	if (!streamsWav[channel].initialized)
	{
		return PSPAALIB_ERROR_WAV_UNINITIALIZED_CHANNEL;
	}
	streamsWav[channel].paused=FALSE;
	streamsWav[channel].stopReason=PSPAALIB_STOP_NOT_STOPPED;
	return PSPAALIB_SUCCESS;
}

int StopWav(int channel)
{
	if ((channel<0)||(channel>31))
	{
		return PSPAALIB_ERROR_WAV_INVALID_CHANNEL;
	}
	if (!streamsWav[channel].initialized)
	{
		return PSPAALIB_ERROR_WAV_UNINITIALIZED_CHANNEL;
	}
	RewindWav(channel);
	streamsWav[channel].paused=TRUE;
	streamsWav[channel].stopReason=PSPAALIB_STOP_ON_REQUEST;
	return PSPAALIB_SUCCESS;
}

int PauseWav(int channel)
{
	if ((channel<0)||(channel>31))
	{
		return PSPAALIB_ERROR_WAV_INVALID_CHANNEL;
	}
	if (!streamsWav[channel].initialized)
	{
		return PSPAALIB_ERROR_WAV_UNINITIALIZED_CHANNEL;
	}
	streamsWav[channel].paused=!streamsWav[channel].paused;
	streamsWav[channel].stopReason=PSPAALIB_STOP_NOT_STOPPED;
	return PSPAALIB_SUCCESS;
}

int SeekWav(int time,int channel)
{
	if ((channel<0)||(channel>31))
	{
		return PSPAALIB_ERROR_WAV_INVALID_CHANNEL;
	}
	if (!streamsWav[channel].initialized)
	{
		return PSPAALIB_ERROR_WAV_UNINITIALIZED_CHANNEL;
	}
	int dataPos=(int) time*streamsWav[channel].bytesPerSecond;
	if (dataPos>streamsWav[channel].dataLength)
	{
		return PSPAALIB_ERROR_WAV_INVALID_SEEK_TIME;
	}
	streamsWav[channel].dataPos=dataPos;
	if (!streamsWav[channel].loadToRam)
	{
		sceIoLseek(streamsWav[channel].file,streamsWav[channel].dataLocation+streamsWav[channel].dataPos,PSP_SEEK_SET);
	}
	return PSPAALIB_SUCCESS;
}

int RewindWav(int channel)
{
	return SeekWav(0,channel);
}

int GetBufferWav(short* buf,int length,float amp,int channel)
{
	if ((channel<0)||(channel>31))
	{
		return PSPAALIB_ERROR_WAV_INVALID_CHANNEL;
	}
	if (streamsWav[channel].paused || !streamsWav[channel].initialized || streamsWav[channel].stopReason==PSPAALIB_STOP_END_OF_STREAM)
	{
		memset((char*)buf,0,4*length);
		return PSPAALIB_WARNING_PAUSED_BUFFER_REQUESTED;
	}
	int i,index;
	int realLength=length*streamsWav[channel].sigBytes*streamsWav[channel].numChannels*streamsWav[channel].sampleRate/PSP_SAMPLE_RATE;
	if (streamsWav[channel].dataPos+realLength>=streamsWav[channel].dataLength)
	{
		RewindWav(channel);
		if (!streamsWav[channel].autoloop)
		{
			streamsWav[channel].paused=TRUE;
			streamsWav[channel].stopReason=PSPAALIB_STOP_END_OF_STREAM;
			memset((char*)buf,0,4*length);
			return PSPAALIB_SUCCESS;
		}
		return GetBufferWav(buf,length,amp,channel);
	}
	if (streamsWav[channel].loadToRam)
	{
		for (i=0;i<length;i++)
		{
			if (streamsWav[channel].sigBytes==1)
			{
				index=streamsWav[channel].numChannels*(int)(i*streamsWav[channel].sampleRate/PSP_SAMPLE_RATE)+streamsWav[channel].dataPos;
				buf[2*i]=(streamsWav[channel].data[index]<<8)*amp;
				index+=((streamsWav[channel].numChannels>1)?1:0);
				buf[2*i+1]=(streamsWav[channel].data[index]<<8)*amp;
			}
			else if (streamsWav[channel].sigBytes==2)
			{
				index=streamsWav[channel].numChannels*(int)(i*streamsWav[channel].sampleRate/PSP_SAMPLE_RATE)+(streamsWav[channel].dataPos/2);
				buf[2*i]=(((short*)streamsWav[channel].data)[index])*amp;
				index+=((streamsWav[channel].numChannels>1)?1:0);
				buf[2*i+1]=(((short*)streamsWav[channel].data)[index])*amp;
			}
			else
			{
				memset((char*)buf,0,4*length);
				return PSPAALIB_WARNING_WAV_INVALID_SBPS;
			}
		}
	}
	else
	{
		sceIoRead(streamsWav[channel].file,streamsWav[channel].data,realLength);
		for (i=0;i<length;i++)
		{
			if (streamsWav[channel].sigBytes==1)
			{
				index=streamsWav[channel].numChannels*(int)(i*streamsWav[channel].sampleRate/PSP_SAMPLE_RATE);
				buf[2*i]=(streamsWav[channel].data[index]<<8)*amp;
				index+=((streamsWav[channel].numChannels>1)?1:0);
				buf[2*i+1]=(streamsWav[channel].data[index]<<8)*amp;
			}
			else if (streamsWav[channel].sigBytes==2)
			{
				index=streamsWav[channel].numChannels*(int)(i*streamsWav[channel].sampleRate/PSP_SAMPLE_RATE);
				buf[2*i]=(((short*)streamsWav[channel].data)[index])*amp;
				index+=((streamsWav[channel].numChannels>1)?1:0);
				buf[2*i+1]=(((short*)streamsWav[channel].data)[index])*amp;
			}
			else
			{
				memset((char*)buf,0,4*length);
				return PSPAALIB_WARNING_WAV_INVALID_SBPS;
			}
		}
	}
	streamsWav[channel].dataPos+=realLength;
	return PSPAALIB_SUCCESS;
}

int LoadWav(char* filename,int channel,bool loadToRam)
{
	if ((channel<0)||(channel>31))
	{
		return PSPAALIB_ERROR_WAV_INVALID_CHANNEL;
	}
	if (streamsWav[channel].initialized) UnloadWav(channel);
	int chunks=0,size=0;
	short compressionCode=0;
	char temp[5];
	temp[4]='\0';
	streamsWav[channel].loadToRam=loadToRam;
	streamsWav[channel].file=sceIoOpen(filename,PSP_O_RDONLY,0777);
	if (streamsWav[channel].file<=0) 
	{
		return PSPAALIB_ERROR_WAV_INVALID_FILE;
	}
	sceIoRead(streamsWav[channel].file,temp,4);
	if (strcmp(temp,"RIFF"))
	{
		sceIoClose(streamsWav[channel].file);
		return PSPAALIB_ERROR_WAV_INVALID_FILE;
	}
	sceIoRead(streamsWav[channel].file,&size,4);
	sceIoRead(streamsWav[channel].file,temp,4);
	if (strcmp(temp,"WAVE"))
	{
		sceIoClose(streamsWav[channel].file);
		return PSPAALIB_ERROR_WAV_INVALID_FILE;
	}
	while (chunks<2)
	{
		sceIoRead(streamsWav[channel].file,temp,4);
		if (!strcmp(temp,"fmt "))
		{
			sceIoRead(streamsWav[channel].file,&size,4);
			sceIoRead(streamsWav[channel].file,&compressionCode,2);
			if ((compressionCode!=0)&&(compressionCode!=1))
			{
				sceIoClose(streamsWav[channel].file);
				return PSPAALIB_ERROR_WAV_COMPRESSED_FILE;
			}
			sceIoRead(streamsWav[channel].file,&(streamsWav[channel].numChannels),2);
			sceIoRead(streamsWav[channel].file,&(streamsWav[channel].sampleRate),4);
			sceIoRead(streamsWav[channel].file,&(streamsWav[channel].bytesPerSecond),4);
			sceIoLseek(streamsWav[channel].file,2,PSP_SEEK_CUR);
			sceIoRead(streamsWav[channel].file,&(streamsWav[channel].sigBytes),2);
			streamsWav[channel].sigBytes=streamsWav[channel].sigBytes>>3;
			sceIoLseek(streamsWav[channel].file,size-16,PSP_SEEK_CUR);
			chunks++;
			continue;
		}
		if (!strcmp(temp,"data"))
		{
			if (chunks<1)
			{
				sceIoClose(streamsWav[channel].file);
				return PSPAALIB_ERROR_WAV_INVALID_FILE;
			}
			sceIoRead(streamsWav[channel].file,&(streamsWav[channel].dataLength),4);
			streamsWav[channel].dataLocation=sceIoLseek(streamsWav[channel].file,0,PSP_SEEK_CUR);
			if (streamsWav[channel].loadToRam)
			{
				streamsWav[channel].data=(char*) malloc(streamsWav[channel].dataLength);
				if(!streamsWav[channel].data)
				{
					sceIoClose(streamsWav[channel].file);
					return PSPAALIB_ERROR_WAV_INSUFFICIENT_RAM;
				}
				sceIoRead(streamsWav[channel].file,streamsWav[channel].data,streamsWav[channel].dataLength);
			}
			else
			{
				streamsWav[channel].data=(char*) malloc(1024*streamsWav[channel].sigBytes*streamsWav[channel].numChannels*streamsWav[channel].sampleRate/PSP_SAMPLE_RATE);
				if(!streamsWav[channel].data)
				{
					sceIoClose(streamsWav[channel].file);
					return PSPAALIB_ERROR_WAV_INSUFFICIENT_RAM;
				}
			}
			streamsWav[channel].dataPos=0;
			chunks++;
			continue;
		}
		sceIoRead(streamsWav[channel].file,&size,4);
		sceIoLseek(streamsWav[channel].file,size,PSP_SEEK_CUR);
	}
	if (streamsWav[channel].loadToRam)
	{
		sceIoClose(streamsWav[channel].file);
	}
	else
	{
		sceIoLseek(streamsWav[channel].file,streamsWav[channel].dataLocation,PSP_SEEK_SET);
	}
	streamsWav[channel].initialized=TRUE;
	streamsWav[channel].paused=TRUE;
	streamsWav[channel].stopReason=PSPAALIB_STOP_JUST_LOADED;
	return PSPAALIB_SUCCESS;
}

int UnloadWav(int channel)
{
	if ((channel<0)||(channel>31))
	{
		return PSPAALIB_ERROR_WAV_INVALID_CHANNEL;
	}
	StopWav(channel);
	if(!streamsWav[channel].loadToRam)
	{
		sceIoClose(streamsWav[channel].file);
	}
	free(streamsWav[channel].data);
	streamsWav[channel].initialized=FALSE;
	streamsWav[channel].stopReason=PSPAALIB_STOP_UNLOADED;
	return PSPAALIB_SUCCESS;
}
