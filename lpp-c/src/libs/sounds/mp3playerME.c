//    Copyright (C) 2007 Sakya
//    sakya_tg@yahoo.it
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
//    CREDITS:
//    This file contains functions to play mp3 files through the PSP's Media Engine.
//    This code is based upon this sample code from ps2dev.org
//    http://forums.ps2dev.org/viewtopic.php?t=8469
//    and the source code of Music prx by joek2100
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "clock.h"
#include "id3.h"
#include "player.h"
#include "mp3playerME.h"

#define THREAD_PRIORITY 12
#define OUTPUT_BUFFER_SIZE	(1152*2*4)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Globals:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MP3ME_threadActive = 0;
int MP3ME_threadExited = 1;
char MP3ME_fileName[262];
static int MP3ME_isPlaying = 0;
int MP3ME_thid = -1;
int MP3ME_audio_channel = 0;
int MP3ME_eof = 0;
struct fileInfo MP3ME_info;
int MP3ME_playingSpeed = 0; // 0 = normal
int MP3ME_volume_boost = 0;
float MP3ME_playingTime = 0;
int MP3ME_volume = 0;
int MP3ME_defaultCPUClock = 40;

//Globals for decoding:
SceUID MP3ME_handle;
unsigned char MP3ME_data_buffer[2889]; //__attribute__((aligned(64)));

static int samplerates[4][3] = 
{
    {11025, 12000, 8000,},//mpeg 2.5
    {0, 0, 0,}, //reserved
    {22050, 24000, 16000,},//mpeg 2
    {44100, 48000, 32000}//mpeg 1
};
static int bitrates[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 };
static int bitrates_v2[] = {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160 };

unsigned long MP3ME_codec_buffer[65]; // __attribute__((aligned(64)));
unsigned char MP3ME_input_buffer[2889]; //__attribute__((aligned(64)));//mp3 has the largest max frame, at3+ 352 is 2176
unsigned char MP3ME_output_buffer[2048*4]; //__attribute__((aligned(64)));//at3+ sample_per_frame*4
int MP3ME_output_index = 0;
unsigned char	OutputBuffer[2][OUTPUT_BUFFER_SIZE],
				*OutputPtrME=OutputBuffer[0];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Private functions:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Seek next valid frame
//NOTE: this function comes from Music prx 0.55 source
//      all credits goes to joek2100.
int SeekNextFrame(SceUID fd)
{
    int offset = 0;
    unsigned char buf[1024];
    unsigned char *pBuffer;
    int i;
    int size = 0;

    offset = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
    sceIoRead(fd, buf, sizeof(buf));
    if (!strncmp((char*)buf, "ID3", 3) || !strncmp((char*)buf, "ea3", 3)) //skip past id3v2 header, which can cause a false sync to be found
    {
        //get the real size from the syncsafe int
        size = buf[6];
        size = (size<<7) | buf[7];
        size = (size<<7) | buf[8];
        size = (size<<7) | buf[9];

        size += 10;

        if (buf[5] & 0x10) //has footer
            size += 10;
    }

    sceIoLseek32(fd, offset, PSP_SEEK_SET); //now seek for a sync
    while(1) 
    {
        offset = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
        size = sceIoRead(fd, buf, sizeof(buf));

        if (size <= 2)//at end of file
            return -1;
    
        if (!strncmp((char*)buf, "EA3", 3))//oma mp3 files have non-safe ints in the EA3 header
        {
            sceIoLseek32(fd, (buf[4]<<8)+buf[5], PSP_SEEK_CUR);
            continue;
        }

        pBuffer = buf;
        for( i = 0; i < size; i++)
        {
            //if this is a valid frame sync (0xe0 is for mpeg version 2.5,2+1)
            if ( (pBuffer[i] == 0xff) && ((pBuffer[i+1] & 0xE0) == 0xE0))
            {
                offset += i;
                sceIoLseek32(fd, offset, PSP_SEEK_SET);
                return offset;
            }
        }
       //go back two bytes to catch any syncs that on the boundary
        sceIoLseek32(fd, -2, PSP_SEEK_CUR);
    } 
}

