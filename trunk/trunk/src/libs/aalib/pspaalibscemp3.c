////////////////////////////////////////////////
//
//		pspaalibscemp3.c
//		Part of the PSP Advanced Audio Library
//		Created by Arshia001
//
//		This file includes functions for playing MP3 files
//		via the PSP's Media Engine.A number of MP3's 
//		may not play this way,in which case error code 10
//		will be returned by AalibLoad().If this method fails,
//		try using MADMP3 instead.
//		The code for skipping ID3v2 tags was written by
//		AlphaDingDong of the PSP-Programming forums.
//
////////////////////////////////////////////////

#include "pspaalibscemp3.h"

#define ENDIAN_SWAP_32BIT(n) (n = ((n&0xFF000000)>>24)|((n&0x00FF0000)>>8)|((n&0x0000FF00)<<8)|((n&0x000000FF)<<24))
#define ID3_TAG_LENGTH_TO_INT(n) (n = (((n&0x7f000000)>>3)|((n&0x7f0000)>>2)|((n&0x7f00)>>1)|(n&0x7f)))

typedef struct
{
	SceUID file;
	SceMp3InitArg args;
	int handle;
	void* mp3Buf;
	void* pcmBuf;
	int lastPosition;
	int bufSize;
	unsigned char* buf;
	int stopReason;
	bool autoloop;
	bool paused;
	bool initialized;
} SceMp3FileInfo;

SceMp3FileInfo streamsSceMp3[2];

bool GetPausedSceMp3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_SCEMP3_INVALID_CHANNEL;
	}
	if (!streamsSceMp3[channel].initialized)
	{
		return PSPAALIB_ERROR_SCEMP3_UNINITIALIZED_CHANNEL;
	}
	return streamsSceMp3[channel].paused;
}

int SetAutoloopSceMp3(int channel,bool autoloop)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_SCEMP3_INVALID_CHANNEL;
	}
	if (!streamsSceMp3[channel].initialized)
	{
		return PSPAALIB_ERROR_SCEMP3_UNINITIALIZED_CHANNEL;
	}
	streamsSceMp3[channel].autoloop=autoloop;
	return PSPAALIB_SUCCESS;
}

int GetStopReasonSceMp3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_SCEMP3_INVALID_CHANNEL;
	}
	if (!streamsSceMp3[channel].initialized)
	{
		return PSPAALIB_ERROR_SCEMP3_UNINITIALIZED_CHANNEL;
	}
	return streamsSceMp3[channel].stopReason;
}

int PlaySceMp3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_SCEMP3_INVALID_CHANNEL;
	}
	if (!streamsSceMp3[channel].initialized)
	{
		return PSPAALIB_ERROR_SCEMP3_UNINITIALIZED_CHANNEL;
	}
	streamsSceMp3[channel].paused=FALSE;
	streamsSceMp3[channel].stopReason=PSPAALIB_STOP_NOT_STOPPED;
	return PSPAALIB_SUCCESS;
}

int StopSceMp3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_SCEMP3_INVALID_CHANNEL;
	}
	if (!streamsSceMp3[channel].initialized)
	{
		return PSPAALIB_ERROR_SCEMP3_UNINITIALIZED_CHANNEL;
	}
	RewindSceMp3(channel);
	streamsSceMp3[channel].paused=TRUE;
	streamsSceMp3[channel].stopReason=PSPAALIB_STOP_ON_REQUEST;
	return PSPAALIB_SUCCESS;
}

int PauseSceMp3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_SCEMP3_INVALID_CHANNEL;
	}
	if (!streamsSceMp3[channel].initialized)
	{
		return PSPAALIB_ERROR_SCEMP3_UNINITIALIZED_CHANNEL;
	}
	streamsSceMp3[channel].paused=!streamsSceMp3[channel].paused;
	streamsSceMp3[channel].stopReason=PSPAALIB_STOP_NOT_STOPPED;
	return PSPAALIB_SUCCESS;
}

