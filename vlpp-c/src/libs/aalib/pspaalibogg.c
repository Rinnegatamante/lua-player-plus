////////////////////////////////////////////////
//
//		pspaalibogg.c
//		Part of the PSP Advanced Audio Library
//		Created by Arshia001
//
//		This file includes functions for playing OGG/Vorbis
//		files.
//		The code for this file is based on the source of
//		LightMP3,written by Sayka (sakya_tg@yahoo.it).
//
////////////////////////////////////////////////

#include "pspaalibogg.h"

typedef struct{
	SceUID file;
	OggVorbis_File oggVorbisFile;
	char* data;
	long dataPos;
	long dataSize;
	int channel;
	short tempBuf[2048];
	short* buf;
	long bufLength;
	int stopReason;
	bool paused;
	bool outputInProgress;
	bool initialized;
	bool autoloop;
	bool loadToRam;
} OggFileInfo;

OggFileInfo streamsOgg[10];

size_t OggCallbackRead(void *buf,size_t length,size_t memBlockSize,void *ch)
{
	int* channel=(int*) ch;
    if (streamsOgg[*channel].loadToRam)
	{
		if (length*memBlockSize + streamsOgg[*channel].dataPos<=streamsOgg[*channel].dataSize)
		{
			memcpy(buf,streamsOgg[*channel].data+streamsOgg[*channel].dataPos,length*memBlockSize);
			streamsOgg[*channel].dataPos+=length*memBlockSize;
			return length*memBlockSize;
		}
		else
		{
			int remaining=streamsOgg[*channel].dataSize-streamsOgg[*channel].dataPos;
			memcpy(buf,streamsOgg[*channel].data+streamsOgg[*channel].dataPos,remaining);
			streamsOgg[*channel].dataPos=streamsOgg[*channel].dataSize;
			return remaining;
		}
	}
	else
	{
		return sceIoRead(streamsOgg[*channel].file,buf,length*memBlockSize);
	}
}

int OggCallbackSeek(void *ch,ogg_int64_t offset,int position)
{
	int* channel=(int*) ch;
	long seekPos;
	switch (position)
	{
	case PSP_SEEK_SET:
		seekPos=(long)offset;
		break;
	case PSP_SEEK_CUR:
		seekPos=streamsOgg[*channel].dataPos+(long)offset;
		break;
	case PSP_SEEK_END:
		seekPos=streamsOgg[*channel].dataSize+(long)offset;
		break;
	default:
		return 0;
	}
	if(seekPos==0)
	{
		if (!streamsOgg[*channel].autoloop)
		{
			streamsOgg[*channel].paused=TRUE;
			streamsOgg[*channel].stopReason=PSPAALIB_STOP_END_OF_STREAM;
		}
	}
	if (streamsOgg[*channel].loadToRam)
	{
		streamsOgg[*channel].dataPos=seekPos;
	}
	else
	{
		sceIoLseek32(streamsOgg[*channel].file,(unsigned int)offset,position);
	}
	return seekPos;
}

long OggCallbackTell(void *ch)
{
	int* channel=(int*) ch;
    if (streamsOgg[*channel].loadToRam)
	{
		return streamsOgg[*channel].dataPos;
	}
	else
	{
		return sceIoLseek32(streamsOgg[*channel].file,0,SEEK_CUR);
	}
}

int OggCallbackClose(void *ch)
{
	int* channel=(int*) ch;
	if (streamsOgg[*channel].loadToRam)
	{
		free(streamsOgg[*channel].data);
		return TRUE;
	}
	else
	{
		return sceIoClose(streamsOgg[*channel].file);
	}
}

bool GetPausedOgg(int channel)
{
	if ((channel<0)||(channel>9))
	{
		return PSPAALIB_ERROR_OGG_INVALID_CHANNEL;
	}
	if (!streamsOgg[channel].initialized)
	{
		return PSPAALIB_ERROR_OGG_UNINITIALIZED_CHANNEL;
	}
	return streamsOgg[channel].paused;
}