//Decode thread:
int decodeThread(SceSize args, void *argp){
    int res;
    unsigned char MP3ME_header_buf[4];
    int MP3ME_header;
    int version;
    int bitrate;
    int padding;
    int frame_size;
    int size;
    int total_size;
	int offset = 0;

	sceAudiocodecReleaseEDRAM(MP3ME_codec_buffer); //Fix: ReleaseEDRAM at the end is not enough to play another mp3.
	MP3ME_threadActive = 1;
    MP3ME_threadExited = 0;
    OutputBuffer_flip = 0;
    OutputPtrME = OutputBuffer[0];

    MP3ME_handle = sceIoOpen(MP3ME_fileName, PSP_O_RDONLY, 0777);
    if (MP3ME_handle < 0)
        MP3ME_threadActive = 0;

	//now search for the first sync byte, tells us where the mp3 stream starts
	total_size = sceIoLseek32(MP3ME_handle, 0, PSP_SEEK_END);
	size = total_size;
	sceIoLseek32(MP3ME_handle, 0, PSP_SEEK_SET);
	data_start = ID3v2TagSize(MP3ME_fileName);
	sceIoLseek32(MP3ME_handle, data_start, PSP_SEEK_SET);
    data_start = SeekNextFrame(MP3ME_handle);
    
	if (data_start < 0)
		MP3ME_threadActive = 0;

    size -= data_start;

    memset(MP3ME_codec_buffer, 0, sizeof(MP3ME_codec_buffer));

    if ( sceAudiocodecCheckNeedMem(MP3ME_codec_buffer, 0x1002) < 0 )
        MP3ME_threadActive = 0;

    if ( sceAudiocodecGetEDRAM(MP3ME_codec_buffer, 0x1002) < 0 )
        MP3ME_threadActive = 0;

    getEDRAM = 1;

    if ( sceAudiocodecInit(MP3ME_codec_buffer, 0x1002) < 0 )
        MP3ME_threadActive = 0;

    MP3ME_eof = 0;
	MP3ME_info.framesDecoded = 0;

	while (MP3ME_threadActive){
		while( !MP3ME_eof && MP3ME_isPlaying )
		{
			if ( sceIoRead( MP3ME_handle, MP3ME_header_buf, 4 ) != 4 ){
				MP3ME_isPlaying = 0;
				MP3ME_threadActive = 0;
				continue;
			}

			MP3ME_header = MP3ME_header_buf[0];
			MP3ME_header = (MP3ME_header<<8) | MP3ME_header_buf[1];
			MP3ME_header = (MP3ME_header<<8) | MP3ME_header_buf[2];
			MP3ME_header = (MP3ME_header<<8) | MP3ME_header_buf[3];

			bitrate = (MP3ME_header & 0xf000) >> 12;
			padding = (MP3ME_header & 0x200) >> 9;
			version = (MP3ME_header & 0x180000) >> 19;
			samplerate = samplerates[version][ (MP3ME_header & 0xC00) >> 10 ];

			if ((bitrate > 14) || (version == 1) || (samplerate == 0) || (bitrate == 0))//invalid frame, look for the next one
			{
				data_start = SeekNextFrame(MP3ME_handle);
				if(data_start < 0)
				{
					MP3ME_eof = 1;
					continue;
				}
				size -= (data_start - offset); 
				offset = data_start;
				continue;
			}

			if (version == 3) //mpeg-1
			{
				sample_per_frame = 1152;
				frame_size = 144000*bitrates[bitrate]/samplerate + padding;
				MP3ME_info.instantBitrate = bitrates[bitrate] * 1000;
			}else{
				sample_per_frame = 576;
				frame_size = 72000*bitrates_v2[bitrate]/samplerate + padding;
				MP3ME_info.instantBitrate = bitrates_v2[bitrate] * 1000;
			}

			sceIoLseek32(MP3ME_handle, data_start, PSP_SEEK_SET); //seek back
					   
			size -= frame_size;
			if ( size <= 0)
			{
			   MP3ME_eof = 1;
			   continue;
			}

			//since we check for eof above, this can only happen when the file
			// handle has been invalidated by syspend/resume/usb
			if ( sceIoRead( MP3ME_handle, MP3ME_input_buffer, frame_size ) != frame_size ){
                //Resume from suspend:
                if ( MP3ME_handle >= 0 )
                   sceIoClose(MP3ME_handle);
                MP3ME_handle = sceIoOpen(MP3ME_fileName, PSP_O_RDONLY, 0777);
                if (MP3ME_handle < 0){
                    MP3ME_isPlaying = 0;
                    MP3ME_threadActive = 0;
                    continue;
                }
                size = sceIoLseek32(MP3ME_handle, 0, PSP_SEEK_END);
                sceIoLseek32(MP3ME_handle, offset, PSP_SEEK_SET);
                data_start = offset;
				continue;
			}
			data_start += frame_size;
			offset = data_start;

			MP3ME_codec_buffer[6] = (unsigned long)MP3ME_input_buffer;
			MP3ME_codec_buffer[8] = (unsigned long)MP3ME_output_buffer;
				
			MP3ME_codec_buffer[7] = MP3ME_codec_buffer[10] = frame_size;
			MP3ME_codec_buffer[9] = sample_per_frame * 4;

			res = sceAudiocodecDecode(MP3ME_codec_buffer, 0x1002);

			if ( res < 0 )
			{
				//instead of quitting see if the next frame can be decoded
				//helps play files with an invalid frame
				//we must look for a valid frame, the offset above may be wrong
				data_start = SeekNextFrame(MP3ME_handle);
				if(data_start < 0)
				{
					MP3ME_eof = 1;
					continue;
				}
				size -= (data_start - offset); 
				offset = data_start;
				continue;
			}
            MP3ME_playingTime += (float)sample_per_frame/(float)samplerate;
		    MP3ME_info.framesDecoded++;

            //Output:
			memcpy( OutputPtrME, MP3ME_output_buffer, sample_per_frame*4);
			OutputPtrME += (sample_per_frame * 4);
			if( OutputPtrME + (sample_per_frame * 4) > &OutputBuffer[OutputBuffer_flip][OUTPUT_BUFFER_SIZE])
			{
                audioOutput(MP3ME_volume, OutputBuffer[OutputBuffer_flip]);

				OutputBuffer_flip ^= 1;
				OutputPtrME = OutputBuffer[OutputBuffer_flip];
		        //Check for playing speed:
                if (MP3ME_playingSpeed){
                    long old_start = data_start;
                    sceIoLseek32(MP3ME_handle, data_start + frame_size * 5 * MP3ME_playingSpeed, PSP_SEEK_SET);
                    //MP3ME_playingTime += (float)sample_per_frame/(float)samplerate * 5.0 * (float)MP3ME_playingSpeed;
                    data_start = SeekNextFrame(MP3ME_handle);
    				if(data_start < 0){
    					MP3ME_eof = 1;
    					continue;
    				}
    				float framesSkipped = (float)abs(old_start - data_start) / (float)frame_size;
    				if (MP3ME_playingSpeed > 0)
        				MP3ME_playingTime += framesSkipped * (float)sample_per_frame/(float)samplerate;
                    else
        				MP3ME_playingTime -= framesSkipped * (float)sample_per_frame/(float)samplerate;

    				offset = data_start;
    				size = total_size - data_start;
                }
			}
		}
		sceKernelDelayThread(10000);
	}
    if (getEDRAM)
        sceAudiocodecReleaseEDRAM(MP3ME_codec_buffer);

    if ( MP3ME_handle >= 0)
      sceIoClose(MP3ME_handle);
    MP3ME_threadExited = 1;
    return 0;
}

