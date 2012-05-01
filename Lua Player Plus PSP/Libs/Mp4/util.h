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
 
#ifndef __MP4INFO_UTIL_H__
#define __MP4INFO_UTIL_H__

#include <pspiofilemgr.h>

#define CACHE_BUFFER_SIZE 4096

typedef struct {
	SceUID handle;
	int32_t length;
	int32_t cache_first_position;
	int32_t cache_last_position;
	int32_t current_position;
	uint8_t cache_buffer[CACHE_BUFFER_SIZE];
} cache_io_t;

#ifdef __cplusplus
extern "C" {
#endif

int32_t io_set_position(void* handle, const int32_t position);
int32_t io_get_position(void* handle);
uint32_t io_read_data(void* handle, uint8_t* data, const uint32_t size);
uint64_t io_read_int64(void* handle);
uint32_t io_read_int32(void* handle);
uint32_t io_read_int24(void* handle);
uint16_t io_read_int16(void* handle);
uint8_t io_read_int8(void* handle);

#ifdef __cplusplus
}
#endif

#endif
