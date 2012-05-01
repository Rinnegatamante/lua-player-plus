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
 
#include "mp4_file.h"
#include "time_math.h"

void mp4_file_safe_constructor(struct mp4_file_struct *p) {
	p->info = 0;
	p->video_track_id = -1;
	p->audio_tracks = 0;
	p->audio_double_sample = 0;
	p->maximum_video_sample_size = 0;
	p->maximum_video_trunk_size = 0;
	p->maximum_audio_trunk_size = 0;
	p->maximum_audio_sample_size = 0;
}


void mp4_file_close(struct mp4_file_struct *p) {
	if (p->info != 0) {
		mp4info_close(p->info);
	}
	mp4_file_safe_constructor(p);
}


static void minimum_and_maximum_number_of_audio_frames_get(struct mp4_file_struct *p, unsigned int *minimum, unsigned int *maximum) {
	struct time_math_interleaving_struct t;

	time_math_interleaving_constructor(&t, 
		p->video_rate, 
		p->video_scale, 
		p->audio_rate, 
		p->audio_resample_scale);

	unsigned int video_frame_number = 0;
	while (video_frame_number < p->number_of_video_frames) {
		time_math_interleaving_get(&t);

		if (video_frame_number == 0) {
			*minimum = t.output_number_of_audio_frames;
			*maximum = t.output_number_of_audio_frames;
		}
		else {
			if (t.output_number_of_audio_frames < *minimum) *minimum = t.output_number_of_audio_frames;
			if (t.output_number_of_audio_frames > *maximum) *maximum = t.output_number_of_audio_frames;
		}
		video_frame_number++;
	}
}

	

