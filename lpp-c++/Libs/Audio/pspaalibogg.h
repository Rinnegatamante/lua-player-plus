////////////////////////////////////////////////
//
//		pspaalibogg.h
//		Part of the PSP Advanced Audio Library
//		Created by Arshia001
//
//		This file includes function declarations for
//		pspaalibogg.c.
//
////////////////////////////////////////////////

#ifndef _PSPAALIBOGG_H_
#define _PSPAALIBOGG_H_

#include "pspaalibcommon.h"

bool GetPausedOgg(int channel);
int SetAutoloopOgg(int channel,bool autoloop);
int GetStopReasonOgg(int channel);
int PlayOgg(int channel);
int StopOgg(int channel);
int PauseOgg(int channel);
int RewindOgg(int channel);
int SeekOgg(int channel,int time);
int GetBufferOgg(short* buf,int length,float amp,int channel);
int LoadOgg(char* filename,int channel,bool loadToRam);
int UnloadOgg(int channel);

#endif
