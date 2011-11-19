////////////////////////////////////////////////
//
//		pspaalibscemp3.h
//		Part of the PSP Advanced Audio Library
//		Created by Arshia001
//
//		This file includes function declarations for
//		pspaalibscemp3.c.
//
////////////////////////////////////////////////

#ifndef _PSPAALIBSCEMP3_H_
#define _PSPAALIBSCEMP3_H_

#include  "pspaalibcommon.h"

bool GetPausedSceMp3(int channel);
int SetAutoloopSceMp3(int channel,bool autoloop);
int GetStopReasonSceMp3(int channel);
int PlaySceMp3(int channel);
int StopSceMp3(int channel);
int PauseSceMp3(int channel);
int RewindSceMp3(int channel);
int GetBufferSceMp3(short* buf,int length,float amp,int channel);
int LoadSceMp3(char* filename,int channel);
int UnloadSceMp3(int channel);
int InitSceMp3();

#endif
