////////////////////////////////////////////////
//
//		pspaalibwav.h
//		Part of the PSP Advanced Audio Library
//		Created by Arshia001
//
//		This file includes function declarations for
//		pspaalibwav.c.
//
////////////////////////////////////////////////

#ifndef _PSPAALIBWAV_H_
#define _PSPAALIBWAV_H_

#include "pspaalibcommon.h"

bool GetPausedWav(int channel);
int SetAutoloopWav(int channel,bool autoloop);
int GetStopReasonWav(int channel);
int PlayWav(int channel);
int StopWav(int channel);
int PauseWav(int channel);
int SeekWav(int time,int channel);
int RewindWav(int channel);
int GetBufferWav(short* buf,int length,float amp,int channel);
int LoadWav(char* filename,int channel,bool loadToRam);
int UnloadWav(int channel);

#endif