int SetAutoloopOgg(int channel,bool autoloop)
{
	if ((channel<0)||(channel>9))
	{
		return PSPAALIB_ERROR_OGG_INVALID_CHANNEL;
	}
	if (!streamsOgg[channel].initialized)
	{
		return PSPAALIB_ERROR_OGG_UNINITIALIZED_CHANNEL;
	}
	streamsOgg[channel].autoloop=autoloop;
	return PSPAALIB_SUCCESS;
}

int GetStopReasonOgg(int channel)
{
	if ((channel<0)||(channel>9))
	{
		return PSPAALIB_ERROR_OGG_INVALID_CHANNEL;
	}
	if (!streamsOgg[channel].initialized)
	{
		return PSPAALIB_ERROR_OGG_UNINITIALIZED_CHANNEL;
	}
	return streamsOgg[channel].stopReason;
}

int PlayOgg(int channel)
{
	if ((channel<0)||(channel>9))
	{
		return PSPAALIB_ERROR_OGG_INVALID_CHANNEL;
	}
	if (!streamsOgg[channel].initialized)
	{
		return PSPAALIB_ERROR_OGG_UNINITIALIZED_CHANNEL;
	}
	streamsOgg[channel].paused=FALSE;
	streamsOgg[channel].stopReason=PSPAALIB_STOP_NOT_STOPPED;
	return PSPAALIB_SUCCESS;
}

int StopOgg(int channel)
{
	if ((channel<0)||(channel>9))
	{
		return PSPAALIB_ERROR_OGG_INVALID_CHANNEL;
	}
	if (!streamsOgg[channel].initialized)
	{
		return PSPAALIB_ERROR_OGG_UNINITIALIZED_CHANNEL;
	}
	RewindOgg(channel);
	streamsOgg[channel].paused=TRUE;
	streamsOgg[channel].stopReason=PSPAALIB_STOP_ON_REQUEST;
	return PSPAALIB_SUCCESS;
}

int PauseOgg(int channel)
{
	if ((channel<0)||(channel>9))
	{
		return PSPAALIB_ERROR_OGG_INVALID_CHANNEL;
	}
	if (!streamsOgg[channel].initialized)
	{
		return PSPAALIB_ERROR_OGG_UNINITIALIZED_CHANNEL;
	}
	streamsOgg[channel].paused=!streamsOgg[channel].paused;
	streamsOgg[channel].stopReason=PSPAALIB_STOP_NOT_STOPPED;
	return PSPAALIB_SUCCESS;
}

int RewindOgg(int channel)
{
	return SeekOgg(channel,0);
}

int SeekOgg(int channel,int time)
{
	if ((channel<0)||(channel>9))
	{
		return PSPAALIB_ERROR_OGG_INVALID_CHANNEL;
	}
	if (!streamsOgg[channel].initialized)
	{
		return PSPAALIB_ERROR_OGG_UNINITIALIZED_CHANNEL;
	}
	bool tempPause=streamsOgg[channel].paused;
	streamsOgg[channel].paused=TRUE;
	ov_pcm_seek(&(streamsOgg[channel].oggVorbisFile),time*44100);
	streamsOgg[channel].paused=tempPause;
	return PSPAALIB_SUCCESS;
}

int GetBufferOgg(short* buf,int length,float amp,int channel)
{
	if ((channel<0)||(channel>9))
	{
		return PSPAALIB_ERROR_OGG_INVALID_CHANNEL;
	}
	int byteLength=length<<2;
	if (!streamsOgg[channel].initialized || streamsOgg[channel].paused)
	{
		memset((char*)buf,0,byteLength);
		return PSPAALIB_WARNING_PAUSED_BUFFER_REQUESTED;
	}
	streamsOgg[channel].outputInProgress = 1;
	int currentSection,i;
	while (streamsOgg[channel].bufLength<byteLength)
	{
		unsigned long bytesToRead=byteLength-streamsOgg[channel].bufLength;	
		unsigned long bytesRead=ov_read(&(streamsOgg[channel].oggVorbisFile),(char*)streamsOgg[channel].tempBuf,bytesToRead,&currentSection);
		if (!bytesRead)
		{
			if (!streamsOgg[channel].autoloop)
			{
				streamsOgg[channel].paused=TRUE;
				streamsOgg[channel].outputInProgress=FALSE;
				streamsOgg[channel].stopReason=PSPAALIB_STOP_END_OF_STREAM;
				return PSPAALIB_WARNING_END_OF_STREAM_REACHED;
			}
			RewindOgg(channel);
		}
		streamsOgg[channel].buf=(short*)realloc(streamsOgg[channel].buf,streamsOgg[channel].bufLength+bytesRead);
		memcpy((void*)streamsOgg[channel].buf+streamsOgg[channel].bufLength,streamsOgg[channel].tempBuf,bytesRead);
		streamsOgg[channel].bufLength+=bytesRead;
	}
	for(i=0;i<2*length;i++)
	{
		buf[i]=streamsOgg[channel].buf[i]*amp;
	}
	streamsOgg[channel].bufLength-=byteLength;
	memmove(streamsOgg[channel].buf,streamsOgg[channel].buf+byteLength,streamsOgg[channel].bufLength);
	streamsOgg[channel].outputInProgress=FALSE;
	return PSPAALIB_SUCCESS;
}

