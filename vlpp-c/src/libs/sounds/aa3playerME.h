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
#define TYPE_ATRAC3 0x270
#define TYPE_ATRAC3PLUS 0xFFFE

extern int AA3ME_defaultCPUClock;

//private functions
void AA3ME_Init(int channel);
int AA3ME_Play();
void AA3ME_Pause();
int AA3ME_Stop();
void AA3ME_End();
void AA3ME_FreeTune();
int AA3ME_Load(char *filename);
void AA3ME_GetTimeString(char *dest);
int AA3ME_EndOfStream();
struct fileInfo AA3ME_GetInfo();
struct fileInfo AA3ME_GetTagInfoOnly(char *filename);
int AA3ME_GetStatus();
int AA3ME_GetPercentage();
void AA3ME_setVolumeBoostType(char *boostType);
void AA3ME_setVolumeBoost(int boost);
int AA3ME_getVolumeBoost();
int AA3ME_getPlayingSpeed();
int AA3ME_setPlayingSpeed(int playingSpeed);
int AA3ME_setMute(int onOff);
void AA3ME_fadeOut(float seconds);

//Functions for filter (equalizer):
int AA3ME_setFilter(double tFilter[32], int copyFilter);
void AA3ME_enableFilter();
void AA3ME_disableFilter();
int AA3ME_isFilterEnabled();
int AA3ME_isFilterSupported();

//Manage suspend:
int AA3ME_suspend();
int AA3ME_resume();

