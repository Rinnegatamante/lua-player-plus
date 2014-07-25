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
 
#ifndef __MP4INFO_TYPE_H__
#define __MP4INFO_TYPE_H__

#include <pspiofilemgr.h>

#define MAX_TRACKS 1024
#define TRACK_UNKNOWN 0
#define TRACK_AUDIO   1
#define TRACK_VIDEO   2
#define TRACK_SYSTEM  3

typedef struct {
	uint32_t type;
	uint32_t video_type;
	uint32_t audio_type;
	
	uint32_t time_scale;
	uint32_t duration;
	
	uint32_t width;
	uint32_t height;
	
	uint32_t channels;
	uint32_t samplerate;
	uint32_t samplebits;
	
	uint32_t stts_entry_count;
	uint32_t *stts_sample_count;
	uint32_t *stts_sample_duration;
	
	uint32_t ctts_entry_count;
	uint32_t *ctts_sample_count;
	uint32_t *ctts_sample_offset;
	
	uint32_t stss_entry_count;
	uint32_t *stss_sync_sample;
	
	uint32_t stsc_entry_count;
	uint32_t *stsc_first_chunk;
	uint32_t *stsc_samples_per_chunk;
	uint32_t *stsc_sample_desc_id;
	
	uint32_t stsz_sample_size;
	uint32_t stsz_entry_count;
	uint32_t *stsz_sample_size_table;
	
	uint32_t stco_entry_count;
	uint32_t *stco_chunk_offset;
	
	uint32_t avc_profile;
	uint32_t avc_sps_size;
	uint8_t* avc_sps;
	uint32_t avc_pps_size;
	uint8_t* avc_pps;
	uint32_t avc_nal_prefix_size;
	
	uint32_t mp4v_decinfo_size;
	uint8_t* mp4v_decinfo;
	
} mp4info_track_t;

typedef struct {
	void* handle;
	
	int32_t time_scale;
	int32_t duration;

	int32_t total_tracks;
	mp4info_track_t* tracks[MAX_TRACKS];
	
} mp4info_t;

#endif
