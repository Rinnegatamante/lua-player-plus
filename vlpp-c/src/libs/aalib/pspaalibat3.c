////////////////////////////////////////////////
//
//		pspaalibat3.c
//		Part of the PSP Advanced Audio Library
//		Created by Arshia001
//
//		This file includes functions for playing AT3 files
//		via the PSP's Media Engine.
//		Special thanks to mowglisanu of the PSP-Programming
//		forums for helping me with the following code.
//
////////////////////////////////////////////////

#include "pspaalibat3.h"

typedef struct
{
	int id;
	unsigned short tempBuf[4096];
	unsigned short* buf;
	int bufSize;
	void* data;
	int dataSize;
	int stopReason;
	bool paused;
	bool initialized;
	bool autoloop;
} At3FileInfo;

At3FileInfo streamsAt3[2];

bool GetPausedAt3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_AT3_INVALID_CHANNEL;
	}
	if (!streamsAt3[channel].initialized)
	{
		return PSPAALIB_ERROR_AT3_UNINITIALIZED_CHANNEL;
	}
	return streamsAt3[channel].paused;
}

int SetAutoloopAt3(int channel,bool autoloop)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_AT3_INVALID_CHANNEL;
	}
	if (!streamsAt3[channel].initialized)
	{
		return PSPAALIB_ERROR_AT3_UNINITIALIZED_CHANNEL;
	}
	streamsAt3[channel].autoloop=autoloop;
	return PSPAALIB_SUCCESS;
}

int GetStopReasonAt3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_AT3_INVALID_CHANNEL;
	}
	if (!streamsAt3[channel].initialized)
	{
		return PSPAALIB_ERROR_AT3_UNINITIALIZED_CHANNEL;
	}
	return streamsAt3[channel].stopReason;
}

int PlayAt3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_AT3_INVALID_CHANNEL;
	}
	if (!streamsAt3[channel].initialized)
	{
		return PSPAALIB_ERROR_AT3_UNINITIALIZED_CHANNEL;
	}
	streamsAt3[channel].paused=FALSE;
	streamsAt3[channel].stopReason=PSPAALIB_STOP_NOT_STOPPED;
	return PSPAALIB_SUCCESS;
}

int StopAt3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_AT3_INVALID_CHANNEL;
	}
	if (!streamsAt3[channel].initialized)
	{
		return PSPAALIB_ERROR_AT3_UNINITIALIZED_CHANNEL;
	}
	RewindAt3(channel);
	streamsAt3[channel].paused=TRUE;
	streamsAt3[channel].stopReason=PSPAALIB_STOP_ON_REQUEST;
	return PSPAALIB_SUCCESS;
}

int PauseAt3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_AT3_INVALID_CHANNEL;
	}
	if (!streamsAt3[channel].initialized)
	{
		return PSPAALIB_ERROR_AT3_UNINITIALIZED_CHANNEL;
	}
	streamsAt3[channel].paused=!streamsAt3[channel].paused;
	streamsAt3[channel].stopReason=PSPAALIB_STOP_NOT_STOPPED;
	return PSPAALIB_SUCCESS;
}

int RewindAt3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_AT3_INVALID_CHANNEL;
	}
	if (!streamsAt3[channel].initialized)
	{
		return PSPAALIB_ERROR_AT3_UNINITIALIZED_CHANNEL;
	}
	sceAtracResetPlayPosition(streamsAt3[channel].id,0,0,0);
	return PSPAALIB_SUCCESS;
}

int GetBufferAt3(short* buf,int length,float amp,int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_AT3_INVALID_CHANNEL;
	}
	int byteLength=length<<2;
	if ((streamsAt3[channel].paused)||(!streamsAt3[channel].initialized))
	{
		memset(buf,0,byteLength);
		return PSPAALIB_WARNING_PAUSED_BUFFER_REQUESTED;
	}
	int num,end,remain,i;
	while (streamsAt3[channel].bufSize<byteLength)
	{
		sceAtracDecodeData(streamsAt3[channel].id,(void*)streamsAt3[channel].tempBuf,&num,&end,&remain);
		if (end)
		{
			if (!streamsAt3[channel].autoloop)
			{
				streamsAt3[channel].paused=TRUE;
				streamsAt3[channel].stopReason=PSPAALIB_STOP_END_OF_STREAM;
				memset(buf,0,byteLength);
				return PSPAALIB_WARNING_END_OF_STREAM_REACHED;
			}
			RewindAt3(channel);
		}
		streamsAt3[channel].buf=(unsigned short*)realloc(streamsAt3[channel].buf,streamsAt3[channel].bufSize+(4*num));
		memcpy((void*)streamsAt3[channel].buf+streamsAt3[channel].bufSize,streamsAt3[channel].tempBuf,4*num);
		streamsAt3[channel].bufSize+=4*num;
	}
	for (i=0;i<2*length;i++)
	{
		buf[i]=streamsAt3[channel].buf[i]*amp;
	}
	streamsAt3[channel].bufSize-=byteLength;
	memmove((void*)streamsAt3[channel].buf,(void*)streamsAt3[channel].buf+byteLength,streamsAt3[channel].bufSize);
	return PSPAALIB_SUCCESS;
}

int LoadAt3(char* filename,int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_AT3_INVALID_CHANNEL;
	}
	if (streamsAt3[channel].initialized)
	{
		UnloadAt3(channel);
	}
	SceUID file=sceIoOpen(filename,PSP_O_RDONLY,0777);
	if (file<=0)
	{
		return PSPAALIB_ERROR_AT3_INVALID_FILE;
	}
	streamsAt3[channel].dataSize=sceIoLseek(file,0,PSP_SEEK_END);
	sceIoLseek(file,0,PSP_SEEK_SET);
	streamsAt3[channel].data=malloc(streamsAt3[channel].dataSize);
	if (!streamsAt3[channel].data)
	{
		sceIoClose(file);
		return PSPAALIB_ERROR_AT3_INSUFFICIENT_RAM;
	}
	sceIoRead(file,streamsAt3[channel].data,streamsAt3[channel].dataSize);
	sceIoClose(file);
	streamsAt3[channel].id=sceAtracSetDataAndGetID(streamsAt3[channel].data,streamsAt3[channel].dataSize);
	if (streamsAt3[channel].id<0)
	{
		free(streamsAt3[channel].data);
		sceIoClose(file);
		return PSPAALIB_ERROR_AT3_GET_ID;
	}
	streamsAt3[channel].bufSize=0;
	streamsAt3[channel].paused=TRUE;
	streamsAt3[channel].autoloop=FALSE;
	streamsAt3[channel].initialized=TRUE;
	streamsAt3[channel].stopReason=PSPAALIB_STOP_JUST_LOADED;
	return PSPAALIB_SUCCESS;
}

int UnloadAt3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_AT3_INVALID_CHANNEL;
	}
	StopAt3(channel);
	if (!streamsAt3[channel].initialized)
	{
		return PSPAALIB_SUCCESS;
	}
	streamsAt3[channel].paused=TRUE;
	streamsAt3[channel].stopReason=PSPAALIB_STOP_UNLOADED;
	free(streamsAt3[channel].data);
	free(streamsAt3[channel].buf);
	streamsAt3[channel].initialized=FALSE;
	return PSPAALIB_SUCCESS;
}

int InitAt3()
{
	sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC);
	sceUtilityLoadModule(PSP_MODULE_AV_ATRAC3PLUS);
	return PSPAALIB_SUCCESS;
}
