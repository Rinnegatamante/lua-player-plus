#include "mp4_read.h"


void mp4_read_safe_constructor(struct mp4_read_struct *p) {
	mp4_file_safe_constructor(&p->file);

	p->video_handle = -1;
	p->audio_handle = -1;
	
	p->current_audio_track = 0;
	
	p->video_buffer_0 = 0;
	p->video_buffer_1 = 0;
	p->audio_buffer_0 = 0;
	p->audio_buffer_1 = 0;
	p->audio_cache_buffer = 0;
}


void mp4_read_close(struct mp4_read_struct *p) {
	
	mp4_file_close(&p->file);

	if (!(p->video_handle < 0))
		sceIoClose(p->video_handle);
	if (!(p->audio_handle < 0))
		sceIoClose(p->audio_handle);
	
	if (p->video_buffer_0 != 0)
		free_64(p->video_buffer_0);
	if (p->video_buffer_1 != 0)
		free_64(p->video_buffer_1);
	if (p->audio_buffer_0 != 0)
		free_64(p->audio_buffer_0);
	if (p->audio_buffer_1 != 0)
		free_64(p->audio_buffer_1);
	
	if (p->audio_cache_buffer != 0)
		free_64(p->audio_cache_buffer);
	
	mp4_read_safe_constructor(p);
}


static char *fill_asynchronous_buffer(struct mp4_read_struct *reader, struct mp4_asynchronous_buffer *p, SceUID handle, int track_id, unsigned int trunk_index) {
	 
	mp4info_track_t* track = reader->file.info->tracks[track_id];
	
	int i, j;
	for( i = 0; i < track->stsc_entry_count-1; i++ ) {
		if ( (trunk_index+1) >= track->stsc_first_chunk[i] && (trunk_index+1) < track->stsc_first_chunk[i+1] )
			break;
	}
	p->first_sample = 0;
	for( j = 0; j < i; j++ ) {
		p->first_sample += ( ( track->stsc_first_chunk[j+1] - track->stsc_first_chunk[j] ) * track->stsc_samples_per_chunk[j] );
	}
	p->first_sample += ( ( (trunk_index+1) - track->stsc_first_chunk[i] ) * track->stsc_samples_per_chunk[i] );
	p->last_sample = p->first_sample + track->stsc_samples_per_chunk[i] - 1;
	
	p->trunk_size = 0;
	
	for(i = p->first_sample; i <= p->last_sample; i++) { 
		p->sample_buffer[i-p->first_sample] = p->buffer + p->trunk_size;
		p->trunk_size += ( track->stsz_sample_size ? track->stsz_sample_size : track->stsz_sample_size_table[i]);
	}
	p->trunk_position = track->stco_chunk_offset[trunk_index];
	p->trunk_index = trunk_index;

	if (sceIoLseek32(handle, p->trunk_position, PSP_SEEK_SET) != p->trunk_position) {
		return("fill_asynchronous_buffer: seek failed");
	}


	if (sceIoReadAsync(handle, p->sample_buffer[0], p->trunk_size) < 0) {
		return("fill_asynchronous_buffer: read failed");
	}

	return(0);
}


static char *wait_asynchronous_buffer(struct mp4_read_struct *reader, struct mp4_asynchronous_buffer *p, SceUID handle) {
	long long result;

	if (sceIoWaitAsync(handle, &result) < 0) {
		return("wait_asynchronous_buffer: wait failed");
	}


	if (p->trunk_size != result) {
		return("wait_asynchronous_buffer: read failed");
	}


	return(0);
}


static char *fill_and_wait_asynchronous_buffer(struct mp4_read_struct *reader, struct mp4_asynchronous_buffer *p, SceUID handle, int track_id, unsigned int trunk_index) {
	char *result = fill_asynchronous_buffer(reader, p, handle, track_id, trunk_index);
	if (result != 0) {
		return(result);
	}

	result = wait_asynchronous_buffer(reader, p, handle);
	if (result != 0) {
		return(result);
	}

	return(0);
}


static char *video_fill_next_asynchronous_buffer(struct mp4_read_struct *reader) {
	
	unsigned int trunk_index = reader->video_current_asynchronous_buffer->trunk_index + 1;
	if (trunk_index == reader->file.info->tracks[reader->file.video_track_id]->stco_entry_count) {
		trunk_index = 0;
	}

	char *result = fill_asynchronous_buffer(
		reader, reader->video_next_asynchronous_buffer, 
		reader->video_handle, 
		reader->file.video_track_id, 
		trunk_index);
	if (result != 0) {
		return(result);
	}

	return(0);
}