int LoadOgg(char* filename,int channel,bool loadToRam)
{
	if ((channel<0)||(channel>9))
	{
		return PSPAALIB_ERROR_OGG_INVALID_CHANNEL;
	}
	if (streamsOgg[channel].initialized)
	{
		UnloadOgg(channel);
	}
	loadToRam=FALSE;//////Something wrong with the seek/read function.Can't rewind when on ram.
	streamsOgg[channel].file=sceIoOpen(filename,PSP_O_RDONLY,0777);
	if (!streamsOgg[channel].file)
	{
		return PSPAALIB_ERROR_OGG_INVALID_FILE;
	}
	streamsOgg[channel].dataSize=sceIoLseek(streamsOgg[channel].file,0,PSP_SEEK_END);
	sceIoLseek(streamsOgg[channel].file,0,PSP_SEEK_SET);
	streamsOgg[channel].channel=channel;
	ov_callbacks oggCallbacks;
	oggCallbacks.read_func=OggCallbackRead;
	oggCallbacks.seek_func=OggCallbackSeek;
	oggCallbacks.close_func=OggCallbackClose;
	oggCallbacks.tell_func=OggCallbackTell;
	if (ov_open_callbacks(&(streamsOgg[channel].channel),&(streamsOgg[channel].oggVorbisFile),NULL,0,oggCallbacks)<0)
	{
		sceIoClose(streamsOgg[channel].file);
		return PSPAALIB_ERROR_OGG_OPEN_CALLBACKS;
	}
	streamsOgg[channel].loadToRam=loadToRam;
	if (streamsOgg[channel].loadToRam)
	{
		streamsOgg[channel].data=(char*)malloc(streamsOgg[channel].dataSize);
		if (!streamsOgg[channel].data)
		{
			ov_clear(&(streamsOgg[channel].oggVorbisFile));
			sceIoClose(streamsOgg[channel].file);
			return PSPAALIB_ERROR_OGG_INSUFFICIENT_RAM;
		}
		sceIoRead(streamsOgg[channel].file,streamsOgg[channel].data,streamsOgg[channel].dataSize);
	}
	streamsOgg[channel].bufLength=0;
	streamsOgg[channel].paused=TRUE;
	streamsOgg[channel].initialized=TRUE;
	streamsOgg[channel].stopReason=PSPAALIB_STOP_JUST_LOADED;
	return PSPAALIB_SUCCESS;
}

int UnloadOgg(int channel)
{
	if ((channel<0)||(channel>9))
	{
		return PSPAALIB_ERROR_OGG_INVALID_CHANNEL;
	}
	StopOgg(channel);
	if (!streamsOgg[channel].initialized)
	{
		return PSPAALIB_SUCCESS;
	}
	streamsOgg[channel].paused=TRUE;	
	while (streamsOgg[channel].outputInProgress)
	{
		sceKernelDelayThread(10000);
	}
	ov_clear(&(streamsOgg[channel].oggVorbisFile));
	sceIoClose(streamsOgg[channel].file);
	streamsOgg[channel].stopReason=PSPAALIB_STOP_UNLOADED;
	streamsOgg[channel].initialized=FALSE;
	return PSPAALIB_SUCCESS;
}
