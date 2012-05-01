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

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "util.h"
#include "mp4info_type.h"

#define ATOM_TYPE(a,b,c,d) \
	( ((uint32_t)d) | (((uint32_t)c) << 8) | (((uint32_t)b) << 16) | (((uint32_t)a) << 24) )
	

static uint32_t aac_samplerates[] = {96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000};

static uint32_t atom_get_size(uint8_t *data) {
	uint32_t result;
	uint32_t a, b, c, d;
	
	a = (uint8_t)data[0];
	b = (uint8_t)data[1];
	c = (uint8_t)data[2];
	d = (uint8_t)data[3];
	result = (a << 24) | (b << 16) | (c << 8) | d;
	
	return result;
}

static uint32_t atom_get_type(uint8_t *data) {
	uint32_t result;
	uint32_t a, b, c, d;
	
	a = (uint8_t)data[0];
	b = (uint8_t)data[1];
	c = (uint8_t)data[2];
	d = (uint8_t)data[3];
	result = (a << 24) | (b << 16) | (c << 8) | d;
	
	return result;
}

static uint64_t atom_read_header(void* handle, uint32_t *atom_type, uint32_t *header_size) {
    uint64_t size;
    uint32_t ret;
    uint8_t atom_header[8];

    ret = io_read_data(handle, atom_header, 8);
    if (ret != 8)
        return 0;

    size = atom_get_size(atom_header);
    *header_size = 8;

    if (size == 1) {
        *header_size = 16;
        size = io_read_int64(handle);
    }

    *atom_type = atom_get_type(&atom_header[4]);

    return size;
}

static void parse_unused_atom(mp4info_t* info, const uint64_t total_size) {
	io_set_position(info->handle, io_get_position(info->handle) + total_size);
}

static uint32_t read_mp4_descr_length(mp4info_t* info) {
	uint8_t b;
	uint8_t numBytes = 0;
	uint32_t length = 0;

	do {
		b = io_read_int8(info->handle);
		numBytes++;
		length = (length << 7) | (b & 0x7F);
	} while ((b & 0x80) && numBytes < 4);
	return length;
}

static void read_esds_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	io_read_int8(info->handle); //version
	io_read_int24(info->handle); //flag
	
	uint8_t tag;
	uint32_t temp;
	
	tag = io_read_int8(info->handle);
	if (tag == 0x03) {
        	if (read_mp4_descr_length(info) < 5 + 15) {
        		io_set_position(info->handle, dest_position);
        		return;
        	}
        	io_read_int24(info->handle);
	} else {
        	io_read_int16(info->handle);
	}

	if (io_read_int8(info->handle) != 0x04) {
		io_set_position(info->handle, dest_position);
        	return;
	}

	temp = read_mp4_descr_length(info);
	if (temp < 13) {
		io_set_position(info->handle, dest_position);
        	return;
	}

	io_read_int8(info->handle); //objectType
	io_read_int32(info->handle);
	io_read_int32(info->handle);//maxBitrate
	io_read_int32(info->handle);//avgBitrate

	if (io_read_int8(info->handle) != 0x05) {
		io_set_position(info->handle, dest_position);
        	return;
	}
	temp = read_mp4_descr_length(info);
	if ( info->tracks[info->total_tracks-1]->audio_type == ATOM_TYPE('m','p','4','a') ) {
		uint32_t samplerate = ((io_read_int8(info->handle) & 0x7) << 1) | (io_read_int8(info->handle)>>7);
		info->tracks[info->total_tracks-1]->samplerate = aac_samplerates[samplerate];
	}
	else if ( info->tracks[info->total_tracks-1]->video_type == ATOM_TYPE('m','p','4','v') ) {
		info->tracks[info->total_tracks-1]->mp4v_decinfo_size = temp;
		info->tracks[info->total_tracks-1]->mp4v_decinfo = (uint8_t*)malloc(info->tracks[info->total_tracks - 1]->mp4v_decinfo_size);
		if ( info->tracks[info->total_tracks-1]->mp4v_decinfo )
			io_read_data(info->handle, info->tracks[info->total_tracks - 1]->mp4v_decinfo, info->tracks[info->total_tracks - 1]->mp4v_decinfo_size);
		else
			info->tracks[info->total_tracks-1]->mp4v_decinfo_size = 0;		
	}
	
	io_set_position(info->handle, dest_position);
}

