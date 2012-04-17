////////////////////////////////////////////////
//
//		pspaalibeffects.h
//		Part of the PSP Advanced Audio Library
//		Created by Arshia001
//
//		This file includes function declarations for 
//		pspaalibeffects.c.
//
////////////////////////////////////////////////

#ifndef _PSPAALIBEFFECTS_H_
#define _PSPAALIBEFFECTS_H_

#include "pspaalibcommon.h"

#define sgn(a) ((a>0)?(1):(-1))

void GetBufferSpeedEffect(short* dest,short* src,int length,float speed,bool mix);
AalibVolume GetVolumes(ScePspFVector2 sourcePosition,ScePspFVector2 observerPosition,ScePspFVector2 observerFront);
float GetDopplerPlaySpeed(ScePspFVector2 sourcePosition,ScePspFVector2 sourceVelocity,ScePspFVector2 observerPosition,ScePspFVector2 observerVelocity);
float GetStrengthByPosition(ScePspFVector2 sourcePosition,ScePspFVector2 observerPosition);

#endif