static char *video_fill_current_and_next_asynchronous_buffer(struct mp4_read_struct *reader, unsigned int trunk_index) {
	char *result = fill_and_wait_asynchronous_buffer(reader, 
		reader->video_current_asynchronous_buffer, 
		reader->video_handle, 
		reader->file.video_track_id, 
		trunk_index);
	if (result != 0) {
		return(result);
	}

	result = video_fill_next_asynchronous_buffer(reader);
	if (result != 0) {
		return(result);
	}

	return(0);
}

static void video_swap_asynchronous_buffers(struct mp4_read_struct *reader) {
	struct mp4_asynchronous_buffer *swap    = reader->video_current_asynchronous_buffer;
	reader->video_current_asynchronous_buffer = reader->video_next_asynchronous_buffer;
	reader->video_next_asynchronous_buffer    = swap;
}

static char *audio_fill_next_asynchronous_buffer(struct mp4_read_struct *reader) {
	
	unsigned int trunk_index = reader->audio_current_asynchronous_buffer->trunk_index + 1;
	if (trunk_index == reader->file.info->tracks[reader->file.audio_track_ids[reader->current_audio_track]]->stco_entry_count) {
		trunk_index = 0;
	}

	char *result = fill_asynchronous_buffer(
		reader, reader->audio_next_asynchronous_buffer, 
		reader->audio_handle, 
		reader->file.audio_track_ids[reader->current_audio_track], 
		trunk_index);
	if (result != 0) {
		return(result);
	}

	return(0);
}


static char *audio_fill_current_and_next_asynchronous_buffer(struct mp4_read_struct *reader, unsigned int trunk_index) {
	char *result = fill_and_wait_asynchronous_buffer(reader, 
		reader->audio_current_asynchronous_buffer, 
		reader->audio_handle, 
		reader->file.audio_track_ids[reader->current_audio_track], 
		trunk_index);
	if (result != 0) {
		return(result);
	}

	result = audio_fill_next_asynchronous_buffer(reader);
	if (result != 0) {
		return(result);
	}

	return(0);
}

static void audio_swap_asynchronous_buffers(struct mp4_read_struct *reader) {
	struct mp4_asynchronous_buffer *swap      = reader->audio_current_asynchronous_buffer;//struct asynchronous_buffer *
	reader->audio_current_asynchronous_buffer = reader->audio_next_asynchronous_buffer;
	reader->audio_next_asynchronous_buffer    = swap;
}

