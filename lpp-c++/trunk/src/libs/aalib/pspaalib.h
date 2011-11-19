////////////////////////////////////////////////
//
//		pspaalib.h
//		Part of the PSP Advanced Audio Library
//		Created by Arshia001
//
//		This file includes all public function declarations 
//		for the PSPAALIB.You should only use the functions
//		which are included here.
//
//		Note that all functions return integer values
//		indicating whether they were successful.
//		A return value of 0 (PSPAALIB_SUCCESS) means
//		successful completion of a command.
//		A return value greater than 0 (PSPAALIB_ERROR_*)
//		means an error,while a return value less than 0
//		(PSPAALIB_WARNING_*) means a warning for all
//		functions except AalibGetStatus() and 
//		AalibGetStopReason().
//		
//		You can check the error codes,and compare them
//		against pspaalibcommon.h to understand what caused
//		the error.
////////////////////////////////////////////////

#ifndef _PSPAALIB_H_
#define _PSPAALIB_H_

#include "pspaalibwav.h"
#include "pspaalibscemp3.h"
#include "pspaalibcommon.h"
#include "pspaalibogg.h"
#include "pspaalibat3.h"
#include "pspaalibeffects.h"

////////////////////////////////////////////////
//		Set a stream's amplification value.
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		amplificationValue:float value indicating new
//				amplification value.All samples are 
//				multiplied by this value.
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibSetAmplification(int channel,float amplificationValue);


////////////////////////////////////////////////
//		Set a stream's volume.
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		volume:AalibVolume structure which contains the new
//				volumes.1.0f means maximum volume,0.0f means
//				mute.
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibSetVolume(int channel,AalibVolume volume);


////////////////////////////////////////////////
//		Set a stream's play speed.
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		playSpeed:float value indicating new play speed,1.0f
//				being the default speed.
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibSetPlaySpeed(int channel,float playSpeed);


////////////////////////////////////////////////
//		Set a stream's source position in 2D space.
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		position:The new position for the source.
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibSetPosition(int channel,ScePspFVector2 position);


////////////////////////////////////////////////
//		Set a stream's source velocity in 2D space.
//		Needed for Doppler effect.
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		velocity:The new velocity for the source.
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibSetVelocity(int channel,ScePspFVector2 velocity);


////////////////////////////////////////////////
//		Set the observer's velocity in 2D space.
//		Needed for Doppler effect.
//		
//		velocity:The new velocity for the observer.
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibSetObserverVelocity(ScePspFVector2 velocity);


////////////////////////////////////////////////
//		Set the observer's position in 2D space.
//		
//		position:The new position for the observer.
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibSetObserverPosition(ScePspFVector2 position);


////////////////////////////////////////////////
//		Set the direction which the observer is facing.
//		
//		front:ScePspFVector2 structure describing the
//				observer's direction.The length of the vector
//				is not important.
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibSetObserverFront(ScePspFVector2 front);


////////////////////////////////////////////////
//		Enable an effect.You must first set the required
//		values(position,velocity,volume,etc.).
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		effect:One of PSPAALIB_EFFECT_*
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibEnable(int channel,int effect);


////////////////////////////////////////////////
//		Disable an effect.
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		effect:One of PSPAALIB_EFFECT_*
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibDisable(int channel,int effect);


////////////////////////////////////////////////
//		Initalize the PSP Advanced Audio Library.
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibInit();


////////////////////////////////////////////////
//		Load an audio file and prepare it for playing.
//		
//		filename:The name of the audio file.
//		channel:One of PSPAALIB_CHANNEL_*.The channel
//				onto which the file is opened.Note that you 
//				specify a file's format using the channel number.
//				The extensions are not scanned.
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibLoad(char* filename,int channel,bool loadToRam);


////////////////////////////////////////////////
//		Unload an audio file and release all resources used.
//		
//		channel:One of PSPAALIB_CHANNEL_*.
//		
//		Returns:0 on success,>0 on error.
////////////////////////////////////////////////

int AalibUnload(int channel);


////////////////////////////////////////////////
//		Reserve a hardware channel and start playing an 
//		audio stream.
//		There are a total of 8 hardware channels,which
//		means you can't have more than 8 streams playing at
//		once.In case there are no more hardware channels
//		left,error code -5 is returned.
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibPlay(int channel);


////////////////////////////////////////////////
//		Stop playing a stream and release the hardware
//		channel.
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibStop(int channel);


////////////////////////////////////////////////
//		Pause/unpause a stream.
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibPause(int channel);


////////////////////////////////////////////////
//		Rewind a stream.
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibRewind(int channel);


////////////////////////////////////////////////
//		Set whether a stream should repeat forever.
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		autoloop:Boolean value:
//				TRUE:Enable autoloop and play until stopped.
//				FALSE:Stop when end of stream is reached.
//		
//		Returns 0 on success,>0 on error.
////////////////////////////////////////////////

int AalibSetAutoloop(int channel,bool autoloop);


////////////////////////////////////////////////
//		Retrieve value indicating why a stream stopped
//		playing.
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		
//		Returns one of PSPAALIB_STOP_*,or >0 on error.
////////////////////////////////////////////////

int AalibGetStopReason(int channel);


////////////////////////////////////////////////
//		Retrieve a channel's status.
//		
//		channel:One of PSPAALIB_CHANNEL_*
//		
//		returns one of PSPAALIB_STATUS_*,or >0 on
//		error.
////////////////////////////////////////////////

int AalibGetStatus(int channel);

#endif