int RewindSceMp3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_SCEMP3_INVALID_CHANNEL;
	}
	if (!streamsSceMp3[channel].initialized)
	{
		return PSPAALIB_ERROR_SCEMP3_UNINITIALIZED_CHANNEL;
	}
	bool tempPause=streamsSceMp3[channel].paused;
	sceMp3ResetPlayPosition(streamsSceMp3[channel].handle);
	streamsSceMp3[channel].paused=tempPause;
	return PSPAALIB_SUCCESS;
}

void FillBuffer(int channel)
{
	SceUChar8* dst;
	int num;
	int pos;
	sceMp3GetInfoToAddStreamData(streamsSceMp3[channel].handle,&dst,&num,&pos);
	if (streamsSceMp3[channel].lastPosition>pos)
	{
		if (!streamsSceMp3[channel].autoloop)
		{
			streamsSceMp3[channel].paused=TRUE;
			streamsSceMp3[channel].stopReason=PSPAALIB_STOP_END_OF_STREAM;
		}
	}
	streamsSceMp3[channel].lastPosition=pos;
	sceIoLseek32(streamsSceMp3[channel].file,pos,PSP_SEEK_SET);
	int read=sceIoRead(streamsSceMp3[channel].file,(char*)dst,num);
	sceMp3NotifyAddStreamData(streamsSceMp3[channel].handle,read);
}

int GetBufferSceMp3(short* buf,int length,float amp,int channel)
{
	int byteLength = length<<2,i=0;
	if (streamsSceMp3[channel].paused || !streamsSceMp3[channel].initialized) 
	{
		memset(buf,0,byteLength);
		return PSPAALIB_WARNING_PAUSED_BUFFER_REQUESTED;
	}
	while(streamsSceMp3[channel].bufSize<byteLength)
	{
		if(sceMp3CheckStreamDataNeeded(streamsSceMp3[channel].handle))
		{
			FillBuffer(channel);
		}
		short* decodeBuf;
		unsigned int bytesDecoded=sceMp3Decode(streamsSceMp3[channel].handle,&decodeBuf);
		streamsSceMp3[channel].buf=(u8*)realloc(streamsSceMp3[channel].buf,streamsSceMp3[channel].bufSize+bytesDecoded);
		memcpy(streamsSceMp3[channel].buf+streamsSceMp3[channel].bufSize,decodeBuf,bytesDecoded);
		streamsSceMp3[channel].bufSize+=bytesDecoded;
	}
	for (i=0;i<2*length;i++)
	{
		buf[i]=((short)(streamsSceMp3[channel].buf[2*i] | streamsSceMp3[channel].buf[2*i+1]<<8))*amp;
	}
	streamsSceMp3[channel].bufSize-=byteLength;
	memmove(streamsSceMp3[channel].buf,streamsSceMp3[channel].buf+byteLength,streamsSceMp3[channel].bufSize);
	return PSPAALIB_SUCCESS;
}