char *mp4_read_open(struct mp4_read_struct *p, char *s) {
	
	mp4_read_safe_constructor(p);
	char *result = mp4_file_open(&p->file, s);
	if (result != 0) {
		mp4_read_close(p);
		return(result);
	}


	p->video_handle = sceIoOpen(s, PSP_O_RDONLY, 0777);
	if (p->video_handle < 0) {
		mp4_read_close(p);
		return("mp4_read_open: can't open file");
	}

	if (sceIoChangeAsyncPriority(p->video_handle, 0x10) < 0) {
		mp4_read_close(p);
		return("mp4_read_open: sceIoChangeAsyncPriority failed");
	}
	
	p->audio_handle = sceIoOpen(s, PSP_O_RDONLY, 0777);
	if (p->audio_handle < 0) {
		mp4_read_close(p);
		return("mp4_read_open: can't open file");
	}

	if (sceIoChangeAsyncPriority(p->audio_handle, 0x10) < 0) {
		mp4_read_close(p);
		return("mp4_read_open: sceIoChangeAsyncPriority failed");
	}


	p->video_buffer_0 = malloc_64(p->file.maximum_video_trunk_size);
	if (p->video_buffer_0 == 0) {
		mp4_read_close(p);
		return("mp4_read_open: malloc_64 failed on buffer_0");
	}
	memset(p->video_buffer_0, 0, p->file.maximum_video_trunk_size);

	p->video_buffer_1 = malloc_64(p->file.maximum_video_trunk_size);
	if (p->video_buffer_1 == 0) {
		mp4_read_close(p);
		return("mp4_read_open: malloc_64 failed on buffer_1");
	}
	memset(p->video_buffer_1, 0, p->file.maximum_video_trunk_size);
	
	p->audio_buffer_0 = malloc_64(p->file.maximum_audio_trunk_size);
	if (p->audio_buffer_0== 0) {
		mp4_read_close(p);
		return("mp4_read_open: malloc_64 failed on buffer_0");
	}
	memset(p->audio_buffer_0, 0, p->file.maximum_audio_trunk_size);
	
	p->audio_buffer_1 = malloc_64(p->file.maximum_audio_trunk_size);
	if (p->audio_buffer_1== 0) {
		mp4_read_close(p);
		return("mp4_read_open: malloc_64 failed on buffer_0");
	}
	memset(p->audio_buffer_1, 0, p->file.maximum_audio_trunk_size);
	
	p->audio_cache_buffer = malloc_64((sizeof(unsigned int)+p->file.maximum_audio_sample_size) * p->file.maximun_audio_sample_number);
	if (p->audio_cache_buffer== 0) {
		mp4_read_close(p);
	}
	memset(p->audio_cache_buffer, 0, (sizeof(unsigned int)+p->file.maximum_audio_sample_size) * p->file.maximun_audio_sample_number);
	p->audio_output_length = (unsigned int*)p->audio_cache_buffer;
	p->audio_output_buffer = p->audio_cache_buffer + sizeof(unsigned int)*p->file.maximun_audio_sample_number;

	p->video_asynchronous_buffer_0.buffer = p->video_buffer_0;
	p->video_asynchronous_buffer_1.buffer = p->video_buffer_1;

	p->video_current_asynchronous_buffer  = &p->video_asynchronous_buffer_0;
	p->video_next_asynchronous_buffer     = &p->video_asynchronous_buffer_1;
	
	p->audio_asynchronous_buffer_0.buffer = p->audio_buffer_0;
	p->audio_asynchronous_buffer_1.buffer = p->audio_buffer_1;

	p->audio_current_asynchronous_buffer  = &p->audio_asynchronous_buffer_0;
	p->audio_next_asynchronous_buffer     = &p->audio_asynchronous_buffer_1;
	
	time_math_interleaving_constructor(&p->interleaving, 
		p->file.video_rate, 
		p->file.video_scale, 
		p->file.audio_rate, 
		p->file.audio_resample_scale);
	time_math_interleaving_get(&p->interleaving);


	result = video_fill_current_and_next_asynchronous_buffer(p, 0);
	if (result != 0) {
		mp4_read_close(p);
		return(result);
	}
	result = audio_fill_current_and_next_asynchronous_buffer(p, 0);
	if (result != 0) {
		mp4_read_close(p);
		return(result);
	}

	return(0);
}

static unsigned int find_trunk_index_by_sample(struct mp4_read_struct *reader, int track_id, unsigned int sample) {
	mp4info_track_t* track = reader->file.info->tracks[track_id];
	int k;
	for( k = 0; k < track->stco_entry_count; k++ ) {
		
		int i, j;
		unsigned int first_sample, last_sample;
		for( i = 0; i < track->stsc_entry_count-1; i++ ) {
			if ( (k+1) >= track->stsc_first_chunk[i] && (k+1) < track->stsc_first_chunk[i+1] )
				break;
		}
		first_sample = 0;
		for( j = 0; j < i; j++ ) {
			first_sample += ( ( track->stsc_first_chunk[j+1] - track->stsc_first_chunk[j] ) * track->stsc_samples_per_chunk[j] );
		}
		first_sample += ( ( (k+1) - track->stsc_first_chunk[i] ) * track->stsc_samples_per_chunk[i] );
		last_sample = first_sample + track->stsc_samples_per_chunk[i] - 1;
		
		if ( sample >= first_sample && sample <= last_sample )
			return k;
	}
	return 0;
}


static char *get_video_sample(struct mp4_read_struct *reader, unsigned int sample, void **buffer) {
	if(sample >= reader->video_current_asynchronous_buffer->first_sample && sample <= reader->video_current_asynchronous_buffer->last_sample){//if sample is in the buffer
		*buffer = reader->video_current_asynchronous_buffer->sample_buffer[sample - reader->video_current_asynchronous_buffer->first_sample];
	}else{
		char *result = wait_asynchronous_buffer(reader, reader->video_next_asynchronous_buffer, reader->video_handle);
		if (result != 0) {
			return(result);
		}

		if (sample >= reader->video_next_asynchronous_buffer->first_sample && sample <= reader->video_next_asynchronous_buffer->last_sample) {
			video_swap_asynchronous_buffers(reader);

			result = video_fill_next_asynchronous_buffer(reader);
			if (result != 0) {
				return(result);
			}

			*buffer = reader->video_current_asynchronous_buffer->sample_buffer[sample - reader->video_current_asynchronous_buffer->first_sample];
		}
		else {
			unsigned int trunk_index;
			trunk_index = find_trunk_index_by_sample(reader, reader->file.video_track_id, sample);
			result = video_fill_current_and_next_asynchronous_buffer(reader, trunk_index);
			if (result != 0){
				return(result);
			}
			*buffer = reader->video_current_asynchronous_buffer->sample_buffer[sample - reader->video_current_asynchronous_buffer->first_sample];
		}
	}

	return(0);
}