static void read_mp4a_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	int32_t i;
	for(i = 0; i < 6; i++)
		io_read_int8(info->handle); //reserved1
	io_read_int16(info->handle); //dataReferenceIndex
	io_read_int16(info->handle); //soundVersion 
	for(i = 0; i < 6; i++)
		io_read_int8(info->handle); //reserved2
	info->tracks[info->total_tracks-1]->channels = io_read_int16(info->handle);
	info->tracks[info->total_tracks-1]->samplebits = io_read_int16(info->handle);
	io_read_int16(info->handle); //packetSize 
	info->tracks[info->total_tracks-1]->samplerate = io_read_int32(info->handle);
	io_read_int16(info->handle); //reserved3
	
	uint32_t atom_type = 0;
	uint32_t header_size = 0;
	uint64_t size = 0;
	
	size = atom_read_header(info->handle, &atom_type, &header_size);
	if (atom_type == ATOM_TYPE('e','s','d','s')) {
		read_esds_atom(info, size - header_size);
	} 
	
	io_set_position(info->handle, dest_position);
}

static void read_avcC_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	io_read_int8(info->handle); //configurationVersion
	info->tracks[info->total_tracks - 1]->avc_profile = io_read_int8(info->handle); //AVCProfileIndication
	io_read_int8(info->handle); //profile_compatibility
	io_read_int8(info->handle); //AVCLevelIndication
	uint8_t value = io_read_int8(info->handle); //reserved <6bits> & lengthSizeMinusOne <2bits>
	info->tracks[info->total_tracks - 1]->avc_nal_prefix_size = (value & 0x03) + 1;
	io_read_int8(info->handle); //reserved1 <3bits> & numOfSequenceParameterSets <5bits>
    
	info->tracks[info->total_tracks - 1]->avc_sps_size = io_read_int16(info->handle);
	info->tracks[info->total_tracks - 1]->avc_sps = (uint8_t*)malloc(info->tracks[info->total_tracks - 1]->avc_sps_size);
	if ( info->tracks[info->total_tracks - 1]->avc_sps ) {
		io_read_data(info->handle, info->tracks[info->total_tracks - 1]->avc_sps, info->tracks[info->total_tracks - 1]->avc_sps_size);
		
		io_read_int8(info->handle); /* numOfPictureParameterSets */
		info->tracks[info->total_tracks - 1]->avc_pps_size = io_read_int16(info->handle);
		info->tracks[info->total_tracks - 1]->avc_pps = (uint8_t*)malloc(info->tracks[info->total_tracks - 1]->avc_pps_size);
		if ( info->tracks[info->total_tracks - 1]->avc_pps ) {
			io_read_data(info->handle, info->tracks[info->total_tracks - 1]->avc_pps, info->tracks[info->total_tracks - 1]->avc_pps_size);
		} else {
			info->tracks[info->total_tracks - 1]->avc_pps_size = 0;
		}
	} else {
		info->tracks[info->total_tracks - 1]->avc_sps_size = 0;
	}
	
	io_set_position(info->handle, dest_position);
}

static void read_avc1_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	int32_t i;
	for(i = 0; i < 6; i++)
		io_read_int8(info->handle); //reserved1
	io_read_int16(info->handle); //dataReferenceIndex
	io_read_int32(info->handle); //reserved2
	io_read_int32(info->handle); //reserved2
	io_read_int32(info->handle); //reserved2
	io_read_int32(info->handle); //reserved2
	info->tracks[info->total_tracks - 1]->width = io_read_int16(info->handle);
	info->tracks[info->total_tracks - 1]->height = io_read_int16(info->handle);
	for (i = 0; i < 14; i++) 
		io_read_int8(info->handle); //reserved3
	for (i = 0; i < 32; i++) 
		io_read_int8(info->handle); //compressorName
	io_read_int32(info->handle); //reserved4
	
	uint32_t atom_type = 0;
	uint32_t header_size = 0;
	uint64_t size = 0;
	
	size = atom_read_header(info->handle, &atom_type, &header_size);
	if (atom_type == ATOM_TYPE('a','v','c','C')) {
		read_avcC_atom(info, size - header_size);
	} 
	
	io_set_position(info->handle, dest_position);
}