int LoadSceMp3(char* filename,int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_SCEMP3_INVALID_CHANNEL;
	}
	if (streamsSceMp3[channel].initialized)
	{
		UnloadSceMp3(channel);
	}
	char* c=(char*)malloc(4);
	c[3]='\0';
	streamsSceMp3[channel].mp3Buf=memalign(64,16*1024);
	streamsSceMp3[channel].pcmBuf=memalign(64,8*1152);
	streamsSceMp3[channel].file=sceIoOpen(filename,PSP_O_RDONLY,0777);
	if (!streamsSceMp3[channel].file)
	{
		free(streamsSceMp3[channel].mp3Buf);
		free(streamsSceMp3[channel].pcmBuf);
		return PSPAALIB_ERROR_SCEMP3_INVALID_FILE;
	}
	streamsSceMp3[channel].args.mp3StreamStart=0;
	streamsSceMp3[channel].args.mp3StreamEnd=sceIoLseek(streamsSceMp3[channel].file,0,PSP_SEEK_END);
	streamsSceMp3[channel].args.unk1=0;
	streamsSceMp3[channel].args.unk2=0;
	streamsSceMp3[channel].args.mp3Buf=streamsSceMp3[channel].mp3Buf;
	streamsSceMp3[channel].args.mp3BufSize=16*1024;
	streamsSceMp3[channel].args.pcmBuf=streamsSceMp3[channel].pcmBuf;
	streamsSceMp3[channel].args.pcmBufSize=8*1152;
	sceIoLseek(streamsSceMp3[channel].file,-128,PSP_SEEK_END);
	sceIoRead(streamsSceMp3[channel].file,c,3);
	if(!strcmp(c,"TAG"))
	{
		streamsSceMp3[channel].args.mp3StreamEnd -= 128;
	}
	sceIoLseek(streamsSceMp3[channel].file,0,PSP_SEEK_SET);
	sceIoRead(streamsSceMp3[channel].file,c,3);
	if(!strcmp(c,"ID3")) 
	{
		int tagSize;
		sceIoLseek(streamsSceMp3[channel].file,6,PSP_SEEK_SET);
		sceIoRead(streamsSceMp3[channel].file,&tagSize,4);
		ENDIAN_SWAP_32BIT(tagSize);
		ID3_TAG_LENGTH_TO_INT(tagSize);
		streamsSceMp3[channel].args.mp3StreamStart = tagSize + 10;
	}
	streamsSceMp3[channel].handle=sceMp3ReserveMp3Handle(&(streamsSceMp3[channel].args));
	if (streamsSceMp3[channel].handle<0)
	{
		sceIoClose(streamsSceMp3[channel].file);
		free(streamsSceMp3[channel].mp3Buf);
		free(streamsSceMp3[channel].pcmBuf);
		return PSPAALIB_ERROR_SCEMP3_RESERVE_HANDLE;
	}
	FillBuffer(channel);
	if (sceMp3Init(streamsSceMp3[channel].handle)<0)
	{
		sceMp3ReleaseMp3Handle(streamsSceMp3[channel].handle);
		sceIoClose(streamsSceMp3[channel].file);
		free(streamsSceMp3[channel].mp3Buf);
		free(streamsSceMp3[channel].pcmBuf);
		return PSPAALIB_ERROR_SCEMP3_INIT;
	}
	streamsSceMp3[channel].initialized=TRUE;
	streamsSceMp3[channel].paused=TRUE;
	streamsSceMp3[channel].stopReason=PSPAALIB_STOP_JUST_LOADED;
	return PSPAALIB_SUCCESS;
}

int UnloadSceMp3(int channel)
{
	if ((channel<0)||(channel>1))
	{
		return PSPAALIB_ERROR_SCEMP3_INVALID_CHANNEL;
	}
	StopSceMp3(channel);
	if (!streamsSceMp3[channel].initialized)
	{
		return PSPAALIB_SUCCESS;
	}
	sceMp3ReleaseMp3Handle(streamsSceMp3[channel].handle);
	sceIoClose(streamsSceMp3[channel].file);
	free(streamsSceMp3[channel].mp3Buf);
	free(streamsSceMp3[channel].pcmBuf);
	streamsSceMp3[channel].initialized=FALSE;
	streamsSceMp3[channel].paused=TRUE;
	streamsSceMp3[channel].stopReason=PSPAALIB_STOP_UNLOADED;
	return PSPAALIB_SUCCESS;
}

int InitSceMp3()
{
	sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC);
	sceUtilityLoadModule(PSP_MODULE_AV_MP3);
	if (sceMp3InitResource()<0)
	{
		return PSPAALIB_ERROR_SCEMP3_INIT_RESOURCE;
	}
	return PSPAALIB_SUCCESS;
}