char *mp4_file_open(struct mp4_file_struct *p, char *s) {
	mp4_file_safe_constructor(p);
	
	p->info = mp4info_open(s);
	
	if (p->info == 0){
		mp4_file_close(p);
		return("mp4_file_open: can't open file");
	}
	
	int i;
	
	for(i = 0; i < p->info->total_tracks; i++) {
		mp4info_track_t* track = p->info->tracks[i];
		if (track->type != TRACK_VIDEO)
			continue;
//		if ( track->video_type != 0x61766331 /*avc1*/)
//			continue; 
		if ( track->width < 1 || track->height < 1 )
			continue;
//		if ( track->width > 480 || track->height > 320 )
//			continue;
		p->video_track_id = i;
		p->video_type = track->video_type;
		break;
	}
	if ( p->video_track_id < 0 ) {
		mp4_file_close(p);
		return("mp4_file_open: can't found video track in mp4 file");
	} 
	
	for(i = 0; i < p->info->total_tracks; i++) {
		mp4info_track_t* track = p->info->tracks[i];
		if (track->type != TRACK_AUDIO)
			continue;
		if ( p->audio_tracks == 0 ) {
			if ( track->audio_type != 0x6D703461 /*mp4a*/)
				continue;
//			if ( track->channels != 2 )
//				continue;
			if ( track->samplerate != 22050 && track->samplerate != 24000 && track->samplerate != 44100 && track->samplerate != 48000 )
				continue;
			if ( track->samplebits != 16 )
				continue;
			p->audio_tracks++;
			p->audio_track_ids[p->audio_tracks-1] = i;
			p->audio_type = track->audio_type;
			if ( track->samplerate == 22050 || track->samplerate == 24000 )
				p->audio_double_sample = 1;
		}
		else {
			mp4info_track_t* old_track = p->info->tracks[p->audio_track_ids[p->audio_tracks-1]];
			if ( old_track->audio_type != track->audio_type )
				continue;
//			if ( old_track->channels != track->channels )
//				continue;
			if ( old_track->samplerate != track->samplerate )
				continue;
			if ( old_track->samplebits != track->samplebits )
				continue;
			p->audio_tracks++;
			p->audio_track_ids[p->audio_tracks-1] = i;
		}
		if ( p->audio_tracks == 6 )
			break;
	}
	if ( p->audio_tracks == 0 ) {
		mp4_file_close(p);
		return("mp4_file_open: can't found audio track in mp4 file");
	}
	
	int sample_id = 0;
	unsigned int trunk_size = 0;
	int j, k;
	
	mp4info_track_t* video_track = p->info->tracks[p->video_track_id];
	for( i = 0; i < video_track->stsc_entry_count-1; i++ ) {
		int trunk_num = video_track->stsc_first_chunk[i+1] - video_track->stsc_first_chunk[i];
		for( j = 0; j < trunk_num; j++ ) {
			trunk_size = 0;
			for( k = 0; k < video_track->stsc_samples_per_chunk[i]; k++, sample_id++) {
				unsigned int sample_size = (video_track->stsz_sample_size ? video_track->stsz_sample_size : video_track->stsz_sample_size_table[sample_id]);
				if ( sample_size > p->maximum_video_sample_size )
					p->maximum_video_sample_size = sample_size;
				trunk_size += sample_size;
			}
			if ( trunk_size > p->maximum_video_trunk_size )
				p->maximum_video_trunk_size = trunk_size;
		}
	}
	trunk_size = 0;
	for( k = 0; k < video_track->stsc_samples_per_chunk[i]; k++, sample_id++)
		trunk_size += (video_track->stsz_sample_size ? video_track->stsz_sample_size : video_track->stsz_sample_size_table[sample_id]);	
	if ( trunk_size > p->maximum_video_trunk_size )
		p->maximum_video_trunk_size = trunk_size;
		
	int l;
	for( l = 0; l < p->audio_tracks; l++ ) {
		sample_id = 0;
		mp4info_track_t* audio_track = p->info->tracks[p->audio_track_ids[l]];
		for( i = 0; i < audio_track->stsc_entry_count-1; i++ ) {
			int trunk_num = audio_track->stsc_first_chunk[i+1] - audio_track->stsc_first_chunk[i];
			for( j = 0; j < trunk_num; j++ ) {
				trunk_size = 0;
				for( k = 0; k < audio_track->stsc_samples_per_chunk[i]; k++, sample_id++) {
					unsigned int sample_size = (audio_track->stsz_sample_size ? audio_track->stsz_sample_size : audio_track->stsz_sample_size_table[sample_id]);
					if ( sample_size > p->maximum_audio_sample_size )
						p->maximum_audio_sample_size = sample_size;
					trunk_size += sample_size;
				}
				if ( trunk_size > p->maximum_audio_trunk_size )
					p->maximum_audio_trunk_size = trunk_size;
			}
		}
		trunk_size = 0;
		for( k = 0; k < audio_track->stsc_samples_per_chunk[i]; k++, sample_id++) {
			unsigned int sample_size = (audio_track->stsz_sample_size ? audio_track->stsz_sample_size : audio_track->stsz_sample_size_table[sample_id]);
			if ( sample_size > p->maximum_audio_sample_size )
				p->maximum_audio_sample_size = sample_size;
			trunk_size += sample_size;
		}
		if ( trunk_size > p->maximum_audio_trunk_size )
			p->maximum_audio_trunk_size = trunk_size;
	}
	
	p->video_width = p->info->tracks[p->video_track_id]->width;
	p->video_height = p->info->tracks[p->video_track_id]->height;
	p->number_of_video_frames = 0;
	for( i = 0; i < p->info->tracks[p->video_track_id]->stts_entry_count; i++)
		p->number_of_video_frames += p->info->tracks[p->video_track_id]->stts_sample_count[i];
	//p->number_of_video_frames = p->info->tracks[p->video_track_id]->stts_sample_count[0];
	p->video_rate = p->info->tracks[p->video_track_id]->time_scale;
	p->video_scale = p->info->tracks[p->video_track_id]->duration / p->number_of_video_frames;
	//p->video_scale = p->info->tracks[p->video_track_id]->stts_sample_duration[0];
	
	p->audio_actual_rate = p->info->tracks[p->audio_track_ids[0]]->samplerate;
	p->audio_rate = p->audio_actual_rate * (p->audio_double_sample?2:1) ;
	p->audio_scale = (p->audio_type == 0x6D703461 ? 1024 : 1024);
	p->audio_resample_scale = p->audio_scale * (p->audio_double_sample?2:1);
	//p->audio_scale = p->info->tracks[p->audio_track_ids[0]]->stts_sample_duration[0] / (p->audio_double_sample?2:1);
	p->audio_stereo = 1;//(p->info->tracks[p->audio_track_ids[0]]->channels == 2 ? 1 : 0);
	
	unsigned int minimum_number_of_audio_frames = 0;
	unsigned int maximum_number_of_audio_frames = 0;
	minimum_and_maximum_number_of_audio_frames_get(p, &minimum_number_of_audio_frames, &maximum_number_of_audio_frames);
	
	p->maximun_audio_sample_number = maximum_number_of_audio_frames;
	
	return(0);
}

