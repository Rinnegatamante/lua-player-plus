/* 
 *	Copyright (C) 2008 cooleyes
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
 
#ifndef __MP4INFO_H__
#define __MP4INFO_H__

#include "mp4info_type.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//typedef void* mp4info_t;

mp4info_t* mp4info_open(const char* filename);
void mp4info_close(mp4info_t* info);

void mp4info_dump(mp4info_t* info, const char* dumpfile);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
