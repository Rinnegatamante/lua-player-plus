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

#include <stdlib.h>
#include <string.h>
#include "util.h"

//typedef struct {
//	SceUID handle;
//	int32_t length;
//	int32_t cache_first_position;
//	int32_t cache_last_position;
//	int32_t current_position;
//	uint8_t cache_buffer[CACHE_BUFFER_SIZE];
//} cache_io_t; 
 
int32_t io_set_position(void* handle, const int32_t position) {
	cache_io_t* cache_io_handle = (cache_io_t*)handle;
	if ( position >= cache_io_handle->cache_first_position && position < cache_io_handle->cache_last_position ) {
		cache_io_handle->current_position = position;
		return position;
	}
	if ( position < 0 )
		cache_io_handle->cache_first_position = sceIoLseek32(cache_io_handle->handle, 0, PSP_SEEK_SET);
	else if ( position >= cache_io_handle->length )
		cache_io_handle->cache_first_position = sceIoLseek32(cache_io_handle->handle, 0, PSP_SEEK_END);
	else
		cache_io_handle->cache_first_position = sceIoLseek32(cache_io_handle->handle, position, PSP_SEEK_SET);
	cache_io_handle->current_position = cache_io_handle->cache_first_position;
	cache_io_handle->cache_last_position = cache_io_handle->cache_first_position + CACHE_BUFFER_SIZE ;
	if ( cache_io_handle->cache_last_position >= cache_io_handle->length )
		cache_io_handle->cache_last_position = cache_io_handle->length;
	if ( cache_io_handle->cache_last_position - cache_io_handle->cache_first_position > 0 )
		sceIoRead(cache_io_handle->handle, cache_io_handle->cache_buffer, cache_io_handle->cache_last_position - cache_io_handle->cache_first_position);
	return cache_io_handle->current_position;
}

int32_t io_get_position(void* handle) {
	cache_io_t* cache_io_handle = (cache_io_t*)handle;
	return cache_io_handle->current_position;
}

uint32_t io_read_data(void* handle, uint8_t* data, uint32_t size) {
	cache_io_t* cache_io_handle = (cache_io_t*)handle;
	
	if ( cache_io_handle->current_position == cache_io_handle->length )
		return 0; 
	
	if ( size <= cache_io_handle->cache_last_position - cache_io_handle->current_position ) {
		memcpy(data, 
			cache_io_handle->cache_buffer+(cache_io_handle->current_position-cache_io_handle->cache_first_position),
			size);
		cache_io_handle->current_position+=size;
		if ( cache_io_handle->current_position == cache_io_handle->cache_last_position )
			io_set_position(handle, cache_io_handle->cache_last_position);
		return size; 
	}
	else {
		uint32_t data_size;
		memcpy(data, 
			cache_io_handle->cache_buffer+(cache_io_handle->current_position-cache_io_handle->cache_first_position),
			cache_io_handle->cache_last_position - cache_io_handle->current_position);
		data_size = (cache_io_handle->cache_last_position - cache_io_handle->current_position);
		io_set_position(cache_io_handle, cache_io_handle->cache_last_position);
		data_size += io_read_data(handle, data+data_size, size - data_size);
		return data_size;
	}
}

uint64_t io_read_int64(void* handle) {
	uint8_t data[8];
	uint64_t result = 0;
	int i;
	
	io_read_data(handle, data, 8);
	for (i = 0; i < 8; i++) {
		result |= ((uint64_t)data[i]) << ((7 - i) * 8);
	}

	return result;
}

uint32_t io_read_int32(void* handle) {
	uint8_t data[4];
	uint32_t result = 0;
	uint32_t a, b, c, d;
	
	io_read_data(handle, data, 4);
	a = (uint8_t)data[0];
	b = (uint8_t)data[1];
	c = (uint8_t)data[2];
	d = (uint8_t)data[3];
	result = (a<<24) | (b<<16) | (c<<8) | d;

	return result;
}

uint32_t io_read_int24(void* handle) {
	uint8_t data[4];
	uint32_t result = 0;
	uint32_t a, b, c;
	
	io_read_data(handle, data, 3);
	a = (uint8_t)data[0];
	b = (uint8_t)data[1];
	c = (uint8_t)data[2];
	result = (a<<16) | (b<<8) | c;

	return result;
}

uint16_t io_read_int16(void* handle) {
	uint8_t data[2];
	uint16_t result = 0;
	uint16_t a, b;
	
	io_read_data(handle, data, 2);
	a = (uint8_t)data[0];
	b = (uint8_t)data[1];
	result = (a<<8) | b;

	return result;
}

uint8_t io_read_int8(void* handle){
	uint8_t result;
    
	io_read_data(handle, &result, 1);
    
	return result;
}

