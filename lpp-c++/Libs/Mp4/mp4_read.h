#ifndef __MP4_READ_H__
#define __MP4_READ_H__


#include <string.h>
#include <pspiofilemgr.h>
#include "mp4_file.h"
#include "time_math.h"
#include "../mem64/mem64.h"

#define MAXIMUM_SAMPLES_PER_TRUNK 128

struct mp4_asynchronous_buffer {
	unsigned int  first_sample;
	unsigned int  last_sample;
	unsigned int  trunk_size;
	unsigned int  trunk_position;
	unsigned int  trunk_index;
	void         *buffer;
	void         *sample_buffer[MAXIMUM_SAMPLES_PER_TRUNK];
};


struct mp4_read_struct {
	struct mp4_file_struct file;
	
	struct time_math_interleaving_struct interleaving;

	SceUID video_handle;
	SceUID audio_handle;
	
	int current_audio_track;
	
	void         *video_buffer_0;
	void         *video_buffer_1;
	void         *audio_buffer_0;
	void         *audio_buffer_1;
	
	void         *audio_cache_buffer;
	void         *audio_output_buffer;
	unsigned int *audio_output_length;
	
	
	struct mp4_asynchronous_buffer video_asynchronous_buffer_0;
	struct mp4_asynchronous_buffer video_asynchronous_buffer_1;
	struct mp4_asynchronous_buffer audio_asynchronous_buffer_0;
	struct mp4_asynchronous_buffer audio_asynchronous_buffer_1;

	struct mp4_asynchronous_buffer *video_current_asynchronous_buffer;
	struct mp4_asynchronous_buffer *video_next_asynchronous_buffer;
	struct mp4_asynchronous_buffer *audio_current_asynchronous_buffer;
	struct mp4_asynchronous_buffer *audio_next_asynchronous_buffer;
};


struct mp4_read_output_struct {
	unsigned int number_of_audio_frames;
	unsigned int number_of_skip_audio_parts;
	unsigned int number_of_audio_parts;

	int first_delay;
	int last_delay;
	
	unsigned int  video_length;
	unsigned int *audio_length;

	void *video_buffer;
	void *audio_buffer;
};

struct mp4_video_read_output_struct {

	unsigned int  video_length;
	void *video_buffer;

};

struct mp4_audio_read_output_struct {
	unsigned int number_of_audio_frames;

	int first_delay;
	int last_delay;
	
	unsigned int *audio_length;

	void *audio_buffer;
};

void mp4_read_safe_constructor(struct mp4_read_struct *p);
void mp4_read_close(struct mp4_read_struct *p);
char *mp4_read_open(struct mp4_read_struct *p, char *s);
char *mp4_read_get(struct mp4_read_struct *p, unsigned int packet, unsigned int audio_stream, struct mp4_read_output_struct *output);
char *mp4_read_get_video(struct mp4_read_struct *p, unsigned int packet, struct mp4_video_read_output_struct *output);
char *mp4_read_get_audio(struct mp4_read_struct *p, unsigned int packet, unsigned int audio_stream, struct mp4_audio_read_output_struct *output);

#endif
