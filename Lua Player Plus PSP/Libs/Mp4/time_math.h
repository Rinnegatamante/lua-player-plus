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


#ifndef time_math_h
#define time_math_h


// 128 is ok with audio_numerator == 44100 && audio_denominator == 1152 && !(video_rate < video_scale)
// maximum real value is 44100 / 1152
#define maximum_audio_frames_between_video_frames 128


struct time_math_struct
	{
	unsigned long long numerator;
	unsigned long long denominator;
	};


struct time_math_interleaving_struct
	{
	struct time_math_struct video_frames_per_second;
	struct time_math_struct audio_frames_per_second;

	unsigned int video_frame_number;
	unsigned int audio_frame_number;

	unsigned long long current_video_time;
	unsigned long long next_audio_time;


	unsigned int output_video_frame_number;
	unsigned int output_audio_frame_number;

	unsigned int output_number_of_audio_frames;

	unsigned int output_first_delay;
	unsigned int output_last_delay;
	};


void time_math_constructor(struct time_math_struct *t, unsigned long long numerator, unsigned long long denominator);
unsigned long long time_math_get(struct time_math_struct *t, unsigned long long frame_number);


void time_math_interleaving_constructor(struct time_math_interleaving_struct *t, unsigned long long video_numerator, unsigned long long video_denominator, unsigned long long audio_numerator, unsigned long long audio_denominator);
void time_math_interleaving_get(struct time_math_interleaving_struct *t);


#endif
