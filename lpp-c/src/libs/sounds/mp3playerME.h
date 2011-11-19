//    LightMP3
//    Copyright (C) 2007 Sakya
//    sakya_tg@yahoo.it
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#include <mad.h>

extern int MP3ME_defaultCPUClock;

//private functions
void MP3ME_Init(int channel);
int MP3ME_Play();
void MP3ME_Pause();
int MP3ME_Stop();
void MP3ME_End();
void MP3ME_FreeTune();
int MP3ME_Load(char *filename);
void MP3ME_GetTimeString(char *dest);
int MP3ME_EndOfStream();
struct fileInfo MP3ME_GetInfo();
struct fileInfo MP3ME_GetTagInfoOnly(char *filename);
int MP3ME_GetStatus();
int MP3ME_GetPercentage();
void MP3ME_setVolumeBoostType(char *boostType);
void MP3ME_setVolumeBoost(int boost);
int MP3ME_getVolumeBoost();
int MP3ME_getPlayingSpeed();
int MP3ME_setPlayingSpeed(int playingSpeed);
int MP3ME_setMute(int onOff);
void MP3ME_fadeOut(float seconds);

//Functions for filter (equalizer):
int MP3ME_setFilter(double tFilter[32], int copyFilter);
void MP3ME_enableFilter();
void MP3ME_disableFilter();
int MP3ME_isFilterEnabled();
int MP3ME_isFilterSupported();

//Manage suspend:
int MP3ME_suspend();
int MP3ME_resume();