static void read_mp4v_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	int32_t i;
	for(i = 0; i < 6; i++)
		io_read_int8(info->handle); //reserved1
	io_read_int16(info->handle); //dataReferenceIndex
	io_read_int32(info->handle); //reserved2
	io_read_int32(info->handle); //reserved2
	io_read_int32(info->handle); //reserved2
	io_read_int32(info->handle); //reserved2
	info->tracks[info->total_tracks - 1]->width = io_read_int16(info->handle);
	info->tracks[info->total_tracks - 1]->height = io_read_int16(info->handle);
	for (i = 0; i < 14; i++) 
		io_read_int8(info->handle); //reserved3
	for (i = 0; i < 32; i++) 
		io_read_int8(info->handle); //compressorName
	io_read_int32(info->handle); //reserved4
	
	uint32_t atom_type = 0;
	uint32_t header_size = 0;
	uint64_t size = 0;
	
	size = atom_read_header(info->handle, &atom_type, &header_size);
	if (atom_type == ATOM_TYPE('e','s','d','s')) {
		read_esds_atom(info, size - header_size);
	} 
	
	io_set_position(info->handle, dest_position);
}

static void read_stsd_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	io_read_int8(info->handle); //version
	io_read_int24(info->handle); //flags
	
	int32_t stsd_entry_count = io_read_int32(info->handle);
	int32_t i;
	
	uint32_t atom_type = 0;
	uint32_t header_size = 0;
	uint64_t size = 0;
	
	for(i = 0; i < stsd_entry_count; i++) {
		size = atom_read_header(info->handle, &atom_type, &header_size);
		if (atom_type == ATOM_TYPE('m','p','4','a')) {
			info->tracks[info->total_tracks-1]->audio_type = atom_type;
			read_mp4a_atom(info, size - header_size);
	 	}
		else if (atom_type == ATOM_TYPE('a','v','c','1')) {
			info->tracks[info->total_tracks-1]->video_type = atom_type;
			read_avc1_atom(info, size - header_size);
	 	}
	 	else if (atom_type == ATOM_TYPE('m','p','4','v')) {
			info->tracks[info->total_tracks-1]->video_type = atom_type;
			read_mp4v_atom(info, size - header_size);
	 	}
		else {
			parse_unused_atom(info, size - header_size);
		}
	}
	
	io_set_position(info->handle, dest_position);	
}

static void read_stts_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	mp4info_track_t* current_track = info->tracks[info->total_tracks-1];
	
	io_read_int8(info->handle); //version
	io_read_int24(info->handle); //flags
	
	int32_t i;
	
	current_track->stts_entry_count = io_read_int32(info->handle);
	
	current_track->stts_sample_count = (uint32_t*)malloc(current_track->stts_entry_count * sizeof(uint32_t));
	current_track->stts_sample_duration = (uint32_t*)malloc(current_track->stts_entry_count * sizeof(uint32_t));

	if ( !(current_track->stts_sample_count) || !(current_track->stts_sample_duration) ){
		if (current_track->stts_sample_count) {
			free(current_track->stts_sample_count);
			current_track->stts_sample_count = 0;
		}
		if (current_track->stts_sample_duration) {
			free(current_track->stts_sample_duration);
			current_track->stts_sample_duration = 0;
		}
		current_track->stts_entry_count = 0;
	}
	else {
		for (i = 0; i < current_track->stts_entry_count; i++) {
			current_track->stts_sample_count[i] = io_read_int32(info->handle);
			current_track->stts_sample_duration[i] = io_read_int32(info->handle);
		}
	}
	io_set_position(info->handle, dest_position);
}

