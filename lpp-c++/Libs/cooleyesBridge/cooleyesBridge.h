/* 
 *	Copyright (C) 2007 cooleyes
 *	eyes.cooleyes@gmail.com 
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
#ifndef COOLEYESBRIDGE_H
#define COOLEYESBRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
  * Set audio sampling frequency
  *
  * @param frequency - Sampling frequency to set audio output to - either 44100 or 48000.
  *
  * @returns 0 on success, an error if less than 0.
  */
int cooleyesAudioSetFrequency(int devkitVersion, int frequency);

int cooleyesMeBootStart(int devkitVersion, int mebooterType);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* COOLEYESBRIDGE_H */