static char *get_audio_sample(struct mp4_read_struct *reader, unsigned int track_index, unsigned int sample, void **buffer) {
	char *result = 0;
	if ( track_index != reader->current_audio_track ) {
		result = wait_asynchronous_buffer(reader, reader->audio_next_asynchronous_buffer, reader->audio_handle);
		if (result != 0) {
			return(result);
		}
		reader->current_audio_track = track_index;
		unsigned int trunk_index ;
		trunk_index = find_trunk_index_by_sample(reader, reader->file.audio_track_ids[reader->current_audio_track], sample);
		result = audio_fill_current_and_next_asynchronous_buffer(reader, trunk_index);
		if (result != 0){
			return(result);
		}
		*buffer = reader->audio_current_asynchronous_buffer->sample_buffer[sample - reader->audio_current_asynchronous_buffer->first_sample];
	}	
	else if (sample >= reader->audio_current_asynchronous_buffer->first_sample && sample <= reader->audio_current_asynchronous_buffer->last_sample) {
		*buffer = reader->audio_current_asynchronous_buffer->sample_buffer[sample - reader->audio_current_asynchronous_buffer->first_sample];
	}
	else {
		result = wait_asynchronous_buffer(reader, reader->audio_next_asynchronous_buffer, reader->audio_handle);
		if (result != 0) {
			return(result);
		}

		if (sample >= reader->audio_next_asynchronous_buffer->first_sample && sample <= reader->audio_next_asynchronous_buffer->last_sample) {
			audio_swap_asynchronous_buffers(reader);

			result = audio_fill_next_asynchronous_buffer(reader);
			if (result != 0) {
				return(result);
			}

			*buffer = reader->audio_current_asynchronous_buffer->sample_buffer[sample - reader->audio_current_asynchronous_buffer->first_sample];
		}
		else {
			unsigned int trunk_index;
			trunk_index = find_trunk_index_by_sample(reader, reader->file.audio_track_ids[reader->current_audio_track], sample);
			result = audio_fill_current_and_next_asynchronous_buffer(reader, trunk_index);
			if (result != 0){
				return(result);
			}
			*buffer = reader->audio_current_asynchronous_buffer->sample_buffer[sample - reader->audio_current_asynchronous_buffer->first_sample];
		}
	}

	return(0);
}