static void read_ctts_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	mp4info_track_t* current_track = info->tracks[info->total_tracks-1];
	
	io_read_int8(info->handle); //version
	io_read_int24(info->handle); //flags
	
	int32_t i;
	
	current_track->ctts_entry_count = io_read_int32(info->handle);
	
	current_track->ctts_sample_count = (uint32_t*)malloc(current_track->ctts_entry_count * sizeof(uint32_t));
	current_track->ctts_sample_offset = (uint32_t*)malloc(current_track->ctts_entry_count * sizeof(uint32_t));

	if ( !(current_track->ctts_sample_count) || !(current_track->ctts_sample_offset) ){
		if (current_track->ctts_sample_count) {
			free(current_track->ctts_sample_count);
			current_track->ctts_sample_count = 0;
		}
		if (current_track->ctts_sample_offset) {
			free(current_track->ctts_sample_offset);
			current_track->ctts_sample_offset = 0;
		}
		current_track->ctts_entry_count = 0;
	}
	else {
		for (i = 0; i < current_track->ctts_entry_count; i++) {
			current_track->ctts_sample_count[i] = io_read_int32(info->handle);
			current_track->ctts_sample_offset[i] = io_read_int32(info->handle);
		}
	}
	io_set_position(info->handle, dest_position);
}

static void read_stss_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	mp4info_track_t* current_track = info->tracks[info->total_tracks-1];
	
	io_read_int8(info->handle); //version
	io_read_int24(info->handle); //flags
	
	int32_t i;
	
	current_track->stss_entry_count = io_read_int32(info->handle);
	
	current_track->stss_sync_sample = (uint32_t*)malloc(current_track->stss_entry_count * sizeof(uint32_t));
	
	if ( !(current_track->stss_sync_sample) ){
		current_track->stss_entry_count = 0;
	}
	else {
		for (i = 0; i < current_track->stss_entry_count; i++) {
			current_track->stss_sync_sample[i] = io_read_int32(info->handle);
		}
	}
	io_set_position(info->handle, dest_position);
}

static void read_stsc_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	mp4info_track_t* current_track = info->tracks[info->total_tracks-1];
	
	io_read_int8(info->handle); //version
	io_read_int24(info->handle); //flags
	
	int32_t i;
	
	current_track->stsc_entry_count = io_read_int32(info->handle);
	
	current_track->stsc_first_chunk = (uint32_t*)malloc((current_track->stsc_entry_count+1) * sizeof(uint32_t));
	current_track->stsc_samples_per_chunk = (uint32_t*)malloc((current_track->stsc_entry_count+1) * sizeof(uint32_t));
	current_track->stsc_sample_desc_id = (uint32_t*)malloc((current_track->stsc_entry_count+1) * sizeof(uint32_t));

	if ( !(current_track->stsc_first_chunk) || !(current_track->stsc_samples_per_chunk) || !(current_track->stsc_sample_desc_id) ){
		if (current_track->stsc_first_chunk) {
			free(current_track->stsc_first_chunk);
			current_track->stsc_first_chunk = 0;
		}
		if (current_track->stsc_samples_per_chunk) {
			free(current_track->stsc_samples_per_chunk);
			current_track->stsc_samples_per_chunk = 0;
		}
		if (current_track->stsc_sample_desc_id) {
			free(current_track->stsc_sample_desc_id);
			current_track->stsc_sample_desc_id = 0;
		}
		current_track->stsc_entry_count = 0;
	}
	else {
		for (i = 0; i < current_track->stsc_entry_count; i++) {
			current_track->stsc_first_chunk[i] = io_read_int32(info->handle);
			current_track->stsc_samples_per_chunk[i] = io_read_int32(info->handle);
			current_track->stsc_sample_desc_id[i] = io_read_int32(info->handle);
		}
	}
	io_set_position(info->handle, dest_position);
}

static void read_stsz_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	mp4info_track_t* current_track = info->tracks[info->total_tracks-1];
	
	io_read_int8(info->handle); //version
	io_read_int24(info->handle); //flags
	
	int32_t i;
	
	current_track->stsz_sample_size = io_read_int32(info->handle);
	current_track->stsz_entry_count = io_read_int32(info->handle);
	
	if ( current_track->stsz_sample_size == 0 ) {
	
		current_track->stsz_sample_size_table = (uint32_t*)malloc(current_track->stsz_entry_count * sizeof(uint32_t));
	
		if ( !(current_track->stsz_sample_size_table) ){
			current_track->stsz_entry_count = 0;
		}
		else {
			for (i = 0; i < current_track->stsz_entry_count; i++) {
				current_track->stsz_sample_size_table[i] = io_read_int32(info->handle);
			}
		}
	}
	io_set_position(info->handle, dest_position);
}

