////////////////////////////////////////////////
//
//		pspaalibeffects.c
//		Part of the PSP Advanced Audio Library
//		Created by Arshia001
//
//		This file includes functions for applying certain 
//		effects to the audio streams.The effects include
//		play speed,Doppler effect,changing the strength
//		based on how far the source is,and changing the 
//		volume of the left/right channels based on the
//		source's position to create a stereo effect in 
//		2D/3D world.If you want to use the stereo effect,
//		you should use the mix effect as well.
//
////////////////////////////////////////////////

#include "pspaalibeffects.h"

void GetBufferSpeedEffect(short* dest,short* src,int length,float speed,bool mix)
{
	int i;
	if (mix)
	{
		for (i=0;i<length;i++)
		{
			dest[2*i]=(src[2*(int)(i*speed)]+src[2*(int)(i*speed)+1])/2;
			dest[2*i+1]=dest[2*i];
		}
	}
	else
	{
		for (i=0;i<length;i++)
		{
			dest[2*i]=src[2*(int)(i*speed)];
			dest[2*i+1]=src[2*(int)(i*speed)+1];
		}
	}
	
}

float GetVectorAngle(ScePspFVector2 v)
{
	if (v.x==0)
	{
		v.x=0.000001;
	}
	return ((v.x>0)?(atan(v.y/v.x)):(atan(v.y/v.x)+PI));
}

float GetVectorLength(ScePspFVector2 v)
{
	return sqrt(pow(v.x,2)+pow(v.y,2));
}

void RotateVector(ScePspFVector2* v,float angle)
{
	float length=GetVectorLength(*v);
	float vectorAngle=GetVectorAngle(*v);
	v->x=length*cos(vectorAngle+angle);
	v->y=length*sin(vectorAngle+angle);
}

float GetClippedAngle(float angle,float clip)
{
	while (angle>clip)
	{
		angle-=2*clip;
	}
	while (angle<(-clip))
	{
		angle+=2*clip;
	}
	return angle;
}

AalibVolume GetVolumes(ScePspFVector2 sourcePosition,ScePspFVector2 observerPosition,ScePspFVector2 observerFront)
{
	AalibVolume result;
	ScePspFVector2 relativePosition={sourcePosition.x-observerPosition.x,sourcePosition.y-observerPosition.y};
	float pad=sin(GetClippedAngle(GetVectorAngle(relativePosition)-GetVectorAngle(observerFront),PI));
	result.right=((pad>0)?(MAXA(0,1-pad)):(1.0f));
	result.left=((pad<0)?(MAXA(0,1+pad)):(1.0f));
	return result;
}

float GetDopplerPlaySpeed(ScePspFVector2 sourcePosition,ScePspFVector2 sourceVelocity,ScePspFVector2 observerPosition,ScePspFVector2 observerVelocity)
{
	ScePspFVector2 relativePosition={observerPosition.x-sourcePosition.x,observerPosition.y-sourcePosition.y};
	float angle=GetVectorAngle(relativePosition);
	RotateVector(&observerVelocity,-angle);
	RotateVector(&sourceVelocity,-angle);
	return ((340-observerVelocity.x)/(340-sourceVelocity.x));
}

float GetStrengthByPosition(ScePspFVector2 sourcePosition,ScePspFVector2 observerPosition)
{
	ScePspFVector2 relativePosition={sourcePosition.x-observerPosition.x,sourcePosition.y-observerPosition.y};
	return MINA(1,sqrt(1/GetVectorLength(relativePosition)));
}
