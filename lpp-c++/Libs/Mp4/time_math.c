/*
PMP Mod
Copyright (C) 2006 jonny

Homepage: http://jonny.leffe.dnsalias.com
E-mail:   jonny@leffe.dnsalias.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
heart of av sync, timestamps for av interleaving are calculated here
*/


#include "time_math.h"


void time_math_constructor(struct time_math_struct *t, unsigned long long numerator, unsigned long long denominator)
	{
	unsigned long long v = 1000000;

	t->numerator   = numerator;
	t->denominator = v * denominator;
	}


unsigned long long time_math_get(struct time_math_struct *t, unsigned long long frame_number)
	{
	unsigned long long v = t->denominator * frame_number;

	return(v / t->numerator);
	}


void time_math_interleaving_constructor(struct time_math_interleaving_struct *t, unsigned long long video_numerator, unsigned long long video_denominator, unsigned long long audio_numerator, unsigned long long audio_denominator)
	{
	time_math_constructor(&t->video_frames_per_second, video_numerator, video_denominator);
	time_math_constructor(&t->audio_frames_per_second, audio_numerator, audio_denominator);

	t->video_frame_number = 0;
	t->audio_frame_number = 0;
	t->current_video_time = 0;
	t->next_audio_time    = 0;
	
	t->output_video_frame_number = -1;
	t->output_audio_frame_number = 0;
	t->output_number_of_audio_frames = 0;
	}


void time_math_interleaving_get(struct time_math_interleaving_struct *t)
	{

	t->output_video_frame_number += 1;
	t->output_audio_frame_number += t->output_number_of_audio_frames;
	t->output_number_of_audio_frames = 0;

	unsigned long long last_time = t->current_video_time;

	t->video_frame_number++;
	unsigned long long next_video_time = time_math_get(&t->video_frames_per_second, t->video_frame_number);
	
	t->output_first_delay = 0;

	while (t->next_audio_time < next_video_time)
		{
		if (t->output_number_of_audio_frames == 0)
			{
			t->output_first_delay = t->next_audio_time - last_time;
			}
		
		last_time = t->next_audio_time;

		t->audio_frame_number++;
		t->next_audio_time = time_math_get(&t->audio_frames_per_second, t->audio_frame_number);


		t->output_number_of_audio_frames++;
		}


	t->output_last_delay = next_video_time - last_time;

	t->current_video_time = next_video_time;
	}