static void read_stco_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	mp4info_track_t* current_track = info->tracks[info->total_tracks-1];
	
	io_read_int8(info->handle); //version
	io_read_int24(info->handle); //flags
	
	int32_t i;
	
	current_track->stco_entry_count = io_read_int32(info->handle);
	
	current_track->stco_chunk_offset = (uint32_t*)malloc(current_track->stco_entry_count * sizeof(uint32_t));
	
	if ( !(current_track->stco_chunk_offset) ){
		current_track->stco_entry_count = 0;
	}
	else {
		for (i = 0; i < current_track->stco_entry_count; i++) {
			current_track->stco_chunk_offset[i] = io_read_int32(info->handle);
		}
	}
	io_set_position(info->handle, dest_position);
}

static void parse_stbl_atom(mp4info_t* info, const uint64_t total_size) {
	uint64_t current_position = 0;
	
	uint32_t atom_type = 0;
	uint32_t header_size = 0;
	uint64_t size = 0;
	
	while( current_position < total_size ) {
		size = atom_read_header(info->handle, &atom_type, &header_size);
		if (size == 0)
			break;
		if (atom_type == ATOM_TYPE('s','t','s','d')) {
			read_stsd_atom(info, size - header_size);
	 	}
	 	else if (atom_type == ATOM_TYPE('s','t','t','s')) {
			read_stts_atom(info, size - header_size);
	 	}
	 	else if (atom_type == ATOM_TYPE('c','t','t','s')) {
			read_ctts_atom(info, size - header_size);
	 	}
	 	else if (atom_type == ATOM_TYPE('s','t','s','s')) {
			read_stss_atom(info, size - header_size);
	 	}
	 	else if (atom_type == ATOM_TYPE('s','t','s','c')) {
			read_stsc_atom(info, size - header_size);
	 	}
	 	else if (atom_type == ATOM_TYPE('s','t','s','z')) {
			read_stsz_atom(info, size - header_size);
	 	}
	 	else if (atom_type == ATOM_TYPE('s','t','c','o')) {
			read_stco_atom(info, size - header_size);
	 	}
		else {
			parse_unused_atom(info, size - header_size);
		}
		current_position += size;
	}
}

static void parse_minf_atom(mp4info_t* info, const uint64_t total_size) {
	uint64_t current_position = 0;
	
	uint32_t atom_type = 0;
	uint32_t header_size = 0;
	uint64_t size = 0;
	
	while( current_position < total_size ) {
		size = atom_read_header(info->handle, &atom_type, &header_size);
		if (size == 0)
			break;
		if (atom_type == ATOM_TYPE('v','m','h','d')) {
			info->tracks[info->total_tracks-1]->type = TRACK_VIDEO;
	 		parse_unused_atom(info, size - header_size);
	 	}
	 	else if (atom_type == ATOM_TYPE('s','m','h','d')) {
	 		info->tracks[info->total_tracks-1]->type = TRACK_AUDIO;
	 		parse_unused_atom(info, size - header_size);
	 	}
	 	else if (atom_type == ATOM_TYPE('s','t','b','l')) {
	 		parse_stbl_atom(info, size - header_size);
	 	}
		else {
			parse_unused_atom(info, size - header_size);
		}
		current_position += size;
	}
}

static void read_mdhd_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	io_read_int8(info->handle); //version
	io_read_int24(info->handle); //flags
	io_read_int32(info->handle); //creationTime
	io_read_int32(info->handle); //modificationTime
	
	info->tracks[info->total_tracks-1]->time_scale = io_read_int32(info->handle);
	info->tracks[info->total_tracks-1]->duration = io_read_int32(info->handle);
		
	io_set_position(info->handle, dest_position);
}