void getMP3METagInfo(char *filename, struct fileInfo *targetInfo){
    //ID3:
    struct ID3Tag ID3;
    ID3 = ParseID3(filename);
    strcpy(targetInfo->title, ID3.ID3Title);
    strcpy(targetInfo->artist, ID3.ID3Artist);
    strcpy(targetInfo->album, ID3.ID3Album);
    strcpy(targetInfo->year, ID3.ID3Year);
    strcpy(targetInfo->genre, ID3.ID3GenreText);
    strcpy(targetInfo->trackNumber, ID3.ID3TrackText);
    targetInfo->length = ID3.ID3Length / 1000;
}

//Get info on file:
//Uso LibMad per calcolare la durata del pezzo perché
//altrimenti dovrei gestire il buffer anche nella seekNextFrame (senza è troppo lenta).
//E' una porcheria ma è più semplice. :)
int MP3MEgetInfo(){
	unsigned long FrameCount = 0;
    int fd;
    int bufferSize = 1024*512;
    u8 *localBuffer;
    long dataRed = 0;
	mad_timer_t libMadlength;
	struct mad_stream stream;
	struct mad_header header;
    int timeFromID3 = 0;
    
    getMP3METagInfo(MP3ME_fileName, &MP3ME_info);

    int oldClock = getCpuClock();
	mad_stream_init (&stream);
	mad_header_init (&header);
    
    localBuffer = (unsigned char *) malloc(bufferSize);
    fd = sceIoOpen(MP3ME_fileName, PSP_O_RDONLY, 0777);
    if (fd < 0)
        return -1;

    setCpuClock(222);
	long size = sceIoLseek(fd, 0, PSP_SEEK_END);
    sceIoLseek(fd, 0, PSP_SEEK_SET);
    
	int startPos = ID3v2TagSize(MP3ME_fileName);
	sceIoLseek32(fd, startPos, PSP_SEEK_SET);
    startPos = SeekNextFrame(fd);
    size -= startPos;
    
    MP3ME_info.fileType = MP3_TYPE;
    MP3ME_info.defaultCPUClock = MP3ME_defaultCPUClock;
    MP3ME_info.needsME = 1;
	MP3ME_info.fileSize = size;
    MP3ME_info.framesDecoded = 0;
	mad_timer_reset(&libMadlength);
	while (1){
        if (dataRed >= size)
            break;
        memset(localBuffer, 0, bufferSize);
        dataRed += sceIoRead(fd, localBuffer, bufferSize);
    	mad_stream_buffer (&stream, localBuffer, bufferSize);

        while (1){
    		if (mad_header_decode (&header, &stream) == -1){
                if (stream.buffer == NULL || stream.error == MAD_ERROR_BUFLEN)
                    break;
    			else if (MAD_RECOVERABLE(stream.error)){
    				continue;
    			}else{
    				break;
    			}
    		}
    		//Informazioni solo dal primo frame:
    	    if (FrameCount == 0){
    			switch (header.layer) {
    			case MAD_LAYER_I:
    				strcpy(MP3ME_info.layer,"I");
    				break;
    			case MAD_LAYER_II:
    				strcpy(MP3ME_info.layer,"II");
    				break;
    			case MAD_LAYER_III:
    				strcpy(MP3ME_info.layer,"III");
    				break;
    			default:
    				strcpy(MP3ME_info.layer,"unknown");
    				break;
    			}

    			MP3ME_info.kbit = header.bitrate / 1000;
    			MP3ME_info.instantBitrate = header.bitrate;
    			MP3ME_info.hz = header.samplerate;
    			switch (header.mode) {
    			case MAD_MODE_SINGLE_CHANNEL:
    				strcpy(MP3ME_info.mode, "single channel");
    				break;
    			case MAD_MODE_DUAL_CHANNEL:
    				strcpy(MP3ME_info.mode, "dual channel");
    				break;
    			case MAD_MODE_JOINT_STEREO:
    				strcpy(MP3ME_info.mode, "joint (MS/intensity) stereo");
    				break;
    			case MAD_MODE_STEREO:
    				strcpy(MP3ME_info.mode, "normal LR stereo");
    				break;
    			default:
    				strcpy(MP3ME_info.mode, "unknown");
    				break;
    			}

    			switch (header.emphasis) {
    			case MAD_EMPHASIS_NONE:
    				strcpy(MP3ME_info.emphasis,"no");
    				break;
    			case MAD_EMPHASIS_50_15_US:
    				strcpy(MP3ME_info.emphasis,"50/15 us");
    				break;
    			case MAD_EMPHASIS_CCITT_J_17:
    				strcpy(MP3ME_info.emphasis,"CCITT J.17");
    				break;
    			case MAD_EMPHASIS_RESERVED:
    				strcpy(MP3ME_info.emphasis,"reserved(!)");
    				break;
    			default:
    				strcpy(MP3ME_info.emphasis,"unknown");
    				break;
    			}
    			
    			//Check if lenght found in tag info:
                if (MP3ME_info.length > 0){
                    timeFromID3 = 1;
                    break;
                }

            }
    		//Controllo il cambio di sample rate (ma non dovrebbe succedere)
    		if (header.samplerate > MP3ME_info.hz)
      		   MP3ME_info.hz = header.samplerate;

    		//Conteggio frame e durata totale:
    		FrameCount++;
    		mad_timer_add (&libMadlength, header.duration);
		}
	}
	mad_header_finish (&header);
	mad_stream_finish (&stream);
	free(localBuffer);
    sceIoClose(fd);
    
	MP3ME_info.frames = FrameCount;
	//Formatto in stringa la durata totale:
    if (!timeFromID3){
    	MP3ME_info.length = mad_timer_count(libMadlength, MAD_UNITS_SECONDS);
    	mad_timer_string(libMadlength, MP3ME_info.strLength, "%02lu:%02u:%02u", MAD_UNITS_HOURS, MAD_UNITS_MILLISECONDS, 0);
    }else{
        int secs = MP3ME_info.length;
    	int h = secs / 3600;
    	int m = (secs - h * 3600) / 60;
    	int s = secs - h * 3600 - m * 60;
    	snprintf(MP3ME_info.strLength, sizeof(MP3ME_info.strLength), "%2.2i:%2.2i:%2.2i", h, m, s);
    }

    setCpuClock(oldClock);
    return 0;
}
/*int MP3MEgetInfo(){
    unsigned char MP3ME_header_buf[4];
    int MP3ME_header;
    int version;
    int bitrate;
    int padding;
    int frame_size;
    int size;
	int offset = 0;
    float totalLength = 0;

    MP3ME_eof = 0;

    MP3ME_handle = sceIoOpen(MP3ME_fileName, PSP_O_RDONLY, 0777);
    if (MP3ME_handle < 0)
        MP3ME_eof = 1;

	//now search for the first sync byte, tells us where the mp3 stream starts
	size = sceIoLseek32(MP3ME_handle, 0, PSP_SEEK_END);
    MP3ME_info.fileSize = size;

	sceIoLseek32(MP3ME_handle, 0, PSP_SEEK_SET);
	data_start = SeekNextFrame(MP3ME_handle);
	if (data_start < 0)
		MP3ME_eof = 1;

    size -= data_start;

    MP3ME_info.frames = 0;
    while( !MP3ME_eof ){
        if ( sceIoRead( MP3ME_handle, MP3ME_header_buf, 4 ) != 4 ){
            MP3ME_eof = 1;
            continue;
        }

        MP3ME_header = MP3ME_header_buf[0];
        MP3ME_header = (MP3ME_header<<8) | MP3ME_header_buf[1];
        MP3ME_header = (MP3ME_header<<8) | MP3ME_header_buf[2];
        MP3ME_header = (MP3ME_header<<8) | MP3ME_header_buf[3];

        bitrate = (MP3ME_header & 0xf000) >> 12;
        padding = (MP3ME_header & 0x200) >> 9;
        version = (MP3ME_header & 0x180000) >> 19;
        samplerate = samplerates[version][ (MP3ME_header & 0xC00) >> 10 ];

        if ((bitrate > 14) || (version == 1) || (samplerate == 0) || (bitrate == 0))//invalid frame, look for the next one
        {
            data_start = SeekNextFrame(MP3ME_handle);
            if(data_start < 0)
            {
                MP3ME_eof = 1;
                continue;
            }
            size -= (data_start - offset);
            offset = data_start;
            continue;
        }

        if (version == 3) //mpeg-1
        {
            sample_per_frame = 1152;
            frame_size = 144000*bitrates[bitrate]/samplerate + padding;
            MP3ME_info.instantBitrate = bitrates[bitrate];
        }else{
            sample_per_frame = 576;
            frame_size = 72000*bitrates_v2[bitrate]/samplerate + padding;
            MP3ME_info.instantBitrate = bitrates_v2[bitrate];
        }

        sceIoLseek32(MP3ME_handle, data_start, PSP_SEEK_SET); //seek back

        size -= frame_size;
        if ( size <= 0)
        {
           MP3ME_eof = 1;
           continue;
        }

        if (MP3ME_info.frames == 0){
            MP3ME_info.hz = samplerate;
            MP3ME_info.kbit = MP3ME_info.instantBitrate;
            switch (version){
                case 1:
                    strcpy(MP3ME_info.layer,"I");
                    break;
                default:
                    strcpy(MP3ME_info.layer,"III");
                    break;
            }
            strcpy(MP3ME_info.emphasis,"no");
        }
        totalLength += 32.0*36.0/(float)samplerate;
        MP3ME_info.frames++;
    }

    if ( MP3ME_handle )
        sceIoClose(MP3ME_handle);

    //Formatto la durata:
    MP3ME_info.length = (long)totalLength;
    int secs = (int)MP3ME_info.length;
    int hh = secs / 3600;
    int mm = (secs - hh * 3600) / 60;
    int ss = secs - hh * 3600 - mm * 60;
    snprintf(MP3ME_info.strLength, sizeof(MP3ME_info.strLength), "%2.2i:%2.2i:%2.2i", hh, mm, ss);

    return 0;
}*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Public functions:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MP3ME_Init(int channel){
    MP3ME_audio_channel = channel;
	MP3ME_playingSpeed = 0;
    MP3ME_playingTime = 0;
	MP3ME_volume_boost = 0;
	MP3ME_volume = PSP_AUDIO_VOLUME_MAX;
    //MIN_PLAYING_SPEED=-10;
    //MAX_PLAYING_SPEED=9;
    MIN_PLAYING_SPEED=0;
    MAX_PLAYING_SPEED=0;
	initMEAudioModules();
}


int MP3ME_Load(char *fileName){
    initFileInfo(&MP3ME_info);
    strcpy(MP3ME_fileName, fileName);
    if (MP3MEgetInfo() != 0){
        strcpy(MP3ME_fileName, "");
        return ERROR_OPENING;
    }
    
    releaseAudio();
    if (setAudioFrequency(OUTPUT_BUFFER_SIZE/4, MP3ME_info.hz, 2) < 0){
        MP3ME_End();
        return ERROR_INVALID_SAMPLE_RATE;
    }

    MP3ME_thid = -1;
    MP3ME_eof = 0;
    MP3ME_thid = sceKernelCreateThread("decodeThread", decodeThread, THREAD_PRIORITY, 0x10000, PSP_THREAD_ATTR_USER, NULL);
    if(MP3ME_thid < 0)
        return ERROR_CREATE_THREAD;

    sceKernelStartThread(MP3ME_thid, 0, NULL);
    return OPENING_OK;
}

int MP3ME_Play(){
    if(MP3ME_thid < 0)
        return -1;
    MP3ME_isPlaying = 1;
    return 0;
}


void MP3ME_Pause(){
    MP3ME_isPlaying = !MP3ME_isPlaying;
}

int MP3ME_Stop(){
    MP3ME_isPlaying = 0;
    MP3ME_threadActive = 0;
    while (!MP3ME_threadExited)
        sceKernelDelayThread(100000);
    sceKernelDeleteThread(MP3ME_thid);
    return 0;
}

int MP3ME_EndOfStream(){
    return MP3ME_eof;
}

void MP3ME_End(){
    MP3ME_Stop();
}


struct fileInfo MP3ME_GetInfo(){
    return MP3ME_info;
}


int MP3ME_GetPercentage(){
    return (int)(MP3ME_playingTime/(double)MP3ME_info.length*100.0);
}


int MP3ME_getPlayingSpeed(){
	return MP3ME_playingSpeed;
}


int MP3ME_setPlayingSpeed(int playingSpeed){
	if (playingSpeed >= MIN_PLAYING_SPEED && playingSpeed <= MAX_PLAYING_SPEED){
		MP3ME_playingSpeed = playingSpeed;
		if (playingSpeed == 0)
			MP3ME_volume = PSP_AUDIO_VOLUME_MAX;
		else
			MP3ME_volume = FASTFORWARD_VOLUME;
		return 0;
	}else{
		return -1;
	}
}


void MP3ME_setVolumeBoost(int boost){
    MP3ME_volume_boost = boost;
}


int MP3ME_getVolumeBoost(){
    return(MP3ME_volume_boost);
}

void MP3ME_GetTimeString(char *dest){
    char timeString[9];
    int secs = (int)MP3ME_playingTime;
    int hh = secs / 3600;
    int mm = (secs - hh * 3600) / 60;
    int ss = secs - hh * 3600 - mm * 60;
    snprintf(timeString, sizeof(timeString), "%2.2i:%2.2i:%2.2i", hh, mm, ss);
    strcpy(dest, timeString);
}

struct fileInfo MP3ME_GetTagInfoOnly(char *filename){
    struct fileInfo tempInfo;
    initFileInfo(&tempInfo);
    getMP3METagInfo(filename, &tempInfo);
	return tempInfo;
}

int MP3ME_isFilterSupported(){
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Set mute:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MP3ME_setMute(int onOff){
	if (onOff)
    	MP3ME_volume = MUTED_VOLUME;
	else
    	MP3ME_volume = PSP_AUDIO_VOLUME_MAX;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Fade out:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MP3ME_fadeOut(float seconds){
    int i = 0;
    long timeToWait = (long)((seconds * 1000.0) / (float)MP3ME_volume);
    for (i=MP3ME_volume; i>=0; i--){
        MP3ME_volume = i;
        sceKernelDelayThread(timeToWait);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Manage suspend:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MP3ME_suspend(){
    return 0;
}

int MP3ME_resume(){
    return 0;
}

void MP3ME_setVolumeBoostType(char *boostType){
    //Only old method supported
    //MAX_VOLUME_BOOST = 4;
    MAX_VOLUME_BOOST = 0;
    MIN_VOLUME_BOOST = 0;
}
//TODO:

int MP3ME_GetStatus(){
    return 0;
}


int MP3ME_setFilter(double tFilter[32], int copyFilter){
    return 0;
}

void MP3ME_enableFilter(){
}

void MP3ME_disableFilter(){
}

int MP3ME_isFilterEnabled(){
    return 0;
}