char *mp4_read_get(struct mp4_read_struct *p, unsigned int packet, unsigned int audio_stream, struct mp4_read_output_struct *output) {
	
	void *video_buffer;

	char *result = get_video_sample(p, packet, &video_buffer);
	if (result != 0) {
		return(result);
	}
	
	if (p->interleaving.output_video_frame_number > packet) {
		time_math_interleaving_constructor(&p->interleaving, 
			p->file.video_rate, 
			p->file.video_scale, 
			p->file.audio_rate, 
			p->file.audio_resample_scale);
		time_math_interleaving_get(&p->interleaving);
	}

	while (p->interleaving.output_video_frame_number != packet) {
		time_math_interleaving_get(&p->interleaving);
	}
	mp4info_track_t* track;
	
	unsigned int first_audio_frame, last_audio_frame;
	if ( p->file.audio_double_sample ) {
		first_audio_frame = p->interleaving.output_audio_frame_number / 2;
		last_audio_frame = (p->interleaving.output_audio_frame_number + p->interleaving.output_number_of_audio_frames - 1) / 2;
		output->number_of_audio_frames = last_audio_frame - first_audio_frame + 1;
		output->number_of_skip_audio_parts = p->interleaving.output_audio_frame_number % 2;
		output->number_of_audio_parts = p->interleaving.output_number_of_audio_frames;
	}
	else {
		first_audio_frame = p->interleaving.output_audio_frame_number;
		last_audio_frame = p->interleaving.output_audio_frame_number + p->interleaving.output_number_of_audio_frames - 1;
		output->number_of_audio_frames = p->interleaving.output_number_of_audio_frames;
		output->number_of_skip_audio_parts = 0;
		output->number_of_audio_parts = 0;
	}
	track = p->file.info->tracks[p->file.audio_track_ids[audio_stream]];
	int i;
	void *audio_buffer;
	void *audio_output_buffer = p->audio_output_buffer;
	memset(audio_output_buffer, 0, p->file.maximum_audio_sample_size * p->file.maximun_audio_sample_number);
	for( i = 0; i < output->number_of_audio_frames; i++ ) {
		if ( first_audio_frame+i >= track->stts_sample_count[0] )
			break; 
		p->audio_output_length[i] = track->stsz_sample_size ? track->stsz_sample_size : track->stsz_sample_size_table[first_audio_frame+i];
		result = get_audio_sample(p, audio_stream, first_audio_frame+i, &audio_buffer);
		if (result != 0) {
			return(result);
		}
		memcpy(audio_output_buffer, audio_buffer, p->audio_output_length[i]);
		audio_output_buffer += p->audio_output_length[i];
	}
	
	track = p->file.info->tracks[p->file.video_track_id];
	output->first_delay  = p->interleaving.output_first_delay;
	output->last_delay   = p->interleaving.output_last_delay;
	output->video_length = track->stsz_sample_size ? track->stsz_sample_size : track->stsz_sample_size_table[packet];
	output->audio_length = p->audio_output_length;
	output->video_buffer = video_buffer;
	output->audio_buffer = p->audio_output_buffer;

	return(0);
}

char *mp4_read_get_video(struct mp4_read_struct *p, unsigned int packet, struct mp4_video_read_output_struct *output) {
	
	void *video_buffer;

	char *result = get_video_sample(p, packet, &video_buffer);
	if (result != 0) {
		return(result);
	}
	mp4info_track_t* track;
	track = p->file.info->tracks[p->file.video_track_id];
	output->video_length = track->stsz_sample_size ? track->stsz_sample_size : track->stsz_sample_size_table[packet];
	output->video_buffer = video_buffer;
	
	return(0);
}

char *mp4_read_get_audio(struct mp4_read_struct *p, unsigned int packet, unsigned int audio_stream, struct mp4_audio_read_output_struct *output){
	char *result = 0;
	if (p->interleaving.output_video_frame_number > packet) {
		time_math_interleaving_constructor(&p->interleaving, 
			p->file.video_rate, 
			p->file.video_scale, 
			p->file.audio_rate, 
			p->file.audio_resample_scale);
		time_math_interleaving_get(&p->interleaving);
	}

	while (p->interleaving.output_video_frame_number != packet) {
		time_math_interleaving_get(&p->interleaving);
	}
	mp4info_track_t* track;
	
	unsigned int first_audio_frame, last_audio_frame;
	
	first_audio_frame = p->interleaving.output_audio_frame_number;
	last_audio_frame = p->interleaving.output_audio_frame_number + p->interleaving.output_number_of_audio_frames - 1;
	output->number_of_audio_frames = p->interleaving.output_number_of_audio_frames;
	
	track = p->file.info->tracks[p->file.audio_track_ids[audio_stream]];
	int i;
	void *audio_buffer;
	void *audio_output_buffer = p->audio_output_buffer;
	memset(audio_output_buffer, 0, p->file.maximum_audio_sample_size * p->file.maximun_audio_sample_number);
	for( i = 0; i < output->number_of_audio_frames; i++ ) {
		if ( first_audio_frame+i >= track->stts_sample_count[0] )
			break; 
		p->audio_output_length[i] = track->stsz_sample_size ? track->stsz_sample_size : track->stsz_sample_size_table[first_audio_frame+i];
		result = get_audio_sample(p, audio_stream, first_audio_frame+i, &audio_buffer);
		if (result != 0) {
			return(result);
		}
		memcpy(audio_output_buffer, audio_buffer, p->audio_output_length[i]);
		audio_output_buffer += p->audio_output_length[i];
	}
	
	output->first_delay  = p->interleaving.output_first_delay;
	output->last_delay   = p->interleaving.output_last_delay;
	output->audio_length = p->audio_output_length;
	output->audio_buffer = p->audio_output_buffer;

	return(0);
}