static void parse_mdia_atom(mp4info_t* info, const uint64_t total_size) {
	uint64_t current_position = 0;
	
	uint32_t atom_type = 0;
	uint32_t header_size = 0;
	uint64_t size = 0;
	
	while( current_position < total_size ) {
		size = atom_read_header(info->handle, &atom_type, &header_size);
		if (size == 0)
			break;
		if (atom_type == ATOM_TYPE('m','d','h','d')) {
			read_mdhd_atom(info, size - header_size);
		}
		else if (atom_type == ATOM_TYPE('m','i','n','f')) {
	 		parse_minf_atom(info, size - header_size);
	 	}
		else {
			parse_unused_atom(info, size - header_size);
		}
		current_position += size;
	}
}

static void read_tkhd_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	io_read_int8(info->handle); //version
	io_read_int24(info->handle); //flags
	io_read_int32(info->handle); //creationTime
	io_read_int32(info->handle); //modificationTime
	io_read_int32(info->handle); //trackId
	io_read_int32(info->handle); //reserved1 <4 bytes>
	io_read_int32(info->handle); //duration
	int i;
	for(i = 0; i < 12; i++)
		io_read_int8(info->handle); //reserved2 <12 bytes>
	io_read_int16(info->handle); //volume 
	for(i = 0; i < 38; i++)
		io_read_int8(info->handle); //reserved3 <38 bytes>
	
	info->tracks[info->total_tracks-1]->width = io_read_int32(info->handle);
	info->tracks[info->total_tracks-1]->height = io_read_int32(info->handle);
		
	io_set_position(info->handle, dest_position);
}

static void add_track(mp4info_t* info) {
	info->total_tracks++;
	info->tracks[info->total_tracks-1] = (mp4info_track_t*)malloc(sizeof(mp4info_track_t));
	memset(info->tracks[info->total_tracks-1], 0, sizeof(mp4info_track_t));
}

static void parse_trak_atom(mp4info_t* info, const uint64_t total_size) {
	uint64_t current_position = 0;
	
	uint32_t atom_type = 0;
	uint32_t header_size = 0;
	uint64_t size = 0;
	
	while( current_position < total_size ) {
		size = atom_read_header(info->handle, &atom_type, &header_size);
		if (size == 0)
			break;
		if (atom_type == ATOM_TYPE('t','k','h','d')) {
			read_tkhd_atom(info, size - header_size);
		}
	 	else if (atom_type == ATOM_TYPE('m','d','i','a')) {
	 		parse_mdia_atom(info, size - header_size);
	 	}
		else {
			parse_unused_atom(info, size - header_size);
		}
		current_position += size;
	}
}

static void read_mvhd_atom(mp4info_t* info, const uint64_t total_size) {
	int32_t dest_position = io_get_position(info->handle) + total_size;
	
	io_read_int8(info->handle); //version
	io_read_int24(info->handle); //flags
	io_read_int32(info->handle); //creationTime
	io_read_int32(info->handle); //modificationTime
	info->time_scale = io_read_int32(info->handle); //timeScale
	info->duration = io_read_int32(info->handle); //duration
	//rate = <4 bytes>
	//volume = <2 bytes>
	//reserved1 = <70 bytes> 
   	//nextTrackId = <4 bytes>	
	io_set_position(info->handle, dest_position);
}

static void parse_moov_atom(mp4info_t* info, const uint64_t total_size) {
	uint64_t current_position = 0;
	
	uint32_t atom_type = 0;
	uint32_t header_size = 0;
	uint64_t size = 0;
	
	while( current_position < total_size ) {
		size = atom_read_header(info->handle, &atom_type, &header_size);
		if (size == 0)
			break;
		if (atom_type == ATOM_TYPE('m','v','h','d')) {
			read_mvhd_atom(info, size - header_size);
		}
	 	else if (atom_type == ATOM_TYPE('t','r','a','k')) {
	 		add_track(info);
	 		parse_trak_atom(info, size - header_size);
	 	}
		else {
			parse_unused_atom(info, size - header_size);
		}
		current_position += size;
	}
}

void parse_atoms(mp4info_t* info) {
	uint32_t atom_type = 0;
	uint32_t header_size = 0;
	uint64_t size = 0;
	
	while( (size = atom_read_header(info->handle, &atom_type, &header_size)) != 0 ) {
		if (atom_type == ATOM_TYPE('m','o','o','v') && size > header_size) {
			parse_moov_atom(info, size - header_size);
		}
		else {
			parse_unused_atom(info, size - header_size);
		}
	}
}

