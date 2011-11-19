////////////////////////////////////////////////
//
//		pspaalibat3.h
//		Part of the PSP Advanced Audio Library
//		Created by Arshia001
//
//		This file includes function declarations for
//		pspaalibat3.c.
//
////////////////////////////////////////////////

#ifndef _PSPAALIBAT3_H_
#define _PSPAALIBAT3_H_

#include "pspaalibcommon.h"

bool GetPausedAt3(int channel);
int SetAutoloopAt3(int channel,bool autoloop);
int GetStopReasonAt3(int channel);
int PlayAt3(int channel);
int StopAt3(int channel);
int PauseAt3(int channel);
int RewindAt3(int channel);
int GetBufferAt3(short* buf,int length,float amp,int channel);
int LoadAt3(char* filename,int channel);
int UnloadAt3(int channel);
int InitAt3();

#endif
