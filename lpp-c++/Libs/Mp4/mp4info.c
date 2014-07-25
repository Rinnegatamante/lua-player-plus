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

#include "atom.h"
#include "util.h"
#include "mp4info_type.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

mp4info_t* mp4info_open(const char* filename) {
	mp4info_t* info = (mp4info_t*)malloc(sizeof(mp4info_t));
	if (!info)
		return 0;
	memset(info, 0, sizeof(mp4info_t));

	cache_io_t cache_io;
	cache_io.handle = sceIoOpen(filename, PSP_O_RDONLY, 0777);
	if ( !cache_io.handle ) {
		free(info);
		return 0;
	}
	cache_io.cache_first_position = 0;
	cache_io.cache_last_position = 0;
	cache_io.current_position = 0;
	cache_io.length = sceIoLseek32(cache_io.handle, 0, PSP_SEEK_END);
	io_set_position(&cache_io, 0);
	info->handle = &cache_io;
	parse_atoms(info);
	sceIoClose(cache_io.handle);
	info->handle = 0;

	int i;
	for(i = 0; i < info->total_tracks; i++) {
		mp4info_track_t* track = info->tracks[i];
		if ( track->stsc_first_chunk[track->stsc_entry_count-1] < track->stco_entry_count ) {
			track->stsc_first_chunk[track->stsc_entry_count] = track->stco_entry_count;
			track->stsc_samples_per_chunk[track->stsc_entry_count] = track->stsc_samples_per_chunk[track->stsc_entry_count-1];
			track->stsc_sample_desc_id[track->stsc_entry_count] = track->stsc_sample_desc_id[track->stsc_entry_count-1];
			track->stsc_entry_count += 1;
		}
	}

	return info;
}

void mp4info_close(mp4info_t* info) {

	int32_t i;

	for (i = 0; i < info->total_tracks; i++) {
		if (info->tracks[i]) {

			if (info->tracks[i]->stts_sample_count)
				free(info->tracks[i]->stts_sample_count);
			if (info->tracks[i]->stts_sample_duration)
				free(info->tracks[i]->stts_sample_duration);
			if (info->tracks[i]->ctts_sample_count)
				free(info->tracks[i]->ctts_sample_count);
			if (info->tracks[i]->ctts_sample_offset)
				free(info->tracks[i]->ctts_sample_offset);
			if (info->tracks[i]->stss_sync_sample)
				free(info->tracks[i]->stss_sync_sample);
			if (info->tracks[i]->stsc_first_chunk)
				free(info->tracks[i]->stsc_first_chunk);
			if (info->tracks[i]->stsc_samples_per_chunk)
				free(info->tracks[i]->stsc_samples_per_chunk);
			if (info->tracks[i]->stsc_sample_desc_id)
				free(info->tracks[i]->stsc_sample_desc_id);
			if (info->tracks[i]->stsz_sample_size_table)
				free(info->tracks[i]->stsz_sample_size_table);
			if (info->tracks[i]->stco_chunk_offset)
				free(info->tracks[i]->stco_chunk_offset);

			if (info->tracks[i]->avc_sps)
				free(info->tracks[i]->avc_sps);
			if (info->tracks[i]->avc_pps)
				free(info->tracks[i]->avc_pps);
			if (info->tracks[i]->mp4v_decinfo)
				free(info->tracks[i]->mp4v_decinfo);
			free(info->tracks[i]);
		}
	}
	if (info) free(info);
}

void mp4info_dump(mp4info_t* info, const char* dumpfile) {
	FILE* fp = fopen(dumpfile, "w+");
	fprintf(fp, "time_scale : %d(0x%08X)\n", (int)info->time_scale, (unsigned int)info->time_scale);
	fprintf(fp, "duration : %d(0x%08X)\n", (unsigned int)info->duration, (unsigned int)info->duration);
	fprintf(fp, "total_tracks : %d\n", (int)info->total_tracks);
	fprintf(fp, "\n");
	int32_t i;
	for(i = 0; i < info->total_tracks; i++) {
		fprintf(fp, "[track%d] : \n", (int)i+1);
		fprintf(fp, "\ttype : %d\n", (int)info->tracks[i]->type);

		fprintf(fp, "\ttime_scale : %d\n", (int)info->tracks[i]->time_scale);
		fprintf(fp, "\tduration : %d\n", (int)info->tracks[i]->duration);

		fprintf(fp, "\tvideo_type : 0x%08X\n", (unsigned int)info->tracks[i]->video_type);
		fprintf(fp, "\tvideo_width : %d\n", (int)info->tracks[i]->width);
		fprintf(fp, "\tvideo_height : %d\n", (int)info->tracks[i]->height);

		fprintf(fp, "\tvideo_avc_sps_size : %d\n", (int)info->tracks[i]->avc_sps_size);
		fprintf(fp, "\tvideo_avc_pps_size : %d\n", (int)info->tracks[i]->avc_pps_size);

		fprintf(fp, "\taudio_type : 0x%08X\n", (unsigned int)info->tracks[i]->audio_type);
		fprintf(fp, "\taudio_channels : %d\n", (int)info->tracks[i]->channels);
		fprintf(fp, "\taudio_samplerate : %d\n", (int)info->tracks[i]->samplerate);
		fprintf(fp, "\taudio_samplebits : %d\n", (int)info->tracks[i]->samplebits);

		fprintf(fp, "\tstts_entry_count : %d(0x%08X)\n", (int)info->tracks[i]->stts_entry_count, (unsigned int)info->tracks[i]->stts_entry_count);
		fprintf(fp, "\tctts_entry_count : %d(0x%08X)\n", (int)info->tracks[i]->ctts_entry_count, (unsigned int)info->tracks[i]->ctts_entry_count);
		fprintf(fp, "\tstss_entry_count : %d(0x%08X)\n", (int)info->tracks[i]->stss_entry_count, (unsigned int)info->tracks[i]->stss_entry_count);
		fprintf(fp, "\tstsc_entry_count : %d(0x%08X)\n", (int)info->tracks[i]->stsc_entry_count, (unsigned int)info->tracks[i]->stsc_entry_count);
		fprintf(fp, "\tstsz_entry_count : %d(0x%08X)\n", (int)info->tracks[i]->stsz_entry_count, (unsigned int)info->tracks[i]->stsz_entry_count);
		fprintf(fp, "\tstco_entry_count : %d(0x%08X)\n", (int)info->tracks[i]->stco_entry_count, (unsigned int)info->tracks[i]->stco_entry_count);

		fprintf(fp, "\n");
	}
	fclose(fp);
}
