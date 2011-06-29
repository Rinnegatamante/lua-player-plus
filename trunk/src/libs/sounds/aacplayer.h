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
extern int AAC_defaultCPUClock;

//private functions
void AAC_Init(int channel);
int AAC_Play();
void AAC_Pause();
int AAC_Stop();
void AAC_End();
void AAC_FreeTune();
int AAC_Load(char *filename);
void AAC_GetTimeString(char *dest);
int AAC_EndOfStream();
struct fileInfo AAC_GetInfo();
struct fileInfo AAC_GetTagInfoOnly(char *filename);
int AAC_GetStatus();
int AAC_GetPercentage();
void AAC_setVolumeBoostType(char *boostType);
void AAC_setVolumeBoost(int boost);
int AAC_getVolumeBoost();
int AAC_getPlayingSpeed();
int AAC_setPlayingSpeed(int playingSpeed);
int AAC_setMute(int onOff);
void AAC_fadeOut(float seconds);

//Functions for filter (equalizer):
int AAC_setFilter(double tFilter[32], int copyFilter);
void AAC_enableFilter();
void AAC_disableFilter();
int AAC_isFilterEnabled();
int AAC_isFilterSupported();

//Manage suspend:
int AAC_suspend();
int AAC_resume();
